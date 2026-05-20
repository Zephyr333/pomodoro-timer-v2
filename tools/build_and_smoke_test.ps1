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

if ((-not $gcc -or -not $windres) -and -not $zig) {
    $localZig = Get-ChildItem -Path '.\\tools\\zig' -Filter 'zig.exe' -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($localZig) { $zig = $localZig.FullName }
}

if ($gcc -and $windres) {
    Write-Host "Using gcc: $gcc"
    Write-Host "Using windres: $windres"
    Remove-Item '.\\pomodoro-timer.exe' -ErrorAction SilentlyContinue
    & $windres 'pomodoro-timer.rc' -o 'pomodoro-timer_res.o'
    if ($LASTEXITCODE -ne 0) { throw 'windres failed' }
    & $gcc -ffunction-sections -fdata-sections -s -o 'pomodoro-timer.exe' 'pomodoro-timer.c' 'pomodoro-timer_res.o' -mwindows -lwinmm -lole32 '-Wl,--gc-sections' -static-libgcc
    if ($LASTEXITCODE -ne 0) { throw 'gcc failed' }
}
elseif ($zig) {
    Write-Host "gcc/windres not found. Falling back to zig: $zig" -ForegroundColor Yellow
    Remove-Item '.\\pomodoro-timer.exe' -ErrorAction SilentlyContinue
    & $zig rc '/fo' 'pomodoro-timer.res' 'pomodoro-timer.rc'
    if ($LASTEXITCODE -ne 0) { throw 'zig rc failed' }
    & $zig cc 'pomodoro-timer.c' 'pomodoro-timer.res' -municode -lwinmm -lgdi32 -lshell32 -lole32 '-Wl,--subsystem,windows' -o 'pomodoro-timer.exe'
    if ($LASTEXITCODE -ne 0) { throw 'zig cc failed' }
}
else {
    Write-Host 'No gcc/windres or zig compiler found. Build cannot continue.' -ForegroundColor Red
    exit 1
}

if (-not (Test-Path '.\\pomodoro-timer.exe')) {
    Write-Host 'Build failed: pomodoro-timer.exe was not generated.' -ForegroundColor Red
    exit 1
}

Write-Host 'Build succeeded. Running smoke test (process should stay alive > 6s)...'
$proc = Start-Process -FilePath '.\\pomodoro-timer.exe' -PassThru

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
