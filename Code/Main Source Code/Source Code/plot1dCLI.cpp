#define WINVER _WIN32_WINNT 
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h>
#include "stdafx.h"
#include "allocate.h"
#include "plot1dCLI.h"
#include "cArg.h"
#include "defineWindows.h"
#include "error.h"
#include "evaluate.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "Inset.h"
#include "interface.h"
#include "files.h"
#include "plotLine.h"
#include "list_functions.h"
#include "mymath.h"
#include "plot.h"
#include "PlotWindow.h"
#include "plotWinDefaults.h"
#include "process.h"
#include "plotText.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "structure.h"
#include "TracePar.h"
#include "TraceParCLI.h"
#include "variablesOther.h"
#include "variablesClass.h"
#include <algorithm>
#include <deque>
#include <string>
#include <algorithm>
#include "memoryLeak.h"

using std::string;
using std::exception;
using std::deque;
using namespace Gdiplus;
using namespace std;

int IsPlotIdle(Interface *itfc, char *args);


// Other functions.

int gPlotViewVersion = 1;

int PlotViewVersion(Interface* itfc ,char arg[])
{
   short n;
   long version = gPlotViewVersion;

   if((n = ArgScan(itfc,arg,1,"version","e","l",&version)) < 0)
      return(n);

   gPlotViewVersion = version;

   return(OK);
}


int IsPlotIdle(Interface* itfc, char args[])
{
   short nrArgs;
   CText whichPlot, cmd;
   short xPos, yPos;
   CText title;
   CArg carg;
   Plot *plt;


   nrArgs = carg.Count(args);

   // Return region class
   if (nrArgs == 0)
   {
      if (Plot::curPlot())
      {
         plt = Plot::curPlot();
      }
      else
      {
         ErrorMessage("no current plot defined");
         return(ERR);
      }
   }

   // Set current plot
   else if (nrArgs == 1)
   {
      Variable region;

      if ((nrArgs = ArgScan(itfc, args, 1, "region", "e", "v", &region)) < 0)
         return(nrArgs);

      if (region.GetType() == CLASS)
      {
         ClassData *cd = (ClassData*)region.GetData();

         if (CheckClassValidity(cd, true) == ERR)
            return(ERR);

         if (cd->type == PLOT_CLASS)
         {
            plt = (Plot*)cd->data;
            if (!plt || ((plt->getDimension() != 1) && (plt->getDimension() != 2)))
            {
               ErrorMessage("invalid plot dimension");
               return(ERR);
            }
         }
      }
      else
      {
         ErrorMessage("invalid plot region argument");
         return(ERR);
      }
   }



   return(OK);
}

// Allow Prospa to save 1D plots in old formats
int PlotFileSaveVersion(Interface* itfc, char args[])
{
   short n;
   extern long plot1DSaveVersion;

   long version = nint(fileVersionConstantToNumber(plot1DSaveVersion,1));

   if((n = ArgScan(itfc,args,1,"version","e","l",&version)) < 0)
      return(n);

   version =  fileVersionNumberToConstant(version,1);
   if(version == -1)
      return(ERR);

   plot1DSaveVersion =  version;

   return(OK);
}

int Clear1D(Interface* itfc, char args[])
{
	Plot1D* cur1DPlot = Plot1D::curPlot();
   if(cur1DPlot)
   {
		cur1DPlot->clearData();
      cur1DPlot->setOverRideAutoRange(false);
      MyInvalidateRect(cur1DPlot->win,NULL,false);
      cur1DPlot->initialiseMenuChecks("clear");
	   //std::for_each(cur1DPlot->insets_.begin(), cur1DPlot->insets_.end(), delete_object());
    //  cur1DPlot->insets_.clear();
	   std::for_each(cur1DPlot->lines_.begin(), cur1DPlot->lines_.end(), delete_object());
      cur1DPlot->lines_.clear();
   }
   itfc->nrRetValues = 0;
   return(OK);
}

int SetPlotState(Interface *itfc, char args[])
{
   short r;
   CText state;

	if((r = ArgScan(itfc,args,1,"state","e","t",&state)) < 0)
	   return(r); 
	   
   if(Plot::curPlot())
		Plot::curPlot()->setPlotState(state.Str());
      
   return(OK);
}


int GetPlotState(Interface *itfc, char arg[])
{
   if(Plot::curPlot())
      itfc->retVar[1].MakeAndSetString(Plot::curPlot()->getPlotState());
	itfc->nrRetValues = 1;
   return(OK);
}

/*********************************************************************************
  Draw a 1D plot
 
  Draws an x-y plot in one of several modes.
  
  Arguments:   plot(y)  ... plot y vs its index (float 1D matrix of N elements)
               plot(x,y) .. plot y vs x (float 1D matrices of N elements each)
             
             
  Options arguments are parameter,value pairs describing the plot trace parameters
  
   symbolshape .... opensquare/square/opencircle/circle/
                    opentriangle/triangle/openinvtriangle/invtriangle/
                    opendiamond/diamond/cross/plus/none
   symbolcolor .... [R,G,B] (0-255)
   symbolsize ..... (1-19)
   tracetype ...... lines/stairs/dots/none
   tracecolor ..... [R,G,B]
   errorbars ...... [barup,bardn] float 2D matrix of 2 by N elements 
   errorcolor ..... [R,G,B]   
   
*********************************************************************************/

int PlotXY(Interface* itfc ,char arg[])
{
   long r;
	MSG msg;

	Plot1D* cur1DPlot = Plot1D::curPlot();

   if(!cur1DPlot)
   {
      ErrorMessage("no 1D plot window present");
      return(ERR);
   }

 //  while(cur1DPlot->plotParent->isBusy())
 //     PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
//	printf("Entering critical section PLOTXY\n");

   EnterCriticalSection(&cs1DPlot);
   cur1DPlot->plotParent->incCriticalSectionLevel();

   r = cur1DPlot->DrawPlot(itfc, arg);

   LeaveCriticalSection(&cs1DPlot);
   cur1DPlot->plotParent->decCriticalSectionLevel();
	//printf("Leaving critical section PLOTXY\n");

   if(r == ERR)
      return(ERR);

   itfc->retVar[1].MakeAndSetFloat(r); // Return trace ID
   itfc->nrRetValues = 1;
	cur1DPlot->makeCurrentPlot();

   return(OK);
}


int GetCurrentPlot(Interface* itfc, char arg[])
{
   itfc->nrRetValues = 2;

	if(Plot1D::curPlot())
   {
		PlotWindow* pp = Plot1D::curPlot()->plotParent;
      ObjectData *obj = pp->obj;
      itfc->retVar[1].MakeAndSetFloat(obj->winParent->nr);
      itfc->retVar[2].MakeAndSetFloat(obj->nr());
      return(OK);
   }

   itfc->retVar[1].MakeAndSetFloat(-1.0);
   itfc->retVar[2].MakeAndSetFloat(-1.0);
   return(OK);
}

/************************************************************************************
 Toggle auto-ranging
************************************************************************************/
 
int AutoRange(Interface* itfc, char arg[])
{
   CText ar;
   short r;

	Plot* curPlot = Plot::curPlot();
	Plot1D* cur1DPlot = Plot1D::curPlot();
	Plot2D* cur2DPlot = Plot2D::curPlot();

   if(curPlot)
   {  
		if(curPlot->getOverRideAutoRange())
		   ar = "off";
		else
		   ar = "on";
   } 
   else
      return(OK);
      
   if((r = ArgScan(itfc,arg,1,"on/off","e","t",&ar)) < 0)
      return(r); 
    
   if(curPlot->win == cur2DPlot->win)
   {  
      HMENU hMenu = GetMenu(cur2DPlot->win);	   
      if(ar == "on")
      {
         CheckMenuItem(hMenu,ID_PLOT2D_AUTOSCALE,MF_CHECKED);
         cur2DPlot->setOverRideAutoRange(false);
      }
      else if(ar == "off")
      {
         CheckMenuItem(hMenu,ID_PLOT2D_AUTOSCALE,MF_UNCHECKED);
         cur2DPlot->setOverRideAutoRange(true);
      }
		else
		{
		  ErrorMessage("invalid parameter");
		  return(ERR);
		} 
   }
   else if(curPlot->win == cur1DPlot->win)
   {  

      HMENU hMenu = GetMenu(cur1DPlot->win);	   
      if(ar == "on")
      { 
         cur1DPlot->setOverRideAutoRange(false);
      }
      else if(ar == "off")
      {
         cur1DPlot->setOverRideAutoRange(true);
      }
		else
		{
		  ErrorMessage("invalid parameter");
		  return(ERR);
		} 
   }
   else
   {
      ErrorMessage("no current plot defined");
      return(ERR);
   }

	itfc->nrRetValues = 0;
           
	return(0);
}

 
/************************************************************************************
           Redraw plot using current drawing method
************************************************************************************/

int DrawPlot(Interface* itfc, char args[])
{
   short nrArgs;
   CText draw;

	Plot1D* cur1DPlot = Plot1D::curPlot();

   if(cur1DPlot->plotParent->updatePlots())
      draw = "true";
   else
      draw = "false";
   
   if((nrArgs = ArgScan(itfc,args,1,"true/false","e","t",&draw)) < 0)
     return(nrArgs);
   
   if(draw == "true" || draw == "yes" || draw == "on")
   {
      cur1DPlot->plotParent->updatePlots(true);
   }
   else if(draw == "false" || draw == "no" || draw == "off")
   {
      cur1DPlot->plotParent->updatePlots(false);
   }
   else
   {
      ErrorMessage("invalid argument");
      return(ERR);
   }

   cur1DPlot->plotParent->DisplayAll(false);  
   
   itfc->nrRetValues = 0;

   return(0);     
}	


/************************************************************************************
   CLI call to interface to "GetOrSetTraceParameters", allowing trace parameters
   to be modified.
************************************************************************************/

int ModifyTrace(Interface* itfc, char args[])
{
   CArg carg;

	Plot* curPlot = Plot::curPlot();
	Plot1D* cur1DPlot = Plot1D::curPlot();

// Check for current plot
   if(curPlot == NULL)
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }

	if(cur1DPlot->hasNoCurTrace())
   {
      ErrorMessage("no data defined");
      return(ERR);
   }
        
   short nrArgs = carg.Count((char*)args);

// Prompt user if no arguments supplied
   if(nrArgs == 0)
   {
      Trace *di = cur1DPlot->curTrace();
      TracePar *tp = &(di->tracePar);
		TextMessage(formatNoArgHeader().c_str());
		TextMessage(di->FormatState().c_str());
		TextMessage(tp->FormatState().c_str());
      return(0);
   }

	Trace* di = cur1DPlot->curTrace();
   if((GetOrSetTraceParameters(itfc,&carg,nrArgs,1,di,di->tracePar,cur1DPlot)) == OK)
   {
      cur1DPlot->DisplayAll(false); 
      return(0);
   }

	return(ERR);
}


/************************************************************************************
   CLI call to interface to "GetOrSetTraceParameters", allowing default trace parameters
   to be modified.
************************************************************************************/

int ModifyTraceDefault(Interface* itfc ,char args[])
{
   int r;
   CArg carg;
   short nrArgs = carg.Count((char*)args);
	Plot1D* cur1DPlot = Plot1D::curPlot();

   if(cur1DPlot == NULL)
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }

// Prompt user if no arguments supplied
   if(nrArgs == 0)
   {
      TracePar *tp = cur1DPlot->getTracePar();
		TextMessage(formatNoArgHeader().c_str());
		TextMessage(tp->FormatState().c_str());
      return(0);
   }
  
   r = GetOrSetTraceParameters(itfc, &carg, nrArgs, 1, NULL, *cur1DPlot->getTracePar(),cur1DPlot);
   return(r);
}



/********************************************************************
 Transfer a color from the variable colIn to the color-reference colOut.
 Check for correct data type and other errors.
 Return OK if correct ERR if not.
********************************************************************/

short SetColor(COLORREF &colOut, Variable *colIn)
{
   short type = colIn->GetType();
   short red,green,blue,other = 0;

   if(type != MATRIX2D || colIn->GetDimY() != 1)
   {
      ErrorMessage("invalid type for color - should be a row vector");
      return(ERR);
   }

   short xDim = colIn->GetDimX();

   if(xDim == 3 || xDim == 4)
   {
      red   = nint(colIn->GetMatrix2D()[0][0]);
      green = nint(colIn->GetMatrix2D()[0][1]);
      blue  = nint(colIn->GetMatrix2D()[0][2]);
   }
   else
   {
      ErrorMessage("color vector must have length 3 or 4");
      return(ERR);
   }

   if(colIn->GetDimX() == 4)
   {
      other   = nint(colIn->GetMatrix2D()[0][3]);
   }

   if(red < 0 || red > 255 ||
      green < 0 || green > 255 || 
      blue < 0 || blue > 255 ||
      other < 0 || other > 255)
   {
      ErrorMessage("invalid color value (valid range is 0 to 255)");
      return(ERR);
   }

   colOut = RGB((BYTE)red,(BYTE)green,(BYTE)blue);

   colOut = colOut | ((BYTE)other)<<24;

   return(OK);
}

/******************************************************************
   Convert ansVar from a 1 by 2 array to two floats: (out1,out2)
******************************************************************/
 
short ConvertAnsVar(Variable* ans, short type, float &out1, float &out2)
{
   if(type != MATRIX2D)
   {
      ErrorMessage("invalid type for tick spacing");
      return(ERR);
   }

   out1 = ans->GetMatrix2D()[0][0];  
   out2 = ans->GetMatrix2D()[0][1];
      
   if(out1 < 0 || out2 < 0)
   {
      ErrorMessage("tick spacing must be > 0");
      return(ERR);
   }

   if(out2 < 0)
   {
      ErrorMessage("ticks per label must be > 0");
      return(ERR);
   }

   if(out2 != nint(out2))
   {
      ErrorMessage("ticks per label must be integer");
      return(ERR);
   }
            
   return(0);
}

/******************************************************************
   Convert ansVar to a boolean
******************************************************************/

short ConvertAnsVar(Variable* ans, char name[], short type, bool &out)
{
   if(type != UNQUOTED_STRING)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }
   
   if(!strcmp(ans->GetString(),"on") || !strcmp(ans->GetString(),"true"))
   {
      out = true;
   }
   else if(!strcmp(ans->GetString(),"off") || !strcmp(ans->GetString(),"false"))
   {
      out = false;
   }
   else
   {
      ErrorMessage("invalid value for %s",name);
      return(ERR);
   }
   return(0); 
}


/******************************************************************
   Convert ansVar to a positive long integer (out)
******************************************************************/

short ConvertAnsVar(Variable* ans, char name[], short type, long &out)
{
   if(type != FLOAT32)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }
   
   if(ans->GetReal() < 0)
   {
      ErrorMessage("%s must be > 0",name);
      return(ERR);
   }
   
   out = nint(ans->GetReal()); 
   return(0); 
}


