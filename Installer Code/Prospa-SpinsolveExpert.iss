; Prospa-SpinsolveExpert installer (FX3/DSP version)
; This version also allows for a clean install and will uninstall an existing Spinsolve version if overwriting.

[Setup]
AppName=Prospa
AppVerName=Prospa V3.128 + SpinsolveExpert V2.02.10 (25-March-2025)
AppPublisher=Magritek
AppPublisherURL=http://www.magritek.com
AppSupportURL=http://www.magritek.com
AppUpdatesURL=http://www.magritek.com
AppendDefaultDirName=no
DefaultDirName={userdocs}\..\Applications\SpinsolveExpert
;DefaultDirName={pf}\SpinsolveExpert
DefaultGroupName=SpinsolveExpert
LicenseFile=..\License.txt
Compression=lzma
SolidCompression=yes
WizardImageBackColor=$B0B0B0
WizardImageFile=..\largeIcon.bmp
WizardImageStretch=no
WizardSmallImageFile=..\smallIcon.bmp
OutputDir="..\Installer Output\SpinsolveExpert"
OutputBaseFilename="SpinsolveExpert V2.02.10 (25-March-2025)"

UsePreviousAppDir=no
;InfoBeforeFile="SpinsolveExpertInfoBeforeInstall.txt"
InfoAfterFile="SpinsolveExpertInfoAfterInstall.txt"
;PrivilegesRequired=admin
PrivilegesRequired=none

[InstallDelete]


[Tasks]
Name: "cleaninstall"; Description: "Perform a clean install by resetting the user preferences"; Flags: unchecked;
Name: "uninstall"; Description: "Uninstall existing SpinsolveExpert if overwriting folder"; 
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; Flags: unchecked

[Dirs]
Name: "{userappdata}\Prospa\Preferences V3.4\License"
Name: "{app}\Preferences\Menus"
Name: "{app}\License"

[Files]
; Executable
Source: "..\Prospa.exe"; DestDir: "{app}"; Flags: ignoreversion; BeforeInstall: RemoveFile('{userappdata}\Prospa\Preferences V3.4\SpinsolveParameters\ExpertInterface.par');
Source: "..\Prospa.lib"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\ProspaDebug.lib"; DestDir: "{app}"; Flags: ignoreversion

; Preferences
Source: "..\Preferences\Windows\original.par";                    DestDir: "{app}\Preferences\Windows"; Flags: ignoreversion recursesubdirs
Source: "..\Preferences\Windows\lastProspaLayout.mac";            DestDir: "{app}\Preferences\Windows"; Flags: ignoreversion recursesubdirs
Source: "..\Preferences\Core Macros\*";                           DestDir: "{app}\Preferences\Core Macros"; Excludes: *.txt; Flags : ignoreversion recursesubdirs
Source: "..\Preferences\Startup\*";                               DestDir: "{app}\Preferences\Startup"; Excludes: "userMenus*.lst, directories*.mac"; Flags : ignoreversion recursesubdirs
Source: "..\Preferences\Startup\userMenus-SpinsolveExpert.lst";   DestDir: "{app}\Preferences\Startup"; DestName: "userMenus.lst"; Flags: ignoreversion recursesubdirs
Source: "..\Preferences\Startup\directories-SpinsolveExpert.mac"; DestDir: "{app}\Preferences\Startup"; DestName: "directories.mac"; Flags: ignoreversion recursesubdirs
Source: "..\movePref.bat";                                        DestDir: "{app}";                     Flags: ignoreversion;
                   
; Menu options
Source: "..\Preferences\Core Macros\thisIsANewInstall.txt"; DestDir: "{userappdata}\Prospa\Preferences V3.4\SpinsolveParameters"; DestName: "thisIsANewInstall.txt"; Flags: ignoreversion recursesubdirs

