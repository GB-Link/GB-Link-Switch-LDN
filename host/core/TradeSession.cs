using System.Diagnostics;
using System.Text;
using System.Text.Json;

namespace Frlg.Trade.Core;

public sealed class TradeSession(Action<object> emit)
{
    public const ulong FireRedId = 0x01006fa0233f8000;
    private void Phase(string message) => emit(new { @event = "phase", message });
    private void Log(string message) => emit(new { @event = "log", message });
    private static string Text(SerialFrame frame) => Encoding.UTF8.GetString(frame.Payload);
    private static LdnNetwork? Advertisement(SerialFrame frame, KeyFile keys)
    {
        if (frame.Kind != 3) return null;
        var fields = Text(frame).Split(' ');
        if (fields.Length != 4 || fields[0] != "LDN_ADV") return null;
        try { return LdnKeys.Decode(keys, Bin.Hex(fields[3]), Bin.Hex(fields[1].Replace(":", "")), int.Parse(fields[2])); }
        catch (Exception e) when (e is InvalidDataException or FormatException or Org.BouncyCastle.Crypto.InvalidCipherTextException or System.Security.Cryptography.CryptographicException) { return null; }
    }
    public void Run(string port, byte[]?[] party, int selected, string runPath, CancellationToken cancel)
    {
        using var timing = new SessionTiming();
        var engine = new TradeEngine(party, selected);
        var keys = KeyFile.LoadDefault();
        Directory.CreateDirectory(runPath); emit(new { @event = "run", path = runPath });
        using var log = new StreamWriter(Path.Combine(runPath, "native.log")) { AutoFlush = true };
        void Record(string text) { log.WriteLine($"{DateTimeOffset.Now:O} {text}"); Log(text); }
        engine.Log += Record;
        engine.OpponentParty += (values, name) => emit(new { @event = "opponent_party", name, party = values.Select(v => v == null ? null : Convert.ToHexString(v)).ToArray() });
        engine.Committed += (data, slot) =>
        {
            // Save at commit, before UI notification or the room-exit handshake.
            string temp = Path.Combine(runPath, "received.pk3.tmp"), target = Path.Combine(runPath, "received.pk3");
            File.WriteAllBytes(temp, data); File.Move(temp, target, true);
            emit(new { @event = "received", slot, pk3 = Convert.ToHexString(data) }); Record($"Trade committed; received {TradeEngine.Parse(data).Nickname}");
        };
        Phase("Identifying the serial device");
        using var device = new SerialDevice(port, cancel);
        bool started = false;
        try
        {
            device.Handshake(); started = true; Record($"Protocol v1 device: {device.Model}");
            emit(new { @event = "device", model = device.Model });
            Phase("Searching for a Leader room");
            var scanTime = Stopwatch.StartNew(); LdnNetwork? network = null;
            int[] channels = [1, 6, 11, 2, 3, 4, 5, 7, 8, 9, 10];
            while (network == null && scanTime.Elapsed.TotalSeconds < 25)
            {
                foreach (int channel in channels)
                {
                    cancel.ThrowIfCancellationRequested(); device.Command($"LDN_SCAN {channel}");
                    var dwell = Stopwatch.StartNew();
                    while (dwell.ElapsedMilliseconds < 500 && network == null)
                    {
                        foreach (var frame in device.Drain())
                        {
                            var room = Advertisement(frame, keys);
                            if (room?.CommunicationId == FireRedId && room.Policy != 1 && room.Members.Count < room.Maximum) { network = room; break; }
                        }
                        Thread.Sleep(2);
                    }
                    if (network != null || scanTime.Elapsed.TotalSeconds >= 25) break;
                }
            }
            if (network == null) throw new ConnectionException("No joinable FireRed Leader room found; make sure the host is still waiting.");
            var derived = new LdnKeys(keys, network.Protocol); var auth = new LdnAuthentication(network, derived);
            Record($"Room verified: protocol={network.Protocol} version={network.Version} channel={network.Channel}");
            Phase("Associating and authenticating with the room");
            device.Command($"LDN_CONFIG {network.Channel} {Convert.ToHexString(network.Ssid).ToLowerInvariant()} {Bin.Mac(network.Host)} {Convert.ToHexString(derived.Data(network))}", 8);
            byte[]? ourMac = null; LdnNetwork? membership = null; bool accepted = false;
            var joinTime = Stopwatch.StartNew(); double nextStatus = 0, nextAuth = double.PositiveInfinity; int attempts = 0;
            while (joinTime.Elapsed.TotalSeconds < 40 && !(accepted && membership != null))
            {
                double now = joinTime.Elapsed.TotalSeconds;
                if (now >= nextStatus)
                {
                    foreach (string line in device.Command("LDN_STATUS"))
                        if (line.StartsWith("LDN_LINK 1 ") && ourMac == null) { ourMac = Bin.Hex(line.Split(' ')[2].Replace(":", "")); nextAuth = now; }
                    nextStatus = now + 2;
                }
                if (now >= nextAuth && !accepted && attempts < 3)
                { device.Command("LDN_TX " + Convert.ToHexString(auth.Request)); nextAuth = now + 0.7; attempts++; Record($"LDN authentication request {attempts}"); }
                foreach (var frame in device.Drain())
                {
                    string text = frame.Kind == 3 ? Text(frame) : "";
                    if (text.StartsWith("LDN_LINK 1 ") && ourMac == null) { ourMac = Bin.Hex(text.Split(' ')[2].Replace(":", "")); nextAuth = now; }
                    if (text.StartsWith("LDN_ERROR ")) throw new ConnectionException(text);
                    if (text.StartsWith("LDN_RX "))
                    {
                        var fields = text.Split(' ');
                        if (fields.Length != 3 || !Bin.Hex(fields[1].Replace(":", "")).AsSpan().SequenceEqual(network.Host)) continue;
                        auth.Accept(Bin.Hex(fields[2])); accepted = true; Record("LDN response and challenge verified");
                    }
                    var room = Advertisement(frame, keys);
                    if (room != null && room.Host.AsSpan().SequenceEqual(network.Host))
                    {
                        VerifySameRoom(network, room);
                        if (ourMac != null && room.Members.Any(m => m.Mac.AsSpan().SequenceEqual(ourMac))) membership = room;
                    }
                }
                Thread.Sleep(2);
            }
            if (ourMac == null || !accepted || membership == null) throw new ConnectionException("Room authentication timed out; check the Leader room and retry.");
            var ours = membership.Members.Single(m => m.Mac.AsSpan().SequenceEqual(ourMac));
            var host = membership.Members.Single(m => m.Index == 0);
            ValidateMembers(membership, ours, host);
            device.Command($"LDN_NET {ours.Ip} {host.Ip}");
            foreach (var m in membership.Members.Where(m => m.Index != ours.Index)) device.Command($"LDN_NEIGH {m.Ip} {Convert.ToHexString(m.Mac)}");
            Record($"Network ready: serialBad={device.BadFrames}");
            var peers = membership.Members.Where(m => m.Index != ours.Index).ToDictionary(m => m.Ip, m => m.Mac);
            emit(new { @event = "connected" });
            Phase("Joined the room, waiting for the Leader");
            using var capture = new StreamWriter(Path.Combine(runPath, "pia.jsonl")) { AutoFlush = true };
            capture.WriteLine(JsonSerializer.Serialize(new { rec = "meta", ip = ours.Ip, host = host.Ip, ssid_hex = Convert.ToHexString(network.Ssid) }));
            var clock = Stopwatch.StartNew();
            void Capture(string dir, byte[] data, string source, string destination) => capture.WriteLine(JsonSerializer.Serialize(new { rec = "pkt", t = clock.Elapsed.TotalSeconds, dir, src = source + ":12345", dst = destination + ":12345", hex = Convert.ToHexString(data) }));
            using var simulator = new Simulator(network.Ssid, ourMac, network.Host, ours.Ip, host.Ip, engine,
                (data, destination) => { device.SendDatagram(data, destination); Capture("out", data, ours.Ip, destination); });
            simulator.Log += Record;
            double nextPing = 0, nextTick = 0, lastReceive = 0, closeAt = double.PositiveInfinity, leaveAt = double.PositiveInfinity;
            bool closeSeen = false, doneSeen = false;
            while (!simulator.HostDisconnected)
            {
                cancel.ThrowIfCancellationRequested(); double now = clock.Elapsed.TotalSeconds;
                if (now >= nextPing) { device.Command("LDN_PING", 5); nextPing = now + 1; }
                var frames = device.Drain();
                // The reason arrives in the same batch as LINK 0; keep it in the log before failing.
                foreach (var frame in frames) if (frame.Kind == 3 && Text(frame).StartsWith("LDN_DISCONNECTED ")) Record(Text(frame));
                foreach (var frame in frames)
                {
                    if (frame.Kind == 5 && frame.Payload.Length >= 4)
                    {
                        string source = new System.Net.IPAddress(frame.Payload[..4]).ToString();
                        if (!peers.ContainsKey(source)) throw new ConnectionException("Received data from an unauthenticated room member.");
                        var data = frame.Payload[4..]; Capture("in", data, source, ours.Ip);
                        int before = simulator.ReceivedPackets; simulator.Receive(data, source);
                        if (before != simulator.ReceivedPackets) lastReceive = now;
                    }
                    else if (frame.Kind == 3)
                    {
                        string text = Text(frame);
                        if (text.StartsWith("LDN_ERROR ") || text.StartsWith("LDN_LINK 0 ") || text.StartsWith("LDN_NET_LOST") || text.StartsWith("LDN_UDP_ERROR "))
                            throw new ConnectionException("Wireless connection lost: " + text);
                        var room = Advertisement(frame, keys);
                        if (room != null && room.Host.AsSpan().SequenceEqual(network.Host))
                        {
                            VerifySameRoom(network, room); ValidateMembers(room, ours, host);
                            var current = room.Members.Where(m => m.Index != ours.Index).ToDictionary(m => m.Ip, m => m.Mac);
                            foreach (string ip in peers.Keys.Except(current.Keys).ToArray()) device.Command($"LDN_FORGET {ip}");
                            foreach (var (ip, mac) in current) if (!peers.TryGetValue(ip, out var previous) || !mac.AsSpan().SequenceEqual(previous)) device.Command($"LDN_NEIGH {ip} {Convert.ToHexString(mac)}");
                            peers = current;
                        }
                    }
                }
                if (now >= nextTick) { simulator.Tick(); nextTick = now + 1 / 59.727; }
                if (engine.Barrier.Mode == 2 && !closeSeen) { closeSeen = true; closeAt = now + 1.5; }
                if (engine.Done && !doneSeen) { doneSeen = true; leaveAt = now + 120; Phase("Trade finished, waiting for the Leader to leave the room"); }
                if (now >= closeAt || now >= leaveAt) break;
                if (now - lastReceive > 30) throw new ConnectionException("Host communication timed out; the connection was closed.");
                Thread.Sleep(1);
            }
            Record($"Link closed: rx={simulator.ReceivedPackets} decryptFailed={simulator.DecryptFailures} tx={simulator.SentPackets} serialBad={device.BadFrames}");
        }
        finally
        {
            // Cancellation does not require a board-specific reset or an external process.
            if (started) { try { device.Stop(); } catch (Exception error) { Record("Stop acknowledgement unavailable: " + error.Message); } }
        }
    }
    private static void VerifySameRoom(LdnNetwork previous, LdnNetwork current)
    {
        if (!current.Id.AsSpan().SequenceEqual(previous.Id) || !current.Random.AsSpan().SequenceEqual(previous.Random))
            throw new ConnectionException("The Leader room has changed; reconnect.");
    }
    private static void ValidateMembers(LdnNetwork room, LdnMember ours, LdnMember host)
    {
        if (!room.Members.Any(m => m.Ip == ours.Ip && m.Mac.AsSpan().SequenceEqual(ours.Mac)) ||
            !room.Members.Any(m => m.Index == 0 && m.Ip == host.Ip && m.Mac.AsSpan().SequenceEqual(host.Mac)) ||
            room.Members.Select(m => m.Ip).Distinct().Count() != room.Members.Count)
            throw new ConnectionException("Room membership changed; the current connection is no longer valid.");
        var prefix = ours.Ip[..(ours.Ip.LastIndexOf('.') + 1)];
        if (!prefix.StartsWith("169.254.") || room.Members.Any(m => !m.Ip.StartsWith(prefix) || m.Ip.EndsWith(".0") || m.Ip.EndsWith(".255")))
            throw new ConnectionException("The room assigned an invalid link-local address.");
    }
}