/******************************************************************
   Convert ansVar to a positive long integer (out)
******************************************************************/

short ConvertAnsVar(Variable* ans, char name[], short type, long *out)
{
   if(type != FLOAT32)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }
   
   if(ans->GetReal() < 0)
   {
      ErrorMessage("%s must be > 0",name);
      return(ERR);
   }
   
   *out = nint(ans->GetReal()); 
   return(0); 
}

/******************************************************************
   Convert ansVar to a positive short integer (out)
******************************************************************/

short ConvertAnsVar(Variable* ans, char name[], short type, short *out)
{
   if(type != FLOAT32)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }
   
   if(ans->GetReal() < 0)
   {
      ErrorMessage("%s must be > 0",name);
      return(ERR);
   }
   
   *out = nint(ans->GetReal()); 
   return(0); 
}


/******************************************************************
   Convert ansVar to a CText string
******************************************************************/

short ConvertAnsVar(Variable* ans, char name[], short type, CText *out)
{
   if(type != UNQUOTED_STRING)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }

   *out = ans->GetString(); 
   return(0); 
}


/******************************************************************
   Convert ansVar to a positive short integer (out)
******************************************************************/

short ConvertAnsVar(Variable* ans, char name[], short type, short &out)
{
   if(type == UNQUOTED_STRING)
   {
      if(!strcmp(ans->GetString(),"") || !strcmp(ans->GetString(),"getarg"))
         return(1);
   }

   if(type != FLOAT32)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }

   if(ans->GetReal() < 0)
   {
      ErrorMessage("%s must be > 0",name);
      return(ERR);
   }
   
   out = nsint(ans->GetReal()); 
   return(0); 
}

/******************************************************************
   Check if ansVar is a string
******************************************************************/

short ConvertAnsVar(Variable* ans, char name[], short type)
{
   if(type != UNQUOTED_STRING)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }

   if(!strcmp(ans->GetString(),"") || !strcmp(ans->GetString(),"getarg"))
      return(1);
   
   return(0); 
}

/******************************************************************
   Convert ansVar to a float (out)
******************************************************************/

short ConvertAnsVar(Variable* ans, char name[], short type, float &out)
{
   if(type != FLOAT32)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }
   
   out = ans->GetReal(); 
   return(0); 
}

/******************************************************************
   Convert ans from a 1 by 3 array to an RGB color (out)
******************************************************************/
   
short ConvertAnsVar(Variable *ans, char name[], short type, COLORREF &out)
{
   BYTE red,blue,green,other = 0;

// Check we have a matrix
   if(type == UNQUOTED_STRING)
   {
      if(!strcmp(ans->GetString(),"") || !strcmp(ans->GetString(),"getarg"))
         return(1);
      else
      {
         ErrorMessage("invalid type for %s",name);
         return(ERR);
      }
   }
   else if(type != MATRIX2D)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }

   short len = ans->GetDimX();

   if(len != 3 && len != 4)
   {
      ErrorMessage("colour vector must have length 3 or 4");
      return(ERR);
   }    

   red   = nint(ans->GetMatrix2D()[0][0]);
   green = nint(ans->GetMatrix2D()[0][1]);
   blue  = nint(ans->GetMatrix2D()[0][2]);

   if(len == 4)
      other = nint(ans->GetMatrix2D()[0][3]);

// Special case of default background colour
   if(red == 255 && green == 255 && blue == 255 && other == 255)
   {
      out = 0xFFFFFFFF;
      return(OK);
   }

// Check that this is a valid RGB colour   
   if(red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255)
   {
      ErrorMessage("invalid colour value (valid range is 0 to 255)");
      return(ERR);
   }

// Convert   
   out = RGB(red,green,blue); 
   return(OK); 
}

/******************************************************************
   Convert ans from a 1 by 4 array to an RGBA color (out)
******************************************************************/
   
short ConvertAnsVar4Color(Variable *ans, char name[], short type, COLORREF &out)
{
   BYTE red,blue,green,alpha = 0;

// Check we have a matrix
   if(type == UNQUOTED_STRING)
   {
      if(!strcmp(ans->GetString(),"") || !strcmp(ans->GetString(),"getarg"))
         return(1);
      else
      {
         ErrorMessage("invalid type for %s",name);
         return(ERR);
      }
   }
   else if(type != MATRIX2D)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }

   short len = ans->GetDimX();

   if(len != 4)
   {
      ErrorMessage("alpha colour vector must have length 4");
      return(ERR);
   }    

   red   = nint(ans->GetMatrix2D()[0][0]);
   green = nint(ans->GetMatrix2D()[0][1]);
   blue  = nint(ans->GetMatrix2D()[0][2]);
   alpha = nint(ans->GetMatrix2D()[0][3]);

// Check that this is a valid RGB colour   
   if(red < 0 || red > 255     ||
      green < 0 || green > 255 ||
      blue < 0 || blue > 255   ||
      alpha < 0 || alpha > 255)
   {
      ErrorMessage("invalid colour value (valid range is 0 to 255)");
      return(ERR);
   }

// Convert   
   out = (DWORD)(alpha<<24) + (DWORD)(blue<<16) + (DWORD)(green<<8) + (DWORD)(red); 
   return(OK); 
}

/******************************************************************
   Convert ans from a 1 by 3 array to an RGB color (out)
******************************************************************/
   
short ConvertAnsVar(Variable *ans, char name[], short type, COLORREF *out)
{
   short red,blue,green,other = 0;

// Check we have a matrix
   if(type == UNQUOTED_STRING)
   {
      if(!strcmp(ans->GetString(),"") || !strcmp(ans->GetString(),"getarg"))
         return(1);
      else
      {
         ErrorMessage("invalid type for %s",name);
         return(ERR);
      }
   }
   else if(type != MATRIX2D)
   {
      ErrorMessage("invalid type for %s",name);
      return(ERR);
   }

   short len = ans->GetDimX();

   if(len != 3 && len != 4)
   {
      ErrorMessage("colour vector must have length 3 or 4");
      return(ERR);
   }    

   red   = nint(ans->GetMatrix2D()[0][0]);
   green = nint(ans->GetMatrix2D()[0][1]);
   blue  = nint(ans->GetMatrix2D()[0][2]);

   if(len == 4)
      other = nint(ans->GetMatrix2D()[0][3]);

// Special case of default background colour
   if(red == 255 && green == 255 && blue == 255 && other == 255)
   {
      *out = 0xFFFFFFFF;
      return(OK);
   }

// Check that this is a valid RGB colour   
   if(red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255)
   {
      ErrorMessage("invalid colour value (valid range is 0 to 255)");
      return(ERR);
   }

// Convert   
   *out = RGB(red,green,blue); 
   return(OK); 
}

// Check
int SetPlotHoldMode(Interface* itfc, char args[])
{
   CText hold;
   short r;
   
	Plot1D* cur1DPlot = Plot1D::curPlot();

   if(cur1DPlot->displayHold)
      hold = "on";
   else
      hold = "off";
   
   if((r = ArgScan(itfc,args,0,"on/off","e","t",&hold)) < 0)
      return(r); 

	WinData* parent = cur1DPlot->plotParent->obj->winParent;
   if(hold == "on")
   {
      cur1DPlot->displayHold = true;
      parent->setToolBarItemCheck(cur1DPlot->plotParent->toolbarName(), "hold", true);
      parent->setMenuItemCheck(cur1DPlot->plotParent->menuName(),"hold",true);
   }   
   else if(hold == "off")
   {
		cur1DPlot->displayHold = false;
      parent->setToolBarItemCheck(cur1DPlot->plotParent->toolbarName(), "hold", false);
      parent->setMenuItemCheck(cur1DPlot->plotParent->menuName(),"hold",false);
   }
   else
   {
      cur1DPlot->displayHold = !cur1DPlot->displayHold;
      parent->setToolBarItemCheck(cur1DPlot->plotParent->toolbarName(),"hold", cur1DPlot->displayHold);
      parent->setMenuItemCheck(cur1DPlot->plotParent->menuName(),"hold",cur1DPlot->displayHold);
   }  

   itfc->nrRetValues = 0;
   
   return(0);
}

/****************************************************************************
   
*****************************************************************************/



// Zoom into the current 1D plot
int Zoom1D(Interface* itfc, char args[])
{
	Plot1D* cur1DPlot = Plot1D::curPlot();
   if(!cur1DPlot)
   {
      ErrorMessage("1D plot not defined");
      return(ERR);
   }

   short r = cur1DPlot->Zoom(itfc,args);
   if(r != -2)
      itfc->nrRetValues = 0;

   return(r);
}

/*****************************************************************
  Set or get the current 1D plot trace

  Syntax: CLASS trc = curtrace()  # gets current plot trace
          curtrace(INT ID) # sets current trace in current plot given trace ID
          curtrace(CLASS trc) # set current trace given a trace class

*****************************************************************/
  

