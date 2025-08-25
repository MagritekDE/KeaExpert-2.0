#include "stdafx.h"
#include "load_save_data.h"
#include <dlgs.h>
#include "cArg.h"
#include "allocate.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "edit_files.h"
#include "error.h"
#include "files.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "import_export_utilities.h"
#include "interface.h"
#include "list_functions.h"
#include "main.h"
#include "message.h"
#include "mymath.h"
#include "plot.h"
#include "plot1dCLI.h"
#include "plot2dCLI.h"
#include "PlotFile.h"
#include "process.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "evaluate.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/*************************************************************************
*               Raw data i/o - as accessed from CLI and macros           *
*                                                                        *
* LoadDataDialog                                                         *
* SaveDataDialog                                                         *
* Load1DRealBinaryDataFromFile                                           *
* Load1DComplexBinaryDataFromFile                                        *
* Load1DRealAsciiDataFromFile                                            *
* Load1DComplexAsciiDataFromFile                                         *
* Save1DDataToBinaryFile                                                 *
* Save1DDataToAsciiFile                                                  *
* LoadData                                                               *
* SaveData                                                               *
*                                                                        *
*************************************************************************/

#define CharIsEOL(c) ((c == '\r') || (c == '\n'))

// Don't change these - they are used in prospa type file headers
#define REAL_DATA_TYPE    500
#define COMPLEX_DATA_TYPE 501
#define DOUBLE_DATA_TYPE  502
#define XY_REAL_DATA_TYPE  503
#define XY_COMPLEX_DATA_TYPE  504

#define BUFFER_SIZE 1000

long GetFileDataInfo(char[], long&, long&, long&, long&);
void SetVariableMenu(HWND hWnd, Variable *var, short index);

/************************************************************************
            Loads a data file using a modified Open File dialog          
************************************************************************/

short LoadDataDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index = 1;
   char oldDir[MAX_PATH];
   
// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);

   if(IsPlacesBarHidden())
   {
      err = FileDialog(hWnd, true, pathName, fileName, 
                       "Load data", LoadDataHookProc, "LOADDATADLG", templateFlag, 4, &index,
                       "1D data Files","1d",
                       "2D data Files","2d",
                       "3D data Files","3d",
                       "4D data Files","4d");
   }
   else
   {
      err = FileDialog(hWnd, true, pathName, fileName, 
                       "Load data", LoadDataHookProc, "LOADDATADLG2", templateFlag, 4, &index,
                       "1D data Files","1d",
                       "2D data Files","2d",
                       "3D data Files","3d",
                       "4D data Files","4d");
   }


// Update data directory
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,pathName);

// Restore the directory
   SetCurrentDirectory(oldDir);
               
   return(err);
}

/*************************************************************************** 
   Hook procedure to process events from import dialog
   (attached to standard load dialog)
***************************************************************************/

bool gDisplayLoadedData = false;

UINT LoadDataHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{   
	switch (message)
	{
		case(WM_INITDIALOG):
		{ 
	      SetWindowText(GetDlgItem(hWnd,ID_VAR_NAME),"v1");
         SendDlgItemMessage(hWnd,ID_DISPLAY_DATA,BM_SETCHECK,gDisplayLoadedData,0);
		   EnableWindow(GetDlgItem(hWnd,ID_VAR_LBL),false);
		   EnableWindow(GetDlgItem(hWnd,ID_VAR_NAME),false);
		   EnableWindow(GetDlgItem(hWnd,ID_DISPLAY_DATA),false);
	      return false;
		}

		case(WM_NOTIFY): // Process events from standard part of dialog
		{
		   LPOFNOTIFY lpon = (LPOFNOTIFY)lParam;
		   switch(lpon->hdr.code)
		   {
		      case(CDN_INITDONE): // Std dialog has finished updating so centre it in window
		      {
               PlaceDialogOnTopInCentre(hWnd,lpon->hdr.hwndFrom);              
			      break;
			   }

			   case(CDN_FILEOK): // OK button has been pressed in std dialog
			   {
               char name[MAX_PATH];
               char varName[50];
               long width,height,depth,hypers;
               CText arg,name1,name2;

			      SendMessage(lpon->hdr.hwndFrom,CDM_GETSPEC ,(WPARAM)MAX_STR,(LPARAM) (LPTSTR)name);

				   GetDlgItemText(hWnd,ID_VAR_NAME,varName,50);

			      if(GetFileDataInfo(name,width,height,depth,hypers) > -1) // Check file info and load data
			      {  
		            reportErrorToDialog = true;
                  SetCursor(LoadCursor(NULL,IDC_WAIT));
                  Interface itfc;
	               Variable var1,var2;
		            if(LoadData(&itfc, ".", name, &var1, &var2) == ERR)
		       	   {
                     SetCursor(LoadCursor(NULL,IDC_ARROW));			       	   
		               reportErrorToDialog = false;
		       	      SetWindowLong(hWnd,DWL_MSGRESULT,1);
		       	      return(1);
		       	   }

                  // Save loaded data to global variable
                  if(itfc.nrRetValues == 2) // x-y data
                  {
                     name1 = varName;
						   name1 = name1 + "_x";
						   AssignToExpression(&itfc,GLOBAL,name1.Str(),&var1,true);
                     name2 = varName;
						   name2 = name2 + "_y";
						   AssignToExpression(&itfc,GLOBAL,name2.Str(),&var2,true);
                  }
                  else
                  {
						   AssignToExpression(&itfc,GLOBAL,varName,&var1,true);
                  }

		            reportErrorToDialog = false; 

                  gDisplayLoadedData = IsDlgButtonChecked(hWnd,ID_DISPLAY_DATA);
	            	               
	               if(gDisplayLoadedData &&  IsWindowEnabled(GetDlgItem(hWnd,ID_DISPLAY_DATA)))
	               {
	                  if(height == 1)
	                  {
                        if(Plot1D::curPlot())
                        {
									Plot1D::curPlot()->setTitleText(name);
									if(itfc.nrRetValues == 2) // x-y data
									{
										arg = name1 + "," + name2;
										PlotXY(&itfc,arg.Str());
									}
									else // y data
									{
										PlotXY(&itfc,varName);
									}
                        }
	                  }
	                  else if(height > 1)
	                  {
                        if(Plot2D::curPlot())
                        {
									Plot2D::curPlot()->setTitleText(name);
	                        sprintf(name,"%s",varName);
	                        DisplayMatrixAsImage(&itfc,name);
                        }
	                  }
	               }
                  SetCursor(LoadCursor(NULL,IDC_ARROW));		               
	            }
       	      else // Display error message but don't remove dialog
       	      {
			         MessageDialog(prospaWin,MB_ICONERROR,"Error","Invalid file type");
	       	      SetWindowLong(hWnd,DWL_MSGRESULT,1);
	       	      return(1);			         
               }       	      
	         	break;
			   }
			   case(CDN_SELCHANGE): // Check for file selection change
			   {
			      char fileName[MAX_PATH];
			      char str[MAX_STR];
			      short type;
			      long width,height,depth,hyperd;
			      
			      CommDlg_OpenSave_GetFilePath(lpon->hdr.hwndFrom, fileName, MAX_PATH);
			      type = GetFileDataInfo(fileName,width,height,depth,hyperd);
			      
			      SetDlgItemText(hWnd,ID_TYPE_STR,"");
			      SetDlgItemText(hWnd,ID_XSIZE_STR,"");			      
			      SetDlgItemText(hWnd,ID_YSIZE_STR,"");
			      SetDlgItemText(hWnd,ID_ZSIZE_STR,"");
			      SetDlgItemText(hWnd,ID_QSIZE_STR,"");
				   EnableWindow(GetDlgItem(hWnd,ID_VAR_LBL),false);
				   EnableWindow(GetDlgItem(hWnd,ID_VAR_NAME),false);
				   EnableWindow(GetDlgItem(hWnd,ID_DISPLAY_DATA),false);
			      
			      switch(type)
			      {
			         case(-1):
					      SetDlgItemText(hWnd,ID_XSIZE_STR,"File can't be opened");
					      break;
				      case(-2):
                     sprintf(str,"Not a %s file",APPLICATION_NAME);
				         SetDlgItemText(hWnd,ID_XSIZE_STR,str);
					      break;
				      case(-3):
				         SetDlgItemText(hWnd,ID_XSIZE_STR,"Not a data file");
					      break;
				      case(-4):
				         SetDlgItemText(hWnd,ID_XSIZE_STR,"Unkown file version");
					      break;					      				         
				      case(-5):
				         SetDlgItemText(hWnd,ID_XSIZE_STR,"Unkown data type");
					      break;
					   default:
				         if(type ==  REAL_DATA_TYPE)
				            strcpy(str,"Type: real data");
				         else if(type == DOUBLE_DATA_TYPE)
				            strcpy(str,"Type: double precision real");
				         else if(type == COMPLEX_DATA_TYPE)
				            strcpy(str,"Type: complex data");
				         else if(type == XY_REAL_DATA_TYPE)
				            strcpy(str,"Type: x-y real data");
				         else if(type == XY_COMPLEX_DATA_TYPE)
				            strcpy(str,"Type: x-y complex data");

					      SetDlgItemText(hWnd,ID_TYPE_STR,str);
				         sprintf(str,"width : %ld",width);
					      SetDlgItemText(hWnd,ID_XSIZE_STR,str);
				         sprintf(str,"height: %ld",height);
					      SetDlgItemText(hWnd,ID_YSIZE_STR,str);
				         sprintf(str,"depth: %ld",depth);
					      SetDlgItemText(hWnd,ID_ZSIZE_STR,str);
				         sprintf(str,"hyper: %ld",hyperd);
					      SetDlgItemText(hWnd,ID_QSIZE_STR,str);

					      EnableWindow(GetDlgItem(hWnd,ID_VAR_LBL),true);
					      EnableWindow(GetDlgItem(hWnd,ID_VAR_NAME),true);
				         EnableWindow(GetDlgItem(hWnd,ID_DISPLAY_DATA),true);
					      break;			      				      
			      }			   				   			      
			   }
			   case(CDN_TYPECHANGE): // File type to be displayed has changed
			   {                     //  so modify default variable name
			      short index = lpon->lpOFN->nFilterIndex;
			      if(index == 1)
			      {
	                SetWindowText(GetDlgItem(hWnd,ID_VAR_NAME),"m1d");
		             EnableWindow(GetDlgItem(hWnd,ID_DISPLAY_DATA),true);
	            }    
			      else if(index == 2)
			      {
	                SetWindowText(GetDlgItem(hWnd,ID_VAR_NAME),"m2d");	
	                EnableWindow(GetDlgItem(hWnd,ID_DISPLAY_DATA),true);
		         }
			      else if(index == 3)
			      {
	                SetWindowText(GetDlgItem(hWnd,ID_VAR_NAME),"m3d");	
		             EnableWindow(GetDlgItem(hWnd,ID_DISPLAY_DATA),false);
	            } 
			      else if(index == 4)
			      {
	                SetWindowText(GetDlgItem(hWnd,ID_VAR_NAME),"m4d");	
		             EnableWindow(GetDlgItem(hWnd,ID_DISPLAY_DATA),false);
	            }  
			      break;			      
			   }
		   }
         break;
		}

      case(WM_DESTROY):
      {
         DialogTidyUp(hWnd,true);
         break;
      }
   }   
   return(0);
}

short SaveDataDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index;
   char oldDir[MAX_PATH];
   
// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);

   err = FileDialog(hWnd, false, pathName, fileName, 
                    "Save data", SaveDataHookProc, "SAVEDATADLG", templateFlag, 4, &index,
                    "1D data Files","1d",
                    "2D data Files","2d",
                    "3D data Files","3d",
                    "4D data Files","4d");
                    
// Make sure returned path has standard form
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,pathName); 

// Restore the directory
   SetCurrentDirectory(oldDir);
   
   return(err);
}

/*************************************************************************** 
   Hook procedure to process save data events
***************************************************************************/

