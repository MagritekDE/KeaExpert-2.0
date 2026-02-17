# Demonstrates how to start KeaExpert and then run different experiments or commands
# via Windows message passing. Tested usinng a Python environment installing
# packages: matplotlib, numpy and pywin32
# Note that KeaExpert must be able to run the desired experiments with just
# the default and common parameters! see makePyEnv.bat
# Tested with Python 3.12.3
import win32con, win32api, win32gui, win32ui
import ctypes, ctypes.wintypes
import subprocess,time
import matplotlib.pyplot as plt
import os
import numpy as np

# Defines data structure used to pass information between Python and Prospa
class COPYDATASTRUCT(ctypes.Structure):
    _fields_ = [
    ('dwData', ctypes.wintypes.LPARAM), # User define parameter
    ('cbData', ctypes.wintypes.DWORD), # Size of string
    ('lpData', ctypes.c_char_p) # String containing data
    ]
PCOPYDATASTRUCT = ctypes.POINTER(COPYDATASTRUCT)

# A class allowing communications between Python and Prospa
class Comms:

    # Defines an invisible win32 window to receive and send messages
    def __init__(self, dstWin):
        message_map = {
        win32con.WM_COPYDATA: self.OnUser
        }
        wc = win32gui.WNDCLASS()
        wc.lpfnWndProc = message_map
        wc.lpszClassName = 'MyWindowClass'
        hinst = wc.hInstance = win32api.GetModuleHandle(None)
        classAtom = win32gui.RegisterClass(wc)
        self.hwnd = win32gui.CreateWindow (
        classAtom,
        "win32gui test",
        0,
        0,
        0,
        win32con.CW_USEDEFAULT,
        win32con.CW_USEDEFAULT,
        0,
        0,
        hinst,
        None
        )
        self.returnMessage = 'init'
        self.prospaWin = dstWin

    # Detect a message coming back from Prospa
    def OnUser(self, hwnd, msg, wparam, lparam):
        pCDS = ctypes.cast(lparam, PCOPYDATASTRUCT)
        # print (pCDS.contents.dwData)
        # print (pCDS.contents.cbData)
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

# Load the .1d file identified by 'fileName'
def LoadFile(fileName):
    if os.path.isfile(fileName) is False:
        return None, None
    f = open(fileName, "rb")
    owner = f.read(4)
    if owner != b"SORP":
        raise Exception("Not a Prospa file")
    format = f.read(4)
    if format != b"ATAD":
        raise Exception("Not a Prospa data file")
    version = f.read(4)
    if version != b"1.1V":
        raise Exception("Not a Prospa data V1.1 file")
    typeNr = np.fromfile(f, dtype=np.int32, count=1)
    if typeNr != 504:
        raise Exception("Not a complex 1D data file")
    width = np.fromfile(f, dtype=np.int32, count=1)
    height = np.fromfile(f, dtype=np.int32, count=1)
    depth = np.fromfile(f, dtype=np.int32, count=1)
    hyper = np.fromfile(f, dtype=np.int32, count=1)
    if height > 1 or depth > 1 or hyper > 1:
        raise Exception("Not a 1D data file")
    x = np.fromfile(f, dtype=np.float32, count=width[0])
    y = np.fromfile(f, dtype=np.complex64, count=width[0])
    return x, y

# Plot the (x,y) data into 'fig' at with size row x col in suplot idx with x-zoom and title
def PlotData(fig,row,col,idx,x,y,zoom,title):
    ax = fig.add_subplot(row,col,idx)
    ax.plot(x, y)
    plt.title(title)
    if(zoom != None):
        plt.xlim(zoom[0],zoom[1])
    fig.canvas.draw()
    fig.show()
    plt.pause(0.05)

# Open KeaExpert - needs V2.0 or later
# Prospa executable and KeaExpert macro location - change as required
prospa = 'C:\\Users\\Craig\\Projects\\Expert-Kea  2.0 DSP-FX3 - V143 MASTER\\prospa.exe'
startMacro = 'C:\\Users\\Craig\\Projects\\Expert-Kea  2.0 DSP-FX3 - V143 MASTER\\Macros\\Kea-Expert\\KeaExpertInterface.pex'
# Start Prospa ('hidden' for hidden 'normal' for visible). Give it time to load
subprocess.Popen([prospa, startMacro,'"normal"'])
time.sleep(4)
# Find the Prospa window
prospaWin = win32gui.FindWindowEx(0, 0, 'MAIN_PROSPAWIN', None)

# Make a comms class object to communicate with prospa
com = Comms(prospaWin)

print("Experiments started\n")
# Load the first experiment in to the interface
com.RunProspaMacro(b'CPMGAdd(["nrScans = 64","echoTime=150","nrPnts=64"])')
# Change the current file comment
com.RunProspaMacro(b'gView->sampleNameCtrl->text("Test")')
# Run the first experiment
com.RunProspaMacro(b'gExpt->runExperiment()')
#Get data location
dir = com.returnMessage
os.chdir(dir)
# Load the data
(xa, ya) = LoadFile('rawCPMGAdd.1d')
# Plot the result
fig = plt.figure(1)
PlotData(fig,2,1,1,xa,ya,None,'CPMGAdd')

# Load and run the second experiment
com.RunProspaMacro(b'CPMGFast(["nrScans = 128","echoTime=100","nrPnts=4","dwellTime=0.5","nrEchoes=300","fitMode=\\"realTime\\"", "fitType=\\"exp\\""])')
com.RunProspaMacro(b'gExpt->runExperiment()')
dir = com.returnMessage
os.chdir(dir)
(xa, ya) = LoadFile('rawCPMG.1d')
PlotData(fig,2,1,2,xa,ya,None,'CPMGFast')

print("Experiments finished\n")

# Close Expert after 2 seconds
com.RunProspaMacro(b'showwindow(0)')
time.sleep(2)
com.RunProspaMacro(b'exit(1)')

#  Keep plots
plt.show()

