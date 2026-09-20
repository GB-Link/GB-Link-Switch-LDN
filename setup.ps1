[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
& dotnet restore (Join-Path $root 'host/desktop/Frlg.Trade.Desktop.csproj') --locked-mode
if ($LASTEXITCODE -ne 0) { throw '.NET dependency restore failed.' }
& dotnet publish (Join-Path $root 'host/desktop/Frlg.Trade.Desktop.csproj') -c Release --no-restore -p:PublishProfile=FolderProfile
if ($LASTEXITCODE -ne 0) { throw 'Desktop publish failed.' }
Write-Host 'Ready: app/Frlg.Trade.Desktop.exe (win-x64, framework-dependent single file; requires the .NET 10 Runtime x64)'
