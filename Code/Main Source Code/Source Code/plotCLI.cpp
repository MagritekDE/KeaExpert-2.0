#include "stdafx.h"
#include "plotCLI.h"
#include "allocate.h"
#include "array.h"
#include "cArg.h"
#include "trace.h"
#include "evaluate.h"
#include "font.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "mymath.h"
#include "plot_load_save_1d.h"
#include "plot1dCLI.h"
#include "plot2dFiles.h"
#include "plot.h"
#include "PlotGrid.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "TracePar.h"
#include "TraceParCLI.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

// Locally defined functions

short SetPlotParameter(Interface *itfc, CText &parameter, Variable *valueVar, PlotWinDefaults *pwd, PlotWindow *pp, short nrPlots);
short SetPlotParameter(Interface *itfc, CText &parameter, Variable *valueVar, PlotWinDefaults *pwd);
int CopyPlot(Interface* itfc, char args[]);
int PastePlot(Interface* itfc, char args[]);
int PasteIntoPlot(Interface* itfc, char args[]);

/*************************************************************************
*        Copy plot or image to window save                               *
*************************************************************************/

int CopyPlot(Interface* itfc, char args[])
{
   Variable plotRegion;
   short nrArgs;
   Plot *plt;
   PlotWindow *pp;

   if((nrArgs = ArgScan(itfc,args,0,"plot,mode","e","v",&plotRegion)) < 0)
     return(nrArgs);

   if(nrArgs == 0)
   {
      plt = Plot::curPlot();
      pp = plt->plotParent;
      if(!pp->CopyCurrentPlot())
      {
         ErrorMessage("No plot to copy or no current plot defined");
         return(ERR);
      }
   }
   else
   {
      if(plotRegion.GetType() == CLASS)
      {
         ClassData *cd = (ClassData*)plotRegion.GetData();

         if(CheckClassValidity(cd,true) == ERR)
            return(ERR);

         if(cd->type != PLOT_CLASS)
         {
            ErrorMessage("invalid argument should be a plot region");
            return(ERR);
         }

         plt = (Plot*)cd->data;
         pp = plt->plotParent;
         if(!pp->CopyPlot(plt))
         {
            ErrorMessage("No plot data to copy or no current plot defined");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid argument should be a plot region");
         return(ERR);
      }
   }
   

	itfc->nrRetValues = 0;
   return(OK);
}

/*************************************************************************
*        Paste saved image to   window save                              *
*************************************************************************/

int PastePlot(Interface* itfc, char args[])
{
   Variable plotRegion;
   short nrArgs;
   Plot *plt;
   PlotWindow *pp;
   
   if((nrArgs = ArgScan(itfc,args,0,"plot,mode","e","v",&plotRegion)) < 0)
     return(nrArgs);

   if(nrArgs == 0)
   {
	   Plot* curPlot = Plot::curPlot();
      PlotWindow *pp = curPlot->plotParent;
      if(!pp->PasteSavedPlot())
      {
         ErrorMessage("No plot to paste, invalid dimension or no current plot");
         return(ERR);
      }
      pp->DisplayAll(false);
   }
   else
   {
      if(plotRegion.GetType() == CLASS)
      {
         ClassData *cd = (ClassData*)plotRegion.GetData();

         if(CheckClassValidity(cd,true) == ERR)
            return(ERR);

         if(cd->type != PLOT_CLASS)
         {
            ErrorMessage("invalid argument should be a plot region");
            return(ERR);
         }

         plt = (Plot*)cd->data;
         pp = plt->plotParent;
         if(!pp->PastePlot(plt))
         {
            ErrorMessage("No plot to paste, invalid dimension or plot destination");
            return(ERR);
         }
         pp->DisplayAll(false);
      }
      else
      {
         ErrorMessage("invalid argument should be a plot region");
         return(ERR);
      }
   }

	itfc->nrRetValues = 0;
   return(OK);
}


/*************************************************************************
*        Paste saved image to   window save                              *
*************************************************************************/

int PasteIntoPlot(Interface* itfc, char args[])
{
   Variable plotRegion;
   short nrArgs;
   Plot* plt;
   PlotWindow* pp;

   if ((nrArgs = ArgScan(itfc, args, 0, "plot,mode", "e", "v", &plotRegion)) < 0)
      return(nrArgs);

   if (nrArgs == 0)
   {
      Plot* curPlot = Plot::curPlot();
      PlotWindow* pp = curPlot->plotParent;
      pp->PasteSavedPlotInto();
      pp->DisplayAll(false);
   }
   else
   {
      if (plotRegion.GetType() == CLASS)
      {
         ClassData* cd = (ClassData*)plotRegion.GetData();

         if (CheckClassValidity(cd, true) == ERR)
            return(ERR);

         if (cd->type != PLOT_CLASS)
         {
            ErrorMessage("invalid argument should be a plot region");
            return(ERR);
         }

         plt = (Plot*)cd->data;
         pp = plt->plotParent;
         pp->PasteSavedPlotInto(plt);
         pp->DisplayAll(false);
      }
      else
      {
         ErrorMessage("invalid argument should be a plot region");
         return(ERR);
      }
   }

   itfc->nrRetValues = 0;
   return(OK);
}


/*************************************************************************
*        Set load plot mode                                              *
*************************************************************************/

int LoadPlotMode(Interface* itfc ,char args[])
{
   CText mode,plot;
   short nrArgs;
   short r = OK;

// Get mode from user *************  
   if((nrArgs = ArgScan(itfc,args,1,"plot,mode","ee","tt",&plot,&mode)) < 0)
     return(nrArgs);  

// Based on plot call appropriate routine
   if(plot == "1d")
      r = Plot1D::curPlot()->plotParent->setPlotMode(mode);
   else if(plot == "2d")
		r = Plot2D::curPlot()->plotParent->setPlotMode(mode);
   else
   {
      ErrorMessage("invalid plot (1d/2d)");
      return(ERR);
   }
	itfc->nrRetValues = 0;
   return(r);
}

// Toggle the display of the current plot border

int ShowPlotBorder(Interface* itfc ,char args[])
{
   CText  showBorder;
   short nrArgs;
	Plot* curPlot = Plot::curPlot();

// Get arguments
   if((nrArgs = ArgScan(itfc,args,1,"show border (true/false)","e","t",&showBorder)) < 0)
      return(nrArgs); 
  
// Check for current plot
   if(curPlot == NULL)
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }

   PlotWindow *pp = curPlot->plotParent;

   if(showBorder == "true")
   {
      HMENU hMenu = GetMenu(curPlot->win);
      	      
      CheckMenuItem(hMenu,ID_DISPLAY_BORDERS,MF_CHECKED);	
      pp->showLabels = true;

   }
   else
   {
      HMENU hMenu = GetMenu(curPlot->win);
      	      
      if(curPlot->getDimension() == 1)
      {
         CheckMenuItem(hMenu,ID_DISPLAY_BORDERS,MF_UNCHECKED);	
         pp->showLabels = false;
      }
      if(curPlot->getDimension() == 2)
      {
         CheckMenuItem(hMenu,ID_DISPLAY_BORDERS,MF_UNCHECKED);	
         pp->showLabels = false;
      }
   }

// Redraw relevant plot
   if(curPlot->getDimension() == 1)
   {
      curPlot->DisplayAll(false); 
   }
// Make sure 2D plot bitmap is resized
   else if(curPlot->getDimension() == 2)
   {
      RECT r;
      long width,height;
	   GetClientRect(curPlot->win,&r);
		width = r.right-r.left+1;
		height = r.bottom-r.top+1;
		SendMessage(curPlot->win,WM_SIZE,(WPARAM)SIZE_RESTORED,(LPARAM)(width+(height<<16)));  
   }
   else
   {
      ErrorMessage("invalid dimension passed to set title");
      return(ERR);
   }
      
	itfc->nrRetValues = 0;
   return(OK);
}

int SetTitleParameters(Interface* itfc, char args[])
{
	Plot* curPlot;
// Check for current plot
	if(!(curPlot = Plot::curPlot()))
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }
     
// Process passed parameters
   return(curPlot->ProcessLabelParameters(itfc,"title",args));
}


int SetXLabelParameters(Interface* itfc, char args[])
{
	Plot* curPlot;
// Check for current plot
   if(!(curPlot = Plot::curPlot()))
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }
     
// Process passed parameters
   return(curPlot->ProcessLabelParameters(itfc,"xlabel",args));
}


int SetYLabelParameters(Interface* itfc, char args[])
{
	Plot* curPlot;
// Check for current plot
   if(!(curPlot = Plot::curPlot()))
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }
     
// Process passed parameters
   return(curPlot->ProcessLabelParameters(itfc,"ylabel",args));
}

int SetLeftYLabelParameters(Interface* itfc, char args[])
{
	Plot* curPlot;
// Check for current plot
   if(!(curPlot = Plot::curPlot()))
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }
     
// Process passed parameters
   return(curPlot->ProcessLabelParameters(itfc,"ylabelleft",args));
}

int SetRightYLabelParameters(Interface* itfc, char args[])
{
	Plot* curPlot;
// Check for current plot
   if(!(curPlot = Plot::curPlot()))
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }
     
// Process passed parameters
   return(curPlot->ProcessLabelParameters(itfc,"ylabelright",args));
}
           
int ModifyAxes(Interface* itfc, char args[])
{
	Plot* curPlot;
// Check for current plot
   if(!(curPlot = Plot::curPlot()))
	{
      ErrorMessage("no plot defined");
      return(ERR);
   }

   return(curPlot->ProcessAxesParameters(itfc,args));                   
}


void ReturnColour(Interface *itfc, COLORREF colIn, float* colOut)
{
   colOut[0] = GetRValue(colIn);
   colOut[1] = GetGValue(colIn);
   colOut[2] = GetBValue(colIn);

   short other = LOBYTE((colIn)>>24);

   if(other != 0)
   {
      colOut[3] = other;
      itfc->retVar[1].MakeMatrix2DFromVector(colOut,4,1);
   }
   else
   {
      colOut[3] = 0;
      itfc->retVar[1].MakeMatrix2DFromVector(colOut,3,1);
   }
   itfc->nrRetValues = 1;
}


/***************************************************************************
      Return the dimensions of the selected rectangle in the current 
      1D or 2D plot. Returns (-1,-1,-1,-1) if not defined

      Syntax: (x1,y1,x2,y2) = getrect()
****************************************************************************/

