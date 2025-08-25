/***************************************************************************************************
 Code to allow access to the FX3 based Spinsolve and generation of
 pulse programs

 FX3 comms commands

 fx3open
 fx3close
 fx3read
 fx3write
 fx3count
 fx3setport
 fx3timeout

 Pulse program organisation

 The pulse program running in the TRex is an event table. Each event consists of 96 bits

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)

An event is activated when the number of counted clock cycles starting at the first event
equals the sum of the preceeding durations. Looping can cause the event order to change.

The organisation of the arguments depends on the command. Possible command codes are currently

0x00 - Write to TRex address
0x01 - Read from TRex address
0x03 - Reset pulse sequence
0x04 - Loop command
0x05 - End loop command
0x06 - Kea rxGain
0x07 - Kea gradient
0x08 - Spinsolve rxGain and channel selection
0x0D - Spinsolve shim ramp controller
0x0E - Spinsolve shim controller

Addresses are 16 bits with the top 4 bits specifying the TRex module

0x0 - Legacy
0x1 - DDS
0x2 - ADC
0x3 - BRAM
0x4 - DDC
0x5 - 
0x6 - FIFO
0x7 - 

Minimum read/write durations are 15 (150 ns). For table based versions of these commands use 

Detailed command syntax (durations are the minimum nr of clock cycles of 10 ns)

Write to a TRex address:

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
       15          0x00        adrs (12)       value (16)

Write to a TRex address the value in the write table buffer

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
       20          0x00        adrs (12)      0x01000000

Write to a TRex address the value in the write table buffer plus an increment

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
		 20          0x00        adrs (12)      0x02000000 + inc

Write to a TRex address the value in the read table buffer minus a decrement

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
		 20          0x00        adrs (12)      0x03000000 - dec

Read from a TRex address and store in write table buffer and shim16 table buffer

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
		 15          0x01        adrs (12)      0x00000000

Read from an address and save in write buffer

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
		 20          0x01        adrs (12)      0x00000000

**************************************************************************************************/



/**************************************************************************************************
	Version history

V0.3 (10/10/24)

  1. Added 'm' option to fx3read/write to allow control of a motor


**************************************************************************************************/

#pragma pack(push,8)
#include <wtypes.h>
#include "CyAPI.h"
#pragma pack(pop)

#include "../Global files/includesDLL.h"
#include "stdafx.h"
#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>

using namespace std;

#define VERSION 0.3

typedef unsigned long long uint64;
typedef signed long long int64;
typedef unsigned long uint32;
typedef signed long int32;

// DLL helper functions
EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);

// FX3 USB commands
short FX3Write(DLLParameters* par, char *args);
short FX3Read(DLLParameters* par, char *args);
short FX3CountPorts(DLLParameters* par, char* args);
short FX3SetPort(DLLParameters* par, char* args);
short FX3Open(DLLParameters* par, char* args);
short FX3Close(DLLParameters* par, char *args);
short FX3TimeOut(DLLParameters* par, char* args);

// FX3 USB helper commands
bool WriteToEndPoint(CCyUSBEndPoint *endPoint,BYTE* data, long len);
bool ReadFromEndPoint(CCyUSBEndPoint *endPoint, BYTE* data, long &len);
bool EnumerateEndpointForTheSelectedDevice(CCyUSBDevice	*USBDevice, int deviceNr);
int OpenFXDeviceByNr(CCyUSBDevice* USBDevice, uint32 port, uint32& nrDevices);


// FX3 PS utilities
short Report(DLLParameters* par, char* args);

// General utilities
double GetMsTime();
long nint(float num);
long nint(double num);
uint32 nuint(float num);
int64 nlint(double num);
uint64 nulint(double num);
uint64 nulint(float num);

// Global variables
static CCyUSBDevice *USBDevice = 0;
uint32 fx3TimeOut = 10000; // Timeout for reads and writes in ms
int32 defaultPort = -1; // Default FX3 device port nr

