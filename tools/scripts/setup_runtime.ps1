# Setup Portable Python Runtime for Library App
# Run this script to auto-download and configure a standalone Python environment.

$ErrorActionPreference = "Stop"

$pythonUrl = "https://www.python.org/ftp/python/3.11.5/python-3.11.5-embed-amd64.zip"
$pipUrl = "https://bootstrap.pypa.io/get-pip.py"
$runtimeDir = "python_runtime"

Write-Host ">>> Starting Setup for Portable Python Runtime..." -ForegroundColor Cyan

# 1. Download Python Embeddable
if (-Not (Test-Path "$runtimeDir\python.exe")) {
    Write-Host "Downloading Python 3.11 Embeddable..."
    Invoke-WebRequest -Uri $pythonUrl -OutFile "python.zip"
    
    Write-Host "Extracting Python..."
    Expand-Archive -Path "python.zip" -DestinationPath $runtimeDir -Force
    Remove-Item "python.zip"
} else {
    Write-Host "Python runtime already exists."
}

# 2. Configure .pth file to enable 'import site' (Crucial for pip)
# The default python311._pth has 'import site' commented out. We must enable it.
$pthFile = Get-ChildItem "$runtimeDir\*._pth" | Select-Object -First 1
if ($pthFile) {
    Write-Host "Configuring $($pthFile.Name)..."
    $content = Get-Content $pthFile.FullName
    if ($content -notcontains "import site") {
        Add-Content -Path $pthFile.FullName -Value "import site"
        Write-Host "Enabled 'import site' in .pth file." -ForegroundColor Green
    }
}

# 3. Install Pip
if (-Not (Test-Path "$runtimeDir\Scripts\pip.exe")) {
    Write-Host "Downloading get-pip.py..."
    Invoke-WebRequest -Uri $pipUrl -OutFile "$runtimeDir\get-pip.py"
    
    Write-Host "Installing pip..."
    Start-Process -FilePath "$runtimeDir\python.exe" -ArgumentList "$runtimeDir\get-pip.py" -Wait -NoNewWindow
    
    Remove-Item "$runtimeDir\get-pip.py"
}

# 4. Install Dependencies (openpyxl)
Write-Host "Installing required libraries (openpyxl)..."
Start-Process -FilePath "$runtimeDir\python.exe" -ArgumentList "-m pip install openpyxl pandas" -Wait -NoNewWindow // Added pandas just in case

Write-Host ">>> SETUP COMPLETED SUCCESSFULLY!" -ForegroundColor Green
Write-Host "Portable Python is ready at: $PWD\$runtimeDir"
Write-Host "You can now distribute this folder with your application."
Start-Sleep -Seconds 3
