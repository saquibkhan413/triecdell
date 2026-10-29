# Packaging & Deployment Script for Windows Release

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Packaging Standalone Windows Release for Spell Checker" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

$RootDir = $PSScriptRoot
$DistDir = Join-Path $RootDir "dist"
$DistDataDir = Join-Path $DistDir "data"

# 1. Clean and recreate dist/ directory
if (Test-Path $DistDir) {
    Remove-Item -Path $DistDir -Recurse -Force | Out-Null
}
New-Item -ItemType Directory -Path $DistDataDir -Force | Out-Null

$MainSrc = Join-Path $RootDir "src\main.cpp"
$IncludeDir = Join-Path $RootDir "include"
$DistExe = Join-Path $DistDir "spell_check.exe"
$DictSrc = Join-Path $RootDir "data\dictionary.txt"
$DictDest = Join-Path $DistDataDir "dictionary.txt"

# 2. Compile with static linking for portability across all Windows PCs
Write-Host "Compiling standalone static binary -> $DistExe..." -ForegroundColor Yellow
$Gxx = "g++"
$CompileArgs = @(
    "-std=c++20",
    "-O3",
    "-static",
    "-static-libgcc",
    "-static-libstdc++",
    "-I$IncludeDir",
    "-pthread",
    $MainSrc,
    "-o",
    $DistExe
)

$proc = Start-Process -FilePath $Gxx -ArgumentList $CompileArgs -Wait -PassThru -NoNewWindow
if ($proc.ExitCode -ne 0) {
    Write-Host "Failed to compile static binary!" -ForegroundColor Red
    exit 1
}

# 3. Copy dictionary.txt
Write-Host "Copying frequency dictionary..." -ForegroundColor Yellow
Copy-Item -Path $DictSrc -Destination $DictDest -Force

# 4. Create double-clickable run.bat launcher
$RunBatPath = Join-Path $DistDir "run.bat"
$RunBatContent = @"
@echo off
title Trie-Based Spell Checker
color 0b
echo ======================================================
echo Launching Trie-Based Spell Checker & Autocorrect...
echo ======================================================
spell_check.exe
pause
"@
Set-Content -Path $RunBatPath -Value $RunBatContent

# 5. Create README_RELEASE.txt
$ReadmePath = Join-Path $DistDir "README.txt"
$ReadmeContent = @"
Trie-Based Spell Checker & Autocorrect Engine (C++20)
======================================================

How to Run:
1. Double-click 'run.bat' or open a terminal and run '.\spell_check.exe'.
2. Type any sentence, e.g.:
   aple cat
   Helo wrld, this is a computr test.

Commands:
- <sentence>        : Real-time autocorrect
- word <word>       : Detailed edit-distance & frequency ranking
- suggest <prefix>  : Prefix autocomplete
- demo              : Automated test run
- stats             : Trie and LRU cache metrics
- exit              : Quit
"@
Set-Content -Path $ReadmePath -Value $ReadmeContent

# 6. Create Zip archive
$ZipFile = Join-Path $RootDir "spell_checker_v1.0_windows.zip"
if (Test-Path $ZipFile) {
    Remove-Item -Path $ZipFile -Force | Out-Null
}
Write-Host "Compressing into $ZipFile..." -ForegroundColor Yellow
Compress-Archive -Path "$DistDir\*" -DestinationPath $ZipFile -Force

Write-Host ""
Write-Host "Deployment Package Created Successfully!" -ForegroundColor Green
Write-Host "  Folder:  $DistDir" -ForegroundColor White
Write-Host "  Zip:     $ZipFile" -ForegroundColor White