UINT SaveDataHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   static short index = 1;
   
	switch (message)
	{
		case(WM_INITDIALOG):
		{ 
	      Variable *var = &globalVariable;
         SetVariableMenu(hWnd,var,index);
         SendDlgItemMessage(hWnd, ID_X_VAR_LIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
         SendDlgItemMessage(hWnd, ID_Y_VAR_LIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		   SendDlgItemMessage(hWnd,ID_USE_XY,BM_SETCHECK,(WPARAM)1,0);

	      return false;
		}

		case(WM_NOTIFY): // Process events from standard part of dialog
		{
		   LPOFNOTIFY lpon = (LPOFNOTIFY)lParam;
		   switch(lpon->hdr.code)
		   {
		      case(CDN_INITDONE): // Std dialog has finished updating so centre it in window
		      {
               PlaceDialogOnTopInCentre(hWnd,lpon->hdr.hwndFrom);            
			      break;
			   }
			   case(CDN_FILEOK): // OK button has been pressed in the std dialog
			   {
               char fileName[MAX_PATH];
               char varNameX[50];
               char varNameY[50];
               char ext[10];

			      index = lpon->lpOFN->nFilterIndex;

			      SendMessage(lpon->hdr.hwndFrom,CDM_GETSPEC ,(WPARAM)MAX_STR,(LPARAM) (LPTSTR)fileName);
			      strcpy(ext,GetExtension(fileName));
			      if(ext[0] == '\0')
			      {
			         if(index == 1)
			            strcat(fileName,".1d");
			        	else if(index == 2)
			            strcat(fileName,".2d");
			        	else if(index == 3)
			            strcat(fileName,".3d");	
			        	else if(index == 4)
			            strcat(fileName,".4d");	
			      }
			      
				   GetDlgItemText(hWnd,ID_X_VAR_LIST,varNameX,50);
				   GetDlgItemText(hWnd,ID_Y_VAR_LIST,varNameY,50);
			   //   GetCurrentDirectory(MAX_PATH,gCurrentDir); 
			
			  // Attempt to save the data to current directory
	            reportErrorToDialog = true;
               short varType;

					if(index == 1 && SendDlgItemMessage(hWnd, ID_USE_XY, BM_GETCHECK, 0, 0)) // Save 1D data
					{
						Variable *varX = globalVariable.Get(ALL_VAR | DO_NOT_RESOLVE,varNameX,varType);
						Variable *varY = globalVariable.Get(ALL_VAR | DO_NOT_RESOLVE,varNameY,varType);
						if(!varX || !varY)
						{
							ErrorMessage("Variable '%s' or '%s' is not defined",varNameX,varNameY);
							reportErrorToDialog = false;
	       				SetWindowLong(hWnd,DWL_MSGRESULT,1);
	       				return(1);
						}
	       			if(SaveData(hWnd, ".", fileName, varX, varY) == ERR)
	       			{
							reportErrorToDialog = false;
	       				SetWindowLong(hWnd,DWL_MSGRESULT,1);
	       				return(1);
	       			}
					}
					else
					{
						Variable *varY = globalVariable.Get(ALL_VAR | DO_NOT_RESOLVE,varNameY,varType);
						if(!varY)
						{
							ErrorMessage("Variable '%s' is not defined",varNameY);
							reportErrorToDialog = false;
	       				SetWindowLong(hWnd,DWL_MSGRESULT,1);
	       				return(1);
						}
	       			if(SaveData(hWnd, ".", fileName, varY) == ERR)
	       			{
							reportErrorToDialog = false;
	       				SetWindowLong(hWnd,DWL_MSGRESULT,1);
	       				return(1);
	       			}
					}
	            reportErrorToDialog = false;              
	         	break;
			   }
			   case(CDN_TYPECHANGE): // File type to be displayed has changed
			   {                     //  so modify default variable name
               short indexbak = index;
			      index = lpon->lpOFN->nFilterIndex;
				   Variable *var = &globalVariable;
               SetVariableMenu(hWnd,var,index);
               SendDlgItemMessage(hWnd, ID_X_VAR_LIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
               SendDlgItemMessage(hWnd, ID_Y_VAR_LIST, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
					if(index == 1)
					{
						 EnableWindow(GetDlgItem(hWnd,ID_X_VAR_LIST),true);
						 EnableWindow(GetDlgItem(hWnd,ID_X_VAR_STEXT),true);
						 SetWindowText(GetDlgItem(hWnd,ID_X_VAR_STEXT),"x variable:");
						 SetWindowText(GetDlgItem(hWnd,ID_Y_VAR_STEXT),"y variable:");
						 EnableWindow(GetDlgItem(hWnd,ID_USE_XY_STEXT),true);
						 EnableWindow(GetDlgItem(hWnd,ID_USE_XY),true);
					}
					else
					{
						 EnableWindow(GetDlgItem(hWnd,ID_X_VAR_LIST),false);
						 EnableWindow(GetDlgItem(hWnd,ID_X_VAR_STEXT),false);
						 SetWindowText(GetDlgItem(hWnd,ID_X_VAR_STEXT),"");
						 SetWindowText(GetDlgItem(hWnd,ID_Y_VAR_STEXT),"matrix:");
						 EnableWindow(GetDlgItem(hWnd,ID_USE_XY_STEXT),false);
						 EnableWindow(GetDlgItem(hWnd,ID_USE_XY),false);
					}

               index = indexbak; // Only modify index if ok pressed so restore it here.
	            break;
	         }
			}
		}
		case(WM_COMMAND): // Make sure the correct controls are enabled
		{
		   switch(LOWORD(wParam))
		   {
	       	case(ID_USE_XY):
				{
					bool useXY = SendDlgItemMessage(hWnd, ID_USE_XY, BM_GETCHECK, 0, 0);
					if(useXY)
					{

						 EnableWindow(GetDlgItem(hWnd,ID_X_VAR_LIST),true);
						 EnableWindow(GetDlgItem(hWnd,ID_X_VAR_STEXT),true);
					}
					else
					{
						 EnableWindow(GetDlgItem(hWnd,ID_X_VAR_LIST),false);
						 EnableWindow(GetDlgItem(hWnd,ID_X_VAR_STEXT),false);
					}
				}
	      }
		   break;
		}
		
      case(WM_DESTROY):
      {
         DialogTidyUp(hWnd,true);
         break;
      }
   }   
   return(0);
}

/*********************************************************************
   Fill in the variable menu based on the current data type (1/2/3/4)
*********************************************************************/

void SetVariableMenu(HWND hWnd, Variable *var, short index)
{
   short type;
	short n;

// Delete old lists
	do
		n = SendDlgItemMessage(hWnd,ID_X_VAR_LIST, CB_DELETESTRING, 0, 0);
	while(n > 0);	
	do
		n = SendDlgItemMessage(hWnd,ID_Y_VAR_LIST, CB_DELETESTRING, 0, 0);
	while(n > 0);	
// Make a new one
   if(index == 1) // 1D
   {
	   while(var)
	   {
		   var = var->GetNext(type);
		   if(!var) break;
         if(!var->GetVisible()) continue;
		   if(type == DMATRIX2D || type == MATRIX2D || type == CMATRIX2D)
		   {
			   if(var->GetDimX() == 1 || var->GetDimY() == 1)
			   {
			      SetDlgItemText(hWnd,ID_X_VAR_LIST,var->GetName());
			      SendDlgItemMessage(hWnd,ID_X_VAR_LIST,CB_ADDSTRING,0,(LPARAM) var->GetName());
			      SetDlgItemText(hWnd,ID_Y_VAR_LIST,var->GetName());
			      SendDlgItemMessage(hWnd,ID_Y_VAR_LIST,CB_ADDSTRING,0,(LPARAM) var->GetName());
			   }
		   }
	   }
   }			      
   else if(index == 2) // 2D
   {
	   while(var)
	   {
		   var = var->GetNext(type);
		   if(!var) break;
         if(!var->GetVisible()) continue;
		   if(type == DMATRIX2D || type == MATRIX2D || type == CMATRIX2D)
		   {
			   if(var->GetDimX() > 1 && var->GetDimY() > 1)
			   {
			      SetDlgItemText(hWnd,ID_Y_VAR_LIST,var->GetName());
			      SendDlgItemMessage(hWnd,ID_Y_VAR_LIST,CB_ADDSTRING,0,(LPARAM) var->GetName());
			   }
		   }
	   }			      
   }  	
   else if(index == 3) // 3D
   {
	   while(var)
	   {
		   var = var->GetNext(type);
		   if(!var) break;
         if(!var->GetVisible()) continue;
		   if(type == MATRIX3D || type == CMATRIX3D)
		   {
			   SetDlgItemText(hWnd,ID_Y_VAR_LIST,var->GetName());
			   SendDlgItemMessage(hWnd,ID_Y_VAR_LIST,CB_ADDSTRING,0,(LPARAM) var->GetName());
		   }
	   }			      
   }
   else if(index == 4) // 4D
   {
	   while(var)
	   {
		   var = var->GetNext(type);
		   if(!var) break;
         if(!var->GetVisible()) continue;
		   if(type == MATRIX4D || type == CMATRIX4D)
		   {
			   SetDlgItemText(hWnd,ID_Y_VAR_LIST,var->GetName());
			   SendDlgItemMessage(hWnd,ID_Y_VAR_LIST,CB_ADDSTRING,0,(LPARAM) var->GetName());
		   }
	   }			      
   }
}

/************************************************************************
       Sets the current working folder - called from CLI or Macro        
************************************************************************/

UINT SetFolderHookProc(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam);

int SetCurrentFolder(Interface *itfc, char arg[])
{
   short err;
   long flag = (OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_NONETWORKBUTTON | OFN_NOVALIDATE);
   char fileName[MAX_STR];
   static short index = 0;
   
   strcpy(fileName,"Move into desired directory, then press open");
      
   err = FileDialog(prospaWin, true, gCurrentDir, fileName, 
                    "Set Current Directory", SetFolderHookProc,
                    "", flag, 0, &index);
      
   if(err == ABORT)
   {
	  itfc->retVar[1].MakeAndSetString("cancel");
   }
   else
   {
  // Make sure the directory name is in standard format
      GetCurrentDirectory(MAX_PATH,gCurrentDir);
	  itfc->retVar[1].MakeAndSetString(gCurrentDir);
   }
   itfc->nrRetValues = 1;
	   	                             
   return(0);
}


/************************************************************************
       Sets the current working folder - called from the main menu.      
************************************************************************/

short SetFolderDialog(HWND hWnd)
{
   short err;
   long flag = (OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_NONETWORKBUTTON | OFN_NOVALIDATE);
   char fileName[MAX_STR];
   static short index = 0;
   
   strcpy(fileName,"Move into desired folder then press open");
   
   err = FileDialog(hWnd, true, gCurrentDir, fileName, 
                    "Set Current Folder", SetFolderHookProc, "", flag, 0, &index);
	
// Make sure the directory name is in standard format
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,gCurrentDir);

   return(err);
}

/************************************************************************
                        Select a folder name                             
************************************************************************/

//TODO - convert str to CText in FileDialog
int GetFolderName(Interface* itfc ,char args[])
{
   short err,r;
   long flag = (OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_NONETWORKBUTTON | OFN_NOVALIDATE);
   char fileName[MAX_PATH];
   char folder[MAX_PATH];
   static short index = 0;
   CText oldDir;
   CText title = "Select Folder";
   WinData* oldGUI;

// Save the current working directory as the following command changes it
   GetDirectory(oldDir);

   if((r = ArgScan(itfc,args,0,"[[folder],title]","ee","st",folder,&title)) < 0)
      return(r);

   oldGUI = GetGUIWin();  

   strcpy(fileName,"Move into desired folder then press open");
   if(r == 0)
      strcpy(folder,oldDir.Str());
   
   err = FileDialog(prospaWin, true, folder, fileName, 
                    title.Str(), SetFolderHookProc, "", flag, 0, &index);
   
   if(err == ABORT)
      itfc->retVar[1].MakeAndSetString("cancel");
	else
   {
      folder[strlen(folder)-1] = '\0'; // Remove final '\' to be consistent with pwd and getcwd
	   itfc->retVar[1].MakeAndSetString(folder);
   }

   itfc->nrRetValues = 1;
	   
// Restore the directory
   SetDirectory(oldDir);

   WinData::SetGUIWin(oldGUI);   	                             
   return(0);
}

/*************************************************************************
   Hook procedure to process events from set folder dialog
   (attached to standard load dialog)
*************************************************************************/

UINT SetFolderHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
	{
		case(WM_INITDIALOG): // Hide a few controls and change OK label
		{ 
		   HWND hdlg = GetParent(hWnd);
		   CommDlg_OpenSave_HideControl(hdlg,edt1);
		   CommDlg_OpenSave_HideControl(hdlg,stc3);
		   CommDlg_OpenSave_HideControl(hdlg,cmb1);
		   CommDlg_OpenSave_HideControl(hdlg,stc2);
         CommDlg_OpenSave_SetControlText(hdlg,IDOK,"Select folder");
	      return false;
		}

		case(WM_NOTIFY): // Process events from standard part of dialog
		{
		   LPOFNOTIFY lpon = (LPOFNOTIFY)lParam;
		   
		   switch(lpon->hdr.code)
		   {
		      case(CDN_INITDONE): // Std dialog has finished updating so centre it in window
		      {
               PlaceDialogOnTopInCentre(hWnd,lpon->hdr.hwndFrom); 
			      break;
			   }		   			   
			}
         break;
		}
		
      case(WM_DESTROY):
      {
         DialogTidyUp(hWnd,true);
         break;
      }
   }   
   return(0);
}


/************************************************************************
             Return the dimensions of a Prospa data file  

   syntax: (w,h,d,type) = getfilesize

   If file is not a Prospa file then return zeros.
************************************************************************/

int GetFileDimensions(Interface* itfc ,char args[])
{
   long width,height,depth,hypers;
   CText fileName;
   short nrArgs;
   short type;
   
   if((nrArgs = ArgScan(itfc,args,1,"file name","e","t",&fileName)) < 0)
    return(nrArgs);

   if((type = GetFileDataInfo(fileName.Str(),width,height,depth,hypers)) > -1)
   {
      itfc->retVar[1].MakeAndSetFloat(width);   
	   itfc->retVar[2].MakeAndSetFloat(height);   
	   itfc->retVar[3].MakeAndSetFloat(depth); 
	   itfc->retVar[4].MakeAndSetFloat(hypers); 
      if(type == REAL_DATA_TYPE)
	      itfc->retVar[5].MakeAndSetString("real");  
      else if(type == COMPLEX_DATA_TYPE)
   	   itfc->retVar[5].MakeAndSetString("complex");   
      else if(type == DOUBLE_DATA_TYPE)
   	   itfc->retVar[5].MakeAndSetString("double");  
      else if(type == XY_REAL_DATA_TYPE)
   	   itfc->retVar[5].MakeAndSetString("xyreal");  
      else if(type == XY_COMPLEX_DATA_TYPE)
   	   itfc->retVar[5].MakeAndSetString("xycomplex");  
      itfc->nrRetValues = 5; 
	   return(OK);
   }
   else
   {
      if(itfc->inCLI)
      {
         switch(type)
         {
	         case(-1):
		         ErrorMessage("File can't be opened");
		         break;
	         case(-2):
	            ErrorMessage("Not a %s data file",APPLICATION_NAME);
		         break;
	         case(-3):
	            ErrorMessage("Not a data file");
		         break;
	         case(-4):
	            ErrorMessage("Unkown file version");
		         break;					      				         
	         case(-5):
	            ErrorMessage("Unkown data type");
		         break;
         }
         return(ERR);
      }
      else // In a macro
      {
         switch(type)
         {
	         case(-1):
		         ErrorMessage("File can't be opened");
		         break;
	         case(-2):
	         case(-3):
	         case(-4):					      				         
	         case(-5):
               itfc->retVar[1].MakeAndSetFloat(0);   
	            itfc->retVar[2].MakeAndSetFloat(0);   
	            itfc->retVar[3].MakeAndSetFloat(0);  
	            itfc->retVar[4].MakeAndSetString("");  
               itfc->nrRetValues = 4; 
	            return(OK);
         }
         return(ERR);
      }

   }
}


/************************************************************************
     Extract some information from a data file 
     (Core routine for GetFileDimensions)
************************************************************************/

long GetFileDataInfo(char file[], long &width, long &height, long &depth, long &hypers)
{
   FILE *fp;
   long owner;
   long type;
   long version;
   long dataType;
   
// Open file for binary read *******************
   if(!(fp = fopen(file,"rb")))
	{
//	   ErrorMessage("file '%s' not found",file);
	   return(ERR);
	}

// Read file owner *****************************	
	fread(&owner,sizeof(long),1,fp);
   if(owner != 'PROS')
   {
//	   ErrorMessage("file '%s' has unknown owner",file);
      fclose(fp);      
	   return(-2);   
   }
   
// Make sure its a data file ********************
   fread(&type,sizeof(long),1,fp);
   if(type != 'DATA')
   {
//	   ErrorMessage("file '%s' has unknown type",file);
      fclose(fp);
	   return(-3);   
   }
   
// Check file version number *********************
   fread(&version,sizeof(long),1,fp); 
   if(version != PLOTFILE_VERSION_1_0 && 
		version != PLOTFILE_VERSION_1_1)
   {
//	   ErrorMessage("file '%s' has unknown version",file);   
      fclose(fp);   
	   return(-4);   
   } 
	
// Check out data type ***************************
   fread(&dataType,sizeof(long),1,fp); 

   if(dataType != REAL_DATA_TYPE && 
      dataType != DOUBLE_DATA_TYPE && 
      dataType != COMPLEX_DATA_TYPE &&
		dataType != XY_REAL_DATA_TYPE  &&
      dataType != XY_COMPLEX_DATA_TYPE)
   {
//	   ErrorMessage("file '%s' has unknown data type",file);   
       fclose(fp);   
	   return(-5);   
   }

// Get data size **********************************
   fread(&width,sizeof(long),1,fp); 
   fread(&height,sizeof(long),1,fp); 
   fread(&depth,sizeof(long),1,fp); 
   if(version == PLOTFILE_VERSION_1_1)
      fread(&hypers,sizeof(long),1,fp);
   else
      hypers = 1; 

// Close file and return data type ****************
   fclose(fp);
   return(dataType);
}

/*************************************************************************
* Load 1D real binary data from a file and store in arrays "x" and "y"   *
* The data can be x,y data or y data - the procedure is informed via the *
* type parameter (REAL_DATA or XYDATA).                                  *
* The binary data can be in 32 bit float, 32 bit integer format or       *
* 16 bit int format.                                                     *
* XY data is stored as an array of X data and then an array of Y data    *
* Checks for file open error and invalid file format                     *
* Returns OK if completed or ERR is error occured                        *
* length is an optional parameter which requires that not all the data   *
* will be loaded.                                                        *
*************************************************************************/

long Load1DRealBinaryDataFromFile(char fileName[],  long type, float **x, float **y, long header, long length)
{
   FILE *fp;
   char *temp = NULL;
   long fileLength,nrPoints = 0;

   // Open this file
   if(!(fp = fopen(fileName,"rb")))
   {
      ErrorMessage("File '%s' can't be opened",fileName);
      return(ERR);
   }

   // Figure out how much data there is
   fileLength = GetFileLength(fp);

   // Skip over file header (if any)
   if(header > 0)
   {
      fseek(fp,header,SEEK_CUR);
      fileLength -= header;
   }

   // Option to read only part of the data in
   if(length > 0 && length < fileLength)
   {
      fileLength = length; 
   }

   // Check for invalid header length  
   if(fileLength <= 0)
   {
      ErrorMessage("header length larger than file");
      fclose(fp);
      return(ERR);
   }

   // Allocate temporary space and read all data into it
   if(!(temp = new char[fileLength]))
   {
      ErrorMessage("Can't allocate memory for 1D temporary data");
      fclose(fp);
      return(ERR);
   } 
   if(fread(temp,sizeof(char),fileLength,fp) != fileLength)
   {
      delete [] temp;
      ErrorMessage("Can't read all %ld bytes in file '%s'",fileLength,fileName);
      fclose(fp);      
      return(ERR);
   }

   // If Big Endian fix up the order before doing anything else
   if(type & BIG_ENDIAN) 
   {
      if(type & FLOAT_32 || type & INT_32) // Reverse byte order for 32 bit data
      {   
         for(long i = 0; i < fileLength; i+=4)
         {
            unsigned char byte3 = temp[i];
            unsigned char byte2 = temp[i+1];
            unsigned char byte1 = temp[i+2];
            unsigned char byte0 = temp[i+3];

            temp[i]   = byte0;
            temp[i+1] = byte1;
            temp[i+2] = byte2;
            temp[i+3] = byte3;
         }
      }
      else if(type & FLOAT_64) // Reverse byte order for 64 bit data
      {   
         for(long i = 0; i < fileLength; i+=8)
         {
            unsigned char byte7 = temp[i];
            unsigned char byte6 = temp[i+1];
            unsigned char byte5 = temp[i+2];
            unsigned char byte4 = temp[i+3];
            unsigned char byte3 = temp[i+4];
            unsigned char byte2 = temp[i+5];
            unsigned char byte1 = temp[i+6];
            unsigned char byte0 = temp[i+7];

            temp[i]   = byte0;
            temp[i+1] = byte1;
            temp[i+2] = byte2;
            temp[i+3] = byte3;
            temp[i+4] = byte4;
            temp[i+5] = byte5;
            temp[i+6] = byte6;
            temp[i+7] = byte7;
         }
      }
      else if(type & INT_16) // Reverse byte order for 16 bit data
      {   
         for(long i = 0; i < fileLength; i+=2)
         {
            unsigned char byte1 = temp[i];
            unsigned char byte0 = temp[i+1];

            temp[i]   = byte0;
            temp[i+1] = byte1;
         }
      } 
   }

   // Figure out the correct number of data points based on data type passed   
   if(type & FLOAT_32)
      nrPoints = fileLength/(sizeof(float));
   else if(type & FLOAT_64)
      nrPoints = fileLength/(sizeof(double));
   else if(type & INT_32)
      nrPoints = fileLength/(sizeof(long));
   else if(type & INT_16)
      nrPoints = fileLength/(sizeof(short));
   else if(type & INT_8)
      nrPoints = fileLength/(sizeof(UCHAR));

   // Modify for XYDATA and COMP_DATA
   if((type & XYDATA) || (type & COMP_DATA))
      nrPoints /= 2;

   // Allocate space for x data
   (*x) = MakeVectorNR(0L,nrPoints-1);
   if(!(*x))
   {
      delete [] temp;
      ErrorMessage("Can't allocate memory for 1D x data");
      fclose(fp);      
      return(ERR);
   } 

   // Allocate y data
   (*y) = MakeVectorNR(0L,nrPoints-1);
   if(!(*y))
   {
      delete [] temp;
      FreeVector(*x);
      ErrorMessage("Can't allocate memory for 1D y data");
      fclose(fp);      
      return(ERR);
   }	

   // Convert data to appropriate format 
   if(type & FLOAT_32) // Read in float data
   {   
      if(type & XYDATA)
      {
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = ((float*)temp)[i];
         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((float*)temp)[i+nrPoints]; 	   
      }
      else if(type & REAL_DATA)
      {
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = i; 

         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((float*)temp)[i];
      }
   }
   else if(type & FLOAT_64) // Read in double data
   {   
      if(type & XYDATA)
      {
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = ((double*)temp)[i];
         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((double*)temp)[i+nrPoints]; 	   
      }
      else if(type & REAL_DATA)
      {
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = i; 

         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((double*)temp)[i];
      }
   }
   else if(type & INT_32) // Read in long integer data
   {
      if(type & XYDATA)
      {
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = ((long*)temp)[i];  
         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((long*)temp)[i+nrPoints];  
      }
      else if(type & REAL_DATA) // Y data only, so add index as x array
      {
         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((long*)temp)[i];
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = i;      
      }
   }
   else if(type & INT_16) // Read in short integer data
   {
      if(type & XYDATA)
      {
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = ((short*)temp)[i];  
         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((short*)temp)[i+nrPoints];  
      }
      else if(type & REAL_DATA) // Y data only, so add index as x array
      {
         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((short*)temp)[i];
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = i;      
      }
   }
   else if(type & INT_8) // Read in byte integer data
   {
      if(type & REAL_DATA) // Y data only, so add index as x array
      {
         for(long i = 0; i < nrPoints; i++)
            (*y)[i] = ((UCHAR*)temp)[i];
         for(long i = 0; i < nrPoints; i++)
            (*x)[i] = i;      
      }
   }

   delete [] temp;
   fclose(fp);

   return(nrPoints);  
}


/*************************************************************************
* Load 1D double binary data from a file and store in arrays "x" and "y" *
* The data can be x,y data or y data - the procedure is informed via the *
* type parameter (DOUBLE_DATA).                                          *
* The binary data can be in 64/32 bit float, 32 bit integer format or    *
* 16 bit int format.                                                     *
* XY data is stored as an array of X data and then an array of Y data    *
* Checks for file open error and invalid file format                     *
* Returns OK if completed or ERR is error occured                        *
* length is an optional parameter which requires that not all the data   *
* will be loaded.                                                        *
*************************************************************************/

long Load1DDoubleBinaryDataFromFile(char fileName[], long type, double** x, double** y, long header, long length)
{
	FILE* fp;
	char* temp = NULL;
	long fileLength, nrPoints = 0;

	// Open this file
	if (!(fp = fopen(fileName, "rb")))
	{
		ErrorMessage("File '%s' can't be opened", fileName);
		return(ERR);
	}

	// Figure out how much data there is
	fileLength = GetFileLength(fp);

	// Skip over file header (if any)
	if (header > 0)
	{
		fseek(fp, header, SEEK_CUR);
		fileLength -= header;
	}

	// Option to read only part of the data in
	if (length > 0 && length < fileLength)
	{
		fileLength = length;
	}

	// Check for invalid header length  
	if (fileLength <= 0)
	{
		ErrorMessage("header length larger than file");
		fclose(fp);
		return(ERR);
	}

	// Allocate temporary space and read all data into it
	if (!(temp = new char[fileLength]))
	{
		ErrorMessage("Can't allocate memory for 1D temporary data");
		fclose(fp);
		return(ERR);
	}
	if (fread(temp, sizeof(char), fileLength, fp) != fileLength)
	{
		delete[] temp;
		ErrorMessage("Can't read all %ld bytes in file '%s'", fileLength, fileName);
		fclose(fp);
		return(ERR);
	}

	// If Big Endian fix up the order before doing anything else
	if (type & BIG_ENDIAN)
	{
		if (type & FLOAT_32 || type & INT_32) // Reverse byte order for 32 bit data
		{
			for (long i = 0; i < fileLength; i += 4)
			{
				unsigned char byte3 = temp[i];
				unsigned char byte2 = temp[i + 1];
				unsigned char byte1 = temp[i + 2];
				unsigned char byte0 = temp[i + 3];

				temp[i] = byte0;
				temp[i + 1] = byte1;
				temp[i + 2] = byte2;
				temp[i + 3] = byte3;
			}
		}
		else if (type & FLOAT_64) // Reverse byte order for 64 bit data
		{
			for (long i = 0; i < fileLength; i += 8)
			{
				unsigned char byte7 = temp[i];
				unsigned char byte6 = temp[i + 1];
				unsigned char byte5 = temp[i + 2];
				unsigned char byte4 = temp[i + 3];
				unsigned char byte3 = temp[i + 4];
				unsigned char byte2 = temp[i + 5];
				unsigned char byte1 = temp[i + 6];
				unsigned char byte0 = temp[i + 7];

				temp[i] = byte0;
				temp[i + 1] = byte1;
				temp[i + 2] = byte2;
				temp[i + 3] = byte3;
				temp[i + 4] = byte4;
				temp[i + 5] = byte5;
				temp[i + 6] = byte6;
				temp[i + 7] = byte7;
			}
		}
		else if (type & INT_16) // Reverse byte order for 16 bit data
		{
			for (long i = 0; i < fileLength; i += 2)
			{
				unsigned char byte1 = temp[i];
				unsigned char byte0 = temp[i + 1];

				temp[i] = byte0;
				temp[i + 1] = byte1;
			}
		}
	}

	// Figure out the correct number of data points based on data type passed   
	if (type & FLOAT_32)
		nrPoints = fileLength / (sizeof(float));
	else if (type & FLOAT_64)
		nrPoints = fileLength / (sizeof(double));
	else if (type & INT_32)
		nrPoints = fileLength / (sizeof(long));
	else if (type & INT_16)
		nrPoints = fileLength / (sizeof(short));
	else if (type & INT_8)
		nrPoints = fileLength / (sizeof(UCHAR));

	// Modify for XYDATA and COMP_DATA
	if ((type & XYDATA) || (type & COMP_DATA))
		nrPoints /= 2;

	// Allocate space for x data
	(*x) = MakeDVectorNR(0L, nrPoints - 1);
	if (!(*x))
	{
		delete[] temp;
		ErrorMessage("Can't allocate memory for 1D x data");
		fclose(fp);
		return(ERR);
	}

	// Allocate y data
	(*y) = MakeDVectorNR(0L, nrPoints - 1);
	if (!(*y))
	{
		delete[] temp;
		FreeDVector(*x);
		ErrorMessage("Can't allocate memory for 1D y data");
		fclose(fp);
		return(ERR);
	}

	// Convert data to appropriate format 
	if (type & FLOAT_32) // Read in float data
	{
		if (type & XYDATA)
		{
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = ((float*)temp)[i];
			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((float*)temp)[i + nrPoints];
		}
		else if (type & DOUBLE_DATA)
		{
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = i;

			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((float*)temp)[i];
		}
	}
	else if (type & FLOAT_64) // Read in double data
	{
		if (type & XYDATA)
		{
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = ((double*)temp)[i];
			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((double*)temp)[i + nrPoints];
		}
		else if (type & DOUBLE_DATA)
		{
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = i;

			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((double*)temp)[i];
		}
	}
	else if (type & INT_32) // Read in long integer data
	{
		if (type & XYDATA)
		{
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = ((long*)temp)[i];
			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((long*)temp)[i + nrPoints];
		}
		else if (type & DOUBLE_DATA) // Y data only, so add index as x array
		{
			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((long*)temp)[i];
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = i;
		}
	}
	else if (type & INT_16) // Read in short integer data
	{
		if (type & XYDATA)
		{
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = ((short*)temp)[i];
			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((short*)temp)[i + nrPoints];
		}
		else if (type & DOUBLE_DATA) // Y data only, so add index as x array
		{
			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((short*)temp)[i];
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = i;
		}
	}
	else if (type & INT_8) // Read in byte integer data
	{
		if (type & DOUBLE_DATA) // Y data only, so add index as x array
		{
			for (long i = 0; i < nrPoints; i++)
				(*y)[i] = ((UCHAR*)temp)[i];
			for (long i = 0; i < nrPoints; i++)
				(*x)[i] = i;
		}
	}

	delete[] temp;
	fclose(fp);

	return(nrPoints);
}

/*************************************************************************
* Load 1D binary data from a file and store in data arrays               *
* The data must be complex data                                          *
* The binary data can be : 32 bit float, 32 bit integer format or        *
* 16 bit int format.                                                     *
* Checks for file open error and invalid file format                     *
* Also skips over header if header size > 0                              *
* Returns OK if completed or ERR is error occured                        *
*************************************************************************/

long Load1DComplexBinaryDataFromFile(char fileName[],  long type, complex **data, long header, long length)
{
   FILE *fp;
   char *temp = NULL;
   long fileLength,nrPoints;
   long i;

   // Open this file
   if(!(fp = fopen(fileName,"rb")))
   {
      ErrorMessage("File '%s' can't be opened",fileName);
      return(ERR);
   }

   // Figure out how much data there is
   fileLength = GetFileLength(fp);

   // Skip over file header (if any)
   if(header > 0)
   {
      fseek(fp,header,SEEK_CUR);
      fileLength -= header;
   }

   // Option to read only part of the data in
   if(length > 0 && length < fileLength)
   {
      fileLength = length; 
   }

   // Check for invalid header length   
   if(fileLength <= 0)
   {
      ErrorMessage("header length larger than file");
      fclose(fp);
      return(ERR);
   }

   // Allocate temporary space and read all data into it
   if(!(temp = new char[fileLength]))
   {
      ErrorMessage("Can't allocate memory for 1D temporary data");
      return(ERR);
   } 
   if(fread(temp,sizeof(char),fileLength,fp) != fileLength)
   {
      delete [] temp;
      ErrorMessage("Can't read all %ld bytes in file '%s'",fileLength,fileName);
      return(ERR);
   }

   // If byte reversed fix up the order before doing anything else
   if(type & BIG_ENDIAN) 
   {
      if(type & FLOAT_32 || type & INT_32) // Reverse byte order for 32 bit data
      {   
         for(i = 0; i < fileLength; i+=4)
         {
            unsigned char byte3 = temp[i];
            unsigned char byte2 = temp[i+1];
            unsigned char byte1 = temp[i+2];
            unsigned char byte0 = temp[i+3];

            temp[i]   = byte0;
            temp[i+1] = byte1;
            temp[i+2] = byte2;
            temp[i+3] = byte3;
         }
      }
      else if(type & FLOAT_64) // Reverse byte order for 64 bit data
      {   
         for(i = 0; i < fileLength; i+=8)
         {
            unsigned char byte7 = temp[i];
            unsigned char byte6 = temp[i+1];
            unsigned char byte5 = temp[i+2];
            unsigned char byte4 = temp[i+3];
            unsigned char byte3 = temp[i+4];
            unsigned char byte2 = temp[i+5];
            unsigned char byte1 = temp[i+6];
            unsigned char byte0 = temp[i+7];

            temp[i]   = byte0;
            temp[i+1] = byte1;
            temp[i+2] = byte2;
            temp[i+3] = byte3;
            temp[i+4] = byte4;
            temp[i+5] = byte5;
            temp[i+6] = byte6;
            temp[i+7] = byte7;
         }
      }
      if(type & INT_16) // Reverse byte order for 16 bit data
      {   
         for(i = 0; i < fileLength; i+=2)
         {
            unsigned char byte1 = temp[i];
            unsigned char byte0 = temp[i+1];

            temp[i]   = byte0;
            temp[i+1] = byte1;
         }
      } 
   }

   // Figure out the correct number of data points based on data type passed    
   nrPoints = fileLength/sizeof(complex);

   if(type & INT_16)
      nrPoints *= 2;

   // Allocate space for complex data
   (*data) = MakeCVector(nrPoints);
   if(!(*data))
   {
      delete [] temp;
      ErrorMessage("Can't allocate memory for 1D complex data");
      fclose(fp);      
      return(ERR);
   } 

   // Read data from temp into complex array
   if(type & FLOAT_32) 
   {   
      for(i = 0; i < nrPoints; i++)
         (*data)[i] = ((complex*)temp)[i];
   }
   else if(type & INT_32) 
   {
      for(i = 0; i < nrPoints; i++)
      {
         (*data)[i].r = ((long*)temp)[i*2];
         (*data)[i].i = ((long*)temp)[i*2+1];
      }
   }
   else if(type & FLOAT_64) // Read in double data
   {   
      for(i = 0; i < nrPoints; i++)
      {
         (*data)[i].r = ((double*)temp)[i*2];
         (*data)[i].i = ((double*)temp)[i*2+1];	
      }
   }
   else if(type & INT_16)
   {
      for(i = 0; i < nrPoints; i++)
      {
         (*data)[i].r = ((short*)temp)[i*2];
         (*data)[i].i = ((short*)temp)[i*2+1];
      }
   } 

   delete [] temp;
   fclose(fp);

   return(nrPoints);  
}



/*************************************************************************
* Load real ASCII data from a file and store in arrays "x" and "y"       *
* The data can be x,y data or y data, the program will figure it out the *
* best it can. The data can be return, space, tab or comma delimited.    *                                            
* Checks for file open error, invalid file format and too much data      *
* Returns OK if completed or ERR is error occured                        *
*                                                                        *
* Y data is stored in row form like this:                                *
*       10.2 3.4 5.6 7.8                                                 *
* or this                                                                *
*       10.2,3.4,5.6,7.8                                                 *
* or in column form                                                      *
* XY data is stored in XY pairs but is otherwise indistinguishable from  *
* Y data. Real data is like Y data and complex data is like XY data      *
* but as real imaginary pairs.                                           *
*                                                  CDE 19/7/2005          *
*************************************************************************/

bool IsDelimiter(char c)
{
   return(c == ' ' || c == '\t' || c == '\r' || c == ',' || c == '\n');
}

#define MAX_NUM_LEN 40

long Load1DRealAsciiDataFromFile(short reportError, char fileName[], long type, float **x, float **y)
{
   FILE *fp;
   char buf[BUFFER_SIZE];
   char numStr[50];
   long numBytes,count,j ;
      
// Load this file (we assume it is ascii data) ************************
   if(!(fp = fopen(fileName,"r")))
   {
      if(reportError)
         ErrorMessage("File '%s' can't be opened",fileName);
      return(ERR);
   }

// Quickly scan through file counting number of data values ***********
   count = 0;
   j = 0;
   do
   {
      numBytes = fread(buf,1,BUFFER_SIZE,fp);
      for(long i = 0; i < numBytes; i++)
      {
         if(IsDelimiter(buf[i]))
         {
            if(j > 0)
            {
               count++;
               j = 0;
            }
         }
         else
            j++;
      }

      if(feof(fp))  // Check for end of file (this can be a delimiter too)
      {
         if(!IsDelimiter(buf[numBytes-1]))
            count++; 
         break;
      } 
   }
   while(numBytes > 0);

// Check for even number of values in an XY data set
   if(type & XYDATA)
   {
      if(count%2 != 0)
		{
         if(reportError)
		      ErrorMessage("Odd number of values in XY data set");
		   fclose(fp);
		   return(ERR);
		} 
		else
		{
		   count /= 2;
		}
   }     
   
// Move back to start of file ********************************************
	fseek(fp, 0L, SEEK_SET);

// Allocate space for y data *********************************************
   (*y) = MakeVector(count);
   if(!(*y))
   {
      if(reportError)
         ErrorMessage("Can't allocate memory for 1D y data");
	   fclose(fp);      
      return(ERR);
   }

// Allocate space for x data *********************************************  
   if(type & XYDATA)
   {
      (*x) = MakeVector(count);
      if(!(*x))
      {
         FreeVector(*y);
         if(reportError)
            ErrorMessage("Can't allocate memory for 1D x data");
	      fclose(fp);            
         return(ERR);
      } 
   }
    
// Rescan file storing y or real data *****************************************
   if(type & REAL_DATA)
   {
	   count = 0;
	   j = 0;
	   do
	   {
	      numBytes = fread(buf,1,BUFFER_SIZE,fp);
	      for(long i = 0; i < numBytes; i++)
	      {
            if(IsDelimiter(buf[i]))
            {
               if(j > 0)
               {
	               numStr[j] = '\0';
	               if(sscanf(numStr,"%f",&(*y)[count]) != 1)
	               {
                     if(reportError)
		                  ErrorMessage("Y data value %ld is invalid",count);
	                  fclose(fp);
	                  return(ERR);
	               }
                  count++;
	               j = 0;
               }
            }
	         else
	         {
	            numStr[j++] = buf[i];
	            if(j > MAX_NUM_LEN)
	            {
                  if(reportError)
		               ErrorMessage("Data string %ld is too long",count);
	               fclose(fp);
	               return(ERR);
	            }
	         }
	      }
	   }
	   while(numBytes > 0);
	
   // Get last word (if there is no delimiter at end)
	   if(j > 0)
	   {
		   numStr[j] = '\0';
		   if(sscanf(numStr,"%f",&(*y)[count]) != 1)
		   {
            if(reportError)
		         ErrorMessage("Y data value %ld is invalid",count);
		      return(ERR);
		   }	            
		   count++;	 
		} 
	}  // End Y or real data
	
// Rescan file storing xy data *****************************************
   else if(type & XYDATA)
   {
	   count = 0;
	   j = 0;
		enum {X_VALUE,Y_VALUE} whichValue = X_VALUE;

	   do
	   {
	      numBytes = fread(buf,1,BUFFER_SIZE,fp);
	      for(long i = 0; i < numBytes; i++)
	      {
            if(IsDelimiter(buf[i]))
            {
               if(j > 0)
               {
	               numStr[j] = '\0';

	               if(whichValue == X_VALUE)
	               {
		               if(sscanf(numStr,"%f",&(*x)[count]) != 1)
		               {
                        if(reportError)
		            	      ErrorMessage("X data value %ld is invalid",count);
		                  fclose(fp);
		                  return(ERR);
		               }
		               j = 0;
		               whichValue = Y_VALUE;
		            }	
	               else if(whichValue == Y_VALUE)
	               {
		               if(sscanf(numStr,"%f",&(*y)[count]) != 1)
		               {
                        if(reportError)
		            	      ErrorMessage("Y data value %ld is invalid",count);
		                  fclose(fp);
		                  return(ERR);
		               }
		               j = 0;
		               whichValue = X_VALUE;
		               count++;	            
		            }
               }
            }
	         else
	         {
	            numStr[j++] = buf[i];
	            if(j > MAX_NUM_LEN)
	            {
                  if(reportError)
		               ErrorMessage("Data string %ld is too long",count);
	               fclose(fp);
	               return(ERR);
	            }
	         }
	      }
	   }
	   while(numBytes > 0);
	
   // Get last word (if there is no delimiter at end)
	   if(j > 0 && whichValue == Y_VALUE)
	   {
		   numStr[j] = '\0';
		   if(sscanf(numStr,"%f",&(*y)[count]) != 1)
		   {
            if(reportError)
	            ErrorMessage("Y data value %ld is invalid",count);
		      fclose(fp);
		      return(ERR);
		   }	            
		   count++;	 
         whichValue = X_VALUE;		   
		} 
		
	// Make sure we have an even number of values 		
		if(whichValue != X_VALUE)
		{
	      fclose(fp);
         if(reportError)
		      ErrorMessage("Odd number of value in XY data set");
		   return(ERR);
		}
				
	} 	// End XY data
	            
	fclose(fp);
	
   return(count);
}



/*************************************************************************
  Load complex ASCII data from a file and store in array "data"          
  The data can be space, tab, comma or return delimited.                                                                     
  Checks for file open error, invalid file format and too much data       
  Returns OK if completed or ERR is error occurred                         
*************************************************************************/


long Load1DComplexAsciiDataFromFile(char fileName[], long type, complex **data)
{
   FILE *fp;
   char buf[BUFFER_SIZE];
   char numStr[50];
   long numBytes,count,j;
   enum {REAL,IMAGINARY} whichValue = REAL;
      
// Load this file (we assume it is ascii data) ************************
   if(!(fp = fopen(fileName,"r")))
   {
      ErrorMessage("File '%s' can't be opened",fileName);
      return(ERR);
   }

// Quickly scan through file counting number of data values ***********
   count = 0;
   j = 0;
   do
   {
      numBytes = fread(buf,1,BUFFER_SIZE,fp);
      for(long i = 0; i < numBytes; i++)
      {
         if(IsDelimiter(buf[i]))
         {
            if(j > 0)
            {
               count++;
               j = 0;
            }
         }
         else
            j++;
      }

      if(feof(fp))  // Check for end of file (this can be a delimiter too)
      {
         if(!IsDelimiter(buf[numBytes-1]))
            count++; 
         break;
      } 
   }
   while(numBytes > 0);


// Check for even count
   if(count%2 != 0)
   {
      ErrorMessage("Odd number of values in complex data set");
		fclose(fp);      
      return(ERR);
   }
   count /= 2;
   
// Move back to start of file ********************************************
	fseek(fp, 0L, SEEK_SET);

// Allocate space for complex data ***************************************
   (*data) = MakeCVector(count);
   if(!(*data))
   {
      ErrorMessage("Can't allocate memory for complex data");
	   fclose(fp);      
      return(ERR);
   }
      
// Rescan file storing real and imaginary data *****************************************
   count = 0;
   j = 0;
   long k = 0;
   do
   {
      numBytes = fread(buf,1,BUFFER_SIZE,fp);
      for(long i = 0; i < numBytes; i++)
      {
         if(IsDelimiter(buf[i]))
         {
            if(j > 0)
            {
               numStr[j] = '\0';

               if(whichValue == REAL)
               {
                  if(sscanf(numStr,"%f",&(*data)[count].r) != 1)
                  {
	                  ErrorMessage("Data value %ld is invalid",count);
	                  fclose(fp);
	                  return(ERR);
	               }
	               whichValue = IMAGINARY;
		         }	
	            else
	            {
	               if(sscanf(numStr,"%f",&(*data)[count].i) != 1)
	               {
	                  ErrorMessage("Data value %ld is invalid",count);
	                  fclose(fp);
	                  return(ERR);
	               }
                  count++;	            
	               whichValue = REAL;
	            }		         	            
               j = 0;
            }
         }
	      else
	      {
	         numStr[j++] = buf[i];
	         if(j > MAX_NUM_LEN)
	         {
		         ErrorMessage("Data string %ld is too long",count);
	            fclose(fp);
	            return(ERR);
	         }
	      }
      }
	}
	while(numBytes > 0);

// Get last word (if there is no delimiter at end)
   if(j > 0 && whichValue == IMAGINARY)
   {
	   numStr[j] = '\0';
	   if(sscanf(numStr,"%f",&(*data)[count].i) != 1)
	   {
	      fclose(fp);
	      ErrorMessage("Data value %ld is invalid",count);
	      return(ERR);
	   }	  
      whichValue = REAL;             
	   count++;	 
	} 
	
// Make sure we have an even number of values 	
	if(whichValue != REAL)
	{
      fclose(fp);
	   ErrorMessage("Odd number of values in complex data set");
	   return(ERR);
	}

// Close data set and return number of values
	fclose(fp);
	
   return(count);
}



/*************************************************************************
  Load x-complex ASCII data from a file and store in arrays "x" and "data"          
  The data can be space, tab, comma or return delimited.                                                                     
  Checks for file open error, invalid file format and too much data       
  Returns OK if completed or ERR is error occurred                         
*************************************************************************/


long Load1DXComplexAsciiDataFromFile(char fileName[], long type, float **x, complex **data)
{
   FILE *fp;
   char buf[BUFFER_SIZE];
   char numStr[50];
   long numBytes,count,j;
   enum {ABSCISSA,REAL,IMAGINARY} whichValue = ABSCISSA;
      
// Load this file (we assume it is ascii data) ************************
   if(!(fp = fopen(fileName,"r")))
   {
      ErrorMessage("File '%s' can't be opened",fileName);
      return(ERR);
   }

// Quickly scan through file counting number of data values ***********
   count = 0;
   j = 0;
   do
   {
      numBytes = fread(buf,1,BUFFER_SIZE,fp);
      for(long i = 0; i < numBytes; i++)
      {
         if(IsDelimiter(buf[i]))
         {
            if(j > 0)
            {
               count++;
               j = 0;
            }
         }
         else
            j++;
      }

      if(feof(fp))  // Check for end of file (this can be a delimiter too)
      {
         if(!IsDelimiter(buf[numBytes-1]))
            count++; 
         break;
      } 
   }
   while(numBytes > 0);


// Check for odd count
   if(count%3 != 0)
   {
      ErrorMessage("Even number of values in x-complex data set");
		fclose(fp);      
      return(ERR);
   }
   count /= 3;
   
// Move back to start of file ********************************************
	fseek(fp, 0L, SEEK_SET);

// Allocate space for complex data ***************************************
   (*data) = MakeCVector(count);
   if(!(*data))
   {
      ErrorMessage("Can't allocate memory for complex y data");
	   fclose(fp);      
      return(ERR);
   }

   (*x) = MakeVector(count);
   if(!(*x))
   {
      FreeCVector(*data);
      ErrorMessage("Can't allocate memory for 1D x data");
	   fclose(fp);            
      return(ERR);
   } 
      
// Rescan file storing x and real and imaginary y data *****************************************
   count = 0;
   j = 0;
   long k = 0;
   do
   {
      numBytes = fread(buf,1,BUFFER_SIZE,fp);
      for(long i = 0; i < numBytes; i++)
      {
         if(IsDelimiter(buf[i]))
         {
            if(j > 0)
            {
               numStr[j] = '\0';

               if(whichValue == ABSCISSA)
               {
                  if(sscanf(numStr,"%f",&(*x)[count]) != 1)
                  {
                     FreeVector(*x);
                     FreeCVector(*data);
	                  ErrorMessage("x value %ld is invalid",count);
	                  fclose(fp);
	                  return(ERR);
	               }
	               whichValue = REAL;
               }
               else if(whichValue == REAL)
               {
                  if(sscanf(numStr,"%f",&(*data)[count].r) != 1)
                  {
                     FreeVector(*x);
                     FreeCVector(*data);
	                  ErrorMessage("Data value %ld is invalid",count);
	                  fclose(fp);
	                  return(ERR);
	               }
	               whichValue = IMAGINARY;
		         }	
	            else
	            {
	               if(sscanf(numStr,"%f",&(*data)[count].i) != 1)
	               {
                     FreeVector(*x);
                     FreeCVector(*data);
	                  ErrorMessage("Data value %ld is invalid",count);
	                  fclose(fp);
	                  return(ERR);
	               }
                  count++;	            
	               whichValue = ABSCISSA;
	            }		         	            
               j = 0;
            }
         }
	      else
	      {
	         numStr[j++] = buf[i];
	         if(j > MAX_NUM_LEN)
	         {
		         ErrorMessage("Data string %ld is too long",count);
	            fclose(fp);
	            return(ERR);
	         }
	      }
      }
	}
	while(numBytes > 0);

// Get last word (if there is no delimiter at end)
   if(j > 0 && whichValue == IMAGINARY)
   {
	   numStr[j] = '\0';
	   if(sscanf(numStr,"%f",&(*data)[count].i) != 1)
	   {
	      fclose(fp);
	      ErrorMessage("Data value %ld is invalid",count);
	      return(ERR);
	   }	  
      whichValue = REAL;             
	   count++;	 
	} 
	
// Make sure we have an odd number of values 	
	if(whichValue != ABSCISSA)
	{
      fclose(fp);
	   ErrorMessage("Even number of values in x-complex data set");
	   return(ERR);
	}

// Close data set and return number of values
	fclose(fp);
	
   return(count);
}


/************************************************************************
                        Export y or x-y data in binary format           
************************************************************************/


short Save1DDataToBinaryFile(char fileName[], long type, 
                              float *x, float *y, complex *yc, long size)
{
   FILE *fp;

   
// Open file **************************************************
   if(!(fp = fopen(fileName,"wb")))
	{
	   ErrorMessage("Can't save file '%s'",fileName);
	   return(ERR);
	}
        
// Write data in reversed order *******************************
   if(type & BIG_ENDIAN) 
   {
	   if(type & XYDATA) // Save the x data then the y data
	   { 
		   if(type & FLOAT_32)
		   {  
		      unsigned long temp;
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse32(((unsigned long*)x)[i]);
				   fwrite(&temp,4,1,fp);
				}
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse32(((unsigned long*)y)[i]);
				   fwrite(&temp,4,1,fp);
				}
		   }        
		   else if(type & INT_32)
		   {  
		      unsigned long temp;
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse32(nint(x[i]));
				   fwrite(&temp,4,1,fp);
				}
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse32(nint(y[i]));
				   fwrite(&temp,4,1,fp);
				}
		   }
		   else if(type & INT_16) 
		   {   
		      unsigned short temp;
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse16(nsint(x[i]));
				   fwrite(&temp,2,1,fp);
				}
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse16(nsint(y[i]));
				   fwrite(&temp,2,1,fp);
				}
		   } 
	   } // XYDATA
	   else if(type & REAL_DATA) // Just save the y data
	   {
		   if(type & FLOAT_32)
		   {  
		      unsigned long temp;
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse32(((unsigned long*)y)[i]);
				   fwrite(&temp,4,1,fp);
				}
		   }        
		   else if(type & INT_32)
		   {  
		      unsigned long temp;
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse32(nint(y[i]));
				   fwrite(&temp,4,1,fp);
				}
		   }
		   else if(type & INT_16) 
		   {   
		      unsigned short temp;
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse16(nsint(y[i]));
				   fwrite(&temp,2,1,fp);
				}
		   }  	  
      } // Y data
      else if(type & COMP_DATA)
      {
		   if(type & FLOAT_32)
		   {  
		      unsigned long temp;
		      float *real,*imag;
				for(long i = 0; i < size; i++)
				{
				   real = &yc[i].r;
				   temp = ByteReverse32(*((unsigned long*)real));
				   fwrite(&temp,4,1,fp);
				   imag = &yc[i].i;
				   temp = ByteReverse32(*((unsigned long*)imag));
				   fwrite(&temp,4,1,fp);				   				   
				}
		   }        
		   else if(type & INT_32)
		   {  
		      unsigned long temp;
				for(long i = 0; i < size; i++)
				{
				   temp = ByteReverse32(nint(yc[i].r));
				   fwrite(&temp,4,1,fp);
				   temp = ByteReverse32(nint(yc[i].i));
				   fwrite(&temp,4,1,fp);				   
				}
		   }
		   else if(type & INT_16) 
		   {   
		      unsigned short temp;
				for(long i = 0; i < size; i++)
				{				
				   temp = ByteReverse16(nsint(yc[i].r ));
				   fwrite(&temp,2,1,fp);
				   temp = ByteReverse16(nsint(yc[i].i));
				   fwrite(&temp,2,1,fp);				   
				}
		   }  	        
      } // Complex
      else
      {
         ErrorMessage("Export of this data type not implemented\n");
         return(ERR);
      }

   } // Reverse order
   

