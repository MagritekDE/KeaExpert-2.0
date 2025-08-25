/************************************************************************
* Interface to Magritek DSP USB 2 board for 32/64 bit Windows PCs
* using the WinUSB driver and running Prospa V3.xx
*
* dspread ............... read data from DSP
* dspwrite .............. write data to DSP
* dspreadpp ............. read a pulse program from a file and store in vector
* dsprunpp .............. run a pulse program in the DSP
* dsprunreptimepp ....... run a p.p. with a defined repetition time
* dsprunietimepp ........ run a p.p. with a defined inter-experiment time
* dspdllversion ......... return the current DLL version number.
* dspfwversion .......... get the current firmware version number.
* helpfolder ............ return help folder location
* dsprunmspp ............ run fast multiscan experiment
* dspsetmspar ........... set fast multiscan parameter block (small)
* dspgetmsdata .......... get result of fast multiscan experiment (<= 256 words)

* Version history
*
* 1.0 Initial version modified from V2.11 of 32 bit version
* 1.1 Updated to support the "fast" multiscan commands, tidied up file
* 1.2 Updated GetDeviceHandle to support multiple USB ports
* 1.3 Added the command DSP_RunTimedPulseProgram which calls firmware which includes a
*     includes a repetition time timer before starting each scan.
*     Added the command DSP_RunInterExpTimedPulseProgram which calls firmware which 
*     includes a delay between experiments.
*     Added the command dspdllversion to return the DLL version number.
*     Added the command dspfwverson to return the DSP firmware number.
* 1.4 Modified dspfwversion to return DSP firmware type and version number.
* 1.5 Put addition error checking in for USB read/write commands.
* 1.6 Removed global variables and made threadFlag an array.
*     Added usbPort value check.
*     Added 200 us delay in dspwrite command to give time for DSP to accept data.
* 1.7 Added dspusbreset command and dspdebug
* 1.8 Restored 200 us delay and removed timeout limits
* 1.9 Added code in Read and WriteData to ignore timeout error on control transfer
*
* Last modified 18/5/18 CDE
*
*************************************************************************/ 


#define VERSION 1.9

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <initguid.h>
#include <process.h>
#include <time.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "usb.h"
#include "winusbio.h"
#include "winusb.h"
#include "usb100.h"
#include <initguid.h>
#include "setupapi.h"
#include <pshpack1.h> 
#include <time.h>

// Linked libraries
#pragma comment (lib , "setupapi.lib" )
#pragma comment (lib , "winusb.lib" )

#include "../Global files/includesDLL.h"

#define NO_MEM 0x00
#define P_MEM  0x10
#define X_MEM  0x20
#define Y_MEM  0x30

DEFINE_GUID(USBIODS_GUID, 0x77f49320, 0x16ef, 0x11d2, 0xad, 0x51, 0x0, 0x60, 0x97, 0xb5, 0x14, 0xdd);

// Global structures

typedef struct _IO_REQUEST
{
   unsigned short uAddressL;
   unsigned char bAddressH;
   unsigned short uSize;
   unsigned char bCommand;
}IO_REQUEST, *PIO_REQUEST;

typedef struct
{
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   long size;
   UCHAR pID;
   int usbPort;
} ButtonPar;

typedef struct 
{
    UCHAR  PipeInId;
    UCHAR  PipeOutId;
}PIPE_ID;

// Global constants 
static const GUID OSR_DEVICE_INTERFACE = // Constant for {b35924d6-3e16-4a9e-9782-5524a4b79bac}
{ 0xb35924d6, 0x3e16, 0x4a9e, { 0x97, 0x82, 0x55, 0x24, 0xa4, 0xb7, 0x9b, 0xac } };
static const long bufferSize = 768;                // Buffer size for data transfer

// Global variables
int const MAX_PORTS = 10;
int gInterfaceNumber = 0;                   // Current USB port - used if not passed to function
short threadflag[MAX_PORTS]; // Global thread flag list - one for each USB port

// Locally defined procedure called via command interface
int DSP_Read(DLLParameters*,char[]); 
int DSP_ReadPulseProgram(DLLParameters*,char[]);
int DSP_RunPulseProgram(DLLParameters*,char[]);
int DSP_RunRepTimedPulseProgram(DLLParameters* par,char arg[]) ;
int DSP_RunInterExpTimedPulseProgram(DLLParameters* par,char arg[]) ;
int DSP_AbortPulseProgram(DLLParameters*,char[]);
int DSP_Write(DLLParameters*,char[]); 
int DSP_SetUSBPort(DLLParameters*,char arg[]);
int DSP_DLL_Version(DLLParameters*,char *args);
int DSP_HelpFolder(DLLParameters*,char *args);
int DSP_SetMultiScanParameterList(DLLParameters*,char *args);
int DSP_GetMultiScanData(DLLParameters*,char *args);
int DSP_RunMultiScanPulseProgram(DLLParameters*,char *args);
int DSP_Firmware_Version(DLLParameters*,char *args);
int DSP_USBReset(DLLParameters*,char *args);
int DSP_Debug(DLLParameters*,char *args);

// Helper procedures to the above
short WriteData(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pID, unsigned long, unsigned char, short, unsigned char*);
unsigned char  *ReadData(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pID, unsigned long, unsigned char, short, unsigned char*);
void CheckForDSPEvent(PVOID);
short WaitForDSPResponse(WINUSB_INTERFACE_HANDLE hWinUSBHandle);
long GetFileLength(FILE *fp);
short WriteFastData(unsigned char *data, long size, long usbPort);
short RunPulseProgram(WINUSB_INTERFACE_HANDLE hWinUSBHandle, unsigned long, short flag, bool ignoreError = FALSE);
bool ReportDevices(void);
long nint(float);
double GetMsTime();
void Wait(double tm);

// Low level procedures relating to WinUSB
BOOL GetDeviceHandle (GUID guidDeviceInterface, PHANDLE hDeviceHandle);
BOOL GetWinUSBHandle(HANDLE hDeviceHandle, PWINUSB_INTERFACE_HANDLE phWinUSBHandle);
BOOL GetUSBDeviceSpeed(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pDeviceSpeed);
BOOL QueryDeviceEndpoints (WINUSB_INTERFACE_HANDLE hWinUSBHandle, PIPE_ID* pipeid, PIPE_ID* intpipeid);
WINUSB_INTERFACE_HANDLE setupUSB(HANDLE &hDeviceHandle, PIPE_ID &PipeID, PIPE_ID &intPipeID, long usbPort);
short AbortPP(WINUSB_INTERFACE_HANDLE hWinUSBHandle, char *idata);
int GetData(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pID, unsigned char *data);

int USB_Open(DLLParameters*, char[]);

// Locally defined procedures which are accessible from Prospa
EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);

bool GetBufferSize(WINUSB_INTERFACE_HANDLE hWinUSBHandle, unsigned char *idata)	;

/*******************************************************************************
   Extension procedure to add commands to Prospa  
*******************************************************************************/

EXPORT short AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

 //  if(!stricmp(command,"dspreset"))               r = DSP_AbortPulseProgram(dpar,parameters);
   if(!stricmp(command,"dspdllversion"))          r = DSP_DLL_Version(dpar,parameters);
   else if(!stricmp(command,"dspfwversion"))      r = DSP_Firmware_Version(dpar,parameters);
   else if(!stricmp(command,"dspdebug"))          r = DSP_Debug(dpar,parameters);
   else if(!stricmp(command,"dspusbreset"))       r = DSP_USBReset(dpar,parameters);
   else if(!stricmp(command,"dspgetmsdata"))      r = DSP_GetMultiScanData(dpar,parameters);
   else if(!stricmp(command,"dspreadpp"))         r = DSP_ReadPulseProgram(dpar,parameters);
   else if(!stricmp(command,"dsprunietimepp"))    r = DSP_RunInterExpTimedPulseProgram(dpar,parameters);
   else if(!stricmp(command,"dsprunmspp"))        r = DSP_RunMultiScanPulseProgram(dpar,parameters);
   else if(!stricmp(command,"dsprunpp"))          r = DSP_RunPulseProgram(dpar,parameters);
   else if(!stricmp(command,"dsprunreptimepp"))   r = DSP_RunRepTimedPulseProgram(dpar,parameters);
   else if(!stricmp(command,"dspsetmspar"))       r = DSP_SetMultiScanParameterList(dpar,parameters);
   else if(!stricmp(command,"dspsetport"))        r = DSP_SetUSBPort(dpar,parameters);
   else if(!stricmp(command,"dspwrite"))          r = DSP_Write(dpar,parameters);
   else if(!stricmp(command,"helpfolder"))        r = DSP_HelpFolder(dpar,parameters);   
   else if(!stricmp(command,"dspread"))           r = DSP_Read(dpar,parameters);   


   return(r);
}


/*******************************************************************************
   Extension procedure to list commands in DLL  
*******************************************************************************/

