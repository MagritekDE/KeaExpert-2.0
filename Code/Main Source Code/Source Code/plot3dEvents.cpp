#include "stdafx.h"
#include "plot3dEvents.h"
#include <math.h>
#include "bitmap.h"
#include "defineWindows.h"
#include "files.h"
#include "globals.h"
#include "cli_files.h"
#include "import_export_utilities.h"
#include "guiMakeObjects.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "interface.h"
#include "load_save_data.h"
#include "main.h"
#include "metafile.h"
#include "plot.h"
#include "plot3dClass.h"
#include "plot3dSurface.h"
#include "process.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesOther.h"
#include "gl/gl.h" 
#include "gl/glu.h"
#include <shellapi.h>
#include "memoryLeak.h"

#pragma warning (disable: 4996) // Ignore deprecated library functions
#pragma warning (disable: 4311) // Ignore pointer truncation warnings

short Make3DImageFile(HWND hWnd, char *inputFileName);


/*****************************************************************************
*            Routines relating to the 3D user defined plot window
*
* Plot3DFunctions ................. plot3dfunc command arguments to access image functions: 
*
*****************************************************************************/

short Render(HWND hwnd, HDC hdc);
void OpenGLBitmapToClipboard(HWND hwnd);
short Make3DImageFile(HWND hWnd, char *inputFileName);

float rotateStep3d = 2;
float shiftStep3d  = 2;
float scaleStep3d  = 0.8;
float zStep3d      = 5;

/*****************************************************************************
*                   Event procedure for 3D plot window
* Events
*
* WM_MOUSEACTIVATE ... Plot selected
* WM_PAINT ........... Redraw plot
* WM_ERASEBKGND ...... Prevent flicker
* WM_LBUTTONDOWN ..... Reset mouse coordinates 
* WM_LBUTTONUP ....... Reset mouse coordinates
* WM_RBUTTONDOWN ..... Contextual menus
* WM_MOUSEMOVE ....... Move, data display region selection.
* WM_KEYDOWN ......... Show hide GUI window
*
*****************************************************************************/

LRESULT CALLBACK Plot3DEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   static long oldx,oldy,oldz;