int GetSelectRectangle(Interface *itfc, char args[])
{
	Plot* curPlot;
// Check for current plot
   if(!(curPlot = Plot::curPlot()))
   {
      ErrorMessage("no current plot");
      return(ERR);
   }
   
   if(curPlot->rectSelected)
   {    
	   itfc->retVar[1].MakeAndSetFloat(curPlot->selectRect.left); 
	   itfc->retVar[2].MakeAndSetFloat(curPlot->selectRect.top);
	   itfc->retVar[3].MakeAndSetFloat(curPlot->selectRect.right);
	   itfc->retVar[4].MakeAndSetFloat(curPlot->selectRect.bottom);
      itfc->nrRetValues = 4;
   }
   else
   {
	   itfc->retVar[1].MakeAndSetFloat(-1); 
	   itfc->retVar[2].MakeAndSetFloat(-1);
	   itfc->retVar[3].MakeAndSetFloat(-1);
	   itfc->retVar[4].MakeAndSetFloat(-1);
	   itfc->nrRetValues = 4;
   }
   return(OK);
}

/***************************************************************************
   Function   :  PlotPreferences
   Description:  CLI interface function for specifing the default values for
                 displaying 1D and 2D plots.
   Arguments  :  (str parameter, variant value, ...)
   Returns    :  OK or ERR.   
****************************************************************************/


int PlotPreferences(Interface* itfc, char args[])
{
   CArg carg;
   CText  parameter;
   short type;
   Variable parVar;
   CText  value;  
   Variable valueVar;
   short r;
   bool extractArg = false;

   short nrArgs = carg.Count(args);

// Prompt user if no arguments supplied
   if(nrArgs == 0)
   {
      TextMessage("\n\n   PARAMETER       VALUE\n\n");
		TextMessage("   antialiasing1d ...... \"%s\"\n",pwd->antiAliasing ? "true" : "false");
      TextMessage("   axescolor ........... %s\n",Plot::GetColorStr(pwd->axesColor)); 
      TextMessage("   axesfontcolor ....... %s\n",Plot::GetColorStr(pwd->ticks->fontColor()));             
      TextMessage("   axesfontname ........ \"%s\"\n",pwd->axesFontName);             
      TextMessage("   axesfontsize ........ %hd\n",pwd->ticks->fontSize());             
      TextMessage("   axesfontstyle ....... \"%s\"\n",GetFontStyleStr(pwd->ticks->fontStyle()));             
      TextMessage("   axestype ............ \"%s\"\n",pwd->GetAxesTypeStr());             
      TextMessage("   bkgcolor ............ %s\n",Plot::GetColorStr(pwd->bkColor));
      TextMessage("   bordercolor ......... %s\n",Plot::GetColorStr(pwd->borderColor));
      TextMessage("   errbarcolor ......... %s\n",Plot::GetColorStr(pwd->tracePar.getBarColor()));
      TextMessage("   errbarfixed ......... \"%s\"\n",pwd->tracePar.isFixedErrorBars() ? "true" : "false");
      TextMessage("   errbarshow .......... \"%s\"\n",pwd->tracePar.isShowErrorBars() ? "true" : "false");
      TextMessage("   errbarsize .......... %g\n",pwd->tracePar.getBarFixedHeight());
      TextMessage("   finegridcolor ....... %s\n",Plot::GetColorStr(pwd->fineGridColor));       
      TextMessage("   gridcolor ........... %s\n",Plot::GetColorStr(pwd->gridColor));  
      TextMessage("   imagcolor ........... %s\n",Plot::GetColorStr(pwd->tracePar.getImagColor()));
      TextMessage("   imagstyle ........... %hd\n",pwd->tracePar.getImagStyle());
      TextMessage("   labelcolor .......... %s\n",Plot::GetColorStr(pwd->labelFontColor));
      TextMessage("   labelfontname ....... \"%s\"\n",pwd->labelFontName); 
      TextMessage("   labelfontsize ....... %hd\n",pwd->labelFontSize); 
      TextMessage("   labelfontstyle ...... \"%s\"\n",GetFontStyleStr(pwd->labelFontStyle));             
		TextMessage("   majorticklength ..... %hd\n",pwd->ticks->majorLength());
      TextMessage("   minorticklength ..... %hd\n",pwd->ticks->minorLength());  
      TextMessage("   realcolor ........... %s\n",Plot::GetColorStr(pwd->tracePar.getRealColor()));
      TextMessage("   realstyle ........... %hd\n",pwd->tracePar.getRealStyle());
      TextMessage("   realsymbolcolor ..... %s\n",Plot::GetColorStr(pwd->tracePar.getRealSymbolColor()));
      TextMessage("   imagsymbolcolor ..... %s\n",Plot::GetColorStr(pwd->tracePar.getImagSymbolColor()));
      TextMessage("   symbolshape ......... \"%s\"\n",TracePar::GetSymbolTypeStr(pwd->tracePar.getSymbolType()));
      TextMessage("   symbolsize .......... %hd\n",pwd->tracePar.getSymbolSize());
      TextMessage("   titlecolor .......... %s\n",Plot::GetColorStr(pwd->titleFontColor));
      TextMessage("   titlefontname ....... \"%s\"\n",pwd->titleFontName);
      TextMessage("   titlefontsize ....... %hd\n",pwd->titleFontSize);  
      TextMessage("   titlefontstyle ...... \"%s\"\n",GetFontStyleStr(pwd->titleFontStyle));             
      TextMessage("   tracetype ........... \"%s\"\n",TracePar::GetTraceTypeStr(pwd->tracePar.getTraceType()));
      TextMessage("   tracewidth .......... %hd\n",pwd->tracePar.getTraceWidth()/2.0+0.5);
      TextMessage("   zoombkgcolor ........ %s\n",Plot::GetAlphaColorStr(pwd->zoomBkgColor));
      TextMessage("   zoombordercolor ..... %s\n",Plot::GetAlphaColorStr(pwd->zoomBorderColor));
      TextMessage("   zoomrectmode ........ %s\n",pwd->zoomRectMode);
      return(0);
   }

// Check for valid number of arguments (should be even unless returning a value)
   if(nrArgs != 1 && nrArgs%2 != 0)
   {
      ErrorMessage("number of arguments must be even or one");
      return(ERR);
   }

   if(nrArgs == 1)
   {
      extractArg = true;
      nrArgs = 2;
   }

   // Extract arguments    
   short argCnt = 1;
   while(argCnt < nrArgs)
   {   
   // Extract parameter name
      parameter = carg.Extract(argCnt);
      if((type = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&parVar)) < 0)
         return(ERR); 

      if(type != UNQUOTED_STRING)
      {
         ErrorMessage("invalid data type for parameter '%s'",parameter.Str());
         return(ERR);
      }  
      parameter = parVar.GetString();
           

   // Extract parameter value
      if(!extractArg)
      {
         value = carg.Extract(argCnt+1);
         if((type = Evaluate(itfc,RESPECT_ALIAS,value.Str(),&valueVar)) < 0)
            return(ERR);          
         value.RemoveQuotes();
      }
      else
      {
         valueVar.MakeAndSetString("");
      }

   // Process different kinds of parameter values using ConvertAnsVar  
   // store result in the plotWindowDefaults structure
      r = SetPlotParameter(itfc, parameter, &valueVar, pwd);

      if(r == OK || r == ERR)
         return(r);
                                          
      argCnt+=2; // Look at next parameter:value pair
   }

   return(OK); 
}



short SetPlotParameter(Interface *itfc, CText &parameter, Variable *valueVar, PlotWinDefaults *pwd)
{
   float col[4];
   short r;
   short type = valueVar->GetType();

// Get a parameter

   if(type == UNQUOTED_STRING)
   {
      char *value = valueVar->GetString();

      if(!strcmp(value,"") || !strcmp(value,"\"\"") ||
         !strcmp(value,"\"getargs\"") || !strcmp(value,"\"getarg\""))
      {
			if(parameter == "antialiasing1d")
         { 
				itfc->retVar[1].MakeAndSetString(pwd->antiAliasing? ("true") : ("false"));
         }
         else if(parameter == "axescolor")
         { 
            ReturnColour(itfc,pwd->axesColor,col);
         }
         else if(parameter == "axesfontcolor")
         { 
            ReturnColour(itfc,pwd->ticks->fontColor(),col);
         } 
         else if(parameter == "axesfontname")
         { 
            itfc->retVar[1].MakeAndSetString(pwd->axesFontName);
         }
         else if(parameter == "axesfontsize")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->ticks->fontSize());
         }
         else if(parameter == "axesfontstyle")
         { 
            itfc->retVar[1].MakeAndSetString(GetFontStyleStr(pwd->ticks->fontStyle()));
         }
         else if(parameter == "axestype")
         { 
            itfc->retVar[1].MakeAndSetString(pwd->GetAxesTypeStr());
         }
         else if(parameter == "bkgcolor" || parameter == "bkcolor")
         { 
            ReturnColour(itfc,pwd->bkColor,col);
         } 
         else if(parameter == "bordercolor")
         { 
            ReturnColour(itfc,pwd->borderColor,col);
         }
         else if(parameter == "errbarcolor")
         { 
            ReturnColour(itfc,pwd->tracePar.getBarColor(),col);
         } 
         else if(parameter == "errbarfixed")
         { 
            itfc->retVar[1].MakeAndSetString(pwd->tracePar.isFixedErrorBars() ? ("true") : ("false"));
         } 
         else if(parameter == "errbarshow")
         { 
            itfc->retVar[1].MakeAndSetString(pwd->tracePar.isShowErrorBars() ? ("true") : ("false"));
         } 
         else if(parameter == "errbarsize")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->tracePar.getBarFixedHeight());
         } 
         else if(parameter == "finegridcolor")
         { 
            ReturnColour(itfc,pwd->fineGridColor,col);
         }
         else if(parameter == "gridcolor")
         { 
            ReturnColour(itfc,pwd->gridColor,col);
         }
         else if(parameter == "imagcolor")
         { 
            ReturnColour(itfc,pwd->tracePar.getImagColor(),col);
         } 
         else if(parameter == "imagstyle")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->tracePar.getImagStyle());
         } 
         else if(parameter == "labelcolor")
         { 
            ReturnColour(itfc,pwd->labelFontColor,col);
         }
         else if(parameter == "labelfontname")
         { 
            itfc->retVar[1].MakeAndSetString(pwd->labelFontName);
         }
         else if(parameter == "labelfontsize")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->labelFontSize);
         }
         else if(parameter == "labelfontstyle")
         { 
            itfc->retVar[1].MakeAndSetString(GetFontStyleStr(pwd->labelFontStyle));
         }
         else if(parameter == "minorticklength")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->ticks->minorLength());
         }
         else if(parameter == "majorticklength")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->ticks->majorLength());
         }   
         else if(parameter == "realcolor")
         { 
            ReturnColour(itfc,pwd->tracePar.getRealColor(),col);
         }
         else if(parameter == "realstyle")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->tracePar.getRealStyle());
         }
         else if(parameter == "symbolcolor")
         { 
            ReturnColour(itfc,pwd->tracePar.getRealSymbolColor(),col);
         } 
         else if(parameter == "realsymbolcolor")
         { 
            ReturnColour(itfc,pwd->tracePar.getRealSymbolColor(),col);
         }
         else if(parameter == "imagsymbolcolor")
         { 
            ReturnColour(itfc,pwd->tracePar.getImagSymbolColor(),col);
         }
         else if(parameter == "symbolshape")
         { 
				char* c = strdup(TracePar::GetSymbolTypeStr(pwd->tracePar.getSymbolType()));
            itfc->retVar[1].MakeAndSetString(c);
				free(c);
         }
         else if(parameter == "titlecolor")
         { 
            ReturnColour(itfc,pwd->titleFontColor,col);
         } 
         else if(parameter == "titlefontname")
         { 
            itfc->retVar[1].MakeAndSetString(pwd->titleFontName);
         }
         else if(parameter == "titlefontsize")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->titleFontSize);
         }
         else if(parameter == "titlefontstyle")
         { 
            itfc->retVar[1].MakeAndSetString(GetFontStyleStr(pwd->titleFontStyle));
         }
         else if(parameter == "tracetype")
         {     
				char* c = strdup(TracePar::GetTraceTypeStr(pwd->tracePar.getTraceType()));
            itfc->retVar[1].MakeAndSetString(c);
				free(c);
         }
         else if(parameter == "tracewidth")
         { 
            itfc->retVar[1].MakeAndSetFloat(pwd->tracePar.getTraceWidth()/2.0+0.5);
         }
         else if(parameter == "zoombkgcolor")
         { 
            ReturnColour(itfc,pwd->zoomBkgColor,col);
         }
         else if(parameter == "zoombordercolor")
         { 
            ReturnColour(itfc,pwd->zoomBorderColor,col);
         }
	      else if(parameter == "zoomrectmode")
         { 
				itfc->retVar[1].MakeAndSetString(pwd->zoomRectMode.Str()); 
         } 
         else
         {
            ErrorMessage("invalid parameter; %s",parameter);
            return(ERR);
         } 
            
         itfc->nrRetValues = 1;
         return(OK);
      }
   }

