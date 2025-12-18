param(
    [string]$BuildDir = "release_v2",
    [string]$DeployDir = "QuanLyThuVien_Portable"
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  PACKAGING LIBRARY MANAGEMENT APP" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$exePath = Join-Path $BuildDir "QuanLyThuVien.exe"
if (-not (Test-Path $exePath)) {
    Write-Host "[ERROR] File not found: $exePath" -ForegroundColor Red
    exit 1
}

Write-Host "[OK] Found: $exePath" -ForegroundColor Green

if (Test-Path $DeployDir) {
    Remove-Item -Recurse -Force $DeployDir
}

New-Item -ItemType Directory -Path $DeployDir | Out-Null
Write-Host "[OK] Created: $DeployDir`n" -ForegroundColor Green

Copy-Item $exePath $DeployDir
Write-Host "[OK] Copied executable" -ForegroundColor Green

$windeployqt = "C:\Qt\6.10.1\mingw_64\bin\windeployqt.exe"
$deployExe = Join-Path $DeployDir "QuanLyThuVien.exe"

if (Test-Path $windeployqt) {
    Write-Host "`nRunning windeployqt..." -ForegroundColor Cyan
    & $windeployqt $deployExe --release --no-translations --no-system-d3d-compiler --no-opengl-sw
    Write-Host "[OK] Qt dependencies collected" -ForegroundColor Green
}

Write-Host "`nCopying MinGW DLLs..." -ForegroundColor Cyan
$mingwBin = "C:\Qt\Tools\mingw1310_64\bin"
$mingwDlls = @("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")

foreach ($dll in $mingwDlls) {
    $dllPath = Join-Path $mingwBin $dll
    if (Test-Path $dllPath) {
        Copy-Item $dllPath $DeployDir
        Write-Host "  [OK] $dll" -ForegroundColor Green
    }
}

if (Test-Path "files") {
    Write-Host "`nCopying data folder..." -ForegroundColor Cyan
    Copy-Item -Recurse "files" $DeployDir
    Write-Host "[OK] Data copied" -ForegroundColor Green
}

$size = (Get-ChildItem -Recurse $DeployDir | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "[SUCCESS] COMPLETED!" -ForegroundColor Green
Write-Host "Folder: $DeployDir" -ForegroundColor White
Write-Host "Size: $([math]::Round($size, 2)) MB" -ForegroundColor White
Write-Host "========================================`n" -ForegroundColor Cyan
