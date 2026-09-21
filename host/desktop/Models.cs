using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text.Json;
using Avalonia;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using PKHeX.Core;

namespace Frlg.Trade.Desktop;

public static class Paths
{
    public static readonly string Root = Frlg.Trade.Core.ProgramDirectory.Path;
    public static string Local => Path.Combine(Root, "local");
}

public static class DefaultAssets
{
    public static Stream Open(string name) => typeof(DefaultAssets).Assembly.GetManifestResourceStream("Assets." + name)
        ?? throw new FileNotFoundException($"Missing embedded resource: {name}");

    public static string Party()
    {
        using var reader = new StreamReader(Open("party.json"));
        return reader.ReadToEnd();
    }
}

public abstract class Observable : INotifyPropertyChanged
{
    public event PropertyChangedEventHandler? PropertyChanged;
    protected void Notify([CallerMemberName] string? name = null) => PropertyChanged?.Invoke(this, new(name));
}

public sealed record TrainerIdentity(ushort Tid, ushort Sid, string Name, byte Gender)
{
    public static TrainerIdentity From(PK3 pk) => new(pk.TID16, pk.SID16, pk.OriginalTrainerName, pk.OriginalTrainerGender);
    public string Label => $"{Name}  ·  {(Gender == 0 ? "Male" : "Female")}  ·  TID {Tid:D5}  /  SID {Sid:D5}";
}

public static class PokemonData
{
    public static PK3 Parse(byte[] bytes)
    {
        if (bytes.Length is not (80 or 100)) throw new InvalidDataException("A PK3 must be 80 or 100 bytes.");
        var pk = new PK3(bytes.ToArray());
        if (!pk.ChecksumValid || pk.Species is 0 or > 386 || pk.FlagIsBadEgg)
            throw new InvalidDataException("PK3 checksum failed, or this is not a valid Generation 3 Pokémon.");
        if (bytes.Length == 80 || pk.Stat_Level == 0) pk.ResetPartyStats();
        return pk;
    }
    public static byte[] Export(PK3 pk)
    {
        var copy = pk.Clone();
        copy.RefreshChecksum();
        return copy.Data.ToArray();
    }
    public static PK3 WithTrainer(PK3 pk, TrainerIdentity trainer)
    {
        var copy = pk.Clone();
        copy.TID16 = trainer.Tid;
        copy.SID16 = trainer.Sid;
        copy.OriginalTrainerTrash.Fill(0xFF);
        copy.OriginalTrainerName = trainer.Name;
        copy.OriginalTrainerGender = trainer.Gender;
        if (copy.OriginalTrainerName != trainer.Name)
            throw new InvalidDataException($"The language of {pk.Nickname} cannot store the trainer name \"{trainer.Name}\".");
        copy.RefreshChecksum();
        return copy;
    }
}

