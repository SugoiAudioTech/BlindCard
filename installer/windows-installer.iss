[Setup]
AppName=Blind Card
AppVersion=1.0.1
AppPublisher=Sugoi Audio Tech
AppPublisherURL=https://sugoiaudio.com
DefaultDirName={commonpf}\Sugoi Audio\Blind Card
DefaultGroupName=Sugoi Audio\Blind Card
OutputDir=..\build\installer
OutputBaseFilename=BlindCard-1.0.1-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
LicenseFile=..\LICENSE
WizardStyle=modern
DisableProgramGroupPage=yes

[Types]
Name: "full"; Description: "Full installation (VST3 + Standalone)"
Name: "vst3only"; Description: "VST3 Plugin only"
Name: "standalone"; Description: "Standalone application only"

[Components]
Name: "vst3"; Description: "VST3 Plugin"; Types: full vst3only
Name: "standalone"; Description: "Standalone Application"; Types: full standalone

[Files]
; VST3 Plugin - install to standard VST3 directory
Source: "..\build\BlindCard_artefacts\Release\VST3\Blind Card.vst3\*"; DestDir: "{commoncf}\VST3\Blind Card.vst3"; Components: vst3; Flags: recursesubdirs

; Standalone application
Source: "..\build\BlindCard_artefacts\Release\Standalone\Blind Card.exe"; DestDir: "{app}"; Components: standalone

[Icons]
Name: "{group}\Blind Card"; Filename: "{app}\Blind Card.exe"; Components: standalone
Name: "{commondesktop}\Blind Card"; Filename: "{app}\Blind Card.exe"; Components: standalone; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; Components: standalone

[Run]
Filename: "{app}\Blind Card.exe"; Description: "Launch Blind Card"; Flags: nowait postinstall skipifsilent; Components: standalone
