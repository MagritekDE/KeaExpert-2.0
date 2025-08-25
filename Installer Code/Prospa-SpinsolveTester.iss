; Prospa-SpinsolveTester installer (history and batch mode)
; Note this requires a special version of Prospa compiled so it doesn't need a license.

[Setup]
AppName=Prospa
AppVerName=Prospa V3.125 + SpinsolveTester V2.02.04 (30-Jan-2024)
AppPublisher=Magritek
AppPublisherURL=http://www.magritek.com
AppSupportURL=http://www.magritek.com
AppUpdatesURL=http://www.magritek.com
AppendDefaultDirName=no
DefaultDirName={userdocs}\..\Applications\SpinsolveTester
DefaultGroupName=SpinsolveTester
LicenseFile=..\License.txt
Compression=lzma
SolidCompression=yes
WizardImageBackColor=$B0B0B0
WizardImageFile=..\largeIcon.bmp
WizardImageStretch=no
WizardSmallImageFile=..\smallIcon.bmp
OutputDir="..\Installer Output\SpinsolveTester"
OutputBaseFilename="SpinsolveTester V2.02.04 (30-Jan-2024)"
UsePreviousAppDir=no
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
Source: "..\Prospa_no_license.exe"; DestDir: "{app}"; DestName: "prospa.exe"; Flags: ignoreversion; BeforeInstall: RemoveFile('{userappdata}\Prospa\Preferences V3.4\SpinsolveParameters\ExpertInterface.par');

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
Source: "..\Macros\coreMacros\*";                               DestDir: "{app}\Macros\coreMacros";     Flags: ignoreversion recursesubdirs;  Excludes: "nnls*, LHInvert2D.mac"
Source: "..\Macros\1D_Macros\*";                                DestDir: "{app}\Macros\1D_Macros";      Flags: ignoreversion recursesubdirs ; Excludes: "filter_parameters*"
Source: "..\Macros\1D_Macros\Filters\filter_parameters_spinsolve.lst"; DestDir: "{app}\Macros\1D_Macros\Filters"; DestName: "filter_parameters.lst"; Flags: ignoreversion recursesubdirs;
Source: "..\Macros\2D_Macros\*";                                DestDir: "{app}\Macros\2D_Macros";      Flags: ignoreversion recursesubdirs
Source: "..\Macros\3D_Macros\*";                                DestDir: "{app}\Macros\3D_Macros";      Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\*";                           DestDir: "{app}\Macros\Windows_Layout"; Excludes: "simple.mac, allinone.mac, AddMacros.mac, original*.mac, RCA.mac, *Expert.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\originalSpinsolveExpert.mac"; DestDir: "{app}\Macros\Windows_Layout"; DestName: "original.mac";        Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-1D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-1D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-2D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-2D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\original-3D.mac";             DestDir: "{app}\Macros\Windows_Layout"; DestName: "original-3D.mac";     Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\SpinsolveExpert.mac";         DestDir: "{app}\Macros\Windows_Layout"; DestName: "SpinsolveExpert.mac"; Flags: ignoreversion recursesubdirs

; Tester Macros
Source: "..\Macros\CarbonTest\*"; DestDir: "{app}\Macros\CarbonTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   
Source: "..\Macros\FluorineTest\*"; DestDir: "{app}\Macros\FluorineTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   
Source: "..\Macros\ProtonTest\*"; DestDir: "{app}\Macros\ProtonTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   
Source: "..\Macros\XChannelTest\*"; DestDir: "{app}\Macros\XChannelTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   
Source: "..\Macros\LockTest\*"; DestDir: "{app}\Macros\LockTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs  
Source: "..\Macros\HardwareTest\*"; DestDir: "{app}\Macros\HardwareTest"; Excludes: *.py, *PyDefault.par;  Flags: ignoreversion recursesubdirs   

