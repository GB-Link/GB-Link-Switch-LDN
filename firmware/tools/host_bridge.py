"""Bridge a GB-Link on USB to the bridge firmware on USB, through this computer.

A reference for the web client's bridge mode and a way to test it without a browser:
speaks the console's binary protocol to the ESP, asks for the adapter on the host
port, and pipes GB frames between the ESP (kinds 6 and 7) and the GB-Link's serial
port. Each direction is forwarded by its own thread as soon as bytes arrive. The
firmware restarts itself when a session ends; the bridge reconnects and carries on.

Needs pyserial. Usage: host_bridge.py [seconds] [logfile]
"""
import sys, time, glob, zlib, struct, threading, serial

ESP_GLOB = "/dev/serial/by-id/*Espressif*"
PICO_GLOB = "/dev/serial/by-id/*ZEPHYR*"
T0 = time.time()


def cobs_encode(data):
    out = bytearray(); idx = 0
    while True:
        end = data.find(b"\x00", idx)
        block = data[idx:] if end < 0 else data[idx:end]
        while len(block) >= 254:
            out.append(255); out += block[:254]; block = block[254:]
        out.append(len(block) + 1); out += block
        if end < 0: break
        idx = end + 1
    return bytes(out)


def cobs_decode(data):
    out = bytearray(); i = 0
    while i < len(data):
        code = data[i]; i += 1
        if code == 0 or i + code - 1 > len(data): return None
        out += data[i:i + code - 1]; i += code - 1
        if code != 255 and i < len(data): out.append(0)
    return bytes(out)


class Log:
    def __init__(self, path):
        self.file = open(path, "w", buffering=1) if path else None
        self.lock = threading.Lock()
    def __call__(self, text):
        line = f"{time.time() - T0:8.2f} {text}"
        with self.lock:
            print(line, flush=True)
            if self.file: self.file.write(line + "\n")


class Esp:
    """The console's binary protocol: COBS frames, CRC-32, request ids, a session."""
    def __init__(self, port, log):
        self.log = log
        self.s = serial.Serial(); self.s.port = port; self.s.baudrate = 115200; self.s.timeout = 0.2
        self.s.dtr = True; self.s.rts = False; self.s.open(); self.s.dtr = False   # without resetting the chip
        self.session = 0; self.request = 0
        self.write_lock = threading.Lock(); self.lines = []; self.lines_lock = threading.Lock()
        self.on_adapter = None; self.alive = True; self.to_adapter = 0
        self.rebooted = False; self.tail = b""
    def send(self, kind, payload, request=0):
        raw = struct.pack("<BBIIH", 1, kind, request, self.session, len(payload)) + payload
        raw += struct.pack("<I", zlib.crc32(raw) & 0xffffffff)
        with self.write_lock: self.s.write(cobs_encode(raw) + b"\x00")
    def reader(self):
        buf = bytearray()
        try:
            while self.alive:
                data = self.s.read(1)
                if not data: continue
                data += self.s.read(self.s.in_waiting or 0)
                # A software restart leaves the USB port open, so the boot banner is the
                # only sign that the chip is back in text mode and needs a new handshake.
                if b"LDN_READY" in self.tail + data: self.rebooted = True
                self.tail = data[-16:]
                buf += data
                while b"\x00" in buf:
                    chunk, _, rest = bytes(buf).partition(b"\x00"); buf = bytearray(rest)
                    raw = cobs_decode(chunk) if chunk else None
                    if not raw or len(raw) < 16 or raw[0] != 1: continue
                    if struct.unpack("<I", raw[-4:])[0] != (zlib.crc32(raw[:-4]) & 0xffffffff): continue
                    kind, payload = raw[1], raw[12:-4]
                    if kind == 6:
                        self.to_adapter += 1
                        if self.on_adapter: self.on_adapter(payload)
                    elif kind in (2, 3):
                        text = payload.decode("utf-8", "replace").strip()
                        with self.lines_lock: self.lines.append(text)
                        if kind == 3: self.log("ESP  " + text)
        except (serial.SerialException, OSError):
            pass
        self.alive = False
    def command(self, text, wait=1.5):
        self.request += 1
        with self.lines_lock: start = len(self.lines)
        self.send(1, text.encode(), self.request)
        deadline = time.time() + wait
        while time.time() < deadline and self.alive:
            with self.lines_lock: got = self.lines[start:]
            if "LDN_DONE" in got: return [l for l in got if l != "LDN_DONE"]
            time.sleep(0.01)
        with self.lines_lock: return [l for l in self.lines[start:] if l != "LDN_DONE"]
    def close(self):
        self.alive = False
        try: self.s.close()
        except Exception: pass