// Normal order *****************************  
   else
   {
   
	// Save data in xy or just y format
	
	   if(type & XYDATA) // Save the x data then the y data
	   { 	 
	      if(type & FLOAT_32) // save in float format
	      {
	         fwrite(x,4,size,fp);
	         fwrite(y,4,size,fp);
	      }
	      else if(type & INT_32) // save in long integer format
	      {
	         long temp32;
		      for(long i = 0; i < size; i++)
		      { 
		         temp32 = nint(x[i]);
		         fwrite(&temp32,4,1,fp);
		      }
		      for(long i = 0; i < size; i++)
		      { 
		         temp32 = nint(y[i]);
		         fwrite(&temp32,4,1,fp);
		      }       
	      }
	      else if(type & INT_16) // save in short integer format
	      {
	         long temp16;
		      for(long i = 0; i < size; i++)
		      { 
		         temp16 = nsint(x[i]);
		         fwrite(&temp16,2,1,fp);
		      }
		      for(long i = 0; i < size; i++)
		      { 
		         temp16 = nsint(y[i]);
		         fwrite(&temp16,2,1,fp);
		      } 
	      }
		} // XYDATA
		else if(type & REAL_DATA) // Just save the Y data
		{
	      if(type & FLOAT_32) // Save in float format
	      {
	         fwrite(y,4,size,fp);
	      }
	      else if(type & INT_32) // Save in long integer format
	      {
	         long temp32;
		      for(long i = 0; i < size; i++)
		      { 
		         temp32 = nint(y[i]);
		         fwrite(&temp32,4,1,fp);
		      }
	      }
	      else if(type & INT_16) // Save in short integer format
	      {
	         long temp16;
		      for(long i = 0; i < size; i++)
		      { 
		         temp16 = nsint(y[i]);
		         fwrite(&temp16,2,1,fp);
		      }
	      }
	      else if(type & INT_8) // Save in byte format
	      {
	         long temp8;
		      for(long i = 0; i < size; i++)
		      { 
		         temp8 = (unsigned char)(y[i] + 0.5);
		         fwrite(&temp8,1,1,fp);
		      }
	      }
	   } // REAL_DATA
	   else if(type & COMP_DATA)
	   {
	      if(type & FLOAT_32) // Save in float format
	      {
		      fwrite(yc,8,size,fp);
	      }
	      else if(type & INT_32) // Save in long integer format
	      {
	         long temp32;
		      for(long i = 0; i < size; i++)
		      { 
		         temp32 = nint(yc[i].r);
		         fwrite(&temp32,4,1,fp);
		         temp32 = nint(yc[i].i);
		         fwrite(&temp32,4,1,fp);		         
		      }
	      }
	      else if(type & INT_16) // Save in short integer format
	      {
	         long temp16;
		      for(long i = 0; i < size; i++)
		      { 
		         temp16 = nsint(yc[i].r);
		         fwrite(&temp16,2,1,fp);		      
		         temp16 = nsint(yc[i].i);
		         fwrite(&temp16,2,1,fp);
		      }
	      }
	   }
      else
      {
         ErrorMessage("Export of this data type not implemented\n");
         return(ERR);
      }
	} // Normal (PC) order
   fclose(fp);
   return(OK);
}


