
#include "../Global files/includesDLL.h"

#include "stdafx.h"
#include "visa.h"

// Locally defined procedure and global variables
EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short VisaWrite(DLLParameters*,char *args);
short VisaQuery(DLLParameters*, char *args);
short VisaGetDeviceName(DLLParameters*, char *args);
//short VisaStart(DLLParameters*, char *args);
//short VisaEnd(DLLParameters*, char *args);
//short VisaOpen(DLLParameters*, char *args);
//short VisaClose(DLLParameters*, char *args);


char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list
//ViSession defaultRM, instr; // Communication channels 

/*******************************************************************************
Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

   if(!strcmp(command,"visawrite"))                 r = VisaWrite(dpar,parameters);      
   else if (!strcmp(command, "visaquery"))          r = VisaQuery(dpar, parameters);
   else if (!strcmp(command, "visagetdevicename"))  r = VisaGetDeviceName(dpar, parameters);
 //  else if (!strcmp(command, "visastart"))          r = VisaStart(dpar, parameters);
//   else if (!strcmp(command, "visaend"))            r = VisaEnd(dpar, parameters);
//   else if (!strcmp(command, "visaopen"))           r = VisaOpen(dpar, parameters);
 //  else if (!strcmp(command, "visaclose"))          r = VisaClose(dpar, parameters);

   return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   Visa DLL module\n\n");
   TextMessage("   visawrite .............. write a string to a NI-Visa device\n");
   TextMessage("   visaquery .............. write a string and then read a string back from a NI-Visa device\n");
   TextMessage("   visagetdevicename ...... given a device serial number return the NI-Visa USB device name\n");
}

/*******************************************************************************
Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if (!strcmp(cmd, "visawrite"))                strcpy(syntax, "visawrite(STR device, STR value)");
   else if (!strcmp(cmd, "visaquery"))          strcpy(syntax, ", STR value = visaquery(STR device, STR value, [INT nr_bytes, [STR string/array8]])");
   else if (!strcmp(cmd, "visagetdevicename"))   strcpy(syntax, ", STR name = visagetdevicename(STR device_serial_number)");


   if(syntax[0] == '\0')
      return(false);
   return(true);
}
//
//short VisaStart(DLLParameters* par, char *args)
//{
//   ViStatus status;            // For checking errors 
//
//   // Begin by initializing the system
//   status = viOpenDefaultRM(&defaultRM);
//   if (status < VI_SUCCESS)
//   {
//      defaultRM = 0;
//      ErrorMessage("Can't intialise NI_Visa");
//      return(ERR);
//   }
//   return(OK);
//}
//
//
//short VisaEnd(DLLParameters* par, char *args)
//{
//   short nrArgs;
//   ViStatus status;            // For checking errors 
//   ViUInt32 retCount;          // Return count from string I/O 
//   CText deviceStr;
//   CText commandStr;
//
//   // Close down the system 
//   if (defaultRM)
//   {
//      status = viClose(defaultRM);
//      defaultRM = 0;
//   }
//   return(OK);
//}
//
//short VisaOpen(DLLParameters* par, char *args)
//{
//   short nrArgs;
//   ViStatus status;            // For checking errors 
//   CText deviceStr;
//
//   if ((nrArgs = ArgScan(par->itfc, args, 1, "device ID", "e", "t", &deviceStr)) < 0)
//      return(nrArgs);
//
//   if(!defaultRM)
//   {
//      ErrorMessage("NI-Visa not initialised");
//      return(ERR);
//   }
//
//   // Open communication with device
//   status = viOpen(defaultRM, (ViRsrc)deviceStr.Str(), VI_NULL, VI_NULL,&instr);
//   if (status < VI_SUCCESS)
//   {
//      instr = 0;
//      status = viClose(defaultRM);
//      ErrorMessage("Can't open device '%s'", deviceStr.Str());
//      return(ERR);
//   }
//   // Set the timeout for message-based communication
//   status = viSetAttribute(instr, VI_ATTR_TMO_VALUE, 5000);
//
//   return(OK);
//}
//
///****************************************************************************
//
//*****************************************************************************/
//
//
//#define MAX_CNT 200
//
//short VisaClose(DLLParameters* par, char *args)
//{
//   short nrArgs;
//   ViStatus status;            // For checking errors 
//   CText deviceStr;
//
//   if ((nrArgs = ArgScan(par->itfc, args, 1, "device ID", "e", "t", &deviceStr)) < 0)
//      return(nrArgs);
//
//   if (!defaultRM)
//   {
//      ErrorMessage("NI-Visa not initialised");
//      return(ERR);
//   }
//
//   if (!instr)
//   {
//      ErrorMessage("Instrument not initialised");
//      return(ERR);
//   }
//
//   status = viClose(instr);
//   return(OK);
//}
//

/****************************************************************************
 
*****************************************************************************/


