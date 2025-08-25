/********************************************************************************************
 *
 * Prospa program - entry point and main window definition and call-back procedure
 *
 * WinMain .................... entry point and initialization of application.
 * ProspaWinEventsProc ........ callback event procedure for the main Prospa window
 *
 * OpenCommandLineFile ........ opens any command lint file and displays it
 * RegisterWindowClasses ...... registers all windows being used by application.
 * RunStartUpMacros ........... run all macros in the preferences folder.
 * MakePreferencesFolder ...... make a user preferences folder is it doesn't exist.
 * CopyMacrosToPreferences .... copies macro between supplied and user folder.
 * SaveWindowPositions ........ save all windows positions to the preferences folder.
 * SaveDirectories ............ save all directory preferences.
 * CloseApplication ........... close the application down.
 * UpdateMainMenu ............. add menu folders to main menu bar.
 * AddMenuMacros .............. adds the names of all macros in current folder to a menu.
 * RunMacroDialog ............. allow the user to select a macro to run from a file dialog.
 * LoadMenuAndSearchPaths ..... load the menu and search paths from the preferences folder.
 * InitializeVariables ........ initialize some global prospa variables.
 * MakeUserFonts .............. define fonts used by application.
 *
 *
 **********************************************************************************************/

#pragma warning (disable: 4311) // Ignore pointer truncation warnings
#pragma warning (disable: 4996) // Ignore deprecated library functions

#pragma pack(push,8) // Must not use 1 byte packing for ACCEL structure
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h>
#include <math.h>
#include <time.h>
#include "stdafx.h"
#include "htmlhelp.h"
#include <process.h>
#include "allocate.h"
#include "BabyGrid.h"
#include "htmlviewer.h"
#include "cArg.h"
#include "classFunctions.h"
#include "cli_files.h"
#include "CommandRegistry.h"
#include "command.h"
#include "command_other.h"
#include "debug.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "dll.h"
#include "edit_class.h"
#include "edit_files.h"
#include "edit_utilities.h"
#include "error.h"
#include "evaluate.h"
#include "events_edit.h"
#include "files.h"
#include "font.h"
#include "globals.h"
#include "guiMakeObjects.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "import_export_utilities.h"
#include "interface.h"
#include "list_functions.h"
#include "license.h"
#include "load_save_data.h"
#include "main.h"
#include "macro_class.h"
#include "message.h"
#include "mymath.h"
#include "plot.h"
#include "plot_dialog_1d.h"
#include "plot1dCLI.h"
#include "plot2dCLI.h"
#include "plot3dEvents.h"
#include "PlotFile.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "process.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "start_exit.h"
#include "string_utilities.h"
#include "syntax_viewer.h"
#include "thread.h"
#include "TracePar.h"
#include "variablesOther.h"
#include "winEnableClass.h"
#include <Shlobj.h>
#include <io.h>
#include <fcntl.h>
#include "../Resources/preferences.h"
#include "memoryLeak.h"

using namespace Gdiplus;

// To compile with local preferences
//#define localPreferences 1
//#define PREFERENCE_NAME "Preferences"
// To run with user preferences
//#define localPreferences 0
#define PREFERENCE_NAME "Preferences V3.4"

//  Function prototypes
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM );
LRESULT CALLBACK ProspaWinEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HelpWinEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PlotEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DividerEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

bool  RegisterWindowClasses(HINSTANCE hInst);
short MakePreferencesFolder(void);
short RunMacroDialog(HWND, char*, char*);
short RunStartUpMacros(void);
short SaveDirectories(bool);
short SaveWindowPositions(bool);
short  CopyStartUpMacrosToPreferences(void);
short CopyCoreMacrosToPreferences(void);
short  CopyWindowsMacrosToPreferences(void);
short CopyMenuListsToPreferences(void);
void  InitializeVariables(void);
short  LoadMenuAndSearchPaths(void);
void  MakeUserFonts(void);
void  OpenCommandLineFile(char *fileName, char *path);
void  ProcessCommandLine(void);
void  LoadSystemDLLs(void);
short SaveLayoutDialog(HWND hWnd, char *pathName, char *fileName);
void  SaveUserMenus(char **list, long size);
void  GetMinimisedSize(HWND win, long &x, long &y, long &w, long &h);
void SaveGUIWindowsInfo(void);
short RunLastLayout(void);
void UpdateMainMenu(HWND hWnd);
int SetFonts(Interface* itfc, char args[]);
int GetMainWindow(Interface* itfc ,char args[]);

long mainThreadID;

// Global variables 
HINSTANCE prospaInstance;
HWND currentAppWindow;
HACCEL currentAccel;
HACCEL hAccelProspa;
HACCEL hAccelPlot1D;
HACCEL hAccelPlot2D;
HACCEL hAccelPlot3D;
HACCEL hAccelCLI;        
HACCEL hAccelEdit;
HACCEL hAccelUser;

char applicationHomeDir[MAX_PATH];  // Path to prospa executable     
char gCurrentDir[MAX_PATH];
char userPrefDir[MAX_PATH];         // Path to user preferences directory
char prospaTempDir[MAX_PATH];       // Path to tempary folder
HFONT cliFont;
HFONT editFont;
HFONT controlFont;
HFONT controlFontBold;
HFONT numberFont;


short editFontSize = 10;

Variable *userMenusVar;
Variable *macroPathVar;
Variable *dllPathVar;
Variable *prospaPrefVar;
Variable *userWorkingVar;
Variable *userMacroVar;
Variable *prospaTempVar;

FloatRect2 plot1DMinRect =  {-999,-999,-999,-999};
FloatRect2 plot2DMinRect =  {-999,-999,-999,-999};
FloatRect2 plot3DMinRect =  {-999,-999,-999,-999};
FloatRect2 cliMinRect =  {-999,-999,-999,-999};
FloatRect2 editMinRect =  {-999,-999,-999,-999};

short resizableWinBorderSize;
short fixedSizeWinBorderSize;
short titleBarNMenuHeight;
short titleBarHeight;

// Application name
char AppName[] = APPLICATION_NAME;

bool isModelessDialog;
DWORD cookie;
float NaN,Inf;
void LoadBitMaps();
void DisplayConsole(void);
bool IsRunningInWine(void);

bool exitProgram = false;

void 	ShutDown(void);

int __cdecl _purecall(void);

HBITMAP iconBMPs[50]; 
HANDLE ghMutex; 

bool gIsConsole = false;

// Macro to run if no valid macro can be found
char defaultMacro[] = 
"procedure(default);\
\
   n = window(\"PROSPA STARTUP ERROR\",-1,-1,600,400,\"resizable\"); \
   \
      cliWin(n,1);\
      editorwin(n,60);\
      divider(70,0,\"wh/3\",\"rw\",3,\"horiz\");\
      setpar(n,1,\"region\",[-1,-2,-3,70]);\
      setpar(n,60,\"region\",[-1,-2,70,-4]);\
      setpar(n,70,\"region\",[-5,-5,-5,-5]);\
      setwindowpar(n,\"focus\",1);\
      setwindowpar(n,\"keepInFront\",\"false\");\
      setwindowpar(n,\"show_menu\",\"false\"); \
      setwindowpar(n,\"exit_procedure\",\"exit()\"); \
\
   showwindow(n,\"recalc\");\
   setfocus(n);\
   pr (\"\n\");\
   funcedit(\"show fault\");\
   setwindowpar(n,\"title\",\"PROSPA STARTUP ERROR\");\
\
\
endproc();";

void DisplayConsole()
{
   AllocConsole();

   freopen("CON", "r", stdin);
   freopen("CON", "w", stdout);
   freopen("CON", "w", stderr);

   //HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
   //int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
   //FILE* hf_out = _fdopen(hCrt, "w");
   //setvbuf(hf_out, NULL, _IONBF, 1);
   //*stdout = *hf_out;

   //HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
   //hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
   //FILE* hf_in = _fdopen(hCrt, "r");
   //setvbuf(hf_in, NULL, _IONBF, 128);
   //*stdin = *hf_in;
}

int __cdecl _purecall(void)
{
	printf("Pure call detected!!!\n");
	return(0);
}

extern bool gUsingWine;

/***********************************************************************
*                             Win32 entry-point routine                
***********************************************************************/

