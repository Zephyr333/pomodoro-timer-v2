param([switch]$Interactive, [switch]$Stress, [switch]$UiCopyOnly, [switch]$ReadyOnly, [switch]$UxOnly, [switch]$FlexibleOnly, [switch]$MicroPreviewOnly, [switch]$SurfaceOnly, [switch]$ClickOnly, [switch]$WaitOvertimeOnly, [switch]$StrongOnly, [switch]$ReminderMenuOnly, [switch]$ConsistencyOnly, [switch]$ReliabilityOnly, [switch]$TrayLayoutOnly, [switch]$VisualOnly, [switch]$RepairOnly, [switch]$SealOnly, [switch]$HardenOnly, [switch]$ResumeOnly, [int]$GuardLive=-1, [switch]$FocusLive)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
Push-Location $projectRoot
try {
    $vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
    $vsInstall = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsInstall) { throw 'MSVC Build Tools are required for the native integration harness.' }
    $vcvars = Join-Path $vsInstall 'VC\Auxiliary\Build\vcvars64.bat'
    $runPath = 'tools\.fullscreen-test\' + (Get-Date -Format 'yyyyMMdd-HHmmss')
    New-Item -ItemType Directory -Path $runPath -Force | Out-Null
    if ($HardenOnly) {
        $fixtureCurrent = Join-Path (Get-Location) ($runPath + '\identity-current')
        $fixtureAlias = Join-Path (Get-Location) ($runPath + '\identity-alias')
        New-Item -ItemType Directory -Path $fixtureCurrent -Force | Out-Null
        New-Item -ItemType Junction -Path $fixtureAlias -Target $fixtureCurrent | Out-Null
    }

    $build = 'call "' + $vcvars + '" >nul && rc.exe /nologo /DNO_MANIFEST /fo "' + $runPath + '\test.res" pomodoro-timer.rc && cl.exe /nologo /utf-8 /D_WIN32_WINNT=0x0600 /D_CRT_SECURE_NO_WARNINGS /W4 /O2 /Fe:"' + $runPath + '\fullscreen_test.exe" /Fo:"' + $runPath + '\test.obj" tools\fullscreen_test.c "' + $runPath + '\test.res" /link user32.lib advapi32.lib winmm.lib shell32.lib ole32.lib gdi32.lib comdlg32.lib'
    & cmd.exe /d /s /c $build
    if ($LASTEXITCODE -ne 0) { throw 'Integration harness build failed.' }
    if ($HardenOnly) {
        $helperBuild = 'call "' + $vcvars + '" >nul && cl.exe /nologo /utf-8 /D_CRT_SECURE_NO_WARNINGS /W4 /O2 /Fe:"' + $runPath + '\taskbar_fixture.exe" /Fo:"' + $runPath + '\fixture.obj" tools\taskbar_fixture.c /link user32.lib'
        & cmd.exe /d /s /c $helperBuild
        if ($LASTEXITCODE -ne 0) { throw 'Owned taskbar fixture build failed.' }
    }

    $testArgs = @($runPath)
    if ($Interactive) { $testArgs += '--interactive' }
    if ($Stress) { $testArgs += '--stress' }
    if ($UiCopyOnly) { $testArgs += '--ui-copy-only' }
    if ($ReadyOnly) { $testArgs += '--ready-only' }
    if ($UxOnly) { $testArgs += '--ux-only' }
    if ($FlexibleOnly) { $testArgs += '--flexible-only' }
    if ($MicroPreviewOnly) { $testArgs += '--micro-preview-only' }
    if ($SurfaceOnly) { $testArgs += '--surface-only' }
    if ($ClickOnly) { $testArgs += '--click-only' }
    if ($WaitOvertimeOnly) { $testArgs += '--wait-overtime-only' }
    if ($StrongOnly) { $testArgs += '--strong-only' }
    if ($ReminderMenuOnly) { $testArgs += '--reminder-menu-only' }
    if ($ConsistencyOnly) { $testArgs += '--consistency-only' }
    if ($ReliabilityOnly) { $testArgs += '--reliability-only' }
    if ($TrayLayoutOnly) { $testArgs += '--tray-layout-only' }
    if ($VisualOnly) { $testArgs += '--visual-only' }
    if ($RepairOnly) { $testArgs += '--repair-only' }
    if ($SealOnly) { $testArgs += '--seal-only' }
    if ($HardenOnly) { $testArgs += '--harden-only' }
    if ($ResumeOnly) { $testArgs += '--resume-only' }
    if ($GuardLive -ge 0) { $testArgs += @('--guard-live', [string]$GuardLive) }
    if ($FocusLive) { $testArgs += '--focus-live' }
    $process = Start-Process -FilePath (Join-Path $runPath 'fullscreen_test.exe') -ArgumentList $testArgs -PassThru -WindowStyle Hidden -RedirectStandardOutput (Join-Path $runPath 'results.txt') -RedirectStandardError (Join-Path $runPath 'errors.txt')
    if (-not $process.WaitForExit(45000)) {
        Stop-Process -Id $process.Id -Force
        throw 'Integration harness timed out; only the test process was stopped.'
    }
    $process.WaitForExit()
    $results = Get-Content -LiteralPath (Join-Path $runPath 'results.txt')
    $results
    Get-Content -LiteralPath (Join-Path $runPath 'errors.txt')
    Write-Host "Test artifacts: $projectRoot\$runPath"
    if ($null -ne $process.ExitCode -and $process.ExitCode -ne 0) { throw "Integration tests failed: $($process.ExitCode)" }
    $passPattern = if ($ResumeOnly) { 'RESUME_TEST: PASS \(0 failures\)' } elseif ($HardenOnly) { 'HARDEN_TEST: PASS \(0 failures\)' } elseif ($GuardLive -ge 0) { 'GUARD_LIVE_TEST: PASS \(0 failures\)' } elseif ($FocusLive) { 'FOCUS_LIVE_TEST: PASS \(0 failures\)' } elseif ($SealOnly) { 'SEAL_FIX_TEST: PASS \(0 failures\)' } elseif ($RepairOnly) { 'GUI_REPAIR_TEST: PASS \(0 failures\)' } elseif ($VisualOnly) { 'VISUAL_STABILITY_TEST: PASS \(0 failures\)' } elseif ($TrayLayoutOnly) { 'TRAY_LAYOUT_TEST: PASS \(0 failures\)' } elseif ($ReliabilityOnly) { 'RELIABILITY_TEST: PASS \(0 failures\)' } elseif ($ConsistencyOnly) { 'SESSION_CONSISTENCY_TEST: PASS \(0 failures\)' } elseif ($ReminderMenuOnly) { 'REMINDER_MENU_TEST: PASS \(0 failures\)' } elseif ($StrongOnly) { 'STRONG_REMINDER_TEST: PASS \(0 failures\)' } elseif ($WaitOvertimeOnly) { 'MICRO_WAIT_OVERTIME_TEST: PASS \(0 failures\)' } elseif ($ClickOnly) { 'FULLSCREEN_CLICK_TEST: PASS \(0 failures\)' } elseif ($SurfaceOnly) { 'SURFACE_BUG_TEST: PASS \(0 failures\)' } elseif ($MicroPreviewOnly) { 'MICRO_PREVIEW_TEST: PASS \(0 failures\)' } elseif ($FlexibleOnly) { 'FLEXIBLE_CONTROLS_TEST: PASS \(0 failures\)' } elseif ($UxOnly) { 'UX_CONSISTENCY_TEST: PASS \(0 failures\)' } elseif ($ReadyOnly) { 'READY_STATE_TEST: PASS \(0 failures\)' } elseif ($UiCopyOnly) { 'UI_COPY_TEST: PASS \(0 failures\)' } else { 'FULLSCREEN_TEST: PASS \(0 failures\)' }
    if (-not ($results -match $passPattern)) { throw 'Integration tests failed.' }
} finally {
    Pop-Location
}