int GetOrSetCurrentTrace(Interface* itfc, char args[])
{
   short n;
   Variable ID;
   Trace *di = 0;
	CArg carg;

   n = carg.Count(args);

// If no arguments then return data list class instance
   if(n == 0)
   {
      if(Plot1D::curPlot() && Plot1D::curPlot()->curTrace())
      {
         itfc->retVar[1].MakeClass(TRACE_CLASS,(void*)Plot1D::curPlot()->curTrace());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         ErrorMessage("no current trace");
         return(ERR);
      }
   }

   if((n = ArgScan(itfc,args,1,"trace ID/trace object","e","v",&ID)) < 0)
      return(n);

// Set the trace via id number or trace class instance
	if(Plot1D::curPlot())
   {
      if(ID.GetType() == CLASS)
      {
         ClassData *cdata = (ClassData*)ID.GetData();
         if(cdata->type == TRACE_CLASS)
            di = (Trace*)cdata->data;
      }
      else if(ID.GetType() == FLOAT32)
      {
         di = Plot1D::curPlot()->FindTraceByID(ID.GetReal());
      }
      
      if(di)
      {
			Plot1D::curPlot()->setCurTrace((Trace*)di);
         HWND hWnd = Plot1D::curPlot()->win;
         MyInvalidateRect(hWnd,NULL,false);
      }
      else
      {
         ErrorMessage("no trace data found with this ID");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("no current 1D plot found");
      return(ERR);
   }   
	itfc->nrRetValues = 0;
   return(OK);
}

/**********************************************************************************************************
* Allow the user to select a 1D coordinate. Return this coordinate.
*
* Syntax  x = getx()
*
***********************************************************************************************************/

int Get1DCoordinate(Interface *itfc, char args[])
{
	MSG msg ;
   short x1,y1;
	long xmin,xmax,ymin,ymax;
   bool drawing = false;
   CText mode = "value";
   short r;
   HCURSOR outofboundsCursor;
   HCURSOR trackingCursor;
   extern bool gScrollWheelEvent;

	Plot1D* cur1DPlot = Plot1D::curPlot();

// Don't bother if there is no 1D plot   
   if(!cur1DPlot)
     return(0);

// See if user wants to use index mode
	if((r = ArgScan(itfc,args,0,"mode","e","t",&mode)) < 0)
	   return(r); 

   if(r == 1 && (mode != "index" && mode != "value" && mode != "both"))
   {
      ErrorMessage("invalid mode");
      return(ERR);
   }
               
// Set cursor to steer user toward 1D plot
   outofboundsCursor = OneDCursor;
   trackingCursor = LoadCursor(NULL,IDC_ARROW);
   SetCursor(outofboundsCursor);
       
// Get some variables defined
	PlotDimensions dim (cur1DPlot->GetTop(),
		cur1DPlot->GetLeft(), 
		cur1DPlot->GetWidth(),
		cur1DPlot->GetHeight());
   
	xmin = dim.left();
	ymin = dim.top();
	xmax = xmin+dim.width();
	ymax = ymin+dim.height();
	   
   HWND hWnd = cur1DPlot->win;

   gBlockWaitCursor = true;
   gScrollWheelEvent = false;


// Make sure the event queue is processed before capturing the cursor
// This ensures the button which initiated this call is deactivated
// This loop is exited after we have left the button and repainted it
	if(itfc->obj && itfc->obj->type == BUTTON)
	{
		int state = 0;
		bool hasLeft = false;

      if (itfc->obj->hWnd)
      {
         state = SendMessage(itfc->obj->hWnd, BM_GETSTATE, 0, 0);
        // TextMessage("Button nr = % d\n", itfc->obj->nr());
      }

      PushButtonInfo* info = (PushButtonInfo*)itfc->obj->data;
      if(info->hovering || (state & BST_HOT))
		{
			while(true)
			{
            if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
            {
               if ((msg.message == WM_MOUSELEAVE && itfc->obj->hWnd))
               {
                  InvalidateRect(itfc->obj->hWnd, 0, true);
                  hasLeft = true;
               }
               // In case the leave event was missed
               if (msg.message == WM_MOUSEMOVE && msg.hwnd != itfc->obj->hWnd)
               {
                  SendMessage(itfc->obj->hWnd, WM_MOUSELEAVE, NULL, NULL);
               }
               // Try and prevent events going to other buttons - not always working though??
               if (msg.hwnd != itfc->obj->hWnd)
               {
                  TranslateMessage(&msg);
                  DispatchMessage(&msg);
               }

					if(hasLeft && msg.message == WM_PAINT)
						break;
				}
			}
		}
	}


// Force plot window to capture mouse events
   SetCapture(hWnd);
   	
// Draw cursor until user presses left mouse button
	bool button_pressed = false;
   while(true)
   {
	   if(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	   {	
         if(msg.hwnd == hWnd)
         {
				if(msg.message == WM_MOUSEMOVE && msg.wParam == 0) // Track movement
				{
	      		x1 = LOWORD(msg.lParam);
					y1 = HIWORD(msg.lParam);
			  
				 // Check for out of bounds mouse movement			   
					if(x1 <= xmin || x1 >= xmax || y1 <= ymin || y1 >= ymax)
						 SetCursor(outofboundsCursor);
					else
						 SetCursor(trackingCursor);
	             			   
					if(x1 <= xmin) x1 = xmin+1; 
					if(x1 >= xmax) x1 = xmax-1;
					if(y1 <= ymin) y1 = ymin+1;
					if(y1 >= ymax) y1 = ymax-1;

					cur1DPlot->pointSelection.xScrn = x1;
	     	  
					InvalidateRect(hWnd,0,false);
					UpdateWindow(hWnd); // Make sure the zoom rectangle is drawn
				}
	      	         
				else if(msg.message == WM_LBUTTONDOWN && !button_pressed) // Record final coordinates and amplitude
				{
	      		x1 = LOWORD(msg.lParam);
					y1 = HIWORD(msg.lParam);
			  
				 // Check for out of bounds mouse movement			   
					if(x1 <= xmin || x1 >= xmax || y1 <= ymin || y1 >= ymax)
					{
						 MessageBeep(MB_OK);
						 continue;
					}

					cur1DPlot->pointSelection.xScrn = x1;
			  
					InvalidateRect(hWnd,0,false);
					UpdateWindow(hWnd); // Make sure the zoom rectangle is drawn
	             	      
					button_pressed = true;  
				}

				else if(msg.message == WM_LBUTTONUP && button_pressed) // Record final coordinates and amplitude
				{
					break;
				} 
			}
	   }
   }
   gBlockWaitCursor = false;

   SetCursor(LoadCursor(NULL,IDC_WAIT));
   ReleaseCapture();   

	cur1DPlot->pointSelection.xScrn = -1;

   //if (itfc->obj->hWnd)
   //{
   //   EnableWindow(itfc->obj->hWnd, false);
   //   RedrawWindow(itfc->obj->hWnd, NULL,NULL, RDW_UPDATENOW);
   //   EnableWindow(itfc->obj->hWnd, true);
   //   RedrawWindow(itfc->obj->hWnd, NULL, NULL, RDW_UPDATENOW);
   //   TextMessage("Redrawing button\n");
   //}

// Update the display to remove the cursor
   InvalidateRect(hWnd,0,false);
   UpdateWindow(hWnd);

	if(mode == "index")
   {               
      itfc->retVar[1].MakeAndSetFloat(cur1DPlot->pointSelection.xIndex); 
      itfc->nrRetValues = 1;
   }
	else if(mode == "value")
   {
      itfc->retVar[1].MakeAndSetFloat(cur1DPlot->pointSelection.xData); 
      itfc->nrRetValues = 1;
   }
   else
   {
      itfc->retVar[1].MakeAndSetFloat(cur1DPlot->pointSelection.xIndex); 
      itfc->retVar[2].MakeAndSetFloat(cur1DPlot->pointSelection.xData); 
      itfc->nrRetValues = 2;
   }

   return(OK);
}

/*****************************************************************************
*                 Process a plot class function call
*****************************************************************************/

short ProcessPlotClassReferences(Interface *itfc, Plot *pd, char* name, char *args)
{
   short r;
   CArg carg;
   short nrArgs = carg.Count(args);

   ToLowerCase(name);


   if(!pd)
   {
      ErrorMessage("Plot not defined");
      return(ERR);
   }

// --------- 1D plot functions ------------------
   if(pd->getDimension() == 1)
   {
      // Set plot current trace x-y data
      if(!strcmp(name,"plot"))
      {
         if(args)
         {
		    	MSG msg;
			//	while(pd->plotParent->isBusy())
			//		PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
				printf("In critical section CLASSREF INIT\n");

            EnterCriticalSection(&cs1DPlot);
            pd->plotParent->incCriticalSectionLevel();
            short id = static_cast<Plot1D*>(pd)->DrawPlot(itfc,args);
            LeaveCriticalSection(&cs1DPlot);
            pd->plotParent->decCriticalSectionLevel();
				printf("Leaving critical section CLASSREF INIT\n");

            if(id == ERR)
               return(ERR);
            itfc->retVar[1].MakeAndSetFloat(id);
            itfc->nrRetValues = 1;
            return(OK);
         }
         else
         {
            ErrorMessage("plot command needs arguments");
            return(ERR);
         }
      }
      // Paste from temp storage to plot
      else if(!strcmp(name,"pasteinto"))
      {
         Plot *svPlt = pd->plotParent->GetSavedPlot();
         svPlt->PasteInto(pd);
         pd->Invalidate();
         itfc->nrRetValues = 0;
         return(OK);
      }
      // Get plot current trace x-y data
      else if(!strcmp(name,"getdata"))
      {    
         CText modeTxt = "all";
         short mode = 1;
         if(ArgScan(itfc,args,0,"all/current","e","t",&modeTxt) < 0)
         {
            ErrorMessage("Expecting 0/1 arguments (all/current)");
            return(ERR);
         }
         if(modeTxt != "all")
             mode = 0;
         if(pd->GetData(itfc, mode) == ERR)
            return(ERR);
         return(OK);
      }
      // Set or get plot trace x border
      else if(!strcmp(name,"tracexborder"))
	   {
         float factor;

	   // Get arguments from user *************
		   if(ArgScan(itfc,args,1,"trace x border factor","e","f",&factor) < 0)
         {
            ErrorMessage("Expecting 1 argument (trace s-border factor)");
            return(ERR);
         }

         if(factor < 0 || factor >= 1)
	      {
	         ErrorMessage("invalid trace border factor value [0->1]");
	         return(ERR);
	      }
         pd->traceXBorderFactor = factor;
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
      // Set or get plot trace y border
	   else if(!strcmp(name,"traceyborder"))
	   {
         float factor;

	   // Get arguments from user *************
		   if(ArgScan(itfc,args,1,"trace y border factor","e","f",&factor) < 0)
         {
            ErrorMessage("Expecting 1 argument (trace y-border factor)");
            return(ERR);
         }

         if(factor < 0 || factor >= 1)
	      {
	         ErrorMessage("invalid trace border factor value [0->1]");
	         return(ERR);
	      }
         pd->traceYBorderFactor = factor;
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
      else if (!strcmp(name, "indicatorsize")) // Return the size of the lot indicator
      {
         long size;
         // Get arguments from user *************
         if (ArgScan(itfc, args, 1, "indicator size", "e", "l", &size) < 0)
         {
            ErrorMessage("Expecting 1 argument (indicator size)");
            return(ERR);
         }
         Plot1D* pr = dynamic_cast<Plot1D*>(pd);
         pr->indicatorSize = size;
         pd->DisplayAll(false);
         itfc->nrRetValues = 0;
         return(OK);
      }
      // Clear the plot
      else if(!strcmp(name,"clear"))
      {
			dynamic_cast<Plot1D*>(pd)->clearData();
         dynamic_cast<Plot1D*>(pd)->setOverRideAutoRange(false);
         MyInvalidateRect(pd->plotParent->hWnd,NULL,false);
         itfc->nrRetValues = 0;
         return(OK);
      }
      else if (!strcmp(name, "filtertrace"))
      {
         Plot1D* pr = dynamic_cast<Plot1D*>(pd);
         CText isFiltered;
         (pr->IsFiltered()) ? (isFiltered = "true") : (isFiltered = "false");
         r = ArgScan(itfc, args, 1, "true/false", "e", "t", &isFiltered);
         if (r == ERR)
            return(ERR);
         else if (r == -2)
            return(OK);

         if (isFiltered == "true")
         {
            pr->SetFiltered(true);
         }
         else
         {
            pr->SetFiltered(false);
         }
         itfc->nrRetValues = 0;
         pd->Invalidate();
         return OK;
      }
      // Display all trace data
      else if(!strcmp(name,"fullregion"))
      {
         Plot1D *p1d = dynamic_cast<Plot1D*>(pd);
         HWND hWnd = p1d->win;
	      HDC hdc = GetDC(hWnd);
		   p1d->setOverRideAutoRange(false);
         p1d->curXAxis()->setAutorange(true);
         p1d->curYAxis()->setAutorange(true); 
		   if (p1d->syncAxes())
		   {
			   p1d->otherYAxis()->setAutorange(true);	
		   }		
         p1d->resetZoomCount(); 
         p1d->HideSelectionRectangle(hdc);      
         p1d->ResetZoomPoint();
		   p1d->updateStatusWindowForZoom(hWnd);
         p1d->Invalidate();	
         ReleaseDC(hWnd,hdc);
         itfc->nrRetValues = 0;
         return(OK);
      }
      // Remove the specified trace
      else if(!strcmp(name,"rmtrace"))
      {
         long id;

         if((nrArgs = ArgScan(itfc,args,1,"trace ID","e","l",&id)) < 0)
         {
            if(nrArgs == ERR)
            {
               return(ERR);
            }
            else
            {
               ErrorMessage("Expecting 1 argument (trace ID)");
               return(ERR);
            }
         }

			itfc->nrRetValues = 0;
			if (OK != dynamic_cast<Plot1D*>(pd)->removeTrace(id))
			{
				ErrorMessage("trace ID '%ld' not found",id);
				return ERR;
			}
         return OK;
      }
      // Don't delete the plot traces when drawing next trace
      else if(!strcmp(name,"hold"))
      {
         CText hold = "on";
         if(ArgScan(itfc,args,0,"on/off","e","t",&hold) < 0)
         {
            ErrorMessage("Expecting 0/1 arguments (on/off)");
            return(ERR);
         } 

			WinData* parent = pd->plotParent->obj->winParent;
         if(hold == "on")
         {
				pd->setHold(true);
            parent->setToolBarItemCheck(pd->plotParent->toolbarName(),"hold",true);
            parent->setMenuItemCheck(pd->plotParent->menuName(),"hold",true);
         }
         else if(hold == "off")
         {
				pd->setHold(false);
            parent->setToolBarItemCheck(pd->plotParent->toolbarName(),"hold",false);
            parent->setMenuItemCheck(pd->plotParent->menuName(),"hold",false);
         }
         else // Toggles state
         {
            pd->setHold(!pd->getHold()); 
            parent->setToolBarItemCheck(pd->plotParent->toolbarName(),"hold",pd->getHold());
            parent->setMenuItemCheck(pd->plotParent->menuName(),"hold",pd->getHold());
         }
         itfc->nrRetValues = 0;
         return(OK);
      }
      else if (!strcmp(name,"showlines"))
      {
			Plot1D *pr = dynamic_cast<Plot1D*>(pd);
			CText showlines;
			(pr->showLines == true) ? (showlines	= "true") : (showlines	= "false");
			r = ArgScan(itfc,args,1,"true/false","e","t",&showlines);
			if(r == ERR)
				return(ERR); 
			else if(r == -2)
				return(OK);

			if(showlines == "true")
            pr->showLines = true;
         else
            pr->showLines = false;
			itfc->nrRetValues = 0;
         pd->Invalidate();	
         return OK;
      }
      else if (!strcmp(name,"showtext"))
      {
		   Plot1D *pr = dynamic_cast<Plot1D*>(pd);
			CText showtext;
			(pr->showText == true) ? (showtext	= "true") : (showtext	= "false");
			r = ArgScan(itfc,args,1,"true/false","e","t",&showtext);
			if(r == ERR)
				return(ERR); 
			else if(r == -2)
				return(OK);
			if(showtext == "true")
            pr->showText = true;
         else
            pr->showText = false;
			itfc->nrRetValues = 0;
         pd->Invalidate();	
         return OK;
      }
      else if (!strcmp(name,"showinsets"))
      {
		   Plot1D *pr = dynamic_cast<Plot1D*>(pd);
			CText showinsets;
			(pr->showInsets == true) ? (showinsets	= "true") : (showinsets	= "false");
			r = ArgScan(itfc,args,1,"true/false","e","t",&showinsets);
			if(r == ERR)
				return(ERR); 
			else if(r == -2)
				return(OK);
			if(showinsets == "true")
            pr->showInsets = true;
         else
            pr->showInsets = false;
			itfc->nrRetValues = 0;
         pd->Invalidate();	
         return OK;
      }
      else if (!strcmp(name,"showimag"))
      {
		   Plot1D *pr = dynamic_cast<Plot1D*>(pd);
			CText showimag;
			(pr->display1DComplex & SHOW_IMAGINARY) ? (showimag	= "true") : (showimag	= "false");
			r = ArgScan(itfc,args,1,"true/false","e","t",&showimag);
			if(r == ERR)
				return(ERR); 
			else if(r == -2)
				return(OK);

			if(showimag == "true")
         {
            pr->display1DComplex |= SHOW_IMAGINARY;
         }
         else
         {
            USHORT mask = ~(SHOW_IMAGINARY);
            pr->display1DComplex &= mask;
         }
			itfc->nrRetValues = 0;
         pd->Invalidate();	
         return OK;
      }
      else if (!strcmp(name,"showreal"))
      {
		   Plot1D *pr = dynamic_cast<Plot1D*>(pd);
			CText showreal;
			(pr->display1DComplex & SHOW_REAL) ? (showreal	= "true") : (showreal	= "false");
			r = ArgScan(itfc,args,1,"true/false","e","t",&showreal);
			if(r == ERR)
				return(ERR); 
			else if(r == -2)
				return(OK);

			if(showreal == "true")
         {
            pr->display1DComplex |= SHOW_REAL;
         }
         else
         {
            USHORT mask = ~SHOW_REAL;
            pr->display1DComplex &= mask;
         }
			itfc->nrRetValues = 0;
         pd->Invalidate();	
         return OK;
      }
		else if (!strcmp(name,"limitfunc"))
      {
		   Plot1D *pr = dynamic_cast<Plot1D*>(pd);
			CText limitfunc;
			(pr->limitfunc == true) ? (limitfunc	= "true") : (limitfunc	= "false");
			r = ArgScan(itfc,args,1,"true/false","e","t",&limitfunc);
			if(r == ERR)
				return(ERR); 
			else if(r == -2)
				return(OK);

			if(limitfunc == "true")
            pr->limitfunc = true;
         else
            pr->limitfunc = false;

			itfc->nrRetValues = 0;
         return OK;
      }
      // Get or set trace antialiasing status
		else if (!strcmp(name, "antialiasing1d") || !strcmp(name, "antialiasing"))
		{
			CText antialiasing = "undefined";
			if(ArgScan(itfc,args,0,"true/false","e","t",&antialiasing) < 0)
         {
            ErrorMessage("Expecting 0/1 argument (true/false)");
            return(ERR);
         }

			WinData* parent = pd->plotParent->obj->winParent;
			if (antialiasing == "true")
			{
				pd->setAntiAliasing(true);
            parent->setMenuItemCheck(pd->plotParent->menuName(),"antialiasing",false);
            pd->DisplayAll(false);  
				itfc->nrRetValues = 0;
			}
			else if (antialiasing == "false")
			{
				pd->setAntiAliasing(false);
            parent->setMenuItemCheck(pd->plotParent->menuName(),"antialiasing",false);
            pd->DisplayAll(false);  
				itfc->nrRetValues = 0;
			}
			else
			{
				if(pd->isAntiAliasing())
				{
					itfc->retVar[1].MakeAndSetString("true");
				}
				else
				{
					itfc->retVar[1].MakeAndSetString("false");
				}
				itfc->nrRetValues = 1;
			}
         return(OK);
      }
      // Get trace by index
      else if(!strcmp(name,"trace"))
      {
         short n;

         if(!args)
         {
            ErrorMessage("Trace command needs arguments");
            return(ERR);
         }
         if(ArgScan(itfc,args,1,"","e","d",&n) == 1)
         {
            Trace *dl = dynamic_cast<Plot1D*>(pd)->FindTraceByID(n);
            if(dl)
            {
               itfc->retVar[1].MakeClass(TRACE_CLASS,(void*)dl);
               itfc->nrRetValues = 1;
               return(OK);
            }
            else
            {
               ErrorMessage("Trace %hd not found",n);
               return(ERR);
            }
         }
         else
         {
            ErrorMessage("Invalid trace number %s",args);
            return(ERR);
         }
      }
      // Get the current trace for this plot
      else if(!strcmp(name,"curtrace"))
      {
         Trace *dl = dynamic_cast<Plot1D*>(pd)->curTrace();
         if(dl)
         {
            itfc->retVar[1].MakeClass(TRACE_CLASS,(void*)dl);
            itfc->nrRetValues = 1;
            return(OK);
         }
         else
         {
            ErrorMessage("Current trace not found");
            return(ERR);
         }
      }
      // Get inset by index
		else if (!strcmp(name,"inset") || !strcmp(name,"annotation"))
		{
			short n;
			if (!args)
			{
				ErrorMessage("Inset command needs an argument");
				return (ERR);
			}
			if (ArgScan(itfc, args, 1, "","e","d",&n) == 1)
			{
				Inset* inset = pd->findInsetByID(n);
				if (inset)
				{
					itfc->retVar[1].MakeClass(INSET_CLASS, (void*)inset);
					itfc->nrRetValues = 1;
					return(OK);
				}
				else
				{
					ErrorMessage("Invalid inset number %s",args);
					return(ERR);
				}
			}
		}
      // Get or set trace preferences
      else if(!strcmp(name,"tracepref"))
      {
         CArg carg;
         short nrArgs = carg.Count((char*)args);
         r = GetOrSetTraceParameters(itfc, &carg, nrArgs, 1, NULL, *(static_cast<Plot1D*>(pd)->getTracePar()),static_cast<Plot1D*>(pd));
         return(r);
      }
      // Get list of trace indices
      else if(!strcmp(name,"tracelist"))
      {
         int sz;
         float* ids = dynamic_cast<Plot1D*>(pd)->GetTraceIDs(&sz);
         itfc->retVar[1].MakeMatrix2DFromVector(ids,sz,1);
         delete [] ids;
         itfc->nrRetValues = 1;
         return(OK);
      }
      // Zoom plot
      else if(!strcmp(name,"zoom") ||!strcmp(name,"zoom1d")) 
      {    
         r = pd->Zoom(itfc,args);
         if(r == 0)
            itfc->nrRetValues = 0;
         else if(r == -2)
            return(OK);
         return(r);
      }
      // Get the current y-axis (left or right)
		else if (!strcmp(name,"currentaxis"))
		{
			CText currentaxis = "undefined";
			if(ArgScan(itfc,args,0,"left/right","e","t",&currentaxis) < 0)
         {
            ErrorMessage("Expecting 0/1 arguments (left/right)");
            return(ERR);
         }
			if ("left" == currentaxis)
			{
				pd->setCurYAxis(LEFT_VAXIS_SIDE);
				itfc->nrRetValues = 0;
			}
			else if ("right" == currentaxis)
			{
				pd->setCurYAxis(RIGHT_VAXIS_SIDE);
				itfc->nrRetValues = 0;
			}
			else
			{
				if (pd->currentVerticalAxis() == LEFT_VAXIS_SIDE)
					itfc->retVar[1].MakeAndSetString("left");	
				else
					itfc->retVar[1].MakeAndSetString("right");	
				itfc->nrRetValues = 1;
			}

			return OK;
		}
     // Synchronise left and right axis when panning
		else if (!strcmp(name,"syncaxes"))
		{
			CText synch = "undefined";
			if(ArgScan(itfc,args,0,"true/false","e","t",&synch) < 0)
         {
            ErrorMessage("Expecting 0/1 arguments (true/false)");
            return(ERR);
         }
			if ("true" == synch)
			{
				pd->setSyncAxes(true);
            pd->plotParent->obj->winParent->setMenuItemCheck(pd->plotParent->menuName(),"sync_axes",true);
				itfc->nrRetValues = 0;
			}
			else if ("false" == synch)
			{
				pd->setSyncAxes(false);
            pd->plotParent->obj->winParent->setMenuItemCheck(pd->plotParent->menuName(),"sync_axes",false);
				itfc->nrRetValues = 0;
			}
			else 
			{
				if (pd->syncAxes())
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");
				itfc->nrRetValues = 1;
			}
			return OK;
		}
      // Lock left and right grids
		else if (!strcmp(name,"lockgrid"))
		{
			CText lock = "undefined";
			if(ArgScan(itfc,args,0,"true/false","e","t",&lock) < 0)
         {
            ErrorMessage("Expecting 0/1 arguments (true/false)");
            return(ERR);
         }
			if ("true" == lock)
			{
				pd->lockGrid(true);
            pd->plotParent->obj->winParent->setMenuItemCheck(pd->plotParent->menuName(),"lock_grid",true);
				itfc->nrRetValues = 0;
			}
			else if ("false" == lock)
			{
				pd->lockGrid(false);
            pd->plotParent->obj->winParent->setMenuItemCheck(pd->plotParent->menuName(),"lock_grid",false);
				itfc->nrRetValues = 0;
			}
			else
			{
				if (pd->gridLocked())
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");
				itfc->nrRetValues = 1;
			}
			return OK;
		}

		else if (!strcmp(name,"load")) // Load a  file into a plot region
      {
         extern void GetExtension(char *file, char *extension);

         CText fileName;
         if(ArgScan(itfc,args,1,"","e","t",&fileName) < 0)
         {
            ErrorMessage("Invalid argument to load command");
            return(ERR);
         }

         char ext[MAX_STR];
         GetExtension(fileName.Str(),ext);
         if(strcmp(ext,"pt1"))
         {
            ErrorMessage("Invalid extension for 1D plot");
            return(ERR);
         }
         PlotWindow *pw;
         pw = pd->plotParent;
         pd->makeCurrentPlot();
			pd->makeCurrentDimensionalPlot();
         short bak = pw->getPlotInsertMode();    
         pw->setPlotInsertMode(ID_REPLACE_PLOTS);   
         pw->LoadPlots(".",fileName.Str());
         pw->setPlotInsertMode(bak);    
			InvalidateRect(pw->hWnd,NULL,false); 
         itfc->nrRetValues = 0;

         return(OK);
      }
		else if (!strcmp(name,"save")) // Save a plot region to a file
      {
         extern void GetExtension(char *file, char *extension);

         CText fileName;
         if(ArgScan(itfc,args,1,"","e","t",&fileName) < 0)
         {
            ErrorMessage("Invalid argument to save command");
            return(ERR);
         }

         char ext[MAX_STR];
         GetExtension(fileName.Str(),ext);

         if(!strcmp(ext,"1d"))
			{
            Plot1D *pt1d = dynamic_cast<Plot1D*>(pd);
				return(pt1d->SaveData(".",fileName.Str()));
			}

         if(strcmp(ext,"pt1"))
         {
            ErrorMessage("Invalid extension for 1D plot");
            return(ERR);
         }
         FILE *fp;
	      if(!(fp = fopen(fileName.Str(),"wb")))
	      {
		      ErrorMessage("Can't save file '%s'",fileName.Str());
		      return(ERR);
	      }

	      long ti = 'PROS'; fwrite(&ti,4,1,fp);   // Owner
	      ti = 'PL1D'; fwrite(&ti,4,1,fp);   // File type
         short rowCols = 1;
		   fwrite(&rowCols,sizeof(short),1,fp);   // Number of plot region rows
		   fwrite(&rowCols,sizeof(short),1,fp);   // Number of plot region cols

			pd->save(fp);
			pd->SaveV3_8Info(fp);
		   fclose(fp);

         pd->setFileName(fileName.Str());
         CText filePath;
         GetCurrentDirectory(filePath);
         pd->setFilePath(filePath.Str());
               
	
         itfc->nrRetValues = 0;

         return(OK);
      }

      // Show or hide the plot legend
		else if (!strcmp(name,"showlegend"))
		{			
			CText show = "undefined";

			if(ArgScan(itfc,args,0,"true/false","e","t",&show) < 0)
         {
            ErrorMessage("Expecting 0/1 arguments (true/false)");
            return(ERR); 
         }
			if ("true" == show)
			{
				pd->showLegend(true);
            pd->plotParent->obj->winParent->setMenuItemCheck(pd->plotParent->menuName(),"show_legend",true);

				itfc->nrRetValues = 0;
			}
			else if ("false" == show)
			{
				pd->showLegend(false);
            pd->plotParent->obj->winParent->setMenuItemCheck(pd->plotParent->menuName(),"show_legend",false);

				itfc->nrRetValues = 0;
			}
			else
			{
				if (pd->legendIsVisible())
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");
				itfc->nrRetValues = 1;
			}
			return OK;
		}
   }

// --------- 2D plot functions ------------------

   else if(pd->getDimension() == 2)
   {
      // Get image data
      if(!strcmp(name,"getdata"))
      {
         if(args)
         {
            short mode = 1;
            CText modeStr = "all";
			   if(ArgScan(itfc,args,0,"all/current","e","t",&modeStr) < 0)
            {
               ErrorMessage("Expecting 0/1 arguments (all/current)");
               return(ERR); 
            }
            if(modeStr == "all") 
               mode = 1;
            else
               mode = 0;
            if(pd->GetData(itfc, mode))
               return(ERR);
            return(OK);
         }
         else
         {
            ErrorMessage("getdata command needs arguments");
            return(ERR);
         }
      }
      // Get basic information about image - size and data type (0 real 1 complex)
      else if(!strcmp(name,"getinfo"))
      {
         Plot2D *p2d = dynamic_cast<Plot2D*>(pd);

         int width = p2d->matWidth();
         int height = p2d->matHeight();
         int isComplex = (p2d->cmat() != 0);
         itfc->retVar[1].MakeAndSetFloat(width);
         itfc->retVar[2].MakeAndSetFloat(height);
         itfc->retVar[3].MakeAndSetFloat(isComplex);
         itfc->nrRetValues = 3;
         return(OK);
      }
		else if (!strcmp(name,"limitfunc"))
      {
			CText limitfunc = "false";
			if(ArgScan(itfc,args,0,"true/false","e","t",&limitfunc) == ERR)
         {
            ErrorMessage("Expecting 0/1 argument (true/false)");
            return(ERR);
         }

         Plot2D *pr = dynamic_cast<Plot2D*>(pd);
			if(limitfunc == "true")
         {
            pr->limitfunc = true;
         }
         else
         {
            pr->limitfunc = false;
         }
			itfc->nrRetValues = 0;
         return OK;
      }
      // Draw image to plot
      else if(!strcmp(name,"image"))
      {
         if(args)
         {
				MSG msg;
            short r = OK;

            while(pd->plotParent->isBusy())
					PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

            EnterCriticalSection(&cs1DPlot);

            if(!pd->plotParent->isBusy())
	         {
		         pd->plotParent->setBusy(true);
               LeaveCriticalSection(&cs1DPlot);     
               r = dynamic_cast<Plot2D*>(pd)->Draw2DImage(itfc,args);
               pd->plotParent->setBusyWithCriticalSection(false);
            }
            else
		         LeaveCriticalSection(&cs1DPlot);


            itfc->nrRetValues = 0;
            return(r);
         }
         else
         {
            ErrorMessage("Image command needs arguments");
            return(ERR);
         }
      }
      // Clear plot data
      else if(!strcmp(name,"clear"))
      {
         pd->clearData();
         pd->setOverRideAutoRange(false);
         pd->removeLines();
         pd->removeInsets();
         pd->removeTexts();
         pd->DisplayAll(false); 

         itfc->nrRetValues = 0;
         return(OK);
      }
      // Set or get image color-map
      else if(!strcmp(name,"cmap"))
      {
         Variable cmap;
         Plot2D *p2d = dynamic_cast<Plot2D*>(pd);

         if(!args || args[0] == '\0')
         {
            if(p2d->colorMapLength() > 0 && p2d->colorMap() != NULL)
            {
               itfc->retVar[1].MakeAndLoadMatrix2D(p2d->colorMap(),3,p2d->colorMapLength());
               itfc->nrRetValues = 1;
            }
            else
            {
               ErrorMessage("No colormap defined");
               return(ERR);
            }
            return(OK);
         }
         else
         {
            if(ArgScan(itfc,args,1,"cmap name","e","v",&cmap) < 0)
            {
               ErrorMessage("Expecting 1 argument (colormap name)");
               return(ERR); 
            }
            p2d->SetColorMap(&cmap);
            pd->DisplayAll(false); 
            itfc->nrRetValues = 0;
            return(OK);
         }
      }
      // Draw contours or return contour levels
      else if(!strcmp(name,"contour"))
      {
         short contMode = DISPLAY_CONTOURS;
         long nr;
         Plot2D *p2d;
         Variable levelVar;
         Variable fixedColorVar;
         short fixedLevels = 0;

         float *levels = NULL;

         p2d = dynamic_cast<Plot2D*>(pd);

         if(!args || args[0] == '\0')
         {
            nr = p2d->nrContourLevels;
            contMode = p2d->drawMode();
            levels = p2d->contourLevels;
            if(levels)
               levelVar.MakeMatrix2DFromVector(levels,nr,1);
            else
               levelVar.MakeAndSetFloat(nr);
            COLORREF col = p2d->fixedContourColor();
            float *colorf = new float[3];
            colorf[0] = GetRValue(col);
            colorf[1] = GetGValue(col);
            colorf[2] = GetBValue(col);
            fixedColorVar.MakeMatrix2DFromVector(colorf,3,1);
            delete [] colorf;
            fixedLevels = (float)p2d->useFixedContourColor();
     
            nr = p2d->nrContourLevels;
            contMode = p2d->drawMode();
            levels = p2d->contourLevels;
            if(levels)
               levelVar.MakeMatrix2DFromVector(levels,nr,1);
            else
               levelVar.MakeAndSetFloat(nr);

            itfc->retVar[1].CopyWithAlias(&levelVar);
            itfc->retVar[2].MakeAndSetFloat(contMode);
            itfc->retVar[3].FullCopy(&fixedColorVar);
            itfc->retVar[4].MakeAndSetFloat(fixedLevels);
            itfc->nrRetValues = 4;
            fixedColorVar.FreeData();
            return(OK);
         }
         else
         {
            if(ArgScan(itfc,args,1,"number/levels, [[mode], fixed_color, use_fixed_levels]","eeee","vdvd",&levelVar,&contMode,&fixedColorVar,&fixedLevels) < 0)
            {
               ErrorMessage("Expecting 1-4 arguments");
               return(ERR); 
            }
                 
         // Get levels
            if(levelVar.GetType() == MATRIX2D && levelVar.GetDimY() == 1)
            {
               levels = new float[levelVar.GetDimX()];
               for(int x = 0; x < levelVar.GetDimX(); x++)
                  levels[x] = levelVar.GetMatrix2D()[0][x];
               nr = levelVar.GetDimX();
            }
            else if(levelVar.GetType() == FLOAT32)
            {
               nr = nint(levelVar.GetReal());
            }
            else
            {
               ErrorMessage("Invalid first argument, should be integer or level array");
               return(ERR);
            }

            p2d->setUseFixedContourColor(false);

         // Get fixed color
            if(nrArgs >= 3)
            {
               if(fixedColorVar.GetType() == MATRIX2D && fixedColorVar.GetDimX() == 3 && fixedColorVar.GetDimY() == 1 && fixedLevels >= 0)
               {
                  float *fcol = fixedColorVar.GetMatrix2D()[0];
                  COLORREF col = RGB(nint(fcol[0]),nint(fcol[1]),nint(fcol[2]));
                  p2d->setFixedContourColor(col);
                  p2d->setUseFixedContourColor(fixedLevels>0);
               }
               else
               {
                  ErrorMessage("Invalid fixed contour color");
                  return(ERR);
               }
            }

         // Set mode
            p2d->setDrawMode(contMode);
            
         // Set levels to plot *********************
            p2d->nrContourLevels = nr;

         // Set contour levels
            if(p2d->contourLevels)
               delete [] p2d->contourLevels;
            p2d->contourLevels = levels;

         // Make sure window is redrawn *********************** 
            pd->DisplayAll(false);  
            itfc->nrRetValues = 0;
            return(OK);
         }
      }
      // Draw contours or return contour levels
      else if (!strcmp(name, "contourlinewidth"))
      {
         float lw;
 
         if (ArgScan(itfc, args, 1, "", "e", "f", &lw) == 1)
         {
            Plot2D* p2d;

            p2d = dynamic_cast<Plot2D*>(pd);
            p2d->contourLineWidth = lw;
            // Make sure window is redrawn *********************** 
            pd->DisplayAll(false);
            itfc->nrRetValues = 0;
            return(OK);
         }
         else
         {
            ErrorMessage("contourlinewidth command needs an argument");
            return (ERR);
         }
      }
      // Set data mapping
		else if(!strcmp(name,"datamapping"))
		{
         Plot2D *p2d = dynamic_cast<Plot2D*>(pd);
         if(!args || args[0] == '\0')
			{
			   if(p2d->dataMapping == LINEAR_MAPPING)
		         itfc->retVar[1].MakeAndSetString("linear");
				else
					itfc->retVar[1].MakeAndSetString("log");
				itfc->nrRetValues = 1;
			}
			else
			{		
				CText mapping;
				if(ArgScan(itfc,args,1,"mapping","e","t",&mapping) < 0)
            {
               ErrorMessage("Expecting 1 argument (log/lin)");
               return(ERR); 
            }
				if(mapping == "linear" || mapping == "lin")
					p2d->dataMapping = LINEAR_MAPPING;
				else if(mapping == "logarithmic" || mapping == "log")
					p2d->dataMapping = LOG_MAPPING;
				pd->DisplayAll(false);  
            itfc->nrRetValues = 0;
			}
			return(OK);
		}
      // Get or set image color scale mapping range
      else if(!strcmp(name,"imagerange"))
      {
      //   Variable colorVar;

         if(!args || args[0] == '\0')
         {
            itfc->retVar[1].MakeAndSetFloat(static_cast<Plot2D*>(pd)->minVal());
            itfc->retVar[2].MakeAndSetFloat(static_cast<Plot2D*>(pd)->maxVal());
            itfc->nrRetValues = 2;
            return(OK);
         }
         else
         {
            float minV,maxV;
            if(ArgScan(itfc,args,2,"min,max","ee","ff",&minV,&maxV) < 0)
            {
               ErrorMessage("Expecting 2 arguments (minValue,maxValue)");
               return(ERR); 
            }
      
            if(minV > maxV)
            {
               ErrorMessage("Invalid range");
               return(ERR);
            }

         // Set new range
            static_cast<Plot2D*>(pd)->setMinVal(minV);
            static_cast<Plot2D*>(pd)->setMaxVal(maxV);
            pd->DisplayAll(false);  
            itfc->nrRetValues = 0;
            return(OK);
         }
      }
      // get image data range
      else if(!strcmp(name,"datarange"))
      {
         CText mode = "full";

         if(ArgScan(itfc,args,0,"full/visible","e","t",&mode) < 0)
         {
            ErrorMessage("Expecting 0 or 1 argument (full/visble)");
            return(ERR); 
         }

         if(mode == "full")
         {
            float minVal,maxVal;
            static_cast<Plot2D*>(pd)->FindFullMatrixRange(minVal,maxVal);	  
            itfc->retVar[1].MakeAndSetFloat(minVal);
            itfc->retVar[2].MakeAndSetFloat(maxVal);
            itfc->nrRetValues = 2;
            return(OK);
         }
         else if(mode == "visible")
         {
            float minVal,maxVal;
            static_cast<Plot2D*>(pd)->FindMatrixRange(minVal,maxVal);	  
            itfc->retVar[1].MakeAndSetFloat(minVal);
            itfc->retVar[2].MakeAndSetFloat(maxVal);
            itfc->nrRetValues = 2;
            return(OK);
         }
         else
         {
            ErrorMessage("Invalid data range mode (full/visible)");
            return(ERR);
         }
      }

		else if (!strcmp(name,"load")) // Load a  file into a plot region
      {
         extern void GetExtension(char *file, char *extension);

         CText fileName;
         if(ArgScan(itfc,args,1,"","e","t",&fileName) < 0)
         {
            ErrorMessage("Invalid argument to load command");
            return(ERR);
         }

         char ext[MAX_STR];
         GetExtension(fileName.Str(),ext);
         if(strcmp(ext,"pt2"))
         {
            ErrorMessage("Invalid extension for 2D plot");
            return(ERR);
         }

         PlotWindow *pw;
         pw = pd->plotParent;
         pd->makeCurrentPlot();
			pd->makeCurrentDimensionalPlot();
         short bak = pw->getPlotInsertMode();    
         pw->setPlotInsertMode(ID_REPLACE_PLOTS);    
         pw->LoadPlots(".",fileName.Str());
         pw->setPlotInsertMode(bak);  
			InvalidateRect(pw->hWnd,NULL,false); 
         itfc->nrRetValues = 0;

         return(OK);
      }

		else if (!strcmp(name,"save")) // Save a image region to a file
      {
         extern void GetExtension(char *file, char *extension);

         CText fileName;
         if(ArgScan(itfc,args,1,"","e","t",&fileName) < 0)
         {
            ErrorMessage("Invalid argument to save command");
            return(ERR);
         }

         char ext[MAX_STR];
         GetExtension(fileName.Str(),ext);

         // Save as simple 2D data file
         if(!strcmp(ext,"2d"))
			{
            Plot2D *pt2d = dynamic_cast<Plot2D*>(pd);
				return(pt2d->SaveData(".",fileName.Str()));
			}
         // Save as 2D plot file
         if(strcmp(ext,"pt2"))
         {
            ErrorMessage("Invalid extension for 2D image");
            return(ERR);
         }

         FILE *fp;
	      if(!(fp = fopen(fileName.Str(),"wb")))
	      {
		      ErrorMessage("Can't save file '%s'",fileName.Str());
		      return(ERR);
	      }

	      long ti = 'PROS'; fwrite(&ti,4,1,fp);   // Owner
	      ti = 'PL2D'; fwrite(&ti,4,1,fp);   // File type
         short rowCols = 1;
		   fwrite(&rowCols,sizeof(short),1,fp);   // Number of plot region rows
		   fwrite(&rowCols,sizeof(short),1,fp);   // Number of plot region cols

			pd->save(fp);
		   fclose(fp);

         pd->setFileName(fileName.Str());
         CText pathName;
         GetCurrentDirectory(pathName);
         pd->setFilePath(pathName.Str());
	
         itfc->nrRetValues = 0;

         return(OK);
      }
     // Set plot color scale visibility
      else if(!strcmp(name,"showcmap"))
      {
         CText show = "false";

         if(static_cast<Plot2D*>(pd)->displayColorScale())
            show = "true";

      // Get arguments from user *************
         r = ArgScan(itfc,args,1,"show?","e","t",&show);

         if(r == ERR)
            return(ERR); 

         if(r == -2)
            return(OK);

         if(r == 0)
         {
            itfc->nrRetValues = 1;

				if(show == "true")
					itfc->retVar[1].MakeAndSetString("true");
				else
					itfc->retVar[1].MakeAndSetString("false");

            return(OK);
         }

         if(show == "true")
            static_cast<Plot2D*>(pd)->setDisplayColorScale(true);
         else
            static_cast<Plot2D*>(pd)->setDisplayColorScale(false);

         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
      // Zoom image
      else if(!strcmp(name,"zoom") ||!strcmp(name,"zoom2d")) 
      {    
         r = pd->Zoom(itfc,args);
         if(r == 0)
            itfc->nrRetValues = 0;
         else if(r == -2)
            return(OK);
         return(r);
      }
   }

// --------- General functions (1D or 2D) ------------------

   // Add an annotation to a plot
	if (!strcmp(name,"addannotation"))
	{
		CText text;
		int width = 0;
		int left = 0;
		int top = 0;
		if(ArgScan(itfc,args,0,"text, [width, left, top]","eeee","tddd",&text,&width,&left,&top) < 0)
			return(ERR); 
		pd->addInset(Inset::makeInset(ANNOTATION, string(text.Str()), width, left, top));
		pd->DisplayAll(false);
		itfc->nrRetValues = 0;
		return(OK);			
	}


   // Add a line to a plot
	else if (!strcmp(name,"addline"))
	{
		CText text;
      Variable colVec;
      COLORREF color = RGB(255,0,0);
      CText styleTxt = "dash";
      LineStyle style = LineStyle::SOLID;
      short thickness = 1;

		float x0 = 0, x1 = 0, y0 = 0, y1 = 0;
      short units = 0;
		int n = ArgScan(itfc,args,4,"x0, y0, x1, y1, color, thickness, style, units","eeeeeeee","ffffvdtd",&x0,&y0,&x1,&y1,&colVec,&thickness,&styleTxt,&units);
      if(n < 0)
      {
         ErrorMessage("Expecting 4 arguments");
         return(ERR);
      }
      if(n > 4)
      {
         if(colVec.GetType() == MATRIX2D && colVec.GetDimX() == 3 && colVec.GetDimY() == 1)
         {
            float **colMat = colVec.GetMatrix2D();
            color = RGB(colMat[0][0],colMat[0][1],colMat[0][2]);
         }
         else
         {
            ErrorMessage("invalid color code");
            return(ERR);
         }
      }
      if(n > 6)
      {
         if(styleTxt == "dashes")           style = LineStyle::DASH;
         else if(styleTxt == "dots")        style = LineStyle::DOT;
         else if(styleTxt == "solid")       style = LineStyle::SOLID;
         else if(styleTxt == "dashdot")     style = LineStyle::DASHDOT;
         else if(styleTxt == "dashdotdot")  style = LineStyle::DASHDOTDOT;
         else
         {
            ErrorMessage("invalid style (dashes,dots,dashdot,dashdotdot,solid");
            return(ERR);
         }
      }

      PlotLine* ln = new PlotLine(x0,y0,x1,y1,color,thickness,style,units);
      pd->lines_.push_back(ln);
		pd->DisplayAll(false);
		itfc->nrRetValues = 0;
		return(OK);		
	}

	else if (!strcmp(name,"addlines"))
	{
		CText text;
      Variable vec;
      Variable colVec;
      COLORREF color = RGB(255,0,0);
      CText styleTxt = "dash";
      LineStyle style = LineStyle::SOLID;
      short thickness = 1;
      float **lineVectors;
		int units=0;

		int n = ArgScan(itfc,args,1,"vector, color, width, style","eeeee","vvdtd",&vec,&colVec,&thickness,&styleTxt,&units);
      if(n < 0)
      {
         ErrorMessage("Expecting 1-5 arguments");
         return(ERR);
      }

      if(nrArgs == 1 && vec.GetType() != STRUCTURE_ARRAY)
      {
         ErrorMessage("invalid line structure array");
         return(ERR);
      }

      if(nrArgs > 1)  // Multi-argument input
      {
         if(vec.GetType() == MATRIX2D && (vec.GetDimX() == 4 || vec.GetDimX() == 7) && vec.GetDimY() >= 1)
         {
            lineVectors = vec.GetMatrix2D();
         }
         else
         {
            ErrorMessage("invalid line vector");
            return(ERR);
         }

         // Get the color vector
         if(n > 1)
         {
            if(colVec.GetType() == MATRIX2D && colVec.GetDimX() == 3 && colVec.GetDimY() == 1)
            {
               float **colMat = colVec.GetMatrix2D();
               color = RGB(colMat[0][0],colMat[0][1],colMat[0][2]);
            }
            else
            {
               ErrorMessage("invalid color code");
               return(ERR);
            }
         }

         // Get the style
         if(n > 3)
         {
            if(styleTxt == "dashes")           style = LineStyle::DASH;
            else if(styleTxt == "dots")        style = LineStyle::DOT;
            else if(styleTxt == "solid")       style = LineStyle::SOLID;
            else if(styleTxt == "dashdot")     style = LineStyle::DASHDOT;
            else if(styleTxt == "dashdotdot")  style = LineStyle::DASHDOTDOT;
            else
            {
               ErrorMessage("invalid style (dashes,dots,dashdot,dashdotdot,solid");
               return(ERR);
            }
         }
      }

      if(nrArgs == 1)
      {
         Variable* strucArray = (Variable*)vec.GetData();
         int size = vec.GetDimX();
         Variable *struc,*value;
         float x0,y0,x1,y1;
         float *colMat;
         CText styleTxt;
			short units;


         for(int i = 0; i < size; i++)
         {
            struc = strucArray[i].GetStruct();

            try
            {
               (value = struc->GetStructVariable("x0"))         ? (x0 = value->GetReal())               : throw("x0"); 
               (value = struc->GetStructVariable("x1"))         ? (x1 = value->GetReal())               : throw("x1"); 
               (value = struc->GetStructVariable("y0"))         ? (y0 = value->GetReal())               : throw("y0"); 
               (value = struc->GetStructVariable("y1"))         ? (y1 = value->GetReal())               : throw("y1"); 
               (value = struc->GetStructVariable("thickness"))  ? (thickness = nint(value->GetReal()))  : throw("thickness"); 
               (value = struc->GetStructVariable("color"))      ? (colMat = value->GetMatrix2D()[0])    : throw("color"); 
               (value = struc->GetStructVariable("style"))      ? (styleTxt = value->GetString())       : throw("style"); 
               (value = struc->GetStructVariable("units"))      ? (units = short(value->GetReal()))     : throw("units"); 
            }
            catch(const char* errStr)
            {
                ErrorMessage("Missing structure parameter '%s'",errStr);
                return(ERR);
            }
            if(styleTxt      == "dashes")      style = LineStyle::DASH;
            else if(styleTxt == "dots")        style = LineStyle::DOT;
            else if(styleTxt == "solid")       style = LineStyle::SOLID;
            else if(styleTxt == "dashdot")     style = LineStyle::DASHDOT;
            else if(styleTxt == "dashdotdot")  style = LineStyle::DASHDOTDOT;
            else
            {
               ErrorMessage("invalid style (dashes,dots,dashdot,dashdotdot,solid");
               return(ERR);
            }
            color = RGB(colMat[0],colMat[1],colMat[2]);
            PlotLine* ln = new PlotLine(x0,y0,x1,y1,color,thickness,style,units);
            pd->lines_.push_back(ln);
         }
      }
      else
      {
         if(vec.GetDimX() == 4)
         {
            // Add the lines
            for(int i = 0; i < vec.GetDimY(); i++)
            {
               PlotLine* ln = new PlotLine(lineVectors[i][0],lineVectors[i][1],lineVectors[i][2],lineVectors[i][3],color,thickness,style,units);
               pd->lines_.push_back(ln);
            }
         }
         else if(vec.GetDimX() == 7)
         {
            for(int i = 0; i < vec.GetDimY(); i++)
            {
               PlotLine* ln = new PlotLine(lineVectors[i][0],lineVectors[i][1],lineVectors[i][2],lineVectors[i][3],lineVectors[i][4],lineVectors[i][5],lineVectors[i][6]);
               pd->lines_.push_back(ln);
            }

         }
      }
	   pd->DisplayAll(false);

	   itfc->nrRetValues = 0;
	   return(OK);	
	}

   else if (!strcmp(name,"getlines")) // Extract any lines and return in a structure array
	{

      int nrLines = pd->lines_.size();
      if(nrLines == 0)
      {
         itfc->retVar[1].MakeNullVar();
	      itfc->nrRetValues = 1;
	      return(OK);	
      }

      itfc->retVar[1].MakeStructArray(nrLines);
      Variable *sOut = itfc->retVar[1].GetStruct();
      CText style = "solid";

      for(int i = 0; i < nrLines; i++)
      {
         PlotLine *ln = pd->lines_[i];
         Variable *s = sOut[i].GetStruct();      
         s->AddToStructure("x0",ln->x0);
         s->AddToStructure("x1",ln->x1);
         s->AddToStructure("y0",ln->y0);
         s->AddToStructure("y1",ln->y1);
         s->AddToStructure("thickness",(float)ln->thickness);
         if(ln->style == LineStyle::DASH) style = "dashes";
         else if(ln->style == LineStyle::DOT) style = "dots";
         else if(ln->style == LineStyle::SOLID) style = "solid";
         else if(ln->style == LineStyle::DASHDOT) style = "dashdot";
         else if(ln->style == LineStyle::DASHDOTDOT) style = "dashdotdot";
         s->AddToStructure("style",style.Str());
         s->AddToStructure("color",ln->color);
         s->AddToStructure("units",(float)ln->units);
      }
	   itfc->nrRetValues = 1;
	   return(OK);	
	}

   else if (!strcmp(name,"gettext")) // Extract any text and return in 6 column matrix and a list
	{
      float xPos,yPos;
      PlotText *txt;

      if((nrArgs = ArgScan(itfc,args,0,"[[x,] y]","ee","ff",&xPos,&yPos)) < 0)
      {
         ErrorMessage("Expecting 0-2 arguments");
         return(ERR);
      }

      int nrStrings = pd->text_.size();
      if(nrStrings == 0)
      {
         itfc->retVar[1].MakeNullVar();
	      itfc->nrRetValues = 1;
	      return(OK);	
      }

      if(nrArgs == 0)
      {
         itfc->retVar[1].MakeStructArray(nrStrings);
         Variable *sOut = itfc->retVar[1].GetStruct();

         for(int i = 0; i < nrStrings; i++)
         {
            txt = pd->text_[i];
            Variable *s = sOut[i].GetStruct();
            s->AddToStructure("x",txt->x);
            s->AddToStructure("y",txt->y);
            s->AddToStructure("text",txt->txt.Str());
            s->AddToStructure("font",txt->font.Str());
            s->AddToStructure("color",txt->color);
            s->AddToStructure("xw",txt->xw);
            s->AddToStructure("xh",txt->xh);
            s->AddToStructure("yw",txt->yw);
            s->AddToStructure("yh",txt->yh);
            s->AddToStructure("angle",txt->angle);
            s->AddToStructure("size",(float)txt->size);
				char unitsStr[3];
				unitsStr[0] = (char)(((txt->units & 0xF0)>>4)+48);
				unitsStr[1] = (char)((txt->units & 0x0F) + 48);
				unitsStr[2] = '\0';
            s->AddToStructure("units",unitsStr);
          //  s->AddToStructure("units",(txt->units) ? "pixels" : "user");
         }
	      itfc->nrRetValues = 1;
	      return(OK);	
      }

      if(nrArgs == 1) // Find text closest to xPos
      {
         int pos = -1;
         float dis = 1e30;
         for(int i = 0; i < nrStrings; i++)
         {
            PlotText *txt = pd->text_[i];
            float x = txt->x;
            float r = fabs(x-xPos);
            if(r < dis)
            {
               pos = i;
               dis = r;
            }
         }
         if(pos == -1)
            return(ERR);
         txt = pd->text_[pos];
      }
      else if(nrArgs == 2) // Find text closest to (xPos,yPos)
      {
         int pos = -1;
         float dis = 1e30;
         for(int i = 0; i < pd->text_.size(); i++)
         {
            PlotText *txt = pd->text_[i];
            float x = txt->x;
            float y = txt->y;
            float r = sqrt((x-xPos)*(x-xPos) + (y-yPos)*(y-yPos));
            if(r < dis)
            {
               pos = i;
               dis = r;
            }
         }
         if(pos == -1)
            return(ERR);
         txt = pd->text_[pos];
      }

      itfc->retVar[1].MakeStruct();
      Variable *s = itfc->retVar[1].GetStruct();

      s->AddToStructure("x",txt->x);
      s->AddToStructure("y",txt->y);
      s->AddToStructure("text",txt->txt.Str());
      s->AddToStructure("font",txt->font.Str());
      s->AddToStructure("color",txt->color);
      s->AddToStructure("xw",txt->xw);
      s->AddToStructure("xh",txt->xh);
      s->AddToStructure("yw",txt->yw);
      s->AddToStructure("yh",txt->yh);
      s->AddToStructure("angle",txt->angle);
      s->AddToStructure("size",(float)txt->size);
      s->AddToStructure("units",(txt->units) ? "pixels" : "user");
      
      itfc->nrRetValues = 1;
	   return(OK);	
	}

  // Add text to a plot 
	else if (!strcmp(name,"addtext"))
	{
		CText text;
      Variable posVar;
      Variable shiftVar;
      Variable textVar;
      CText fontName = "Arial";
      int fontSize = 8;
      Variable fontColorVar;
      float **fontColorVec;
      COLORREF fontColor;
      CText fontStyle = "regular";
      float **posVector;
      float **shiftVector;
      float **shiftMatrix;
      char **textVector;
      CText units = "user";
      float fontAngle = 0;
      short n;

		if((n = ArgScan(itfc,args,1,"pos/info[, text, textShift, font, fontSize, fontAngle, style, color, [units]]","eeeeeeeee","vvvtdftvt",&posVar,&textVar, &shiftVar,&fontName,&fontSize,&fontAngle,&fontStyle,&fontColorVar,&units)) < 0)
      {
         ErrorMessage("Expecting 1-9 arguments");
         return(ERR);
      }

      if(nrArgs == 1 && posVar.GetType() != STRUCTURE_ARRAY)
      {
         ErrorMessage("invalid text structure array");
         return(ERR);
      }

      if(nrArgs > 1)  // Multi-argument input
      {

       // Get the position vector
         if(posVar.GetType() == MATRIX2D && posVar.GetDimX() == 2 && posVar.GetDimY() >= 1)
         {
            posVector = posVar.GetMatrix2D();
         }
         else
         {
            ErrorMessage("invalid position vector");
            return(ERR);
         }

       // Get the text vector
         if(textVar.GetType() == LIST && textVar.GetDimY() >= 1)
         {
            textVector = textVar.GetList();
         }
         else if(textVar.GetType() != UNQUOTED_STRING)
         {
            ErrorMessage("invalid text list");
            return(ERR);
         }

        // Get the text shift matrix
         if(nrArgs >= 8)
         {
            if(shiftVar.GetType() == MATRIX2D && shiftVar.GetDimX() == 2 && shiftVar.GetDimY() == 2)
            {
               shiftMatrix = shiftVar.GetMatrix2D();
            }
            else
            {
               ErrorMessage("invalid text shift matrix (should be 2*2)");
               return(ERR);
            }
         }
         else
         {
            shiftMatrix = MakeMatrix2D(2,2);
         }
      }

      if(nrArgs == 1)
      {
         Variable* strucArray = (Variable*)posVar.GetData();
         int size = posVar.GetDimX();
         Variable *struc;
         float xpos,ypos;
         shiftMatrix = MakeMatrix2D(2,2);

         for(int i = 0; i < size; i++)
         {
            struc = strucArray[i].GetStruct();
            Variable *value;
            try
            {
               (value = struc->GetStructVariable("x"))     ? (xpos = value->GetReal())              : throw("x"); 
               (value = struc->GetStructVariable("y"))     ? (ypos = value->GetReal())              : throw("y");
               (value = struc->GetStructVariable("angle")) ? (fontAngle = value->GetReal())         : throw("angle");
               (value = struc->GetStructVariable("color")) ? (fontColorVec = value->GetMatrix2D())  : throw("color"); 
               (value = struc->GetStructVariable("text"))  ? (text = value->GetString())            : throw("text");
               (value = struc->GetStructVariable("font"))  ? (fontName = value->GetString())        : throw("font");
               (value = struc->GetStructVariable("units")) ? (units = value->GetString())           : throw("units");
               (value = struc->GetStructVariable("xw"))    ? (shiftMatrix[0][0] = value->GetReal()) : throw("xw");
               (value = struc->GetStructVariable("xh"))    ? (shiftMatrix[0][1] = value->GetReal()) : throw("xh"); 
               (value = struc->GetStructVariable("yw"))    ? (shiftMatrix[1][0] = value->GetReal()) : throw("yw");
               (value = struc->GetStructVariable("yh"))    ? (shiftMatrix[1][1] = value->GetReal()) : throw("yh"); 
               (value = struc->GetStructVariable("size"))  ? (fontSize = nint(value->GetReal()))    : throw("size");
            }
            catch(const char* errStr)
            {
                FreeMatrix2D(shiftMatrix);
                ErrorMessage("Missing structure parameter '%s'",errStr);
                return(ERR);
            }

            fontColor = RGB(fontColorVec[0][0],fontColorVec[0][1],fontColorVec[0][2]);
            PlotText* txt = new PlotText(xpos, ypos, shiftMatrix, text.Str(), fontName, fontSize, fontAngle, fontStyle, fontColor, units);
            pd->text_.push_back(txt); 

         }
         FreeMatrix2D(shiftMatrix);

      }
      else
      {

       // Get the text color
         if(nrArgs >= 8)
         {
            if(fontColorVar.GetType() == MATRIX2D && fontColorVar.GetDimX() >= 3 && fontColorVar.GetDimY() == 1)
            {
               fontColorVec = fontColorVar.GetMatrix2D();
               fontColor = RGB(fontColorVec[0][0],fontColorVec[0][1],fontColorVec[0][2]);
            }
            else
            {
               ErrorMessage("invalid text color (should be 3*1)");
               return(ERR);
            }
         }
         else
         {
            fontColor = RGB(255,0,0);
         }

      // Add the text to the plot pd
         if(posVar.GetDimY() == 1) // 1 item
         {
            if(textVar.GetType() == LIST)
            {
               PlotText* txt = new PlotText(posVector[0][0], posVector[0][1], shiftMatrix, textVector[0], fontName, fontSize, fontAngle, fontStyle, fontColor, units);
               pd->text_.push_back(txt);    
            }
            else
            {
               PlotText* txt = new PlotText(posVector[0][0], posVector[0][1], shiftMatrix, textVar.GetString(), fontName, fontSize, fontAngle, fontStyle, fontColor, units);
               pd->text_.push_back(txt);   
            }
         }
         else // Multiple items
         {
            for(int i = 0; i < posVar.GetDimY(); i++)
            {
               PlotText* txt = new PlotText(posVector[i][0], posVector[i][1], shiftMatrix, textVector[i], fontName, fontSize, fontAngle, fontStyle, fontColor, units);
               pd->text_.push_back(txt);
            }
         }
      }

	   pd->DisplayAll(false);

	   itfc->nrRetValues = 0;
	   return(OK);	
	}

   // Copy from plot to temp storage
   else if(!strcmp(name,"copy"))
   {
	   if(pd->DataPresent())
		   pd->plotParent->SetSavedPlot(pd->clone());
      itfc->nrRetValues = 0;
      return(OK);
   }
   // Copy plot to clipboard
   else if(!strcmp(name,"copytoclipboard"))
   {
      CText modeTxt = "current";
      if((nrArgs = ArgScan(itfc,args,0,"all/current","e","t",&modeTxt)) < 0)
      {
         ErrorMessage("Expecting 0/1 arguments");
         return(ERR);
      }
	   if(pd->DataPresent())
         pd->plotParent->CopyPlotsToClipboard(modeTxt == "all");
      itfc->nrRetValues = 0;
      return(OK);
   }
   // Paste from temp storage to plot
   else if(!strcmp(name,"paste"))
   {
      Plot *np = pd->plotParent->PasteSavedPlot(pd);
      np->Invalidate();
      itfc->nrRetValues = 0;
      return(OK);
   }

   // Remove all lines from a plot
	else if (!strcmp(name,"rmlines"))
	{
      float xPos,yPos;

      if((nrArgs = ArgScan(itfc,args,0,"[x,] y]","ee","ff",&xPos,&yPos)) < 0)
      {
         ErrorMessage("Expecting 0-2 arguments");
         return(ERR);
      }

      if(nrArgs == 0)
      {
	      std::for_each(pd->lines_.begin(), pd->lines_.end(), delete_object());
         pd->lines_.clear();
		   pd->DisplayAll(false);
		   itfc->nrRetValues = 0;
		   return(OK);
      }
      else if(nrArgs == 1) // Remove line closest to xPos
      {
         int pos = -1;
         float dis = 1e30;
         for(int i = 0; i < pd->lines_.size(); i++)
         {
            PlotLine *ln = pd->lines_[i];
            float x = (ln->x0+ln->x1)/2.0;
            float r = fabs(x-xPos);
            if(r < dis)
            {
               pos = i;
               dis = r;
            }
         }
         if(pos >= 0)
            pd->lines_.erase(pd->lines_.begin() + pos);
		   pd->DisplayAll(false);
		   itfc->nrRetValues = 0;
		   return(OK);	
      }
      else // Remove line closest to (xPos,yPos)
      {
         int pos = -1;
         float dis = 1e30;

         for(int i = 0; i < pd->lines_.size(); i++)
         {
            PlotLine *ln = pd->lines_[i];
            float x = (ln->x0+ln->x1)/2.0;
            float y = (ln->y0+ln->y1)/2.0;
            float r = sqrt((x-xPos)*(x-xPos) + (y-yPos)*(y-yPos));
            if(r < dis)
            {
               pos = i;
               dis = r;
            }
         }
         if(pos >= 0)
            pd->lines_.erase(pd->lines_.begin() + pos);
		   pd->DisplayAll(false);
		   itfc->nrRetValues = 0;
		   return(OK);	
      }
   }

   // Remove text annotations from a plot [all, by position, by text]
	else if (!strcmp(name,"rmtext"))
	{
      Variable var1,var2;

      if((nrArgs = ArgScan(itfc,args,0,"[[x/txt,] y]","ee","vv",&var1,&var2)) < 0)
      {
         ErrorMessage("Expecting 0-2 arguments");
         return(ERR);
      }

      if(nrArgs == 0)
      {
         for(int i = 0; i < pd->text_.size(); i++)
         {
            PlotText *txt = pd->text_[i];
            delete txt;
         }
         pd->text_.clear();
		   pd->DisplayAll(false);
		   itfc->nrRetValues = 0;
		   return(OK);	
      }
      else if(nrArgs == 1) // Remove text closest to xPos
      {
			if(var1.GetType() == FLOAT32)
			{
				int xPos = nint(var1.GetReal());
				int pos = -1;
				float dis = 1e30;
				for(int i = 0; i < pd->text_.size(); i++)
				{
					PlotText *txt = pd->text_[i];
					float x = txt->x;
					float r = fabs(x-xPos);
					if(r < dis)
					{
						pos = i;
						dis = r;
					}
				}
				if(pos >= 0)
					pd->text_.erase(pd->text_.begin() + pos);
				pd->DisplayAll(false);
				itfc->nrRetValues = 0;
			}
			else if(var1.GetType() == UNQUOTED_STRING)
			{
				CText inputTxt = var1.GetString();
				int pos = -1;
				for(int i = 0; i < pd->text_.size(); i++)
				{
					PlotText *ptxt = pd->text_[i];
					if(ptxt->txt == inputTxt)
					{
						pos = i;
						break;
					}	
				}
				if(pos >= 0)
					pd->text_.erase(pd->text_.begin() + pos);
				pd->DisplayAll(false);
			}
			else
			{
				ErrorMessage("Invalid data type for rmtext command");
				return(ERR);
			}
		   return(OK);	
      }
      else // Remove text closest to (xPos,yPos)
      {
			if(var1.GetType() == FLOAT32 && var2.GetType() == FLOAT32)
			{
				int xPos = nint(var1.GetReal());
				int yPos = nint(var2.GetReal());

				int pos = -1;
				float dis = 1e30;
				for(int i = 0; i < pd->text_.size(); i++)
				{
					PlotText *txt = pd->text_[i];
					float x = txt->x;
					float y = txt->y;
					float r = sqrt((x-xPos)*(x-xPos) + (y-yPos)*(y-yPos));
					if(r < dis)
					{
						pos = i;
						dis = r;
					}
				}
				if(pos >= 0)
					pd->text_.erase(pd->text_.begin() + pos);
				pd->DisplayAll(false);
				itfc->nrRetValues = 0;
				return(OK);	
			}
			else
			{
				ErrorMessage("Invalid data types for rmtext command");
				return(ERR);
			}
      }
   }

   // Set or get the plot margins
   else if(!strcmp(name,"margins"))
   {
      if(!args || args[0] == '\0')
      {
         Margins& m = pd->getMargins();
         itfc->retVar[1].MakeAndSetFloat(m.left());
         itfc->retVar[2].MakeAndSetFloat(m.top());
         itfc->retVar[3].MakeAndSetFloat(m.right());
         itfc->retVar[4].MakeAndSetFloat(m.base());
         itfc->nrRetValues = 4;
         return(OK);
      }
      else
      {
         long left,right,top,base;
         if(ArgScan(itfc,args,4,"left,right,top,base","eeee","llll",&left,&top,&right,&base) < 0)
         {
            ErrorMessage("Expecting 4 arguments");
            return(ERR);
         }
	      Margins m(left,right,top,base);
         pd->setMargins(m);
         pd->DisplayAll(false);
         itfc->nrRetValues = 0;
         return(OK);
      }
   }

 // Add an image inset to a plot
	else if (!strcmp(name,"addimageinset") || !strcmp(name,"addimageannoation"))
	{
		CText text;
		int width=0;
		int left = 0;
		int top = 0;
		if(ArgScan(itfc,args,1,"text, [width, left, top]","eeee","tddd",&text,&width,&left,&top) == ERR)
      {
         ErrorMessage("Expecting 1 or 4 arguments");
         return(ERR);
      }
		try
		{
			pd->addInset(Inset::makeInset(IMAGE,string(text.Str()),width, left, top));
		}
		catch (std::exception& e)
		{
			ErrorMessage(e.what());
			return ERR;
		}
		pd->DisplayAll(false);
		itfc->nrRetValues = 0;
		return OK;		
	}

 // List all insets
	else if(!strcmp(name,"insets") || !strcmp(name,"annotations"))
   {
		string* insets = pd->describeInsets();
		itfc->retVar[1].MakeAndSetString(insets->c_str());
      delete insets;
      itfc->nrRetValues = 1;
      return(OK);
	}

 // Remove an inset
	else if(!strcmp(name,"rminset") || !strcmp(name,"rmannotation"))
   {
		long id;

      if((nrArgs = ArgScan(itfc,args,1,"inset ID","e","l",&id)) < 0)
      {
         ErrorMessage("Expecting 1 argument");
         return(ERR);
      }
		itfc->nrRetValues = 0;
		if (OK != pd->removeInset(id))
		{
			ErrorMessage("Inset/annotation ID '%ld' not found",id);
			return ERR;
		}
		pd->DisplayAll(false);
      return OK;
	}
   else if(!strcmp(name,"axes"))
   {
      if(!args || args[0] == '\0') // Return axes class e.g. r->axes()
      {
          itfc->retVar[1].MakeClass(AXES_CLASS,(void*)pd);
          itfc->nrRetValues = 1;
      }
      else // Set axes parameter e.g. r->axes("color",[255,0,0])
      {
	    	 MSG msg;
		//	 while(pd->plotParent->isBusy())
		//		PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
       //   pd->plotParent->setBusyWithCriticalSection(true);
			// printf("In critical section CLASSREF AXES\n");
          EnterCriticalSection(&cs1DPlot);
          pd->plotParent->incCriticalSectionLevel();
          pd->ProcessAxesParameters(itfc,args);  
          LeaveCriticalSection(&cs1DPlot);
          pd->plotParent->decCriticalSectionLevel();
			// printf("Leaving critical section CLASSREF AXES\n");

      }
      return(OK);
   }

   // Set the plot border color
   else if(!strcmp(name,"bordercolor"))
   {
      Variable colorVar;

      if(!args || args[0] == '\0')
      {
         float colors[4];
         colors[0] = GetRValue(pd->borderColor);
         colors[1] = GetGValue(pd->borderColor);
         colors[2] = GetBValue(pd->borderColor);
         colors[3] = ((DWORD)(pd->borderColor))>>24;
         (&colorVar)->MakeMatrix2DFromVector(colors,4,1);
         itfc->retVar[1].MakeMatrix2DFromVector(colors,4,1);
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
      // Get arguments from user *************
         if(ArgScan(itfc,args,1,"color","e","v",&colorVar) < 0)
         {
            ErrorMessage("Expecting 1 argument");
            return(ERR);
         }
       
         if(ConvertAnsVar(&colorVar,"border color", colorVar.GetType(),pd->borderColor) == ERR)
            return(ERR);

      // Redraw
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;

         return(OK);
      }
   }
   // Set the plot background color
   else if(!strcmp(name,"bkcolor") || !strcmp(name,"bkgcolor"))
   {
      Variable colorVar;

      if(!args || args[0] == '\0')
      {
         float colors[4];
         colors[0] = GetRValue(pd->bkColor);
         colors[1] = GetGValue(pd->bkColor);
         colors[2] = GetBValue(pd->bkColor);
         colors[3] = ((DWORD)(pd->bkColor))>>24;
         (&colorVar)->MakeMatrix2DFromVector(colors,4,1);
         itfc->retVar[1].MakeMatrix2DFromVector(colors,4,1);
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
      // Get arguments from user *************
         if(ArgScan(itfc,args,1,"color","e","v",&colorVar) < 0)
         {
            ErrorMessage("Expecting 1 argument");
            return(ERR);
         }
       
         if(ConvertAnsVar(&colorVar,"background color", colorVar.GetType(),pd->bkColor) == ERR)
            return(ERR);

      // Redraw
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
   // Set the zoom rectangle background color
   else if(!strcmp(name,"zoombkgcolor"))
   {
      Variable colorVar;

      if(!args || args[0] == '\0')
      {
         float colors[4];
         colors[0] = GetRValue(pd->zoomBkgColor);
         colors[1] = GetGValue(pd->zoomBkgColor);
         colors[2] = GetBValue(pd->zoomBkgColor);
         colors[3] = GetAValue(pd->zoomBkgColor);
         (&colorVar)->MakeMatrix2DFromVector(colors,4,1);
         itfc->retVar[1].MakeMatrix2DFromVector(colors,4,1);
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
      // Get arguments from user *************
         if(ArgScan(itfc,args,1,"color","e","v",&colorVar) < 0)
         {
            ErrorMessage("Expecting 1 argument");
            return(ERR);
         } 
       
         if(ConvertAnsVar4Color(&colorVar,"zoom background color", colorVar.GetType(),pd->zoomBkgColor) == ERR)
            return(ERR);

      // Redraw
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
   // Set the zoom rectangle border color
   else if(!strcmp(name,"zoombordercolor"))
   {
      Variable colorVar;

      if(!args || args[0] == '\0')
      {
         float colors[4];
         colors[0] = GetRValue(pd->zoomBorderColor);
         colors[1] = GetGValue(pd->zoomBorderColor);
         colors[2] = GetBValue(pd->zoomBorderColor);
         colors[3] = GetAValue(pd->zoomBorderColor);
         (&colorVar)->MakeMatrix2DFromVector(colors,4,1);
         itfc->retVar[1].MakeMatrix2DFromVector(colors,4,1);
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
      // Get arguments from user *************
         if(ArgScan(itfc,args,1,"color","e","v",&colorVar) < 0)
         {
            ErrorMessage("Expecting 1 argument");
            return(ERR);
         } 
       
         if(ConvertAnsVar4Color(&colorVar,"zoom background color", colorVar.GetType(),pd->zoomBorderColor) == ERR)
            return(ERR);

      // Redraw
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
   // Set the zoom rectangle mode
   else if(!strcmp(name,"zoomrectmode"))
   {
      CText mode = pd->zoomRectMode;

   // Get arguments from user *************
      r = ArgScan(itfc,args,1,"use dotted rect?","e","t",&mode);

      if(r == ERR)
         return(ERR); 

      if(r == -2)
         return(OK);

      pd->zoomRectMode = mode;

      pd->DisplayAll(false);  
      itfc->nrRetValues = 0;
      return(OK);
   }

  // Set or get whether the plot can be clicked on to become the current plot
  // Still needs work before this can be used correctly. Too many UI plot functions reply
 // on the concept of a current plot
 /*  else if(!strcmp(name,"allowtobecurrent"))
   {
      if(!args || args[0] == '\0')
      {
         if(pd->allowMakeCurrentPlot_)
             itfc->retVar[1].MakeAndSetString("true");
         else
             itfc->retVar[1].MakeAndSetString("false");
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         CText current;
         if(ArgScan(itfc,args,1,"true/false","e","t",&current) == ERR)
            return(ERR);
         
         if(current == "true" || current == "yes" || current == "on")
         {
            pd->allowMakeCurrentPlot_ = true;
         }
         else if(current == "false" || current == "no" || current == "off")
         {
            pd->allowMakeCurrentPlot_ = false;
         }
         else
         {
            ErrorMessage("invalid argument");
            return(ERR);
         }
         itfc->nrRetValues = 0;
         return(OK);
      }
   }*/
   // Set or get the plot drawing status
   else if(!strcmp(name,"draw"))
   {
      if(!args || args[0] == '\0')
      {
         if(pd->updatePlots())
             itfc->retVar[1].MakeAndSetString("true");
         else
             itfc->retVar[1].MakeAndSetString("false");
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         CText draw;
         if(ArgScan(itfc,args,1,"true/false","e","t",&draw) < 0)
         {
            ErrorMessage("Expecting 1 argument");
            return(ERR);
         } 
         
         if(draw == "true" || draw == "yes" || draw == "on")
         {
            pd->updatePlots(true);
         }
         else if(draw == "false" || draw == "no" || draw == "off")
         {
				pd->updatingPlots(true);
            pd->updatePlots(false);
         }
         else
         {
            ErrorMessage("invalid argument");
            return(ERR);
         }

         pd->DisplayAll(false); 

         if(draw == "true" || draw == "yes" || draw == "on")
				pd->updatingPlots(false);

         itfc->nrRetValues = 0;
         return(OK);
      }
   }
   // Get the grid class instance or set grid parameters
   else if(!strcmp(name,"grid"))
   {
      if(!args || args[0] == '\0') // Return grid class e.g. r->grid()
      {
          itfc->retVar[1].MakeClass(GRID_CLASS,(void*)pd);
          itfc->nrRetValues = 1;
      }
      else // Set grid parameter e.g. r->grid("color",[255,0,0])
      {
	    	// MSG msg;
			// while(pd->plotParent->isBusy())
			//	PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
         // pd->plotParent->setBusyWithCriticalSection(true);
         // pd->ProcessGridParameters(itfc,args);  
         // pd->plotParent->setBusyWithCriticalSection(false);
			// printf("In critical section CLASSREF GRID\n");
          EnterCriticalSection(&cs1DPlot);
          pd->plotParent->incCriticalSectionLevel();
          pd->ProcessGridParameters(itfc,args);
          LeaveCriticalSection(&cs1DPlot);
          pd->plotParent->decCriticalSectionLevel();
		  //  printf("Leaving critical section CLASSREF GRID\n");
      }
      return(OK);
   }
   // Get the plot dimension
   else if(!strncmp(name,"dim",3))
   {
      if(pd->getDimension() == 1)
         itfc->retVar[1].MakeAndSetString("1d");
      else
         itfc->retVar[1].MakeAndSetString("2d");
      itfc->nrRetValues = 1;
      return(OK);
   }
   // Get the plot file version
   else if(!strcmp(name,"fileversion"))
   {
      itfc->retVar[1].MakeAndSetFloat((float)pd->getFileVersion());
      itfc->nrRetValues = 1;
      return(OK);
   }
  // Get the plot file name
   else if(!strcmp(name,"filename"))
   {
      if (!args || args[0] == '\0')
      {
         itfc->retVar[1].MakeAndSetString(pd->getFileName());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         CText fileName;
         if (ArgScan(itfc, args, 1, "filename", "e", "t", &fileName) < 0)
         {
            ErrorMessage("Expecting 1 argument");
            return(ERR);
         }
         pd->setFileName(fileName.Str());
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
  // Get the plot file path
   else if(!strcmp(name,"filepath"))
   {
      {
         if (!args || args[0] == '\0')
         {
            itfc->retVar[1].MakeAndSetString(pd->getFilePath());
            itfc->nrRetValues = 1;
            return(OK);
         }
         else
         {
            CText filePath;
            if (ArgScan(itfc, args, 1, "filepath", "e", "t", &filePath) < 0)
            {
               ErrorMessage("Expecting 1 argument");
               return(ERR);
            }
            pd->setFilePath(filePath.Str());
            itfc->nrRetValues = 0;
            return(OK);
         }
      }
   }
   // Get the subplot cell position within parent
   else if(!strcmp(name,"position"))
   {
      itfc->retVar[1].MakeAndSetFloat(pd->colNr+1);
      itfc->retVar[2].MakeAndSetFloat(pd->rowNr+1);
      itfc->nrRetValues = 2;
      return(OK);
   }
   // Get plot parent class
   else if(!strcmp(name,"parent"))
   {
      ObjectData *obj = pd->plotParent->obj;
      itfc->retVar[1].MakeClass(OBJECT_CLASS, (void*)obj);
      itfc->nrRetValues = 1;
      return(OK);
   }
   // Set or get plot xlabel
   else if(!strcmp(name,"xlabel"))
   {
      if(!args || args[0] == '\0') // Return label class e.g. r->xlabel()
      {
          itfc->retVar[1].MakeClass(XLABEL_CLASS,(void*)pd);
          itfc->nrRetValues = 1;
      }
      else // Set label parameter e.g. r->xlabel("text","my text")
      {
	    	// MSG msg;
			// while(pd->plotParent->isBusy())
			//	PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
        //  pd->plotParent->setBusyWithCriticalSection(true);
        //  pd->ProcessLabelParameters(itfc,"xlabel", args);  
         // pd->plotParent->setBusyWithCriticalSection(false);
		 //   printf("In critical section CLASSREF XLABEL\n");
          EnterCriticalSection(&cs1DPlot);
          pd->plotParent->incCriticalSectionLevel();
          pd->ProcessLabelParameters(itfc,"xlabel", args);  
          LeaveCriticalSection(&cs1DPlot);
          pd->plotParent->decCriticalSectionLevel();
			// printf("Leaving critical section CLASSREF XLABE\n");
       }
      return(OK);
   }
   // Set or get plot current ylabel
   else if(!strcmp(name,"ylabel"))
   {
       if(!args || args[0] == '\0')
       {
          itfc->retVar[1].MakeClass(YLABEL_CLASS,(void*)pd);
          itfc->nrRetValues = 1;
      }
       else
       {
	    //	 MSG msg;
		//	 while(pd->plotParent->isBusy())
			//	PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
        //  pd->plotParent->setBusyWithCriticalSection(true);
        //  pd->ProcessLabelParameters(itfc,"ylabel", args);  
        //  pd->plotParent->setBusyWithCriticalSection(false);
		  //  printf("In critical section CLASSREF YLABEL\n");
          EnterCriticalSection(&cs1DPlot);
          pd->plotParent->incCriticalSectionLevel();
          pd->ProcessLabelParameters(itfc,"ylabel", args);  
          LeaveCriticalSection(&cs1DPlot);
          pd->plotParent->decCriticalSectionLevel();
			// printf("Leaving critical section CLASSREF YLABEL\n");
       }
      return(OK);
   }
   // Set or get plot left ylabel
	else if(!strcmp(name,"ylabelleft"))
   {
       if(!args || args[0] == '\0')
       {
          itfc->retVar[1].MakeClass(YLABEL_LEFT_CLASS,(void*)pd);
          itfc->nrRetValues = 1;
      }
       else
       {
	    //	 MSG msg;
		//	 while(pd->plotParent->isBusy())
			//	PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
        //  pd->plotParent->setBusyWithCriticalSection(true);
        //  pd->ProcessLabelParameters(itfc,"ylabelleft", args);  
        //  pd->plotParent->setBusyWithCriticalSection(false);
		  //  printf("In critical section CLASSREF YLABEL-LEFT\n");
          EnterCriticalSection(&cs1DPlot);
          pd->plotParent->incCriticalSectionLevel();
          pd->ProcessLabelParameters(itfc,"ylabelleft", args);  
          LeaveCriticalSection(&cs1DPlot);
          pd->plotParent->decCriticalSectionLevel();
			// printf("Leaving critical section CLASSREF YLABEL-LEFT\n");

       }
      return(OK);
   }
   // Set or get plot right ylabel
	else if(!strcmp(name,"ylabelright"))
   {
       if(!args || args[0] == '\0')
       {
          itfc->retVar[1].MakeClass(YLABEL_RIGHT_CLASS,(void*)pd);
          itfc->nrRetValues = 1;
      }
       else
       {
	   // 	 MSG msg;
			 //while(pd->plotParent->isBusy())
				//PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
    //      pd->plotParent->setBusyWithCriticalSection(true);
    //      pd->ProcessLabelParameters(itfc,"ylabelright", args);  
    //      pd->plotParent->setBusyWithCriticalSection(false);
			 				//	printf("Entering critical section 7\n");
		 //   printf("In critical section CLASSREF YLABEL-RIGHT\n");
          EnterCriticalSection(&cs1DPlot);
          pd->plotParent->incCriticalSectionLevel();
          pd->ProcessLabelParameters(itfc,"ylabelright", args);  
          LeaveCriticalSection(&cs1DPlot);
          pd->plotParent->decCriticalSectionLevel();
			// printf("Leaving critical section CLASSREF YLABEL-RIGHT\n");

       }
      return(OK);
   }
   // Set or get vertical status of ylabel
	else if(!strcmp(name,"ylabelvert"))
   {
      if(!args || args[0] == '\0')
      {
         if(pd->yLabelVert == true)
             itfc->retVar[1].MakeAndSetString("true");
         else
             itfc->retVar[1].MakeAndSetString("false");
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         CText vert = "false";
         if(pd->yLabelVert)
            vert = "true";

      // Get arguments from user *************
         if((r = ArgScan(itfc,args,1,"vertical?","e","t",&vert)) < 0)
         {
            ErrorMessage("Expecting 1 argument");
            return(ERR);
         } 

         if(vert == "true")
            pd->yLabelVert = true;
         else
            pd->yLabelVert = false;

         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
   // Set or get plot title
   else if(!strcmp(name,"title"))
   {
      if(!args || args[0] == '\0')
      {
          itfc->retVar[1].MakeClass(TITLE_CLASS,(void*)pd);
          itfc->nrRetValues = 1;
      }
      else
       {
	   // 	 MSG msg;
			 //while(pd->plotParent->isBusy())
			 //	PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
    //      pd->plotParent->setBusyWithCriticalSection(true);
    //      if(pd->ProcessLabelParameters(itfc,"title", args) == ERR)
    //      {
    //         pd->plotParent->setBusyWithCriticalSection(false);
    //         return(ERR);
    //      }
    //      pd->plotParent->setBusyWithCriticalSection(false);
		  //  printf("In critical section CLASSREF TITLE\n");
          EnterCriticalSection(&cs1DPlot);
          pd->plotParent->incCriticalSectionLevel();
          if(pd->ProcessLabelParameters(itfc,"title", args) == ERR)
          {
             LeaveCriticalSection(&cs1DPlot);
             pd->plotParent->decCriticalSectionLevel();
             return(ERR);
          }
          LeaveCriticalSection(&cs1DPlot);
          pd->plotParent->decCriticalSectionLevel();
		  	// printf("Leaving critical section  CLASSREF TITLE\n");
       }
      return(OK);
   }
  // Set plot border status
   else if(!strcmp(name,"border"))
   {
      CText show = "false";

      if(pd->plotParent->showLabels)
         show = "true";

   // Get arguments from user *************
      r = ArgScan(itfc,args,1,"show?","e","t",&show);

      if(r == ERR)
         return(ERR); 

      if(r == -2)
         return(OK);

      if(show == "true")
         pd->plotParent->showLabels = true;
      else
         pd->plotParent->showLabels = false;

      pd->plotParent->obj->winParent->setMenuItemCheck(pd->plotParent->menuName(), "toggle_border", pd->plotParent->showLabels);

      pd->DisplayAll(false);  
      itfc->nrRetValues = 0;
      return(OK);
   }
   // Will plot autorange
   else if(!strcmp(name,"autorange"))
   {
      CText autoRange = "false";

      if(!pd->getOverRideAutoRange())
         autoRange = "true";

   // Get arguments from user *************
      r = ArgScan(itfc,args,1,"auto-range","e","t",&autoRange);

      if(r == ERR)
         return(ERR); 

      if(r == -2)
         return(OK);

      if(autoRange == "true")
         pd->setOverRideAutoRange(false);
      else
         pd->setOverRideAutoRange(true);

      pd->DisplayAll(false);  
      itfc->nrRetValues = 0;
      return(OK);
   }
  // Get or set plot x offset from parent
   else if(!strcmp(name,"x"))
   {
      if(!args || args[0] == '\0')
      {
         itfc->retVar[1].MakeAndSetFloat(pd->GetLeft());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         int x;
         r = ArgScan(itfc,args,1,"x offset","e","d",&x);
         if(r == ERR)
            return(ERR); 
         if(r == -2)
            return(OK);
          pd->SetLeft(x);
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
  // Get or set plot y offset from parent
   else if(!strcmp(name,"y"))
   {
      if(!args || args[0] == '\0')
      {
         itfc->retVar[1].MakeAndSetFloat(pd->GetTop());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         int y;
         r = ArgScan(itfc,args,1,"y offset","e","d",&y);
         if(r == ERR)
            return(ERR); 
         if(r == -2)
            return(OK);

         pd->SetTop(y);
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
  // Get or set plot width
   else if(!strcmp(name,"width"))
   {
      if(!args || args[0] == '\0')
      {
         itfc->retVar[1].MakeAndSetFloat(pd->GetWidth());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         int width;
         r = ArgScan(itfc,args,1,"width","e","d",&width);
         if(r == ERR)
            return(ERR); 
         if(r == -2)
            return(OK);
          pd->SetWidth(width);
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
  // Get or set plot height
   else if(!strcmp(name,"height"))
   {
      int height;
      if(!args || args[0] == '\0')
      {
         itfc->retVar[1].MakeAndSetFloat(pd->GetHeight());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         r = ArgScan(itfc,args,1,"width","e","d",&height);
         if(r == ERR)
            return(ERR); 
         if(r == -2)
            return(OK);
          pd->SetHeight(height);
         pd->DisplayAll(false);  
         itfc->nrRetValues = 0;
         return(OK);
      }
   }
	else
   {
      ErrorMessage("Unknown or invalid plot function '%s'",name);
      return(ERR);
   }
}


long fileVersionNumberToConstant(long fileVersionNumber, short dim)
{
   if(dim == 1)
   {
      switch(fileVersionNumber)
      {
         case(320):
            return(PLOTFILE_VERSION_3_2);
         case(330):
            return(PLOTFILE_VERSION_3_3);
         case(340):
            return(PLOTFILE_VERSION_3_4);
         case(350):
            return(PLOTFILE_VERSION_3_5);
         case(360):
            return(PLOTFILE_VERSION_3_6);
         case(370):
            return(PLOTFILE_VERSION_3_7);
         case(380):
            return(PLOTFILE_VERSION_3_8);
         default:
            ErrorMessage("Unknown or unsupported file format (accepted:  320,330,340,350,360,370,380)");
            return(ERR);
      }
   }
   else
   {
     switch(fileVersionNumber)
      {
         case(210):
            return(PLOTFILE_VERSION_2_1);
         case(320):
            return(PLOTFILE_VERSION_3_2);
         case(330):
            return(PLOTFILE_VERSION_3_3);
         case(340):
            return(PLOTFILE_VERSION_3_4);
         case(350):
            return(PLOTFILE_VERSION_3_5);
         case(360):
            return(PLOTFILE_VERSION_3_6);
         case(370):
            return(PLOTFILE_VERSION_3_7);
         default:
            ErrorMessage("Unknown or unsupported file format (accepted: 210,320,330,340,350,360,370)");
            return(ERR);
      }

   }
   return(OK);
}


long fileVersionConstantToNumber(long fileVersion, short dim)
{
   if(dim == 1)
   {
      switch(fileVersion)
      {
         case(PLOTFILE_VERSION_3_2):
            return(320);
         case(PLOTFILE_VERSION_3_3):
            return(330);
         case(PLOTFILE_VERSION_3_4):
            return(340);
         case(PLOTFILE_VERSION_3_5):
            return(350);
         case(PLOTFILE_VERSION_3_6):
            return(360);
         case(PLOTFILE_VERSION_3_7):
            return(370);
         case(PLOTFILE_VERSION_3_8):
            return(380);
         default:
            ErrorMessage("Unknown or unsupported file format (accepted: 320,330,340,350,360,370,380)");
            return(ERR);
      }
   }
   else
   {
      switch(fileVersion)
      {
         case(PLOTFILE_VERSION_2_1):
            return(210);
         case(PLOTFILE_VERSION_3_0):
            return(300);
         case(PLOTFILE_VERSION_3_2):
            return(320);
         case(PLOTFILE_VERSION_3_3):
            return(330);
         case(PLOTFILE_VERSION_3_4):
            return(340);
         case(PLOTFILE_VERSION_3_5):
            return(350);   
         case(PLOTFILE_VERSION_3_6):
            return(360);
         case(PLOTFILE_VERSION_3_7):
            return(370);
         default:
            ErrorMessage("Unknown or unsupported file format (accepted: 210,300,320,330,340,350,360,370)");
            return(ERR);
      }
   }
   return(OK);
}