int WINAPI WinMain(HINSTANCE hInst,  HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
{
   MSG lpMsg;

   char currentPath[MAX_PATH];
   char backupPath[MAX_PATH];
   char fileName[MAX_PATH];
   extern double GetMsTime();
	
   InitializeCriticalSection(&cs1DPlot);
   InitializeCriticalSection(&cs2DPlot);
   InitializeCriticalSection(&csVariable);
   InitializeCriticalSection(&csAssignVariable);
   InitializeCriticalSection(&csCache);
   InitializeCriticalSection(&csThread);

#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_crtBreakAlloc = 1483906;
	//int *temp = new int[100];
#endif

   gUsingWine = IsRunningInWine();

	SetupTextLists();

   gIsConsole = ((GetAsyncKeyState(VK_SHIFT) & 0x08000) && (GetAsyncKeyState(VK_CONTROL) & 0x08000));
   if (gIsConsole)
   {
      DisplayConsole();
      printf("Started!\n");
   }

// Make a mutext
    ghMutex = CreateMutex( NULL, FALSE, NULL); 

// Initialise random number generator.
   unsigned int seed = (int)GetMsTime();
   srand(seed);

// Make the base window;
	rootWin = new WinData();

//   VLDDisable(); // Memory leak testing off
   CArg arg(' ');

   GdiplusStartupInput gdiplusStartupInput;
   ULONG_PTR           gdiplusToken;

// Extract application path from command line
   char *cmd = GetCommandLine();

// Replace escaped quotes with normal quotes
	ReplaceEscapedQuotes(cmd);
// Remove any end spaces
	CText command = cmd;
	command.Trim(CText::trim::BOTH);

// Show command line arguments
  // MessageBox(NULL,cmd,"Command line",MB_OKCANCEL | MB_ICONEXCLAMATION);

   int nrArgs = arg.Count(command.Str());
  // printf("%s %d\n", cmd, nrArgs);

// Save the startup path
   GetCurrentDirectory(MAX_PATH,backupPath);

 //  VLDDisable();

// If double clicking on Prospa files don't make a new instance if one is already present
   HWND curWin = FindWindow("MAIN_PROSPAWIN", NULL);
	//printf("curWin = %X\n",curWin);
   fileName[0] = '\0';
   if(curWin)
   {
      char filePath[MAX_PATH];
      // Extract the cmd line argument path and name
      CText path;
      CText text;
      if(nrArgs == 2) 
      {
         path = arg.Extract(1);
			path.RemoveQuotes();
         long i = path.ReverseSearch('\\');
         if(i > 0)
         {
            path = path.Start(i-1);
            strncpy_s(applicationHomeDir,MAX_PATH,path.Str(),_TRUNCATE);
         }

         path = arg.Extract(2);
			path.RemoveQuotes();

         i = path.ReverseSearch('\\');
         if(i > 0)
         {
            text = path.End(i+1);
            strncpy_s(fileName,MAX_PATH,text.Str(),_TRUNCATE);
            text = path.Start(i-1);
            strncpy_s(filePath,MAX_PATH,text.Str(),_TRUNCATE);
         }

      // If filename is supplied then use current version of Prospa
      // Otherwise just start up Prospa normally
         if(fileName[0] != '\0' && strcmp(GetExtension(fileName),"pex"))
         {
         // Display the current window
 	         SetForegroundWindow(curWin);

            SendMessage(curWin,WM_USER_LOADDATA,(WPARAM)strlen(filePath),0);
            for(i = 0; i < strlen(filePath); i++)
            {
               SendMessage(curWin,WM_USER_LOADDATA,(WPARAM)(long)filePath[i],0);
            }

         // Send the file to load character by character
            SendMessage(curWin,WM_USER_LOADDATA,(WPARAM)strlen(fileName),0);
            for(i = 0; i < strlen(fileName); i++)
            {
               SendMessage(curWin,WM_USER_LOADDATA,(WPARAM)(long)fileName[i],0);
            }
 	         return(0);
         }
      }
   }

	TraceSymbol::init();

// Initialize GDI+.
   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

// Add support for HTML window
   if(!RegisterHTMLViewer())
      return(0);

   if ((dwTlsIndex = TlsAlloc()) == 0xFFFFFFFF) 
      return(0); 

   ThreadGlobals* data = new ThreadGlobals;
   data->curGUIWin = NULL;
	data->errInfo.lineNr = -1;
	data->curCmdInfo.lineNr = -1;
	data->errInfo.errorFound = false;
	data->errInfo.blocked = false;

   if (!TlsSetValue(dwTlsIndex, (void*)data)) 
      return(ERR); 

	mainThreadID = GetCurrentThreadId();
   gErrorInfo.blocked = false;
	gErrorInfo.errorFound = false;
// Start macro abort thread
 //  _beginthread(AbortMacro,0,(PVOID)0);
   DWORD id;
   HANDLE th = CreateThread(NULL,0,AbortMacro,(PVOID)0,0,&id);

   threadAbortIDs.clear();
   threadAbortPresent = false;

// Load DLLs Prospa needs
   LoadSystemDLLs();

// Store current application instance as a global
   prospaInstance = hInst;

// Get the current folder
   GetCurrentDirectory(MAX_PATH,currentPath);
      
// Load bitmaps
   LoadBitMaps();

   gMainWindowColor = GetSysColor(COLOR_BTNFACE);
  // gMainWindowColor = RGB(80,80,80); Dark Mode!

// Extract application path from command line
   {
      CText path;
      path = arg.Extract(1);
		path.RemoveQuotes();

	//	MessageBox(NULL,path.Str(),"Arg1 ",MB_OKCANCEL | MB_ICONEXCLAMATION);

      long i = path.ReverseSearch('\\');
      if(i > 0)
      {
         path = path.Start(i-1);
         strncpy_s(applicationHomeDir,MAX_PATH,path.Str(),_TRUNCATE);
      }
   } 


// This becomes the prospa 'appdir'  
   SetCurrentDirectory(applicationHomeDir);

  // printf("%s\n",applicationHomeDir);

// Make the users preferences folder if it doesn't exist
   MakePreferencesFolder();
   
// Initialize global variable
   InitializeVariables();
         
// Set up window classes and register them      
   if(!RegisterWindowClasses(prospaInstance))
      return(false);
 
// Register the grid class
	if (!BabyGrid::RegisterGridClass(prospaInstance))
		return (false);

// Make fonts used by windows
   MakeUserFonts();
         
// Initialise cursors, border sizes and pens
   MiscInit();

   TextMessage("> ");
    
// Initalise displayed dialog list
   InitialiseDialogList();

// Set up command list and operator precedence for CLI
   InitializeProspaCommandList();
	InitializeWidgetCommandList();
   SetUpPrecedence();

// Get macro menu and search path
   LoadMenuAndSearchPaths();

// Initialize any external DLLs
	{
		Interface itfc;
		if(LoadDLLs(&itfc,"") == ERR)
			exit(0);
	}

// Check if this is a valid license (comment out for SpinsolveTester)
   if((!strcmp(APPLICATION_NAME,"Prospa") || !strcmp(APPLICATION_NAME,"SpinsolveExpert") || gReadOnly) && !strcmp(VERSION_SPECIAL,"Normal"))
   {
      int result = OK;
      if(!gReadOnly)
         result = ValidLicense();
      if(result != OK)
      {
         CText txt;
         txt.Format("You don't have a valid license to run Prospa! (Error code %d).\rThe invalid license file will be deleted if you press OK.\rPlease contact Magritek support to obtain one.\n\nsupport@magritek.com",result);
         if(MessageBox(NULL,txt.Str(),"Invalid license",MB_OKCANCEL | MB_ICONEXCLAMATION) ==  IDCANCEL)
            exit(0);
         DeleteFile("license.txt");
         DeleteFile("prospaLicense.lic");
         exit(0);
      }
   }

   SetCurrentDirectory(backupPath);
      
// Load the edit history
   LoadEditList();

// Intialise debugger
   DebugInitialise();

// Load some initialization macros
   int r = RunStartUpMacros();

// Extract command line argument 
// And open any file

   if(r == OK)
   {
      if(nrArgs == 2) // Double clicked on a prospa file and prospa is not already running
      {
         CText path;
         CText file;
         path = arg.Extract(2);
	   	path.RemoveQuotes();

			if(path.Size() == 0)
			{
				r = RunLastLayout();
			}
			else
			{
				long i = path.ReverseSearch('\\');
				if(i > 0)
				{
					file = path.End(i+1);
					path = path.Start(i-1);
					char *ext = GetExtension(file);
					if(!strcmp(ext,"pex")) // Double clicked on a .pex file
						r = ProcessMacroStr(0,NULL,NULL,file.Str(),"","",file.Str(),path.Str());
					else // Double clicked on a Prospa file (e.g. .1d, .mac etc)
					{
						r = RunLastLayout();
						if(r == OK)
							OpenCommandLineFile(file.Str(),path.Str());
					}
				}
			}
      }
      else if(nrArgs > 2) // Run prospa from CLI with optional argument. Note all arguments must be quoted ("prospa_path" "pex path" "arg1" "arg2" ... "argn")
      {
         CText path;
         CText file;
         CText command;
         path = arg.Extract(2);
	   	path.RemoveQuotes();

         long i = path.ReverseSearch('\\');

         if(i > 0) // A path has been included in pex file so extract it
         {
            file = path.End(i+1);
            path = path.Start(i-1);
            char *ext = GetExtension(file);
            CText arguments = "";
				path.RemoveQuotes();
            for(int i = 3; i <= nrArgs; i++)
				{
					CText argN = arg.Extract(i);
				   argN.RemoveQuotes();
					if(i == 3)
						arguments = "\"" + argN +  "\"";
					else
						arguments = arguments + "," +  "\"" + argN +  "\"";
				}
            command = file + "(" + arguments + ")";
				//printf("Command = %s\n",command.Str());

        //    printf("1: %s %s %s\n", command.Str(), file.Str(), path.Str());

            if(!strcmp(ext,"mac") || !strcmp(ext,"pex") ) 
               r = ProcessMacroStr(0,NULL,NULL,command.Str(),"","",file.Str(),path.Str());
         }
			else // No path given so use the current folder
			{
            file = path;
            char *ext = GetExtension(file);
            CText arguments = "\"";
            CText argN = arg.Extract(3);
            arguments = arguments +argN + "\"";
            for(int i = 4; i < nrArgs; i++)
				{
					argN = arg.Extract(i);
               arguments = arguments + "," +  "\"" + argN +  "\"";
				}
            command = file + "(" + arguments + ")";
        //    printf("2: %s %s %s\n", command.Str(), file.Str(), currentPath);

            if(!strcmp(ext,"mac") || !strcmp(ext,"pex") ) 
               r = ProcessMacroStr(0,NULL,NULL,command.Str(),"","",file.Str(),currentPath);
			}
      } 
      else // Double clicked on Prospa
      {
         r = RunLastLayout();
      }
   }

// If there has been an error running startup macros or Prospa then run the default macro to find error
   if(r == ERR)
   {
      r = ProcessMacroStr(LOCAL,defaultMacro); 
      if(r != ERR)
      {
         TextMessage("\n   Error: %s",GetErrorInfo()->lastError.Str());
         if(GetErrorInfo()->procedure == "")
           TextMessage("\n   Error occurred in line #%d of macro '%s:%s'\n\n> ",GetErrorInfo()->lineNr,GetErrorInfo()->macro.Str(),GetErrorInfo()->procedure.Str());
         else
           TextMessage("\n   Error occurred in line #%d of macro '%s'\n\n> ",GetErrorInfo()->lineNr,GetErrorInfo()->macro.Str());
      }
      else // Even the default macro is bad!
      {
         PostQuitMessage(0);
         exit(0);
      }
   }

// Restore current folder
   SetCurrentDirectory(currentPath);
   strncpy_s(gCurrentDir,MAX_PATH,currentPath,_TRUNCATE);

// Begin the message loop ***************************************
   while(GetMessage(&lpMsg,NULL,0,0))    
   {
      if(isModelessDialog) // Process returns any tabs for modeless dialogs
      {
         r = IsDialogMessage(currentAppWindow,&lpMsg);
      }
      else // Process accelerator keys for other windows
      {
         r = TranslateAccelerator(currentAppWindow,currentAccel,&lpMsg);
      }
      
      if(r == 0) // Process all other events for non-dialog windows
      {
         TranslateMessage(&lpMsg);
         HTMLViewerMessage(&lpMsg);
         DispatchMessage(&lpMsg);
      }
		if(exitProgram)
			break;
   }

	ShutDown();

   GdiplusShutdown(gdiplusToken);

	return(0);
   //return(lpMsg.wParam);
}

void LoadBitMaps()
{
   iconBMPs[0] = LoadBitmap(prospaInstance,"FOLDER_BMP");
   iconBMPs[1] = LoadBitmap(prospaInstance,"PROSPA_FILE");
   iconBMPs[2] = LoadBitmap(prospaInstance,"CTRL_BUTTON");
   iconBMPs[3] = LoadBitmap(prospaInstance,"CTRL_COLORBOX");
   iconBMPs[4] = LoadBitmap(prospaInstance,"CTRL_CHECKBOX");
   iconBMPs[5] = LoadBitmap(prospaInstance,"CTRL_GROUPBOX");
   iconBMPs[6] = LoadBitmap(prospaInstance,"CTRL_LISTBOX");
   iconBMPs[7] = LoadBitmap(prospaInstance,"CTRL_PROGRESSBAR");
   iconBMPs[8] = LoadBitmap(prospaInstance,"CTRL_RADIOBUTTON");
   iconBMPs[9] = LoadBitmap(prospaInstance,"CTRL_SLIDER");
   iconBMPs[10] = LoadBitmap(prospaInstance,"CTRL_STEXT");
   iconBMPs[11] = LoadBitmap(prospaInstance,"CTRL_STATUSBOX");
   iconBMPs[12] = LoadBitmap(prospaInstance,"CTRL_TEXTBOX");
   iconBMPs[13] = LoadBitmap(prospaInstance,"CTRL_TEXTMENU");
   iconBMPs[14] = LoadBitmap(prospaInstance,"CTRL_HTMLBOX");
   iconBMPs[15] = LoadBitmap(prospaInstance,"CTRL_UPDOWN");
   iconBMPs[16] = LoadBitmap(prospaInstance,"CTRL_1D");
   iconBMPs[17] = LoadBitmap(prospaInstance,"CTRL_2D");
   iconBMPs[18] = LoadBitmap(prospaInstance,"CTRL_3D");
   iconBMPs[19] = LoadBitmap(prospaInstance,"CTRL_CLI");
   iconBMPs[20] = LoadBitmap(prospaInstance,"CTRL_EDITOR");
   iconBMPs[21] = LoadBitmap(prospaInstance,"BOOK_PIX");
   iconBMPs[22] = LoadBitmap(prospaInstance,"PAGE_PIX");
   iconBMPs[23] = LoadBitmap(prospaInstance,"OPEN_BOOK_PIX");
   iconBMPs[24] = LoadBitmap(prospaInstance,"CLOSED_FOLDER_PIX");
   iconBMPs[25] = LoadBitmap(prospaInstance,"OPEN_FOLDER_PIX");
   iconBMPs[26] = LoadBitmap(prospaInstance,"CTRL_DIVIDER");
   iconBMPs[27] = LoadBitmap(prospaInstance,"CTRL_PICTURE");
   iconBMPs[28] = LoadBitmap(prospaInstance,"CTRL_TAB");
   iconBMPs[29] = LoadBitmap(prospaInstance,"PAGE2_PIX");
   iconBMPs[30] = LoadBitmap(prospaInstance,"LINK_BMP");
	iconBMPs[31] = LoadBitmap(prospaInstance,"CTRL_GRID");
	iconBMPs[32] = LoadBitmap(prospaInstance,"BLANK_PIX");
	iconBMPs[33] = LoadBitmap(prospaInstance,"FULL_UCS");
	iconBMPs[34] = LoadBitmap(prospaInstance,"FULL_UCS1");
	iconBMPs[35] = LoadBitmap(prospaInstance,"FULL_UCS2");
	iconBMPs[36] = LoadBitmap(prospaInstance,"FULL_UCS3");
	iconBMPs[37] = LoadBitmap(prospaInstance,"FULL_UCS4");
	iconBMPs[38] = LoadBitmap(prospaInstance,"FULL_UCS5");
	iconBMPs[39] = LoadBitmap(prospaInstance,"CENTER_UCS");
	iconBMPs[40] = LoadBitmap(prospaInstance,"SPINSOLVE");

}


short RunLastLayout()
{
// Load the rest of the windows
   SetCurrentDirectory(userPrefDir);
   SetCurrentDirectory("..\\Windows");

   reportErrorToDialog= true;
   if(ProcessMacroStr(LOCAL,"lastProspaLayout.mac"))
      return(ERR);
   if(IsWindowVisible(cliWin)) reportErrorToDialog = false;
   return(OK);
}


/*****************************************************************
    Load DLLs required for Prospa to run
*****************************************************************/

void LoadSystemDLLs()
{
   if(!LoadLibrary("RICHED20.DLL"))
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't find DLL riched20.dll - aborting");
      exit(0);
   }
}

