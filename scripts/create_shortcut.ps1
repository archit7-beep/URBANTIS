param(
    [string]$TargetDir = ""
)

$ErrorActionPreference = "Stop"

if (-not $TargetDir -or -not (Test-Path $TargetDir)) {
    $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    if (Test-Path (Join-Path $scriptDir "Urbantis.exe")) {
        $TargetDir = $scriptDir
    } elseif (Test-Path (Join-Path $scriptDir "dist_app\Urbantis-win32-x64\Urbantis.exe")) {
        $TargetDir = Join-Path $scriptDir "dist_app\Urbantis-win32-x64"
    } else {
        $TargetDir = $scriptDir
    }
}

$exePath = Join-Path $TargetDir "Urbantis.exe"
$icoPath = Join-Path $TargetDir "urbantis.ico"

if (-not (Test-Path $exePath)) {
    Write-Host "[ERROR] Could not find Urbantis.exe in: $TargetDir" -ForegroundColor Red
    exit 1
}

$wshShell = New-Object -ComObject WScript.Shell

# 1. Desktop Shortcut
$desktopPath = [Environment]::GetFolderPath('Desktop')
$desktopShortcutPath = Join-Path $desktopPath "URBANTIS.lnk"
$desktopShortcut = $wshShell.CreateShortcut($desktopShortcutPath)
$desktopShortcut.TargetPath = $exePath
$desktopShortcut.WorkingDirectory = $TargetDir
if (Test-Path $icoPath) {
    $desktopShortcut.IconLocation = "$icoPath,0"
}
$desktopShortcut.Description = "URBANTIS - Pune Smart City Operations and Cyber-Resilience"
$desktopShortcut.Save()
Write-Host "[SUCCESS] Desktop Shortcut created: $desktopShortcutPath" -ForegroundColor Green

# 2. Start Menu Shortcut
$programsPath = [Environment]::GetFolderPath('Programs')
$startShortcutPath = Join-Path $programsPath "URBANTIS.lnk"
$startShortcut = $wshShell.CreateShortcut($startShortcutPath)
$startShortcut.TargetPath = $exePath
$startShortcut.WorkingDirectory = $TargetDir
if (Test-Path $icoPath) {
    $startShortcut.IconLocation = "$icoPath,0"
}
$startShortcut.Description = "URBANTIS - Pune Smart City Operations and Cyber-Resilience"
$startShortcut.Save()
Write-Host "[SUCCESS] Start Menu Shortcut created: $startShortcutPath" -ForegroundColor Green
