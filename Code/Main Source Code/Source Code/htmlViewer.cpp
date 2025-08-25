#include "stdafx.h"
#include "htmlviewer.h"
#include <mshtml.h>
#include <comdef.h>
#include <comdefsp.h>
#include <exdisp.h>
#include <ole2.h>
#include "globals.h"
#include "ax.h"
#include "guiWindowClass.h"
#include "main.h"
#include "process.h"
#include "variablesOther.h"
#include "string_utilities.h"
#include "scanstrings.h"
#include "interface.h"
//#include <atlstr.h>
#include "memoryLeak.h"


HWND htmlHwnd;
WinData* htmlWinParent;

IHTMLDocument2 *GetDocument(HWND hWnd);
void ContentWrite(HWND hWnd, char *txt);
void RunJavaScript(HWND hWnd, char *txt);
BSTR GetBStrFromCharStr(char *txt);
int HTMLReplaceString(Interface* itfc ,char arg[]);
int HTMLCountSubString(Interface* itfc ,char arg[]);


/************************************************************************
    Replace all instances of oldsubstr with newsubstr in string text
    Ignore all matches inside <head> tag
    Note that all instances oldsubstr in newsubstr will have the same
    case as in the original text.
    Syntax: result = htmlreplacestr(str,oldsubstr,newsubstr)

************************************************************************/

int HTMLReplaceString(Interface* itfc ,char args[])
{
   CText text;
   CText oldStr;
   CText newStr;
   CText newStrCnt;
   CText result;
   CText startTag;
   CText endTag;
   CText caseSensitive = "ignorecase";
   long i,j,k,l,m;
   bool ignoreCase = true;
   short nrArgs;
   long lenText,lenOld,lenNew;
   bool inTag = false;
   bool inHead = false;
   int matchCnt = 0;

// Get string to search and substring to replace *************  
   if((nrArgs = ArgScan(itfc,args,3,"text, oldsubstr, newsubstr [, casesensitivity]","eeee","tttt",&text,&oldStr,&newStr,&caseSensitive)) < 0)
      return(nrArgs);

   if(caseSensitive == "ignorecase")
       ignoreCase = true;
   else if(caseSensitive == "casesensitive")
       ignoreCase = false;
   else
   {
      ErrorMessage("invalid casesensitivity parameter (ignorecase/casesensitive)");
      return(ERR);
   }

   lenText = text.Size();
   lenOld = oldStr.Size();
   lenNew = newStr.Size();
   if(oldStr == "")
   {
      result = text;
      itfc->retVar[1].MakeAndSetString(result.Str());
      itfc->retVar[2].MakeAndSetFloat(0);
      itfc->nrRetValues = 2;

      return(OK);
   }


   for(i = 0; i <= lenText; i++)
   {
      if(text[i] == '<')
      {
         for(j = i+1; j <= lenText; j++)
         {
            if(text[j] == ' ' || text[j] == '>')
            {
               startTag = text.Middle(i+1,j-1);
               if(!inHead && startTag == "head")
                  inHead = true;
               break;
            }
         }
         inTag = true;

      }
      if(text[i] == '>')
      {
         for(j = i-1; j > 0; j--)
         {
            if(text[j] == '/')
            {
               endTag = text.Middle(j+1,i-1);
               if(inHead && endTag == "head")
                  inHead = false;

               break;
            }
         }

         inTag = false;
      }

      if(text[i] !=  '>' && !inTag && !inHead)
      {
         for(j = 0; j < lenOld; j++)
         {
            if(ignoreCase)
            {
               if(tolower(text[i+j]) != tolower(oldStr[j]))
                  break;
            }
            else
            {
               if(text[i+j] != oldStr[j])
                  break;
            }         
         }
         if(j == lenOld) // Match found
         {
            m = 0;
            int pos = newStr.FindSubStr(0,"%cnt%"); // Check for search counter
            if(pos == -1)
               newStrCnt = newStr;
            else
            {
               newStrCnt.Format("%s%d%s",newStr.Start(pos-1).Str(),matchCnt++,newStr.End(pos+5).Str());
            }

          // Make sure that the all instances of oldStr in newStr has the same case
          //  if(!ignoreCase)
            {
               CText newStrLower = newStrCnt;
               CText oldStrLower = oldStr;
               CText oldStrOrig = text.Middle(i,i+oldStr.Size()-1); // Original text format
               newStrLower.LowerCase();
               oldStrLower.LowerCase();
               pos = 0;
               do
               {
                  pos = newStrLower.FindSubStr(pos,oldStrLower.Str()); // Ignore tag
                  if(pos >= 0)
                  {
                     for(k = pos; k < pos+oldStr.Size(); k++)
                        newStrCnt[k] = oldStrOrig[k-pos];
                     pos = pos + oldStr.Size();
                  }
               }
               while(pos >= 0);
            }

          // Add new string to result
            for(k = 0; k < newStrCnt.Size(); k++)
               result.Append(newStrCnt[k]);
            i += lenOld-1;
         }
         else
            result.Append(text[i]);
      }
      else
      {
         result.Append(text[i]);
      }
   }
 
// Copy the result string to ansVar and free used memory
   itfc->retVar[1].MakeAndSetString(result.Str());
   itfc->retVar[2].MakeAndSetFloat(matchCnt);
   itfc->nrRetValues = 2;

   return(OK);
}


