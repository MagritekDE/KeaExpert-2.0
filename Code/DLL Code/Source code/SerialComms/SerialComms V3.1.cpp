//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Implements basic open/read/write/close commands for the serial (RS232) port.
//
// Version history
//
// 3.0 (19-June-15) -----------------------------
//
// 1. Added storage for multiple serial connections. 
//    All commands now require the port name passed as the first argument
//
// 3.1 (2-Oct-15)
//
// 1. Extended write command to handle vector inputs for data streams 
//    containing zeros.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <shellapi.h>

#define VERSION 3.1 // 19-6-15

#define MAXPORTS 50

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);

short OpenSerialOverlapping(DLLParameters*,char*);
short CloseSerial(DLLParameters*,char*);
short ReadSerialOverlapping(DLLParameters*,char*);
short WriteSerialOverlapping(DLLParameters*,char*);
short GetHelpFolder(DLLParameters*,char*);
HANDLE GetPortHandle(char *portName, short &portNr);
bool CheckPortName(char *portName, short &portNr);
short EndPP(DLLParameters*,char *args);

void InsertUniqueStringIntoList(char *str, char ***list, long &position);


HANDLE ports[MAXPORTS];

char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list
long label;     // label counter (to generate unique label)

#define SERBUF 1000 // Maximum number of characters to transfer at one time

// Extension procedure to add commands to Prospa 

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;
      
   if(!strcmp(command,"openserial"))          r = OpenSerialOverlapping(dpar,parameters);      
   else if(!strcmp(command,"closeserial"))    r = CloseSerial(dpar,parameters);      
   else if(!strcmp(command,"readserial"))     r = ReadSerialOverlapping(dpar,parameters);      
   else if(!strcmp(command,"writeserial"))    r = WriteSerialOverlapping(dpar,parameters);      
   else if(!strcmp(command,"helpfolder"))     r = GetHelpFolder(dpar,parameters);      
 //  else if(!strcmp(cmd,"serialversion"))     strcpy(syntax,"(INT v) = ppversion()");
    
                
   return(r);
}


EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   Serial Comms DLL module (V%1.2f)\n\n",VERSION);
   TextMessage("   openserial ....... open serial port\n");
   TextMessage("   closeserial ...... close serial port\n");
   TextMessage("   writeserial ...... write to serial port\n");
   TextMessage("   readserial ....... read from serial port\n");
}

/*******************************************************************************
   Extension procedure to return syntax in DLL 
*******************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"openserial"))       strcpy(syntax,"openserial([port, baudrate, databits, parity, stop bit])");
   else if(!strcmp(cmd,"writeserial")) strcpy(syntax,"writeserial(port,data)");
   else if(!strcmp(cmd,"readserial"))  strcpy(syntax,"data = readserial(port)");
   else if(!strcmp(cmd,"closeserial")) strcpy(syntax,"closeserial(port)");
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
*******************************************************************************/

short OpenSerialOverlapping(DLLParameters* par, char *args)
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


   if((nrArgs = ArgScan(par->itfc,args,0,"port, baudrate, databits, parity, stop bit",
                               "eeeee","tllss", &port, &baudrate, &bytesize, parity, stopbits)) < 0)
      return(nrArgs);

   port.UpperCase();
   if(!CheckPortName(port.Str(),portNr))
      return(ERR);

   hComm = CreateFile(port.Str(),  
                     GENERIC_READ | GENERIC_WRITE, 
                     0, 
                     0, 
                     OPEN_EXISTING,
                     FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING,
                     0);

   if(hComm == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("can't open serial port");
      return(ERR);
   }

   DCB dcb = {0};

   if (!GetCommState(hComm, &dcb))
   {
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
      ErrorMessage("invalid parity");
      return(ERR);
   }

   if(!strcmp(stopbits,"1"))        dcb.StopBits = ONESTOPBIT;
   else if(!strcmp(stopbits,"1.5")) dcb.StopBits = ONE5STOPBITS;
   else if(!strcmp(stopbits,"2"))   dcb.StopBits = TWOSTOPBITS;
   else
   {
      ErrorMessage("invalid stop bits");
      return(ERR);
   }

   if(!SetCommState(hComm, &dcb))
   {
      DWORD err = GetLastError();
      ErrorMessage("invalid serial port state");
      return(ERR);
   }

   COMMTIMEOUTS cto;

   GetCommTimeouts(hComm, &cto);

   cto.ReadIntervalTimeout = MAXDWORD;
   cto.ReadTotalTimeoutMultiplier = MAXDWORD;
   cto.ReadTotalTimeoutConstant = MAXDWORD-1;

   SetCommTimeouts(hComm, &cto);

   // Save the port handle
   ports[portNr] = hComm;

   return(OK);
}

/*******************************************************************************
   Close a connection to the RS232 device 
*******************************************************************************/

