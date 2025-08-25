#include "stdafx.h"
#include "export_data_1d.h"
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

/**********************************************************************
*             Routines connected with exporting 1D data               *
*                                                                     *
* Export1DDataCLI ... CLI interface to the 1D import procedures.      *
* Export1DDataParameters ... Initialise the import parameters (CLI)   *
* Export1DDataDialog .... Dialog interface to 1D export procedures.   *
* Export1DHookProc ..... Modification to standard open dialog.        *
* Export1DData ......... Code to actually import the 1D data.         *
*                                                                     *
**********************************************************************/


// Export 1D file wide variables
   
static long exportType  = REAL_DATA + ASCII + FLOAT_32 + SPACE_DELIMIT + OVERWRITE_MODE;
static long fileHeader = 0;
static long rowHeader = 0; // Not used in 1D
static char file[MAX_STR]     = "test"; 

/************************************************************************
*                      CLI interface for exporting 1D data              *
************************************************************************/
      
int Export1DDataCLI(Interface* itfc ,char args[])
{
   short nrArgs;
   static CText xVarName = "x";
   static CText yVarName = "y";
   CArg carg;

// Count number of arguments ***********************************
   nrArgs = carg.Count(args);

// Arguments extracted depends on number passed ****************
   if(nrArgs == 2)
   {
      if((nrArgs = ArgScan(itfc,args,2,"y, file","ce","ts",&yVarName,file)) < 0)
        return(nrArgs);  
   }
   else if(nrArgs == 3)
   {
     if((nrArgs = ArgScan(itfc,args,3,"x, y, file","cce","tts",&xVarName,&yVarName,file)) < 0)
        return(nrArgs);  
   }
   else
   {
      TextMessage("\n\n   arguments: [x], y, file\n");
      return(OK);
   }

// Export the data **********************************************
   return(Export1DData(itfc,file,exportType,xVarName.Str(),yVarName.Str()));
}

/************************************************************************
*               Select export parameters for export1d command           *
************************************************************************/

int Export1DDataParameters(Interface* itfc ,char args[])
{
   static char xyrc[50]      = "real";
   static char ab[50]        = "ascii";
   static char fls[50]       = "float";
   static char delimit[50]   = "space";
   static char machine[50]   = "win";
   static char mode[50]      = "overwrite";
   CArg carg;

// Initialize prompt values based on information in exportType **
   InitialiseImportExportPrompts(exportType, xyrc, ab, fls, delimit, machine);

// Prompt user if no arguments supplied **************************
   short nrArgs = carg.Count((char*)args);
	itfc->nrRetValues = 0;
   if(nrArgs == 0) // Help mode
   {
      TextMessage("\n\n   PARAMETER           VALUE                 DEFAULT\n\n");
      TextMessage("   ab .......... ascii/binary ........................ %s\n",ab);
      TextMessage("   xyrc ........ xydata/real/complex/xcomplex ........ %s\n",xyrc);
      TextMessage("   fls ......... float/double/long/short ............. %s\n",fls);
      TextMessage("   machine ..... win/mac ............................. %s\n",machine);
      TextMessage("   delimiter ... space/tab/comma ..................... %s\n",delimit);
      TextMessage("   fileheader .. number .............................. %ld\n",fileHeader);  
      TextMessage("   mode ........ overwrite/append .................... %s\n",mode);      
      return(0);
   }

// Extract arguments (they should be supplied in pairs) **********  
   if(ExtractImportExportArguments(itfc, args, nrArgs, xyrc, ab, fls, delimit, machine, mode, fileHeader, rowHeader) == ERR)
      return(ERR);
	
// Work out type from passed strings
   if((exportType = DetermineDataType(xyrc, ab, fls, delimit, machine, mode)) == ERR)
      return(ERR);
  
   return(OK);
}


/************************************************************************
*           Export a file using a modified Save File dialog             *
************************************************************************/

UINT  Export1DHookProc(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam);

short Export1DDataDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index;
   char oldDir[MAX_PATH];

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);

   if(IsPlacesBarHidden())
   {
      err = FileDialog(hWnd, false, pathName, fileName, 
                       "Export 1D data", Export1DHookProc, "EXPORT1DDLG", templateFlag, 1, &index, "All files","*");
   }
   else
   {
      err = FileDialog(hWnd, false, pathName, fileName, 
                       "Export 1D data", Export1DHookProc, "EXPORT1DDLG2", templateFlag, 1, &index, "All files","*");
   }

// Make sure returned path has standard form
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,pathName);  
   
// Restore the directory
   SetCurrentDirectory(oldDir);

   return(err);
}   


/************************************************************************
*           Hook procedure to process events from export dialog         *
************************************************************************/