/************************************************************************
    Count all instances of substr in string text
    Ignore all matches inside <head> tag

    Syntax: cnt = HTMLCountSubString(text,substr)

************************************************************************/

int HTMLCountSubString(Interface* itfc ,char args[])
{
   CText text;
   CText subStr;
  // CText result;
   CText startTag;
   CText endTag;
   CText caseSensitive = "ignorecase";
   long i,j,k,l,m;
   bool ignoreCase = true;
   short nrArgs;
   long lenText,lenSubStr;
   bool inTag = false;
   bool inHead = false;
   int matchCnt = 0;

// Get string to search and substring to replace *************  
   if((nrArgs = ArgScan(itfc,args,2,"text, substr [, casesensitivity]","eee","ttt",&text,&subStr,&caseSensitive)) < 0)
      return(nrArgs);  

   if(caseSensitive == "ignorecase")
       ignoreCase = true;
   else if(caseSensitive == "casesensitive")
       ignoreCase = false;
   else
   {
      ErrorMessage("invalid casesensitivity parameter (ignorecase/casesensitive)");
      return(ERR);
   }

   lenText = text.Size();
   lenSubStr = subStr.Size();

   for(i = 0; i < lenText; i++)
   {
      if(text[i] == '<')
      {
         for(j = i+1; j < lenText; j++)
         {
            if(text[j] == ' ' || text[j] == '>')
            {
               startTag = text.Middle(i+1,j-1);
               if(!inHead && startTag == "head")
                  inHead = true;
               break;
            }
         }
         inTag = true;

      }
      if(text[i] == '>')
      {
         for(j = i-1; j > 0; j--)
         {
            if(text[j] == '/')
            {
               endTag = text.Middle(j+1,i-1);
               if(inHead && endTag == "head")
                  inHead = false;

               break;
            }
         }

         inTag = false;
      }

      if(text[i] !=  '>' && !inTag && !inHead)
      {
         for(j = 0; j < lenSubStr; j++)
         {
            if(ignoreCase)
            {
               if(tolower(text[i+j]) != tolower(subStr[j]))
                  break;
            }
            else
            {
               if(text[i+j] != subStr[j])
                  break;
            }         
         }
         if(j == lenSubStr) // Match found
         {
            matchCnt++;
            i += lenSubStr-1;
         }
      }
   }
 
// Return the number of matches
   itfc->retVar[1].MakeAndSetFloat(matchCnt);
   itfc->nrRetValues = 1;

   return(OK);
}

// Clear the current html viewer window

