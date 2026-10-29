# PowerShell Build Script for Trie-Based Spell Checker

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Building Trie-Based Spell Checker & Autocorrect (C++20)" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

$BinDir = Join-Path $PSScriptRoot "bin"
if (-not (Test-Path $BinDir)) {
    New-Item -ItemType Directory -Path $BinDir | Out-Null
}

$IncludeDir = Join-Path $PSScriptRoot "include"
$MainSrc = Join-Path $PSScriptRoot "src\main.cpp"
$BenchSrc = Join-Path $PSScriptRoot "src\benchmark.cpp"
$MainExe = Join-Path $BinDir "spell_check.exe"
$BenchExe = Join-Path $BinDir "benchmark.exe"

$Gxx = "g++"
$CommonFlags = @("-std=c++20", "-O3", "-Wall", "-Wextra", "-I$IncludeDir", "-pthread")

Write-Host "Compiling interactive spell checker -> $MainExe..." -ForegroundColor Yellow
$proc1 = Start-Process -FilePath $Gxx -ArgumentList ($CommonFlags + @($MainSrc, "-o", $MainExe)) -Wait -PassThru -NoNewWindow
if ($proc1.ExitCode -ne 0) {
    Write-Host "Failed to compile spell_check.exe!" -ForegroundColor Red
    exit 1
}

Write-Host "Compiling benchmark suite -> $BenchExe..." -ForegroundColor Yellow
$proc2 = Start-Process -FilePath $Gxx -ArgumentList ($CommonFlags + @($BenchSrc, "-o", $BenchExe)) -Wait -PassThru -NoNewWindow
if ($proc2.ExitCode -ne 0) {
    Write-Host "Failed to compile benchmark.exe!" -ForegroundColor Red
    exit 1
}

Write-Host "Build Succeeded!" -ForegroundColor Green
Write-Host "Executables located at:" -ForegroundColor Green
Write-Host "  $MainExe" -ForegroundColor White
Write-Host "  $BenchExe" -ForegroundColor White