UINT  Export1DHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   static char xName[50] = "x" ,yName[50] = "y";

	switch (message)
	{
		case(WM_INITDIALOG):
		{
       // Initialise radio buttons

			SendDlgItemMessage(hWnd,ID_ASCII_FILE,BM_SETCHECK,(exportType & ASCII),0); 
			SendDlgItemMessage(hWnd,ID_BINARY_FILE,BM_SETCHECK,(exportType & BINARY),0); 
			SendDlgItemMessage(hWnd,ID_XY_DATA,BM_SETCHECK,(exportType & XYDATA),0); 
			SendDlgItemMessage(hWnd,ID_REAL_DATA,BM_SETCHECK,(exportType & REAL_DATA),0); 
			SendDlgItemMessage(hWnd,ID_COMPLEX_DATA,BM_SETCHECK,(exportType & COMP_DATA),0); 
			SendDlgItemMessage(hWnd,ID_X_COMPLEX_DATA,BM_SETCHECK,(exportType & X_COMP_DATA),0); 
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
	      
	         
	      EnableWindow(GetDlgItem(hWnd,ID_X_LBL),(exportType & XYDATA) | (exportType & X_COMP_DATA));
	      EnableWindow(GetDlgItem(hWnd,ID_X_NAME),(exportType & XYDATA) | (exportType & X_COMP_DATA));
	      
      // Initialize combo box with variable list
      
	      Variable *var = &globalVariable;
         short type;
	      
	      while(var)
	      {
	         var = var->GetNext(type);
	         if(!var) break;
            if(!var->GetVisible()) continue;
	         if(type == MATRIX2D || type == CMATRIX2D)
            {
	            if(var->GetDimX() == 1 || var->GetDimY() == 1)
	            {
                  SendDlgItemMessage(hWnd,ID_X_NAME,CB_ADDSTRING,0,(LPARAM) var->GetName());
                  SendDlgItemMessage(hWnd,ID_Y_NAME,CB_ADDSTRING,0,(LPARAM) var->GetName());
               }               
            }
         }    
             
         if(SendDlgItemMessage(hWnd, ID_X_NAME, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)"x") == CB_ERR)         
            SendDlgItemMessage(hWnd, ID_X_NAME, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);         

         if(SendDlgItemMessage(hWnd, ID_Y_NAME, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)"y") == CB_ERR)         
            SendDlgItemMessage(hWnd, ID_Y_NAME, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);         
         	      
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
       	                   XYDATA*(IsDlgButtonChecked(hWnd,ID_XY_DATA)) +
       	                   REAL_DATA*(IsDlgButtonChecked(hWnd,ID_REAL_DATA)) +
       	                   COMP_DATA*(IsDlgButtonChecked(hWnd,ID_COMPLEX_DATA)) +
       	                   X_COMP_DATA*(IsDlgButtonChecked(hWnd,ID_X_COMPLEX_DATA)) +
	       	                SPACE_DELIMIT*(IsDlgButtonChecked(hWnd,ID_SPACE_DELIMIT)) +
                            TAB_DELIMIT*(IsDlgButtonChecked(hWnd,ID_TAB_DELIMIT)) +
                            COMMA_DELIMIT*(IsDlgButtonChecked(hWnd,ID_COMMA_DELIMIT)) + OVERWRITE_MODE;

            // Extract x and y variable names
 
 				   GetDlgItemText(hWnd,ID_X_NAME,xName,50);
				   GetDlgItemText(hWnd,ID_Y_NAME,yName,50);
				
				// Check for names
				
				   if((exportType & REAL_DATA) || (exportType & COMP_DATA) || (exportType & XYDATA) || (exportType & X_COMP_DATA))
				   {
				      if(yName[0] == '\0')
				      {
				         MessageDialog(prospaWin,MB_ICONERROR,"Error","y variable not defined");
       	            SetWindowLong(hWnd,DWL_MSGRESULT,1);
				         return(1);
				      }
				   }
				   if(exportType & XYDATA)
				   {
				      if(xName[0] == '\0')
				      {
				         MessageDialog(prospaWin,MB_ICONERROR,"Error","x variable not defined");
       	            SetWindowLong(hWnd,DWL_MSGRESULT,1);				         
				         return(1);
				      }
				   }
				   				   
			    // Get filename
			    
			      SendMessage(lpon->hdr.hwndFrom,CDM_GETSPEC,(WPARAM)MAX_STR,(LPARAM) (LPTSTR)file);

				// Export the data, reprompt on error

	            reportErrorToDialog = true;
               Interface itfc;
	       	   if(Export1DData(&itfc,file,exportType,xName,yName) == ERR)
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
			      if(!IsDlgButtonChecked(hWnd,ID_REAL_DATA))
			      {
			         EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),true);
				      EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),true);
				      EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),true);
				   }				   
	         	return(0);
	       	case(ID_BINARY_FILE):
			      EnableWindow(GetDlgItem(hWnd,ID_32_FLOAT),true);
			      EnableWindow(GetDlgItem(hWnd,ID_32_INTEGER),true);
			      EnableWindow(GetDlgItem(hWnd,ID_16_INTEGER),true);
			      EnableWindow(GetDlgItem(hWnd,ID_BIG_ENDIAN),true);
		         EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),false);
			      EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),false);
			      EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),false);
	         	return(0);
	         case(ID_XY_DATA):

			      if(!IsDlgButtonChecked(hWnd,ID_BINARY_FILE))
			      {
			         EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),true);	         
			         EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),true);
			         EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),true);	
			      }
			      EnableWindow(GetDlgItem(hWnd,ID_X_LBL),true);
			      EnableWindow(GetDlgItem(hWnd,ID_X_NAME),true);			      
			      return(0);         
	         case(ID_COMPLEX_DATA):
			      if(!IsDlgButtonChecked(hWnd,ID_BINARY_FILE))
			      {
			         EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),true);	         
			         EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),true);
			         EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),true);
			      }         
			      EnableWindow(GetDlgItem(hWnd,ID_X_LBL),false);
			      EnableWindow(GetDlgItem(hWnd,ID_X_NAME),false);	
			      return(0);	
	         case(ID_X_COMPLEX_DATA):
			      if(!IsDlgButtonChecked(hWnd,ID_BINARY_FILE))
			      {
			         EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),true);	         
			         EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),true);
			         EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),true);
			      }         
			      EnableWindow(GetDlgItem(hWnd,ID_X_LBL),true);
			      EnableWindow(GetDlgItem(hWnd,ID_X_NAME),true);	
			      return(0);	
	         case(ID_REAL_DATA):
			      EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),false);
			      EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),false);	         
			      EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),false);	         
			      EnableWindow(GetDlgItem(hWnd,ID_X_LBL),false);
			      EnableWindow(GetDlgItem(hWnd,ID_X_NAME),false);	
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
*                      Export 1D data in specified type                 
*
* Last modification 23/2/07 - fixed error reporting for incorrect
* vector types.
*
************************************************************************/

