#include "stdafx.h"
#include "plot3dSurface.h"
#include <math.h>
#include "allocate.h"
#include "edit_class.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "interface.h"
#include "mymath.h"
#include "opengl.h"
#include "plot3d.h"
#include "plot3dClass.h"
#include "plot3dSurface.h"
#include "plot.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "gl/gl.h" 
#include "gl/glu.h"
#include "memoryLeak.h"

typedef struct
{
	float r,g,b;
} RGB;

typedef struct
{
	float x,y,z;
} XYZ;

typedef struct
{
   XYZ p[8];
	XYZ n[8];
   float val[8];  // Value at each vertex
} GRIDCELL;


typedef struct {
	XYZ p[3];			/* Vertices */
	XYZ c;				/* Centroid */
	XYZ n;			   /* Normal   */
} TRIANGLE;



void DrawCellTriangles(float*** mat, long x, long y, long z, float level, long &);
XYZ VertexInterp(float isolevel,XYZ p1,XYZ p2,float valp1,float valp2);
short PolygoniseCube(GRIDCELL g, float iso, TRIANGLE *tri);
XYZ CalcNormalXYZ(XYZ p,XYZ p1,XYZ p2);
void Normalise(XYZ *p);
void DrawXYZScale(float r, float g, float b, float size);
void DrawXYZBox(float r, float g, float b, float size);
void Find3DRange(float ***, short, short, short, float, float&, float&, float&, float&, float&, float&);
void DrawWireFrameBox(float minx, float maxx, float miny(), float maxy, float minz, float maxz);
void DrawCylinder(XYZ base, XYZ top, float radius);
float VectorLength(XYZ p1,XYZ p2);
void CalcNormalf(float x1, float y1, float z1,float x2, float y2, float z2,float x3, float y3, float z3, float xn, float yn, float zn);
void CalcRotationVector(float x, float y, float z, float &rx, float &ry, float &rz, float &angle);
XYZ GetXYZ(float x, float y, float z);
RGB GetRGB(float r, float g, float b);
void Set3DMapping(short,short,short,short);
void Get2DRange(float **mat, long dataLeft, long dataWidth, long dataTop, long dataHeight, float &minVal, float &maxVal);
void Get2DRange(complex **cmat, long dataLeft, long dataWidth, long dataTop, long dataHeight, float &minVal, float &maxVal);
void DrawColorScale(HWND hWnd);
void ClippingControl(void);
int DrawCone(Interface* itfc ,char args[]);

// Light position
float xLight = -50;
float yLight = 50;
float zLight = 100;
// Background colour
GLfloat	bkColor[]  = { 0.0f, 0.0f, 0.3f, 0.0f };
// Lighting
bool specular = true;  // Externally visible
// Fog
float fogStart = 0;
float fogEnd = 1;
GLfloat	fogColor[]  = { 0.0f, 0.0f, 0.3f, 0.0f };
// Rotation 
float elevationOrigin = 0;
float azimuthOrigin = 0;
float twistOrigin = 0;
float xRotOrigin = 0;
float yRotOrigin = 0;
float zRotOrigin = 0;
bool show_rotation_axis = false; // Externally accessible

// Axis parameters
float gAxesTickLength = 1;
float gAxesLabelLength = 2;
float gAxesNumberSize = 5;
float gTextSize3D = 8;
// Rotation mode 
char rotmode[] = "x";

// Clipping plane
bool clipPlane1 = true;
short clipPlane1Side = 1;
bool clipPlane2 = true;
short clipPlane2Side = 1;
float clipPlane1Position = 1000;
short clipPlane1Direction = XY_PLANE;
float clipPlane2Position = 1000;
short clipPlane2Direction = XY_PLANE;
bool gEnableClipping = false;

// Antilaliasing
bool gAntiAlias = false;
#define FLAT_SHADING   1
#define SMOOTH_SHADING 2
#define WATER_FALL 3
// Depth cueing
bool gDepthCue = false;
// Data range
float gxMin = -1;
float gxMax = 1;
float gyMin = -1;
float gyMax = 1;
float gzMin = -1;
float gzMax = 1;

// Externally accessible
bool update3d = true;
short surface2DMode = SMOOTH_SHADING;

CPlot3D *plot3d;
void Plot3DTest(HDC hdc);

/**********************************************************************************
  Called when the 3D plot is being created
**********************************************************************************/

short Initialize3DPlot(HWND hWnd)
{
   plot3d->glWin3d.Initialize(hWnd);
   Initialize3DParameters();
   plot3d->count3D = 512;
   plot3d->glBaseList = 513;
   return(OK);   
}   



void Initialize3DParameters()
{
   plot3d->viewDistance = 200;
   plot3d->zoff = 0; 
   plot3d->xoff = 0;
   plot3d->yoff = 0;
   plot3d->elevation = 0;
   plot3d->azimuth = 0;
   plot3d->twist = 0;
   plot3d->xScale = 1.0;
   plot3d->yScale = 1.0;
   plot3d->zScale = 1.0;
}

/**********************************************************************************
  Call to display the current 3D lists
**********************************************************************************/

//void Plot3DTest(HDC hdc)
//{
//   RECT r;
//
// //  GetClientRect(openGlWin,&r);
//
//   int x = 0;
//   int y = 0;
//   int w = r.right;
//   int h = r.bottom;
//
//
//   glShadeModel(GL_FLAT);
//
//   glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//   float f = tan(VIEW_ANGLE/2.0*PI/180);
//   glFrustum(-f,f,-f,f,1,5000.0);
//   glMatrixMode(GL_MODELVIEW);
//   glViewport(x, y, w, h);  
//   glDisable (GL_DEPTH_TEST);
//
//   glClearColor(0,0,0.3,0.0);
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//
//	glLoadIdentity();
//   glColor3f(1,1,1);
//
//   glBlendFunc (GL_SRC_ALPHA_SATURATE, GL_ONE);
//   glEnable (GL_BLEND);
//   glEnable(GL_POLYGON_SMOOTH);
//   glBegin(GL_POLYGON); 
//   glVertex3f(0,-5,-50);
//   glVertex3f(5,0,-50); 
//   glVertex3f(0,5,-50);
//   glVertex3f(-5,0,-50);
//   glEnd();
//
//
//   plot3d->glWin3d.Draw();
//
//}


/**********************************************************************************
  Redraw the current 3D plot
**********************************************************************************/

void SurfacePlot3D(HWND hWnd, HDC hdc)
{ 
   long w;
	GLfloat	 lightPos[] = { -50.f, 50.0f, 100.0f, 1.0f};
   float pi = 3.14159;
   RECT r;

// Don't redraw if update is off
   if(!update3d)
      return;

// Reset the mapping
   GetClientRect(hWnd,&r);

   if(plot3d->displayColorScale)
      w = 80*r.right/100; // This controls the right clipping boundary
   else
      w = r.right;
   Set3DMapping(0,0,w,r.bottom);

// Enable specularlighting
   SetUpLightModel();
   glEnable(GL_LIGHTING);

// Clear the window
   glClearColor(bkColor[0],bkColor[1],bkColor[2],bkColor[3]);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);   
   glPushMatrix();
	glLoadIdentity();

// Enable antialiasing (note should probably do whole scene antialiasing
// since we need to throw away depth cueing for this).
   //if(gAntiAlias)
   //{
   //   glEnable(GL_POLYGON_SMOOTH);
   //   glEnable(GL_LINE_SMOOTH);
   //   glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
   //   glEnable(GL_BLEND);
   //   glDisable(GL_DEPTH_TEST);
   //}
   //else
   //{
   //   glDisable(GL_POLYGON_SMOOTH);
   //   glDisable(GL_LINE_SMOOTH);
   //   glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);
   //   glDisable(GL_BLEND);
   //   glEnable(GL_DEPTH_TEST);
   //}

// Set up lighting positions
	lightPos[0] = xLight;
	lightPos[1] = yLight;
	lightPos[2] = zLight;
	lightPos[3] = 0;
	glLightfv(GL_LIGHT0,GL_POSITION,lightPos);	
	
// Enable specular lighting
   glEnable(GL_LIGHTING);

// Enable clipping
   glEnable (GL_CLIP_PLANE0);
   glEnable (GL_CLIP_PLANE1);

// Work out view point: r is eye coorindates and n is vertical vector at object
   float nx,ny,nz;
   float rx = plot3d->viewDistance*cos(-plot3d->elevation/180*pi)*sin(-plot3d->azimuth/180*pi);
   float ry = plot3d->viewDistance*sin(-plot3d->elevation/180*pi);
   float rz = plot3d->viewDistance*cos(-plot3d->elevation/180*pi)*cos(-plot3d->azimuth/180*pi);

   if(plot3d->elevation < 0)
   {
      nz = -rz;
      nx = -rx;
   }
   else
   {
      nx = rx;
      nz = rz;
   }
   if(ry == 0)
      ny = 1;
   else
      ny = (rx*rx + rz*rz)/fabs(ry);

// Enable view point
   gluLookAt(rx,ry,rz,0,0,0,nx,ny,nz);
   glRotatef(plot3d->twist, 0.0f, 0.0f, 1.0f);	 
   glTranslatef(plot3d->xoff,plot3d->yoff,plot3d->zoff);

// Set up clipping planes
   if(clipPlane1)
   {
      float D;

      if(clipPlane1Position == 0)
      {
         clipPlane1Position = 1;
         D = 0;
      }
      else
         D = 1;

      if(clipPlane1Position < 0)
         D = -D;
      D = D*clipPlane1Side;

      switch(clipPlane1Direction)
      {
         case(XY_PLANE):
         {
            GLdouble eqn[4] = {0.0, 0.0, clipPlane1Side*fabs(1.0/clipPlane1Position), D}; 
            glClipPlane (GL_CLIP_PLANE0, eqn);
            break;
         }
         case(XZ_PLANE):
         {
            GLdouble eqn[4] = {0.0, clipPlane1Side*fabs(1.0/clipPlane1Position), 0.0, D}; 
            glClipPlane (GL_CLIP_PLANE0, eqn);
            break;
         }
         case(YZ_PLANE):
         {
            GLdouble eqn[4] = {clipPlane1Side*fabs(1.0/clipPlane1Position), 0.0, 0.0, D}; 
            glClipPlane (GL_CLIP_PLANE0, eqn);
            break;
         }
      }
   }
   else
   {
       GLdouble eqn[4] = {0.0, 0.0, 0.0, 0.0}; 
       glClipPlane (GL_CLIP_PLANE0, eqn);
   }

   if(clipPlane2)
   {
      float D;

      if(clipPlane2Position == 0)
      {
         clipPlane2Position = 1;
         D = 0;
      }
      else
         D = 1;

      if(clipPlane2Position < 0)
         D = -D;
      D = D*clipPlane2Side;

      switch(clipPlane2Direction)
      {
         case(XY_PLANE):
         {
            GLdouble eqn[4] = {0.0, 0.0, clipPlane2Side*fabs(1.0/clipPlane2Position), D}; 
            glClipPlane (GL_CLIP_PLANE1, eqn);
            break;
         }
         case(XZ_PLANE):
         {
            GLdouble eqn[4] = {0.0, clipPlane2Side*fabs(1.0/clipPlane2Position), 0.0, D}; 
            glClipPlane (GL_CLIP_PLANE1, eqn);
            break;
         }
         case(YZ_PLANE):
         {
            GLdouble eqn[4] = {clipPlane2Side*fabs(1.0/clipPlane2Position), 0.0, 0.0, D}; 
            glClipPlane (GL_CLIP_PLANE1, eqn);
            break;
         }
      }
   }
   else
   {
       GLdouble eqn[4] = {0.0, 0.0, 0.0, 0.0}; 
       glClipPlane (GL_CLIP_PLANE1, eqn);
   }

// Draw red x-y-z axes at the rotation origin
   if(show_rotation_axis)
   {
   //   CPlot3D c3d;
	   glPushMatrix(); 
	   glDisable(GL_LIGHTING);
      plot3d->SetSizes(gAxesTickLength,gAxesLabelLength,gAxesNumberSize,2);
      glTranslatef(-plot3d->xoff,-plot3d->yoff,-plot3d->zoff);
      glColor3f(1,0,0);
      plot3d->DrawSimpleAxes(10);
	   glPopMatrix(); 
   }

   glScalef(plot3d->xScale,plot3d->yScale,plot3d->zScale);

// Enable fog
   if(gDepthCue)
   {
      glEnable(GL_FOG);
	   glFogf(GL_FOG_MODE, GL_LINEAR);
	   glFogfv(GL_FOG_COLOR,fogColor);
      glFogf(GL_FOG_START,fogStart+plot3d->viewDistance);
	   glFogf(GL_FOG_END,fogEnd+plot3d->viewDistance);
   }
   else
   {
      glDisable(GL_FOG);
   }

// Draw the current display list
	if(plot3d->count3D >= plot3d->glBaseList)
	{
	   for(short i = plot3d->glBaseList; i <= plot3d->count3D; i++)
      {
	      glCallList(i); 
      }
	}

// Draw cutting plane
   if(plot3d->draw3DPlane)
   {
	   glPushMatrix(); 

	   glEnable(GL_LIGHTING);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(plot3d->planeRed,plot3d->planeGreen,plot3d->planeBlue,plot3d->planeAlpha);

      glBegin(GL_POLYGON);

      switch(plot3d->planeDirection)
      {
         case(XY_PLANE):
            glVertex3f(plot3d->planeXMin, plot3d->planeYMin,plot3d->planePosition);
            glVertex3f(plot3d->planeXMin, plot3d->planeYMax,plot3d->planePosition); 
            glVertex3f(plot3d->planeXMax, plot3d->planeYMax,plot3d->planePosition);
            glVertex3f(plot3d->planeXMax, plot3d->planeYMin,plot3d->planePosition); 
            break;
         case(XZ_PLANE):
            glVertex3f(plot3d->planeXMin,plot3d->planePosition, plot3d->planeYMin);
            glVertex3f(plot3d->planeXMin,plot3d->planePosition, plot3d->planeYMax); 
            glVertex3f(plot3d->planeXMax,plot3d->planePosition, plot3d->planeYMax);
            glVertex3f(plot3d->planeXMax,plot3d->planePosition, plot3d->planeYMin); 
            break;
         case(YZ_PLANE):
            glVertex3f(plot3d->planePosition,plot3d->planeXMin, plot3d->planeYMin);
            glVertex3f(plot3d->planePosition,plot3d->planeXMin, plot3d->planeYMax); 
            glVertex3f(plot3d->planePosition,plot3d->planeXMax, plot3d->planeYMax);
            glVertex3f(plot3d->planePosition,plot3d->planeXMax, plot3d->planeYMin); 
            break;
      }
      glEnd( );
      glDisable(GL_BLEND);
	   glDisable(GL_LIGHTING);
	   glPopMatrix(); 
   }



// Tidy up
   glDisable(GL_FOG);

   glPopMatrix();

// Draw a color scale if desired
   if(plot3d->displayColorScale)
   {
      glPushMatrix();
      DrawColorScale(hWnd);
      glPopMatrix();
   }

   plot3d->glWin3d.Draw();
}