// Find object receiving events
   HWND parWin = GetParent(hWnd);
   WinData* win = rootWin->FindWinByHWND(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);    
   ObjectData *obj = GetObjData(hWnd); 
   if(!obj || !obj->data) return(1);

   plot3d = (CPlot3D*)obj->data;
   cur3DWin = hWnd;


	switch(messg)
	{
      case(WM_DROPFILES): // User dropped a file onto the window
      {
         CText path,file,ext;
         if(GetDropFileInfo((HDROP)wParam, path, file, ext,0) == OK)
         {
            DragFinish((HDROP)wParam);
            Interface itfc;
		      char cmd[MAX_STR];
            SetCursor(LoadCursor(NULL,IDC_WAIT));

			   if(ext == "2d")
            {
               CText varName;
		         varName = file.Str();
		         RemoveExtension(varName.Str());

		         if(LoadData(&itfc, path.Str(), file.Str(), varName.Str(), GLOBAL) == OK)
		         { 
                   sprintf(cmd,"surf2dParameters:plot_data(\"%s\")",varName.Str());
                   ProcessMacroStr(1,NULL,NULL,cmd,"","","surf2dParameters.mac","");
               }
            }
			   else	if(ext == "3d")
			   {
               CText varName;
		         varName = file.Str();
		         RemoveExtension(varName.Str());

		         if(LoadData(&itfc, path.Str(), file.Str(), varName.Str(), GLOBAL) == OK)
		         { 
                  sprintf(cmd,"display3dIsosurface:plot_data(\"%s\")",varName.Str());
                  ProcessMacroStr(1,NULL,NULL,cmd,"","","display3dIsosurface.mac","");
               }
			   }
            SetCursor(LoadCursor(NULL,IDC_ARROW));	
         }
         return(0);
      }

    // User has clicked on window - update title, menu and toolbar
      case(WM_MOUSEACTIVATE): 
      {
         SelectFixedControls(win, obj);
			CText txt;
			if(win->titleUpdate)
         {
			   txt.Format("3D Plot (%hd)",win->nr);
         	SetWindowText(win->hWnd,txt.Str());	
         }
			//else
			//   txt.Format("3D Plot");

         if(win && !win->keepInFront)
            ChangeGUIParent(parWin);
         SetFocus(hWnd);
         if(plot3d)
             wglMakeCurrent(plot3d->glWin3d.hdcGL,plot3d->glWin3d.hRC);
         break;
      }

   // Redraw window when damaged
		case(WM_PAINT): 
		{
		   PAINTSTRUCT p;	
         HDC hdc;
         if(wParam == 0)
            hdc = BeginPaint(hWnd, &p ); 
         else 
            hdc = (HDC)wParam; // Allow for WM_PRINTCLIENT call
 
         if(plot3d)
         {
            if(plot3d->initialised)
               SurfacePlot3D(hWnd,hdc);
            else
            {
               HDC hDC = GetDC(hWnd);
               HBRUSH bkBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
               RECT r;
               SetRect(&r,0,0,obj->wo,obj->ho);
               FillRect(hDC,&r,bkBrush);
               ReleaseDC(hWnd,hDC);
            }
         }
         if(wParam == 0) // Allow for WM_PRINTCLIENT call
		      EndPaint(hWnd, &p );	

         if(obj->selected_)
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawSelectRect(hdc);
            ReleaseDC(obj->hwndParent,hdc);
         }
         if(win->displayObjCtrlNrs)
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawControlNumber(hdc);
            ReleaseDC(obj->hwndParent,hdc);
         }
         if(win->displayObjTabNrs)
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawTabNumber(hdc);
            ReleaseDC(obj->hwndParent,hdc);
         }

		   break;	 	   
      }

  // Draw the window to a specific device context
		case(WM_PRINTCLIENT): 
		{
         SendMessage(hWnd, WM_PAINT, wParam, lParam);
         break;
      }

   // Prevent background being cleared before resize ****
	   case(WM_ERASEBKGND): 
	   {
	      return(1);
	   }


    // Select contextual menu   
		case(WM_RBUTTONDOWN): 
		{ 
		   POINT p;
         Variable *var;
		   short type,cnt = 1;

         p.x = LOWORD(lParam);
         p.y = HIWORD(lParam);	
	         
	      HMENU hMenu1 = LoadMenu (prospaInstance, "PLOT3DMENU");
         HMENU hMenu = GetSubMenu(hMenu1,0) ;

      // Add 2D menu
	      HMENU hMenu2D = CreatePopupMenu();
      // Add all 2D matrices
		   var = &globalVariable;
	      while(var)
		   {
		      var = var->GetNext(type);
		      if(!var) break;
		      if(var->GetVisible() && (type == MATRIX2D || type == CMATRIX2D)  && VarWidth(var) > 1 && VarHeight(var) > 1)
		      {
		         AppendMenu(hMenu2D,MF_STRING,cnt++,var->GetName());
		      }
		   }
         InsertMenu(hMenu, 2, (UINT) MF_POPUP | MF_BYPOSITION	,(UINT)hMenu2D ,"Display 2D Surface Plot");

      // Add 3D menu
	      HMENU hMenu3D = CreatePopupMenu();	      
      // Add all 3D matrices
		   var = &globalVariable;
	      while(var)
		   {
		      var = var->GetNext(type);
		      if(!var) break;
		      if(var->GetVisible() && type == MATRIX3D)
		      {
		         AppendMenu(hMenu3D,MF_STRING,cnt++,var->GetName());
		      }
		   }

         InsertMenu(hMenu, 3, (UINT) MF_POPUP | MF_BYPOSITION	,(UINT)hMenu3D ,"Display 3D Surface Plot");

      // Let the user choose!
         ClientToScreen(hWnd,&p);
         short item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL) ;
         if(item >= WINDOW_MENU_START)
         { 
		    //  SendMessage(plot3DWin,WM_COMMAND,(WPARAM)item,(LPARAM)NULL);
         }
         else  // Display the matrix
         {
		      char matrixName[MAX_STR];
		      char cmd[MAX_STR];

		      if(item >= 1)
            {
               if(item == ID_CLEAR_3D_PLOT)
               {
                  Interface itfc;
                  update3d = true;
	               Clear3D(&itfc,"");
               }
               else
               {
                  SetCursor(LoadCursor(NULL,IDC_WAIT));
                  GetMenuString(hMenu,item,matrixName,100,MF_BYCOMMAND);
		            var = &globalVariable;
	               while(var)
		            {
		               var = var->GetNext(type);
		               if(!var) break;
                     if(!var->GetVisible()) continue;
		               if(!strcmp(var->GetName(),matrixName))
                     {
                        if(type == MATRIX2D || type == CMATRIX2D) // Display a 2D surface plot
		                  {
                           sprintf(cmd,"surf2dParameters:plot_data(\"%s\")",matrixName);
                           ProcessMacroStr(1,NULL,NULL,cmd,"","","surf2dParameters.mac","");
		                  }
                        else if(type == MATRIX3D)
		                  {
                           sprintf(cmd,"display3dIsosurface(\"%s\")",matrixName);
 	                        ProcessMacroStr(1,NULL,NULL,cmd,"","","display3dIsosurface.mac","");
		                  }
                     }
		            }
                  SetCursor(LoadCursor(NULL,IDC_ARROW));	
               }
            }
         }	         
         DestroyMenu(hMenu);  
         DestroyMenu(hMenu1);  
         DestroyMenu(hMenu2D);  
         DestroyMenu(hMenu3D);	  		               
         break;
      }


   // Reset the old mouse coordinates
      case(WM_LBUTTONDOWN): 
      case(WM_LBUTTONUP): 
         oldx = -1;
         oldy = -1;
         oldz = -1;
         break;


   // Mouse has been moved so rotate or shift plot *****************************
      case(WM_MOUSEMOVE): 
      { 
         RECT r;

     // Set the cursor
         SetCursor(LoadCursor(NULL,IDC_ARROW));  

     // Note dimensions of window
         GetClientRect(hWnd,&r);
         long w = r.right;
         long h = r.bottom;

	  // Note location of cursor	
         long x = LOWORD(lParam);  // horizontal position of cursor 
         long y = HIWORD(lParam);  // vertical position of cursor 
                     
     // Left mouse button is down         
         if((wParam & MK_LBUTTON))
         {
         // Left mouse button + shift is down while cursor is moving (rotate twist angle)        
            if((wParam & MK_SHIFT) && !(wParam & MK_CONTROL))
            {
               show_rotation_axis = false;
               if(oldx != -1)
               {
                  float xc = x-w/2;
                  float yc = y-h/2;
                  float r = pow((float)(xc*xc + yc*yc),(float)0.5);
                  float theta = acos(xc/r);
                  if(yc < 0) theta = 2*PI-theta;

                  float oxc = oldx-w/2;
                  float oyc = oldy-h/2;
                  float or = pow((float)(oxc*oxc + oyc*oyc),(float)0.5);
                  float otheta = acos(oxc/or);
                  if(yc < 0) otheta = 2*PI-otheta;

                  float dtheta = (theta-otheta)/PI*180;
     
                  if(plot3d->azimuth > 90 && plot3d->azimuth < 270) // Make sure rotation is in right direction
                     dtheta = -dtheta;

                  if(dtheta > 0)
	                  Rotate3D(ID_ROTATE_CCW_TWIST,dtheta);
                  else
	                  Rotate3D(ID_ROTATE_CW_TWIST,-dtheta);
           
               }
         	   oldx = x;
	            oldy = y;
		      }
         // Left mouse button + control is down while cursor is moving (pan mode)        
            else if(!(wParam & MK_SHIFT) && (wParam & MK_CONTROL))
            {
               show_rotation_axis = true;
               if(oldx != -1)
               {
	               long dx = x - oldx;
	               long dy = y - oldy;

                  if(dx >= 1)
	                  Shift3DXYZ(ID_SHIFT_3D_RIGHT,shiftStep3d);
                  else if(dx <= -1)
	                  Shift3DXYZ(ID_SHIFT_3D_LEFT,shiftStep3d);

                  if(dy >= 1)
	                  Shift3DXYZ(ID_SHIFT_3D_DOWN,shiftStep3d);
                  else if(dy <= -1)
	                  Shift3DXYZ(ID_SHIFT_3D_UP,shiftStep3d);
               }
         	   oldx = x;
	            oldy = y;
		      }
            else if((wParam & MK_SHIFT) && (wParam & MK_CONTROL))
            {
               show_rotation_axis = true;
               if(oldz != -1)
               {
	               long dz = y - oldz;

                  if(dz >= 1)
	                  Shift3DXYZ(ID_SHIFT_3D_IN,shiftStep3d);
                  else if(dz <= -1)
	                  Shift3DXYZ(ID_SHIFT_3D_OUT,shiftStep3d);
               }
	            oldz = y;
            }
         // Left mouse button is down while cursor is moving (rotate twist and elevation)        
            else
            {
               show_rotation_axis = false;
               if(oldx != -1)
               {
	               long dx = x - oldx;
	               long dy = y - oldy;
                 
                  if(dx >= 1)
	                  Rotate3D(ID_ROTATE_CW_AZIMUTH,dx/3.0);
                  else if(dx <= -1)
	                  Rotate3D(ID_ROTATE_CCW_AZIMUTH,-dx/3.0);
            
                  if(dy >= 1)
	                  Rotate3D(ID_ROTATE_CCW_ELEVATION,dy/3.0);
                  else if(dy <= -1)
	                  Rotate3D(ID_ROTATE_CW_ELEVATION,-dy/3.0);
               }
         	   oldx = x;
	            oldy = y;
            }
		   }
         break;
      } 

   // Check for show/hide gui windows
      case(WM_KEYDOWN):
      {
         unsigned char key = (unsigned char)wParam;

         if(key == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);
         }

         break;
      }

		default: // Other Events - let system handle them
		{
		   return(DefWindowProc( hWnd, messg, wParam, lParam ));
		}
	}

	return(0L);
}