EXPORT void ListCommands(void)
{
   TextMessage("\n\n   WinUSB based interface module for Magritek DSP board (V%1.2f)\n\n",VERSION);
   TextMessage("   dspdllversion ..... return DSP DLL version\n");
   TextMessage("   dspfwversion ...... return DSP firmware version\n");
   TextMessage("   dspgetmsdata ...... get a short block of data returned by a multi-scan pulse program\n");
   TextMessage("   dspread ........... read data from the DSP\n");
   TextMessage("   dspreadpp ......... read pulse program from file and store in a vector\n");
   TextMessage("   dsprunietimepp .... run a pulse program with an inter-experiment delay in the DSP\n");
   TextMessage("   dsprunmspp ........ run a multi-scan pulse program\n");
   TextMessage("   dsprunpp .......... run a pulse program in the DSP\n");
   TextMessage("   dsprunreptimepp ... run a pulse program with reptime delay in the DSP\n");
   TextMessage("   dspsetmspar ....... set a short block of parameters for multiscan mode\n");
   TextMessage("   dspsetport ........ choose which DSP port to communicate with (0/1/2/..) \n");
   TextMessage("   dspusbreset ....... reset the USB interface on the spectrometer\n");
   TextMessage("   dspwrite .......... write data to the DSP\n");

}

/*******************************************************************************
   Extension procedure to return syntax in DLL 
*******************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';

        if(!stricmp(cmd,"dspdllversion"))      strcpy(syntax,"FLOAT v = dspdllversion()");
   else if(!stricmp(cmd,"dspfwversion"))       strcpy(syntax,"FLOAT v = dspfwversion()");
   else if(!stricmp(cmd,"dspdebug"))           strcpy(syntax,"FLOAT v = dspdebug()");
   else if(!stricmp(cmd,"dspusbreset"))        strcpy(syntax,"dspusbreset()");
   else if(!stricmp(cmd,"dspgetmsdata"))       strcpy(syntax,"dspgetmsdata(INT points, [INT port])");
   else if(!stricmp(cmd,"dspread"))            strcpy(syntax,"VEC data = dspread(STR memory_type,INT address, INT length, [INT port])");
   else if(!stricmp(cmd,"dspreadpp"))          strcpy(syntax,"VEC pp = dspreadpp(STR filename)");
   else if(!stricmp(cmd,"dsprunietimepp"))     strcpy(syntax,"STR result = dsprunietimepp([INT address [,[INT port]])");
   else if(!stricmp(cmd,"dsprunmspp"))         strcpy(syntax,"dsprunmspp([INT address [, INT port]])");
   else if(!stricmp(cmd,"dsprunpp"))           strcpy(syntax,"STR result = dsprunpp([INT address [[,INT version [,INT port]])");
   else if(!stricmp(cmd,"dsprunreptimepp"))    strcpy(syntax,"STR result = dsprunreptimepp([INT address [,[INT port]])");
   else if(!stricmp(cmd,"dspsetmspar"))        strcpy(syntax,"dspsetmspar(VEC parList, [INT port])");
   else if(!stricmp(cmd,"dspsetport"))         strcpy(syntax,"dspsetport(NUM port)");
   else if(!stricmp(cmd,"dspwrite"))           strcpy(syntax,"dspwrite(STR memory_type, INT address, VEC pp, [INT port])");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}


/*******************************************************************************
   Return the DLL version number 

   ver = dspdllversion()

*******************************************************************************/

int DSP_DLL_Version(DLLParameters* par,char *args)
{
   par->retVar[1].MakeAndSetFloat((long)(VERSION*100+0.5));
   par->nrRetVar = 1;
   return(OK);
}

/*******************************************************************************
  Return the name of the help file (used internally by the help system)

  name = helpfile()

*******************************************************************************/

int DSP_HelpFolder(DLLParameters* par,char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\DSPWinUSB");
   par->nrRetVar = 1;
   return(OK);
}

/***************************************************************************
* Runs the pulse program currently loaded into the DSP memory (dsprunpp)
*
*  This is done using following commands e.g.
*
*   pp = dspreadpp("pgse.p")  # Load pulse program
*   dspwrite("p",0x2000,pp)   # Save at DSP program memory location 0x2000
*   r = dsprunpp(0x2000)      # Run pulse program from location 0x2000
*
* The return parameter r has the following meaning:
* r = 0 ... pulse program completed normally - abort button not pressed.
* r = 1 ... panic button pressed and p.p aborted.
* r = 2 ... abort button pressed but p.p. completed normally
*
****************************************************************************/

int DSP_RunPulseProgram(DLLParameters* par,char arg[]) 
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   UCHAR DeviceSpeed;
   short r;
   bool cls;
   long address = 0x2000;
   short dspVersion = 1;
   long usbPort = -1;

