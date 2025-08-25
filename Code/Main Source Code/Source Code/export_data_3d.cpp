#include "stdafx.h"
#include "export_data_3d.h"
#include "cArg.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "error.h"
#include "files.h"
#include "globals.h"
#include "import_export_utilities.h"
#include "interface.h"
#include "load_save_data.h"
#include "message.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"


/***********************************************************************
*             Routines connected with importing 3D data                *
*                                                                      *
* Export3DDataCLI ......... CLI interface to the 3D export procedures. *
* Export3DDataParameters .. Initialise the export parameters (CLI)     *
* Export3DDataDialog ...... Dialog interface to 3D export procedures.  *
* Export3DHookProc ........ Modification to standard open dialog.      *
* Export3DData ............ Code to actually export the 3D data.       *
*                                                                      *
***********************************************************************/

// Export 3D file wide variables
   
static long exportType    = REAL_DATA + ASCII + FLOAT_32 + SPACE_DELIMIT + OVERWRITE_MODE;
static char file[MAX_STR]     = "test";
static long fileHeader = 0;
static long rowHeader = 0;

/************************************************************************
*                      CLI interface for exporting 2D data              *
************************************************************************/
     
int Export3DDataCLI(Interface* itfc ,char args[])
{
   short nrArgs;
   static CText matrixName;
   
// Get matrix and filename ***********************************
   if((nrArgs = ArgScan(itfc,args,2,"matrix, file","ce","ts",&matrixName,file)) < 0)
      return(nrArgs);  

// Export the data **********************************************
   return(Export3DData(itfc,file,exportType,matrixName.Str()));
}

/************************************************************************
*               Select export parameters for export1d command           *
************************************************************************/

int Export3DDataParameters(Interface* itfc ,char args[])
{
   static char rc[50]        = "real";
   static char ab[50]        = "ascii";
   static char fls[50]       = "float";
   static char delimit[50]   = "space";
   static char machine[50]   = "win";
   static char mode[50]      = "mode";
   CArg carg;

// Initialize prompt values based on information in exportType **
   InitialiseImportExportPrompts(exportType, rc, ab, fls, delimit, machine);

// Prompt user if no arguments supplied **************************
   short nrArgs = carg.Count((char*)args);
   
	itfc->nrRetValues = 0;
   if(nrArgs == 0) // Help mode
   {
      TextMessage("\n\n   PARAMETER           VALUE                 DEFAULT\n\n");
      TextMessage("   ab .......... ascii/binary ............... %s\n",ab);
      TextMessage("   rc .......... real/complex ............... %s\n",rc);
      TextMessage("   fls ......... float/long/short ........... %s\n",fls);
      TextMessage("   machine ..... win/mac .................... %s\n",machine);
      TextMessage("   delimiter ... space/tab/comma ............ %s\n",delimit);
      TextMessage("   fileheader .. number ..................... %ld\n",fileHeader); 
      TextMessage("   mode ........ overwrite/append ..... ..... %s\n",mode);      
      return(0);
   }

// Extract arguments (they should be supplied in pairs) **********  
   if(ExtractImportExportArguments(itfc, args, nrArgs, rc, ab, fls, delimit, machine, mode, fileHeader, rowHeader) == ERR)
      return(ERR);
	
// Work out type from passed strings
   if((exportType = DetermineDataType(rc, ab, fls, delimit, machine, mode)) == ERR)
      return(ERR);

   return(OK);
}

/************************************************************************
*           Exports a 2D file using a modified Save File dialog         *
************************************************************************/

UINT Export3DHookProc(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam);

short Export3DDataDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index;
   char oldDir[MAX_PATH];

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);

   if(IsPlacesBarHidden())
   {
      err = FileDialog(hWnd, false, pathName, fileName, 
                               "Export 3D data", Export3DHookProc, "EXPORT3DDLG", 
                                templateFlag, 1, &index,  "All files", "*");
   }
   else
   {
      err = FileDialog(hWnd, false, pathName, fileName, 
                               "Export 3D data", Export3DHookProc, "EXPORT3DDLG2", 
                                templateFlag, 1, &index,  "All files", "*");
   }