def wait_for(pattern, seconds):
    deadline = time.time() + seconds
    while time.time() < deadline:
        found = glob.glob(pattern)
        if found: return found[0]
        time.sleep(0.2)
    return None


def main():
    seconds = float(sys.argv[1]) if len(sys.argv) > 1 else 600
    log = Log(sys.argv[2] if len(sys.argv) > 2 else None)
    pico = serial.Serial(wait_for(PICO_GLOB, 10), 115200, timeout=0.2)
    pico.reset_input_buffer()
    stats = {"from_pico": 0, "to_pico": 0}
    holder = {"esp": None}

    def pico_reader():
        while time.time() - T0 < seconds:
            try:
                data = pico.read(1)
                if not data: continue
                data += pico.read(pico.in_waiting or 0)
            except (serial.SerialException, OSError):
                log("GB-Link port lost"); return
            esp = holder["esp"]
            if esp and esp.alive:
                stats["from_pico"] += len(data)
                try:
                    for o in range(0, len(data), 1024): esp.send(7, data[o:o + 1024])
                except (serial.SerialException, OSError): pass
    threading.Thread(target=pico_reader, daemon=True).start()

    def to_pico(frame):
        stats["to_pico"] += 1
        try: pico.write(frame)
        except (serial.SerialException, OSError): pass

    while time.time() - T0 < seconds:
        port = wait_for(ESP_GLOB, 15)
        if not port: log("ESP not found"); break
        try: esp = Esp(port, log)
        except (serial.SerialException, OSError): time.sleep(0.5); continue
        threading.Thread(target=esp.reader, daemon=True).start()

        def attach():
            """Binary mode, a session, and the adapter on the host port."""
            esp.rebooted = False
            time.sleep(0.3)
            with esp.write_lock: esp.s.write(b"\nLDN_BINARY\n\x00")
            time.sleep(0.4)
            if not esp.command("LDN_HELLO"): return False
            esp.session = (int(time.time() * 1000) & 0xffffffff) | 1
            esp.command(f"LDN_BEGIN {esp.session:08x}")
            info = esp.command("LDN_INFO")
            esp.on_adapter = to_pico
            holder["esp"] = esp
            took = esp.command("LDN_ADAPTER host")
            log(f"attached: {info} {took}" if took else "attach failed")
            return bool(took)

        attached = attach()
        last = time.time(); silent = 0
        while esp.alive and time.time() - T0 < seconds:
            time.sleep(0.25)
            if esp.rebooted:
                log("ESP restarted (it does when a session ends); attaching again")
                time.sleep(1.5)
                attached = attach()
                continue
            if not attached:
                time.sleep(1.0); attached = attach(); continue
            if time.time() - last >= 15:
                last = time.time()
                status = esp.command("LDN_BRIDGE_STATUS", 1.0)
                brief = ""
                if status:
                    silent = 0
                    f = dict(p.split("=", 1) for p in status[0].split() if "=" in p)
                    brief = " ".join(f"{k}={f.get(k)}" for k in ("state", "conn_state", "child", "pia_rx", "to_gba", "from_gba", "queued", "shed", "overflow"))
                else:
                    silent += 1
                    if silent >= 2: attached = False; silent = 0
                log(f"pipe ESP->GB-Link frames={stats['to_pico']} GB-Link->ESP bytes={stats['from_pico']} | {brief}")
        holder["esp"] = None
        esp.close()
        if time.time() - T0 < seconds: log("ESP went away (it restarts when a session ends); reconnecting")
        time.sleep(1.0)
    log("done")


if __name__ == "__main__":
    main()
