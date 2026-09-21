using System.Collections.Concurrent;
using Avalonia.Media.Imaging;

namespace Frlg.Trade.Desktop;

// Pokémon pictures come from the PokeAPI sprite collection, as the web client's do, so
// the program ships no artwork. Each is downloaded the first time it is shown and kept
// in local/sprites, which also serves it when there is no network.
public static class Sprites
{
    private const string Source = "https://cdn.jsdelivr.net/gh/PokeAPI/sprites@master/sprites/pokemon/";
    private static readonly HttpClient Http = new() { Timeout = TimeSpan.FromSeconds(10) };
    private static readonly ConcurrentDictionary<int, Task<Bitmap?>> Loaded = new();

    public static string Folder { get; private set; } = Path.Combine(Paths.Local, "sprites");
    public static Func<int, Task<byte[]>> Download { get; private set; } = FromSource;

    private static Task<byte[]> FromSource(int species) => Http.GetByteArrayAsync($"{Source}{species}.png");

    // The UI checks keep their pictures in a folder of their own and never use the network.
    public static void Use(string folder, Func<int, Task<byte[]>> download)
    {
        Folder = folder;
        Download = download;
        Loaded.Clear();
    }

    // Null when there is no picture to be had. That is not remembered, so the next time
    // the Pokémon is shown the download is tried again.
    public static async Task<Bitmap?> For(int species)
    {
        var loading = Loaded.GetOrAdd(species, Load);
        var bitmap = await loading.ConfigureAwait(false);
        if (bitmap is null) Loaded.TryRemove(new KeyValuePair<int, Task<Bitmap?>>(species, loading));
        return bitmap;
    }

    private static async Task<Bitmap?> Load(int species)
    {
        string file = Path.Combine(Folder, $"{species}.png");
        try
        {
            if (File.Exists(file)) return Decode(await File.ReadAllBytesAsync(file).ConfigureAwait(false));
        }
        catch { }   // a copy that cannot be read is downloaded again
        try
        {
            byte[] bytes = await Download(species).ConfigureAwait(false);
            var bitmap = Decode(bytes);   // throws for a reply that is not a picture, which is then not kept
            Directory.CreateDirectory(Folder);
            string temp = file + ".tmp";
            await File.WriteAllBytesAsync(temp, bytes).ConfigureAwait(false);
            File.Move(temp, file, true);
            return bitmap;
        }
        catch { return null; }
    }

    private static Bitmap Decode(byte[] bytes)
    {
        using var stream = new MemoryStream(bytes);
        return new Bitmap(stream);
    }
}
