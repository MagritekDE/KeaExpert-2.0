#include "stdafx.h"
#include "import_export_utilities.h"
#include "shlwapi.h"
#include "cArg.h"
#include "cli_files.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "evaluate.h"
#include "export_data_1d.h"
#include "export_data_2d.h"
#include "export_data_3d.h"
#include "files.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "import_data_1d.h"
#include "import_data_2d.h"
#include "import_data_3d.h"
#include "interface.h"
#include "main.h"
#include "mymath.h"
#include "plot.h"
#include "PlotFile.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "winEnableClass.h"
#include "memoryLeak.h"

void InitialiseImportExportPrompts(long type, char *rc, char *ab, char *fls,
                                   char *delimit, char *machine)
{
   if(type & XYDATA)             strcpy(rc,"xydata");
   else if(type & REAL_DATA)     strcpy(rc, "real");
   else if(type & DOUBLE_DATA)   strcpy(rc, "double");
   else if(type & COMP_DATA)     strcpy(rc,"complex");
   else if(type & X_COMP_DATA)   strcpy(rc,"xcomplex");

   if(type & ASCII)              strcpy(ab,"ascii");
   else if(type & BINARY)        strcpy(ab,"binary");

   if(type & FLOAT_32)           strcpy(fls,"float");
   else if(type & FLOAT_64)      strcpy(fls,"double");
   else if(type & INT_32)        strcpy(fls,"long");
   else if(type & INT_16)        strcpy(fls,"short");

   if(type & SPACE_DELIMIT)      strcpy(delimit,"space");
   else if(type & COMMA_DELIMIT) strcpy(delimit,"comma");
   else if(type & TAB_DELIMIT)   strcpy(delimit,"tab");
   else if(type & RETURN_DELIMIT)   strcpy(delimit,"return");

   if(type & BIG_ENDIAN)         strcpy(machine,"bigend");
   else                          strcpy(machine,"littleend");
}

short ExtractImportExportArguments(Interface *itfc, char *args, short nrArgs, 
                                  char *xyrc, char *ab, char *fls,
                                  char *delimit, char *machine, char *mode, long &header, long &rowHeader)
{
   CText  parameter;
   CText  value;
   short stype;
   CArg carg;
   Variable result;

// Check for valid number of arguments
   nrArgs = carg.Count(args);

   if(nrArgs%2 != 0)
   {
      ErrorMessage("number of arguments must be even");
      return(ERR);
   }
   
// Extract arguments (they should be supplied in pairs)   

   short i = 1;
   while(i < nrArgs)
   {
   
   // Extract parameter name
      parameter =  carg.Extract(i);
      if((stype = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&result)) < 0)
         return(ERR); 
      
      if(stype != UNQUOTED_STRING)
      {
         ErrorMessage("invalid data type for parameter '%s'",parameter.Str());
         return(ERR);
      }  
            
   // Extract parameter value
      value = carg.Extract(i+1);
      if((stype = Evaluate(itfc,RESPECT_ALIAS,value.Str(),&result)) < 0)
         return(ERR); 

   // Process different kinds of parameter    

      parameter.RemoveQuotes();
      if(parameter == "xyrc" || parameter == "rc")
      { 
         strcpy(xyrc,result.GetString());       
      }
      else if(parameter == "fls")
      { 
         strcpy(fls,result.GetString());       
      }
      else if(parameter == "delimiter")
      { 
         strcpy(delimit,result.GetString());       
      }      
      else if(parameter == "ab")
      { 
         strcpy(ab,result.GetString());       
      }
      else if(parameter == "machine")
      { 
         strcpy(machine,result.GetString());    
      }
      else if(parameter == "fileheader")
      { 
         header = nint(result.GetReal());    
      } 
      else if(parameter == "rowheader")
      { 
         rowHeader = nint(result.GetReal());    
      } 
      else if(parameter == "mode")
      { 
         strcpy(mode,result.GetString());       
      }
      else
      {
         ErrorMessage("invalid parameter; %s",parameter.Str());
         return(ERR);
      }                                            
      i+=2;
   }
   return(OK);
}