/*****************************************************************
    Open the file the user has selected in the Windows file
    system or which has been sent via the WM_USER_LOADDATA
    message.
*****************************************************************/

void OpenCommandLineFile(char *fileName, char *path)
{
	const int EXTENSION_STR_LENGTH = 50;
   char extension[EXTENSION_STR_LENGTH];
   Interface itfc;

  // TextMessage("Open %s %s\n",fileName,path);

   strncpy_s(extension,EXTENSION_STR_LENGTH,GetExtension(fileName),_TRUNCATE);   
   ToLowerCase(extension);  

   if(!strcmp(extension,"pt1"))
   {
      Plot1D::curPlot()->plotParent->LoadPlots(path,fileName);
      MyInvalidateRect(Plot1D::curPlot()->plotParent->hWnd,NULL,TRUE);
   }
   else if(!strcmp(extension,"pt2"))
   {
      Plot2D::curPlot()->plotParent->LoadPlots(path,fileName);
		Plot2D::curPlot()->Invalidate();
   }
   else if(!strcmp(extension,"1d"))
   {
      Variable *var1,*var2;
	   CText name1 = fileName;
		name1.RemoveExtension();
		name1 = name1 + "_x";
		CText name2 = fileName;
		name2.RemoveExtension();
		name2 = name2 + "_y";
		CText arg = name1 + "," + name2;

		var1 = AddVariable(&itfc,GLOBAL, NULL_VARIABLE, name1.Str());  				   
		var2 = AddVariable(&itfc,GLOBAL, NULL_VARIABLE, name2.Str());  				   
		LoadData(&itfc, path, fileName, var1, var2);
      if(var2->GetType() == NULL_VARIABLE)
         PlotXY(&itfc,name1.Str());
      else
         PlotXY(&itfc,arg.Str());
		Plot1D::curPlot()->Invalidate();
   }
   else if(!strcmp(extension,"2d"))
   {
	   CText name = fileName;
		name.RemoveExtension();
      LoadData(&itfc,path,fileName,name.Str(),GLOBAL);
      DisplayMatrixAsImage(&itfc,name.Str());
		Plot2D::curPlot()->Invalidate();
   }
   else if(!strcmp(extension,"3d"))
   {
	   CText name = fileName;
		name.RemoveExtension();
      LoadData(&itfc,path,fileName,name.Str(),GLOBAL);
   }                     
   else if(!strcmp(extension,"txt") || !strcmp(extension,"par") ||
      !strcmp(extension,"mac") || !strcmp(extension,"asm") ||
      !strcmp(extension,"pex"))
   {
      if(!curEditor)
         return;
      EditParent *ep = curEditor->edParent;
      if(curEditor->CheckForUnsavedEdits(ep->curRegion) == IDCANCEL)
         return;
      strncpy_s(curEditor->edPath,MAX_PATH,path,_TRUNCATE);
      strncpy_s(curEditor->edName,MAX_PATH,fileName,_TRUNCATE);
	   SetCursor(LoadCursor(NULL,IDC_WAIT));
      LoadEditorGivenPath(curEditor->edPath,curEditor->edName);
      SetEditTitle();  
      AddFilenameToList(procLoadList,curEditor->edPath,curEditor->edName);
	   SetCursor(LoadCursor(NULL,IDC_ARROW));  
   } 
   else if(!strcmp(extension,"htm")) // A help file
   {
      CText msg;
      TextMessage("path = %s\n",path);
      TextMessage("fileName = %s\n",fileName);
      msg.Format("HelpViewer,%s:%s",path,fileName);
      SendMessageToGUI(msg.Str(),-1);
   }
}

/*******************************************************************
 * Run all macros in the preferences folder
 ******************************************************************/

short RunStartUpMacros()
{
    WIN32_FIND_DATA findData;
   char backUpPath[MAX_PATH];
   short r = ERR;

// Save the current directory           
   GetCurrentDirectory(MAX_PATH,backUpPath);

// First see if there is user path defined
   r = SetCurrentDirectory(userPrefDir);

// Run all the macros in this folder if found
   if(r != 0)
   {
   // Run all macros in this folder    
      short cnt = CountFiles("*.mac");

      HANDLE hdl =  FindFirstFile("*.mac",&findData);
      
      for(short i = 0; i < cnt; i++)
      {
         r = ProcessMacroStr(GLOBAL,(char*)findData.cFileName); 
         if(r == ERR)
            break;
         FindNextFile(hdl,&findData);
      }
      FindClose(hdl);
   }
// Restore original folder
   SetCurrentDirectory(backUpPath);

   return(r);
}



/*******************************************************************************
  If the preferences folder does not exist in the home directory then make it

  Default path $homedir$\%APPDATA%\APP_NAME\PreferencesV???
*******************************************************************************/

short MakePreferencesFolder()
{
   char currentwd[MAX_PATH];
	char appDataPath[MAX_PATH];
   char suppliedPref[MAX_PATH];
	char tempDir[MAX_PATH];
	bool foundPrefDir = false;

// See if there is a user defined preferences directory
   sprintf(suppliedPref,"%s\\preferences\\Startup\\preferencesFolder.txt",applicationHomeDir);
	if(IsFile(suppliedPref))
	{
		FILE *fp = fopen(suppliedPref,"r");
		if(fp)
		{
			if(fgets(tempDir,MAX_PATH,fp))
			{
            ExpandEnvironmentStrings(tempDir, userPrefDir, MAX_PATH);
            SimplifyDoubleEscapes(userPrefDir);  // Get rid of any duplicate '\'
				foundPrefDir = true;
			}
			fclose(fp);
		}
	}
// Use the default directory location 
	if(!foundPrefDir)
	{
      ExpandEnvironmentStrings("%APPDATA%", appDataPath, MAX_PATH);
      SimplifyDoubleEscapes(appDataPath);  // Get rid of any duplicate '\'      
      sprintf(tempDir,"%%APPDATA%%\\%s\\%s",APPLICATION_NAME,PREFERENCE_NAME);
      ExpandEnvironmentStrings(tempDir, userPrefDir, MAX_PATH);  
      SimplifyDoubleEscapes(userPrefDir);  // Get rid of any duplicate '\'  
	}

// Record current directory
   GetCurrentDirectory(MAX_PATH,currentwd);   
   
// Make the preference directory
	extern int MakeDirectoryCore(char *newDir);
	if(!MakeDirectoryCore(userPrefDir))
	{
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to create preferences directory'%s'",userPrefDir);
      return(ERR);
   }  

// Move into the preference directory
   if(!SetCurrentDirectory(userPrefDir))
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into the preferences directory '%s'",userPrefDir);      
      return(ERR);
   }