void DrawColorScale(HWND hWnd)
{
   RECT rgl;
   int w,h;
   float y;
   char str[20];
   char out[20];
   char exp_str[20];
   float sw,sh,ch,cw;
   float sfx,sfy;
   float spacing,label;
//   const float BORDER_LEFT = 0.15;
//   const float BORDER_TOP  = 0.27;
//   const float BORDER_BASE = -0.27;
   const float SCALE_LEFT  = 0.18;
   const float SCALE_RIGHT = 0.22;
   const float SCALE_BASE  = -0.2;
   const float SCALE_TOP   = 0.2;
   const float TICK_LENGTH = 0.003;

// Must be a colormap
	if(!(Plot2D::curPlot()) || !(Plot2D::curPlot()->colorMap()))
      return;

   GetClientRect(hWnd,&rgl);

	glDisable(GL_LIGHTING);

// Reset coordinate system
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
   gluPerspective(VIEW_ANGLE, 1, 1, 5000);
   glMatrixMode(GL_MODELVIEW);
   w = rgl.right;
   h = rgl.bottom;
   glViewport(0, 0, w, h);  

// Draw scale border
   glColor3f(1,1,1);
   glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

   glBegin(GL_POLYGON); 
   glVertex3f(SCALE_LEFT,SCALE_BASE,-1);
   glVertex3f(SCALE_LEFT,SCALE_TOP,-1); 
   glVertex3f(SCALE_RIGHT,SCALE_TOP,-1);
   glVertex3f(SCALE_RIGHT,SCALE_BASE,-1);
   glEnd();


// Draw scale labels and ticks

// Normal scale (full range)
	if(Plot2D::curPlot()->ColorScaleType() == NORMAL_CMAP)
   { 
		spacing = Axis::getLabelSpacing((double)plot3d->colorScaleMaxValue-(double)plot3d->colorScaleMinValue);      
	   label = (short)(plot3d->colorScaleMinValue/spacing)*spacing;
 	
   // Make the text scale with width not height
      sfx = plot3d->colorScaleFontSize/5*0.02;
      sfy = plot3d->colorScaleFontSize/5*0.02*w/h;

	   if(label < plot3d->colorScaleMinValue) 
	   {
	      label += spacing;
	   }

      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

   // Draw ticks and labels
	   while(true)
	   {
         y = SCALE_BASE +(SCALE_TOP-SCALE_BASE)*(label-plot3d->colorScaleMinValue)/(plot3d->colorScaleMaxValue-plot3d->colorScaleMinValue);

         glColor3f(0,0,0);
         glBegin(GL_LINES); 
            glVertex3f(SCALE_RIGHT-TICK_LENGTH,y,-1);
            glVertex3f(SCALE_RIGHT,y,-1); 
            glVertex3f(SCALE_LEFT,y,-1);
            glVertex3f(SCALE_LEFT+TICK_LENGTH,y,-1);
         glEnd();

         glColor3f(plot3d->colorScaleRed,plot3d->colorScaleGreen,plot3d->colorScaleBlue);

         GetLabel(spacing,label,str);

         glPushMatrix();
            plot3d->glWin3d.StrDim("0",cw,ch,1);
            plot3d->glWin3d.StrDim(str,sw,sh,1);
            glTranslatef(SCALE_RIGHT+TICK_LENGTH+cw*sfx/2.0,y-sfy/2*sh,-1);
            glScalef(sfx,sfy,1); // Make sure the font remains the same size
            plot3d->glWin3d.DrawString(str,1);   // Regardless of windows dimensions
         glPopMatrix();

         label += spacing;
	      if(label > plot3d->colorScaleMaxValue) break;
	   }
   }

// Plusminus scale
   else 
	{
		float maxv;

      if(fabs(plot3d->colorScaleMaxValue) < fabs(plot3d->colorScaleMinValue))
         maxv = fabs(plot3d->colorScaleMinValue);
      else
         maxv = fabs(plot3d->colorScaleMaxValue);

		spacing = Axis::getLabelSpacing((double)(maxv-plot3d->colorScaleMinValue));     
	   label = (short)(plot3d->colorScaleMinValue/spacing)*spacing;
	   if(label < plot3d->colorScaleMinValue) 
	   {
	      label += spacing;
	   }
      float startLabel = label;
   //   if(plot3d->colorScaleMinValue == startLabel && plot3d->colorScaleMinValue != 0)
  //       flag = 1;
 	
   // Make the text scale with width not height
      sfx = plot3d->colorScaleFontSize/5*0.02;
      sfy = plot3d->colorScaleFontSize/5*0.02*w/h;

      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);


   // Draw +ve ticks and labels
	   while(true)
	   {
         y = SCALE_BASE + (SCALE_TOP-SCALE_BASE)/2 + (SCALE_TOP-SCALE_BASE)/2*((label-plot3d->colorScaleMinValue)/(maxv-plot3d->colorScaleMinValue));

         glColor3f(0,0,0);
         glBegin(GL_LINES); 
            glVertex3f(SCALE_RIGHT-TICK_LENGTH,y,-1);
            glVertex3f(SCALE_RIGHT,y,-1); 
            glVertex3f(SCALE_LEFT,y,-1);
            glVertex3f(SCALE_LEFT+TICK_LENGTH,y,-1);
         glEnd();

         glColor3f(plot3d->colorScaleRed,plot3d->colorScaleGreen,plot3d->colorScaleBlue);

         GetLabel(spacing,label,str);
    //     if(flag)
    //     {
    //        sprintf(out,"±%s",str);
    //        flag = 0;
    //     }
    //     else
            strcpy(out,str);

         glPushMatrix();
            plot3d->glWin3d.StrDim("0",cw,ch,1);
            plot3d->glWin3d.StrDim(out,sw,sh,1);
            glTranslatef(SCALE_RIGHT+TICK_LENGTH+cw*sfx/2.0,y-sfy/2*sh,-1);
            glScalef(sfx,sfy,1); // Make sure the font remains the same size
            plot3d->glWin3d.DrawString(out,1);   // Regardless of windows dimensions
         glPopMatrix();

         label += spacing;
	      if(label > maxv) break;
	   }

   // Draw -ve ticks and labels
      label = -startLabel;
      if(plot3d->colorScaleMinValue == startLabel)
      {
         label -= spacing;
      }
	   while(true)
	   {
         y = SCALE_BASE + (SCALE_TOP-SCALE_BASE)/2 - (SCALE_TOP-SCALE_BASE)/2*((-label-plot3d->colorScaleMinValue)/(maxv-plot3d->colorScaleMinValue));

         glColor3f(0,0,0);
         glBegin(GL_LINES); 
            glVertex3f(SCALE_RIGHT-TICK_LENGTH,y,-1);
            glVertex3f(SCALE_RIGHT,y,-1); 
            glVertex3f(SCALE_LEFT,y,-1);
            glVertex3f(SCALE_LEFT+TICK_LENGTH,y,-1);
         glEnd();

         glColor3f(plot3d->colorScaleRed,plot3d->colorScaleGreen,plot3d->colorScaleBlue);

         GetLabel(spacing,label,str);

         glPushMatrix();
            plot3d->glWin3d.StrDim("0",cw,ch,1);
            plot3d->glWin3d.StrDim(str,sw,sh,1);
            glTranslatef(SCALE_RIGHT+TICK_LENGTH+cw*sfx/2.0,y-sfy/2*sh,-1);
            glScalef(sfx,sfy,1); // Make sure the font remains the same size
            plot3d->glWin3d.DrawString(str,1);   // Regardless of windows dimensions
         glPopMatrix();

         label -= spacing;
	      if(label < -maxv) break;
	   }
   }

// Draw exponent for scale numbers
   if(spacing > 30 || spacing < 0.1)
   {
      GetExponent(spacing,exp_str);
      glPushMatrix();
         plot3d->glWin3d.StrDim("×10",sw,sh,1);
         glTranslatef((SCALE_LEFT+SCALE_RIGHT)/2-sw*sfx/2.0,SCALE_TOP+0.01,-1);
         glScalef(sfx,sfy,1); // Make sure the font remains the same size
         plot3d->glWin3d.DrawString("×10",1);   // Regardless of windows dimensions
      glPopMatrix();
      glPushMatrix();
         glTranslatef((SCALE_LEFT+SCALE_RIGHT)/2+0.55*sw*sfx,SCALE_TOP+0.01+sfy/2,-1);
         glScalef(0.7*sfx,0.7*sfy,1); // Make sure the font remains the same size
         plot3d->glWin3d.DrawString(exp_str,1);   // Regardless of windows dimensions
      glPopMatrix();
   }

// Draw colorscale
   if(plot3d->colorMapLength > 0)
   {
      float yold = SCALE_BASE;
		float r,g,b,y;
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      for(int i = 1; i < plot3d->colorMapLength; i++)
      {
		
         y = SCALE_BASE + (SCALE_TOP-SCALE_BASE)*i/(plot3d->colorMapLength-1);
         r = plot3d->colorMap[i-1][0];
         g = plot3d->colorMap[i-1][1];
         b = plot3d->colorMap[i-1][2];
         glColor3f(r,g,b);
         glBegin(GL_POLYGON); 
            glVertex3f(SCALE_LEFT,yold,-1);
            glVertex3f(SCALE_LEFT,y,-1); 
            glVertex3f(SCALE_RIGHT,y,-1);
            glVertex3f(SCALE_RIGHT,yold,-1);
         glEnd();
         yold = y;
      }
   }

  glEnable(GL_LIGHTING);

}

void GetLabel(float spacing, float label, char *txt)
{
   char mantissa[20],exponent[20];
   char format[20];
   short ex,exp_spacing;
   float man;

// Interrogate the label spacing
   FloatSplit(spacing,mantissa,exponent,0);
   sscanf(exponent,"%hd",&exp_spacing);

   if(spacing < 1 && spacing >= 0.1)
   {
	   sprintf(format,"%%1.%hdf",-exp_spacing);
      sprintf(txt,format,label);
   }
   else if(spacing > 30 || spacing < 0.1)
   {
      short i;
      
      FloatSplit(label,mantissa,exponent,5);
      sscanf(exponent,"%hd",&ex);
      sscanf(mantissa,"%f",&man);
      if(ex > exp_spacing)
      {
         for(i = 0; i < ex-exp_spacing; i++)
            man *= 10.0;
      }
      else
      {
         for(i = 0; i < exp_spacing-ex; i++)
            man /= 10.0;
      }
      sprintf(txt,"%ld",nint(man));
   }
   else
      sprintf(txt,"%ld",nint(label));

}

void GetExponent(float label, char *txt)
{
   char mantissa[20],exponent[20];
   short ex;
   float man;
   
// Handle large or small label values
   FloatSplit(label,mantissa,exponent,0);
   sscanf(exponent,"%hd",&ex);
   sscanf(mantissa,"%f",&man);
// Return exponent in simplest form
   sprintf(txt,"%hd",ex);
}

int Set3DDataRange(Interface *itfc, char args[])
{
   short na;
   float xMin,xMax,yMin,yMax,zMin,zMax;

   xMin = gxMin;
   yMin = gyMin;
   zMin = gzMin;
   xMax = gxMax;
   yMax = gyMax;
   zMax = gzMax;

// Get arguments from user *************
   if((na = ArgScan(itfc,args,2,"xmin,xmax,ymin,ymax,zmin,zmax","eeeeee","ffffff",&xMin,&xMax,&yMin,&yMax,&zMin,&zMax)) < 0)
    return(na);

   if(xMin < xMax)
   {
      gxMin = xMin;
      gxMax = xMax;
   }
   else
   {
      ErrorMessage("invalid x range");
      return(ERR);
   }
   if(yMin < yMax)
   {
      gyMin = yMin;
      gyMax = yMax;
   }
   else
   {
      ErrorMessage("invalid y range");
      return(ERR);
   }
   if(zMin < zMax)
   {
      gzMin = zMin;
      gzMax = zMax;
   }
   else
   {
      ErrorMessage("invalid z range");
      return(ERR);
   }

   if(na == 2)
   {
      gzMin = gyMin = gxMin;
      gzMax = gyMax = gxMax;
   }

   return(OK);
}

int Display3dSurface(Interface* itfc ,char args[])
{
   short na;
   Variable mVar,colorVar;
   Variable rangeVar;
   float x1,x2,y1,y2,z1,z2;
   float ix,iy,iz;
   float iw,ih,id;
   long x,y,z;
   long n = 0;
   CText txt;
   float r = 0.8, g = 0.2, b = 0, alpha = 1;

   float ***mat3d;
   long width,height,depth;
   float level;


// Get arguments from user *************
   if((na = ArgScan(itfc,args,3,"matrix, level, color, [range]","eeee","vfvv",&mVar,&level,&colorVar,&rangeVar)) < 0)
    return(na);

   if(VarType(&mVar) != MATRIX3D)
   {
      ErrorMessage("data must be a real 3D matrix");
      return(ERR);
   }

   if(VarType(&colorVar) != MATRIX2D && VarHeight(&colorVar) != 1)
   {
      ErrorMessage("invalid RGB colour");
      return(ERR);
   }

   if(VarWidth(&colorVar) >= 3)
   {
	   r = VarRealMatrix(&colorVar)[0][0];
	   g = VarRealMatrix(&colorVar)[0][1];
	   b = VarRealMatrix(&colorVar)[0][2];
	}
   if(VarWidth(&colorVar) ==  4)
      alpha = VarRealMatrix(&colorVar)[0][3];

// Get the dimensions of the 3D data set
   width  = VarWidth(&mVar);
   height = VarHeight(&mVar);
   depth  = VarDepth(&mVar);
   mat3d  = VarReal3DMatrix(&mVar);

// If the range variables have been given
   if(na == 4)
   {
      if(VarType(&rangeVar) != MATRIX2D || VarWidth(&rangeVar) != 6 || VarHeight(&rangeVar) != 1)
      {
         ErrorMessage("invalid range vector");
         return(ERR);
      }

      float **range = VarRealMatrix(&rangeVar);

      x1 = range[0][0];
      x2 = range[0][1];
      y1 = range[0][2];
      y2 = range[0][3];
      z1 = range[0][4];
      z2 = range[0][5];
   }
   else
   {
      x1 = 0;
      x2 = width;
      y1 = 0;
      y2 = height;
      z1 = 0;
      z2 = depth;
   }


// Find data range (iw,ih,id) == dim of isosurf (ix,iy,iz) == centre of isosuf
   Find3DRange(mat3d, width, height, depth, level,  iw, ih, id, ix, iy, iz);

// Start recording GL commands
   glNewList(++plot3d->count3D, GL_COMPILE);	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); 
   ClippingControl();	  
   glEnable(GL_NORMALIZE);
	glDisable(GL_LIGHTING);

// Scale and shift data set to specified location
   glTranslatef(x1,y1,z1);
   glScalef((x2-x1)/width,(y2-y1)/height,(z2-z1)/depth);

// Set object color
	glEnable(GL_LIGHTING);

   if(alpha < 1)   
   {
     glEnable(GL_BLEND);
     glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
     glColor4f(r,g,b,alpha);
   }
   else
   {
	   glColor3f(r,g,b);
	}

// Draw isosurface
   n = 0;
	for(z = 0; z < depth-1; z++)
	{
	   for(y = 0; y < height-1; y++)
	   {
	      for(x = 0; x < width-1; x++)
	      {
	         DrawCellTriangles(mat3d,x,y,z,level,n);
	      }
	   }
	}  

   if(alpha < 1)   
     glDisable(GL_BLEND);
	
   glPopMatrix();

   glEndList();

   glDisable (GL_CLIP_PLANE1);
   glDisable (GL_CLIP_PLANE0);

   txt.Format("Number of polygons : %ld",n);
   UpdateStatusWindow(cur3DWin,0,txt.Str());

   Invalidate3DPlot(cur3DWin);

   itfc->retVar[1].MakeAndSetFloat(ix-iw/2);
   itfc->retVar[2].MakeAndSetFloat(ix+iw/2);
   itfc->retVar[3].MakeAndSetFloat(iy-ih/2);
   itfc->retVar[4].MakeAndSetFloat(iy+ih/2);
   itfc->retVar[5].MakeAndSetFloat(iz-id/2);
   itfc->retVar[6].MakeAndSetFloat(iz+id/2);
   itfc->nrRetValues = 6;

   return(OK);   
}   