int Plot3DFunctions(Interface* itfc ,char args[])
{
   CText func;
   short r;


   if((r = ArgScan(itfc,args,0,"command","e","t",&func)) < 0)
      return(r); 

// Make sure the current 3D object is in the selected window
   if(itfc->win && (itfc->win->hWnd != plot3d->parent->hwndParent))
   {
      ObjectData *obj = itfc->win->widgets.findByType(OPENGLWINDOW);
      if(obj)
      {
	     Invalidate3DPlot(cur3DWin); // Remove old indent
        CPlot3D* plot3d = (CPlot3D*)obj->data;
        cur3DWin = obj->hWnd;
	     Invalidate3DPlot(cur3DWin); // Remove old indent
        wglMakeCurrent(plot3d->glWin3d.hdcGL,plot3d->glWin3d.hRC);
      }
   }

   if(func == "scale x up")
       Scale3DXYZ(ID_SCALE_X_UP,scaleStep3d);
   else if(func == "scale x down")
       Scale3DXYZ(ID_SCALE_X_DOWN,scaleStep3d);
   else if(func == "scale y up")
       Scale3DXYZ(ID_SCALE_Y_UP,scaleStep3d);
   else if(func == "scale y down")
       Scale3DXYZ(ID_SCALE_Y_DOWN,scaleStep3d);
   else if(func == "scale z up")
       Scale3DXYZ(ID_SCALE_Z_UP,scaleStep3d);
   else if(func == "scale z down")
       Scale3DXYZ(ID_SCALE_Z_DOWN,scaleStep3d);

   else if(func == "rotate cw elevation")
       Rotate3D(ID_ROTATE_CW_ELEVATION,rotateStep3d);
   else if(func == "rotate ccw elevation")
       Rotate3D(ID_ROTATE_CCW_ELEVATION,rotateStep3d);
   else if(func == "rotate cw azimuth")
       Rotate3D(ID_ROTATE_CW_AZIMUTH,rotateStep3d);
   else if(func == "rotate ccw azimuth")
       Rotate3D(ID_ROTATE_CCW_AZIMUTH,rotateStep3d);
   else if(func == "rotate cw twist")
       Rotate3D(ID_ROTATE_CW_TWIST,rotateStep3d);
   else if(func == "rotate ccw twist")
       Rotate3D(ID_ROTATE_CCW_TWIST,rotateStep3d);

   else if(func == "move left")
   {
      show_rotation_axis = true;
      Shift3DXYZ(ID_SHIFT_3D_LEFT,shiftStep3d);
   }
   else if(func == "move right")
   {
      show_rotation_axis = true;
      Shift3DXYZ(ID_SHIFT_3D_RIGHT,shiftStep3d);
   }
   else if(func == "move up")
   {
      show_rotation_axis = true;
      Shift3DXYZ(ID_SHIFT_3D_UP,shiftStep3d);
   }
   else if(func == "move down")
   {
      show_rotation_axis = true;
      Shift3DXYZ(ID_SHIFT_3D_DOWN,shiftStep3d);
   }

   else if(func == "smooth shading")
   {
      if(surface2DMode == 1)
        surface2DMode = 2;
      else
        surface2DMode = 1;
           
      Invalidate3DPlot(cur3DWin);
	}

   else if(func == "smooth shading on")
   {
      surface2DMode = 2;      
      Invalidate3DPlot(cur3DWin);
	}

   else if(func == "smooth shading off")
   {
      surface2DMode = 1;      
      Invalidate3DPlot(cur3DWin);
	}

   else if(func == "specular lighting")
   {
      specular = !specular;
      SetUpLightModel();
      Invalidate3DPlot(cur3DWin);
	}  

   else if(func == "reset parameters")
   {
      Initialize3DParameters();
      UpdateStatusWindow(cur3DWin,0,"");
      Invalidate3DPlot(cur3DWin);
   }

   else if(func == "save image")
   {
      Make3DImageFile(cur3DWin,NULL);
   }

   else if(func == "copy to clipboard")
   {
	   OpenGLBitmapToClipboard(cur3DWin);
   }

   else if(func == "clear plot")
   {
      Clear3D(itfc, "");
   }
   return(OK);
}


