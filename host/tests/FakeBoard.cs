using System.Diagnostics;
using System.Text;
using Frlg.Trade.Core;

// Minimal firmware console: text mode until LDN_BINARY, bridge running until LDN_BRIDGE_STOP, radio commands
// refused while it runs. BootMs simulates a chip that was reset by the port opening.
public sealed class FakeBoard : ISerialLink
{
    public string? Version { get; init; } = "2.0.0";   // null: pre-2.0 firmware, no LDN_INFO
    public int BootMs { get; init; }
    public bool BridgeRunning { get; private set; } = true;
    public List<string> Commands { get; } = [];

    private readonly Stopwatch uptime = Stopwatch.StartNew();
    private readonly Queue<byte> outgoing = new();
    private readonly List<byte> incoming = [];
    private readonly StringBuilder text = new();
    private bool binary, announced;
    private uint session;

    private bool Booted => uptime.ElapsedMilliseconds >= BootMs;

    public int BytesToRead
    {
        get
        {
            if (Booted && !announced)
            {
                announced = true;
                foreach (byte b in Encoding.ASCII.GetBytes("rst:0x1 (POWERON_RESET)\r\nLDN_READY chip=esp32s3 transport=test heap=1\n")) outgoing.Enqueue(b);
            }
            return outgoing.Count;
        }
    }

    public int Read(byte[] buffer, int offset, int count)
    {
        int read = 0;
        while (read < count && outgoing.Count > 0) buffer[offset + read++] = outgoing.Dequeue();
        return read;
    }

    public void DiscardInBuffer() => outgoing.Clear();
    public void Dispose() { }

    public void Write(byte[] buffer, int offset, int count)
    {
        if (!Booted) return;
        for (int i = offset; i < offset + count; i++)
        {
            byte value = buffer[i];
            if (!binary)
            {
                if (value == '\n') { if (text.ToString() == "LDN_BINARY") binary = true; text.Clear(); }
                else if (value != 0) text.Append((char)value);
                continue;
            }
            if (value != 0) { incoming.Add(value); continue; }
            var frame = incoming.Count == 0 ? null : SerialCodec.Decode(incoming.ToArray());
            incoming.Clear();
            if (frame is { Kind: 1 }) Handle(frame);
        }
    }

    private void Handle(SerialFrame frame)
    {
        string command = Encoding.ASCII.GetString(frame.Payload);
        Commands.Add(command.Split(' ')[0]);
        void Reply(string line)
        {
            foreach (byte b in SerialCodec.Encode(new(2, frame.Request, session, Encoding.ASCII.GetBytes(line)))) outgoing.Enqueue(b);
        }
        if (frame.Session != session && command != "LDN_HELLO" && !command.StartsWith("LDN_BEGIN ")) Reply("LDN_ERROR STALE_SESSION");
        else if (command == "LDN_HELLO") { session = 0; Reply("LDN_HELLO 1 esp32s3 dynamic-session,scan,auth,udp 1472"); }
        else if (command.StartsWith("LDN_BEGIN ")) { session = Convert.ToUInt32(command[10..], 16); Reply("LDN_BEGUN"); }
        else if (command == "LDN_INFO" && Version != null) Reply($"LDN_INFO frlg-ldn-bridge {Version} chip=esp32s3 transport=USB Serial/JTAG");
        else if (command == "LDN_BRIDGE_STOP" && Version != null) { BridgeRunning = false; Reply("LDN_BRIDGE_STOPPED"); }
        else if (command == "LDN_BRIDGE_START" && Version != null) { BridgeRunning = true; Reply("LDN_BRIDGE_STARTED"); }
        else if (command.StartsWith("LDN_SCAN ")) Reply(BridgeRunning && Version != null ? "LDN_ERROR BRIDGE_OWNS_RADIO" : "LDN_SCAN_RESULT 0");
        else if (command == "LDN_STOP") Reply("LDN_STOPPED");
        else Reply("LDN_ERROR UNKNOWN_COMMAND");
        Reply("LDN_DONE");
    }
}
