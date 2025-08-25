#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "bitmap.h"
#include "plot3dClass.h"
#include <math.h>
#include "globals.h"
#include "files.h"
#include "mymath.h"
#include "metafile.h"
#include "guiObjectClass.h"
#include "plot3dSurface.h"
#include "string_utilities.h"
#include "scanstrings.h"
#include "gl/gl.h" 
#include "gl/glu.h"
#include "memoryLeak.h"

using namespace Gdiplus;

extern bool gAntiAlias;

CPlot3D::CPlot3D()
{
   xTicksPerLabel = 5;
   yTicksPerLabel = 5;
   zTicksPerLabel = 5;
   tickLength = 1;
   labelLength = 2;
   numberFontScaling = 3;
   labelFontScaling = 3;
   elevation = 320.0f; // Rotation amounts
   azimuth = 130.0f;
   twist = 0.0f;
   xScale = 1.0;       // Scale
   yScale = 1.0;
   zScale = 1.0;
   colorMap = NULL;
   colorMapLength = 0;
   displayColorScale = false;
   colorScaleMinValue = 0;
   colorScaleMaxValue = 1;
   colorScaleRed = 0;
   colorScaleGreen = 1;
   colorScaleBlue = 0;
   colorScaleFontSize = 5;
// Cutting plane
   draw3DPlane = false;
   planeXMin = 0;
   planeXMax = 10;
   planeYMin = 0;
   planeYMax = 10;
   planePosition = 0;
   planeDirection = XY_PLANE;
   planeRed = 0;
   planeGreen = 1;
   planeBlue = 0;
   planeAlpha = 0.4;
   cutPlaneZ = 0;
   initialised = false;
   count3D = 0;
   glBaseList = 0;
}

void CPlot3D::DrawAxes(float x1, float x2, float y1, float y2, float z1, float z2)
{
   xmin = x1;
   xmax = x2;
   ymin = y1;
   ymax = y2;
   zmin = z1;
   zmax = z2;
}

short CPlot3D::CalcTickAndLabelSpacing(float min, float max, float &tickSpacing, long &ticksPerLabel)
{
   float label = GetLabelSpacing(max-min);
   ticksPerLabel = 5;
   tickSpacing = label/ticksPerLabel;
   
   return(OK);
}

void CPlot3D::DrawXAxis(char *axisLabel, float minLabel, float maxLabel, float min, float max, float y, float z, short label_position, short axisDir)
{
   float tick,label,tickStart,labelStart;
   float tickSpacing;
   long ticksPerLabel;
   float xlabel;
   int tickCnt,nrTicks;
   float x;
   char str[10];

   CalcTickAndLabelSpacing(minLabel,maxLabel,tickSpacing,ticksPerLabel);

// Figure out best label and tick spacing
   tick = tickSpacing;
   label = ticksPerLabel*tick;
 
   if(label <= 0 || tick <= 0 || tick > label)
      return;
        
// Calculate leftmost tick and label positions in data points
   tickStart = (int)(minLabel/tick)*tick;
   if (tickStart < minLabel) tickStart += tick;
  
   labelStart = (int)(minLabel/label)*label;
   if (labelStart < minLabel) labelStart += label;

// Correct for rounding problems - tickstart should always be less then labelstart
   if(tickStart > labelStart)
     tickStart = labelStart;

// Count number of ticks "used" so far
   tickCnt = ticksPerLabel-(int)((labelStart-tickStart)/tick+0.5); 
    
// Work out number of ticks to plot
   nrTicks = nint((maxLabel-tickStart)/tick);

// Check for too many ticks!
   if(nrTicks > 5000)
      return;

   float sw=0,sh=0,ch,cw,lw,lh,lhu,lhd;

   if(gAntiAlias)
   {
      glEnable(GL_LINE_SMOOTH);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
   }

// Draw the axis
   for(int i = 0; i <= nrTicks; i++)
   {
      xlabel = tickStart + i*tick;
      if(xlabel > maxLabel) break;

   // Convert from label coords to screen coords
      if(axisDir == 0)
         x = (xlabel-minLabel)/(maxLabel-minLabel)*(max-min) + min;
      else
         x = max - (xlabel-minLabel)/(maxLabel-minLabel)*(max-min);

      if(tickCnt == ticksPerLabel) // Draw x label and long tick
      {
         glBegin(GL_LINES);
            glVertex3f(x,y,z);
            glVertex3f(x,y-labelLength,z);    
         glEnd();
            float xl = nint(xlabel/label)*label;
            sprintf(str,"%g",xl);
         glPushMatrix();
            glWin3d.StrDim(str,sw,sh,1);
            glTranslatef(x-sw*numberFontScaling/2,y-labelLength-sh*1.25*numberFontScaling,z);
            glScalef(numberFontScaling,numberFontScaling,numberFontScaling);
            glWin3d.DrawString(str,1);
         glPopMatrix();

         tickCnt = 0;
      }
      else
      {
         glBegin(GL_LINES);
         glVertex3f(x,y,z);
         glVertex3f(x,y-tickLength,z);                       
         glEnd();
      }
      tickCnt++;  
   }

// Print axes label
   glPushMatrix();

      float sf = labelFontScaling;
      DrawAxesLabelCore(axisLabel,false,lw,lhu,lhd); // Get label width and height
      lh = lhu;
      glWin3d.CharDim('0',cw,ch,1);

   // Work out label position
      float x_off = 0,y_off = 0;
      if(label_position == 0) //  // Centred and below x axis
      {
         x_off = (min+max)/2-lw/2;
         y_off = y-labelLength-sh*1.25*numberFontScaling-0.5*sh*numberFontScaling-lh;
      }
      else if(label_position == 1) // Positive end of x axis
      {
         x_off = max+cw*sf;
         y_off = y-lh/2;
      }
      else if(label_position == -1) // Negative end of x axis
      {
         x_off = min-(lw+cw*sf);
         y_off = y-lh/2;
      }

      glTranslatef(x_off,y_off,z); // Move to start of label 
      DrawAxesLabelCore(axisLabel,true,lw,lhu,lhd); // Draw label

   glPopMatrix();

// Draw main axis line
   glBegin(GL_LINES);
      glVertex3f(min,y,z);
      glVertex3f(max,y,z);                       
   glEnd();

   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_BLEND);

}