void ContentClear(HWND hWnd)
{
 //  SendMessage(hWnd, WM_SETREDRAW, false, 0);
 //  SendMessage(hWnd,AX_INPLACE,0,0);
	//
 //  IWebBrowser2* wb = 0;
 //  SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);

	//if (wb != NULL)
 //  {
	//	CString Content = _T("");

	//	// get document interface

	//	IHTMLDocument2	*document	= GetDocument(hWnd);

 //     IHTMLWindow2* pIHTMLWindow2 ;
 //     document->get_Script(((IDispatch**)&pIHTMLWindow2));
 //     VARIANT ret;
 //     pIHTMLWindow2->execScript(::SysAllocString(L"init();"), ::SysAllocString(L"javascript"),&ret);
 //     
	//	HRESULT hr	= S_OK;
	//	
	//	if (document != NULL) 
 //     {
	//		// close and re-open document to empty contents

	//		document->close();
	//		
	//		VARIANT		open_name;
	//		VARIANT		open_features;
	//		VARIANT		open_replace;
	//		IDispatch	*open_window	= NULL;

	//		::VariantInit(&open_name);

	//		open_name.vt      = VT_BSTR;
	//		open_name.bstrVal = ::SysAllocString(L"_self");

	//		::VariantInit(&open_features);
	//		::VariantInit(&open_replace);
	//		
	//		hr = document->open(::SysAllocString(L"text/html"),
	//		                    open_name,
	//		                    open_features,
	//		                    open_replace,
	//		                    &open_window);

	//		if (hr == S_OK)
 //        {
	//	     wb->Refresh();
	//		}

	//		if (open_window != NULL)
 //        {
	//			open_window->Release();
	//		}

	//		::VariantClear(&open_name);
	//		
	//		document->Release();
	//		document = NULL;
	//	
	//	}
 //  }
 //  SendMessage(hWnd,AX_INPLACE,1,0);
	//SendMessage(hWnd, WM_SETREDRAW, true, 0); 
}

// Run 'script' on current HTML text
void RunJavaScript(HWND hWndParent, char *script)
{
	IHTMLDocument2	*document = GetDocument(hWndParent);
   IHTMLWindow2* htmlWin2;
   document->get_parentWindow(&htmlWin2);
   BSTR bStr = GetBStrFromCharStr(script);
   VARIANT ret;
 //  htmlWin2->execScript(bStr, ::SysAllocString(L"javascript"),&ret);
   htmlWin2->execScript(bStr, ::SysAllocString(L"JScript"),&ret);
   SysFreeString(bStr);
	document->close();
	document->Release();
}

// Convert char string to BSTR. Make sure the BSTR is deallocated with
// SysFreeString when finished
BSTR GetBStrFromCharStr(char *txt)
{
   int sz = strlen(txt);
   int wslen = MultiByteToWideChar(CP_ACP, 0, txt, sz, 0, 0);
   BSTR bStr = SysAllocStringLen(0, wslen);
   MultiByteToWideChar(CP_ACP, 0, txt, sz, bStr, wslen);
   return(bStr);
}

// Write html text to current viewer window
// Note currently does work with links and
// all images must full paths

void ContentWrite(HWND hWnd, char *txt)
{
 //  SendMessage(hWnd, WM_SETREDRAW, false, 0);
 //  SendMessage(hWnd,AX_INPLACE,0,0);
	//
	//// get document interface
	//IHTMLDocument2 *document = GetDocument(hWnd);
	//	
	//if (document != NULL)
 //  {
 //     CString Content = txt;

	//	// construct text to be written to browser as SAFEARRAY

	//	SAFEARRAY *safe_array = SafeArrayCreateVector(VT_VARIANT,0,1);
	//		
	//	VARIANT	*variant;
	//		
	//	SafeArrayAccessData(safe_array,(LPVOID *)&variant);
	//		
	//	variant->vt      = VT_BSTR;
	//	variant->bstrVal = Content.AllocSysString();
	//		
	//	SafeArrayUnaccessData(safe_array);

	//	// write SAFEARRAY to browser document

	//	document->write(safe_array);
	//		
	//	// cleanup
	//		
	//	document->Release();
	//	document = NULL;

	//	::SysFreeString(variant->bstrVal);
	//	variant->bstrVal = NULL;
	//		
	//	SafeArrayDestroy(safe_array);
		
 //  }

	//
 //  SendMessage(hWnd,AX_INPLACE,1,0);
	//SendMessage(hWnd, WM_SETREDRAW, true, 0); 
}


