; Prospa-SpinsolveExpert installer (FX3/DSP version)
; This version also allows for a clean install and will uninstall an existing Spinsolve version if overwriting.

[Setup]
AppName=Prospa
AppVerName=Prospa V3.128 + KeaExpert V2.02.16 (6-May-2026)
AppPublisher=Magritek
AppPublisherURL=http://www.magritek.com
AppSupportURL=http://www.magritek.com
AppUpdatesURL=http://www.magritek.com
AppendDefaultDirName=no
DefaultDirName={userdocs}\..\Applications\KeaExpert
;DefaultDirName={pf}\KeaExpert
DefaultGroupName=KeaExpert
LicenseFile=..\License.txt
Compression=lzma
SolidCompression=yes
WizardImageBackColor=$B0B0B0
WizardImageFile=..\largeIcon.bmp
WizardImageStretch=no
WizardSmallImageFile=..\smallIcon.bmp
OutputDir="..\Installer Output\KeaExpert"
OutputBaseFilename="KeaExpert V2.02.16 (6-May-2026)"

UsePreviousAppDir=no
;InfoBeforeFile="KeaExpertInfoBeforeInstall.txt"
;InfoAfterFile="KeaExpertInfoAfterInstall.txt"
;PrivilegesRequired=admin
PrivilegesRequired=none

[InstallDelete]


[Tasks]
Name: "cleaninstall"; Description: "Perform a clean install by resetting the user preferences"; Flags: unchecked;
Name: "uninstall"; Description: "Uninstall existing KeaExpert if overwriting folder"; 
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; Flags: unchecked

[Dirs]
Name: "{userappdata}\Prospa\Preferences V3.4\License"
Name: "{userappdata}\Prospa\Preferences V3.4\KeaParameters"
Name: "{app}\Preferences\Menus"
Name: "{app}\License"

[Files]
; Executable
Source: "..\Prospa.exe"; DestDir: "{app}"; Flags: ignoreversion; BeforeInstall: RemoveFile('{userappdata}\Prospa\Preferences V3.4\KeaParameters\ExpertInterface.par');
Source: "..\Prospa.lib"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\ProspaDebug.lib"; DestDir: "{app}"; Flags: ignoreversion

; Preferences
Source: "..\Preferences\Windows\original.par";                    DestDir: "{app}\Preferences\Windows"; Flags: ignoreversion recursesubdirs
Source: "..\Preferences\Windows\lastProspaLayout.mac";            DestDir: "{app}\Preferences\Windows"; Flags: ignoreversion recursesubdirs
Source: "..\Preferences\Core Macros\*";                           DestDir: "{app}\Preferences\Core Macros"; Excludes: *.txt; Flags : ignoreversion recursesubdirs
Source: "..\Preferences\Startup\*";                               DestDir: "{app}\Preferences\Startup"; Excludes: "userMenus*.lst, directories*.mac"; Flags : ignoreversion recursesubdirs
Source: "..\Preferences\Startup\userMenus-Kea.lst";               DestDir: "{app}\Preferences\Startup"; DestName: "userMenus.lst"; Flags: ignoreversion recursesubdirs
Source: "..\Preferences\Startup\directories.mac";                 DestDir: "{app}\Preferences\Startup"; DestName: "directories.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Preferences\Startup\defaultKeaParameters.par";        DestDir: "{userappdata}\Prospa\Preferences V3.4\KeaParameters"; DestName: "currentKeaParameters.mac"; Flags: ignoreversion
Source: "..\movePref.bat";                                        DestDir: "{app}";                     Flags: ignoreversion;
                   
; Menu options
Source: "..\Preferences\Core Macros\thisIsANewInstall.txt";        DestDir: "{userappdata}\Prospa\Preferences V3.4\KeaParameters"; DestName: "thisIsANewInstall.txt"; Flags: ignoreversion