/*****************************************************************************
   Save the contents of the 3D plot hWnd as a graphics file
*****************************************************************************/

short Make3DImageFile(HWND hWnd, char *inputFileName)
{
   static char fileName[MAX_STR] = "untitled";
   static char directory[MAX_STR];
   char oldDir[MAX_PATH];

   static short index = 1;

// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);

// Get the image filename from the user
   if(inputFileName == NULL)
   {
      if(directory[0] == '\0')
         strncpy_s(directory,MAX_STR,gPlot3DDirectory,_TRUNCATE);
		short err = FileDialog(hWnd, false, directory, fileName, 
			"Save plot as image", CentreHookSaveProc, NULL, noTemplateFlag,6,&index,
			"EMF","emf",
			"BMP","bmp",
			"TIF","tif",
			"GIF","gif",
			"JPG","jpg",
			"PNG","png");  

      if(err != OK)
         return(ABORT);

   }
// Filename has been passed to function
   else
      strncpy_s(fileName,MAX_STR,inputFileName,_TRUNCATE);

   short r = plot3d->SaveAsImage(fileName);

// Restore the directory
   SetCurrentDirectory(oldDir);

   return(r);
}

// Test function for 3D - use in the Paint event.

short Render(HWND hwnd, HDC hdc)
{
   RECT r;
   GetClientRect(hwnd,&r);
	
   int w = r.right;

   for(int i = 0; i < 1000; i++)
   {
      glViewport(0, 0, w,w);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT	);
      glClearColor(0,0,0.3,0);
	   glColor3f(0.8,0.2,0);

      glEnable(GL_NORMALIZE);
      glShadeModel(GL_SMOOTH);
      glRotatef(0, 1.0f, 0.0f, 0.0f);
      glRotatef(0.1, 0.0f, 1.0f, 0.0f);
      glRotatef(0.1, 0.0f, 0.0f, 1.0f);	

      float s = 0.4;
      glBegin(GL_QUADS);
	   glNormal3f(1.0,0.0,0.0);
      glVertex3f(s,-s,-s);
      glVertex3f(s,s,-s);
      glVertex3f(-s,s,-s);
      glVertex3f(-s,-s,-s);
      glVertex3f(s,-s,-s);
      glEnd();

      glBegin(GL_QUADS);
	   glNormal3f(-1.0,0.0,0.0);
      glVertex3f(s,-s,s);
      glVertex3f(s,s,s);
      glVertex3f(-s,s,s);
      glVertex3f(-s,-s,s);
      glVertex3f(s,-s,s);
      glEnd();

      glBegin(GL_QUADS);
	   glNormal3f(0.0,1.0,0.0);
      glVertex3f(s,-s,-s);
      glVertex3f(s,s,-s);
      glVertex3f(s,s,s);
      glVertex3f(s,-s,s);
      glVertex3f(s,-s,-s);
      glEnd();

      glBegin(GL_QUADS);
      glNormal3f(0.0,-1.0,0.0);
      glVertex3f(-s,-s,-s);
      glVertex3f(-s,s,-s);
      glVertex3f(-s,s,s);
      glVertex3f(-s,-s,s);
      glVertex3f(-s,-s,-s);
      glEnd();

      glBegin(GL_QUADS);
      glNormal3f(0.0,0.0,1.0);
      glVertex3f(s,s,-s);
      glVertex3f(-s,s,-s);
      glVertex3f(-s,s,s);
      glVertex3f(s,s,s);
      glVertex3f(s,s,-s);
      glEnd();

      glBegin(GL_QUADS);
      glNormal3f(0.0,0.0,-1.0);
      glVertex3f(s,-s,-s);
      glVertex3f(-s,-s,-s);
      glVertex3f(-s,-s,s);
      glVertex3f(s,-s,s);
      glVertex3f(s,-s,-s);
      glEnd();
  

      plot3d->glWin3d.Draw();
   }


   return(0);   
}  

