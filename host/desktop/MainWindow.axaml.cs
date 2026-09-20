using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO.Ports;
using System.Text.Json;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Platform.Storage;
using Avalonia.Threading;
using Avalonia.VisualTree;
using PKHeX.Core;
using Frlg.Trade.Core;

namespace Frlg.Trade.Desktop;

public enum ConnectionState { Disconnected, Connecting, Connected, Disconnecting }

public partial class MainWindow : Window
{
    public PartyStore LocalParty { get; } = new();
    public ObservableCollection<PokemonSlot> OpponentSlots { get; } = new(Enumerable.Range(0, 6).Select(i => new PokemonSlot(i, true)));
    public ConnectionState State { get; private set; }
    public bool PendingChanges { get; private set; }
    private readonly BridgeClient bridge = new();
    private bool initialized, closing, failed;
    private bool inTradeMenu, declining;
    private int trades;
    // The party the Switch holds: the snapshot sent at connect, updated by trades.
    private string?[]? sessionParty;
    internal int LastLiveOffer { get; private set; } = -1;
    private string? runPath;
    private TrainerIdentity[] trainers = [];
    private string trainerSignature = "";
    private string SettingsPath => Path.Combine(Paths.Local, "desktop.json");
    private static readonly FilePickerFileType Pk3Files = new("Generation 3 Pokémon") { Patterns = ["*.pk3"] };

    public MainWindow()
    {
        InitializeComponent();
        Directory.CreateDirectory(Paths.Local);
        LocalParty.Load();
        DataContext = this;
        RefreshPortList();
        if (File.Exists(SettingsPath))
        {
            using var settings = JsonDocument.Parse(File.ReadAllText(SettingsPath));
            var root = settings.RootElement;
            if (root.TryGetProperty("port", out var port) && PortBox.ItemsSource is string[] ports && ports.Contains(port.GetString())) PortBox.SelectedItem = port.GetString();
            AutoOt.IsChecked = root.TryGetProperty("autoOt", out var auto) && auto.GetBoolean();
        }
        bridge.Message += message => Dispatcher.UIThread.Post(() => HandleEvent(message));
        bridge.Exited += code => Dispatcher.UIThread.Post(() => HandleExit(code));
        AddHandler(DragDrop.DragOverEvent, Slot_DragOver);
        AddHandler(DragDrop.DropEvent, Slot_Drop);
        Closing += Window_Closing;
        initialized = true;
        SetState(ConnectionState.Disconnected);
    }

    internal void HandleExit(int code)
    {
        SetState(ConnectionState.Disconnected);
        if (!failed) StatusText.Text = code == 0 ? "Connection closed" : "Connection ended unexpectedly; the serial port has been released";
    }

    internal void SetState(ConnectionState state)
    {
        State = state;
        ConnectButton.IsEnabled = state == ConnectionState.Disconnected;
        DisconnectButton.IsEnabled = state is ConnectionState.Connected or ConnectionState.Connecting;
        DisconnectLabel.Text = state == ConnectionState.Connecting ? "Cancel" : "Disconnect";
        PortBox.IsEnabled = RefreshPorts.IsEnabled = state == ConnectionState.Disconnected;
        if (state == ConnectionState.Disconnected) { inTradeMenu = declining = false; sessionParty = null; trades = 0; }
        UpdateCancelTrade();
        if (state == ConnectionState.Disconnected)
        {
            foreach (var slot in OpponentSlots) slot.Set(null);
            OpponentName.Text = "Partner";
            trainers = []; trainerSignature = ""; SyncOt.IsEnabled = false;
            if (PendingChanges) PendingText.Text = "Standby party saved; it will be used for the next connection";
        }
    }

