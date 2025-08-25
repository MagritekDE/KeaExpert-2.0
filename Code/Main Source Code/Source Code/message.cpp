#include "stdafx.h"
#include "message.h"
#include "defineWindows.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "winEnableClass.h"
#include "memoryLeak.h"

/***************************************************************
  CLI interface to Message dialog
****************************************************************/

int DisplayMessage(Interface* itfc ,char args[])
{
   CText title;
   CText text;
   CText icon = "info";
   short r;
   WinData *win;
   HWND hWnd;


   if((r = ArgScan(itfc,args,2,"title,text,[icon]","eee","ttt",&title,&text,&icon)) < 0)
	{
		itfc->nrRetValues = 0;
		return(r);
	}

   win = GetGUIWin();
   if(win)
      hWnd = win->hWnd;
   else
      hWnd = NULL;
   
   if(icon == "error")
      MessageDialogExt(hWnd,MB_ICONERROR,title.Str(),text.Str());
   else if(icon == "warning")
      MessageDialogExt(hWnd,MB_ICONWARNING,title.Str(),text.Str());
   else if(icon == "info")
      MessageDialogExt(hWnd,MB_ICONASTERISK,title.Str(),text.Str());
   else
   {
      ErrorMessage("invalid icon name");
      return(ERR);
   }

//  SetGUIWin(win);
	itfc->nrRetValues = 0;
   return(OK);
}

/***************************************************************
  CLI interface to Yes-No-Cancel (Query) Dialog
****************************************************************/

int YesNoMessage(Interface* itfc, char args[])
{
   CText title;
   CText text;
   CText pos = "yes";
   CText option = "yes/no";
   short r;
   WinData* oldGUI;
   
   if((r = ArgScan(itfc,args,2,"title,text,[[default],option]","eeee","tttt",&title,&text,&pos,&option)) < 0)
	{
		itfc->nrRetValues = 0;
		return(r);
	}

   oldGUI = GetGUIWin();

   if(option == "yes/no")
   {
      if(pos == "yes")
         r = YesNoDialogExt(MB_ICONWARNING,1,title.Str(),text.Str());
      else
         r = YesNoDialogExt(MB_ICONWARNING,2,title.Str(),text.Str());
   }
   else if(option == "yes/no/cancel")
   {
      if(pos == "yes")
         r = YesNoCancelDialogExt(MB_ICONWARNING,1,title.Str(),text.Str());
      else if(pos == "no")
         r = YesNoCancelDialogExt(MB_ICONWARNING,2,title.Str(),text.Str());
      else
         r = YesNoCancelDialogExt(MB_ICONWARNING,3,title.Str(),text.Str());
   }
   
   if(r == IDYES)
      itfc->retVar[1].MakeAndSetString("yes");
   else if(r == IDNO)
      itfc->retVar[1].MakeAndSetString("no");
   else
       itfc->retVar[1].MakeAndSetString("cancel");
     
   itfc->nrRetValues = 1;

   if(oldGUI)
      WinData::SetGUIWin(oldGUI);

   return(OK);
}

/***************************************************************
  Display a dialog with 1 button labeled "OK"
****************************************************************/

void MessageDialog(HWND win,long icon, char title[],char text[],...)
{
   va_list ap;
   char *textEval; 
   CWinEnable winEnable;
   
   va_start(ap,text);
   textEval = vssprintf(text,ap);
   winEnable.Disable(NULL);
   MessageBeep(MB_ICONEXCLAMATION);
   ReplaceSpecialCharacters(textEval, "\\r", "\r", -1);
   MessageBox(win,textEval,title,MB_OK | MB_APPLMODAL | icon);
   winEnable.Enable(NULL);
   va_end(ap);
   delete [] textEval;
}

/***************************************************************
  Display a dialog with 1 button labeled "OK"
  Special procedure for Message command
****************************************************************/

void MessageDialogExt(HWND win,long icon, char title[],char text[])
{
   CWinEnable winEnable;
   winEnable.Disable(NULL);
   MessageBeep(MB_ICONEXCLAMATION);
   ReplaceSpecialCharacters(text, "\\r", "\r", -1);
   MessageBox(win,text,title,MB_OK | MB_APPLMODAL | icon);
   winEnable.Enable(NULL);
}

