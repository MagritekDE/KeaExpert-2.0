##########################################################
# Builds a PyQt interface for running Experiments using
# KeaExpert. Experiment parameter files are stored in the
# subfolder 'Experiments'.
# Before using modify the function make sure your KeaExpert
# installation is in the registry by using
# Prospa3.xxUpdateLinks.exe
# your KeaExpert installation location and Kea ID.
##########################################################
import sys
import threading
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
import os
import glob
import win32con, win32api, win32gui, win32ui
import ctypes, ctypes.wintypes
import subprocess,time
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
import numpy as np
import time
import json
import winreg

##########################################################
# Define a class to hold the matplotlib canvas
##########################################################
class Canvas(FigureCanvas):

    def __init__(self, parent=None, width=300, height=400, dpi=100):
        fig, self.axes = plt.subplots(figsize=(width, height), dpi=dpi)
        super().__init__(fig)
        self.setParent(parent)

##########################################################
# Define a Qt UI to control an Expert experiment
##########################################################
class ExpertController(QWidget):

    def __init__(self):
        super().__init__()

        # Specify the window title
        self.setWindowTitle("PyQt Interface to control KeaExpert")

        # Set the window size
        self.resize(1000, 600)

        # Create main layou
        self.mainLayout = QHBoxLayout()

        # Create grid layout
        self.gridLayout = QGridLayout()

        # Create Kea serial number input
        prompt = QLabel('Kea serial number')
        prompt.setAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter)
        self.keaID = QLineEdit()
        self.keaID.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.keaID.setText('Simulator')
        self.gridLayout.addWidget(prompt, 0, 0)
        self.gridLayout.addWidget(self.keaID, 0, 1)

        # Create experiment combo box
        exptLabel = QLabel("Experiment:")
        exptLabel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        exptLabel.setAlignment(Qt.AlignmentFlag.AlignRight)
        self.exptCombo = QComboBox()

        # Get the list of available experiments and add to a combobox
        exptList = self.getExperiments()
        self.exptCombo.addItems(exptList)
        self.exptCombo.currentIndexChanged.connect(self.updateUI)

        # Add combobox and label to main layout
        self.gridLayout.addWidget(exptLabel,1,0)
        self.gridLayout.addWidget(self.exptCombo,1,1)

        # Add the remaining controls to main layout
        self.updateUI()

        # Add grid and canvas to mainLayout
        self.mainLayout.addLayout(self.gridLayout)
        self.pltCanvas = Canvas(self)
        self.mainLayout.addWidget(self.pltCanvas)

        # Set layout
        self.setLayout(self.mainLayout)

        # Run a timer to check when the experiment is finished
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.checkExptFinished)
        self.timer.start(100)  # Fires every 100 ms

        # Some initialisers
        self.com = None
        self.exptStatus = "idle"

      #  print(f"Location:",getProspaPath())


    ##########################################################
    # Add a label and a textbox with an initial value and ID
    # to layout. row should be incremented between calls
    ##########################################################
    def addParameterControl(self, layout, ID, label, initValue, row):
        prompt = QLabel(label)
        prompt.setAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter)
        value = QLineEdit()
        value.setProperty("id",ID)
        value.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        value.setText(initValue)
        layout.addWidget(prompt,row,0)
        layout.addWidget(value,row,1)

    ##########################################################
    # Get a list of parameter files from the folder "Experiments"
    ##########################################################
    def getExperiments(self):
        path = os.getcwd()
        os.chdir("Experiments")
        files = glob.glob("*.par")
        files = [os.path.splitext(f)[0] for f in files]
        os.chdir("..")
        return files

    ##########################################################
    # Load the parameters stored in exptName.par as a
    # list of dictionaries
    ##########################################################
    def loadParameters(self, exptName):
        path = os.getcwd()
        os.chdir("Experiments")
        fileName = exptName+".par"
        parameters = []
        if os.path.isfile(fileName):
            fp = open(fileName, 'r')
            lines = fp.readlines()
            fp.close()
            for line in lines:
                line = line.replace('\n', '')
                (id, prompt, init) = line.split(',')
                value = {"ID":id, "Init":init, "Prompt":prompt}
                parameters.append(value)
            os.chdir("..")
            return parameters
        else:
            os.chdir("..")
            return None

    ##########################################################
    # Extract all the parameter IDs and value from the UI
    # and make a parameter list
    ##########################################################
    def getWidgetParameters(self):

        parList = []
        parDict = {}
        for idx in range(self.gridLayout.count()):
            widget = self.gridLayout.itemAt(idx).widget()
            if isinstance(widget, QLineEdit):
                key = str(widget.property("id"))
                parList.append("%s = %s" % (key, widget.text()))
                parDict[key] = widget.text()

        return parList, parDict

    ##########################################################
    # Update the user interface based on the selected
    # experiment
    ##########################################################
    def updateUI(self):

        expt = self.exptCombo.currentText()

        rows = round((self.gridLayout.count())/self.gridLayout.columnCount())

        # Delete current rows in gridLayout except experiment list
        for row in range(2,rows):
            try:
                item = self.gridLayout.itemAtPosition(row,0).widget()
                if item:
                    item.deleteLater()
                item = self.gridLayout.itemAtPosition(row,1).widget()
                if item:
                    item.deleteLater()
            except:
                pass

        # Create the parameter widgets adding them to gridLayout
        self.parameters = self.loadParameters(expt)
        row = 2
        for parameter in self.parameters:
            value = parameter
            ID = value['ID']
            label = value['Prompt']
            initValue = value['Init']
            self.addParameterControl(self.gridLayout, ID, label, initValue, row)
            row += 1

        # Create the run and abort buttons adding to gridLayout
        self.runButton = QPushButton("Run")
        self.runButton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.gridLayout.addWidget(self.runButton,row,0,Qt.AlignmentFlag.AlignHCenter)
        self.abortButton = QPushButton("Abort")
        self.abortButton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.gridLayout.addWidget(self.abortButton,row,1,Qt.AlignmentFlag.AlignHCenter)

        # Define callbacks for buttons
        self.runButton.clicked.connect(self.runExperiment)
        self.abortButton.clicked.connect(self.abortExperiment)

    ##########################################################
    # Check every 100 ms if the experiment is finished.
    # If it has then plot the result
    ##########################################################
    def checkExptFinished(self):

        expt = self.exptCombo.currentText()

        if self.exptStatus == 'finished':
            # Plot the result
            if self.xa is not None:
                exptName = self.exptCombo.currentText()
                self.pltCanvas.axes.clear()
                self.pltCanvas.axes.plot(self.xa, self.ya)
                self.pltCanvas.axes.set(xlabel="Time(us)",ylabel="Amplitude",title=exptName)
                self.pltCanvas.axes.grid()
                self.pltCanvas.draw()
                self.setWindowTitle("PyQt Interface to control KeaExpert - %s acquisition complete" % expt)
            self.runButton.setDisabled(False)
            self.exptStatus = "idle"

    ##########################################################
    # The run button has been pressed - send a message
    # to Expert to start the experiment
    ##########################################################
    def runExperiment(self):

        # Find Expert if already opened (returns 0 if not)
        prospaWin = win32gui.FindWindowEx(0, 0, 'MAIN_PROSPAWIN', None)

        # Start up Expert/Prospa if not already present
        if prospaWin == 0:
            self.startProspa()
            while(prospaWin == 0):
                time.sleep(0.1)
                prospaWin = win32gui.FindWindowEx(0, 0, 'MAIN_PROSPAWIN', None)
            time.sleep(2) # May need increasing

        # Make a comms class object to communicate with prospa
        if self.com == None:
            self.com = Comms(prospaWin)

        keaID = self.keaID.text()
        args = 'SelectKea:selectByID("%s")' % keaID
        self.com.RunProspaMacro(args.encode('utf-8'))

        # Get the experiment name and the parameters for this experiment
        expt = self.exptCombo.currentText()
        parList, parDict = self.getWidgetParameters()

        self.setWindowTitle("PyQt Interface to control KeaExpert - Running %s ..." % expt)

        # Run the experiment in a background thread so we can abort in the foreground
        th = threading.Thread(target=self.runExperimentCore,args=(expt,parList,parDict))
        th.start()

        # Disable the run button so we can't start a new experiment until this one is finished
        self.runButton.setDisabled(True)


        print("Disabled")

    ##########################################################
    # The thread function to run the experiment
    ##########################################################
    def runExperimentCore(self,exptName,parList,parDict):
        # Action when button is clicked
        # Find the Prospa window

        self.xa = None

        print("Experiments started\n")
        # Load the first experiment in to the interface
        args = '%s(%s)' % (exptName,json.dumps(parList)) # Need to get double quoted list for Prospa hence json.dumps() not str()
        self.com.RunProspaMacro(args.encode('utf-8')) # A simple byte string
        # Change the current file comment
        self.com.RunProspaMacro(b'gView->sampleNameCtrl->text("Test")')
        # Run the experiment on expert in the background so it returns immediately
        self.com.RunProspaMacro(b'gExpt->runExperiment(0,"bg")')
        print("Running")

        self.exptStatus = "running"

        # Get data location
        dataDir = self.com.returnMessage

        # Wait for the experiment to finish
        while(1):
            time.sleep(0.1)
            self.com.RunProspaMacro(b'gExpt->checkStatus()')
            if self.com.returnMessage == b'idle':
                break
            elif self.com.returnMessage == b'abort':
                print("Experiment aborted 1")
                self.exptStatus = "idle"
                return
            elif self.exptStatus == "abort":
                self.com.RunProspaMacro(b'gExpt->abortExperiment')
                print("Experiment aborted 2")
                self.exptStatus = "idle"
                return

        print("Experiment finished")

        # Move into the Expert data directory
        path = os.getcwd()
        os.chdir(dataDir)
        # Load the data
        (self.xa, self.ya) = LoadFile(parDict['fileName'])
        os.chdir(path)
        self.exptStatus = "finished"

    ##########################################################
    # The abort button has been pressed - send a message to
    # SpinsolveExpert to abort the current experiment
    ##########################################################
    def abortExperiment(self):
        # Action when button is clicked
        print("Aborting")
        self.exptStatus = "abort"
        self.runButton.setDisabled(False)

    ##########################################################
    # Run KeaExpert (Prospa)
    # This requires modification for the location of Prospa
    # and KeaExpert and also the specID to use
    ##########################################################
    def startProspa(self):

        print("Starting prospa")
        keaID = self.keaID.text()
        prospa = getProspaPath()
        startMacro = os.path.normpath(prospa + '\\..\\Macros\\Kea-Expert\\KeaExpertInterface.pex')
       # prospa = 'D:\\Projects\\KeaExpert-Dev\\prospa.exe'
       # startMacro = 'D:\\Projects\\KeaExpert-Dev\\Macros\\Kea-Expert\\KeaExpertInterface.pex'
        # Start Prospa ('hidden' for hidden 'normal' for visible). Give it time to load
        # If you want to select a spectrometer use "specID, MGXXXX" where XXXX is your serial nr.
        # Alternatively use "specID,Simulator". Certain experiments work with the simulator.
        subprocess.Popen([prospa, startMacro, '"specID,%s"' % keaID])