short VisaWrite(DLLParameters* par, char *args)
{
   short nrArgs;
   ViStatus status;            // For checking errors 
   ViSession defaultRM, instr; // Communication channels 
   ViUInt32 retCount;          // Return count from string I/O 
   CText deviceStr;
   CText commandStr;

   if ((nrArgs = ArgScan(par->itfc, args, 1, "device ID, command", "ee", "tt", &deviceStr,&commandStr)) < 0)
      return(nrArgs);



   // Begin by initializing the system
   status = viOpenDefaultRM(&defaultRM);
   if (status < VI_SUCCESS)
   {
      ErrorMessage("Can't intialise NI_Visa");
      return(ERR);
   }
   // Open communication with device
   status = viOpen(defaultRM, (ViRsrc)deviceStr.Str(), VI_NULL, VI_NULL,&instr);
   if (status < VI_SUCCESS)
   {
      status = viClose(defaultRM);
      ErrorMessage("Can't open device '%s'", deviceStr.Str());
      return(ERR);
   }
   // Set the timeout for message-based communication
   status = viSetAttribute(instr, VI_ATTR_TMO_VALUE, 2000);

   // Write the string
   char* txt = commandStr.Str();
   char* pos = strchr(txt, '#');
   if (pos != NULL)
      txt[pos-txt] = '@';
   status = viWrite(instr, (ViBuf)commandStr.Str(), commandStr.Size(), &retCount);

   // Close down the system 
   status = viClose(instr);
   status = viClose(defaultRM);
   return(OK);
}

/****************************************************************************

*****************************************************************************/

short VisaQuery(DLLParameters* par, char *args)
{
   short nrArgs;
   ViStatus status;            // For checking errors 
   ViSession defaultRM, instr; // Communication channels 
   ViUInt32 retCount;          // Return count from string I/O 
   ViChar *buffer;             // Buffer for string I/O 
   CText deviceStr;
   CText commandStr;
   CText format = "string";
   long bufferSize = 200;

   if ((nrArgs = ArgScan(par->itfc, args, 2, "device ID", "eeee", "ttlt", &deviceStr, &commandStr, &bufferSize, &format)) < 0)
      return(nrArgs);

   // Begin by initializing the system
   status = viOpenDefaultRM(&defaultRM);
   if (status < VI_SUCCESS)
   {
      ErrorMessage("Can't intialise NI_Visa");
      return(ERR);
   }
   // Open communication with device
   status = viOpen(defaultRM, (ViRsrc)deviceStr.Str(), VI_NULL, VI_NULL, &instr);
   if (status < VI_SUCCESS)
   {
      status = viClose(defaultRM);
      ErrorMessage("Can't open device '%s'", deviceStr.Str());
      return(ERR);
   }
   // Set the timeout for message-based communication
   status = viSetAttribute(instr, VI_ATTR_TMO_VALUE, 2000);

   // Write the string
   status = viWrite(instr, (ViBuf)commandStr.Str(), commandStr.Size(), &retCount);

   // Get the response
   buffer = new ViChar[bufferSize+1];
   status = viRead(instr, (ViBuf)buffer, bufferSize, &retCount);
   buffer[retCount] = '\0';

   // Close down the system 
   status = viClose(instr);
   status = viClose(defaultRM);

   // Return data in requested format
   if (format == "string")
   {
      par->nrRetVar = 1;
      par->retVar[1].MakeAndSetString(buffer);
   }
   else if (format == "array8")
   {
      float **data = MakeMatrix2D(retCount, 1);
      for (int i = 0; i < retCount; i++)
         data[0][i] = (float)buffer[i];
      par->nrRetVar = 1;
      par->retVar[1].AssignMatrix2D(data,retCount,1);
   }
   else
   {
      ErrorMessage("Format '%s' not supported\n", format.Str());
      delete[] buffer;
      return(-1);
   }

   delete[] buffer;
  
   return(OK);

}


short VisaGetDeviceName(DLLParameters* par, char *args)
{
   short nrArgs;
   ViStatus status;            // For checking errors 
   ViSession defaultRM, instr; // Communication channels 
   ViUInt32 retCount;          // Return count from string I/O 
   CText searchStr;
   ViFindList fList;
   ViChar deviceStr[VI_FIND_BUFLEN];
   ViUInt32 numInstrs;
   ViUInt16 iManf, iModel;

   if ((nrArgs = ArgScan(par->itfc, args, 1, "device search string", "e", "t", &searchStr)) < 0)
      return(nrArgs);

   // Begin by initializing the system
   status = viOpenDefaultRM(&defaultRM);
   if (status < VI_SUCCESS)
   {
      ErrorMessage("Can't intialise NI_Visa");
      return(ERR);
   }

   status = viFindRsrc(defaultRM, searchStr.Str(), &fList, &numInstrs, deviceStr);

   if (status < VI_SUCCESS)
   {
      // Error finding resources ... exiting 
      viClose(defaultRM);
      return(ERR);
   }

   // Open communication with device
   status = viOpen(defaultRM, (ViRsrc)deviceStr, VI_NULL, VI_NULL, &instr);
   if (status < VI_SUCCESS)
   {
      status = viClose(defaultRM);
      ErrorMessage("Can't open device '%s'", (char*)deviceStr);
      return(ERR);
   }

   // Close down the system 
   status = viClose(instr);
   status = viClose(defaultRM);

   par->nrRetVar = 1;
   par->retVar[1].MakeAndSetString(deviceStr);

   return(OK);
}