short CloseSerial(DLLParameters* par, char *args)
{
   short nrArgs,portNr;
   CText port;
   HANDLE hComm;

   if((nrArgs = ArgScan(par->itfc,args,1,"port","e","t", &port)) < 0)
      return(nrArgs);

   port.UpperCase();
   if(!(hComm = GetPortHandle(port.Str(),portNr)))
   {
      return(ERR);
   }

   CloseHandle(hComm);
   ports[portNr]= NULL;

   return(OK);
}

/*******************************************************************************
   Write data to the RS232 device 
*******************************************************************************/

short WriteSerialOverlapping(DLLParameters* par, char *args)
{
   DWORD nrWritten;
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

   unsigned char *data;
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

   if(fRes == FALSE)
   { 
      ErrorMessage("Serial error");
      CloseHandle(osWrite.hEvent);
      return(ERR);
   }

   CloseHandle(osWrite.hEvent);

   return(OK);
}


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
   read data from the rs232 device 
*******************************************************************************/

#define READ_BUF_SIZE 100
#define READ_TIMEOUT 20

short ReadSerialOverlapping(DLLParameters* par, char *args)
{
   DWORD dwRead;
   BOOL fWaitingOnRead = FALSE;
   OVERLAPPED osReader = {0};
   char lpBuf[READ_BUF_SIZE+1];
   char result[READ_BUF_SIZE+1] = "";
   short cnt = 0;
   HANDLE hComm;
   short nrArgs;
   CText portName;
   CText mode = "string";
   short portNr;

 //  if((nrArgs = ArgScan(par->itfc,args,1,"port, mode (string/vector)","ee","tt", &portName, &mode)) < 0)
   if((nrArgs = ArgScan(par->itfc,args,1,"port","e","t", &portName)) < 0)
      return(nrArgs);

   if(mode != "string" && mode != "vector")
   {
      ErrorMessage("invalid mode; should be 'string' or 'vector'");
      return(ERR);
   }

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

   if (!fWaitingOnRead) 
   {
      // Issue read operation.
      if (!ReadFile(hComm, lpBuf, READ_BUF_SIZE, &dwRead, &osReader))
      {
         if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
         {
            ErrorMessage("Serial error");
            ResetEvent(osReader.hEvent);
            CloseHandle(osReader.hEvent);
            CancelIo(hComm);
            return(ERR);
         }
            // Error in communications; report it.
         else
            fWaitingOnRead = TRUE;
      }
      else 
      { 
         ResetEvent(osReader.hEvent);
         CloseHandle(osReader.hEvent);
         CancelIo(hComm);
         // read completed immediately
         lpBuf[dwRead] ='\0';
    //     if(mode == "string")
            par->retVar[1].MakeAndSetString((char*)lpBuf);
 /*        else if(mode == "vector")
         {
            float **data = MakeMatrix2D(dwRead,1);
            for(int i = 0; i < dwRead; i++)
               data[0][i] = (float)lpBuf[i];
            par->retVar[1].AssignMatrix2D(data,dwRead,1);
         }*/
         par->nrRetVar = 1;   
         return(OK);
      }
   }

   DWORD dwRes;

   if(fWaitingOnRead) 
   {
      for(;;)
      {
         dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
         switch(dwRes)
         {
            // Read completed.
            case WAIT_OBJECT_0:
            {
               if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE))
               {
                  ErrorMessage("Serial error");
                  ResetEvent(osReader.hEvent);
                  CloseHandle(osReader.hEvent);
                  CancelIo(hComm);
                  return(ERR);
               }
                  // Error in communications; report it.
               else
                  // Read completed successfully.
               lpBuf[dwRead] ='\0';
               strcat(result,lpBuf); 
                
               do 
               {
               // Read the data from the serial port.
                  ReadFile(hComm, lpBuf, 1, &dwRead, &osReader);

               // Display the data read.
                  if(dwRead > 0)
                  { 
                     lpBuf[dwRead] ='\0';
                     strcat(result,lpBuf); 
                  }

                  if(dwRead == 0)
                  {
                     par->retVar[1].MakeAndSetString(result);
                     par->nrRetVar = 1;   
                     ResetEvent(osReader.hEvent);
                     CloseHandle(osReader.hEvent);
                     CancelIo(hComm);
                     return(OK);
                  }
               }
               while (dwRead != 0);
               break;
            }

            case WAIT_TIMEOUT:
               if(ProcessBackgroundEvents() == 2)
               {
                  ResetEvent(osReader.hEvent);
                  CloseHandle(osReader.hEvent);
                  CancelIo(hComm);
                  return(OK);
               }
               break;                       

            default:
               // Error in the WaitForSingleObject; abort.
               // This indicates a problem with the OVERLAPPED structure's
               // event handle.
               {
                  ErrorMessage("Serial error");
                  ResetEvent(osReader.hEvent);
                  CloseHandle(osReader.hEvent);
                  CancelIo(hComm);
                  return(ERR);
               }
               break;
         }
      }
   }
   return(OK);
}