    // Off Windows the list includes the built-in ttyS ports. On Linux the GB-Link's own CDC port is dropped too.
    private static string[] BoardPorts()
    {
        var names = SerialPort.GetPortNames().Distinct();
        if (!OperatingSystem.IsWindows())
            names = names.Where(n => n.Contains("ttyACM") || n.Contains("ttyUSB") || n.Contains("cu.usb") || n.Contains("cu.SLAB") || n.Contains("cu.wchusb"));
        if (OperatingSystem.IsLinux()) names = names.Where(n => UsbVendor(n) != GbLinkVendor);
        return names.Order(StringComparer.OrdinalIgnoreCase).ToArray();
    }
    private const string GbLinkVendor = "2fe3";
    private static string? UsbVendor(string port)
    {
        try
        {
            // Both are relative symlinks: the device link has to be resolved from the tty entry's real location.
            var tty = Directory.ResolveLinkTarget($"/sys/class/tty/{Path.GetFileName(port)}", true);
            var directory = tty == null ? null : Directory.ResolveLinkTarget(Path.Combine(tty.FullName, "device"), true) as DirectoryInfo;
            for (int level = 0; directory != null && level < 4; level++, directory = directory.Parent)
            {
                string file = Path.Combine(directory.FullName, "idVendor");
                if (File.Exists(file)) return File.ReadAllText(file).Trim().ToLowerInvariant();
            }
        }
        catch (IOException) { }
        catch (UnauthorizedAccessException) { }
        return null;
    }
    private void RefreshPortList()
    {
        var previous = PortBox.SelectedItem as string;
        var ports = BoardPorts();
        PortBox.ItemsSource = ports;
        PortBox.SelectedItem = ports.Contains(previous) ? previous : ports.FirstOrDefault();
    }
    private void Refresh_Click(object? sender, RoutedEventArgs e) => RefreshPortList();
    private void Port_Changed(object? sender, SelectionChangedEventArgs e) { if (initialized) SaveSettings(); }
    private void SaveSettings() => File.WriteAllText(SettingsPath, JsonSerializer.Serialize(new { port = PortBox.SelectedItem as string, autoOt = AutoOt.IsChecked == true }));

    internal async void SetUpKeys()
    {
        if (KeyFile.Present) return;
        if (await KeysPrompt.Show(this, ProgramDirectory.Path)) StatusText.Text = "Keys stored. Create a room on the Switch, then connect";
        else KeysStillNeeded();
    }
    private void KeysStillNeeded() => StatusText.Text = "Your Switch's prod.keys is still needed: drop it on this window, or click Connect";
    private void ImportKeys(string path)
    {
        var prompt = new KeysPrompt(ProgramDirectory.Path);
        StatusText.Text = prompt.TryImport(path) ? "Keys stored" : prompt.Problem;
    }
    private static bool IsKeyFile(string path) => Path.GetExtension(path).Equals(".keys", StringComparison.OrdinalIgnoreCase);

    private void Connect_Click(object? sender, RoutedEventArgs e) => Connect();
    internal async void Connect()
    {
        if (State != ConnectionState.Disconnected) return;
        if (PortBox.SelectedItem is not string port) { ShowError("No serial port available."); return; }
        if (LocalParty.Slots.Count(s => s.Occupied) < 2 || !LocalParty.Slots[LocalParty.Selected].Occupied)
        { ShowError("The party needs at least two Pokémon and a valid offered slot."); return; }
        try
        {
            while (true)
            {
                try { KeyFile.LoadDefault(); break; }
                catch (MissingKeysException missing) { if (!await KeysPrompt.Show(this, missing.DirectoryPath)) { KeysStillNeeded(); return; } }
            }
            LocalParty.Save(); SaveSettings();
            PendingChanges = false; PendingText.Text = ""; failed = false;
            SetState(ConnectionState.Connecting);
            StatusText.Text = "Opening the serial port";
            var snapshot = LocalParty.Snapshot();
            BeginSession(snapshot);
            await bridge.StartAsync(port, snapshot, LocalParty.Selected);
        }
        catch (Exception error) { SetState(ConnectionState.Disconnected); ShowError(error.Message); }
    }
    private void UpdateCancelTrade() => CancelTradeButton.IsEnabled = State == ConnectionState.Connected && inTradeMenu && !declining;
    internal void BeginSession(string?[] snapshot) => sessionParty = snapshot;
    // Only a slot unchanged since the snapshot can be offered live; edits apply on the next connection.
    private bool KnownToSwitch(PokemonSlot slot) => State is ConnectionState.Connecting or ConnectionState.Connected &&
        slot.Pokemon != null && sessionParty?[slot.Index] == Convert.ToHexString(PokemonData.Export(slot.Pokemon));
    private void CancelTrade_Click(object? sender, RoutedEventArgs e)
    {
        bridge.CancelTrade();
        declining = true; UpdateCancelTrade();
    }
    private async void Disconnect_Click(object? sender, RoutedEventArgs e)
    {
        SetState(ConnectionState.Disconnecting);
        StatusText.Text = "Disconnecting";
        await bridge.StopAsync();
        SetState(ConnectionState.Disconnected);
        if (!failed) StatusText.Text = "Connection closed";
    }

