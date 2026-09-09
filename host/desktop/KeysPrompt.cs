using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using Frlg.Trade.Core;

namespace Frlg.Trade.Desktop;

internal static class KeysPrompt
{
    public static bool Show(Window owner, MissingKeysException error)
    {
        var panel = new StackPanel { Margin = new Thickness(22) };
        panel.Children.Add(new TextBlock { Text = "prod.keys not found", FontSize = 19, FontWeight = FontWeights.SemiBold });
        panel.Children.Add(new TextBlock { Text = "Place the file in the program directory and retry.", Margin = new Thickness(0, 12, 0, 8) });
        panel.Children.Add(new TextBlock { Text = error.DirectoryPath, TextWrapping = TextWrapping.Wrap });
        var buttons = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right, Margin = new Thickness(0, 20, 0, 0) };
        var open = new Button { Content = "Open folder", Margin = new Thickness(0, 0, 10, 0) };
        var retry = new Button { Content = "Retry", IsDefault = true };
        buttons.Children.Add(open); buttons.Children.Add(retry); panel.Children.Add(buttons);
        var window = new Window { Owner = owner, Title = "Key file", Width = 540, SizeToContent = SizeToContent.Height,
            ResizeMode = ResizeMode.NoResize, WindowStartupLocation = WindowStartupLocation.CenterOwner, Content = panel };
        open.Click += (_, _) => Process.Start(new ProcessStartInfo(error.DirectoryPath) { UseShellExecute = true });
        retry.Click += (_, _) => window.DialogResult = true;
        return window.ShowDialog() == true;
    }
}