void CPlot3D::DrawYAxis(char *axisLabel, float minLabel, float maxLabel, float min, float max, float z, float x, short label_position, short axisDir)
{
   float tick,label,tickStart,labelStart;
   float tickSpacing;
   long ticksPerLabel;
   float ylabel;
   int tickCnt,nrTicks;
   char str[10];
   float y;

   CalcTickAndLabelSpacing(minLabel,maxLabel,tickSpacing,ticksPerLabel);

// Figure out best label and tick spacing
   tick = tickSpacing;
   label = ticksPerLabel*tick;
 
   if(label <= 0 || tick <= 0 || tick > label)
      return;
        
// Calculate leftmost tick and label positions in data points

   tickStart = (int)(minLabel/tick)*tick;
   if (tickStart < minLabel) tickStart += tick;
  
   labelStart = (int)(minLabel/label)*label;
   if (labelStart < minLabel) labelStart += label;

// Correct for rounding problems - tickstart should always be less then labelstart
   if(tickStart > labelStart)
     tickStart = labelStart;

// Count number of ticks "used" so far
   tickCnt = ticksPerLabel-(int)((labelStart-tickStart)/tick+0.5); 

// Work out number of ticks to plot
   nrTicks = nint((maxLabel-tickStart)/tick);

// Check for too many ticks!
   if(nrTicks > 5000)
      return;

   float sw=0,sh=0,cw,ch,lw,lh,lhu,lhd;
   float swMax = 0;


   if(gAntiAlias)
   {
      glEnable(GL_LINE_SMOOTH);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
   }

// Draw the axis
   for(int i = 0; i <= nrTicks; i++)
   {
      ylabel = tickStart + i*tick;
      if(ylabel > maxLabel) break;

   // Convert from label coords to screen coords
      if(axisDir == 0)
         y = (ylabel-minLabel)/(maxLabel-minLabel)*(max-min) + min;
      else
         y = max - (ylabel-minLabel)/(maxLabel-minLabel)*(max-min);

      if(tickCnt == ticksPerLabel) // Draw x label and long tick
      {
         glBegin(GL_LINES);
            glVertex3f(x,y,z);
            glVertex3f(x-labelLength,y,z);    
         glEnd();
            float yl = nint(ylabel/label)*label;
            sprintf(str,"%g",yl);
         glPushMatrix();
            glWin3d.StrDim(str,sw,sh,1);
            if(sw > swMax) swMax = sw;
            glWin3d.CharDim('0',cw,ch,1);
            glTranslatef(x-labelLength-(sw+cw/2)*numberFontScaling,y-sh*numberFontScaling/2,z);
            glScalef(numberFontScaling,numberFontScaling,numberFontScaling);
            glWin3d.DrawString(str,1);
         glPopMatrix();

         tickCnt = 0;
      }
      else
      {
         glBegin(GL_LINES);
         glVertex3f(x,y,z);
         glVertex3f(x-tickLength,y,z);                       
         glEnd();
      }
      tickCnt++;  
   }

// Print axes label
   glPushMatrix();
      DrawAxesLabelCore(axisLabel,false,lw,lhu,lhd); // Get label width and height
      lh = lhd;
      glWin3d.CharDim('0',cw,ch,1);

   // Work out label position
      float x_off = 0,y_off = 0;
      if(strlen(axisLabel) > 2 && label_position == 0) // Long labels are vertical
      {
         x_off = x-labelLength-(swMax+cw)*numberFontScaling-lh;
         y_off = (min+max)/2-lw/2;
         glTranslatef(x_off,y_off,z); // Move to start of label 
         glRotatef(90,0,0,1);
         DrawAxesLabelCore(axisLabel,true,lw,lhu,lhd); // Draw label
      }
      else
      {
         if(label_position == 0) // Centred and below y axis
         {
            x_off = x-labelLength-(swMax+1.5*cw)*numberFontScaling-lw;
            y_off = (min+max)/2-lh/2;
         }
         else if(label_position == 1) // Positive end of y axis
         {
            x_off = x-lw/2;
            y_off = max+lhd+ch*numberFontScaling;
         }
         else if(label_position == -1) // Negative end of y axis
         {
            x_off = x-lw/2;
            y_off = min-lhu-ch*numberFontScaling;
         }
         glTranslatef(x_off,y_off,z); // Move to start of label 
         DrawAxesLabelCore(axisLabel,true,lw,lhu,lhd); // Draw label
      }

   glPopMatrix();

// Draw main axis line
   glBegin(GL_LINES);
      glVertex3f(x,min,z);
      glVertex3f(x,max,z);                       
   glEnd();

   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_BLEND);

}

