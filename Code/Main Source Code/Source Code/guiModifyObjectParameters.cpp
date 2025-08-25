#pragma pack(push, 8)
// Must not use 1 byte packing for ACCEL structure
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "guiModifyObjectParameters.h"
#include "allocate.h"
#include "BabyGrid.h"
#include "cArg.h"
#include "edit_class.h"
#include "evaluate.h"
#include "dividers.h"
#include "files.h"
#include "globals.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiMakeObjects.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "htmlviewer.h"
#include "interface.h"
#include "list_functions.h"
#include "listbox.h"
#include "main.h"
#include "mymath.h"
#include "plot.h"
#include "plot1dCLI.h"
#include "plot2dEvents.h"
#include "plot3dClass.h"
#include "plot3dSurface.h"
#include "PlotWindow.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "tab.h"
#include "variablesOther.h"
#include "syntax_viewer.h"
#include "edit_utilities.h"
#include "memoryLeak.h"

using namespace Gdiplus;

#pragma warning (disable: 4996) // Ignore deprecated library functions

int GetCheckBoxParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetColorBoxParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetMessageParameter(ObjectData *obj, CText &parameter, Interface *itfc);
int GetProgressBarParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetRadioButtonParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetStaticTextParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetTextMenuParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans);
int GetHTMLWinParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans);
int GetUpDownParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetDividerParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetMenuParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetTextEditorParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans);
int GetTabParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetPlotWinParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetImageWinParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetGridCtrlParameter(ObjectData *obj, CText &parameter, Variable *ans);

int SetGridCtrlParameter(ObjectData* obj, CText &parameter, Variable *ans);
int SetButtonParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetCheckBoxParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetColorBoxParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetTextMenuParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetHTMLWinParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetImageWinParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetPictureParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetPlotWinParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetProgressBarParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetRadioButtonParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetSliderParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetStaticTextParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetStatusBoxParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetTextBoxParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetUpDownParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetTextEditorParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetDebugStripParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetCLIParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetOpenGLParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetDividerParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetTabParameter(ObjectData *obj, CText &parameter, Variable *ans);
int SetMenuParameter(ObjectData *obj, CText &parameter, Variable *value);
int SetToolBarParameter(ObjectData *obj, CText &parameter, Variable *value);

void ReorientRadioButtons(ObjectData *obj, RadioButtonInfo *info);
void ReorientUpDownControl(ObjectData *obj, char *orient);
bool SetStaticTextXExp(ObjectData *obj, Variable *value);
void ReplaceBackGroundPixels(HBITMAP pix);

int GetParameterCore(Interface *itfc, ObjectData* obj, CText &parameter);
int SetParameterCore(Interface *itfc, ObjectData* obj, CText &parameterIn, CText &valueIn);

/**************************************************************************************
  Reposition/resize all controls in a window as if the window had been resized

  winNr ... the window number (0 is current gui window)
**************************************************************************************/

int AdjustObjects(Interface *itfc, char *args)
{
	short nrArgs;
   WinData *win;
   short winNr,objNr;
   ObjectData *obj;
   Variable objVar;
   extern void ResizeObjectsWhenParentResizes(WinData *win, ObjectData *obj, HWND hWnd, short x, short y);


// Get window and object numbers **************
   if((nrArgs = ArgScan(itfc,args,1,"window","e","d",&winNr)) < 0)
    return(nrArgs);
   
// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// Find window instance ************************
	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%ld' not found",(long)winNr);
	   return(ERR);
	}

// Resize and reposition objects based on 
   ResizeObjectsWhenParentResizes(win,NULL,NULL,0,0);
   InvalidateRect(win->hWnd,NULL,false);

   return(OK);

}

/**************************************************************************************
  Search for an object in a specified window returning the object number

  window ... the window number (0 is current gui window)
  method ... must be "name" at present
  value .... the name of the control/object (as stored in the obj->valueName variable)
**************************************************************************************/

int FindObjectCLI(Interface *itfc, char args[])
{
   short nrArgs;
   static short  winNr;
   WinData *win = 0;
   ObjectData *obj = 0;
   CText method;
   CText value;
   
   if((nrArgs = ArgScan(itfc,args,3,"window, method, value","eee","dtt",&winNr,&method,&value)) < 0)
     return(nrArgs);
   
/// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%d' not found",winNr);
	   return(ERR);
   }

// Search for the object by name
   if(method == "name" || method == "valueID")
   {
      obj = win->FindObjectByValueID(value.Str());
      if(obj)
         itfc->retVar[1].MakeAndSetFloat((float)obj->nr());
      else
         itfc->retVar[1].MakeAndSetFloat((float)-1);
   }
   else if(method == "objID")
   {
      obj = win->FindObjectByObjectID(value.Str());
      if(obj)
         itfc->retVar[1].MakeAndSetFloat((float)obj->nr());
      else
         itfc->retVar[1].MakeAndSetFloat((float)-1);
   }
// Search for object by type
   else if(method == "type")
   {
      short type = obj->GetTypeAsNumber(value);
      obj = win->FindObjectByType(type);
      if(obj)
         itfc->retVar[1].MakeAndSetFloat((float)obj->nr());
      else
         itfc->retVar[1].MakeAndSetFloat((float)-1);
   }
   else
   {
      ErrorMessage("invalid method options = [\"name\"]");
      return(ERR);
   }

   itfc->nrRetValues = 1;
   
   return(OK);
}
      
   
/***************************************************************************
*
*                          Get an object parameter.
*
*  Syntax: parameter_value = getpar(window_nr, oject_nr, object_parameter)
*
****************************************************************************/
    
int GetParameter(Interface *itfc, char arg[])
{
   short nrArgs;
   static short  winNr,objNr;
   CText parameter = "name";
   WinData *win = 0;
   ObjectData *obj = 0;
   Variable objVar;
   
   if((nrArgs = ArgScan(itfc,arg,3,"window,object,parameter","eee","dvt",&winNr,&objVar,&parameter)) < 0)
    return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%d' not found",winNr);
	   return(ERR);
   }
   
// Find control instance ***********************
   if(objVar.GetType() == FLOAT32) // By number
   {
      objNr = nint(objVar.GetReal());
      if(!(obj = win->widgets.findByNr(objNr)))
      {
         ErrorMessage("object '%ld' not found",(long)objNr);
         return(ERR);
      }
   }
   else if(objVar.GetType() == UNQUOTED_STRING) // By valueID or objectID
   {
      if(!(obj = win->widgets.findByValueID(objVar.GetString())))
      {
         if(!(obj = win->widgets.findByObjectID(objVar.GetString())))
         {
            ErrorMessage("object '%s' not found",objVar.GetString());
            return(ERR);
         }
      }
      objNr = obj->nr();
   }

   return(GetParameterCore(itfc,obj,parameter));

}

int GetParameterCore(Interface *itfc, ObjectData* obj, CText &parameter)
{
   Variable *ans = &itfc->retVar[1];
   int err=OK;

   parameter.LowerCase();

// Generic parameters **************************
   if(parameter == "parent")
   {
      ans->MakeClass(WINDOW_CLASS,(void*)obj->winParent); 
   }
   else if(parameter == "menubar")
   {
      int sz = obj->menuListSize;
      if(sz > 0)
      {
         float* list = new float[sz]; 
         for(int k = 0; k < sz; k++)
            list[k] = obj->menuList[k];
         ans->MakeMatrix2DFromVector(list,sz,1); 
         delete [] list;
      }
      else
         ans->MakeAndSetFloat(-1); 
   }
	else if(parameter == "dragndropproc")
	{  
      ans->MakeAndSetString(obj->dragNDropProc.Str());
   }
   else if(parameter == "enable")
   {
      bool state = IsWindowEnabled(obj->hWnd);
      if(state)
         ans->MakeAndSetString("true");         
      else
         ans->MakeAndSetString("false");         
   }
   else if(parameter == "active")
   {
      if(obj->active)
         ans->MakeAndSetString("true");         
      else
         ans->MakeAndSetString("false");         
   }
   else if(parameter == "highlitechanges")
   {
      if(obj->highLiteChanges)
         ans->MakeAndSetString("true");         
      else
         ans->MakeAndSetString("false");         
   }
   else if(parameter == "valuechanged")
   {
      if(obj->valueChanged)
         ans->MakeAndSetString("true");         
      else
         ans->MakeAndSetString("false");         
   }
   else if(parameter == "readonly")
   {
      if(obj->readOnly)
         ans->MakeAndSetString("true");         
      else
         ans->MakeAndSetString("false");         
   }
   else if(parameter == "readonlyoutput")
   {
      if(obj->readOnlyOutput)
         ans->MakeAndSetString("true");         
      else
         ans->MakeAndSetString("false");         
   }
   else if(parameter == "keepfocus")
   {
      if(obj->keepFocus)
         ans->MakeAndSetString("true");         
      else
         ans->MakeAndSetString("false");         
   }
   else if(parameter == "eventactive")
   {
      if(obj->eventActive)
         ans->MakeAndSetString("true");         
      else
         ans->MakeAndSetString("false");         
   }
   else if(parameter == "visible")
   {
      if(obj->type == RADIO_BUTTON)
      {
	      RadioButtonInfo* info = (RadioButtonInfo*)obj->data;
         if(info->nrBut > 0 && IsWindowVisible(info->hWnd[0]))
            ans->MakeAndSetString("true");         
         else
            ans->MakeAndSetString("false"); 
      }
      else
      {
         bool state = IsWindowVisible(obj->hWnd);
         if(state)
            ans->MakeAndSetString("true");         
         else
            ans->MakeAndSetString("false");
      }
   }
   else if(parameter == "valueid" || parameter == "name")
   {
      ans->MakeAndSetString(obj->valueName);         
   } 
   else if(parameter == "ctrlid")
   {
      ans->MakeAndSetString(obj->objName);         
   }
   else if(parameter == "event")
   {
      ans->MakeAndSetString(obj->cb_event);         
   } 
   else if(parameter == "ctrlnr" || parameter == "objnr")
   {
      ans->MakeAndSetFloat(obj->nr());         
   } 
   else if(parameter == "tag")
   {
      ans->MakeAndSetString(obj->tag);         
   } 
   else if(parameter == "tabnr" || parameter == "tab_number") 
   {
      ans->MakeAndSetFloat(obj->tabNr);         
   }  
   else if(parameter == "winnr") // Return parent window number
   {
      ans->MakeAndSetFloat(obj->winParent->nr);         
   }   
   else if(parameter == "focus") 
   {
      if(GetFocus() == obj->hWnd)
         ans->MakeAndSetString("true");
      else 
         ans->MakeAndSetString("false");
   }
   else if(parameter == "tooltip") 
   {
      ans->MakeAndSetString(obj->GetToolTip());
   }
   else if(parameter == "uservar")
   {
      Variable *var = obj->varList.next;
      if(var)
         ans->FullCopy(var);
      else
         ans->MakeNullVar();
   }
   else if(parameter == "range") 
   {
      float range[2];
      if(obj->type == SLIDER)
      {
	      range[0] = (float)SendMessage((HWND)obj->hWnd,TBM_GETRANGEMIN,0,0);
		   range[1] = (float)SendMessage((HWND)obj->hWnd,TBM_GETRANGEMAX,0,0);
      }
      else if(obj->type == PROGRESSBAR)
      {
	      range[0] = (float)SendMessage((HWND)obj->hWnd,PBM_GETRANGE,true,0);
		   range[1] = (float)SendMessage((HWND)obj->hWnd,PBM_GETRANGE,false,0);
      }
      else
      {
         range[0] = obj->lower;
         range[1] = obj->upper;
      }
      ans->MakeMatrix2DFromVector(range,2,1);
   } 
	else if(parameter == "tabparent") // Get current tab parent and tab page
	{
		float parent[2];
		if(obj->tabParent != 0)
		{
			parent[0] = obj->tabParent->nr();
			parent[1] = obj->tabPageNr;
		}
		else
		{
			parent[0] = -1;
			parent[1] = -1;
		}
      ans->MakeMatrix2DFromVector(parent,2,1);
	}
	else if(parameter == "panelparent") // Get current panel parent
	{
      if(obj->panelParent)
         ans->MakeAndSetFloat(obj->panelParent->nr());
      else
         ans->MakeAndSetFloat(-1);
	}
	else if(parameter == "region")
	{ 
       //  obj->regionSized = true;
      float region[4];
      region[0] = obj->region.left;
      region[1] = obj->region.right;
      region[2] = obj->region.top;
      region[3] = obj->region.bottom;
      ans->MakeMatrix2DFromVector(region,4,1);  
	}
   else if(parameter == "type" || parameter == "datatype") 
   {
      if(obj->dataType == INTEGER)
         ans->MakeAndSetString("integer");
      else if(obj->dataType == FLOAT32)
         ans->MakeAndSetString("float");
      else if(obj->dataType == FLOAT32ORBLANK)
         ans->MakeAndSetString("floatorblank");
      else if(obj->dataType == FLOAT64)
         ans->MakeAndSetString("double");
      else if(obj->dataType == FLOAT64ORBLANK)
         ans->MakeAndSetString("doubleorblank");
      else if(obj->dataType == UNQUOTED_STRING)
         ans->MakeAndSetString("string");
      else if(obj->dataType == FILENAME)
         ans->MakeAndSetString("filename");
      else if(obj->dataType == PATHNAME)
         ans->MakeAndSetString("pathname");
      else if(obj->dataType == MATRIX2D)
         ans->MakeAndSetString("array");
      else
         ans->MakeAndSetString("undefined");
   }  
   else if(parameter == "procedure")
	{  
      if(obj->command)
         ans->MakeAndSetString(obj->command);
      else
         ans->MakeAndSetString("");
   }

   else if(parameter == "toolbar") 
   {
      ans->MakeAndSetFloat(obj->toolbar);
   } 
   else if(parameter == "statusbox") 
   { 
      ans->MakeAndSetFloat(obj->statusbox);  
   } 


   else if(parameter == "controltype" || parameter == "ctrltype" || parameter == "objtype")
   {
	   switch(obj->type)
	   {
	      case(RADIO_BUTTON):
	         ans->MakeAndSetString("radio button"); break;
	      case(CHECKBOX):
	         ans->MakeAndSetString("check box"); break;
	      case(STATICTEXT):
	         ans->MakeAndSetString("static text"); break;
	      case(TEXTBOX):
	         ans->MakeAndSetString("text box"); break;
	      case(TEXTMENU):
	         ans->MakeAndSetString("text menu"); break;
	      case(LISTBOX):
	         ans->MakeAndSetString("list box"); break;
	      case(SLIDER):
	         ans->MakeAndSetString("slider"); break;
	      case(GETMESSAGE):
	         ans->MakeAndSetString("get message"); break;
	      case(PROGRESSBAR):
	         ans->MakeAndSetString("progress bar"); break;
	      case(STATUSBOX):
	         ans->MakeAndSetString("status box"); break;	         	         	         
	      case(COLORSCALE):
	         ans->MakeAndSetString("color scale"); break;
	      case(GROUP_BOX):
	         ans->MakeAndSetString("group box"); break;
	      case(BUTTON):
	         ans->MakeAndSetString("button"); break;	  
	      case(COLORBOX):
	         ans->MakeAndSetString("color box"); break;
	      case(UPDOWN):
	         ans->MakeAndSetString("updown control"); break;	
	      case(DIVIDER):
	         ans->MakeAndSetString("divider"); break;	
	      case(PLOTWINDOW):
	         ans->MakeAndSetString("plot1d"); break;
	      case(IMAGEWINDOW):
	         ans->MakeAndSetString("plot2d"); break;
	      case(OPENGLWINDOW):
	         ans->MakeAndSetString("plot3d"); break;
	      case(CLIWINDOW):
	         ans->MakeAndSetString("cli"); break;
	      case(TEXTEDITOR):
	         ans->MakeAndSetString("text editor"); break;
	      case(PICTURE):
	         ans->MakeAndSetString("picture"); break;
	      case(MENU):
	         ans->MakeAndSetString("menu"); break;
	      case(TABCTRL):
	         ans->MakeAndSetString("tab"); break;
			case(GRIDCTRL):
				ans->MakeAndSetString("grid control"); break;
	      default:
	         ans->MakeAndSetString("unknown"); break;
      }
   } 
// Dimensions
   else if(parameter == "x_exp" || parameter == "xexp")
   {
      CText x_exp;
      obj->GetXExp(&x_exp,false);
      ans->MakeAndSetString(x_exp.Str());
   }
   else if(parameter == "y_exp" || parameter == "yexp")
   {
      CText y_exp;
      obj->GetYExp(&y_exp,false);
      ans->MakeAndSetString(y_exp.Str());
   }
   else if(parameter == "w_exp" || parameter == "wexp")
   {
      CText w_exp;
      obj->GetWExp(&w_exp,false);
      ans->MakeAndSetString(w_exp.Str());
   }
   else if(parameter == "h_exp" || parameter == "hexp")
   {
      CText h_exp;
      obj->GetHExp(&h_exp,false);
      ans->MakeAndSetString(h_exp.Str());
   }

   else if(parameter == "x")
      ans->MakeAndSetFloat(obj->xo);
   else if(parameter == "y")
      ans->MakeAndSetFloat(obj->yo);
   else if(parameter == "width")
      ans->MakeAndSetFloat(obj->wo);
   else if(parameter == "height")
      ans->MakeAndSetFloat(obj->ho);
   else if(parameter == "label")
   {
      CText txt;
      GetWindowTextEx(obj->hWnd,txt);
	   ans->MakeAndSetString(txt.Str());
   }   
   //else if(parameter == "dimensionstxt" || parameter == "dimensiontxt")
   //{
   //   CText txt;

   //   CText x_exp;
   //   obj->GetXExp(&x_exp,false);

   //    AppendStringToList(txt.Str(),&x_exp.Str(),0);
   //    GetSizeExp("wh",&txt,win->ySzScale,win->ySzOffset,false);
   //    AppendStringToList(txt.Str(),&dim,1);
   //    GetSizeExp("ww",&txt,win->wSzScale,win->wSzOffset,false);
   //    AppendStringToList(txt.Str(),&dim,2);
   //    GetSizeExp("wh",&txt,win->hSzScale,win->hSzOffset,false);
   //    AppendStringToList(txt.Str(),&dim,3);

	  //   ans->MakeAndSetList(dim,4);
	  //   FreeList(dim,4);
   //}
   
// Call appropriate object routine ************
   else
   {      	
	   switch(obj->type)
	   {
	      case(BUTTON):
	      {
	         err = GetButtonParameter(obj,parameter,ans);
	         break;
	      }
			case(CLIWINDOW):
	      {
	         err = GetCLIParameter(obj,parameter,ans);
	         break;
	      }
	      case(RADIO_BUTTON):
	      {
	         err = GetRadioButtonParameter(obj,parameter,ans);
	         break;
	      }
	      case(CHECKBOX):
	      {
	         err = GetCheckBoxParameter(obj,parameter,ans);
	         break;
	      }
	      case(STATICTEXT):
	      {
	         err = GetStaticTextParameter(obj,parameter,ans);
	         break;
	      }
	      case(TEXTBOX):
	      {
	         err = GetTextBoxParameter(itfc,obj,parameter,ans);
	         break;
	      }
	      case(TEXTEDITOR):
	      {
	         err = GetTextEditorParameter(itfc,obj,parameter,ans);
	         break;
	      }
	      case(GETMESSAGE):
	      {
	         err = GetMessageParameter(obj,parameter,itfc);
            return(err);
	      } 
	      case(TEXTMENU):
	      {
	         err = GetTextMenuParameter(itfc,obj,parameter,ans);
	         break;
	      }  
	      case(LISTBOX):
	      {
	         err = GetListBoxParameter(itfc,obj,parameter,ans);
	         break;
	      } 
	      case(SLIDER):
	      {
	         err = GetSliderParameter(obj,parameter,ans); 
	         break;
	      }
	      case(PROGRESSBAR):
	      {
	         err = GetProgressBarParameter(obj,parameter,ans); 
	         break;
	      }
         case(COLORBOX):
         {
            err = GetColorBoxParameter(obj,parameter,ans); 
	         break;
	      }
         case(UPDOWN):
         {
            err = GetUpDownParameter(obj,parameter,ans); 
	         break;
	      }
         case(DIVIDER):
         {
            err = GetDividerParameter(obj,parameter,ans); 
	         break;
	      }
         case(MENU):
         {
            err = GetMenuParameter(obj,parameter,ans); 
	         break;
	      }
         case(TABCTRL):
         {
            err = GetTabParameter(obj,parameter,ans); 
	         break;
	      }
         case(PLOTWINDOW):
         {         
            err = GetPlotWinParameter(obj,parameter,ans); 
	         break;
         }
         case(IMAGEWINDOW):
         {         
            err = GetImageWinParameter(obj,parameter,ans); 
	         break;
         }
			case(HTMLBOX):
			{
				err = GetHTMLWinParameter(itfc,obj,parameter,ans); 
				break;
			}
			case(GRIDCTRL):
			{
				err = GetGridCtrlParameter(obj,parameter,ans); 
				break;
			}

         default:
         {
            ErrorMessage("invalid parameter %s",parameter.Str());
            err = ERR;
         }
	   }
   }
   if(err == OK)
      itfc->nrRetValues = 1;
   
   return(err);
}


/***************************************************************************
*
*                          Modify an object parameter.
*
*  Syntax: setpar(window_nr, oject_nr, object_parameter1, parameter_value1, ...)
*
****************************************************************************/

int SetParameter(Interface *itfc, char arg[])
{
   short nrArgs;
   static short  winNr,objNr;
   WinData *win = 0;
   ObjectData *obj = 0;
   CText s;
   CText parameterIn;
   CText valueIn;
   Variable objVar;


// Get window and object numbers **************
   if((nrArgs = ArgScan(itfc,arg,2,"window,object","ee","dv",&winNr,&objVar)) < 0)
    return(nrArgs);
   
// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// Find window instance ************************
	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%ld' not found",(long)winNr);
	   return(ERR);
	}

// Find control instance ***********************
   if(objVar.GetType() == FLOAT32) // By number
   {
      objNr = nint(objVar.GetReal());
      if(!(obj = win->widgets.findByNr(objNr)))
      {
         ErrorMessage("object '%ld' not found",(long)objNr);
         return(ERR);
      }
   }
   else if(objVar.GetType() == UNQUOTED_STRING) // By valueID or objectID
   {
      if(!(obj = win->widgets.findByValueID(objVar.GetString())))
      {
         if(!(obj = win->widgets.findByObjectID(objVar.GetString())))
         {
            ErrorMessage("object '%s' not found",objVar.GetString());
            return(ERR);
         }
      }
      objNr = obj->nr();
   }

// Start processing parameters (should come in pairs) ***********
   CArg carg;
   nrArgs = carg.Count(arg);
	if(nrArgs%2 != 0)
	{
		ErrorMessage("expecting an even number of arguments");
      return(ERR);
   }

	short i = 3;

   
   while(i < nrArgs)
   {  
   // Extract parameter name
      parameterIn = carg.Extract(i);

   // Extract parameter value
      valueIn = carg.Extract(i+1);

      if(SetParameterCore(itfc,obj,parameterIn,valueIn) == ERR)
         return(ERR);

      i+=2;

   }

   return(OK);
}

