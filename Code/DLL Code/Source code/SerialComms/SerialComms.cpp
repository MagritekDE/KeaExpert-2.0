/**************************************************************************************
  
   Implements basic open/read/write/close commands for the serial (RS232) port.
  
   Version history
  
   3.0 (19-June-15) -----------------------------
  
      1. Added storage for multiple serial connections. 
         All commands now require the port name passed as the first argument
  
   3.1 (2-Oct-15) -----------------------------
  
      1. Extended write command to handle vector inputs for data streams 
         containing zeros.
  
   3.2 (20-Dec-16) -----------------------------
  
      1. Added Synchronous mode option, set with new command serialmode
  
   3.3 (19-Jan-17) -----------------------------
  
      1. Added CRC calculations
      2. Added escapebytes and unescapebytes command for P&G project

	3.4 (30-July-21) -----------------------------

		1. Fixed up the synchronous read to make it more reliable.
		2. Fixed the timeout option in openserial.
		3. Disabled asynchrous options until I have time to check these out.

   3.5 (22-September-21) -----------------------------
	   1. Timeout defaults to infinite
		2. EscapeByteArray corrected: bytes >= FC replaced with FC, FX xor Byte
		3. Aborting serialread message no longer flagged to user as an error

**************************************************************************************/


#include "stdafx.h"
#include "stdlib.h"
#include "../Global files/includesDLL.h"
#include <shellapi.h>

#define VERSION 3.5 // 22.9.21
#define MAXPORTS 50

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);

short OpenSerial(DLLParameters*,char*);
short CloseSerial(DLLParameters*,char*);
short ReadSerial(DLLParameters*,char*);
short WriteSerial(DLLParameters*,char*);
short ReadSerialOverlapped(DLLParameters*,char*);
short WriteSerialOverlapped(DLLParameters*,char*);
short SetOverlappingMode(DLLParameters*,char*);
short GetHelpFolder(DLLParameters*,char*);
HANDLE GetPortHandle(char *portName, short &portNr);
bool CheckPortName(char *portName, short &portNr);
short EndPP(DLLParameters*,char *args);
short FloatsToBytes(DLLParameters* par, char *args);
short BytesToFloats(DLLParameters* par, char *args);
short EscapeByteArray(DLLParameters* par, char *args);
short UnEscapeByteArray(DLLParameters* par, char *args);
short CancelSynchronouseIO(DLLParameters* par, char *args);

short CRCCalc(DLLParameters* par, char *args);
short CRCReset(DLLParameters* par, char *args);

void InsertUniqueStringIntoList(char *str, char ***list, long &position);
long nint(float num);

// An array of port handles (one for COM1, COM2 etc)
HANDLE ports[MAXPORTS];

char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list
long label;     // label counter (to generate unique label)
bool Asychronous = true;


// Extension procedure to add commands to Prospa 

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;
   
   //if(Asychronous)
   //{
   //   if(!strcmp(command,"openserial"))          r = OpenSerial(dpar,parameters);      
   //   else if(!strcmp(command,"closeserial"))    r = CloseSerial(dpar,parameters);      
   //   else if(!strcmp(command,"readserial"))     r = ReadSerialOverlapped(dpar,parameters);      
   //   else if(!strcmp(command,"writeserial"))    r = WriteSerialOverlapped(dpar,parameters); 
   //   else if(!strcmp(command,"serialmode"))     r = SetOverlappingMode(dpar,parameters);      
   //   else if(!strcmp(command,"helpfolder"))     r = GetHelpFolder(dpar,parameters);      
   //}
   //else
   //{
      if(!strcmp(command,"openserial"))          r = OpenSerial(dpar,parameters);      
      else if(!strcmp(command,"closeserial"))    r = CloseSerial(dpar,parameters);      
      else if(!strcmp(command,"readserial"))     r = ReadSerial(dpar,parameters);      
      else if(!strcmp(command,"writeserial"))    r = WriteSerial(dpar,parameters);  
      else if(!strcmp(command,"serialmode"))     r = SetOverlappingMode(dpar,parameters);      
      else if(!strcmp(command,"helpfolder"))     r = GetHelpFolder(dpar,parameters);      
   //}

   if(!strcmp(command,"crccalc"))                r = CRCCalc(dpar,parameters);      
   else if(!strcmp(command,"crcreset"))          r = CRCReset(dpar,parameters);      
   else if(!strcmp(command,"floatstobytes"))     r = FloatsToBytes(dpar,parameters);      
   else if(!strcmp(command,"bytestofloats"))     r = BytesToFloats(dpar,parameters);      
   else if(!strcmp(command,"escapebytes"))       r = EscapeByteArray(dpar,parameters);      
   else if(!strcmp(command,"unescapebytes"))     r = UnEscapeByteArray(dpar,parameters);      
   else if(!strcmp(command,"cancelsyncio"))      r = CancelSynchronouseIO(dpar,parameters);      

 //  else if(!strcmp(cmd,"serialversion"))     strcpy(syntax,"(INT v) = ppversion()");
    
                
   return(r);
}