void DrawXYZScale(float size, float r, float g, float b)
{
   float w,h;
   float sf = size/10;

   plot3d->glWin3d.CharDim('X',w,h,1);

// Draw the x,y,z labels
   glColor3f(r,g,b);
   glPushMatrix();
   {
      glTranslatef(size+w,-h,0);
      glScalef(sf,sf,sf);
      plot3d->glWin3d.DrawString("X",1);
   } 
   glPopMatrix();

   glPushMatrix();
   {
      glTranslatef(-w,size+h,0);
      glScalef(sf,sf,sf);
      plot3d->glWin3d.DrawString("Y",1);
   } 
   glPopMatrix();

   glPushMatrix();
   {
      glTranslatef(-w,-h,size+w);
      glScalef(sf,sf,sf);
      plot3d->glWin3d.DrawString("Z",1);
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

void DrawXYZBox(float s, float r, float g, float b)
{
	glColor3f(r,g,b);

   glBegin(GL_LINE_STRIP);
     glVertex3f(s,-s,-s);
     glVertex3f(s,s,-s);
     glVertex3f(-s,s,-s);
     glVertex3f(-s,-s,-s);
     glVertex3f(s,-s,-s);
   glEnd();

   glBegin(GL_LINE_STRIP);
     glVertex3f(s,-s,s);
     glVertex3f(s,s,s);
     glVertex3f(-s,s,s);
     glVertex3f(-s,-s,s);
     glVertex3f(s,-s,s);
   glEnd();
   
   glBegin(GL_LINES);
     glVertex3f(s,-s,s);
     glVertex3f(s,-s,-s);
     glVertex3f(s,s,s);
     glVertex3f(s,s,-s);
     glVertex3f(-s,s,s);
     glVertex3f(-s,s,-s);
     glVertex3f(-s,-s,s);
     glVertex3f(-s,-s,-s);
   glEnd();   
}

void DrawWireFrameBox(float minx, float maxx, float miny, float maxy, float minz, float maxz)
{
   glBegin(GL_LINE_STRIP);
     glVertex3f(maxx,miny,minz);
     glVertex3f(maxx,maxy,minz);
     glVertex3f(minx,maxy,minz);
     glVertex3f(minx,miny,minz);
     glVertex3f(maxx,miny,minz);
   glEnd();

   glBegin(GL_LINE_STRIP);
     glVertex3f(maxx,miny,maxz);
     glVertex3f(maxx,maxy,maxz);
     glVertex3f(minx,maxy,maxz);
     glVertex3f(minx,miny,maxz);
     glVertex3f(maxx,miny,maxz);
   glEnd();
   
   glBegin(GL_LINES);
     glVertex3f(maxx,miny,maxz);
     glVertex3f(maxx,miny,minz);
     glVertex3f(maxx,maxy,maxz);
     glVertex3f(maxx,maxy,minz);
     glVertex3f(minx,maxy,maxz);
     glVertex3f(minx,maxy,minz);
     glVertex3f(minx,miny,maxz);
     glVertex3f(minx,miny,minz);
   glEnd(); 
 }



void DrawCylinder(XYZ base, XYZ top, float radius)
{
   short i;
   float theta,theta1,theta2;
   float x,y,x1,y1,x2,y2;
   float length;
   float a,b,c;
   float rx,ry,rz,angle;
   short rsectors = 36;
   

   glEnable(GL_NORMALIZE);
   glShadeModel(GL_SMOOTH);
	ClippingControl();

   length = VectorLength(base,top);

   a = top.x - base.x;
   b = top.y - base.y;
   c = top.z - base.z;
   CalcRotationVector(a, b, c, rx, ry, rz, angle);

   glTranslatef(base.x,base.y,base.z);
        
   glRotatef(angle*180/PI, rx, ry, rz);     

   glBegin(GL_TRIANGLE_FAN);
	   glNormal3f(0.0,0.0,-1.0);
      glVertex3f(0.0,0.0,0.0);
	   for(i = 0; i <= rsectors; i++)
	   {
	      theta = i*2*PI/rsectors;
	      x = radius*cos(theta);
	      y = radius*sin(theta);
	      glVertex3f(x,y,0.0);
	   }
   glEnd();  

   glBegin(GL_TRIANGLE_FAN);
	   glNormal3f(0.0,0.0,1.0);
      glVertex3f(0.0,0.0,length);
	   for(i = 0; i <= rsectors; i++)
	   {
	      theta = i*2*PI/rsectors;
	      x = radius*cos(theta);
	      y = radius*sin(theta);
	      glVertex3f(x,y,length);
	   }
   glEnd();    

   glBegin(GL_QUADS);
	   for(i = 0; i < rsectors; i++)
	   {
	      theta1 = i*2*PI/rsectors;
	      theta2 = (i+1)*2*PI/rsectors;
	      x1 = radius*cos(theta1);
	      y1 = radius*sin(theta1);
	      x2 = radius*cos(theta2);
	      y2 = radius*sin(theta2);	
	      glNormal3f(x1, y1, 0.0);
	      glVertex3f(x1,y1,0.0);
	      glNormal3f(x1, y1, 0.0);	      
	      glVertex3f(x1,y1,length);
	      glNormal3f(x2, y2, 0.0);	      
	      glVertex3f(x2,y2,length);
	      glNormal3f(x2, y2, 0.0);	      
	      glVertex3f(x2,y2,0.0);
	   } 
   glEnd();    
}

void DrawCellTriangles(float*** mat, long x, long y, long z, float level, long &n)
{
   short nrTri,i;
   TRIANGLE t[10];
   GRIDCELL g;

// Set up gridcell
   
   g.val[0] = mat[z][y][x];
   g.val[1] = mat[z][y+1][x];
   g.val[2] = mat[z][y+1][x+1];
   g.val[3] = mat[z][y][x+1];
   g.val[4] = mat[z+1][y][x];
   g.val[5] = mat[z+1][y+1][x];
   g.val[6] = mat[z+1][y+1][x+1];
   g.val[7] = mat[z+1][y][x+1];
   
   g.p[0].x = x;   g.p[0].y = y;   g.p[0].z = z;
   g.p[1].x = x;   g.p[1].y = y+1; g.p[1].z = z;
   g.p[2].x = x+1; g.p[2].y = y+1; g.p[2].z = z;
   g.p[3].x = x+1; g.p[3].y = y;   g.p[3].z = z;
   g.p[4].x = x;   g.p[4].y = y;   g.p[4].z = z+1;
   g.p[5].x = x;   g.p[5].y = y+1; g.p[5].z = z+1;
   g.p[6].x = x+1; g.p[6].y = y+1; g.p[6].z = z+1;
   g.p[7].x = x+1; g.p[7].y = y;   g.p[7].z = z+1;
   
   if((nrTri = PolygoniseCube(g,level,t)) == 0)
      return;

// Calculate normals for triangles

   for(i = 0; i < nrTri; i++)
   {  
      t[i].n = CalcNormalXYZ(t[i].p[2], t[i].p[1], t[i].p[0]);
   }
      
   glBegin(GL_TRIANGLES);
	   for(i = 0; i < nrTri; i++)
	   { 
			glNormal3f(t[i].n.x, t[i].n.y, t[i].n.z);
			glVertex3f(t[i].p[2].x, t[i].p[2].y, t[i].p[2].z);
			glVertex3f(t[i].p[1].x, t[i].p[1].y, t[i].p[1].z);
			glVertex3f(t[i].p[0].x, t[i].p[0].y, t[i].p[0].z);
			n++;
	   }
	glEnd(); 

/*
   glColor3f(0.0f, 0.0f, 0.0f);
   for(i = 0; i < nrTri; i++)
   {  
      glBegin(GL_LINE_LOOP);
			glVertex3f(t[i].p[0].x, t[i].p[0].y, t[i].p[0].z);
			glVertex3f(t[i].p[1].x, t[i].p[1].y, t[i].p[1].z);
			glVertex3f(t[i].p[2].x, t[i].p[2].y, t[i].p[2].z);
      glEnd(); 
   } 
*/  
}


/*
	int edgeTable[256].  It corresponds to the 2^8 possible combinations of
	of the eight (n) vertices either existing inside or outside (2^n) of the
	surface.  A vertex is inside of a surface if the value at that vertex is
	less than that of the surface you are scanning for.  The table index is
	constructed bitwise with bit 0 corresponding to vertex 0, bit 1 to vert
	1.. bit 7 to vert 7.  The value in the table tells you which edges of
	the table are intersected by the surface.  Once again bit 0 corresponds
	to edge 0 and so on, up to edge 12. 
	Constructing the table simply consisted of having a program run thru
	the 256 cases and setting the edge bit if the vertices at either end of
	the edge had different values (one is inside while the other is out). 
	The purpose of the table is to speed up the scanning process.  Only the
	edges whose bit's are set contain vertices of the surface.
	Vertex 0 is on the bottom face, back edge, left side.  
	The progression of vertices is clockwise around the bottom face
	and then clockwise around the top face of the cube.  Edge 0 goes from
	vertex 0 to vertex 1, Edge 1 is from 2->3 and so on around clockwise to
	vertex 0 again. Then Edge 4 to 7 make up the top face, 4->5, 5->6, 6->7
	and 7->4.  Edge 8 thru 11 are the vertical edges from vert 0->4, 1->5,
	2->6, and 3->7.
	    4--------5     *---4----*
 	   /|       /|    /|       /|    
 	  / |      / |   7 |      5 |    
 	 /  |     /  |  /  8     /  9    
 	7--------6   | *----6---*   | 
 	|   |    |   | |   |    |   |
 	|   0----|---1 |   *---0|---*  
 	|  /     |  /  11 /     10 /   
 	| /      | /   | 3      | 1
 	|/       |/    |/       |/    
 	3--------2     *---2----*
*/
static int edgeTable[256]={
0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0   };

/*
	int triTable[256][16] also corresponds to the 256 possible combinations
	of vertices.
	The [16] dimension of the table is again the list of edges of the cube
	which are intersected by the surface.  This time however, the edges are
	enumerated in the order of the vertices making up the triangle mesh of
	the surface.  Each edge contains one vertex that is on the surface. 
	Each triple of edges listed in the table contains the vertices of one
	triangle on the mesh.  The are 16 entries because it has been shown that
	there are at most 5 triangles in a cube and each "edge triple" list is
	terminated with the value -1. 
	For example triTable[3] contains 
	{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}
	This corresponds to the case of a cube whose vertex 0 and 1 are inside
	of the surface and the rest of the verts are outside (00000001 bitwise
	OR'ed with 00000010 makes 00000011 == 3).  Therefore, this cube is
	intersected by the surface roughly in the form of a plane which cuts
	edges 8,9,1 and 3.  This quadrilateral can be constructed from two
	triangles: one which is made of the intersection vertices found on edges
	1,8, and 3; the other is formed from the vertices on edges 9,8, and 1. 
	Remember, each intersected edge contains only one surface vertex.  The
	vertex triples are listed in counter clockwise order for proper facing.
*/
static int triTable[256][16] =
{{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1},
{3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1},
{1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1},
{4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1},
{9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1},
{2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1},
{10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1},
{5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1},
{5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1},
{8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1},
{2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1},
{7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1},
{11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1},
{5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1},
{11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1},
{11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1},
{9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1},
{6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1},
{6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1},
{6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1},
{8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1},
{7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1},
{3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1},
{5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1},
{0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1},
{9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1},
{8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1},
{5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1},
{0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1},
{6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1},
{10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1},
{10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1},
{0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1},
{3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1},
{6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1},
{9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1},
{8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1},
{3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1},
{6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1},
{10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1},
{10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1},
{7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1},
{7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1},
{2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1},
{1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1},
{11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1},
{8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1},
{0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1},
{7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1},
{7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1},
{1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1},
{10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1},
{0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1},
{7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1},
{6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1},
{9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1},
{6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1},
{4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1},
{10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1},
{8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1},
{1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1},
{8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1},
{10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1},
{4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1},
{10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1},
{9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1},
{6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1},
{7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1},
{3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1},
{7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1},
{3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1},
{6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1},
{9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1},
{1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1},
{4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1},
{7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1},
{6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1},
{0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1},
{6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1},
{0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1},
{11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1},
{6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1},
{5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1},
{9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1},
{1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1},
{1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1},
{10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1},
{0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1},
{5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1},
{10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1},
{11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1},
{9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1},
{7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1},
{2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1},
{9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1},
{9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1},
{1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1},
{9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1},
{9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1},
{0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1},
{10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1},
{2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1},
{0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1},
{0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1},
{9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1},
{5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1},
{3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1},
{5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1},
{0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1},
{9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1},
{1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1},
{3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1},
{4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1},
{9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1},
{11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1},
{11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1},
{2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1},
{9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1},
{3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1},
{1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1},
{4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1},
{4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1},
{0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
{3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1},
{3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1},
{0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1},
{9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1},
{1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};
 
short PolygoniseCube(GRIDCELL g, float iso, TRIANGLE *tri)
{
	long i,ntri = 0;
	long cubeindex;
	XYZ vertlist[12]; // Found itersections

   /*
      Determine the index into the edge table which
      tells us which vertices are inside of the surface
   */
   
   cubeindex = 0;
   if (g.val[0] < iso) cubeindex |= 1;
   if (g.val[1] < iso) cubeindex |= 2;
   if (g.val[2] < iso) cubeindex |= 4;
   if (g.val[3] < iso) cubeindex |= 8;
   if (g.val[4] < iso) cubeindex |= 16;
   if (g.val[5] < iso) cubeindex |= 32;
   if (g.val[6] < iso) cubeindex |= 64;
   if (g.val[7] < iso) cubeindex |= 128;
   /* Cube is entirely in/out of the surface */
   if (edgeTable[cubeindex] == 0)
      return(0);

   /* Find the vertices where the surface intersects the cube */
   if (edgeTable[cubeindex] & 1) {
      vertlist[0] = VertexInterp(iso,g.p[0],g.p[1],g.val[0],g.val[1]);
	}
   if (edgeTable[cubeindex] & 2) {
      vertlist[1] = VertexInterp(iso,g.p[1],g.p[2],g.val[1],g.val[2]);
	}
   if (edgeTable[cubeindex] & 4) {
      vertlist[2] = VertexInterp(iso,g.p[2],g.p[3],g.val[2],g.val[3]);
	}
   if (edgeTable[cubeindex] & 8) {
      vertlist[3] = VertexInterp(iso,g.p[3],g.p[0],g.val[3],g.val[0]);
	}
   if (edgeTable[cubeindex] & 16) {
      vertlist[4] = VertexInterp(iso,g.p[4],g.p[5],g.val[4],g.val[5]);
	}
   if (edgeTable[cubeindex] & 32) {
      vertlist[5] = VertexInterp(iso,g.p[5],g.p[6],g.val[5],g.val[6]);
	}
   if (edgeTable[cubeindex] & 64) {
      vertlist[6] = VertexInterp(iso,g.p[6],g.p[7],g.val[6],g.val[7]);
	}
   if (edgeTable[cubeindex] & 128) {
      vertlist[7] = VertexInterp(iso,g.p[7],g.p[4],g.val[7],g.val[4]);
	}
   if (edgeTable[cubeindex] & 256) {
      vertlist[8] = VertexInterp(iso,g.p[0],g.p[4],g.val[0],g.val[4]);
	}
   if (edgeTable[cubeindex] & 512) {
      vertlist[9] = VertexInterp(iso,g.p[1],g.p[5],g.val[1],g.val[5]);
	}
   if (edgeTable[cubeindex] & 1024) {
      vertlist[10] = VertexInterp(iso,g.p[2],g.p[6],g.val[2],g.val[6]);
	}
   if (edgeTable[cubeindex] & 2048) {
      vertlist[11] = VertexInterp(iso,g.p[3],g.p[7],g.val[3],g.val[7]);
	}

   /* Create the triangles */
   for (i=0;triTable[cubeindex][i]!=-1;i+=3) {
      tri[ntri].p[0] = vertlist[triTable[cubeindex][i  ]];
      tri[ntri].p[1] = vertlist[triTable[cubeindex][i+1]];
      tri[ntri].p[2] = vertlist[triTable[cubeindex][i+2]];
      ntri++;
   }

	return(ntri);
}

/*-------------------------------------------------------------------------
   Return the point between two points in the same ratio as
   isolevel is between valp1 and valp2
*/
XYZ VertexInterp(float isolevel,XYZ p1,XYZ p2,float valp1,float valp2)
{
   float mu;
   XYZ p;

   if (fabs(isolevel-valp1) < 0.00001)
      return(p1);
   if (fabs(isolevel-valp2) < 0.00001)
      return(p2);
   if (fabs(valp1-valp2) < 0.00001)
      return(p1);
   mu = (isolevel - valp1) / (valp2 - valp1);
   p.x = p1.x + mu * (p2.x - p1.x);
   p.y = p1.y + mu * (p2.y - p1.y);
   p.z = p1.z + mu * (p2.z - p1.z);

   return(p);
}


/*-------------------------------------------------------------------------
	Calculate the unit normal at p given two other points 
	p1,p2 on the surface. The normal points in the direction 
	of p1 crossproduct p2
*/
XYZ CalcNormalXYZ(XYZ p,XYZ p1,XYZ p2)
{
	XYZ n,pa,pb;

   pa.x = p1.x - p.x;
   pa.y = p1.y - p.y;
   pa.z = p1.z - p.z;
	pb.x = p2.x - p.x;
	pb.y = p2.y - p.y;
	pb.z = p2.z - p.z;
   n.x = (pa.y * pb.z - pa.z * pb.y);
   n.y = (pa.z * pb.x - pa.x * pb.z);
   n.z = (pa.x * pb.y - pa.y * pb.x);
	Normalise(&n);

	return(n);
}


/*-------------------------------------------------------------------------
	Normalise a vector
*/
void Normalise(XYZ *p)
{
   double length;

   length = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
   if (length != 0) {
      p->x /= length;
      p->y /= length;
      p->z /= length;
   } else {
		p->x = 0;
		p->y = 0;
		p->z = 0;
	}	
}

// Scan through the matrix mat looking for cells which contain level l.
// Use this to determine the maximum dimensions of the isosurface

void Find3DRange(float ***mat, short w, short h, short d, float l, float &iw, float &ih, float &id,
                                                              float &ix, float &iy, float &iz)
{
   long minx = 1e9;
   long miny = 1e9;
   long minz = 1e9;
   long maxx = -1e9;
   long maxy = -1e9;
   long maxz = -1e9;
   long x,y,z;
   long n;
   
   for(z = 0; z < d-1; z++)
   {
      for(y = 0; y < h-1; y++)
      {
         for(x = 0; x < w-1; x++)
         {
            n = (mat[z][y][x] > l) +
                (mat[z][y+1][x] > l) +
                (mat[z][y+1][x+1] > l) + 
                (mat[z][y][x+1] > l) +
                (mat[z+1][y][x] > l) +
                (mat[z+1][y+1][x] > l) +
                (mat[z+1][y+1][x+1] > l) +
                (mat[z+1][y][x+1] > l);
                
            if(n != 8 && n != 0)
            {
               if(x < minx) minx = x;
               if(x > maxx) maxx = x;
               if(y < miny) miny = y;
               if(y > maxy) maxy = y;
               if(z < minz) minz = z;
               if(z > maxz) maxz = z;  
            }
         }
      }
   }
   ix = (maxx + minx)/2;
   iy = (maxy + miny)/2;
   iz = (maxz + minz)/2;
   iw = maxx - minx + 1;
   ih = maxy - miny + 1;
   id = maxz - minz + 1;
}    

//	Return the distance between two points

float VectorLength(XYZ p1,XYZ p2)
{
	XYZ d;
	
	d.x = p1.x - p2.x;
	d.y = p1.y - p2.y;
	d.z = p1.z - p2.z;

	return(sqrt(d.x*d.x + d.y*d.y + d.z*d.z));
}
        
   
void CalcNormalf(float x1, float y1, float z1,float x2, float y2, float z2,float x3, float y3, float z3, float xn, float yn, float zn)
{
	float v1[3],v2[3];
	static const int x = 0;
	static const int y = 1;
	static const int z = 2;

	// Calculate two vectors from the three points
	v1[x] = x1-x2;
	v1[y] = y1-y2;
	v1[z] = z1-z2;

	v2[x] = x2-x3;
	v2[y] = y2-y3;
	v2[z] = z2-z3;

	// Take the cross product of the two vectors to get
	// the normal vector which will be stored in out
	xn = v1[y]*v2[z] - v1[z]*v2[y];
	yn = v1[z]*v2[x] - v1[x]*v2[z];
	zn = v1[x]*v2[y] - v1[y]*v2[x];

	// Normalize the vector (shorten length to one)
//	ReduceToUnit(out);
}

// Find vector normal to vector (1,0,0) and (x,y,z) and angle between (1,0,0) and (x,y,z)

void CalcRotationVector(float x, float y, float z, float &rx, float &ry, float &rz, float &angle)
{
	rx = -y;
	ry = x;
	rz = 0;
	
	angle = acos(z/(sqrt(x*x+y*y+z*z)));
}

XYZ GetXYZ(float x, float y, float z)
{
   XYZ c;
   
   c.x = x;
   c.y = y;
   c.z = z;
   
   return(c);
}

RGB GetRGB(float r, float g, float b)
{
   RGB c;
   
   c.r = r;
   c.g = g;
   c.b = b;
   
   return(c);
}

/*****************************************************************************
* Add a cylinder to the display list
*****************************************************************************/

int Draw3DCylinder(Interface *itfc, char args[])
{
   Variable baseVar,topVar,colVar;
   float radius;
   short nrArgs;
   float r = 0,g = 0,b = 0,alpha = 1;

   if(!plot3d)
      return(0);

// Get filename from user *************
   
   if((nrArgs = ArgScan(itfc,args,4,"base,top,radius,colour","eeee","vvfv",&baseVar,&topVar,&radius,&colVar)) < 0)
     return(nrArgs); 

   if(VarType(&baseVar) != MATRIX2D && VarRowSize(&baseVar) != 1 && VarColSize(&baseVar) != 3)
   {
      ErrorMessage("invalid base coordinate");
      return(ERR);
   }

   if(VarType(&topVar) != MATRIX2D && VarRowSize(&topVar) != 1 && VarColSize(&topVar) != 3)
   {
      ErrorMessage("invalid top coordinate");
      return(ERR);
   }

   if(VarType(&colVar) != MATRIX2D && VarRowSize(&colVar) != 1)
   {
      ErrorMessage("invalid RGB colour");
      return(ERR);
   }
   
   float xb = VarRealMatrix(&baseVar)[0][0];
   float yb = VarRealMatrix(&baseVar)[0][1];
   float zb = VarRealMatrix(&baseVar)[0][2];
 
   float xt = VarRealMatrix(&topVar)[0][0];
   float yt = VarRealMatrix(&topVar)[0][1];
   float zt = VarRealMatrix(&topVar)[0][2]; 
   
   if(VarColSize(&colVar) >= 3)
   {
	   r = VarRealMatrix(&colVar)[0][0];
	   g = VarRealMatrix(&colVar)[0][1];
	   b = VarRealMatrix(&colVar)[0][2];
	}
   if(VarColSize(&colVar) == 4)
      alpha = VarRealMatrix(&colVar)[0][3];

   glNewList(++plot3d->count3D, GL_COMPILE);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
    	glEnable(GL_LIGHTING);
      ClippingControl();
	   if(alpha < 1)   
	   {
	     glEnable(GL_BLEND);
	     glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	     glColor4f(r,g,b,alpha);
	   }
	   else
	   {
		   glColor3f(r,g,b);
		}   

      DrawCylinder(GetXYZ(xb,yb,zb), GetXYZ(xt,yt,zt), radius);

	   if(alpha < 1)   
	     glDisable(GL_BLEND);
	   glPopMatrix();
   glEndList();

   Invalidate3DPlot(cur3DWin);
   
	itfc->nrRetValues = 0;
   return(OK);
}

/*****************************************************************************
* Add a sphere to the display list
*****************************************************************************/

int DrawSphere(Interface* itfc ,char args[])
{
   float radius;
   Variable colVar,posVar;
   short nrArgs;
   GLUquadricObj *obj;
   short steps = 18;

// Get arguments from user *************
   
   if((nrArgs = ArgScan(itfc,args,3,"pos,radius,colour,steps","eeee","vfvd",&posVar,&radius,&colVar,&steps)) < 0)
     return(nrArgs); 

   if(VarType(&posVar) != MATRIX2D && VarRowSize(&posVar) != 1 && VarColSize(&posVar) != 3)
   {
      ErrorMessage("invalid sphere coordinate");
      return(ERR);
   }
        
   if(VarType(&colVar) != MATRIX2D && VarRowSize(&colVar) != 1 && VarColSize(&colVar) != 3 && VarColSize(&colVar) != 4)
   {
      ErrorMessage("invalid RGB colour");
      return(ERR);
   }

   float x = VarRealMatrix(&posVar)[0][0];
   float y = VarRealMatrix(&posVar)[0][1];
   float z = VarRealMatrix(&posVar)[0][2];


   float r = VarRealMatrix(&colVar)[0][0];
   float g = VarRealMatrix(&colVar)[0][1];
   float b = VarRealMatrix(&colVar)[0][2];
   float a = 1.0;
   if(VarColSize(&colVar) == 4)
      a = VarRealMatrix(&colVar)[0][3];

   obj = gluNewQuadric();
   
   glNewList(++plot3d->count3D, GL_COMPILE);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
    	glEnable(GL_LIGHTING);
      ClippingControl();
      glTranslatef(x,y,z);
      if(a < 1)
      {
         glEnable(GL_BLEND);
	      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      }
      glColor4f(r,g,b,a);
      gluSphere(obj,radius,steps,steps);
      glPopMatrix();
   glEndList();

   Invalidate3DPlot(cur3DWin);
	itfc->nrRetValues = 0;
   return(OK); 
}

//int SetGLMode(char args[])
//{
//
//   short nrArgs;
//   char mode[20];
//
//// Get arguments from user *************
//   
//   if((nrArgs = ArgScan(itfc,args,1,"mode","e","s",mode)) < 0)
//     return(nrArgs); 
//
//   if(!strcmp(mode,"bmp"))
//      plot3d->glWin3d.SetMode(GLBITMAP);
//   else
//      plot3d->glWin3d.SetMode(GLSCREEN);
//
//   RECT r;
//
//   GetClientRect(openGlWin, &r); 
//   Initialize3DParameters();
//   Set3DMapping(0,0, r.right, r.bottom);  
//   SetUpLightModel();
//
//   return(0);
//
//}

int SetFogRange(Interface *itfc, char args[])
{
   short nrArgs;

// Get arguments from user ************* 
   if((nrArgs = ArgScan(itfc,args,2,"start,end","ee","ff",&fogStart,&fogEnd)) < 0)
     return(nrArgs); 

   Invalidate3DPlot(cur3DWin);

	itfc->nrRetValues = 0;
   return(OK); 
}




int LineWidth3d(Interface* itfc ,char args[])
{
   short nrArgs;
   float lineWidth;

// Get arguments from user ************* 
   if((nrArgs = ArgScan(itfc,args,1,"line width","e","f",&lineWidth)) < 0)
     return(nrArgs); 

   glNewList(++plot3d->count3D, GL_COMPILE);
      glLineWidth(lineWidth);
   glEndList();

	itfc->nrRetValues = 0;
   return(OK); 
}

int Draw3DPlane(Interface *itfc, char args[])
{
   short nrArgs;
   Variable colorVar;
   float xMin,xMax,yMin,yMax,position;
   CText direction;

// Get arguments from user ************* 
   if((nrArgs = ArgScan(itfc,args,7,"direction, position, xmin, xmax, ymin, ymax, color","eeeeeee","tfffffv",&direction,&position,&xMin,&xMax,&yMin,&yMax,&colorVar)) < 0)
     return(nrArgs); 

   if(direction == "off")
   {
      plot3d->draw3DPlane = false;
   }
   else
   {
      plot3d->planePosition = position; 
      plot3d->draw3DPlane = true;
      if(nrArgs == 7)
      {
         plot3d->planeXMin = xMin;
         plot3d->planeXMax = xMax;
         plot3d->planeYMin = yMin;
         plot3d->planeYMax = yMax;
         if(direction == "xy")
            plot3d->planeDirection = XY_PLANE;
         else if(direction == "xz")
            plot3d->planeDirection = XZ_PLANE;
         else if(direction == "yz")
            plot3d->planeDirection = YZ_PLANE;

         if(VarType(&colorVar) != MATRIX2D && VarHeight(&colorVar) != 1)
         {
            ErrorMessage("invalid RGB colour");
            return(ERR);
         }

         if(VarWidth(&colorVar) >= 3)
         {
	         plot3d->planeRed = VarRealMatrix(&colorVar)[0][0];
	         plot3d->planeGreen = VarRealMatrix(&colorVar)[0][1];
	         plot3d->planeBlue = VarRealMatrix(&colorVar)[0][2];
	      }
         if(VarWidth(&colorVar) == 4)
         {
	         plot3d->planeAlpha = VarRealMatrix(&colorVar)[0][3];
	      }
      }
   }

// Redraw the plot
   Invalidate3DPlot(cur3DWin);

	itfc->nrRetValues = 0;
   return(OK); 
}


int Set3DClippingPlane(Interface *itfc, char args[])
{
   short nrArgs;
   static short number;
   float position;
   CText plane;
   CText side;
   short sign;

// Initialize variable ***********************************
   if(clipPlane1Direction == XY_PLANE)
      plane = "xy";
   else if(clipPlane1Direction == XZ_PLANE)
      plane = "xz";
   else if(clipPlane1Direction == YZ_PLANE)
      plane = "yz";

   if(clipPlane1Side == 1)
      side = "plus";
   else if(clipPlane1Side == -1)
      side = "minus";

   position = clipPlane1Position;


// Get arguments from user ******************************* 
   if((nrArgs = ArgScan(itfc,args,4,"number, position, plane, side","eeee","dftt",&number,&position,&plane,&side)) < 0)
     return(nrArgs); 

// See if we are switching off the clipping plane ********
   if(plane == "off")
   {
      if(number == 1)
         clipPlane1 = false;
      else if(number == 2)
         clipPlane2 = false;
      else
      {
         ErrorMessage("Invalid plane number");
         return(ERR);
      }
      Invalidate3DPlot(cur3DWin);
      return(OK); 
   }

// Which side to we want to be visible?
   if(side == "plus")
   {
      sign = 1;
   }
   else if(side == "minus")
   {
      sign = -1;
   }
   else
   {
      ErrorMessage("invalid clipping side");
      return(ERR);
   }

// Set up the clipping global variables ********************
   if(number == 1)
   {
      clipPlane1 = true;
      clipPlane1Side = sign;
      clipPlane1Position = position;
      if(plane == "xy")
         clipPlane1Direction = XY_PLANE;
      else if(plane == "xz")
         clipPlane1Direction = XZ_PLANE;
      else if(plane == "yz")
         clipPlane1Direction = YZ_PLANE;
   }
   else if(number == 2)
   {
      clipPlane2 = true;
      clipPlane2Side = sign;
      clipPlane2Position = position;
      if(plane == "xy")
         clipPlane2Direction = XY_PLANE;
      else if(plane == "xz")
         clipPlane2Direction = XZ_PLANE;
      else if(plane == "yz")
         clipPlane2Direction = YZ_PLANE;
   }
   else
   {
      ErrorMessage("Invalid plane number");
      return(ERR);
   }

// Redraw the plot
   Invalidate3DPlot(cur3DWin);
   
   return(OK); 
}

// Add a color scale to the display list

int Draw3DColorScale(Interface *itfc, char args[])
{
   short nrArgs;
   if(!plot3d)
      return(0);

   float fontSize = plot3d->colorScaleFontSize;
 	Variable scaleV,rangeV,fontColorV;

// Get arguments from user ************* 
   if((nrArgs = ArgScan(itfc,args,2,"scale, range [font color[, font_size]]","eeee","vvvf",&scaleV,&rangeV,&fontColorV,&fontSize)) < 0)
     return(nrArgs); 

// Get color scale *********************
   if(VarType(&scaleV) == MATRIX2D && VarWidth(&scaleV) == 3 && VarHeight(&scaleV) >= 1)
   {
      int h = VarHeight(&scaleV);
      if(plot3d->colorMap) FreeMatrix2D(plot3d->colorMap);
      plot3d->colorMap = CopyMatrix(VarRealMatrix(&scaleV),3,h);
      plot3d->colorMapLength = h;
   }
   else
   {
      ErrorMessage("Invalid scale matrix");
      return(ERR);
   }

// Get range vector *********************
   if(VarType(&rangeV) == MATRIX2D && VarWidth(&rangeV) == 2 && VarHeight(&rangeV) == 1)
   {
	   plot3d->colorScaleMinValue = VarRealMatrix(&rangeV)[0][0];
	   plot3d->colorScaleMaxValue = VarRealMatrix(&rangeV)[0][1];
   }
   else
   {
      ErrorMessage("Invalid range vector");
      return(ERR);
   }

// Get font color vector *********************
   if(nrArgs > 2)
   {
      if(VarType(&fontColorV) == MATRIX2D && VarWidth(&fontColorV) == 3 && VarHeight(&fontColorV) == 1)
      {
	      plot3d->colorScaleRed = VarRealMatrix(&fontColorV)[0][0];
	      plot3d->colorScaleGreen = VarRealMatrix(&fontColorV)[0][1];
	      plot3d->colorScaleBlue = VarRealMatrix(&fontColorV)[0][2];
      }
      else
      {
         ErrorMessage("Invalid font colour vector");
         return(ERR);
      }
   }

// Get font size
   if(nrArgs == 4)
   {
      if(fontSize < 1 || fontSize > 10)
      {
         ErrorMessage("Invalid font size");
         return(ERR);
      }
      plot3d->colorScaleFontSize = fontSize;
   }
	
	itfc->nrRetValues = 0;
   return(OK);
}

/**************************************************************************
  Set the size for the current 3D text 
**************************************************************************/

int Set3DTextSize(Interface *itfc, char args[])
{
   short nrArgs;
   float size;

// Get arguments from user ************* 
   if((nrArgs = ArgScan(itfc,args,1,"text-size","e","f",&size)) < 0)
     return(nrArgs); 

   if(size >= 0)
      gTextSize3D = size;
   else
   {
      ErrorMessage("text size must be positive");
      return(ERR);
   }

	itfc->nrRetValues = 0;
   return(OK); 
}



int Set3DLabelSizes(Interface *itfc, char args[])
{
   short nrArgs;
   float tickLength;
   float axesLabelLength;
   float numberSize;
   float labelSize;

// Get arguments from user ************* 
   if((nrArgs = ArgScan(itfc,args,3,"tick length, label length, number size, label size ","eeeee","fffff",&tickLength,&axesLabelLength,&numberSize,&labelSize)) < 0)
     return(nrArgs); 
 
   if(tickLength >= 0 && axesLabelLength >= 0 && numberSize >= 0 && labelSize >= 0)
   {
      gAxesTickLength = tickLength;
      gAxesLabelLength = axesLabelLength;
      gAxesNumberSize = numberSize;
      gTextSize3D = labelSize;
   }
   else
   {
      ErrorMessage("all parameters must be positive");
      return(ERR);
   }
   return(OK); 
}

int DrawBox(Interface* itfc ,char args[])
{
   float xmin,xmax,ymin,ymax,zmin,zmax;
   Variable colVar,posVar,dimVar;
   short nrArgs;

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,6,"xmin,xmax,ymin,ymax,zmin,zmax","eeeeee","ffffff",&xmin,&xmax,&ymin,&ymax,&zmin,&zmax)) < 0)
     return(nrArgs); 

   glNewList(++plot3d->count3D, GL_COMPILE);
	   glMatrixMode(GL_MODELVIEW);
	   glPushMatrix();
	   glDisable(GL_LIGHTING);
      ClippingControl();
      DrawWireFrameBox(xmin,xmax,ymin,ymax,zmin,zmax);
	   glEnable(GL_LIGHTING);
	   glPopMatrix();
   glEndList();

   Invalidate3DPlot(cur3DWin);
   
   return(OK); 
}


// Make sure that the 3D plot region is redrawn but not the boundaries

void Invalidate3DPlot(HWND hWnd)
{
 	PAINTSTRUCT p;
	MyInvalidateRect(hWnd,NULL,false);

	HDC hdc = BeginPaint(hWnd, &p ); 
     SurfacePlot3D(hWnd,hdc);                  
	EndPaint(hWnd, &p );
}




/*****************************************************************************
* Set the current 3D color
*****************************************************************************/

//int Set3DRotOrigin(char args[])
//{
//
//   short nrArgs;
//
//// Get arguments from user *************
//   
//   if((nrArgs = ArgScan(itfc,args,1,"mode","e","s",rotmode)) < 0)
//     return(nrArgs); 
// 
//  
//   return(OK); 
//}


/*****************************************************************************
* Set the current 3D color
*****************************************************************************/

int Set3DColor(Interface *itfc, char arg[])
{
   Variable colorVar;
   float r,g,b;
   short nrArgs;

// Get arguments from user *************
   
   if((nrArgs = ArgScan(itfc,arg,1,"color","e","v",&colorVar)) < 0)
     return(nrArgs); 
 
   if(VarType(&colorVar) != MATRIX2D || (VarHeight(&colorVar) != 1 || VarWidth(&colorVar) != 3))
   {
      ErrorMessage("invalid RGB colour");
      return(ERR);
   }

   r = VarRealMatrix(&colorVar)[0][0];
   g = VarRealMatrix(&colorVar)[0][1];
   b = VarRealMatrix(&colorVar)[0][2];

   glNewList(++plot3d->count3D, GL_COMPILE);
     glColor3f(r,g,b);
   glEndList();

	itfc->nrRetValues = 0;
   return(OK); 
}

/*****************************************************************************
* Set the current 3D color
*****************************************************************************/

int Set3DBkColor(Interface *itfc, char args[])
{
   Variable colorVar;
   short nrArgs;

   float* colors = new float[3];
   colors[0] = bkColor[0];
   colors[1] = bkColor[1];
   colors[2] = bkColor[2];
   (&colorVar)->MakeMatrix2DFromVector(colors,3,1);
   delete [] colors;

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,1,"color","e","v",&colorVar)) < 0)
     return(nrArgs); 
 
