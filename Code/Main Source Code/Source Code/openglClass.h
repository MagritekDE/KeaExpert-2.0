#ifndef COPENGLWIN_H
#define COPENGLWIN_H

class COpenGLWin
{
   public:

      HBITMAP hBitMap;    // Plot bitmap
      HGLRC   hRC;        // OpenGL rendering context
      HDC     hdcGL;      // Drawing context for GL
      HWND    parentWin;  // Parent window
      short   mode;
      short   nrBits;
      short   zDepth;
      GLYPHMETRICSFLOAT gmf1[256]; // Metric for font1
      GLYPHMETRICSFLOAT gmf2[256]; // Metrics for font2
      int listbase1;  // Base for font1
      int listbase2;  // Base for font2
      COpenGLWin(void);
      ~COpenGLWin(void);
      void Initialize(HWND);
      bool SetupPixelFormat(void);
      bool CreateBitMap(HDC hdc , int width, int height);
      bool ResizeBitMap(int w, int h);
      void SetMode(short newMode);
      void MakeFont(char *fontname, short listbase);
      void Draw(void);
      void DrawString(char *s, short listbase);
      void CharDim(char c, float &width, float &height, short listbase);
      void StrDim(char *s, float &width, float &height, short listbase);
};


#define GLBITMAP 0
#define GLSCREEN 1

#endif // define COPENGLWIN_H