/*******************************************************************************
Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

   if(!strcmp(command,"fx3write"))              r = FX3Write(dpar,parameters);      
   else if (!strcmp(command, "fx3read"))        r = FX3Read(dpar, parameters);
	else if (!strcmp(command, "fx3open"))        r = FX3Open(dpar, parameters);
	else if (!strcmp(command, "fx3count"))       r = FX3CountPorts(dpar, parameters);
	else if (!strcmp(command, "fx3setport"))     r = FX3SetPort(dpar, parameters);
	else if (!strcmp(command, "fx3close"))       r = FX3Close(dpar, parameters);
	else if (!strcmp(command, "fx3timeout"))     r = FX3TimeOut(dpar, parameters);


   return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   FX3 USB DLL module (V%1.2f)\n\n",VERSION);
   TextMessage("   fx3write .............. write binary data to the fx3\n");
   TextMessage("   fx3read ............... read binary data from the fx3\n");
   TextMessage("   fx3open ............... make a connection to the fx3\n");
	TextMessage("   fx3close .............. close connection to the fx3\n");
	TextMessage("   fx3count......,........ return the number of connected FX3 devices\n");
	TextMessage("   fx3setport......,...... sets or returns current FX3 port number\n");
	TextMessage("   fx3timeout ............ specify timeout in ms for read and writes\n");
}

/*******************************************************************************
Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if (!strcmp(cmd, "fx3write"))              strcpy(syntax, "fx3write(STR i/e/f, INT address, D/MATRIX1D data)");
   else if (!strcmp(cmd, "fx3read"))          strcpy(syntax, "D/MATRIX data = fx3read(STR i/e/f, INT address, INT nrValues, [STR single/double])");
   else if (!strcmp(cmd, "fx3open"))          strcpy(syntax, "INT result = fx3open()");
	else if (!strcmp(cmd, "fx3close"))         strcpy(syntax, "INT result = fx3close()");
	else if (!strcmp(cmd, "fx3count"))         strcpy(syntax, "INT nr = fx3count()");
	else if (!strcmp(cmd, "fx3setport"))       strcpy(syntax, "INT port = fx3setport([port])");
	else if (!strcmp(cmd, "fx3timeout"))       strcpy(syntax, "fx3timeout(INT timeOut_ms)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

// Open FX3 device by number (default 0) returning the number of devices found

short FX3Open(DLLParameters* par, char *args)
{
	short nrArgs;
	uint32 nrDevices = 0;
	float portF = defaultPort;

	if ((nrArgs = ArgScan(par->itfc, args, 0, "port", "e", "f", &portF)) < 0)
		return(nrArgs);

	if (portF == -1)
		portF = 0;

	uint32 port = nint(portF);

	if(USBDevice)
	{
		USBDevice->Close();
		delete USBDevice;
	}
	USBDevice = new CCyUSBDevice(NULL, CYUSBDRV_GUID, true);

	if(!USBDevice)
	{
		ErrorMessage("No connection to FX3");
		defaultPort = -1;
		return(ERR);
	}
   int deviceNr = OpenFXDeviceByNr(USBDevice, port, nrDevices);
	if(deviceNr == -1)
	{
		ErrorMessage("Can't find connected FX3");
		return(ERR);
	}

	defaultPort = deviceNr;

   //EnumerateEndpointForTheSelectedDevice(USBDevice, deviceNr);
	//TextMessage("Opening FX3\n");
	return OK;
}

// Set or return the currently opened port

short FX3SetPort(DLLParameters * par, char* args)
{
	short r;

	// Get name of file to convert ***************************
	if ((r = ArgScan(par->itfc, args, 0, "portnumber", "e", "l", &defaultPort)) < 0)
		return(r);

	if (r == 0)
	{
		par->retVar[1].MakeAndSetFloat(defaultPort);
		par->nrRetVar = 1;
	}
	return(OK);
}


// Count the number of open FX3 ports

short FX3CountPorts(DLLParameters* par, char* args)
{
	short nrArgs;
	uint32 nrDevices = 0;
	float portF = 0;

	if (USBDevice)
	{
		USBDevice->Close();
		delete USBDevice;
	}
	USBDevice = new CCyUSBDevice(NULL, CYUSBDRV_GUID, true);

	if (!USBDevice)
	{
		ErrorMessage("Can't make USB device");
		return(ERR);
	}
	OpenFXDeviceByNr(USBDevice, 1000, nrDevices); // Try and open a large port # and so fail

	USBDevice->Close();
	delete USBDevice;
	USBDevice = 0;

	par->nrRetVar = 1;
	par->retVar[1].MakeAndSetFloat(nrDevices);

	return OK;
}

short FX3Close(DLLParameters* par, char *args)
{
	if(USBDevice)
	{
		USBDevice->Close();
		delete USBDevice;
	}
	USBDevice = 0;
	defaultPort = -1;

	return OK;
}

short FX3TimeOut(DLLParameters* par, char* args)
{
	short nrArgs;
	long timeOut = fx3TimeOut;

	if ((nrArgs = ArgScan(par->itfc, args, 1, "timeout (ms)", "e", "l", &timeOut)) < 0)
		return(nrArgs);

	if (timeOut > 0)
		fx3TimeOut = timeOut;

	return(OK);
}

/****************************************************************************
  Send a vector to the Cypress FX3 via USB
*****************************************************************************/