; Spinsolve Macros
Source: "..\Macros\Spinsolve-Expert\*";                    DestDir: "{app}\Macros\Spinsolve-Expert"; Excludes: *old.mac;Flags: ignoreversion recursesubdirs;
Source: "..\Macros\Spinsolve-Expert\testerName.mac";       DestDir: "{app}\Macros\Spinsolve-Expert"; DestName: "expertName.mac"; Excludes: *old.mac, *.EXE, *.DOC;Flags: ignoreversion recursesubdirs;
Source: "..\Macros\UCS-Core\*";                            DestDir: "{app}\Macros\UCS-Core";         Flags: ignoreversion recursesubdirs;
Source: "..\Macros\UCS-PP\*";                              DestDir: "{app}\Macros\UCS-PP";           Excludes: "CompileSpinsolvePulseProgram.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\Setup\*";                               DestDir: "{app}\Macros\Setup";            Excludes: *.py, *PyDefault.par, *old.mac;           Flags: ignoreversion recursesubdirs
Source: "..\Macros\UCS-Update\*";                          DestDir: "{app}\Macros\UCS-Update";       Excludes: *.py, *PyDefault.par, V 43f*, PP_JTAG\*, Kea*, Lock Updater\*, SpinsolveFirmwareUpdater.mac, SpinsolveFirmwareUpdater\*;  Flags: ignoreversion recursesubdirs
Source: "..\Macros\BatchCommands\*";                       DestDir: "{app}\Macros\BatchCommands";    Flags: ignoreversion
Source: "..\Macros\UserScripts\*";                         DestDir: "{app}\Macros\UserScripts";      Flags: ignoreversion
Source: "..\Macros\UCS-Core\ucsDefaultPrefTester.par";     DestDir: "{app}\Macros\UCS-Core"; DestName: "ucsDefaultPreferences.par"; Flags: ignoreversion recursesubdirs;
Source: "..\Macros\Scripts\lockAndCalibrate.mac";          DestDir: "{app}\Macros\Scripts";  DestName: "lockAndCalibrate.mac";      Flags: ignoreversion
Source: "..\Macros\Scripts\adjustPresatPhase.mac";         DestDir: "{app}\Macros\Scripts";  DestName: "adjustPresatPhase.mac";     Flags: ignoreversion
Source: "..\Macros\Scripts\importStandardShim.mac";        DestDir: "{app}\Macros\Scripts";  DestName: "importStandardShim.mac";    Flags: ignoreversion
Source: "..\Macros\Scripts\exportExpertShim.mac";          DestDir: "{app}\Macros\Scripts";  DestName: "exportExpertShim.mac";      Flags: ignoreversion
Source: "..\Macros\Scripts\loadExpertShim.mac";            DestDir: "{app}\Macros\Scripts";  DestName: "loadExpertShim.mac";        Flags: ignoreversion
Source: "..\Macros\Scripts\sampleControl.mac";             DestDir: "{app}\Macros\Scripts";  DestName: "sampleControl.mac";         Flags: ignoreversion
Source: "..\Macros\Scripts\checkShim.mac";                 DestDir: "{app}\Macros\Scripts";  DestName: "checkShim.mac";             Flags: ignoreversion
Source: "..\Macros\Scripts\loadUserShim.mac";              DestDir: "{app}\Macros\Scripts";  DestName: "loadUserShim.mac";          Flags: ignoreversion
Source: "..\Macros\Scripts\RunPilot.mac";                  DestDir: "{app}\Macros\Scripts";  DestName: "RunPilot.mac";              Flags: ignoreversion

; Macro libraries
Source: "..\Macros\1D_Macros\*";      DestDir: "{app}\Libraries\1D_Macros";      Excludes: "AddMacros.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\2D_Macros\*";      DestDir: "{app}\Libraries\2D_Macros";      Excludes: "AddMacros.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\3D_Macros\*";      DestDir: "{app}\Libraries\3D_Macros";      Excludes: "AddMacros.mac"; Flags: ignoreversion recursesubdirs
Source: "..\Macros\Windows_Layout\*"; DestDir: "{app}\Libraries\Windows_Layout"; Excludes: "AddMacros.mac"; Flags: ignoreversion recursesubdirs

; Driver
Source: "..\DSP-USB Driver\WinUSB Files\*";           DestDir: "{app}\DSP-USB Driver"; Flags: ignoreversion recursesubdirs; Check: IsVistaOrLater
Source: "..\DSP-USB Driver\WinUSB Files\*";           DestDir: "{app}\DSP-USB Driver\Driver Files"; Flags: ignoreversion recursesubdirs; Check: IsXP
Source: "..\DSP-USB Driver\Installer (Not XP)\*";     DestDir: "{app}\DSP-USB Driver"; Flags: ignoreversion recursesubdirs; Check: IsVistaOrLater
Source: "..\DSP-USB Driver\Installer (XP 32 bit)\*";  DestDir: "{app}\DSP-USB Driver"; Flags: ignoreversion recursesubdirs; Check: IsXP_X86
Source: "..\DSP-USB Driver\Installer (XP 64 bit)\*";  DestDir: "{app}\DSP-USB Driver"; Flags: ignoreversion recursesubdirs; Check: IsXP_X64
Source: "..\FX3-USB Driver\*";                        DestDir: "{app}\FX3-USB Driver"; Flags: ignoreversion recursesubdirs;

; HTML Documentation
Source: "..\Documentation\Macros\Spinsolve-Expert\*";  DestDir: "{app}\Documentation\Macros\Spinsolve-Expert";    Flags: ignoreversion recursesubdirs; Excludes: "*.xml, *.thmx, *.doc, *.docx, *.pptx, *.pdf"

; DLLs
Source: "..\DLLs\DSPWinUSBRun.dll";        DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\UCSPPRun.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FX3PPRun.dll";            DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\MSP430Run.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\FX3USBRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\UCSLockPPRun.dll";        DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\fittingRun.dll";          DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\SerialRun.dll";           DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
Source: "..\DLLs\sseUtilitiesRun.dll";     DestDir: "{app}\DLLs"; Flags: ignoreversion recursesubdirs
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
Name: "{group}\SpinsolveTester"; Filename: "{app}\Prospa.exe" ;IconFilename: "{app}\Macros\Spinsolve-Expert\Other Macros\Bitmaps\test.ico"
Name: "{group}\{cm:UninstallProgram,Prospa}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\SpinsolveTester"; Filename: "{app}\Prospa.exe"; Parameters: """{app}\Macros\Spinsolve-Expert\SpinsolveExpertInterface.pex"""; IconFilename: "{app}\Macros\Spinsolve-Expert\Other Macros\Bitmaps\test.ico"

[CustomMessages]
LaunchExpert=Launch SpinsolveTester

[Run]
Filename: "{app}\Prospa.exe"; Parameters: """{app}\Macros\Spinsolve-Expert\SpinsolveExpertInterface.pex"""; Description: "{cm:LaunchExpert}"; Flags: shellexec nowait postinstall skipifsilent unchecked 

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