; General Macros
Source: "..\Macros\coreMacros\*";                               DestDir: "{app}\Macros\coreMacros";     Flags: ignoreversion recursesubdirs;  Excludes: "nnls*, LHInvert2D.mac, Thumbs.db"
Source: "..\Macros\1D_Macros\*";                                DestDir: "{app}\Macros\1D_Macros";      Flags: ignoreversion recursesubdirs ; Excludes: "filter_parameters*, Thumbs.db"
;Source: "..\Macros\1D_Macros\Filters\filter_parameters_spinsolve.lst"; DestDir: "{app}\Macros\1D_Macros\Filters"; DestName: "filter_parameters.lst"; Flags: ignoreversion recursesubdirs;
Source: "..\Macros\1D_Macros\Filters\*";                        DestDir: "{app}\Macros\1D_Macros\Filters";  Flags: ignoreversion recursesubdirs;
Source: "..\Macros\2D_Macros\*";                                DestDir: "{app}\Macros\2D_Macros";      Flags: ignoreversion recursesubdirs ; Excludes: "Thumbs.db"
Source: "..\Macros\3D_Macros\*";                                DestDir: "{app}\Macros\3D_Macros";      Flags: ignoreversion recursesubdirs ; Excludes: "Thumbs.db"
Source: "..\Macros\NMRI\*";                                     DestDir: "{app}\Macros\NMRI";           Flags: ignoreversion recursesubdirs
Source: "..\Macros\NNLS\*";                                     DestDir: "{app}\Macros\NNLS";           Flags: ignoreversion recursesubdirs
Source: "..\Macros\GUI_macros\*";                               DestDir: "{app}\Macros\GUI_macros";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\*";                           DestDir: "{app}\Macros\Windows_Layout"; Excludes: "simple.mac, allinone.mac, AddMacros.mac, original*.mac, RCA.mac, *Expert.mac"; Flags: ignoreversion recursesubdirs
;Source: "..\Macros\Windows_Layout\originalSpinsolveExpert.mac"; DestDir: "{app}\Macros\Windows_Layout"; DestName: "original.mac";        Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original.mac";                DestDir: "{app}\Macros\Windows_Layout"; DestName: "original.mac";        Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-1D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-1D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-2D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-2D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-3D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-3D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\SpinsolveExpert.mac";         DestDir: "{app}\Macros\Windows_Layout"; DestName: "SpinsolveExpert.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\Demo_Macros\*";                              DestDir: "{app}\Macros\Demo_Macros";    Excludes: "solenoid_coil.mac, segments_test2.mac, vortices.mac"; Flags: ignoreversion recursesubdirs

; Tester Macros
;Source: "..\Macros\CarbonTest\*"; DestDir: "{app}\Macros\CarbonTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   
;Source: "..\Macros\FluorineTest\*"; DestDir: "{app}\Macros\FluorineTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   
;Source: "..\Macros\ProtonTest\*"; DestDir: "{app}\Macros\ProtonTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   
;Source: "..\Macros\XChannelTest\*"; DestDir: "{app}\Macros\XChannelTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   
;Source: "..\Macros\LockTest\*"; DestDir: "{app}\Macros\LockTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   

; DSP specific test macros
;Source: "..\Macros\DSPTests\*"; DestDir: "{app}\Macros\DSPTests"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   

; FX3 specific test macros
;Source: "..\Macros\FX3Tests\*"; DestDir: "{app}\Macros\FX3Tests"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   