    internal void HandleEvent(JsonElement message)
    {
        if (!message.TryGetProperty("event", out var kind)) return;
        switch (kind.GetString())
        {
            case "device":
                ToolTip.SetTip(DeviceLabel, $"Bridge firmware {message.GetProperty("firmware").GetString()} · {message.GetProperty("model").GetString()}"); break;
            case "run":
                runPath = message.GetProperty("path").GetString(); break;
            case "phase":
                StatusText.Text = message.GetProperty("message").GetString(); break;
            case "connected":
                if (State == ConnectionState.Disconnecting) break;
                SetState(ConnectionState.Connected);
                StatusText.Text = "Connected, waiting for the partner's party";
                break;
            case "opponent_party":
                if (State is ConnectionState.Disconnected or ConnectionState.Disconnecting) break;
                var values = message.GetProperty("party").EnumerateArray().ToArray();
                if (values.Length != 6) { ShowError("The received party has an unexpected number of slots."); break; }
                try
                {
                    var parsed = values.Select(v => v.ValueKind == JsonValueKind.Null ? null : PokemonData.Parse(Convert.FromHexString(v.GetString()!))).ToArray();
                    for (int i = 0; i < 6; i++) OpponentSlots[i].Set(parsed[i]);
                    OpponentName.Text = message.GetProperty("name").GetString() ?? "Switch";
                    var onOffer = LocalParty.Slots[LocalParty.Selected];
                    StatusText.Text = trades > 0 && KnownToSwitch(onOffer) ? $"Ready for another trade: {onOffer.Nickname} is on offer" : "Partner's party received";
                    inTradeMenu = true; UpdateCancelTrade();
                    trainers = GetTrainers(); SyncOt.IsEnabled = trainers.Length > 0;
                    var signature = JsonSerializer.Serialize(trainers);
                    if (signature != trainerSignature)
                    {
                        trainerSignature = signature;
                        if (AutoOt.IsChecked == true) SyncTrainer();
                    }
                }
                catch (Exception error) { ShowError(error.Message); }
                break;
            case "received":
                try
                {
                    var received = PokemonData.Parse(Convert.FromHexString(message.GetProperty("pk3").GetString()!));
                    int index = message.GetProperty("slot").GetInt32();
                    inTradeMenu = false; trades++; UpdateCancelTrade();
                    // Goes into the traded slot unless the user changed that slot during the connection.
                    bool untouched = index is >= 0 and < 6 && (sessionParty == null ? !PendingChanges : KnownToSwitch(LocalParty.Slots[index]));
                    if (untouched) { LocalParty.Slots[index].Set(received); LocalParty.Save(); }
                    if (sessionParty != null && index is >= 0 and < 6) sessionParty[index] = Convert.ToHexString(PokemonData.Export(received));
                    StatusText.Text = untouched ? $"Trade complete; received {received.Nickname}"
                        : $"Trade complete; received {received.Nickname}. Slot {index + 1} was changed meanwhile, so it is kept in the session's folder (Logs)";
                }
                catch (Exception error) { ShowError(error.Message); }
                break;
            case "declining":
                declining = message.GetProperty("value").GetBoolean(); UpdateCancelTrade(); break;
            case "error":
                failed = true; ShowError(message.GetProperty("message").GetString() ?? "Connection failed"); break;
            case "log":
                if (runPath != null) File.AppendAllText(Path.Combine(runPath, "desktop.log"), message.GetProperty("message").GetString() + "\n");
                break;
            // The worker completion callback fires after serial teardown.
            case "disconnected": break;
        }
    }

    internal TrainerIdentity[] GetTrainers() => OpponentSlots.Where(s => s.Pokemon != null)
        .Select(s => TrainerIdentity.From(s.Pokemon!)).Distinct().OrderBy(t => t.Name).ThenBy(t => t.Tid).ThenBy(t => t.Sid).ThenBy(t => t.Gender).ToArray();