int SetParameterCore(Interface *itfc, ObjectData* obj, CText &parameterIn, CText &valueIn)
{
   Variable parName;
   Variable parValue;
   bool correctType = true;
   CText parameter;
   CText value;
	short parType;
   WinData *win = obj->winParent;
   int err = OK;
   bool region;

   // Work out client rectangle size
   RECT rect;
   GetClientRect(win->hWnd,&rect);
   short ww = (short)rect.right;
   short wh = (short)rect.bottom;

   if((parType = Evaluate(itfc,RESPECT_ALIAS,parameterIn.Str(),&parName)) < 0)
      return(ERR); 

   parameterIn.RemoveQuotes();
   
   if(parType != UNQUOTED_STRING)
   {
      ErrorMessage("invalid data type for parameter '%s'",parameterIn.Str());
      return(ERR);
   }  

   parameter = parName.GetString();
   parameter.LowerCase();

	if((parType = Evaluate(itfc,RESPECT_ALIAS,valueIn.Str(),&parValue)) == ERR)
		return(ERR);

// Could be one of many types so prepare
   CText valString;
   float valReal = 0;
   float** valMat = 0;
   long dim1 = 0;
   long dim2 = 0;

   if(parType == UNQUOTED_STRING)
      valString = parValue.GetString();        
   else if(parType == FLOAT32)
      valReal = parValue.GetReal();
   else if(parType == MATRIX2D)
   {
      valMat = parValue.GetMatrix2D();
      dim1 = parValue.GetDimX();
      dim2 = parValue.GetDimY();
   }


   itfc->nrRetValues = 0; // Make sure we don't return anything from this function

// Parameters generic to all controls *****************************************
	if(parameter == "enable")
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            if(!obj->readOnlyOutput)
            {
               obj->enable = true;
               obj->EnableObject(true);
            }
         }
			else 
         {
            obj->enable = false;
            obj->EnableObject(false);
         }
		} 
		else
		   correctType = false;     
	}
   else if (parameter == "dehover") // Fixes a bug which occurs sometimes using getx when an adjacent button can be left highlighted
   {
      SendMessage(obj->hWnd, WM_MOUSELEAVE, NULL, NULL);
      MyInvalidateRect(obj->hWnd, NULL, 1);
   }
   else if(parameter == "draw") // Control drawing of an object (on true redraws object's parent)
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            SendMessage(obj->hWnd, WM_SETREDRAW, true, 0);
            MyInvalidateRect(obj->hwndParent,NULL,1);
         }
			else 
         {
            SendMessage(obj->hWnd, WM_SETREDRAW, false, 0);
         }
		} 
		else
		   correctType = false;     
	}
	else if(parameter == "drawobj") // Control drawing of an object (on true only redraws object)
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            SendMessage(obj->hWnd, WM_SETREDRAW, true, 0);
            MyInvalidateRect(obj->hWnd,NULL,1);
				UpdateWindow(obj->hWnd);
         }
			else 
         {
            SendMessage(obj->hWnd, WM_SETREDRAW, false, 0);
         }
		} 
		else
		   correctType = false;     
	}
	else if(parameter == "readonly")
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            obj->readOnly = true;
         }
			else 
         {
            obj->readOnly = false;
         }
		} 
		else
		   correctType = false;     
	}
	else if(parameter == "readonlyoutput")
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            obj->readOnlyOutput = true;
            obj->enable = false;
            obj->EnableObject(false);
         }
			else 
         {
            obj->readOnlyOutput = false;
            obj->enable = true;
            obj->EnableObject(true);
         }
		} 
		else
		   correctType = false;     
	}
	else if(parameter == "keepfocus")
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            obj->keepFocus = true;
         }
			else 
         {
            obj->keepFocus = false;
         }
		} 
		else
		   correctType = false;     
	}
	else if(parameter == "active")
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            obj->active = true;
            win->activeCtrl = true;
         }
			else 
         {
            obj->active = false;
         }
		} 
		else
		   correctType = false;     
	}
	else if(parameter == "eventactive")
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            obj->eventActive = true;
         }
			else 
         {
            obj->eventActive = false;
         }
		} 
		else
		   correctType = false;     
	}
	else if(parameter == "highlitechanges")
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            obj->highLiteChanges = true;
         }
			else 
         {
            obj->highLiteChanges = false;
         }
		} 
		else
		   correctType = false;     
	}	
	else if(parameter == "valuechanged")
	{
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            obj->valueChanged = true;
         }
			else 
         {
            obj->valueChanged = false;
         }
		} 
		else
		   correctType = false;     
	}	
   else if(parameter == "menubar") // Set menu and accelerator table
   {
      if(parType == MATRIX2D && dim2 == 1) // Row vector
      {   
         long w =  dim1; // Number of menus
         short objNr;
         ObjectData *menuObj,*menuObjPR;
         MenuInfo *info;
         float *menuList = valMat[0]; // Menu list

      // Make a new menu - blank for now
         HMENU menu = CreateMenu();

      // Count the total number of menu items in the supplied menus
         int cnt = 0;
         for(int i = 0; i < w; i++) // Loop over menus
         {
            objNr = nsint(menuList[i]); // Extract menu object number

				if(objNr < 0) continue;

            if(!(menuObj = win->widgets.findByNr(objNr))) // Get the menu object
            {
               DestroyMenu(menu);
               ErrorMessage("object '%ld' not found",(long)objNr);
               return(ERR);
            }
            if(menuObj->type != MENU)
            {
               DestroyMenu(menu);
               ErrorMessage("object '%ld' is not a menu",(long)objNr);
               return(ERR);
            }
            info = (MenuInfo*)menuObj->data;

          // Search menu for pull-rights and add them to the item count
            for(int j = 0; j < info->nrItems; j++)
            {
               if(!strcmp(info->label[j],"\"Pull_right\""))
               {
                  short objNrPR;
                  if(sscanf(info->cmd[j],"%hd",&objNrPR)!= 1)
                  {
                     DestroyMenu(menu);
                     ErrorMessage("'%s' is not a valid menu object number",info->cmd[j]);
                     return(ERR);
                  }
                  if(!(menuObjPR = win->widgets.findByNr(objNrPR))) // Get the pull rightmenu object
                  {
                     DestroyMenu(menu);
                     ErrorMessage("object '%hd' not found",objNrPR);
                     return(ERR);
                  }
                  MenuInfo* infoPR = (MenuInfo*)menuObjPR->data;
                  cnt += infoPR->nrItems;
               }
            }

            cnt += info->nrItems; // Add the number of items in this menu
         }

       // Make a backup of the existing window accelerator table
         ACCEL *accelNew;
         int j,k,nMenu = 0;

       // Allocate sufficent space for new accelerators
         ACCEL *accelMenu = new ACCEL[cnt];
         nMenu = 0;

      // Load all the accelerators
         for(int i = 0; i < w; i++) // Loop over menus
         {
            objNr = nsint(menuList[i]); // Extract menu object number
				if(objNr < 0) // See if its a user menu (i.e. folder menu)
            {
                AppendUserMenu(menu);
                continue;
            }
            if(!(menuObj = win->widgets.findByNr(objNr)))// Get the menu object
            {
               DestroyMenu(menu);
               delete [] accelMenu;
               ErrorMessage("object '%ld' not found",(long)objNr);
               return(ERR);
            }

            info = (MenuInfo*)menuObj->data;

          // Search menu for pull-rights
            for(int k = 0; k < info->nrItems; k++)
            {
               if(!strcmp(info->label[k],"\"Pull_right\""))
               {
                  short objNrPR;
                  sscanf(info->cmd[k],"%hd",&objNrPR);
                  menuObjPR = win->widgets.findByNr(objNrPR); // Get the pull rightmenu object

                  if(menuObjPR->type != MENU)
                  {
                     DestroyMenu(menu);
                     delete [] accelMenu;
                     ErrorMessage("pull-right '%ld' is not a menu",(long)objNrPR);
                     return(ERR);
                  }

                  MenuInfo* infoPR = (MenuInfo*)menuObjPR->data;
                  cnt = infoPR->nrItems;
              
                  for(int j = 0; j < cnt; j++) // Add the accelerators for the pull-right
                  {
                     if(infoPR->accel[j].key != 0)
                        accelMenu[nMenu++] = infoPR->accel[j];
                  }
               }
            }
            cnt = info->nrItems;

          // Add the new menu to the window menu
            AppendMenu(menu,MF_POPUP,(UINT_PTR)info->menu,info->name);

         // Add the accelerator keys from this menu 
            for(int j = 0; j < cnt; j++) // Loop over items in each menu
            {
               if(info->accel[j].key != 0)
                  accelMenu[nMenu++] = info->accel[j];
            }
         }

      // Make a new accelerator table which combines existing and new
         accelNew = new ACCEL[nMenu];

      // Add menu entries (WHY DO THIS?)
         k = 0;
         for(j = 0; j < nMenu; j++)
            accelNew[k++] = accelMenu[j];

		// Remove the previous menus
			while(RemoveMenu(obj->menu, 0, MF_BYPOSITION) != 0);

	   // Remove old accelerator table and menu
			if(obj->accelTable)
				DestroyAcceleratorTable(obj->accelTable);
			if(obj->menu)
				DestroyMenu(obj->menu);

       // Add this new menu to the object
         obj->menu = menu;
         obj->accelTable = CreateAcceleratorTable((LPACCEL)accelNew,k);

         if(accelMenu)
            delete [] accelMenu;
         if(accelNew)
            delete [] accelNew;

      // Save the object numbers which contributed to the window menu
         if(obj->menuList) delete obj->menuList;
         obj->menuList = new short[w];
         obj->menuListSize = w;
         for(k = 0; k < w; k++)
            obj->menuList[k] = nsint(menuList[k]);
      }
      else
		   correctType = false;  
   }

	else if(parameter == "visible")
	{ 
		if(parType == UNQUOTED_STRING)
		{           
			if(valString == "true" || valString == "yes") 
         {
            obj->visible = true;
            obj->Show(true);
         }
			else
         {
            obj->visible = false;
            obj->Show(false);
         }
		}
		else
		   correctType = false;                
	}
	else if(parameter == "label")
	{
		if(parType == UNQUOTED_STRING)
      {
         if(obj->type == GROUP_BOX)
         {
            obj->EraseGroupBox();
         }
         SetWindowText(obj->hWnd,valString.Str());
      }
		else
		   correctType = false;          
	}    
	else if(parameter == "name" || parameter == "valueid")
	{
		if(parType == UNQUOTED_STRING)
			strncpy_s(obj->valueName,MAX_NAME,valString.Str(),_TRUNCATE);
		else
		   correctType = false;           
	}
	else if(parameter == "ctrlid" || parameter == "objid")
	{
		if(parType == UNQUOTED_STRING)
			strncpy_s(obj->objName,MAX_NAME,valString.Str(),_TRUNCATE);
		else
		   correctType = false;           
	}
	else if(parameter == "tabnr" || parameter == "tab_number")
	{ 
		if(parType == FLOAT32)
      {
			obj->tabNr = nsint(valReal);
         win->tabMode = TAB_BY_TAB_NUMBER;
      }
		else
		  correctType = false;          
	}
   else if(parameter == "uservar")
   {
		if(parType == STRUCTURE)
      {
         CText par;
         obj->varList.RemoveAll();
         Variable *dstVar = obj->varList.Add(STRUCTURE,"objStruct");
         Variable *srcVar = &parValue;
         srcVar->SetType(STRUCTURE);
         CopyVariable(dstVar,srcVar,FULL_COPY);
      }
		else
      {
         ErrorMessage("User variable should be a parameter string");
         return(ERR);
      }
   }
   else if(parameter == "tooltip")
   {
      char TipText[MAX_STR];

    // Get the tooltip
		if(parType == UNQUOTED_STRING)
			strncpy_s(TipText,MAX_STR,valString.Str(),_TRUNCATE);
		else
      {
         ErrorMessage("Tooltip should be a string");
         return(ERR);
      }

      // Get the parent window for the tooltip
      HWND hParWin = obj->hWnd;
      if(obj->type == RADIO_BUTTON)
      {
	      RadioButtonInfo* info = (RadioButtonInfo*)obj->data;
         hParWin = info->hWnd[0];
      }

    // Delete the old tooltip if it exists
      if(obj->toolTipHwnd)
      {
         TOOLINFO toolinfo; // Tool Tip Info structure
         char toolText[MAX_STR];
         memset(&toolinfo, 0, sizeof(TOOLINFO));
         toolinfo.cbSize = sizeof(TOOLINFO);
         toolinfo.hwnd = obj->hwndParent;
         toolinfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND ;
         toolinfo.uId = (UINT)obj->hWnd;
         SendMessage (obj->toolTipHwnd, TTM_DELTOOL, 0, (LPARAM)&toolinfo );
         DestroyWindow(obj->toolTipHwnd);  
         obj->toolTipHwnd = NULL;
      }

     // Create a tool tip class and set its parent to the Parent Window
      HWND ToolTipWnd = CreateWindow(TOOLTIPS_CLASS, NULL, WS_POPUP /*| TTS_BALLOON*/, 0,0,0,0, hParWin, NULL, prospaInstance, 0);

      SendMessage(ToolTipWnd, TTM_ACTIVATE, TRUE, 0); // Send this message to Activate ToolTips for the window
      SendMessage(ToolTipWnd, TTM_SETMAXTIPWIDTH, 0, 150);
     // Pass a FALSE when you wish to deactivate the tool tip.


      TOOLINFO toolinfo; // Tool Tip Info structure
      memset(&toolinfo, 0, sizeof(TOOLINFO));
      toolinfo.cbSize = sizeof(TOOLINFO);
      toolinfo.hwnd = obj->hwndParent;
      toolinfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND ;
      toolinfo.uId = (UINT)hParWin;
      toolinfo.hinst = NULL;
      toolinfo.lpszText = TipText; // Text you wish displayed when the mouse is over the control
      obj->toolTipHwnd = ToolTipWnd;
      SendMessage (ToolTipWnd, TTM_ADDTOOL, 0, (LPARAM)&toolinfo );
   }
   else if(parameter == "rmtooltip")
   {
      if(obj->toolTipHwnd)
      {
         TOOLINFO toolinfo; // Tool Tip Info structure
         char toolText[MAX_STR];
         memset(&toolinfo, 0, sizeof(TOOLINFO));
         toolinfo.cbSize = sizeof(TOOLINFO);
         toolinfo.hwnd = obj->hwndParent;
         toolinfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND ;
         toolinfo.uId = (UINT)obj->hWnd;
         SendMessage (obj->toolTipHwnd, TTM_DELTOOL, 0, (LPARAM)&toolinfo );
         DestroyWindow(obj->toolTipHwnd);  
         obj->toolTipHwnd = NULL;
      }

   }
	else if(parameter == "ctrlnr" || parameter == "objnr")
	{ 
		if(parType == FLOAT32)
      {
			obj->nr(nsint(valReal));
      }
      else
		  correctType = false;          
	}
	else if(parameter == "datatype" || parameter == "type")
	{ 
		if(parType == UNQUOTED_STRING)
		{
			if(valString == "integer")
				obj->dataType = INTEGER;
         else if (valString == "hex")
            obj->dataType = HEX;
			else if(valString == "float")
				obj->dataType = FLOAT32;
			else if(valString == "double")
				obj->dataType = FLOAT64;
			else if(valString == "string")
				obj->dataType = UNQUOTED_STRING;
			else if(valString == "filename")
				obj->dataType = FILENAME;
			else if(valString == "pathname")
				obj->dataType = PATHNAME;
			else if(valString == "floatorblank")
            obj->dataType = FLOAT32ORBLANK;
			else if(valString == "doubleorblank")
            obj->dataType = FLOAT64ORBLANK;
			else if(valString == "array")
            obj->dataType = MATRIX2D;
			else if(valString == "undefined")
            obj->dataType = NULL_VARIABLE;
			else
			{
				ErrorMessage("unsupported data type for parameter '%s'",parameter);
				return(ERR);
			}  
		}
		else
		   correctType = false;          
	}
	else if(parameter == "range")
	{
		if(parType == MATRIX2D && dim1 == 2 && dim2 == 1)
		{
			float lower = valMat[0][0];
			float upper = valMat[0][1];
			if(lower > upper)
			{
				ErrorMessage("invalid data range for parameter '%s'",parameter);
				return(ERR);
			} 
			obj->lower = lower;
			obj->upper = upper;
         obj->rangeCheck = true;
			if(obj->type == SLIDER)
				err = SetSliderParameter(obj,parameter,&parValue); 
         else if(obj->type == PROGRESSBAR)
				err = SetProgressBarParameter(obj,parameter,&parValue); 

         if(obj->dataType == NULL_VARIABLE) // Ensure data type is set. 15/2/07
            obj->dataType = FLOAT32;
        
		}
		else
		   correctType = false;          
	}
  else if(parameter == "panelparent") // Set current panel parent
  {
		if(parType == FLOAT32)
      {
         if(obj->SetPanelParent(nsint(valReal)) == ERR)
            return(ERR);
      }
		else
		{
			ErrorMessage("invalid panel-parent type : should be control number");
			return(ERR);
		}
  }
  else if(parameter == "panelset" || parameter == "scrollposition") // Set current panel scroll position
  {
		if(parType == FLOAT32)
      {
         SCROLLINFO si;
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_ALL;
         GetScrollInfo(obj->hWnd,SB_CTL, &si);
         int pos = si.nPos;
         int max = si.nMax-si.nPage+2;

         if(valReal >= 0 && valReal <= 100)
         {
			   short pos = nsint(valReal*max/100.0);
            obj->UpdatePanelFromScrollBar(pos);
         }
         else
		   {
			   ErrorMessage("panel set value should be from 0 to 100");
			   return(ERR);
		   }
      }
		else
		{
			ErrorMessage("invalid panel-parent type : should be number");
			return(ERR);
		}
  }
  else if(parameter == "panelupdate") // Set current panel parent
  {
     if(obj->type == PANEL)
     {
			if(parType == UNQUOTED_STRING)
			{
				if(valString == "visible controls")
               obj->UpdatePanelStruct(1);
				else
               obj->UpdatePanelStruct(0);
			}
     }
  }
  else if(parameter == "tabparent") // Set current tab parent and tab page
  {
		if(parType == MATRIX2D && dim1 == 2 && dim2 == 1)
		{
         short parentTabNr = nint(valMat[0][0]);

         if(parentTabNr != -1)
         {
			   ObjectData *tabCtrl = obj->winParent->FindObjectByNr(parentTabNr);
			   if(!tabCtrl || tabCtrl->type != TABCTRL)
			   {
				   ErrorMessage("invalid tab parent number");
				   return(ERR);
			   }
            obj->tabParent = tabCtrl;
			   obj->tabPageNr = nint(valMat[0][1]);
         }
         else
         {
            obj->tabParent = NULL;
			   obj->tabPageNr = -1;
         }
		}
		else if(parType == UNQUOTED_STRING)
		{
			CText numTxt;
			short parentTabNr,tabPage;
			CArg arg(',');
			int n = arg.Count(valString.Str());
			if(n == 2)
			{
				numTxt = arg.Extract(1);
				if(sscanf(numTxt.Str(),"%hd",&parentTabNr) != 1)
				{
					ErrorMessage("invalid tab parent number");
					return(ERR);
				}

				numTxt = arg.Extract(2);
				if(sscanf(numTxt.Str(),"%hd",&tabPage) != 1)
				{
					ErrorMessage("invalid tab page number");
					return(ERR);
				}

            if(parentTabNr != -1)
            {
			      ObjectData *tabCtrl = obj->winParent->FindObjectByNr(parentTabNr);
				   if((!tabCtrl || tabCtrl->type != TABCTRL))
				   {
					   ErrorMessage("invalid tab parent number");
					   return(ERR);
				   }
               obj->tabParent = tabCtrl;
               obj->tabPageNr = tabPage;
            }
            else
            {
               obj->tabParent = NULL;
			      obj->tabPageNr = -1;
            }
			}
		}
		else
		{
			ErrorMessage("invalid value type : should be [tabParent,tabPage]");
			return(ERR);
		}
   }
	else if(parameter == "region")
	{ 
		if(parType == MATRIX2D && dim1 == 4 && dim2 == 1)
		{
			obj->region.left = valMat[0][0];
			obj->region.right = valMat[0][1];
			obj->region.top = valMat[0][2];
			obj->region.bottom = valMat[0][3];
         obj->regionSized = true;
		}
		else
		   correctType = false;          
	}
	else if(parameter == "focus")
	{
		SetFocus(obj->hWnd);         
	}      
	else if(parameter == "x" || parameter == "x_exp" || parameter == "xexp")
	{
      if(obj->type == STATICTEXT && (parameter == "x_exp" || parameter == "xexp"))
      {
         correctType = SetStaticTextXExp(obj,&parValue);
      }
      else
      {
		   if(parType == FLOAT32)
		   {
            obj->xo = nint(valReal);        
            obj->xSzScale = 0;
            obj->xSzOffset = obj->xo;
            obj->Move(obj->xo,obj->yo,obj->wo,obj->ho);
         //   ResizeObjectsWhenParentResizes(obj->winParent,NULL,NULL,0,0);
		   }  
		   else if(parType == UNQUOTED_STRING)
		   {
            float scale,off;
            if(GetObjectDimensions(valString.Str(), &scale, &off, &region) == ERR)
            {
               ErrorMessage("invalid x position");
               return(ERR);
            }   
            obj->xo = ww*scale + off;        
            obj->xSzScale = scale;
            obj->xSzOffset = off;
            obj->Move(obj->xo,obj->yo,obj->wo,obj->ho); 
        //    ResizeObjectsWhenParentResizes(obj->winParent,NULL,NULL,0,0);
		   } 
		   else
		      correctType = false;   
      }
	}
	else if(parameter == "y" || parameter == "y_exp"|| parameter == "yexp")
	{   
		if(parType == FLOAT32)
		{
         obj->yo = nint(valReal);        
         obj->ySzScale = 0;
         obj->ySzOffset = obj->yo;
         obj->Move(obj->xo,obj->yo,obj->wo,obj->ho);
         //ResizeObjectsWhenParentResizes(obj->winParent,NULL,NULL,0,0);
		}  
		else if(parType == UNQUOTED_STRING)
		{
         float scale,off;
         if(GetObjectDimensions(valString.Str(), &scale, &off, &region) == ERR)
         {
            ErrorMessage("invalid y position");
            return(ERR);
         }   
         obj->yo = wh*scale + off;        
         obj->ySzScale = scale;
         obj->ySzOffset = off;
         obj->Move(obj->xo,obj->yo,obj->wo,obj->ho);
        // ResizeObjectsWhenParentResizes(obj->winParent,NULL,NULL,0,0);
		} 
		else
		   correctType = false;       
	}      
	else if(parameter == "width" || parameter == "w_exp" || parameter == "wexp")
	{  
		if(parType == FLOAT32)
		{
         obj->wo = nint(valReal);        
         obj->wSzScale = 0;
         obj->wSzOffset = obj->wo;
         obj->Move(obj->xo,obj->yo,obj->wo,obj->ho);
        // ResizeObjectsWhenParentResizes(obj->winParent,NULL,NULL,0,0);
		}  
		else if(parType == UNQUOTED_STRING)
		{
         float scale,off;

         if(GetObjectDimensions(valString.Str(), &scale, &off, &region) == ERR)
         {
            ErrorMessage("invalid width");
            return(ERR);
         }   
         obj->wo = ww*scale + off;        
         obj->wSzScale = scale;
         obj->wSzOffset = off;
         obj->Move(obj->xo,obj->yo,obj->wo,obj->ho);
        // ResizeObjectsWhenParentResizes(obj->winParent,NULL,NULL,0,0);
		} 
		else
		   correctType = false;          
	}      
	else if(parameter == "height" || parameter == "h_exp"|| parameter == "hexp")
	{ 
		if(parType == FLOAT32)
		{
         obj->ho = nint(valReal);        
         obj->hSzScale = 0;
         obj->hSzOffset = obj->ho;
         obj->Move(obj->xo,obj->yo,obj->wo,obj->ho);
        // ResizeObjectsWhenParentResizes(obj->winParent,NULL,NULL,0,0);
		}  
		else if(parType == UNQUOTED_STRING)
		{
         float scale,off;
         if(GetObjectDimensions(valString.Str(), &scale, &off, &region) == ERR)
         {
            ErrorMessage("invalid height");
            return(ERR);
         }   
         obj->ho = wh*scale + off;        
         obj->hSzScale = scale;
         obj->hSzOffset = off;
         obj->Move(obj->xo,obj->yo,obj->wo,obj->ho);
       //  ResizeObjectsWhenParentResizes(obj->winParent,NULL,NULL,0,0);
		}
		else
		   correctType = false;         
	}
	else if(parameter == "procedure")
	{  
      if(parType == UNQUOTED_STRING)
      {
			if(obj->type == MENU)
			{
              MenuInfo *info = (MenuInfo*)obj->data;
				  if(info->nrItems == 1)
				  {
					  if(info->cmd)
					  {
						   if(info->cmd[0])
                        delete [] info->cmd[0];
							ReplaceSpecialCharacters(valString.Str(),"\r\n","",-1);
							info->cmd[0] = new char[strlen(valString.Str())+1];
							strcpy(info->cmd[0],valString.Str());
					  }
				  }
			}
			else
			{
				if(obj->command)
					delete [] obj->command;
				ReplaceSpecialCharacters(valString.Str(),"\r\n","",-1);
				obj->command = new char[strlen(valString.Str())+1];
				strcpy(obj->command,valString.Str());
			}
		}
		else
		   correctType = false;   
   }
	else if(parameter == "dragndropproc")
	{  
      if(parType == UNQUOTED_STRING)
      {
         obj->dragNDropProc = valString.Str();
         DragAcceptFiles(obj->hWnd,(obj->dragNDropProc != ""));
		}
   }
   else if(parameter == "event")
   {
      if(parType == UNQUOTED_STRING)
         strncpy_s(obj->cb_event,MAX_NAME,valString.Str(),_TRUNCATE); 
		else
		   correctType = false;     
   } 
   else if(parameter == "tag") 
   {
      if(parType == UNQUOTED_STRING)
         strncpy_s(obj->tag,MAX_NAME,valString.Str(),_TRUNCATE); 
		else
		   correctType = false;   
   } 
   else if(parameter == "toolbar") 
   {
      if(parType == FLOAT32)
         obj->toolbar = nint(valReal);   
		else
		   correctType = false;    
   } 
   else if(parameter == "statusbox") 
   { 
      if(parType == FLOAT32)
         obj->statusbox = nint(valReal);  
		else
		   correctType = false;   
   } 


