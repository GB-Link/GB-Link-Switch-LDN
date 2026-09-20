// Installs the bridge firmware on an ESP32 over WebSerial with esptool-js. The chip is
// identified first and the matching images are taken from the manifest, so the same
// button serves every supported board. The three images are written separately, which
// leaves the stored keys (in the NVS partition between them) in place across updates.

import { ESPLoader, Transport } from '../vendor/esptool-js/bundle.js';
import { md5 } from './md5.js';
import { fetchBytes } from './manifest.js';

const ESPRESSIF_VENDOR_ID = 0x303a;

// port: a closed SerialPort. Resolves with { chip, version } once the board has been
// reset into the new firmware; the port is closed again either way.
export async function flashBridge(port, manifest, { onStatus = () => {}, onProgress = () => {}, onLog = () => {}, eraseAll = false } = {}) {
    const terminal = {
        clean() {},
        write() {},
        writeLine(text) { if (text && !text.startsWith('Writing at')) onLog(text); },
    };
    // The rate means nothing on the chip's own USB port, and asking for a change there
    // only closes and reopens it; a UART bridge is worth speeding up.
    const native = safeInfo(port).usbVendorId === ESPRESSIF_VENDOR_ID;
    const transport = new Transport(port, false);
    const loader = new ESPLoader({ transport, baudrate: native ? 115200 : 460800, romBaudrate: 115200, terminal, debugLogging: false });
    try {
        onStatus('Connecting to the chip…');
        await loader.main();
        const chip = loader.chip.CHIP_NAME;
        const entry = manifest.bridge.chips[chip];
        if (!entry) throw new Error(`There is no bridge firmware for the ${chip}.`);

        onStatus(`Found an ${chip}. Downloading firmware ${manifest.bridge.version}…`);
        const files = [];
        for (const part of entry.parts) files.push({ address: part.address, data: await fetchBytes(manifest.base + part.path) });

        const sizes = files.map((file) => file.data.length);
        const total = sizes.reduce((sum, size) => sum + size, 0);
        const flashSize = await loader.detectFlashSize();
        onStatus(eraseAll ? 'Erasing the whole flash, then writing…' : 'Writing… do not unplug the board.');
        await loader.writeFlash({
            fileArray: files,
            flashSize,
            flashMode: 'keep',
            flashFreq: 'keep',
            eraseAll,
            compress: true,
            calculateMD5Hash: md5,
            reportProgress: (index, written, length) => {
                const before = sizes.slice(0, index).reduce((sum, size) => sum + size, 0);
                onProgress((before + sizes[index] * (length ? written / length : 1)) / total);
            },
        });
        onStatus('Written and verified. Restarting the board…');
        await hardReset(transport);
        return { chip, version: manifest.bridge.version };
    } finally {
        try { await transport.disconnect(); } catch {}
    }
}

// RTS asserted with DTR released pulls the chip's reset line, through the auto-reset
// circuit of a dev board or its emulation in the USB Serial/JTAG peripheral. The
// loader's own hard reset (esptool-js 0.6.1) only ever releases RTS, which leaves the
// chip sitting in the flasher stub.
async function hardReset(transport) {
    await transport.setDTR(false);
    await transport.setRTS(true);
    await new Promise((resolve) => setTimeout(resolve, 100));
    await transport.setRTS(false);
}

function safeInfo(port) {
    try { return port?.getInfo?.() ?? {}; } catch { return {}; }
}
