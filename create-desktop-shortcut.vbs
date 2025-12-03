Set objShell = CreateObject("WScript.Shell")
Set objFSO = CreateObject("Scripting.FileSystemObject")

' Get desktop path
strDesktop = objShell.SpecialFolders("Desktop")

' Get current directory
strCurrentDir = objFSO.GetParentFolderName(WScript.ScriptFullName)

' Create shortcut
Set objShortcut = objShell.CreateShortcut(strDesktop & "\Quản Lý Thư Viện.lnk")
objShortcut.TargetPath = strCurrentDir & "\release\LibraryManagement.exe"
objShortcut.WorkingDirectory = strCurrentDir
objShortcut.Description = "Hệ Thống Quản Lý Thư Viện - Qt Desktop App"
objShortcut.IconLocation = strCurrentDir & "\release\LibraryManagement.exe,0"
objShortcut.Save

WScript.Echo "✅ Shortcut created on Desktop!"