void CPlot3D::DrawZAxis(char *axisLabel, float minLabel, float maxLabel, float min, float max, float x, float y, short label_position, short axisDir)
{
   float tick,label,tickStart,labelStart;
   int tickCnt,nrTicks;
   float tickSpacing;
   long ticksPerLabel;
   char str[10];
   float z;
   float zlabel;

   CalcTickAndLabelSpacing(minLabel,maxLabel,tickSpacing,ticksPerLabel);

// Figure out best label and tick spacing
   tick = tickSpacing;
   label = ticksPerLabel*tick;
 
   if(label <= 0 || tick <= 0 || tick > label)
      return;
        
// Calculate leftmost tick and label positions in data points

   tickStart = (int)(minLabel/tick)*tick;
   if (tickStart < minLabel) tickStart += tick;
  
   labelStart = (int)(minLabel/label)*label;
   if (labelStart < minLabel) labelStart += label;

// Correct for rounding problems - tickstart should always be less then labelstart
   if(tickStart > labelStart)
     tickStart = labelStart;

// Count number of ticks "used" so far
   tickCnt = ticksPerLabel-(int)((labelStart-tickStart)/tick+0.5); 

// Work out number of ticks to plot
   nrTicks = nint((maxLabel-tickStart)/tick);

// Check for too many ticks!
   if(nrTicks > 5000)
      return;

   float sw=0,sh=0,cw,ch;

   if(gAntiAlias)
   {
      glEnable(GL_LINE_SMOOTH);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
   }

// Draw the axis
   for(int i = 0; i <= nrTicks; i++)
   {
      zlabel = tickStart + i*tick;
      if(zlabel > maxLabel) break;

   // Convert from label coords to screen coords
      if(axisDir == 0)
         z = (zlabel-minLabel)/(maxLabel-minLabel)*(max-min) + min;
      else
         z = max - (zlabel-minLabel)/(maxLabel-minLabel)*(max-min);

      if(tickCnt == ticksPerLabel) // Draw x label and long tick
      {
         glBegin(GL_LINES);
            glVertex3f(x,y,z);
            glVertex3f(x,y-labelLength,z);    
         glEnd();
            float zl = nint(zlabel/label)*label;
            sprintf(str,"%g",zl);
         glPushMatrix();
            glWin3d.StrDim(str,sw,sh,1);
            glWin3d.CharDim('0',cw,ch,1);
            glTranslatef(x,y-labelLength-sh*1.25*numberFontScaling,z+sw*numberFontScaling/2);
            glRotatef(90.0, 0.0f, 1.0f, 0.0f);
            glScalef(numberFontScaling,numberFontScaling,numberFontScaling);
            glWin3d.DrawString(str,1);
         glPopMatrix();

         tickCnt = 0;
      }
      else
      {
         glBegin(GL_LINES);
         glVertex3f(x,y,z);
         glVertex3f(x,y-tickLength,z);                       
         glEnd();
      }
      tickCnt++;  
   }

// Print axes label
   float lw,lh,lhu,lhd;
   glPushMatrix();

      float sf = labelFontScaling;
      DrawAxesLabelCore(axisLabel,false,lw,lhu,lhd); // Get label width and height
      lh = lhu;
      glWin3d.CharDim('0',cw,ch,1);

   // Work out label position
      float y_off = 0,z_off = 0;
      if(label_position == 0) // Centred and below z axis
      {
         y_off = y-labelLength-sh*1.25*numberFontScaling-0.5*sh*numberFontScaling-lh;
         z_off = (min+max)/2+lw/2;
      }
      else if(label_position == 1) // Positive end of z axis
      {
         y_off = y-lh/2;
         z_off = max+(lw+cw*sf);
      }
      else if(label_position == -1) // Negative end of z axis
      {
         y_off = y-lh/2;
         z_off = min-cw*sf;
      }

      glTranslatef(x,y_off,z_off); // Move to start of label 
      glRotatef(90.0, 0.0f, 1.0f, 0.0f);
      DrawAxesLabelCore(axisLabel,true,lw,lhu,lhd); // Draw label
   glPopMatrix();

// Draw main axis line
   glBegin(GL_LINES);
      glVertex3f(x,y,min);
      glVertex3f(x,y,max);                       
   glEnd();

   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_BLEND);

}


