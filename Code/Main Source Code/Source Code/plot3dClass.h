#ifndef PLOT3DCLASS_H
#define PLOT3DCLASS_H

#include "openglClass.h"

class ObjectData;

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

#define VIEW_ANGLE 30  // Viewing angle
#define XY_PLANE 0
#define XZ_PLANE 1
#define YZ_PLANE 2

class CPlot3D
{
   public:
      CPlot3D(void);
      COpenGLWin glWin3d;
      float xmin,xmax;
      float ymin,ymax;
      float zmin,zmax;
      float xLabelSpacing,yLabelSpacing,zLabelSpacing;
      float xTickSpacing,yTickSpacing,zTickSpacing;
      float xTicksPerLabel,yTicksPerLabel,zTicksPerLabel;
      float tickLength;
      float labelLength;
      float numberFontScaling;
      float labelFontScaling;
     // 3D view angles
      float elevation;
      float azimuth;
      float twist;
      float viewDistance; 
    // Offset
      float zoff;
      float xoff;
      float yoff;
    // Scale
      float xScale;       
      float yScale;
      float zScale;
    // Colormap
      float **colorMap; 
      long  colorMapLength;
      bool  displayColorScale;
      float colorScaleMinValue;
      float colorScaleMaxValue;
      float colorScaleRed;
      float colorScaleGreen;
      float colorScaleBlue;
      float colorScaleFontSize;
    // Cutting plane
      bool draw3DPlane;
      float planeXMin;
      float planeXMax;
      float planeYMin;
      float planeYMax;
      float planePosition;
      short planeDirection;
      float planeRed;
      float planeGreen;
      float planeBlue;
      float planeAlpha;
      bool update3d;
      bool initialised;
      float cutPlaneZ;
   // Display list
      short count3D;
      short glBaseList;
     // Parent object
      ObjectData *parent;
      void DrawAxes(float x1, float x2, float y1, float y2, float z1, float z2);
      void DrawXAxis(char *label, float minLabel, float maxLabel, float minAxes, float maxAxes, float y, float z, short label_position, short axis_direction);
      void DrawYAxis(char *label, float minLabel, float maxLabel, float minAxes, float maxAxes, float z, float x, short label_position, short axis_direction);
      void DrawZAxis(char *label, float minLabel, float maxLabel, float minAxes, float maxAxes, float x, float y, short label_position, short axis_direction);
      void DrawSimpleAxes(float size);
      float GetLabelSpacing(float);
      short CalcTickAndLabelSpacing(float min, float max, float &tickSpacing, long &ticksPerLabel);
      void SetSizes(float tickSize, float labelLength, float numberSize, float labelSize);
      void DrawAxesLabelCore(char *axisLabel, bool write, float &width, float &heightUp, float &heightDown);
      short SaveAsImage(char *fileName);
      void Copy3DToBitmap(HBITMAP hBitmap, long x, long y, long w, long h);
};


extern CPlot3D *plot3d;
extern HWND cur3DWin;

#endif // define PLOT3DCLASS_H