/************************************************************************
               Export y, x-y or complex data in ASCII format             
************************************************************************/

short Save1DDataToAsciiFile(char fileName[], long type, 
                            float *x, float *y, complex *yc, long size)
{
   FILE *fp;
   char delimiter;

// Open the file for append or write *******************************************   
   if(type & APPEND_MODE)
   {  
      if(!(fp = fopen(fileName,"a")))
	   {
	      ErrorMessage("Can't save file '%s'",fileName);
	      return(ERR);
	   }
   }
   else 
   {
      if(!(fp = fopen(fileName,"w")))
	   {
	      ErrorMessage("Can't save file '%s'",fileName);
	      return(ERR);
	   }
   }

// Choose delimiter for x-y data ***************************
   if(type & SPACE_DELIMIT)
      delimiter = ' ';
   else if(type & TAB_DELIMIT)
      delimiter = '\t';
   else if(type & RETURN_DELIMIT)
      delimiter = '\n';
   else if(type & COMMA_DELIMIT)
      delimiter = ',';
   else
   {
		fclose(fp);
	   ErrorMessage("Unknown delimiter");
	   return(ERR);
	}   

// Save data in xy, y or complex format ********************
   if(type & XYDATA)
   { 	   
		for(long i = 0; i < size; i++)
		{
		   fprintf(fp,"%g%c%g\n",x[i],delimiter,y[i]);
		}
	}
	else if(type & REAL_DATA)
	{
		for(long i = 0; i < size; i++)
		{
		   fprintf(fp,"%g\n",y[i]);
		}
   }
	else if(type & COMP_DATA)
	{
      for(long i = 0; i < size; i++)
	   {
			fprintf(fp,"%g%c%g\n",yc[i].r,delimiter,yc[i].i);
		}
	}
	else if(type & X_COMP_DATA)
	{
      for(long i = 0; i < size; i++)
	   {
			fprintf(fp,"%g%c%g%c%g\n",x[i],delimiter,yc[i].r,delimiter,yc[i].i);
		}
	}

// Close file **********************************************	
   fclose(fp);
  
   return(OK);
}

