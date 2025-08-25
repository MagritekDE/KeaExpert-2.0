#include "stdafx.h"
#include "import_data_2d.h"
#include "allocate.h"
#include "cArg.h"
#include "control.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "edit_files.h"
#include "error.h"
#include "files.h"
#include "globals.h"
#include "import_export_utilities.h"
#include "interface.h"
#include "load_save_data.h"
#include "message.h"
#include "mymath.h"
#include "plot.h"
#include "plot2dCLI.h"
#include "process.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "DLGS.H"
#include "memoryLeak.h"

/***********************************************************************
*             Routines connected with importing 2D data                *
*                                                                      *
* Import2DDataCLI ......... CLI interface to the 2D import procedures. *
* Import2DDataParameters .. Initialise the import parameters (CLI)     *
* Import2DDataDialog ...... Dialog interface to 2D import procedures.  *
* Import2DHookProc ........ Modification to standard open dialog.      *
* Import2DData ............ Code to actually import the 1D data.       *
*                                                                      *
***********************************************************************/


complex **ConvertBufferToComplexMatrix(char *buffer, long xsize, long ysize, long type);
float **ConvertBufferToRealMatrix(char *buffer, long xsize, long ysize, long type);
long GetAsciiImportFileDataInfo(char file[], long &width, long &height, long &depth);
size_t CalcNumberPointsInFile(long size, long type, long xsize, long fileHeader, long rowHeader, size_t &matrixSize);
size_t CalcExpectedFileSize(long type, long xsize,long ysize, long zsize, long fileHeader, long rowHeader);
short LoadFileInto2DMatrixBuffer(Interface *itfc, FILE* fp, size_t allocaSize, size_t fileLength, size_t matrixSize, char **buffer);
// Import 2D file wide variables
   
static long importType    = REAL_DATA + ASCII + FLOAT_32 + SPACE_DELIMIT;
static long fileHeader = 0;
static long rowHeader = 0;
static char file[MAX_STR]     = "test";
static bool displayData = false;

/************************************************************************
*                      CLI interface for importing 2D data              *
************************************************************************/
      
int Import2DDataCLI(Interface* itfc ,char args[])
{
   short nrArgs;
   long xsize,ysize;
   CText fileName;
   FILE *fp;
   
// Get filename from user *************
   if((nrArgs = ArgScan(itfc,args,3,"filename, width, height","eee","tll", &fileName, &xsize, &ysize)) < 0)
     return(nrArgs);  

// Check for file existence ***********
   if(!abortOnError)
   {
		if(!(fp = fopen(fileName.Str(),"rb")))
		{
         itfc->retVar[0].SetType(NULL_VARIABLE);
         itfc->nrRetValues = 1;
		   return(OK);
		} 
		fclose(fp);
	}
	
// Import data ************************
   return(Import2DData(itfc,false,fileName.Str(),importType,"",xsize,ysize,true));
}


/************************************************************************
*               Select import parameters for import2d command           *
************************************************************************/