// Setting a parameter

	if(parameter == "antialiasing1d")
   { 
      if((r = ConvertAnsVar(valueVar, "antialiasing1d", type, pwd->antiAliasing)) < 0)      
         return(ERR);  
   } 
   else if(parameter == "axescolor")
   { 
      if((r = ConvertAnsVar(valueVar, "axes color", type, pwd->axesColor)) < 0)      
         return(ERR);
   }
   else if(parameter == "axesfontcolor")
   { 
		COLORREF col;
      if((r = ConvertAnsVar(valueVar, "axes font color", type, col)) < 0)      
         return(ERR);
		pwd->ticks->setFontColor(col);
   } 
   else if(parameter == "axesfontname")
   { 
      if((r = ConvertAnsVar(valueVar, "axes font name", type)) < 0)      
         return(ERR); 
      strncpy_s(pwd->axesFontName,FONT_NAME_LENGTH,valueVar->GetString(),_TRUNCATE);
   }
   else if(parameter == "axestype")
   { 
      if((r = ConvertAnsVar(valueVar, "axes type", type)) < 0)      
         return(ERR); 
      if(valueVar->GetType() == UNQUOTED_STRING)
      {
         CText txt = valueVar->GetString();
         if(pwd->SetAxesType(txt))
         {
            ErrorMessage("invalid axes type: (corner/box/cross)");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("axestype parameter should be a string");
         return(ERR);
      }
   }
   else if(parameter == "axesfontsize")
   { 
      short result;
      if((r = ConvertAnsVar(valueVar, "axes font size", type, result)) < 0)      
         return(ERR);   
		pwd->ticks->setFontSize(result);
   }
   else if(parameter == "axesfontstyle")
   {  
      short result;
      if((result = ParseFontStyleStr(valueVar->GetString())) == ERR)
         return(ERR);
		pwd->ticks->setFontStyle(result);
   }
   else if(parameter == "bkgcolor" || parameter == "bkcolor")
   { 
      if((r = ConvertAnsVar(valueVar, "background color", type, pwd->bkColor)) < 0)      
         return(ERR);        
   }      
   else if(parameter == "bordercolor")
   { 
      if((r = ConvertAnsVar(valueVar, "border color", type, pwd->borderColor)) < 0)      
         return(ERR); 
   }
   else if(parameter == "errbarcolor")
   { 
		COLORREF c;
      if((r = ConvertAnsVar(valueVar, "error bar color", type, c)) < 0)      
         return(ERR);  
		pwd->tracePar.setBarColor(c);
   } 
   else if(parameter == "errbarfixed")
   { 
		bool b;
      if((r = ConvertAnsVar(valueVar, "error bar fixed", type, b)) < 0)      
         return(ERR);
		pwd->tracePar.setFixedErrorBars(b);
   } 
   else if(parameter == "errbarshow")
   { 
		bool b;
      if((r = ConvertAnsVar(valueVar, "error bar show", type, b)) < 0)      
         return(ERR);  
		pwd->tracePar.setShowErrorBars(b);
   } 
   else if(parameter == "errbarsize")
   { 
		short s;
      if((r = ConvertAnsVar(valueVar, "error bar size", type, s)) < 0)      
         return(ERR);  
		pwd->tracePar.setBarFixedHeight(s);
   } 
   else if(parameter == "finegridcolor")
   { 
      if((r = ConvertAnsVar(valueVar, "fine-grid color", type, pwd->fineGridColor)) < 0)      
         return(ERR);    
   }
   else if(parameter == "gridcolor")
   { 
      if((r = ConvertAnsVar(valueVar, "grid color", type, pwd->gridColor)) < 0)      
         return(ERR);     
   }
   else if(parameter == "imagcolor")
   { 
		COLORREF c;
      if((r = ConvertAnsVar(valueVar, "imaginary trace color", type, c)) < 0)      
         return(ERR);    
		pwd->tracePar.setImagColor(c);
   } 
   else if(parameter == "imagstyle")
   { 
		short s;
      if(ConvertAnsVar(valueVar, "imaginary trace style", type, s) < 0)      
         return(ERR); 
		pwd->tracePar.setImagStyle(s);
   } 
   else if(parameter == "labelcolor")
   { 
      if((r = ConvertAnsVar(valueVar, "label color", type, pwd->labelFontColor)) < 0)      
         return(ERR);   
   }
   else if(parameter == "labelfontname")
   { 
      if((r = ConvertAnsVar(valueVar, "label font name", type)) < 0)      
         return(ERR);
      strncpy_s(pwd->labelFontName,FONT_NAME_LENGTH,valueVar->GetString(),_TRUNCATE);
   }
   else if(parameter == "labelfontsize")
   { 
      if((r = ConvertAnsVar(valueVar, "label font size", type, pwd->labelFontSize)) < 0)      
         return(ERR);   
   }
   else if(parameter == "labelfontstyle")
   {    
      short result;
      if((result = ParseFontStyleStr(valueVar->GetString())) == ERR)
         return(ERR);
      pwd->labelFontStyle = result;
   }
   else if(parameter == "minorticklength")
   { 
		float minorLength;
      if((r = ConvertAnsVar(valueVar, "minor tick length", type, minorLength)) < 0)      
         return(ERR);
		pwd->ticks->setMinorLength(minorLength);
   }
   else if(parameter == "majorticklength")
   { 
		float majorLength;
      if((r = ConvertAnsVar(valueVar, "major title color", type, majorLength)) < 0)      
         return(ERR);
		pwd->ticks->setMajorLength(majorLength);
   }  
   else if(parameter == "realcolor")
   { 
		COLORREF c;
      if((r = ConvertAnsVar(valueVar, "real trace color", type, c)) < 0)      
         return(ERR);  
		pwd->tracePar.setRealColor(c);
   }
   else if(parameter == "realstyle")
   { 
		short s;
      if(ConvertAnsVar(valueVar, "real trace style", type, s) < 0)      
         return(ERR); 
		pwd->tracePar.setRealStyle(s);
   }
   else if(parameter == "symbolcolor")
   { 
		COLORREF c;
      if((r = ConvertAnsVar(valueVar, "symbol color", type, c)) < 0)      
         return(ERR); 
		pwd->tracePar.setRealSymbolColor(c);
   }
   else if(parameter == "realsymbolcolor")
   { 
		COLORREF c;
      if((r = ConvertAnsVar(valueVar, "real symbol color", type, c)) < 0)      
         return(ERR); 
		pwd->tracePar.setRealSymbolColor(c);
   }
   else if(parameter == "imagsymbolcolor")
   { 
		COLORREF c;
      if((r = ConvertAnsVar(valueVar, "imag symbol color", type, c)) < 0)      
         return(ERR); 
		pwd->tracePar.setImagSymbolColor(c);
   }
   else if(parameter == "symbolshape")
   { 
		if((r = TracePar::GetSymbolTypeCode(valueVar->GetString())) < 0)
         return(ERR);
      pwd->tracePar.setSymbolType(r);
   } 
   else if(parameter == "symbolsize")
   { 
		short s;
      if(ConvertAnsVar(valueVar, "symbol size", type, s) < 0)      
         return(ERR); 
		pwd->tracePar.setSymbolSize(s);
   }
   else if(parameter == "titlecolor")
   { 
      if((r = ConvertAnsVar(valueVar, "title color", type, pwd->titleFontColor)) < 0)      
         return(ERR); 
   } 
   else if(parameter == "titlefontname")
   { 
      if((r = ConvertAnsVar(valueVar, "title font name", type)) < 0)      
         return(ERR);  
      strncpy_s(pwd->titleFontName,FONT_NAME_LENGTH,valueVar->GetString(),_TRUNCATE);
   }
   else if(parameter == "titlefontsize")
   { 
      if((r = ConvertAnsVar(valueVar, "title font size", type, pwd->titleFontSize)) < 0)      
         return(ERR); 
   }
   else if(parameter == "titlefontstyle")
   { 
      short result;
      if((result = ParseFontStyleStr(valueVar->GetString())) == ERR)
         return(ERR);
      pwd->titleFontStyle = result;
   }
   else if(parameter == "tracetype")
   { 
      if((r = TracePar::GetTraceTypeCode(valueVar->GetString())) < 0)
         return(ERR);
      pwd->tracePar.setTraceType(r);
   }
   else if(parameter == "tracewidth")
   { 
		float w;
      if((r = ConvertAnsVar(valueVar, "real trace style", type, w)) < 0)      
         return(ERR); 
		pwd->tracePar.setTraceWidth((w-0.5)*2.0);
   }
   else if(parameter == "zoombkgcolor")
   { 
		COLORREF c;
      if((r = ConvertAnsVar4Color(valueVar, "zoom background color", type, c)) < 0)      
         return(ERR); 
		pwd->zoomBkgColor = c;
   }
   else if(parameter == "zoombordercolor")
   { 
		COLORREF c;
      if((r = ConvertAnsVar4Color(valueVar, "zoom background color", type, c)) < 0)      
         return(ERR); 
		pwd->zoomBorderColor = c;
   }
	else if(parameter == "zoomrectmode")
   { 
      if(type != UNQUOTED_STRING)
      {
         ErrorMessage("invalid type for zoomrectmode");
         return(ERR);
      }

      CText out = valueVar->GetString();

      if(out != "dottedrect" && out != "solidrect")
      {
         ErrorMessage("invalid value for zoomrectmode (dottedrect/solidrect)");
         return(ERR);
      }
      pwd->zoomRectMode = out;
   } 
   else
   {
      ErrorMessage("invalid parameter; %s",parameter);
      return(ERR);
   } 

   itfc->nrRetValues = 0;

   return(1);
}