// Make sure its and RGB matrix
   if(VarType(&colorVar) != MATRIX2D && VarHeight(&colorVar) != 1 && VarWidth(&colorVar) != 3)
   {
      ErrorMessage("invalid RGB colour");
      return(ERR);
   }

// Upate the background color
   bkColor[0] = fogColor[0] = VarRealMatrix(&colorVar)[0][0]/255.0;
   bkColor[1] = fogColor[1] = VarRealMatrix(&colorVar)[0][1]/255.0;
   bkColor[2] = fogColor[2] = VarRealMatrix(&colorVar)[0][2]/255.0;
 
   Invalidate3DPlot(cur3DWin);
  
   return(OK); 
}


/*****************************************************************************
* Add a 3D axis to the display list
*****************************************************************************/

int Draw3DAxis(Interface *itfc, char args[])
{
   Variable axisRangeVar;
   Variable labelRangeVar;
   short nrArgs;
   CText direction;
   CText label;
   float pos1,pos2;
   float minLabel = 0,maxLabel = 0;
   CText label_position = "centre";
   short label_pos = 0;
   CText axisDirection = "forward";
   short axisDir = 0;

// Get arguments from user *************   
   if((nrArgs = ArgScan(itfc,args,4,"direction,axisrange,pos1,pos2,[labelrange,label,label_position,axis_direction]","eeeeeeee","tvffvttt",&direction,&axisRangeVar,&pos1,&pos2,&labelRangeVar,&label,&label_position,&axisDirection)) < 0)
     return(nrArgs); 
    

   if(VarType(&axisRangeVar) != MATRIX2D && VarHeight(&axisRangeVar) != 1 && VarWidth(&axisRangeVar) != 2)
   {
      ErrorMessage("invalid axis range vector");
      return(ERR);
   }

   float minAxis = VarRealMatrix(&axisRangeVar)[0][0];
   float maxAxis = VarRealMatrix(&axisRangeVar)[0][1];

   if(minAxis >= maxAxis)
   {
      ErrorMessage("invalid axis range vector");
      return(ERR);
   }

   if(nrArgs == 4)
   {
      if(direction[0] == 'x')
         label = "x";
      else if(direction[0] == 'y')
         label = "y";
      else if(direction[0] == 'z')
         label = "z";

      minLabel = minAxis;
      maxLabel = maxAxis;
   }

   if(nrArgs == 5)
   {
      if(direction[0] == 'x')
         label = "x";
      else if(direction[0] == 'y')
         label = "y";
      else if(direction[0] == 'z')
         label = "z";
   }

   if(nrArgs >= 6)
   {
      if(VarType(&labelRangeVar) != MATRIX2D && VarHeight(&labelRangeVar) != 1 && VarWidth(&labelRangeVar) != 2)
      {
         ErrorMessage("invalid label range vector");
         return(ERR);
      }

      minLabel = VarRealMatrix(&labelRangeVar)[0][0];
      maxLabel = VarRealMatrix(&labelRangeVar)[0][1];

      if(minLabel >= maxLabel)
      {
         ErrorMessage("invalid axis range vector");
         return(ERR);
      }
   }

   if(label_position == "centre")
      label_pos = 0;
   else if(label_position == "positive_end")
      label_pos = 1;
   else if(label_position == "negative_end")
      label_pos = -1;

   plot3d->SetSizes(gAxesTickLength,gAxesLabelLength,gAxesNumberSize,gTextSize3D);

   if(axisDirection == "forward")
      axisDir = 0;
   else
      axisDir = 1;

// Draw axis at origin
   glNewList(++plot3d->count3D, GL_COMPILE);
	   glMatrixMode(GL_MODELVIEW);
	   glPushMatrix();
	   glDisable(GL_LIGHTING);
      ClippingControl();
      if(direction == "x") plot3d->DrawXAxis(label.Str(),minLabel,maxLabel,minAxis,maxAxis,pos1,pos2,label_pos,axisDir);
      if(direction == "y") plot3d->DrawYAxis(label.Str(),minLabel,maxLabel,minAxis,maxAxis,pos1,pos2,label_pos,axisDir);
      if(direction == "z") plot3d->DrawZAxis(label.Str(),minLabel,maxLabel,minAxis,maxAxis,pos1,pos2,label_pos,axisDir);
	   glEnable(GL_LIGHTING);
	   glPopMatrix();
   glEndList();

   Invalidate3DPlot(cur3DWin);
   
   return(OK);   
}