long DetermineDataType(char *xyrc, char *ab, char *fls,
                      char *delimit, char *machine, char* mode)
{
   long type;
                     
   type = 0;
   
   if(!strcmp(xyrc,"xcomplex"))     type += X_COMP_DATA;
   else if(!strcmp(xyrc,"complex")) type += COMP_DATA;
   else if (!strcmp(xyrc,"real"))   type += REAL_DATA;
   else if (!strcmp(xyrc,"double")) type += DOUBLE_DATA;
   else if(!strcmp(xyrc,"xydata"))  type += XYDATA;
   else
   {
     ErrorMessage("'%s' is an invalid type xyrc parameter",xyrc);
     return(ERR);
   }
   
        if(!strcmp(ab,"ascii"))   type += ASCII;
   else if(!strcmp(ab,"binary"))  type += BINARY;
   else
   {
     ErrorMessage("'%s' is an invalid type for ab parameter",ab);
     return(ERR);
   }  

    
        if(!strcmp(fls,"float"))  type += FLOAT_32;
   else if(!strcmp(fls,"double")) type += FLOAT_64;
   else if(!strcmp(fls,"long"))   type += INT_32;
   else if(!strcmp(fls,"short"))  type += INT_16;
   else if(!strcmp(fls,"byte"))   type += INT_8;
   else
   {
     ErrorMessage("'%s' is an invalid type for fls parameter",fls);
     return(ERR);
   } 

        if(!strcmp(delimit,"space"))   type += SPACE_DELIMIT;
   else if(!strcmp(delimit,"comma"))   type += COMMA_DELIMIT;
   else if(!strcmp(delimit,"return"))  type += RETURN_DELIMIT;
   else if(!strcmp(delimit,"tab"))     type += TAB_DELIMIT;
   else
   {
     ErrorMessage("'%s' is an invalid type for delimit parameter",delimit);
     return(ERR);
   }
       

        if(!strcmp(machine,"win"))         type += 0;
   else if(!strcmp(machine,"mac"))         type += BIG_ENDIAN;
   else if(!strcmp(machine,"bigend"))      type += BIG_ENDIAN;
   else if(!strcmp(machine,"littleend"))   type += 0;
   else
   {
     ErrorMessage("'%s' is an invalid type for machine parameter",machine);
     return(ERR);
   } 

        if(!strcmp(mode,"overwrite"))      type += OVERWRITE_MODE;
   else if(!strcmp(mode,"append"))         type += APPEND_MODE;

   return(type);
}