void CPlot3D::DrawAxesLabelCore(char *axisLabel, bool write, float &width, float &heightUp, float &heightDown)
{
   int i,j;
   char out[MAX_STR];
   float lw,lh;
   float sf = labelFontScaling;
   int start = 0;
   float subHeight = 0;
   float supHeight = 0;
   float normHeight = 0;
   float stdW,stdH;

   width = 0;
   heightUp = heightDown = 0;

// Get the standard width and height for a capital
   glWin3d.StrDim("X",stdW,stdH,1);

	for(i = 0; i < strlen(axisLabel); i++)
	{
	   if(axisLabel[i] == '^' && axisLabel[i+1] == '(') // Superscript
	   {
	      StrSubCopy(out,axisLabel,start,i-1);
         glScalef(sf,sf,sf);
         glWin3d.StrDim(out,lw,lh,1);
         if(write)
            glWin3d.DrawString(out,1);
         glScalef(1/sf,1/sf,1/sf);
         width += lw*sf;
         if(lh*sf > normHeight) normHeight = lh*sf;

	      start = i+2;
	      for(j = i+2; j < strlen(axisLabel); j++)
	      {
	         if(axisLabel[j] == ')')
	         {
	            StrSubCopy(out,axisLabel,start,j-1);
               glScalef(sf,sf,sf);
               glWin3d.StrDim(out,lw,lh,1);
               if(write)
               {

                  glTranslatef(0,0.75*stdH,0); 
                  glScalef(0.5,0.5,0.5);
                  glWin3d.DrawString(out,1);
                  glScalef(2,2,2);
                  glTranslatef(0,-0.75*stdH,0); 

               }
               glScalef(1/sf,1/sf,1/sf);
	            i = j;
	            start = j+1;
               width += lw/2*sf;
               if(lh*sf > supHeight) supHeight = lh*sf;
               break;
            }
         }
      }
	   if(axisLabel[i] == '_' && axisLabel[i+1] == '(') // Subscript
	   {
	      StrSubCopy(out,axisLabel,start,i-1);
         glScalef(sf,sf,sf);
         glWin3d.StrDim(out,lw,lh,1);
         if(write)
            glWin3d.DrawString(out,1);
         glScalef(1/sf,1/sf,1/sf);
         width += lw*sf;
         if(lh*sf > normHeight) normHeight = lh*sf;

	      start = i+2;
	      for(j = i+2; j < strlen(axisLabel); j++)
	      {
	         if(axisLabel[j] == ')')
	         {
	            StrSubCopy(out,axisLabel,start,j-1);
               glScalef(sf,sf,sf);
               glWin3d.StrDim(out,lw,lh,1);
               if(write)
               {
                  glTranslatef(0,-0.25*stdH,0); 
                  glScalef(0.5,0.5,0.5);
                  glWin3d.DrawString(out,1);
                  glScalef(2,2,2);
                  glTranslatef(0,0.25*stdH,0); 
               }
               glScalef(1/sf,1/sf,1/sf);
	            i = j;
	            start = j+1;
               width += lw/2*sf;
               if(lh*sf > subHeight) subHeight = lh*sf;
               break;
            }
         }
      }
	   if(axisLabel[i] == '\\' && axisLabel[i+1] == 'G' && axisLabel[i+2] == '(') // Greek
	   {
	      StrSubCopy(out,axisLabel,start,i-1);
         glScalef(sf,sf,sf);
         glWin3d.StrDim(out,lw,lh,1);
         if(write)
            glWin3d.DrawString(out,1);
         glScalef(1/sf,1/sf,1/sf);
         width += lw*sf;
         if(lh*sf > normHeight) normHeight = lh*sf;

	      start = i+3;
	      for(j = i+3; j < strlen(axisLabel); j++)
	      {
	         if(axisLabel[j] == ')')
	         {
	            StrSubCopy(out,axisLabel,start,j-1);
               glScalef(sf,sf,sf);
               glWin3d.StrDim(out,lw,lh,2);
               if(write)
                  glWin3d.DrawString(out,2);
               glScalef(1/sf,1/sf,1/sf);
	            i = j;
	            start = j+1;
               width += lw*sf;
               if(lh*sf > normHeight) normHeight = lh*sf;
	            break;
	         }
	      }
	   } 
   }
	StrSubCopy(out,axisLabel,start,i-1);
   glScalef(sf,sf,sf);
   glWin3d.StrDim(out,lw,lh,1);
   if(write)
   {
      glWin3d.DrawString(out,1);
      glScalef(1/sf,1/sf,1/sf);
      glTranslatef(lw,0,0); 
   }
   else
      glScalef(1/sf,1/sf,1/sf);
   if(lh*sf > normHeight) normHeight = lh*sf;
   
   width += lw*sf;

   if(normHeight > 0)
   {
      heightUp  = sf*stdH;
      heightDown = 0;
   }
   if(subHeight > 0) 
      heightDown = 0.25*sf*stdH;
   if(supHeight > 0) 
      heightUp   = 1.25*sf*stdH;
}