int ModifyGrid(Interface* itfc, char args[])
{
	Plot* curPlot;
	if(!(curPlot = Plot::curPlot()))
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }

   return(curPlot->ProcessGridParameters(itfc,args));                  
}

int SetBkColor(Interface *itfc, char args[])
{
   Variable colorVar;
   short nrArgs;
	Plot* curPlot = Plot::curPlot();

   float colors[4];
   colors[0] = GetRValue(curPlot->bkColor);
   colors[1] = GetGValue(curPlot->bkColor);
   colors[2] = GetBValue(curPlot->bkColor);
   (&colorVar)->MakeMatrix2DFromVector(colors,4,1);

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,1,"color","e","v",&colorVar)) < 0)
     return(nrArgs); 
 
   if(ConvertAnsVar(&colorVar,"background color", colorVar.GetType(),curPlot->bkColor) == ERR)
      return(ERR);

// Redraw
   MyInvalidateRect(curPlot->win,NULL,false);
   itfc->nrRetValues = 0;

   return(OK); 
}

int SetBorderColor(Interface *itfc, char args[])
{
   Variable colorVar;
   short nrArgs;

	Plot* curPlot = Plot::curPlot();

   float colors[4];
   colors[0] = GetRValue(curPlot->borderColor);
   colors[1] = GetGValue(curPlot->borderColor);
   colors[2] = GetBValue(curPlot->borderColor);
   colors[3] = ((DWORD)(curPlot->borderColor))>>24;
   (&colorVar)->MakeMatrix2DFromVector(colors,4,1);

// Get arguments from user *************
   if((nrArgs = ArgScan(itfc,args,1,"color","e","v",&colorVar)) < 0)
     return(nrArgs); 
 
   if(ConvertAnsVar(&colorVar,"border color", colorVar.GetType(),curPlot->borderColor) == ERR)
      return(ERR);

// Redraw
   MyInvalidateRect(curPlot->win,NULL,false);
   itfc->nrRetValues = 0;
  
   return(OK); 

}


/*****************************************************************************************
*                      Draw a new plot (plot() command)                                  *
*****************************************************************************************/
 
long Plot1D::DrawPlot(Interface *itfc, char arg[])
{
   short			nrArgs,curArg=1;
   CText    	parameter;
   short       type;
   long       ID;
   float       *xdata = 0,*ydata = 0;
   complex		*ycdata;
   long        size;
   CArg        carg;
   Variable    result; 
	Trace*		temp = 0;

// List parameters if none passed ************************************************************************     
   nrArgs = carg.Count((char*)arg);

   if(nrArgs == 0) 
   {
      TextMessage("\n\n   PARAMETER       VALUE\n\n");
      TextMessage("   x, y .......... variables to plot\n");
      TextMessage("               ... followed by trace parameters\n");
      TextMessage("   or\n\n");
      TextMessage("      y .......... variable to plot\n");
      TextMessage("               ... followed by trace parameters\n");
      return(OK);
   }

// Extract 1st argument - should be a 1D matrix
   parameter = carg.Extract(curArg++);
   if((type = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&result)) < 0)
   {
      return(ERR);
   }

// Hide the trace cursor
   HDC hdc = GetDC(this->win);
   this->HideDataCursor(hdc);
   ReleaseDC(this->win,hdc);

// Clear display list unless 'hold' is true
   if(this->displayHold == false)
       this->clearData();

	char traceName[MAX_STR];

// See if first argument is a real row vector i.e. format is:  plot(real,[real/complex],[param]) *************   
   if((type == MATRIX2D || type == DMATRIX2D) && result.GetDimY() == 1) 
   {	   
	   size = result.GetDimX(); 
      if(type == MATRIX2D)
	      xdata = CopyArray(result.GetMatrix2D()[0],size); // Copy data from &result
      else if(type == DMATRIX2D)
	      xdata = CopyDtoFArray(result.GetDMatrix2D()[0],size); // Copy data from &result
	   
	   if(nrArgs > 1) // Get argument 2
	   {
		   parameter = carg.Extract(curArg++);
		   if((type = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&result)) < 0)
         {
				if (xdata)
				{
					FreeVector(xdata);
				}
		      return(ERR);
         }
	   }
	   else // No more arguments
	   {
         // Add default labels
			strncpy_s(traceName,MAX_STR, carg.Extract(1), _TRUNCATE);
			if (!curXAxis()->label().isTextLocked())
			{
				curXAxis()->label().setText("index");
			}
			if (!title().isTextLocked())
			{
				title().setText(carg.Extract(1));
			}

	      type = NULL_VARIABLE;
	   }

   // Argument 2 is not a vector so format is plot(y)
   // Make xdata an index array and ydata = old xdata
	   if(type != MATRIX2D && type != CMATRIX2D && type != DMATRIX2D) 
	   {
	      curArg--;
	      ydata = xdata;
	      xdata = MakeLinearArray(size);
         CText xName = carg.Extract(1);
			strncpy_s(traceName,MAX_STR,xName.Str(),_TRUNCATE);
      // Update axis labels and titles unless they are already occupied.
			if (!curXAxis()->label().isTextLocked())
				curXAxis()->label().setText("index");
			if (!curYAxis()->label().isTextLocked())
				curYAxis()->label().setText(xName.Str());
         if(!title().isTextLocked())
			   title().setText(xName.Str());

			temp = new TraceReal(xdata,ydata,size,traceName,&this->tracePar, dynamic_cast<Plot1D*>(this),false);
		
			// Verify that we aren't trying to stick a trace with negative values on an axis into a plot
			//  that is log on that axis.
			if (!canAddTrace(temp))
			{
            setNoCurTrace();
				ErrorMessage("Cannot add trace with a value <= 0 to a log axis.");
				delete(temp);
			//	plotParent->setBusyWithCriticalSection(false);
				return ERR;
			}
         appendTrace(temp);
         // Copy default plot colours to dataInfo class
		   ID = temp->getID();
         dynamic_cast<Plot1D*>(this)->setCurTrace(temp); // Last trace made in this plot
	   } 

   // Argument 2 is a vector so format is plot(x,y)	   
	   else if((type == MATRIX2D || type == DMATRIX2D)  && result.GetDimY() == 1) 
	   {
         CText xName = carg.Extract(1);
         CText yName = carg.Extract(2);
         CText newTitle;
         newTitle.Format("%s vs %s",yName.Str(),xName.Str());
			strncpy_s(traceName,MAX_STR, yName.Str(),_TRUNCATE);

      // Update axis labels and titles unless they are already occupied.
			if (!curXAxis()->label().isTextLocked())
				curXAxis()->label().setText(xName.Str());
			if (!curYAxis()->label().isTextLocked())
				curYAxis()->label().setText(yName.Str());
         if(!title().isTextLocked())
			   title().setText(newTitle.Str());

	      if(result.GetDimX() == size) // Arrays must be same size
	      {
            if(type == MATRIX2D)
	            ydata = CopyArray(result.GetMatrix2D()[0],result.GetDimX()); // Copy data from &result
            else if(type == DMATRIX2D)
	            ydata = CopyDtoFArray(result.GetDMatrix2D()[0],result.GetDimX()); // Copy data from &result
	      }
	      else
	      {
	         ErrorMessage("Vectors '%s' and '%s' have different sizes",xName.Str(),yName.Str());
	         FreeVector(xdata);
			//	plotParent->setBusyWithCriticalSection(false);
	         return(ERR);
	      }	
			temp = new TraceReal(xdata,ydata,size,traceName,&this->tracePar, dynamic_cast<Plot1D*>(this),false);
					
			// Verify that we aren't trying to stick a trace with negative values on an axis into a plot
			//  that is log on that axis.
			if (!canAddTrace(temp))
			{
				ErrorMessage("Cannot add trace with a value <= 0 to a log axis.");
				delete(temp);
			//	plotParent->setBusyWithCriticalSection(false);
            setNoCurTrace();
				return(ERR);
			}
         appendTrace(temp);
		   ID = temp->getID();		
         dynamic_cast<Plot1D*>(this)->setCurTrace(temp); // Last trace made in this plot
	   }

    // Argument 2 is a complex vector so format is plot(x,yc)	   
	   else if(type == CMATRIX2D && result.GetDimY() == 1) // Is a complex vector
	   {
	      CText xName = carg.Extract(1);
         CText yName = carg.Extract(2);
         CText newTitle;
         newTitle.Format("%s vs %s",yName.Str(),xName.Str());
			strncpy_s(traceName,MAX_STR, yName.Str(), _TRUNCATE);

      // Update axis labels and titles unless they are already occupied.
			if (!curXAxis()->label().isTextLocked())
				curXAxis()->label().setText(xName.Str());
			if (!curYAxis()->label().isTextLocked())
				curYAxis()->label().setText(yName.Str());
         if(!title().isTextLocked())
			   title().setText(newTitle.Str());
	   
	      if(result.GetDimX() == size) // Arrays must be same size
	      {
	         ycdata = CopyCArray(result.GetCMatrix2D()[0],size);
	      }
	      else
	      {
	         ErrorMessage("Vectors '%s' and '%s' have different sizes",xName.Str(),yName.Str());
	         FreeVector(xdata);
				//plotParent->setBusyWithCriticalSection(false);
	         return(ERR);
	      }

			temp = new TraceComplex(xdata,ycdata,size,traceName,&this->tracePar, dynamic_cast<Plot1D*>(this),false);
		
			// Verify that we aren't trying to stick a trace with negative values on an axis into a plot
			//  that is log on that axis.
			if (!canAddTrace(temp))
			{
				ErrorMessage("Cannot add trace with a value <= 0 to a log axis.");
				delete(temp);
			//	plotParent->setBusyWithCriticalSection(false);
            setNoCurTrace();
				return(ERR);
			}
         appendTrace(temp);
         // Copy default plot colours to dataInfo class
		   ID = temp->getID();
         dynamic_cast<Plot1D*>(this)->setCurTrace(temp); // Last trace made in this plot
	   } 
      else
      {
         ErrorMessage("invalid 2nd argument - should be a row vector");
		//	plotParent->setBusyWithCriticalSection(false);
         return(ERR);
      }
	   	
	// Check for further plot parameters (color etc)	
	   if((nrArgs > 1 && curArg == 2) || (nrArgs > 2 && curArg == 3))
	   {
         if(GetOrSetTraceParameters(itfc, &carg, nrArgs, curArg, dynamic_cast<Plot1D*>(this)->curTrace(), dynamic_cast<Plot1D*>(this)->curTrace()->tracePar,this) < 0)
	      {
				dynamic_cast<Plot1D*>(this)->removeTrace(temp);
			//   plotParent->setBusyWithCriticalSection(false);
	         return(ERR);
	      }
	   }
	}
   // See if first argument is a complex row vector i.e. format is:  plot(complex,[param]) **************
	else if(type == CMATRIX2D &&  result.GetDimY() == 1) // Data is complex and 1D
	{
      CText yName = carg.Extract(1);
      
      size = result.GetDimX();
	   
	   xdata = MakeLinearArray(size); // Make real x axis
	   ycdata = CopyCArray(result.GetCMatrix2D()[0],size);
      strncpy_s(traceName,MAX_STR, yName.Str(),_TRUNCATE);

   // Update axis labels and titles unless they are already occupied.
      if(!title().isTextLocked())
		   title().setText(yName.Str());			
		if (!curXAxis()->label().isTextLocked())
			curXAxis()->label().setText("index");
		if (!curYAxis()->label().isTextLocked())
			curYAxis()->label().setText(yName.Str());

		temp = new TraceComplex(xdata,ycdata,size,traceName,&this->tracePar,dynamic_cast<Plot1D*>(this),false);
				
		// Verify that we aren't trying to stick a trace with negative values on an axis into a plot
		//  that is log on that axis.
		if (!canAddTrace(temp))
		{
         ErrorMessage("Cannot add trace with a value <= 0 to a log axis.");
			delete(temp);
		 //  plotParent->setBusyWithCriticalSection(false);
         setNoCurTrace();
			return ERR;
		}
      appendTrace(temp);
		temp->setName("complex data");
	   ID = temp->getID();
      dynamic_cast<Plot1D*>(this)->setCurTrace(temp); // Last trace made in this plot
	   
	   if(nrArgs > 2)
      {
         ErrorMessage("plot parameters are ignored for complex data");
	      FreeVector(xdata);
		//	plotParent->setBusyWithCriticalSection(false);
         return(ERR);
      }	      
	} 
	else
   {
      ErrorMessage("invalid 1st argument - should be a row vector");
	//	plotParent->setBusyWithCriticalSection(false);
      return(ERR);
   }