// Make a startup folder if one doesn't exist  
   if(!SetCurrentDirectory("Startup"))
   {
      if(!CreateDirectory("Startup",NULL))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to create '%s\\Startup'",userPrefDir);
         return(ERR);
      }   
      if(!SetCurrentDirectory("Startup"))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s\\Startup'",userPrefDir);      
         return(ERR);
      }
   // Copy supplied preferences folder to the new one
      if(CopyStartUpMacrosToPreferences() == ERR)
         return(ERR);
   }  

// Make a Core Macros folder if one doesn't exist 
   SetCurrentDirectory(userPrefDir);
   if(!SetCurrentDirectory("Core Macros"))
   {
      if(!CreateDirectory("Core Macros",NULL))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to create '%s\\Core Macros'",userPrefDir);
         return(ERR);
      }   
      if(!SetCurrentDirectory("Core Macros"))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s\\Core Macros'",userPrefDir);      
         return(ERR);
      }
   // Copy supplied preferences folder to the new one
      if(CopyCoreMacrosToPreferences() == ERR)
         return(ERR);
   }  

// Make a windows folder if one doesn't exist  
   SetCurrentDirectory(userPrefDir);
   if(!SetCurrentDirectory("Windows"))
   {
      if(!CreateDirectory("Windows",NULL))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to create '%s\\Windows'",userPrefDir);
         return(ERR);
      }   
      if(!SetCurrentDirectory("Windows"))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s\\Windows'",userPrefDir);      
         return(ERR);
      }
   // Copy supplied preferences folder to the new one
      if(CopyWindowsMacrosToPreferences() == ERR)
         return(ERR);
   }  

// Make a menus folder if one doesn't exist 
   SetCurrentDirectory(userPrefDir);
   if(!SetCurrentDirectory("Menus"))
   {
      if(!CreateDirectory("Menus",NULL))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to create '%s\\Menus'",userPrefDir);
         return(ERR);
      }   
      if(!SetCurrentDirectory("Menus"))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s\\Menus'",userPrefDir);      
         return(ERR);
      }
   // Copy supplied preferences folder to the new one
      if(CopyMenuListsToPreferences() == ERR)
         return(ERR);
   }  

   SetCurrentDirectory(currentwd);

   return(OK);
}

/***************************************************************************
 Copy all preferences macros from the supplied folder to the user folder
****************************************************************************/

short CopyStartUpMacrosToPreferences(void)
{
   char userPref[MAX_PATH];
   char suppliedPref[MAX_PATH];
   char srcFile[MAX_PATH];
   char dstFile[MAX_PATH];
   WIN32_FIND_DATA findData;

   GetCurrentDirectory(MAX_PATH,userPref);   
   sprintf(suppliedPref,"%s\\preferences\\Startup",applicationHomeDir);

// Move into the preferences folder
   if(!SetCurrentDirectory(suppliedPref))
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s'",suppliedPref);      
      return(ERR);
   }

   short cnt = CountFiles("*.*");

   HANDLE hdl =  FindFirstFile("*.*",&findData);
   
   for(short i = 0; i < cnt; i++)
   {
      sprintf(srcFile,"%s",findData.cFileName);
      sprintf(dstFile,"%s\\%s",userPref,findData.cFileName);
      if((srcFile[0] != '.') && strcmp(srcFile,".") && strcmp(srcFile,".."))
      {
         short result = CopyFile(srcFile,dstFile,TRUE);
         if(result == 0)
         {
            MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to copy files from Prospa application\r folder to user preferences folder");      
            return(ERR);
         }
      }

      FindNextFile(hdl,&findData);
   }

   FindClose(hdl);
   return(OK);
}

short CopyCoreMacrosToPreferences(void)
{
   char userPref[MAX_PATH];
   char suppliedPref[MAX_PATH];
   char srcFile[MAX_PATH];
   char dstFile[MAX_PATH];
   WIN32_FIND_DATA findData;

   GetCurrentDirectory(MAX_PATH,userPref);   
   sprintf(suppliedPref,"%s\\preferences\\Core Macros",applicationHomeDir);

// Move into the preferences folder
   if(!SetCurrentDirectory(suppliedPref))
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s'",suppliedPref);      
      return(ERR);
   }

   short cnt = CountFiles("*.*");

   HANDLE hdl =  FindFirstFile("*.*",&findData);
   
   for(short i = 0; i < cnt; i++)
   {
      sprintf(srcFile,"%s",findData.cFileName);
      sprintf(dstFile,"%s\\%s",userPref,findData.cFileName);
      if((srcFile[0] != '.') && strcmp(srcFile,".") && strcmp(srcFile,".."))
      {
         short result = CopyFile(srcFile,dstFile,TRUE);
         if(result == 0)
         {
            MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to copy files from Prospa application\r folder to user preferences folder");      
            return(ERR);
         }
      }

      FindNextFile(hdl,&findData);
   }

   FindClose(hdl);
   return(OK);
}

/***************************************************************************
 Copy all preferences macros from the supplied folder to the user folder
****************************************************************************/

short CopyWindowsMacrosToPreferences(void)
{
   char userPref[MAX_PATH];
   char suppliedPref[MAX_PATH];
   char srcFile[MAX_PATH];
   char dstFile[MAX_PATH];
   WIN32_FIND_DATA findData;


// Copy windows folder
   GetCurrentDirectory(MAX_PATH,userPref);   
   sprintf(suppliedPref,"%s\\preferences\\Windows",applicationHomeDir);

// Move into the preferences folder
   if(!SetCurrentDirectory(suppliedPref))
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s'",suppliedPref);      
      return(ERR);
   }

   short cnt = CountFiles("*.*");

   HANDLE hdl =  FindFirstFile("*.*",&findData);
   
   for(short i = 0; i < cnt; i++)
   {
      sprintf(srcFile,"%s",findData.cFileName);
      sprintf(dstFile,"%s\\%s",userPref,findData.cFileName);
      if((srcFile[0] != '.') && strcmp(srcFile,".") && strcmp(srcFile,".."))
      {
         short result = CopyFile(srcFile,dstFile,TRUE);
         if(result == 0)
         {
            MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to copy files from Prospa application\r folder to user preferences folder");      
            return(ERR);
         }
      }

      FindNextFile(hdl,&findData);
   }
   FindClose(hdl);
   SetCurrentDirectory(applicationHomeDir);
   return(OK);
}


/***************************************************************************
 Copy all menu lists from the supplied folder to the user folder
****************************************************************************/

short CopyMenuListsToPreferences(void)
{
   char userPref[MAX_PATH];
   char suppliedPref[MAX_PATH];
   char srcFile[MAX_PATH];
   char dstFile[MAX_PATH];
   WIN32_FIND_DATA findData;


// Copy menus folder
   GetCurrentDirectory(MAX_PATH,userPref);   
   sprintf(suppliedPref,"%s\\preferences\\Menus",applicationHomeDir);

// Move into the preferences folder
   if(!SetCurrentDirectory(suppliedPref))
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s'",suppliedPref);      
      return(ERR);
   }

   short cnt = CountFiles("*.*");

   HANDLE hdl =  FindFirstFile("*.*",&findData);
   
   for(short i = 0; i < cnt; i++)
   {
      sprintf(srcFile,"%s",findData.cFileName);
      sprintf(dstFile,"%s\\%s",userPref,findData.cFileName);
      if((srcFile[0] != '.') && strcmp(srcFile,".") && strcmp(srcFile,".."))
      {
         short result = CopyFile(srcFile,dstFile,TRUE);
         if(result == 0)
         {
            MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to copy files from Prospa application\r folder to user preferences folder");      
            return(ERR);
         }
      }

      FindNextFile(hdl,&findData);
   }
   FindClose(hdl);
   SetCurrentDirectory(applicationHomeDir);
   return(OK);
}

/*******************************************************************
*   Save window positions
*******************************************************************/

short SaveWindowPositions(bool report)
{
   FILE *fp;
   char currentwd[MAX_PATH];

// Save current directory *****************
   GetCurrentDirectory(MAX_PATH,currentwd);

// Move to preferences folder *************
   if(!SetCurrentDirectory(userPrefDir))
   {
      SetCurrentDirectory(currentwd);
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s'",userPrefDir); 
      return(ERR);
   }   

// Open a windows file for write ***********   
   if(!(fp = fopen("windows.mac","w")))
   {
      SetCurrentDirectory(currentwd);
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't save window parameters\rIs <user_home>/preferences/windows.mac write protected?");
      return(ERR);
   }

// Save the window parameters **************
   SaveWinParameters(fp);
   fclose(fp);
 
// Report save to user *********************
   if(report)
     MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","Window positions saved");
   SetCurrentDirectory(currentwd);

   return(OK);
}

/*******************************************************************
* Save directory locations
********************************************************************/

