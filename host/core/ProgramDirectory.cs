namespace Frlg.Trade.Core;

// An AppImage runs from a read-only mount, so its files go next to the AppImage file instead.
public static class ProgramDirectory
{
    public static string Path { get; } = Find(AppContext.BaseDirectory,
        Environment.GetEnvironmentVariable("APPIMAGE"), Environment.GetEnvironmentVariable("APPDIR"));

    // Child processes inherit APPIMAGE and APPDIR, so they only count when this program runs from that mount.
    public static string Find(string baseDirectory, string? appImage, string? appDir)
    {
        bool mounted = !string.IsNullOrEmpty(appImage) && !string.IsNullOrEmpty(appDir) &&
            baseDirectory.StartsWith(appDir.TrimEnd('/') + "/", StringComparison.Ordinal);
        return mounted ? System.IO.Path.GetDirectoryName(appImage) ?? baseDirectory : baseDirectory;
    }
}
