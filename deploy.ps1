param(
	[string]$BuildDir = "release_v2",
	[string]$DeployDir = "QuanLyThuVien_Portable",
	[string]$QtDir = "C:\Qt\6.10.1\mingw_64",
	[string]$MingwDir = "C:\Qt\Tools\mingw1310_64\bin",
	[string]$InnoSetupPath = ""  # Optional path to Inno Setup's ISCC.exe
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  PACKAGING QUANLYTHUVIEN APP" -ForegroundColor Cyan
Write-Host "========================================`n" -ForegroundColor Cyan

$exeName = "QuanLyThuVien.exe"
$exePath = Join-Path $BuildDir $exeName
if (-not (Test-Path $exePath)) {
	Write-Host "[ERROR] File not found: $exePath" -ForegroundColor Red
	Write-Host "Build the project in Release mode first (DESTDIR=$BuildDir)." -ForegroundColor Yellow
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

$windeployqt = Join-Path $QtDir "bin\windeployqt.exe"
$deployExe = Join-Path $DeployDir $exeName

if (Test-Path $windeployqt) {
	Write-Host "`nRunning windeployqt..." -ForegroundColor Cyan
	& $windeployqt $deployExe --release --no-translations --no-system-d3d-compiler --no-opengl-sw
	Write-Host "[OK] Qt dependencies collected" -ForegroundColor Green
} else {
	Write-Host "[WARN] windeployqt not found at $windeployqt. Please install Qt or set -QtDir accordingly." -ForegroundColor Yellow
}

Write-Host "`nCopying MinGW DLLs..." -ForegroundColor Cyan
$mingwDlls = @("libgcc_s_seh-1.dll", "libstdc++-6.dll", "libwinpthread-1.dll")

foreach ($dll in $mingwDlls) {
	$dllPath = Join-Path $MingwDir $dll
	if (Test-Path $dllPath) {
		Copy-Item $dllPath $DeployDir
		Write-Host "  [OK] $dll" -ForegroundColor Green
	} else {
		Write-Host "  [WARN] $dll not found in $MingwDir" -ForegroundColor Yellow
	}
}

# Copy data and files folders if present
if (Test-Path "data") {
	Write-Host "`nCopying data folder..." -ForegroundColor Cyan
	Copy-Item -Recurse -Force "data" (Join-Path $DeployDir "data")
	Write-Host "[OK] Data copied" -ForegroundColor Green
}

if (Test-Path "files") {
	Write-Host "`nCopying files folder..." -ForegroundColor Cyan
	Copy-Item -Recurse -Force "files" (Join-Path $DeployDir "files")
	Write-Host "[OK] Files copied" -ForegroundColor Green
}

# Optional: create a zip as portable package
$zipName = "QuanLyThuVien_Portable.zip"
if (Test-Path $zipName) { Remove-Item $zipName -Force }
Write-Host "`nCompressing to $zipName..." -ForegroundColor Cyan
Compress-Archive -Path $DeployDir\* -DestinationPath $zipName
Write-Host "[OK] Created: $zipName" -ForegroundColor Green

# Optional: build installer with Inno Setup if ISCC path provided
if ($InnoSetupPath -ne "" -and (Test-Path $InnoSetupPath)) {
	Write-Host "`nBuilding installer with Inno Setup..." -ForegroundColor Cyan
	$iss = Join-Path (Split-Path -Parent $PSCommandPath) "installer\QuanLyThuVien.iss"
	if (-not (Test-Path $iss)) {
		Write-Host "[ERROR] Inno Setup script not found: $iss" -ForegroundColor Red
	} else {
		& $InnoSetupPath $iss
		Write-Host "[OK] Installer build attempted (check ISCC output)" -ForegroundColor Green
	}
} else {
	Write-Host "`nSkipping Inno Setup build. To enable installer creation, pass -InnoSetupPath 'C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe'" -ForegroundColor Yellow
}

$size = (Get-ChildItem -Recurse $DeployDir | Measure-Object -Property Length -Sum).Sum / 1MB
Write-Host "`n========================================" -ForegroundColor Cyan
Write-Host "[SUCCESS] COMPLETED!" -ForegroundColor Green
Write-Host "Folder: $DeployDir" -ForegroundColor White
Write-Host "Zip: $zipName" -ForegroundColor White
Write-Host "Size: $([math]::Round($size, 2)) MB" -ForegroundColor White
Write-Host "========================================`n" -ForegroundColor Cyan

