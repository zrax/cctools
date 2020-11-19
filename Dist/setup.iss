[Setup]
OutputDir=.
OutputBaseFilename=CCTools
AppName=CCTools
AppVerName=CCTools 3.0 Beta1
AppPublisher=Michael Hansen
AppVersion=2.95.0
AppID={{CB9B9C3F-2810-44FB-8659-A0C5F9E24F1D}
LicenseFile=..\LICENSE
ChangesAssociations=true
DefaultDirName={pf}\CCTools
AllowNoIcons=true
DefaultGroupName=CCTools 3.0
UninstallDisplayIcon={app}\CCEdit.exe
UninstallDisplayName=CCTools 3.0
Compression=lzma/ultra
InternalCompressLevel=ultra
SolidCompression=true
VersionInfoDescription=CCTools Setup
VersionInfoVersion=2.95.0
VersionInfoCompany=Michael Hansen
VersionInfoCopyright=Copyright (C) 2020 Michael Hansen
VersionInfoProductName=CCTools 3.0 Beta1
VersionInfoProductVersion=2.95.0

[Types]
Name: Full; Description: Full Install
Name: Custom; Description: Custom; Flags: iscustom

[Components]
Name: CCEdit; Description: CCEdit Level Editor; Types: Custom Full
Name: CC2Edit; Description: CC2Edit Map and Game Editor; Types: Custom Full
Name: CCPlay; Description: CCPlay Levelset Manager; Types: Custom Full
Name: CCHack; Description: CCHack Game Customizer; Types: Custom Full

[Files]
Source: vc_redist.x86.exe; DestDir: {tmp}; Flags: deleteafterinstall; Check: NeedMSVCRTInstaller
Source: ..\changelog.txt; DestDir: {app}
Source: ..\LICENSE; DestDir: {app}
Source: ..\LICENSE-oxygen-icon-theme; DestDir: {app}
Source: CCEdit.exe; DestDir: {app}; Flags: ignoreversion; Components: CCEdit
Source: CC2Edit.exe; DestDir: {app}; Flags: ignoreversion; Components: CC2Edit
Source: CCPlay.exe; DestDir: {app}; Flags: ignoreversion; Components: CCPlay
Source: CCHack.exe; DestDir: {app}; Flags: ignoreversion; Components: CCHack
Source: KF5SyntaxHighlighting.dll; DestDir: {app}; Flags: ignoreversion; Components: CC2Edit
Source: Qt5Core.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: Qt5Gui.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: Qt5Network.dll; DestDir: {app}; Flags: ignoreversion; Components: CC2Edit
Source: Qt5Sql.dll; DestDir: {app}; Flags: ignoreversion; Components: CCPlay
Source: Qt5Widgets.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: Qt5Xml.dll; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: bearer\qgenericbearer.dll; DestDir: {app}\bearer; Flags: ignoreversion; Components: CC2Edit
Source: imageformats\qico.dll; DestDir: {app}\imageformats; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: imageformats\qwbmp.dll; DestDir: {app}\imageformats; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: platforms\qwindows.dll; DestDir: {app}\platforms; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: sqldrivers\qsqlite.dll; DestDir: {app}\sqldrivers; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: styles\qwindowsvistastyle.dll; DestDir: {app}\styles; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_ar.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_bg.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_ca.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_cs.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_da.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_de.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_en.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_es.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_fi.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_fr.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_gd.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_he.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_hu.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_it.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_ja.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_ko.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_lv.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_pl.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_ru.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_sk.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_uk.qm; DestDir: {app}\translations; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: translations\qt_zh_TW.qm; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CC2Edit CCPlay CCHack
Source: CC2.tis; DestDir: {app}; Flags: ignoreversion; Components: CC2Edit
Source: TW32.tis; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay
Source: WEP.tis; DestDir: {app}; Flags: ignoreversion; Components: CCEdit CCPlay

[Run]
Filename: {tmp}\vc_redist.x86.exe; Parameters: "/norestart /quiet"; Check: NeedMSVCRTInstaller

[InstallDelete]
Name: {app}\libpng15.dll; Type: files
Name: {app}\QtCore4.dll; Type: files
Name: {app}\QtGui4.dll; Type: files
Name: {app}\QtSql4.dll; Type: files
Name: {app}\QtXml4.dll; Type: files
Name: {app}\zlib1.dll; Type: files

[UninstallDelete]
Name: {app}; Type: dirifempty

[Tasks]
Name: CC1Files; Description: Associate .ccl Files with CCEdit; Components: CCEdit
Name: CC2Files; Description: Associate .c2g and .c2m Files with CC2Edit; Components: CC2Edit
Name: CCHackFiles; Description: Associate .ccp Files with CCHack; Components: CCHack

[Registry]
Root: HKCR; SubKey: .ccl; ValueType: string; ValueData: CCLFile; Flags: uninsdeletekey; Tasks: CC1Files; Components: CCEdit
Root: HKCR; SubKey: CCLFile; ValueType: string; ValueData: Chip's Challenge Levelset; Flags: uninsdeletekey; Tasks: CC1Files; Components: CCEdit
Root: HKCR; SubKey: CCLFile\Shell\Edit\Command; ValueType: string; ValueData: """{app}\CCEdit.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: CC1Files; Components: CCEdit
Root: HKCR; Subkey: CCLFile\DefaultIcon; ValueType: string; ValueData: {app}\CCEdit.exe,1; Flags: uninsdeletevalue; Tasks: CC1Files; Components: CCEdit
Root: HKCR; SubKey: .c2g; ValueType: string; ValueData: C2GFile; Flags: uninsdeletekey; Tasks: CC2Files; Components: CC2Edit
Root: HKCR; SubKey: C2GFile; ValueType: string; ValueData: Chip's Challenge 2 Game Script; Flags: uninsdeletekey; Tasks: CC2Files; Components: CC2Edit
Root: HKCR; SubKey: C2GFile\Shell\Edit\Command; ValueType: string; ValueData: """{app}\CC2Edit.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: CC2Files; Components: CC2Edit
Root: HKCR; Subkey: C2GFile\DefaultIcon; ValueType: string; ValueData: {app}\CC2Edit.exe,2; Flags: uninsdeletevalue; Tasks: CC2Files; Components: CC2Edit
Root: HKCR; SubKey: .c2m; ValueType: string; ValueData: C2MFile; Flags: uninsdeletekey; Tasks: CC2Files; Components: CC2Edit
Root: HKCR; SubKey: C2MFile; ValueType: string; ValueData: Chip's Challenge 2 Map; Flags: uninsdeletekey; Tasks: CC2Files; Components: CC2Edit
Root: HKCR; SubKey: C2MFile\Shell\Edit\Command; ValueType: string; ValueData: """{app}\CC2Edit.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: CC2Files; Components: CC2Edit
Root: HKCR; Subkey: C2MFile\DefaultIcon; ValueType: string; ValueData: {app}\CC2Edit.exe,1; Flags: uninsdeletevalue; Tasks: CC2Files; Components: CC2Edit
Root: HKCR; SubKey: .ccp; ValueType: string; ValueData: CCPFile; Flags: uninsdeletekey; Tasks: CCHackFiles; Components: CCHack
Root: HKCR; SubKey: CCPFile; ValueType: string; ValueData: CCHack Patch File; Flags: uninsdeletekey; Tasks: CCHackFiles; Components: CCHack
Root: HKCR; SubKey: CCPFile\Shell\Open\Command; ValueType: string; ValueData: """{app}\CCHack.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: CCHackFiles; Components: CCHack
Root: HKCR; Subkey: CCPFile\DefaultIcon; ValueType: string; ValueData: {app}\CCHack.exe,1; Flags: uninsdeletevalue; Tasks: CCHackFiles; Components: CCHack

[Icons]
Name: {group}\CCEdit; Filename: {app}\CCEdit.exe; WorkingDir: {app}; IconFilename: {app}\CCEdit.exe; IconIndex: 0; Components: CCEdit
Name: {group}\CC2Edit; Filename: {app}\CC2Edit.exe; WorkingDir: {app}; IconFilename: {app}\CC2Edit.exe; IconIndex: 0; Components: CC2Edit
Name: {group}\CCPlay; Filename: {app}\CCPlay.exe; WorkingDir: {app}; IconFilename: {app}\CCPlay.exe; IconIndex: 0; Components: CCPlay
Name: {group}\CCHack; Filename: {app}\CCHack.exe; WorkingDir: {app}; IconFilename: {app}\CCHack.exe; IconIndex: 0; Components: CCHack

[Code]
function NeedMSVCRTInstaller: Boolean;
var
  Version: String;
begin
  if RegQueryStringValue(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86', 'Version', Version)
        or RegQueryStringValue(HKEY_LOCAL_MACHINE, 'Computer\HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\X86', 'Version', Version) then
  begin
    Result := (CompareStr(Version, 'v14.28.29325.02') < 0);
  end
  else
  begin
    Result := True;
  end;
end;
