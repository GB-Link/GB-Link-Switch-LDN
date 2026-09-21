using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;

namespace Frlg.Trade.Desktop;

public partial class App : Application
{
    public override void Initialize() => AvaloniaXamlLoader.Load(this);

    public override void OnFrameworkInitializationCompleted()
    {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            string[] args = desktop.Args ?? [];
            bool smokeTest = args.Contains("--smoke-test");
            try
            {
                if (smokeTest) SmokeTests.Prepare();
                var window = new MainWindow();
                desktop.MainWindow = window;
                window.Opened += async (_, _) =>
                {
                    if (args.Contains("--connect")) window.Connect();
                    else if (!smokeTest) window.SetUpKeys();
                    if (!smokeTest) return;
                    int code = 0;
                    try { await SmokeTests.Run(window); }
                    catch (Exception error) { code = 1; ReportStartupFailure(error); }
                    Dispatcher.UIThread.Post(() => desktop.Shutdown(code));
                };
            }
            catch (Exception error)
            {
                ReportStartupFailure(error);
                if (smokeTest) { Dispatcher.UIThread.Post(() => desktop.Shutdown(1)); }
                else desktop.MainWindow = MessageDialog.Create("Startup failed", error.Message);
            }
        }
        base.OnFrameworkInitializationCompleted();
    }

    private static void ReportStartupFailure(Exception error)
    {
        Directory.CreateDirectory(Paths.Local);
        File.WriteAllText(Path.Combine(Paths.Local, "desktop-error.log"), error.ToString());
    }
}