// Parameters specific to a particular type of control  *******************	      
	else
	{  
		switch(obj->type)
		{
			case(BUTTON):
			{
				err = SetButtonParameter(obj,parameter,&parValue);
				break;
			}   
			case(RADIO_BUTTON):
			{
				err = SetRadioButtonParameter(obj,parameter,&parValue);
				break;
			}
			case(LISTBOX):
			{
				err = SetListBoxParameter(obj,parameter,&parValue);
				break;
			}
			case(CHECKBOX):
			{
				err = SetCheckBoxParameter(obj,parameter,&parValue);
				break;
			}
			case(TEXTBOX):
			{
				err = SetTextBoxParameter(obj,parameter,&parValue);
				break;
			}
			case(STATICTEXT):
			{
				err = SetStaticTextParameter(obj,parameter,&parValue); 
				break;
			}
			case(SLIDER):
			{
				err = SetSliderParameter(obj,parameter,&parValue); 
				break;
			} 
			case(PROGRESSBAR):
			{
				err = SetProgressBarParameter(obj,parameter,&parValue); 
				break;
			} 
			case(STATUSBOX):
			{
				err = SetStatusBoxParameter(obj,parameter,&parValue); 
				break;
			} 
			case(TEXTMENU):
			{
				err = SetTextMenuParameter(obj,parameter,&parValue); 
				break;
			}  
			case(COLORBOX):
			{
				err = SetColorBoxParameter(obj,parameter,&parValue); 
				break;
			}
			case(HTMLBOX):
			{
				err = SetHTMLWinParameter(obj,parameter,&parValue); 
				break;
			}
			case(PLOTWINDOW):
			{
				err = SetPlotWinParameter(obj,parameter,&parValue); 
				break;
			}
			case(IMAGEWINDOW):
			{
				err = SetImageWinParameter(obj,parameter,&parValue); 
				break;
			}
         case(UPDOWN):
         {
            err = SetUpDownParameter(obj,parameter,&parValue); 
				break;
			}
         case(PICTURE):
         {
            err = SetPictureParameter(obj,parameter,&parValue); 
				break;
			}
         case(TEXTEDITOR):
         {
            err = SetTextEditorParameter(obj,parameter,&parValue); 
				break;
			}
         case(CLIWINDOW):
         {
            err = SetCLIParameter(obj,parameter,&parValue); 
				break;
			}
         case(DIVIDER):
         {
            err = SetDividerParameter(obj,parameter,&parValue); 
				break;
			}
         case(DEBUGSTRIP):
         {
            err = SetDebugStripParameter(obj,parameter,&parValue); 
				break;
			}
         case(OPENGLWINDOW):
         {
            err = SetOpenGLParameter(obj,parameter,&parValue); 
				break;
			}
         case(TABCTRL):
         {
            err = SetTabParameter(obj,parameter,&parValue); 
		      break;
         }
			case(MENU):
         {
            err = SetMenuParameter(obj,parameter,&parValue); 
		      break;
         }
			case(TOOLBAR):
         {
            err = SetToolBarParameter(obj,parameter,&parValue); 
		      break;
         }
			case(GRIDCTRL):
			{
				err = SetGridCtrlParameter(obj,parameter,&parValue); 
		      break;
			}
		}
	}

   if(correctType == false)
	{
		ErrorMessage("invalid data type for parameter '%s'",parameterIn.Str());
		return(ERR);
	} 

   return(err);
}


int GetCLIParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
	if(parameter == "getmacroname")
	{
		HWND hwnd = obj->hWnd;

	   CText cname;
      long startWord,endWord;
      char *name;
		bool classCmd=false;
		bool funcCmd=false;
		short userClassCmd=0;
		char **list = 0;
		int cnt = 0;
		extern CText *gClassName;
		extern CText *gClassMacro;
		extern int gNrClassDeclarations;

      GetEditSelection(hwnd,startWord,endWord);	      
      cname = ExpandToFullWord(hwnd," +-*/%^=(,[$\t<>:"," ()\n",startWord,endWord,classCmd,userClassCmd,funcCmd);
		int pos;
	   if((pos = cname.FindSubStr(0,"->")) >= 0)
		{
			CText className = cname.Middle(0,pos-1);
	     	int n = gNrClassDeclarations;
			for(int i = 0; i < n; i++) // Convert class to macro name if possible
			{
				if(gClassName[i] == className)
				{
					className = gClassMacro[i];
					break;
				}
			}
			ans->MakeAndSetString(className.Str());
			return(OK);
		}
	   if((pos = cname.FindSubStr(0,":")) >= 0)
		{
			CText macroName = cname.Middle(0,pos-1);
			ans->MakeAndSetString(macroName.Str());
			return(OK);
		}
		ans->MakeNullVar();
		return(OK);
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

 	return(OK);
}


int GetButtonParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   if(parameter == "mode")
   {
      long type = GetWindowLong(obj->hWnd,GWL_STYLE);
      if(type & BS_DEFPUSHBUTTON)
	      ans->MakeAndSetString("default");
      else
      {
         HWND hWnd = obj->hwndParent;
	      WinData* win = GetWinDataClass(hWnd);
         if(win->abortID == obj->nr())
         {
	         ans->MakeAndSetString("abort");
         }
         else if(win->panicID == obj->nr())
         {
	         ans->MakeAndSetString("panic");
         }
         else if(win->cancelID == obj->nr())
         {
	         ans->MakeAndSetString("cancel");
         }
         //else if(obj->active)
         //{
	        // ans->MakeAndSetString("active");
         //}
         else
         {
	         ans->MakeAndSetString("normal");
         }
      }
	}
   else if(parameter == "color" || parameter == "fgcolor" )
   {
	   BYTE *data = (BYTE*)&obj->fgColor;
      float colors[3];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      ans->MakeMatrix2DFromVector(colors,3,1);
   }
   //else if(parameter == "bkgcolor" || parameter == "bgcolor" )
   //{
	  // BYTE *data = (BYTE*)&obj->bgColor;
   //   float colors[3];
   //   colors[0] = data[0];
   //   colors[1] = data[1];
   //   colors[2] = data[2];
   //   ans->MakeMatrix2DFromVector(colors,3,1);
   //}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

 	return(OK);
}

int GetRadioButtonParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   RadioButtonInfo *info;
   long i;
 
   info = (RadioButtonInfo*)(obj->data);
	CArg carg;
	carg.Count(info->states);

   if(parameter == "text")
   {
	   for(i = 0; i < info->nrBut; i++)
	   {
	      if(SendMessage(info->hWnd[i],BM_GETCHECK,0,0))
	        break;
	   }
	   ans->MakeAndSetString(carg.Extract(i+1));
	}
   else if(parameter == "value")
   {
	   for(i = 0; i < info->nrBut; i++)
	   {
	      if(SendMessage(info->hWnd[i],BM_GETCHECK,0,0))
	        break;
	   }
	   ans->MakeAndSetFloat((float)(i+1));
	}
   else if(parameter == "init")
   {
	   ans->MakeAndSetString(carg.Extract(info->init));
	}	
   else if(parameter == "option_list") // return options as list
   {
      long width;
      char **list = MakeListFromText(info->states, &width);
	   ans->MakeAndSetList(list,width);
	   FreeList(list,width);
	}
   else if(parameter == "option_string") // return options as string
   {
	   ans->MakeAndSetString(info->states);
	}
   else if(parameter == "spacing") // return spacing as number
   {
	   ans->MakeAndSetFloat(info->spacing);
	}
   else if(parameter == "orientation") // return orientation as string
   {
      if(info->orient == 'h')
	      ans->MakeAndSetString("horizontal");
      else
	      ans->MakeAndSetString("vertical");
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

 	return(OK);
}

// Get the position of the trackbar parameter - note that vertical sliders  have
// there range upsidedown hence the more complex extraction method
int GetSliderParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   if(parameter == "value")
   {
		long type = GetWindowLong(obj->hWnd,GWL_STYLE);
		long trackPos;
		if(type & TBS_VERT) 
		{   
		  long trackMax = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMAX,0,0);	
		  long trackMin = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMIN,0,0);	
		  trackPos = trackMax+trackMin-SendMessage((HWND)obj->hWnd,TBM_GETPOS,0,0);
		}
		else
		{
		  trackPos = SendMessage((HWND)obj->hWnd,TBM_GETPOS,0,0);
		}
		ans->MakeAndSetFloat((float)(trackPos));
	}
   else if(parameter == "range")
   {
	   long trackMin = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMIN,0,0);
		long trackMax = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMAX,0,0);
      float** m = MakeMatrix2D(2,1);
      m[0][0] = trackMin;
      m[0][1] = trackMax;
		ans->AssignMatrix2D(m,2,1);
	}
   else if(parameter == "orientation")
   {
		long type = GetWindowLong(obj->hWnd,GWL_STYLE);
		if(type & TBS_VERT) 
		   ans->MakeAndSetString("vertical");
      else
		   ans->MakeAndSetString("horizontal");
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
 	return(OK);
}



int GetPlotWinParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   PlotWindow *pp;
   
   pp = (PlotWindow*)(obj->data);
   
   if(parameter == "tracemenu")
   {
      ans->MakeAndSetFloat(pp->traceMenuNr);
   }
   else if(parameter == "bkgmenu")
   {
      ans->MakeAndSetFloat(pp->bkgMenuNr);
   }  
   else if(parameter == "axesmenu")
   {
      ans->MakeAndSetFloat(pp->axesMenuNr);
   }     
   else if(parameter == "titlemenu")
   {
      ans->MakeAndSetFloat(pp->titleMenuNr);
   }     
   else if(parameter == "labelmenu")
   {
      ans->MakeAndSetFloat(pp->labelMenuNr);
   }  
	else if(parameter == "annotationcallback")
	{
		ans->MakeAndSetString(pp->annotationCallback);
	}
	else if(parameter == "imageinsetcallback")
	{
		ans->MakeAndSetString(pp->imageInsetCallback);
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

   return(OK);
}



int GetImageWinParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   PlotWindow *pp = (PlotWindow*)(obj->data);

   if(parameter == "bkgmenu")
   {
      ans->MakeAndSetFloat(pp->bkgMenuNr);
   }  
   else if(parameter == "axesmenu")
   {
      ans->MakeAndSetFloat(pp->axesMenuNr);
   }     
   else if(parameter == "titlemenu")
   {
      ans->MakeAndSetFloat(pp->titleMenuNr);
   }     
   else if(parameter == "labelmenu")
   {
      ans->MakeAndSetFloat(pp->labelMenuNr);
   }  
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

   return(OK);
}


int GetProgressBarParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   long progMin,progMax,progValue;
   
   if(parameter == "min_value" || parameter == "minvalue")
   {
	   progMin = SendMessage((HWND)obj->hWnd,PBM_GETRANGE,true,0);	
		ans->MakeAndSetFloat((float)(progMin));
	}
   else if(parameter == "max_value" || parameter == "maxvalue")
   {
		progMax = SendMessage((HWND)obj->hWnd,PBM_GETRANGE,false,0);	
		ans->MakeAndSetFloat((float)(progMax));
	}
   else if(parameter == "value")
   {
		progValue = SendMessage((HWND)obj->hWnd,PBM_GETPOS,0,0);	
		ans->MakeAndSetFloat((float)(progValue));
	}
   else if(parameter == "range")
   {
	   progMin = SendMessage((HWND)obj->hWnd,PBM_GETRANGE,true,0);
		progMax = SendMessage((HWND)obj->hWnd,PBM_GETRANGE,false,0);
      float** m = MakeMatrix2D(2,1);
      m[0][0] = progMin;
      m[0][1] = progMax;
		ans->AssignMatrix2D(m,2,1);
	}
   else if(parameter == "orientation")
   {
		long type = GetWindowLong(obj->hWnd,GWL_STYLE);
		if(type & PBS_VERTICAL) 
		   ans->MakeAndSetString("vertical");
      else
		   ans->MakeAndSetString("horizontal");
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
 	return(OK);
}

int GetMessageParameter(ObjectData *obj, CText &parameter, Interface *itfc)
{  
   if(parameter == "text")
   {
      CArg arg;
      if(obj->data)
      {
         short n = arg.Count(obj->data);
         if(n != 2)
	      {
	         ErrorMessage("should be 2 parameters for getmessage");
	         return(ERR);
	      }
         itfc->retVar[1].MakeAndSetString(arg.Extract(1));
		   itfc->retVar[2].MakeAndSetString(arg.Extract(2));
         itfc->nrRetValues = 2;
      }
      else
	   {
	      ErrorMessage("no text defined for object %hd",obj->nr());
	      return(ERR);
	   }
	}
   else if(parameter == "source")  // Return the calling window number
   {                                     // -1 if called from a non-gui window
      WinData* win = messageSource;
      if(win)
		   itfc->retVar[1].MakeAndSetFloat(win->nr);
      else
		   itfc->retVar[1].MakeAndSetFloat(-1);
         itfc->nrRetValues = 1;
	}
   else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
 	return(OK);
}

/*******************************************************************************
* Extract the parameters for a check box
*
* Command syntax is value = getpar(win,obj,parameter)
*
* parameter  
*  "text" ... the text for the currently selected state
*  "value" .. the check state: 0-unchecked, 1-checked, 2-greyed
*  "init" ... return the initial state for the check box
*
*******************************************************************************/

int GetCheckBoxParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   CheckButtonInfo *info;
   long value;

   info = (CheckButtonInfo*)(obj->data);
 	CArg carg;
	carg.Count(info->states);

   if(parameter == "text")
   {
	   value = SendMessage(obj->hWnd,BM_GETCHECK,0,0);
      if(value == BST_INDETERMINATE)
	      ans->MakeAndSetString("disabled");
      else
	      ans->MakeAndSetString(carg.Extract(value+1));
   }
   else if(parameter == "value")
   {
	   value = SendMessage(obj->hWnd,BM_GETCHECK,0,0);
	   ans->MakeAndSetFloat((float)value);
	}
   else if(parameter == "init")
   {
	   ans->MakeAndSetString(carg.Extract(info->init));
	}	
   else if(parameter == "options") // return string as a list
   {
      long width;
      char **list = MakeListFromText(info->states, &width);
	   ans->MakeAndSetList(list,width);
	   FreeList(list,width);
	}	
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
 	return(OK);
}

/*******************************************************************************
* Extract the parameters for a text box
*
* Command syntax is value = getpar(win,obj,parameter)
*
* parameter  
*  "text" ... return the window text unchanged
*  "value" .. evaluate the window text first before returning
*
*******************************************************************************/

int GetTextBoxParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans)
{ 
   CText data;
   GetWindowTextEx(obj->hWnd,data);

   if(parameter == "text") // Return text verbatim
   {
	   ans->MakeAndSetString(data.Str());
	}
   else if(parameter == "color" || parameter == "fgcolor" )
   {
	   BYTE *data = (BYTE*)&obj->fgColor;
      float colors[3];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      ans->MakeMatrix2DFromVector(colors,3,1);
   }
   else if(parameter == "bkgcolor" || parameter == "bgcolor" )
   {
	   BYTE *data = (BYTE*)&obj->bgColor;
      float colors[3];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      ans->MakeMatrix2DFromVector(colors,3,1);
   }
   else if(parameter == "value") // Evaluate first
   {
      if(data.Size() == 0)
	   {
	      ans->MakeAndSetFloat(NaN); // Return NaN if blank
	   }
	   else
	   {
	      if(Evaluate(itfc,RESPECT_ALIAS,data.Str(),ans) == -1) // Evaluate and return in ans
         {
	         return(ERR);  
         }
	   }
	}	
   else if(parameter == "acceptkeyevents")
   {
		if(obj->acceptKeyEvents)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}	
   
 	return(OK);
}



int GetTextEditorParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans)
{ 
   CText data;
   GetWindowTextEx(obj->hWnd,data);
   EditParent *ep = (EditParent*)obj->data;

   if(parameter == "text") // Return text verbatim
   {
	   ans->MakeAndSetString(data.Str());
	}
   else if(parameter == "filename") // Return the filename
   {
      ans->MakeAndSetString(ep->editData[ep->curRegion]->edName);
	}
   else if (parameter == "getcurline") // Get currently selected line nr.
   {
      EditRegion* er = ep->editData[ep->curRegion];
      long linePos = GetCurrentLineNumber(er->edWin);
      ans->MakeAndSetFloat((float)linePos);
   }
   else if (parameter == "getcursorpos") // Get the current cursor position
   {
      long start, end;
      EditRegion* er = ep->editData[ep->curRegion];
      GetEditSelection(er->edWin, start, end);
      ans->MakeAndSetFloat((float)start);
   }
   else if (parameter == "gettopline") // Get the top visible line number
   {
      EditRegion* er = ep->editData[ep->curRegion];
      long lineNr = SendMessage(er->edWin, EM_GETFIRSTVISIBLELINE, (WPARAM)0, (LPARAM)0);
      ans->MakeAndSetFloat((float)lineNr);
   }
   else if(parameter == "pathname") // Return the pathname
   {
      char *path = ep->editData[ep->curRegion]->edPath;
      int sz = strlen(path);
      if(path[sz-1] == '\\')
      {
         char *temp = new char[sz+1];
         strcpy(temp,path);
         temp[sz-1] = '\0';         
		   ans->MakeAndSetString(temp);
         delete [] temp;
      }
      else
		   ans->MakeAndSetString(path);
	}
   else if(parameter == "wordwrap") // Whether word wrap is enabled or not
   {
      if(ep->wordWrap)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
	}
   else if(parameter == "readonlytext") // Whether the text is readonly or not
   {
      EditRegion *er = ep->editData[ep->curRegion];
      if(er->readOnly)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
	}
   else if(parameter == "showcontextualmenu") // Whether a contextual menu is displayed or not
   {
      if(ep->showContextualMenu)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
	}
   else if(parameter == "showsyntaxcoloring") // Whether syntax colouring should be used or not
   {
      if(ep->showSyntaxColoring)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
	}
   else if(parameter == "showsyntaxdescription") // Whether syntax descriptions should be displayed or not
   {
      if(ep->showSyntaxDescription)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
	}
   else if(parameter == "syntaxcoloringstyle") // The type of syntax coloring to use
   {
      EditRegion *er = ep->editData[ep->curRegion];
      if(er->syntaxColoringStyle == ASM_STYLE)
		   ans->MakeAndSetString("asm");
      else if(er->syntaxColoringStyle == MACRO_STYLE)
		   ans->MakeAndSetString("macro"); 
      else
		   ans->MakeAndSetString("none"); 
	}
   else if(parameter == "modified") // Whether the text has been modified or not
   {
      EditRegion *er = ep->editData[ep->curRegion];
      if(er->edModified)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
	}
	else if(parameter == "labelobj")
	{
		EditParent* ep = (EditParent*)obj->data;
      EditRegion *er = ep->editData[ep->curRegion];
		ans->MakeAndSetFloat(er->labelCtrlNr);
	}
   else if(parameter == "iscurrent") // Whether the editor is current
   {
      EditRegion *er = ep->editData[ep->curRegion];
      if(er == curEditor)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
	}
	else if(parameter == "getmacroname")
	{
		EditParent *ep = (EditParent*)obj->data;
		EditRegion *er = ep->editData[ep->curRegion];	

	   CText cname;
      long startWord,endWord;
      char *name;
		bool classCmd=false;
		bool funcCmd=false;
		short userClassCmd=0;
		char **list = 0;
		int cnt = 0;
		extern CText *gClassName;
		extern CText *gClassMacro;
		extern int gNrClassDeclarations;

      GetEditSelection(er->edWin,startWord,endWord);	      
      cname = ExpandToFullWord(curEditor->edWin," +-*/%^=(,[$\t<>:"," ()\n",startWord,endWord,classCmd,userClassCmd,funcCmd);
		int pos;
	   if((pos = cname.FindSubStr(0,"->")) >= 0)
		{
			CText className = cname.Middle(0,pos-1);
			if(className == "self")
				className = er->edName;
	     	int n = gNrClassDeclarations;
			for(int i = 0; i < n; i++) // Convert class to macro name if possible
			{
				if(gClassName[i] == className)
				{
					className = gClassMacro[i];
					break;
				}
			}
			ans->MakeAndSetString(className.Str());
			return(OK);
		}
	   if((pos = cname.FindSubStr(0,":")) >= 0)
		{
			CText macroName = cname.Middle(0,pos-1);
			ans->MakeAndSetString(macroName.Str());
			return(OK);
		}
		ans->MakeNullVar();
		return(OK);
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}	
   
 	return(OK);
}

/*******************************************************************************
* Extract the parameters for statictext
*
* Command syntax is value = getpar(win,obj,parameter)
*
* parameter  
*  "text" ... return the window text unchanged
*  "just" ... left, right, centre
*
*******************************************************************************/

