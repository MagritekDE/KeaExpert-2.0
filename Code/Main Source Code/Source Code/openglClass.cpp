#include "stdafx.h"
#include "openglClass.h"
#include "gl/gl.h" 
#include "gl/glu.h"
#include "memoryLeak.h"

COpenGLWin::COpenGLWin()
{
   mode = GLSCREEN;
   nrBits = 24;
   zDepth = 16;
   hdcGL = NULL;
   hBitMap = NULL;
   hRC = NULL;
}

COpenGLWin::~COpenGLWin()
{
   if(hdcGL && hRC)
   {
      wglMakeCurrent(hdcGL, NULL); 
      wglDeleteContext(hRC); 
      DeleteDC(hdcGL);
      if(hBitMap)
         DeleteObject(hBitMap);
   }
}

void COpenGLWin::Initialize(HWND hWnd)
{
   RECT r;
   parentWin = hWnd;
   GetClientRect(hWnd,&r);
   HDC hdc = GetDC(hWnd);
   if(mode == GLBITMAP)
   {
      CreateBitMap(hdc, r.right,r.bottom); 
      hdcGL = CreateCompatibleDC(hdc);
	   SelectObject(hdcGL,hBitMap);
   }
   else
      hdcGL = hdc;
   SetupPixelFormat(); 
   hRC = wglCreateContext(hdcGL); 
   wglMakeCurrent(hdcGL,hRC);

   if(mode == GLSCREEN)
   {
      MakeFont("Arial",1);
      MakeFont("Symbol",2);
   }


 //  ReleaseDC(hWnd,hdc); This should only be released with the opengl window is closed.
}

bool COpenGLWin::SetupPixelFormat()
{ 
    PIXELFORMATDESCRIPTOR pfd, *ppfd; 
    int pixelformat; 
 
    ppfd = &pfd; 
 
 // Desired pixel format
    memset(&pfd, 0, sizeof(pfd));
    ppfd->nSize = sizeof(PIXELFORMATDESCRIPTOR); 
    ppfd->nVersion = 1; 
    if(mode == GLBITMAP)
       ppfd->dwFlags = PFD_DRAW_TO_BITMAP |  PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL;
    else
       ppfd->dwFlags = PFD_DRAW_TO_WINDOW  | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    ppfd->dwLayerMask = PFD_MAIN_PLANE; 
    ppfd->iPixelType = PFD_TYPE_RGBA; 
    ppfd->cColorBits = nrBits; 
    ppfd->cDepthBits = 8; //zDepth; 
    ppfd->cAccumBits = 0; 
    ppfd->cStencilBits = 0; 
    ppfd->cAlphaBits = 8;
 
 // Find the pixel format closest to the above value
    if(!(pixelformat = ChoosePixelFormat(hdcGL, ppfd))) 
        return(false); 
 
 // Make this the current format
    if(!SetPixelFormat(hdcGL, pixelformat, ppfd)) 
        return(false); 

    return(true); 
} 

bool COpenGLWin::CreateBitMap(HDC hdc, int width, int height) 
{
   BITMAPINFOHEADER bih;
   void* pBits;

// Initialize the bitmapinfo header
	int size = sizeof(BITMAPINFOHEADER) ;
	memset(&bih, 0, size);

// Populate bitmapinfo header
	bih.biSize = size;
	bih.biWidth = ((((int) width * 8) + 31) & ~31) >> 3;
	bih.biHeight = height;
	bih.biPlanes = 1;
	bih.biBitCount = nrBits;
	bih.biCompression = BI_RGB;


// Create the DIB section.
	hBitMap = CreateDIBSection(hdc,
							(BITMAPINFO*)&bih,
							DIB_RGB_COLORS,
							&pBits,
							NULL,
							0);
   if(!hBitMap)
      return(false);

   return(true);
}



bool COpenGLWin::ResizeBitMap(int w, int h)
{
   if(mode == GLBITMAP)
   {
      HDC hdc = GetDC(parentWin);
	   if (hBitMap) DeleteObject(hBitMap);
      CreateBitMap(hdc, w,h); 
	   SelectObject(hdcGL,hBitMap);
      MakeFont("Arial",1);
      ReleaseDC(parentWin,hdc);
   }

   return(true);
}


void COpenGLWin::Draw()
{
   RECT r;
   HDC hdc = GetDC(parentWin);
   GetClientRect(parentWin,&r);
   glFlush();
   if(mode == GLBITMAP)
      BitBlt(hdc,0,0,r.right,r.bottom,hdcGL,0,0,SRCCOPY);
   else
      SwapBuffers(hdc); 
   ReleaseDC(parentWin,hdc);
}

