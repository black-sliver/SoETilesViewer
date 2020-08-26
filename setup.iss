; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "SoETilesViewer"
#define MyAppVersion "0.1.5"
#define MyAppPublisher "black_sliver"
#define MyAppURL "evermizer.com"
#define MyAppExeName "SoETilesViewer.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{A5619364-A5B1-4BFF-9077-45B6379DD8EB}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=LICENSE
OutputDir=..\build-SoETilesViewer-Desktop_mingw-Release
OutputBaseFilename=SoETilesViewer-setup
SolidCompression=yes
InternalCompressLevel=ultra64
Compression=lzma/ultra64
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\build-SoETilesViewer-Desktop_mingw-Release\release\SoETilesViewer.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build-SoETilesViewer-Desktop_mingw-Release\release\libgcc_s_seh-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build-SoETilesViewer-Desktop_mingw-Release\release\libstdc++-6.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build-SoETilesViewer-Desktop_mingw-Release\release\libwinpthread-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build-SoETilesViewer-Desktop_mingw-Release\release\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build-SoETilesViewer-Desktop_mingw-Release\release\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build-SoETilesViewer-Desktop_mingw-Release\release\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\build-SoETilesViewer-Desktop_mingw-Release\release\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
