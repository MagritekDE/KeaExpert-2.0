#include "stdafx.h"
#include "import_data_1d.h"
#include "allocate.h"
#include "cArg.h"
#include "control.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "error.h"
#include "files.h"
#include "globals.h"
#include "import_export_utilities.h"
#include "interface.h"
#include "message.h"
#include "plot.h"
#include "plot1dCLI.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/***********************************************************************
*             Routines connected with importing 1D data                *
*                                                                      *
* Import1DDataCLI ......... CLI interface to the 1D import procedures. *
* Import1DDataParameters .. Initialise the import parameters (CLI)     *
* Import1DDataDialog ...... Dialog interface to 1D import procedures.  *
* Import1DHookProc ........ Modification to standard open dialog.      *
* Import1DData ............ Code to actually import the 1D data.       *
*                                                                      *
***********************************************************************/


// Import 1D file wide variables
   
static long importType    = REAL_DATA + ASCII + FLOAT_32 + SPACE_DELIMIT;
static long fileHeader = 0;
static long rowHeader = 0;
static char file[MAX_STR]     = "test";
static bool displayData = false;

/************************************************************************
         CLI interface for importing 1D data              

         Syntax: (x,y) = import1d("filename") or
                   y = import1d("filename")

************************************************************************/
      
int Import1DDataCLI(Interface* itfc ,char args[])
{
   short nrArgs;
   FILE *fp;
   long length = 0; // Number of elements to read in (0 means ignore this parameter)
   
// Get filename from user *************
   if((nrArgs = ArgScan(itfc,args,1,"filename, [length]","ee","sl",file,&length)) < 0)
     return(nrArgs);  

// Check for file existence ***********
   if(!abortOnError)
   {
		if(!(fp = fopen(file,"rb")))
		{
         itfc->nrRetValues = 1;
         itfc->retVar[1].SetType(NULL_VARIABLE);
		   return(OK);
		} 
		fclose(fp);
	}
	
// Import data ************************
	if((importType & XYDATA) | (importType & X_COMP_DATA))
   {
      if(Import1DData(itfc,false,file,importType,"ans1","ans2",&itfc->retVar[1],&itfc->retVar[2],0) == ERR)
         return(ERR);
      itfc->nrRetValues  = 2;
   }    
   else // Real or complex
   {
      if(importType & COMP_DATA) length *= sizeof(complex);
      if (importType & REAL_DATA) length *= sizeof(float);
      if (importType & DOUBLE_DATA) length *= sizeof(double);
      if(Import1DData(itfc,false,file,importType,"","ans",(Variable*)0,&itfc->retVar[1],length) == ERR)
         return(ERR);
      itfc->nrRetValues  = 1;
   }
   
   return(OK);
}

/************************************************************************
          Select import parameters for import1d command    

          Syntax: import1dpar(parameter, value, ...)
************************************************************************/

int Import1DDataParameters(Interface* itfc ,char args[])
{
   static char xyrc[50]      = "real";
   static char ab[50]        = "ascii";
   static char fls[50]       = "float";
   static char delimit[50]   = "space";
   static char machine[50]   = "win";
   static char mode[50]      = "overwrite";
	CArg carg;

// Initialize prompt values based on information in importType **

   InitialiseImportExportPrompts(importType, xyrc, ab, fls, delimit, machine);

// Prompt user if no arguments supplied **************************

   short nrArgs = carg.Count((char*)args);
   
   if(nrArgs == 0) // Help mode
   {
      TextMessage("\n\n   PARAMETER           VALUE                 DEFAULT\n\n");
      TextMessage("   ab .......... ascii/binary .................... %s\n",ab);
      TextMessage("   xyrc ........ xydata/real/double/complex ...... %s\n",xyrc);
      TextMessage("   fls ......... float/double/long/short ......... %s\n",fls);
      TextMessage("   machine ..... bigend/littleend ................ %s\n",machine);
      TextMessage("   delimiter ... space/tab/comma ................. %s\n",delimit);
      TextMessage("   fileheader .. number .......................... %ld\n",fileHeader);      
      return(0);
   }

// Extract arguments (they should be supplied in pairs) **********     
   if(ExtractImportExportArguments(itfc, args, nrArgs, xyrc, ab, fls, delimit, machine, mode, fileHeader,rowHeader) == ERR)
      return(ERR);
	
// Work out type from passed strings
   if((importType = DetermineDataType(xyrc, ab, fls, delimit, machine, mode)) == ERR)
      return(ERR);
  
   return(OK);
}


