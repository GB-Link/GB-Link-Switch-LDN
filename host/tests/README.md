# Console host

`Frlg.Trade.Tests` doubles as the console front end, for machines without a desktop or when a
terminal is handier than the desktop app.
All commands run from the repository root; the port is `/dev/ttyACM0`, `/dev/ttyUSB0`, `COMx`, etc.

```bash
R="dotnet run --project host/tests/Frlg.Trade.Tests.csproj -c Release --"
$R                        # offline protocol suite (synthetic keys, fixtures in fixtures/)
$R --device <port>        # board diagnostic: handshake, scan, command rejection, recovery, stop
$R --live <port>          # full trade session with a Switch FireRed Leader room
$R --live <port> --offer 3
$R --party                # show the standby party
$R --party-set 3 file.pk3 # import an 80- or 100-byte PK3 into slot 3
$R --party-offer 3        # choose the slot offered in the next trade
$R --party-clear 3
```

The standby party lives in `local/party.json` at the repository root, in the desktop app's format
(`selected` index plus six hex PK3 slots); the desktop app keeps its own next to the program. Without
that file the built-in party in `assets/party.json` is used, with the slot it names on offer. A trade
needs at least two Pokémon.

`--live` writes each session to `local/runs/<timestamp>-native-console/`: `offered.pk3` (the Pokémon
sent), `received.pk3` (the one received, saved at commit; `received-2.pk3` and so on when more trades
follow in the same session), `native.log` and the `pia.jsonl` capture.
After a committed trade the received Pokémon replaces the offered slot in `local/party.json`, so the
next session sends it back unless you change the slot or the selection first. Keys are read from
`prod.keys` next to the built binary or from `~/.switch/prod.keys`.