short Export1DData(Interface *itfc, char fileName[], long type, char *xName, char *yName)
{
   Variable *varX = NULL,*varY = NULL;
   float *x = NULL,*y = NULL;
   complex *yc = NULL;
   long size;
   short varType;

// Get data from variables *******************************************

   if((type & XYDATA) || (type & X_COMP_DATA))
   {
		varX = GetVariable(itfc, ALL_VAR,xName,varType);
		if(!varX)
		{
		   ErrorMessage("Variable '%s' is not defined",xName);
		   return(ERR);
		}
		if(varX->GetType() == MATRIX2D && VarRowSize(varX) != 1)
		{
		   ErrorMessage("Variable '%s' is not a row vector",xName);
		   return(ERR);
		}
		if(varX->GetType() != MATRIX2D || VarRowSize(varX) != 1)
		{
		   ErrorMessage("Variable '%s' is not a real vector",xName);
		   return(ERR);
		}	
      x = VarRealMatrix(varX)[0];
   }
   
   if((type & XYDATA) || (type & REAL_DATA))
   {
		varY = GetVariable(itfc,ALL_VAR,yName,varType);
		if(!varY)
		{
		   ErrorMessage("Variable '%s' is not defined",yName);
		   return(ERR);
		}
		if(varY->GetType() == MATRIX2D && VarRowSize(varY) != 1)
		{
		   ErrorMessage("Variable '%s' is not a row vector",yName);
		   return(ERR);
		}
		if(varY->GetType() != MATRIX2D || VarRowSize(varY) != 1)
		{
		   ErrorMessage("Variable '%s' is not a real vector",yName);
		   return(ERR);
		}		
      y = VarRealMatrix(varY)[0];

      if(varX && (varY->GetDimX() != varX->GetDimX()))
		{
		   ErrorMessage("X and Y variables (%s,%s) have different sizes",xName,yName);
		   return(ERR);
		}		
   }
   else if((type & COMP_DATA) || (type & X_COMP_DATA))
   {
		varY = GetVariable(itfc,ALL_VAR,yName,varType);
		if(!varY)
		{
		   ErrorMessage("Variable '%s' is not defined",yName);
		   return(ERR);
		}
		if(varY->GetType() == CMATRIX2D && VarRowSize(varY) != 1)
		{
		   ErrorMessage("Variable '%s' is not a row vector",yName);
		   return(ERR);
		}
		if(varY->GetType() != CMATRIX2D || VarRowSize(varY) != 1)
		{
		   ErrorMessage("Variable '%s' is not a complex vector",yName);
		   return(ERR);
		}
      yc = VarComplexMatrix(varY)[0];
   } 
      
   size = VarColSize(varY);  
	      
   if(type & BINARY)
   {
      if(Save1DDataToBinaryFile(fileName, type, x, y, yc, size) == ERR)
         return(ERR);
   }
   else if(type & ASCII)
   {
      if(Save1DDataToAsciiFile(fileName, type, x, y, yc, size) == ERR)
         return(ERR);
   }

   return(OK);
}
 
 