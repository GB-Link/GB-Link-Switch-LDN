namespace Frlg.Trade.Core;

// PiaLink whose adapter payloads come from and go to a real GBA behind a GB-Link adapter: the host's WT
// frames become HOST_SEND payloads and the GBA's CLIENT_SEND payloads become WT frames, byte for byte.
//
// The child->parent frames must be delivered in order with NO drops: each carries a mod-8 sequence tag
// the parent validates (+1 mod 8), so a single dropped or reordered frame desyncs the trade and the
// Switch disconnects a few frames later. So this queue is lossless. Exact consecutive duplicates are a
// genuine RFU retransmit and collapse to one; when the GBA has produced nothing new, the current frame
// is repeated with its original timestamp (the Switch dedups it by timestamp), which is how "no new
// data" is signalled on the real link.
public sealed class RelayLink(byte[] ssid, byte[] ourMac, byte[] hostMac, string ourIp, string hostIp, Action<byte[], string> send)
    : PiaLink(ssid, ourMac, hostMac, ourIp, hostIp, send)
{
    private readonly Queue<byte[]> outbound = [];
    private byte[]? lastEnqueued, currentWrapped;
    public Action<byte[]>? HostPayload { get; set; }
    public bool ConnectRequested { get; set; }
    public int Coalesced { get; private set; }
    public int Repeated { get; private set; }
    public int HighWater { get; private set; }
    public int Overflow { get; private set; }
    public int Pending { get { lock (outbound) return outbound.Count; } }
    protected override bool WantConnect => ConnectRequested;
    // Host framing: byte 8 is the adapter send length; the payload is trimmed of trailing zeros, so zero-extend.
    public static byte[] HostPayloadOf(byte[] frame)
    {
        int length = frame[8] & 0x7F; var payload = new byte[length];
        frame.AsSpan(12, Math.Max(0, Math.Min(length, frame.Length - 12))).CopyTo(payload);
        return payload;
    }
    protected override void Deliver(byte[] frame) => HostPayload?.Invoke(HostPayloadOf(frame));
    protected override byte[]? Next()
    {
        lock (outbound)
        {
            if (outbound.TryDequeue(out var payload)) { currentWrapped = Rfu.Wrap(payload, NextTime()); return currentWrapped; }
            if (currentWrapped == null) return null;
            Repeated++; return currentWrapped;   // same bytes, same timestamp: a retransmit the Switch dedups
        }
    }
    public void Enqueue(byte[] payload)
    {
        lock (outbound)
        {
            if (lastEnqueued != null && lastEnqueued.AsSpan().SequenceEqual(payload)) { Coalesced++; return; }
            outbound.Enqueue(payload); lastEnqueued = payload;
            if (outbound.Count > HighWater) HighWater = outbound.Count;
            // A cap only as a last resort against a genuinely wedged drain; dropping here would desync, so
            // it is loud and should never fire in a balanced session.
            if (outbound.Count > 512) { outbound.Dequeue(); Overflow++; }
        }
    }
}
