#include "stdafx.h"
#include "import_data_3d.h"
#include "allocate.h"
#include "cArg.h"
#include "control.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "error.h"
#include "files.h"
#include "globals.h"
#include "import_export_utilities.h"
#include "import_data_2d.h"
#include "interface.h"
#include "load_save_data.h"
#include "message.h"
#include "plot.h"
#include "plot2dCLI.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/***********************************************************************
*             Routines connected with importing 3D data                *
*                                                                      *
* Import3DDataCLI ......... CLI interface to the 3D import procedures. *
* Import3DDataParameters .. Initialise the import parameters (CLI)     *
* Import3DDataDialog ...... Dialog interface to 3D import procedures.  *
* Import3DHookProc ........ Modification to standard open dialog.      *
* Import3DData ............ Code to actually import the 3D data.       *
*                                                                      *
***********************************************************************/


// Import 3D file wide variables
   
static long importType    = REAL_DATA + ASCII + FLOAT_32 + SPACE_DELIMIT;
static long fileHeader = 0;
static long rowHeader = 0;
static char file[MAX_STR]     = "test";
static bool displayData = false;

float ***ConvertBufferToReal3DMatrix(char *buffer, long xsize, long ysize, long zsize, long type);
complex ***ConvertBufferToComplex3DMatrix(char *buffer, long xsize, long ysize, long zsize, long type);

/************************************************************************
*                      CLI interface for importing 3D data              *
************************************************************************/
      
int Import3DDataCLI(Interface* itfc ,char args[])
{
   short nrArgs;
   long xsize,ysize,zsize;
   CText fileName;
   FILE *fp;
   
// Get filename from user *************
   if((nrArgs = ArgScan(itfc,args,4,"filename, width, height, depth","eeee","tlll", &fileName, &xsize, &ysize, &zsize)) < 0)
     return(nrArgs);  

// Check for file existence ***********
   if(!abortOnError)
   {
		if(!(fp = fopen(fileName.Str(),"rb")))
		{
         itfc->retVar[1].SetType(NULL_VARIABLE);
         itfc->nrRetValues = 1;
		   return(OK);
		} 
		fclose(fp);
	}
	
// Import data ************************
   return(Import3DData(itfc,false,fileName.Str(),importType,"",xsize,ysize,zsize,true));
}

/************************************************************************
*               Select import parameters for import2d command           *
************************************************************************/

int Import3DDataParameters(Interface* itfc ,char args[])
{
   static char rc[50]        = "real";
   static char ab[50]        = "ascii";
   static char fls[50]       = "float";
   static char delimit[50]   = "space";
   static char machine[50]   = "win";
   static char mode[50]      = "overwrite";
	CArg carg;

// Initialize prompt values based on information in importType **

   InitialiseImportExportPrompts(importType, rc, ab, fls, delimit, machine);

// Prompt user if no arguments supplied **************************

   short nrArgs = carg.Count((char*)args);
   
   if(nrArgs == 0) // Help mode
   {
      TextMessage("\n\n   PARAMETER           VALUE                 DEFAULT\n\n");
      TextMessage("   ab .......... ascii/binary ............... %s\n",ab);
      TextMessage("   xyrc ........ xydata/real/complex ........ %s\n",rc);
      TextMessage("   fls ......... float/double/long/short .... %s\n",fls);
      TextMessage("   machine ..... win/mac .................... %s\n",machine);
      TextMessage("   delimiter ... space/tab/comma ............ %s\n",delimit);
      TextMessage("   fileheader .. number ..................... %ld\n",fileHeader);  
      TextMessage("   rowheader ... number ..................... %ld\n",rowHeader);
      return(0);
   }

// Extract arguments (they should be supplied in pairs) **********  
   
   if(ExtractImportExportArguments(itfc, args, nrArgs, rc, ab, fls, delimit, machine, mode, fileHeader,rowHeader) == ERR)
      return(ERR);
	
// Work out type from passed strings

   if((importType = DetermineDataType(rc, ab, fls, delimit, machine, mode)) == ERR)
      return(ERR);
  
   return(OK);
}