/*******************************************************************************
   List all available serial commands 
*******************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   Serial Comms DLL module (V%1.2f)\n\n",VERSION);
   TextMessage("   openserial ....... open serial port\n");
   TextMessage("   closeserial ...... close serial port\n");
   TextMessage("   writeserial ...... write to serial port\n");
   TextMessage("   readserial ....... read from serial port\n");
   TextMessage("   serialmode ....... sets synchronous or asynchronous mode\n");
   TextMessage("   crcreset ......... reset the internal CRC value\n");
   TextMessage("   crccalc .......... calculate the new CRC value\n");
   TextMessage("   floatstobytes .... expand an array of floats to component bytes\n");
   TextMessage("   bytestofloats .... pack an array of bytes into floats\n");
   TextMessage("   escapebytes ...... escape bytes >= FD\n");
   TextMessage("   unescapebytes .... remove escape bytes\n");
   TextMessage("   cancelsyncio ..... cancel a pending synchronous serial i/o\n");
}

/*******************************************************************************
   Extension procedure to return syntax in DLL 
*******************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"openserial"))           strcpy(syntax,"openserial([port, baudrate, databits, parity, stop bit, [timeout ms])");
   else if(!strcmp(cmd,"writeserial"))     strcpy(syntax,"writeserial(port,byte_data)");
   else if(!strcmp(cmd,"readserial"))      strcpy(syntax,"data = readserial(port,exit_char/nr_char[,return_type]])");
   else if(!strcmp(cmd,"closeserial"))     strcpy(syntax,"closeserial(port)");
   else if(!strcmp(cmd,"serialmode"))      strcpy(syntax,"serialmode(\"sync\"/\"async\")");
   else if(!strcmp(cmd,"crcreset"))        strcpy(syntax,"crcreset()");
   else if(!strcmp(cmd,"crccalc"))         strcpy(syntax,"result = crccalc(value)");
   else if(!strcmp(cmd,"floatstobytes"))   strcpy(syntax,"result = floatstobytes(array)");
   else if(!strcmp(cmd,"bytestofloats"))   strcpy(syntax,"result = bytestofloats(array)");
   else if(!strcmp(cmd,"escapebytes"))     strcpy(syntax,"result = escapebytes(array)");
   else if(!strcmp(cmd,"unescapebytes"))   strcpy(syntax,"result = unescapebytes(array)");
   else if(!strcmp(cmd,"cancelsyncio"))    strcpy(syntax,"cancelsyncio(threadid)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short GetHelpFolder(DLLParameters* par, char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\Serial");
   par->nrRetVar = 1;
   return(OK);
}


/*******************************************************************************
   Open a connection to the RS232 device 

   openserial(port, baudrate, databits, parity, stop_bit, timeout)
*******************************************************************************/

short SetOverlappingMode(DLLParameters* par, char *args)
{
   CText mode = "async";
   short nrArgs;

// Get the command arguments
   if((nrArgs = ArgScan(par->itfc,args,0,"(sync/async)","e","t", &mode)) < 0)
      return(nrArgs);

   if(mode == "async")
      Asychronous = true;
   else
      Asychronous = false;

   par->nrRetVar = 0;
   return(OK);
}


/*******************************************************************************
   Get the handle for the port
*******************************************************************************/

HANDLE GetPortHandle(char *portName, short &portNr)
{   
   HANDLE hComm;
   if(!CheckPortName(portName, portNr))
      return(NULL);

   if(!(hComm = ports[portNr]))
   {
      ErrorMessage("serial port COM%hd not opened",portNr);
      return(NULL);
   }
   return(hComm);
}


/*******************************************************************************
   Check that the COM port name is valid (COM1 ... COM50)
*******************************************************************************/

bool CheckPortName(char *portName, short &portNr)
{
   if(sscanf(portName,"COM%hd",&portNr) != 1)
   {
      ErrorMessage("invalid port name");
      return(0);
   }
   if(portNr < 0 || portNr >= MAXPORTS)
   {
      ErrorMessage("invalid port number");
      return(0);
   }
   return(1);
}


/*******************************************************************************
   Open a connection to the RS232 device 

   openserial(port, baudrate, databits, parity, stop_bit, timeout)
*******************************************************************************/

short OpenSerial(DLLParameters* par, char *args)
{
   char dir[MAX_PATH];
   short nrArgs;
   static long baudrate = 9600;
   static long bytesize = 8;
   static CText port = "com1";    // One of com1, com2, ...
   static char stopbits[50] = "1";    // One of 1, 1.5, 2
   static char parity[50] = "none";  // One of: none, even, odd, mark, space
   HANDLE hComm;
   short portNr;
   long timeout = 0; // No timeout

// Get the command arguments
   if((nrArgs = ArgScan(par->itfc,args,5,"port, baudrate, databits, parity, stop bit, timeout",
                               "eeeeee","tllssl", &port, &baudrate, &bytesize, parity, stopbits, &timeout)) < 0)
      return(nrArgs);

// Check for valid port name
   port.UpperCase();
   if(!CheckPortName(port.Str(),portNr))
      return(ERR);

// Comm part number >= 10 require a different syntax
   if(portNr >= 10)
   {
      char numStr[20];
      sprintf(numStr,"\\\\.\\COM%d",portNr);
      port = numStr;
   }

   int overlap = 0;
   if(Asychronous)
      overlap = FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING;

// Open the port for read/write
   hComm = CreateFile(port.Str(),  
                     GENERIC_READ | GENERIC_WRITE, 
                     0, 
                     0, 
                     OPEN_EXISTING,
                     overlap,
                     0);

   if(hComm == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("can't open serial port");
      return(ERR);
   }

// Initialise the serial parameters
   DCB dcb = {0};
   dcb.DCBlength = sizeof(DCB);

   if (!GetCommState(hComm, &dcb))
   {
      CloseHandle(hComm);
      ErrorMessage("can't get serial port state");
      return(ERR);
   }

   dcb.BaudRate = baudrate;
   dcb.ByteSize = bytesize;

   if(!strcmp(parity,"none"))  
      dcb.Parity = NOPARITY;
   else if(!strcmp(parity,"even"))  
      dcb.Parity = EVENPARITY;
   else if(!strcmp(parity,"odd"))
      dcb.Parity = ODDPARITY;
   else if(!strcmp(parity,"mark"))
      dcb.Parity = MARKPARITY;
   else if(!strcmp(parity,"space")) 
      dcb.Parity = SPACEPARITY;
   else
   {
      CloseHandle(hComm);
      ErrorMessage("invalid parity");
      return(ERR);
   }

   if(!strcmp(stopbits,"1"))        dcb.StopBits = ONESTOPBIT;
   else if(!strcmp(stopbits,"1.5")) dcb.StopBits = ONE5STOPBITS;
   else if(!strcmp(stopbits,"2"))   dcb.StopBits = TWOSTOPBITS;
   else
   {
      CloseHandle(hComm);
      ErrorMessage("invalid stop bits");
      return(ERR);
   }

	dcb.fAbortOnError = FALSE;

   if(!SetCommState(hComm, &dcb))
   {
      CloseHandle(hComm);
      DWORD err = GetLastError();
      ErrorMessage("invalid serial port state");
      return(ERR);
   }

// Initialize the timeouts to a very large time
   COMMTIMEOUTS cto;
   GetCommTimeouts(hComm, &cto);
	if(timeout == 0)
	{
		cto.ReadIntervalTimeout = 0;
		cto.ReadTotalTimeoutMultiplier = 0;
		cto.ReadTotalTimeoutConstant = 0;
		SetCommTimeouts(hComm, &cto);
	}
	else
	{
		cto.ReadIntervalTimeout = MAXDWORD;
		cto.ReadTotalTimeoutMultiplier = 0;
		cto.ReadTotalTimeoutConstant = timeout;
		SetCommTimeouts(hComm, &cto);
	}

// Set up the buffer sizes
//   SetupComm(hComm,bufferSize,bufferSize);

// Save the port handle
   ports[portNr] = hComm;

   return(OK);
}