short SaveDirectories(bool report)
{
   FILE *fp;
   char currentwd[MAX_PATH];
   char workingDir[MAX_PATH];
   char macroDir[MAX_PATH];

// Save current directory *****************
   GetCurrentDirectory(MAX_PATH,currentwd);

// Move to preferences folder *************
   if(!SetCurrentDirectory(userPrefDir))
   {
      SetCurrentDirectory(currentwd);
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s'",userPrefDir); 
      return(ERR);
   }

// Open a directories file for write ******
   if(!(fp = fopen("directories.mac","w")))
   {
      SetCurrentDirectory(currentwd);
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't save directories\rIs <user_home>/preferences/directories.mac write protected?");
      return(ERR);
   } 

// Save the directory paths - making sure that all backslashes are escaped.   
 /*  ReplaceSpecialCharacters(gPlot1DDirectory,"\\","\\\\");
   ReplaceSpecialCharacters(gPlot2DDirectory,"\\","\\\\");
   ReplaceSpecialCharacters(gDataDirectory,"\\","\\\\");
   ReplaceSpecialCharacters(gImport1DDataDirectory,"\\","\\\\");
   ReplaceSpecialCharacters(gExport1DDataDirectory,"\\","\\\\");   
   ReplaceSpecialCharacters(gImport2DDataDirectory,"\\","\\\\");
   ReplaceSpecialCharacters(gExport2DDataDirectory,"\\","\\\\");  
   ReplaceSpecialCharacters(gImport3DDataDirectory,"\\","\\\\");
   ReplaceSpecialCharacters(gExport3DDataDirectory,"\\","\\\\");   
   ReplaceSpecialCharacters(gMacroDirectory,"\\","\\\\");   */
   strncpy_s(workingDir,MAX_PATH,VarString(userWorkingVar),_TRUNCATE);
   strncpy_s(macroDir,MAX_PATH,VarString(userMacroVar),_TRUNCATE);
 //  ReplaceSpecialCharacters(workingDir,"\\","\\\\");  
  // ReplaceSpecialCharacters(macroDir,"\\","\\\\");  

   fprintf(fp,"pathnames(\"plot1d\",\"%s\")\n",   PlotFile1D::getCurrPlotDirectory());
	fprintf(fp,"pathnames(\"plot2d\",\"%s\")\n",   PlotFile2D::getCurrPlotDirectory());
	fprintf(fp,"pathnames(\"data\",\"%s\")\n",     PlotFile::getCurrDataDirectory());
	fprintf(fp,"pathnames(\"import1d\",\"%s\")\n", PlotFile1D::getCurrImportDataDirectory());
	fprintf(fp,"pathnames(\"export1d\",\"%s\")\n", PlotFile1D::getCurrExportDataDirectory());   
   fprintf(fp,"pathnames(\"import2d\",\"%s\")\n", PlotFile2D::getCurrImportDataDirectory());
	fprintf(fp,"pathnames(\"export2d\",\"%s\")\n", PlotFile2D::getCurrExportDataDirectory());  
   fprintf(fp,"pathnames(\"import3d\",\"%s\")\n", gImport3DDataDirectory);
   fprintf(fp,"pathnames(\"export3d\",\"%s\")\n", gExport3DDataDirectory);   
   fprintf(fp,"pathnames(\"macrodata\",\"%s\")\n",PlotFile::getCurrMacroDirectory()); 
   fprintf(fp,"pathnames(\"workdir\",\"%s\")\n",  workingDir); 
   fprintf(fp,"pathnames(\"macrodir\",\"%s\")\n", macroDir); 
   fclose(fp);

// Report result to user ******************
   if(report)
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","Current directories saved");
 
// Restore original directory *************
   SetCurrentDirectory(currentwd);

   return(OK);
}




/***********************************************************************
*  Callback procedure for the main prospa window                 
***********************************************************************/

bool prospaInBackGround = true;
bool inAppActivated = false;

/******************************************************************************************
   When loading data by opening a data file this routine is called multiple times
   until the data file's directory and name have been loaded.
   The function works by sending the directory length, then the directory, one character
   at a time, the filename length and then the filename one character at a time.
   This information is sent from a Prospa instance which is started and then exits
   after this information has been sent to the currently running instance.
******************************************************************************************/

void LoadUserData(WPARAM wParam)
{
   static enum  {LOAD_DIR_LENGTH,LOAD_DIR,LOAD_FILE_LENGTH,LOAD_FILE} state = LOAD_DIR_LENGTH;
   static long directorySize = 0;
   static long directoryIndex = 0;
   static char directory[MAX_PATH];
   static long fileSize = 0;
   static long fileIndex = 0;
   static char file[MAX_PATH];

   if(state == LOAD_DIR_LENGTH)
   {
      state = LOAD_DIR;
      directorySize = (long)wParam;
      directoryIndex = 0;
      fileIndex = 0;
   }
   else if(state == LOAD_DIR && directoryIndex <= directorySize-1)
   {
      if(directoryIndex == MAX_PATH) directoryIndex--;
      directory[directoryIndex] = (char)wParam;
      directoryIndex++;

      if(directoryIndex == directorySize)
      {
         directory[directoryIndex] = '\0';
         state = LOAD_FILE_LENGTH;
         directoryIndex = 0;
      }
   }

   else if(state == LOAD_FILE_LENGTH)
   {
      state = LOAD_FILE;
      fileSize = (long)wParam;
      fileIndex = 0;
   }
   else if(state == LOAD_FILE && fileIndex <= fileSize)
   {
      if(fileIndex == MAX_PATH) fileIndex--;
      file[fileIndex] = (char)wParam;
      fileIndex++;

      if(fileIndex == fileSize)
      {
         file[fileIndex] = '\0';
         state = LOAD_DIR_LENGTH;
         fileIndex = 0;
         strncpy_s(gCurrentDir,MAX_PATH,directory,_TRUNCATE);
         SetCurrentDirectory(directory);
         OpenCommandLineFile(file,directory);
      }
   }
}

/********************************************************************
   CLI call to update the main menu
*********************************************************************/

int UpdateMainMenu(Interface* itfc, char args[])
{
   UpdateMainMenu(prospaWin);
   return(OK);
}

/********************************************************************
   Add the names of all the user menu folders to the main menu bar
   and also append the help menu to the end.
*********************************************************************/

void UpdateMainMenu(HWND hWnd)
{
   char menuName[MAX_PATH];
   HMENU hMenu;
   short menuNr;
   int i,j;

   WinData* win = rootWin->FindWinByHWND(prospaWin);

   short *menuLst = win->menuList;
   short listSize = win->menuListSize;
   HMENU winMenu = win->menu;

// Search for user menu
   for(i = 0; i < listSize; i++)
   {
      menuNr = menuLst[i];

      if(menuNr == -1)
      {
         break;
      }
   }

// If -1 found that means there is a user menu to add
   if(i < listSize)
   {
      int cnt = GetMenuItemCount(winMenu);

      for(j = i; j < cnt-(listSize-1-i); j++)
         DeleteMenu(winMenu,i,MF_BYPOSITION);


   // Add macro menu names to menu bar
      for(j = 0; j < VarWidth(userMenusVar)/2; j++)
      {
         strncpy_s(menuName,MAX_PATH,VarList(userMenusVar)[j*2],_TRUNCATE);
         hMenu = CreatePopupMenu();
         InsertMenu(winMenu, i+j, MF_BYPOSITION| MF_STRING | MF_POPUP, (UINT)hMenu, menuName);

      } 

   // Redraw the menu bar
      DrawMenuBar(prospaWin); 

   }
}


void AppendUserMenu(HMENU winMenu)
{
   HMENU hMenu;
   char menuName[MAX_PATH];

// Add macro menu names to menu bar
   for(int i = 0; i < VarWidth(userMenusVar)/2; i++)
   {
      strncpy_s(menuName,MAX_PATH,VarList(userMenusVar)[i*2],_TRUNCATE);
      hMenu = CreatePopupMenu();
      AppendMenu(winMenu, MF_STRING | MF_POPUP, (UINT)hMenu, menuName);
   } 

}
            
/*********************************************************
*   Adds names of all folders in current folder to
*    menu if they contain a macro of the same name
*********************************************************/

int AddMenuFolders(HMENU menu, UINT &cnt)
{
   WIN32_FIND_DATA findData;
   HANDLE h;
   
   
// Find first file/folder *************************
   if((h = FindFirstFile("*",&findData)) == INVALID_HANDLE_VALUE)
   {
      return(OK);
   }

// Find all files adding to menu ************
   UINT base = cnt;
   do
   {
      if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // Is it a directory
      {
       // Ignore '.' and '..' directories
         if(!strcmp(findData.cFileName,".") || !strcmp(findData.cFileName,".."))
				continue;

		// Ignore hidden folders
         if(findData.cFileName[0] == '.' && strlen(findData.cFileName) > 1 && findData.cFileName[1] != '.')
            continue;

	   // Add the menu
         AppendMenu(menu, MF_STRING ,cnt++, findData.cFileName);
         if(cnt-base-1 > MAX_MENU_ITEMS)
         {
            TextMessage("\n   More than %d menu items\n",(int)MAX_MENU_ITEMS);
            break;
         }
      }
	}
   while(FindNextFile(h,&findData));

   
   FindClose(h);
   return(OK);
}

          
/*********************************************************
*   Adds names of macro files to menu in current folder
*********************************************************/

int AddMenuMacros(HMENU menu, UINT &cnt)
{
   WIN32_FIND_DATA findData;
   HANDLE h;
   char fileName[MAX_PATH];
   
   
// Find first file *************************
   if((h = FindFirstFile("*.mac",&findData)) == INVALID_HANDLE_VALUE)
   {
      return(OK);
   }

// Find all files adding to menu ************
   UINT base = cnt;
   while(1)
   {
      if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         strncpy_s(fileName,MAX_PATH,findData.cFileName,_TRUNCATE);
         RemoveExtension(fileName);
         if(_strnicmp(fileName,"macrohelp",9) && _strnicmp(fileName,"addmacros",9) && _strnicmp(fileName, "information", 11)) // Ignore help/addMacro file
         {
				char* idx = 0;
				if(idx = strstr(fileName,"_menu")) // Strip of this suffix and replace with a space so its not visible
				{                                  // Suffix will be replaced before calling this macro. 
					idx[0] = ' ';
					idx[1] = '\0';
				}
            AppendMenu(menu, MF_STRING ,cnt++, fileName);
            if(cnt-base-1 > MAX_MENU_ITEMS)
            {
               TextMessage("\n   More than %d menu items\n",(int)MAX_MENU_ITEMS);
               break;
            }
         }
      }
      if(!FindNextFile(h,&findData)) break;
   }
  
   FindClose(h);
   return(OK);
}

int ProspaFunctions(Interface* itfc ,char args[])
{
   CText func;
   short r;
   char fileName[MAX_PATH];

   if((r = ArgScan(itfc,args,0,"command","e","t",&func)) < 0)
      return(r);

   HWND hWnd = currentAppWindow;

   if(func == "load data")
   {
      fileName[0] = '\0';               
      LoadDataDialog(hWnd,PlotFile::getCurrDataDirectory(),fileName);
   }
   else if(func == "save data")
   {
      fileName[0] = '\0';               
      SaveDataDialog(hWnd,PlotFile::getCurrDataDirectory(),fileName);
   }
   else if(func == "import data")
   {
      DialogBox(prospaInstance,"IMPORTDLG",hWnd,ImportDlgProc);
   } 
   else if(func == "export data")
   {
      DialogBox(prospaInstance,"EXPORTDLG",hWnd,ExportDlgProc);
   } 
   else if(func == "about prospa")
   {
      CWinEnable winEnable;
      winEnable.Disable(NULL);
      DialogBox(prospaInstance,"ABOUTDLG",hWnd,AboutDlgProc); 
      winEnable.Enable(NULL);    
   }
   else if(func == "halt macro")
   {
      gAbortMacro = true;
   }
   else if(func == "preferences")
   {
      DialogBox(prospaInstance, "PREFERENCESDLG", hWnd, PreferencesDlgProc);      
   }
   else if(func == "general help")
   {
      char args[MAX_PATH];
      sprintf(args,"%s\\documentation\\Prospa.chm::/Default.htm",applicationHomeDir);
      HtmlHelp(GetDesktopWindow(),args,HH_DISPLAY_TOPIC, 0); 
   }  
   else if(func == "help viewer")
   {
      CText command,msg;
      command.Format("ProspaHelpViewer(\"General Information\\Introduction\",\"Introduction\")");
      msg.Format("HelpViewer,General Information:Introduction");
      WinData *win = rootWin->FindWinByTitle("Command and Macro Help");
      if(!win)
         ProcessMacroStr(0,NULL,NULL,command.Str(),"","","ProspaHelpViewer.mac","");
      else
      {
         SendMessageToGUI(msg.Str(),0);
      }
   } 
   else if(func == "show/hide windows")
   {
      if(AnyGUIWindowVisible())
         HideGUIWindows();
      else
         ShowGUIWindows(SW_SHOWNOACTIVATE);
   }  
   return(OK);
}