void CPlot3D::DrawSimpleAxes(float size)
{
   float w,h;

   glWin3d.CharDim('X',w,h,1);
   w *= labelFontScaling/2;
   h *= labelFontScaling/2;

// Draw the x,y,z labels

   glPushMatrix();
   {
      glTranslatef(size+w,-h,0);
      glScalef(labelFontScaling,labelFontScaling,labelFontScaling);
      glWin3d.DrawString("X",1);
   } 
   glPopMatrix();

   glPushMatrix();
   {
      glTranslatef(-w,size+h,0);
      glScalef(labelFontScaling,labelFontScaling,labelFontScaling);
      glWin3d.DrawString("Y",1);
   } 
   glPopMatrix();

   glPushMatrix();
   {
      glTranslatef(-w,-h,size+w);
      glScalef(labelFontScaling,labelFontScaling,labelFontScaling);
      glWin3d.DrawString("Z",1);
   } 
   glPopMatrix();


// Draw the axes lines
   glBegin(GL_LINES);
     glVertex3f(0.0,0.0,0.0);
     glVertex3f(size,0.0,0.0);         
     glVertex3f(0.0,0.0,0.0);
     glVertex3f(0.0,size,0.0);                         
     glVertex3f(0.0,0.0,0.0);
     glVertex3f(0.0,0.0,size);               
   glEnd();
}