; General Macros
Source: "..\Macros\coreMacros\*";                               DestDir: "{app}\Macros\coreMacros";     Flags: ignoreversion recursesubdirs;  Excludes: "nnls*, LHInvert2D.mac, Thumbs.db"
Source: "..\Macros\1D_Macros\*";                                DestDir: "{app}\Macros\1D_Macros";      Flags: ignoreversion recursesubdirs ; Excludes: "filter_parameters*, Thumbs.db"
;Source: "..\Macros\1D_Macros\Filters\filter_parameters_Kea.lst"; DestDir: "{app}\Macros\1D_Macros\Filters"; DestName: "filter_parameters.lst"; Flags: ignoreversion recursesubdirs;
Source: "..\Macros\1D_Macros\Filters\*";                        DestDir: "{app}\Macros\1D_Macros\Filters";  Flags: ignoreversion recursesubdirs;
Source: "..\Macros\2D_Macros\*";                                DestDir: "{app}\Macros\2D_Macros";      Flags: ignoreversion recursesubdirs ; Excludes: "Thumbs.db"
Source: "..\Macros\3D_Macros\*";                                DestDir: "{app}\Macros\3D_Macros";      Flags: ignoreversion recursesubdirs ; Excludes: "Thumbs.db"
Source: "..\Macros\NMRI\*";                                     DestDir: "{app}\Macros\NMRI";           Flags: ignoreversion recursesubdirs
Source: "..\Macros\NNLS\*";                                     DestDir: "{app}\Macros\NNLS";           Flags: ignoreversion recursesubdirs
Source: "..\Macros\GUI_macros\*";                               DestDir: "{app}\Macros\GUI_macros";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\*";                           DestDir: "{app}\Macros\Windows_Layout"; Excludes: "simple.mac, allinone.mac, AddMacros.mac, original*.mac, RCA.mac, *Expert.mac"; Flags: ignoreversion recursesubdirs
;Source: "..\Macros\Windows_Layout\originalKeaExpert.mac"; DestDir: "{app}\Macros\Windows_Layout"; DestName: "original.mac";        Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original.mac";                DestDir: "{app}\Macros\Windows_Layout"; DestName: "original.mac";        Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-1D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-1D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-2D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-2D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-3D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-3D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\KeaExpert.mac";         DestDir: "{app}\Macros\Windows_Layout"; DestName: "KeaExpert.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\Demo_Macros\*";                              DestDir: "{app}\Macros\Demo_Macros";    Excludes: "solenoid_coil.mac, segments_test2.mac, vortices.mac"; Flags: ignoreversion recursesubdirs


; Kea Macros
Source: "..\Macros\Kea-Expert\*";                      DestDir: "{app}\Macros\Kea-Expert";                    Excludes: *old.mac, Thumbs.db, *.docx;Flags: ignoreversion recursesubdirs;
Source: "..\Macros\UCS-Core\*";                        DestDir: "{app}\Macros\UCS-Core";                      Flags: ignoreversion recursesubdirs;
Source: "..\Macros\UCS-PP\*";                          DestDir: "{app}\Macros\UCS-PP";                        Excludes: "CompileKeaPulseProgram.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\1Pulse\*";                  DestDir: "{app}\Macros\Kea-NMR\1Pulse";                Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\1PulseAmplitudeSweep\*";    DestDir: "{app}\Macros\Kea-NMR\1PulseAmplitudeSweep";  Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\1PulseDurationSweep\*";     DestDir: "{app}\Macros\Kea-NMR\1PulseDurationSweep";   Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\CPMG\*";                    DestDir: "{app}\Macros\Kea-NMR\CPMG";                  Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\CPMGAdd\*";                 DestDir: "{app}\Macros\Kea-NMR\CPMGAdd";               Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\CPMGAmplitudeSweep\*";      DestDir: "{app}\Macros\Kea-NMR\CPMGAmplitudeSweep";    Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\CPMGDurationSweep\*";       DestDir: "{app}\Macros\Kea-NMR\CPMGDurationSweep";     Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\CPMGFast\*";                DestDir: "{app}\Macros\Kea-NMR\CPMGFast";              Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\MonitorNoise\*";            DestDir: "{app}\Macros\Kea-NMR\MonitorNoise";          Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\SpinEcho\*";                DestDir: "{app}\Macros\Kea-NMR\SpinEcho";              Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\SSE\*";                     DestDir: "{app}\Macros\Kea-NMR\SSE";                   Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\T1Sat\*";                   DestDir: "{app}\Macros\Kea-NMR\T1Sat";                 Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\T1Sat-ie\*";                DestDir: "{app}\Macros\Kea-NMR\T1Sat-ie";              Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\T1-IR-Add\*";               DestDir: "{app}\Macros\Kea-NMR\T1-IR-Add";             Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-NMR\Wobble\*";                  DestDir: "{app}\Macros\Kea-NMR\Wobble";                Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
;Source: "..\Macros\Kea-MRI\*";                         DestDir: "{app}\Macros\Kea-MRI";          Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Kea-Tests\*";                       DestDir: "{app}\Macros\Kea-Tests";        Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\NMR-Mouse\*";                       DestDir: "{app}\Macros\NMR-Mouse";        Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
;Source: "..\Macros\Setup\*";                           DestDir: "{app}\Macros\Setup";            Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\PSExamples\*";                      DestDir: "{app}\Macros\PSExamples";       Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\UCS-Update\*";                      DestDir: "{app}\Macros\UCS-Update";       Excludes: *.py, *PyDefault.par, V 43f*, PP_JTAG\*, Spinsolve*, Lock Updater\*, V200*.xsvf;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\BatchCommands\*";                   DestDir: "{app}\Macros\BatchCommands";    Flags: ignoreversion
Source: "..\Macros\UserScripts\*";                     DestDir: "{app}\Macros\UserScripts";      Flags: ignoreversion recursesubdirs
;Source: "..\Macros\TestScripts\*";                     DestDir: "{app}\Macros\TestScripts";      Flags: ignoreversion
;Source: "..\Macros\Python\*";                          DestDir: "{app}\Macros\Python";                                        Flags: ignoreversion