/************************************************************************
*           Imports a file using a modified Open File dialog            *
************************************************************************/

UINT Import1DHookProc(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam);

short Import1DDataDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index = 1;
   char oldDir[MAX_PATH];

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);


   if(IsPlacesBarHidden())
   {
      err = FileDialog(hWnd, true, pathName, fileName, 
                               "Import 1D data", Import1DHookProc, "IMPORT1DDLG",
                               templateFlag, 1, &index, "All files", "*");
   }
   else
   {
      err = FileDialog(hWnd, true, pathName, fileName, 
                               "Import 1D data", Import1DHookProc, "IMPORT1DDLG2",
                               templateFlag, 1, &index, "All files", "*");
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

UINT Import1DHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   static char xName[50] = "x" ,yName[50] = "y";
   char str[50];
   
	switch (message)
	{
		case(WM_INITDIALOG):
		{
			SendDlgItemMessage(hWnd,ID_ASCII_FILE,BM_SETCHECK,(importType & ASCII),0); 
			SendDlgItemMessage(hWnd,ID_BINARY_FILE,BM_SETCHECK,(importType & BINARY),0); 
			SendDlgItemMessage(hWnd,ID_XY_DATA,BM_SETCHECK,(importType & XYDATA),0); 
			SendDlgItemMessage(hWnd,ID_X_COMPLEX_DATA,BM_SETCHECK,(importType & X_COMP_DATA),0); 
			SendDlgItemMessage(hWnd,ID_REAL_DATA,BM_SETCHECK,(importType & REAL_DATA),0); 
			SendDlgItemMessage(hWnd,ID_COMPLEX_DATA,BM_SETCHECK,(importType & COMP_DATA),0); 
			SendDlgItemMessage(hWnd,ID_SPACE_DELIMIT,BM_SETCHECK,(importType & SPACE_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_TAB_DELIMIT,BM_SETCHECK,(importType & TAB_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_COMMA_DELIMIT,BM_SETCHECK,(importType & COMMA_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_32_FLOAT,BM_SETCHECK,(importType & FLOAT_32),0); 
			SendDlgItemMessage(hWnd,ID_32_INTEGER,BM_SETCHECK,(importType & INT_32),0); 
			SendDlgItemMessage(hWnd,ID_16_INTEGER,BM_SETCHECK,(importType & INT_16),0); 
			SendDlgItemMessage(hWnd,ID_BIG_ENDIAN,BM_SETCHECK,(importType & BIG_ENDIAN),0); 

         SendDlgItemMessage(hWnd,ID_DISPLAY_DATA,BM_SETCHECK,displayData,0);

	      SetWindowText(GetDlgItem(hWnd,ID_X_NAME),xName);
	      SetWindowText(GetDlgItem(hWnd,ID_Y_NAME),yName);
         
	      EnableWindow(GetDlgItem(hWnd,ID_COMMA_DELIMIT),(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_TAB_DELIMIT),(importType & ASCII));	         
	      EnableWindow(GetDlgItem(hWnd,ID_SPACE_DELIMIT),(importType & ASCII));			
	      EnableWindow(GetDlgItem(hWnd,ID_32_FLOAT),!(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_32_INTEGER),!(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_16_INTEGER),!(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_BIG_ENDIAN),!(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_FILE_HEADER),!(importType & ASCII));
	      
	      EnableWindow(GetDlgItem(hWnd,ID_X_LBL),(importType & XYDATA) | (importType & X_COMP_DATA));
	      EnableWindow(GetDlgItem(hWnd,ID_X_NAME),(importType & XYDATA) | (importType & X_COMP_DATA));

		   sprintf(str,"%ld",fileHeader);
		   SetDlgItemText(hWnd,ID_FILE_HEADER,str);
		   
	      break;
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
			    // Get file name
			      SendMessage(lpon->hdr.hwndFrom,CDM_GETSPEC ,(WPARAM)MAX_STR,(LPARAM) (LPTSTR)file);

             // Get radio button settings and update importType
       	   	importType = BINARY*(IsDlgButtonChecked(hWnd,ID_BINARY_FILE)) +
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
                            COMMA_DELIMIT*(IsDlgButtonChecked(hWnd,ID_COMMA_DELIMIT));

            // Extract x and y variable names
				   GetDlgItemText(hWnd,ID_X_NAME,xName,50);
				   GetDlgItemText(hWnd,ID_Y_NAME,yName,50);

				 // Get header dimensions (in bytes)
				   GetDlgItemText(hWnd,ID_FILE_HEADER,str,50);
				   sscanf(str,"%ld",&fileHeader);

				// Check for names				
				   if((importType & REAL_DATA) || (importType & COMP_DATA) || (importType & XYDATA) || (importType & X_COMP_DATA))
				   {
				      if(yName[0] == '\0')
				      {
				         MessageDialog(prospaWin,MB_ICONERROR,"Error","y variable not defined");
       	            SetWindowLong(hWnd,DWL_MSGRESULT,1);
                     return(1);
				      }
				   }
				   if(importType & XYDATA)
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
				 
				 // Display the data?
               displayData = IsDlgButtonChecked(hWnd,ID_DISPLAY_DATA);

				// Import the data, reprompt on error
		         reportErrorToDialog = true;
               SetCursor(LoadCursor(NULL,IDC_WAIT)); 

 	            Variable* xVar = AddGlobalVariable( MATRIX2D, xName);
 	            Variable* yVar = AddGlobalVariable( MATRIX2D, yName);
               if(xVar->GetReadOnly())
			      {
				      MessageDialog(prospaWin,MB_ICONERROR,"Error","x variable is read only");
       	         SetWindowLong(hWnd,DWL_MSGRESULT,1);				         
                  return(1);
				   }
               if(yVar->GetReadOnly())
			      {
				      MessageDialog(prospaWin,MB_ICONERROR,"Error","y variable is read only");
       	         SetWindowLong(hWnd,DWL_MSGRESULT,1);				         
                  return(1);
				   }
               Interface itfc;
	       	   if(Import1DData(&itfc,displayData,file,importType,xName,yName,xVar,yVar,0) == ERR)
	       	   {
		            reportErrorToDialog = false; 
                  SetCursor(LoadCursor(NULL,IDC_ARROW)); 		            
	       	      SetWindowLong(hWnd,DWL_MSGRESULT,1);
                  return(1);
	       	   }
		         reportErrorToDialog = false; 
               SetCursor(LoadCursor(NULL,IDC_ARROW)); 	       	   	       	   
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
			      EnableWindow(GetDlgItem(hWnd,ID_FILE_HEADER),false);
               break;     
	       	case(ID_BINARY_FILE):
			      EnableWindow(GetDlgItem(hWnd,ID_REAL_DATA),true); 
			      EnableWindow(GetDlgItem(hWnd,ID_XY_DATA),true);
			      EnableWindow(GetDlgItem(hWnd,ID_32_FLOAT),true);
			      EnableWindow(GetDlgItem(hWnd,ID_32_INTEGER),true);
			      EnableWindow(GetDlgItem(hWnd,ID_16_INTEGER),true);
			      EnableWindow(GetDlgItem(hWnd,ID_BIG_ENDIAN),true);
			      EnableWindow(GetDlgItem(hWnd,ID_FILE_HEADER),true);			      
               break;     
	       	case(ID_REAL_DATA):	         	
	       	case(ID_COMPLEX_DATA):
				   EnableWindow(GetDlgItem(hWnd,ID_X_LBL),false);
				   EnableWindow(GetDlgItem(hWnd,ID_X_NAME),false);
               break;     
	       	case(ID_XY_DATA):
	       	case(ID_X_COMPLEX_DATA):
				   EnableWindow(GetDlgItem(hWnd,ID_X_LBL),true);
				   EnableWindow(GetDlgItem(hWnd,ID_X_NAME),true);
               break;        		         	
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
*                       Import 1D in specified type                     *
*                                                                       *
* Routine structure                                                     *
*                                                                       *
* Is Type Binary?                                                       *
*    Is Type Complex?                                                   *
*        Load Complex Data                                              *
*        Save to Complex Variable                                       *
*        Plot Data if desired                                           *
*        End Complex                                                    *
*    Is Type XY Data?                                                   *
*        Load XY Data                                                   *
*        Save X Data to Real Variable                                   *
*        Save Y Data to Real Variable                                   *                               
*        Plot Data if desired                                           *
*        End XY                                                         *
*    Is Type Y Data?                                                    *
*        Load Y Data                                                    *
*        Save Y Data to Real Variable                                   *
*        Plot Data if desired                                           *
*        End Y                                                          *
*    End Binary                                                         *
* Is Type ASCII?                                                        *
*    Is Type Complex?                                                   *
*        Load Complex ASCII Data                                        *
*        Save to Complex Variable                                       *
*        Plot Data if desired                                           *
*        End Complex                                                    *
*    Is Type XY Data?                                                   *
*        Load XY Data                                                   *
*        Save X Data to Real Variable                                   *
*        Save Y Data to Real Variable                                   *
*        Plot Data if desired                                           *
*        End XY                                                         *
*    Is Type Y Data?                                                    *
*        Load Y Data                                                    *
*        Save Y Data to Real Variable                                   *
*        Plot Data if desired                                           *
*        End Y                                                          *
*    End ASCII                                                         *
************************************************************************/

short Import1DData(Interface *itfc, bool dispData, char file[], long type, char *xName, char *yName, Variable* xVar, Variable* yVar, long length)
{
   float* x  = NULL,*y = NULL;
   double* xd = NULL, *yd = NULL;
   complex *cy = NULL;
   long nrPoints;
   
   if(type & BINARY)
   {
      if(type & COMP_DATA)
      {   
         if((nrPoints = Load1DComplexBinaryDataFromFile(file, type, &cy, fileHeader,length)) == ERR)
            return(ERR);
  
       // Save data in a complex variable
			 yVar->MakeCMatrix2DFromCVector(cy, nrPoints,1);	

       // Remove original data
          FreeCVector(cy);
      
       // If requested, dispData data
          if(dispData)
          {
				 Plot1D::curPlot()->setTitleText(file);// Update title          
	       	 PlotXY(itfc,yName);
          }
      }
      else if(type & XYDATA)
      {
       // Load data from file - data returned in x and y
      
         if((nrPoints = Load1DRealBinaryDataFromFile(file, type, &x, &y, fileHeader,length)) == ERR)
            return(ERR);

       // Save x data in a 1D matrix
			 xVar->MakeMatrix2DFromVector(x, nrPoints,1);	

       // Save y data in a 1D matrix
			 yVar->MakeMatrix2DFromVector(y, nrPoints,1);	

       // Remove original data
          FreeVector(x);
          FreeVector(y);
      
       // If requested, dispData data
          if(dispData)
          {
             char str[MAX_STR];
				 Plot1D::curPlot()->setTitleText(file);
             sprintf(str,"%s,%s",xName,yName);
	          PlotXY(itfc,str);
          }
      } 
      else if(type & REAL_DATA)
      {
       // Load data from file - data returned in x and y
         if((nrPoints = Load1DRealBinaryDataFromFile(file, type, &x, &y, fileHeader,length)) == ERR)
            return(ERR);

       // Save y data in a 1D matrix
			 yVar->MakeMatrix2DFromVector(y, nrPoints,1);	
	
	   // Remove original data
          FreeVector(x);
          FreeVector(y);
                
       // If requested, dispData data
          if(dispData)
          {
				 Plot1D::curPlot()->setTitleText(file);
	          PlotXY(itfc,yName);
          }
      } 
      else if (type & DOUBLE_DATA)
      {
         // Load data from file - data returned in x and y
         if ((nrPoints = Load1DDoubleBinaryDataFromFile(file, type, &xd, &yd, fileHeader, length)) == ERR)
            return(ERR);

         // Save y data in a 1D matrix
         yVar->MakeDMatrix2DFromVector(yd, nrPoints, 1);

         // Remove original data
         FreeDVector(xd);
         FreeDVector(yd);

         // If requested, dispData data
         if (dispData)
         {
            Plot1D::curPlot()->setTitleText(file);
            PlotXY(itfc, yName);
         }
      }
      else
      {
         ErrorMessage("Import of this data type not implemented\n");
         return(ERR);
      }
   }
   else if(type & ASCII)
   {
     if(type & COMP_DATA)
     {
         if((nrPoints = Load1DComplexAsciiDataFromFile(file, type, &cy)) == ERR)
            return(ERR);

      // Save data in a complex variable
			 yVar->MakeCMatrix2DFromCVector(cy, nrPoints,1);	

       // Remove original data
          FreeCVector(cy);
                 
       // If requested, dispData data
         if(dispData)
         {
				Plot1D::curPlot()->setTitleText(file);
	       	PlotXY(itfc,yName);
         }
      }
      else if(type & X_COMP_DATA)
      {
       // Load data from file - data returned in x and y
         if((nrPoints = Load1DXComplexAsciiDataFromFile(file, type, &x, &cy)) == ERR)
            return(ERR);
       
       // Save x data in a 1D matrix
			 xVar->MakeMatrix2DFromVector(x, nrPoints,1);	

       // Save y data in a 1D matrix
			 yVar->MakeCMatrix2DFromCVector(cy, nrPoints,1);	

       // Remove original data
          FreeVector(x);
          FreeCVector(cy);
      
       // If requested, dispData data
          if(dispData)
          {
             char str[MAX_STR];
				 Plot1D::curPlot()->setTitleText(file);
             sprintf(str,"%s,%s",xName,yName);
	          PlotXY(itfc,str);
          }
      } 
      else if(type & XYDATA)
      {
       // Load data from file - data returned in x and y
         if((nrPoints = Load1DRealAsciiDataFromFile(true, file, type, &x, &y)) == ERR)
            return(ERR);
       
       // Save x data in a 1D matrix
			 xVar->MakeMatrix2DFromVector(x, nrPoints,1);	

       // Save y data in a 1D matrix
			 yVar->MakeMatrix2DFromVector(y, nrPoints,1);	

       // Remove original data
          FreeVector(x);
          FreeVector(y);
      
       // If requested, dispData data
          if(dispData)
          {
             char str[MAX_STR];
				 Plot1D::curPlot()->setTitleText(file);
             sprintf(str,"%s,%s",xName,yName);
	          PlotXY(itfc,str);
          }
      } 
      else if(type & REAL_DATA)
      {
       // Load data from file - data returned in x and y      
         if((nrPoints = Load1DRealAsciiDataFromFile(true, file, type, &x, &y)) == ERR)
            return(ERR);      

       // Save y data in a 1D matrix
			 yVar->MakeMatrix2DFromVector(y, nrPoints,1);	
	
	   // Remove original data
          FreeVector(y);
          
       // If requested, dispData data
          if(dispData)
          {
				 Plot1D::curPlot()->setTitleText(file);
				 PlotXY(itfc,yName);
          }      
      } 
      else
      {
         ErrorMessage("Import of this data type not implemented\n");
         return(ERR);
      }
   }
   return(OK);
}