/************************************************************************
*           Imports a 3D file using a modified Open File dialog         *
************************************************************************/

UINT Import3DHookProc(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam);

short Import3DDataDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index = 1;
   char oldDir[MAX_PATH];

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);

   if(IsPlacesBarHidden())
   {
      err = FileDialog(hWnd, true, pathName, fileName, 
                               "Import 3D data", Import3DHookProc, "IMPORT3DDLG",
                               templateFlag, 1, &index, "All files", "*");
   }
   else
   {
      err = FileDialog(hWnd, true, pathName, fileName, 
                               "Import 3D data", Import3DHookProc, "IMPORT3DDLG2",
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
*        Hook procedure to process events from 3D import dialog         *
************************************************************************/

UINT Import3DHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   static char matrixName[20] = "m1";
   static char xsizeStr[20] = "256";
   static char ysizeStr[20] = "256";
   static char zsizeStr[20] = "256";
   static char str[20];
   
	switch (message)
	{
		case(WM_INITDIALOG):
		{
			SendDlgItemMessage(hWnd,ID_ASCII_FILE,BM_SETCHECK,(importType & ASCII),0); 
			SendDlgItemMessage(hWnd,ID_BINARY_FILE,BM_SETCHECK,(importType & BINARY),0); 
			SendDlgItemMessage(hWnd,ID_REAL_DATA,BM_SETCHECK,(importType & REAL_DATA),0); 
			SendDlgItemMessage(hWnd,ID_COMPLEX_DATA,BM_SETCHECK,(importType & COMP_DATA),0); 
			SendDlgItemMessage(hWnd,ID_SPACE_DELIMIT,BM_SETCHECK,(importType & SPACE_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_TAB_DELIMIT,BM_SETCHECK,(importType & TAB_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_COMMA_DELIMIT,BM_SETCHECK,(importType & COMMA_DELIMIT),0); 
			SendDlgItemMessage(hWnd,ID_32_FLOAT,BM_SETCHECK,(importType & FLOAT_32),0); 
			SendDlgItemMessage(hWnd,ID_32_INTEGER,BM_SETCHECK,(importType & INT_32),0); 
			SendDlgItemMessage(hWnd,ID_16_INTEGER,BM_SETCHECK,(importType & INT_16),0); 
			SendDlgItemMessage(hWnd,ID_BIG_ENDIAN,BM_SETCHECK,(importType & BIG_ENDIAN),0); 

	      SetWindowText(GetDlgItem(hWnd,ID_MATRIX_NAME),matrixName);

         SendDlgItemMessage(hWnd,ID_DISPLAY_DATA,BM_SETCHECK,displayData,0);

		   sprintf(str,"%ld",fileHeader);
		   SetDlgItemText(hWnd,ID_FILE_HEADER,str);
         sprintf(str,"%ld",rowHeader);
         SetDlgItemText(hWnd,ID_ROW_HEADER,str);

		   SetDlgItemText(hWnd,ID_X_SIZE,xsizeStr);
		   SetDlgItemText(hWnd,ID_Y_SIZE,ysizeStr);
		   SetDlgItemText(hWnd,ID_Z_SIZE,zsizeStr);
         
	      EnableWindow(GetDlgItem(hWnd,ID_32_FLOAT),!(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_32_INTEGER),!(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_16_INTEGER),!(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_BIG_ENDIAN),!(importType & ASCII));
	      EnableWindow(GetDlgItem(hWnd,ID_FILE_HEADER),!(importType & ASCII));
         EnableWindow(GetDlgItem(hWnd,ID_ROW_HEADER),!(importType & ASCII));

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
               long xsize,ysize,zsize;
			      
             // Get radio button settings and update importType
       	   	importType = BINARY*(IsDlgButtonChecked(hWnd,ID_BINARY_FILE)) +
       	   	             ASCII*(IsDlgButtonChecked(hWnd,ID_ASCII_FILE)) +
       	   	             BIG_ENDIAN*(IsDlgButtonChecked(hWnd,ID_BIG_ENDIAN)) +
       	   	             FLOAT_32*(IsDlgButtonChecked(hWnd,ID_32_FLOAT)) +
       	                   INT_32*(IsDlgButtonChecked(hWnd,ID_32_INTEGER)) +
       	                   INT_16*(IsDlgButtonChecked(hWnd,ID_16_INTEGER)) +
       	                   REAL_DATA*(IsDlgButtonChecked(hWnd,ID_REAL_DATA)) +
       	                   COMP_DATA*(IsDlgButtonChecked(hWnd,ID_COMPLEX_DATA)) +
	       	                SPACE_DELIMIT*(IsDlgButtonChecked(hWnd,ID_SPACE_DELIMIT)) +
                            TAB_DELIMIT*(IsDlgButtonChecked(hWnd,ID_TAB_DELIMIT)) +
                            COMMA_DELIMIT*(IsDlgButtonChecked(hWnd,ID_COMMA_DELIMIT));

             // Get dimensions and matrix variable name             
				   GetDlgItemText(hWnd,ID_X_SIZE,xsizeStr,50);
				   sscanf(xsizeStr,"%ld",&xsize);
				   GetDlgItemText(hWnd,ID_Y_SIZE,ysizeStr,50);
				   sscanf(ysizeStr,"%ld",&ysize);
				   GetDlgItemText(hWnd,ID_Z_SIZE,zsizeStr,50);
				   sscanf(zsizeStr,"%ld",&zsize);

				 // Get header dimensions (in bytes)				 
				   GetDlgItemText(hWnd,ID_FILE_HEADER,str,50);
				   sscanf(str,"%ld",&fileHeader);
				   GetDlgItemText(hWnd,ID_ROW_HEADER,str,50);
				   sscanf(str,"%ld",&rowHeader);
				   
				 // Get matrix variable name				 
				   GetDlgItemText(hWnd,ID_MATRIX_NAME,matrixName,50);

			      if(matrixName[0] == '\0')
			      {
			         MessageDialog(prospaWin,MB_ICONERROR,"Error","matrix variable not defined");
    	            SetWindowLong(hWnd,DWL_MSGRESULT,1);
			         return(1);
			      }
				      
				 // Display the data?
               displayData = IsDlgButtonChecked(hWnd,ID_DISPLAY_DATA);

			    // Get filename
			      SendMessage(lpon->hdr.hwndFrom,CDM_GETSPEC ,(WPARAM)MAX_STR,(LPARAM) (LPTSTR)file);

				// Import the data, re-prompt on error
		         reportErrorToDialog = true;
               SetCursor(LoadCursor(NULL,IDC_WAIT));
               Interface itfc;
	       	   if(Import3DData(&itfc,displayData, file, importType, matrixName, xsize, ysize, zsize,false) == ERR)
	       	   {
                  SetCursor(LoadCursor(NULL,IDC_ARROW)); 	       	   
	       	   	reportErrorToDialog = false;	       	   
	       	      SetWindowLong(hWnd,DWL_MSGRESULT,1);
	       	      return(1);
	       	   }
               SetCursor(LoadCursor(NULL,IDC_ARROW));       	   
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
               EnableWindow(GetDlgItem(hWnd,ID_FILE_HEADER),false);
               EnableWindow(GetDlgItem(hWnd,ID_ROW_HEADER),false);
	         	return(0);
	       	case(ID_BINARY_FILE):
			      EnableWindow(GetDlgItem(hWnd,ID_REAL_DATA),true); 
			      EnableWindow(GetDlgItem(hWnd,ID_XY_DATA),true);
			      EnableWindow(GetDlgItem(hWnd,ID_32_FLOAT),true);
			      EnableWindow(GetDlgItem(hWnd,ID_32_INTEGER),true);
			      EnableWindow(GetDlgItem(hWnd,ID_16_INTEGER),true);
			      EnableWindow(GetDlgItem(hWnd,ID_BIG_ENDIAN),true);
               EnableWindow(GetDlgItem(hWnd,ID_FILE_HEADER),true);
               EnableWindow(GetDlgItem(hWnd,ID_ROW_HEADER),true);
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
*                   Import 3D data as specified type                    
* 
* mode ....... display data after importing or not
* fileName ... name of file to load
* type ....... binary or ascii data, complex or real
* matrixName . name of global variable to return (return in itfc otherwise)
* xsize ...... x dimension of data set to import
* ysize ...... y dimension of data set to import
* zsize ...... z dimension of data set to import
* ignorewarnings ... allows part of the data set to be loaded
* 
************************************************************************/

short Import3DData(Interface *itfc, short mode, char fileName[], long type, 
                   char* matrixName, long xsize, long ysize, long zsize, bool ignoreWarnings)
{
   long nrPoints;
   Variable *var;
   char *buffer;

   if(type & BINARY)
   {
      // Load the data file *********************
		short err;
		size_t fileLength;
      if(err = LoadFileIntoBuffer(fileName,fileLength,&buffer))
      {
         ErrorMessage("unable to load data from file '%s' - error '%hd'",fileName,err);
         return(ERR);
      }

      // Remove any headers (file and/or row)
      RemoveHeaders(buffer,fileLength,type,xsize,ysize,zsize,fileHeader,rowHeader);

      // Convert Big Endian to Little Endian if necessary
      ReorderBuffer(buffer,fileLength,type);

      // Determine number of data points
      nrPoints = DataPointsInBuffer(buffer,fileLength,type);

      // Check to see if the supplied data matches the file contents
      if(xsize*ysize*zsize > nrPoints)
      {
         delete [] buffer;
         if(type & COMP_DATA)
             ErrorMessage("%ld complex points in file\r but %ld (%ld*%ld*%ld) expected",nrPoints,xsize*ysize*zsize,xsize,ysize,zsize);
         else
             ErrorMessage("%ld real points in file\r but %ld (%ld*%ld*%ld) expected",nrPoints,xsize*ysize*zsize,xsize,ysize,zsize);
         return(ERR);
      }
      if(xsize*ysize*zsize < nrPoints && !ignoreWarnings)
      {
         short r;
         if(type & COMP_DATA)
             r = YesNoDialog(MB_ICONWARNING,1,"Warning","%ld complex points in file\r but %ld (%ld*%ld*%ld) expected - continue?",nrPoints,xsize*ysize*zsize,xsize,ysize,zsize);
         else
             r = YesNoDialog(MB_ICONWARNING,1,"Warning","%ld real points in file\r but %ld (%ld*%ld*%ld) expected - continue?",nrPoints,xsize*ysize*zsize,xsize,ysize,zsize);
         if(r == IDNO)
         {
            delete [] buffer;
            return(ERR);
         }
      } 

   // Make the returned variable
      if(matrixName[0] != '\0')
      {
         var = AddGlobalVariable(MATRIX2D, matrixName);
         if(var->GetReadOnly())
         {
            ErrorMessage("Variable '%s' is read only",matrixName);
	         return(ERR);
         }
         var->FreeData(); // Erase existing data if any
      }
      else
      {
         var = &(itfc->retVar[1]); // Return via itfc
      }

      if(type & COMP_DATA)
      {   
         complex ***cmat = ConvertBufferToComplex3DMatrix(buffer,xsize,ysize,zsize,type);

       // Save the data to a variable
          var->AssignCMatrix3D(cmat, xsize, ysize, zsize);	

          if(matrixName[0] == '\0')
             itfc->nrRetValues = 1;
   
       // Free up buffer data
          delete [] buffer;

       // If requested, display data
          if(mode & DISPLAY_DATA)
          {
				 Plot2D::curPlot()->setTitleText(fileName);
             CText temp;
             Interface itfc;
             temp.Format("\"%s\"",matrixName);
             DisplayMatrixAsImage(&itfc,temp.Str());
          }
      }
      else if(type & REAL_DATA)
      {
         float ***mat = ConvertBufferToReal3DMatrix(buffer,xsize,ysize,zsize,type);

       // Save the data to a variable
          var->AssignMatrix3D(mat, xsize, ysize, zsize);	

          if(matrixName[0] == '\0')
             itfc->nrRetValues = 1;

         // Free up buffer data
         delete [] buffer;
      
       // If requested, display data
      
          if(mode & DISPLAY_DATA)
          {
             CText temp;
             Interface itfc;
             temp.Format("\"%s\"",matrixName);
             DisplayMatrixAsImage(&itfc,temp.Str());
          }      
      }
   }
   else if(type & ASCII)
   {
     if(type & COMP_DATA)
     {
         complex *cy = NULL;
         
         type = type | XYDATA; // Fool next routine into thinking this is 1D data
     
         if((nrPoints = Load1DComplexAsciiDataFromFile(fileName, type, &cy)) == ERR)
            return(ERR);

         if(xsize*ysize*zsize > nrPoints)
         {
            FreeCVector(cy);
            ErrorMessage("%ld complex points in file\r but %ld (%ld*%ld*%ld) expected",nrPoints,xsize*ysize*zsize,xsize,ysize,zsize);
            return(ERR);
         }

         if(xsize*ysize*zsize < nrPoints && !ignoreWarnings)
         {
            short r = YesNoDialog(MB_ICONWARNING,1,"Warning","%ld complex points in file\r but %ld (%ld*%ld*%ld) expected - continue?",nrPoints,xsize*ysize*zsize,xsize,ysize,zsize);
            if(r == IDNO)
            {
               FreeCVector(cy);            
               return(ERR);
            }
         }

      // Make the returned variable
         if(matrixName[0] != '\0')
         {
            var = AddGlobalVariable(MATRIX2D, matrixName);
            if(var->GetReadOnly())
            {
               ErrorMessage("Variable '%s' is read only",matrixName);
	            return(ERR);
            }
			   var->MakeCMatrix3DFromCVector(cy, xsize, ysize, zsize);	
         }
         else
         {
            itfc->retVar[1].MakeCMatrix3DFromCVector(cy, xsize, ysize, zsize);
            itfc->nrRetValues = 1;
         }
     
       // Remove original data
          FreeCVector(cy);
                 
       // If requested, display data
          if(mode & DISPLAY_DATA)
          {
             CText temp;
             Interface itfc;
             temp.Format("\"real(%s)\"",matrixName);
             DisplayMatrixAsImage(&itfc,temp.Str());
          }
      }
      else if(type & REAL_DATA)
      {
         float *x  = NULL,*y = NULL;

         type = type | REAL_DATA; // Fool next routine into thinking this is 1D data
               
       // Load data from file - data returned in x and y
         if((nrPoints = Load1DRealAsciiDataFromFile(true, fileName, type, &x, &y)) == ERR)
            return(ERR);

         if(xsize*ysize*zsize != nrPoints)
         {
            FreeVector(y);
            ErrorMessage("%ld real points in file\r but %ld (%ld*%ld*%ld) expected",nrPoints,xsize*ysize*zsize,xsize,ysize,zsize);
            return(ERR);
         }
         if(xsize*ysize*zsize < nrPoints && !ignoreWarnings)
         {
            short r = YesNoDialog(MB_ICONWARNING,1,"Warning","%ld real points in file\r but %ld (%ld*%ld*%ld) expected - continue?",nrPoints,xsize*ysize*zsize,xsize,ysize,zsize);
            if(r == IDNO)
            {
               FreeVector(y);            
               return(ERR);
            }
         }

      // Make the returned variable
         if(matrixName[0] != '\0')
         {
            var = AddGlobalVariable(MATRIX2D, matrixName);
            if(var->GetReadOnly())
            {
               ErrorMessage("Variable '%s' is read only",matrixName);
	            return(ERR);
            }
			   var->MakeMatrix3DFromVector(y, xsize, ysize, zsize);	
         }
         else
         {
            itfc->retVar[1].MakeMatrix3DFromVector(y, xsize, ysize, zsize);
            itfc->nrRetValues = 1;
         }

       // Remove original data
          FreeVector(y);
      
       // If requested, display data      
          if(mode & DISPLAY_DATA)
          {
             Interface itfc;
             CText temp;
             temp.Format("\"%s\"",matrixName);
             DisplayMatrixAsImage(&itfc,temp.Str());
          }  
       }
     } 
     return(OK);
}

/****************************************************************************************
Convert the data stored in buffer to complex float and store in a 2D matrix
*****************************************************************************************/

float ***ConvertBufferToReal3DMatrix(char *buffer, long xsize, long ysize, long zsize, long type)
{
   long i = 0;
   long x,y,z;

   float ***mat = MakeMatrix3D(xsize,ysize,zsize);
   if(!mat)
   {
      ErrorMessage("unable to allocate memory while importing 3D data");
      return(NULL);
   }

   // Convert data to appropriate format 
   if(type & FLOAT_32) // Copy float data
   {   
      for(z = 0; z < zsize; z++)
         for(y = 0; y < ysize; y++)
            for(x = 0; x < xsize; x++)
               mat[z][y][x] = ((float*)buffer)[i++];
   }
   else if(type & FLOAT_64) // Copy double data
   {   
      for(z = 0; z < zsize; z++)
         for(y = 0; y < ysize; y++)
            for(x = 0; x < xsize; x++)
               mat[z][y][x] = (float)((double*)buffer)[i++];
   }
   else if(type & INT_32) // Copy long integer data
   {
      for(z = 0; z < zsize; z++)
         for(y = 0; y < ysize; y++)
            for(x = 0; x < xsize; x++)
               mat[z][y][x] = ((long*)buffer)[i++];
   }
   else if(type & INT_16) // Copy short integer data
   {
      for(z = 0; z < zsize; z++)
         for(y = 0; y < ysize; y++)
            for(x = 0; x < xsize; x++)
               mat[z][y][x] = ((short*)buffer)[i++];
   }
   return(mat);
}

/****************************************************************************************
Convert the data stored in buffer to complex float and store in a 2D matrix
*****************************************************************************************/

complex ***ConvertBufferToComplex3DMatrix(char *buffer, long xsize, long ysize, long zsize, long type)
{
   long i = 0;
   long x,y,z;

   complex ***cmat = MakeCMatrix3D(xsize,ysize,zsize);
   if(!cmat)
   {
      ErrorMessage("unable to allocate memory while importing 3D data");
      return(NULL);
   }

   // Convert data to appropriate format 
   if(type & FLOAT_32) // Copy float data
   {   
      for(z = 0; z < zsize; z++)
      {
         for(y = 0; y < ysize; y++)
         {
            for(x = 0; x < xsize; x++)
            {
               cmat[z][y][x].r = ((float*)buffer)[i++];
               cmat[z][y][x].i = ((float*)buffer)[i++];
            }
         }
      }
   }
   else if(type & FLOAT_64) // Copy double data
   {   
      for(z = 0; z < zsize; z++)
      {
         for(y = 0; y < ysize; y++)
         {
            for(x = 0; x < xsize; x++)
            {
               cmat[z][y][x].r = (float)((double*)buffer)[i++];
               cmat[z][y][x].i = (float)((double*)buffer)[i++];
            }
         }
      }
   }
   else if(type & INT_32) // Copy long integer data
   {
      for(z = 0; z < zsize; z++)
      {
         for(y = 0; y < ysize; y++)
         {
            for(x = 0; x < xsize; x++)
            {
               cmat[z][y][x].r = ((long*)buffer)[i++];
               cmat[z][y][x].i = ((long*)buffer)[i++];
            }
         }
      }
   }
   else if(type & INT_16) // Copy short integer data
   {
      for(z = 0; z < zsize; z++)
      {
         for(y = 0; y < ysize; y++)
         {
            for(x = 0; x < xsize; x++)
            {
               cmat[z][y][x].r = ((short*)buffer)[i++];
               cmat[z][y][x].i = ((short*)buffer)[i++];
            }
         }
      }
   }
   return(cmat);
}