/*****************************************************************************
* Add a 3D axes to the display list
*****************************************************************************/

int Draw3DAxes(Interface *itfc, char args[])
{
   Variable colVar,originVar;
   float length;
   short nrArgs;
   float size;
 //  CPlot3D c3d;
   float r = 0,g = 0,b = 0;

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,2,"origin, length, color, font_size","eeee","vfvf",&originVar,&length,&colVar,&size)) < 0)
     return(nrArgs); 
    
   if(VarType(&originVar) != MATRIX2D || VarHeight(&originVar) != 1 || VarWidth(&originVar) != 3)
   {
      ErrorMessage("invalid origin vector");
      return(ERR);
   }

   float x = VarRealMatrix(&originVar)[0][0];
   float y = VarRealMatrix(&originVar)[0][1];
   float z = VarRealMatrix(&originVar)[0][2];

   if(nrArgs >= 3)
   {
      if(VarType(&colVar) != MATRIX2D && VarHeight(&colVar) != 1 && VarWidth(&colVar) != 3)
      {
         ErrorMessage("invalid RGB colour");
         return(ERR);
      }

      r = VarRealMatrix(&colVar)[0][0];
      g = VarRealMatrix(&colVar)[0][1];
      b = VarRealMatrix(&colVar)[0][2];
   }


// Draw axis at origin

   glNewList(++plot3d->count3D, GL_COMPILE);
	   glMatrixMode(GL_MODELVIEW);
	   glPushMatrix();
	   glDisable(GL_LIGHTING);
      ClippingControl();
      if(nrArgs == 4)
         gTextSize3D = size;
      plot3d->SetSizes(gAxesTickLength,gAxesLabelLength,gAxesNumberSize,gTextSize3D);
      glTranslatef(x,y,z);
      if(nrArgs >= 3) glColor3f(r,g,b);
      plot3d->DrawSimpleAxes(length);
	   glPopMatrix();
   glEndList();

   Invalidate3DPlot(cur3DWin);
   
	itfc->nrRetValues = 0;
   return(OK);   
}


//int Draw3DRectange(char args[])
//{
//   Variable colVar;
//   short nrArgs;
//   float width,height;
//
//// Get arguments from user *************
//   if((nrArgs = ArgScan(itfc,args,2,"width,height","ee","ff",&width,&height)) < 0)
//     return(nrArgs); 
//     
//     
//// Draw axis at origin
//   glNewList(++plot3d->count3D, GL_COMPILE);
//	   glMatrixMode(GL_MODELVIEW);
//	   glPushMatrix();
//	   glDisable(GL_LIGHTING);
//      ClippingControl();
//	   glColor3f(1.0,0,0);
//	   glBegin(GL_QUADS);
//	     glVertex3f(-width/2.0,-height/2.0,0.0);
//	     glVertex3f(width/2.0,-height/2.0,0.0);
//	     glVertex3f(width/2.0,height/2.0,0.0);
//	     glVertex3f(-width/2.0,height/2.0,0.0);
//	   glEnd();
//
//	   glPopMatrix();
//   glEndList();
//
//   Invalidate3DPlot(cur3DWin);
//   
//   return(OK);   
//}

/*****************************************************************************
* Specify the rotation angle for the 3D plot
*****************************************************************************/

int Rotate3DPlot(Interface* itfc ,char args[])
{
   short nrArgs;
   CText txt;
   
// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,3,"elevation, azimuth, twist","eee","fff",&plot3d->elevation, &plot3d->azimuth, &plot3d->twist)) < 0)
     return(nrArgs); 

// Update status bar
   txt.Format("elevation = %2.1f azimuth = %2.1f twist = %2.1f distance = %2.1f",plot3d->elevation,plot3d->azimuth,plot3d->twist,plot3d->viewDistance);
   UpdateStatusWindow(cur3DWin,0,txt.Str());

// Force 3D plot to be redisplayed
   Invalidate3DPlot(cur3DWin);
	
	itfc->nrRetValues = 0;
   return(OK);   
}

/*****************************************************************************
* Specify whether images in the 3D windows should be immediately draw or not
*****************************************************************************/

int Draw3DPlot(Interface *itfc, char args[])
{
   short nrArgs;
   CText draw;

   if((nrArgs = ArgScan(itfc,args,1,"true/false","e","t",&draw)) < 0)
     return(nrArgs);
   
   if(draw == "true" || draw == "yes" || draw == "on")
   {
      update3d = true;
   }
   else if(draw == "false" || draw == "no" || draw == "off")
   {
      update3d = false;
   }
   else
   {
      ErrorMessage("invalid argument");
      return(ERR);
   }
   
   if(update3d)
   {
      Invalidate3DPlot(cur3DWin);	
   }
     
	itfc->nrRetValues = 0;
   return(OK);     
}   

/*****************************************************************************
* Set the offsets for the current 3D plot
*****************************************************************************/
 
int Shift3DPlot(Interface *itfc, char args[])
{
   short nrArgs;

// Get arguments from user *************  
   if((nrArgs = ArgScan(itfc,args,3,"xoff, yoff, zoff","eee","fff",&plot3d->xoff, &plot3d->yoff, &plot3d->zoff)) < 0)
     return(nrArgs); 

// Force 3D plot to be redisplayed
   Invalidate3DPlot(cur3DWin);   
  
	itfc->nrRetValues = 0;
   return(OK);   
} 

/*****************************************************************************
* Set the scale factors for the current 3D plot
*****************************************************************************/
 
int Scale3DPlot(Interface* itfc ,char args[])
{
   short nrArgs;
   float xs,ys,zs;

   xs = plot3d->xScale;
   ys = plot3d->yScale;
   zs = plot3d->zScale;

// Get arguments from user *************  
   if((nrArgs = ArgScan(itfc,args,3,"xScale, yScale, zScale","eee","fff",&xs,&ys,&zs)) < 0)
     return(nrArgs); 

   plot3d->xoff = plot3d->xoff/plot3d->xScale*xs;
   plot3d->yoff = plot3d->yoff/plot3d->yScale*ys;
   plot3d->zoff = plot3d->zoff/plot3d->zScale*zs;

   plot3d->xScale = xs;
   plot3d->yScale = ys;
   plot3d->zScale = zs;

// Force 3D plot to be redisplayed
   Invalidate3DPlot(cur3DWin);
  
	itfc->nrRetValues = 0;
   return(OK);   
} 

int Set3DLight(Interface *itfc, char args[])
{
   short nrArgs;

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,3,"xPos, yPos, zPos","eee","fff",&xLight, &yLight, &zLight)) < 0)
     return(nrArgs); 

// Force 3D plot to be redisplayed
   Invalidate3DPlot(cur3DWin);
	  
	itfc->nrRetValues = 0;
   return(OK);   
}    

/*****************************************************************************
* Add a 2D surface plot to the display list
*****************************************************************************/