/*******************************************************************************
   Close a connection to the RS232 device 

   closeserial(port_name)
*******************************************************************************/

short CloseSerial(DLLParameters* par, char *args)
{
   short nrArgs,portNr;
   CText port;
   HANDLE hComm;

// Get the port name
   if((nrArgs = ArgScan(par->itfc,args,1,"port","e","t", &port)) < 0)
      return(nrArgs);

// Get the assoicated handle
   port.UpperCase();
   if(!(hComm = GetPortHandle(port.Str(),portNr)))
   {
      return(ERR);
   }

// Close it
   CloseHandle(hComm);
   CloseHandle(ports[portNr]);
   ports[portNr] = NULL;

   return(OK);
}

/*******************************************************************************
   Write data to the RS232 device (synchronous)

   writeserial(port_name, string_to_write)
*******************************************************************************/

short WriteSerial(DLLParameters* par, char *args)
{
   DWORD nrWritten;
   unsigned char *data = 0;
   int dataSize;
   Variable dataVar;
   short nrArgs;
   CText portName;
   short portNr;
   HANDLE hComm;

   if((nrArgs = ArgScan(par->itfc,args,2,"port_name, data_to_write","ee","tv", &portName, &dataVar)) < 0)
      return(nrArgs);

// Check for open port
   portName.UpperCase();
   if(!(hComm = GetPortHandle(portName.Str(),portNr)))
   {
      ErrorMessage("Invalid comms port name '%s'",portName.Str());
      CloseHandle(hComm);
      ports[portNr]= NULL;
      return(ERR);
   }

// Write the data
   if(dataVar.GetType() == UNQUOTED_STRING)
   {
      data = (unsigned char*)dataVar.GetString();
      dataSize = strlen((char*)data);
   }
   else if(dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
   {
      dataSize = dataVar.GetDimX();
      float *d = dataVar.GetMatrix2D()[0];
      data = new unsigned char[dataSize];
      for(int i = 0; i < dataSize; i++)
         data[i] = (unsigned char)(int)(d[i]+0.5);
   }

   if(!WriteFile(hComm, data, dataSize, &nrWritten, NULL))
   {
      CloseHandle(hComm);
      ports[portNr]= NULL;
      int err = GetLastError();
      if(err == ERROR_OPERATION_ABORTED)
         ErrorMessage("Serial write aborted\n");
      ErrorMessage("Serial write error %d\n",err);
      if(dataVar.GetType() == MATRIX2D && data)
         delete [] data;
      return(ERR);
   }

   if(dataVar.GetType() == MATRIX2D && data)
   {
      delete [] data;
   }

   return(OK);
}

/*******************************************************************************
   Convert an array of real numbers into the corresponding bytes

   array_of_bytes = floatstobytes(array_of_ints)
*******************************************************************************/

short FloatsToBytes(DLLParameters* par, char *args)
{
   short nrArgs;
   Variable dataVar;
   float **data;
   long dataSize;

   if((nrArgs = ArgScan(par->itfc,args,1,"data_to_convert","e","v", &dataVar)) < 0)
      return(nrArgs);

   if(dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
   {
      dataSize = dataVar.GetDimX();
      float *d = dataVar.GetMatrix2D()[0];
   
      data = MakeMatrix2D(4*dataSize,1);
      float *data1d = data[0];

      for(int i = 0; i < dataSize*4; i+=4)
      {
         unsigned char *value = (unsigned char*)&d[i>>2];
         data1d[i]   = value[0];
         data1d[i+1] = value[1];
         data1d[i+2] = value[2];
         data1d[i+3] = value[3];
      }
   }
   else
   {
      ErrorMessage("Only 1D real vector data supported");
      return(ERR);
   }

// Return the data
   par->retVar[1].AssignMatrix2D(data,4*dataSize,1);
   par->nrRetVar = 1;

   return(OK);
}

/*******************************************************************************
   Convert an array of bytes into the corresponding 32 bit integers (2 bytes

   array_of_ints = bytestofloats(array_of_bytes)
*******************************************************************************/

short BytesToFloats(DLLParameters* par, char *args)
{
   short nrArgs;
   Variable dataVar;
   float **data;
   long dataSize;

   if((nrArgs = ArgScan(par->itfc,args,1,"data_to_convert","e","v", &dataVar)) < 0)
      return(nrArgs);

   if(dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
   {
      dataSize = dataVar.GetDimX();
      if(dataSize%4 != 0)
      {
         ErrorMessage("Input data size must be a multiple of 4");
         return(ERR);
      }
      float *d = dataVar.GetMatrix2D()[0];
   
      data = MakeMatrix2D(dataSize/4,1);
      float *data1d = data[0];

      for(int i = 0; i < dataSize; i+=4)
      {
         unsigned char *value = (unsigned char*)&data1d[i/4];

         int value1 = nint(d[i]);
         if(value1 < 0 || value1 > 255)
         {
            FreeMatrix2D(data);
            ErrorMessage("Element %d (%d) is not a byte",i,value1);
            return(ERR);
         }
         int value2 = nint(d[i+1]);
         if(value2 < 0 || value2 > 255)
         {
            FreeMatrix2D(data);
            ErrorMessage("Element %d (%d) is not a byte",i+1,value2);
            return(ERR);
         }
         int value3 = nint(d[i+2]);
         if(value3 < 0 || value3 > 255)
         {
            FreeMatrix2D(data);
            ErrorMessage("Element %d (%d) is not a byte",i+2,value3);
            return(ERR);
         }
         int value4 = nint(d[i+3]);
         if(value4 < 0 || value4 > 255)
         {
            FreeMatrix2D(data);
            ErrorMessage("Element %d (%d) is not a byte",i+3,value4);
            return(ERR);
         }
         value[0] = (unsigned char)value1;
         value[1] = (unsigned char)value2;
         value[2] = (unsigned char)value3;
         value[3] = (unsigned char)value4;
      }
   }
   else
   {
      ErrorMessage("Only 1D real vector data supported");
      return(ERR);
   }

// Return the data
   par->retVar[1].AssignMatrix2D(data,dataSize/4,1);
   par->nrRetVar = 1;

   return(OK);
}

/*******************************************************************************
   Replace bytes >= FC (SHIFT) with two bytes: (SHIFT, SHIFT xor byte)

   array_of_bytes = escapebytes(array_of_bytes)
*******************************************************************************/

#define SFT 0xFD
#define NIL 0xFC

short EscapeByteArray(DLLParameters* par, char *args)
{
   short nrArgs;
   Variable dataVar;
   float **data;
   long dataSizeIn,dataSizeOut;

   if((nrArgs = ArgScan(par->itfc,args,1,"data_to_code","e","v", &dataVar)) < 0)
      return(nrArgs);

   if(dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
   {
      dataSizeIn = dataVar.GetDimX();
      float *d = dataVar.GetMatrix2D()[0];

   // Count number of bytes to escape
      int cnt = 0;
      for(int i = 0; i < dataSizeIn; i++)
      {
         int value = nint(d[i]);
         if(value < 0 || value > 255)
         {
            ErrorMessage("Element %d (%d) is not a byte",i,value);
            return(ERR);
         }

         BYTE bvalue = (BYTE)value;

         if(bvalue >= NIL)
            cnt++;
      }

   // Allocate space for new array
      dataSizeOut = dataSizeIn + cnt;
      data = MakeMatrix2D(dataSizeOut,1);
      float *data1d = data[0];

   // Escape the data saving to output array
      for(int i = 0, j = 0; i < dataSizeIn; i++)
      {
         BYTE value = (BYTE)nint(d[i]);
         if(value >= NIL)
         {
            data1d[j++] = (float)SFT;
            data1d[j++] = (float)(SFT ^ value);
         }
         else
            data1d[j++] = (float)value;
      }
   }
   else
   {
      ErrorMessage("Only 1D real vector data supported");
      return(ERR);
   }

// Return the data
   par->retVar[1].AssignMatrix2D(data,dataSizeOut,1);
   par->nrRetVar = 1;

   return(OK);
}


/*******************************************************************************
   Replace bytes pairs = (SHIFT XX) with one byte: (SHIFT xor byte)

   array_of_bytes = unescapebytes(array_of_bytes)
*******************************************************************************/


short UnEscapeByteArray(DLLParameters* par, char *args)
{
   short nrArgs;
   Variable dataVar;
   float **data;
   long dataSizeIn,dataSizeOut;

   if((nrArgs = ArgScan(par->itfc,args,1,"data_to_code","e","v", &dataVar)) < 0)
      return(nrArgs);

   if(dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
   {
      dataSizeIn = dataVar.GetDimX();
      float *d = dataVar.GetMatrix2D()[0];

   // Count number of bytes to unescape
      int cnt = 0;
      for(int i = 0; i < dataSizeIn; i++)
      {
         int value = nint(d[i]);
         if(value < 0 || value > 255)
         {
            ErrorMessage("Element %d (%d) is not a byte",i,value);
            return(ERR);
         }

         BYTE bvalue = (BYTE)value;

         if(bvalue == SFT)
         {
            i++;
            cnt++;
         }
      }

   // Allocate space for new array
      dataSizeOut = dataSizeIn - cnt;
      data = MakeMatrix2D(dataSizeOut,1);
      float *data1d = data[0];

   // Escape the data saving to output array
      for(int i = 0, j = 0; i < dataSizeIn; i++)
      {
         BYTE value = (BYTE)nint(d[i]);
         if(value == SFT)
         {
            value = (BYTE)nint(d[i+1]);
            data1d[j++] = (float)(SFT ^ value);
            i++;
         }
         else
            data1d[j++] = (float)value;
      }
   }
   else
   {
      ErrorMessage("Only 1D real vector data supported");
      return(ERR);
   }

// Return the data
   par->retVar[1].AssignMatrix2D(data,dataSizeOut,1);
   par->nrRetVar = 1;

   return(OK);
}

/*******************************************************************************
   Write data to the RS232 device (asynchronous)

   writeserial(port_name, string_to_write)
*******************************************************************************/

short WriteSerialOverlapped(DLLParameters* par, char *args)
{
   DWORD nrWritten = 0;
   short nrArgs;
   OVERLAPPED osWrite = {0};
   Variable dataVar;
   short dwRes;
   bool fRes;
   CText portName;
   short portNr;
   HANDLE hComm;

   if((nrArgs = ArgScan(par->itfc,args,2,"portName, data/string to write","ee","tv",&portName,&dataVar)) < 0)
      return(nrArgs);

   portName.UpperCase();
   if(!(hComm = GetPortHandle(portName.Str(),portNr)))
   {
      return(ERR);
   }

// Create the overlapped event. Must be closed before exiting
// to avoid a handle leak.
   osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

   if(osWrite.hEvent == NULL)
   {
      ErrorMessage("Serial error");
      return(ERR);
   }

   unsigned char *data = 0;
   int dataSize;
   if(dataVar.GetType() == UNQUOTED_STRING)
   {
      data = (unsigned char*)dataVar.GetString();
      dataSize = strlen((char*)data);
   }
   else if(dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
   {
      dataSize = dataVar.GetDimX();
      float *d = dataVar.GetMatrix2D()[0];
      data = new unsigned char[dataSize];
      for(int i = 0; i < dataSize; i++)
         data[i] = (unsigned char)(int)(d[i]+0.5);
   }

   if(!WriteFile(hComm, data, dataSize, &nrWritten, &osWrite))
   {
      if(GetLastError() != ERROR_IO_PENDING) 
      { 
          fRes = FALSE;
      }
      else
      {
         dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
         switch(dwRes)
         {
            case (WAIT_OBJECT_0):
            {
               if(!GetOverlappedResult(hComm, &osWrite, &nrWritten, FALSE))
                  fRes = FALSE;
               else
                  fRes = TRUE;
               break;
            }            
            default:
            { 
               fRes = FALSE;
               break;
            }
         }
      }
   }

   if(dataVar.GetType() == MATRIX2D && data)
   {
      delete [] data;
   }

   if(fRes == FALSE)
   { 
      ErrorMessage("Serial error");
      CloseHandle(osWrite.hEvent);
      return(ERR);
   }

   CloseHandle(osWrite.hEvent);

  // TextMessage("Data written %d\n",nrWritten);

   return(OK);
}

//
//short WriteSerialOverlapped(DLLParameters* par, char *args)
//{
//   DWORD nrWritten;
//   short nrArgs;
//   OVERLAPPED osWrite = {0};
//   Variable dataVar;
//   short dwRes;
//   bool fRes;
//   CText portName;
//   short portNr;
//   HANDLE hComm;
//
//   if((nrArgs = ArgScan(par->itfc,args,2,"portName, data/string to write","ee","tv",&portName,&dataVar)) < 0)
//      return(nrArgs);
//
//   portName.UpperCase();
//   if(!(hComm = GetPortHandle(portName.Str(),portNr)))
//   {
//      return(ERR);
//   }
//
//// Create the overlapped event. Must be closed before exiting
//// to avoid a handle leak.
//   osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//
//   if(osWrite.hEvent == NULL)
//   {
//      ErrorMessage("Serial error");
//      return(ERR);
//   }
//
//   unsigned char *data = 0;
//   int dataSize;
//   if(dataVar.GetType() == UNQUOTED_STRING)
//   {
//      data = (unsigned char*)dataVar.GetString();
//      dataSize = strlen((char*)data);
//   }
//   else if(dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
//   {
//      dataSize = dataVar.GetDimX();
//      float *d = dataVar.GetMatrix2D()[0];
//      data = new unsigned char[dataSize];
//      for(int i = 0; i < dataSize; i++)
//         data[i] = (unsigned char)(int)(d[i]+0.5);
//   }
//
//   if(!WriteFile(hComm, data, dataSize, &nrWritten, &osWrite))
//   {
//      if(GetLastError() != ERROR_IO_PENDING) 
//      { 
//          fRes = FALSE;
//      }
//      else
//      {
//         dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
//         switch(dwRes)
//         {
//            case (WAIT_OBJECT_0):
//            {
//               if(!GetOverlappedResult(hComm, &osWrite, &nrWritten, FALSE))
//                  fRes = FALSE;
//               else
//                  fRes = TRUE;
//               break;
//            }            
//            default:
//            { 
//               fRes = FALSE;
//               break;
//            }
//         }
//      }
//   }
//
//   if(dataVar.GetType() == MATRIX2D && data)
//   {
//      delete [] data;
//   }
//
//   if(fRes == FALSE)
//   { 
//      ErrorMessage("Serial error");
//      CloseHandle(osWrite.hEvent);
//      return(ERR);
//   }
//
//   CloseHandle(osWrite.hEvent);
//
//   return(OK);
//}

#define READ_TIMEOUT 2

short ReadSerialOverlapped(DLLParameters* par, char *args)
{
   BOOL fWaitingOnRead = FALSE;
   OVERLAPPED osReader = {0};
   HANDLE hComm;
   short nrArgs;
   CText portName;
   short portNr;
   CText type = "char";
   CText exitChar = "";
   unsigned char *data;
   DWORD nrRead = 0;

  // TextMessage("Read asynchronous command\n");

// Get the parameters
   if((nrArgs = ArgScan(par->itfc,args,1,"portname, exit_char, [type]","eee","ttt", &portName,&exitChar,&type)) < 0)
      return(nrArgs);

// Read the port name
   portName.UpperCase();
   if(!(hComm = GetPortHandle(portName.Str(),portNr)))
   {
      return(ERR);
   }

// Create the overlapped event. Must be closed before exiting
// to avoid a handle leak.
   osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

// Error creating overlapped event; abort.
   if(osReader.hEvent == NULL)
   {
      ErrorMessage("Serial error");
      return(ERR);
   }

	//DWORD dwEventMask = EV_RXCHAR;
	//BOOL Status = SetCommMask(hComm,EV_RXCHAR);
	//Status = WaitCommEvent(hComm, &dwEventMask, &osReader);
	//if(dwEventMask == EV_RXCHAR)
	//	fWaitingOnRead = TRUE;

// Read the data
   int n = 0;
   char d = 'z';
   data = new unsigned char[11]; 
   data[0] = '\0';
   int szAlloc = 10;
   char ec = exitChar[0];

   DWORD lastError = 0;

   //if(!fWaitingOnRead) 
   //{
   //   // Issue read operation.
   //   if (!ReadFile(hComm, &d, 1, &nrRead, &osReader))
   //   {
   //      lastError = GetLastError();
   //      if (lastError == ERROR_IO_PENDING)  // Data pending
   //      {
   //          fWaitingOnRead = TRUE;
   //      }
   //      else // Error in communications; report it.
   //      {
   //         ErrorMessage("Serial error %d",lastError);
   //         ResetEvent(osReader.hEvent);
   //         CloseHandle(osReader.hEvent);
   //         CancelIo(hComm);
   //         CloseHandle(hComm);
   //         ports[portNr]= NULL;
   //         return(ERR);
   //      }
   //   }
   //   else // All data read immediately
   //   { 
   //      ResetEvent(osReader.hEvent);
   //      CloseHandle(osReader.hEvent);
   //      CancelIo(hComm);
   //     // TextMessage("Data read immediately\n");
   //      data[0] = d;
   //      data[nrRead] ='\0';
   //      par->retVar[1].MakeAndSetString((char*)data);
   //      par->nrRetVar = 1;  
   //      return(OK);
   //   }
   //}
   //if(nrRead == 1)
   //{
   //   data[n++] = d;
   //}
	fWaitingOnRead = TRUE;

// Data pending so wait for it and then read in 1 byte at a time
   DWORD dwRes;

   if(fWaitingOnRead) 
   {
      for(;;)
      {
         dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
         switch(dwRes)
         {
           // Read completed.
            case (WAIT_OBJECT_0):
            {
               if (!GetOverlappedResult(hComm, &osReader, &nrRead, FALSE))
               {
                  ErrorMessage("Serial error");
                  ResetEvent(osReader.hEvent);
                  CloseHandle(osReader.hEvent);
                  CancelIo(hComm);
                  CloseHandle(hComm);
                  ports[portNr]= NULL;
                  return(ERR);
               }
               if(nrRead == 1)
               {
                  data[n++] = d;
               }
             // nrRead bytes of data is pending so read it in 
               DWORD num;
               do
               {
 
               // Read the data from the serial port 1 byte at a time.
                  ReadFile(hComm, &d, 1, &num, &osReader);

                  if(num == 1)
                  {
                     if(ec != '\0' && d == ec)
                         break;

                  // Save the data, resizing buffer as required
                     if(n == szAlloc)
                     { 
                        szAlloc *= 2;
                        unsigned char *temp = new unsigned char[szAlloc+1];
                        memcpy(temp,data,n);
                        delete [] data;
                        data = temp;
                     }
                     data[n++] = d;
                  }
               }
               while(num != 0);

               data[n] = '\0';

               if(type == "float")
               {
                  float **fData = MakeMatrix2D(n,1);
                  for(int i = 0; i < n; i++)
                     fData[0][i] = (float)data[i];
                  par->retVar[1].AssignMatrix2D(fData,n,1);
               }
               else
               {
                  par->retVar[1].MakeAndSetString((const char*)data);
               }
               par->nrRetVar = 1;

               delete [] data;
               ResetEvent(osReader.hEvent);
               CloseHandle(osReader.hEvent);
             //  CancelIo(hComm);
             //  CloseHandle(hComm);
               return(OK);
            }

            // Nothing happening so do some background event handling
            case (WAIT_TIMEOUT):
            {
              //    TextMessage("#9 Waiting for data\n");

               if(ProcessBackgroundEvents() == 2)
               {
                  ResetEvent(osReader.hEvent);
                  CloseHandle(osReader.hEvent);
                  CancelIo(hComm);
                  TextMessage("#8 User abort!\n");
                  CloseHandle(hComm);
                  ports[portNr]= NULL;
                  return(OK);
               }
               break;    
            }

            default: // Error in the WaitForSingleObject; abort.
            {
               ErrorMessage("Serial error");
               ResetEvent(osReader.hEvent);
               CloseHandle(osReader.hEvent);
               CancelIo(hComm);
               CloseHandle(hComm);
               ports[portNr]= NULL;
               return(ERR);
            }
            break;
         }
      }
   }
   ResetEvent(osReader.hEvent);
   CloseHandle(osReader.hEvent);
   return(OK);
}

/*******************************************************************************
   read data from the rs232 device (asynchronous)
*******************************************************************************/
//
//#define READ_BUF_SIZE 100
//#define READ_TIMEOUT 100
//
//short ReadSerialOverlapped(DLLParameters* par, char *args)
//{
//   DWORD dwRead;
//   BOOL fWaitingOnRead = FALSE;
//   OVERLAPPED osReader = {0};
//   char lpBuf[READ_BUF_SIZE+1];
//   char result[READ_BUF_SIZE+1] = "";
//   short cnt = 0;
//   HANDLE hComm;
//   short nrArgs;
//   CText portName;
//   CText mode = "string";
//   short portNr;
//
//   if((nrArgs = ArgScan(par->itfc,args,1,"port, mode (string/vector)","ee","tt", &portName, &mode)) < 0)
//  // if((nrArgs = ArgScan(par->itfc,args,1,"port","e","t", &portName)) < 0)
//      return(nrArgs);
//
//   if(mode != "string" && mode != "vector")
//   {
//      ErrorMessage("invalid mode; should be 'string' or 'vector'");
//      return(ERR);
//   }
//
//   portName.UpperCase();
//   if(!(hComm = GetPortHandle(portName.Str(),portNr)))
//   {
//      return(ERR);
//   }
//
//// Create the overlapped event. Must be closed before exiting
//// to avoid a handle leak.
//   osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//
//// Error creating overlapped event; abort.
//   if(osReader.hEvent == NULL)
//   {
//      ErrorMessage("Serial error");
//      return(ERR);
//   }
//
//   DWORD lastError = 0;
//
//   if (!fWaitingOnRead) 
//   {
//      // Issue read operation.
//      if (!ReadFile(hComm, lpBuf, READ_BUF_SIZE, &dwRead, &osReader))
//      {
//         lastError = GetLastError();
//         if (lastError != ERROR_IO_PENDING)     // read not delayed?
//         {
//            ErrorMessage("Serial error");
//            ResetEvent(osReader.hEvent);
//            CloseHandle(osReader.hEvent);
//            CancelIo(hComm);
//            return(ERR);
//         }
//            // Error in communications; report it.
//         else
//            fWaitingOnRead = TRUE;
//      }
//      else 
//      { 
//         ResetEvent(osReader.hEvent);
//         CloseHandle(osReader.hEvent);
//         CancelIo(hComm);
//         lpBuf[dwRead] ='\0';
//         par->retVar[1].MakeAndSetString((char*)lpBuf);
//         par->nrRetVar = 1;   
//         return(OK);
//      }
//   }
//
//   DWORD dwRes;
//
//   if(fWaitingOnRead) 
//   {
//      for(;;)
//      {
//         dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
//         switch(dwRes)
//         {
//            // Read completed.
//            case WAIT_OBJECT_0:
//            {
//               if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE))
//               {
//                  ErrorMessage("Serial error");
//                  ResetEvent(osReader.hEvent);
//                  CloseHandle(osReader.hEvent);
//                  CancelIo(hComm);
//                  return(ERR);
//               }
//                  // Error in communications; report it.
//               else
//                  // Read completed successfully.
//               lpBuf[dwRead] ='\0';
//               strcat(result,lpBuf); 
//             //  TextMessage("#5 Number of bytes read = %ld\n",dwRead);
//       
//               do 
//               {
//               // Read the data from the serial port.
//                  ReadFile(hComm, lpBuf, 1, &dwRead, &osReader);
//
//               // Display the data read.
//                  if(dwRead > 0)
//                  { 
//                     lpBuf[dwRead] ='\0';
//                     strcat(result,lpBuf); 
//                  }
//
//                  if(dwRead == 0)
//                  {
//                     par->retVar[1].MakeAndSetString(result);
//                     par->nrRetVar = 1;   
//                     ResetEvent(osReader.hEvent);
//                     CloseHandle(osReader.hEvent);
//                     CancelIo(hComm);
//                 //    TextMessage("#6 Read zero bytes so finished\n");
//
//                     return(OK);
//                  }
//               }
//               while (dwRead != 0);
//               break;
//            }
//
//            case WAIT_TIMEOUT:
//               if(ProcessBackgroundEvents() == 2)
//               {
//                  ResetEvent(osReader.hEvent);
//                  CloseHandle(osReader.hEvent);
//                  CancelIo(hComm);
//                //  TextMessage("#7 Time out!\n");
//                  return(OK);
//               }
//               break;                       
//
//            default:
//               // Error in the WaitForSingleObject; abort.
//               // This indicates a problem with the OVERLAPPED structure's
//               // event handle.
//               {
//                  ErrorMessage("Serial error");
//                  ResetEvent(osReader.hEvent);
//                  CloseHandle(osReader.hEvent);
//                  CancelIo(hComm);
//                  return(ERR);
//               }
//               break;
//         }
//      }
//   }
//   return(OK);
//}

/*******************************************************************************
   Read data from the rs232 device (synchronous). Return as a floating point
	array of bytes or as a string.
*******************************************************************************/

short ReadSerial(DLLParameters* par, char *args)
{
   DWORD nrRead;
   unsigned char *data;
   short nrArgs;
   CText portName;
   CText type = "string";
   short portNr;
   HANDLE hComm;
   CText exitChar = "";
	Variable modeVar;
	long nrChar = 0;

   if((nrArgs = ArgScan(par->itfc,args,2,"portname, exit_char/number_char, [\"float/string\"']","eee","tvt", &portName,&modeVar,&type)) < 0)
      return(nrArgs);

	if(modeVar.GetType() == UNQUOTED_STRING)
	{
		exitChar = modeVar.GetString();
		if(exitChar.Size() != 1)
		{
			ErrorMessage("Exit character string should be 1 character long");
			return(ERR);
		}
	}
	else if(modeVar.GetType() == FLOAT32)
	{
		nrChar = nint(modeVar.GetReal());
	}
	else
	{
		ErrorMessage("Invalid mode variable (shoudl be character or length)");
		return(ERR);
	}

// Read the port name
   portName.UpperCase();
   if(!(hComm = GetPortHandle(portName.Str(),portNr)))
   {
      CloseHandle(hComm);
      ports[portNr]= NULL;
      return(ERR);
   }


// Read the data
   int n = 0;
   char d;
   data = new unsigned char[11];
   int szAlloc = 10;
   char ec = exitChar[0];


   do
   {
      if (!ReadFile(hComm, &d, 1, &nrRead, NULL)) // Read a single character
      {
         CloseHandle(hComm);
         ports[portNr]= NULL;
         delete [] data;
         int err = GetLastError();
         if(err == ERROR_OPERATION_ABORTED)
            TextMessage("Serial read aborted\n");
			else
				ErrorMessage("Serial read error %d",err);
         return(ERR);
      }

		if(nrRead == 0) // Exit on time-out event
		{
         delete [] data;
         ErrorMessage("Serial timeout\n");
         return(ERR);
		}

		if(nrRead == 1)
		{
			if((ec != '\0') && (nrChar == 0) && (d == ec)) // Reached the end character
				 break;

			if(n == szAlloc)
			{
				szAlloc *= 2;
				unsigned char *temp = new unsigned char[szAlloc+1];
				memcpy(temp,data,n);
				delete [] data;
				data = temp;
			}
			data[n++] = d;

			if((ec == '\0') && (nrChar > 0) && (n == nrChar)) // Read the required number of characters
				break;
		}
	}
   while(1);

   data[n] = '\0';

   if(type == "float")
   {
      float **fData = MakeMatrix2D(n,1);
      for(int i = 0; i < n; i++)
         fData[0][i] = (float)data[i];
      par->retVar[1].AssignMatrix2D(fData,n,1);
   }
   else // String type
   {
      par->retVar[1].MakeAndSetString((const char*)data);
   }
   par->nrRetVar = 1;
   delete [] data;

   return(OK);
}

/* CRC calculations ****************************/

unsigned int AddToCrc(unsigned char Z);

// Length of ’int’ is 16 bit.
#define CRC_POLY 0x8408
#define CRC_START_VALUE 0xFFFF

static unsigned int CRCValue = CRC_START_VALUE;

// Calculate the CRC value of a single number or a 1D array.


short CRCCalc(DLLParameters* par, char *args)
{
   short nrArgs;
   long value;
   Variable crcValue;
   unsigned int result;

   if((nrArgs = ArgScan(par->itfc,args,1,"value","e","v", &crcValue)) < 0)
      return(nrArgs);

   if(crcValue.GetType() == FLOAT32)
   {
      unsigned long data = ( unsigned long)(crcValue.GetReal() + 0.5);
      if(data < 0 || data > 255)
      {
         ErrorMessage("Invalid CRC character value %d\n",data);
         return(ERR);
      }
      result = AddToCrc((unsigned char)data);
   }
   else if(crcValue.GetType() == MATRIX2D && crcValue.GetDimY() == 1 && crcValue.GetDimZ() == 1 && crcValue.GetDimQ() == 1)
   {
      int sz = crcValue.GetDimX();
      for(int i = 0; i < sz; i++)
      {
         unsigned long data = (unsigned long)((crcValue.GetMatrix2D())[0][i]+0.5);
         if(data < 0 || data > 255)
         {
            ErrorMessage("Invalid CRC character value %d\n",data);
            return(ERR);
         }
         result = AddToCrc((unsigned char)data);
      }
   }
   else
   {
      ErrorMessage("Unsupported input value for CRC calculation");
      return(ERR);
   }

// Return the data
   par->retVar[1].MakeAndSetFloat(result);
   par->nrRetVar = 1;
   return(OK);
}

// initalise CRC value.
short CRCReset(DLLParameters* par, char *args)
{
   CRCValue = CRC_START_VALUE;
   par->nrRetVar = 0;
   return(OK);
}

// Add Byte Z to checksum.
// Return: new value of checksum after adding Byte Z

unsigned int AddToCrc(unsigned char Z)
{
   CRCValue ^= ((unsigned short) Z);
   for(int k = 0; k < 8; k++)
   {
      if((CRCValue&0x01)!=0)
      {
         CRCValue>>= 1;
         CRCValue^=CRC_POLY;
      }
      else
      {
         CRCValue>>= 1;
      }
   } 
   return(CRCValue);
}

/*****************************************************************************************
*                           Return the nearest integer to the float num                  *
*****************************************************************************************/

long nint(float num)
{
   if(num > 0)
      return((long)(num+0.5));
   else
      return((long)(num-0.5));
   
}

/*****************************************************************************************
*              Cancel a pending synchronous serial i/o in another thread                 *
*****************************************************************************************/

short CancelSynchronouseIO(DLLParameters* par, char *args)
{
   short nrArgs;
   long threadID;

   if((nrArgs = ArgScan(par->itfc,args,1,"value","ee","l", &threadID)) < 0)
      return(nrArgs);

   if(Asychronous)
   {
      ErrorMessage("Command only works in synchronous mode");
      return(ERR);
   }

   HANDLE hThread = OpenThread(THREAD_TERMINATE,FALSE,threadID);

   if(hThread)
   {
     if(!CancelSynchronousIo((HANDLE)hThread))
     {
        ErrorMessage("I/O cancellation failed, error number %ld\n",GetLastError());
        return(ERR);
     }
   }
   else
   {
      ErrorMessage("Can't open thread, error number %ld\n",GetLastError());
      return(ERR);
   }
   
   return(OK);
}
