
#include "../Global files/includesDLL.h"
#include "ftd2xx.h"

#pragma comment(lib, "FTD2XX.lib")
#pragma warning(disable:4996)

FT_HANDLE ftHandle = 0;
UCHAR BitMode;
long ioMask = 0;

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short FTDIOpen(DLLParameters*,char *args);
short FTDIClose(DLLParameters*,char *args);
short FTDIWrite(DLLParameters*,char *args);
short FTDIRead(DLLParameters*,char *args);


/*******************************************************************************
Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
  short r = RETURN_FROM_DLL;


  if(!strcmp(command,"ftdiopen"))          r = FTDIOpen(dpar,parameters);      
  else if(!strcmp(command,"ftdiclose"))    r = FTDIClose(dpar,parameters);      
  else if(!strcmp(command,"ftdiwrite"))    r = FTDIWrite(dpar,parameters);      
  else if(!strcmp(command,"ftdiread"))     r = FTDIRead(dpar,parameters);  
 
  return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
  TextMessage("\n\n   FTDI DLL module\n\n");
  TextMessage("   ftdiopen ..... open a connection to the FTDI device\n");
  TextMessage("   ftdiclose..... close a connection to the FTDI device\n");
  TextMessage("   ftdiwrite .... write to the TTL outputs ob the FTDI device\n");
  TextMessage("   ftdiread ..... read from the TTL inputs of the FTDI device\n");
}

/*******************************************************************************
Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
  syntax[0] = '\0';
  if(!strcmp(cmd,"ftdiopen"))           strcpy(syntax,"NUM y = ftdioopen(STR name, INT i/o code, [INT baud_rate])");
  else if (!strcmp(cmd, "ftdiclose"))    strcpy(syntax, "ftdiclose()");
  else if (!strcmp(cmd, "ftdiread"))    strcpy(syntax, "ftdiread(INT mask)");
  else if (!strcmp(cmd, "ftdiwrite"))    strcpy(syntax, "ftdiwrite(INT bits)");


  if(syntax[0] == '\0')
    return(false);
  return(true);
}

// Initialise the device - note that a valid device name (serial number) is required
short init_usb(char* name)
{

   return(0);
}//init_usb

/****************************************************************************

*****************************************************************************/

short FTDIOpen(DLLParameters* par, char *args)
{
   short nrArgs;
   CText mode="r";
   CText deviceName;
   long baudRate = 115200;

   if((nrArgs = ArgScan(par->itfc,args,1,"device name, io_mask, [Baud_Rate]","eee","tll",&deviceName, &ioMask, &baudRate)) < 0)
      return(nrArgs); 

   if (ftHandle)
      FT_Close(ftHandle);

   FT_STATUS ftStatus = FT_OpenEx((PVOID)deviceName.Str(), FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
   if (ftStatus == FT_OK)
   {
      FT_SetBaudRate(ftHandle, baudRate);
   }
   else
   {
      ftHandle = 0;
      ErrorMessage("Unable to initialise FTDI device");
      return(ERR);
   }

  FT_SetBitMode(ftHandle, (UCHAR)ioMask, 0x01);

  return(OK);
}
  
short FTDIClose(DLLParameters* par, char* args)
{
   if(ftHandle)
      FT_Close(ftHandle);

   ftHandle = 0;
   return(OK);
}

short FTDIWrite(DLLParameters* par, char* args)
{
   short nrArgs;
   long bits;
   unsigned char TxBuffer[1];
   DWORD BytesWritten;

   if ((nrArgs = ArgScan(par->itfc, args, 1, "bits to write", "e", "l", &bits)) < 0)
      return(nrArgs);

   TxBuffer[0] = (unsigned char)bits;
   FT_Write(ftHandle, TxBuffer, sizeof(TxBuffer), &BytesWritten);
   return(OK);
}

short FTDIRead(DLLParameters* par, char* args)
{
   short nrArgs;
   long bits;
   UCHAR BitMode;

   if ((nrArgs = ArgScan(par->itfc, args, 1, "bits to read", "e", "l", &bits)) < 0)
      return(nrArgs);

   while (1)
   {
      FT_GetBitMode(ftHandle, &BitMode);
      if ((BitMode & ~ioMask) == bits)
         break;
   }
   return(OK);
}