/*********************************************************
*   Allow the user to select a macro to run
*********************************************************/

short RunMacroDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index = 1;

// Set the path if unknown name
   strncpy_s(pathName,MAX_PATH,PlotFile::getCurrMacroDirectory(),_TRUNCATE);
      
   err = FileDialog(hWnd, true, pathName, fileName, 
                    "Macro to run", CentreHookProc, NULL, noTemplateFlag, 1, &index,
                    "Script Files","mac");
          
// Make sure returned path has standard form
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,PlotFile::getCurrMacroDirectory());

	return(err);
}

/*********************************************************
*   Allow the user to save a layout file
*********************************************************/

short SaveLayoutDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index = 1;

   strncpy_s(pathName,MAX_STR,gCurrentDir,_TRUNCATE);
      
   err = FileDialog(hWnd, false, pathName, fileName, 
                    "Save window layout", CentreHookProc, NULL, noTemplateFlag, 1, &index,
                    "Script Files","mac");
      
   return(err);
}



/*********************************************************
*   Load the user defined menu and search paths
*********************************************************/
//TODO convert to CTEXT
//TODO consider how to handle errors
short LoadMenuAndSearchPaths()
{
   char saveFolder[MAX_PATH];
   char newMenuDir[MAX_PATH];
   int r;
   int cnt = 0;
   char name[MAX_PATH];
   Variable* installMenusVar = NULL;
   bool installMenus = false;
   WIN32_FIND_DATA findData;
   Interface itfc;

// Save current folder 
   GetCurrentDirectory(MAX_PATH,saveFolder);

// Try and load the user menu list from the user's home directory
   if(!SetCurrentDirectory(userPrefDir))
   {
      SetCurrentDirectory(saveFolder);
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s'",userPrefDir); 
      if(installMenusVar)
         delete [] installMenusVar;
      return(ERR);
   }

// Load user-defined menu list and return in userMenusVar
   itfc.macro = NULL;
	CText listName;
	r = Load(&itfc,"\"userMenus.lst\"");
	if(r != ERR)
	{
		if(CopyVariable(userMenusVar,&itfc.retVar[1],FULL_COPY) == ERR)
		{
			return(ERR);
		}
	}
	else
	{
       MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't find user-menus list\r(either no 'preferences' folder or no list)");
		 return(ERR);
	}
// Merge with any additional menus (userMenus[1..10].lst)
	//Variable tempVar;
	//char **finalList;
	//for(int i = 1 ; i <= 10; i++)
	//{
	//	listName.Format("\"userMenus%d.lst\"",i);
 //     r = Load(&itfc,listName.Str());
	//	if(r != ERR) // If file exists append to userMenus variable
	//	{
	//		if(CopyVariable(&tempVar,&itfc.retVar[1],FULL_COPY) == ERR)
	//		{
	//			return(ERR);
	//		}
	//		finalList = JoinLists(userMenusVar->GetList(), tempVar.GetList(), userMenusVar->GetDimX(), tempVar.GetDimX());
	//		userMenusVar->AssignList(finalList, userMenusVar->GetDimX() + tempVar.GetDimX());
	//	}
	//}

// Try and load the macro search path
   r = Load(&itfc,"\"macroSearchPath.lst\"");
        
// If list found then copy to 'macroPathVar' variable
   if(r == OK)
   {
      if(CopyVariable(macroPathVar,&itfc.retVar[1],FULL_COPY) == ERR)
      {
         return(ERR);
      }
   }
   else
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't find macro search-path list\r(either no 'preferences' folder or no list)");
   }         

// Try and load the DLL search path
   r = Load(&itfc,"\"dllSearchPath.lst\"");

// If list found then copy to 'dllPathVar' variable
   if(r == OK)
   {
      if(CopyVariable(dllPathVar,&itfc.retVar[1],FULL_COPY) == ERR)
      {
         return(ERR);
      }
   }
   else
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't find dll search-path list\r(either no 'preferences' folder or no list)");
   } 

// Restore the original folder
   SetCurrentDirectory(saveFolder);

   if(installMenusVar)
      delete [] installMenusVar;

   return(OK);
 }               

/***********************************************************************
*  Initialise a few prospa variables                
***********************************************************************/


void InitializeVariables()
{
   Variable *var;
   char path[MAX_PATH];
   char temp [MAX_PATH];
  
// Get current directory   
   GetCurrentDirectory(MAX_PATH,applicationHomeDir);
   strncpy_s(gCurrentDir,MAX_PATH,applicationHomeDir,_TRUNCATE);

// The constant null
   var = AddGlobalVariable(NULL_VARIABLE,"null");
   var->MakeNullVar();   
   var->SetPermanent(true);
   var->SetReadOnly(true);
   var->SetVisible(false);

// The constant pi
   var = AddGlobalVariable(FLOAT32,"pi");
   var->MakeAndSetFloat(3.1415927);   
   var->SetPermanent(true);
   var->SetReadOnly(true);

// The constant pi (double)
   var = AddGlobalVariable(FLOAT64,"PI");
   var->MakeAndSetDouble(3.141592653589793238);   
   var->SetPermanent(true);
   var->SetReadOnly(true);

// The prospa application directory
   var = AddGlobalVariable(UNQUOTED_STRING,"appdir");
   var->MakeAndSetString(applicationHomeDir);   
   var->SetPermanent(true);
   var->SetReadOnly(true);
   var->SetVisible(false);

// The desktop
   SHGetSpecialFolderPath(prospaWin, path, CSIDL_DESKTOP,0);
   SimplifyDoubleEscapes(path);  // Get rid of any duplicate '\' 
   var = AddGlobalVariable(UNQUOTED_STRING,"desktop");
   var->MakeAndSetString(path);   
   var->SetPermanent(true);
   var->SetReadOnly(true);
   var->SetVisible(false);

// The home directory
   strncpy(temp,"%USERPROFILE%",MAX_PATH);
   ExpandEnvironmentStrings(temp, path, MAX_PATH);  
   SimplifyDoubleEscapes(path);  // Get rid of any duplicate '\' 
   var = AddGlobalVariable(UNQUOTED_STRING,"homedir");
   var->MakeAndSetString(path);   
   var->SetPermanent(true);
   var->SetReadOnly(true);
   var->SetVisible(false);

// Make two special numbers not an number and infinity
   var = AddGlobalVariable(FLOAT32,"nan");
   NaN = log10(-1.0);
   var->MakeAndSetFloat(NaN);
   var->SetPermanent(true);
   var->SetReadOnly(true);
   var->SetVisible(false);

   var = AddGlobalVariable(FLOAT32,"inf");
   Inf = log10(0.0);
   var->MakeAndSetFloat(*(float*)&Inf);
   var->SetPermanent(true);
   var->SetReadOnly(true);
   var->SetVisible(false);

// Make a menu name + path list
   userMenusVar = AddGlobalVariable(LIST,"usermenus");
   userMenusVar->SetPermanent(true);
   userMenusVar->SetReadOnly(true);
   userMenusVar->SetVisible(false);

// Make a menu name + path list
   macroPathVar = AddGlobalVariable(LIST,"macrosearchpath");
   macroPathVar->SetPermanent(true);
   macroPathVar->SetReadOnly(true);
   macroPathVar->SetVisible(false);

// Make a menu name + path list
   dllPathVar = AddGlobalVariable(LIST,"dllsearchpath");
   dllPathVar->SetPermanent(true);
   dllPathVar->SetReadOnly(true);
   dllPathVar->SetVisible(false);

// Make variable pointing to users prospa preference directory
   prospaPrefVar = AddGlobalVariable(UNQUOTED_STRING,"prefdir");
   prospaPrefVar->MakeAndSetString(userPrefDir);                
   prospaPrefVar->SetPermanent(true);
   prospaPrefVar->SetReadOnly(true);
   prospaPrefVar->SetVisible(false);

// Make variable pointing to users temporary directory
   strncpy_s(path,MAX_PATH,"%TEMP%",_TRUNCATE);
   ExpandEnvironmentStrings(path, prospaTempDir, MAX_PATH);  
   SimplifyDoubleEscapes(prospaTempDir);  // Get rid of any duplicate '\'  

   prospaTempVar = AddGlobalVariable(UNQUOTED_STRING,"tempdir");
   prospaTempVar->MakeAndSetString(prospaTempDir);                
   prospaTempVar->SetPermanent(true);
   prospaTempVar->SetReadOnly(true);
   prospaTempVar->SetVisible(false);

// Ensure that all Prospa startup files are stored in a folder called Preferences\Startup
   strcat(userPrefDir,"\\Startup");

// Make variable pointing to working directory (defaults to appdir)
   char workingDir[MAX_PATH];
   sprintf(path,"%%USERPROFILE%%\\My Documents\\%s",APPLICATION_NAME);
   ExpandEnvironmentStrings(path, workingDir, MAX_PATH);
   SimplifyDoubleEscapes(workingDir);  // Get rid of any duplicate '\'  

   userWorkingVar = AddGlobalVariable(UNQUOTED_STRING,"workdir");
   userWorkingVar->MakeAndSetString(workingDir);                
   userWorkingVar->SetPermanent(true);
   userWorkingVar->SetVisible(false);

// Make variable pointing to macro directory (defaults to appdir\Macros)
   char macroDir[MAX_PATH];
   strncpy_s(macroDir,MAX_PATH,applicationHomeDir,_TRUNCATE); 
   strcat(macroDir,"\\Macros"); 
   userMacroVar = AddGlobalVariable(UNQUOTED_STRING,"macrodir");
   userMacroVar->MakeAndSetString(macroDir);                
   userMacroVar->SetPermanent(true);
   userMacroVar->SetVisible(false);
}

/***********************************************************************
*  Define the default fonts used by the program                
***********************************************************************/