##########################################################
# Defines data structure used to pass information
# between Python and Prospa
##########################################################
class COPYDATASTRUCT(ctypes.Structure):
    _fields_ = [
        ('dwData', ctypes.wintypes.LPARAM),  # User define parameter
        ('cbData', ctypes.wintypes.DWORD),  # Size of string
        ('lpData', ctypes.c_char_p)  # String containing data
    ]

PCOPYDATASTRUCT = ctypes.POINTER(COPYDATASTRUCT)


##########################################################
# A class allowing communications between Python and Prospa
##########################################################
class Comms:

    ##########################################################
    # Defines an invisible win32 window to receive and send
    # messages
    ##########################################################
    def __init__(self, dstWin):
        message_map = {
            win32con.WM_COPYDATA: self.OnUser
        }
        wc = win32gui.WNDCLASS()
        wc.lpfnWndProc = message_map
        wc.lpszClassName = 'MyWindowClass'
        hinst = wc.hInstance = win32api.GetModuleHandle(None)
        classAtom = win32gui.RegisterClass(wc)
        self.hwnd = win32gui.CreateWindow(
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

    ##########################################################
    # Detect a message coming back from Prospa
    ##########################################################
    def OnUser(self, hwnd, msg, wparam, lparam):
        pCDS = ctypes.cast(lparam, PCOPYDATASTRUCT)
        # print (pCDS.contents.dwData)
        # print (pCDS.contents.cbData)
        self.returnMessage = ctypes.string_at(pCDS.contents.lpData)
        return 1

    ##########################################################
    # Run a macro or command in Prospa by sending the text
    # as a message
    ##########################################################
    def RunProspaMacro(self, macro_str):
        pywnd = win32ui.CreateWindowFromHandle(self.prospaWin)
        cds = COPYDATASTRUCT()
        cds.dwData = 1
        cds.cbData = ctypes.sizeof(ctypes.create_string_buffer(macro_str))
        cds.lpData = ctypes.c_char_p(macro_str)
        lParam = PCOPYDATASTRUCT.from_address(ctypes.addressof(cds))
        pywnd.SendMessage(win32con.WM_COPYDATA, self.hwnd, lParam)

##########################################################
# Load the .1d file identified by 'fileName'
##########################################################
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
    if typeNr != 503 and typeNr != 504 :
        raise Exception("Not a real or complex 1D data file")

    width = np.fromfile(f, dtype=np.int32, count=1)
    height = np.fromfile(f, dtype=np.int32, count=1)
    depth = np.fromfile(f, dtype=np.int32, count=1)
    hyper = np.fromfile(f, dtype=np.int32, count=1)
    if height > 1 or depth > 1 or hyper > 1:
        raise Exception("Not a 1D data file")
    x = np.fromfile(f, dtype=np.float32, count=width[0])
    if typeNr == 504 :
        y = np.fromfile(f, dtype=np.complex64, count=width[0])
    else:
        y = np.fromfile(f, dtype=np.float32, count=width[0])

    return x, y

##############################################################
# Finds the path to the last installed version of Prospa
# by checking the registry key. Note may not be the one
# you want if multiple Prospas are installed. Use the
# program Prospa3.xxUpdateLinks.exe to change the
# default location in the registry without full reinstallation
###############################################################
def getProspaPath():

    root = winreg.HKEY_CLASSES_ROOT
    sub_key = rf"Applications\Prospa.exe\shell\open\command"

    try:
        with winreg.OpenKey(root, sub_key) as key:
            # The (Default) value contains the full path to the executable ("path" "%1")
            value, _ = winreg.QueryValueEx(key, "")
            executablePath = value.split('"')[1]
            return executablePath
    except FileNotFoundError:
        pass

    return None

##########################################################
# Define a PyQt application and show the Expert controller
##########################################################
if __name__ == '__main__':
    app = QApplication(sys.argv)
    style = QStyleFactory.create("Fusion")
    app.setStyle(style)
    window = ExpertController()
    window.show()
    sys.exit(app.exec())