short FX3Write(DLLParameters* par, char *args)
{
   short nrArgs;
   Variable dataVar;          
	long adrs = 0;
	CText mode = "internal";
	char dataLocation = 'I';
	const int headerSize = 6;
	long szRet=4;

   if ((nrArgs = ArgScan(par->itfc, args, 2, "mode, address, data", "eee", "tlv", &mode, &adrs, &dataVar)) < 0)
      return(nrArgs);

	if (mode == "i" || mode == "internal")
		dataLocation = 'I';
	else if (mode == "e" || mode == "external")
		dataLocation = 'E';
	else if (mode == "f" || mode == "flash")
		dataLocation = 'F';
	else if (mode == "d" || mode == "data")
		dataLocation = 'D';
	else if (mode == "m" || mode == "motor")
		dataLocation = 'M';
	else
		dataLocation = mode[0];

	if (!USBDevice)
	{
		FX3Open(par, "");
		if (!USBDevice)
		{
			ErrorMessage("No connection to FX3");
			return(ERR);
		}
	}

	CCyUSBEndPoint* epBulkOut = USBDevice->EndPointOf(0x01);
	CCyUSBEndPoint* epBulkIn = USBDevice->EndPointOf(0x81);
	if (!epBulkIn || !epBulkIn)
	{
		ErrorMessage("Invalid endpoints found");
		return(ERR);
	}

	epBulkIn->TimeOut = fx3TimeOut;
	epBulkOut->TimeOut = fx3TimeOut;

	if (!epBulkOut || !epBulkIn)
	{
		ErrorMessage("No connection to FX3");
		return(ERR);
	}

   if(dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
   {
		float *dataOut = dataVar.GetMatrix2D()[0];
		int N = dataVar.GetDimX();
		int nrBytes = N * 4;

		int maxPacketSize = 4096;
		BYTE *bData = new BYTE[nrBytes];
		BYTE* packet = new BYTE[maxPacketSize];
		BYTE* packetIn = new BYTE[maxPacketSize];

		for(int i = 0, j = 0; i < N; i++)
		{
			uint64 value = nulint(dataOut[i]);
			bData[j++] = (value & 0xFF000000) >> 24;
			bData[j++] = (value & 0x00FF0000) >> 16;
			bData[j++] = (value & 0x0000FF00) >> 8;
			bData[j++] = (value & 0x000000FF);

			//TextMessage("%d %X %X %X %X\n", i, bData[j-4], bData[j - 3], bData[j - 2], bData[j - 1]);

		}

		if (nrBytes <= maxPacketSize - headerSize)
		{
			packet[0] = dataLocation;
			packet[1] = 'W';
			packet[2] = (nrBytes & 0xFF00) >> 8;
			packet[3] = (nrBytes & 0x00FF);
			packet[4] = (adrs & 0xFF00) >> 8;
			packet[5] = (adrs & 0x00FF);
			memcpy(packet + headerSize, bData, nrBytes);
			if (!WriteToEndPoint(epBulkOut, packet, nrBytes + headerSize))
			{
				delete[] bData;
				delete[] packet;
				delete[] packetIn;
				ErrorMessage("Write to endpoint failed");
				return ERR;
			}
			if (!ReadFromEndPoint(epBulkIn, packetIn, szRet))
			{
				delete[] bData;
				delete[] packet;
				delete[] packetIn;
				ErrorMessage("Read handshake failed");
				return ERR;
			}
		}
		else
		{
			int payloadSize = (int)((maxPacketSize - headerSize) / 4) * 4;
			int transfers = nrBytes / payloadSize;
			int remainer = nrBytes % payloadSize;
			int idx = 0;
			for (int i = 0; i < transfers; i++)
			{
				idx = i * payloadSize;
				packet[0] = dataLocation;
				packet[1] = 'W';
				packet[2] = (payloadSize & 0xFF00) >> 8;
				packet[3] = (payloadSize & 0x00FF);
				packet[4] = (adrs & 0xFF00) >> 8;
				packet[5] = (adrs & 0x00FF);
				memcpy(packet + headerSize, bData + idx, payloadSize);
				if (!WriteToEndPoint(epBulkOut, packet, payloadSize + headerSize))
				{
					delete[] bData;
					delete[] packet;
					delete[] packetIn;
					ErrorMessage("Write to endpoint failed");
					return ERR;
				}
				if (!ReadFromEndPoint(epBulkIn, packetIn, szRet))
				{
					delete[] bData;
					delete[] packet;
					delete[] packetIn;
					ErrorMessage("Read handshake failed");
					return ERR;
				}
				if (dataLocation != 'D')
				{
					adrs += payloadSize / 4;
				}
			}
			if (remainer > 0)
			{
				idx += payloadSize;
				packet[0] = dataLocation;
				packet[1] = 'W';
				packet[2] = (remainer & 0xFF00) >> 8;
				packet[3] = (remainer & 0x00FF);
				packet[4] = (adrs & 0xFF00) >> 8;
				packet[5] = (adrs & 0x00FF);
				memcpy(packet + headerSize, bData + idx, remainer);
				if (!WriteToEndPoint(epBulkOut, packet, remainer + headerSize))
				{
					delete[] bData;
					delete[] packet;
					delete[] packetIn;
					ErrorMessage("Write to endpoint failed");
					return ERR;
				}
				if (!ReadFromEndPoint(epBulkIn, packetIn, szRet))
				{
					delete[] bData;
					delete[] packet;
					delete[] packetIn;
					ErrorMessage("Read handshake failed");
					return ERR;
				}
			}
		}
		delete[] bData;
		delete[] packet;
		delete[] packetIn;
	}

	else if (dataVar.GetType() == DMATRIX2D && dataVar.GetDimY() == 1)
	{
		double* dataOut = dataVar.GetDMatrix2D()[0];
		int N = dataVar.GetDimX();
		int nrBytes = N * 4;

		int maxPacketSize = 4096;
		BYTE* bData = new BYTE[nrBytes];
		BYTE* packet = new BYTE[maxPacketSize];
		BYTE* packetIn = new BYTE[maxPacketSize];

		for (int i = 0, j = 0; i < N; i++)
		{
			uint64 value = nulint(dataOut[i]);
			bData[j++] = (value & 0xFF000000) >> 24;
			bData[j++] = (value & 0x00FF0000) >> 16;
			bData[j++] = (value & 0x0000FF00) >> 8;
			bData[j++] = (value & 0x000000FF);

			//TextMessage("%d %X %X %X %X\n", i, bData[j-4], bData[j - 3], bData[j - 2], bData[j - 1]);

		}

		if (nrBytes <= maxPacketSize - headerSize)
		{
			packet[0] = dataLocation;
			packet[1] = 'W';
			packet[2] = (nrBytes & 0xFF00) >> 8;
			packet[3] = (nrBytes & 0x00FF);
			packet[4] = (adrs & 0xFF00) >> 8;
			packet[5] = (adrs & 0x00FF);
			memcpy(packet + headerSize, bData, nrBytes);
			if (!WriteToEndPoint(epBulkOut, packet, nrBytes + headerSize))
			{
				delete[] bData;
				delete[] packet;
				delete[] packetIn;
				ErrorMessage("Write to endpoint failed");
				return ERR;
			}
			if (!ReadFromEndPoint(epBulkIn, packetIn, szRet))
			{
				delete[] bData;
				delete[] packet;
				delete[] packetIn;
				ErrorMessage("Read handshake failed");
				return ERR;
			}
		}
		else
		{
			int payloadSize = (int)((maxPacketSize - headerSize) / 4) * 4;
			int transfers = nrBytes / payloadSize;
			int remainer = nrBytes % payloadSize;
			int idx = 0;
			for (int i = 0; i < transfers; i++)
			{
				idx = i * payloadSize;
				packet[0] = dataLocation;
				packet[1] = 'W';
				packet[2] = (payloadSize & 0xFF00) >> 8;
				packet[3] = (payloadSize & 0x00FF);
				packet[4] = (adrs & 0xFF00) >> 8;
				packet[5] = (adrs & 0x00FF);
				memcpy(packet + headerSize, bData + idx, payloadSize);
				if (!WriteToEndPoint(epBulkOut, packet, payloadSize + headerSize))
				{
					delete[] bData;
					delete[] packet;
					delete[] packetIn;
					ErrorMessage("Write to endpoint failed");
					return ERR;
				}
				if (!ReadFromEndPoint(epBulkIn, packetIn, szRet))
				{
					delete[] bData;
					delete[] packet;
					delete[] packetIn;
					ErrorMessage("Read handshake failed");
					return ERR;
				}
				if (dataLocation != 'D')
				{
					adrs += payloadSize / 4;
				}
			}
			if (remainer > 0)
			{
				idx += payloadSize;
				packet[0] = dataLocation;
				packet[1] = 'W';
				packet[2] = (remainer & 0xFF00) >> 8;
				packet[3] = (remainer & 0x00FF);
				packet[4] = (adrs & 0xFF00) >> 8;
				packet[5] = (adrs & 0x00FF);
				memcpy(packet + headerSize, bData + idx, remainer);
				if (!WriteToEndPoint(epBulkOut, packet, remainer + headerSize))
				{
					delete[] bData;
					delete[] packet;
					delete[] packetIn;
					ErrorMessage("Write to endpoint failed");
					return ERR;
				}
				if (!ReadFromEndPoint(epBulkIn, packetIn, szRet))
				{
					delete[] bData;
					delete[] packet;
					delete[] packetIn;
					ErrorMessage("Read handshake failed");
					return ERR;
				}
			}
		}
		delete[] bData;
		delete[] packet;
		delete[] packetIn;
	}
	else
	{
		ErrorMessage("Invalid argument - should be an integer vector");
		return(ERR);
	}

   return(OK);
}

/****************************************************************************
 
*****************************************************************************/

short FX3Read(DLLParameters* par, char *args)
{
   short nrArgs;
   long size;          
	long adrs0 = 0;
	CText mode = "internal";
	CText precision = "single";
	char dataLocation = 'I';
	const int headerSize = 6;

   if ((nrArgs = ArgScan(par->itfc, args, 3, "mode, adrs, data length (bytes)[,precision]", "eeee", "tllt",&mode, &adrs0, &size, &precision)) < 0)
      return(nrArgs);

   if(mode == "i" || mode == "internal")
		dataLocation = 'I';
   else if(mode == "e" || mode == "external")
		dataLocation = 'E';
   else if(mode == "f" || mode == "flash")
		dataLocation = 'F';
	else if(mode == "d" || mode == "data")
		dataLocation = 'D';
	else if (mode == "m" || mode == "motor")
		dataLocation = 'M';
	else
		dataLocation = mode[0];

	if (!USBDevice)
	{
		
		FX3Open(par, "");
		if (!USBDevice)
		{
			ErrorMessage("No connection to FX3");
			return(ERR);
		}
	}

	if(size <= 0)
	{
		ErrorMessage("Invalid data size");
		return(ERR);
	}

	long nrBytes = size * 4;
	long readBytes;
	long adrsIn = adrs0;
	long adrsOut = 0;


	CCyUSBEndPoint *epBulkIn   = USBDevice->EndPointOf(0x81);
	CCyUSBEndPoint *epBulkOut   = USBDevice->EndPointOf(0x01);

	if(!epBulkIn || !epBulkIn)
	{
		ErrorMessage("Invalid endpoints found");
		return(ERR);
	}
	epBulkIn->TimeOut = fx3TimeOut;
	epBulkOut->TimeOut = fx3TimeOut;

	if(!epBulkIn)
	{
		ErrorMessage("No connection to FX3");
		return(ERR);
	}

	float** dataOutSingle;
	double** dataOutDouble;

	if(precision == "single")
		dataOutSingle = MakeMatrix2D(size,1);
	else
		dataOutDouble = MakeDMatrix2D(size, 1);

	long payloadSize = epBulkIn->MaxPktSize;
	long maxPacketSize = 4096;
	BYTE *packetIn = new BYTE[maxPacketSize];

	if(nrBytes <= maxPacketSize) // All data fits in one packet
	{
		BYTE packetOut[headerSize];
		packetOut[0] = dataLocation;
		packetOut[1] = 'R';
		packetOut[2]  = (nrBytes & 0xFF00) >> 8;
		packetOut[3]  = (nrBytes & 0x00FF);
		packetOut[4]  = (adrsIn & 0xFF00) >> 8;
		packetOut[5]  = (adrsIn & 0x00FF);
		if (!WriteToEndPoint(epBulkOut, packetOut, headerSize))
		{
			delete[] packetIn;
			return ERR;
		}

		if (!ReadFromEndPoint(epBulkIn, packetIn, nrBytes))
		{
			delete[] packetIn;
			return ERR;
		}

		for(int i = 0; i < nrBytes; i+=4)
		{
			uint64 b0 = packetIn[i+0] << 24;
			uint64 b1 = packetIn[i+1] << 16;
			uint64 b2 = packetIn[i+2] << 8;
			uint64 b3 = packetIn[i+3];
			if(precision == "single")
				dataOutSingle[0][i/4] = (float)((int32)(b0 + b1 + b2 + b3));
			else
				dataOutDouble[0][i/4] = (double)((int64)(b0 + b1 + b2 + b3));

		}

		if (precision == "single")
			par->retVar[1].AssignMatrix2D(dataOutSingle,size,1);
		else
			par->retVar[1].AssignDMatrix2D(dataOutDouble, size, 1);

      par->nrRetVar = 1;  
	}
	else // Need multiple packets
	{
		long transfers = nrBytes /(maxPacketSize);
		long remainer = nrBytes %(maxPacketSize);

		for(int i = 0; i < transfers; i++)
		{
			BYTE packetOut[headerSize];
			packetOut[0] = dataLocation;
			packetOut[1] = 'R';
			packetOut[2] = (maxPacketSize & 0xFF00) >> 8;
			packetOut[3] = (maxPacketSize & 0x00FF);
			packetOut[4] = (adrsIn & 0xFF00) >> 8;
			packetOut[5] = (adrsIn & 0x00FF);

			if (!WriteToEndPoint(epBulkOut, packetOut, headerSize))
			{
				delete[] packetIn;
				return ERR;
			}

			if (!ReadFromEndPoint(epBulkIn, packetIn, maxPacketSize))
			{
				delete[] packetIn;
				return ERR;
			}

			for(int i = 0; i < maxPacketSize; i+=4)
			{
				uint64 b0 = packetIn[i+0] << 24;
				uint64 b1 = packetIn[i+1] << 16;
				uint64 b2 = packetIn[i+2] << 8;
				uint64 b3 = packetIn[i+3];
				if (precision == "single")
					dataOutSingle[0][(adrsOut + i) / 4] = (float)((int32)(b0 + b1 + b2 + b3));
				else
					dataOutDouble[0][(adrsOut + i) / 4] = (double)((int64)(b0 + b1 + b2 + b3));
			}

			if(dataLocation != 'D')
			{
			   adrsIn += maxPacketSize /4;
			   adrsOut += maxPacketSize;
			}
			else
			{
			   adrsOut += maxPacketSize;
			}
		}
		if(remainer > 0)
		{
			BYTE packetOut[headerSize];
			packetOut[0] = dataLocation;
			packetOut[1] = 'R';
			packetOut[2] = (remainer & 0xFF00) >> 8;
			packetOut[3] = (remainer & 0x00FF);
			packetOut[4] = (adrsIn & 0xFF00) >> 8;
			packetOut[5] = (adrsIn & 0x00FF);

			if (!WriteToEndPoint(epBulkOut, packetOut, headerSize))
			{
				delete[] packetIn;
				return ERR;
			}

			if (!ReadFromEndPoint(epBulkIn, packetIn, remainer))
			{
				delete[] packetIn;
				return ERR;
			}

			for(int i = 0; i < remainer; i+=4)
			{
				uint64 b0 = packetIn[i+0] << 24;
				uint64 b1 = packetIn[i+1] << 16;
				uint64 b2 = packetIn[i+2] << 8;
				uint64 b3 = packetIn[i+3];
				if (precision == "single")
					dataOutSingle[0][(adrsOut + i) / 4] = (float)((int32)(b0 + b1 + b2 + b3));
				else
					dataOutDouble[0][(adrsOut + i) / 4] = (double)((int64)(b0 + b1 + b2 + b3));
			}

		}

		if (precision == "single")
			par->retVar[1].AssignMatrix2D(dataOutSingle, size, 1);
		else
			par->retVar[1].AssignDMatrix2D(dataOutDouble, size, 1);
		par->nrRetVar = 1;
	}

	delete [] packetIn;

   return(OK);
}

/****************************************************************************
 
*****************************************************************************/


bool ReadFromEndPoint( CCyUSBEndPoint *endPoint, BYTE* data, long &len)
{
	//TextMessage("Reading from End point\n");

	 bool result = endPoint->XferData((PUCHAR)data, len); 
	 if (result == false)
		 ErrorMessage("Error reading from FX end-point");
	// TextMessage("Reading from End point OK\n");

	 return(result);
} 

/****************************************************************************
 
*****************************************************************************/


bool WriteToEndPoint( CCyUSBEndPoint *endPoint, BYTE* data, long len)
{
	//TextMessage("Writing to End point\n");
	 bool result = endPoint->XferData((PUCHAR)data, len); 
	 if (result == false)
		 ErrorMessage("Error writing to FX end-point");
//	 TextMessage("Writing to End point OK\n");

	 return(result);
}

/****************************************************************************
  Open an FX3 device by number (zero based)
*****************************************************************************/

int OpenFXDeviceByNr(CCyUSBDevice* USBDevice, uint32 port, uint32 &nrDevices)
{    
    if (USBDevice != NULL) 
    {
        nrDevices = (uint32)USBDevice->DeviceCount();
		  if(port < nrDevices)
		  {
            USBDevice->Open(port);
            if((USBDevice->VendorID == 0x364E) && (USBDevice->ProductID == 0xAAC2))  // New Kea info
            {
               return(port);
            }
            USBDevice->Close();
        }
    }
    else 
		 return -1;

    return -1;
}

/****************************************************************************
 
*****************************************************************************/

bool EnumerateEndpointForTheSelectedDevice(CCyUSBDevice	*USBDevice, int nDeviceIndex)
{
   // int nDeviceIndex = 2;

    // There are devices connected in the system.       
    //USBDevice->Open(nDeviceIndex);
    int interfaces = USBDevice->AltIntfcCount()+1;    

    for (int nDeviceInterfaces = 0; nDeviceInterfaces < interfaces; nDeviceInterfaces++ )
    {
        USBDevice->SetAltIntfc(nDeviceInterfaces);
        int eptCnt = USBDevice->EndPointCount();

		  TextMessage("\nEnd Points\n");


        // Fill the EndPointsBox
        for (int endPoint = 1; endPoint < eptCnt; endPoint++)
        {
            CCyUSBEndPoint *ept = USBDevice->EndPoints[endPoint];

            // INTR, BULK and ISO endpoints are supported.
            if (ept->Attributes == 2)
            {
                CText strData, strTemp;
                
                strData = strData + ((ept->Attributes == 1) ? "ISOC " : ((ept->Attributes == 2) ? "BULK " : "INTR "));
                strData = strData + (ept->bIn ? "IN, " : "OUT, ");
                strTemp.Format("AltInt - %d and EpAddr - 0x%02X", nDeviceInterfaces, ept->Address);
                strData = strData + strTemp;
					 TextMessage("%s\n",strData.Str());

            }
        }        
    }


    return true;

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

long nint(double num)
{
	if (num > 0)
		return((long)(num + 0.5));
	else
		return((long)(num - 0.5));

}

int64 nlint(double num)
{
	if (num > 0)
		return((int64)(num + 0.5));
	else
		return((int64)(num - 0.5));

}

uint64 nulint(double num)
{
	if (num > 0)
		return((uint64)(num + 0.5));
	else
		return((uint64)(num - 0.5));

}

uint64 nulint(float num)
{
	if (num > 0)
		return((uint64)(num + 0.5));
	else
		return((uint64)(num - 0.5));

}


uint32 nuint(float num)
{
	return((uint32)(num + 0.5));
}