//// General purpose file dialog window
//
//short FileDialog(HWND hWnd, bool open, char *pathName, char *fileName, 
//                   char *title, 
//                   UINT (*hookProc)(HWND,UINT,WPARAM,LPARAM),
//                   char *hookDLG, long flags, short nr, short *index, ... )
//{   
//   static OPENFILENAME ofn;
//   extern HINSTANCE prospaInstance;
//   static char name[MAX_STR] = "";   
//   static char szFilter[1000];  // Maximum string length is 1000!
//   char temp[MAX_STR];
//   short err;
//   char extName[50];
//   char ext[10];
//   static char defaultExt[10] = "";
//   CWinEnable winEnable;
//
//
//// Extract filter expression (pairs of titles & extensions)  
//   va_list ap;
//   va_start(ap,index);
//   short j = 0;
//   for(short i = 1; i <= nr; i++)
//   {
//      strcpy(extName,va_arg(ap,char*));
//      strcpy(ext,va_arg(ap,char*));
//      if(i == 1) strcpy(defaultExt,ext);
//      sprintf(temp,"%s (*.%s)",extName,ext);
//      stradd(szFilter,temp,j);
//      stradd(szFilter,"",j);
//      sprintf(temp,"*.%s",ext);
//      stradd(szFilter,temp,j);      
//      stradd(szFilter,"",j);
//   }
//   stradd(szFilter,"",j);  // Termininating null
//   va_end(ap);  
//    
//   strcpy(name,fileName);
//   
//// Initialise data structure for dialog ******                         
//   ofn.lStructSize = sizeof(OPENFILENAME);
//   ofn.hwndOwner = hWnd;
//   ofn.hInstance = prospaInstance;
//   ofn.lpstrFilter = szFilter;
//   ofn.lpstrCustomFilter = NULL;
//   ofn.nMaxCustFilter = 0;
//   ofn.lpstrFile = name;
//   ofn.nMaxFile = _MAX_PATH;
//   ofn.lpstrFileTitle = NULL;
//   ofn.nMaxFileTitle = 0;
//   ofn.nFilterIndex = *index;
//   ofn.lpstrInitialDir = pathName;
//   ofn.lpstrTitle = title;
//   ofn.Flags = flags;
//  
//        
//   ofn.nFileOffset = 0;
//   ofn.nFileExtension = 0;
//   if(defaultExt[0] == '*')
//      ofn.lpstrDefExt = "";
//   else
//      ofn.lpstrDefExt = defaultExt;
//   ofn.lCustData = 0L;
//   ofn.lpfnHook = (LPOFNHOOKPROC)hookProc;
//   ofn.lpTemplateName = hookDLG;
//   ofn.dwReserved = 0;
//   ofn.FlagsEx = 0;
//
//// Disable all other windows ******************
//   winEnable.Disable(NULL);
//
//// Open dialog (load/save) ********************  
//   if(open)
//      err = GetOpenFileName(&ofn);
//   else
//      err = GetSaveFileName(&ofn);
//
//// Enable all other windows *******************
//   winEnable.Enable(NULL);
//
//// If successful extract path and file names ** 
//   if(err)
//   {
//      int i;
//      for(i = 0; i < ofn.nFileOffset; i++)
//         pathName[i] = name[i];
//      pathName[i] = '\0';
//      for(i = ofn.nFileOffset; i < strlen(name); i++)
//         fileName[i-ofn.nFileOffset] = name[i];
//      fileName[i-ofn.nFileOffset] = '\0';
//      *index = ofn.nFilterIndex;
//      return(OK);
//   }
//   return(ABORT);
//}  


short SaveAndSetPlaceBar(DWORD rtype[], DWORD rvalue[], char *rdata[]);
void RestorePlaceBar(short nrPlaceBarEntries, DWORD rtype[], DWORD dvalue[], char *rdata[]);