/*********************************************************
    Copy the current opengl bitmap to the clipboard
*********************************************************/

void OpenGLBitmapToClipboard(HWND hwnd) 
{ 
   HDC hdc;

// Make sure window is redrawn
   Invalidate3DPlot(hwnd);

// Get client geometry 
   RECT r; 
   GetClientRect(hwnd,&r);

// Work out width and height of destination image
	long h = r.bottom;
   long w = r.right;

   w -= w % 4; 

// Create a bitmap and select it in the device context 
   hdc = GetDC(hwnd); 
   HDC hdcMem = CreateCompatibleDC(hdc);
   HBITMAP bitmap = CreateCompatibleBitmap(hdc,w,h); 
   SelectObject(hdcMem,bitmap); 

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
   glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,pPixelData); 

// Fill header 
   BITMAPINFOHEADER header; 
   header.biWidth = w; 
   header.biHeight = h; 
   header.biSizeImage = NbBytes; 
   header.biSize = sizeof(BITMAPINFOHEADER); 
   header.biPlanes = 1; 
   header.biBitCount =  3 * 8; // RGB 
   header.biCompression = BI_RGB; 
   header.biXPelsPerMeter = 0; 
   header.biYPelsPerMeter = 0; 
   header.biClrUsed = 0; 
   header.biClrImportant = 0; 