IHTMLDocument2 *GetDocument(HWND hWnd)
{
	IHTMLDocument2 *document = NULL;
   IWebBrowser2* wb = 0;
   SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);

	if (wb != NULL)
   {	
		// get browser document's dispatch interface
		IDispatch *document_dispatch = NULL;
		HRESULT hr = wb->get_Document(&document_dispatch);
		if (SUCCEEDED(hr) && (document_dispatch != NULL)) {
			// get the actual document interface
			hr = document_dispatch->QueryInterface(IID_IHTMLDocument2,
			                                       (void **)&document);
			// release dispatch interface					
			document_dispatch->Release();		
		}		
	}	
	return(document);
}


bool RegisterHTMLViewer()
{
   OleInitialize(0);

   if (!AXRegister())
	   return(false);

   return(true);
}

void UnRegisterHTMLViewer()
{
   OleUninitialize();
}

void InitialiseHtmlViewer(HWND hWnd)
{
   EnableHtmlViewer(hWnd,1);
   htmlHwnd = hWnd;
   HWND parent =  GetParent(hWnd);
   htmlWinParent = GetWinDataClass(parent);
}

void EnableHtmlViewer(HWND hWnd, short enable)
{
   SendMessage(hWnd,AX_INPLACE,enable,0);
}

void LoadURLIntoHTMLViewer(HWND hWnd, char* URL)
{
   SendMessage(hWnd, WM_SETREDRAW, false, 0);
   wchar_t* unicodestr = CharToWChar(URL);
   SendMessage(hWnd,AX_INPLACE,0,0);

   // Navigate
   IWebBrowser2* wb = 0;
   SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);
   if(wb)
   {
      wb->Navigate(unicodestr,0,0,0,0);
      wb->Release();
   }
   SysFreeString(unicodestr);

   SendMessage(hWnd,AX_INPLACE,1,0);
	SendMessage(hWnd, WM_SETREDRAW, true, 0); 
}

// Go to the last page
void GoBackHTMLViewer(HWND hWnd)
{
   SendMessage(hWnd, WM_SETREDRAW, false, 0); 
   IWebBrowser2* wb = 0;
   SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);
   if(wb)
   {
      wb->GoBack();
      wb->Release();
   }
   SendMessage(hWnd,AX_INPLACE,0,0);
   SendMessage(hWnd,AX_INPLACE,1,0);
	SendMessage(hWnd, WM_SETREDRAW, true, 0);    
}

// Go to the next page
void GoForwardHTMLViewer(HWND hWnd)
{
   SendMessage(hWnd, WM_SETREDRAW, false, 0); 
   IWebBrowser2* wb = 0;
   SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);
   if(wb)
   {
      wb->GoForward();
      wb->Release();
   }
   SendMessage(hWnd,AX_INPLACE,0,0);
   SendMessage(hWnd,AX_INPLACE,1,0);
   SendMessage(hWnd, WM_SETREDRAW, true, 0); 
}



// Copy selection in HTML window hWnd to clip board.  

void CopyHTMLViewerSelectionToClipBoard(HWND hWnd)
{
   if(htmlHwnd && htmlWinParent)
   {
      IWebBrowser2* wb = 0;
      SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);
      if(wb)
      {
         IDispatch* pDisp,*range;
         IHTMLDocument2* pHTMLDocument2;
         IHTMLTxtRange *tr;
         IHTMLSelectionObject* p;
         HRESULT hr;
         BSTR selStr;
         HGLOBAL hGlobal = 0;

         wb->get_Document(&pDisp);
			hr = pDisp->QueryInterface( IID_IHTMLDocument2,(void**)&pHTMLDocument2 );
         pHTMLDocument2->get_selection(&p);
         hr = p->createRange(&range);

         hr =  range->QueryInterface(IID_IHTMLTxtRange,(void**)&tr );
         tr->get_text(&selStr);
         if(selStr)
         {
            long len = SysStringLen(selStr) + 1;
            hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,len);
            char *aselStr = (char*)GlobalLock(hGlobal);
            WideCharToMultiByte(CP_ACP,0,selStr,len,aselStr,len,NULL,FALSE);
            GlobalUnlock(hGlobal);
         }
         tr->Release();
         range->Release();
         p->Release();
         pDisp->Release();
         wb->Release();

         if(selStr && OpenClipboard(hWnd)) 
         { 
            EmptyClipboard();
				if (hGlobal)
				{
					SetClipboardData(CF_TEXT,hGlobal) ; 
				}
            CloseClipboard(); 
         }
      }
   }

}