// Make sure returned path has standard form
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,pathName); 
   
// Restore the directory
   SetCurrentDirectory(oldDir);

   return(err);
}

/************************************************************************
*           Hook procedure to process events from import dialog         *
************************************************************************/

UINT Export3DHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   static char matrixName[MAX_NAME] = "m1";
   
	switch (message)
	{
		case(WM_INITDIALOG):
		{
       // Initialise radio buttons

			SendDlgItemMessage(hWnd,ID_ASCII_FILE,BM_SETCHECK,(exportType & ASCII),0); 
			SendDlgItemMessage(hWnd,ID_BINARY_FILE,BM_SETCHECK,(exportType & BINARY),0); 
			SendDlgItemMessage(hWnd,ID_REAL_DATA,BM_SETCHECK,(exportType & REAL_DATA),0); 
			SendDlgItemMessage(hWnd,ID_COMPLEX_DATA,BM_SETCHECK,(exportType & COMP_DATA),0); 
			SendDlgItemMessage(hWnd,ID_SPACE_DELIMIT,BM_SETCHECK,(exportType & SPACE_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_TAB_DELIMIT,BM_SETCHECK,(exportType & TAB_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_COMMA_DELIMIT,BM_SETCHECK,(exportType & COMMA_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_32_FLOAT,BM_SETCHECK,(exportType & FLOAT_32),0); 
			SendDlgItemMessage(hWnd,ID_32_INTEGER,BM_SETCHECK,(exportType & INT_32),0); 
			SendDlgItemMessage(hWnd,ID_16_INTEGER,BM_SETCHECK,(exportType & INT_16),0); 
			SendDlgItemMessage(hWnd,ID_BIG_ENDIAN,BM_SETCHECK,(exportType & BIG_ENDIAN),0); 
	
	      EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),(exportType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),(exportType & ASCII));	         
	      EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),(exportType & ASCII));			
	      EnableWindow(GetDlgItem(hWnd,ID_32_FLOAT),!(exportType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_32_INTEGER),!(exportType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_16_INTEGER),!(exportType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_BIG_ENDIAN),!(exportType & ASCII));

      // Initialize combo box with variable list
      
	      Variable *var = &globalVariable;
         short type;
	      
	      while(var)
	      {
	         var = var->GetNext(type);
	         if(!var) break;
            if(!var->GetVisible()) continue;
	         if(type == MATRIX3D || type == CMATRIX3D)
            {
               SetDlgItemText(hWnd,ID_MATRIX_NAME,var->GetName());
               SendDlgItemMessage(hWnd,ID_MATRIX_NAME,CB_ADDSTRING,0,(LPARAM) var->GetName());             
            }
         }    
         SendDlgItemMessage(hWnd, ID_MATRIX_NAME, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);           	
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

             // Get radio button settings and update exportType


       	   	exportType = BINARY*(IsDlgButtonChecked(hWnd,ID_BINARY_FILE)) +
       	   	             ASCII*(IsDlgButtonChecked(hWnd,ID_ASCII_FILE)) +
       	   	             BIG_ENDIAN*(IsDlgButtonChecked(hWnd,ID_BIG_ENDIAN)) +
       	   	             FLOAT_32*(IsDlgButtonChecked(hWnd,ID_32_FLOAT)) +
       	                   INT_32*(IsDlgButtonChecked(hWnd,ID_32_INTEGER)) +
       	                   INT_16*(IsDlgButtonChecked(hWnd,ID_16_INTEGER)) +
       	                   REAL_DATA*(IsDlgButtonChecked(hWnd,ID_REAL_DATA)) +
       	                   COMP_DATA*(IsDlgButtonChecked(hWnd,ID_COMPLEX_DATA)) +
	       	                SPACE_DELIMIT*(IsDlgButtonChecked(hWnd,ID_SPACE_DELIMIT)) +
                            TAB_DELIMIT*(IsDlgButtonChecked(hWnd,ID_TAB_DELIMIT)) +
                            COMMA_DELIMIT*(IsDlgButtonChecked(hWnd,ID_COMMA_DELIMIT)) + OVERWRITE_MODE;

            // Extract matrixName variable name

				   GetDlgItemText(hWnd,ID_MATRIX_NAME,matrixName,MAX_NAME);

				// Check for name error
				
			      if(matrixName[0] == '\0')
			      {
			         MessageDialog(prospaWin,MB_ICONERROR,"Error","Matrix variable not defined");
    	            SetWindowLong(hWnd,DWL_MSGRESULT,1);
			         return(1);
			      }

			    // Get filename

			      SendMessage(lpon->hdr.hwndFrom,CDM_GETSPEC ,(WPARAM)MAX_STR,(LPARAM) (LPTSTR)file);

				// Export the data, reprompt on error
	            
	            reportErrorToDialog = true;
               Interface itfc;
	       	   if(Export3DData(&itfc,file,exportType,matrixName) == ERR)
	       	   {
	               reportErrorToDialog = false;
	       	      SetWindowLong(hWnd,DWL_MSGRESULT,1);
	       	      return(1);
	       	   }
	            reportErrorToDialog = false;	       	   
	         	break;
			   }
		   }
         break;
		}
		
		case(WM_COMMAND): // Make sure the correct controls are enabled
		{
		   switch(LOWORD(wParam))
		   {
	       	case(ID_ASCII_FILE):
			      EnableWindow(GetDlgItem(hWnd,ID_32_FLOAT),false);
			      EnableWindow(GetDlgItem(hWnd,ID_32_INTEGER),false);
			      EnableWindow(GetDlgItem(hWnd,ID_16_INTEGER),false);
			      EnableWindow(GetDlgItem(hWnd,ID_BIG_ENDIAN),false);
			      EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),true);
				   EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),true);
				   EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),true);
	         	return(0);
	       	case(ID_BINARY_FILE):
			      EnableWindow(GetDlgItem(hWnd,ID_REAL_DATA),true); 
			      EnableWindow(GetDlgItem(hWnd,ID_XY_DATA),true);
			      EnableWindow(GetDlgItem(hWnd,ID_32_FLOAT),true);
			      EnableWindow(GetDlgItem(hWnd,ID_32_INTEGER),true);
			      EnableWindow(GetDlgItem(hWnd,ID_16_INTEGER),true);
			      EnableWindow(GetDlgItem(hWnd,ID_BIG_ENDIAN),true);
		         EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),false);
			      EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),false);
			      EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),false);
	         	return(0);
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
*                  Export 3D data as specified type                     *
************************************************************************/