short FileDialog(HWND hWnd, bool open, char *pathName, char *fileName, 
                   char *title, 
                   UINT (*hookProc)(HWND,UINT,WPARAM,LPARAM),
                   char *hookDLG, long flags, short nr, short *index, ... )
{   
   static OPENFILENAME ofn;
   static char name[MAX_STR] = "";   
   static char szFilter[1000];  // Maximum string length is 1000!
   char temp[MAX_STR];
   short err;
   char extName[50];
   char ext[10];
   static char defaultExt[10] = "";
   CWinEnable winEnable;
   short nrPlaceBarEntries;
   unsigned long rtype[5],rvalue[5];
   char* rdata[5];

// Extract filter expression (pairs of titles & extensions)  
   va_list ap;
   va_start(ap,index);
   short j = 0;
   for(short i = 1; i <= nr; i++)
   {
      strcpy(extName,va_arg(ap,char*));
      strcpy(ext,va_arg(ap,char*));
      if(i == 1) strcpy(defaultExt,ext);
      sprintf(temp,"%s (*.%s)",extName,ext);
      stradd(szFilter,temp,j);
      stradd(szFilter,"",j);
      sprintf(temp,"*.%s",ext);
      stradd(szFilter,temp,j);      
      stradd(szFilter,"",j);
   }
   stradd(szFilter,"",j);  // Termininating null
   va_end(ap);  
    

   strcpy(name,fileName);
   
// Initialise data structure for dialog ******                         
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = hWnd;
   ofn.hInstance = prospaInstance;
   ofn.lpstrFilter = szFilter;
   ofn.lpstrCustomFilter = NULL;
   ofn.nMaxCustFilter = 0;
   ofn.lpstrFile = name;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.nFilterIndex = *index;
   ofn.lpstrInitialDir = pathName;
   ofn.lpstrTitle = title;
   ofn.Flags = flags;
//   ofn.FlagsEx = OFN_EX_NOPLACESBAR;
  
        
   ofn.nFileOffset = 0;
   ofn.nFileExtension = 0;
   if(defaultExt[0] == '*')
      ofn.lpstrDefExt = "";
   else
      ofn.lpstrDefExt = defaultExt;
   ofn.lCustData = 0L;
   ofn.lpfnHook = (LPOFNHOOKPROC)hookProc;
   ofn.lpTemplateName = hookDLG;
//   ofn.dwReserved = 0;
//   ofn.FlagsEx = 0;

// Disable all other windows ******************
   winEnable.Disable(NULL);

// Save and set-up the placebar
   nrPlaceBarEntries = SaveAndSetPlaceBar(rtype, rvalue, rdata);

// Open dialog (load/save) ********************  
   if(open)
      err = GetOpenFileName(&ofn);
   else
      err = GetSaveFileName(&ofn);

// Restore the placebar
   RestorePlaceBar(nrPlaceBarEntries,rtype, rvalue, rdata);

// Enable all other windows *******************
   winEnable.Enable(NULL);

// If successful extract path and file names ** 
   if(err)
   {
      int i;
      for(i = 0; i < ofn.nFileOffset; i++)
         pathName[i] = name[i];
      pathName[i] = '\0';
      for(i = ofn.nFileOffset; i < strlen(name); i++)
         fileName[i-ofn.nFileOffset] = name[i];
      fileName[i-ofn.nFileOffset] = '\0';
      *index = ofn.nFilterIndex;
      return(OK);
   }
   return(ABORT);
}  