int GetStaticTextParameter(ObjectData *obj, CText &parameter, Variable *ans)
{ 
   CText data;
   StaticTextInfo *info;
   info = (StaticTextInfo*)(obj->data);

   GetWindowTextEx(obj->hWnd,data);

   if(parameter == "text") // Return text verbatim
   {
	   ans->MakeAndSetString(data.Str());
	}
   else if(parameter == "multiline")
   {
      if(info->multiLine)
         ans->MakeAndSetString("yes");
      else
         ans->MakeAndSetString("no");
   }
   else if(parameter == "color" || parameter == "fgcolor" )
   {
	   BYTE *data = (BYTE*)&obj->fgColor;
      float colors[3];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      ans->MakeMatrix2DFromVector(colors,3,1);
   }
   else if(parameter == "bkgcolor" || parameter == "bgcolor" )
   {
	   BYTE *data = (BYTE*)&obj->bgColor;
      float colors[3];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      ans->MakeMatrix2DFromVector(colors,3,1);
   }
   else if(parameter == "justification" || parameter == "just")
   {
      long style = GetWindowLong(obj->hWnd,GWL_STYLE);
      style = style & 0x0003;

   // Figure out new position      
      if(style == SS_LEFT)
         ans->MakeAndSetString("left");
      else if(style == SS_RIGHT)
         ans->MakeAndSetString("right");
      else if(style == SS_CENTER)
         ans->MakeAndSetString("centre");
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}	
      
 	return(OK);
}

/*******************************************************************************
* Extract the parameters for a text menu 
*
* Command syntax is value = getpar(win,obj,parameter)
*
* parameter  
*  "text" ... return the window text unchanged
*  "value" .. evaluate the window text first before returning
*  "menu" ... extract the menu as a list
*  "index" .. extract current index (1 based)
*  "zindex" . extract current index (0 based)
*
*******************************************************************************/

int GetTextMenuParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans)
{
   HWND hWnd;
   short index;
   
   hWnd = obj->hWnd;
   
   if(parameter == "text") // Return text verbatim
   {
      CText data;
      GetWindowTextEx(hWnd,data);
		ans->MakeAndSetString(data.Str());
	}
   else if(parameter == "value") // Evaluate first
   {
      CText data ;
      GetWindowTextEx(hWnd,data);
		ans->MakeAndSetString(data.Str());
	   if(Evaluate(itfc,RESPECT_ALIAS,data.Str(),ans) == -1)
	      return(ERR);  
	}
   else if(parameter == "index") // Current selection index (1 based)
   {
      index = SendMessage(obj->hWnd,CB_GETCURSEL,0,0);
		ans->MakeAndSetFloat((float)index+1);  
	}
   else if(parameter == "zindex") // Current selection index (0 based)
   {
      index = SendMessage(obj->hWnd,CB_GETCURSEL,0,0);
		ans->MakeAndSetFloat((float)index);  
	}
	else if(parameter == "acceptkeyevents")
   {
		if(obj->acceptKeyEvents)
		   ans->MakeAndSetString("true"); 
      else
		   ans->MakeAndSetString("false"); 
   }
   else if(parameter == "menu") // return menu as a list
   {
      long entries = SendMessage(obj->hWnd,CB_GETCOUNT,0,0);
      if(entries > 0)
      {
         char **list = NULL; 
         long i;
         for(i = 0; i < entries; i++)
         {
            long length = SendMessage(obj->hWnd,CB_GETLBTEXTLEN,(WPARAM)i,0);
            char *data = new char[length+1];
            SendMessage(obj->hWnd,CB_GETLBTEXT,(WPARAM)i,(LPARAM)data);
            AppendStringToList(data,&list,i);
         }
	      ans->MakeAndSetList(list,i);
	      FreeList(list,i);
      }
      else
      {
         ans->MakeNullVar();
      }
	}	
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}	
      
 	return(OK);
}


int SetGridCtrlParameter(ObjectData * obj, CText &parameter, Variable *value)
{
	BabyGrid* grid = (BabyGrid*) obj->data;
	if (!grid)
	{
		ErrorMessage("No data associated with grid");
		return (ERR);
	}

	if (parameter == "columns")
	{		
      if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
		grid->setColCount(value->GetReal());
	}
	else if (parameter == "rows")
	{
		if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
		grid->setRowCount(value->GetReal());		
	}
	else if (parameter == "colheaderheight")
	{
		if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
		grid->setHeaderRowHeight(value->GetReal());
	}
	else if (parameter == "rowheaderwidth")
	{
		if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
		grid->setColWidth(0,value->GetReal());
	}
	else if (parameter == "rowheight")
	{
		if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
		grid->setRowHeight(value->GetReal());
	}	
	else if (parameter == "showcollabels")
	{
		if(value->GetType() == UNQUOTED_STRING)
      {
			CText mode = value->GetString();
			grid->setColsNumbered(("false" == mode)? true : false);
		}
	}
	else if (parameter == "showrowlabels")
	{
		if(value->GetType() == UNQUOTED_STRING)
      {
			CText mode = value->GetString();
			grid->setRowsNumbered(("false" == mode)? true : false);
		}
	}
   else
	{
	   ErrorMessage("invalid parameter requested");	
	   return(ERR);
	}
	return OK;
}


/*******************************************************************************
* Modify the parameters for a button
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter         value
*  "mode"        special button functions: default, panic, abort
*
*******************************************************************************/

int SetButtonParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   HWND hWnd;
   WinData *win;
 
   PushButtonInfo* info = (PushButtonInfo*)obj->data;

   if(parameter == "mode")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         long type = GetWindowLong(obj->hWnd,GWL_STYLE);
         hWnd = obj->hwndParent;
	      win = GetWinDataClass(hWnd);

         CText mode = value->GetString();

         if(mode == "default")
         {   
            if(!win7Mode)
               SetWindowLong(obj->hWnd,GWL_STYLE,type | BS_DEFPUSHBUTTON);
            info->defaultButton = true;
            win->defaultID = obj->nr();
         }
         else if(mode == "normal")
         {
            if(!win7Mode)
               SetWindowLong(obj->hWnd,GWL_STYLE,type & ~BS_DEFPUSHBUTTON);
             info->defaultButton = false;
         }
         else if(mode == "cancel")
         { 
            win->cancelID = obj->nr();
         }
         else if(mode == "abort")
         {   
            win->abortID = obj->nr();
         } 
         else if(mode == "panic")
         {   
            win->panicID = obj->nr();
         } 
         else
         {
            ErrorMessage("invalid button mode");
            return(ERR);
         }
         MyInvalidateRect(obj->hWnd,NULL,false);
      }
   }
   else if(parameter == "icon")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *fileName = value->GetString();
         if(IsFile(fileName))
         {
            PushButtonInfo* info = (PushButtonInfo*)obj->data;
            if(info)
            {
               if(win7Mode)
               {
                  if(info->hImage) // Delete any button image 
                     ::delete (Bitmap*) info->hImage;

                  wchar_t* fileNameWide = CharToWChar(fileName);
                  Gdiplus::Bitmap *bm = ::new Gdiplus::Bitmap(fileNameWide);
                  info->hImage = (void*)bm;
                  SysFreeString((BSTR)fileNameWide);
               }
               else
               {
                  if(info->hImage) // Delete any button image 
                     DeleteObject(info->hImage);
   
                  HBITMAP hBitmap;
                  wchar_t* fileNameWide = CharToWChar(fileName);
                  Bitmap bm(fileNameWide);
                  bm.GetHBITMAP(NULL, &hBitmap);
                  long type = GetWindowLong(obj->hWnd,GWL_STYLE);
                  SetWindowLong(obj->hWnd,GWL_STYLE,type | BS_BITMAP);
                  SendMessage(obj->hWnd, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
                  info->hImage = (void*)hBitmap;
                  SysFreeString(fileNameWide);
               }
					MyInvalidateRect(obj->hWnd,NULL,false);
            }
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
	else if(parameter == "text")
	{
      if(value->GetType() == UNQUOTED_STRING)
      {
         SetWindowText(obj->hWnd,value->GetString());
      }
      else
      {
         ErrorMessage("text value should be a string");
         return(ERR);
      }        
	} 
   else if(parameter == "color" || parameter == "fgcolor")
   {
      if(SetColor(obj->fgColor, value) == ERR)
         return(ERR);
   }
   else if(parameter == "bgcolor" || parameter == "bkgcolor")
   {
      if(SetColor(obj->bgColor, value) == ERR)
         return(ERR);
   }
   else if(parameter == "italics")
   {
      if(info && value->GetType() == UNQUOTED_STRING)
      {
         char* txt = value->GetString();
         if(!strcmp(txt,"true"))
            info->italicText = true;
         else
            info->italicText = false;
         MyInvalidateRect(obj->hWnd,NULL,false);
      }
      else
      {
         ErrorMessage("italic value should be 'true' or 'false'");
         return(ERR);
      }
   }
   else if(parameter == "bold")
   {
      if(info && value->GetType() == UNQUOTED_STRING)
      {
         char* txt = value->GetString();
         if(!strcmp(txt,"true"))
            info->boldText = true;
         else
            info->boldText = false;
         MyInvalidateRect(obj->hWnd,NULL,false);
      }
      else
      {
         ErrorMessage("bold value should be 'true' or 'false'");
         return(ERR);
      }
   }
   else if(parameter == "fontname")
   {
      if(info && value->GetType() == UNQUOTED_STRING)
      {
         char* txt = value->GetString();
         info->fontName  = txt;
         MyInvalidateRect(obj->hWnd,NULL,false);
      }
      else
      {
         ErrorMessage("boldtext value should be 'true' or 'false'");
         return(ERR);
      }
   }
   else if(parameter == "fontsize")
   {
      if(info && value->GetType() == FLOAT32)
      {
         int height = nint(value->GetReal());
         if(height >= 0)
         {
            info->fontHeight = height;
            MyInvalidateRect(obj->hWnd,NULL,false);
         }
         else
         {
            ErrorMessage("fontsize value should be a positive integer");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("fontsize value should be an integer");
         return(ERR);
      }
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}   
   return(OK);
}


   
/*******************************************************************************
* Modify the parameters for a radio button
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter                value
*  "value"         which option to choose (1,2 ...)
*  "text"          choose option by specify option text
*  "init"          set the initial value for the control
*                  by passing the initial option text.
*
*******************************************************************************/

int SetRadioButtonParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   RadioButtonInfo *info;
   short type;
   long i,rbValue = 0;

   info = (RadioButtonInfo*)(obj->data);
	CArg carg;
	carg.Count(info->states);

   type = value->GetType();

   if(parameter == "value")
   {
      if(type == FLOAT32)
      {
         float result = value->GetReal();
         if(result == nint(result))
         {
            rbValue = nint(result) - 1 ;
         }
      }
      else
		{
		   ErrorMessage("expecting an integer");
		   return(ERR);
		}      
   }
   else if(parameter == "text")
   {
      if(type == UNQUOTED_STRING)
      {
         for(i = 1; i <= info->nrBut; i++)
         {
            if(!strcmp(value->GetString(),carg.Extract(i))) 
            {
               rbValue = i-1;
               break;
            }
         }
      }
      else
		{
		   ErrorMessage("expecting a string (value/text)");
		   return(ERR);
		}       
   }
   else if(parameter == "options" || parameter == "option_string"  || parameter == "option_list")
   {
      if(type == UNQUOTED_STRING)
      {
         char* options = value->GetString();
         if(info->states) delete [] info->states;
         info->states = new char[strlen(options)+1];
         strcpy(info->states,options);
         ReorientRadioButtons(obj,info);
         return(OK);
      }
      else if(type == LIST)
      {
         char **list = value->GetList();
         int i;
         long length = 0; 
         long nrOptions = value->GetDimX();
         if(nrOptions <= 0)
         {
            ErrorMessage("invalid number of options");
            return(ERR);      
         } 
         if(info->states) delete [] info->states;
         for(i = 0; i < nrOptions; i++)
            length += strlen(list[i])+1; // +1 for comma
         info->states = new char[length+1]; // +1 for null
         info->states[0] = '\0';
         for(i = 0; i < nrOptions-1; i++)
         {
            strcat(info->states,list[i]);
            strcat(info->states,",");
         }
         strcat(info->states,list[i]);
         ReorientRadioButtons(obj,info);
         return(OK);
      }
      else
      {
         ErrorMessage("invalid data type for option parameter");
         return(ERR);      
      } 
	}
   else if(parameter == "orientation") // set orientation
   {
      CText txt;
      if(!strncmp(value->GetString(),"horizontal",5))
      {
         if(info->orient == 'v')
         {
            info->orient = 'h';
            ReorientRadioButtons(obj,info);
            return(OK);
         }
      }
      else if(!strncmp(value->GetString(),"vertical",4))
      {
         if(info->orient == 'h')
         {
            info->orient = 'v';
            ReorientRadioButtons(obj,info);
            return(OK);
         }
      }
      else
      {
         ErrorMessage("invalid orientation");
         return(ERR);      
      } 
	}
   else if(parameter == "init")
   {
      short n;
      if(type == FLOAT32)
      {
         n = nint(value->GetReal());
         if(n >= 1 && n <= info->nrBut)
         {
            info->init = n;
         }
         else
	      {
	         ErrorMessage("invalid initial value");
	         return(ERR);      
	      } 
      }
      else if(type == UNQUOTED_STRING)
      {
         n = carg.Find(value->GetString());
         if(n >= 1 && n <= info->nrBut)
         {
            info->init = n;
         }
         else
	      {
	         ErrorMessage("invalid initial value");
	         return(ERR);      
	      }             
      }
      else
      {
         ErrorMessage("invalid data type for init parameter");
         return(ERR);      
      } 
   }
   else if(parameter == "spacing")
   {
      info->spacing = value->GetReal();
	   for(int i = 0; i < info->nrBut; i++)
	   {
         if(info->orient == 'h')
            MoveWindow(info->hWnd[i],obj->xo+info->spacing*i,obj->yo,14,14,true);  
         else
            MoveWindow(info->hWnd[i],obj->xo,obj->yo+info->spacing*i,14,14,true);  
	   }
      if(info->orient == 'h')
      {
         obj->wo = (info->nrBut-1)*info->spacing+14;
         obj->ho = 14;
      }
      else
      {
         obj->wo = 14;
         obj->ho = (info->nrBut-1)*info->spacing+14;
      }
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	} 

	
// Check for errors
   if(rbValue < 0 || rbValue >= info->nrBut)
   {
      ErrorMessage("invalid radiobutton value/text selection");
      return(ERR);
   }
 
// Set requested radio button
   for(int i = 0; i < info->nrBut; i++)
   {
      if(i != rbValue)
         SendMessage(info->hWnd[i],BM_SETCHECK,0,0);
      else
         SendMessage(info->hWnd[i],BM_SETCHECK,1,0);
   }

// Make sure the object is redrawn
   UpdateObject(obj);

 	return(OK);
}

// When radio buttons have their orientation changed or the number of options modified
// we need to make a new set to replace the old.
void ReorientRadioButtons(ObjectData *obj, RadioButtonInfo *info)
{
   CText orient;
   CText txt;
   Interface itfc;
	CArg carg;

	int nr = obj->nr();
   CText x_exp,y_exp;
   obj->GetXExp(&x_exp,false);
   obj->GetYExp(&y_exp,false);
   int sp = info->spacing;
   if(info->orient == 'v')
      orient = "vertical";
   else
      orient = "horizontal";

   char *init;
   init = new char[strlen(info->states)+1];
	carg.Count(info->states);
   strcpy(init,carg.Extract(info->init));
   txt.Format("%d,\"%s\",\"%s\",%d,\"%s\",\"%s\",\"%s\",\"%s\"",nr,x_exp.Str(),y_exp.Str(),sp,orient.Str(),info->states,init,obj->command);
   delete [] init;
   MyInvalidateRect(obj->hwndParent,NULL,true);
   bool sel = obj->selected_;
   bool en = IsWindowEnabled(info->hWnd[0]);

   if(ReplaceRadioButtonObject(&itfc ,obj, txt.Str()) == OK)
   {
      obj->EnableObject(en);
      obj->selected_ = sel;
      UpdateObject(obj);
   }
}

// Reorient the updown control obj
void ReorientUpDownControl(ObjectData *obj, char *orient)
{
   CText txt;
   Interface itfc;

   int nr = obj->nr();
   CText x_exp,y_exp;
   obj->GetXExp(&x_exp,false);
   obj->GetYExp(&y_exp,false);
   int w = obj->wo;
   int h = obj->ho;

   txt.Format("%d,\"%s\",\"%s\",%d,%d,\"%s\",\"%s\"",nr,x_exp.Str(),y_exp.Str(),w,h,orient,obj->command);

   MyInvalidateRect(obj->hwndParent,NULL,true);
   bool sel = obj->selected_;
   bool en = IsWindowEnabled(obj->hWnd);

   if(ReplaceUpDownObject(&itfc ,obj, txt.Str()) == OK)
   {
      obj->EnableObject(en);
      obj->selected_ = sel;
      UpdateObject(obj);
   }
}

/*******************************************************************************
* Modify the parameters for the combo box
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter         value
*  "menu"        1D matrix or list
*  "text" ...... string
*  "index" ..... extract current index (1 based)
*  "zindex" .... extract current index (0 based)
*******************************************************************************/

int SetTextMenuParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   char str[50];
   short type = value->GetType();

   if(parameter == "menu") // Modify the contents of the combo box menu
   {
	   if(type == MATRIX2D) // Apply the contents of a 1D matrix to a menu
	   {
	      short rows = value->GetDimY();
	      short cols = value->GetDimX();
         float **mat = value->GetMatrix2D();
			short n = 0;

	      do
	      {
            n = SendMessage(obj->hWnd, CB_DELETESTRING, 0, 0);
         }
         while(n > 0);
	               
	      if(rows == 1)
	      {
	         for(short i = 0; i < cols; i++)
	         {
	            if(mat[0][i] == nint(mat[0][i]))
	               sprintf(str,"%ld",nint(mat[0][i]));
	            else
	               sprintf(str,"%g",mat[0][i]);

               SendMessage(obj->hWnd, CB_ADDSTRING, 0, (LPARAM) str);
            }
         }
         else
         {
            ErrorMessage("Can only apply a 1D matrix to a menu");
            return(ERR);
         }
       }
       else if(type == LIST)	// Apply the contents of a text list to a menu
       {		  
			 int n = 0;
	       do // Delete current menu
	       {
             n = SendMessage(obj->hWnd, CB_DELETESTRING, 0, 0);
          }
          while(n > 0);
               
          for(short i = 0; i < value->GetDimX(); i++) // Add new one from list
          {
              SendMessage(obj->hWnd, CB_ADDSTRING, 0, (LPARAM) value->GetList()[i]);
          }	 
       }            
	}
	else if(parameter == "text") // Modify the contents of the combo box text field
	{
	   short index;
	   if(type == UNQUOTED_STRING)
	   {
         index = SendMessage(obj->hWnd, CB_FINDSTRINGEXACT, -1, (LPARAM)value->GetString());
         if(index == CB_ERR)
            SendMessage(obj->hWnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)value->GetString());
         else 
            SendMessage(obj->hWnd,CB_SETCURSEL,(WPARAM)index,(LPARAM)0);         
      }
      else if(type == FLOAT32)
      {
	      sprintf(str,"%g",value->GetReal());      
         index = SendMessage(obj->hWnd, CB_FINDSTRINGEXACT, -1, (LPARAM)str);
         if(index == CB_ERR)
            SendMessage(obj->hWnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)str);
         else 
            SendMessage(obj->hWnd,CB_SETCURSEL,(WPARAM)index,(LPARAM)0); 
      }	      
	}
	else if(parameter == "index") // Copy the indexed menu item to the text field
	{
	   if(type == FLOAT32)
	   {
	      short pos = nint(value->GetReal()-1);
	      if(pos >= 0)
	      {
            if(SendMessage(obj->hWnd, CB_SETCURSEL, (WPARAM) pos, (LPARAM)0) == CB_ERR)
            {
               ErrorMessage("invalid index");
               return(ERR);
            }
         }
         else
	      {
	         ErrorMessage("'index' parameter must be >= 1");
	         return(ERR);
	      }             
      }  
      else
      {
         ErrorMessage("invalid data type for 'index' parameter");
         return(ERR);
      }    
	}
	else if(parameter == "zindex") // Copy the indexed menu item to the text field
	{
	   if(type == FLOAT32)
	   {
	      short pos = nint(value->GetReal());
	      if(pos >= 0)
	      {
            if(SendMessage(obj->hWnd, CB_SETCURSEL, (WPARAM) pos, (LPARAM)0) == CB_ERR)
            {
               ErrorMessage("invalid index");
               return(ERR);
            }
         }
         else
	      {
	         ErrorMessage("'zindex' parameter must be >= 0");
	         return(ERR);
	      }             
      }  
      else
      {
         ErrorMessage("invalid data type for 'zindex' parameter");
         return(ERR);
      }    
	}	
	else if(parameter == "acceptkeyevents")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"yes") || !strcmp(response,"true"))
         {
             obj->acceptKeyEvents = true;
         }
         else
         {
             obj->acceptKeyEvents = false;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
   else if(parameter == "selectall")
   {
 	   SendMessage(obj->hWnd, EM_SETSEL,(WPARAM)0, (LPARAM)-1);
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

// Make sure the object is redrawn
   UpdateObject(obj);

 	return(OK);
}


/*******************************************************************************
* Modify the parameters for track-bar (slider)
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter                value
*  "value"           set value for track-bar thumb.
*  "range"           set maximum and minimum range for track-bar.
*  "pagestep"        set step size when pressing page up/down keys.
*  "tickstep"        set step size for drawing ticks next to slider.
*  "arrowstep"       set step size when pressing arrow keys.
*
*******************************************************************************/

int SetSliderParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   short type = value->GetType();

   if(parameter == "value")
   {
	   if(type != FLOAT32)
	   {
	      ErrorMessage("invalid type for trackbar value");
	      return(ERR);
	   }
      long objType = GetWindowLong(obj->hWnd,GWL_STYLE);
		long trackPos;
      if(objType & TBS_VERT) // Range is inverted wrt our requirements when vertical
      {   
        long trackMax = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMAX,0,0);	
        long trackMin = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMIN,0,0);	
        trackPos = trackMax+trackMin-nint(value->GetReal());
      }
      else
      {
         trackPos = nint(value->GetReal());
      }	      
      trackPos = SendMessage((HWND)obj->hWnd,TBM_SETPOS,true,trackPos);
	}
	else if(parameter == "range")
	{
   // Set the range
	   short min = obj->lower;
	   short max = obj->upper;
	   if(max <= min)
	   {
	      ErrorMessage("invalid range for trackbar");
	      return(ERR);
	   }
      SendMessage((HWND)obj->hWnd,TBM_SETRANGE,true,(LPARAM) MAKELONG(min, max));	

   // Now set the value to zero
      //long type = GetWindowLong(obj->hWnd,GWL_STYLE);	   
      //if(type & TBS_VERT) // Range is inverted wrt our requirements when vertical
      //{   
      //  trackMax = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMAX,0,0);	
      //  trackMin = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMIN,0,0);	
      //  trackPos = trackMax+trackMin;
      //}
      //else
      //{
      //   trackPos = 0;
      //}	      
      //SendMessage((HWND)obj->hWnd,TBM_SETPOS,true,trackPos);
	}
	else if(parameter == "pagestep")
	{
	   if(type != FLOAT32)
	   {
	      ErrorMessage("invalid type for trackbar pagestep");
	      return(ERR);
	   } 	   
      SendMessage((HWND)obj->hWnd,TBM_SETPAGESIZE,(WPARAM)0,(LPARAM)nint(value->GetReal()));	
	}
	else if(parameter == "tickstep")
	{
	   if(type != FLOAT32)
	   {
	      ErrorMessage("invalid type for trackbar tickstep");
	      return(ERR);
	   } 
	   short step = nint(value->GetReal());
	   if(step <=  0)
	   {
         long objType = GetWindowLong(obj->hWnd,GWL_STYLE);	   
         SetWindowLong(obj->hWnd,GWL_STYLE, objType | TBS_NOTICKS);
      }
      else
      {	
         long objType = GetWindowLong(obj->hWnd,GWL_STYLE);	   
         SetWindowLong(obj->hWnd,GWL_STYLE, (objType & (~TBS_NOTICKS)) | TBS_AUTOTICKS);
         SendMessage((HWND)obj->hWnd,TBM_SETTICFREQ,(WPARAM)step,(LPARAM)0);
      }    	
	}
	else if(parameter == "arrowstep")
	{
	   if(type != FLOAT32)
	   {
	      ErrorMessage("invalid type for trackbar tickstep");
	      return(ERR);
	   } 
	   short step = nint(value->GetReal());
      SendMessage((HWND)obj->hWnd,TBM_SETLINESIZE,(WPARAM)0,(LPARAM)step);               	
	}
  else if(parameter == "orientation") // set orientation
  {
		long type = GetWindowLong(obj->hWnd,GWL_STYLE);

      if(!strncmp(value->GetString(),"horizontal",5))
      {
         if(type & TBS_VERT)
         {
				type = type & ~TBS_VERT;
				type = type | TBS_HORZ;
				SetWindowLong(obj->hWnd,GWL_STYLE,type);
            obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
            return(OK);
         }
      }
      else if(!strncmp(value->GetString(),"vertical",4))
      {
         if(!(type & TBS_VERT))
         {
				type = type & ~TBS_HORZ;
				type = type | TBS_VERT;
				SetWindowLong(obj->hWnd,GWL_STYLE,type);
            obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
            return(OK);
         }
      }
      else
      {
         ErrorMessage("invalid orientation");
         return(ERR);      
      } 
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

// Make sure the object is redrawn
   UpdateObject(obj);

 	return(OK);
}

