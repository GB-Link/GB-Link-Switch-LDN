using Avalonia;
using Avalonia.Headless;

namespace Frlg.Trade.Desktop;

internal static class Program
{
    [STAThread]
    public static int Main(string[] args) => BuildAvaloniaApp(args.Contains("--smoke-test")).StartWithClassicDesktopLifetime(args);

    // The self-test renders headless, so it needs no display.
    private static AppBuilder BuildAvaloniaApp(bool offScreen)
    {
        var builder = AppBuilder.Configure<App>().WithInterFont();
        return offScreen
            ? builder.UseSkia().UseHeadless(new AvaloniaHeadlessPlatformOptions { UseHeadlessDrawing = false })
            : builder.UsePlatformDetect();
    }

    // For the XAML previewer.
    public static AppBuilder BuildAvaloniaApp() => BuildAvaloniaApp(false);
}