/************************************************************************
                        Export y or x-y data in binary format            
************************************************************************/


short Save2DDataToBinaryFile(char fileName[], long type, 
                             float **rDat, complex **cDat, long xSize, long ySize)
{
   FILE *fp;
   long x,y;
   
   
// Open file **************************************************
   if(!(fp = fopen(fileName,"wb")))
	{
	   ErrorMessage("Can't save file '%s'",fileName);
	   return(ERR);
	}
        
// Write data in reversed order *******************************
   if(type & BIG_ENDIAN) 
   {
	   if(type & REAL_DATA) 
	   {
		   if(type & FLOAT_32)
		   {  
		      unsigned long temp;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
				      temp = ByteReverse32(((unsigned long**)rDat)[y][x]);
				      fwrite(&temp,4,1,fp);
				   }
				}
		   }        
		   else if(type & INT_32)
		   {  
		      unsigned long temp;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
				      temp = ByteReverse32(nint(rDat[y][x]));
				      fwrite(&temp,4,1,fp);
				   }
				}				
		   }
		   else if(type & INT_16) 
		   {   
		      unsigned short temp;

				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
				      temp = ByteReverse16(nsint(rDat[y][x]));
				      fwrite(&temp,2,1,fp);
				   }
				}
								
		   }  	  
      } // Y data
      else if(type & COMP_DATA)
      {
		   if(type & FLOAT_32)
		   {  
		      unsigned long temp;
		      float *real,*imag;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
					   real = &cDat[y][x].r;
					   temp = ByteReverse32(*((unsigned long*)real));
					   fwrite(&temp,4,1,fp);
					   imag = &cDat[y][x].i;
					   temp = ByteReverse32(*((unsigned long*)imag));
					   fwrite(&temp,4,1,fp);				   				   
				      
				   }
				}				
		   }        
		   else if(type & INT_32)
		   {  
		      unsigned long temp;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
				      temp = ByteReverse32(nint(cDat[y][x].r));
				      fwrite(&temp,4,1,fp);
				      temp = ByteReverse32(nint(cDat[y][x].i));
				      fwrite(&temp,4,1,fp);				   
				   }
				}				
		   }
		   else if(type & INT_16) 
		   {   
		      unsigned short temp;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
				      temp = ByteReverse16(nint(cDat[y][x].r));
				      fwrite(&temp,2,1,fp);
				      temp = ByteReverse16(nint(cDat[y][x].i));
				      fwrite(&temp,2,1,fp);				   
				   }
				}					
		   }  	        
      } // Complex
   } // Reverse (Big Endian) order
   

// Normal order *****************************
   else
   {
   
      if(type & REAL_DATA) // Just save the Y data
		{
	      if(type & FLOAT_32) // Save in float format
	      {
				for(y = 0; y < ySize; y++)
				{
	            fwrite(rDat[y],4,xSize,fp);
				}	      
	      }
	      else if(type & INT_32) // Save in long integer format
	      {
	         long temp32;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
		            temp32 = nint(rDat[y][x]);
		            fwrite(&temp32,4,1,fp);
				   }
				}		      
	      }
	      else if(type & INT_16) // Save in short integer format
	      {
	         short temp16;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
		            temp16 = nsint(rDat[y][x]);
		            fwrite(&temp16,2,1,fp);
				   }
				}		      
	      }
	   } // REAL_DATA
	   
	   else if(type & COMP_DATA)
	   {
	      if(type & FLOAT_32) // Save in float format
	      {
				for(y = 0; y < ySize; y++)
				{
	            fwrite(cDat[y],8,xSize,fp);
				}	      
	      }
	      else if(type & INT_32) // Save in long integer format
	      {
	         long temp32;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
		            temp32 = nint(cDat[y][x].r);
		            fwrite(&temp32,4,1,fp);
		            temp32 = nint(cDat[y][x].i);
		            fwrite(&temp32,4,1,fp);
				   }
				}		      
	      }
	      else if(type & INT_16) // Save in short integer format
	      {
	         short temp16;
				for(y = 0; y < ySize; y++)
				{
				   for(x = 0; x < xSize; x++)
				   {
		            temp16 = nsint(cDat[y][x].r);
		            fwrite(&temp16,2,1,fp);
		            temp16 = nsint(cDat[y][x].i);
		            fwrite(&temp16,2,1,fp);
				   }
				}		      
	      }
	   }
	} // Normal (PC) order
	
   fclose(fp);
   return(OK);
}


/************************************************************************
       Export real or complex data in ASCII format to a 2D file          
************************************************************************/

short Save2DDataToAsciiFile(char fileName[], long type,  
                            float **rData, complex **cData,
                            long xsize, long ysize)
{
   FILE *fp;
   char delimiter = ',';
   long x,y;

// Open the file for append or write *******************************************   
   if(type & APPEND_MODE)
   {  
      if(!(fp = fopen(fileName,"a")))
	   {
	      ErrorMessage("Can't save file '%s'",fileName);
	      return(ERR);
	   }
   }
   else 
   {
      if(!(fp = fopen(fileName,"w")))
	   {
	      ErrorMessage("Can't save file '%s'",fileName);
	      return(ERR);
	   }
   }

// Choose delimiter for data *******************************
   if(type & SPACE_DELIMIT)
      delimiter = ' ';
   else if(type & TAB_DELIMIT)
      delimiter = '\t';
   else if(type & RETURN_DELIMIT)
      delimiter = '\n';
   else if(type & COMMA_DELIMIT)
      delimiter = ',';

// Save data in real format ********************************
	if(type & REAL_DATA)
	{
		for(y = 0; y < ysize; y++)
		{
		   for(x = 0; x < xsize-1; x++)
		   {
		      fprintf(fp,"%g%c",rData[y][x],delimiter);
		   }
		   fprintf(fp,"%g\n",rData[y][x]);
		}
   }
   
// Save data in complex format *****************************
	else if(type & COMP_DATA)
	{
		for(y = 0; y < ysize; y++)
		{
		   for(x = 0; x < xsize-1; x++)
		   {
		      fprintf(fp,"%g%c%g%c",cData[y][x].r,delimiter,cData[y][x].i,delimiter);
		   }
		   fprintf(fp,"%g%c%g\n",cData[y][x].r,delimiter,cData[y][x].i);
		}
	}
	
// Close file **********************************************
	
   fclose(fp);
  
   return(OK);
}



/************************************************************************
                   Export 3D data to a file in binary format             
************************************************************************/