// Swap red and blue (why?)
   unsigned char ctemp;
   for(int i = 0; i < w * h; i++)
   {
      ctemp = pPixelData[i*3];
      pPixelData[i*3] = pPixelData[i*3+2];
      pPixelData[i*3+2] = ctemp;
   }

// Generate handle 
   HANDLE handle = (HANDLE)GlobalAlloc (GHND,sizeof(BITMAPINFOHEADER) + NbBytes); 
   if(handle != NULL) 
   { 
      // Lock handle 
      char *pData = (char *) GlobalLock((HGLOBAL)handle); 
      // Copy header and data 
      memcpy(pData,&header,sizeof(BITMAPINFOHEADER)); 
      memcpy(pData+sizeof(BITMAPINFOHEADER),pPixelData,NbBytes); 
      // Unlock 
      GlobalUnlock((HGLOBAL)handle); 

      // Push DIB in clipboard 
      if(OpenClipboard(NULL))
      {
         EmptyClipboard(); 
         SetClipboardData(CF_DIB,handle); 
         CloseClipboard(); 
      }
   } 

// Cleanup 
   DeleteDC(hdcMem); 
   DeleteObject(bitmap); 
   delete [] pPixelData; 
} 

/*********************************************************
 Copy the current opengl bitmap to an enhanced metafile dc
*********************************************************/