    private void AutoOt_Changed(object? sender, RoutedEventArgs e)
    {
        if (!initialized) return;
        SaveSettings();
        if (AutoOt.IsChecked == true && trainers.Length > 0) SyncTrainer();
    }
    private void SyncOt_Click(object? sender, RoutedEventArgs e) => SyncTrainer();
    private async void SyncTrainer()
    {
        if (trainers.Length == 0) return;
        var trainer = trainers.Length == 1 ? trainers[0] : await TrainerPicker.Pick(this, trainers);
        if (trainer == null) return;
        try
        {
            LocalParty.ApplyTrainer(trainer);
            OtStatus.Text = trainer.Label;
            MarkChanged();
        }
        catch (Exception error) { ShowError(error.Message); }
    }
    internal void MarkChanged()
    {
        PendingChanges = State != ConnectionState.Disconnected;
        PendingText.Text = PendingChanges ? "Right side is the standby party · changes take effect on the next connection" : "Party saved";
        LocalParty.Save();
    }
    private void ShowError(string message) => StatusText.Text = message;

    private static PokemonSlot? SlotOf(object? source) => (source as Visual)?.GetSelfAndVisualAncestors().OfType<Control>().Select(c => c.Tag).OfType<PokemonSlot>().FirstOrDefault();
    private static PokemonSlot? MenuSlot(object? sender) => (sender as MenuItem)?.DataContext as PokemonSlot;
    private static bool IsPk3(string path) => Path.GetExtension(path).Equals(".pk3", StringComparison.OrdinalIgnoreCase);
    private static string[] DroppedPaths(DragEventArgs e) =>
        e.DataTransfer.TryGetFiles()?.Select(f => f.TryGetLocalPath()).OfType<string>().ToArray() ?? [];

    private void Slot_Click(object? sender, PointerReleasedEventArgs e)
    {
        if (e.InitialPressMouseButton != MouseButton.Left || (sender as Control)?.Tag is not PokemonSlot slot || slot.IsOpponent) return;
        if (!slot.Occupied) { ImportFrom(slot); return; }
        LocalParty.Select(slot.Index);
        if (!KnownToSwitch(slot)) { MarkChanged(); return; }
        LocalParty.Save(); bridge.Offer(slot.Index); LastLiveOffer = slot.Index;
        StatusText.Text = $"Now offering {slot.Nickname}";
    }
    private void Slot_DragOver(object? sender, DragEventArgs e)
    {
        var paths = DroppedPaths(e);
        bool keys = paths.Length == 1 && IsKeyFile(paths[0]);
        e.DragEffects = keys || SlotOf(e.Source) is { IsOpponent: false } && paths.Length > 0 && paths.All(IsPk3) ? DragDropEffects.Copy : DragDropEffects.None;
        e.Handled = true;
    }
    private void Slot_Drop(object? sender, DragEventArgs e)
    {
        var paths = DroppedPaths(e);
        if (paths.Length == 1 && IsKeyFile(paths[0])) ImportKeys(paths[0]);
        else if (SlotOf(e.Source) is { IsOpponent: false } slot && paths.Length > 0) ImportFiles(slot, paths);
        e.Handled = true;
    }
    private async void ImportFrom(PokemonSlot slot)
    {
        var picked = await StorageProvider.OpenFilePickerAsync(new() { Title = "Import PK3", AllowMultiple = true, FileTypeFilter = [Pk3Files] });
        var paths = picked.Select(f => f.TryGetLocalPath()).OfType<string>().ToArray();
        if (paths.Length > 0) ImportFiles(slot, paths);
    }
    internal void ImportFiles(PokemonSlot slot, string[] paths)
    {
        if (slot.IsOpponent) return;
        try
        {
            if (paths.Length == 0 || slot.Index + paths.Length > 6 || !paths.All(IsPk3)) throw new InvalidDataException("More files than remaining slots, or a file is not a PK3.");
            var loaded = paths.Select(p => PokemonData.Parse(File.ReadAllBytes(p))).ToArray();
            for (int i = 0; i < loaded.Length; i++) LocalParty.Slots[slot.Index + i].Set(loaded[i]);
            if (!LocalParty.Slots[LocalParty.Selected].Occupied) LocalParty.Select(slot.Index);
            MarkChanged(); StatusText.Text = $"Loaded {loaded.Length} Pokémon";
        }
        catch (Exception error) { ShowError(error.Message); }
    }
    private void Import_Click(object? sender, RoutedEventArgs e) { if (MenuSlot(sender) is { IsOpponent: false } slot) ImportFrom(slot); }
    private async void Export_Click(object? sender, RoutedEventArgs e)
    {
        if (MenuSlot(sender)?.Pokemon is not { } pk) return;
        var file = await StorageProvider.SaveFilePickerAsync(new() { Title = "Export PK3", SuggestedFileName = $"{pk.Species:D3}.pk3", DefaultExtension = "pk3", FileTypeChoices = [Pk3Files] });
        if (file?.TryGetLocalPath() is not { } path) return;
        try { File.WriteAllBytes(path, PokemonData.Export(pk)); }
        catch (IOException error) { ShowError(error.Message); }
    }
    private async void Details_Click(object? sender, RoutedEventArgs e)
    { if (MenuSlot(sender) is { Occupied: true } slot) await MessageDialog.Create(slot.Nickname, slot.Details).ShowDialog(this); }
    private void Clear_Click(object? sender, RoutedEventArgs e)
    {
        if (MenuSlot(sender) is not { IsOpponent: false } slot) return;
        slot.Set(null);
        if (slot.Index == LocalParty.Selected) LocalParty.Select(LocalParty.Slots.FirstOrDefault(s => s.Occupied)?.Index ?? 0);
        MarkChanged();
    }
    private void OpenLogs_Click(object? sender, RoutedEventArgs e)
        => Process.Start(new ProcessStartInfo(runPath ?? Paths.Local) { UseShellExecute = true });
    private async void Window_Closing(object? sender, WindowClosingEventArgs e)
    {
        if (SmokeTests.Active) return;
        if (closing) return;
        if (bridge.Running)
        {
            e.Cancel = true;
            SetState(ConnectionState.Disconnecting);
            await bridge.StopAsync(); closing = true; Close();
        }
        else { LocalParty.Save(); SaveSettings(); }
    }
}

