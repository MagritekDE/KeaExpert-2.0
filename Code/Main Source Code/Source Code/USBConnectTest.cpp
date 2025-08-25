#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#pragma pack(push,default) // Must not use 1 byte packing in USB device structure
#include <dbt.h>
#pragma pack(pop)
#include "cArg.h"
#include "debug.h"
#include "defineWindows.h"
#include "defines.h"
#include "edit_class.h"
#include "events_edit.h"
#include "edit_files.h"
#include "files.h"
#include "globals.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiModifyObjectParameters.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "htmlviewer.h"
#include "interface.h"
#include "main.h"
#include "memoryLeak.h"

// This GUID is for all USB serial host PnP drivers, but you can replace it 
// with any valid device class guid.
//GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72, 
//                      0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };

GUID WceusbshGUID = { 0xa5dcbf10, 0x6530, 0x11d2, { 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed }  };

static const GUID GuidInterfaceList[] =
{
{ 0xa5dcbf10, 0x6530, 0x11d2, { 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed } },
{ 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
{ 0x4d1e55b2, 0xf16f, 0x11Cf, { 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
{ 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } },
{ 0x219d0508, 0x57a8, 0x4ff5, {0x97, 0xa1, 0xbd, 0x86, 0x58, 0x7c, 0x6c, 0x7e   } },
{0x86e0d1e0L, 0x8089, 0x11d0, {0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73   } },
};

// For informational messages and window titles
LPCSTR g_pszAppName;

// Forward declarations
void OutputMessage(HWND hOutWnd, WPARAM wParam, LPARAM lParam);
void ErrorHandler(LPTSTR lpszFunction);

int USBTestEntry(void);
    static HWND hEditWnd;
//
// DoRegisterDeviceInterfaceToHwnd
//
BOOL DoRegisterDeviceInterfaceToHwnd( 
                                     IN GUID InterfaceClassGuid, 
                                     IN HWND hWnd,
                                     OUT HDEVNOTIFY *hDeviceNotify)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

     
       NotificationFilter.dbcc_classguid = WceusbshGUID;

       *hDeviceNotify = RegisterDeviceNotification( 
           hWnd,                       // events recipient
           &NotificationFilter,        // type of device
           DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
           );

       if ( NULL == *hDeviceNotify ) 
       {
           ErrorHandler(TEXT("RegisterDeviceNotification"));
           return FALSE;
       }

    return TRUE;
}

//
// MessagePump
//
void MessagePump(
    HWND hWnd
)
// Routine Description:
//     Simple main thread message pump.
//

// Parameters:
//     hWnd - handle to the window whose messages are being dispatched

// Return Value:
//     None.
{
    MSG msg; 
    int retVal;

    // Get all messages for any window that belongs to this thread,
    // without any filtering. Potential optimization could be
    // obtained via use of filter values if desired.

    while( (retVal = GetMessage(&msg, NULL, 0, 0)) != 0 ) 
    { 
        if ( retVal == -1 )
        {
            ErrorHandler(TEXT("GetMessage"));
            break;
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } 
}

//
// WinProcCallback
//
LRESULT CALLBACK WinProcCallback(
                              HWND hWnd,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam
                              )
{
    LRESULT lRet = 1;


    static ULONGLONG msgCount = 0;

    switch (message)
    {
    case WM_CREATE:
       

        break;

    case WM_SETFOCUS: 
        SetFocus(hEditWnd); 

        break;

    case WM_SIZE: 
        // Make the edit control the size of the window's client area. 
        MoveWindow(hEditWnd, 
                   0, 0,                  // starting x- and y-coordinates 
                   LOWORD(lParam),        // width of client area 
                   HIWORD(lParam),        // height of client area 
                   TRUE);                 // repaint window 

        break;

    case WM_DEVICECHANGE:
    {
        //
        // This is the actual message from the interface via Windows messaging.
        // This code includes some additional decoding for this particular device type
        // and some common validation checks.
        //
        // Note that not all devices utilize these optional parameters in the same
        // way. Refer to the extended information for your particular device type 
        // specified by your GUID.
        //
        PDEV_BROADCAST_DEVICEINTERFACE b = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;
        TCHAR strBuff[256];

        // Output some messages to the window.
        switch (wParam)
        {
        case DBT_DEVICEARRIVAL:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: DBT_DEVICEARRIVAL\n"), msgCount);
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: DBT_DEVICEREMOVECOMPLETE\n"), msgCount);
            break;
        case DBT_DEVNODES_CHANGED:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: DBT_DEVNODES_CHANGED\n"), msgCount);
            break;
        default:
            msgCount++;
            StringCchPrintf(
                strBuff, 256, 
                TEXT("Message %d: WM_DEVICECHANGE message received, value %d unhandled.\n"), 
                msgCount, wParam);
            break;
        }
        OutputMessage(hEditWnd, wParam, (LPARAM)strBuff);
    }
            break;
    case WM_CLOSE:
        //if ( ! UnregisterDeviceNotification(hDeviceNotify) )
        //{
        //   ErrorHandler(TEXT("UnregisterDeviceNotification")); 
        //}
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        // Send all other messages on to the default windows handler.
        lRet = DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return lRet;
}

#define WND_CLASS_NAME TEXT("SampleAppWindowClass")

//
// InitWindowClass
//
BOOL InitWindowClass()
{
    WNDCLASS wndClass;

    wndClass.lpszClassName = "SampleAppWindowClass";
    wndClass.hInstance = prospaInstance;
    wndClass.lpfnWndProc = WinProcCallback;
    wndClass.hCursor = LoadCursor(0, IDC_ARROW);
    wndClass.hIcon = LoadIcon(0,IDI_APPLICATION);
    wndClass.hbrBackground = CreateSolidBrush(RGB(192,192,192));
    wndClass.style  = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpszMenuName = NULL;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;


    if ( ! RegisterClass(&wndClass) )
    {
        ErrorHandler(TEXT("RegisterClass"));
        return FALSE;
    }
    return TRUE;
}

//
// main
//

int USBTestEntry()
{
    static HDEVNOTIFY hDeviceNotify;
    WNDCLASS wc;

// Main gui window
   wc.lpszClassName    = "USERWIN2";
   wc.hInstance       = prospaInstance;
   wc.lpfnWndProc      = WinProcCallback;
   wc.hCursor         = LoadCursor( NULL, IDC_ARROW );
   wc.hIcon           = NULL;
   wc.lpszMenuName   = NULL;
   wc.hbrBackground   = CreateSolidBrush(GetSysColor(COLOR_BTNFACE)); 
   wc.style            = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
   wc.cbClsExtra      = 0;
   wc.cbWndExtra      = 0;
   if(!RegisterClass(&wc))
      return false;  

    HWND hWnd = CreateWindow(
                    "USERWIN2",
                    "My Win",
                     WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX| WS_MINIMIZEBOX,
                    CW_USEDEFAULT, 0, 
                    640, 480,
                    NULL, NULL, 
                    prospaInstance, 
                    NULL);


    if ( hWnd == NULL )
    {
        ErrorHandler(TEXT("CreateWindowEx: main appwindow hWnd"));
        return -1;
    }

       DoRegisterDeviceInterfaceToHwnd(
                     WceusbshGUID, 
                     hWnd,
                     &hDeviceNotify);
      //{
      //   // Terminate on failure.
      //   ErrorHandler(TEXT("DoRegisterDeviceInterfaceToHwnd"));
      //   ExitProcess(1);
      //}

        hEditWnd = CreateWindow(TEXT("EDIT"),// predefined class 
                                NULL,        // no window title 
                                WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
                                ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 
                                0, 0, 200,200,  // set size in WM_SIZE message 
                                hWnd,        // parent window 
                                (HMENU)1,    // edit control ID 
                                (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE), 
                                NULL);       // pointer not needed 

        if ( hEditWnd == NULL )
        {
            // Terminate on failure.
            ErrorHandler(TEXT("CreateWindow: Edit Control"));
            ExitProcess(1);
        }
        // Add text to the window. 
        SendMessage(hEditWnd, WM_SETTEXT, 0, 
            (LPARAM)TEXT("Registered for USB device notification...\n")); 


    // Actually draw the window.

    ShowWindow(hWnd, SW_SHOWNORMAL);
    ShowWindow(hEditWnd, SW_SHOWNORMAL);
    MyUpdateWindow(hWnd);

    // The message pump loops until the window is destroyed.

  //  MessagePump(hWnd);

    return 1;
}

//
// OutputMessage
//
void OutputMessage(
    HWND hOutWnd, 
    WPARAM wParam, 
    LPARAM lParam
)
// Routine Description:
//     Support routine.
//     Send text to the output window, scrolling if necessary.

// Parameters:
//     hOutWnd - Handle to the output window.
//     wParam  - Standard windows message code, not used.
//     lParam  - String message to send to the window.

// Return Value:
//     None

// Note:
//     This routine assumes the output window is an edit control
//     with vertical scrolling enabled.

//     This routine has no error checking.
{
    LRESULT   lResult;
    LONG      bufferLen;
    LONG      numLines;
    LONG      firstVis;
  
    // Make writable and turn off redraw.
    lResult = SendMessage(hOutWnd, EM_SETREADONLY, FALSE, 0L);
    lResult = SendMessage(hOutWnd, WM_SETREDRAW, FALSE, 0L);

    // Obtain current text length in the window.
    bufferLen = SendMessage (hOutWnd, WM_GETTEXTLENGTH, 0, 0L);
    numLines = SendMessage (hOutWnd, EM_GETLINECOUNT, 0, 0L);
    firstVis = SendMessage (hOutWnd, EM_GETFIRSTVISIBLELINE, 0, 0L);
    lResult = SendMessage (hOutWnd, EM_SETSEL, bufferLen, bufferLen);

    // Write the new text.
    lResult = SendMessage (hOutWnd, EM_REPLACESEL, 0, lParam);

    // See whether scrolling is necessary.
    if (numLines > (firstVis + 1))
    {
        int        lineLen = 0;
        int        lineCount = 0;
        int        charPos;

        // Find the last nonblank line.
        numLines--;
        while(!lineLen)
        {
            charPos = SendMessage(
                hOutWnd, EM_LINEINDEX, (WPARAM)numLines, 0L);
            lineLen = SendMessage(
                hOutWnd, EM_LINELENGTH, charPos, 0L);
            if(!lineLen)
                numLines--;
        }
        // Prevent negative value finding min.
        lineCount = numLines - firstVis;
        lineCount = (lineCount >= 0) ? lineCount : 0;
        
        // Scroll the window.
        lResult = SendMessage(
            hOutWnd, EM_LINESCROLL, 0, (LPARAM)lineCount);
    }

    // Done, make read-only and allow redraw.
    lResult = SendMessage(hOutWnd, WM_SETREDRAW, TRUE, 0L);
    lResult = SendMessage(hOutWnd, EM_SETREADONLY, TRUE, 0L);
}  

//
// ErrorHandler
//
void ErrorHandler(
   LPTSTR lpszFunction
) 
// Routine Description:
//     Support routine.
//     Retrieve the system error message for the last-error code
//     and pop a modal alert box with usable info.

// Parameters:
//     lpszFunction - String containing the function name where 
//     the error occurred plus any other relevant data you'd 
//     like to appear in the output. 

// Return Value:
//     None

// Note:
//     This routine is independent of the other windowing routines
//     in this application and can be used in a regular console
//     application without modification.
{ 

  //  LPVOID lpMsgBuf;
  //  LPVOID lpDisplayBuf;
  //  DWORD dw = GetLastError(); 

  //  FormatMessage(
  //      FORMAT_MESSAGE_ALLOCATE_BUFFER | 
  //      FORMAT_MESSAGE_FROM_SYSTEM |
  //      FORMAT_MESSAGE_IGNORE_INSERTS,
  //      NULL,
  //      dw,
  //      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
  //      (LPTSTR) &lpMsgBuf,
  //      0, NULL );

  //  // Display the error message and exit the process.

  //  lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
  //      (lstrlen((LPCTSTR)lpMsgBuf)
  //                + lstrlen((LPCTSTR)lpszFunction)+40)
  //                * sizeof(TCHAR)); 
  //  StringCchPrintf((LPTSTR)lpDisplayBuf, 
  //      LocalSize(lpDisplayBuf) / sizeof(TCHAR),
  //      TEXT("%s failed with error %d: %s"), 
  //      lpszFunction, dw, lpMsgBuf); 
  ////  MessageBox(NULL, (LPCTSTR)lpDisplayBuf, g_pszAppName, MB_OK); 

  //  LocalFree(lpMsgBuf);
  //  LocalFree(lpDisplayBuf);
}