/*******************************************************************************
* Modify the parameters for progress-bar
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter                value
*  "value"           set value for progress bar.
*  "range"           set maximum and minimum range for progress bar.
*  "stepsize"        set smallest increment progress bar can move.
*
*******************************************************************************/

int SetProgressBarParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   if(parameter == "value")
   {
	   if(value->GetType() != FLOAT32)
	   {
	      ErrorMessage("invalid type for progressbar position");
	      return(ERR);
	   }   
      int ivalue = nint(value->GetReal());
      SetProgressBarValue(obj,ivalue);
	}
	else if(parameter == "range")
	{
	   if(value->GetType() != MATRIX2D || value->GetDimX() != 2 || value->GetDimY() != 1)
	   {
	      ErrorMessage("invalid type or size for progressbar range");
	      return(ERR);
	   } 
      short min = nint(value->GetMatrix2D()[0][0]);
	   short max = nint(value->GetMatrix2D()[0][1]);
	   if(max <= min || min < 0)
	   {
	      ErrorMessage("invalid range for progressbar");
	      return(ERR);
	   }
	   
      SendMessage((HWND)obj->hWnd,PBM_SETRANGE,(WPARAM)0,(LPARAM) MAKELONG(min, max));	     	
	}
	else if(parameter == "stepsize")
	{
	   if(value->GetType() != FLOAT32)
	   {
	      ErrorMessage("invalid type for progressbar step size");
	      return(ERR);
	   }    
      SendMessage((HWND)obj->hWnd,PBM_SETSTEP,(WPARAM)nint(value->GetReal()),(LPARAM)0);     	
	}
	else if(parameter == "stepit")
	{
      SendMessage((HWND)obj->hWnd,PBM_STEPIT,(WPARAM)0,(LPARAM)0);	     	
	}	
   else if(parameter == "orientation")
   {
		long type = GetWindowLong(obj->hWnd,GWL_STYLE);

      if(!strncmp(value->GetString(),"horizontal",5))
      {
         if(type & PBS_VERTICAL)
         {
				type = type & ~PBS_VERTICAL;
				SetWindowLong(obj->hWnd,GWL_STYLE,type);
            obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
            return(OK);
         }
      }
      else if(!strncmp(value->GetString(),"vertical",4))
      {
         if(!(type & PBS_VERTICAL))
         {
				type = type | PBS_VERTICAL;
				SetWindowLong(obj->hWnd,GWL_STYLE,type);
            obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
            return(OK);
         }
      }
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

// Make sure the object is redrawn
   UpdateObject(obj); 

 	return(OK);
}

// This overrides the animation of the progress bar by stepping one higher and then back to the correct
// value. Found the suggestion here http://stackoverflow.com/questions/2217688/windows-7-aero-theme-progress-bar-bug

void SetProgressBarValue(ObjectData *obj, int value)
{
	int progMin = SendMessage(obj->hWnd,PBM_GETRANGE,true,0);	
	int progMax = SendMessage(obj->hWnd,PBM_GETRANGE,false,0);	
   if(value == progMin) // Code by passes animation
   {
      SendMessage(obj->hWnd,PBM_SETPOS,progMin+1,(LPARAM)0);     	
      SendMessage(obj->hWnd,PBM_SETPOS,progMin,(LPARAM)0); 
   }
   else if(value == progMax)
   {
      SendMessage(obj->hWnd,PBM_SETRANGE,(WPARAM)0,(LPARAM) MAKELONG(0, progMax+1));	     	

      SendMessage(obj->hWnd,PBM_SETPOS,progMax+1,(LPARAM)0);     	
      SendMessage(obj->hWnd,PBM_SETPOS,progMax,(LPARAM)0);  

      SendMessage(obj->hWnd,PBM_SETRANGE,(WPARAM)0,(LPARAM) MAKELONG(0, progMax));	     	

   }
   else
   {
      SendMessage((HWND)obj->hWnd,PBM_SETPOS,value+1,(LPARAM)0);     	
      SendMessage((HWND)obj->hWnd,PBM_SETPOS,value,(LPARAM)0); 
   }
}

/*******************************************************************************
* Modify the parameters for status-bar (bottom of window)
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter                value
*  "text"           text to display in status bar
*
*******************************************************************************/

int SetStatusBoxParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   if(parameter == "text")
   {
	   if(value->GetType() != UNQUOTED_STRING)
	   {
	      ErrorMessage("invalid type for status region text");
	      return(ERR);
	   }
      UpdateStatusWindow(obj->hWnd,0,value->GetString());
   }
   else if(parameter == "textregion")
   {
	   if(value->GetType() != UNQUOTED_STRING)
	   {
	      ErrorMessage("invalid type for status region text");
	      return(ERR);
	   }
      char *txt = value->GetString();
      CArg arg(',');
      int n = arg.Count(txt);
      if(n == 2)
      {
         int pos = 0;
         if(sscanf(arg.Extract(1),"%d",&pos) != 1)
	      {
	         ErrorMessage("invalid text position");
	         return(ERR);
	      }
         UpdateStatusWindow(obj->hWnd,pos,arg.Extract(2));
      }
      else
	   {
	      ErrorMessage("invalid format for status region text - should be 'region, text'");
	      return(ERR);
	   }
   }
   else if(parameter == "syntaxwindow")
   {
	   if(value->GetType() == UNQUOTED_STRING)
	   {
         char* valueTxt = value->GetString();
         StatusBoxInfo* info = (StatusBoxInfo*)obj->data;

         if(info)
         {
            if((!strcmp(valueTxt,"") || !strcmp(valueTxt,"add")))
            {
               if(info->subWindow)
                  DestroyWindow(info->subWindow);

	            info->subWindow = CreateWindow("SYNTAXWIN","",
                                    WS_VISIBLE | WS_CHILD,
                                    1,3,0,0,
                                    obj->hWnd,NULL,prospaInstance,NULL);
            }
            else if((!strcmp(valueTxt,"remove") || !strcmp(valueTxt,"delete")))
            {
               if(info->subWindow)
                  DestroyWindow(info->subWindow);
            }
         }
      }
	   else
	   {
	      ErrorMessage("invalid value type - expecting (\"add/remove\")");
	      return(ERR);
	   }
   }
	else
	{
	   ErrorMessage("invalid parameter specified");
	   return(ERR);
	}

// Make sure the object is redrawn
   MyUpdateWindow(obj->hwndParent);
 //  UpdateObject(obj->hwndParent); 

 	return(OK);
}

/*******************************************************************************
* Modify the parameters for a check box
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter                value
*  "value"         which option to choose (0/1)          [INT]
*  "options"       specify checked and unchecked options [STRING/LIST]
*  "text"          choose option by specify option text  [STRING]
*  "init"          set the initial value for the control [STRING/INT]
*                  by passing the initial option text
*                  or value (0/1)
*
*******************************************************************************/

int SetCheckBoxParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   long cbValue = 0;
   CheckButtonInfo *info;
   short type = value->GetType();

   info = (CheckButtonInfo*)(obj->data);
	CArg carg;
	carg.Count(info->states);

   if(parameter == "value")
   {
      if(value->GetType() == FLOAT32)
      {
         cbValue = nint(value->GetReal());

         if(cbValue != 0 && cbValue != 1)
         {
            ErrorMessage("check box value should be 0 or 1");
            return(ERR);      
         }
	      SendMessage(obj->hWnd,BM_SETCHECK,cbValue,0);
         UpdateObject(obj);  
      }
      else
      {
         ErrorMessage("invalid data type for value parameter");
         return(ERR);      
      }       
   }
   else if(parameter == "text")
   {
      if(type == UNQUOTED_STRING)
      {
			int i;
         for(i = 1; i <= 2; i++)
         {
            if(!strcmp(value->GetString(),carg.Extract(i))) 
            {
               cbValue = i-1;
               type = INTEGER;
               break;
            }
         }
         if(i == 3)
         {
            ErrorMessage("invalid checkbox status string");
            return(ERR);      
         }
	      SendMessage(obj->hWnd,BM_SETCHECK,cbValue,0);
         UpdateObject(obj);  
      }      
      else
      {
         ErrorMessage("invalid data type for text parameter");
         return(ERR);      
      }          	
	}
   else if(parameter == "options")
   {
      if(type == UNQUOTED_STRING)
      {
         char* options = value->GetString();
         if(info->states) delete [] info->states;
         info->states = new char[strlen(options)+1];
         strcpy(info->states,options);
      }
      else if(type == LIST)
      {
         char **list = value->GetList();
         if(info->states) delete [] info->states;
         long length = strlen(list[0]) + strlen(list[1]) + 1;
         info->states = new char[length+1];
         strcpy(info->states,list[0]);
         strcat(info->states,",");
         strcat(info->states,list[1]);
      }
      else
      {
         ErrorMessage("invalid data type for option parameter");
         return(ERR);      
      } 
	}	
   else if(parameter == "init")
   {
      short n;
      if(type == FLOAT32)
      {
         n = nint(value->GetReal());
         if(n == 0 || n == 1)
         {
            info->init = n+1;
         }
         else
	      {
	         ErrorMessage("invalid initial value");
	         return(ERR);      
	      } 
      }
      else if(type == UNQUOTED_STRING)
      {
         n = carg.Find(value->GetString());
         if(n == 1 || n == 2)
         {
            info->init = n;
         }
         else
	      {
	         ErrorMessage("invalid initial value");
	         return(ERR);      
	      }             
      }
      else
      {
         ErrorMessage("invalid data type for init parameter");
         return(ERR);      
      } 
   }	
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

// Make sure the object is redrawn
   UpdateObject(obj); 

   return(OK);
}

/************************************************************************
* Modify the parameters for the text box
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter      value
*  "text"        string
*  "password"    "yes"/"no"
*
************************************************************************/

int SetTextBoxParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   HWND hWnd;
   char str[50];
   short type = value->GetType();
   
   hWnd = obj->hWnd;

   if(parameter == "text")
   {  
      switch(type)
      {
         case(UNQUOTED_STRING):
         {
          // Remove \r\n or \n or \r at end of line
            char *txt = value->GetString();
            long len = strlen(txt);
            if(txt[len-1] == '\n' || txt[len-1] == '\r')
               txt[len-1] = '\0';
            len = strlen(txt);
            if(txt[len-1] == '\r')
               txt[len-1] = '\0';
            SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)value->GetString());   
            break;
         }
         case(FLOAT32):
            if(IsInteger(value->GetReal()))
			      sprintf(str,"%ld",(long)value->GetReal());
            else
			      sprintf(str,"%g",value->GetReal());
            SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)str);   
            break;
         case(FLOAT64):
            if(IsInteger(value->GetDouble()))
			      sprintf(str,"%ld",(long)value->GetDouble());
            else
			      sprintf(str,"%.17g",value->GetDouble());
            SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)str);   
            break; 
         case(INTEGER):
			   sprintf(str,"%ld",nint(value->GetReal()));
            SendMessage(hWnd,WM_SETTEXT,0,(LPARAM)(LPCTSTR)str);   
            break; 
         default:
            ErrorMessage("invalid data type");
            return(ERR);
      }
      UpdateObject(obj);      	
	}
   else if(parameter == "color" || parameter == "fgcolor")
   {
      if(SetColor(obj->fgColor, value) == ERR)
         return(ERR);
   }
   else if(parameter == "bgcolor" || parameter == "bkgcolor")
   {
      if(SetColor(obj->bgColor, value) == ERR)
         return(ERR);
   }
   else if(parameter == "password")
   {
      if(type == UNQUOTED_STRING)
      {
         if(!strcmp(value->GetString(),"on"))
         {
            SendMessage(hWnd,EM_SETPASSWORDCHAR,(WPARAM)'*',(LPARAM)0);
            UpdateObject(obj);   
       //     SetFocus(hWnd);
         }
         else if(!strcmp(value->GetString(),"off"))
         {
            SendMessage(hWnd,EM_SETPASSWORDCHAR,(WPARAM)0,(LPARAM)0);
            UpdateObject(obj);  
      //      SetFocus(hWnd);
         }
         else
         {
            ErrorMessage("invalid password status");
            return(ERR);
         }
      }
   }
  
   else if(parameter == "debug")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"yes") || !strcmp(response,"true"))
         {
             obj->debug = true;
         }
         else
         {
             obj->debug = false;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }

   else if(parameter == "selectall")
   {
 	   SendMessage(hWnd, EM_SETSEL,(WPARAM)0, (LPARAM)-1);
   }

	else if(parameter == "acceptkeyevents")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"yes") || !strcmp(response,"true"))
         {
             obj->acceptKeyEvents = true;
         }
         else
         {
             obj->acceptKeyEvents = false;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }

   //else if(parameter == "mode")
   //{
   //   if(value->GetType() == UNQUOTED_STRING)
   //   {
   //      long type = GetWindowLong(obj->hWnd,GWL_STYLE);
   //      HWND hWnd = obj->hwndParent;
	  //    WinData *win = GetWinDataClass(hWnd);

   //      CText mode = value->GetString();

   //      if(mode == "active")
   //      {   
   //         win->activeCtrl = true;
   //         obj->active = true;
   //      }  
   //      else
   //      {
   //         ErrorMessage("invalid textbox mode");
   //         return(ERR);
   //      }
   //      MyInvalidateRect(obj->hWnd,NULL,false);
   //   }
   //}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
   return(OK);
}


/*******************************************************************************
* Modify the parameters for static text (labels)
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter                     value
*  "text"               specify text to display
*  "justification"      how the text is to be placed
*
*******************************************************************************/

int SetStaticTextParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   HWND hWnd;
   long style;
   long x,y,w,h;
   char *str;
   StaticTextInfo *info;
   extern void  GetObjSizeParameters(WinData *win, ObjectData *objCur, RECT region, short ww, short wh, short yoff);


   info = (StaticTextInfo*)(obj->data);

   hWnd = obj->hWnd;
   
   HDC hdc = GetDC(hWnd);

   if(parameter == "text")
   {  
      switch(value->GetType())
      {
         case(UNQUOTED_STRING):
            str = new char[strlen(value->GetString())+1];
            strcpy(str,value->GetString());
            char replace[2];
            replace[0] = (char)0xB1; replace[1] = '\0';
            ReplaceSpecialCharacters(str,"+-",replace,-1);
            break;
         case(FLOAT32):
            str = new char[50];
			   sprintf(str,"%g",value->GetReal());
            break; 
         case(FLOAT64):
            str = new char[50];
			   sprintf(str,"%g",value->GetDouble());
            break;  
         case(INTEGER):
            str = new char[50];
			   sprintf(str,"%ld",(long)value->GetReal());
            break; 
         default:
            ErrorMessage("invalid data type");
            ReleaseDC(hWnd,hdc);
            return(ERR);
      }
   // Get width of old text
      CText txt;
      GetWindowTextEx(obj->hWnd,txt);
      SIZE size;
      HFONT oldFont = (HFONT)SelectObject(hdc,controlFont);
      GetTextExtentPoint32(hdc,txt.Str(),txt.Size(),&size);
      long wOld = size.cx;
      long hOld = size.cy;

   // Get new width and height of new textl   
      GetTextExtentPoint32(hdc,str,strlen(str),&size);
      w = size.cx;
      h = size.cy;

   // Get positioning mode
      style = GetWindowLong(hWnd,GWL_STYLE);
      style = style & 0x0003;

      if(obj->regionSized) // Region sized
      {
         long ww,wh;
         RECT r;
         GetClientRect(obj->winParent->hWnd,&r);
         ww = r.right-r.left;
         wh = r.bottom-r.top;
         GetObjSizeParameters(obj->winParent,obj,obj->region,ww,wh,0);
      }

   // Figure out new position      
		long xp = obj->xo;
		if(style == SS_LEFT)
         xp = obj->xo;
      else if(style == SS_RIGHT)
         xp = obj->xo + wOld - w;
		else if(style == SS_CENTER)
			xp = obj->xo + (wOld - w)/2;

   // Update the expression x position and width and height factors
      obj->xSzOffset = obj->xSzOffset + (xp-obj->xo);
      obj->wSzOffset = obj->wSzOffset + (w-wOld);
      obj->hSzOffset = obj->hSzOffset + (h-hOld); // In case old or new string is blank

   // Allow for multi-lined text
      if(info->multiLine == 1)
      {
         w = obj->wo;
         h = obj->ho;
         x = obj->xo;
         y = obj->yo;
      }
      else
      {
         obj->wo = w;
         obj->ho = 0;
         x = xp;
         y = obj->yo;
      }

   // Place the object 
      obj->Place(x, y, w, h,false);

   // Modify the window text
      SetWindowText(hWnd,str);
      delete [] str;

   // Tidy up   
      SelectObject(hdc,oldFont);
	}
   else if(parameter == "justification" || parameter == "just") 
   {
      if(value->GetType() != UNQUOTED_STRING)
      {
         ErrorMessage("invalid value type");
         ReleaseDC(hWnd,hdc);
         return(ERR);
      }

  // Get static text width and height
      CText txt;
      GetWindowTextEx(obj->hWnd,txt);
      SIZE size;
      HFONT oldFont = (HFONT)SelectObject(hdc,controlFont);
      GetTextExtentPoint32(hdc,txt.Str(),txt.Size(),&size);
      long w = size.cx;
      long h = size.cy;

      long oldStype = GetWindowLong(obj->hWnd,GWL_STYLE); // Get justification
      style = oldStype & 0x0003;
      long xjust = obj->xo;

   // Work out current xjust coordinate
      if(style == SS_LEFT)
         xjust = obj->xo;
      else if(style == SS_RIGHT)
         xjust = obj->xo + w;
      else if(style == SS_CENTER)
         xjust = obj->xo + w/2;

   // Get old style without justification
       long oldstyle = oldStype & (~0x0003);

      if(info->multiLine == 1)
      {
         w = obj->wo;
         h = obj->ho;
      }

      if(obj->regionSized) // Region sized
      {
         long ww,wh;
         RECT r;
         GetClientRect(obj->winParent->hWnd,&r);
         ww = r.right-r.left;
         wh = r.bottom-r.top;
         GetObjSizeParameters(obj->winParent,obj,obj->region,ww,wh,0);
      }

   // Figure out new position     
      long xp;
      if(!strcmp(value->GetString(),"left"))
      {
         xp = xjust;
         style = SS_LEFT;
      }
      else if(!strcmp(value->GetString(),"right"))
      {
         xp = xjust - w;
         style = SS_RIGHT;
      }
      else if(!strcmp(value->GetString(),"center") || !strcmp(value->GetString(),"centre"))
      {
         xp = xjust - w/2;
         style = SS_CENTER;
      }
      else
      {
         ErrorMessage("invalid justification option");
         ReleaseDC(hWnd,hdc);
         return(ERR);
      }

   // Update the expression position and width factors
      obj->xSzOffset = obj->xSzOffset + (xp-obj->xo);
      obj->xo = xp;


   // Allow for multi-lined text
      //if(obj->ho > 0)
      //{
      //   w = obj->wo;
      //   h = obj->ho;
      //}
      //else
      //{
      //   obj->wo = w;
      //   obj->ho = 0;
      //}
      if(info->multiLine == 1)
      {
         w = obj->wo;
         h = obj->ho;
         x = obj->xo;
         y = obj->yo;
      }
      else
      {
         obj->wo = w;
         obj->ho = 0;
         x = obj->xo;
         y = obj->yo;
      }
      SetWindowLong(obj->hWnd,GWL_STYLE,oldstyle|style);
     // Place the object  
      obj->Place(obj->xo, obj->yo, w, h, false);

   // Tidy up   
      SelectObject(hdc,oldFont);
   }
   else if(parameter == "color" || parameter == "fgcolor")
   {
      if(SetColor(obj->fgColor, value) == ERR)
      {
         ReleaseDC(hWnd,hdc);
         return(ERR);
      }
   }
   else if(parameter == "bgcolor" || parameter == "bkgcolor")
   {
      if(SetColor(obj->bgColor, value) == ERR)
      {
         ReleaseDC(hWnd,hdc);
         return(ERR);
      }
   }
   else if(parameter == "multiline")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         if(!strcmp(value->GetString(),"yes"))
         {
            info->multiLine = 1;
         }
         else
         {
            info->multiLine = 0;
         }
         CText txt;
         GetWindowTextEx(obj->hWnd,txt);
         SIZE size;
         HFONT oldFont = (HFONT)SelectObject(hdc,controlFont);

      // Get text dimensions for single line mode
         GetTextExtentPoint32(hdc,txt.Str(),txt.Size(),&size);
         w = size.cx;
         h = size.cy;

      // Get positioning mode
         style = GetWindowLong(hWnd,GWL_STYLE);
         style = style & 0x0003;

      // Figure out position      
	      long xp = obj->xo;

         obj->xSzOffset = xp;

      // Allow for multi-lined text
         if(info->multiLine == 1)
         {
            w = obj->wo;
            h = obj->ho;
            x = obj->xo;
            y = obj->yo;
         }
         else
         {
            obj->wo = w;
            obj->ho = 0;
            x = obj->xo;
            y = obj->yo;
         }

      // Place the object  
         obj->Place(x, y, w, h,true);

      // Tidy up   
         SelectObject(hdc,oldFont);
      }
      else
      {
         ErrorMessage("invalid data type for value parameter");
         ReleaseDC(hWnd,hdc);
         return(ERR);      
      }       
   }
   else if(parameter == "italics")
   {
      if(info && value->GetType() == UNQUOTED_STRING)
      {
         char* txt = value->GetString();
         if(!strcmp(txt,"true"))
            info->italicText = true;
         else
            info->italicText = false;
          HFONT ctrlFont = (HFONT)SendMessage(obj->hWnd,WM_GETFONT,(WPARAM)0,(LPARAM)0);
          LOGFONT lf = {0};
          GetObject(ctrlFont,sizeof(LOGFONT),&lf);

         if(info->italicText)
            lf.lfItalic = TRUE;

         HFONT newFont = CreateFontIndirect(&lf);

         SendMessage(obj->hWnd,WM_SETFONT,(WPARAM)newFont,(LPARAM)FALSE);

         if(ctrlFont != controlFont)
             DeleteObject(ctrlFont);

         MyInvalidateRect(obj->hWnd,NULL,false); 
      }
      else
      {
         ErrorMessage("italic value should be 'true' or 'false'");
         ReleaseDC(hWnd,hdc);
         return(ERR);
      }
   }
   else if(parameter == "bold")
   {
      if(info && value->GetType() == UNQUOTED_STRING)
      {
         char* txt = value->GetString();
         if(!strcmp(txt,"true"))
            info->boldText = true;
         else
            info->boldText = false;
          HFONT ctrlFont = (HFONT)SendMessage(obj->hWnd,WM_GETFONT,(WPARAM)0,(LPARAM)0);
          LOGFONT lf = {0};
          GetObject(ctrlFont,sizeof(LOGFONT),&lf);

         if(info->boldText)
            lf.lfWeight = FW_BOLD;

         HFONT newFont = CreateFontIndirect(&lf);

         SendMessage(obj->hWnd,WM_SETFONT,(WPARAM)newFont,(LPARAM)FALSE);

         if(ctrlFont != controlFont)
             DeleteObject(ctrlFont);

         MyInvalidateRect(obj->hWnd,NULL,false);
      }
      else
      {
         ErrorMessage("bold value should be 'true' or 'false'");
         ReleaseDC(hWnd,hdc);
         return(ERR);
      }
   }
   else if(parameter == "fontname")
   {
      if(info && value->GetType() == UNQUOTED_STRING)
      {
         char* txt = value->GetString();
         info->fontName  = txt;

          HFONT ctrlFont = (HFONT)SendMessage(obj->hWnd,WM_GETFONT,(WPARAM)0,(LPARAM)0);

          LOGFONT lf = {0};
          GetObject(ctrlFont,sizeof(LOGFONT),&lf);

          if(info->fontName.Size() > 0)
            strcpy(lf.lfFaceName,info->fontName.Str());

          HFONT newFont = CreateFontIndirect(&lf);

          SendMessage(obj->hWnd,WM_SETFONT,(WPARAM)newFont,(LPARAM)FALSE);

          if(ctrlFont != controlFont)
             DeleteObject(ctrlFont);

         MyInvalidateRect(obj->hWnd,NULL,false);
      }
      else
      {
         ErrorMessage("boldtext value should be 'true' or 'false'");
         ReleaseDC(hWnd,hdc);
         return(ERR);
      }
   }
   else if(parameter == "fontsize")
   {
      if(info && value->GetType() == FLOAT32)
      {
         int height = nint(value->GetReal());

         if(height >= 0)
         {
            info->fontHeight = height;
            
            LOGFONT lf = {0};

            HFONT ctrlFont = (HFONT)SendMessage(obj->hWnd,WM_GETFONT,(WPARAM)0,(LPARAM)0);

             GetObject(ctrlFont,sizeof(LOGFONT),&lf);

            if(info->fontHeight > 0)
               lf.lfHeight = info->fontHeight;

            HFONT newFont = CreateFontIndirect(&lf);

            SendMessage(obj->hWnd,WM_SETFONT,(WPARAM)newFont,(LPARAM)TRUE);


            if(ctrlFont != controlFont)
               DeleteObject(ctrlFont);

            MyInvalidateRect(obj->hWnd,NULL,false);
         }
         else
         {
            ErrorMessage("fontsize value should be a positive integer");
            ReleaseDC(hWnd,hdc);
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("fontsize value should be an integer");
         ReleaseDC(hWnd,hdc);
         return(ERR);
      }
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
      ReleaseDC(hWnd,hdc);
	   return(ERR);
	}

   ReleaseDC(hWnd,hdc);
   return(OK);
}