internal static class Dialog
{
    public static Window Frame(string title, double width, Control content) => new()
    {
        Title = title, Width = width, SizeToContent = SizeToContent.Height, CanResize = false,
        WindowStartupLocation = WindowStartupLocation.CenterOwner, Content = content,
        Background = Brushes.White, Foreground = Brushes.Black,
    };
}

public static class MessageDialog
{
    public static Window Create(string title, string text)
    {
        var panel = new StackPanel { Margin = new Thickness(22) };
        panel.Children.Add(new TextBlock { Text = text, TextWrapping = TextWrapping.Wrap });
        var close = new Button { Content = "Close", HorizontalAlignment = HorizontalAlignment.Right, Margin = new Thickness(0, 18, 0, 0), IsDefault = true, IsCancel = true };
        panel.Children.Add(close);
        var window = Dialog.Frame(title, 420, panel);
        close.Click += (_, _) => window.Close();
        return window;
    }
}

public static class TrainerPicker
{
    public static async Task<TrainerIdentity?> Pick(Window owner, TrainerIdentity[] identities)
    {
        var choices = new ListBox { ItemsSource = identities.Select(i => i.Label).ToArray(), SelectedIndex = 0, Margin = new Thickness(0, 12, 0, 18), MinHeight = 100 };
        var panel = new StackPanel { Margin = new Thickness(22) };
        panel.Children.Add(new TextBlock { Text = "Choose the partner's trainer", FontSize = 19, FontWeight = FontWeight.SemiBold });
        panel.Children.Add(choices);
        var confirm = new Button { Content = "Use this trainer", HorizontalAlignment = HorizontalAlignment.Right, IsDefault = true };
        panel.Children.Add(confirm);
        var window = Dialog.Frame("Trainer", 600, panel);
        confirm.Click += (_, _) => window.Close(true);
        return await window.ShowDialog<bool>(owner) && choices.SelectedIndex >= 0 ? identities[choices.SelectedIndex] : null;
    }
}