short Save3DDataToBinaryFile(char fileName[], long type,     // Window, filename, data type
                             float ***rDat, complex ***cDat,            // Real data, complex data
                             long xSize, long ySize, long zSize)        // Data set dimensions
{
   FILE *fp;
   long x,y,z;

// Open file **************************************************      
   if(!(fp = fopen(fileName,"wb")))
	{
	   ErrorMessage("Can't save file '%s'",fileName);
	   return(ERR);
	}
        
// Write data in Big-Endian order *******************************
   if(type & BIG_ENDIAN) 
   {
	   if(type & REAL_DATA) 
	   {
		   if(type & FLOAT_32)
		   {  
		      unsigned long temp;
				for(z = 0; z < zSize; z++)
				{
					for(y = 0; y < ySize; y++)
				   {
				      for(x = 0; x < xSize; x++)
				      {
				         temp = ByteReverse32(((unsigned long***)rDat)[z][y][x]);
				         fwrite(&temp,4,1,fp);
				      }
				   }
				}
		   }        
		   else if(type & INT_32)
		   {  
		      unsigned long temp;
				for(z = 0; z < zSize; z++)
				{		      
				   for(y = 0; y < ySize; y++)
				   {
				      for(x = 0; x < xSize; x++)
				      {
				         temp = ByteReverse32(nint(rDat[z][y][x]));
				         fwrite(&temp,4,1,fp);
				      }
				   }	
				}			
		   }
		   else if(type & INT_16) 
		   {   
		      unsigned short temp;

				for(z = 0; z < zSize; z++)
				{
					for(y = 0; y < ySize; y++)
					{
					   for(x = 0; x < xSize; x++)
					   {
					      temp = ByteReverse16(nsint(rDat[z][y][x]));
					      fwrite(&temp,2,1,fp);
					   }
					}
				}
		   }  	  
      } // End Real data
      else if(type & COMP_DATA) // Complex data
      {
		   if(type & FLOAT_32)
		   {  
		      unsigned long temp;
		      float *real,*imag;
				for(z = 0; z < zSize; z++)
				{
					for(y = 0; y < ySize; y++)
					{
					   for(x = 0; x < xSize; x++)
					   {
						   real = &cDat[z][y][x].r;
						   temp = ByteReverse32(*((unsigned long*)real));
						   fwrite(&temp,4,1,fp);
						   imag = &cDat[z][y][x].i;
						   temp = ByteReverse32(*((unsigned long*)imag));
						   fwrite(&temp,4,1,fp);				   				   
					      
					   }
					}
				}				
		   }        
		   else if(type & INT_32)
		   {  
		      unsigned long temp;
				for(z = 0; z < zSize; z++)
				{
					for(y = 0; y < ySize; y++)
					{
					   for(x = 0; x < xSize; x++)
					   {
					      temp = ByteReverse32(nint(cDat[z][y][x].r));
					      fwrite(&temp,4,1,fp);
					      temp = ByteReverse32(nint(cDat[z][y][x].i));
					      fwrite(&temp,4,1,fp);				   
					   }
					}
				}				
		   }
		   else if(type & INT_16) 
		   {   
		      unsigned short temp;
				for(z = 0; z < zSize; z++)
				{
					for(y = 0; y < ySize; y++)
					{
					   for(x = 0; x < xSize; x++)
					   {
					      temp = ByteReverse16(nint(cDat[z][y][x].r));
					      fwrite(&temp,2,1,fp);
					      temp = ByteReverse16(nint(cDat[z][y][x].i));
					      fwrite(&temp,2,1,fp);				   
					   }
					}	
				}				
		   }  	        
      } // End Complex
   } // End Big Endian order
   

// Little Endian order *****************************
   else
   {
      if(type & REAL_DATA) // Just save the Y data
		{
	      if(type & FLOAT_32) // Save in float format
	      {
				for(z = 0; z < zSize; z++)
				{
					for(y = 0; y < ySize; y++)
					{
		            fwrite(rDat[z][y],4,xSize,fp);
					}	
				}      
	      }
	      else if(type & INT_32) // Save in long integer format
	      {
	         long temp32;
				for(z = 0; z < zSize; z++)
				{
					for(y = 0; y < ySize; y++)
					{
					   for(x = 0; x < xSize; x++)
					   {
			            temp32 = nint(rDat[z][y][x]);
			            fwrite(&temp32,4,1,fp);
					   }
					}
				}		      
	      }
	      else if(type & INT_16) // Save in short integer format
	      {
	         short temp16;
				for(z = 0; z < zSize; z++)
				{	         
					for(y = 0; y < ySize; y++)
					{
					   for(x = 0; x < xSize; x++)
					   {
			            temp16 = nsint(rDat[z][y][x]);
			            fwrite(&temp16,2,1,fp);
					   }
					}	
				}	      
	      }
	   } // End real data
	   
	   else if(type & COMP_DATA) // Complex data
	   {
	      if(type & FLOAT_32) // Save in float format
	      {
				for(z = 0; z < zSize; z++)
				{	      
					for(y = 0; y < ySize; y++)
					{
		            fwrite(cDat[z][y],8,xSize,fp);
					}	
				}      
	      }
	      else if(type & INT_32) // Save in long integer format
	      {
	         long temp32;
				for(z = 0; z < zSize; z++)
				{	         
					for(y = 0; y < ySize; y++)
					{
					   for(x = 0; x < xSize; x++)
					   {
			            temp32 = nint(cDat[z][y][x].r);
			            fwrite(&temp32,4,1,fp);
			            temp32 = nint(cDat[z][y][x].i);
			            fwrite(&temp32,4,1,fp);
					   }
					}	
				}	      
	      }
	      else if(type & INT_16) // Save in short integer format
	      {
	         short temp16;
				for(z = 0; z < zSize; z++)
				{		         
					for(y = 0; y < ySize; y++)
					{
					   for(x = 0; x < xSize; x++)
					   {
			            temp16 = nsint(cDat[z][y][x].r);
			            fwrite(&temp16,2,1,fp);
			            temp16 = nsint(cDat[z][y][x].i);
			            fwrite(&temp16,2,1,fp);
					   }
					}
				}		      
	      }
	   }
	} // Little Endian (PC) order
	
   fclose(fp);
   return(OK);
}

/************************************************************************
       Export real or complex data in ASCII format to a 2D file          
************************************************************************/

short Save3DDataToAsciiFile(char fileName[], long type, 
                            float ***rData, complex ***cData,
                            long xsize, long ysize, long zsize)
{
   FILE *fp;
   char delimiter = ',';
   long x,y,z;
   
// Open the file *******************************************
   if(type & APPEND_MODE)
   {  
      if(!(fp = fopen(fileName,"a")))
	   {
	      ErrorMessage("Can't save file '%s'",fileName);
	      return(ERR);
	   }
   }
   else 
   {
      if(!(fp = fopen(fileName,"w")))
	   {
	      ErrorMessage("Can't save file '%s'",fileName);
	      return(ERR);
	   }
   }

// Choose delimiter for data *******************************
   if(type & SPACE_DELIMIT)
      delimiter = ' ';
   else if(type & TAB_DELIMIT)
      delimiter = '\t';
   else if(type & RETURN_DELIMIT)
      delimiter = '\n';
   else if(type & COMMA_DELIMIT)
      delimiter = ',';

// Save data in real format ********************************
	if(type & REAL_DATA)
	{
		for(z = 0; z < zsize; z++)
		{	
			for(y = 0; y < ysize; y++)
			{
			   for(x = 0; x < xsize-1; x++)
			   {
			      fprintf(fp,"%g%c",rData[z][y][x],delimiter);
			   }
			   fprintf(fp,"%g\n",rData[z][y][x]);
			}
		}
   }
  
// Save data in complex format *****************************   
	else if(type & COMP_DATA)
	{
		for(z = 0; z < zsize; z++)
		{		
			for(y = 0; y < ysize; y++)
			{
			   for(x = 0; x < xsize-1; x++)
			   {
			      fprintf(fp,"%g%c%g%c",cData[z][y][x].r,delimiter,cData[z][y][x].i,delimiter);
			   }
			   fprintf(fp,"%g%c%g\n",cData[z][y][x].r,delimiter,cData[z][y][x].i);
			}
		}
	}
	
// Close file **********************************************
   fclose(fp);
  
   return(OK);
}

/************************************************************************
               Reverse the byte order in 32bit or 16bit words          
************************************************************************/

unsigned long ByteReverse32(unsigned long number)
{
 	unsigned long byte0,byte1,byte2,byte3;
  
   byte0 = (number & 0x000000FF)<<24;
   byte1 = (number & 0x0000FF00)<<8;
   byte2 = (number & 0x00FF0000)>>8;
   byte3 = (number & 0xFF000000)>>24;
   
   return(byte0 + byte1 + byte2 + byte3);
}

unsigned short ByteReverse16(unsigned short number)
{
 	unsigned short byte0,byte1;
  
   byte0 = (number & 0x00FF)<<8;
   byte1 = (number & 0xFF00)>>8;
   
   return(byte0 + byte1);
}



/*************************************************************************
*  Load binary data from a file into a real or complex matrix variable   *
*                                                                        *
* Can load 1D, 2D ,3D or 4D data. File format is:                        *
*                                                                        *
* owner ... PROS                                                         *
* format .. DATA                                                         *
* version . V1.0                                                         *
* type .... ID_REAL_TYPE/ID_COMPLEX_TYPE                                 *
* width ... integer                                                      *
* height .. integer                                                      *
* depth ... integer                                                      *
* data .... list of 4-byte floats                                        *
*************************************************************************/

short LoadData(Interface *itfc, char* pathName, char* fileName, char *varName, short varScope)
{
   FILE *fp;
   long owner;
   long format;
   long version;
   long dataType;
   Variable *var;
   long width,height,depth,hypers=1;
   long i,j;
      
// Open file for binary read ******************
   SetCurrentDirectory(pathName);  
   if(!(fp = fopen(fileName,"rb")))
	{
	   ErrorMessage("Can't open file '%s'",fileName);
	   return(ERR);
	}

// Make sure its a Prospa file ****************
   fread(&owner,sizeof(long),1,fp);
   if(owner != 'PROS')
   {
      fclose(fp);
	   ErrorMessage("'%s' is not a native %s 1D data file",fileName,APPLICATION_NAME);
	   return(ERR);   
   }

// Make sure its a data file ********************
   fread(&format,sizeof(long),1,fp);
   if(format != 'DATA')
   {
      fclose(fp);
	   ErrorMessage("'%s' is not a data file",fileName);
	   return(ERR);   
   }
      
// Check file version number ******************
   fread(&version,sizeof(long),1,fp); 
   if(version != PLOTFILE_VERSION_1_0 && 
		version != PLOTFILE_VERSION_1_1)
   {
      fclose(fp);   
	   ErrorMessage("Unknown or newer file version number");
	   return(ERR);   
   } 

// Check out data type ************************
   fread(&dataType,sizeof(long),1,fp); 

   if(dataType != REAL_DATA_TYPE && 
      dataType != DOUBLE_DATA_TYPE &&
      dataType != COMPLEX_DATA_TYPE)
   {
      fclose(fp);   
	   ErrorMessage("Unknown or newer data type");
	   return(ERR);   
   }

// Get data size ******************************
   fread(&width,sizeof(long),1,fp); 
   fread(&height,sizeof(long),1,fp); 
   fread(&depth,sizeof(long),1,fp);
   if(version == PLOTFILE_VERSION_1_1)
      fread(&hypers,sizeof(long),1,fp);

// Make variables and read in data ************
   if(hypers > 1) // 4D
   {
	   switch(dataType)
	   {
			case(REAL_DATA_TYPE):      
		   {
				var = AddVariable(itfc,varScope, MATRIX2D, varName);  				   
				var->MakeMatrix4DFromVector(NULL, width,height,depth,hypers);
				for(long k = 0; k < hypers; k++)	
				   for(j = 0; j < depth; j++)	
				      for(i = 0; i < height; i++)	
				         fread(VarReal4DMatrix(var)[k][j][i],sizeof(float),width,fp);
				break;
			}
			case(COMPLEX_DATA_TYPE):
		   {
            var = AddVariable(itfc,varScope, CMATRIX2D, varName);  				
				var->MakeCMatrix4DFromCVector(NULL, width,height,depth,hypers);
				for(long k = 0; k < hypers; k++)	
				   for(j = 0; j < depth; j++)	
				      for(i = 0; i < height; i++)	
				      fread(VarComplex4DMatrix(var)[k][j][i],sizeof(complex),width,fp);
				break;
			}
		} 
   }
   else if(depth > 1 && hypers == 1) // 2D or 3D
   {
	   switch(dataType)
	   {
			case(REAL_DATA_TYPE):      
		   {
				var = AddVariable(itfc,varScope, MATRIX2D, varName);  				   
				var->MakeMatrix3DFromVector(NULL, width,height,depth);
				for(j = 0; j < depth; j++)	
				   for(i = 0; i < height; i++)	
				      fread(VarReal3DMatrix(var)[j][i],sizeof(float),width,fp);
				break;
			}
			case(COMPLEX_DATA_TYPE):
		   {
            var = AddVariable(itfc,varScope, CMATRIX2D, varName);  				
				var->MakeCMatrix3DFromCVector(NULL, width,height,depth);
				for(j = 0; j < depth; j++)	
				   for(i = 0; i < height; i++)	
				      fread(VarComplex3DMatrix(var)[j][i],sizeof(complex),width,fp);
				break;
			}
		} 
   }
   else // 1D or 2D data
   {   
	   switch(dataType)
	   {
			case(REAL_DATA_TYPE):      
		   {
			   var = AddVariable(itfc,varScope, MATRIX2D, varName);  			  
				var->MakeAndLoadMatrix2D(NULL,width,height);
				for(i = 0; i < height; i++)	
				   fread(var->GetMatrix2D()[i],sizeof(float),width,fp);
				break;
			}
			case(DOUBLE_DATA_TYPE):      
		   {
			   var = AddVariable(itfc,varScope, DMATRIX2D, varName);  			  
				var->MakeAndLoadDMatrix2D(NULL,width,height);
				for(i = 0; i < height; i++)	
               fread(var->GetDMatrix2D()[i],sizeof(double),width,fp);
				break;
			}
			case(COMPLEX_DATA_TYPE):
		   {
		      var = AddVariable(itfc,varScope, CMATRIX2D, varName);  
				var->MakeAndLoadCMatrix2D(NULL,width,height);
				for(i = 0; i < height; i++)	
				   fread(var->GetCMatrix2D()[i],sizeof(complex),width,fp);
				break;
			}
		}	
   }

// Close file **********************************
	fclose(fp);
	return(OK);
}