// Set the static text x position using an expression

bool SetStaticTextXExp(ObjectData *obj, Variable *value)
{
   HWND hWnd = obj->hWnd;
   HDC hdc = GetDC(hWnd);
   long x,y;
   StaticTextInfo *info;

   info = (StaticTextInfo*)(obj->data);

// Get static text width and height
   CText txt;
   GetWindowTextEx(obj->hWnd,txt);
   SIZE size;
   HFONT oldFont = (HFONT)SelectObject(hdc,controlFont);
   GetTextExtentPoint32(hdc,txt.Str(),txt.Size(),&size);
   long w = size.cx;
   long h = size.cy;

   long style = GetWindowLong(obj->hWnd,GWL_STYLE); // Getjustification
   style = style & 0x0003;

	if(value->GetType() == FLOAT32)
	{
      int xjust = nint(value->GetReal());

      if(style == SS_LEFT)
         obj->xo = xjust;
      else if(style == SS_RIGHT)
         obj->xo = xjust - w;
      else if(style == SS_CENTER)
         obj->xo = xjust - w/2;

      obj->xSzScale = 0;
      obj->xSzOffset = obj->xo;
	}  
	else if(value->GetType() == UNQUOTED_STRING)
	{
      float scale,off;
      bool region;
      if(GetObjectDimensions(value->GetString(), &scale, &off, &region) == ERR)
      {
         ErrorMessage("invalid x position");
         return(ERR);
      }  
      RECT rect;
      GetClientRect(obj->winParent->hWnd,&rect);
      short ww = (short)rect.right;
      if(style == SS_LEFT)
         obj->xo = ww*scale + off; 
      else if(style == SS_RIGHT)
         obj->xo = ww*scale + off - w;
      else if(style == SS_CENTER)
         obj->xo = ww*scale + off - w/2;
   
      obj->xSzScale = scale;
      obj->xSzOffset = off;
	} 
   else
   {
      SelectObject(hdc,oldFont);
      ReleaseDC(hWnd,hdc);
      return(false);
   }

   // Allow for multi-lined text
   //if(obj->ho > 0)
   //{
   //   w = obj->wo;
   //   h = obj->ho;
   //}
   //else
   //{
   //   obj->wo = w;
   //   obj->ho = 0;
   //}

   // Allow for multi-lined text
   if(info->multiLine == 1)
   {
      w = obj->wo;
      h = obj->ho;
      x = obj->xo;
      y = obj->yo;
   }
   else
   {
      obj->wo = w;
      obj->ho = 0;
      x = obj->xo;
      y = obj->yo;
   }

   // Place the object  
   obj->Place(x, y, w, h,true);

// Tidy up   
   SelectObject(hdc,oldFont);
   ReleaseDC(hWnd,hdc);

   return(true);
}

/*******************************************************************************
* Modify the parameters for color box control
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter                value
*  "color"          RGB color as a vector
*
*******************************************************************************/

int SetColorBoxParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   if(parameter == "color")
   {
      if(value->GetType() == MATRIX2D)
      {
         if(value->GetDimX() == 3 && value->GetDimY() == 1)
         {
            obj->data[0]  = nint(value->GetMatrix2D()[0][0]);
            obj->data[1]  = nint(value->GetMatrix2D()[0][1]);
            obj->data[2]  = nint(value->GetMatrix2D()[0][2]);
            obj->data[3]  = 0;
         }
         else if(value->GetDimX() == 4 && value->GetDimY() == 1)
         {
            obj->data[0]  = nint(value->GetMatrix2D()[0][0]);
            obj->data[1]  = nint(value->GetMatrix2D()[0][1]);
            obj->data[2]  = nint(value->GetMatrix2D()[0][2]);
            obj->data[3]  = nint(value->GetMatrix2D()[0][3]);
         }
      }
      else
      {
         ErrorMessage("invalid color matrix");
         return(ERR);
      }
  }
  else if(parameter == "debug")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"yes") || !strcmp(response,"true"))
         {
             obj->debug = true;
         }
         else
         {
             obj->debug = false;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

// Make sure the object is redrawn
   MyInvalidateRect(obj->hWnd,NULL,false);

   return(OK);
}


/*******************************************************************************
* Extract the parameters for a check box
*
* Command syntax is value = getpar(win,obj,parameter)
*
* parameter  
*  "text" ... the text for the currently selected state
*  "value" .. the check state: 0-unchecked, 1-checked, 2-greyed
*  "init" ... return the initial state for the check box
*
*******************************************************************************/

int GetColorBoxParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   if(parameter == "color")
   {
	   BYTE *data = (BYTE*)obj->data;
      float colors[4];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      colors[3] = data[3];
      ans->MakeMatrix2DFromVector(colors,4,1);
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
 	return(OK);
}


void UpdateObject(ObjectData *obj)
{
    MSG msg;
	 HWND hWnd = obj->hwndParent;

  // if(!obj->winParent->drawing)
//		return;


   if(PeekMessage(&msg, hWnd,0,0,PM_REMOVE))
   {     
      if(msg.message == WM_PAINT) // Not necessary since this always happens
         DispatchMessage(&msg);	  

      if(msg.message == WM_DRAWITEM)
         DispatchMessage(&msg);

      if(msg.message == WM_LBUTTONUP) // This is included so sliders work properly
         DispatchMessage(&msg);	  
             
      TranslateMessage(&msg);
   } 
}



/************************************************************************
* Modify the parameters for the HTML window
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter      value
*  "file"        string

*
************************************************************************/

int SetHTMLWinParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   extern void ContentWrite(HWND hWnd, char *txt);
   extern void ContentClear(HWND hWnd);
   extern void RunJavaScript(HWND hWnd, char *txt);

   HWND hWnd = obj->hWnd;

   if(parameter == "url")
   { 
      switch(value->GetType())
      {
         case(UNQUOTED_STRING):
         {
            LoadURLIntoHTMLViewer(hWnd,value->GetString());
            break;
         }
         default:
            ErrorMessage("invalid data type");
            return(ERR);
      }   	
	}
   else if(parameter == "text")
   {
      if(value->GetType() == UNQUOTED_STRING)
         ContentWrite(hWnd, value->GetString());
   }
   else if(parameter == "javascript")
   {
      if(value->GetType() == UNQUOTED_STRING)
         RunJavaScript(hWnd,value->GetString());
   }
   else if(parameter == "clear")
   {
      ContentClear(hWnd);
   }
   else if(parameter == "goback")
   {
      GoBackHTMLViewer(hWnd);
   }
   else if(parameter == "goforward")
   {
      GoForwardHTMLViewer(hWnd);
   }
   else if(parameter == "copyselection")
   {
      CopyHTMLViewerSelectionToClipBoard(hWnd);
   }
   else if(parameter == "runselection")
   {
      RunHTMLViewerSelection(hWnd);
   }
   else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
   return(OK);
}


int GetGridCtrlParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
	BabyGrid* grid = (BabyGrid*) obj->data;

	if (parameter == "columns")
	{		
		ans->MakeAndSetFloat(grid->getCols());       
	}
	else if (parameter == "rows")
	{
		ans->MakeAndSetFloat(grid->getRows());
	}
   else if (parameter == "currentcolumn")
   {
      ans->MakeAndSetFloat(grid->getCurrentCol()-1);
   }
   else if (parameter == "currentrow")
   {
      ans->MakeAndSetFloat(grid->getCurrentRow()-1);
   }
	else if (parameter == "colheaderheight")
	{
		ans->MakeAndSetFloat(grid->getHeaderRowHeight());
	}
	else if (parameter == "rowheaderwidth")
	{
		ans->MakeAndSetFloat(grid->getColWidth(0));
	}
	else if (parameter == "showcollabels")
	{
		if (grid->getColsNumbered())
			ans->MakeAndSetString("false");
		else
			ans->MakeAndSetString("true");	
	}
	else if (parameter == "showrowlabels")
	{
		if (grid->getRowsNumbered())
			ans->MakeAndSetString("false");
		else
			ans->MakeAndSetString("true");	
	}
   else
	{
	   ErrorMessage("invalid parameter requested");	
	   return(ERR);
	}
	return OK;
}


/************************************************************************
* Retrieve the parameters for the HTML window
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter      value
*  "file"        string

*
************************************************************************/

int GetHTMLWinParameter(Interface* itfc, ObjectData *obj, CText &parameter, Variable *ans)
{
	// TODO: Even though nrRetValues is set to zero where appropriate here, the zero is blown away at the end of 
	//  GetParameter. These aren't really "get"s, but have been put here since zero-argument functions are understood
	//  by the interpreter to be "get"s. The choice is either to call, eg, goback() with an empty string so that it's 
	//  interpreted as a "set" (ie, goback("")) or call it with an empty param list so it's interpreted as a "get" (ie,
	//  goback(). goback() isn't quite a get or a set, but we can make the fns called from here fit as "get"s by returning 
	//  the resulting url from goback, goforward, and returning the relevant text for copyselection() and runselection()
	
	HWND hWnd = obj->hWnd;
	
	if(parameter == "goback")
	{
		GoBackHTMLViewer(hWnd);
		itfc->nrRetValues = 0;
	}
	else if(parameter == "goforward")
	{
		GoForwardHTMLViewer(hWnd);
		itfc->nrRetValues = 0;
	}
	else if(parameter == "copyselection")
	{
		CopyHTMLViewerSelectionToClipBoard(hWnd);
	}
	else if(parameter == "runselection")
	{
		RunHTMLViewerSelection(hWnd);
	}
	else
	{
		ErrorMessage("invalid parameter requested");
		return(ERR);
	}

	return(OK);
}

/************************************************************************
    Evaluate object dimensions and position and return as fixed numbers
*************************************************************************/

short ProcessObjectPosition(WinData *win,Variable *wx, 
                            Variable *wy, Variable *ww, Variable *wh,
                            short &x,short &y, short &w, short &h,
                            ObjPos *pos)
{
// Work out client rectangle size
   RECT rect;
   GetClientRect(win->hWnd,&rect);

   short aw = (short)rect.right;
   short ah = (short)rect.bottom;

   pos->region = false;

// Process X position 
   if(wx)
   {
		if (ERR == ProcessValuePosition(wx, &pos->xs, &pos->xo, &pos->region, "x"))
		{
			return ERR;
		}
      x = aw*pos->xs + pos->xo;
   }
// Process Y position 
   if(wy)
   {
		if (ERR == ProcessValuePosition(wy, &pos->ys, &pos->yo, &pos->region, "y"))
		{
			return ERR;
		}
      y = ah*pos->ys + pos->yo;
   }

// Process width 
   if(ww)
   {
		if (ERR == ProcessValuePosition(ww, &pos->ws, &pos->wo, &pos->region, "x"))
		{
			return ERR;
		}
      w = aw*pos->ws + pos->wo;
   }

// Process height 
   if(wh)
   {
		if (ERR == ProcessValuePosition(wh, &pos->hs, &pos->ho, &pos->region, "y"))
		{
			return ERR;
		}
      h = ah*pos->hs + pos->ho;
   }

   return(OK);
}



/***************************************************************************************
*            Convert input position string for a screen object to two numbers          *
***************************************************************************************/

short GetObjectDimensions(char *str , float *scale, float *off, bool *region)
{
   long dimPos, mdPos, pmPos1,pmPos2;
   char md = '\0';
   CText input = str;
   *region = false;

// Parse string of the form [h,w] [/,*] [+,-] width [+,-] offset 
// Extract direction, width and offset of command 

   input.RemoveChar(' ');

   dimPos = input.FindSubStr(0,"ww");
   if(dimPos == -1)
   {
      dimPos = input.FindSubStr(0,"wh");
      if(dimPos == -1)
      {
         dimPos = input.FindSubStr(0,"rw");
         if(dimPos == -1)
         {
            dimPos = input.FindSubStr(0,"rh");
         }
         if(dimPos != -1)
            *region = true;
      }
   }
   mdPos = input.Search(0,'*');
   if(mdPos == -1)
      mdPos = input.Search(0,'/');
      
   pmPos1 = input.Search(0,'+');
   if(pmPos1 == -1)
      pmPos1 = input.Search(0,'-');

   pmPos2 = input.Search(pmPos1+1,'+');
   if(pmPos2 == -1)
      pmPos2 = input.Search(pmPos1+1,'-');

// Full expression - [h,w] [/,*] [+,-] scale [+,-] offset
   if(dimPos != -1 && mdPos != -1 && pmPos1 != -1 && pmPos2 != -1)
   {
      if(dimPos != 0 || mdPos != 2)
      {
         ErrorMessage("invalid object position expression");
         return(ERR);
      }

      CText scaleStr = input.Middle(3,pmPos2-1);
      CText offStr = input.End(pmPos2);
      md = input[mdPos];
      if(sscanf(scaleStr.Str(),"%f",scale) == 1)
      {
         if(md == '/') *scale = 1.0/(*scale);
         if(sscanf(offStr.Str(),"%f",off) == 1)
            return(OK);
      }

      ErrorMessage("invalid object position expression");
      return(ERR);
   }

// Expression - [h,w] [/,*] scale [+,-] offset or [h,w] [/,*] [+,-] scale
   else if(dimPos != -1 && mdPos != -1 && pmPos1 != -1 && pmPos2 == -1)
   {
      if(dimPos != 0 || mdPos != 2)
      {
         ErrorMessage("invalid object position expression");
         return(ERR);
      }

      CText scaleStr = input.Middle(3,pmPos1-1);
      CText offStr = input.End(pmPos1);
      md = input[mdPos];
      if(scaleStr.Size() == 0)
      {
         *off = 0;
         if(sscanf(offStr.Str(),"%f",scale) == 1)
            return(OK);
      }
      if(sscanf(scaleStr.Str(),"%f",scale) == 1)
      {
         if(md == '/') *scale = 1.0/(*scale);
         if(sscanf(offStr.Str(),"%f",off) == 1)
            return(OK);
      }

      ErrorMessage("invalid object position expression");
      return(ERR);
   }


// Dimensions and offset only
   else if(dimPos != -1 && mdPos == -1 && pmPos1 != -1 && pmPos2 == -1)
   {
      if(dimPos != 0 || pmPos1 != 2)
      {
         ErrorMessage("invalid object position expression");
         return(ERR);
      }

      CText offStr = input.End(pmPos1);

      if(sscanf(offStr.Str(),"%f",off) == 1)
      {
         *scale = 1;
         return(OK);
      }

      ErrorMessage("invalid object position expression");
      return(ERR);
   }

// Dimension only
   else if(dimPos == 0 && mdPos == -1 && pmPos1 == -1 && pmPos2 == -1)
   {
      *off = 0;
      *scale = 1;
      return(OK);
   }

// Scale factor only
   else if(dimPos != -1 && mdPos != -1 && pmPos1 == -1 && pmPos2 == -1)
   {
      if(dimPos != 0 || mdPos != 2)
      {
         ErrorMessage("invalid object position expression");
         return(ERR);
      }

      CText scaleStr = input.End(3);
      md = input[mdPos];

      if(sscanf(scaleStr.Str(),"%f",scale) == 1)
      {
         if(md == '/') *scale = 1.0/(*scale);
         *off = 0;
         return(OK);
      }

      ErrorMessage("invalid object position expression");
      return(ERR);
   }

// Offset factor only
   else if(dimPos == -1 && mdPos == -1 && pmPos1 != -1 && pmPos2 == -1)
   {
      if(sscanf(input.Str(),"%f",off) == 1)
      {
         *scale = 0;
         return(OK);
      }


      ErrorMessage("invalid object position expression");
      return(ERR);
   }

   if(sscanf(input.Str(),"%f",off) == 1)
   {
      *scale = 0;
      return(OK);
   }
   ErrorMessage("invalid object position expression");
   return(ERR);
}


/************************************************************************
    Save the expression which describes the object position and dim
    Expression must be valid.
*************************************************************************/

short SaveObjectPosition(ObjectData *obj, ObjPos *pos)
{
   obj->xSzScale  = pos->xs;
   obj->xSzOffset = pos->xo;
   obj->ySzScale  = pos->ys;
   obj->ySzOffset = pos->yo;
   obj->wSzScale  = pos->ws;
   obj->wSzOffset = pos->wo;
   obj->hSzScale  = pos->hs;
   obj->hSzOffset = pos->ho;
   obj->regionSized = pos->region;

   return(OK);
}



// Write to the current status window
short UpdateStatusWindow(HWND hWnd, int pos, char *text)
{
   HWND parent = GetParent(hWnd);
   WinData *win = GetWinDataClass(parent);
   if(!win || win->inRemoval) return(ERR); 
//   if(TryEnterCriticalSection(&criticalSection))  
//   {

      if(win->statusbox)
         SendMessage(win->statusbox->hWnd,SB_SETTEXT, (WPARAM)pos, (LPARAM)text);  
  //    LeaveCriticalSection(&criticalSection);
  // }
   return(OK);
}

/*******************************************************************************
* Modify the parameters for PlotWindow control
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter                value
*  "menu1"
*
*******************************************************************************/

int SetPlotWinParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   if(parameter == "tracemenu" || parameter == "bkgmenu" ||
      parameter == "axesmenu"  || parameter == "titlemenu" ||
      parameter == "labelmenu")
   {
	   WinData *win = rootWin->FindWinByHWND(obj->hwndParent);

      if(value->GetType() == FLOAT32)
      {
         short objNr = value->GetReal();
         ObjectData *menuObj = win->widgets.findByNr(objNr);
         if(!menuObj)
         {
            ErrorMessage("Object %hd not found",objNr);
            return(ERR);
         }
         if(menuObj->type != MENU)
         {
            ErrorMessage("Object %hd is not a menu",objNr);
            return(ERR);
         }
         MenuInfo* info = (MenuInfo*)menuObj->data;
         PlotWindow *pp = (PlotWindow*)obj->data;
         if(parameter == "tracemenu")
         {
            pp->traceMenu = info->menu;
            pp->traceMenuNr = objNr;
         }
         else if(parameter == "bkgmenu")
         {
            pp->bkgMenu = info->menu;
            pp->bkgMenuNr = objNr;
         }
         else if(parameter == "labelmenu")
         {
            pp->labelMenu = info->menu;
            pp->labelMenuNr = objNr;
         }
         else if(parameter == "titlemenu")
         {
            pp->titleMenu = info->menu;
            pp->titleMenuNr = objNr;
         }
         else if(parameter == "axesmenu")
         {
            pp->axesMenu = info->menu;
            pp->axesMenuNr = objNr;
         }
      // Get the exisiting accelerator table for the plot object
         ACCEL *accelOld = NULL,*accelNew = NULL,*accelMenu = NULL;
         int j,k,nMenu = 0,nOld = 0;
      // Extract old table
         if(obj->accelTable)
         {
            nOld = CopyAcceleratorTable(obj->accelTable,NULL,0);
            accelOld = new ACCEL[nOld];
            CopyAcceleratorTable(obj->accelTable,(LPACCEL)accelOld,nOld);
         }
      // Extract menu table
         accelMenu = new ACCEL[info->nrItems];
         for(j = 0; j < info->nrItems; j++) // Loop over items in each menu
         {
            if(info->accel[j].key != 0)
               accelMenu[nMenu++] = info->accel[j];
         }
      // Copy old table
         accelNew = new ACCEL[nOld+nMenu];
         k = 0;
			if (accelOld)
			{
				for(k = 0, j = 0; j < nOld; j++)
            accelNew[k++] = accelOld[j];
			}
      // Add menu entries
         for(j = 0; j < nMenu; j++)
            accelNew[k++] = accelMenu[j];
         if(obj->accelTable)
            DestroyAcceleratorTable(obj->accelTable);
      // Update the accelerator table
         obj->accelTable = CreateAcceleratorTable((LPACCEL)accelNew,k);
         if(accelNew)
            delete [] accelNew;
         if(accelOld)
            delete [] accelOld;
         if(accelMenu)
            delete [] accelMenu;
      }
      else
      {
         ErrorMessage("invalid parameter type");
         return(ERR);
      }
	}
	else if (parameter == "annotationcallback" || parameter == "imageinsetcallback")
	{
	   WinData *win = rootWin->FindWinByHWND(obj->hwndParent);
      if(value->GetType() == QUOTED_STRING || value->GetType() == UNQUOTED_STRING)
      {
         PlotWindow *pp = (PlotWindow*)obj->data;
			if (parameter == "annotationcallback")
			{
				pp->annotationCallback = value->GetString();
			}
			else if (parameter == "imageinsetcallback")
			{
				pp->imageInsetCallback = value->GetString();
			}
		}
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

// Make sure the object is redrawn
 //  MyInvalidateRect(obj->hWnd,NULL,false);


   return(OK);
}


int SetImageWinParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   if(parameter == "bkgmenu" || parameter == "axesmenu"  ||
      parameter == "titlemenu" || parameter == "labelmenu")
   {

	   WinData *win = rootWin->FindWinByHWND(obj->hwndParent);

      if(value->GetType() == FLOAT32)
      {
         short objNr = value->GetReal();
         ObjectData *menuObj = win->widgets.findByNr(objNr);
         if(!menuObj)
         {
            ErrorMessage("Object %hd not found",objNr);
            return(ERR);
         }
         if(menuObj->type != MENU)
         {
            ErrorMessage("Object %hd is not a menu",objNr);
            return(ERR);
         }
         MenuInfo* info = (MenuInfo*)menuObj->data;
         PlotWindow *pp = (PlotWindow*)obj->data;
         if(parameter == "bkgmenu")
         {
            pp->bkgMenu = info->menu;
            pp->bkgMenuNr = objNr;
         }
         else if(parameter == "labelmenu")
         {
            pp->labelMenu = info->menu;
            pp->labelMenuNr = objNr;
         }
         else if(parameter == "titlemenu")
         {
            pp->titleMenu = info->menu;
            pp->titleMenuNr = objNr;
         }
         else if(parameter == "axesmenu")
         {
            pp->axesMenu = info->menu;
            pp->axesMenuNr = objNr;
         }

      // Get the exisiting accelerator table for the plot object
         ACCEL *accelOld = NULL,*accelNew = NULL,*accelMenu = NULL;
         int j,k,nMenu = 0,nOld = 0;
      // Extract old table
         if(obj->accelTable)
         {
            nOld = CopyAcceleratorTable(obj->accelTable,NULL,0);
            accelOld = new ACCEL[nOld];
            CopyAcceleratorTable(obj->accelTable,(LPACCEL)accelOld,nOld);
         }
      // Extract menu table
         accelMenu = new ACCEL[info->nrItems];
         for(j = 0; j < info->nrItems; j++) // Loop over items in each menu
         {
            if(info->accel[j].key != 0)
               accelMenu[nMenu++] = info->accel[j];
         }
      // Copy old table
         accelNew = new ACCEL[nOld+nMenu];
         k = 0;
			if (accelOld)
			{
				for(k = 0, j = 0; j < nOld; j++)
					accelNew[k++] = accelOld[j];
			}
      // Add menu entries
         for(j = 0; j < nMenu; j++)
            accelNew[k++] = accelMenu[j];
         if(obj->accelTable)
            DestroyAcceleratorTable(obj->accelTable);
      // Update the accelerator table
         obj->accelTable = CreateAcceleratorTable((LPACCEL)accelNew,k);
         if(accelNew)
            delete [] accelNew;
         if(accelOld)
            delete [] accelOld;
         if(accelMenu)
            delete [] accelMenu;
      }
      else
      {
         ErrorMessage("invalid parameter type");
         return(ERR);
      }
	}

   //else if(parameter == "titleupdate")
   //{
   //   if(value->GetType() == UNQUOTED_STRING)
   //   {
   //      long type = GetWindowLong(obj->hWnd,GWL_STYLE);
   //      CText mode = value->GetString();
   //      PlotParent *pp = (PlotParent*)obj->data;

   //      if(mode == "true" || mode == "on")
   //      {   
   //         pp->titleUpdate = true;
   //      }  
   //      else if(mode == "false" || mode == "off")
   //      {   
   //         pp->titleUpdate = false;
   //      }
   //   }
   //}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

// Make sure the object is redrawn
   MyInvalidateRect(obj->hWnd,NULL,false);


   return(OK);
}

// Modify the current toolbar, statusbox or menu when the user clicks on 
// certain objects (e.g. plots or editors

void SelectFixedControls(WinData *win, ObjectData *src)
{	
 //  if(win->statusbox)
//	ShowWindow(win->blankStatusBar,SW_HIDE);
 //  if(win->toolbar)
//	ShowWindow(win->blankToolBar,SW_HIDE);

	for(ObjectData* obj: win->widgets.getWidgets())
   {
      if(obj->type == TOOLBAR && src->toolbar != -1)
      {
         if(obj->nr() != src->toolbar)
         {
            obj->visible = false;
            obj->Show(false);
         }
         else
         {
            obj->visible = true;
            obj->Show(true);
            win->toolbar = obj;
            ShowWindow(win->blankToolBar,false);
         }
      }
      if(obj->type == TOOLBAR && src->toolbar == -1)
      {
         obj->visible = false;
         obj->Show(false);
         ShowWindow(win->blankToolBar,true);
      }
      if(obj->type == STATUSBOX && src->statusbox != -1)
      {
         if(obj->nr() != src->statusbox)
         {
            obj->visible = false;
            obj->Show(false);
         }
         else
         {
            obj->visible = true;
            obj->Show(true);
            win->statusbox = obj;
            ShowWindow(win->blankStatusBar,false);
            StatusBoxInfo* info = (StatusBoxInfo*)obj->data;
            if(info)
            {
               if(src->type == CLIWINDOW || src->type == TEXTEDITOR)
                   ShowWindow(info->subWindow,true);
               else
                   ShowWindow(info->subWindow,false);
            }
         }
      }

      //if(obj->type == STATUSBOX && src->statusbox == -1)
      //{
      //   obj->visible = false;
      //   obj->Show(false);
      //   ShowWindow(win->blankStatusBar,true);
      //}
   }

   CText txt;
   Interface itfc;

   txt.Format("%hd,\"menu\",%hd",win->nr,src->nr());
   SetWindowParameter(&itfc,txt.Str());
}

void SelectFixedControls(WinData *win)
{
   if(!win)
      return;

// Hide all toobars and status boxes
	for(ObjectData* obj: win->widgets.getWidgets())
   {
      if(obj->type == TOOLBAR)
      {
         obj->visible = false;
         obj->Show(false);
      }
      if(obj->type == STATUSBOX)
      {
         obj->visible = false;
         obj->Show(false);
      }
   }

// If window 
   if(win->statusbox && !(win->defaultStatusbox))
      ShowWindow(win->blankStatusBar,true);
   else if(win->statusbox && win->defaultStatusbox)
      ShowWindow(win->defaultStatusbox->hWnd,true);

   if(win->toolbar && !(win->defaultToolbar))
      ShowWindow(win->blankToolBar,true);
   else if(win->toolbar && win->defaultToolbar)
      ShowWindow(win->defaultToolbar->hWnd,true);

   CText txt;
   Interface itfc;

   txt.Format("%hd,\"menu\",%hd",win->nr,0);
   SetWindowParameter(&itfc,txt.Str());
}


/*******************************************************************************
* Extract the parameters for an updown box
*
* Command syntax is value = getpar(win,obj,parameter)
*
* parameter  
*  "text" ... the text for the currently selected state
*  "value" .. the check state: 0-unchecked, 1-checked, 2-greyed
*  "init" ... return the initial state for the check box
*
*******************************************************************************/

int GetUpDownParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   UpDownInfo *info;
  
   info = (UpDownInfo*)obj->data;

   if(parameter == "value")
   {
      ans->MakeAndSetFloat((float)info->value);
   }
   else if(parameter == "base")
   {
      ans->MakeAndSetFloat((float)info->base);
   }
   else if(parameter == "stepsize")
   {
      ans->MakeAndSetFloat((float)info->stepSize);
   }
   else if(parameter == "nrsteps")
   {
      ans->MakeAndSetFloat((float)info->nrSteps);
   }
   else if(parameter == "orientation")
   {
		long type = GetWindowLong(obj->hWnd,GWL_STYLE);
		if(type & UDS_HORZ) 
		   ans->MakeAndSetString("horizontal");
      else
		   ans->MakeAndSetString("vertical");
	}

	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
 	return(OK);
}


int GetDividerParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   DividerInfo *info;
  
   info = (DividerInfo*)obj->data;

	if(info)
	{
		if(parameter == "orientation")
		{
			if(info->orientation == WindowLayout::HORIZONTAL)
				ans->MakeAndSetString("horizontal");
			else
				ans->MakeAndSetString("vertical");
		}
      else if(parameter == "limits")
      {
         float **mat = MakeMatrix2D(2,1);
			mat[0][0] = info->minPos.xo;
			mat[0][1] = info->maxPos.xo;
         ans->AssignMatrix2D(mat,2,1);
      }
		else
		{
			ErrorMessage("invalid parameter requested");
			return(ERR);
		}
	}
	else
	{
		ErrorMessage("no divider information");
		return(ERR);
	}
 	return(OK);
}


int GetMenuParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   MenuInfo *info = (MenuInfo*)obj->data;

   if(parameter == "menu")
   {
      int nr = info->nrItems;
      char **lst = NULL;
      for(int k = 0; k < nr; k++)
      {
         AppendStringToList(info->label[k],&lst,k);
      }
      ans->AssignList(lst,nr);
   }
   else if(parameter == "nritems")
   {
      ans->MakeAndSetFloat(info->nrItems);
   }
   else if(parameter == "menuname")
   {
      ans->MakeAndSetString(info->name);
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
 	return(OK);
}

/*****************************************************************************
  Get tab control parameters

  currenttab ......... returns the current-tab zero based index
  index .............. returns the current-tab one based index
  zindex ............. returns the current-tab zero based index
  tablist ............ returns tab names as a list

******************************************************************************/


int GetTabParameter(ObjectData *obj, CText &parameter, Variable *ans)
{
   if(parameter == "currenttab" || parameter == "zindex")
   {
      int selected = TabCtrl_GetCurSel(obj->hWnd);
      ans->MakeAndSetFloat(selected);
   }
   else if(parameter == "color" || parameter == "fgcolor" )
   {
	   BYTE *data = (BYTE*)&obj->fgColor;
      float colors[3];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      ans->MakeMatrix2DFromVector(colors,3,1);
   }
   else if(parameter == "tabname")
   {
      char name[MAX_STR];
      TCITEM tci;
       int selected = TabCtrl_GetCurSel(obj->hWnd);

       tci.mask = TCIF_TEXT;
       tci.iImage = -1;
		 tci.cchTextMax = MAX_STR;
		 tci.pszText = name;
       TabCtrl_GetItem(obj->hWnd,selected,&tci);
       ans->MakeAndSetString(name);
   }
   else if(parameter == "index")
   {
      int selected = TabCtrl_GetCurSel(obj->hWnd);
      ans->MakeAndSetFloat(selected+1);
   }
   else if(parameter == "tablist") // Get tab names as a list
   { 
      char name[MAX_STR];
      TCITEM tci;

      int cnt =  TabCtrl_GetItemCount(obj->hWnd);

		char **lst = NULL;

      for(short i = 0; i < cnt; i++) // Add new one from list
      {
          tci.mask = TCIF_TEXT;
          tci.iImage = -1;
			 tci.cchTextMax = MAX_STR;
			 tci.pszText = name;
          TabCtrl_GetItem(obj->hWnd,i,&tci);
          AppendStringToList(name,&lst,i);
      } 
     
		ans->AssignList(lst,cnt);
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
 	return(OK);
}

/*****************************************************************************
  Set tab control parameters

  inittabs/tablist ... initialise the tab control with a list of tab names
  currenttab ......... specify the current tab by zero based index or string
  currentpage ........ specify the current tab and page by index or string
  renametabs ......... update the tab name list

******************************************************************************/


int SetTabParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   if(parameter == "inittabs" || parameter == "tablist") // Add new tabs to tab control
   { 
      if(value->GetType() == LIST)
      {
         char name[MAX_STR];
         TCITEM tci;
         tci.mask = TCIF_TEXT;
         tci.iImage = -1;
         tci.pszText = name;
         tci.cchTextMax = MAX_STR;

         TabInfo *info = (TabInfo*)obj->data;
         delete [] info->tabLabels;
         info->nrTabs = value->GetDimX();
         info->tabLabels = new CText[value->GetDimX()];

         TabCtrl_DeleteAllItems(obj->hWnd);
          for(short i = 0; i < value->GetDimX(); i++) // Add new one from list
          {
             strncpy_s(name,MAX_STR,value->GetList()[i],_TRUNCATE);
             TabCtrl_InsertItem(obj->hWnd, i, &tci);
             info->tabLabels[i] = name;
          } 
      }
      else if(value->GetType() == UNQUOTED_STRING)
      {
         char name[MAX_STR];
         char* txt = value->GetString();
         TCITEM tci;
         tci.mask = TCIF_TEXT;
         tci.iImage = -1;
         tci.pszText = name;
         tci.cchTextMax = MAX_STR;

         TabInfo *info = (TabInfo*)obj->data;
         delete [] info->tabLabels;
         CArg arg(',');
         
         info->nrTabs = arg.Count(txt);
         info->tabLabels = new CText[info->nrTabs];

         TabCtrl_DeleteAllItems(obj->hWnd);

         for(short i = 0; i < info->nrTabs; i++) // Add new one from list
         {
             strncpy_s(name,MAX_STR,arg.Extract(i+1),_TRUNCATE);
             info->tabLabels[i] = name;
             TabCtrl_InsertItem(obj->hWnd, i, &tci);
         }        
      }
      else
	   {
	      ErrorMessage("invalid value type");
	      return(ERR);
	   }
   }
   else if(parameter == "color" || parameter == "fgcolor" )
   {
      if(SetColor(obj->fgColor, value) == ERR)
         return(ERR);
      MyInvalidateRect(obj->hWnd,NULL,true);
   }
   else if(parameter == "zindex") // Set current tab number (zero based)
   {
      if(value->GetType() == FLOAT32)	
      {
        int cnt =  TabCtrl_GetItemCount(obj->hWnd);
        int sel = nint(value->GetReal());
        if(sel < 0 || sel > cnt)
        {
           ErrorMessage("invalid tab number");
           return(ERR);
        }
        TabCtrl_SetCurSel(obj->hWnd,sel);
      }
      else
	   {
	      ErrorMessage("invalid value type");
	      return(ERR);
	   }
   }
   else if(parameter == "currenttab") // Set current tab number (zero based or string)
   {
      if(value->GetType() == FLOAT32)	
      {
        int cnt =  TabCtrl_GetItemCount(obj->hWnd);
        int sel = nint(value->GetReal());
        if(sel < 0 || sel > cnt)
        {
           ErrorMessage("invalid tab number");
           return(ERR);
        }
        TabCtrl_SetCurSel(obj->hWnd,sel);
      }
      else if(value->GetType() == UNQUOTED_STRING) // A tab name has been entered
      {
         int i;
         char *name = value->GetString();
         TabInfo *info = (TabInfo*)obj->data;

         for(i = 0; i < info->nrTabs; i++) // find the match
         {
            if(info->tabLabels[i] == name)
            {
               TabCtrl_SetCurSel(obj->hWnd,i);
               break;
            }
         }    
         if(i == info->nrTabs)
	      {
	         ErrorMessage("invalid tab name '%s'",name);
	         return(ERR);
	      }
      }
      else
	   {
	      ErrorMessage("invalid value type");
	      return(ERR);
	   }
   }
   else if(parameter == "index") // Set current tab number (one based)
   {
      if(value->GetType() == FLOAT32)	
      {
        int cnt =  TabCtrl_GetItemCount(obj->hWnd);
        int sel = nint(value->GetReal())-1;
        if(sel < 0 || sel > cnt)
        {
           ErrorMessage("invalid tab number");
           return(ERR);
        }
        TabCtrl_SetCurSel(obj->hWnd,sel);
      }
      else
	   {
	      ErrorMessage("invalid value type");
	      return(ERR);
	   }
   }
   else if(parameter == "currentpage") // Select current tab page with all the relevant controls
   {
      if(value->GetType() == FLOAT32) // A zero based index has been entered	
      {
        int cnt =  TabCtrl_GetItemCount(obj->hWnd);
        int sel = nint(value->GetReal());
        if(sel < 0 || sel > cnt)
        {
           ErrorMessage("invalid tab number");
           return(ERR);
        }
        TabCtrl_SetCurSel(obj->hWnd,sel);
        ControlVisibleWithTabs(obj->winParent,obj);
		  SetWindowPos(obj->hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); // Make sure tabs are at the bottom and therefore visible
      }
      else if(value->GetType() == UNQUOTED_STRING) // A tab name has been entered
      {
         int i;
         char *name = value->GetString();
         TabInfo *info = (TabInfo*)obj->data;

         for(i = 0; i < info->nrTabs; i++) // find the match
         {
            if(info->tabLabels[i] == name)
            {
               TabCtrl_SetCurSel(obj->hWnd,i);
               ControlVisibleWithTabs(obj->winParent,obj);
               SetWindowPos(obj->hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); // Make sure tabs are at the bottom and therefore visible
               break;
            }
         }    
         if(i == info->nrTabs)
	      {
	         ErrorMessage("invalid tab name '%s'",name);
	         return(ERR);
	      }
      }
      else
	   {
	      ErrorMessage("invalid value type");
	      return(ERR);
	   }
   }
   else if(parameter == "renametabs") // Set the tab names
   { 
      if(value->GetType() == LIST)	// Apply the contents of a text list to a menu
      {
         char name[MAX_STR];
         TCITEM tci;

         int cnt =  TabCtrl_GetItemCount(obj->hWnd);
         if(cnt == value->GetDimX())
         {
            TabInfo *info = (TabInfo*)obj->data;
            for(short i = 0; i < value->GetDimX(); i++) // Add new one from list
            {
                tci.mask = TCIF_TEXT;
                tci.iImage = -1;
					 tci.cchTextMax = MAX_STR;
					 tci.pszText = name;
                TabCtrl_GetItem(obj->hWnd,i,&tci);

                strncpy_s(name,MAX_STR,value->GetList()[i],_TRUNCATE);
					 tci.pszText = name;
                TabCtrl_SetItem(obj->hWnd,i,&tci);
                info->tabLabels[i] = name;
            } 
         }
         else
         {
            ErrorMessage("list length doesn't match number of tabs");
            return(ERR);
         }
      }
      else
	   {
	      ErrorMessage("invalid value type");
	      return(ERR);
	   }
   }  
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
 	return(OK);
}

int SetUpDownParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   UpDownInfo *info;
  
   info = (UpDownInfo*)obj->data;


   if(parameter == "value")
   {
      if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
      info->value = value->GetReal();
   }
   else if(parameter == "base")
   {
      if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
      info->base = value->GetReal();
   }
   else if(parameter == "nrsteps")
   {
      if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
      info->nrSteps = nint(value->GetReal());
   }
   else if(parameter == "stepsize")
   {
      if(value->GetType() != FLOAT32)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }
      info->stepSize = value->GetReal();
   }
   else if(parameter == "orientation") // set orientation
   {
      if(value->GetType() != UNQUOTED_STRING)
      {
         ErrorMessage("Invalid data type");
         return(ERR);
      }

		if(!strncmp(value->GetString(),"horizontal",5))
      {
         ReorientUpDownControl(obj,"horizontal");
         return(OK);
      }	
      else if(!strncmp(value->GetString(),"vertical",4))
      {
         ReorientUpDownControl(obj,"vertical");
         return(OK);
      }
      else
      {
         ErrorMessage("invalid orientation");
         return(ERR);      
      } 
	}

	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

 	return(OK);
}

int SetPictureParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   HBITMAP bmp;
   PictureInfo *pixInfo;

   if(obj->data)
      pixInfo = (PictureInfo*)obj->data;
  
   if(parameter == "file") // Replace all blue pixels with background
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char* fileName = value->GetString();
         wchar_t* fileNameWide = CharToWChar(fileName);

          Bitmap bp(fileNameWide);

          SysFreeString(fileNameWide);

          int w = bp.GetWidth();
          int h = bp.GetHeight();
          if(w > 0 && h > 0)
          {
             if(!pixInfo->resizePixToFitFrame)
             {
                obj->wo = w;        
                obj->wSzScale = 0;
                obj->wSzOffset = obj->wo;
                obj->ho = h;        
                obj->hSzScale = 0;
                obj->hSzOffset = obj->ho;
                obj->Move(obj->xo,obj->yo,obj->wo,obj->ho);
             }
             bp.GetHBITMAP( 0, &bmp );

             if(bmp)
             {
                if(pixInfo->useBlueScreen)
                {
                  ReplaceBackGroundPixels(bmp);
                }
                if(pixInfo->bmp)
                  DeleteObject(pixInfo->bmp);
                pixInfo->bmp = bmp;
             }
             else
             {
                ErrorMessage("can't find bitmap '%s'",fileName);
                return(ERR);
             }
          }
          else
          {
             ErrorMessage("can't find bitmap '%s'",fileName);
             return(ERR);
          }
          MyInvalidateRect(obj->hWnd,NULL,0);
      }
   }
   else if(parameter == "usebluescreen") // Do not replace background pixels
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *val = value->GetString();
         if(!strcmp(val,"true"))
            pixInfo->useBlueScreen = true;
         else if(!strcmp(val,"false"))

            pixInfo->useBlueScreen = false;
         else
         {
            ErrorMessage("invalid parameter value (true/false)");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid parameter value type - should be a string");
         return(ERR);
      }
   }
   else if(parameter == "resizetoframe") // Resize picture to fit the frame
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *val = value->GetString();
         if(!strcmp(val,"true"))
            pixInfo->resizePixToFitFrame = true;
         else if(!strcmp(val,"false"))

            pixInfo->resizePixToFitFrame = false;
         else
         {
            ErrorMessage("invalid parameter value (true/false)");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid parameter value type - should be a string");
         return(ERR);
      }
   }
   else if(parameter == "showframe")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char* txt = value->GetString();
         if(!strcmp(txt,"true") || !strcmp(txt,"yes"))
            obj->showFrame = true;
         else if(!strcmp(txt,"false") || !strcmp(txt,"no"))
            obj->showFrame = false;
         else
         {
            ErrorMessage("invalid value");
            return(ERR);
         }
      }
   }
   else if(parameter == "clearpicture")
   {
      if(pixInfo->bmp)
         DeleteObject(pixInfo->bmp);
      pixInfo->bmp = NULL;
      MyInvalidateRect(obj->hWnd,NULL,1);
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

   return(OK);
}





int SetMenuParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   MenuInfo *info = (MenuInfo*)obj->data;

   if(parameter == "checkitem")
   {
      if(value->GetType() == FLOAT32)
		{
			int item = nint(value->GetReal());
			if(item < 0 || item > info->nrItems)
			{
				ErrorMessage("invalid menu item number");
				return(ERR);
			}
	      CheckMenuItem(info->menu,item, MF_BYPOSITION | MF_CHECKED);
		}
		else if(value->GetType() == UNQUOTED_STRING)
		{
		   char *menuKey = value->GetString();
         obj->setMenuItemCheck(menuKey,true);	
		}
		else
		{
			ErrorMessage("Invalid data type for checkitem, should be the menu item position or a a key string");
		   return(ERR);
		}

	}
   else if(parameter == "uncheckitem")
   {
      if(value->GetType() == FLOAT32)
		{
			int item = nint(value->GetReal());
			if(item < 0 || item > info->nrItems)
			{
				ErrorMessage("invalid menu item number");
				return(ERR);
			}
	      CheckMenuItem(info->menu,item, MF_BYPOSITION | MF_UNCHECKED);
		}
		else if(value->GetType() == UNQUOTED_STRING)
		{
		   char *menuKey = value->GetString();
         obj->setMenuItemCheck(menuKey,false);	
		}
		else
		{
			ErrorMessage("Invalid data type for checkitem, should be the menu item position or a a key string");
		   return(ERR);
		}
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
 	return(OK);
}



int SetToolBarParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   ToolBarInfo *info = (ToolBarInfo*)obj->data;

   if(parameter == "checkitem")
   {
      if(value->GetType() == FLOAT32)
		{
			int item = nint(value->GetReal());
			if(item < 0 || item > info->nrItems)
			{
				ErrorMessage("invalid toolbar button number");
				return(ERR);
			}
	      SendMessage(obj->hWnd,TB_CHECKBUTTON,(WPARAM)item,(LPARAM) MAKELONG(true, 0));
		}
	}
   else if(parameter == "uncheckitem")
   {
      if(value->GetType() == FLOAT32)
		{
			int item = nint(value->GetReal());
			if(item < 0 || item > info->nrItems)
			{
				ErrorMessage("invalid toolbar button number");
				return(ERR);
			}
	      SendMessage(obj->hWnd,TB_CHECKBUTTON,(WPARAM)item,(LPARAM) MAKELONG(false, 0));
		}
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}
	
 	return(OK);
}

// 
int SetDebugStripParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   DebugStripInfo *info;
  
   info = (DebugStripInfo*)obj->data;

   if(parameter == "editor")
   {
      info->editor = nint(value->GetReal());
   }
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

   return(OK);
}

int SetOpenGLParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   CPlot3D *info;
  
   info = (CPlot3D*)obj->data;

   if(parameter == "initialise")
   {
      info->initialised = true;
      Initialize3DPlot(obj->hWnd);
      MyInvalidateRect(obj->hwndParent,NULL,true);
   }
   //else if(parameter == "titleupdate")
   //{
   //   if(value->GetType() == UNQUOTED_STRING)
   //   {
   //      long type = GetWindowLong(obj->hWnd,GWL_STYLE);
   //      CText mode = value->GetString();

   //      if(mode == "true" || mode == "on")
   //      {   
   //         info->titleUpdate = true;
   //      }  
   //      else if(mode == "false" || mode == "off")
   //      {   
   //         info->titleUpdate = false;
   //      }
   //   }
   //}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

   return(OK);
}


//
int SetTextEditorParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   if(parameter == "debug")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"yes") || !strcmp(response,"true"))
         {
             SendMessage(obj->hWnd,EM_SETMARGINS ,EC_LEFTMARGIN	,(LPARAM) MAKELONG(30, 0)); // Find first visible line
             curEditor->debug = true;
         }
         else
         {
             SendMessage(obj->hWnd,EM_SETMARGINS ,EC_LEFTMARGIN	,(LPARAM) MAKELONG(5, 0)); // Find first visible line
             curEditor->debug = false;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
   else if(parameter == "text")
   {
      switch(value->GetType())
      {
         case(UNQUOTED_STRING):
         {
            char *str = new char[strlen(value->GetString())+1];
            strcpy(str,value->GetString());
            EditParent *ep = (EditParent*)obj->data;
            EditRegion *er = ep->editData[ep->curRegion];
            er->CopyTextToEditor(str);
            delete [] str;
            SendMessage(er->edWin,WM_HSCROLL,SB_LEFT,NULL);  // Scroll line to left of window	      	   	   
            break;
         }
         default:
            ErrorMessage("invalid parameter value, should be a string");
	         return(ERR);
      }
   }
	else if(parameter == "insertoffset")
	{
      switch(value->GetType())
      {
			case(FLOAT32):
         {
				long startSel,endSel;
				int offset = nint(value->GetReal());
            EditParent *ep = (EditParent*)obj->data;
            EditRegion *er = ep->editData[ep->curRegion];
	         SendMessage(er->edWin,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);
	         SendMessage(er->edWin,EM_SETSEL,(WPARAM)endSel+offset, (LPARAM)endSel+offset);
				break;
			}
         default:
            ErrorMessage("invalid parameter value, should be an integer");
	         return(ERR);
		}
	}
   else if(parameter == "insert" || parameter == "inserttext")
   {
      switch(value->GetType())
      {
         case(UNQUOTED_STRING):
         {
            extern void SetEditTitle(void);
            char *str = new char[strlen(value->GetString())+1];
            strcpy(str,value->GetString());
            int sz = strlen(str);
            EditParent *ep = (EditParent*)obj->data;
            EditRegion *er = ep->editData[ep->curRegion];
            er->CopySelectionToUndo(REPLACE_TEXT,strlen(str)); 
            long startSel,endSel;
	         SendMessage(er->edWin,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);
         	SendMessage(er->edWin,EM_REPLACESEL,(WPARAM)false,(LPARAM)str);
			   long firstLine = SendMessage(er->edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);	
			   long lastLine = SendMessage(er->edWin,EM_LINEFROMCHAR,(WPARAM)startSel+sz,(LPARAM)0);			   
			   er->ContextColorLines(firstLine,lastLine);
            if(!er->edModified)
            {
               er->edModified = true;
               //SendMessageToGUI("Editor,Modified",-1); 
               if(er == curEditor)
                  SetEditTitle(); 
            }
				//SetFocus(er->edWin);
            delete [] str;
            break;
         }
         default:
            ErrorMessage("invalid parameter value, should be a string");
	         return(ERR);
      }
   }
   else if(parameter == "filename") // Set the filename
   {
      switch(value->GetType())
      {
         case(UNQUOTED_STRING):
         {
            EditParent *ep = (EditParent*)obj->data;
            strncpy(ep->editData[ep->curRegion]->edName,value->GetString(),MAX_PATH);
            break;
         }
         default:
            ErrorMessage("invalid parameter value, should be a string");
	         return(ERR);
      }
	}
   else if(parameter == "pathname") // Set the pathname
   {
      switch(value->GetType())
      {
         case(UNQUOTED_STRING):
         {
            EditParent *ep = (EditParent*)obj->data;
            strncpy(ep->editData[ep->curRegion]->edPath,value->GetString(),MAX_PATH);
            break;
         }
         default:
            ErrorMessage("invalid parameter value, should be a string");
	         return(ERR);
      }
	}	
   else if (parameter == "scrolltoline")
   {
      int lineNr = nint(value->GetReal());
      EditParent* ep = (EditParent*)obj->data;
      EditRegion* er = ep->editData[ep->curRegion];
      er->ScrollToEditorLine(lineNr);
   }
   else if (parameter == "setcursorpos")
   {
      int cursPos = nint(value->GetReal());
      EditParent* ep = (EditParent*)obj->data;
      EditRegion* er = ep->editData[ep->curRegion];
      SelectChar(er->edWin, cursPos);
   }
   else if(parameter == "showcontextualmenu")
   {
      EditParent* ep = (EditParent*)obj->data;

      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"no") || !strcmp(response,"false"))
         {
            ep->showContextualMenu = false;
         }
         else
         {
            ep->showContextualMenu = true;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
	else if(parameter == "showsyntax")
	{
		long startSel,endSel;
		int offset = nint(value->GetReal());
		EditParent *ep = (EditParent*)obj->data;
		EditRegion *er = ep->editData[ep->curRegion];
		SendMessage(er->edWin,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);
	   UpDateEditSyntax(obj->winParent, curEditor, startSel,endSel,true);
	}
   else if(parameter == "showsyntaxcoloring")
   {
      EditParent* ep = (EditParent*)obj->data;

      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"no") || !strcmp(response,"false"))
         {
            ep->showSyntaxColoring = false;
         }
         else
         {
            ep->showSyntaxColoring = true;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
   else if(parameter == "syntaxcoloringstyle")
   {
      EditParent* ep = (EditParent*)obj->data;
      EditRegion *er = ep->editData[ep->curRegion];

      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"asm"))
            er->syntaxColoringStyle = ASM_STYLE;
         else if(!strcmp(response,"macro"))
            er->syntaxColoringStyle = MACRO_STYLE;
         else if (!strcmp(response, "par"))
            er->syntaxColoringStyle = PAR_STYLE;
         else
            er->syntaxColoringStyle = NO_STYLE;
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
   else if(parameter == "showsyntaxdescription")
   {
      EditParent* ep = (EditParent*)obj->data;

      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"no") || !strcmp(response,"false"))
         {
            ep->showSyntaxDescription = false;
         }
         else
         {
            ep->showSyntaxDescription = true;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
   else if(parameter == "current") // Make this editor the current one.
   {
      extern void SetEditTitle();
      EditParent* ep = (EditParent*)obj->data;
      EditRegion *er = ep->editData[ep->curRegion];
	   curEditor = er;  // Make this the current editor
		obj->winParent->currentEditor = er; // Also for the window
      ep->curRegion = er->regNr;
	   SetFocus(curEditor->edWin);         // Give it window focus		   
	   SetEditTitle();                     // Set the window title
   }
   else if(parameter == "modified") // Whether the text has been modified or not
   {
      EditParent* ep = (EditParent*)obj->data;
      EditRegion *er = ep->editData[ep->curRegion];
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"no") || !strcmp(response,"false"))
         {
            er->edModified = false;
         }
         else
         {
            er->edModified = true;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
	}
   else if(parameter == "readonlytext") // Whether the text is read only
   {
      EditParent* ep = (EditParent*)obj->data;
      EditRegion *er = ep->editData[ep->curRegion];
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"no") || !strcmp(response,"false"))
         {
            er->readOnly = false;
         }
         else
         {
            er->readOnly = true;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
	}
   else if(parameter == "wordwrap")
   {
      extern void MakeEditWindows(bool save, EditParent *ep, short nry, short nrx, short saveThisOne);
      extern void ResizeEditSubwindows(EditParent *ep);

      EditParent* ep = (EditParent*)obj->data;

      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"no") || !strcmp(response,"false"))
         {
            ep->wordWrap = false;
         }
         else
         {
            ep->wordWrap = true;
         }
          MakeEditWindows(true,ep,ep->rows,ep->cols,-1);	
          ResizeEditSubwindows(ep);
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
   else if(parameter == "labelobj")
	{
		EditParent* ep = (EditParent*)obj->data;
      EditRegion *er = ep->editData[ep->curRegion];
		er->labelCtrlNr = value->GetReal();
		return(OK);
	}

	
   //else if(parameter == "mode")
   //{
   //   if(value->GetType() == UNQUOTED_STRING)
   //   {
   //      long type = GetWindowLong(obj->hWnd,GWL_STYLE);
   //      CText mode = value->GetString();

   //      if(mode == "active")
   //      {   
   //         obj->active = true;
   //      }  
   //      else if(mode == "normal")
   //      {   
   //         obj->active = false;
   //      }
   //   }
   //}
   //else if(parameter == "titleupdate")
   //{
   //   if(value->GetType() == UNQUOTED_STRING)
   //   {
   //      long type = GetWindowLong(obj->hWnd,GWL_STYLE);
   //      CText mode = value->GetString();

   //      if(mode == "true" || mode == "on")
   //      {   
   //         info->titleUpdate = true;
   //      }  
   //      else if(mode == "false" || mode == "off")
   //      {   
   //         info->titleUpdate = false;
   //      }
   //   }
   //}

	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

   return(OK);
}



int SetCLIParameter(ObjectData *obj, CText &parameter, Variable *value)
{     
   if(parameter == "debug")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"yes") || !strcmp(response,"true"))
         {
             obj->debug = true;
         }
         else
         {
             obj->debug = false;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
	else if(parameter == "insertoffset")
	{
      switch(value->GetType())
      {
			case(FLOAT32):
         {
				long startSel,endSel;
				int offset = nint(value->GetReal());
		      HWND hwnd = obj->hWnd;
	         SendMessage(hwnd,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);
	         SendMessage(hwnd,EM_SETSEL,(WPARAM)endSel+offset, (LPARAM)endSel+offset);
				break;
			}
         default:
            ErrorMessage("invalid parameter value, should be an integer");
	         return(ERR);
		}
	}
   else if(parameter == "insert" || parameter == "inserttext")
   {
      switch(value->GetType())
      {
         case(UNQUOTED_STRING):
         {
            extern void SetEditTitle(void);
            char *str = new char[strlen(value->GetString())+1];
            strcpy(str,value->GetString());
            int sz = strlen(str);
		      HWND hwnd = obj->hWnd;
            long startSel,endSel;
	         SendMessage(hwnd,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);
         	SendMessage(hwnd,EM_REPLACESEL,(WPARAM)false,(LPARAM)str);
			   long firstLine = SendMessage(hwnd,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);	
			   long lastLine = SendMessage(hwnd,EM_LINEFROMCHAR,(WPARAM)startSel+sz,(LPARAM)0);			   
            delete [] str;
            break;
         }
         default:
            ErrorMessage("invalid parameter value, should be a string");
	         return(ERR);
      }
   }
	else if(parameter == "showsyntax")
	{
		long startSel,endSel;
		int offset = nint(value->GetReal());
		HWND hwnd = obj->hWnd;
		SendMessage(hwnd,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);
	   UpDateCLISyntax(obj->winParent, hwnd, startSel,endSel,true);
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

   return(OK);

}



int SetDividerParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   DividerInfo *info;

   info = (DividerInfo*)obj->data;
  
   if(parameter == "orientation")
   {
      if(value->GetType() == UNQUOTED_STRING)
      {
         char *response = value->GetString();
         if(!strcmp(response,"horiz") || !strcmp(response,"horizontal"))
         {
            info->orientation = WindowLayout::HORIZONTAL;
         }
         else
         {
            info->orientation = WindowLayout::VERTICAL;
         }
      }
	   else
	   {
	      ErrorMessage("invalid parameter value, should be a string");
	      return(ERR);
	   }
   }
   else if(parameter == "limits")
   {
      if(value->GetType() == MATRIX2D && value->GetDimX() == 2 && value->GetDimY() == 1)
      {
         long min = nint(value->GetMatrix2D()[0][0]);
         long max = nint(value->GetMatrix2D()[0][1]);
         if(max <= min || min < 0)
	      {
	         ErrorMessage("invalid limit values max must be > min and both should be positive");
	         return(ERR);
	      }
         info->minPos.xs = 0;
         info->minPos.xo = min;
         info->minPos.region = false;
         info->maxPos.xs = 0;
         info->maxPos.xo = max;
         info->maxPos.region = false;
         info->useLimits = true;
      }
      else if(value->GetType() == LIST && value->GetDimX() == 2)
      {
         char* minStr = value->GetList()[0];
         char* maxStr = value->GetList()[1];
         GetObjectDimensions(minStr, &info->minPos.xs, &info->minPos.xo, &info->minPos.region);
         GetObjectDimensions(maxStr, &info->maxPos.xs, &info->maxPos.xo, &info->maxPos.region);
         info->useLimits = true;
      }
      else
      {
	      ErrorMessage("expecting a 2 element row vector or list");
	      return(ERR);
      }
   }
   //else if(parameter == "mode")
   //{
   //   if(value->GetType() == UNQUOTED_STRING)
   //   {
   //      long type = GetWindowLong(obj->hWnd,GWL_STYLE);
   //      HWND hWnd = obj->hwndParent;
	  //    WinData *win = GetWinDataClass(hWnd);

   //      CText mode = value->GetString();

   //      if(mode == "active")
   //      {   
   //         win->activeCtrl = true;
   //         obj->active = true;
   //      }  
   //      else
   //      {
   //         ErrorMessage("invalid divider mode");
   //         return(ERR);
   //      }
   //      MyInvalidateRect(obj->hWnd,NULL,false);
   //   }
   //}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

   return(OK);
}
//
//short SetObjectParameter(Interface *itfc, ObjectData *obj,  char *parameter, char *value)
//{
//   if(!obj)
//   {
//      ErrorMessage("Object undefined\n");
//      return(ERR);
//   }
//
//   switch(obj->type)
//   {
//	   case(BUTTON):
//	   {
//	      err = GetButtonParameter(obj,parameter,ans);
//	      break;
//	   }
//	   case(RADIO_BUTTON):
//	   {
//	      err = GetRadioButtonParameter(obj,parameter,ans);
//	      break;
//	   }
//	   case(CHECKBOX):
//	   {
//	      err = GetCheckBoxParameter(obj,parameter,ans);
//	      break;
//	   }
//	   case(STATICTEXT):
//	   {
//	      err = GetStaticTextParameter(obj,parameter,ans);
//	      break;
//	   }
//	   case(TEXTBOX):
//	   {
//	      err = SetTextBoxParameter(itfc,obj,parameter,ans);
//	      break;
//	   }
//	   case(GETMESSAGE):
//	   {
//	      err = GetMessageParameter(obj,parameter,itfc);
//         return(err);
//	   } 
//	   case(TEXTMENU):
//	   {
//	      err = GetTextMenuParameter(itfc,obj,parameter,ans);
//	      break;
//	   }  
//	   case(LISTBOX):
//	   {
//	      err = GetListBoxParameter(itfc,obj,parameter,ans);
//	      break;
//	   } 
//	   case(SLIDER):
//	   {
//	      err = GetSliderParameter(obj,parameter,ans); 
//	      break;
//	   }
//	   case(PROGRESSBAR):
//	   {
//	      err = GetProgressBarParameter(obj,parameter,ans); 
//	      break;
//	   }
//      case(COLORBOX):
//      {
//         err = GetColorBoxParameter(obj,parameter,ans); 
//	      break;
//	   }
//      case(UPDOWN):
//      {
//         err = GetUpDownParameter(obj,parameter,ans); 
//	      break;
//	   }
//      case(DIVIDER):
//      {
//         err = GetDividerParameter(obj,parameter,ans); 
//	      break;
//	   }
//   }
//
//   return(OK);
//}

// Replaces all blue pixels [0,0,255] in a bitmap with the background color
void ReplaceBackGroundPixels(HBITMAP pix)
{
   BITMAP bitmap;
   COLORREF *rgb;
   COLORREF bakcol = GetSysColor(COLOR_BTNFACE);
   bakcol = ((bakcol&0x00FF0000)>>16)  + ((bakcol&0x0000FF00)) + ((bakcol&0x000000FF)<<16);
   GetObject(pix,sizeof(BITMAP),&bitmap);
   long w = bitmap.bmWidth;
   long h = bitmap.bmHeight;
   rgb = new COLORREF[4*w*h];
   GetBitmapBits(pix,w*h*4,(LPVOID)rgb);
   for(long i = 0; i < w*h; i++)
   {
      rgb[i] = rgb[i] & 0x00ffffff;
      if(rgb[i] == 0xff) 
        rgb[i] = bakcol;
   }
   SetBitmapBits(pix,w*h*4,(LPVOID)rgb);
   delete [] rgb;
}


int LoadPictureTo3DMatrix(Interface *itfc, char *args)
{
   CText file;
   short nrArgs;
   HBITMAP bmp;
   COLORREF col;

   if((nrArgs = ArgScan(itfc,args,1,"filename","e","t",&file)) < 0)
      return(nrArgs); 

   char* fileName = file.Str();
   wchar_t* fileNameWide = CharToWChar(fileName);

   Bitmap bp(fileNameWide);

   SysFreeString(fileNameWide);

    int w = bp.GetWidth();
    int h = bp.GetHeight();
    if(w > 0 && h > 0)
    {
       Status st = bp.GetHBITMAP( 0, &bmp );

       if(st == OutOfMemory)
       {
          ErrorMessage("can't allocate memory for bitmap");
          return(ERR);
       }
       else if(st != Ok)
       {
          ErrorMessage("can't load bitmap - error %d",(int)st);
          return(ERR);
       }


       if(bmp)
       {
         BITMAP pix;
         COLORREF *rgb;
         GetObject(bmp,sizeof(BITMAP),&pix);
         long w = pix.bmWidth;
         long h = pix.bmHeight;

         rgb = (COLORREF*)MakeIVector(4*w*h-1);
         if(!rgb)
         {
            DeleteObject(bmp);
            ErrorMessage("can't allocate memory for RGB image");
            return(ERR);
         }
     
         GetBitmapBits(bmp,w*h*4,(LPVOID)rgb);
         float ***imag = MakeMatrix3D(w,h,3);
         if(!imag)
         {
            FreeIVector((long*)rgb);
            DeleteObject(bmp);
            ErrorMessage("can't allocate memory for 3D matrix");
            return(ERR);
         }
         for(int y = 0; y < h; y++)
         {
            for(int x = 0; x < w; x++)
            {
               col = rgb[x+w*(h-y-1)];
               imag[0][y][x] = GetBValue(col); // note this is reversed
               imag[1][y][x] = GetGValue(col);
               imag[2][y][x] = GetRValue(col);
            }
         }
         FreeIVector((long*)rgb);

         DeleteObject(bmp);
			// FIXME: nrRetValues is always 1 here, so the 3D matrix is only ever
			//  displayed; it can't be assigned to the LHS.
			if(itfc->nrRetValues == 0) // Copy picture to a 3D matrix variable
         {
            itfc->retVar[1].AssignMatrix3D(imag,w,h,3);
            itfc->nrRetValues = 1;
         }
         else // Copy image to the current plot
         {
				Plot2D* cur2DPlot = Plot2D::curPlot();
            cur2DPlot->clearData();
				cur2DPlot->clearColorMap();
            cur2DPlot->setMatRGB(imag);

            cur2DPlot->setVisibleTop(0);
            cur2DPlot->setVisibleLeft(0);
            cur2DPlot->setVisibleWidth(w);
            cur2DPlot->setVisibleHeight(h);
            cur2DPlot->setMatWidth(w);
            cur2DPlot->setMatHeight(h);
            cur2DPlot->curXAxis()->setBase(0);
            cur2DPlot->curXAxis()->setLength(w);
            cur2DPlot->curYAxis()->setBase(0);
				cur2DPlot->curYAxis()->setLength(h);
			   UpdateStatusWindow(cur2DPlot->win,3,cur2DPlot->statusText);
				cur2DPlot->Invalidate();
         }
       }
       else
       {
          ErrorMessage("can't find bitmap '%s'",fileName);
          return(ERR);
       }
    }
    else
    {
       ErrorMessage("can't find bitmap '%s'",fileName);
       return(ERR);
    }
	 // FIXME: If the problem assigning the matrix is fixed, 
	 //  then this line unconditionally setting nrRetValues = 0 must be removed!
	 itfc->nrRetValues = 0;
	 // 
    return(OK);

}

int ProcessValuePosition(Variable* value, float* in_scale, float* in_offset, bool* in_region, char* description)
{
	if(value->GetType() == FLOAT32) // Already a float
	{
		*in_scale = 0; *in_offset = value->GetReal();
	}
	else if(value->GetType() == UNQUOTED_STRING) // An expression?
   {
		float scale, off;
		bool region;
      if(GetObjectDimensions(value->GetString(), &scale, &off, &region) == ERR)
      {
			ErrorMessage("invalid %s position", description);
         return(ERR);
		}
      *in_scale = scale; *in_offset = off; *in_region = region;
	}
   else // Something else
   {
		ErrorMessage("Invalid data type");
      return(ERR);
	}
	return (OK);
}