// Get DSP program address and version number   
   if((r = ArgScan(par->itfc,arg,0," [address, version, port]","eee","ldl",&address,&dspVersion,&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }

// Start pulse program **************
	if(RunPulseProgram(hWinUSBHandle,address,0) == ERR)
	{
	   CloseHandle(hDeviceHandle);
		WinUsb_Free(hWinUSBHandle);
	   return(ERR);
	}

// Wait for signal that pp is finished or aborted *************     
// Start thread running to check for the end of the pulse-program *********
   threadflag[usbPort] = 0;
   ButtonPar bp;
   bp.hWinUSBHandle = hWinUSBHandle;
   bp.pID = intPipeID.PipeInId;
   bp.usbPort = usbPort;

   if(dspVersion == 1)
      bp.size = 1024;
   else
      bp.size = 4;

// Start a thread to detect a reset
    _beginthread(CheckForDSPEvent,0,(PVOID)&bp);

// While pulse program is running and we are waiting for it to finish,
// poll for an abort (either the abort button pressed or escape).
   short result;
   short abortSet = 0;
   while(threadflag[usbPort] == 0)
   {
      result = ProcessBackgroundEvents(); // Check for background processes
 
      if(result == 1 || result == -5) // Program should finish and exit gracefully
      {           
         MessageBeep(MB_ICONASTERISK);  
         par->retVar[1].MakeAndSetString("abort"); 
         SetCursor(LoadCursor(NULL,IDC_WAIT));
         abortSet = 1;
      }
      else if(result == 2 && abortSet == 0) // Escape button pressed or panic signal - do a reset
      {
         MessageBeep(MB_ICONASTERISK);  
         SetCursor(LoadCursor(NULL,IDC_WAIT));
      }
      Sleep(10); // Let the rest of the system do something
   }

   CloseHandle(hDeviceHandle);
   WinUsb_Free(hWinUSBHandle);

   if(abortSet == 0)
      par->retVar[1].MakeAndSetString("ok"); 
 
   par->nrRetVar = 1;

   return(OK);
}

/***************************************************************************
* Runs the pulse program currently loaded into the DSP memory (dsprunpp)
* on the first scan a timer is started and the second scan will not begin
* until the timer runs out. The timer is loaded from p[16] in the pulse program.
* This stores the repetition time.
*
*  This is done using following commands e.g.
*
*   pp = dspreadpp("pgse.p")         # Load pulse program
*   dspwrite("p",0x2000,pp)          # Save at DSP program memory location 0x2000
*   r = dsprunreptimedpp(0x2000)     # Run pulse program from location 0x2000
*
* The return parameter r has the following meaning:
* r = 0 ... pulse program completed normally - abort button not pressed.
* r = 1 ... panic button pressed and p.p aborted.
* r = 2 ... abort button pressed but p.p. completed normally
*
****************************************************************************/


int DSP_RunRepTimedPulseProgram(DLLParameters* par,char arg[]) 
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   UCHAR DeviceSpeed;
   short r;
   bool cls;
   long address = 0x2000;
   short dspVersion = 1;
   long usbPort = -1;

// Get DSP program address and version number   
   if((r = ArgScan(par->itfc,arg,0," [address, version, port]","eee","ldl",&address,&dspVersion,&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }

// Start pulse program (note the 2 flag) **************
	if(RunPulseProgram(hWinUSBHandle,address,2) == ERR)
	{
	   CloseHandle(hDeviceHandle);
		WinUsb_Free(hWinUSBHandle);
	   return(ERR);
	}

// Wait for signal that pp is finished or aborted *************     
// Start thread running to check for the end of the pulse-program *********
   threadflag[usbPort] = 0;
   ButtonPar bp;
   bp.hWinUSBHandle = hWinUSBHandle;
   bp.pID = intPipeID.PipeInId;
   bp.usbPort = usbPort;

   if(dspVersion == 1)
      bp.size = 1024;
   else
      bp.size = 4;

// Start a thread to detect a reset
    _beginthread(CheckForDSPEvent,0,(PVOID)&bp);

// While pulse program is running and we are waiting for it to finish,
// poll for an abort (either the abort button pressed or escape).
   short result;
   short abortSet = 0;
   while(threadflag[usbPort] == 0)
   {
      result = ProcessBackgroundEvents(); // Check for background processes
 
      if(result == 2 && abortSet == 0) // Escape button pressed or panic signal - do a reset
      {
       // Abort the runpp command
         result = WinUsb_AbortPipe(hWinUSBHandle, intPipeID.PipeInId);
         CloseHandle(hDeviceHandle);
         WinUsb_Free(hWinUSBHandle);
         unsigned char data[1024];
      //   TextMessage("Sending stop message\n");
       // Send a message to stop the timer
         for(int i = 0; i < 10; i++)
            data[i] = 0;
         if(!WriteFastData(data,1,-1))
         {
            return(ERR);
         }
       // Receive the result of this message *******
         hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
         if(hWinUSBHandle == INVALID_HANDLE_VALUE)
         {
            ErrorMessage("Unable to open device - error %d\n",GetLastError());
	         return(false);	   	
         }
         bp.hWinUSBHandle = hWinUSBHandle;
         bp.pID = intPipeID.PipeInId;
         bp.size = 1024;
         bp.usbPort = usbPort;

     //    TextMessage("Waiting for stop message\n");
         CheckForDSPEvent((PVOID)&bp);
         CloseHandle(hDeviceHandle);
         WinUsb_Free(hWinUSBHandle);

         par->nrRetVar = 1;
         par->retVar[1].MakeAndSetString("abortPP"); 
         return(OK);
      }
      Sleep(10); // Let the rest of the system do something
   }
// When aborting wait here until DSP is finished
   CloseHandle(hDeviceHandle);
   WinUsb_Free(hWinUSBHandle);

   par->nrRetVar = 1;

   if(threadflag[usbPort] == 2)
      par->retVar[1].MakeAndSetString("shortTD"); 

   else if(threadflag[usbPort] == 3)
      par->retVar[1].MakeAndSetString("abortPP"); 

   else if(abortSet == 0)
      par->retVar[1].MakeAndSetString("ok"); 

   return(OK);
}

/***************************************************************************
* Runs the pulse program currently loaded into the DSP memory (dsprunpp)
* after the first scan a timer is started and the second scan will not begin
* until the timer runs out. The timer is loaded from p[16] in the pulse program.
* This stores the inter-experiment delay.
*
*  This is done using following commands e.g.
*
*   pp = dspreadpp("pgse.p")         # Load pulse program
*   dspwrite("p",0x2000,pp)          # Save at DSP program memory location 0x2000
*   r = dspruninterexppp(0x2000)     # Run pulse program from location 0x2000
*
* The return parameter r has the following meaning:
* r = 0 ... pulse program completed normally - abort button not pressed.
* r = 1 ... panic button pressed and p.p aborted.
* r = 2 ... abort button pressed but p.p. completed normally
*
****************************************************************************/


int DSP_RunInterExpTimedPulseProgram(DLLParameters* par,char arg[]) 
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   UCHAR DeviceSpeed;
   short r;
   bool cls;
   long address = 0x2000;
   short dspVersion = 1;
   long usbPort = -1;

// Get DSP program address and version number   
   if((r = ArgScan(par->itfc,arg,0," [address, version, port]","eee","ldl",&address,&dspVersion,&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }

// Start pulse program (note the 4 flag) **************
	if(RunPulseProgram(hWinUSBHandle,address,4) == ERR)
	{
	   CloseHandle(hDeviceHandle);
		WinUsb_Free(hWinUSBHandle);
	   return(ERR);
	}

// Wait for signal that pp is finished or aborted *************     
// Start thread running to check for the end of the pulse-program *********
   threadflag[usbPort] = 0;
   ButtonPar bp;
   bp.hWinUSBHandle = hWinUSBHandle;
   bp.pID = intPipeID.PipeInId;
   bp.usbPort = usbPort;

   if(dspVersion == 1)
      bp.size = 1024;
   else
      bp.size = 4;

// Start a thread to detect a reset
    _beginthread(CheckForDSPEvent,0,(PVOID)&bp);

// While pulse program is running and we are waiting for it to finish,
// poll for an abort (either the abort button pressed or escape).
   short result;
   short abortSet = 0;
   while(threadflag[usbPort] == 0)
   {
      result = ProcessBackgroundEvents(); // Check for background processes
 
      if(result == 2 && abortSet == 0) // Escape button pressed or panic signal - do a reset
      {
       // Abort the runpp command
         result = WinUsb_AbortPipe(hWinUSBHandle, intPipeID.PipeInId);
         CloseHandle(hDeviceHandle);
         WinUsb_Free(hWinUSBHandle);
         unsigned char data[1024];
       //  TextMessage("Sending stop message\n");
       // Send a message to stop the timer
         for(int i = 0; i < 10; i++)
            data[i] = 0;
         if(!WriteFastData(data,1,-1))
         {
            return(ERR);
         }
       // Receive the result of this message *******
         hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
         if(hWinUSBHandle == INVALID_HANDLE_VALUE)
         {
            ErrorMessage("Unable to open device - error %d\n",GetLastError());
	         return(false);	   	
         }
         bp.hWinUSBHandle = hWinUSBHandle;
         bp.pID = intPipeID.PipeInId;
         bp.size = 1024;
         bp.usbPort = usbPort;

       //  TextMessage("Waiting for stop message\n");
         CheckForDSPEvent((PVOID)&bp);
         CloseHandle(hDeviceHandle);
         WinUsb_Free(hWinUSBHandle);
       //  TextMessage("Stop message received\n");

         par->nrRetVar = 1;
         par->retVar[1].MakeAndSetString("abortPP"); 
         return(OK);
      }
      Sleep(10); // Let the rest of the system do something
   }
// When aborting wait here until DSP is finished
   CloseHandle(hDeviceHandle);
   WinUsb_Free(hWinUSBHandle);

   par->nrRetVar = 1;

   if(threadflag[usbPort] == 2)
      par->retVar[1].MakeAndSetString("shortIET"); 

   else if(threadflag[usbPort] == 3)
      par->retVar[1].MakeAndSetString("abortPP"); 

   else if(abortSet == 0)
      par->retVar[1].MakeAndSetString("ok"); 

   return(OK);
}

int DSP_Firmware_Version(DLLParameters* par,char arg[]) 
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   UCHAR DeviceSpeed;
   short r;
   bool cls;
   long address = 0x2000;
   long usbPort = -1;

// Get DSP program address and version number   
   if((r = ArgScan(par->itfc,arg,0," [port]","e","l",&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }

// Start pulse program (note the 3 flag) **************
	if(RunPulseProgram(hWinUSBHandle,address,3) == ERR)
	{
	   CloseHandle(hDeviceHandle);
		WinUsb_Free(hWinUSBHandle);
	   return(ERR);
	}

// Get the version type and number
   ULONG intpack_size = 4;
   char rData[4];
   ULONG nBytes;
   bool result = WinUsb_ReadPipe(hWinUSBHandle, intPipeID.PipeInId, (PUCHAR)rData, intpack_size, &nBytes, 0);

   int extVersion = (int)rData[0];
   int format = (int)rData[1];
   int fwType = (int)rData[2];
   int fwVersion = (int)rData[3];

   if(extVersion != 0xFFFFFFFF) // Original version
   {
      fwType = extVersion;
      fwVersion = 0;
   }
   else // New version 
   {
      if(format != 1)
      {
         ErrorMessage("Unknown DSP firmware version format");
         CloseHandle(hDeviceHandle);
	      WinUsb_Free(hWinUSBHandle);
         return(ERR);
      }
   }

// Close handles
   CloseHandle(hDeviceHandle);
	WinUsb_Free(hWinUSBHandle);

   par->nrRetVar = 2;
   par->retVar[1].MakeAndSetFloat(fwType); 
   par->retVar[2].MakeAndSetFloat(fwVersion); 

   return(OK);
}

int DSP_USBReset(DLLParameters* par,char arg[]) 
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   UCHAR DeviceSpeed;
   short r;
   bool cls;
   long address = 0x2000;
   long usbPort = -1;

// Get DSP program address and version number   
   if((r = ArgScan(par->itfc,arg,0," [port]","e","l",&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }

// Start pulse program (note the 6 flag) **************
	if(RunPulseProgram(hWinUSBHandle,address,6,TRUE) == ERR)
	{
	   CloseHandle(hDeviceHandle);
		WinUsb_Free(hWinUSBHandle);
	   return(ERR);
	}

// Close handles
   CloseHandle(hDeviceHandle);
	WinUsb_Free(hWinUSBHandle);

   par->nrRetVar = 0;

   return(OK);
}

int DSP_Debug(DLLParameters* par,char arg[]) 
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   UCHAR DeviceSpeed;
   short r;
   bool cls;
   long address = 0x2000;
   long usbPort = -1;

// Get DSP program address and version number   
   if((r = ArgScan(par->itfc,arg,0," [port]","e","l",&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }

// Start pulse program (note the 5 flag) **************
	if(RunPulseProgram(hWinUSBHandle,address,5) == ERR)
	{
	   CloseHandle(hDeviceHandle);
		WinUsb_Free(hWinUSBHandle);
	   return(ERR);
	}

// Get the version type and number
   ULONG intpack_size = 4;
   char rData[4];
   ULONG nBytes;
   bool result = WinUsb_ReadPipe(hWinUSBHandle, intPipeID.PipeInId, (PUCHAR)rData, intpack_size, &nBytes, 0);

   int var0 = (int)rData[0];
   int var1 = (int)rData[1];
   int var2 = (int)rData[2];
   int var3 = (int)rData[3];

// Close handles
   CloseHandle(hDeviceHandle);
	WinUsb_Free(hWinUSBHandle);

   par->nrRetVar = 4;
   par->retVar[1].MakeAndSetFloat(var0); 
   par->retVar[2].MakeAndSetFloat(var1); 
   par->retVar[3].MakeAndSetFloat(var2); 
   par->retVar[4].MakeAndSetFloat(var3); 

   return(OK);
}

/*******************************************************************************
   Run this command to stop a pulse program from running
*******************************************************************************/

int DSP_AbortPulseProgram(DLLParameters* par,char arg[]) 
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   char data[10];
   short r;
   long usbPort = -1;

// Get DSP program address and version number   
   if((r = ArgScan(par->itfc,arg,0," [port]","e","l",&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Open device *******************************************
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }
   
// Abort pulse program ************************************
	AbortPP(hWinUSBHandle,data);
	CloseHandle(hDeviceHandle);
   WinUsb_Free(hWinUSBHandle);

   return(OK);
}

/*******************************************************************************
      Stop the current pulse program running  
*******************************************************************************/

short AbortPP(WINUSB_INTERFACE_HANDLE hWinUSBHandle, char *idata)	
{
   long nBytes;
   short result;
   WINUSB_SETUP_PACKET SetupPacket;
   ZeroMemory(&SetupPacket, sizeof(WINUSB_SETUP_PACKET));
   ULONG cbSent = 0;
   idata[0] = 0;
	
   //Create the setup packet
   SetupPacket.RequestType = 0xC0;
   SetupPacket.Request = 0x0C;
   SetupPacket.Value = 0;
   SetupPacket.Index = 0x0475; 
   SetupPacket.Length = 0;

   result = WinUsb_ControlTransfer(hWinUSBHandle, SetupPacket, (PUCHAR)&idata, 0, &cbSent, 0);
   DWORD err = GetLastError();

   return(OK);
}


/*******************************************************************************
   Set the DSP usbPort number
*******************************************************************************/

int DSP_SetUSBPort(DLLParameters* par, char arg[]) 
{
   short r;
   
// Get name of file to convert ***************************
   if((r = ArgScan(par->itfc,arg,0,"portnumber","e","d",&gInterfaceNumber)) < 0)
      return(r);

   if(r == 0)
   {
     // ReportDevices();
      par->retVar[1].MakeAndSetFloat(gInterfaceNumber);
      par->nrRetVar = 1;
   }
   return(OK);
}
   

/*******************************************************************************
   Read a DSP s-record from a .p file and return in a 1D vector variable

   s-records (from Motorola) are a series of lines with the following format

   +-------------------//------------------//-----------------------+
   | type | count | address  |            data           | checksum |
   +-------------------//------------------//-----------------------+

   type -- A char[2] field. These characters describe the type of record 
           (S0, S1, S2, S3, S5, S7, S8, or S9).

           S0 ... header.
           S1 ... 2 byte address + loadable data
           S9 ... 2 byte start address for program


   count -- A char[2] field. These characters when paired and interpreted
            as a hexadecimal value, display the count of remaining character
            pairs in the record.

   address -- A char[4,6, or 8] field. These characters grouped and interpreted 
              as a hexadecimal value, display the address at which the data field
              is to be loaded into memory. The length of the field depends on the 
              number of bytes necessary to hold the address. A 2-byte address
              uses 4 characters, a 3-byte address uses 6 characters, and a 
              4-byte address uses 8 characters.

    data -- A char [0-64] field. These characters when paired and interpreted
            as hexadecimal values represent the memory loadable 
            data or descriptive information.

    checksum -- A char[2] field. These characters when paired and interpreted 
                as a hexadecimal value display the least significant byte
                of the ones complement of the sum of the byte values represented
                by the pairs of characters making up the count, the address, 
                and the data fields.

    Each record is terminated with a line feed. If any additional or different 
    record terminator(s) or delay characters are needed during transmission to
    the target system it is the responsibility of the transmitting program to provide them. 

*******************************************************************************/

// S-Record constants
#define SLEN          515   // (allows for 514 characters - see S-Record description)
#define SNUM_LENGTH     2   // Length of S number (e.g. S0 or S1)
#define CNT_LENGTH      2   // Length of count field (eg. 21)
#define ADDRESS_LENGTH  4   // Length of address field (e.g. 1200)
#define CHECKSUM_LENGTH 2   // Length of check sum field (e.f. 8D)


int DSP_ReadPulseProgram(DLLParameters* par,char arg[]) 
{
   short r;
   CText fileName;
   FILE *fp;
   char *line;
   char *lineBack;
   char sNumber;
   short len;
   char word[10];
   long cnt = 0;
   float result;
   Variable *ans;
   float **mat;

   ans = &par->retVar[1];
   
// Get name of file to convert ***************************
   if((r = ArgScan(par->itfc,arg,1,"fileName","e","t",&fileName)) < 0)
      return(r);

   line = new char[SLEN];  // S-Records should be 78 bytes or less in length but we play safe
   lineBack = line;   

// Open file for binary read *****************************
   if((fp = fopen(fileName.Str(),"rb")) != NULL) 
   {
   	long size = GetFileLength(fp)/2;
      ans->MakeAndLoadMatrix2D(NULL,size,1);
      mat = ans->GetMatrix2D();
      cnt = 0;

      while(!feof(fp))
      {
         line = lineBack;
      // Read in a line from the S-Record file
         fgets(line,SLEN,fp);
         len = strlen(line)-2;
         line[len] = '\0';
         if(line[0] != 'S')
         {
            delete [] line; 
            fclose(fp);
            ErrorMessage("'%s' is not an S-record",fileName);
            return(ERR);
         }
         sNumber = line[1];
         
      // Only S0, S1 and S9 are of interest
         if(sNumber == '0') continue; // Just the header
         if(sNumber == '9') break;    // Last line

       // Reformat the line to exclude the s-number, count and address
         line = line + SNUM_LENGTH + CNT_LENGTH + ADDRESS_LENGTH;
         line[len - SNUM_LENGTH - CNT_LENGTH - ADDRESS_LENGTH - CHECKSUM_LENGTH] = '\0';
         len = strlen(line);
       // Load in 24 bit words from the current line, packing as bytes in
       // string format and then convering into float i.e. '0x4e' -> 78.0
       // Note that only 1-byte is stored per floating point number in the result vector. 
       // Also note that the bytes are read in in reverse order i.e. 012345 becomes 45.0,23.0,1.0
         for(long i = 0; i < len; i+=6)
         {
            word[0] = '0';
            word[1] = 'x';
            word[2] = line[i+4];
            word[3] = line[i+5];
            word[4] = '\0';
            if(isnan(result = StringToFloat(word)))
            {
               delete [] line; 
               ErrorMessage("invalid number in S-record file '%s'",fileName);
               fclose(fp);
               return(ERR);
            }

            unsigned long word1 = nint(result);
            
            word[0] = '0';
            word[1] = 'x';
            word[2] = line[i+2];
            word[3] = line[i+3];
            word[4] = '\0';
            if(isnan(result = StringToFloat(word)))
            {
               delete [] line; 
               ErrorMessage("invalid number in S-record file '%s'",fileName);
               fclose(fp);
               return(ERR);
            }

            unsigned long word2 = nint(result);
            
            word[0] = '0';
            word[1] = 'x';
            word[2] = line[i+0];
            word[3] = line[i+1];
            word[4] = '\0';
            if(isnan(result = StringToFloat(word)))
            {
               delete [] line; 
               ErrorMessage("invalid number in S-record file '%s'",fileName);
               fclose(fp);
               return(ERR);
            } 

            unsigned long word3 = nint(result);

            result = (word3 + (word2<<8) + (word1<<16));

            mat[0][cnt++] = result;
         }
      }
      par->retVar[1].SetDim1(cnt);  
      fclose(fp);    
   }
   else
   {
      delete [] line;
      ErrorMessage("file '%s' not found",fileName);
      return(ERR);
   }
 
   delete [] line;   
   par->nrRetVar = 1;

   return(OK);
}

/*****************************************************************************
  Reads "size" words from word memory location "address" on DSP unit packing *
  result into the &par->retVar[1]. 
*****************************************************************************/

int DSP_Read(DLLParameters* par,char arg[]) 
{
   unsigned long address;
   long n,remainder;
   long i,j;
   short r;
   long size;
   unsigned char *data;
   CText memoryStr; // X or Y memory
   BYTE memory;
   long word1,word2,word3,word4;
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   long usbPort = -1;


// Get length of data string to read *********************  
   if((r = ArgScan(par->itfc,arg,3,"memory, address, size, [port]","eeee","tlll",&memoryStr,&address,&size,&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Allocate memory for data ******************************  
   data = new unsigned char[bufferSize];

// Convert memory argument *******************************
   if(memoryStr[0] == 'x' || memoryStr[0] == 'X')
       memory = (BYTE)X_MEM;
   else if(memoryStr[0] == 'y' || memoryStr[0] == 'Y')
       memory = (BYTE)Y_MEM;
   else if(memoryStr[0] == 'p' || memoryStr[0] == 'P')
       memory = (BYTE)P_MEM; 
   else if(memoryStr[0] == 'n' || memoryStr[0] == 'N')
       memory = (BYTE)NO_MEM;               
   else
   {
      delete [] data;
      ErrorMessage("invalid memory destination (X/Y/P/N are correct options)");
      return(ERR);
   }

// Check address size
   if(address > 0x00FFFFFF)
   {
      delete [] data;
      ErrorMessage("invalid memory address (only 24bit)");
      return(ERR);
   }   
        
// Open device *******************************************
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      delete [] data;
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }
   else // Device successfully opened
   {
    // Set a timeout on the read
    //  ULONG ReadTimeOut = 2000;
    //  WinUsb_SetPipePolicy(hWinUSBHandle, pipeID.PipeInId, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), &ReadTimeOut);		

    // Read chunks of size 'bufferSize' and save in answer matrix
   
      long word;
      par->retVar[1].MakeAndLoadMatrix2D(NULL,size,1);
	   n = nint(size*3/bufferSize);
	   long off = 0;
      for(i = 0; i < n; i++) 
      {
   	   if(!(data = ReadData(hWinUSBHandle, &pipeID.PipeInId, address,memory,bufferSize,data)))
         {
            delete [] data;
			   CloseHandle(hDeviceHandle);
			   WinUsb_Free(hWinUSBHandle);
   	      return(ERR);
         }
	      for(j = 0; j < bufferSize; j+=3)
	      {
	         word1 = data[j+0];
	         word2 = data[j+1];
	         word3 = data[j+2];
	         if(word3 > 0x7f) word4 = 0xFF; else word4 = 0x00;
	         word = word1 + (word2<<8) + (word3<<16) + (word4<<24);
	         VarRealMatrix(&par->retVar[1])[0][j/3 + off] = (float)word;
	      }
   	   address += bufferSize/3;
   	   off += bufferSize/3;
   	}
	   remainder = (size*3)%(bufferSize);
	   
	 // Read remainder (if data size not evenly divisible by 'bufferSize')  	   
      if(remainder > 0) 
      {
   	   if(!(data = ReadData(hWinUSBHandle, &pipeID.PipeInId, address,memory,remainder,data)))
         {
            delete [] data;
			   CloseHandle(hDeviceHandle);
			   WinUsb_Free(hWinUSBHandle);
   	      return(ERR);
         }

	      for(j = 0; j < remainder; j+=3)
	      {
	         word1 = data[j+0];
	         word2 = data[j+1];
	         word3 = data[j+2];
	         if(word3 > 0x7f) word4 = 0xFF; else word4 = 0x00;
	         word = word1 + (word2<<8) + (word3<<16) + (word4<<24);
	         VarRealMatrix(&par->retVar[1])[0][j/3 + off] = (float)word;
	      }
      }	   	
	   CloseHandle(hDeviceHandle);
	   WinUsb_Free(hWinUSBHandle);
   }
   delete [] data;
   par->nrRetVar = 1;
   return(OK);
}


/*******************************************************************************
      Write pulse program parameters to the DSP at specified address 
      (Note this is the reverse of DSP_Read

  dspwrite(memory, address, vector)

*******************************************************************************/

int DSP_Write(DLLParameters* par,char arg[]) 
{
   unsigned long address;
   BYTE memory;
   short r;
   unsigned char *data;
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   UCHAR DeviceSpeed;
   CText matName;
   CText memoryStr;
   Variable result;
   long usbPort = -1;
   

// Get name of matrix to write ***************************   
   if((r = ArgScan(par->itfc,arg,3,"memory,address,matrix,[port]","eece","tltl",&memoryStr,&address,&matName,&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Allocate memory for data ******************************   
   data = new unsigned char[bufferSize];

// Convert memory argument *******************************
   if(memoryStr[0] == 'x' || memoryStr[0] == 'X')
       memory = (BYTE)X_MEM;
   else if(memoryStr[0] == 'y' || memoryStr[0] == 'Y')
       memory = (BYTE)Y_MEM;
   else if(memoryStr[0] == 'p' || memoryStr[0] == 'P')
       memory = (BYTE)P_MEM; 
   else if(memoryStr[0] == 'n' || memoryStr[0] == 'N')
       memory = (BYTE)NO_MEM;              
   else
   {
      delete [] data;
      ErrorMessage("invalid memory destination (X/Y/P/N are correct options)");
      return(ERR);
   }

// Check address size
   if(address > 0x00FFFFFF)
   {
      delete [] data;
      ErrorMessage("invalid memory address (only 24bit)");
      return(ERR);
   }
	
// Open device *******************************************
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      delete [] data;
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
      return(ERR);
   }		
   else  // Device successfully opened
   {
      short type;
      long i,j;
      long size,n,remainder;
      long number;
  
    // Set a timeout on the write
   //   ULONG writeTimeOut = 2000;
    //  WinUsb_SetPipePolicy(hWinUSBHandle, pipeID.PipeOutId, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), &writeTimeOut);		

    // Extract data matrix ********************     
      if((type = Evaluate(par->itfc,NORMAL,matName.Str(),&result)) < 0)
      {
         WinUsb_Free(hWinUSBHandle);
		   CloseHandle(hDeviceHandle);
         delete [] data;
         return(ERR);
      }
	   
	 // Get number of bytes to write ***********	 
	   size = VarColSize(&result)*3;
	   n = nint(size/bufferSize);
	   
	 // Write data ******************************	 
	   if(type == MATRIX2D && VarRowSize(&result) == 1)
      {	
         for(i = 0; i < n; i++) // Write chunks of size 'bufferSize'
         {   		   
	   	   for(j = 0; j < bufferSize; j+=3)
	   	   {
	     		   number = nint(VarRealMatrix(&result)[0][(j + i*bufferSize)/3]);
	     		   data[j+0] = (number & 0xFF0000) >> 16;
	     		   data[j+1] = (number & 0x00FF00) >> 8;
	     		   data[j+2] = (number & 0x0000FF);
	     		}
            if(!WriteData(hWinUSBHandle,&pipeID.PipeOutId,address,memory,bufferSize,data))
            {
               WinUsb_Free(hWinUSBHandle);
				   CloseHandle(hDeviceHandle);
               delete [] data;
               return(ERR);
            }
            address += bufferSize/3;
	      }
	      remainder = size%bufferSize;
	      if(remainder > 0) // Write remainder (if any)
	      {
	   	   for(j = 0; j < remainder; j+=3)
	   	   {
	     		   number = nint(VarRealMatrix(&result)[0][(j + n*bufferSize)/3]);
	     		   data[j+0] = (number & 0xFF0000) >> 16;
	     		   data[j+1] = (number & 0x00FF00) >> 8;
	     		   data[j+2] = (number & 0x0000FF);
	     		}	      
            if(!WriteData(hWinUSBHandle,&pipeID.PipeOutId,address,memory,remainder,data))
            {
                WinUsb_Free(hWinUSBHandle);
				    CloseHandle(hDeviceHandle);
                delete [] data;
                return(ERR);
            }
	      }
	   }
	   else
	   {
         WinUsb_Free(hWinUSBHandle);
			CloseHandle(hDeviceHandle);
         delete[] data;
	      ErrorMessage("invalid data type for save");
	      return(ERR);
	   }
      WinUsb_Free(hWinUSBHandle);
	   CloseHandle(hDeviceHandle);
	}
   delete [] data;
   par->nrRetVar = 0;
	return(OK);
}

/*******************************************************************************
      Start the current pulse program running  
*******************************************************************************/

short RunPulseProgram(WINUSB_INTERFACE_HANDLE hWinUSBHandle, unsigned long address, short flag, bool ignoreError)
{
   ULONG nBytes;
   short result;
   BOOL Result = TRUE;
   IO_REQUEST ioRequest;

   WINUSB_SETUP_PACKET SetupPacket;
   ZeroMemory(&SetupPacket, sizeof(WINUSB_SETUP_PACKET));

   ioRequest.uAddressL = address;	// address 002000
   ioRequest.bAddressH = 0x00;
   ioRequest.uSize = 768;
   ioRequest.bCommand = flag;

   //Create the setup packet
   SetupPacket.RequestType = 0x40;
   SetupPacket.Request = 0x0C;
   SetupPacket.Value = 0;
   SetupPacket.Index = 0x0476;		// run PP command 
   SetupPacket.Length = 6;

   result = WinUsb_ControlTransfer(hWinUSBHandle, SetupPacket, (PUCHAR)&ioRequest, 6, &nBytes, (LPOVERLAPPED)NULL);

   if(result == false && !ignoreError)
   {
      ErrorMessage("Run pulse program request failed!- error %d",GetLastError());
      return(ERR);
   }
   
   return(OK);
}


/*******************************************************************************
 Read 'size' bytes of data from the DSP from specified 'address' 
 and return in 'data'
*******************************************************************************/

#define TIMEOUT 121
unsigned char* ReadData(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pID, unsigned long address, unsigned char dest, short size, unsigned char *data)
{

// Setup I/O request
    IO_REQUEST ioRequest;
    ioRequest.uAddressL = LOWORD(address);      // address on the DSP
    ioRequest.bAddressH = LOBYTE(HIWORD(address));
    ioRequest.uSize = size;             // Number of bytes to read from address
    ioRequest.bCommand = 0x01 + dest;   // read plus memory destination
       
//Create the setup packet
    WINUSB_SETUP_PACKET SetupPacket;
    ZeroMemory(&SetupPacket, sizeof(WINUSB_SETUP_PACKET));
    SetupPacket.RequestType = 0x40;
    SetupPacket.Request = 0x0C;
    SetupPacket.Value = 0;
    SetupPacket.Index = 0x0471;		// set up DMA command 
    SetupPacket.Length = 6;

// Set up the read transfer - ignore timeouts
    ULONG nBytes = 0;
    if(!WinUsb_ControlTransfer(hWinUSBHandle, SetupPacket, (PUCHAR)&ioRequest, 6, &nBytes, (LPOVERLAPPED)NULL))
    {
       DWORD error = GetLastError();
       if (error != TIMEOUT)
       {
          ErrorMessage("USB read setup failed!- error %d", error);
          return(NULL);
       }
    }

// Wait for 200 us- gives DSP time to prepare data
    Wait(0.2);

// Read the data from the DSP
    if(!WinUsb_ReadPipe(hWinUSBHandle, *pID, (PUCHAR)data, size, &nBytes, 0))
    {
       ErrorMessage("USB Read failed!- error %d",GetLastError());
       return(NULL);
    } 
   
    return(data);
}


/*****************************************************************************************
* Write 'size' bytes in 'data' to 'address' of memory 'dest' in 'hdevice'
*****************************************************************************************/

short WriteData(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pID, unsigned long address, unsigned char dest, short size,  unsigned char *data)
{
// Setup I/O request
   IO_REQUEST ioRequest;
   ioRequest.uAddressL = LOWORD(address);      // address on the DSP
   ioRequest.bAddressH = LOBYTE(HIWORD(address));
   ioRequest.uSize = size;              // Number of bytes stored at address
   ioRequest.bCommand = 0x00 + dest;    // write plus memory destination
       
//Create the setup packet
   WINUSB_SETUP_PACKET SetupPacket;
   ZeroMemory(&SetupPacket, sizeof(WINUSB_SETUP_PACKET));
   SetupPacket.RequestType = 0x40;
   SetupPacket.Request = 0x0C;
   SetupPacket.Value = 0;
   SetupPacket.Index = 0x0471;		// set up DMA request 
   SetupPacket.Length = 6;

// Set up the write transfer - ignore timeouts
   ULONG nBytes;
   if(!WinUsb_ControlTransfer(hWinUSBHandle, SetupPacket, (PUCHAR)&ioRequest, 6, &nBytes, (LPOVERLAPPED)NULL))
   {
      DWORD error = GetLastError();
      if (error != TIMEOUT)
      {
         ErrorMessage("USB write setup failed!- error %d", error);
         return(false);
      }
   }
   
// Change byte order 
   unsigned char byte;
   for(long i = 0; i < size; i+=3)
   {
      byte = data[i+2];
      data[i+2] = data[i];
      data[i] = byte;
   }

// Wait for 200 us- gives DSP time to accept data
   Wait(0.2); 

// Save data to DSP
   if(!WinUsb_WritePipe(hWinUSBHandle, *pID, (PUCHAR)data, size, &nBytes, 0))
   {
      ErrorMessage("USB write failed!- error %d",GetLastError());
      return(false);
   }
      
   return(true);
}


bool GetBufferSize(WINUSB_INTERFACE_HANDLE hWinUSBHandle, unsigned char *idata)	
{
   ULONG nBytes;
   short result;
   const long info_size = 4;
   WINUSB_SETUP_PACKET SetupPacket;
   ZeroMemory(&SetupPacket, sizeof(WINUSB_SETUP_PACKET));
   
	//Create the setup packet
   SetupPacket.RequestType = 0xC0;
   SetupPacket.Request = 0x0C;
   SetupPacket.Value = 0;
   SetupPacket.Index = 0x0474; 
   SetupPacket.Length = 4;

   result = WinUsb_ControlTransfer(hWinUSBHandle, SetupPacket, (PUCHAR)idata, 4, &nBytes, (LPOVERLAPPED)NULL);

   if(result == false)
   {
      TextMessage("\nGet info request failed!- error %d\n",GetLastError());
      return(false);
   }

   return(true);

}

/**************************************************************
   Wait for the DSP board to return a reset from a DEVICE_EVENT.
***************************************************************/

void CheckForDSPEvent(PVOID par)
{
   ULONG nBytes;
   char rData[1024];
   short result;
   long intpack_size = 1024;
   UCHAR pID;

   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   ButtonPar *p;
   
   p = (ButtonPar*)par;
   hWinUSBHandle = p->hWinUSBHandle;
   intpack_size = p->size;
   pID = p->pID;
   rData[0] = -1;
   int usbPort = p->usbPort;

   result = WinUsb_ReadPipe(hWinUSBHandle, pID, (PUCHAR)rData, intpack_size, &nBytes, 0);
   int ret = (int)rData[0];
   if(ret == 1)
      threadflag[usbPort] = 2;
   else if(ret == 2)
      threadflag[usbPort] = 3;
   else
      threadflag[usbPort] = 1;
}


///*******************************************************************************
//   Sit in a loop doing nothing for 2 seconds
//*******************************************************************************/
//
//void ThreadTest(PVOID par)
//{
//   float fdelay = 2.0;
//   unsigned long t1,t2,ldelay;
//
//   t1 = clock();
//   t2 = CLOCKS_PER_SEC;
//   ldelay = nint(fdelay*CLOCKS_PER_SEC);
//   
//   while((clock()-t1) < ldelay)
//   {
//      ;
//   }
//   threadflag[usbPort] = 1;
//}


/*******************************************************************************
  Wait for DSP reset button press
*******************************************************************************/

short WaitForDSPResponse(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pID)
{
   ULONG nBytes;
   char rData[10];
   short result;
   const long intpack_size = 4;

   result = WinUsb_ReadPipe(hWinUSBHandle, *pID, (PUCHAR)rData, intpack_size, &nBytes, 0);

   if(result == false)
   {
      TextMessage("read button failed!- error %d\n",GetLastError());
      return(ERR);
   }
   else
   {
      TextMessage("Button pressed! Data is %d\n",(short)rData[0]);
      return((short)rData[0]);
   }   
}

/*******************************************************************************
 Return nearest integer
*******************************************************************************/

long nint(float num)
{
   if(num > 0)
      return((long)(num+0.5));
   else
      return((long)(num-0.5));  
}


/*******************************************************************************
 Return length of file specified by file pointer fp
*******************************************************************************/

long GetFileLength(FILE *fp)
{
	int currentPos, fileLength;

	currentPos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	fileLength = ftell(fp);
	fseek(fp, currentPos, SEEK_SET);
	return(fileLength);
}



/*******************************************************************************
  Run a multi-scan pulse program - only limited data is returned (<= 256 c.p.)
*******************************************************************************/

int DSP_RunMultiScanPulseProgram(DLLParameters* par,char *args)
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   long address = 0x2000;
   long usbPort = -1;
   short r;

// Get parameters ***************************   
   if((r = ArgScan(par->itfc,args,0,"[address,[port]]","ee","ll",&address,&usbPort)) < 0)
      return(r);

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

   // Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }

// Start pulse program **************
	if(RunPulseProgram(hWinUSBHandle,address,1) == ERR)
	{
	   CloseHandle(hDeviceHandle);
	   WinUsb_Free(hWinUSBHandle);
	   return(ERR);
	}
	
	CloseHandle(hDeviceHandle);
	WinUsb_Free(hWinUSBHandle);

   return(OK);
}

int DSP_SetMultiScanParameterList(DLLParameters* par, char *args)
{
   short r;  
   CText matName,devName;
   Variable result;
   unsigned char data[1024];
   long usbPort = -1;

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

// Get name of matrix to write ***************************   
   if((r = ArgScan(par->itfc,args,1,"matrix, [port]","ce","tl",&matName,&usbPort)) < 0)
      return(r);

   short type;
   long i,j;
   long size,n,remainder;
   long number;
   
 // Extract data matrix ********************     
   if((type = Evaluate(par->itfc,NORMAL,matName.Str(),&result)) < 0)
   {
      return(ERR);
   }

 // Get number of bytes to write ***********	 
   long height = VarRowSize(&result);
   long width = VarColSize(&result);
   
 // Write data ******************************	 
   if(type == MATRIX2D && height == 1)
   {	
      for(i = 0, j= 0; i < width; i++,j+=2) 
      {   		   
  		   number = nint(VarRealMatrix(&result)[0][i]);
  		   data[j+0] = (number & 0x00FF00) >> 8;
  		   data[j+1] = (number & 0x0000FF);
     }
     if(!WriteFastData(data,width*2,usbPort))
     {
        return(ERR);
     }
   }
   else
   {
      ErrorMessage("invalid data type for parameter list");
      return(ERR);
   }

   par->nrRetVar = 0;
	return(OK);
}


/*******************************************************************************
  In the process of running a multi-scan sequence.
  Collect the data from the last experiment
  Close connection if finished
*******************************************************************************/


int DSP_GetMultiScanData(DLLParameters* par,char *args)
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   long n;
   long points;
   short r;
   Variable *ansVar = &par->retVar[1];
   unsigned char data[1024];
   long usbPort = -1;

   if(usbPort == -1)
       usbPort = gInterfaceNumber;

   if(usbPort < 0 || usbPort >= MAX_PORTS)
   {
      ErrorMessage("USB port number is out of range (0-9)\n");
	   return(ERR);	   	
   }

   // Get number of scans ***************************
   if((r = ArgScan(par->itfc,args,1,"points,[port]","ee","ll",&points,&usbPort)) < 0)
      return(r);

// We can only transfer 1024 bytes in this mode
   if(points*3 > 1024)
   {
      ErrorMessage("Invalid number of points");
      return(ERR);
   }

// Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(ERR);	   	
   }

	n = GetData(hWinUSBHandle,&intPipeID.PipeInId,data);

   if(n == -1)
   {
      ErrorMessage("can't get data from DSP");
      CloseHandle(hDeviceHandle);
	   WinUsb_Free(hWinUSBHandle);
      return(ERR);
   }

// Load up return matrix with data
   long word,word1,word2,word3,word4;
   ansVar->MakeAndLoadMatrix2D(NULL,points,1);

   for(long k = 0, j = 0; j < points*3; j+=3, k++)
   {
      word1 = data[j+0];
	   word2 = data[j+1];
	   word3 = data[j+2];
	   if(word3 > 0x7f) word4 = 0xFF; else word4 = 0x00;
	   word = word1 + (word2<<8) + (word3<<16) + (word4<<24);
      VarRealMatrix(ansVar)[0][k] = (float)(word);
   }

   par->nrRetVar = 1;
   CloseHandle(hDeviceHandle);
   WinUsb_Free(hWinUSBHandle);

   return(OK);
}



/**************************************************************
   Wait for the DSP board to return data packet
***************************************************************/

int GetData(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pID, unsigned char *data)
{
   ULONG nBytes;
   short result;
   const long intpack_size = 1024;   

   result = WinUsb_ReadPipe(hWinUSBHandle, *pID, (PUCHAR)data, intpack_size, &nBytes, 0);

   if(!result)
      return(-1);

   return(nBytes);
}

/*****************************************************************************************
* Write 'size' bytes in 'data' to 'address' of memory 'dest' in 'hdevice'
*****************************************************************************************/

short WriteFastData(unsigned char *data, long size, long usbPort)
{
   HANDLE hDeviceHandle;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle;
   PIPE_ID pipeID;
   PIPE_ID intPipeID;
   long nBytes;
   short result;
   unsigned char byte;
 

// Change byte order 
   for(long i = 0; i < size; i+=2)
   {
      byte = data[i+1];
      data[i+1] = data[i];
      data[i] = byte;
   }

// Open device *******
   hWinUSBHandle = setupUSB(hDeviceHandle,pipeID,intPipeID,usbPort);
   if(hWinUSBHandle == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("Unable to open device - error %d\n",GetLastError());
	   return(false);	   	
   }

// Save data to DSP
	result = WinUsb_WritePipe(hWinUSBHandle, pipeID.PipeOutId, (PUCHAR)data, size, (PULONG)&nBytes, 0);


   if(result == false)
   {
      CloseHandle(hDeviceHandle);
      WinUsb_Free(hWinUSBHandle);
      ErrorMessage("Write failed!- error %d",GetLastError());
      return(false);
   }

   CloseHandle(hDeviceHandle);
   WinUsb_Free(hWinUSBHandle);
   return(true);
}

BOOL GetDeviceHandle (GUID guidDeviceInterface, PHANDLE hDeviceHandle, long usbPort)
{
    BOOL bResult = TRUE;
    HDEVINFO hDeviceInfo;
    SP_DEVINFO_DATA DeviceInfoData;

    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = NULL;

    ULONG requiredLength=0;

    LPTSTR lpDevicePath = NULL;

    DWORD index = 0;

    // Get information about all the installed devices for the specified
    // device interface class.
    if((hDeviceInfo = SetupDiGetClassDevs( &guidDeviceInterface,NULL, NULL,DIGCF_PRESENT | DIGCF_DEVICEINTERFACE))  == INVALID_HANDLE_VALUE)
    { 
        TextMessage("\n   USB error: SetupDiGetClassDevs: %d.\n", GetLastError());
        goto done;
    }

    //  Enumerate all the device interfaces in the device information set.
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (index = 0; SetupDiEnumDeviceInfo(hDeviceInfo, index, &DeviceInfoData); index++)
    {
      //  DWORD DataT;
      //  LPTSTR buffer = NULL;
      //  DWORD buffersize = 0;

      //  //
      //  // Call function with null to begin with,
      //  // then use the returned buffer size
      //  // to Alloc the buffer. Keep calling until
      //  // success or an unknown failure.
      //  //
      //  while (!SetupDiGetDeviceRegistryProperty(
      //      hDeviceInfo,
      //      &DeviceInfoData,
      //      SPDRP_LOCATION_INFORMATION,
      //      &DataT,
      //      (PBYTE)buffer,
      //      buffersize,
      //      &buffersize))
      //  {
      //      if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      //      {
      //          // Change the buffer size.
      //          if (buffer) LocalFree(buffer);
      //          buffer = (LPTSTR)LocalAlloc(LPTR,buffersize);
      //      }
      //      else
      //      {
      //         goto done;
      //      }
      //  }

      ////  TextMessage("Device %i location is: %s\n",index, buffer);

      //  if (buffer) LocalFree(buffer);
    }


    if (GetLastError()!=NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS)
    {
       TextMessage("n   USB error: SetupDiEnumDeviceInfo: %d.\n", GetLastError());
       goto done;
    }

    long interfaceNumber;
    if(usbPort == -1)
       interfaceNumber = gInterfaceNumber;
    else
       interfaceNumber = usbPort;

 // Check to see if device is present
    if(interfaceNumber + 1 > index)
    {
       // TextMessage("\n   USB error: DSP device '%ld' is not connected",interfaceNumber);
        goto done;
    }

 // Get deviceInterfaceData for device interfaceNumber
    deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);
    if(!SetupDiEnumDeviceInterfaces(hDeviceInfo,NULL,&guidDeviceInterface,interfaceNumber,&deviceInterfaceData))
    {
        if(GetLastError() == ERROR_NO_MORE_ITEMS)
           TextMessage("\n   USB error: No interface since there are no more items found.\n");
        else 
           TextMessage("\n   USB error: No interface unknown reason.\n");
        goto done;
    }

 // Get details about specified device - first get length of structure
     if (!SetupDiGetDeviceInterfaceDetail(hDeviceInfo,&deviceInterfaceData,NULL, 0,&requiredLength,NULL)) 
     {
         if((GetLastError() == ERROR_INSUFFICIENT_BUFFER) && (requiredLength > 0))
         {
             //we got the size, allocate buffer
             if(!(pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, requiredLength)))
             { 
                 TextMessage("\n   USB error: allocating memory for the device detail buffer.\n");
                 goto done;
             }
         }
         else
         {
             TextMessage("\n   USB error: SetupDiEnumDeviceInterfaces: %d.\n", GetLastError());
             goto done;
         }
     }

   // Get the interface detailed data
     pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

   // Now call it with the correct size and allocated buffer
     if(!SetupDiGetDeviceInterfaceDetail(hDeviceInfo,&deviceInterfaceData,pInterfaceDetailData,requiredLength,NULL,&DeviceInfoData))
     {
         ErrorMessage("SetupDiGetDeviceInterfaceDetail: %d.\n", GetLastError());
         goto done;
     }

   // Copy device path
     size_t nLength = strlen (pInterfaceDetailData->DevicePath) + 1;  
     lpDevicePath = (TCHAR *) LocalAlloc (LPTR, nLength * sizeof(TCHAR));
     StringCchCopy(lpDevicePath, nLength, pInterfaceDetailData->DevicePath);
     lpDevicePath[nLength-1] = 0;
                     
  //   TextMessage("Device path:  %s\n", lpDevicePath);

    if (!lpDevicePath)
    {
        TextMessage("\n   USB error: %d.", GetLastError());
        goto done;
    }

  // Open the device using returned path
    if((*hDeviceHandle = CreateFile(lpDevicePath,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL)) == INVALID_HANDLE_VALUE)
    {
        TextMessage("\n   USB error: %d.", GetLastError());
    }

  // Tidy up
done:
    LocalFree(lpDevicePath);
    LocalFree(pInterfaceDetailData);    
    bResult = SetupDiDestroyDeviceInfoList(hDeviceInfo);
 
    return(bResult);
}


BOOL GetWinUSBHandle(HANDLE hDeviceHandle, PWINUSB_INTERFACE_HANDLE phWinUSBHandle)
{
    if (hDeviceHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BOOL bResult = WinUsb_Initialize(hDeviceHandle, phWinUSBHandle);
    if(!bResult)
    {
        //Error.
        ErrorMessage("WinUsb_Initialize Error %d.", GetLastError());
        return FALSE;
    }

    return bResult;
}

BOOL GetUSBDeviceSpeed(WINUSB_INTERFACE_HANDLE hWinUSBHandle, UCHAR* pDeviceSpeed)
{
    if (!pDeviceSpeed || hWinUSBHandle==INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BOOL bResult = TRUE;

    ULONG length = sizeof(UCHAR);

    bResult = WinUsb_QueryDeviceInformation(hWinUSBHandle, DEVICE_SPEED, &length, pDeviceSpeed);
    if(!bResult)
    {
 //       TextMessage("Error getting device speed: %d.\n", GetLastError());
        goto done;
    }

    if(*pDeviceSpeed == LowSpeed)
    {
 //       TextMessage("Device speed: %d (Low speed).\n", *pDeviceSpeed);
        goto done;
    }
    if(*pDeviceSpeed == FullSpeed)
    {
        TextMessage("Device speed: %d (Full speed).\n", *pDeviceSpeed);
        goto done;
    }
    if(*pDeviceSpeed == HighSpeed)
    {
//       TextMessage("Device speed: %d (High speed).\n", *pDeviceSpeed);
        goto done;
    }

done:
    return bResult;
}

BOOL QueryDeviceEndpoints (WINUSB_INTERFACE_HANDLE hWinUSBHandle, PIPE_ID* pipeid, PIPE_ID* intpipeid)
{
    if (hWinUSBHandle==INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    BOOL bResult = TRUE;

    USB_INTERFACE_DESCRIPTOR InterfaceDescriptor;
	int n = sizeof(USB_INTERFACE_DESCRIPTOR);
    ZeroMemory(&InterfaceDescriptor, n);

    WINUSB_PIPE_INFORMATION  Pipe;
    n = sizeof(WINUSB_PIPE_INFORMATION);
    ZeroMemory(&Pipe, n);
	n = sizeof(USBD_PIPE_TYPE);
	n = sizeof(UCHAR);
	n = sizeof(USHORT);

    
    bResult = WinUsb_QueryInterfaceSettings(hWinUSBHandle, (UCHAR)0, &InterfaceDescriptor);



    if (bResult)
    {
   //     for (int index = 0; index < 1; index++)
        for (int index = 0; index < InterfaceDescriptor.bNumEndpoints; index++)
        {
            bResult = WinUsb_QueryPipe(hWinUSBHandle, (UCHAR)0, (UCHAR)index, &Pipe);

            if (bResult)
            {
                if (Pipe.PipeType == UsbdPipeTypeControl)
                {
     //               TextMessage("Endpoint index: %d Pipe type: Control Pipe ID: %d.\n", index, Pipe.PipeType, Pipe.PipeId);
					//TextMessage("MaximumPacketSize %d\n",Pipe.MaximumPacketSize);
					//TextMessage("Interval %0X\n",Pipe.Interval);
                }
                if (Pipe.PipeType == UsbdPipeTypeIsochronous)
                {
 //                   TextMessage("Endpoint index: %d Pipe type: Isochronous Pipe ID: %d.\n", index, Pipe.PipeType, Pipe.PipeId);
	//				TextMessage("MaximumPacketSize %d\n",Pipe.MaximumPacketSize);
	//				TextMessage("Interval %0X\n",Pipe.Interval);*/
                }
                if (Pipe.PipeType == UsbdPipeTypeBulk)
                {
                    if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
                    {
   /*                     TextMessage("Endpoint index: %d Pipe type: Bulk Pipe ID: %c.\n", index, Pipe.PipeType, Pipe.PipeId);
						TextMessage("MaximumPacketSize %d\n",Pipe.MaximumPacketSize);
						TextMessage("Interval %0X\n",Pipe.Interval);*/
                        pipeid->PipeInId = Pipe.PipeId;
                    }
                    if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
                    {
      //                  TextMessage("Endpoint index: %d Pipe type: Bulk Pipe ID: %c.\n", index, Pipe.PipeType, Pipe.PipeId);
						//TextMessage("MaximumPacketSize %d\n",Pipe.MaximumPacketSize);
						//TextMessage("Interval %0X\n",Pipe.Interval);
                        pipeid->PipeOutId = Pipe.PipeId;
                    }

                }
                if (Pipe.PipeType == UsbdPipeTypeInterrupt)
                {
                    if (USB_ENDPOINT_DIRECTION_IN(Pipe.PipeId))
                    {
      //                  TextMessage("Endpoint index: %d Pipe type: Interrupt Pipe ID: %d.\n", index, Pipe.PipeType, Pipe.PipeId);
						//TextMessage("MaximumPacketSize %d\n",Pipe.MaximumPacketSize);
						//TextMessage("Interval %0X\n",Pipe.Interval);
                        intpipeid->PipeInId = Pipe.PipeId;
                    }
                    if (USB_ENDPOINT_DIRECTION_OUT(Pipe.PipeId))
                    {
      //                  TextMessage("Endpoint index: %d Pipe type: Interrupt Pipe ID: %d.\n", index, Pipe.PipeType, Pipe.PipeId);
						//TextMessage("MaximumPacketSize %d\n",Pipe.MaximumPacketSize);
						//TextMessage("Interval %0X\n",Pipe.Interval);
                        intpipeid->PipeOutId = Pipe.PipeId;
                    }
                }
            }
            else
            {
                continue;
            }
        }
    }
    return bResult;
}

WINUSB_INTERFACE_HANDLE setupUSB(HANDLE &hDeviceHandle, PIPE_ID &PipeID, PIPE_ID &intPipeID, long usbPort)
{
   GUID guidDeviceInterface = OSR_DEVICE_INTERFACE; //in the INF file

   BOOL bResult = TRUE;

   UCHAR DeviceSpeed;


   hDeviceHandle = INVALID_HANDLE_VALUE;
   WINUSB_INTERFACE_HANDLE hWinUSBHandle = INVALID_HANDLE_VALUE;

   bResult = GetDeviceHandle(guidDeviceInterface, &hDeviceHandle, usbPort);
   if(!bResult)
   {
      return(INVALID_HANDLE_VALUE);
   }

   bResult = GetWinUSBHandle(hDeviceHandle, &hWinUSBHandle);
   if(!bResult)
   {
      CloseHandle(hDeviceHandle);
      return(INVALID_HANDLE_VALUE);
   }

   bResult = GetUSBDeviceSpeed(hWinUSBHandle, &DeviceSpeed);
   if(!bResult)
   {
      return(INVALID_HANDLE_VALUE);
   }

   bResult = QueryDeviceEndpoints(hWinUSBHandle, &PipeID, &intPipeID);
   if(!bResult)
   {
      return(INVALID_HANDLE_VALUE);
   }

   return(hWinUSBHandle);
}

bool ReportDevices()
{
    GUID guidDeviceInterface = OSR_DEVICE_INTERFACE; //in the INF file
    HDEVINFO hDeviceInfo;
    long err;
    DWORD index = 0;

// Get information about all the installed devices for the specified
// device interface class.
    if((hDeviceInfo = SetupDiGetClassDevs( &guidDeviceInterface,NULL, NULL,DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)) == INVALID_HANDLE_VALUE) 
    { 
        TextMessage("\n   USB error: SetupDiGetClassDevs: %d.\n", GetLastError());
        goto done;
    }

    TextMessage("\n\n-------- USB DSP port information-------- \n");

// Enumerate all the device interfaces in the device information set.
    SP_DEVINFO_DATA DeviceInfoData;
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (index = 0; ; index++)
    {
        SetLastError(NO_ERROR);
        if(!SetupDiEnumDeviceInfo(hDeviceInfo, index, &DeviceInfoData))
        {
           err = GetLastError();
           break;
        }
        
        DWORD DataT;
        LPTSTR buffer = NULL;
        DWORD buffersize = 0;

        //
        // Call function with null to begin with,
        // then use the returned buffer size
        // to Alloc the buffer. Keep calling until
        // success or an unknown failure.
        //
        while (!SetupDiGetDeviceRegistryProperty(
            hDeviceInfo,
            &DeviceInfoData,
            SPDRP_LOCATION_INFORMATION,
            &DataT,
            (PBYTE)buffer,
            buffersize,
            &buffersize))
        {
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                // Change the buffer size.
                if (buffer) LocalFree(buffer);
                buffer = (LPTSTR)LocalAlloc(LPTR,buffersize);
            }
            else
            {
               goto done;
            }
        }

        TextMessage("\n   Device %i location is: %s",index, buffer);

        if (buffer) LocalFree(buffer);
    }


    if(index > 0)
       TextMessage("\n\n   Current default device is #%d\n",gInterfaceNumber);

    if(err != NO_ERROR && err != ERROR_NO_MORE_ITEMS)
    {
       TextMessage("\n   USB error: SetupDiEnumDeviceInfo: %d.\n", GetLastError());
       goto done;
    }

done:
    SetupDiDestroyDeviceInfoList(hDeviceInfo);

    return(true);
}

/*************************************************************************
*       Return the value of the highres clock in milliseconds
*************************************************************************/

double GetMsTime()
{
	LARGE_INTEGER tick,freq;
	QueryPerformanceCounter(&tick);
	QueryPerformanceFrequency(&freq);
	double time = 1000.0L*(double)tick.QuadPart/(double)freq.QuadPart;
	return(time);
}

/*************************************************************************
*       Wait tm milliseconds
*************************************************************************/

void Wait(double tm)
{
   double tStart = GetMsTime();
   while((GetMsTime()-tStart) < tm);
}