/***************************************************************
   Display a dialog with 3 buttons labeled "yes", "no" and "cancel"
****************************************************************/

int QueryDialog(long icon, char title[],char text[],...)
{
   va_list ap;
   char *textEval;
   int r;
   CWinEnable winEnable;
   
   va_start(ap,text);
   textEval = vssprintf(text,ap);
   winEnable.Disable(NULL);
   MessageBeep(MB_ICONEXCLAMATION);
   ReplaceSpecialCharacters(textEval, "\\r", "\r", -1);
   r = MessageBox(prospaWin,textEval,title,MB_YESNOCANCEL | icon | MB_APPLMODAL);
   winEnable.Enable(NULL);
   va_end(ap);
   delete [] textEval;
   return(r);
}

/***************************************************************
   Display a dialog with 3 buttons labeled "yes", "no" and "cancel"
   Special procedure current not called
****************************************************************/

int QueryDialogExt(long icon, char title[],char text[])
{
   int r;
   CWinEnable winEnable;
   
   winEnable.Disable(NULL);
   MessageBeep(MB_ICONEXCLAMATION);
   ReplaceSpecialCharacters(title, "\\r", "\r", -1);
   r = MessageBox(prospaWin,title,title,MB_YESNOCANCEL | icon | MB_APPLMODAL);
   winEnable.Enable(NULL);

   return(r);
}


/***************************************************************
   Display a dialog with 2 buttons labeled "yes", "no"
****************************************************************/

int YesNoDialog(long icon, short def, char title[],char text[],...)
{
   va_list ap;
   char *textEval;
   int r;
   CWinEnable winEnable;
   
   va_start(ap,text);
   textEval = vssprintf(text,ap);
   winEnable.Disable(NULL);
   MessageBeep(MB_ICONEXCLAMATION);
   ReplaceSpecialCharacters(textEval, "\\r", "\r", -1);
   if(def == 1)
      r = MessageBox(prospaWin,textEval,title,MB_YESNO | icon | MB_APPLMODAL | MB_DEFBUTTON1);
   else
      r = MessageBox(prospaWin,textEval,title,MB_YESNO | icon | MB_APPLMODAL | MB_DEFBUTTON2);

   winEnable.Enable(NULL);	
   va_end(ap);
   delete [] textEval;     
   return(r);
}

/***************************************************************
   Display a dialog with 2 buttons labeled "yes", "no"
   Special procedure for Query command
****************************************************************/

int YesNoDialogExt(long icon, short def, char title[],char text[])
{
   int r;
   CWinEnable winEnable;
   
   winEnable.Disable(NULL);
   MessageBeep(MB_ICONEXCLAMATION);
   ReplaceSpecialCharacters(text, "\\r", "\r", -1);
   if(def == 1)
      r = MessageBox(prospaWin,text,title,MB_YESNO | icon | MB_APPLMODAL | MB_DEFBUTTON1);
   else
      r = MessageBox(prospaWin,text,title,MB_YESNO | icon | MB_APPLMODAL | MB_DEFBUTTON2);

   winEnable.Enable(NULL);	

   return(r);
}

/***************************************************************
   Display a dialog with 2 buttons labeled "yes", "no"
   Special procedure for Query command
****************************************************************/

int YesNoCancelDialogExt(long icon, short def, char title[],char text[])
{
   int r;
   CWinEnable winEnable;
   
   winEnable.Disable(NULL);
   MessageBeep(MB_ICONEXCLAMATION);
   ReplaceSpecialCharacters(text, "\\r", "\r", -1);
   if(def == 1)
      r = MessageBox(prospaWin,text,title,MB_YESNOCANCEL | icon | MB_APPLMODAL | MB_DEFBUTTON1);
   else if(def == 2)
      r = MessageBox(prospaWin,text,title,MB_YESNOCANCEL | icon | MB_APPLMODAL | MB_DEFBUTTON2);
   else
      r = MessageBox(prospaWin,text,title,MB_YESNOCANCEL | icon | MB_APPLMODAL | MB_DEFBUTTON3);

   winEnable.Enable(NULL);	

   return(r);
}