; Spinsolve Macros
Source: "..\Macros\Spinsolve-Expert\*";                DestDir: "{app}\Macros\Spinsolve-Expert"; Excludes: *old.mac, Thumbs.db, *.docx;Flags: ignoreversion recursesubdirs;
Source: "..\Macros\UCS-Core\*";                        DestDir: "{app}\Macros\UCS-Core";         Flags: ignoreversion recursesubdirs;
Source: "..\Macros\UCS-PP\*";                          DestDir: "{app}\Macros\UCS-PP";           Excludes: "CompileSpinsolvePulseProgram.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\Suppression\*";                     DestDir: "{app}\Macros\Suppression";      Flags: ignoreversion recursesubdirs
Source: "..\Macros\SelectivePulses\*";                 DestDir: "{app}\Macros\SelectivePulses";      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Diffusion\*";                       DestDir: "{app}\Macros\Diffusion";        Flags: ignoreversion recursesubdirs; Excludes: PGSTEWetSup*
Source: "..\Macros\Proton\*";                          DestDir: "{app}\Macros\Proton";           Excludes: *.py, *PyDefault.par, CosyWetCDec*, WETT2Filter*, WETSuppression*;  Flags: ignoreversion recursesubdirs   
Source: "..\Macros\ProtonExamples\*";                  DestDir: "{app}\Macros\ProtonExamples";   Excludes: *.py, *PyDefault.par, CosyWetCDec*, WETT2Filter*, WETSuppression*;  Flags: ignoreversion recursesubdirs   
Source: "..\Macros\Boron\*";                           DestDir: "{app}\Macros\Boron";            Excludes: *.py, *PyDefault.par, *super;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Carbon\*";                          DestDir: "{app}\Macros\Carbon";           Excludes: *.py, *PyDefault.par, *super;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\CarbonMLEV\*";                      DestDir: "{app}\Macros\CarbonMLEV";       Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Fluorine\*";                        DestDir: "{app}\Macros\Fluorine";         Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Phosphorus\*";                      DestDir: "{app}\Macros\Phosphorus";       Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Lithium\*";                         DestDir: "{app}\Macros\Lithium";          Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Silicon\*";                         DestDir: "{app}\Macros\Silicon";          Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Sodium\*";                          DestDir: "{app}\Macros\Sodium";           Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Nitrogen\*";                        DestDir: "{app}\Macros\Nitrogen";         Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Imaging\*";                         DestDir: "{app}\Macros\Imaging";          Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\X\*";                               DestDir: "{app}\Macros\X";                Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\NUS\*";                             DestDir: "{app}\Macros\NUS";              Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\NOAH\*";                            DestDir: "{app}\Macros\NOAH";             Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\Setup\*";                           DestDir: "{app}\Macros\Setup";            Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\PSExamples\*";                      DestDir: "{app}\Macros\PSExamples";       Excludes: *.py, *PyDefault.par, *old.mac, *.docx;      Flags: ignoreversion recursesubdirs
Source: "..\Macros\UCS-Update\*";                      DestDir: "{app}\Macros\UCS-Update";       Excludes: *.py, *PyDefault.par, V 43f*, PP_JTAG\*, Kea*, Lock Updater\*, SpinsolveFirmwareUpdater.mac, SpinsolveFirmwareUpdater\*;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\BatchCommands\*";                   DestDir: "{app}\Macros\BatchCommands";    Flags: ignoreversion
Source: "..\Macros\UserScripts\*";                     DestDir: "{app}\Macros\UserScripts";      Flags: ignoreversion recursesubdirs
Source: "..\Macros\TestScripts\*";                     DestDir: "{app}\Macros\TestScripts";      Flags: ignoreversion
Source: "..\Macros\Scripts\lockAndCalibrate.mac";      DestDir: "{app}\Macros\Scripts"; DestName: "lockAndCalibrate.mac";     Flags: ignoreversion
Source: "..\Macros\Scripts\adjustPresatPhase.mac";     DestDir: "{app}\Macros\Scripts"; DestName: "adjustPresatPhase.mac";    Flags: ignoreversion
Source: "..\Macros\Scripts\importStandardShim.mac";    DestDir: "{app}\Macros\Scripts"; DestName: "importStandardShim.mac";   Flags: ignoreversion
Source: "..\Macros\Scripts\exportExpertShim.mac";      DestDir: "{app}\Macros\Scripts"; DestName: "exportExpertShim.mac";     Flags: ignoreversion
Source: "..\Macros\Scripts\loadExpertShim.mac";        DestDir: "{app}\Macros\Scripts"; DestName: "loadExpertShim.mac";       Flags: ignoreversion
Source: "..\Macros\Scripts\sampleControl.mac";         DestDir: "{app}\Macros\Scripts"; DestName: "sampleControl.mac";        Flags: ignoreversion
Source: "..\Macros\Scripts\checkShim.mac";             DestDir: "{app}\Macros\Scripts"; DestName: "checkShim.mac";            Flags: ignoreversion
Source: "..\Macros\Scripts\loadUserShim.mac";          DestDir: "{app}\Macros\Scripts"; DestName: "loadUserShim.mac";         Flags: ignoreversion
Source: "..\Macros\Scripts\RunPilot.mac";              DestDir: "{app}\Macros\Scripts"; DestName: "RunPilot.mac";             Flags: ignoreversion
Source: "..\Macros\Python\*";                          DestDir: "{app}\Macros\Python";                                        Flags: ignoreversion

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
Source: "..\Documentation\Macros\Spinsolve-Expert\*";  DestDir: "{app}\Documentation\Macros\Spinsolve-Expert";    Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pptx, *.pdf"
Source: "..\Documentation\Macros\Pulse Programming\*"; DestDir: "{app}\Documentation\Macros\Pulse Programming";   Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Post Processing\*";  DestDir: "{app}\Documentation\Macros\Post Processing";      Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Proton\*";            DestDir: "{app}\Documentation\Macros\Proton";              Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Carbon\*";            DestDir: "{app}\Documentation\Macros\Carbon";              Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Fluorine\*";          DestDir: "{app}\Documentation\Macros\Fluorine";            Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\Phosphorus\*";        DestDir: "{app}\Documentation\Macros\Phosphorus";          Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\Macros\X\*";                 DestDir: "{app}\Documentation\Macros\X";                   Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pdf"
Source: "..\Documentation\helplist_UCS.lst";           DestDir: "{app}\Documentation"; DestName: "helplist.lst";  Flags: ignoreversion

; PDF Documents
Source: "..\PDFs\Pulse Programming Guide (FX3).pdf";                                  DestDir: "{app}\PDF Documentation"; Flags: ignoreversion recursesubdirs;
Source: "..\PDFs\Pulse Programming Guide (DSP).pdf";                                  DestDir: "{app}\PDF Documentation"; Flags: ignoreversion recursesubdirs;
Source: "..\PDFs\Prospa programming manual.pdf";                                      DestDir: "{app}\PDF Documentation"; Flags: ignoreversion recursesubdirs;
Source: "..\PDFs\SpinsolveExpert - User Manual V2.02.04.pdf";                         DestDir: "{app}\PDF Documentation"; Flags: ignoreversion recursesubdirs;
Source: "..\PDFs\SpinsolveExpert release notes.pdf";                                  DestDir: "{app}\PDF Documentation"; Flags: ignoreversion

