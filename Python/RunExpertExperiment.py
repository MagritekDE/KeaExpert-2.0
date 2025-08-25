# Demonstrates how to start SpinsolveExpert and then run different experiments or commands
# via Windows message passing. Tested using Python 3.8 with Anaconda
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
    plt.xlim(zoom[0],zoom[1])
    fig.canvas.draw()
    fig.show()
    plt.pause(0.05)

# Open SpinsolveExpert - needs V2.0 or later
# Prospa executable and SpinsolveExpert macro location - change as required
prospa = 'C:\\Users\\Craig\\Projects\\Expert 2.0 DSP-FX3 - V143\\prospa.exe'
startMacro = 'C:\\Users\\Craig\\Projects\\Expert 2.0 DSP-FX3 - V143\\Macros\\Spinsolve-Expert\\SpinsolveExpertInterface.pex'
# Start Prospa ('false' for hidden 'true' for visible). Give it time to load
subprocess.Popen([prospa, startMacro,'"false"'])
time.sleep(4)
# Find the Prospa window
prospaWin = win32gui.FindWindowEx(0, 0, 'MAIN_PROSPAWIN', None)

# Make a comms class object to communicate with prospa
com = Comms(prospaWin)

print("Experiments started\n")
# Load the first experiment in to the interface
com.RunProspaMacro(b'Proton(["nrScans = 1"])')
# Change the current file comment
com.RunProspaMacro(b'gView->sampleNameCtrl->text("Propylbenzoate")')
# Run the first experiment
com.RunProspaMacro(b'gExpt->runExperiment()')
#Get data location
dir = com.returnMessage
os.chdir(dir)
# Load the data
(xa, ya) = LoadFile('data.1d')
fig = plt.figure(1)
# Perform the Fourier transform
delT = (xa[1]-xa[0])/1000
sz = np.size(xa)
faxis = np.linspace(-0.5/delT,0.5/delT,sz)
spectrum = np.fft.fftshift(np.fft.fft(ya))
# Plot the result
PlotData(fig,2,1,1,faxis,spectrum,[-1000,1000],'Proton FID for "Propylbenzoate"')

# Load and run the second experiment
com.RunProspaMacro(b'Carbon(["nrScans = 1"])')
com.RunProspaMacro(b'gExpt->runExperiment()')
dir = com.returnMessage
os.chdir(dir)
(xa, ya) = LoadFile('data.1d')
delT = (xa[1]-xa[0])/1000
sz = np.size(xa)
faxis = np.linspace(-0.5/delT,0.5/delT,sz)
spectrum = np.fft.fftshift(np.fft.fft(ya))
PlotData(fig,2,1,2,faxis,spectrum,[-1000,1000],'Carbon FID for "Propylbenzoate"')

print("Experiments finished\n")

# Close Expert after 2 seconds
com.RunProspaMacro(b'showwindow(0)')
time.sleep(2)
com.RunProspaMacro(b'exit(1)')

# # Keep plots
plt.show()

# Some other commands

# com.RunProspaMacro(b'ReactionMonitoring(["nrSteps = 5","nrScans = 2","ppmRange = [-2,12]", "repTime=2000"])')
# com.RunProspaMacro(b'gExpt->runExperiment()')
# print(com.returnMessage)
#RunProspaMacro(win,b'QuickShim(["peakPositionPPM = 1","startMethod=\\"last\\"","shimMethod=\\"order12\\""])')
#RunProspaMacro(win,b'LockAndCalibrate(["refPeakPPM = 1"])')