// Get rid of view mode
	RemoveCharFromString(this->statusText,'V');
	plotParent->clearPlotData(plotParent->getSavedPlotList());

// Remove the offset mode
   RemoveCharFromString(this->statusText,'O');
   this->removeOffset();

// Remove the zoom flag
   this->resetZoomCount(); 
   this->ResetZoomPoint();
   RemoveCharFromString(this->statusText,'Z');

	//if(this->plotParent->updatePlots() && updatePlots_)
	//{
	//	if(plotParent->inCriticalSection()) // This can cause a dead lock without the critical section check
	//	{
	//		LeaveCriticalSection(&criticalSection);
	//		UpdateStatusWindow(this->win, 3, this->statusText);
	//		EnterCriticalSection(&criticalSection);
	//	}
	//	else
	//		UpdateStatusWindow(this->win, 3, this->statusText);
	//}

// Indicate that a new plot has been drawn
	setPlotState("newdata");
	plotParent->modified(true);
   plotParent->DrawPlotTitle();
   
// Draw full plot 
   curXAxis()->setAutorange(true);
   curYAxis()->setAutorange(true);

// Display the plot if desired ****************
   DisplayAll(false);

// Send message if plot updated from CLI
   if(itfc->inCLI)
      SendMessageToGUI("1D Plot,NewPlot",0); 

//	plotParent->setBusyWithCriticalSection(false);
   return(ID);
}



int Plot::ProcessGridParameters(Interface* itfc, char args[])
{
   CText  parameter;
   CText  value;
   short i;
   short type;
   CArg carg;
   Variable parVar;
   Variable valueVar;
   bool extractArg = false;

   short nrArgs = carg.Count(args);

// Prompt user if no arguments supplied 
   if(nrArgs == 0)
   {
      TextMessage("\n\n   PARAMETER     VALUE\n\n");
      TextMessage("   parent ...... (%hd,%hd)\n",this->plotParent->obj->winParent->nr,this->plotParent->obj->nr());
      TextMessage("   xgrid ....... \"%s\"\n",(this->curXAxis()->grid()->drawGrid()) ? "true" : "false");
      TextMessage("   ygrid ....... \"%s\"\n",(this->curXAxis()->grid()->drawGrid()) ? "true" : "false");
      TextMessage("   finexgrid ... \"%s\"\n",(this->curXAxis()->grid()->drawFineGrid()) ? "true" : "false");
      TextMessage("   fineygrid ... \"%s\"\n",(this->curXAxis()->grid()->drawFineGrid()) ? "true" : "false");
      TextMessage("   color ....... %s\n",Plot::GetColorStr(this->curXAxis()->grid()->color()));
      TextMessage("   finecolor ... %s\n",Plot::GetColorStr(this->curXAxis()->grid()->fineColor()));
      return(0);
   }

// Check for valid number of arguments
   if(nrArgs == 1)
   {
      extractArg = true;
      nrArgs = 2;
   }
   else if(nrArgs%2 != 0)
   {
      ErrorMessage("number of arguments must be even");
      return(ERR);
   }
   
// Extract arguments    

   i = 1;
   while(i < nrArgs)
   {
   // Extract parameter name
      parameter = carg.Extract(i);
      if((type = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&parVar)) < 0)
         return(ERR); 
   
      if(type != UNQUOTED_STRING)
      {
         ErrorMessage("invalid data type for parameter '%s'",parameter.Str());
         return(ERR);
      }  
      parameter = parVar.GetString();   

   // Extract parameter value
      value = carg.Extract(i+1);
      float col[4];

   // Return parameter value
      if(extractArg || value == "" || value == "\"\"" || value == "\"getargs\"" || value == "\"getarg\"")
      {
         if(parameter == "parent")
         {
            itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)this);
         }  
         else if(parameter == "xgrid" || parameter == "xgridstatus")
         { 
            if(this->curXAxis()->grid()->drawGrid())
               itfc->retVar[1].MakeAndSetString("on");
            else
               itfc->retVar[1].MakeAndSetString("off");         
         }
         else if(parameter == "ygrid" || parameter == "ygridstatus")
         { 
            if(this->curYAxis()->grid()->drawGrid())
               itfc->retVar[1].MakeAndSetString("on");
            else
               itfc->retVar[1].MakeAndSetString("off"); 
         }
         else if(parameter == "finexgrid" || parameter == "finexgridstatus")
         { 
            if(this->curXAxis()->grid()->drawFineGrid())
               itfc->retVar[1].MakeAndSetString("on");
            else
               itfc->retVar[1].MakeAndSetString("off");  
         }
         else if(parameter == "fineygrid" || parameter == "fineygridstatus")
         { 
            if(this->curYAxis()->grid()->drawFineGrid())
               itfc->retVar[1].MakeAndSetString("on");
            else
               itfc->retVar[1].MakeAndSetString("off");
         }
         else if(parameter == "color" || parameter == "gridcolor")
         {
            ReturnColour(itfc,this->curXAxis()->grid()->color(),col);      
         }
         else if(parameter == "finecolor" || parameter == "finegridcolor")
         { 
            ReturnColour(itfc,this->curXAxis()->grid()->fineColor(),col);
         }   
         else
         {
            ErrorMessage("invalid parameter; %s",parameter);
            return(ERR);
         }
         itfc->nrRetValues = 1;
         return(OK);
      }

      if((type = Evaluate(itfc,RESPECT_ALIAS,value.Str(),&valueVar)) < 0)
         return(ERR);          
      value.RemoveQuotes();
      itfc->nrRetValues = 0; 

   // Process different kinds of parameter    
      if(parameter == "xgrid" || parameter == "xgridstatus")
      { 
			bool draw = false;
         if(ConvertAnsVar(&valueVar, "grid status", type, draw) < 0)
            return(ERR);       
			curXAxis()->grid()->setDrawGrid(draw);
      }
      else if(parameter == "ygrid" || parameter == "ygridstatus")
      { 
			bool draw = false;
         if(ConvertAnsVar(&valueVar, "grid status", type, draw) < 0)
            return(ERR); 
			curYAxis()->grid()->setDrawGrid(draw);
			otherYAxis()->grid()->setDrawGrid(draw);
      }
      else if(parameter == "finexgrid" || parameter == "finexgridstatus" || parameter == "xfinegridstatus")
      { 
			bool draw = false;
         if(ConvertAnsVar(&valueVar, "grid status", type, draw) < 0)
            return(ERR);  
			curXAxis()->grid()->setDrawFineGrid(draw);
      }
      else if(parameter == "fineygrid" || parameter == "fineygridstatus" || parameter == "yfinegridstatus")
      { 
			bool draw = false;
         if(ConvertAnsVar(&valueVar, "grid status", type, draw) < 0)
            return(ERR);
			curYAxis()->grid()->setDrawFineGrid(draw);
			otherYAxis()->grid()->setDrawFineGrid(draw);
      }
      else if(parameter == "color" || parameter == "gridcolor")
      { 
			COLORREF col;
         if(ConvertAnsVar(&valueVar, "grid color", type, col) < 0)
            return(ERR); 
			for(Axis* axis: axisList_)
			{
				axis->grid()->setColor(col);
			}
      }
      else if(parameter == "finecolor" || parameter == "finegridcolor")
      { 
			COLORREF col;
         if(ConvertAnsVar(&valueVar, "fine grid color", type, col) < 0)
            return(ERR);  
			for(Axis* axis: axisList_)
			{
				axis->grid()->setFineColor(col);
			}
      }   
      else
      {
         ErrorMessage("invalid parameter; %s",parameter);
         return(ERR);
      }                                            
      i+=2;
   }
   
	if((this->getDimension() == 1) || (this->getDimension() == 2))
   {
      this->DisplayAll(false); 
   }
          
   return(OK);                     
}