short CPlot3D::SaveAsImage(char *fileName)
{
   short w,h;
   RECT pr;
   HBITMAP bitmap;

   HWND hWnd = this->parent->hWnd;

	// Get image type
	char extension[MAX_STR];
	GetExtension(fileName,extension);

	if(!strcmp(extension,"emf"))
		return(Make3DWMF(hWnd,fileName));

	CText mode;
	if(!strcmp(extension,"jpg"))
		mode = "image/jpeg";
	else if(!strcmp(extension,"tif"))
		mode = "image/tiff";
	else if(!strcmp(extension,"png") || !strcmp(extension,"gif") || !strcmp(extension,"bmp"))
		mode.Format("image/%s",extension);
	else
	{
		ErrorMessage("invalid extension for image save");
		return(ERR);
	}
	wchar_t *wMode = CharToWChar(mode.Str());
	// Convert image name
	wchar_t *wFileName = CharToWChar(fileName);

// Make sure window is redrawn
   Invalidate3DPlot(hWnd);

// Get the plot window dimensions 
	GetClientRect(hWnd,&pr);

// Work out width and height of destination image
	h = pr.bottom;
   w = pr.right;

// Draw plots to bitmap - scale bitmap if required
	HDC hdc = GetDC(hWnd);
	RECT r;
	GetClientRect(hWnd,&r);
   long newWidth;
	GenerateBitMap((r.right-r.left),(r.bottom-r.top), &bitmap, hdc, newWidth, false);
   Copy3DToBitmap(bitmap,0,0,w,h); 
	ReleaseDC(hWnd,hdc);

	// Convert to gdi+ and then save file
	Bitmap *bmp = Bitmap::FromHBITMAP(bitmap,NULL);
	CLSID pngClsid;
	GetEncoderClsid(wMode, &pngClsid);
	Status st = bmp->Save(wFileName, &pngClsid, NULL);
// Clean up
	SysFreeString(wFileName);
	SysFreeString(wMode);
   DeleteObject(bitmap);
   delete bmp;
   bmp = NULL;

	// Test for errors
	if(st != Ok)
	{
		ErrorMessage("can't save image - GDI+ status error no. %d%",(int)st);
		return(ERR);
	}

   return(OK);
}


void CPlot3D::Copy3DToBitmap(HBITMAP hBitmap, long x, long y, long w, long h) 
{ 
   w -= w % 4; 

   DIBSECTION section; 

// Get point to bitmap in DIB section
   GetObject(hBitmap,sizeof(DIBSECTION),&section);   
   BITMAP *bitmap   = &(section.dsBm);
   BYTE* pbm      = (BYTE*)bitmap->bmBits;

// Alloc pixel bytes 
   int NbBytes = 3 * w * h; 
   unsigned char *pPixelData = new unsigned char[NbBytes]; 

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
   glReadPixels(x,y,w,h,GL_RGB,GL_UNSIGNED_BYTE,pPixelData); 

// Swap red and blue (why?)
   unsigned char ctemp;
   for(int i = 0; i < w * h; i++)
   {
      ctemp = pPixelData[i*3];
      pPixelData[i*3] = pPixelData[i*3+2];
      pPixelData[i*3+2] = ctemp;
   }

// Copy to DIB
   memcpy(pbm,pPixelData,NbBytes); 

// Cleanup 
   delete [] pPixelData; 
}

void CPlot3D::SetSizes(float ts, float ll, float ns, float ls)
{
   tickLength = ts;
   labelLength = ll;
   numberFontScaling = ns;
   labelFontScaling = ls;
}

float CPlot3D::GetLabelSpacing(float width)
{
   float labelSpacing; 
   float temp;
   long diff;
   long cl,n;
   
   labelSpacing = fabs(width/5.0);
   temp = log10((double)labelSpacing);
   if(temp < 0) temp = temp-1;
   n = (long)(labelSpacing*pow(10.0,(double)(-(long)temp))+0.5);
   diff = abs(n-10), cl = 10;
   if(abs(n-1) < diff) diff = abs(n-1), cl = 1;
   if(abs(n-2) < diff) diff = abs(n-2), cl = 2;
   if(abs(n-5) < diff) cl = 5;
   labelSpacing = cl*pow(10.0,(double)(long)temp);
   if(labelSpacing > MAX_FLOAT)
      labelSpacing = MAX_FLOAT;
   return(labelSpacing); 
}