void COpenGLWin::SetMode(short newMode)
{
   if(mode == newMode) return;
   mode = newMode;
   RECT r;
   GetClientRect(parentWin,&r);
   HDC hdc = GetDC(parentWin);

   if(newMode == GLBITMAP)
   {
      wglMakeCurrent(hdcGL, NULL); 
      wglDeleteContext(hRC); 

      CreateBitMap(hdc, r.right,r.bottom); 
      hdcGL = CreateCompatibleDC(hdc);
	   SelectObject(hdcGL,hBitMap);
   }
   else
   {
      wglMakeCurrent(hdcGL, NULL); 
      wglDeleteContext(hRC); 
      DeleteDC(hdcGL);
      if(hBitMap)
         DeleteObject(hBitMap);
      hdcGL = hdc;
   }
   SetupPixelFormat(); 
   hRC = wglCreateContext(hdcGL); 
   wglMakeCurrent(hdcGL,hRC);
 //  ReleaseDC(parentWin,hdc);
}

void COpenGLWin::DrawString(char *s, short listbase)
{
    GLsizei len = GLsizei(strlen(s));
    if(s && len > 0)
    {
		// Must save/restore the list base.
	  glPushAttrib(GL_LIST_BIT);
     {
         if(listbase == 1)
         {
        //    glScalef(1.25,1.25,1.25);
//			   glListBase(glWin3d.listbase1);
			   glListBase(listbase1);
         	glCallLists(len, GL_UNSIGNED_BYTE, (const GLvoid*)s);
        //    glScalef(0.8,0.8,0.8);
         }
         else
         {
       //     glScalef(1.5625,1.5625,1.5625);
            glScalef(1.25,1.25,1.25);
			   glListBase(listbase2);
//			   glListBase(glWin3d.listbase2);
         	glCallLists(len, GL_UNSIGNED_BYTE, (const GLvoid*)s);
       //     glScalef(0.64,0.64,0.64);
            glScalef(0.8,0.8,0.8);
         }
		} 
      glPopAttrib();
    }
}

void COpenGLWin::MakeFont(char *fontname, short lb)
{
   if (hdcGL && fontname && strlen(fontname) > 0) 
   {
      short listbase = glGenLists(256);

        // Setup the Font characteristics
      LOGFONT logfont;
      logfont.lfHeight        = -12;
      logfont.lfWidth         = 0;
      logfont.lfEscapement    = 0;
      logfont.lfOrientation   = logfont.lfEscapement;
      logfont.lfWeight        = FW_NORMAL;
      logfont.lfItalic        = FALSE;
      logfont.lfUnderline     = FALSE;
      logfont.lfStrikeOut     = FALSE;
      logfont.lfCharSet       = ANSI_CHARSET;
      logfont.lfOutPrecision  = OUT_DEFAULT_PRECIS;
      logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
      logfont.lfQuality       = DEFAULT_QUALITY;
      logfont.lfPitchAndFamily = FF_DONTCARE|DEFAULT_PITCH;
      strcpy(logfont.lfFaceName, fontname);

      HFONT font = CreateFontIndirect(&logfont);
      HFONT oldfont = (HFONT)SelectObject(hdcGL,font);
      if(lb == 1)
      {
         if (!wglUseFontOutlines(hdcGL,0, 256, listbase, 0.0, 0.0, WGL_FONT_POLYGONS, gmf1))
         {
            glDeleteLists(listbase, 256);
         }
         else 
         {
            SelectObject(hdcGL,oldfont);
         }
      }
      else
      {
         if (!wglUseFontOutlines(hdcGL,0, 256, listbase, 0.0, 0.0, WGL_FONT_POLYGONS, gmf2))
         {
            glDeleteLists(listbase, 256);
         }
         else 
         {
            SelectObject(hdcGL,oldfont);
         }
      }

      if(lb == 1)
         listbase1 = listbase;
      else
         listbase2 = listbase;
   }
}

void COpenGLWin::CharDim(char c, float &width, float &height, short listbase)
{
   int index = (int)c;
   if(listbase == 1)
   {
      width = gmf1[index].gmfBlackBoxX;
      height = gmf1[index].gmfBlackBoxY;
   }
   else
   {
      width = gmf2[index].gmfBlackBoxX;
      height = gmf2[index].gmfBlackBoxY;
   }
}

void COpenGLWin::StrDim(char *s, float &width, float &height, short listbase)
{
   float ht;
   unsigned int index;
   width = 0;
   height = 0;

   if(listbase == 1)
   {
      for(int i = 0; i < strlen(s); i++)
      {
         index = (unsigned char)s[i];
         width += gmf1[index].gmfCellIncX;
         ht = gmf1[index].gmfBlackBoxY;
         if(ht > height) height = ht;
      }
   }
   else
   {
      for(int i = 0; i < strlen(s); i++)
      {
         index = (unsigned char)s[i];
         width += gmf2[index].gmfCellIncX;
         ht = gmf2[index].gmfBlackBoxY;
         if(ht > height) height = ht;
      }
   }
}