int Plot::ProcessAxesParameters(Interface* itfc, char args[])
{
   CText  parameter;
   CText  value;
   short i;
   short type;
   CArg carg;
   Variable parVar;
   Variable valueVar;
   bool extractArg = false;

   short nrArgs = carg.Count(args);

// Prompt user if no arguments supplied
   if(nrArgs == 0)
   {
      TextMessage("\n\n   PARAMETER     VALUE\n\n");
      TextMessage("   autoscale ....... \"%s\"\n",(this->curXAxis()->autoScale())?"true":"false");
      TextMessage("   axescolor ....... %s\n",Plot::GetColorStr(this->axesColor));
      TextMessage("   fontcolor ....... %s\n",Plot::GetColorStr(this->curXAxis()->ticks().fontColor()));
      TextMessage("   fontname ........ \"%s\"\n",this->curXAxis()->ticks().fontName());
		TextMessage("   fontsize ........ %hd\n",(int)this->curXAxis()->ticks().fontSize());
      TextMessage("   fontstyle ....... \"%s\"\n",GetFontStyleStr(this->curXAxis()->ticks().fontStyle()));
      TextMessage("   lgticksize ...... %g\n",this->curXAxis()->ticks().majorLength());
      TextMessage("   linewidth ....... %hd\n",this->curXAxis()->lineWidth());
      TextMessage("   parent .......... (%hd,%hd)\n",this->plotParent->obj->winParent->nr,this->plotParent->obj->nr());
      TextMessage("   smticksize ...... %g\n",this->curXAxis()->ticks().minorLength());
      TextMessage("   type ............ %s\n",Plot::GetAxesTypeStr());
      TextMessage("   xdirection ...... \"%s\"\n",(this->curXAxis()->plotDirection() == PLT_FORWARD) ? "forward":"reversed");
		TextMessage("   xmapping ........ \"%s\"\n",this->curXAxis()->mapping_s());
      TextMessage("   xppmscale ........ \"%s\"\n",(this->curXAxis()->ppmScale())?"true":"false");
      TextMessage("   xrange .......... [%g,%g]\n",this->curXAxis()->Min(),this->curXAxis()->Max());
      TextMessage("   xtickspacing .... %g\n",this->curXAxis()->ticks().spacing());
      TextMessage("   xticksperlabel .. %g\n",this->curXAxis()->ticks().perLabel());
      TextMessage("   ydirection ...... \"%s\"\n",(this->curYAxis()->plotDirection() == PLT_FORWARD) ? "forward":"reversed");
		TextMessage("   ymapping ........ \"%s\"\n",this->curYAxis()->mapping_s());
      TextMessage("   yppmscale ........ \"%s\"\n",(this->curYAxis()->ppmScale())?"true":"false");
      TextMessage("   yrange .......... [%g,%g]\n",this->curYAxis()->Min(),this->curYAxis()->Max());
      TextMessage("   ytickspacing .... %g\n",this->curYAxis()->ticks().spacing());
      TextMessage("   yticksperlabel .. %g\n",this->curYAxis()->ticks().perLabel());
      itfc->nrRetValues = 0;
      return(0);
   }

// Check for valid number of arguments
   if(nrArgs == 1)
   {
      extractArg = true;
      nrArgs = 2;
   }
   else if(nrArgs%2 != 0)
   {
      ErrorMessage("number of arguments must be even");
      return(ERR);
   }
   
// Extract arguments    
   i = 1;
   while(i < nrArgs)
   {
   
   // Extract parameter name
      parameter = carg.Extract(i);
      if((type = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&parVar)) < 0)
         return(ERR); 
          
      if(type != UNQUOTED_STRING)
      {
         ErrorMessage("invalid data type for parameter '%s'",parameter.Str());
         return(ERR);
      } 
      parameter = parVar.GetString();

   // Extract parameter value
      value = carg.Extract(i+1);
      float col[4];

// Get axes parameter ****************************************************
      if(extractArg || value == "" || value == "\"\"" || value == "\"getargs\"" || value == "\"getarg\"")
      {
         if(parameter == "parent")
         {
            itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)this);
            return(OK);
         } 
         else if(parameter == "axescolor")
         { 
            ReturnColour(itfc,this->axesColor,col);  
         } 
         else if(parameter == "type")
         { 
				itfc->retVar[1].MakeAndSetString(this->GetAxesTypeStr());   
            itfc->nrRetValues = 1;
         }
         else if(parameter == "fontname")
         { 
				itfc->retVar[1].MakeAndSetString(this->curXAxis()->ticks().fontName());   
            itfc->nrRetValues = 1;
         }
         else if(parameter == "fontstyle")
         { 
            itfc->retVar[1].MakeAndSetString(GetFontStyleStr(this->curXAxis()->ticks().fontStyle()));   
            itfc->nrRetValues = 1;
         }
         else if(parameter == "fontcolor")
         { 
            ReturnColour(itfc,this->curXAxis()->ticks().fontColor(),col);     
         } 
         else if(parameter == "fontsize")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curXAxis()->ticks().fontSize());
         } 
         else if(parameter == "xrange")
         { 
            float xrange[2];
            xrange[0] =  this->curXAxis()->Min();
            xrange[1] =  this->curXAxis()->Max();
            itfc->retVar[1].MakeMatrix2DFromVector(xrange,2,1);
         } 
         else if(parameter == "yrange")
         { 
            float yrange[2];
            yrange[0] =  this->curYAxis()->Min();
            yrange[1] =  this->curYAxis()->Max();
            itfc->retVar[1].MakeMatrix2DFromVector(yrange,2,1);
         }              
         else if(parameter == "autoscale")
         { 
            if(this->curXAxis()->autoScale())
               itfc->retVar[1].MakeAndSetString("true");
            else
               itfc->retVar[1].MakeAndSetString("false");
         }  
         else if(parameter == "autorange")
         { 
            if(this->curXAxis()->autoRange())
               itfc->retVar[1].MakeAndSetString("true");
            else
               itfc->retVar[1].MakeAndSetString("false");
         } 
         else if(parameter == "xppmscale")
         { 
            if(this->curXAxis()->ppmScale())
               itfc->retVar[1].MakeAndSetString("true");
            else
               itfc->retVar[1].MakeAndSetString("false");
         }  
         else if(parameter == "yppmscale")
         { 
            if(this->curYAxis()->ppmScale())
               itfc->retVar[1].MakeAndSetString("true");
            else
               itfc->retVar[1].MakeAndSetString("false");
         } 
         else if(parameter == "xdirection")
         { 
            itfc->retVar[1].MakeAndSetString((this->curXAxis()->plotDirection() == PLT_FORWARD) ? "forward":"reversed");
         } 
         else if(parameter == "ydirection")
         { 
            itfc->retVar[1].MakeAndSetString((this->curYAxis()->plotDirection() == PLT_FORWARD) ? "forward":"reversed");
         }
         else if(parameter == "xtickspacing")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curXAxis()->ticks().spacing());
         } 
         else if(parameter == "ytickspacing")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curYAxis()->ticks().spacing());
         } 
         else if(parameter == "xticksperlabel")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curXAxis()->ticks().perLabel());
         } 
         else if(parameter == "yticksperlabel")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curYAxis()->ticks().perLabel());
         } 
         else if(parameter == "smticksize")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curXAxis()->ticks().minorLength());
         }
         else if(parameter == "lgticksize")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curXAxis()->ticks().majorLength());
         } 
         else if(parameter == "minaxisvalue")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curYAxis()->MinIndep());
         } 
         else if(parameter == "origminaxisvalue")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curYAxis()->MinIndepOrig());
         } 
         else if(parameter == "maxaxisvalue")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curYAxis()->MaxIndep());
         } 
         else if(parameter == "xmapping")
         { 
				char mapping[32] = {'\0'};
				strncpy_s(mapping, 32, curXAxis()->mapping_s(), _TRUNCATE);
				itfc->retVar[1].MakeAndSetString(mapping);
         }
         else if(parameter == "ymapping")
         { 
				char mapping[32] = {'\0'};
				strncpy_s(mapping, 32, curYAxis()->mapping_s(), _TRUNCATE);
				itfc->retVar[1].MakeAndSetString(mapping);
         } 
         else if(parameter == "linewidth")
         { 
            itfc->retVar[1].MakeAndSetFloat(this->curXAxis()->lineWidth());
         } 
         else
         {
            ErrorMessage("invalid parameter; %s",parameter);
            return(ERR);
         }
         itfc->nrRetValues = 1;
         return(OK);
      }

