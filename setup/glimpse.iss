; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "glimpse"
#define MyAppVersion "0.1"
#define MyAppPublisher "Hochschule Augsburg"
#define MyAppURL "http://www.measure-it.de"
#define MyAppExeName "mobile.exe"

#define QtDir "C:\Qt\Qt5.1.1\5.1.1\msvc2012_64"

[Setup]
; Restrict to 64 bit
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{CF7758A3-6828-4C4F-8C34-48FE8E75251B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputBaseFilename={#MyAppName}_{#MyAppVersion}_x64_setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
; NOTE: Don't use "Flags: ignoreversion" on any shared system files
; Glimpse Files
Source: "C:\Users\gri\Desktop\glimpse\mobile.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\gri\Desktop\glimpse\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "C:\Users\gri\Desktop\glimpse\qml\*"; DestDir: "{app}\qml"; Flags: ignoreversion createallsubdirs recursesubdirs
Source: "urls\*"; DestDir: "{app}\urls"; Flags: ignoreversion createallsubdirs recursesubdirs

; Qt Files
Source: "{#QtDir}\bin\d3dcompiler_46.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\icudt51.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\icuin51.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\icuuc51.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\libGLESv2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Qml.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Quick.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5V8.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#QtDir}\bin\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{group}\{cm:WebsiteUrls}\Glimpse"; Filename: "{app}\urls\measure-it.url"
Name: "{group}\{cm:WebsiteUrls}\Hochschule Augsburg"; Filename: "{app}\urls\hsa.url"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[CustomMessages]
german.WebsiteUrls=Webseiten
english.WebsiteUrls=Websites

[Messages]
BeveledLabel=Hochschule Augsburg
