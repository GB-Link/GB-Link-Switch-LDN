using Frlg.Trade.Core;

// Plays the link leader for a TradeEngine: requests blocks, echoes fragments as the RFU parent does, and
// records the blocks and link commands the engine sends.
public sealed class EngineBench
{
    public TradeEngine Engine { get; }
    public List<(int Command, int Cursor)> Sent { get; } = [];
    public List<byte[]> Blocks { get; } = [];
    public List<(string Nickname, int Slot)> Received { get; } = [];
    private byte[] echo = new byte[14];
    private byte[] block = [];
    private uint seen;
    private bool inBlock, noted;

    public EngineBench(byte[]?[] party, int offered, params byte[][] hostParty)
    {
        Engine = new TradeEngine(party, offered) { AnimationFrames = 5 };
        Engine.Committed += (data, slot) => Received.Add((TradeEngine.Parse(data).Nickname, slot));
        Step(Slot(0xa100, 1)); Run(40);   // player block
        Open(hostParty);
    }

    // Menu setup, at the start and after every trade: three party blocks each way, then the ribbons block.
    public void Open(params byte[][] hostParty)
    {
        for (int i = 0; i < 3; i++) { Step(Slot(0xa100, 1)); Run(40); }
        for (int i = 0; i < 3; i++)
        {
            var data = new byte[204];
            for (int k = 0; k < 2; k++) if (i * 2 + k < hostParty.Length) hostParty[i * 2 + k].CopyTo(data, k * 100);
            Engine.HostBlock(17, data);
        }
        Engine.HostBlock(4, new byte[48]);
        Run(10);
    }

    // SetMons, both confirmations, Start, then the finish handshake.
    public void Trade(int hostCursor)
    {
        Host(TradeEngine.SetMons, hostCursor); Run(30);
        Host(TradeEngine.InitBlock, hostCursor); Host(TradeEngine.Start); Run(Engine.AnimationFrames + 40);
        Host(TradeEngine.ReadyFinish); Host(TradeEngine.ConfirmFinish); Run(5);
    }

    public void Host(int command, int cursor = 0) => Engine.HostBlock(2, TradeEngine.LinkCommand(command, cursor));
    public void Run(int steps) { for (int i = 0; i < steps; i++) Step(); }

    private static byte[] Slot(params int[] words)
    {
        var slot = new byte[14];
        for (int i = 0; i < words.Length; i++) Bin.W16(slot, i * 2, words[i]);
        return slot;
    }

    private void Step(byte[]? host = null)
    {
        Engine.Feed([host ?? new byte[14], echo, new byte[14], new byte[14], new byte[14]]);
        var words = Engine.Tick();
        echo = Slot(words);
        int op = words[0] & 0xff00;
        if (op == 0x8800) { if (!inBlock) { inBlock = true; noted = false; block = new byte[words[1] * 12]; seen = 0; } }
        else if (op == 0x8900 && inBlock)
        {
            int index = words[0] & 31;
            echo.AsSpan(2, 12).CopyTo(block.AsSpan(index * 12));
            seen |= 1u << index;
            if (!noted && seen == (1u << (block.Length / 12)) - 1)
            {
                noted = true; Blocks.Add(block);
                if (block.Length == 24) Sent.Add((Bin.U16(block), Bin.U16(block, 2)));
            }
        }
        else inBlock = false;
    }
}