// Set axes parameter value ****************************************
      if((type = Evaluate(itfc,RESPECT_ALIAS,value.Str(),&valueVar)) < 0)
         return(ERR);          
   
      itfc->nrRetValues = 0; 
      value.RemoveQuotes();

   // Process different kinds of parameter    
      if(parameter == "axescolor")
      { 
         if((ConvertAnsVar(&valueVar,"axes color", type, this->axesColor)) < 0)
            return(ERR);   
      } 
      else if(parameter == "type")
      {
         if(type == UNQUOTED_STRING)
         {
            CText type = valueVar.GetString();
            SetAxesType(type);
            WinData *win = plotParent->obj->winParent;
			   const char* const tbn = plotParent->toolbarName();
            win->setToolBarItemCheck(tbn, "corner_axes", axesMode == PLOT_AXES_CORNER);
            win->setToolBarItemCheck(tbn, "border_axes", axesMode == PLOT_AXES_BOX);
            win->setToolBarItemCheck(tbn, "crossed_axes", axesMode == PLOT_AXES_CROSS);
         }
         else
         {
            ErrorMessage("invalid type");
            return(ERR);
         }
      }
      else if(parameter == "fontcolor")
      { 
			COLORREF col;
         if((ConvertAnsVar(&valueVar, "font color", type, col)) < 0)
            return(ERR);   
			for(Axis* axis: axisList_)
			{
				axis->ticks().setFontColor(col);
			}
      } 
      else if(parameter == "fontname")
      { 
         if(ConvertAnsVar(&valueVar, "font name", type) < 0)
            return(ERR);
			for(Axis* axis: axisList_)
			{
				axis->ticks().setFontName(valueVar.GetString());
			}
      }
      else if(parameter == "fontsize")
      { 
			short fontSize = 0;
         if(ConvertAnsVar(&valueVar, "font size", type, fontSize) < 0)
            return(ERR);
			for(Axis* axis: axisList_)
			{
				axis->ticks().setFontSize(fontSize);
			}
      } 
      else if(parameter == "fontstyle")
      {
         int result;
         if(ConvertAnsVar(&valueVar, "font style", type) < 0)
            return(ERR);
         if((result = ParseFontStyleStr(valueVar.GetString())) == ERR)
            return(ERR);
			for(Axis* axis: axisList_)
			{
				axis->ticks().setFontStyle(result);
			}
      }
      else if(parameter == "xrange")
      { 
         if(this->getDimension() == 2)
         {
            this->curXAxis()->setMin(valueVar.GetMatrix2D()[0][0]);  
            this->curXAxis()->setMax(valueVar.GetMatrix2D()[0][1]);

            long width = dynamic_cast<Plot2D*>(this)->matWidth();

            long xMin = this->curXAxis()->userToData(this->curXAxis()->Min());
            if(xMin < 0 || xMin >= width) 
				{
			   //   TextMessage("\n   Warning - xrange lower limit out of range - resetting\n");
               xMin = 0; 
				}
            long xMax = this->curXAxis()->userToData(this->curXAxis()->Max());

            if(xMax >= width || xMax <= 0)
				{
               xMax = width-1;
			   //   TextMessage("\n   Warning - xrange upper limit out of range - resetting\n");
				}

            if(xMin >= xMax)
            {
               xMin = 0;
               xMax = width-1;
				//	TextMessage("\n   Warning - xrange limits reversed - resetting\n");
            }

            dynamic_cast<Plot2D*>(this)->setVisibleLeft(xMin);
            dynamic_cast<Plot2D*>(this)->setVisibleWidth(xMax-xMin);
            this->curXAxis()->setAutorange(false);
         }
         else
         {
				curXAxis()->setMin(valueVar.GetMatrix2D()[0][0]);  
            curXAxis()->setMax(valueVar.GetMatrix2D()[0][1]);
				curXAxis()->setAutorange(false);
         }
      } 
      else if(parameter == "yrange")
      { 
         if(this->getDimension() == 2)
         {
            this->curYAxis()->setMin(valueVar.GetMatrix2D()[0][0]);  
            this->curYAxis()->setMax(valueVar.GetMatrix2D()[0][1]);

            long height = dynamic_cast<Plot2D*>(this)->matHeight();

            long yMin = this->curYAxis()->userToData(this->curYAxis()->Min());
            if(yMin < 0 || yMin >= height) 
				{
			//		TextMessage("\n   Warning - yrange lower limit out of range - resetting\n");
					yMin = 0;    
				}
            long yMax = this->curYAxis()->userToData(this->curYAxis()->Max());
            if(yMax >= height || yMax <= 0)
				{
				//	TextMessage("\n   Warning - yrange upper limit out of range - resetting\n");
               yMax = height-1;
				}

            if(yMin >= yMax)
            {
				//	TextMessage("\n   Warning - yrange limits reversed - resetting\n");
               yMin = 0;
               yMax = height-1;
            }

            dynamic_cast<Plot2D*>(this)->setVisibleTop(yMin);
            dynamic_cast<Plot2D*>(this)->setVisibleHeight(yMax-yMin);
       
            this->curYAxis()->setAutorange(false);
         }
         else
         {
				curYAxis()->setMin(valueVar.GetMatrix2D()[0][0]);  
            curYAxis()->setMax(valueVar.GetMatrix2D()[0][1]);
				curYAxis()->setAutorange(false);
         }
      }              
      else if(parameter == "autoscale")
      { 
         if(valueVar.GetType() == UNQUOTED_STRING)
         {
				bool response;
            if(StrTrueCheck(valueVar.GetString(),response) < 0)
               return(ERR);
				for(Axis* axis: axisList())
				{
					axis->setAutoscale(response);
				}
         }
         else
         {
            ErrorMessage("invalid parameter value type for 'autoscale'");
            return(ERR);
         }
      }
      else if(parameter == "autorange")
      { 
         if(valueVar.GetType() == UNQUOTED_STRING)
         {
				bool response;
            if(StrTrueCheck(valueVar.GetString(),response) < 0)
               return(ERR);
				for(Axis* axis: axisList())
				{
					axis->setAutorange(response);
				}
         }
         else
         {
            ErrorMessage("invalid parameter value type for 'autorange'");
            return(ERR);
         }
      }
      else if(parameter == "xppmscale")
      {
         if(valueVar.GetType() == UNQUOTED_STRING)
         {
            if(!strcmp(valueVar.GetString(),"true"))
               this->curXAxis()->setPPMScale(true);
            else if(!strcmp(valueVar.GetString(),"false"))
               curXAxis()->setPPMScale(false);
            else
            {
               ErrorMessage("invalid xppmscale string (true,false)");
               return(ERR);
            }
         }
         else
         {
            ErrorMessage("invalid parameter value type for 'xppmscale'");
            return(ERR);
         }
      }
      else if(parameter == "yppmscale")
      {
         if(valueVar.GetType() == UNQUOTED_STRING)
         {
            if(!strcmp(valueVar.GetString(),"true"))
               this->curYAxis()->setPPMScale(true);
            else if(!strcmp(valueVar.GetString(),"false"))
               curYAxis()->setPPMScale(false);
            else
            {
               ErrorMessage("invalid yppmscale string (true,false)");
               return(ERR);
            }
         }
         else
         {
            ErrorMessage("invalid parameter value type for 'yppmscale'");
            return(ERR);
         }
      }
      else if(parameter == "xdirection")
      {
         if(valueVar.GetType() == UNQUOTED_STRING)
         {
            if(!strcmp(valueVar.GetString(),"forward"))
               curXAxis()->setDirection(PLT_FORWARD);
            else if(!strcmp(valueVar.GetString(),"reversed"))
               curXAxis()->setDirection(PLT_REVERSE);
            else
            {
               ErrorMessage("invalid direction string (forward,reversed)");
               return(ERR);
            }
         }
         else
         {
            ErrorMessage("invalid parameter value type for 'direction'");
            return(ERR);
         }
      }
      else if(parameter == "ydirection")
      {
         if(valueVar.GetType() == UNQUOTED_STRING)
         {
            if(!strcmp(valueVar.GetString(),"forward"))
               curYAxis()->setDirection(PLT_FORWARD);
            else if(!strcmp(valueVar.GetString(),"reversed"))
               curYAxis()->setDirection(PLT_REVERSE);
            else
            {
               ErrorMessage("invalid direction string (forward,reversed)");
               return(ERR);
            }
         }
         else
         {
            ErrorMessage("invalid parameter value type for 'direction'");
            return(ERR);
         }
      }
      else if(parameter == "xtickspacing")
      { 
			float spacing;
         if(ConvertAnsVar(&valueVar, parameter.Str(), type, spacing) < 0)
            return(ERR);
			this->getXTicks().setSpacing(spacing);
         this->curXAxis()->setAutoscale(false);
      } 
      else if(parameter == "ytickspacing")
      { 
			float spacing;
         if(ConvertAnsVar(&valueVar,parameter.Str(), type, spacing) < 0)
            return(ERR);
			this->getYTicks().setSpacing(spacing);
         this->curYAxis()->setAutoscale(false);
      }
      else if(parameter == "xticksperlabel")
      { 
			float perLabel;
         if(ConvertAnsVar(&valueVar, parameter.Str(),type, perLabel) < 0)
            return(ERR);
			this->getXTicks().setPerLabel(perLabel);
         this->curXAxis()->setAutoscale(false);
      } 
      else if(parameter == "yticksperlabel")
      { 
			float perLabel;
         if(ConvertAnsVar(&valueVar,parameter.Str(), type, perLabel) < 0)
            return(ERR);
         this->getYTicks().setPerLabel(perLabel);
         this->curYAxis()->setAutoscale(false);
      }  
      else if(parameter == "linewidth")
      { 
			float width;
         if(ConvertAnsVar(&valueVar, parameter.Str(),type, width) < 0)
            return(ERR);
         this->curXAxis()->setLineWidth(nsint(width));
         this->yAxisLeft()->setLineWidth(nsint(width));
         this->yAxisRight()->setLineWidth(nsint(width));
      } 
      else if(parameter == "minaxisvalue")
      { 
			float value;
         if(ConvertAnsVar(&valueVar, parameter.Str(),type, value) < 0)
            return(ERR);
         this->yAxisLeft()->setMinIndepOrig(value);
         this->yAxisLeft()->setMinIndep(value);
      } 
      else if(parameter == "maxaxisvalue")
      { 
			float value;
         if(ConvertAnsVar(&valueVar, parameter.Str(),type, value) < 0)
            return(ERR);
         this->yAxisLeft()->setMaxIndep(value);
      } 
      else if(parameter == "smticksize")
      { 
			float xTickLength;
         if(ConvertAnsVar(&valueVar, "tick size", type, xTickLength) < 0)
            return(ERR);
			this->getXTicks().setMinorLength(xTickLength);
			float yTickLength;
         if(ConvertAnsVar(&valueVar, "tick size", type, yTickLength) < 0)
            return(ERR);
			this->getYTicks().setMinorLength(yTickLength);
      }
      else if(parameter == "lgticksize")
      { 
			float xTickLength;
         if(ConvertAnsVar(&valueVar, "tick size", type, xTickLength) < 0)
            return(ERR);
			this->getXTicks().setMajorLength(xTickLength);

			float yTickLength;
         if(ConvertAnsVar(&valueVar, "tick size", type, yTickLength) < 0)
            return(ERR);
			this->getYTicks().setMajorLength(yTickLength);
      } 
      else if(parameter == "xmapping")
      { 
		   if(type != UNQUOTED_STRING)
		   {
		      ErrorMessage("invalid type for mapping");
		      return(ERR);
		   }
         WinData *win = plotParent->obj->winParent;
			const char* const tbn = plotParent->toolbarName();
         if(!strncmp(valueVar.GetString(),"log",3))
         {
            setAxisMapping(PLOT_LOG_X);
            win->setToolBarItemCheck(tbn, "linear_axis", false);
            win->setToolBarItemCheck(tbn, "log_axis", true);
         }
         else if(!strncmp(valueVar.GetString(),"lin",3))
         {
            setAxisMapping(PLOT_LINEAR_X);
            win->setToolBarItemCheck(tbn, "linear_axis", true);
            win->setToolBarItemCheck(tbn, "log_axis", false);
         }
      }
      else if(parameter == "ymapping")
      { 
		   if(type != UNQUOTED_STRING)
		   {
		      ErrorMessage("invalid type for mapping");
		      return(ERR);
		   }
         
         if(!strncmp(valueVar.GetString(),"log",3))
            setAxisMapping(PLOT_LOG_Y);
         else if(!strncmp(valueVar.GetString(),"lin",3))
            setAxisMapping(PLOT_LINEAR_Y);
      }   
      else
      {
         ErrorMessage("invalid parameter; %s",parameter);
         return(ERR);
      }                                           
      i+=2;
   }
   
   if((this->getDimension() == 1) || (this->getDimension() == 2))
   {
      this->DisplayAll(false); 
   }
 
   return(0);                     
}