; Macro libraries
Source: "..\Macros\1D_Macros\*";      DestDir: "{app}\Libraries\1D_Macros";      Excludes: "AddMacros.mac, Thumbs.db"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\2D_Macros\*";      DestDir: "{app}\Libraries\2D_Macros";      Excludes: "AddMacros.mac, Thumbs.db"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\3D_Macros\*";      DestDir: "{app}\Libraries\3D_Macros";      Excludes: "AddMacros.mac, Thumbs.db"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\*"; DestDir: "{app}\Libraries\Windows_Layout"; Excludes: "AddMacros.mac"; Flags: ignoreversion recursesubdirs

; Driver
Source: "..\DSP-USB Driver\WinUSB Files\*";           DestDir: "{app}\DSP-USB Driver"; Flags: ignoreversion recursesubdirs; Check: IsVistaOrLater
Source: "..\DSP-USB Driver\WinUSB Files\*";           DestDir: "{app}\DSP-USB Driver\Driver Files"; Flags: ignoreversion recursesubdirs; Check: IsXP
Source: "..\DSP-USB Driver\Installer (Not XP)\*";     DestDir: "{app}\DSP-USB Driver"; Flags: ignoreversion recursesubdirs; Check: IsVistaOrLater
Source: "..\DSP-USB Driver\Installer (XP 32 bit)\*";  DestDir: "{app}\DSP-USB Driver"; Flags: ignoreversion recursesubdirs; Check: IsXP_X86
Source: "..\DSP-USB Driver\Installer (XP 64 bit)\*";  DestDir: "{app}\DSP-USB Driver"; Flags: ignoreversion recursesubdirs; Check: IsXP_X64
Source: "..\FX3-USB Driver\*";                        DestDir: "{app}\FX3-USB Driver"; Flags: ignoreversion recursesubdirs;