short LoadData(Interface *itfc, char* pathName, char* fileName, Variable *ans, Variable *ans2)
{
   FILE *fp;
   long owner;
   long format;
   long version;
   long dataType;
   Variable *var;
   long width,height,depth,hypers=1;
   long i,j;
      
// Open file for binary read ******************
   SetCurrentDirectory(pathName);  
   if(!(fp = fopen(fileName,"rb")))
	{
	   ErrorMessage("Can't open file '%s'",fileName);
	   return(ERR);
	}

// Make sure its a Prospa file ****************
   fread(&owner,sizeof(long),1,fp);
   if(owner != 'PROS')
   {
      fclose(fp);
	   ErrorMessage("'%s' is not a native %s 1D data file",fileName,APPLICATION_NAME);
	   return(ERR);   
   }

// Make sure its a data file ********************
   fread(&format,sizeof(long),1,fp);
   if(format != 'DATA')
   {
      fclose(fp);
	   ErrorMessage("'%s' is not a data file",fileName);
	   return(ERR);   
   }
      
// Check file version number ******************
   fread(&version,sizeof(long),1,fp); 
   if(version != PLOTFILE_VERSION_1_0 && 
		version != PLOTFILE_VERSION_1_1)
   {
      fclose(fp);   
	   ErrorMessage("Unknown or newer file version number");
	   return(ERR);   
   } 

// Check out data type ************************
   fread(&dataType,sizeof(long),1,fp); 

   if(dataType != REAL_DATA_TYPE && 
      dataType != DOUBLE_DATA_TYPE &&
      dataType != COMPLEX_DATA_TYPE && 
		dataType != XY_REAL_DATA_TYPE && 
		dataType != XY_COMPLEX_DATA_TYPE)
   {
      fclose(fp);   
	   ErrorMessage("Unknown or newer data type");
	   return(ERR);   
   }

// Get data size ******************************
   fread(&width,sizeof(long),1,fp); 
   fread(&height,sizeof(long),1,fp); 
   fread(&depth,sizeof(long),1,fp); 
   if(version == PLOTFILE_VERSION_1_1)
      fread(&hypers,sizeof(long),1,fp);

// Get the axes
  // if(version == PLOTFILE_VERSION_1_2)
  // {
  //    long xAxisSize,yAxisSize;
  //    float *xAxis,*yAxis;
  //    fread(&xAxisSize,sizeof(long),1,fp);
  //    fread(&yAxisSize,sizeof(long),1,fp);
		//ans3->MakeAndLoadMatrix2D(NULL, xAxisSize,1);
		//ans4->MakeAndLoadMatrix2D(NULL, yAxisSize,1);
  //    xAxis = ans3->GetMatrix2D();
  //    yAxis = ans4->GetMatrix2D();
	 //  fread(xAxis[0],sizeof(float),xAxisSize,fp);
	 //  fread(yAxis[0],sizeof(float),yAxisSize,fp);
  // }
	itfc->nrRetValues = 1;

// Make variables and read in data ************
   if(hypers > 1) // 4D
   {
	   switch(dataType)
	   {
			case(REAL_DATA_TYPE):      
		   {
			   var = ans;   				   
				var->MakeMatrix4DFromVector(NULL, width,height,depth,hypers);
				for(long k = 0; k < hypers; k++)	
				   for(j = 0; j < depth; j++)	
				      for(i = 0; i < height; i++)	
				         fread(VarReal4DMatrix(var)[k][j][i],sizeof(float),width,fp);
				break;
			}
			case(COMPLEX_DATA_TYPE):
		   {
			   var = ans;  				
				var->MakeCMatrix4DFromCVector(NULL, width,height,depth,hypers);
				for(long k = 0; k < hypers; k++)	
				   for(j = 0; j < depth; j++)	
				      for(i = 0; i < height; i++)	
				      fread(VarComplex4DMatrix(var)[k][j][i],sizeof(complex),width,fp);
				break;
			}
		} 
   }
   else if(depth > 1 && hypers == 1) // 2D or 3D
   {
	   switch(dataType)
	   {
			case(REAL_DATA_TYPE):      
		   {
			   var = ans;   
				var->MakeMatrix3DFromVector(NULL, width,height,depth);
				for(j = 0; j < depth; j++)	
				   for(i = 0; i < height; i++)	
				      fread(VarReal3DMatrix(var)[j][i],sizeof(float),width,fp);
				break;
			}
			case(COMPLEX_DATA_TYPE):
		   {
				var = ans;   
				var->MakeCMatrix3DFromCVector(NULL, width,height,depth);
				for(j = 0; j < depth; j++)	
				   for(i = 0; i < height; i++)	
				      fread(VarComplex3DMatrix(var)[j][i],sizeof(complex),width,fp);
				break;
			}
		} 
   }
   else // 1D & 2D data
   {   
	   switch(dataType)
	   {
			case(REAL_DATA_TYPE):      
		   {
			   var = ans;  
				var->MakeAndLoadMatrix2D(NULL, width,height);
				for(i = 0; i < height; i++)	
				   fread(VarRealMatrix(var)[i],sizeof(float),width,fp);
				break;
			}
			case(DOUBLE_DATA_TYPE):      
		   {
			   var = ans; 
				var->MakeAndLoadDMatrix2D(NULL,width,height);
				for(i = 0; i < height; i++)	
               fread(var->GetDMatrix2D()[i],sizeof(double),width,fp);
				break;
			}
			case(COMPLEX_DATA_TYPE):
		   {
				var = ans;   
				var->MakeAndLoadCMatrix2D(NULL, width,height);
				for(i = 0; i < height; i++)	
				   fread(VarComplexMatrix(var)[i],sizeof(complex),width,fp);
				break;
			}
			case(XY_REAL_DATA_TYPE):
		   {
				ans->MakeAndLoadMatrix2D(NULL, width,height);
				ans2->MakeAndLoadMatrix2D(NULL, width,height);
				fread(VarRealMatrix(ans)[0],sizeof(float),width,fp);
				fread(VarRealMatrix(ans2)[0],sizeof(float),width,fp);
				itfc->nrRetValues = 2;
				break;
			}
			case(XY_COMPLEX_DATA_TYPE):
		   {
				ans->MakeAndLoadMatrix2D(NULL, width,height);
				ans2->MakeAndLoadCMatrix2D(NULL, width,height);
				fread(VarRealMatrix(ans)[0],sizeof(float),width,fp);
				fread(VarComplexMatrix(ans2)[0],sizeof(complex),width,fp);
				itfc->nrRetValues = 2;
				break;
			}
		}	
   }

// Close file **********************************
	fclose(fp);
	return(OK);
}

/********************************************************************
*     Load a file from disk and interpret the contents as a list
*
* Usage:  list = load("file.lst")
*
* File must consist of a series of strings separated by commas or
* carriage-return line-feeds.
*
*********************************************************************/

short LoadListData(char* pathName, char* fileName, Variable *ans)
{
   FILE *fp;
   char *str;
   char *err;
   long bytes,sz;
   char *txtlist;
   bool eolFound = false;

// Change to specified folder *****************
   if(pathName[0] != '\0')
      SetCurrentDirectory(pathName);  

// Open file for binary read ******************   
   if(!(fp = fopen(fileName,"rb")))
	{
	   ErrorMessage("Can't open file '%s'",fileName);
	   return(ERR);
	}

// Get file length
   bytes = GetFileLength(fp);

// If the file is empty return null
   if(bytes == 0)
	{
	   fclose(fp);
      ans->MakeNullVar();
	   return(OK);
	}
 
// Allocate space for list *********************
   txtlist = new char[bytes+2]; // Allow for addition of possible comma and null
   str = new char[bytes+2]; // Temporary string
   txtlist[0] = '\0';

// Load data into list array *********************
	while(1)
	{
      eolFound = false;
	   err = fgets(str,bytes+1,fp); // Read a line 
	   if(!err) break;
	   sz = strlen(str);
	   if(sz >= 2 && str[sz - 2] == '\r' && str[sz - 1] == '\n') // Windows file
      {
	      str[sz - 2] = '\0';	   
         eolFound = true;
      }
      else if(sz >= 2 && str[sz - 2] != '\r' && str[sz - 1] == '\n') // Unix file
      {
         str[sz - 1] = '\0';
         eolFound = true;
      }
      else if(sz >= 1 && str[sz - 1] == '\r') // Mac file
      {
         str[sz - 1] = '\0';
         eolFound = true;
      }
      if(str[0] == '\0' || IsWhiteSpaceString(str) || IsCommentLine(str)) // Ignore empty or line full of whitespace
         continue;
	   strcat(txtlist,str);
      if(eolFound)
	     strcat(txtlist,",");
	} 
	fclose(fp);
   delete [] str;
	bytes = strlen(txtlist);

   if(bytes > 0)
   {
      if(txtlist[bytes-1] == ',')
         txtlist[bytes-1] = '\0'; // Remove trailing comma
      
   // Convert the text to a list and then return in ansVar
	   char **list;
	   long numStr;
	   list = MakeListFromText(txtlist,&numStr);	
	   ans->MakeAndSetList(list,numStr);
	   FreeList(list,numStr);
   }
	else
	   ans->MakeNullVar();

   delete [] txtlist; 	
  	return(OK);
}

/********************************************************************
* Load a file from disk and interpret the contents as a parameter list
* and long real numbers (>= 7 characters are interpreted as doubles)
*
* Usage:  list = load("file.lst")
*
* File must consist of a series of strings separated by commas or
* carriage-return line-feeds.
*
*********************************************************************/

short LoadListDataWithTrueDoubles(char* pathName, char* fileName, Variable *ans)
{
   FILE *fp;
   char *str;
   char *err;
   long bytes,sz;
   bool eolFound = false;

// Change to specified folder *****************
   if(pathName[0] != '\0')
      SetCurrentDirectory(pathName);  

// Open file for binary read ******************   
   if(!(fp = fopen(fileName,"rb")))
	{
	   ErrorMessage("Can't open file '%s'",fileName);
	   return(ERR);
	}

// Get file length
   bytes = GetFileLength(fp);

// If the file is empty return null
   if(bytes == 0)
	{
	   fclose(fp);
      ans->MakeNullVar();
	   return(OK);
	}
 
// Allocate space for list *********************
   CText txtlist; 
   CArg carg('=');
   str = new char[bytes+2]; // Temporary string

// Load data into list array *********************
	while(1)
	{
      eolFound = false;
	   err = fgets(str,bytes+1,fp); // Read a line 
	   if(!err) break;
	   sz = strlen(str);
	   if(sz >= 2 && str[sz - 2] == '\r' && str[sz - 1] == '\n') // Windows file
      {
	      str[sz - 2] = '\0';	   
         eolFound = true;
      }
      else if(sz >= 2 && str[sz - 2] != '\r' && str[sz - 1] == '\n') // Unix file
      {
         str[sz - 1] = '\0';
         eolFound = true;
      }
      else if(sz >= 1 && str[sz - 1] == '\r') // Mac file
      {
         str[sz - 1] = '\0';
         eolFound = true;
      }
      if(str[0] == '\0' || IsWhiteSpaceString(str) || IsCommentLine(str)) // Ignore empty or line full of whitespace or comments
         continue;

      extern bool IsDoubleByPrecision(char*);
      int cnt = carg.Count(str);
      if(cnt == 2)
      {
         char *left = new char[sz+1];
         char *right = new char[sz+1];
         strcpy(left,carg.Extract(1));
         strcpy(right,carg.Extract(2));
         char *endPtr;
         double num = strtod(right,&endPtr);
         int sz = endPtr-right;
         if(sz == strlen(right) && IsDoubleByPrecision(right))
         {
            txtlist.Concat(left);
            txtlist.Concat(" = ");
            txtlist.Concat(right);
	         txtlist.Append('d');
         }
         else
         {
	         txtlist.Concat(str);
         }
         delete [] left;
         delete [] right;
      }
      else
      {    
	     txtlist.Concat(str);
      }
      if(eolFound)
	      txtlist.Append(',');
	} 
	fclose(fp);
   delete [] str;
	bytes = txtlist.Size();

   if(bytes > 0)
   {
      if(txtlist[bytes-1] == ',')
         txtlist[bytes-1] = '\0'; // Remove trailing comma
      
   // Convert the text to a list and then return in ansVar
	   char **list;
	   long numStr;
	   list = MakeListFromText(txtlist.Str(),&numStr);	
	   ans->MakeAndSetList(list,numStr);
	   FreeList(list,numStr);
   }
  	return(OK);
}

/********************************************************************
*                    Save a list to disk
*
* Usage:  save("file.lst",list)
*
*********************************************************************/

short SaveListData(HWND hWnd, char* pathName, char* fileName, Variable *var)
{
   FILE *fp;
   
// Check variable
   if(var != NULL)
   {
		if(var->GetType() != LIST)
		{
		   ErrorMessage("Variable '%s' is not a list",var->GetName());
		   return(ERR);
		}
   }
   else
	{
	   ErrorMessage("Variable is not defined");
	   return(ERR);
	}
      
// Change to specified folder *****************
   if(pathName[0] != '\0')
      SetCurrentDirectory(pathName);  

// Open file for ASCII write ****************** 
   if(!(fp = fopen(fileName,"w")))
	{
	   ErrorMessage("Can't open file '%s'",fileName);
	   return(ERR);
	}

// Loop over entries in list ******************
   for(int i = 0; i < VarWidth(var); i++)
   {
      if(&VarList(var)[i])
      {
         fprintf(fp,"%s\n",VarList(var)[i]);
      }
      else
      {
         fclose(fp);
	      ErrorMessage("Invalid entry %d in list '%s'",i,var->GetName()); // Shouldn't happen
	      return(ERR);
	   }
	}      
  
// Close file **********************************
   fclose(fp);
  	return(OK);
}


/********************************************************************
*                Save a structure to disk as a list
*
* Usage:  save("file.lst","list")
*
*********************************************************************/