public sealed class PokemonSlot(int index, bool opponent) : Observable
{
    private PK3? pokemon;
    private bool selected;
    public int Index { get; } = index;
    public bool IsOpponent { get; } = opponent;
    public PK3? Pokemon => pokemon;
    public bool Occupied => pokemon != null;
    public string Nickname => pokemon?.Nickname ?? "";
    public string Level => pokemon is null ? "" : $"Lv. {pokemon.CurrentLevel}";
    public string Gender => pokemon?.Gender switch { 0 => "♂", 1 => "♀", 2 => "-", _ => "" };
    public bool CanClear => !IsOpponent && Occupied;
    public IBrush GenderBrush => pokemon?.Gender == 1 ? Brushes.LightPink : Brushes.LightCyan;
    public string Selection => selected && Occupied ? "Offered" : "";
    public IBrush Outline => selected && Occupied ? new SolidColorBrush(Color.FromRgb(255, 207, 70)) : new SolidColorBrush(Color.FromArgb(90, 118, 181, 230));
    public string Details => pokemon is null ? "" : $"{pokemon.Nickname} · #{pokemon.Species:D3}\n{TrainerIdentity.From(pokemon).Label}\nIVs {pokemon.IV_HP}/{pokemon.IV_ATK}/{pokemon.IV_DEF}/{pokemon.IV_SPA}/{pokemon.IV_SPD}/{pokemon.IV_SPE}\nPK3 checksum OK";
    public IImage? Sprite { get; private set; }
    public void Set(PK3? value)
    {
        pokemon = value;
        Sprite = null;
        foreach (var name in new[] { nameof(Pokemon), nameof(Occupied), nameof(CanClear), nameof(Nickname), nameof(Level), nameof(Gender), nameof(GenderBrush), nameof(Selection), nameof(Outline), nameof(Details), nameof(Sprite) }) Notify(name);
        if (value != null) _ = ShowSprite(value.Species);
    }
    // The picture follows the Pokémon, from local/sprites or the network, and is left out
    // when neither has it.
    private async Task ShowSprite(ushort species)
    {
        var bitmap = await Sprites.For(species);
        if (bitmap is null) return;
        Dispatcher.UIThread.Post(() =>
        {
            if (pokemon?.Species != species) return;
            Sprite = CropSprite(bitmap);
            Notify(nameof(Sprite));
        });
    }
    public void Select(bool value) { selected = value; Notify(nameof(Outline)); Notify(nameof(Selection)); }
    // Sprites are padded to a common size; crop to the drawn pixels.
    private static IImage CropSprite(Bitmap source)
    {
        int width = source.PixelSize.Width, height = source.PixelSize.Height;
        var bytes = new byte[width * height * 4];
        var pinned = GCHandle.Alloc(bytes, GCHandleType.Pinned);
        try { source.CopyPixels(new PixelRect(0, 0, width, height), pinned.AddrOfPinnedObject(), bytes.Length, width * 4); }
        finally { pinned.Free(); }
        int left = width, right = 0, top = height, bottom = 0;
        for (int y = 0; y < height; y++) for (int x = 0; x < width; x++)
            if (bytes[(y * width + x) * 4 + 3] != 0)
            { left = Math.Min(left, x); right = Math.Max(right, x); top = Math.Min(top, y); bottom = Math.Max(bottom, y); }
        if (left > right || top > bottom) return source;
        return new CroppedBitmap(source, new PixelRect(left, top, right - left + 1, bottom - top + 1));
    }
}

public sealed class PartyStore
{
    public ObservableCollection<PokemonSlot> Slots { get; } = new(Enumerable.Range(0, 6).Select(i => new PokemonSlot(i, false)));
    public int Selected { get; private set; } = 1;
    private string FilePath => Path.Combine(Paths.Local, "party.json");
    public void Load() => Read(File.Exists(FilePath) ? File.ReadAllText(FilePath) : DefaultAssets.Party());
    public void LoadDefault() => Read(DefaultAssets.Party());
    private void Read(string record)
    {
        using var doc = JsonDocument.Parse(record);
        var entries = doc.RootElement.GetProperty("slots").EnumerateArray().ToArray();
        if (entries.Length != 6) throw new InvalidDataException("The party record must have six slots.");
        for (int i = 0; i < 6; i++)
            Slots[i].Set(entries[i].ValueKind == JsonValueKind.Null ? null : PokemonData.Parse(Convert.FromHexString(entries[i].GetString()!)));
        Selected = doc.RootElement.GetProperty("selected").GetInt32();
        if (Selected < 0 || Selected > 5 || !Slots[Selected].Occupied)
            Selected = Slots.FirstOrDefault(s => s.Occupied)?.Index ?? 0;
        Select(Selected);
    }
    public void Select(int index)
    {
        Selected = index;
        foreach (var slot in Slots) slot.Select(slot.Index == index);
    }
    public string?[] Snapshot() => Slots.Select(s => s.Pokemon is null ? null : Convert.ToHexString(PokemonData.Export(s.Pokemon))).ToArray();
    public void Save()
    {
        Directory.CreateDirectory(Paths.Local);
        var temp = FilePath + ".tmp";
        File.WriteAllText(temp, JsonSerializer.Serialize(new { selected = Selected, slots = Snapshot() }));
        File.Move(temp, FilePath, true);
    }
    public void ApplyTrainer(TrainerIdentity trainer)
    {
        var updated = Slots.Select(s => s.Pokemon is null ? null : PokemonData.WithTrainer(s.Pokemon, trainer)).ToArray();
        for (int i = 0; i < 6; i++) Slots[i].Set(updated[i]);
        Save();
    }
}