short FileDialog2(HWND hWnd, bool open, char *pathName, char *fileName, 
                   char *title, 
                   UINT (*hookProc)(HWND,UINT,WPARAM,LPARAM),
                   char *hookDLG, long flags, short *index, char *filterTitles, char *filterExt)
{   
   static OPENFILENAME ofn;
   static char name[MAX_STR] = "";   
   static char szFilter[1000];  // Maximum string length is 1000!
   char temp[MAX_STR];
   short err;
   char extName[50];
   char ext[10];
   static char defaultExt[10] = "";
   CWinEnable winEnable;
   short nrPlaceBarEntries;
   unsigned long rtype[5],rvalue[5];
   char* rdata[5];
   CArg carg1,carg2;

// Extract filter expression (pairs of titles & extensions)  
   short nr1 = carg1.Count(filterTitles);
   short nr2 = carg2.Count(filterExt);

   if(nr1 != nr2)
   {
      ErrorMessage("Number of filetype titles doesn't match number filetype extensions");
      return(ERR);
   }

   short j = 0;
   for(short i = 1; i <= nr1; i++)
   {
      strcpy(extName,carg1.Extract(i));
      strcpy(ext,carg2.Extract(i));
      if(i == 1) strcpy(defaultExt,ext);
      sprintf(temp,"%s (*.%s)",extName,ext);
      stradd(szFilter,temp,j);
      stradd(szFilter,"",j);
      sprintf(temp,"*.%s",ext);
      stradd(szFilter,temp,j);      
      stradd(szFilter,"",j);
   }
   stradd(szFilter,"",j);  // Termininating null
    

   strcpy(name,fileName);
   
// Initialise data structure for dialog ******   
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = hWnd;
   ofn.hInstance = prospaInstance;
   ofn.lpstrFilter = szFilter;
   ofn.lpstrCustomFilter = NULL;
   ofn.nMaxCustFilter = 0;
   ofn.lpstrFile = name;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrFileTitle = NULL;
   ofn.nMaxFileTitle = 0;
   ofn.nFilterIndex = *index;
   

   ofn.lpstrInitialDir = pathName;
   ofn.lpstrTitle = title;
   ofn.Flags = flags;
//   ofn.FlagsEx = OFN_EX_NOPLACESBAR;
  
        
   ofn.nFileOffset = 0;
   ofn.nFileExtension = 0;
   if(defaultExt[0] == '*')
      ofn.lpstrDefExt = "";
   else
      ofn.lpstrDefExt = defaultExt;
   ofn.lCustData = 0L;
   ofn.lpfnHook = (LPOFNHOOKPROC)hookProc;
   ofn.lpTemplateName = hookDLG;
//   ofn.dwReserved = 0;
//   ofn.FlagsEx = 0;

// Disable all other windows ******************
   winEnable.Disable(NULL);

// Save and set-up the placebar
   nrPlaceBarEntries = SaveAndSetPlaceBar(rtype, rvalue, rdata);

// Open dialog (load/save) ********************  
   if(open)
      err = GetOpenFileName(&ofn);
   else
      err = GetSaveFileName(&ofn);

// Restore the placebar
   RestorePlaceBar(nrPlaceBarEntries,rtype, rvalue, rdata);

// Enable all other windows *******************
   winEnable.Enable(NULL);

// If successful extract path and file names ** 
   if(err)
   {
      int i;
      for(i = 0; i < ofn.nFileOffset; i++)
         pathName[i] = name[i];
      pathName[i] = '\0';
      for(i = ofn.nFileOffset; i < strlen(name); i++)
         fileName[i-ofn.nFileOffset] = name[i];
      fileName[i-ofn.nFileOffset] = '\0';
      *index = ofn.nFilterIndex;
      return(OK);
   }
   return(ABORT);
}  


// Default location of PlacesBar information in registry
#define KEY_COMDLG	   _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\ComDlg32")
#define KEY_PLACES_BAR	_T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\ComDlg32\\PlacesBar")

// Check to see if the places bar is to be hidden

bool IsPlacesBarHidden()
{
   static bool isPresent = false;
   static bool firstTime = true;

   HKEY hSubKey;
   DWORD rType,dwSize,rValue;

// Create the sub-key Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar

   if(firstTime) // If user changes registry key setting while Prospa is running ignore after
   {             // dialog loaded first time otherwise control placement is incorrect

	   long result = RegOpenKeyEx(HKEY_CURRENT_USER, KEY_COMDLG, 0, KEY_READ, &hSubKey);

      if(result == ERROR_SUCCESS)
      {
         if(RegQueryValueExA(hSubKey,"NoPlacesBar", NULL, &rType, NULL, &dwSize) == ERROR_SUCCESS)
         {
            if(rType == REG_DWORD)
            {
               RegQueryValueExA(hSubKey,"NoPlacesBar", NULL, &rType, (LPBYTE)&rValue, &dwSize);
               if(rValue == 1)
                  isPresent = true;
            }
         }
         else
            isPresent = false;

         RegCloseKey(hSubKey);
      }
      firstTime = false;
   }


   return(isPresent);
}