; DLLs
Source: "..\DLLs\DSPWinUSBRun.dll";        DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\UCSPPRun.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FX3PPRun.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\MSP430Run.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FX3USBRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\UCSLockPPRun.dll";        DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FTPRun.dll";              DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FTDIRun.dll";             DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\ftd2xx.dll";              DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\fittingRun.dll";          DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\NNLSRun.dll";             DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\SerialRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\SplineRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
;Source: "..\DLLs\SQLiteRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\BiotRun.dll";             DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\sseUtilitiesRun.dll";     DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
; DLLs for linear prediction
Source: "..\DLLs\LinearPredictionRun.dll"; DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FastLinearPredictionRun.dll"; DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\libgcc_s_dw2-1.dll";      DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\libgfortran-3.dll";       DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\liblapack.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\liblapacke.dll";          DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\libblas.dll";             DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\msvcp110.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\msvcr110.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs

; Example data
;Source: "..\Example Data\Demos\*"; DestDir: "{app}\Example Data\Demos"; Flags: ignoreversion recursesubdirs

; Special programs
Source: "..\Spinsolve special programs\GetShimParameters\*"; DestDir: "{userappdata}\Prospa\Preferences V3.4\SpinsolveParameters\GetShimParameters"; Flags: ignoreversion recursesubdirs;
Source: "..\Spinsolve special programs\GetShimParameters\*"; DestDir: "{app}\Spinsolve special programs\GetShimParameters"; Flags: ignoreversion recursesubdirs;
Source: "..\Spinsolve special programs\Autosampler\*";         DestDir: "{app}\Spinsolve special programs\Autosampler"; Flags: ignoreversion recursesubdirs;
Source: "..\Spinsolve special programs\SetShimParameters\*";   DestDir: "{userappdata}\Prospa\Preferences V3.4\SpinsolveParameters\SetShimParameters"; Flags: ignoreversion recursesubdirs;
Source: "..\Spinsolve special programs\SetShimParameters\*";   DestDir: "{app}\Spinsolve special programs\SetShimParameters"; Flags: ignoreversion recursesubdirs;
Source: "..\Spinsolve special programs\TTLController\*";       DestDir: "{app}\Spinsolve special programs\TTLController"; Flags: ignoreversion recursesubdirs;

; Icons
Source: "..\Icons\prospa_file.ico"; DestDir: "{app}\Icons"; Flags: ignoreversion recursesubdirs

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Prospa"; Filename: "{app}\Prospa.exe"
Name: "{group}\{cm:UninstallProgram,Prospa}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Prospa"; Filename: "{app}\Prospa"; Tasks: desktopicon ; WorkingDir: "{app}\Macros"
Name: "{commondesktop}\SpinsolveExpert"; Filename: "{app}\Prospa"; Parameters: """{app}\Macros\Spinsolve-Expert\SpinsolveExpertInterface.pex"""; IconFilename: "{app}\Macros\Spinsolve-Expert\Other Macros\Bitmaps\expert.ico" ; WorkingDir: "{app}\Macros"

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
LaunchExpert=Launch SpinsolveExpert
ViewUCSRelease=View SpinsolveExpert release notes (requires PDF reader)

[Run]
Filename: "{app}\DSP-USB Driver\DriverInstall.exe"; Description: "{cm:InstallDSPDriver}"; Flags: postinstall  runascurrentuser;   Check: IsVistaOrLater  
Filename: "{app}\FX3-USB Driver\InfDefaultInstall.exe"; Parameters: """{app}\FX3-USB Driver\Win10\x64\cyusb3.inf"""; Description: "{cm:InstallFX3Driver}"; Flags: postinstall  runascurrentuser;   Check: IsX64 and IsWin10_11; 
Filename: "{app}\FX3-USB Driver\InfDefaultInstall.exe"; Parameters: """{app}\FX3-USB Driver\Win7\x64\cyusb3.inf"""; Description: "{cm:InstallFX3Driver}"; Flags: postinstall  runascurrentuser;   Check: IsX64 and IsWin7; 
Filename: "{app}\Prospa.exe"; Parameters: """{app}\Macros\Spinsolve-Expert\SpinsolveExpertInterface.pex"""; Description: "{cm:LaunchExpert}"; Flags: shellexec nowait postinstall skipifsilent unchecked 
Filename: "{app}\PDF Documentation\SpinsolveExpert Release notes.pdf"; Description: "{cm:ViewUCSRelease}"; Flags: shellexec nowait postinstall skipifsilent 

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