void MakeUserFonts()
{
// Make some fonts
//   HDC hdc = GetDC(editWin);
   HDC hdc = GetDC(prospaWin);
   cliFont = MakeFont(hdc, "Courier New", 10, 0, 0, 0);                                  
   editFont = MakeFont(hdc, "Courier New", editFontSize, 0, 0, 0);                                  
   controlFont = MakeFont(hdc, "MS Sans Serif",8, 0, 0, 0);
   controlFontBold = MakeFont(hdc, "MS Sans Serif",8, 0, 0, 0);
 //  controlFont = MakeFont(hdc, "Prospa Control Font",7, 0, 0, 0);
   numberFont = MakeFont(hdc, "MS Sans Serif",5, 0, 0, 0); 
                               

//   ReleaseDC(editWin,hdc);
   ReleaseDC(prospaWin,hdc);
}

/***********************************************************************
*  Allow the user to define the fonts used by the program                
***********************************************************************/


int SetFonts(Interface* itfc, char args[])
{
   short nrArgs;
   short winNr;
   CText fontDst = "Control font",fontSrc = "MS Sans Serif";
   short fontSize = 8;

   if((nrArgs = ArgScan(itfc,args,1,"prospa font name, system font name, font size","eee","ttd",&fontDst,&fontSrc,&fontSize)) < 0)
      return(nrArgs); 

   HDC hdc = GetDC(prospaWin);

   if(fontDst == "Control font")
       controlFont = MakeFont(hdc, fontSrc.Str(),fontSize, 0, 0, 0);
   else if(fontDst == "CLI font")
       cliFont = MakeFont(hdc, fontSrc.Str(),fontSize, 0, 0, 0);
   else if(fontDst == "Edit font")
       editFont = MakeFont(hdc, fontSrc.Str(),fontSize, 0, 0, 0);
   else if(fontDst == "Number font")
       numberFont = MakeFont(hdc, fontSrc.Str(),fontSize, 0, 0, 0);
   else
   {
      ErrorMessage("Unknown Prospa font");
      return(ERR);
   }

   return(OK);

}

/***********************************************************************
     Ctrl-Tab moves focus to next window 
     Ctrl-Shift-Tab moves focus to previous window
***********************************************************************/

int ShowNextWindow(Interface* itfc, char args[])
{
   short nrArgs;
   short winNr;
   WinData *w,*next,*nextWin;

   if((nrArgs = ArgScan(itfc,args,1,"winNr","e","d",&winNr)) < 0)
      return(nrArgs); 

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

   next = rootWin->next;

   short nextNr = 10000;
   nextWin = NULL;

   for(w = next; w != NULL; w = w->next)
   {
      if(w->nr > winNr && w->nr < nextNr)
      {
         nextNr = w->nr;
         nextWin = w;
      }
   }

   if(nextWin == NULL)
   {
      nextNr = 10000;
      next = rootWin->next;
      for(w = next; w != NULL; w = w->next)
      {
         if(w->nr < nextNr)
         {
            nextNr = w->nr;
            nextWin = w;
         }
      }
   }

   if(nextWin != NULL)
   {
      if(!nextWin->keepInFront)
         ChangeGUIParent(nextWin->hWnd);
      ShowWindow(nextWin->hWnd,SW_HIDE);
      ShowWindow(nextWin->hWnd,SW_SHOW);
   }

	itfc->nrRetValues = 0;
   return(OK);
}


int ShowLastWindow(Interface* itfc, char args[])
{
   short nrArgs;
   short winNr;
   WinData *w,*next,*lastWin;

   if((nrArgs = ArgScan(itfc,args,1,"winNr","e","d",&winNr)) < 0)
      return(nrArgs); 

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

   short lastNr = -10000;
   next = rootWin->next;
   lastWin = NULL;
   for(w = next; w != NULL; w = w->next)
   {
      if(w->nr < winNr && w->nr > lastNr)
      {
         lastNr = w->nr;
         lastWin = w;
      }
   }

   if(lastWin == NULL)
   {
      next = rootWin->next;

      lastNr = -10000;

      for(w = next; w != NULL; w = w->next)
      {
         if(w->nr > lastNr)
         {
            lastNr = w->nr;
            lastWin = w;
         }
      }
   }

   if(lastWin != NULL)
   {
      if(!lastWin->keepInFront)
         ChangeGUIParent(lastWin->hWnd);
      ShowWindow(lastWin->hWnd,SW_HIDE);
      ShowWindow(lastWin->hWnd,SW_SHOW);
   }

	itfc->nrRetValues = 0;
   return(OK);
}


void GetNewRects(void);

void PositionWindow(HWND win, FloatRect2 *r, long pLeft,long pTop,long newWidth,long newHeight);

bool prospaResizing;

// Resize/positon all sub-windows to match prospa window
void ResizeWindowsToParent()
{
   static RECT oldr = {-999,-999,-999,-999};

   if(oldr.right != -999)
   {
      RECT pr;

   // Get the new size of the resized prospa window
      GetWindowRect(prospaWin,&pr);
      long newWidth = pr.right - pr.left;
      long newHeight = pr.bottom - pr.top;

   // Resize any gui windows that are locked in size to the main prospa window
      long xoff = resizableWinBorderSize + pr.left;
      long yoff = titleBarNMenuHeight + pr.top;
      long ww = newWidth - 2*resizableWinBorderSize;
      long wh = newHeight - (resizableWinBorderSize + titleBarNMenuHeight);

      WinData* win = rootWin->next;

      long x,y,w,h;

      while(win != NULL)
      {
         if(!win->isMainWindow && win->constrained)
         {
            if(!IsIconic(prospaWin) && IsZoomed(win->hWnd))
            {
               GetMaximisedSize(win->hWnd,x,y,w,h,false);
            }
            else
            {
               x = nint(ww*win->xSzScale + win->xSzOffset + xoff);
               y = nint(wh*win->ySzScale + win->ySzOffset + yoff);
               w = nint(ww*win->wSzScale + win->wSzOffset);
               h = nint(wh*win->hSzScale + win->hSzOffset);
            }

            if(x-xoff != 0 || y-yoff != 0 || w != 0 || h != 0)
            {
               prospaResizing = true;
               MoveWindow(win->hWnd,x,y,w,h,true);
               prospaResizing = false;
            }
         }

         win = win->next;
      }

    // Get screen dimensions **************
      //RECT scrnR;
      //SystemParametersInfo(SPI_GETWORKAREA,0,&scrnR,0);
      //long scrnWidth = scrnR.right-scrnR.left;
      //long scrnHeight = scrnR.bottom-scrnR.top;

    // Reposition prospa window
      //prospaRect.x = pr.left/(float)scrnWidth;
      //prospaRect.y = pr.top/(float)scrnHeight;
      //prospaRect.w = (pr.right-pr.left)/(float)scrnWidth;
      //prospaRect.h = (pr.bottom-pr.top)/(float)scrnHeight;
   }


   GetWindowRect(prospaWin,&oldr);

}


void PositionWindow(HWND win, FloatRect2 *r, long parentLeft,long parentTop,long parentWidth,long parentHeight)
{
   long x,y,w,h;

   parentWidth -= 2*resizableWinBorderSize;
   parentHeight -= resizableWinBorderSize + titleBarNMenuHeight;
   parentLeft += resizableWinBorderSize;
   parentTop += titleBarNMenuHeight;

// If we are maximising work out the maximised size
   if(!IsIconic(prospaWin) && IsZoomed(win))
   {
      GetMaximisedSize(win,x,y,w,h,false);
   }
   else // Otherwise normal resizing work out as a fraction of Prospawin size
   {
      // Scale to the new dimensions
      x = nint(r->x*parentWidth)+parentLeft;
      y = nint(r->y*(parentHeight))+parentTop;
      w = nint(r->w*parentWidth);
      h = nint(r->h*parentHeight);
   }

   MoveWindow(win,x,y,w,h,true);

   // If we are maximising the window then record its
   // minimised size for restore.
   if(!IsIconic(prospaWin) && IsZoomed(win))
   {
      WINDOWPLACEMENT wpl;
      long x,y,w,h;
      GetMinimisedSize(win,  x, y, w, h);
      GetWindowPlacement(win,&wpl);
      wpl.rcNormalPosition.left = x;
      wpl.rcNormalPosition.top  = y;
      wpl.rcNormalPosition.right = x+w-1;
      wpl.rcNormalPosition.bottom = y+h-1;
      SetWindowPlacement(win,&wpl);
   }
}

void CalculateWindowRect(HWND hWnd, FloatRect2 *wRect)
{
   RECT pRect,r;
   if(!IsIconic(prospaWin) && !IsZoomed(prospaWin) && !IsIconic(hWnd) && !IsZoomed(hWnd))
   {
      GetWindowRect(prospaWin,&pRect);
      long width = pRect.right - pRect.left - 2*resizableWinBorderSize;
      long height = pRect.bottom - pRect.top - (resizableWinBorderSize + titleBarNMenuHeight);
      GetWindowRect(hWnd,&r);
      wRect->x = (r.left-pRect.left-resizableWinBorderSize)/(float)width;
      wRect->y = (r.top-pRect.top-titleBarNMenuHeight)/(float)height;
      wRect->w = (r.right - r.left)/(float)width;
      wRect->h = (r.bottom - r.top)/(float)height;
   }
}

void LimitMovingRect(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   RECT *r,pRect,wRect;
   r = LPRECT(lParam);
   GetWindowRect(prospaWin,&pRect);
   GetWindowRect(hWnd,&wRect);
   long width = wRect.right - wRect.left;
   long height = wRect.bottom - wRect.top;

   if(r->top < pRect.top+titleBarNMenuHeight)
   {
      r->top = pRect.top+titleBarNMenuHeight;
      r->bottom = r->top+height;
   }

   if(r->bottom > pRect.bottom-resizableWinBorderSize)
   {
      r->bottom = pRect.bottom-resizableWinBorderSize;
      r->top = r->bottom-height;
   }

   if(r->right > pRect.right-resizableWinBorderSize)
   {
      r->right = pRect.right-resizableWinBorderSize;
      r->left = r->right-width;
   }

   if(r->left < pRect.left+resizableWinBorderSize)
   {
      r->left = pRect.left+resizableWinBorderSize;
      r->right = r->left + width;
   }
}

void LimitSizingRect(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
   RECT *r,pRect,wRect;
   r = LPRECT(lParam);
   GetWindowRect(prospaWin,&pRect);
   GetWindowRect(hWnd,&wRect);

   if(r->top < pRect.top+titleBarNMenuHeight)
   {
      r->top = pRect.top+titleBarNMenuHeight;
   }

   if(r->bottom > pRect.bottom-resizableWinBorderSize)
   {
      r->bottom = pRect.bottom-resizableWinBorderSize;
   }

   if(r->right > pRect.right-resizableWinBorderSize)
   {
      r->right = pRect.right-resizableWinBorderSize;
   }

   if(r->left < pRect.left+resizableWinBorderSize)
   {
      r->left = pRect.left+resizableWinBorderSize;
   }
}