short SaveAndSetPlaceBar(DWORD rType[], DWORD rValue[], char *rData[])
{
  DWORD dwSize;
  short nrPlaceBarItems = 0;
  HKEY hSubKey;
  Interface itfc;
  CText path;

// Create the sub-key Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar
	long result = RegCreateKey(HKEY_CURRENT_USER, KEY_PLACES_BAR, &hSubKey);

// Add new entries to PlacesBars key
	if(result == ERROR_SUCCESS)
	{
      char places[MAX_PATH];
      char p[20];

      for(int k = 0; k < 5; k++)
      {
         rValue[k] = -1;
         rData[k] = NULL;
      }

// Read the old registry values
      for(int k = 0; k < 5; k++)
      {
         sprintf(p,"Place%d",k);

         if(RegQueryValueExA(hSubKey,p, NULL, &rType[k], NULL, &dwSize) == ERROR_SUCCESS)
         {      
            if(rType[k] == REG_DWORD)
            {
               RegQueryValueExA(hSubKey,p, NULL, &rType[k], (LPBYTE)&rValue[k], &dwSize);
            //   TextMessage("#%d Type = %ld, Value = %ld\n",k,rType[k],rValue[k]);
            }
            else if(rType[k] == REG_SZ)
            {
               rData[k] = new char[dwSize + 1];
               RegQueryValueExA(hSubKey,p, NULL, &rType[k], (LPBYTE)rData[k], &dwSize);
           //    TextMessage("#%d Type = %ld, Value = %s\n",k,rType[k],rData[k]);
            }
            else if(rType[k] == REG_EXPAND_SZ)
            {
               rData[k] = new char[dwSize + 1];
               RegQueryValueExA(hSubKey,p, NULL, &rType[k], (LPBYTE)rData[k], &dwSize);
           //    TextMessage("#%d Type = %ld, Value = %s\n",k,rType[k],rData[k]);
            }

            nrPlaceBarItems++;
         }
        // else
           // TextMessage("#%d Not found\n",k);
      }

// Add new registry values suitable for Prospa user

// Item #0 Desktop
      int value1 = 0x00;
		RegSetValueExA(hSubKey, "Place0", 0, REG_DWORD, (LPBYTE)&value1, sizeof(int));

// Item #1 My Computer
      int value2 = 0x11;
		RegSetValueExA(hSubKey, "Place1", 0, REG_DWORD, (LPBYTE)&value2, sizeof(int));

// Item #2 Prospa
      sprintf(places,"%s",applicationHomeDir);
		RegSetValueExA(hSubKey, "Place2", 0, REG_SZ, (LPBYTE)places, sizeof(char) * (strlen(places)+1));

// Item #3 Macro directory
      if(userMacroVar && userMacroVar->GetType() == UNQUOTED_STRING)
      {
         path = userMacroVar->GetString();
         if(ReplaceVarInString(&itfc,path) == ERR)
            return(ERR);
         strcpy(places,path.Str()); 
		   RegSetValueExA(hSubKey, "Place3", 0, REG_SZ, (LPBYTE)places, sizeof(char) * (strlen(places)+1));   
      }

// Item #4 Work directory
      if(userWorkingVar && userWorkingVar->GetType() == UNQUOTED_STRING)
      {
         path = userWorkingVar->GetString();
         if(ReplaceVarInString(&itfc,path) == ERR)
            return(ERR);
         strcpy(places,path.Str()); 
		   RegSetValueExA(hSubKey, "Place4", 0, REG_SZ, (LPBYTE)places, sizeof(char) * (strlen(places)+1));   
      }
	}

	// close the sub-key
	RegCloseKey(hSubKey);
   return(nrPlaceBarItems);
}

