param(
    [string]$ExePath = ".\\pomodoro-timer.exe",
    [int]$BootWaitSeconds = 3
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $repoRoot

if (-not (Test-Path -LiteralPath $ExePath)) {
    throw "Executable not found: $ExePath"
}

$resolvedExe = (Resolve-Path -LiteralPath $ExePath).Path
$runningProject = Get-Process -Name "pomodoro-timer" -ErrorAction SilentlyContinue | Where-Object {
    $_.Path -and $_.Path -eq $resolvedExe
}
if ($runningProject) {
    throw "The project executable is running. Exit it before running recovery tests."
}

# Force portable mode so data files live in repo root for deterministic checks.
$regPath = "HKCU:\Software\PomodoroTimer"
$regExisted = Test-Path $regPath
$oldReg = if ($regExisted) { Get-ItemProperty -Path $regPath } else { $null }
$hadMode = $regExisted -and ($oldReg.PSObject.Properties.Name -contains "DataLocationMode")
$hadCustomPath = $regExisted -and ($oldReg.PSObject.Properties.Name -contains "CustomDataPath")
$oldMode = if ($hadMode) { $oldReg.DataLocationMode } else { $null }
$oldCustomPath = if ($hadCustomPath) { $oldReg.CustomDataPath } else { $null }
if (-not (Test-Path $regPath)) {
    New-Item -Path $regPath -Force | Out-Null
}
New-ItemProperty -Path $regPath -Name DataLocationMode -PropertyType DWord -Value 0 -Force | Out-Null
New-ItemProperty -Path $regPath -Name CustomDataPath -PropertyType String -Value "" -Force | Out-Null

$main = Join-Path $repoRoot "pomodoro_settings.json"
$tmp = Join-Path $repoRoot "pomodoro_settings.json.tmp"
$bak = Join-Path $repoRoot "pomodoro_settings.json.bak"

$backupDir = Join-Path $repoRoot "tools\\_recovery_test_backup"
$expectedBackupParent = [System.IO.Path]::GetFullPath((Join-Path $repoRoot "tools"))
if ([System.IO.Path]::GetDirectoryName([System.IO.Path]::GetFullPath($backupDir)) -ne $expectedBackupParent) {
    throw "Unexpected recovery backup path: $backupDir"
}
if (Test-Path -LiteralPath $backupDir) {
    Remove-Item -LiteralPath $backupDir -Recurse -Force
}
New-Item -ItemType Directory -Path $backupDir | Out-Null

$allFiles = @($main, $tmp, $bak)
foreach ($f in $allFiles) {
    if (Test-Path -LiteralPath $f) {
        Copy-Item -LiteralPath $f -Destination (Join-Path $backupDir (Split-Path -Leaf $f)) -Force
    }
}

function Write-JsonFile {
    param([string]$Path, [string]$Json)
    Set-Content -LiteralPath $Path -Value $Json -Encoding ASCII -NoNewline
}

function Start-And-StopApp {
    param([string]$Path, [int]$WaitSeconds)
    $p = Start-Process -FilePath $Path -PassThru -WindowStyle Hidden
    try {
        Start-Sleep -Seconds $WaitSeconds
    }
    finally {
        if (-not $p.HasExited) {
            Stop-Process -Id $p.Id -Force
        }
    }
}

function Assert-FileContains {
    param([string]$Path, [string]$Needle, [string]$Label)
    if (-not (Test-Path -LiteralPath $Path)) {
        throw "[$Label] missing file: $Path"
    }
    $content = Get-Content -LiteralPath $Path -Raw
    if ($content -notmatch [regex]::Escape($Needle)) {
        throw "[$Label] file does not contain expected marker '$Needle': $Path"
    }
}

$validJsonTmp = '{"pomodoro_duration":90,"long_pomodoro_duration":90,"long_pomodoro_count":2,"short_pomodoro_duration":45,"short_break_duration":5,"long_break_duration":15,"custom_duration":10,"adjust_block_minutes":5,"toast_auto_collapse_seconds":10,"enable_clock_sound":0,"enable_completion_sound":1,"show_completion_dialog":1,"default_pomodoro_is_long":1,"default_break_is_long":0,"pomodoro_count":7,"idle_mode":0,"idle_pomodoro_is_long":1,"idle_break_is_long":0}'
$validJsonBak = '{"pomodoro_duration":135,"long_pomodoro_duration":135,"long_pomodoro_count":3,"short_pomodoro_duration":45,"short_break_duration":5,"long_break_duration":15,"custom_duration":10,"adjust_block_minutes":5,"toast_auto_collapse_seconds":10,"enable_clock_sound":0,"enable_completion_sound":1,"show_completion_dialog":1,"default_pomodoro_is_long":1,"default_break_is_long":0,"pomodoro_count":9,"idle_mode":0,"idle_pomodoro_is_long":1,"idle_break_is_long":0}'

$results = @()

try {
    # Case 1: main missing, tmp valid -> main should be restored from tmp.
    Remove-Item -LiteralPath $main -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $bak -ErrorAction SilentlyContinue
    Write-JsonFile -Path $tmp -Json $validJsonTmp
    Start-And-StopApp -Path $ExePath -WaitSeconds $BootWaitSeconds
    Assert-FileContains -Path $main -Needle '"long_pomodoro_count":2' -Label "case1-main-from-tmp"
    Assert-FileContains -Path $bak -Needle '"long_pomodoro_count":2' -Label "case1-bak-created"
    $results += "case1: PASS"

    # Case 2: main missing, tmp missing, bak valid -> main should be restored from bak.
    Remove-Item -LiteralPath $main -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $tmp -ErrorAction SilentlyContinue
    Write-JsonFile -Path $bak -Json $validJsonBak
    Start-And-StopApp -Path $ExePath -WaitSeconds $BootWaitSeconds
    Assert-FileContains -Path $main -Needle '"long_pomodoro_count":3' -Label "case2-main-from-bak"
    $results += "case2: PASS"

    Write-Host "Recovery chain fault injection test passed:"
    $results | ForEach-Object { Write-Host "  $_" }
}
finally {
    foreach ($f in $allFiles) {
        if (Test-Path -LiteralPath $f) {
            Remove-Item -LiteralPath $f -Force -ErrorAction SilentlyContinue
        }
    }

    foreach ($name in @("pomodoro_settings.json", "pomodoro_settings.json.tmp", "pomodoro_settings.json.bak")) {
        $src = Join-Path $backupDir $name
        if (Test-Path -LiteralPath $src) {
            Copy-Item -LiteralPath $src -Destination (Join-Path $repoRoot $name) -Force
        }
    }

    if (Test-Path -LiteralPath $backupDir) {
        Remove-Item -LiteralPath $backupDir -Recurse -Force
    }

    if ($regExisted) {
        if ($hadMode) {
            New-ItemProperty -Path $regPath -Name DataLocationMode -PropertyType DWord -Value $oldMode -Force | Out-Null
        } else {
            Remove-ItemProperty -Path $regPath -Name DataLocationMode -ErrorAction SilentlyContinue
        }
        if ($hadCustomPath) {
            New-ItemProperty -Path $regPath -Name CustomDataPath -PropertyType String -Value $oldCustomPath -Force | Out-Null
        } else {
            Remove-ItemProperty -Path $regPath -Name CustomDataPath -ErrorAction SilentlyContinue
        }
    } elseif (Test-Path $regPath) {
        Remove-Item -Path $regPath -Recurse -Force
    }
}