short Plot::ProcessLabelParameters(Interface *itfc, char *which, char *args)
{
   CText  parameter;
   CText  value;
   char  out[MAX_STR];
   CArg carg;
   Variable parVar;
   Variable valueVar;

   short nrArgs = carg.Count(args);

   float col[4];

// Prompt user if no arguments supplied   
   if(nrArgs == 0)
   {
      TextMessage("\n\n   PARAMETER     VALUE\n\n");
      TextMessage("   parent .... (%hd,%hd)\n",this->plotParent->obj->winParent->nr,this->plotParent->obj->nr());
      if(!strcmp(which,"title"))
         TextMessage("   text ...... \"%s\"\n",this->title().text());
      else if(!strcmp(which,"xlabel"))
         TextMessage("   text ...... \"%s\"\n",this->curXAxis()->label().text());
      else if(!strcmp(which,"ylabel"))
         TextMessage("   text ...... \"%s\"\n",this->curYAxis()->label().text());
      else if(!strcmp(which,"ylabelleft"))
         TextMessage("   text ...... \"%s\"\n",this->yAxisL_->label().text());
      else if(!strcmp(which,"ylabelright"))
         TextMessage("   text ...... \"%s\"\n",this->yAxisR_->label().text());
      TextMessage("   size ...... %d\n",(int)this->title().fontSize());
      TextMessage("   font ...... \"%s\"\n",this->title().font().lfFaceName);
      TextMessage("   color ..... %s\n",Plot::GetColorStr(this->title().fontColor()));
      TextMessage("   style ..... \"%s\"\n",GetFontStyleStr(this->title().fontStyle()));
      return(0);
   }

	PlotLabel* myLabel = 0;

   if(!strcmp(which,"title"))
   {
		myLabel = &(title());
		myLabel->userHasSetText(true);
   }
   else if(!strcmp(which,"xlabel"))
   {
		myLabel = &(curXAxis()->label());
		myLabel->userHasSetText(true);
   }
   else if(!strcmp(which,"ylabel"))
   {
		myLabel = &(curYAxis()->label());
		myLabel->userHasSetText(true);
	}
	else if (!strcmp(which, "ylabelleft"))
	{
		myLabel = &(yAxisL_->label());
		myLabel->userHasSetText(true);
	}
	else if (!strcmp(which, "ylabelright"))
	{
		myLabel = &(yAxisR_->label());
		myLabel->userHasSetText(true);
	}
// Assume title has been passed alone
   if(nrArgs == 1) 
   {
      if((nrArgs = ArgScan(itfc,args,1,"label","e","s",myLabel->text())) < 0)
		{
         return(nrArgs); 
		}
		myLabel->userHasSetText(true);
      itfc->nrRetValues = 0;
   }
   
// Parameter pairs passed
   
   else 
   {
	// Check for valid number of arguments
	   if(nrArgs%2 != 0)
	   {
	      ErrorMessage("number of arguments must be even");
	      return(ERR);
	   }
	   
	// Extract arguments    
		short i = 1;
	   while(i < nrArgs)
	   {
      // Extract parameter name
         parameter = carg.Extract(i);
			short type;
         if((type = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&parVar)) < 0)
            return(ERR); 
	      
	      if(type != UNQUOTED_STRING)
	      {
	         ErrorMessage("invalid data type for parameter '%s'",parameter.Str());
	         return(ERR);
	      }  
         parameter = parVar.GetString();

      // Extract parameter value
         value = carg.Extract(i+1);

      // Return parameter value
         if(value == "" || value == "\"\"" || value == "\"getargs\"" || value == "\"getarg\"")
         {
            if(parameter == "parent")
            {
               itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)this);
               return(OK);
            }  
	         else if(parameter == "text")
	         {
               itfc->retVar[1].MakeAndSetString(myLabel->text());
  	         }
	         else if(parameter == "color")
	         {
               ReturnColour(itfc,myLabel->fontColor(),col);
	         }
            else if(parameter == "size")
            {
               itfc->retVar[1].MakeAndSetFloat(myLabel->fontSize());
            }      
            else if(parameter == "style")
            {
					itfc->retVar[1].MakeAndSetString(GetFontStyleStr(myLabel->fontStyle()));   
               itfc->nrRetValues = 1;
	         }
	         else if(parameter == "font")
	         {
               itfc->retVar[1].MakeAndSetString(myLabel->font().lfFaceName);
	         }
            else
            {
               ErrorMessage("invalid parameter '%s'",parameter.Str());
               return(ERR);
            }

            itfc->nrRetValues = 1;
            return(OK);
         }   

         if((type = Evaluate(itfc,RESPECT_ALIAS,value.Str(),&valueVar)) < 0)
            return(ERR); 

         value.RemoveQuotes();
	
	   // Process different kinds of parameter    
	      strncpy_s(out,MAX_STR,valueVar.GetString(),_TRUNCATE);
         char replace[2];
	      replace[0] = (char)0xB1; replace[1] = '\0';
	      ReplaceSpecialCharacters(out,"+-",replace,-1);
	      ReplaceSpecialCharacters(out,"\\'","'",-1);	
         
         if(parameter == "text")
	      {
				myLabel->setText(out);
  	      }
	      else if(parameter == "color")
	      {
				COLORREF col;
				if((ConvertAnsVar(&valueVar,"font color", type, col)) == ERR)
               return(ERR);
				myLabel->setFontColor(col);
	      }
	      else if(parameter == "size")
	      {
				short size = 0;
				if(ConvertAnsVar(&valueVar,"font size", type, size) == ERR)
               return(ERR);
				myLabel->setFontSize(size);
            HDC hDC = GetDC(this->win);
            myLabel->font().lfHeight = nint(-(myLabel->fontSize()) *  GetDeviceCaps(hDC, LOGPIXELSY)/ 72.0);
            ReleaseDC(this->win,hDC);
	      }      
	      else if(parameter == "style")
	      {
            int result;
            if(ConvertAnsVar(&valueVar, "font style", type) < 0)
               return(ERR);
            if((result = ParseFontStyleStr(valueVar.GetString())) == ERR)
               return(ERR);
				myLabel->setFontStyle(result);
				myLabel->font().lfItalic = result & FONT_ITALIC;
				myLabel->font().lfWeight = ((result & FONT_BOLD)>0)*FW_BOLD + ((result & FONT_BOLD)==0)*FW_NORMAL;
				myLabel->font().lfUnderline = (BYTE)(result & FONT_UNDERLINE);                             
	      } 
	      else if(parameter == "font")
	      {
				strncpy_s(myLabel->font().lfFaceName, LF_FACESIZE, out, _TRUNCATE);
            myLabel->setFontName(out);
	      } 
	      i+=2;
	   }   
	}     

   if((this->getDimension() == 1) || (this->getDimension() == 2))
   {
      this->DisplayAll(false); 
   }
   else
   {
      ErrorMessage("invalid dimension passed to set ylabel");
      return(ERR);
   }

   itfc->nrRetValues = 0;
     
   return(0);
}


/************************************************************************
* Return the plot vector data
*************************************************************************/

short Plot1D::GetData(Interface *itfc, short mode)
{
	if(hasNoCurTrace())
   {
      ErrorMessage("no trace present");
      return(ERR);
   }
	Variable* xAsVar = 0;
	Variable* yAsVar = 0;
   if(mode == 1)
   {		
		currentTraceAsVariables(&xAsVar, &yAsVar);
   }
   else
	{
		currentTraceMinMaxAsVariables(&xAsVar, &yAsVar);
	}
	if (xAsVar && yAsVar)
	{
		itfc->nrRetValues = 2;
		itfc->defaultVar = 2;
		itfc->retVar[1].Assign(xAsVar);
		itfc->retVar[2].Assign(yAsVar);
		xAsVar->NullData();
		delete xAsVar;
		yAsVar->NullData();
		delete yAsVar;
		return(OK);
	}
	return (ERR);
}

// Zoom into the 1D plot or return current zoom limits

short Plot1D::Zoom(Interface* itfc, char args[])
{
   short nrArg;

	float lminx = curXAxis()->Min();
	float lmaxx = curXAxis()->Max();
	float lminy = curYAxis()->Min();
	float lmaxy = curYAxis()->Max();

   if((nrArg = ArgScan(itfc,args,2,"minx,maxx,[miny,maxy]","eeee","ffff",&lminx,&lmaxx,&lminy,&lmaxy)) < 0)
      return(nrArg);

	curXAxis()->setMin(lminx);
   if(!isnan(lminy))
	   curYAxis()->setMin(lminy);
	curXAxis()->setMax(lmaxx);
   if(!isnan(lmaxy))
	   curYAxis()->setMax(lmaxy);

// Check for errors
   if(curXAxis()->Min() >= curXAxis()->Max())
   {
      ErrorMessage("invalid horizontal range");
      return(ERR);
   }

   if(nrArg == 4 && (curYAxis()->Max() <= curYAxis()->Min()))
   {
      ErrorMessage("invalid vertical range");
      return(ERR);
   }

// Update select rect parameters
   selectRect.left   = curXAxis()->Min();
   selectRect.right  = curXAxis()->Max();
   selectRect.top    = curYAxis()->Min();
   selectRect.bottom = curYAxis()->Max();
   rectSelected = true;
   
   if(nrArg == 2)
   {
	   curXAxis()->setAutorange(false);
	   curYAxis()->setAutorange(true);
	}	
	else if(nrArg == 4)
	{
	   curXAxis()->setAutorange(false);
	   curYAxis()->setAutorange(false);
   }

	short r = ZoomRegion();
	updateStatusWindowForZoom(win);

   this->DisplayAll(false); 

   return(r);
}


int SetOrGetCurrentAxis(Interface* itfc, char args[])
{
	
	CArg carg;
	short nrArgs = carg.Count(args);
	Plot1D* plot = Plot1D::curPlot();

	if (nrArgs == 0)
	{
		if (plot)
		{
			if (plot->currentVerticalAxis() == RIGHT_VAXIS_SIDE)
			{
				itfc->retVar[1].MakeAndSetString("right"); 
			}
			else
			{
				itfc->retVar[1].MakeAndSetString("left"); 
			}	
		}	
		itfc->nrRetValues = 1;
	}
	if (nrArgs == 1)
	{
		Variable var;
		CText name;
		short r;
		if((r = ArgScan(itfc,args,1,"axis","e","v",&var)) < 0)
			return(r); 
		if(var.GetType() == UNQUOTED_STRING)
		{
			name = var.GetString();
			if (name == "left")
			{
				plot->setCurYAxis(LEFT_VAXIS_SIDE);
			}
			else if (name == "right")
			{
				plot->setCurYAxis(RIGHT_VAXIS_SIDE);
			}
		}
		itfc->nrRetValues = 0;
	}
	return(0);	
}


int SetOrGetSyncAxes (Interface* itfc, char args[])
{
	CArg carg;
	short nrArgs = carg.Count(args);
	Plot1D* plot = Plot1D::curPlot();

	if (nrArgs == 0)
	{
		if (plot)
		{
			if (plot->syncAxes())
			{
				itfc->retVar[1].MakeAndSetString("true"); 
			}
			else
			{
				itfc->retVar[1].MakeAndSetString("false"); 
			}	
		}	
		itfc->nrRetValues = 1;
	}
	if (nrArgs == 1)
	{
		Variable var;
		CText val;
		short r;
		if((r = ArgScan(itfc,args,1,"synch","e","v",&var)) < 0)
			return(r); 
		if(var.GetType() == UNQUOTED_STRING)
		{
			val = var.GetString();
			if (val == "true")
			{
				plot->setSyncAxes(true);
			}
			else 
			{
				plot->setSyncAxes(false);
			}
		}
		itfc->nrRetValues = 0;
	}
	return(0);	
}