void RestorePlaceBar(short nrPlaceBarItems, DWORD rType[], DWORD rValue[], char *rData[])
{
   // Modify the registry keys (ref: http://www.codeguru.com/cpp/misc/misc/system/article.php/c13407)


	// create the sub-key Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar
	HKEY hSubKey;

// Create the sub-key Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar
	long result = RegCreateKey(HKEY_CURRENT_USER, KEY_PLACES_BAR, &hSubKey);

//  Add old entries to PlacesBars key if they were found

	if(result == ERROR_SUCCESS)
	{
      char p[20];

      if(nrPlaceBarItems > 0)
      {
         for(int k = 0; k <= 4; k++)
         {
            sprintf(p,"Place%d",k);

             if(rType[k] == REG_DWORD)
             { 
		          RegSetValueExA(hSubKey, p, 0, REG_DWORD, (LPBYTE)&rValue[k], sizeof(int));
             }
             else if(rType[k] == REG_SZ)
             {
		          RegSetValueExA(hSubKey, p, 0, REG_SZ, (LPBYTE)rData[k], sizeof(char) * (strlen(rData[k])+1)); 
                delete [] rData[k];
             }
             else if(rType[k] == REG_EXPAND_SZ)
             {
		          RegSetValueExA(hSubKey, p, 0, REG_EXPAND_SZ, (LPBYTE)rData[k], sizeof(char) * (strlen(rData[k])+1)); 
                delete [] rData[k];
             }
         }
         RegCloseKey(hSubKey);
      }
      else
      {
         RegCloseKey(hSubKey);
         RegDeleteKey(HKEY_CURRENT_USER,KEY_PLACES_BAR);
      }
   }
}
int CALLBACK ImportDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char fileName[MAX_STR];
      
	switch(message)
	{
		case(WM_INITDIALOG):
		{ 
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd);
	      return(false);
		}
			
		case(WM_CLOSE): // Destroy dialog and redraw plot window
		{
		   MyEndDialog(hWnd,1);
			return(true);
		}		

		case(WM_COMMAND):
		{
			switch(LOWORD(wParam))
			{
				case(ID_CANCEL): // Cancel button presses - remove window
			   {
					MyEndDialog(hWnd,1);
					return(true);
				}

				case(ID_1D_FILE_IMPORT):
				{
					MyEndDialog(hWnd,1);				
				   fileName[0] = '\0';
				   Import1DDataDialog(prospaWin,PlotFile1D::getCurrImportDataDirectory(),fileName);
					UpdateStatusWindow(Plot1D::curPlot()->win,1,"Select a region");
					Plot1D::curPlot()->ResetZoomPoint();
					return(true);
				}
				
				case(ID_2D_FILE_IMPORT):
				{		
					MyEndDialog(hWnd,1);				
				   fileName[0] = '\0';			      
				   Import2DDataDialog(prospaWin,PlotFile2D::getCurrImportDataDirectory(),fileName);
					return(true);
				}	       
				
				case(ID_3D_FILE_IMPORT):
				{
					MyEndDialog(hWnd,1);				
				   fileName[0] = '\0';			      
				   Import3DDataDialog(prospaWin,gImport3DDataDirectory,fileName);
					return(true);
				}	
	      }
	   }
   }
   return(false);
}



// Callback procedure for the Export dialog box

int CALLBACK ExportDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char fileName[MAX_STR];
      
	switch(message)
	{
		case(WM_INITDIALOG):
		{ 
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd);
	      return(false);
		}
			
		case(WM_CLOSE): // Destroy dialog and redraw plot window
		{
		   MyEndDialog(hWnd,1);
			return(true);
		}		

		case(WM_COMMAND):
		{
			switch(LOWORD(wParam))
			{
				case(ID_CANCEL): // Cancel button presses - remove window
			   {
					MyEndDialog(hWnd,1);
					return(true);
				}

			   case(ID_1D_FILE_EXPORT):
			   {
					MyEndDialog(hWnd,1);							   
			      fileName[0] = '\0';			      
			      Export1DDataDialog(prospaWin,PlotFile1D::getCurrExportDataDirectory(),fileName);
					return(true);
		      }
		      
			   case(ID_2D_FILE_EXPORT):
			   {
					MyEndDialog(hWnd,1);							   
			      fileName[0] = '\0';			      
			      Export2DDataDialog(prospaWin,PlotFile2D::getCurrExportDataDirectory(),fileName);
					return(true);
		      }
			   case(ID_3D_FILE_EXPORT):
			   {
					MyEndDialog(hWnd,1);							   
			      fileName[0] = '\0';			      
			      Export3DDataDialog(prospaWin,gExport3DDataDirectory,fileName);
					return(true);
		      }		      		      
	      }
	   }
   }
   return(false);
}