short SaveStructDataAsList(HWND hWnd, char* pathName, char* fileName, Variable *var)
{
   FILE *fp;
   extern bool IsString(char *str);

   // Check variable
   if (var != NULL)
   {
      if (var->GetType() != STRUCTURE)
      {
         ErrorMessage("Variable '%s' is not a structure", var->GetName());
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Variable is not defined");
      return(ERR);
   }

   // Change to specified folder *****************
   if (pathName[0] != '\0')
      SetCurrentDirectory(pathName);

   // Open file for ASCII write ****************** 
   if (!(fp = fopen(fileName, "w")))
   {
      ErrorMessage("Can't open file '%s'", fileName);
      return(ERR);
   }

   // Get the structure info
   Variable *struc, *svar;

   struc = var->GetStruct();
   if (!struc)
   {
      ErrorMessage("Null structure to print");
      return(ERR);
   }
   svar = struc->next;

   int i = 0;
   CText temp;

   while (svar != NULL)
   {
      ConvertVariableToText(svar, temp, "", false);
      if(IsString(temp.Str()))
         fprintf(fp, "%s = \"%s\"\n", svar->GetName(), temp.Str());
      else
         fprintf(fp, "%s = %s\n", svar->GetName(), temp.Str());
      svar = svar->next;
   }

   // Close file **********************************
   fclose(fp);
   return(OK);
}

/********************************************************************
*                Save a structure to disk as a list
*
* Usage:  save("file.lst","binary")
*
*********************************************************************/

short SaveStructDataInBinary(HWND hWnd, char* pathName, char* fileName, Variable *var)
{
   FILE *fp;

   // Check variable
   if (var != NULL)
   {
      if (var->GetType() != STRUCTURE)
      {
         ErrorMessage("Variable '%s' is not a structure", var->GetName());
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Variable is not defined");
      return(ERR);
   }

   // Change to specified folder *****************
   if (pathName[0] != '\0')
      SetCurrentDirectory(pathName);

   // Open file for ASCII write ****************** 
   if (!(fp = fopen(fileName, "w")))
   {
      ErrorMessage("Can't open file '%s'", fileName);
      return(ERR);
   }

   // Get the structure info
   Variable *struc, *svar;

   struc = var->GetStruct();
   if (!struc)
   {
      ErrorMessage("Null structure to print");
      return(ERR);
   }
   svar = struc->next;

   int i = 0;
   while (svar != NULL)
   {
   //   fprintf(fp, "%s = %s\n", svar->GetName(), svar->GetValueAsString());
      svar = svar->next;
   }

   // Close file **********************************
   fclose(fp);
   return(OK);
}

/********************************************************************
*      Save a parameter list to disk writing out doubles without
*      the trailing 'd' but with at least 15 characters
*
* Usage:  save("file.par",list,"truedoubles")
*
*********************************************************************/

short SaveListDataTrueDoubles(HWND hWnd, char* pathName, char* fileName, Variable *var)
{
   FILE *fp;
   CArg carg('=');

// Check variable
   if(var != NULL)
   {
		if(var->GetType() != LIST)
		{
		   ErrorMessage("Variable '%s' is not a list",var->GetName());
		   return(ERR);
		}
   }
   else
	{
	   ErrorMessage("Variable is not defined");
	   return(ERR);
	}
      
// Change to specified folder *****************
   if(pathName[0] != '\0')
      SetCurrentDirectory(pathName);  

// Open file for ASCII write ****************** 
   if(!(fp = fopen(fileName,"w")))
	{
	   ErrorMessage("Can't open file '%s'",fileName);
	   return(ERR);
	}

// Loop over entries in list ******************
   for(int i = 0; i < VarWidth(var); i++)
   {
      if(&VarList(var)[i])
      {
         CText line = VarList(var)[i];
         int sz = line.Size();
         int cnt = carg.Count(line.Str());

         if(cnt == 2)
         {
            double num;
            char *left = new char[sz+1];
            char *right = new char[sz+1];
            strcpy(left,carg.Extract(1));
            strcpy(right,carg.Extract(2));
            if(IsDouble(right,num))
            {
 /*              if(num > 1e3 || num < 1e-3)
               {
		            int sz = _snprintf(0,0,"%1.15e",num);
                  char *entry = new char[sz+1]; 
				      _snprintf(entry,sz,"%1.15e",num);
                  entry[sz] = '\0';
            
                  line = "";
                  line.Concat(left);
                  line.Concat(" = ");
                  line.Concat(entry);
                  delete [] entry;
               }
               else
               {
		            int sz = _snprintf(0,0,"%1.15f",num);
                  char *entry = new char[sz+1]; 
				      _snprintf(entry,sz,"%1.15f",num);
                  entry[sz] = '\0';
            
                  line = "";
                  line.Concat(left);
                  line.Concat(" = ");
                  line.Concat(entry);
                  delete [] entry;
               }*/
               {
		            int sz = _snprintf(0,0,"%.17g",num);
                  char *entry = new char[sz+1]; 
				      _snprintf(entry,sz,"%.17g",num);
                  entry[sz] = '\0';
            
                  line = "";
                  line.Concat(left);
                  line.Concat(" = ");
                  line.Concat(entry);
                  delete [] entry;
               }
            }  

            delete [] left;
            delete [] right;
         }

         fprintf(fp,"%s\n",line.Str());
      }     
      else
      {
         fclose(fp);
	      ErrorMessage("Invalid entry %d in list '%s'",i,var->GetName()); // Shouldn't happen
	      return(ERR);
	   }
	}      
  
// Close file **********************************
   fclose(fp);
  	return(OK);
}
		
/*************************************************************************
*         Save an XrealY, X,complexY data set with parameters            *
*         so it can be loaded at a later date by this software.          *
*                                                                        *
*  fileName ... name to call data file.                                  *
*  varX ....... name of x variable to save                               *
*  varY ....... name of y variable to save                               *
*                                                                        *
*  Open file for binary save                                             *
*  Save owner (PROS), dimension (DAT1) and version (V1.1)                *
*  Save type (REAL_XY_TYPE,COMPLEX_XY_TYPE)                              *
*  Extract variables                                                     *
*  Save file length (in words)                                           *
*  Save data (real or complex)                                           *
*  Close file                                                            *
*                                                                        *
* Revision history                                                       *
*   20/6/10 - first version                                              *
*************************************************************************/

short SaveData(HWND hWnd, char* pathName, char* fileName, Variable *varX, Variable *varY)
{
   long dataType;
 
   if(gReadOnly)
   {
      MessageBox(prospaWin,"This version of Prospa is readonly, please purchase a full licensed version if you wish to save files","Read-only warning",MB_OK | MB_ICONEXCLAMATION);
      ErrorMessage("This version of Prospa is read only");
      return(ERR);
   }

// Make sure variable names has been supplied
   if(varX == NULL || varY == NULL)
   {
	   ErrorMessage("No variables supplied");
	   return(ERR);
	}	

// Open file for binary write ********************
   FILE *fp;      
   if(!(fp = fopen(fileName,"wb")))
	{
	   ErrorMessage("Can't save file '%s'",fileName);
	   return(ERR);
	}

// Save file owner, type and version *************
   long owner      = 'PROS'; fwrite(&owner,sizeof(long),1,fp);       // File owner (Prospa)
   long type       = 'DATA'; fwrite(&type,sizeof(long),1,fp);        // File type
   long version    = PLOTFILE_VERSION_1_1; 
	fwrite(&version,sizeof(long),1,fp);     // File version

// Check to see if X variable type is correct *********
	if(varX->GetType() == MATRIX2D && varX->GetDimY() == 1)
	{
      if(varY->GetDimY() == 1)
		{
			if(varY->GetType() == MATRIX2D)
		     dataType = XY_REAL_DATA_TYPE;	
			else if(varY->GetType() == CMATRIX2D) 
	        dataType = XY_COMPLEX_DATA_TYPE;
			else
			{
			   fclose(fp);	
				ErrorMessage("Y variable '%s' should be real or complex row vector",varY->GetName());
				return(ERR);
			}
		}
	}
   else
	{
	   fclose(fp);	
		ErrorMessage("X variable '%s' should be a real row vector",varY->GetName());
		return(ERR);
	}

	if(varX->GetDimX() != varY->GetDimX())
	{
	   fclose(fp);	
		ErrorMessage("X and Y variables should have same length");
		return(ERR);
	}

// Save data type (real, double or complex) ***************       
   fwrite(&dataType,sizeof(long),1,fp); 
	   
// Save data dimensions ***************************
   long dim1 = varX->GetDimX();
   long dim2 = 1;
   long dim3 = 1;
   long dim4 = 1;

   fwrite(&dim1,sizeof(long),1,fp); 
   fwrite(&dim2,sizeof(long),1,fp); 
   fwrite(&dim3,sizeof(long),1,fp); 
   fwrite(&dim4,sizeof(long),1,fp); 

// Save data **************************************
  
   switch(dataType)
   {
      case(XY_REAL_DATA_TYPE):
      {
         float *data;
 
			data = varX->GetMatrix2D()[0];	   
		   fwrite(data,sizeof(float),varX->GetDimX(),fp);
			data = varY->GetMatrix2D()[0];	   
		   fwrite(data,sizeof(float),varY->GetDimX(),fp);		   
         break;
      }
      case(XY_COMPLEX_DATA_TYPE):
      {
         float *data;
         complex *cdata;
 
			data = varX->GetMatrix2D()[0];	   
		   fwrite(data,sizeof(float),varX->GetDimX(),fp);
			cdata = varY->GetCMatrix2D()[0];	   
		   fwrite(cdata,sizeof(complex),varY->GetDimX(),fp);		   
         break;
      }
   }
	

// Close file ************************************
   fclose(fp);

   return(OK);
}


short SaveData(HWND hWnd, char* pathName, char* fileName, Variable *var)
{
   short varType;
   long dataType;
   
   if(gReadOnly)
   {
      MessageBox(prospaWin,"This version of Prospa is readonly, please purchase a full licensed version if you wish to save files.","Read-only warning",MB_OK | MB_ICONEXCLAMATION);
      ErrorMessage("This version of Prospa is read only");
      return(ERR);
   }

// Make sure a variable name has been supplied
   if(var == NULL)
   {
	   ErrorMessage("No variable supplied");
	   return(ERR);
	}	

// Open file for binary write ********************
   FILE *fp;      
   if(!(fp = fopen(fileName,"wb")))
	{
	   ErrorMessage("Can't save file '%s'",fileName);
	   return(ERR);
	}

// Save file owner, type and version *************
   long owner      = 'PROS'; fwrite(&owner,sizeof(long),1,fp);       // File owner (Prospa)
   long type       = 'DATA'; fwrite(&type,sizeof(long),1,fp);        // File type
   long version    = PLOTFILE_VERSION_1_1; 
	fwrite(&version,sizeof(long),1,fp);     // File version

// Check to see if variable is defined *********
   varType = var->GetType();
	if(varType != MATRIX2D && varType != CMATRIX2D &&
      varType != MATRIX3D && varType != CMATRIX3D &&
      varType != MATRIX4D && varType != CMATRIX4D &&
      varType != DMATRIX2D)
	{
      fclose(fp);	
	   ErrorMessage("Variable '%s' is not a matrix",var->GetName());
	   return(ERR);
	}
	if(varType == CMATRIX2D || varType == CMATRIX3D || varType == CMATRIX4D)
	   dataType = COMPLEX_DATA_TYPE;
	else	if(varType == DMATRIX2D)
	   dataType = DOUBLE_DATA_TYPE;
   else
	   dataType = REAL_DATA_TYPE;


// Save data type (real, double or complex) ***************       
   fwrite(&dataType,sizeof(long),1,fp); 
	   
// Save data dimensions ***************************
   long dim1 = var->GetDimX();
   long dim2 = var->GetDimY();
   long dim3 = var->GetDimZ();
   long dim4 = var->GetDimQ();

   fwrite(&dim1,sizeof(long),1,fp); 
   fwrite(&dim2,sizeof(long),1,fp); 
   fwrite(&dim3,sizeof(long),1,fp); 
   fwrite(&dim4,sizeof(long),1,fp); 

// Save data **************************************
   if(varType == MATRIX4D || varType == CMATRIX4D) // 4D data
   {
	   switch(dataType)
	   {
	      case(REAL_DATA_TYPE):
	      {
	         float *data;
	         for(long k = 0; k < var->GetDimQ(); k++)
	         {
	            for(long j = 0; j < var->GetDimZ(); j++)
	            {
		            for(long i = 0; i < var->GetDimY(); i++)
		            {
		               data = VarReal4DMatrix(var)[k][j][i];	   
				         fwrite(data,sizeof(float),var->GetDimX(),fp);
				      }
				   }
            }
	         break;
	      }
	      case(COMPLEX_DATA_TYPE):
	      {
	         complex *cdata;
	         for(long k = 0; k < var->GetDimQ(); k++)
	         {	  
	            for(long j = 0; j < var->GetDimZ(); j++)
	            {	         
		            for(long i = 0; i < var->GetDimY(); i++)
		            {
		               cdata = VarComplex4DMatrix(var)[k][j][i];	   
				         fwrite(cdata,sizeof(complex),var->GetDimX(),fp);
				      }   
				   }
            }
	         break;
	      }
	   }   
   }
   else if(varType == MATRIX3D || varType == CMATRIX3D) // 3D data
   {
	   switch(dataType)
	   {
	      case(REAL_DATA_TYPE):
	      {
	         float *data;
	         for(long j = 0; j < var->GetDimZ(); j++)
	         {
		         for(long i = 0; i < var->GetDimY(); i++)
		         {
		            data = VarReal3DMatrix(var)[j][i];	   
				      fwrite(data,sizeof(float),var->GetDimX(),fp);
				   }
				}
	         break;
	      }
	      case(COMPLEX_DATA_TYPE):
	      {
	         complex *cdata;
	         for(long j = 0; j < var->GetDimZ(); j++)
	         {	         
		         for(long i = 0; i < var->GetDimY(); i++)
		         {
		            cdata = VarComplex3DMatrix(var)[j][i];	   
				      fwrite(cdata,sizeof(complex),var->GetDimX(),fp);
				   }   
				}   
	         break;
	      }
	   }   
   }
   else if(varType == MATRIX2D || varType == CMATRIX2D) // 2D data
   {
	   switch(dataType)
	   {
	      case(REAL_DATA_TYPE):
	      {
	         float *data;
	         for(long i = 0; i < var->GetDimY(); i++)
	         {
	            data = VarRealMatrix(var)[i];	   
			      fwrite(data,sizeof(float),var->GetDimX(),fp);
			   }
	         break;
	      }
	      case(COMPLEX_DATA_TYPE):
	      {
	         complex *cdata;
	         for(long i = 0; i < var->GetDimY(); i++)
	         {
	            cdata = VarComplexMatrix(var)[i];	   
			      fwrite(cdata,sizeof(complex),var->GetDimX(),fp);
			   }      
	         break;
	      }
	   }
	}
   else if(varType == DMATRIX2D) // 2D data double precision
   {
	   switch(dataType)
	   {
	      case(DOUBLE_DATA_TYPE):
	      {
	         double *data;
	         for(long i = 0; i < var->GetDimY(); i++)
	         {
               data = var->GetDMatrix2D()[i];	   
			      fwrite(data,sizeof(double),var->GetDimX(),fp);
			   }
	         break;
	      }
	   }
	}

// Close file ************************************
   fclose(fp);

   return(OK);
}

// Save 2D data which includes x and y axes

short Save2DData(char* pathName, char* fileName, Variable *varMat, Variable *varXAxis, Variable *varYAxis)
{
   short matType;
   long dataType;
   
// Check to see if variable is defined *********
   matType = varMat->GetType();
	if(matType != MATRIX2D && matType != DMATRIX2D && matType != CMATRIX2D)
	{
	   ErrorMessage("Variable '%s' is not a 2D matrix",varMat->GetName());
	   return(ERR);
	}

// Check to see if X axis variables is correctly defined *********
	if(varXAxis->GetType() != MATRIX2D || varXAxis->GetDimX() != 2 || varXAxis->GetDimY() != 1)
	{
	   ErrorMessage("X axis '%s' is not a 1 by 2 matrix",varMat->GetName());
	   return(ERR);
	}

// Check to see if Y axis variables is correctly defined *********
	if(varYAxis->GetType() != MATRIX2D || varYAxis->GetDimX() != 2 || varYAxis->GetDimY() != 1)
	{
	   ErrorMessage("Y axis '%s' is not a 1 by 2 matrix",varMat->GetName());
	   return(ERR);
	}

// Open file for binary write ********************
   FILE *fp;      
   if(!(fp = fopen(fileName,"wb")))
	{
	   ErrorMessage("Can't save file '%s'",fileName);
	   return(ERR);
	}

// Save file owner, type and version *************
   long owner      = 'PROS'; fwrite(&owner,sizeof(long),1,fp);       // File owner (Prospa)
   long type       = 'DATA'; fwrite(&type,sizeof(long),1,fp);        // File type
   long version    = PLOTFILE_VERSION_1_2; 
	fwrite(&version,sizeof(long),1,fp);     // File version

// Check to see if variable is defined *********
	if(matType == CMATRIX2D)
	   dataType = COMPLEX_DATA_TYPE;
	else if(matType == DMATRIX2D)
	   dataType = DOUBLE_DATA_TYPE;
   else
	   dataType = REAL_DATA_TYPE;

// Save data type (real, double or complex) ***************       
   fwrite(&dataType,sizeof(long),1,fp); 
	   
// Save data dimensions ***************************
   long dim1 = varMat->GetDimX();
   long dim2 = varMat->GetDimY();
   long dim3 = varMat->GetDimZ();
   long dim4 = varMat->GetDimQ();

   fwrite(&dim1,sizeof(long),1,fp); 
   fwrite(&dim2,sizeof(long),1,fp); 
   fwrite(&dim3,sizeof(long),1,fp); 
   fwrite(&dim4,sizeof(long),1,fp); 

// Save the axes
   float** xAxis = varXAxis->GetMatrix2D();
   float** yAxis = varXAxis->GetMatrix2D();
   long xAxisSize = 2;
   long yAxisSize = 2;
	fwrite(&xAxisSize,sizeof(long),1,fp);
	fwrite(&yAxisSize,sizeof(long),1,fp);
	fwrite(&xAxis[0],sizeof(float),xAxisSize,fp);
	fwrite(&yAxis[0],sizeof(float),yAxisSize,fp);

// Save data **************************************

   if(matType == MATRIX2D || matType == CMATRIX2D) // 2D data
   {
	   switch(dataType)
	   {
	      case(REAL_DATA_TYPE):
	      {
	         float *data;
	         for(long i = 0; i < varMat->GetDimY(); i++)
	         {
	            data = VarRealMatrix(varMat)[i];	   
			      fwrite(data,sizeof(float),varMat->GetDimX(),fp);
			   }
	         break;
	      }
	      case(COMPLEX_DATA_TYPE):
	      {
	         complex *cdata;
	         for(long i = 0; i < varMat->GetDimY(); i++)
	         {
	            cdata = VarComplexMatrix(varMat)[i];	   
			      fwrite(cdata,sizeof(complex),varMat->GetDimX(),fp);
			   }      
	         break;
	      }
	   }
	}
   else if(matType == DMATRIX2D) // 2D data double precision
   {
	   switch(dataType)
	   {
	      case(DOUBLE_DATA_TYPE):
	      {
	         double *data;
	         for(long i = 0; i < varMat->GetDimY(); i++)
	         {
               data = varMat->GetDMatrix2D()[i];	   
			      fwrite(data,sizeof(double),varMat->GetDimX(),fp);
			   }
	         break;
	      }
	   }
	}

// Close file ************************************
   fclose(fp);

   return(OK);
}

/**********************************************************************************
   Load the contents of file into 'buffer' returning 'fileLength' in bytes.

   Returns: 0 ... all ok
            1 ... can't open file
            2 ... can't allocate memory for buffer
            3 ... couldn't read file in correctly

**********************************************************************************/

short LoadFileIntoBuffer(char *fileName, size_t &fileLength, char **buffer)
{
   FILE *fp;

// Open the file ********************************
   if(!(fp = fopen(fileName,"rb")))
   {
      return(1);
   }

// Get the file length in bytes *****************
   fileLength = GetFileLength(fp);

// Allocate memory for the file data ************
   if(!((*buffer) = new char[fileLength]))
   {
      fclose(fp);
      return(2);
   }

// Read in the file data and store in buffer ****
   if(fread(*buffer,sizeof(char),fileLength,fp) != fileLength*sizeof(char))
   {
      delete [] buffer;
      fclose(fp);
      return(3);
   }

   fclose(fp);
   return(0);
}