; HTML Documentation
Source: "..\Documentation\Classes\*";                  DestDir: "{app}\Documentation\Classes";                    Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\DLLs\*";                     DestDir: "{app}\Documentation\DLLs";                       Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx"
Source: "..\Documentation\General Information\*";      DestDir: "{app}\Documentation\General Information";        Flags: ignoreversion recursesubdirs; Excludes: "*.lst, *.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\General Information\keahelplist.lst"; DestDir: "{app}\Documentation\General Information"; DestName: "helplist.lst"; Flags: ignoreversion recursesubdirs   ; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Commands\*";                 DestDir: "{app}\Documentation\Commands";                   Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\User Interface\*";           DestDir: "{app}\Documentation\User Interface";             Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Writing macros\*";           DestDir: "{app}\Documentation\Writing macros";             Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\1D\*";                DestDir: "{app}\Documentation\Macros\1D";                  Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\2D\*";                DestDir: "{app}\Documentation\Macros\2D";                  Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\3D\*";                DestDir: "{app}\Documentation\Macros\3D";                  Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Core\*";              DestDir: "{app}\Documentation\Macros\Core";                Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Demos\*";             DestDir: "{app}\Documentation\Macros\Demos";               Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\GUI\*";               DestDir: "{app}\Documentation\Macros\GUI";                 Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Generic\*";           DestDir: "{app}\Documentation\Macros\Generic";             Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\NMRI\*";              DestDir: "{app}\Documentation\Macros\NMRI";                Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\LayoutKea\*";         DestDir: "{app}\Documentation\Macros\Layout";              Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\NNLS\*";              DestDir: "{app}\Documentation\Macros\NNLS";                Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Kea-Expert\*";  DestDir: "{app}\Documentation\Macros\Kea-Expert";    Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pptx, *.pdf"
Source: "..\Documentation\Macros\Pulse Programming\*"; DestDir: "{app}\Documentation\Macros\Pulse Programming";   Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Post Processing\*";  DestDir: "{app}\Documentation\Macros\Post Processing";      Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\helplist_UCS.lst";           DestDir: "{app}\Documentation"; DestName: "helplist.lst";  Flags: ignoreversion

; PDF Documents
Source: "..\PDFs\Pulse Programming Guide (FX3).pdf";                                  DestDir: "{app}\PDF Documentation"; Flags: ignoreversion recursesubdirs;
Source: "..\PDFs\Pulse Programming Guide (DSP).pdf";                                  DestDir: "{app}\PDF Documentation"; Flags: ignoreversion recursesubdirs;
Source: "..\PDFs\Prospa programming manual.pdf";                                      DestDir: "{app}\PDF Documentation"; Flags: ignoreversion recursesubdirs;
Source: "..\PDFs\KeaExpert - User Manual V2.02.pdf";                                  DestDir: "{app}\PDF Documentation"; Flags: ignoreversion recursesubdirs;
Source: "..\PDFs\KeaExpert release notes.pdf";                                        DestDir: "{app}\PDF Documentation"; Flags: ignoreversion

; DLLs
Source: "..\DLLs\DSPWinUSBRun.dll";        DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\Kea2PPRun.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FX3PPRun.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\MSP430Run.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FX3USBRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FTPRun.dll";              DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\FTDIRun.dll";             DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\ftd2xx.dll";              DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\fittingRun.dll";          DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\NNLSRun.dll";             DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\SerialRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\SplineRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\SQLiteRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\BiotRun.dll";             DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
; DLLs for linear prediction
;Source: "..\DLLs\LinearPredictionRun.dll"; DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FastLinearPredictionRun.dll"; DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\libgcc_s_dw2-1.dll";      DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\libgfortran-3.dll";       DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\liblapack.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\liblapacke.dll";          DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\libblas.dll";             DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\msvcp110.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\msvcr110.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs

; Python code for running Expert
Source: "..\Python\ExpertController\*";          DestDir: "{app}\Python\ExpertController"; Flags: ignoreversion recursesubdirs; Excludes: ".venv, .idea"
Source: "..\Python\Python\*";                    DestDir: "{app}\Python\Python";           Flags: ignoreversion recursesubdirs; 
Source: "..\Python\RunExpertExperiment.py";      DestDir: "{app}\Python"; DestName: "RunExpertExperiment.py";    Flags: ignoreversion; 
Source: "..\Python\makePyEnv.bat";               DestDir: "{app}\Python"; DestName: "makePyEnv.bat";             Flags: ignoreversion; 
Source: "..\Python\startExpertController.bat";   DestDir: "{app}\Python"; DestName: "startExpertController.bat"; Flags: ignoreversion; 
Source: "..\Python\Prospa3.xxUpdateLinks.exe";   DestDir: "{app}\Python"; DestName: "Prospa3.xxUpdateLinks.exe"; Flags: ignoreversion; 