int Display2DSurface(Interface* itfc ,char args[])
{
   Variable scaleVar,matVar,dataRangeVar,colorMapRange;
   short nrArgs;
   long width,height;
   long top,left;
   float **mat = 0;
   complex **cmat = 0;
   float **colorMap;
   float minVal,maxVal;
   float minMap = 0;
   float maxMap = 1;
   long colorMapLength;
   CText xDirTxt = "forward";
   CText yDirTxt = "forward";
   CText makeSquareTxt = "yes";

   if((nrArgs = ArgScan(itfc,args,3,"matrix, colour, data-range, map-range, xdir, ydir","eeeeeee","vvvvttt",
                        &matVar, &scaleVar, &dataRangeVar, &colorMapRange, &xDirTxt, &yDirTxt, &makeSquareTxt)) < 0)
     return(nrArgs); 

// Extract matrix type and sizes **************
   if(VarType(&matVar) == MATRIX2D)
   {
      mat = VarRealMatrix(&matVar);
      cmat = 0;
   }
   else if (VarType(&matVar) == CMATRIX2D)
   {
      cmat = VarComplexMatrix(&matVar);
      mat = 0;
   }
   else
   {
      ErrorMessage("Invalid matrix");
      return(ERR);
   }
   left = top = 0;
   width = VarWidth(&matVar);
   height = VarHeight(&matVar);

// Make sure there is a matrix to display
   if(!mat && !cmat)
   {
      ErrorMessage("no 2D plot defined");
      return(ERR);
   }

// Extract color ********************************
   if(VarWidth(&scaleVar) == 3 && VarHeight(&scaleVar) >= 1)
   {
	   colorMap = VarRealMatrix(&scaleVar);
      colorMapLength = VarHeight(&scaleVar);
   }
   else
   {
      ErrorMessage("Invalid RGB matrix");
      return(ERR);
   }
	

   if(VarWidth(&dataRangeVar) == 2 && VarHeight(&dataRangeVar) ==1)
   {
	   minVal  = VarRealMatrix(&dataRangeVar)[0][0];
	   maxVal  = VarRealMatrix(&dataRangeVar)[0][1];
      if(maxVal <= minVal)
      {
         ErrorMessage("Invalid scale range vector");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Invalid scale range vector (2 elements expected)");
      return(ERR);
   }


   if(nrArgs >= 4)
   {
      if(VarWidth(&colorMapRange) == 2 && VarHeight(&colorMapRange) ==1)
      {
	      minMap  = VarRealMatrix(&colorMapRange)[0][0];
	      maxMap  = VarRealMatrix(&colorMapRange)[0][1];
         if(maxMap <= minMap)
         {
            ErrorMessage("Invalid scale range vector");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("Invalid scale range vector (2 elements expected)");
         return(ERR);
      }
	}


	SetCursor(LoadCursor(NULL,IDC_WAIT));


// Start recording GL commands
   glNewList(++plot3d->count3D, GL_COMPILE);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); 
	  
   glEnable(GL_NORMALIZE);
   ClippingControl();

   plot3d->viewDistance = 1.5*width/(2.0*tan(VIEW_ANGLE/2.0*PI/180));

   short xDir,yDir;
   bool makeSquare;
   (xDirTxt == "forward") ? xDir = 0 : xDir = 1;
   (yDirTxt == "forward") ? yDir = 0 : yDir = 1;
   (makeSquareTxt == "yes") ? makeSquare = true : makeSquare = false;
// Draw the surface plot 	   
  	if(mat)	   	   
      SurfacePlot2D(mat,left,width,top,height,10.0,colorMap,colorMapLength,minVal,maxVal,minMap,maxMap,xDir,yDir,makeSquare);
	
   glPopMatrix();
   glEndList();

// Force 3D plot to be redisplayed
   Invalidate3DPlot(cur3DWin);

   return(OK);
}


/*****************************************************************************
* Add a 2D waterfall plot to the display list
*****************************************************************************/

int Waterfall(Interface* itfc ,char args[])
{
   Variable colorVar,matVar,dataRangeVar,colorMapRange;
   short nrArgs;
   long width,height;
   long top,left;
   float **mat = 0;
   complex **cmat = 0;
   float **colorMap;
   float minVal,maxVal;
   float minMap = 0;
   float maxMap = 1;
   long colorMapLength;
   float lineWidth = 1.5;
   CText xDirTxt = "forward";
   CText yDirTxt = "forward";
   CText makeSquareTxt = "yes";
   float	color[3];

 //  if((nrArgs = ArgScan(itfc,args,3,"matrix, colour, data-range, map-range, xdir, ydir","eeeeeee","vvvvttt",
 //                       &matVar, &scaleVar, &dataRangeVar, &colorMapRange, &xDirTxt, &yDirTxt, &makeSquareTxt)) < 0)

   if((nrArgs = ArgScan(itfc,args,2,"matrix, colour, data-range, xdir, ydir, line-width","eeeeeee","vvvtttf",
                        &matVar, &colorVar,&dataRangeVar,&xDirTxt, &yDirTxt, &makeSquareTxt, &lineWidth)) < 0)
     return(nrArgs); 

// Extract matrix type and sizes **************
   if(VarType(&matVar) == MATRIX2D)
   {
      mat = VarRealMatrix(&matVar);
      cmat = 0;
   }
   else if (VarType(&matVar) == CMATRIX2D)
   {
      cmat = VarComplexMatrix(&matVar);
      mat = 0;
   }
   else
   {
      ErrorMessage("Invalid matrix");
      return(ERR);
   }
   left = top = 0;
   width = VarWidth(&matVar);
   height = VarHeight(&matVar);

// Make sure there is a matrix to display
   if(!mat)
   {
      ErrorMessage("no real 2D plot defined");
      return(ERR);
   }


// Extract color ********************************
   //if(VarWidth(&scaleVar) == 3 && VarHeight(&scaleVar) >= 1)
   //{
	  // colorMap = VarRealMatrix(&scaleVar);
   //   colorMapLength = VarHeight(&scaleVar);
   //}
   //else
   //{
   //   ErrorMessage("Invalid RGB matrix");
   //   return(ERR);
   //}

// Extract color ********************************
   if(VarWidth(&colorVar) == 3 && VarHeight(&colorVar) == 1)
   {
      color[0] = colorVar.GetMatrix2D()[0][0];
      color[1] = colorVar.GetMatrix2D()[0][1];
      color[2] = colorVar.GetMatrix2D()[0][2];
   }
   else
   {
      ErrorMessage("Invalid RGB color vector");
      return(ERR);
   }	

   if(VarWidth(&dataRangeVar) == 2 && VarHeight(&dataRangeVar) ==1)
   {
	   minVal  = VarRealMatrix(&dataRangeVar)[0][0];
	   maxVal  = VarRealMatrix(&dataRangeVar)[0][1];
      if(maxVal <= minVal)
      {
         ErrorMessage("Invalid scale range vector");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Invalid scale range vector (2 elements expected)");
      return(ERR);
   }


 //  if(nrArgs == 4)
 //  {
 //     if(VarWidth(&colorMapRange) == 2 && VarHeight(&colorMapRange) ==1)
 //     {
	//      minMap  = VarRealMatrix(&colorMapRange)[0][0];
	//      maxMap  = VarRealMatrix(&colorMapRange)[0][1];
 //        if(maxMap <= minMap)
 //        {
 //           ErrorMessage("Invalid scale range vector");
 //           return(ERR);
 //        }
 //     }
 //     else
 //     {
 //        ErrorMessage("Invalid scale range vector (2 elements expected)");
 //        return(ERR);
 //     }
	//}


	SetCursor(LoadCursor(NULL,IDC_WAIT));


// Start recording GL commands
   glNewList(++plot3d->count3D, GL_COMPILE);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); 
	  
   glEnable(GL_NORMALIZE);
   ClippingControl();

   plot3d->viewDistance = 1.5*width/(2.0*tan(VIEW_ANGLE/2.0*PI/180));

   short xDir,yDir;
   bool makeSquare;
   (xDirTxt == "forward") ? xDir = 0 : xDir = 1;
   (yDirTxt == "forward") ? yDir = 0 : yDir = 1;
   (makeSquareTxt == "yes") ? makeSquare = true : makeSquare = false;
// Draw the surface plot 	   
  	if(mat)	   	   
      WaterfallPlot(mat,left,width,top,height,10.0,color,maxVal,minVal,xDir,yDir,makeSquare,lineWidth);
	
   glPopMatrix();
   glEndList();

// Force 3D plot to be redisplayed
   Invalidate3DPlot(cur3DWin);

   return(OK);
}


void SetRGBColor(float value, float **colorMap, long length);

/*************************************************************************

   Plot the surface of matrix mat between the limits 
   dataLeft->dataLeft+dataWidth and dataTop->dataTop+dataHeight
   The data is vertically scaled by factor vscale.
   The matrix colorMap is used to linearly map data values to colours while the
   start end parameters determine the limits of this mapping

*************************************************************************/

float GetSurfColor(float data, float minVal, float maxVal, short cs_mode);

void SurfacePlot2D(float **mat, long dataLeft, long dataWidth, long dataTop, long dataHeight, float vscale, float **colorMap, long colorMapLength, float minDatVal, float maxDatVal, float minCsVal, float maxCsVal, short xDir, short yDir, bool makeSquare)
{
   long xd,yd;
   long xm1,ym1;
   long xm2,ym2;
   float xp,yp;
   float v1,v2,v3,v4;
   float c1,c2,c3,c4;
   float vt0[3],vt1[3],vt2[3];
   float n[3];
   float ***normL;
   float ***normU;
   float ***norma;
   float datSF;
   short cs_mode;

	//GLfloat	 lightPos[] = { -50.f, 50.0f, 100.0f, 1.0f };
   
   if(!mat) return;

// Work out scale factor
   datSF = 1.0/(maxDatVal-minDatVal);
   if(nint(colorMap[colorMapLength-1][0]) == PLUS_MINUS_CMAP)
      cs_mode = PLUS_MINUS_CMAP;
   else
      cs_mode = NORMAL_CMAP;
  
// Work out x-y geometry
   float xSF = 1.0;
   float ySF = 1.0;

   if(makeSquare)
   {
      if(dataWidth > dataHeight)
         ySF = dataWidth/(float)dataHeight;
      else
         xSF = dataHeight/(float)dataWidth;
   }

   if(surface2DMode == FLAT_SHADING)
   {   
	   glBegin(GL_TRIANGLES);
	   for(yd = dataTop; yd < dataHeight+dataTop-1; yd++) // Loop over data coords
	   {
         if(yDir)
         {
            ym1 = dataHeight+2*dataTop-1-yd; // Matrix index
            ym2 = ym1-1;
         }
         else
         {
            ym1 = yd;
            ym2 = yd+1;
         }

	   	for(xd = dataLeft; xd < dataWidth+dataLeft-1; xd++) // Loop over data coords 
	      {
            if(xDir)
            {
               xm1 = dataWidth+2*dataLeft-1-xd; // Matrix index
               xm2 = xm1-1;
            }
            else
            {
               xm1 = xd;
               xm2 = xd+1;
            }

        // Calculate vertex amplitudes
            v1 = datSF*(mat[ym1][xm1]);
            v2 = datSF*(mat[ym1][xm2]);
            v3 = datSF*(mat[ym2][xm2]);
            v4 = datSF*(mat[ym2][xm1]);

        // Calculate vertex colors
            c1 = GetSurfColor(mat[ym1][xm1],minCsVal,maxCsVal,cs_mode);
            c2 = GetSurfColor(mat[ym1][xm2],minCsVal,maxCsVal,cs_mode);
            c3 = GetSurfColor(mat[ym2][xm2],minCsVal,maxCsVal,cs_mode);
            c4 = GetSurfColor(mat[ym2][xm1],minCsVal,maxCsVal,cs_mode);

       // Plot coordinates
            xp = (xd+0.5)*xSF;
            yp = (yd+0.5)*ySF;

        // Lower triangle
	         vt0[0] = xp;     vt0[1] = yp;      vt0[2] = v1;
	         vt1[0] = xp+xSF; vt1[1] = yp;      vt1[2] = v2;
	         vt2[0] = xp+xSF; vt2[1] = yp+ySF;  vt2[2] = v3;

	         CalcNormal(vt0,vt1,vt2,n);
			   glNormal3f(n[0],n[1],n[2]);
            SetRGBColor(c1,colorMap,colorMapLength);
		   	glVertex3f(xp,yp,v1);
            SetRGBColor(c2,colorMap,colorMapLength);
			   glVertex3f(xp+xSF,yp,v2);
            SetRGBColor(c3,colorMap,colorMapLength);
			   glVertex3f(xp+xSF,yp+ySF,v3);

         // Upper triangle
	         vt0[0] = xp;     vt0[1] = yp;      vt0[2] = v1;
	         vt1[0] = xp+xSF; vt1[1] = yp+ySF;  vt1[2] = v3;
	         vt2[0] = xp;     vt2[1] = yp+ySF;  vt2[2] = v4;
	         CalcNormal(vt0,vt1,vt2,n);
			   glNormal3f(n[0],n[1],n[2]);

            SetRGBColor(c1,colorMap,colorMapLength);
		   	glVertex3f(xp,yp,v1);
            SetRGBColor(c3,colorMap,colorMapLength);
			   glVertex3f(xp+xSF,yp+ySF,v3);
            SetRGBColor(c4,colorMap,colorMapLength);
			   glVertex3f(xp,yp+ySF,v4);
	      }
	   }
		glEnd();
	}
	else // Smooth shading
	{  
	
	// Calculate normal for each upper and lower triangle and store in an array	
	   normU = MakeMatrix3D(3,dataWidth,dataHeight);
	   normL = MakeMatrix3D(3,dataWidth,dataHeight);
	   
	   for(yd = dataTop; yd < dataHeight+dataTop-1; yd++)
	   {
         if(yDir)
         {
            ym1 = dataHeight+2*dataTop-1-yd; // Matrix index
            ym2 = ym1-1;
         }
         else
         {
            ym1 = yd;
            ym2 = yd+1;
         }

	   	for(xd = dataLeft; xd < dataWidth+dataLeft-1; xd++)
	      { 
            if(xDir)
            {
               xm1 = dataWidth+2*dataLeft-1-xd; // Matrix index
               xm2 = xm1-1;
            }
            else
            {
               xm1 = xd;
               xm2 = xd+1;
            }

        // Calculate vertex amplitudes
            v1 = datSF*(mat[ym1][xm1]);
            v2 = datSF*(mat[ym1][xm2]);
            v3 = datSF*(mat[ym2][xm2]);
            v4 = datSF*(mat[ym2][xm1]);

        // Plot coordinates
            xp = (xd+0.5)*xSF;
            yp = (yd+0.5)*ySF;

         // Lower triangle calcuation
	         vt0[0] = xp;     vt0[1] = yp;      vt0[2] = v1;
	         vt1[0] = xp+xSF; vt1[1] = yp;      vt1[2] = v2;
	         vt2[0] = xp+xSF; vt2[1] = yp+ySF;  vt2[2] = v3;
	         CalcNormal(vt0,vt1,vt2,n);
	         normL[yd-dataTop][xd-dataLeft][0] = n[0];
	         normL[yd-dataTop][xd-dataLeft][1] = n[1];
	         normL[yd-dataTop][xd-dataLeft][2] = n[2];

         // Upper triangle calcuation
	         vt0[0] = xp;     vt0[1] = yp;        vt0[2] = v1;
	         vt1[0] = xp+xSF; vt1[1] = yp+ySF;    vt1[2] = v3;
	         vt2[0] = xp;     vt2[1] = yp+ySF;    vt2[2] = v4;
	         CalcNormal(vt0,vt1,vt2,n);
	         normU[yd-dataTop][xd-dataLeft][0] = n[0];
	         normU[yd-dataTop][xd-dataLeft][1] = n[1];
	         normU[yd-dataTop][xd-dataLeft][2] = n[2];
	      }
	   }
	
	// For each cell take average of 6 surrounding triangle (the edges are missed)
	   norma = MakeMatrix3D(3,dataWidth,dataHeight);
	
	   for(yd = 0; yd < dataHeight-1; yd++)
	   {
	   	for(xd = 0; xd < dataWidth-1; xd++)
	      {
	         norma[yd+1][xd+1][0] = (normU[yd][xd][0] + normL[yd][xd][0] + normU[yd][xd+1][0] + normL[yd+1][xd+1][0] + normU[yd+1][xd+1][0] + normL[yd+1][xd][0])/6.0;
	         norma[yd+1][xd+1][1] = (normU[yd][xd][1] + normL[yd][xd][1] + normU[yd][xd+1][1] + normL[yd+1][xd+1][1] + normU[yd+1][xd+1][1] + normL[yd+1][xd][1])/6.0;
	         norma[yd+1][xd+1][2] = (normU[yd][xd][2] + normL[yd][xd][2] + normU[yd][xd+1][2] + normL[yd+1][xd+1][2] + normU[yd+1][xd+1][2] + normL[yd+1][xd][2])/6.0;
	      }
	   }  

      yd = 0;
   	for(xd = 0; xd < dataWidth; xd++)
	   {
	      norma[yd][xd][0] = (normL[yd][xd][0]+normU[yd][xd][0])/2;
	      norma[yd][xd][1] = (normL[yd][xd][1]+normU[yd][xd][1])/2;
	      norma[yd][xd][2] = (normL[yd][xd][2]+normU[yd][xd][2])/2;
	   }

      xd = 0;
   	for(yd = 0; yd < dataHeight; yd++)
	   {
	      norma[yd][xd][0] = (normL[yd][xd][0]+normU[yd][xd][0])/2;
	      norma[yd][xd][1] = (normL[yd][xd][1]+normU[yd][xd][1])/2;
	      norma[yd][xd][2] = (normL[yd][xd][2]+normU[yd][xd][2])/2;
	   }
	   
	// Draw data set (exclude the edges)	
	   glBegin(GL_TRIANGLES);
	   
	   for(yd = dataTop; yd < dataHeight+dataTop-1; yd++)
	   {
         if(yDir)
         {
            ym1 = dataHeight+2*dataTop-1-yd; // Matrix index
            ym2 = ym1-1;
         }
         else
         {
            ym1 = yd;
            ym2 = yd+1;
         }

	   	for(xd = dataLeft; xd < dataWidth+dataLeft-1; xd++)
	      {
            if(xDir)
            {
               xm1 = dataWidth+2*dataLeft-1-xd; // Matrix index
               xm2 = xm1-1;
            }
            else
            {
               xm1 = xd;
               xm2 = xd+1;
            }

        // Calculate vertex amplitudes
            v1 = datSF*(mat[ym1][xm1]);
            v2 = datSF*(mat[ym1][xm2]);
            v3 = datSF*(mat[ym2][xm2]);
            v4 = datSF*(mat[ym2][xm1]);

        // Calculate vertex colors
            c1 = GetSurfColor(mat[ym1][xm1],minCsVal,maxCsVal,cs_mode);
            c2 = GetSurfColor(mat[ym1][xm2],minCsVal,maxCsVal,cs_mode);
            c3 = GetSurfColor(mat[ym2][xm2],minCsVal,maxCsVal,cs_mode);
            c4 = GetSurfColor(mat[ym2][xm1],minCsVal,maxCsVal,cs_mode);

       // Plot coordinates
            xp = (xd+0.5)*xSF;
            yp = (yd+0.5)*ySF;

      // Lower triangle
	         vt0[0] = xp;     vt0[1] = yp;      vt0[2] = v1;
	         vt1[0] = xp+xSF; vt1[1] = yp;      vt1[2] = v2;
	         vt2[0] = xp+xSF; vt2[1] = yp+ySF;  vt2[2] = v3;

	         n[0] = norma[yd-dataTop][xd-dataLeft][0];
	         n[1] = norma[yd-dataTop][xd-dataLeft][1];
	         n[2] = norma[yd-dataTop][xd-dataLeft][2];
			   glNormal3f(n[0],n[1],n[2]);
            SetRGBColor(c1,colorMap,colorMapLength);
		   	glVertex3f(xp,yp,v1);

	         n[0] = norma[yd-dataTop][xd-dataLeft+1][0];
	         n[1] = norma[yd-dataTop][xd-dataLeft+1][1];
	         n[2] = norma[yd-dataTop][xd-dataLeft+1][2];
			   glNormal3f(n[0],n[1],n[2]);
            SetRGBColor(c2,colorMap,colorMapLength);
			   glVertex3f(xp+xSF,yp,v2);

	         n[0] = norma[yd-dataTop+1][xd-dataLeft+1][0];
	         n[1] = norma[yd-dataTop+1][xd-dataLeft+1][1];
	         n[2] = norma[yd-dataTop+1][xd-dataLeft+1][2];
			   glNormal3f(n[0],n[1],n[2]);
            SetRGBColor(c3,colorMap,colorMapLength);
			   glVertex3f(xp+xSF,yp+ySF,v3);

       // Upper triangle
	         vt0[0] = xp;   vt0[1] = yp;    vt0[2] = v1;
	         vt1[0] = xp+xSF; vt1[1] = yp+ySF;  vt1[2] = v3;
	         vt2[0] = xp;   vt2[1] = yp+ySF;  vt2[2] = v4;

	         n[0] = norma[yd-dataTop][xd-dataLeft][0];
	         n[1] = norma[yd-dataTop][xd-dataLeft][1];
	         n[2] = norma[yd-dataTop][xd-dataLeft][2];
			   glNormal3f(n[0],n[1],n[2]);
            SetRGBColor(c1,colorMap,colorMapLength);
		   	glVertex3f(xp,yp,v1);

	         n[0] = norma[yd-dataTop+1][xd-dataLeft+1][0];
	         n[1] = norma[yd-dataTop+1][xd-dataLeft+1][1];
	         n[2] = norma[yd-dataTop+1][xd-dataLeft+1][2];
			   glNormal3f(n[0],n[1],n[2]);
            SetRGBColor(c3,colorMap,colorMapLength);
			   glVertex3f(xp+xSF,yp+ySF,v3);

	         n[0] = norma[yd-dataTop+1][xd-dataLeft][0];
	         n[1] = norma[yd-dataTop+1][xd-dataLeft][1];
	         n[2] = norma[yd-dataTop+1][xd-dataLeft][2];
			   glNormal3f(n[0],n[1],n[2]);
            SetRGBColor(c4,colorMap,colorMapLength);
			   glVertex3f(xp,yp+ySF,v4);
	      }
	   }    
		glEnd();

		
	   FreeMatrix3D(normU);	
	   FreeMatrix3D(normL);	
	   FreeMatrix3D(norma);	
	 }  
}

void WaterfallPlot(float **mat, long dataLeft, long dataWidth, long dataTop, long dataHeight, float vscale, 
                   float *color, float maxDatVal, float minDatVal, short xDir, short yDir, bool makeSquare, float lineWidth)
{
   long x,y;
   long xm,ym;
   float xf,yf;
   float v1,v2,v3,v4;
   float c1,c2,c3,c4;
   float vt0[3],vt1[3],vt2[3];
   float n[3];
   float ***normL;
   float ***normU;
   float ***norma;
   float datSF;
   short cs_mode;

	//GLfloat	 lightPos[] = { -50.f, 50.0f, 100.0f, 1.0f };
   GLfloat   specref2[]  =  { 1.0f, 1.0f, 1.0f, 1.0f};
   GLfloat   specref1[]  =  { 0.0f, 0.0f, 0.0f, 1.0f};
   GLfloat   diff2[]  =  { 1.0f, 1.0f, 1.0f, 1.0f};
   GLfloat   diff1[]  =  { 0.2f, 0.2f, 0.2f, 1.0f};
   if(!mat) return;

// Work out scale factor
   datSF = 1.0/(maxDatVal-minDatVal);
   //if(nint(colorMap[colorMapLength-1][0]) == PLUS_MINUS_CMAP)
   //   cs_mode = PLUS_MINUS_CMAP;
   //else
   //   cs_mode = NORMAL_CMAP;
  
// Work out x-y geometry
   float xSF = 1.0;
   float ySF = 1.0;

   if(makeSquare)
   {
      if(dataWidth > dataHeight)
         ySF = dataWidth/(float)dataHeight;
      else
         xSF = dataHeight/(float)dataWidth;
   }

   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,specref2);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specref1);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,diff1);
 //  glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,50);

   for(y = dataTop; y < dataHeight+dataTop; y++)
   {
      if(yDir)
         ym = dataHeight+dataTop-1-y;
      else
         ym = y;

      glBegin(GL_QUADS);
      glColor4fv(bkColor);
      for(x = dataLeft; x < dataWidth+dataLeft-1; x++)
      {
         if(xDir)
            xm = dataWidth+2*dataLeft-2-x;
         else
            xm = x;

         v1 = datSF*(mat[ym][xm]);
         v2 = datSF*(mat[ym][xm+1]);

         xf = (x+0.5)*xSF;
         yf = (y+0.5)*ySF;

         if(xDir)
         {
            glVertex3f(xf+xSF,yf,v1);
            glVertex3f(xf,yf,v2);
            glVertex3f(xf,yf,-datSF*maxDatVal*100);
            glVertex3f(xf+xSF,yf,-datSF*maxDatVal*100);
         }
         else
         {
            glVertex3f(xf,yf,v1);
            glVertex3f(xf+xSF,yf,v2);
            glVertex3f(xf+xSF,yf,-datSF*maxDatVal*100);
            glVertex3f(xf,yf,-datSF*maxDatVal*100);
         }

      }
      glEnd();
   }

