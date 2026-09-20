param([switch]$Interactive, [switch]$Stress)

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
    $build = 'call "' + $vcvars + '" >nul && rc.exe /nologo /DNO_MANIFEST /fo "' + $runPath + '\test.res" pomodoro-timer.rc && cl.exe /nologo /utf-8 /D_WIN32_WINNT=0x0600 /D_CRT_SECURE_NO_WARNINGS /W4 /O2 /Fe:"' + $runPath + '\fullscreen_test.exe" /Fo:"' + $runPath + '\test.obj" tools\fullscreen_test.c "' + $runPath + '\test.res" /link user32.lib advapi32.lib winmm.lib shell32.lib ole32.lib gdi32.lib comdlg32.lib'
    & cmd.exe /d /s /c $build
    if ($LASTEXITCODE -ne 0) { throw 'Integration harness build failed.' }
    $testArgs = @($runPath)
    if ($Interactive) { $testArgs += '--interactive' }
    if ($Stress) { $testArgs += '--stress' }
    $process = Start-Process -FilePath (Join-Path $runPath 'fullscreen_test.exe') -ArgumentList $testArgs -PassThru -WindowStyle Hidden -RedirectStandardOutput (Join-Path $runPath 'results.txt') -RedirectStandardError (Join-Path $runPath 'errors.txt')
    if (-not $process.WaitForExit(45000)) {
        Stop-Process -Id $process.Id -Force
        throw 'Integration harness timed out; only the test process was stopped.'
    }
    Get-Content -LiteralPath (Join-Path $runPath 'results.txt')
    Get-Content -LiteralPath (Join-Path $runPath 'errors.txt')
    Write-Host "Test artifacts: $projectRoot\$runPath"
    if ($process.ExitCode -ne 0) { throw "Integration tests failed: $($process.ExitCode)" }
} finally {
    Pop-Location
}
