#include "stdafx.h"
#include "gl/glu.h"
#include "globals.h"
#include "AviFile.h"
#include "CText.h"
#include "ScanStrings.h"
#include "memoryLeak.h"

#pragma comment(lib, "vfw32.lib")

CAviFile    *avi;
extern HWND cur3DWin;
int AddMovieFrame(Interface* itfc , char args[]);
int EndMovie(Interface* itfc , char args[]);

// Make a movie based on the 3D plot

int MakeMovie(Interface* itfc , char args[])
{
   short fps = 1;
   CText fileName;
   CText codec;
   short nrArgs;

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc, args,3,"codec, filename, fps","eee","ttd",&codec,&fileName,&fps)) < 0)
     return(nrArgs);

   if(codec == "")
      avi = new CAviFile(fileName.Str(),0, (DWORD)fps);    // Use MSVC codec
   else
      avi = new CAviFile(fileName.Str(), mmioFOURCC(codec[0],codec[1],codec[2],codec[3]), (DWORD)fps);    // Use MSVC codec

   return(OK);
}

// Add the current 3D plot to the movie

int AddMovieFrame(Interface* itfc , char args[])
{
   CText plot = "3d";
   short nrArgs;

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,0,"plot","e","t", &plot)) < 0)
     return(nrArgs);

   if(plot == "3D" || plot == "3d")
   {
      HDC hdc;
      HWND hwnd = cur3DWin;

   // Get client geometry
      RECT r,sbr;
      GetClientRect(hwnd,&r);

   // Work out width and height of destination image
       long h = r.bottom;
      long w = r.right;

      w -= w % 4;

      // Create a bitmap and select it in the device context
      hdc = GetDC(hwnd);
      HDC hdcMem = CreateCompatibleDC(hdc);
      unsigned char *pPixelData;

      // Alloc pixel bytes
      int NbBytes = 3 * w * h;
      pPixelData = new unsigned char[NbBytes];

      glPixelZoom(1,1);
      glPixelTransferi(GL_MAP_COLOR,0);
      glPixelTransferi(GL_RED_SCALE,1);   glPixelTransferi(GL_RED_BIAS,0);
      glPixelTransferi(GL_GREEN_SCALE,1); glPixelTransferi(GL_GREEN_BIAS,0);
      glPixelTransferi(GL_BLUE_SCALE,1);  glPixelTransferi(GL_BLUE_BIAS,0);

      glPixelStorei(GL_PACK_ALIGNMENT,1);   // byte alignment.
      glPixelStorei(GL_PACK_ROW_LENGTH,0);  // use default value (the arg to pixel routine).
      glPixelStorei(GL_PACK_SKIP_PIXELS,0); //
      glPixelStorei(GL_PACK_SKIP_ROWS,0);   //

      // Copy from OpenGL
      glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,pPixelData);

   // Swap red and blue (why?)
      unsigned char ctemp;
      for(int i = 0; i < w * h; i++)
      {
         ctemp = pPixelData[i*3];
         pPixelData[i*3] = pPixelData[i*3+2];
         pPixelData[i*3+2] = ctemp;
      }

      GdiFlush();

      if(avi->AppendNewFrame(w,h,pPixelData,24) < 0)
      {
			LPCTSTR err = avi->GetLastErrorMessage();

         ErrorMessage("Unable to append new frame");
         return(ERR);
      }

      delete [] pPixelData;
   }

  return(OK);
}


// End the plot movie

int EndMovie(Interface* itfc , char args[])
{
  delete avi;

  return(OK);
}

