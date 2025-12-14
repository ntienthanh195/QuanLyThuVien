; Inno Setup script for QuanLyThuVien
; Usage: run ISCC.exe QuanLyThuVien.iss from project root

[Setup]
AppName=QuanLyThuVien
AppVersion=1.0
; Install to Program Files (all users)
DefaultDirName={commonpf}\QuanLyThuVien
DefaultGroupName=QuanLyThuVien
OutputBaseFilename=QuanLyThuVien_Installer
Compression=lzma
SolidCompression=yes
; Use project icon if present
SetupIconFile=..\resources\app_icon.ico
; WizardImageFile prefers BMP; if you have a custom banner use WizardImageFile
;WizardImageFile=..\resources\installer_banner.bmp

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; Include all files from the portable folder created by deploy.ps1 (project root)
Source: "..\QuanLyThuVien_Portable\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs

[Icons]
Name: "{group}\QuanLyThuVien"; Filename: "{app}\QuanLyThuVien.exe"; IconFilename: "{app}\QuanLyThuVien.exe"
Name: "{commondesktop}\QuanLyThuVien"; Filename: "{app}\QuanLyThuVien.exe"; Tasks: create_desktop_icon; IconFilename: "{app}\QuanLyThuVien.exe"

[Tasks]
Name: create_desktop_icon; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

[Run]
Filename: "{app}\QuanLyThuVien.exe"; Description: "Launch QuanLyThuVien"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; Remove installed data files if the user uninstalls (optional)
;Type: files; Name: "{app}\data\*"
