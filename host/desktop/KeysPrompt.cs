using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Platform.Storage;
using Frlg.Trade.Core;

namespace Frlg.Trade.Desktop;

internal sealed class KeysPrompt
{
    private readonly string directory;
    private readonly TextBlock status = new() { TextWrapping = TextWrapping.Wrap, Foreground = new SolidColorBrush(Color.FromRgb(176, 38, 38)), Margin = new Thickness(0, 12, 0, 0), IsVisible = false };

    public KeysPrompt(string directory) => this.directory = directory;

    public bool TryImport(string path)
    {
        try { KeyFile.Import(path, directory); return true; }
        catch (Exception error) when (error is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            status.Text = error is InvalidDataException ? "That file cannot be used: " + error.Message.Replace(": " + path, "") : error.Message;
            status.IsVisible = true;
            return false;
        }
    }
    public string? Problem => status.IsVisible ? status.Text : null;

    public static Task<bool> Show(Window owner, string directory) => new KeysPrompt(directory).Build().ShowDialog<bool>(owner);

    public Window Build()
    {
        var prompt = this;
        var panel = new StackPanel { Margin = new Thickness(22) };
        panel.Children.Add(new TextBlock { Text = "One thing before the first trade", FontSize = 19, FontWeight = FontWeight.SemiBold });
        panel.Children.Add(new TextBlock { TextWrapping = TextWrapping.Wrap, Margin = new Thickness(0, 12, 0, 14),
            Text = "The Switch encrypts its local wireless with keys from the console, so this program needs the prod.keys of your own Switch." });
        var target = new Border
        {
            BorderBrush = new SolidColorBrush(Color.FromRgb(8, 125, 140)), BorderThickness = new Thickness(2), CornerRadius = new CornerRadius(8),
            Background = new SolidColorBrush(Color.FromRgb(236, 247, 246)), Padding = new Thickness(18, 26),
            Child = new TextBlock { Text = "Drop your prod.keys here", FontSize = 16, FontWeight = FontWeight.SemiBold, HorizontalAlignment = HorizontalAlignment.Center },
        };
        DragDrop.SetAllowDrop(target, true);
        panel.Children.Add(target);
        panel.Children.Add(prompt.status);
        panel.Children.Add(new TextBlock { TextWrapping = TextWrapping.Wrap, FontSize = 12, Foreground = Brushes.DimGray, Margin = new Thickness(0, 14, 0, 0),
            Text = "The file is only read on this computer. The few entries the program uses are kept in a prod.keys next to it, in " + directory });
        var buttons = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right, Margin = new Thickness(0, 20, 0, 0) };
        var later = new Button { Content = "Not now", Classes = { "quiet" }, Margin = new Thickness(0, 0, 10, 0), IsCancel = true };
        var choose = new Button { Content = "Choose the file…", IsDefault = true };
        buttons.Children.Add(later); buttons.Children.Add(choose); panel.Children.Add(buttons);
        var window = Dialog.Frame("First-time setup", 560, new Border { Background = Brushes.White, Child = panel });

        target.AddHandler(DragDrop.DragOverEvent, (_, e) => { e.DragEffects = DragDropEffects.Copy; e.Handled = true; });
        target.AddHandler(DragDrop.DropEvent, (_, e) =>
        {
            e.Handled = true;
            if (e.DataTransfer.TryGetFiles()?.Select(f => f.TryGetLocalPath()).OfType<string>().FirstOrDefault() is { } path && prompt.TryImport(path)) window.Close(true);
        });
        choose.Click += async (_, _) =>
        {
            var picked = await window.StorageProvider.OpenFilePickerAsync(new() { Title = "Choose your prod.keys", AllowMultiple = false,
                FileTypeFilter = [new("Key files") { Patterns = ["*.keys"] }, FilePickerFileTypes.All] });
            if (picked.FirstOrDefault()?.TryGetLocalPath() is { } path && prompt.TryImport(path)) window.Close(true);
        };
        later.Click += (_, _) => window.Close(false);
        return window;
    }
}
