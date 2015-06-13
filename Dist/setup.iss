[Setup]
OutputDir=.
OutputBaseFilename=CCTools
AppName=CCTools
AppVerName=CCTools 2.1.0
AppPublisher=Michael Hansen
AppVersion=2.1.0
AppID={{CB9B9C3F-2810-44FB-8659-A0C5F9E24F1D}
LicenseFile=LICENSE
ChangesAssociations=true
DefaultDirName={pf}\CCTools
AllowNoIcons=true
DefaultGroupName=CCTools 2.1
UninstallDisplayIcon={app}\CCEdit.exe
UninstallDisplayName=CCTools 2.1
Compression=lzma/ultra
InternalCompressLevel=ultra
SolidCompression=true
VersionInfoDescription=CCTools Setup
VersionInfoVersion=2.1.0
VersionInfoCompany=Michael Hansen
VersionInfoCopyright=Copyright (C) 2010 Michael Hansen
VersionInfoProductName=CCTools 2.1
VersionInfoProductVersion=2.1.0

[Types]
Name: Full; Description: Full Install
Name: Custom; Description: Custom; Flags: iscustom

[Components]
Name: CCEdit; Description: CCEdit Level Editor; Types: Custom Full
Name: CCPlay; Description: CCPlay Levelset Manager; Types: Custom Full
Name: CCHack; Description: CCHack Game Customizer (1.2a); Types: Custom Full

[Files]
Source: vcredist_x86.exe; DestDir: {tmp}; Flags: deleteafterinstall ignoreversion
Source: LICENSE; DestDir: {app}
Source: CCEdit.exe; DestDir: {app}; Flags: ignoreversion; Components: CCEdit
Source: CCPlay.exe; DestDir: {app}; Flags: ignoreversion; Components: CCPlay
Source: CCHack.exe; DestDir: {app}; Flags: ignoreversion; Components: CCHack
Source: Qt5Core.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay CCHack
Source: Qt5Gui.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay CCHack
Source: Qt5Sql.dll; DestDir: {app}; Flags: ignoreversion; Components: CCPlay
Source: Qt5Xml.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay
Source: Qt5Widgets.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay CCHack
Source: plugins\platforms\qwindows.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay CCHack
Source: TW32.tis; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay
Source: WEP.tis; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay

[Run]
Filename: {tmp}\vcredist_x86.exe; Parameters: /q

[UninstallDelete]
Name: {app}; Type: dirifempty

[Tasks]
Name: Associations; Description: Associate .ccl Files with CCEdit

[Registry]
Root: HKCR; SubKey: .ccl; ValueType: string; ValueData: CCLFile; Flags: uninsdeletekey; Tasks: Associations; Components: CCEdit
Root: HKCR; SubKey: CCLFile; ValueType: string; ValueData: Chip's Challenge Levelset; Flags: uninsdeletekey; Tasks: Associations; Components: CCEdit
Root: HKCR; SubKey: CCLFile\Shell\Edit\Command; ValueType: string; ValueData: """{app}\CCEdit.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: Associations; Components: CCEdit
Root: HKCR; Subkey: CCLFile\DefaultIcon; ValueType: string; ValueData: {app}\CCEdit.exe,1; Flags: uninsdeletevalue; Tasks: Associations; Components: CCEdit

[Icons]
Name: {group}\CCEdit; Filename: {app}\CCEdit.exe; WorkingDir: {app}; IconFilename: {app}\CCEdit.exe; IconIndex: 0; Components: CCEdit
Name: {group}\CCPlay; Filename: {app}\CCPlay.exe; WorkingDir: {app}; IconFilename: {app}\CCPlay.exe; IconIndex: 0; Components: CCPlay
Name: {group}\CCHack; Filename: {app}\CCHack.exe; WorkingDir: {app}; IconFilename: {app}\CCHack.exe; IconIndex: 0; Components: CCHack