/* Draw underlying geometry */
   glLineWidth(lineWidth);
   if(gAntiAlias)
   {
      glEnable(GL_LINE_SMOOTH);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
   }
   glColor3f(color[0],color[1],color[2]);

   for(y = dataTop; y < dataHeight+dataTop; y++)
   {
      if(yDir)
         ym = dataHeight+dataTop-1-y;
      else
         ym = y;

      glBegin(GL_LINES);


   	for(x = dataLeft; x < dataWidth+dataLeft-1; x++)
      {
         if(xDir)
            xm = dataWidth+2*dataLeft-2-x;
         else
            xm = x;

     // Calculate vertex amplitudes
         v1 = datSF*(mat[ym][xm]);
         v2 = datSF*(mat[ym][xm+1]);

         xf = (x+0.5)*xSF;
         yf = (y+0.5)*ySF; // Was 0.4

         if(xDir)
         {
	         glVertex3f(xf+xSF,yf,v1);
            glVertex3f(xf,yf,v2);
         }
         else
         {
	         glVertex3f(xf,yf,v1);
            glVertex3f(xf+xSF,yf,v2);
         }
      }
      glEnd();
   }
   glLineWidth(1);
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specref2);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,diff2);

//   glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,128);
   glDisable(GL_BLEND);
}


/*******************************************************************************************
    Given a data value, min and max colorscale levels and colorscale mode return a number
    from 0 to 1 representing the position in the color-scale.
*******************************************************************************************/

float GetSurfColor(float data, float minVal, float maxVal, short cs_mode)
{
   float colorPos;

   if(cs_mode == PLUS_MINUS_CMAP) // +/- colormap
   {
      if(fabs(data) < fabs(minVal))
      {
	      colorPos = 0.5;
      }
      else
      {
	      if(data >= 0)
	         colorPos = 0.5+0.5*(data-minVal)/(maxVal-minVal);
	      else
	         colorPos = 0.5+0.5*(data+minVal)/(maxVal-minVal);
      }
   }
   else // Normal colormap 
   {
      if(data < minVal)
	   {
	      colorPos = 0;
	   }	
	   else
	   {
	      colorPos = (data-minVal)/(maxVal-minVal);
      }
   }
   return(colorPos);
}

/*******************************************************************************************
    Use the rgb colors in the array colorMap to set the current color.
    value is a number from 0 to 1 representing the data point
    0 maps to the first index in the color map and
    1 maps to the last index in the color map
*******************************************************************************************/

void SetRGBColor(float value, float **colorMap, long length)
{
   long index;
   index = (long)(value*(length-1));
   if(index > length-2) index = length-2;
   if(index < 0) index = 0;
   float r = colorMap[index][0];
   float g = colorMap[index][1];
   float b = colorMap[index][2];
   glColor3f(r,g,b);
}

void Get2DRange(float **mat, long dataLeft, long dataWidth, long dataTop, long dataHeight, float &minVal, float &maxVal)
{
   long i,j;
   
   maxVal = -1e30;
   minVal = +1e30;

 // Find the maximum and minimum values in the currently visible data set
    
   for(i = dataLeft; i < dataLeft + dataWidth; i++)
   {
      for(j = dataTop; j < dataTop + dataHeight; j++)
      {
         if(mat[j][i] > maxVal) maxVal = mat[j][i];
         if(mat[j][i] < minVal) minVal = mat[j][i];   
      }
   }
}

void Get2DRange(complex **cmat, long dataLeft, long dataWidth, long dataTop, long dataHeight, float &minVal, float &maxVal)
{
   long i,j;
   
   maxVal = -1e30;
   minVal = +1e30;

 // Find the maximum and minimum values in the currently visible data set
    
   for(i = dataLeft; i < dataLeft + dataWidth; i++)
   {
      for(j = dataTop; j < dataTop + dataHeight; j++)
      {
         if(cmat[j][i].r > maxVal) maxVal = cmat[j][i].r;
         if(cmat[j][i].r < minVal) minVal = cmat[j][i].r;          
      }
   }
}

// Clear the 3D display

int Clear3D(Interface* itfc, char args[])
{
   if(!plot3d)
   {
      ErrorMessage("3D plot undefined");
      return(ERR);
   }

   CText txt;
   if(plot3d->count3D >= plot3d->glBaseList)
      glDeleteLists(plot3d->glBaseList,plot3d->count3D);
   plot3d->count3D = plot3d->glBaseList;

// Reset everything
   clipPlane1 = false;
   clipPlane2 = false;
   plot3d->draw3DPlane = false;
   show_rotation_axis = false;
   gEnableClipping = false;
   Initialize3DParameters();
   glLineWidth(1);
   ShowColorScale3D(itfc,"\"false\"");
   txt.Format("elevation = %2.1f azimuth = %2.1f twist = %2.1f distance = %2.1f",plot3d->elevation,plot3d->azimuth,plot3d->twist,plot3d->viewDistance);
   UpdateStatusWindow(cur3DWin,0,txt.Str());
   Invalidate3DPlot(cur3DWin);
	itfc->nrRetValues = 0;
   return(OK);
}


	
// Setup all the lighting used by openGL.
// Also define the surface materials

void SetUpLightModel()
{
// Light values and coordinates

	GLfloat   ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f};
	GLfloat   diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f};
	GLfloat   specularT[] =  { 1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat   specularF[] =  { 0.0f, 0.0f, 0.0f, 0.0f};
	GLfloat   specref[]  =  { 1.0f, 1.0f, 1.0f, 1.0f};
	//GLfloat	 lightPos[] =  { -50.f, 50.0f, 100.0f, 1.0f};

// Hidden surface removal
	glEnable(GL_DEPTH_TEST);	
	glFrontFace(GL_CCW);

// Two sided rendering
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambientLight);

// Specify lights used	
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
	if(specular)
	   glLightfv(GL_LIGHT0,GL_SPECULAR,specularT);
	else
	   glLightfv(GL_LIGHT0,GL_SPECULAR,specularF);

// Turn on the lights
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

// Specify material types
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,specref);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specref);
   glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,128);
	
   glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
}