// Run selected text in HTML help viewer

void RunHTMLViewerSelection(HWND hWnd)
{
   if(htmlHwnd && htmlWinParent)
   {
      IWebBrowser2* wb = 0;
      SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);
      if(wb)
      {
         IDispatch* pDisp,*range;
         IHTMLDocument2* pHTMLDocument2;
         IHTMLTxtRange *tr;
         IHTMLSelectionObject* p;
         HRESULT hr;
         BSTR selStr;
         HGLOBAL hGlobal;

         wb->get_Document(&pDisp);
         hr = pDisp->QueryInterface( IID_IHTMLDocument2,(void**)&pHTMLDocument2 );
         pHTMLDocument2->get_selection(&p);
         hr = p->createRange(&range);

         hr =  range->QueryInterface(IID_IHTMLTxtRange,(void**)&tr );
         tr->get_text(&selStr);
         if(selStr)
         {
            long len = SysStringLen(selStr) + 1;
            hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,len);
            char *aselStr = (char*)GlobalLock(hGlobal);
            WideCharToMultiByte(CP_ACP,0,selStr,len,aselStr,len,NULL,FALSE);
            ProcessMacroStr(LOCAL,aselStr);
            GlobalUnlock(hGlobal);
            GlobalFree(hGlobal);
         }
         tr->Release();
         range->Release();
         p->Release();
         pDisp->Release();
         wb->Release();
      }
   }

}

void SetHTMLViewerFocus(HWND hWnd, HWND parent)
{
   WinData* win = GetWinDataClass(parent);
   if(!win || win->inRemoval) return;         
	ObjectData *obj = win->widgets.findByWin(hWnd); // Find object selected
   win->objWithFocus = obj;
}

void DestroyHTMLViewer(HWND hWnd)
{
   htmlHwnd = NULL;
   htmlWinParent = NULL;
 //  SendMessage(hWnd,WM_DESTROY,(WPARAM)0,(LPARAM)0); Does nothing??
}

// The Viewer is being resize - need to refresh to ensure that 
// the scroll-bars are correctly resized
void ResizeHTMLViewer(HWND hWnd, long width, long height)
{
// SendMessage(hWnd, WM_SETREDRAW, false, 0); 
   IWebBrowser2* wb = 0;
   //SendMessage(hWnd,AX_INPLACE,0,0);
   //SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);
   //if(wb)
   //{
   //   wb->put_Width(width+1);
   //   wb->put_Height(height+1);
   //   wb->Release();
   //}
   //SendMessage(hWnd,AX_INPLACE,1,0);

   //SendMessage(hWnd,AX_INPLACE,0,0);
   //SendMessage(hWnd,AX_QUERYINTERFACE,(WPARAM)&IID_IWebBrowser2,(LPARAM)&wb);
   //if(wb)
   //{
   //   wb->put_Width(width);
   //   wb->put_Height(2000);
   //   wb->Release();
   //}

   //SendMessage(hWnd,AX_INPLACE,1,0);
 //   SendMessage(hWnd,WM_SIZE,SIZE_RESTORED,MAKELPARAM(width+1,height));
 //   SendMessage(hWnd,WM_SIZE,SIZE_RESTORED,MAKELPARAM(width,height));

 //  SendMessage(hWnd, WM_SETREDRAW, true, 0);
}

void HTMLViewerMessage(MSG *msg)
{
   if(htmlHwnd && htmlWinParent->hWnd == currentAppWindow)
   {
      if(htmlWinParent->objWithFocus && htmlWinParent->objWithFocus->hWnd == htmlHwnd)
      {
         if(msg->message>=WM_KEYFIRST && msg->message<=WM_KEYLAST)
            SendMessage(htmlWinParent->hWnd, msg->message, msg->wParam, msg->lParam);
      }
   }
}