int Import2DDataParameters(Interface* itfc ,char args[])
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
   int nrArgs = carg.Count((char*)args);
   
   if(nrArgs == 0) // Help mode
   {
      TextMessage("\n\n   PARAMETER           VALUE                 DEFAULT\n\n");
      TextMessage("   ab .......... ascii/binary ............... %s\n",ab);
      TextMessage("   rc .......... real/complex ............... %s\n",rc);
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
*           Imports a 2D file using a modified Open File dialog         *
************************************************************************/

UINT Import2DHookProc(HWND hdlg,UINT uiMsg,WPARAM wParam,LPARAM lParam);

short Import2DDataDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index = 1;
   char oldDir[MAX_PATH];

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);

   if(IsPlacesBarHidden())
   {
      err = FileDialog(hWnd, true, pathName, fileName, 
                               "Import 2D data", Import2DHookProc, "IMPORT2DDLG",
                               templateFlag, 1, &index, "All files", "*");
   }
   else
   {
      err = FileDialog(hWnd, true, pathName, fileName, 
                               "Import 2D data", Import2DHookProc, "IMPORT2DDLG2",
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

UINT Import2DHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   static char matrixName[20] = "m1";
   static char strx[50] = "256";
   static char stry[50] = "256";
   static char str[50] = "256";
   static LPOFNOTIFY lpon;
	static char fileName[MAX_PATH];
   CText txt;
	long width,height,depth;
   Interface itfc;

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

		   SetDlgItemText(hWnd,ID_X_SIZE,strx);
		   SetDlgItemText(hWnd,ID_Y_SIZE,stry);
		   sprintf(str,"%ld",fileHeader);
		   SetDlgItemText(hWnd,ID_FILE_HEADER,str);
		   sprintf(str,"%ld",rowHeader);
		   SetDlgItemText(hWnd,ID_ROW_HEADER,str);
         
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
         lpon = (LPOFNOTIFY)lParam;
		   switch(lpon->hdr.code)
		   {
		      case(CDN_INITDONE): // Std dialog has finished updating so centre it in window
		      {
               PlaceDialogOnTopInCentre(hWnd,lpon->hdr.hwndFrom);             
			      break;
			   }
			   case(CDN_FILEOK): // OK button has been pressed in std dialog
			   {
               long xsize,ysize;
			      
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
				   GetDlgItemText(hWnd,ID_X_SIZE,strx,50);
				   sscanf(strx,"%ld",&xsize);
				   GetDlgItemText(hWnd,ID_Y_SIZE,stry,50);
				   sscanf(stry,"%ld",&ysize);
				 				 
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
               itfc.ptr = (void*)lpon->hdr.hwndFrom;
	       	   if(Import2DData(&itfc,displayData, file, importType, matrixName, xsize, ysize, false) == ERR)
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
			   case(CDN_SELCHANGE): // Check for file selection change
			   {
			      CommDlg_OpenSave_GetFilePath(lpon->hdr.hwndFrom, fileName, MAX_PATH);

               if(IsDlgButtonChecked(hWnd,ID_ASCII_FILE))
               {
			         if(GetAsciiImportFileDataInfo(fileName,width,height,depth) == OK)
                  {
                     if(IsDlgButtonChecked(hWnd,ID_COMPLEX_DATA))
                        width /= 2;
				         txt.Format("%ld",width);
					      SetDlgItemText(hWnd,ID_X_SIZE,txt.Str());
				         txt.Format("%ld",height);
					      SetDlgItemText(hWnd,ID_Y_SIZE,txt.Str());
                  }
               }
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
               if(IsDlgButtonChecked(hWnd,ID_ASCII_FILE))
               {
			         if(GetAsciiImportFileDataInfo(fileName,width,height,depth) == OK)
                  {
                     if(IsDlgButtonChecked(hWnd,ID_COMPLEX_DATA))
                        width /= 2;
				         txt.Format("%ld",width);
					      SetDlgItemText(hWnd,ID_X_SIZE,txt.Str());
				         txt.Format("%ld",height);
					      SetDlgItemText(hWnd,ID_Y_SIZE,txt.Str());
                  }
               }	         	
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
            case(ID_COMPLEX_DATA):
            case(ID_REAL_DATA):
            {
               if(IsDlgButtonChecked(hWnd,ID_ASCII_FILE))
               {
			         if(GetAsciiImportFileDataInfo(fileName,width,height,depth) == OK)
                  {
                     if(IsDlgButtonChecked(hWnd,ID_COMPLEX_DATA))
                        width /= 2;
				         txt.Format("%ld",width);
					      SetDlgItemText(hWnd,ID_X_SIZE,txt.Str());
				         txt.Format("%ld",height);
					      SetDlgItemText(hWnd,ID_Y_SIZE,txt.Str());
                  }
               }	
               return(0);
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
     Extract some information from an ascii  data file 
************************************************************************/

long GetAsciiImportFileDataInfo(char file[], long &width, long &height, long &depth)
{
   FILE *fp;
   long nrPoints;
   float *x  = NULL,*y = NULL;
 
 // Determine number of data points in file
   if((nrPoints = Load1DRealAsciiDataFromFile(NULL, file, REAL_DATA, &x, &y)) == ERR)
      return(ERR);

   FreeVector(y);

// Open file for ascii read *******************
   if(!(fp = fopen(file,"r")))
	{
	   return(ERR);
	}

// Load in a line of data
   long length = GetFileLength(fp);
   char *line = new char[length+1];
   if(!fgets(line,length,fp))
   {
      delete [] line;
      return(ERR);
   }
   length = strlen(line);

// Count the number of values in this line
   long j = 0;
   width = 0;
   for(long i = 0; i < length; i++)
   {
      if(IsDelimiter(line[i]))
      {
         if(j > 0)
         {
            width++;
            j = 0;
         }
      }
      else
         j++;
   }

   if(width == 0)
   {
      delete [] line;
      return(ERR);
   }

   height = nrPoints/width;

   delete [] line;

// Close file and return data type ****************
   fclose(fp);
   return(OK);
}

/************************************************************************
*                       Import 2D as specified type                     
* 
* mode ....... display data after importing or not
* fileName ... name of file to load
* type ....... binary or ascii data, complex or real
* matrixName . name of global variable to return (return in itfc otherwise)
* xsize ...... x dimension of data set to import
* ysize ...... y dimension of data set to import
* ignorewarnings ... allows part of the data set to be loaded
* 
************************************************************************/

short Import2DData(Interface *itfc, bool displayData, char fileName[], long type,
                   char* matrixName, long xsize, long ysize, bool ignoreWarnings)
{
   long nrPoints;
   Variable *var = NULL;

// Load the data taking into account type
   if(type & BINARY)
   {
      size_t fileLength;
      size_t expectedSize;
      size_t matrixSize;
      FILE *fp;  
      char *buffer;
      short err;

   // Check for either complex or real
      if(!(type & COMP_DATA) && !(type & REAL_DATA))
      { 
         ErrorMessage("Invalid data type");
		   return(ERR);
      }

   // Open the file
      if(!(fp = fopen(fileName,"rb")))
      {
         ErrorMessage("unable to load data from file '%s' - error '1'",fileName);
         return(ERR);
      }
      fileLength = GetFileLength(fp);

   // Determine the expected file size (in bytes)
      expectedSize = CalcExpectedFileSize(type, xsize, ysize, 1L, fileHeader, rowHeader);

   // Check for too large matrix dimensions and or header lengths
      if(expectedSize > fileLength)
      {
         fclose(fp);
         ErrorMessage("incorrect matrix dimensions or header lengths for file '%s'",fileName);
         return(ERR);
      }

   // Work out the number of possible data points in the file
      nrPoints = CalcNumberPointsInFile(fileLength,type,xsize,fileHeader,rowHeader,matrixSize);

   // Check to see if the supplied data matches the file contents
      if(xsize*ysize > nrPoints)
      {
         fclose(fp);

         if(type & COMP_DATA)
             ErrorMessage("%ld complex points in file\r but %ld (%ld*%ld) expected",nrPoints,xsize*ysize,xsize,ysize);
         else
             ErrorMessage("%ld real points in file\r but %ld (%ld*%ld) expected",nrPoints,xsize*ysize,xsize,ysize);
         return(ERR);
      }
      if(xsize*ysize < nrPoints && !ignoreWarnings)
      {
         int r;
         if(type & COMP_DATA)
             r = YesNoDialog(MB_ICONWARNING,1,"Warning","%ld complex points in file\r but %ld (%ld*%ld) expected - continue?",nrPoints,xsize*ysize,xsize,ysize);
         else
             r = YesNoDialog(MB_ICONWARNING,1,"Warning","%ld real points in file\r but %ld (%ld*%ld) expected - continue?",nrPoints,xsize*ysize,xsize,ysize);
         if(r == IDNO)
         {
            fclose(fp);
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

   // Load the data
      size_t ptrMemSize = ysize*sizeof(void*); // Room for row pointers
      if(err = LoadFileInto2DMatrixBuffer(itfc,fp,ptrMemSize,expectedSize,matrixSize,&buffer))
      {
         fclose(fp);

         if(err == 4)
            return(OK);
         else
         {
            ErrorMessage("unable to load data from file '%s' - error '%hd'",fileName,err);
            return(ERR);
         }
      }
      fclose(fp); 

      if(itfc->ptr)
         SetWindowText((HWND)itfc->ptr,"Converting the data ...");

   // Remove any headers (file and/or row)
      RemoveHeaders(buffer+ptrMemSize,expectedSize,type,xsize,ysize,1L,fileHeader,rowHeader);
      expectedSize -= rowHeader*ysize + fileHeader;

   // Convert Big Endian to Little Endian if necessary
      ReorderBuffer(buffer+ptrMemSize,expectedSize,type);

   // Copy the loaded data into a complex 2D matrix ****************
      if(type & COMP_DATA)
      {  
         complex **cmat = ConvertBufferToComplexMatrix(buffer,xsize,ysize,type);

      // Save the data to a variable
          var->AssignCMatrix2D(cmat, xsize, ysize);	

         if(matrixName[0] == '\0')
            itfc->nrRetValues = 1;
         
      // If requested, display data     
         if(displayData)
         {
            if(itfc->ptr)
               SetWindowText((HWND)itfc->ptr,"Displaying the data ...");
				Plot2D::curPlot()->setTitleText(fileName);
            CText temp;
            temp.Format("%s",matrixName);
            DisplayMatrixAsImage(itfc,temp.Str());
         }
      }

   // Copy the loaded data into a real 2D matrix ****************
      else if(type & REAL_DATA)
      { 
         float **mat = ConvertBufferToRealMatrix(buffer,xsize,ysize,type);

         var->AssignMatrix2D(mat, xsize, ysize);	
      
         if(matrixName[0] == '\0')
            itfc->nrRetValues = 1;
      
      // If requested, display data
         if(displayData)
         {
            if(itfc->ptr)
               SetWindowText((HWND)itfc->ptr,"Displaying the data ...");
            CText temp;
            temp.Format("%s",matrixName);
            DisplayMatrixAsImage(itfc,temp.Str());
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

         if(xsize*ysize > nrPoints)
         {
            FreeCVector(cy);
            ErrorMessage("%ld complex points in file\r but %ld (%ld*%ld) expected",nrPoints,xsize*ysize,xsize,ysize);
            return(ERR);
         }

         if(xsize*ysize < nrPoints && !ignoreWarnings)
         {
            short r = YesNoDialog(MB_ICONWARNING,1,"Warning","%ld complex points in file\r but %ld (%ld*%ld) expected - continue?",nrPoints,xsize*ysize,xsize,ysize);
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
            var->FreeData(); // Erase existing data if any
         }
         else
         {
            var = &(itfc->retVar[1]); // Return via itfc
         }

      // Save the data to a variable
         var->MakeCMatrix2DFromCVector(cy, xsize,ysize);	
      
         if(matrixName[0] == '\0')
            itfc->nrRetValues = 1;

       // Remove original data
         FreeCVector(cy);
                 
       // If requested, display data      
         if(displayData)
         {
            CText temp;
            temp.Format("real(%s)",matrixName);
            DisplayMatrixAsImage(itfc,temp.Str());
         }
      }
      else if(type & REAL_DATA)
      {
         float *x  = NULL,*y = NULL;

         type = type | REAL_DATA; // Fool next routine into thinking this is 1D data
               
       // Load data from file - data returned in x and y
         if((nrPoints = Load1DRealAsciiDataFromFile(true, fileName, type, &x, &y)) == ERR)
            return(ERR);

         if(xsize*ysize > nrPoints)
         {
            FreeVector(y);
            ErrorMessage("%ld real points in file\r but %ld (%ld*%ld) expected",nrPoints,xsize*ysize,xsize,ysize);
            return(ERR);
         }
         if(xsize*ysize < nrPoints && !ignoreWarnings)
         {
            short r = YesNoDialog(MB_ICONWARNING,1,"Warning","%ld real points in file\r but %ld (%ld*%ld) expected - continue?",nrPoints,xsize*ysize,xsize,ysize);
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
            var->FreeData(); // Erase existing data if any
         }
         else
         {
            var = &(itfc->retVar[1]); // Return via itfc
         }

      // Save the data to a variable
         var->MakeMatrix2DFromVector(y, xsize,ysize);	
      
         if(matrixName[0] == '\0')
            itfc->nrRetValues = 1;

       // Remove original data
         FreeVector(y);

       // If requested, display data      
         if(displayData)
         {
            CText temp;
            temp.Format("%s",matrixName);
            DisplayMatrixAsImage(itfc,temp.Str());
         }  
      }
   }
   
   else
   {
       ErrorMessage("invalid data type");
       return(ERR);
   }

   return(OK);
}


/**********************************************************************************
    Determine the expected byte size of a file given its dimensions and header lengths
**********************************************************************************/

size_t CalcExpectedFileSize(long type, long xsize,long ysize, long zsize, long fileHeader, long rowHeader)
{
   long wordLength = ((type & INT_32) > 0) * 4 + ((type & INT_16) > 0) * 2 + ((type & FLOAT_32) > 0) * 4;
   wordLength *= ((type & COMP_DATA)>0)*2 + ((type & REAL_DATA)>0)*1;
   size_t size = (size_t)xsize*ysize*zsize*wordLength + ysize*zsize*rowHeader + fileHeader;
   return(size);
}

/**********************************************************************************
    Determine the number of data points a file given its dimensions and header lengths
**********************************************************************************/


size_t CalcNumberPointsInFile(long size, long type, long xsize, long fileHeader, long rowHeader, size_t &matrixSize)
{
   long wordLength = ((type & INT_32) > 0) * 4 + ((type & INT_16) > 0) * 2 + ((type & FLOAT_32) > 0) * 4;
   long compreal = ((type & COMP_DATA)>0)*2 + ((type & REAL_DATA)>0)*1;
   wordLength *= compreal;
   long nrBytes = size - fileHeader;
   long nrRows = nrBytes/(wordLength*xsize+rowHeader);
   matrixSize = nrRows*xsize*compreal*sizeof(float);
   return(nrRows*(size_t)xsize);
}




/**********************************************************************************
            Remove headers from buffer and return the new buffer length
**********************************************************************************/

void RemoveHeaders(char *buffer, size_t &bufferLength, long type, long xsize,long ysize, long zsize, long fileHeader, long rowHeader)
{
   long i,j;

// Remove any fileHeader
   if(fileHeader)
   {
      for(j = 0, i = fileHeader; i < bufferLength; i++,j++)
      {
         buffer[j] = buffer[i];
      }
      bufferLength -= fileHeader;
   }

// Remove any row-headers
   if(rowHeader)
   {
      long rowSize = xsize *(((type & COMP_DATA)>0)*sizeof(complex) + ((type & REAL_DATA)>0)*sizeof(float));
      if(type & INT_16)
         rowSize /= 2;
      i = j = 0;
      for(long y = 0; y < ysize*zsize; y++)
      {
         i += rowHeader;
         for(long x = 0; x < rowSize && j < bufferLength && i < bufferLength; x++,j++,i++)
         {
            buffer[j] = buffer[i];
         }
      }
      bufferLength = j;
   }
}

/**********************************************************************************
                Convert Big Endian data to Little Endian
**********************************************************************************/

void ReorderBuffer(char *buffer, size_t fileLength, long type)
{
   if(type & BIG_ENDIAN) 
   {
      if(type & FLOAT_32 || type & INT_32) // Reverse byte order for 32 bit data
      {   
         for(long i = 0; i < fileLength; i+=4)
         {
            unsigned char byte3 = buffer[i];
            unsigned char byte2 = buffer[i+1];
            unsigned char byte1 = buffer[i+2];
            unsigned char byte0 = buffer[i+3];

            buffer[i]   = byte0;
            buffer[i+1] = byte1;
            buffer[i+2] = byte2;
            buffer[i+3] = byte3;
         }
      }
      if(type & INT_16) // Reverse byte order for 16 bit data
      {   
         for(long i = 0; i < fileLength; i+=2)
         {
            unsigned char byte1 = buffer[i];
            unsigned char byte0 = buffer[i+1];

            buffer[i]   = byte0;
            buffer[i+1] = byte1;
         }
      } 
   }
}

/****************************************************************************************
     Determine the number of data point in the buffer based on the filelength and data type
*****************************************************************************************/

long DataPointsInBuffer(char *buffer, size_t fileLength, long type)
{
   long nrPoints = 0;
// Figure out the correct number of data points based on data type passed   
   if(type & FLOAT_32)
      nrPoints = fileLength/(sizeof(float));
   else if(type & INT_32) 
      nrPoints = fileLength/(sizeof(long));
   else if(type & INT_16)
      nrPoints = fileLength/(sizeof(short));

   if((type & XYDATA) || (type & COMP_DATA))
      nrPoints /= 2;

   return(nrPoints);
}

/****************************************************************************************
    Convert the data stored in buffer to complex float and store in a 2D matrix
*****************************************************************************************/

float **ConvertBufferToRealMatrix(char *buffer, long xsize, long ysize, long type)
{
   long i = 0;
   long x,y;

// Establish row pointers
   float **mat = (float**)buffer;
   mat[0] = (float*)(buffer + ysize*sizeof(float*));
   for(y = 1; y < ysize; y++)
      mat[y] = mat[y-1]+xsize;

   buffer += ysize*sizeof(float*);

// Convert data to appropriate format 
   if(type & FLOAT_32) // Copy float data
   {   
      for(y = 0; y < ysize; y++)
      {
         for(x = 0; x < xsize; x++)
         {
            mat[y][x] = ((float*)buffer)[i++];
         }
      }
   }
   else if(type & FLOAT_64) // Copy float data
   {   
      for(y = 0; y < ysize; y++)
      {
         for(x = 0; x < xsize; x++)
         {
            mat[y][x] = (float)((double*)buffer)[i++];
         }
      }
   }
   else if(type & INT_32) // Copy long integer data
   {
      for(y = 0; y < ysize; y++)
      {
         for(x = 0; x < xsize; x++)
         {
            mat[y][x] = ((long*)buffer)[i++];
         }
      }
   }
   else if(type & INT_16) // Copy short integer data
   {
      i = xsize*ysize - 1;
      for(y = ysize-1; y >= 0; y--)
      {
         for(x = xsize-1; x >= 0; x--)
         {
            mat[y][x] = ((short*)buffer)[i--];
         }
      }
   }
   return(mat);
}

/****************************************************************************************
    Convert the data stored in buffer to complex float and store in a 2D matrix
*****************************************************************************************/

complex **ConvertBufferToComplexMatrix(char *buffer, long xsize, long ysize, long type)
{
   long i = 0;
   long x,y;

// Establish row pointers
   complex **cmat = (complex**)buffer;
   cmat[0] = (complex*)(buffer + ysize*sizeof(complex*));
   for(y = 1; y < ysize; y++)
      cmat[y] = cmat[y-1]+xsize;


   buffer += ysize*sizeof(complex*);

// Convert data to appropriate format 
   if(type & FLOAT_32) // Copy float data
   {   
      for(y = 0; y < ysize; y++)
      {
         for(x = 0; x < xsize; x++)
         {
            cmat[y][x].r = ((float*)buffer)[i++];
            cmat[y][x].i = ((float*)buffer)[i++];
         }
      }
   }
   else if(type & FLOAT_64) // Copy float data
   {   
      for(y = 0; y < ysize; y++)
      {
         for(x = 0; x < xsize; x++)
         {
            cmat[y][x].r = (float)((double*)buffer)[i++];
            cmat[y][x].i = (float)((double*)buffer)[i++];
         }
      }
   }
   else if(type & INT_32) // Copy long integer data
   {
      for(y = 0; y < ysize; y++)
      {
         for(x = 0; x < xsize; x++)
         {
            cmat[y][x].r = ((long*)buffer)[i++];
            cmat[y][x].i = ((long*)buffer)[i++];
         }
      }
   }
   else if(type & INT_16) // Copy short integer data
   {
      i = xsize*ysize*2 - 1;
      for(y = ysize-1; y >= 0; y--)
      {
         for(x = xsize-1; x >= 0; x--)
         {
            cmat[y][x].i = ((short*)buffer)[i--];
            cmat[y][x].r = ((short*)buffer)[i--];
         }
      }
   }
   return(cmat);
}


/*************************************************************************************
   Load data in file (fp) into matrix buffer.
   Allow user to escape file load if it is too slow

   itfc ......... contains a pointer to a window to display progress
   fp ........... file pointer
   ptrMemSize ... amount of memory to allocate before buffer for row pointers
   fileLength ... number of bytes in file
   nrPoints ..... number of points in file
   buffer ....... handle to buffer to hold the matrix 
**************************************************************************************/

short LoadFileInto2DMatrixBuffer(Interface *itfc, FILE* fp, size_t ptrMemSize,
                                 size_t fileLength, size_t matrixSize, char **buffer)
{
// Allocate memory for row table + main matrix buffer
   size_t size;

   if(fileLength > matrixSize)
      size = ptrMemSize+fileLength;
   else
      size = ptrMemSize+matrixSize;

   *buffer = (char*)GlobalAlloc(GMEM_FIXED,(ULONG)(size));
   if(!(*buffer))
   {
      return(2);
   }

// Set up buffer size and data destination pointer
   size_t buf_size = 1024*1024;
   size_t remaining = fileLength;
   char *ptr = *buffer+ptrMemSize;
   size_t loaded = 0;
   CText label;

// Read in the data in 1 Mbyte chunks ****
   while(remaining > buf_size)
   {
       if(fread(ptr,1,buf_size,fp) != buf_size)
       {
         GlobalFree(*buffer);
         return(3);
       }
       remaining -= buf_size;
       ptr += buf_size;
       if(ProcessBackgroundEvents() != OK)
       {
         if(itfc->ptr)
            SetWindowText((HWND)itfc->ptr,"File load aborted");
         GlobalFree(*buffer);
         return(4);
       }
       if(itfc->ptr)
       {
         loaded += buf_size;
         label.Format("%ld%% loaded",nint(((float)loaded*100/(float)fileLength)));
         SetWindowText((HWND)itfc->ptr,label.Str());
       }
   }
// Finish off any leftovers
   if(remaining > 0)
   {
      size_t sz = fread(ptr,1,remaining,fp);

      if(sz != remaining)
      {
         GlobalFree(*buffer);
         return(5);
      }
   }
   if(itfc->ptr) 
      SetWindowText((HWND)itfc->ptr,"File loaded");

   return(0);
}