void Set3DMapping(short x, short y, short w, short h)
{
   float f;
   
// Prevent a divide by zero
	if(h == 0)
		h = 1;

// Set Viewport to window dimensions
    glViewport(x, y, w, h);

// Reset coordinate system
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//   gluPerspective(VIEW_ANGLE,(float)w/h, 1, 5000);
   f = tan(VIEW_ANGLE/2.0*PI/180);
   glFrustum(-f,f,-f*h/(float)w,f*h/(float)w,1,5000.0);
   
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void Shift3DZ(short shift)
{
   char str[MAX_STR];

   plot3d->viewDistance += shift;
   
   if(plot3d->viewDistance < 0) plot3d->viewDistance -=shift;
   sprintf(str,"elevation = %2.1f azimuth = %2.1f twist = %2.1f distance = %2.1f",plot3d->elevation,plot3d->azimuth,plot3d->twist,plot3d->viewDistance);
   UpdateStatusWindow(cur3DWin,0,str);
	Invalidate3DPlot(cur3DWin);
}

  
void Rotate3D(short direction, float step)
{
   char str[MAX_STR];
   
   switch(direction)
   {
      case(ID_ROTATE_CW_ELEVATION):
	   {
	      plot3d->elevation += step;
         if(plot3d->elevation > 90)
            plot3d->elevation = 90;
	      break;
	   }

      case(ID_ROTATE_CCW_ELEVATION):
	   {
	      plot3d->elevation -= step;
         if(plot3d->elevation < -90)
            plot3d->elevation = -90;
	      break;
	   }

      case(ID_ROTATE_CCW_AZIMUTH):
	   {
         if(plot3d->azimuth >= 0 && plot3d->azimuth-step < 0)
            plot3d->azimuth += 360;
	      plot3d->azimuth -= step;
	      break;
	   }

      case(ID_ROTATE_CW_AZIMUTH):
	   {
         if(plot3d->azimuth <= 360 && plot3d->azimuth+step > 360)
            plot3d->azimuth -= 360;
	      plot3d->azimuth += step;
	      break;
	   }
	   
      case(ID_ROTATE_CW_TWIST):
	   {
         if(plot3d->twist <= 360 && plot3d->twist+step > 360)
            plot3d->twist -= 360;
	      plot3d->twist += step;
	      break;
	   }

      case(ID_ROTATE_CCW_TWIST):
	   {
         if(plot3d->twist >= 0 && plot3d->twist-step < 0)
            plot3d->twist += 360;
	      plot3d->twist -= step;
	      break;
	   }	   	   
	}
   sprintf(str,"elevation = %2.1f azimuth = %2.1f twist = %2.1f distance = %2.1f",plot3d->elevation,plot3d->azimuth,plot3d->twist,plot3d->viewDistance);
   UpdateStatusWindow(cur3DWin,0,str);
	Invalidate3DPlot(cur3DWin);
}


void Shift3DXYZ(short direction, float shift)
{
   char str[50];

   switch(direction)
   {
      case(ID_SHIFT_3D_LEFT):
	   {
         plot3d->xoff -= shift;
	      break;
	   }

      case(ID_SHIFT_3D_RIGHT):
	   {
         plot3d->xoff += shift;
	      break;
	   }

      case(ID_SHIFT_3D_UP):
	   {
         plot3d->yoff += shift;
	      break;
	   }

      case(ID_SHIFT_3D_DOWN):
	   {
         plot3d->yoff -= shift;         
	      break;
	   }

      case(ID_SHIFT_3D_IN):
	   {
         plot3d->zoff += shift;
	      break;
	   }

      case(ID_SHIFT_3D_OUT):
	   {
         plot3d->zoff -= shift;         
	      break;
	   }
	}
	sprintf(str,"xoff = %2.0f yoff = %2.0f zoff = %2.0f",plot3d->xoff,plot3d->yoff,plot3d->zoff+plot3d->viewDistance);
   UpdateStatusWindow(cur3DWin,0,str);
	
	Invalidate3DPlot(cur3DWin);
}

void ShiftZCutPlane(short direction)
{
   char str[50];

   switch(direction)
   {
      case(ID_SHIFT_Z_CUTPLANE_UP):
	   {
         plot3d->cutPlaneZ -= 2;
	      break;
	   }

      case(ID_SHIFT_Z_CUTPLANE_DOWN):
	   {
         plot3d->cutPlaneZ += 2;
	      break;
	   }

	}
	sprintf(str,"cutplane = %2.0f",plot3d->cutPlaneZ);
   UpdateStatusWindow(cur3DWin,0,str);
	
	Invalidate3DPlot(cur3DWin);
}


void Scale3DXYZ(short direction, float factor)
{
   char str[50];

   switch(direction)
   {
      case(ID_SCALE_X_UP):
	   {
         plot3d->xoff /= plot3d->xScale;
         plot3d->xScale += factor;
         plot3d->xoff *= plot3d->xScale;
	      break;
	   }

      case(ID_SCALE_X_DOWN):
	   {
         plot3d->xoff /= plot3d->xScale;
         plot3d->xScale -= factor;
         if(plot3d->xScale < factor) plot3d->xScale = factor;
         plot3d->xoff *= plot3d->xScale;
	      break;
	   }

      case(ID_SCALE_Y_UP):
	   {
         plot3d->yoff /= plot3d->yScale;
         plot3d->yScale += factor;
         plot3d->yoff *= plot3d->yScale;
	      break;
	   }

      case(ID_SCALE_Y_DOWN):
	   {
         plot3d->yoff /= plot3d->yScale;
         plot3d->yScale -= factor;
         if(plot3d->yScale < factor) plot3d->yScale = factor; 
         plot3d->yoff *= plot3d->yScale;
	      break;
	   }
	   	      
      case(ID_SCALE_Z_UP):
	   {
         plot3d->zoff /= plot3d->zScale;
         plot3d->zScale += factor;
         plot3d->zoff *= plot3d->zScale;
	      break;
	   }

      case(ID_SCALE_Z_DOWN):
	   {
         plot3d->zoff /= plot3d->zScale;
         plot3d->zScale -= factor;
         if(plot3d->zScale < factor) plot3d->zScale = factor; 
         plot3d->zoff *= plot3d->zScale;
	      break;
	   }
	}
	sprintf(str,"xscale = %2.1f yscale = %2.1f zscale = %2.1f",plot3d->xScale,plot3d->yScale,plot3d->zScale);
   UpdateStatusWindow(cur3DWin,0,str);
	
	Invalidate3DPlot(cur3DWin);
}

//bool isToolBar2D = true;
//#define NUM3DBUTTONS 1
//TBBUTTON tb3DButtons[NUM3DBUTTONS];
//
//void Make3DToolBar()
//{
//   if(isToolBar2D == true)
//   {
////      DestroyWindow(tbWnd2D);
//      
//	   for(short i = 0; i < NUM3DBUTTONS; i++)
//	   {
//	      tb3DButtons[i].iBitmap = i;
//	      tb3DButtons[i].fsState = TBSTATE_ENABLED;
//	      tb3DButtons[i].fsStyle = TBSTYLE_BUTTON;
//	      tb3DButtons[i].dwData  = 0L;
//	      tb3DButtons[i].iString = 0;
//	   }
//
//        //xxx 
//	 /*  tbWnd2D = CreateToolbarEx(plot2DWin,
//	                        WS_VISIBLE | WS_CHILD | WS_BORDER | TBSTYLE_TOOLTIPS,
//	                        ID_2D_TOOLBAR,
//	                        1,
//	                        prospaInstance,
//	                        IDTB_3D_BMP,
//	                        tb3DButtons,
//	                        1,
//	                        0,0,16,16,
//	                        sizeof(TBBUTTON));   */   
//      isToolBar2D = false;
//   }
//
//}
/*
short
PrintOpenGL()
{

	HDC            hDC;
	DOCINFO        di;
	HGLRC          hglrc;
   static PRINTDLG prd;
   
// Initialize the PRINTDLG members 
	 
	prd.lStructSize = sizeof(PRINTDLG); 
	prd.Flags = PD_RETURNDC | PD_ENABLEPRINTHOOK; 
	prd.hwndOwner = openGlWin; 
	prd.nCopies = 1; 
   prd.lpfnPrintHook = PrintHookProc; 

// Display the PRINT dialog box. 
	 
	if(!PrintDlg(&prd))
	   return(OK); 

// Print window
	
   HDC hdcPrint = prd.hDC;
   static DOCINFO di = { sizeof (DOCINFO), TEXT ("") } ;

// Call StartDoc to start the document
	StartDoc( hdcPrint, &di );
	
// Prepare the printer driver to accept data
	StartPage(hdcPrint);
	
// Create a rendering context using the metafile DC
	hglrc = wglCreateContext (hDCmetafile );
	
	// Do OpenGL rendering operations here
	SurfacePlot3D(hDCmetafile);
	 
	wglMakeCurrent(NULL, NULL); // Get rid of rendering context
	    . . .
	EndPage(hdcPrint); // Finish writing to the page
	EndDoc(hdcPrint); // End the print job
}

*/


//int Draw3DVectors(char args[])
//{
//   short na;
//   Variable Vx,Vy,Vz;
//   long width,height,depth;
//   float vLength;
//   long x,y,z;
//   float ***mx,***my,***mz;
//   float l;
//   float minVal,maxVal;
//   float vx,vy,vz;
//   float minv,maxv;
//   
//   if((na = ArgScan(itfc,args,6,"x_matrix, y_matrix, z_matrix, length, minv, maxv","eeeeee","vvvfff",&Vx,&Vy,&Vz,&vLength,&minv,&maxv)) < 0)
//    return(na);
//
//   if(VarType(&Vx) != MATRIX3D || VarType(&Vy) != MATRIX3D || VarType(&Vz) != MATRIX3D)
//   {
//      ErrorMessage("data must be real 3D matrices");
//      return(ERR);
//   }
//
//
//// Get matrix infor
//   
//   mx = VarReal3DMatrix(&Vx);
//   my = VarReal3DMatrix(&Vy);
//   mz = VarReal3DMatrix(&Vz);
//   
//   width  = VarWidth(&Vx);
//   height = VarHeight(&Vy);
//   depth  = VarDepth(&Vz);
//
//// Find range
//   
//   for(z = 0; z < depth; z++)
//   {
//	   for(y = 0; y < height; y++)
//	   {
//	      for(x = 0; x < width; x++)
//	      {
//	         l = sqrt(mx[z][y][x]*mx[z][y][x] + my[z][y][x]*my[z][y][x] + mz[z][y][x]*mz[z][y][x]);
//	         if(l >= minv && l <= maxv && l > maxVal) maxVal = l;
//	         if(l >= minv && l <= maxv && l < minVal) minVal = l;
//	      }
//	   }
//	}
//
//         
//// Plot data to 3D window
//
//   glNewList(++plot3d->count3D, GL_COMPILE);
//
//	glMatrixMode(GL_MODELVIEW);
//	glPushMatrix(); 
//	  
//   glEnable(GL_NORMALIZE);
//
// 	glTranslatef(-width/2.0,-height/2.0,-depth/2.0); 
// 
//   DrawWireFrameBox(0,width,0,height,0,depth);	
//  
//   float sf = vLength/maxVal;
//   glColor3f(1.0,1.0,0.0);
//   float arrowLen = 0.5;
//   float xt,yt,zt;
//   float ct = cos(30*PI/180);
//   float st = sin(30*PI/180);  
//   
//   glBegin(GL_LINES);
//   
//   for(z = 0; z < depth; z++)
//   {
//	   for(y = 0; y < height; y++)
//	   {
//	      for(x = 0; x < width; x++)
//	      {
//	         vx = mx[z][y][x];
//	         vy = my[z][y][x];
//	         vz = mz[z][y][x];
//	         l = sqrt(vx*vx+vy*vy+vz*vz);
//	         if(l >= minv && l <= maxv)
//	         {
//               glVertex3f(x,y,z); // Base of arrow
//               xt = x+vx*sf;
//               yt = y+vy*sf;
//               zt = z+vz*sf;
//               glVertex3f(xt,yt,zt); // Tip of arrow
// 
//				  // Arrow head position at (x1,y1) = (xv - cos(theta-phi)*length, yv - sin(theta-phi)*length)
//				  // Arrow head position at (x2,y2) = (xv - cos(theta+phi)*length, yv - sin(theta+phi)*length)
//				     
//	//			   x1 = (ct*vx + st*vy)*arrowLen/l; // Draw arrow head.
//	//			   y1 = (st*vx - ct*vy)*arrowLen/l;
//	//			   x2 = (ct*vx - st*vy)*arrowLen/l;
//	//			   y2 = (st*vx + ct*vy)*arrowLen/l;
//
//   //            glVertex3f(x+vx*sf,y+vy*sf,z+vz*sf);
//   //            glVertex3f(x+vx*sf-x1,y+vy*sf-y1,z+vz*sf);
//
//   //            glVertex3f(x+vx*sf,y+vy*sf,z+vz*sf);
//    //           glVertex3f(x+vx*sf-x2,y+vy*sf+y2,z+vz*sf);
//            }
//	      }
//	   }
//	}
//   glEnd();
//	
//   glPopMatrix();
//
//   glEndList();
//   Invalidate3DPlot(cur3DWin);
//		         
//   return(OK);
//}



int Draw3DLines(Interface *itfc, char args[])
{
   short na;
   Variable xVar,yVar,zVar;
   long nrSeg;
   long n;

   if((na = ArgScan(itfc,args,3,"x, y, z","eee","vvv",&xVar,&yVar,&zVar)) < 0)
      return(na);

// Check argument type
   if(VarType(&xVar) != MATRIX2D || VarHeight(&xVar) != 1 || VarWidth(&xVar) <= 0)
   {
      ErrorMessage("x data must be real row vector");
      return(ERR);
   }
   if(VarType(&yVar) != MATRIX2D || VarHeight(&yVar) != 1 || VarWidth(&yVar) <= 0)
   {
      ErrorMessage("y data must be real row vector");
      return(ERR);
   }
   if(VarType(&zVar) != MATRIX2D || VarHeight(&zVar) != 1 || VarWidth(&zVar) <= 0)
   {
      ErrorMessage("z data must be real row vector");
      return(ERR);
   }

// Check vector lengths
   if(VarWidth(&xVar) != VarWidth(&yVar) ||
      VarWidth(&xVar) != VarWidth(&zVar) ||
      VarWidth(&yVar) != VarWidth(&zVar))
   {
      ErrorMessage("x, y and z vector must be the same length");
      return(ERR);
   }

// Get matrix information
   float* x = VarRealMatrix(&xVar)[0];
   float* y = VarRealMatrix(&yVar)[0];
   float* z = VarRealMatrix(&zVar)[0];
   nrSeg = VarWidth(&xVar);

// Plot data to 3D window
   glNewList(++plot3d->count3D, GL_COMPILE);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix(); 
   glDisable(GL_LIGHTING);
   glBegin(GL_LINES);

   float x1,y1,z1,x2,y2,z2;
   for(n = 0; n < nrSeg-1; n++)
   {
      x1 = x[n];
      x2 = x[n+1];
      y1 = y[n];
      y2 = y[n+1];
      z1 = z[n];
      z2 = z[n+1];

      glVertex3f(x1,y1,z1); // Start of line
      glVertex3f(x2,y2,z2); // End of line
   }

   glEnd();
   glEnable(GL_LIGHTING);
   glPopMatrix();
   glEndList();

   Invalidate3DPlot(cur3DWin);

	itfc->nrRetValues = 0;
   return(OK);
}

#define XSTART 0
#define YSTART 2
#define ZSTART 4
#define XEND   1
#define YEND   3
#define ZEND   5

int Draw3DSegments(Interface *itfc, char args[])
{
   short na;
   Variable matLines, xVar,yVar,zVar;
   float **lines;
   long nrSeg;
   long n;

   if((na = ArgScan(itfc,args,1,"segment matrix","e","v",&matLines)) < 0)
      return(na);

// Check argument type
   if(VarType(&matLines) != MATRIX2D || VarWidth(&matLines) != 6 || VarHeight(&matLines) <= 0)
   {
      ErrorMessage("line data must be real 6 by n matrix (x,y)");
      return(ERR);
   }

// Get matrix information
   lines = VarRealMatrix(&matLines);
   nrSeg = VarHeight(&matLines);

// Plot data to 3D window
   glNewList(++plot3d->count3D, GL_COMPILE);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix(); 
   glDisable(GL_LIGHTING);
   glBegin(GL_LINES);

   float x1,y1,z1,x2,y2,z2;
   for(n = 0; n < nrSeg; n++)
   {
      x1 = lines[n][XSTART];
      x2 = lines[n][XEND];
      y1 = lines[n][YSTART];
      y2 = lines[n][YEND];
      z1 = lines[n][ZSTART];
      z2 = lines[n][ZEND];

      glVertex3f(x1,y1,z1); // Start of line
      glVertex3f(x2,y2,z2); // End of line
   }
   glEnd();
   glPopMatrix();
   glEndList();

   Invalidate3DPlot(cur3DWin);

	itfc->nrRetValues = 0;
   return(OK);
}


int SetViewDistance(Interface *itfc, char args[])
{
   float dist;
   short nrArgs;

   if(!plot3d)
   {
      ErrorMessage("no 3d plot defined");
      return(ERR);
   }

   dist = plot3d->viewDistance;

// Get arguments from user ************* 
   if((nrArgs = ArgScan(itfc,args,1,"z distance","e","f",&dist)) < 0)
     return(nrArgs); 

// Must be positive ********************
   if(dist <= 0)
   {
      ErrorMessage("z distance must be > 0");
      return(ERR);
   }

// Update global ***********************
   plot3d->viewDistance = dist;

// Print result
   CText txt;
   txt.Format("elevation = %2.1f azimuth = %2.1f twist = %2.1f distance = %2.1f",plot3d->elevation,plot3d->azimuth,plot3d->twist,plot3d->viewDistance);
   UpdateStatusWindow(cur3DWin,0,txt.Str());

// Redraw plot *************************
   Invalidate3DPlot(cur3DWin);

	itfc->nrRetValues = 0;  
   return(OK); 
}

// Show or hide color scale

int ShowColorScale3D(Interface* itfc ,char args[])
{
   short nrArgs;
   CText txt;

   if(plot3d->displayColorScale)
      txt = "true";
   else
      txt = "false";

   if((nrArgs = ArgScan(itfc,args,1,"true/false","e","t",&txt)) < 0)
     return(nrArgs);

	//hMenu = GetMenu(cur2DPlot->win);
	
	if(txt == "true" || txt == "yes" || txt == "on")
	{
	//	CheckMenuItem(hMenu,ID_COLOR_BAR,MF_CHECKED);
		plot3d->displayColorScale = true;
	}
	if(txt == "false" || txt == "no" || txt == "off")
	{
	//	CheckMenuItem(hMenu,ID_COLOR_BAR,MF_UNCHECKED);
		plot3d->displayColorScale = false;
	}
	
// Redraw plot *************************
   Invalidate3DPlot(cur3DWin);

   return(OK);
   
}

// Set current clipping status

int Set3DClippingStatus(Interface *itfc, char arg[])
{
   short nrArgs;
   CText txt;

   if((nrArgs = ArgScan(itfc,arg,1,"on/off","e","t",&txt)) < 0)
     return(nrArgs);

	if(txt == "true" || txt == "yes" || txt == "on")
	{
      gEnableClipping = true;
	}
	if(txt == "false" || txt == "no" || txt == "off")
	{
      gEnableClipping = false;
	}
	
// Redraw plot *************************
   Invalidate3DPlot(cur3DWin);
	itfc->nrRetValues = 0;
   return(OK);
   
}

// Toggle anti-alias mode

int AntiAlias3D(Interface* itfc ,char args[])
{
   short nrArgs;
   CText txt;

   if(gAntiAlias)
      txt = "on";
   else
      txt = "off";

   if((nrArgs = ArgScan(itfc,args,1,"true/false","e","t",&txt)) < 0)
     return(nrArgs);

	//hMenu = GetMenu(cur2DPlot->win);
	
	if(txt == "true" || txt == "yes" || txt == "on")
	{
	//	CheckMenuItem(hMenu,ID_COLOR_BAR,MF_CHECKED);
		gAntiAlias = true;
	}
	if(txt == "false" || txt == "no" || txt == "off")
	{
	//	CheckMenuItem(hMenu,ID_COLOR_BAR,MF_UNCHECKED);
		gAntiAlias = false;
	}
	

// Redraw plot *************************
   Invalidate3DPlot(cur3DWin);

   return(OK);
   
}

// Toggle depth-cueing (fog)

int DepthCue(Interface* itfc ,char args[])
{
   short nrArgs;
   CText txt;

   if(gDepthCue)
      txt = "on";
   else
      txt = "off";

   if((nrArgs = ArgScan(itfc,args,1,"true/false","e","t",&txt)) < 0)
     return(nrArgs);

	//hMenu = GetMenu(cur2DPlot->win);
	
	if(txt == "true" || txt == "yes" || txt == "on")
	{
	//	CheckMenuItem(hMenu,ID_COLOR_BAR,MF_CHECKED);
		gDepthCue = true;
	}
	if(txt == "false" || txt == "no" ||txt == "off")
	{
	//	CheckMenuItem(hMenu,ID_COLOR_BAR,MF_UNCHECKED);
		gDepthCue = false;
	}
	

// Redraw plot *************************
   Invalidate3DPlot(cur3DWin);

	itfc->nrRetValues = 0;
   return(OK);
}

void  ClippingControl()
{
   if(gEnableClipping)
   {
      glEnable (GL_CLIP_PLANE0);
      glEnable (GL_CLIP_PLANE1);
   }
   else
   {
      glDisable (GL_CLIP_PLANE1);
      glDisable (GL_CLIP_PLANE0);
   }
}


/*****************************************************************************
* Add a cone to the display list
*****************************************************************************/

int DrawCone(Interface* itfc ,char args[])
{
   Variable baseVar,topVar,colVar;
   float radius;
   short nrArgs;
   extern bool setFormat;
   float r,g,b,alpha = 1;
   GLUquadricObj *obj;

// Get filename from user *************
   
   if((nrArgs = ArgScan(itfc,args,4,"base,top,radius,colour","eeee","vvfv",&baseVar,&topVar,&radius,&colVar)) < 0)
     return(nrArgs); 

   if(VarType(&baseVar) != MATRIX2D && VarRowSize(&baseVar) != 1 && VarColSize(&baseVar) != 3)
   {
      ErrorMessage("invalid base coordinate");
      return(ERR);
   }

   if(VarType(&topVar) != MATRIX2D && VarRowSize(&topVar) != 1 && VarColSize(&topVar) != 3)
   {
      ErrorMessage("invalid top coordinate");
      return(ERR);
   }

   if(VarType(&colVar) != MATRIX2D && VarRowSize(&colVar) != 1 && (VarColSize(&colVar) != 3 || VarColSize(&colVar) != 4))
   {
      ErrorMessage("invalid RGB colour");
      return(ERR);
   }
   
   XYZ base;
   base.x = VarRealMatrix(&baseVar)[0][0];
   base.y = VarRealMatrix(&baseVar)[0][1];
   base.z = VarRealMatrix(&baseVar)[0][2];


   XYZ top; 
   top.x = VarRealMatrix(&topVar)[0][0];
   top.y = VarRealMatrix(&topVar)[0][1];
   top.z = VarRealMatrix(&topVar)[0][2]; 
   
   if(VarColSize(&colVar) >= 3)
   {
	   r = VarRealMatrix(&colVar)[0][0];
	   g = VarRealMatrix(&colVar)[0][1];
	   b = VarRealMatrix(&colVar)[0][2];
	}
   if(VarColSize(&colVar) == 4)
      alpha = VarRealMatrix(&colVar)[0][3];

   obj = gluNewQuadric();

   glNewList(++plot3d->count3D, GL_COMPILE);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
    	glEnable(GL_LIGHTING);
      ClippingControl();
	   if(alpha < 1)   
	   {
	     glEnable(GL_BLEND);
	     glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	     glColor4f(r,g,b,alpha);
	   }
	   else
	   {
		   glColor3f(r,g,b);
		}   

     XYZ axis;
     axis.x = top.x-base.x;
     axis.y = top.y-base.y;
     axis.z = top.z-base.z;

     XYZ rot;
     float angle;
     CalcRotationVector(axis.x, axis.y, axis.z, rot.x, rot.y, rot.z, angle);

     angle = angle/PI*180.0;

     float len = VectorLength(top,base);

     glTranslatef(base.x,base.y,base.z);

     glRotatef(angle,rot.x,rot.y,rot.z);

      gluCylinder(obj,radius,0,len,36,1);
      gluDisk(obj,0,radius,36,1);


	   if(alpha < 1)   
	     glDisable(GL_BLEND);
	   glPopMatrix();
   glEndList();

   Invalidate3DPlot(cur3DWin);
   
   return(OK);
}