short Export3DData(Interface *itfc, char fileName[], long type, char* matrixName)
{
   Variable *varMat;
   float ***rMat = NULL;
   complex ***cMat = NULL;
   long xSize,ySize,zSize;
   short varType;

// Find the variable specified by 'matrixName'

   varMat = GetVariable(itfc,ALL_VAR,matrixName,varType);
	if(!varMat)
	{
	   ErrorMessage("Variable '%s' is not defined",matrixName);
	   return(ERR);
	}

// Get data from variables *******************************************

   if(type & REAL_DATA)
   {
		if(varMat->GetType() != MATRIX3D)
		{
		   ErrorMessage("Variable '%s' is not a real 3D matrix",matrixName);
		   return(ERR);
		}		
      rMat = VarReal3DMatrix(varMat);
   }

   else if(type & COMP_DATA)
   {
		if(varMat->GetType() != CMATRIX3D)
		{
		   ErrorMessage("Variable '%s' is not a complex 3D matrix",matrixName);
		   return(ERR);
		}		
      cMat = VarComplex3DMatrix(varMat);
   }	 

// Extract data set dimensions

	xSize = VarWidth(varMat);
	ySize = VarHeight(varMat);
	zSize = VarDepth(varMat);

// Save the data
	   			   
   if(type & BINARY)
   {
      if(Save3DDataToBinaryFile(fileName, type, rMat, cMat, xSize, ySize, zSize) == ERR)
         return(ERR);
   }
   else if(type & ASCII)
   {
      if(Save3DDataToAsciiFile(fileName, type, rMat, cMat, xSize, ySize, zSize) == ERR)
         return(ERR);
   }
	itfc->nrRetValues = 0;
   return(OK);
}


