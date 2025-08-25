#ifndef PLOT3DSURFACE_H
#define PLOT3DSURFACE_H

class Interface;

extern bool update3d;
extern bool show_rotation_axis;
extern bool specular;
extern short surface2DMode;

short Initialize3DPlot(HWND);
void Invalidate3DPlot(HWND);
void GetLabel(float spacing, float label, char *txt);
void GetExponent(float label, char *txt);
void SurfacePlot3D(HWND hWnd, HDC hdc);
int Clear3D(Interface* itfc, char args[]);
void Rotate3D(short direction, float size);
void Shift3DXYZ(short dir, float size);
void Scale3DXYZ(short,float);
void Initialize3DParameters(void);
void Shift3DZ(short shift);
void SetUpLightModel();
void ShiftZCutPlane(short);
int Scale3DPlot(Interface* itfc ,char args[]);
int DrawBox(Interface* itfc ,char args[]);
void SurfacePlot2D(float **mat, long left, long width, long top, long height, float vscale, 
                   float **colorMap, long colorMapLength, float minV, float maxV, float minM, float maxM,
                   short xDir, short yDir, bool makeSquare);
void WaterfallPlot(float **mat, long left, long width, long top, long height, float vscale, 
                   float *color, float minVal, float maxVal, short xDir, short yDir, bool makeSquare, float lineWidth);
// CLI calls

int Draw3DCylinder(Interface *itfc, char arg[]);
int Draw3DAxes(Interface *itfc, char arg[]);
int Draw3DAxis(Interface *itfc, char arg[]);
int Shift3DPlot(Interface *itfc, char args[]);
int Rotate3DPlot(Interface* itfc ,char args[]);
int Set3DLight(Interface *itfc, char arg[]);
int DrawSphere(Interface* itfc ,char args[]);
int Display2DSurface(Interface* itfc ,char args[]);
int Display3dSurface(Interface* itfc ,char args[]);
int Draw3DPlot(Interface *itfc, char arg[]);
int Draw3DLines(Interface *itfc, char arg[]);
int SetFogRange(Interface *itfc, char arg[]);
int Set3DColor(Interface *itfc, char arg[]);
int Set3DLabelSizes(Interface *itfc, char args[]);
int SetViewDistance(Interface *itfc, char args[]);
int LineWidth3d(Interface* itfc ,char args[]);
int Draw3DPlane(Interface *itfc, char arg[]);
int Set3DClippingPlane(Interface *itfc, char arg[]);
int Draw3DColorScale(Interface *itfc, char arg[]);
int ShowColorScale3D(Interface* itfc ,char args[]);
int AntiAlias3D(Interface* itfc ,char args[]);
int DepthCue(Interface* itfc ,char args[]);
int Set3DBkColor(Interface *itfc, char arg[]);
int Set3DDataRange(Interface *itfc, char arg[]);
int Set3DClippingStatus(Interface *itfc, char arg[]);
int Draw3DSegments(Interface *itfc, char arg[]);
int Set3DTextSize(Interface *itfc, char arg[]);
int Waterfall(Interface* itfc ,char args[]);
#endif // define PLOT3DSURFACE_H