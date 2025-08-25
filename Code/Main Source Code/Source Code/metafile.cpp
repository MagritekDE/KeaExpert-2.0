#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include <ole2.h>
#include "stdafx.h"
#include "metafile.h"
#include "string_utilities.h"
#include "bitmap.h"
#include "cli_files.h"
#include "files.h"
#include "globals.h"
#include "interface.h"
#include "import_export_utilities.h"
#include "plot.h"
#include "plot1dCLI.h"
#include "plot2dCLI.h"
#include "plot3dEvents.h"
#include "plot3dSurface.h"
#include "PlotFile.h"
#include "PlotWindow.h"
#include "print.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "gl/gl.h" 
#include "gl/glu.h" 
#include "process.h"
#include "memoryLeak.h"

using namespace Gdiplus;

float   gEMFWidth  = 100; // Windows meta file parameters
float   gEMFHeight = 100;
float   gEMFLongTickLength  = 2; 
float   gEMFShortTickLength = 1;
float   gEMFSymbolSize = 3; 
float   gPlotSF = 1; // Copy and Image scale factor



int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  unsigned int  num = 0;    // number of image encoders
  unsigned int  size = 0;   // size of the image encoder array in bytes
  
  Gdiplus::GetImageEncodersSize(&num, &size);
  if(size == 0)return -1;
  
  Gdiplus::ImageCodecInfo* imageCodecInfo = new Gdiplus::ImageCodecInfo[size];
  Gdiplus::GetImageEncoders(num, size, imageCodecInfo);
  
  for(unsigned int i = 0; i < num; ++i)
  {
    if( wcscmp(imageCodecInfo[i].MimeType, format) == 0 )
    {
       *pClsid = imageCodecInfo[i].Clsid;
       delete[] imageCodecInfo;
       return i;
    }    
  }
  delete[] imageCodecInfo;
  return -1;
}



/*****************************************************************************
   Save the contents of the 3D plot hWnd as an enhanced meta file
*****************************************************************************/

short Make3DWMF(HWND hWnd, char *inputFileName)
{
   HDC hMetaFile;
	RECT pr;
   static char fileName[MAX_STR] = "untitled";
   static char directory[MAX_STR];
   short w,h;
   static short index = 1;
   char oldDir[MAX_PATH];

// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);

// Get the EMF filename from the user
   if(inputFileName == NULL)
   {
      if(directory[0] == '\0')
         strncpy_s(directory,MAX_STR,gPlot3DDirectory,_TRUNCATE);
      short err = FileDialog(hWnd, false, directory, fileName, 
                     "Save WMF", CentreHookProc, NULL, noTemplateFlag, 
                        1, &index, "Enhanced Metafile", "emf");  
      if(err != OK)
         return(ABORT);

   }
// Filename has been passed to function
   else
      strncpy_s(fileName,MAX_STR,inputFileName,_TRUNCATE);

// Make sure window is redrawn
   Invalidate3DPlot(hWnd);

// Get the plot window dimensions 
	GetClientRect(hWnd,&pr);

// Work out width and height of destination image
	h = pr.bottom;
   w = pr.right;

// Make an enhanced metafile
   hMetaFile = CreateEnhMetaFile(NULL,fileName,NULL,NULL);

	if(hMetaFile)
	{
		HDC hdcMem = CreateCompatibleDC(hMetaFile);
      OpenGLBitmapToEMF(hMetaFile,hdcMem,0,0,w,h);  
	   DeleteDC(hdcMem); 
      HENHMETAFILE hemf = CloseEnhMetaFile(hMetaFile);
      DeleteEnhMetaFile(hemf);     
   }
   else
   {
	   ErrorMessage("Can't write to file '%s'",fileName);
	}

// Restore the directory
   SetCurrentDirectory(oldDir);
   
   return(OK);
}




char* GetEMFParameter(short which)
{
	static char str[25];

	if(which == ID_EMF_WIDTH)
	{
		sprintf(str,"%g",gEMFWidth);
		return(str);
	}
	else if(which == ID_EMF_HEIGHT)
	{
		sprintf(str,"%g",gEMFHeight);
		return(str);
	}
	else if(which == ID_EMF_SF)
	{
		sprintf(str,"%g",gPlotSF);
		return(str);
	} 
	if(which == ID_LONG_TICK_LENGTH)
	{
		sprintf(str,"%g",gEMFLongTickLength);
		return(str);
	}
	else if(which == ID_SHORT_TICK_LENGTH)
	{
		sprintf(str,"%g",gEMFShortTickLength);
		return(str);
	}
	else if(which == ID_SYMBOL_SIZE)
	{
		sprintf(str,"%g",gEMFSymbolSize);
		return(str);
	} 
	return(NULL);
}

// When plotting data to the clipboard or to a file apply a scale factor to improve
// the quality of the plot. This scale factor is applied to everything except the 
// tracewidth (since it is detail in the tracewidth that we wish to preserve).

int SetPlotScaleFactor(Interface* itfc ,char args[])
{
   short nrArgs;
   float sf = 1.0;

   if((nrArgs = ArgScan(itfc,args,0,"scale_factor","e","f",&sf)) < 0)
     return(nrArgs);

   if(nrArgs == 0)
   {
      itfc->retVar[1].MakeAndSetFloat(gPlotSF);
      itfc->nrRetValues = 1;
      return(OK);
   }

   if(sf <= 0)
   {
      ErrorMessage("Scale factor must be > 0");
      return(ERR);
   }

   gPlotSF = sf;

   return(OK);
}