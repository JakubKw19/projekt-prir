# Buduje i uruchamia wersje referencyjna (MSVC).
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

$candidates = @(
    "${env:ProgramFiles}\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
    "${env:ProgramFiles}\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
    "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $found = & $vswhere -latest -products * -find "VC\Auxiliary\Build\vcvars64.bat" 2>$null
    if ($found) { $candidates = @($found) + $candidates }
}

$vcvars = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $vcvars) {
    Write-Error "Nie znaleziono vcvars64.bat. Zbuduj przez CMake albo uzupelnij sciezke w build.ps1."
}

New-Item -ItemType Directory -Force -Path "$root\build" | Out-Null
$cmd = "call `"$vcvars`" && cl /nologo /EHsc /std:c++17 /O2 /W4 /utf-8 /I `"$root\include`" `"$root\src\main.cpp`" `"$root\src\reference.cpp`" `"$root\src\generate.cpp`" /Fe:`"$root\build\compact_ref.exe`" /Fo:`"$root\build\\`""
cmd /c $cmd
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& "$root\build\compact_ref.exe"
exit $LASTEXITCODE