// Return size of sub-window so it fits inside Prospa window
void GetMaximisedSize(HWND win, long &x, long &y, long &w, long &h, bool correct)
{
   RECT pRect,tRect;

   GetWindowRect(prospaWin,&pRect);
   GetWindowRect(win,&tRect);

   y = pRect.top+titleBarNMenuHeight;
   x = pRect.left+resizableWinBorderSize;
   w = pRect.right - pRect.left - 2*resizableWinBorderSize;
   h = pRect.bottom - pRect.top - (resizableWinBorderSize + titleBarNMenuHeight);

   long ww = GetSystemMetrics(SM_CXFULLSCREEN);

// This fixes the problem of maximising when the subwindow is
// more than half in a second window. 
  if(correct && ((tRect.left + tRect.right)/2 > ww))
      x = x - ww;

}

// Return the minmised size of window based on current size of prospa window
void GetMinimisedSize(HWND hWnd, long &x, long &y, long &w, long &h)
{
   RECT pr;
   

   GetWindowRect(prospaWin,&pr);
   long width = pr.right - pr.left+1;
   long height = pr.bottom - pr.top+1;
   long xoff = pr.left;
   long yoff = pr.top;
   width -= 2*resizableWinBorderSize;
   height -= resizableWinBorderSize + titleBarNMenuHeight;
   xoff += resizableWinBorderSize;
   yoff += titleBarNMenuHeight;

   FloatRect2 fr = cliRect;
	//if(hWnd == cliWin)
   //   fr = cliRect;
   //else if(hWnd == editWin)
   //   fr = editRect;
 //  else if(hWnd == plot1DWin)
 //     fr = plot1DRect;
 //  else if(hWnd == plot2DWin)
 //     fr = plot2DRect;

   x = nint(fr.x*width)+xoff;
   y = nint(fr.y*height)+yoff;
   w = nint(fr.w*width);
   h = nint(fr.h*height);

}

/*****************************************************************
   Register all window classes
*****************************************************************/

bool RegisterWindowClasses(HINSTANCE hInst)
{
   WNDCLASS wc;
   INITCOMMONCONTROLSEX initCtrls;
   HICON appIcon;

   appIcon = LoadIcon( hInst, "APPLICATION_ICON");

   initCtrls.dwSize = sizeof(INITCOMMONCONTROLSEX);
   initCtrls.dwICC = 0x0000FFFF;
   InitCommonControlsEx(&initCtrls);


   // Main gui window
   wc.lpszClassName = "MAIN_PROSPAWIN";
   wc.hInstance = hInst;
   wc.lpfnWndProc = UserWinEventsProc;
   wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   wc.hIcon = appIcon;
   wc.lpszMenuName = NULL;
   wc.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   if (!RegisterClass(&wc))
      return false;

 // 1D and 2D windows
   wc.lpszClassName    = "PLOT";
   wc.hInstance       = hInst;
   wc.lpfnWndProc      = PlotEventsProc;
   wc.hCursor         = NULL;
   wc.hIcon           = appIcon;
   wc.lpszMenuName    = NULL;
   wc.hbrBackground   = (HBRUSH)GetStockObject( WHITE_BRUSH );
   wc.style           = CS_DBLCLKS;
   wc.cbClsExtra      = 0;
   wc.cbWndExtra      = 0;
   if(!RegisterClass(&wc))
      return false;

// Divider window
   wc.lpszClassName    = "DIVIDER";
   wc.hInstance       = hInst;
   wc.lpfnWndProc      = DividerEventsProc;
   wc.hCursor         = HorizDivCursor;
   wc.hIcon           = appIcon;
   wc.lpszMenuName   = NULL;
   wc.hbrBackground   = (HBRUSH)GetStockObject( DKGRAY_BRUSH );
   wc.style            = 0;
   wc.cbClsExtra      = 0;
   wc.cbWndExtra      = 0;
   if(!RegisterClass(&wc))
      return false;

// 3D window
   wc.lpszClassName    = "OPENGL";
   wc.hInstance       = hInst;
   wc.lpfnWndProc      = Plot3DEventsProc;
   wc.hCursor         = NULL;
   wc.hIcon           = appIcon;
   wc.lpszMenuName   = "";
   wc.hbrBackground   = (HBRUSH)GetStockObject( WHITE_BRUSH );
   wc.style            = CS_OWNDC;
   wc.cbClsExtra      = 0;
   wc.cbWndExtra      = 0;
   if(!RegisterClass(&wc))
      return false;

// Other gui windows
   wc.lpszClassName    = "PROSPAWIN";
   wc.hInstance       = hInst;
   wc.lpfnWndProc      = UserWinEventsProc;
   wc.hCursor         = LoadCursor( NULL, IDC_ARROW );
   wc.hIcon           = appIcon;
   wc.lpszMenuName   = NULL;
   wc.hbrBackground   = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)); 
   wc.style            = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
   wc.cbClsExtra      = 0;
   wc.cbWndExtra      = 0;
   if(!RegisterClass(&wc))
      return false;                  
 
// Status bar overlay window for showing command syntax
   wc.lpszClassName    = "SYNTAXWIN";
   wc.hInstance       = hInst;
   wc.lpfnWndProc      = SyntaxEventsProc;
   wc.hCursor         = LoadCursor( NULL, IDC_ARROW );
   wc.hIcon           = appIcon;
   wc.lpszMenuName   = NULL;
   wc.hbrBackground   = (HBRUSH)GetStockObject( WHITE_BRUSH );
   wc.style            = CS_HREDRAW | CS_VREDRAW;
   wc.cbClsExtra      = 0;
   wc.cbWndExtra      = 0;
   if(!RegisterClass(&wc))
      return false;      

   return(true);
}

/****************************************************************************************************
   Save list to the userMenus preferences file. size is the number of values inside list
*****************************************************************************************************/

void SaveUserMenus(char **list, long size)
{
   char curFolder[MAX_PATH];

// Note the current folder
   GetCurrentDirectory(MAX_PATH,curFolder);

// Go to the user preferences folder
   if(!SetCurrentDirectory(userPrefDir))
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into preferences directory.\rIs <prospahome>\\preferences missing or write protected?");
      SetCurrentDirectory(curFolder);
      return;
   }

// Open the userMenus list for updating
   FILE *fp = fopen("userMenus.lst","w");
   if(fp)
   {
      for(int i = 0; i < size; i+=2)
      {
         fprintf(fp,"%s,%s\n",list[i],list[i+1]);
      }
      fclose(fp);                     
   }
   else
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to save menu preferences.\rIs file 'userMenus.lst' write protected?");
      SetCurrentDirectory(curFolder);
      return;
   }                  

   SetCurrentDirectory(curFolder);
}

// Switch memory leak recording on and off
int LeakTest(Interface *itfc, char *args)
{
   short nrArgs;
   CText state = "off";

   if((nrArgs = ArgScan(itfc,args,1,"on/off","e","t",&state)) < 0)
      return(nrArgs); 

 //  if(state == "on")
  //    VLDEnable();
 //  else
 //     VLDDisable();

   return(OK);
}

int GetMainWindow(Interface* itfc ,char args[])
{
   WinData *mainWin;
   if(mainWin = rootWin->FindWinByHWND(prospaWin)) // Remove tag from old one.
   {
      itfc->retVar[1].MakeAndSetFloat((float)mainWin->nr); 
      itfc->nrRetValues = 1;
      return(OK);
   }
   itfc->retVar[1].MakeAndSetFloat(-1.0); 
   itfc->nrRetValues = 1;
   return(OK);
}
  
/*******************************************************************
* Free globally allocated structures/classes
********************************************************************/

void FreeGlobals()
{
	extern DLLINFO *DLLInfo;
	extern CommandRegistry *prospaCommandRegistry;
	extern CommandRegistry *widgetCommandRegistry;
	extern PlotWinDefaults *pwd;
	extern char **licenseList;
	extern short nrLicensedItems;
	extern CText *gClassMacro, *gClassName;
	extern int gNrClassDeclarations;	 

	if(gNrClassDeclarations)
	{
		delete [] gClassMacro;
		delete [] gClassName;
	}

	delete  prospaCommandRegistry;
	delete  widgetCommandRegistry;

	procRunList->RemoveAll();
	procLoadList->RemoveAll();
	delete procRunList;
	delete procLoadList;

	delete pwd;

	delete rootWin;

	for(int i = 0; i < nrLicensedItems; i++)
	    delete [] licenseList[i];
	delete [] licenseList;



	delete [] DLLInfo;
}

/*******************************************************************
* Shut down program. By default it does this immediately
* but if the exitCode is 0 it will exit cleanly. This should
* be the preferred method if run from the Prospa interface.
********************************************************************/

int CloseApplication(Interface* itfc, char args[])
{
   short nrArgs;
   short exitCode = 1;

// Get number of number of levels and the color scheme *************
   if((nrArgs = ArgScan(itfc,args,0,"exit code","e","d",&exitCode)) < 0)
     return(nrArgs);  

// Force an exit
	if(exitCode != 0)
		exit(exitCode);

   if(SaveAllEditSessions() == IDCANCEL)
   {
      itfc->retVar[1].MakeAndSetString("cancel");
      itfc->nrRetValues = 1;
      return(OK);
   }

// Save the edit list
   SaveEditList();
   
   SaveDirectories(false);

   HtmlHelp(NULL,NULL,HH_UNINITIALIZE,cookie); 

// Destroy the main window
	if(prospaWin)
	{
		WinData* win = rootWin->FindWinByHWND(prospaWin);
		char nrStr[10];
		sprintf(nrStr,"%d",win->nr);
	   DestroyMyWindow(itfc,nrStr);
	}

   exitProgram = true;

   itfc->retVar[1].MakeAndSetString("exiting");
   itfc->nrRetValues = 1;

	return(exitCode);
}

void ShutDown()
{
	DeleteGlobalVariables();

	FreeGlobals();

	ThreadGlobals *data = (ThreadGlobals*)TlsGetValue(dwTlsIndex);
	delete data;

   DeleteCriticalSection(&cs1DPlot);
   DeleteCriticalSection(&cs2DPlot);
   DeleteCriticalSection(&csVariable);
   DeleteCriticalSection(&csAssignVariable);

}

// See if we are running under Wine

bool IsRunningInWine()
{
   HMODULE ntdllMod = GetModuleHandle("ntdll.dll");

   if (ntdllMod && GetProcAddress(ntdllMod, "wine_get_version"))
      return true;
   return false;
}