; Example data
;Source: "..\Example Data\Demos\*"; DestDir: "{app}\Example Data\Demos"; Flags: ignoreversion recursesubdirs

; Special programs
;Source: "..\Kea special programs\TTLController\*";       DestDir: "{app}\Kea special programs\TTLController"; Flags: ignoreversion recursesubdirs;

; Icons
Source: "..\Icons\prospa_file.ico"; DestDir: "{app}\Icons"; Flags: ignoreversion recursesubdirs

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Prospa"; Filename: "{app}\Prospa.exe"
Name: "{group}\{cm:UninstallProgram,Prospa}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Prospa"; Filename: "{app}\Prospa"; Tasks: desktopicon ; WorkingDir: "{app}\Macros"
Name: "{commondesktop}\KeaExpert"; Filename: "{app}\Prospa"; Parameters: """{app}\Macros\Kea-Expert\KeaExpertInterface.pex"""; IconFilename: "{app}\Macros\Kea-Expert\Other Macros\Bitmaps\keaExpert.ico" ; WorkingDir: "{app}\Macros"

[Registry]
Root: HKCR; Subkey: ".pt1"; ValueType: "string"; ValueName: ""; ValueData: "prospafile"; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: ".pt2"; ValueType: "string"; ValueName: ""; ValueData: "prospafile"; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: ".par"; ValueType: "string"; ValueName: ""; ValueData: "prospafile"; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: ".mac"; ValueType: "string"; ValueName: ""; ValueData: "prospafile"; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: ".pex"; ValueType: "string"; ValueName: ""; ValueData: "prospafile"; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: ".1d";  ValueType: "string"; ValueName: ""; ValueData: "prospafile"; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: ".2d";  ValueType: "string"; ValueName: ""; ValueData: "prospafile"; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: ".3d";  ValueType: "string"; ValueName: ""; ValueData: "prospafile"; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: "prospafile\shell\open\command";               ValueType: "string"; ValueName: ""; ValueData: """{app}\Prospa.exe"" ""%1"""; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: "prospafile\DefaultIcon";                      ValueType: "string"; ValueName: ""; ValueData: """{app}\Icons\prospa_file.ico"""; Flags: uninsdeletekeyifempty
Root: HKCR; Subkey: "Applications\Prospa.exe\shell\open\command";  ValueType: "string"; ValueName: ""; ValueData: """{app}\Prospa.exe"" ""%1"""; Flags: uninsdeletekeyifempty

[CustomMessages]
InstallDSPDriver=Install the DSP-USB driver
InstallFX3Driver=Install the FX3-USB driver
LaunchExpert=Launch KeaExpert
ViewUCSRelease=View KeaExpert release notes (requires PDF reader)

[Run]
Filename: "{app}\DSP-USB Driver\DriverInstall.exe"; Description: "{cm:InstallDSPDriver}"; Flags: postinstall  runascurrentuser;   Check: IsVistaOrLater  
Filename: "{app}\FX3-USB Driver\InfDefaultInstall.exe"; Parameters: """{app}\FX3-USB Driver\Win10\x64\cyusb3.inf"""; Description: "{cm:InstallFX3Driver}"; Flags: postinstall  runascurrentuser;   Check: IsX64 and IsWin10_11; 
Filename: "{app}\FX3-USB Driver\InfDefaultInstall.exe"; Parameters: """{app}\FX3-USB Driver\Win7\x64\cyusb3.inf"""; Description: "{cm:InstallFX3Driver}"; Flags: postinstall  runascurrentuser;   Check: IsX64 and IsWin7; 
Filename: "{app}\Prospa.exe"; Parameters: """{app}\Macros\Kea-Expert\KeaExpertInterface.pex"""; Description: "{cm:LaunchExpert}"; Flags: shellexec nowait postinstall skipifsilent unchecked 
Filename: "{app}\PDF Documentation\KeaExpert Release notes.pdf"; Description: "{cm:ViewUCSRelease}"; Flags: shellexec nowait postinstall skipifsilent 