void OpenGLBitmapToEMF(HDC hdcEMF, HDC hdcMem, long x, long y, long w, long h) 
{ 
   w -= w % 4; 

   DIBSECTION section; 
   HBITMAP hBitmap;
   long newWidth;
   GenerateBitMap(w,h,&hBitmap,hdcMem,newWidth,false);
   SelectObject(hdcMem,hBitmap); 

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

// Copy to EMF
   BitBlt(hdcEMF,0,0,w,h,hdcMem,0,0,SRCCOPY);

// Cleanup 
   delete [] pPixelData; 
   DeleteObject(hBitmap);
}


  
// Set sensitivity of 3D controls
int Set3DControlPrefs(Interface *itfc, char args[])
{
   short nrArgs;
   float rotateStep,shiftStep,scaleStep,zStep;


// Intialise values
   rotateStep = rotateStep3d;
   shiftStep  = shiftStep3d;
   scaleStep  = scaleStep3d;
   zStep      = zStep3d;

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,4,"rotate, shift, scale, distance","eeee","ffff",&rotateStep, &shiftStep, &scaleStep, &zStep)) < 0)
     return(nrArgs); 

// Test for errors
   if(rotateStep <= 0 || shiftStep <= 0 || scaleStep <= 0 || zStep <= 0)
   {
      ErrorMessage("zero or negative parameter");
      return(ERR);
   }

// Update with new values
   rotateStep3d  = rotateStep;
   shiftStep3d   = shiftStep;
   scaleStep3d   = scaleStep;
   zStep3d       = zStep;

	itfc->nrRetValues = 0;
   return(OK);   
}

// Scroll wheel events come from the parent window 
void Process3DScrollWheelEvents(HWND hWnd, short zDel, short fwKeys)
{
 //  if(macroDepth == 0) // Don't do anything here if running a macro
   {
      float zDelta = -zDel/120.0;    // wheel rotation

      if(fwKeys == MK_SHIFT) // Slow
         zDelta *= zStep3d/5;
      else if(fwKeys == MK_CONTROL) // Fast
         zDelta *= zStep3d*5;
      else
         zDelta *= zStep3d; // Normal

      Shift3DZ(zDelta);
   }
}