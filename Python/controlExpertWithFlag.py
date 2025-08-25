import win32con, win32api, win32gui, win32ui
import ctypes, ctypes.wintypes
import subprocess, time,os
import datetime

# Modifies a flag in Expert which can be used initiate an experiment.

# Defines data structure used to pass information between Python and Prospa
class COPYDATASTRUCT(ctypes.Structure):
    _fields_ = [
        ('dwData', ctypes.wintypes.LPARAM),   # User define parameter
        ('cbData', ctypes.wintypes.DWORD),    # Size of string
        ('lpData', ctypes.c_char_p)           # String containing data
    ]
PCOPYDATASTRUCT = ctypes.POINTER(COPYDATASTRUCT)

# A  class allowing communications between Python and Prospa
class Comms:

   # Defines an invisible win32 window to receive and send messages
    def __init__(self, dstWin):
        message_map = {win32con.WM_COPYDATA: self.OnUser}
        wc = win32gui.WNDCLASS()
        wc.lpfnWndProc = message_map
        wc.lpszClassName = 'MyWindowClass'
        hinst = wc.hInstance = win32api.GetModuleHandle(None)
        classAtom = win32gui.RegisterClass(wc)
        self.hwnd = win32gui.CreateWindow (classAtom,"Python win32gui",0,0,0,
                    win32con.CW_USEDEFAULT,win32con.CW_USEDEFAULT,0,0,hinst,None)
        self.returnMessage = 'init'
        self.prospaWin = dstWin

   # Detect a message coming back from Prospa
    def OnUser(self, hwnd, msg, wparam, lparam):
        pCDS = ctypes.cast(lparam, PCOPYDATASTRUCT)
        self.returnMessage = ctypes.string_at(pCDS.contents.lpData)
        return 1

   # Run a macro or command in Prospa by sending the text as a message
    def RunProspaMacro(self, macro_str):
        pywnd = win32ui.CreateWindowFromHandle(self.prospaWin)
        cds = COPYDATASTRUCT()
        cds.dwData = 1
        cds.cbData = ctypes.sizeof(ctypes.create_string_buffer(macro_str))
        cds.lpData = ctypes.c_char_p(macro_str)
        lParam = PCOPYDATASTRUCT.from_address(ctypes.addressof(cds))
        pywnd.SendMessage(win32con.WM_COPYDATA, self.hwnd, lParam)

prospaWin = win32gui.FindWindowEx(0, 0, 'MAIN_PROSPAWIN', None)

com = Comms(prospaWin)

# Set gView->pytest = 0 in Expert

now = datetime.datetime.now()
com.RunProspaMacro(b'gView->pytest = 0')
print(now)