[Code]
   
function IsX86: Boolean;
begin
  Result := (ProcessorArchitecture = paX86);
end;

function IsX64: Boolean;
begin
  Result := (ProcessorArchitecture = paX64) or (ProcessorArchitecture = paIA64);
end;

procedure RemoveFile(file: STRING);
begin
  DeleteFile(ExpandConstant(file));
end;

procedure DeleteDLLs(dir: STRING);
begin
  DelTree(ExpandConstant(dir), True, True, True);
end;

procedure DeleteExpertPref(dir: STRING);
begin
  DelTree(ExpandConstant(dir), True, True, True);
end;

function IsXP: Boolean;
begin
  Result := (GetWindowsVersion shr 24 < 6);
end;

function IsXP_X86: Boolean;
begin
  Result := (GetWindowsVersion shr 24 < 6) and (ProcessorArchitecture = paX86);
end;

function IsXP_X64: Boolean;
begin
  Result := (GetWindowsVersion shr 24 < 6) and ((ProcessorArchitecture = paX64) or (ProcessorArchitecture = paIA64));
end;

function IsVistaOrLater: Boolean;
begin
  Result := (GetWindowsVersion shr 24 >= 6);
end;

function IsWin7: Boolean;
begin
  Result := (GetWindowsVersion shr 24 = 7);
end;


function IsWin10_11: Boolean;
begin
  Result := (GetWindowsVersion shr 24 >= 10);
end;

// Clean the preferences area by copying to a backup folder

function CleanPreferences(): Boolean;
var
  srcDir: String;
  dstDir1: String;
  dstDir2: String;
begin

  srcDir := ExpandConstant('{userappdata}\Prospa\Preferences V3.4');
  dstDir1 := ExpandConstant('{userappdata}\Prospa\OldPref');
  dstDir2 := ExpandConstant('{userappdata}\Prospa\OldPref\Date(') + GetDateTimeString('ddd dd-mm-yyyy', '-', ':') + ')Time(' + GetDateTimeString('hh-mm-ss', '-', ':') + ')';
  CreateDir(dstDir1)
  RenameFile(srcDir,dstDir2);
  Result := True;

end;
  
// Unistall an old version of the software if installing over the top

function UnInstallOldVersion(): Boolean;
var
  sUnInstallString: String;
  iResultCode: Integer;
begin

  if Exec(ExpandConstant('{app}\unins000.exe'), '/SILENT /NORESTART /SUPPRESSMSGBOXES','', SW_SHOW, ewWaitUntilTerminated, iResultCode) then
    begin
       Result := True;
       Log('Uninstall suceeded');
    end
  else
     begin
     if Exec(ExpandConstant('{app}\unins001.exe'), '/SILENT /NORESTART /SUPPRESSMSGBOXES','', SW_SHOW, ewWaitUntilTerminated, iResultCode) then
        begin
           Result := True;
           Log('Uninstall suceeded');
        end
      else
        begin
          Result := False;
          Log('Uninstall failed with error: ' + IntToStr(iResultCode));
        end
      end;
end;

function BoolToStr(Value : Boolean) : String; 
begin
  if Value then
    result := 'true'
  else
    result := 'false';
end;

// Intercept step after selecting directory to uninstall old folder

function NextButtonClick(CurPageID: Integer): Boolean;

begin
    Log('Button pressed: ' + IntToStr(CurPageID));

    //Index := WizardForm.TasksList.Items.IndexOf('Task Description');
    if  CurPageID = wpReady then
    begin
   // Log('Item checked 1: ' + BoolToStr(WizardForm.TasksList.Checked[0]));
   // Log('Item checked 2: ' + BoolToStr(WizardForm.TasksList.Checked[1]));
        if WizardForm.TasksList.Checked[0] then
           CleanPreferences();
   // Log('Item checked 1: ' + BoolToStr(WizardForm.TasksList.Checked[0]));
   // Log('Item checked 2: ' + BoolToStr(WizardForm.TasksList.Checked[1]));
        if WizardForm.TasksList.Checked[1] then
           UnInstallOldVersion();
    end;

    Result := True;
end;
