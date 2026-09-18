param([switch]$BuildOnly)

$ErrorActionPreference = 'Stop'

function Resolve-Tool([string[]]$names) {
    foreach ($name in $names) {
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Source }
    }
    return $null
}

$gcc = Resolve-Tool @('gcc')
$windres = Resolve-Tool @('windres')
$zig = Resolve-Tool @('zig')
$vsDevCmd = $null

if ((-not $gcc -or -not $windres) -and -not $zig) {
    $localZig = Get-ChildItem -Path '.\\tools\\zig' -Filter 'zig.exe' -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($localZig) { $zig = $localZig.FullName }
}

if ((-not $gcc -or -not $windres) -and -not $zig) {
    $vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $vsInstall = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsInstall) {
            $candidate = Join-Path $vsInstall 'VC\Auxiliary\Build\vcvars64.bat'
            if (Test-Path -LiteralPath $candidate) { $vsDevCmd = $candidate }
        }
    }
}

$projectExe = Join-Path (Get-Location) 'pomodoro-timer.exe'
$running = Get-Process -Name 'pomodoro-timer' -ErrorAction SilentlyContinue | Where-Object {
    $_.Path -and $_.Path -eq $projectExe
}
if ($running) {
    throw 'The project pomodoro-timer.exe is still running. Exit it before building.'
}

if ($gcc -and $windres) {
    Write-Host "Using gcc: $gcc"
    Write-Host "Using windres: $windres"
    Remove-Item '.\\pomodoro-timer.exe' -ErrorAction SilentlyContinue
    & $windres 'pomodoro-timer.rc' -o 'pomodoro-timer_res.o'
    if ($LASTEXITCODE -ne 0) { throw 'windres failed' }
    & $gcc -ffunction-sections -fdata-sections -s -o 'pomodoro-timer.exe' 'pomodoro-timer.c' 'pomodoro-timer_res.o' -mwindows -lwinmm -lole32 -lgdi32 -lshell32 -lcomdlg32 '-Wl,--gc-sections' -static-libgcc
    if ($LASTEXITCODE -ne 0) { throw 'gcc failed' }
}
elseif ($zig) {
    Write-Host "gcc/windres not found. Falling back to zig: $zig" -ForegroundColor Yellow
    Remove-Item '.\\pomodoro-timer.exe' -ErrorAction SilentlyContinue
    & $zig rc '/fo' 'pomodoro-timer.res' 'pomodoro-timer.rc'
    if ($LASTEXITCODE -ne 0) { throw 'zig rc failed' }
    & $zig cc 'pomodoro-timer.c' 'pomodoro-timer.res' -municode -lcomdlg32 -lwinmm -lgdi32 -lshell32 -lole32 '-Wl,--subsystem,windows' -o 'pomodoro-timer.exe'
    if ($LASTEXITCODE -ne 0) { throw 'zig cc failed' }
}
elseif ($vsDevCmd) {
    Write-Host "gcc/windres and zig not found. Falling back to MSVC: $vsDevCmd" -ForegroundColor Yellow
    Remove-Item '.\\pomodoro-timer.exe' -ErrorAction SilentlyContinue
    $buildCommand = 'call "' + $vsDevCmd + '" >nul && rc.exe /nologo /fo pomodoro-timer.res pomodoro-timer.rc && cl.exe /nologo /utf-8 /D_WIN32_WINNT=0x0600 /D_CRT_SECURE_NO_WARNINGS /W4 /O2 /Fe:pomodoro-timer.exe pomodoro-timer.c pomodoro-timer.res /link /SUBSYSTEM:WINDOWS user32.lib advapi32.lib winmm.lib shell32.lib ole32.lib gdi32.lib comdlg32.lib'
    & cmd.exe /d /s /c $buildCommand
    if ($LASTEXITCODE -ne 0) { throw 'MSVC build failed' }
}
else {
    Write-Host 'No gcc/windres, zig, or MSVC Build Tools found. Build cannot continue.' -ForegroundColor Red
    exit 1
}

if (-not (Test-Path '.\\pomodoro-timer.exe')) {
    Write-Host 'Build failed: pomodoro-timer.exe was not generated.' -ForegroundColor Red
    exit 1
}

if ($BuildOnly) {
    Write-Host 'Build succeeded (build only).'
    exit 0
}
if (Get-Process -Name 'pomodoro-timer' -ErrorAction SilentlyContinue) {
    throw 'A pomodoro-timer instance is running. Build succeeded; exit it before smoke testing, or use -BuildOnly.'
}

Write-Host 'Build succeeded. Running smoke test (process should stay alive > 6s)...'
$proc = Start-Process -FilePath '.\\pomodoro-timer.exe' -PassThru -WindowStyle Hidden

Wait-Process -Id $proc.Id -Timeout 6 -ErrorAction SilentlyContinue
if (-not (Get-Process -Id $proc.Id -ErrorAction SilentlyContinue)) {
    Write-Host 'Smoke test failed: process exited within 6 seconds.' -ForegroundColor Yellow
    exit 1
}

Write-Host 'Smoke test passed: process stayed alive > 6 seconds.' -ForegroundColor Green

if (Get-Process -Id $proc.Id -ErrorAction SilentlyContinue) {
    Stop-Process -Id $proc.Id -Force
    Write-Host 'Stopped smoke-test process.' -ForegroundColor Gray
}

Write-Host 'Done. Output: .\\pomodoro-timer.exe' -ForegroundColor Green
