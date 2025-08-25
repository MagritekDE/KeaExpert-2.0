#include "stdafx.h"
#include "globals.h"
#include "allocate.h"
#include "process.h"
#include "main.h"
#include "variablesClass.h"
#include "scanstrings.h"
#include "list_functions.h"
#include "interface.h"
#include "license.h"
#include "edit_files.h"
#include "variablesOther.h"
#include "files.h"
#include <Iphlpapi.h>
#include <Shlobj.h>
#include "memoryLeak.h"



// Global variables
char **licenseList;
short nrLicensedItems = 0;
double serialNumber;

// Code matrices for license decoding
UCHAR decodeMat[400] = {
90,194,60,175,57,214,47,109,204,20,130,120,15,192,58,220,36,40,199,159,
94,167,176,249,59,200,183,247,32,64,146,112,17,223,188,187,56,32,96,6,
124,128,239,187,240,77,95,216,110,5,168,166,13,250,119,180,220,92,176,123,
137,87,183,154,78,101,18,17,239,135,113,88,0,123,140,203,80,180,1,70,
174,23,157,95,223,71,42,35,0,149,61,195,41,60,25,19,143,29,37,65,
238,149,3,187,142,161,201,154,153,240,107,128,24,9,225,91,90,60,255,225,
17,114,160,70,224,144,25,247,183,151,130,224,177,29,226,14,198,85,189,32,
32,69,19,104,67,80,84,54,237,206,85,20,226,184,198,127,118,57,64,238,
21,43,94,144,152,56,0,187,175,193,41,94,33,62,149,105,48,42,129,147,
236,58,178,103,174,247,65,246,155,104,62,37,93,252,152,246,239,72,120,178,
184,217,116,95,192,177,182,117,74,156,153,31,151,8,197,15,25,133,79,54,
231,149,236,84,172,241,180,195,231,247,140,106,183,86,173,52,8,184,97,6,
96,113,117,139,18,4,100,200,45,131,209,213,218,127,74,184,136,188,224,174,
154,54,166,228,236,189,139,245,239,183,172,30,172,63,151,148,110,69,142,214,
93,213,84,113,220,46,129,23,221,212,243,118,11,64,90,169,254,87,112,196,
80,90,212,73,207,98,41,253,158,40,107,6,133,43,12,174,61,242,226,193,
0,124,104,226,35,149,98,201,252,56,97,203,239,149,60,151,211,0,252,144,
130,83,71,172,231,253,211,59,53,100,229,139,175,188,103,194,180,1,160,220,
93,57,15,59,121,164,113,164,39,6,137,34,224,180,154,164,47,136,45,204,
229,97,42,212,15,100,188,237,122,149,32,3,1,255,209,77,129,10,41,146};

USHORT shuffleMat[400] = {
8,332,215,56,256,186,106,258,4,282,133,81,210,131,361,26,207,377,151,120,
284,275,188,341,344,380,331,217,273,35,276,75,50,167,61,229,24,280,87,135,
105,111,30,214,127,318,103,180,291,173,252,89,199,297,91,222,55,296,88,352,
79,196,152,391,223,287,299,398,172,333,394,200,220,72,334,66,44,381,243,393,
164,244,384,314,272,153,140,157,40,125,109,48,201,147,365,168,39,137,121,82,
205,129,396,340,33,99,238,306,198,395,283,176,260,203,262,288,327,7,206,195,
184,356,52,366,158,253,236,12,204,389,102,371,69,239,118,67,360,29,122,149,
212,387,77,315,11,22,345,71,264,84,97,397,322,38,114,255,142,372,86,70,
289,249,148,241,189,320,231,257,42,295,59,254,124,197,80,219,181,143,321,304,
247,62,316,116,95,221,335,348,57,292,226,51,187,303,45,150,336,224,328,25,
294,240,390,302,83,317,169,234,330,183,85,218,324,78,41,339,233,107,326,378,
17,248,15,266,144,376,211,134,115,216,274,193,104,338,311,290,279,362,46,27,
2,265,54,98,277,20,313,208,13,23,323,19,370,246,160,337,92,232,286,245,
93,76,74,383,250,379,96,53,162,64,174,126,123,225,228,10,353,132,155,60,
293,308,3,182,32,358,14,49,175,329,179,58,94,63,112,368,170,351,399,347,
309,6,73,350,154,68,145,43,47,34,382,0,301,388,185,300,36,138,278,101,
130,261,90,9,375,263,242,285,369,18,100,310,28,128,141,386,385,5,209,298,
392,343,367,230,307,268,349,117,342,251,373,113,16,374,364,312,165,136,202,177,
359,146,363,166,269,65,191,227,305,259,1,319,213,21,161,159,354,325,270,357,
178,37,163,110,139,281,190,271,267,346,31,192,355,237,194,119,171,235,108,156};

/***************************************************************************************
  Check if the user has a valid license file. Should match machine MAC address
  and have valid licensed command names. Also needs correct file tag and version 
****************************************************************************************/

bool CopyLicenceFromDesktop(void);
bool CopyLicenseRequestToDeskTop(void);
bool CopyLicenseRequestToDeskTopV2(void);
//int GetSID(Interface* itfc ,char args[]);
short CheckMacAddress(USHORT *macAdrsToCheck, USHORT macSize);
short CheckMultipleMacAddress(USHORT **macAdrsToCheck, USHORT macSz, USHORT nrMacs);
size_t GetHash(UCHAR *data, int length);
unsigned short* GetFirstMACAddress(int &length);
unsigned short* GetMACAddresses(int &length, int &nr);

long dateToDay(int d, int m, int y);
void dayToDate(int day, int &dd, int &mm, int &yy);
extern int GetDate(Interface *itfc, char arg[]);
long gExpiryDay = 0;
short gLicenseVersion = 0;
int ValidLicenseV2(UCHAR* tempData);

#define  MAC_MATCH_FOUND    0
#define  MAC_NO_MATCH       1
#define  MAC_NO_ETHERNET    2
#define  MAC_INTERNAL_ERROR 3
#define  MAC_FAULT          4        

int ValidLicense()
{
   const int fileSize = 400;
   const int rowSize = 20;
   int i,j;
   CText dir;

// First look in the home folder
	dir.Format("%s\\License",applicationHomeDir);
   if(!SetCurrentDirectory(dir.Str()))
      return(1);

  if(!IsFile("prospaLicense.lic"))
  {

//  Then look in the preferences folder make
//  the license folder if it does not exist
		dir.Format("%s\\..\\License",userPrefDir);
		if(IsDirectory(dir.Str()) == false)
		{
			CreateDirectory(dir.Str() , NULL);
		}

		if(!SetCurrentDirectory(dir.Str()))
			return(1);
  }


// Read in the license file
   FILE *fp;
   do
   {
      if(!(fp = fopen("prospaLicense.lic","rb")) && !(fp = fopen("license.txt","rb")))
      {
         bool haveLicense = false;
         int r = MessageBox(NULL,"Prospa is not licensed. Do you have a license file supplied by Magritek?","Missing Prospa license file 'prospaLicense.lic'",MB_YESNO | MB_ICONEXCLAMATION);

         if(r == IDYES)
         {
            r = MessageBox(NULL,"A valid license file 'prospaLicense.lic' from Magritek should be placed on the desktop.\r\rIt will be copied to the Prospa application folder when you press OK.","Missing Prospa license",MB_OKCANCEL | MB_ICONQUESTION);
            if(r == IDOK)
            {
                if(CopyLicenceFromDesktop())
                   haveLicense = true;
            }
         }
         if(r == IDNO)
         {
            int r = MessageBox(NULL,"Would you like to request a license?","Missing license",MB_YESNO | MB_ICONQUESTION);
            if(r == IDYES)
            {
               if(!CopyLicenseRequestToDeskTopV2())
               {
                  exit(0);
               }
            }
         }
         if(!haveLicense)
            exit(0);
      }
   }
   while(fp == NULL);

// Check file size
   if(GetFileLength(fp) != fileSize*sizeof(UCHAR))
   {
      fclose(fp);
      return(2);
   }

// Read the license data
   UCHAR *licenseData = new UCHAR[fileSize];
   int sz = fread(licenseData,sizeof(UCHAR),fileSize,fp);
   fclose(fp);
   if(sz != fileSize)
   {
      delete [] licenseData;
      return(3);
   }

// Make a temporary array for manipulating the license file
   UCHAR *tempData = new UCHAR[fileSize];
   int pos;

// Unshuffle the license file
   for(i = 0; i < fileSize; i++)
   {
      pos = shuffleMat[i];
      tempData[pos] = licenseData[i];
   }
   delete [] licenseData;


// Get the hash value for the license data
    size_t  hash = GetHash(tempData,396);

// Read the appended hash value for tampering check 
   size_t hashIn = (((size_t)tempData[fileSize-4])<<24) + 
                   (((size_t)tempData[fileSize-3])<<16) +
                   (((size_t)tempData[fileSize-2])<<8)  +
                   ((size_t)tempData[fileSize-1]);
   if(hashIn != hash)
   {
    //  delete [] licenseData;
      return(4);
   }


// Un-XOR the license file
   for(i = 0; i < fileSize; i++)
   {
      tempData[i] = tempData[i] ^ decodeMat[i];
   }

// Get the file tag
   USHORT fileTag = (tempData[0]<<8) + tempData[1];
   if(fileTag != 1368) // sum(strtoascii("Prospa License"))
   {
    //  delete [] licenseData;
      delete [] tempData;
      return(5);
   }

// Get the version number
   USHORT verionNumber = tempData[2];
   gLicenseVersion = verionNumber; 
   if(verionNumber == 2) 
   {
      int r = ValidLicenseV2(tempData);
      delete [] tempData;
      return(r);
   }

   if(verionNumber != 1) // Current version is 1
   {
    //  delete [] licenseData;
      delete [] tempData;
      return(6);
   }

// Get the number of licensed items
   nrLicensedItems = tempData[3];

// Get the MAC address from the license matrix
   USHORT  szMacAdrs = (USHORT)tempData[4];
   USHORT *macAdrs = new USHORT[szMacAdrs];
   for(i = 0; i < szMacAdrs; i++)
      macAdrs[i] =  (USHORT)tempData[i+5];

// Check it matches with one of the ethernet MAC addresses
   int result = CheckMacAddress(macAdrs,szMacAdrs);
   delete [] macAdrs;

   if(result == MAC_NO_ETHERNET)
   {
      MessageBox(NULL,"Please enable your Ethernet adapter and try again.","Error",MB_OK | MB_ICONASTERISK);
    //  delete [] licenseData;
      delete [] tempData;
      exit(0);
   }
   else if(result == MAC_NO_MATCH)
   {
  //    delete [] licenseData;
      delete [] tempData;
      return(7);
   }
   else if(result != MAC_MATCH_FOUND)
   {
      CText err;
      err.Format("License internal error %d - please contact Magritek for help.",result);
      MessageBox(NULL,err.Str(),"Error",MB_OK | MB_ICONASTERISK);
   //   delete [] licenseData;
      delete [] tempData;
      exit(0);
   } 

// Read the serial number and convert to a double
// First word is the number of digits
   USHORT szSerialNr = tempData[rowSize];
   int num;
   double multiplier = 1;
   for(i = szSerialNr-1; i >= 0; i--)
   {
      num = (int)(tempData[rowSize + 1 + i] - '0');
      serialNumber += num*multiplier; 
      multiplier *= 10;
   }

// Read in the licensed items
// First word is the string length
   licenseList = new char*[nrLicensedItems];
   for(i = 2; i <= nrLicensedItems+1; i++)
   {
      int sz = tempData[i*rowSize];
      licenseList[i-2] = new char[sz+1];
      for(j = 0; j < sz; j++)
      {
         licenseList[i-2][j] = (char)tempData[i*rowSize + j + 1];
      }
      licenseList[i-2][j] = '\0';
   }

// Find out how many days left and report.
// if <= 0 then abort with error message
   int day = tempData[19*rowSize];
   int month = tempData[19*rowSize+1];
   int year = tempData[19*rowSize+2];
   gExpiryDay = 0; // Default is an infinite license

   if(day > 0 && month > 0 || year > 0)
   {
      SYSTEMTIME loctime;
      GetLocalTime(&loctime);

      int curDay = loctime.wDay;
      int curMonth = loctime.wMonth;
      int curYear = loctime.wYear;

      int curTotalDays = dateToDay(curDay,curMonth,curYear);
      gExpiryDay = dateToDay(day,month,year + 2000);

      int diff = gExpiryDay-curTotalDays;

      if(diff <= 0)
      {
         CText txt;
         txt.Format("Your temporary Prospa license has expired.\rThe old license file will be deleted if you press OK.\rPlease contact Magritek support to obtain a new license.\n\nsupport@magritek.com",result);
         if(MessageBox(NULL,txt.Str(),"Expired license",MB_OKCANCEL | MB_ICONEXCLAMATION) ==  IDCANCEL)
            exit(0);
         DeleteFile("license.txt");
         DeleteFile("prospaLicense.lic");
         exit(0);
      }
      else
      {
         CText txt;
         if(diff == 1)
            txt.Format("You have %d day left on your temporary Prospa license.",diff);
         else
            txt.Format("You have %d days left on your temporary Prospa license.",diff);
         MessageBox(NULL,txt.Str(),"License information",MB_OK | MB_ICONEXCLAMATION);
      }
   }


// Clean up
 //  delete [] licenseData;
   delete [] tempData;

   return(0);
}


int ValidLicenseV2(UCHAR* tempData)
{
   const int fileSize = 400;
   const int rowSize = 20;
   int i,j;

// Get the number of licensed items
   nrLicensedItems = tempData[3];

// Get the MAC address from the license matrix
   USHORT  szMacAdrs = (USHORT)tempData[4];
   USHORT  nrMacAdrs = (USHORT)tempData[5];
   USHORT **macAdrs = new USHORT*[nrMacAdrs];
   for(j = 0; j < nrMacAdrs; j++)
   {
      macAdrs[j] = new USHORT[szMacAdrs];
      for(i = 0; i < szMacAdrs; i++)
      {
         macAdrs[j][i] =  (USHORT)tempData[(j/2)*rowSize + (j%2)*10 + i + rowSize];
         int a = macAdrs[j][i];
         int b = 45;
      }
   }

// Check it matches with one of the ethernet MAC addresses   
   int result = CheckMultipleMacAddress(macAdrs,szMacAdrs,nrMacAdrs);

// Clean up
   for(j = 0; j < nrMacAdrs; j++)
       delete [] macAdrs[j];
   delete [] macAdrs;

   if(result == MAC_NO_ETHERNET)
   {
      MessageBox(NULL,"Please enable your Ethernet adapter and try again.","Error",MB_OK | MB_ICONASTERISK);
      delete [] tempData;
      exit(0);
   }
   else if(result == MAC_NO_MATCH)
   {
    //  delete [] tempData;
      return(7);
   }
   else if(result != MAC_MATCH_FOUND)
   {
      CText err;
      err.Format("License internal error %d - please contact Magritek for help.",result);
      MessageBox(NULL,err.Str(),"Error",MB_OK | MB_ICONASTERISK);
      delete [] tempData;
      exit(0);
   } 

// Read the serial number and convert to a double
// First word is the number of digits
   USHORT szSerialNr = tempData[6];
   int num;
   double multiplier = 1;
   for(i = szSerialNr-1; i >= 0; i--)
   {
      num = (int)(tempData[7 + i] - '0');
      serialNumber += num*multiplier; 
      multiplier *= 10;
   }

// Read in the licensed items. These start from row 4
// First word is the string length
   licenseList = new char*[nrLicensedItems];
   int startRow = 4;
   for(i = startRow; i <= nrLicensedItems+startRow-1; i++)
   {
      int sz = tempData[i*rowSize];
      licenseList[i-startRow] = new char[sz+1];
      for(j = 0; j < sz; j++)
      {
         licenseList[i-startRow][j] = (char)tempData[i*rowSize + j + 1];
      }
      licenseList[i-startRow][j] = '\0';
   }

// Find out how many days left and report.
// This is stored on row 19
// if <= 0 then abort with error message
   int day = tempData[19*rowSize];
   int month = tempData[19*rowSize+1];
   int year = tempData[19*rowSize+2];
   gExpiryDay = 0; // Default is an infinite license

   if(day > 0 && month > 0 || year > 0)
   {
      SYSTEMTIME loctime;
      GetLocalTime(&loctime);

      int curDay = loctime.wDay;
      int curMonth = loctime.wMonth;
      int curYear = loctime.wYear;

      int curTotalDays = dateToDay(curDay,curMonth,curYear);
      gExpiryDay = dateToDay(day,month,year + 2000);

      int diff = gExpiryDay-curTotalDays;

      if(diff <= 0)
      {
         CText txt;
         txt.Format("Your temporary Prospa license has expired.\rThe old license file will be deleted if you press OK.\rPlease contact Magritek support to obtain a new license.\n\nsupport@magritek.com",result);
         if(MessageBox(NULL,txt.Str(),"Expired license",MB_OKCANCEL | MB_ICONEXCLAMATION) ==  IDCANCEL)
            exit(0);
         DeleteFile("license.txt");
         DeleteFile("prospaLicense.lic");
         exit(0);
      }
      else
      {
         CText txt;
         if(diff == 1)
            txt.Format("You have %d day left on your temporary Prospa license.",diff);
         else
            txt.Format("You have %d days left on your temporary Prospa license.",diff);
         MessageBox(NULL,txt.Str(),"License information",MB_OK | MB_ICONEXCLAMATION);
      }
   }

   return(0);
}

/***************************************************************************************
  All division is integer division, operator % is modulus. 
  Given integer d, m, y, calculate day number.
  From http://alcor.concordia.ca/~gpkatch/gdate-algorithm.html
***************************************************************************************/

long dateToDay(int d, int m, int y)
{
   int day;

   m = (m + 9) % 12;
   y = y - m/10;
   day = 365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + (d - 1);

   return(day);
}

/***************************************************************************************
  Calculate date from day number
  All division is integer division, operator % is modulus. 
  Given day number calculate year, month, and day.
  From http://alcor.concordia.ca/~gpkatch/gdate-algorithm.html
***************************************************************************************/

void dayToDate(int day, int &dd, int &mm, int &yy)
{
   int y,ddd,mi;

   y = (10000*day + 14780)/3652425;
   ddd = day - (365*y + y/4 - y/100 + y/400);
   if (ddd < 0)
   {
      y = y - 1;
      ddd = day - (365*y + y/4 - y/100 + y/400);
   }
   mi = (100*ddd + 52)/3060;
   mm = (mi + 2)%12 + 1;
   yy = y + (mi + 2)/12;
   dd = ddd - (mi*306 + 5)/10 + 1;
}


/***************************************************************************************
   Copy a license file from the desktop to the application license folder
****************************************************************************************/

bool CopyLicenceFromDesktop()
{
   CText srcFile;
   CText dstFile;

   char path [MAX_PATH];
   char desktop [MAX_PATH];
   extern HWND prospaWin;

   SHGetSpecialFolderPath(prospaWin, desktop, CSIDL_DESKTOP,0);

   SimplifyDoubleEscapes(desktop);  // Get rid of any duplicate '\'  

   srcFile.Format("%s\\prospaLicense.lic",desktop);
 //  dstFile.Format("%s\\License\\prospaLicense.lic",applicationHomeDir);

   dstFile.Format("%s\\..\\License\\prospaLicense.lic",userPrefDir);

   if(!CopyFile(srcFile.Str(),dstFile.Str(),FALSE))
   {
      int err = GetLastError();
      CText txt;
      if(err == 2)
         txt.Format("File copy failed. License not found on desktop.");
      else
         txt.Format("File copy failed. Error %d. Please copy file 'prospaLicense.lic' manually from desktop to:\r\r%s\\License",err,applicationHomeDir);
      MessageBox(NULL,txt.Str(),"Error",MB_OK | MB_ICONASTERISK);
      return(false);
   }
   return(true);
}

// Code vectors for MAC address
UCHAR requestCoderMat[] = {29,98,184,199,109,115,161,85,13,71,85,216,53,24,252,193,83,212,157,15,
                           175,88,4,191,182,175,162,69,54,122,248,108,84,39,147,143,140,34,21,141};

UCHAR requestShuffleMat[] = {12,7,5,24,15,3,6,11,34,32,30,25,38,9,29,31,2,14,22,17,18,0,4,35,23,13,
                             33,19,1,27,10,37,16,21,8,39,28,36,20,26};

/***************************************************************************************
   Generate a license request file and copy to the desktop
****************************************************************************************/

bool CopyLicenseRequestToDeskTop()
{
   const int bufSz = 40;
   int length;
   USHORT *adrs = GetFirstMACAddress(length);
   if(!adrs)
   {
      MessageBox(NULL,"Please enable your Ethernet adapter and try again.","Error",MB_OK | MB_ICONASTERISK);
      return(false);
   }

   if(length != 6)
      return(false);

   UCHAR *buf = new UCHAR[bufSz];
   for(int i = 0; i < 6; i++)
      buf[i] = (char)adrs[i];
   for(int i = 6; i < bufSz; i++)
      buf[i] = '\0';

   for(int i = 0; i < bufSz; i++)
       buf[i] = buf[i] ^ requestCoderMat[i];

   size_t  hash = GetHash(buf,bufSz-4);
   buf[bufSz-4] = (hash & 0xFF000000)>>24;
   buf[bufSz-3] = (hash & 0x00FF0000)>>16;
   buf[bufSz-2] = (hash & 0x0000FF00)>>8;
   buf[bufSz-1] = (hash & 0x000000FF);

// Unshuffle the license file
   UCHAR *shuffleBuf = new UCHAR[bufSz];

   int pos;
   for(int i = 0; i < bufSz; i++)
   {
      pos = requestShuffleMat[i];
      shuffleBuf[pos] = buf[i];
   }

   char path [MAX_PATH];
   char desktop [MAX_PATH];
   extern HWND prospaWin;
  
   SHGetSpecialFolderPath(prospaWin, desktop, CSIDL_DESKTOP,0);
   SimplifyDoubleEscapes(desktop);  // Get rid of any duplicate '\'  

   FILE *fp;
   if(!SetCurrentDirectory(desktop))
   {
      delete [] adrs;
      delete [] buf;
      delete [] shuffleBuf;
      MessageBox(NULL,"Can't find desktop - contact Magritek for help.","Error",MB_OK | MB_ICONASTERISK);
      return(false);
   }
   if(!(fp = fopen("prospaLicenseRequest.lic","wb")))
   {
      delete [] adrs;
      delete [] buf;
      delete [] shuffleBuf;
      MessageBox(NULL,"Can't save 'prospaLicenseRequest.lic' to desktop - contact Magritek for help.","Error",MB_OK | MB_ICONASTERISK);
      return(false);
   }

   fwrite(shuffleBuf,sizeof(UCHAR),bufSz,fp);
   fclose(fp);
   delete [] adrs;
   delete [] buf;
   delete [] shuffleBuf;
   return(true);
}

// Version 2 can encode up to 5 MAC addresses. The license request file consists of the number of addresses (1 byte) and then 
// each address in a 6 byte block. Finally there is the (version numnber - 1) and then a 4 byte hash address.

bool CopyLicenseRequestToDeskTopV2()
{
   const int bufSz = 40;
   int length;
   int nr;
   bool copiedToDesktop = false;

   USHORT *adrs = GetMACAddresses(length,nr);
   if(!adrs)
   {
      MessageBox(NULL,"Please enable your Ethernet adapter and try again.","Error",MB_OK | MB_ICONASTERISK);
      return(false);
   }

// The MAC address length should be 6
   if(length != 6)
   {
      MessageBox(NULL,"Invalid MAC address length found","Error",MB_OK | MB_ICONASTERISK);
      return(false);
   }

// Limit the number of MAC addresses to 5 (30 bytes)
   if(nr > 5)
   {
      nr = 5;
   }

// Copy the addresses to buffer which will be saved to file
   UCHAR *buf = new UCHAR[bufSz];
   buf[0] = (UCHAR)nr; // Save the number of addresses
   for(int i = 1; i < 6*nr+1; i++) // Copy the addresses
      buf[i] = (char)adrs[i-1];
   for(int i = 6*nr+1; i < bufSz; i++) // Zero the rest
      buf[i] = '\0';

// Set the version number for the request file
   buf[bufSz-5] = 1;

// EXOR buffer with coder matrix
   for(int i = 0; i < bufSz; i++)
       buf[i] = buf[i] ^ requestCoderMat[i];

// Work out a hash and record at end
   size_t  hash = GetHash(buf,bufSz-4);
   buf[bufSz-4] = (hash & 0xFF000000)>>24;
   buf[bufSz-3] = (hash & 0x00FF0000)>>16;
   buf[bufSz-2] = (hash & 0x0000FF00)>>8;
   buf[bufSz-1] = (hash & 0x000000FF);

// Shuffle the license request file
   UCHAR *shuffleBuf = new UCHAR[bufSz];
   int pos;
   for(int i = 0; i < bufSz; i++)
   {
      pos = requestShuffleMat[i];
      shuffleBuf[pos] = buf[i];
   }

// Write the request file to the desktop 
   char path [MAX_PATH];
   char desktop [MAX_PATH];
   extern HWND prospaWin;

// Get the desktop pathname
   SHGetSpecialFolderPath(prospaWin, desktop, CSIDL_DESKTOP,0);

// Get rid of any duplicate '\'  
   SimplifyDoubleEscapes(desktop);  

// Find a folder to save the license request file. Ideally the desktop
   if(!SetCurrentDirectory(desktop))
   {
      MessageBox(NULL,"Can't find the desktop - please choose a folder to save the license request.","Error",MB_OK | MB_ICONASTERISK);
      Interface itfc;
      SelectFolder(&itfc,"\".\",\"Choose folder to save license request\"");
      if(itfc.nrRetValues == 2)
      {
         char* str = itfc.retVar[1].GetString();
         if(!strcmp(str,"cancel"))
         {
            delete [] adrs;
            delete [] buf;
            delete [] shuffleBuf;
            return(false);
         }
         else
         {
            strcpy(path,str);
            if(!SetCurrentDirectory(path))
            {
               delete [] adrs;
               delete [] buf;
               delete [] shuffleBuf;
               MessageBox(NULL,"Can't open requested folder - contact Magritek for help.","Error",MB_OK | MB_ICONASTERISK);
               return(false);
            }
         }
      }
   }
   else
      copiedToDesktop = true;

// Open the request file for write
   FILE *fp;
   if(!(fp = fopen("prospaLicenseRequest.lic","wb")))
   {
      delete [] adrs;
      delete [] buf;
      delete [] shuffleBuf;
      if(copiedToDesktop)
         MessageBox(NULL,"Can't save 'prospaLicenseRequest.lic' to desktop - contact Magritek for help.","Error",MB_OK | MB_ICONASTERISK);
      else
         MessageBox(NULL,"Can't save 'prospaLicenseRequest.lic' to requested folder - contact Magritek for help.","Error",MB_OK | MB_ICONASTERISK);

      return(false);
   }

// Save the license request information
   fwrite(shuffleBuf,sizeof(UCHAR),bufSz,fp);
   fclose(fp);

// Report to user
   if(copiedToDesktop)
       MessageBox(NULL,"A license request file 'prospaLicenseRequest.lic' has been written to the desktop.\r\rPlease email this file to support@magritek.com along with your contact details.","Missing Prospa license",MB_OK | MB_ICONASTERISK);
   else
       MessageBox(NULL,"A license request file 'prospaLicenseRequest.lic' has been written to your selected folder.\r\rPlease email this file to support@magritek.com along with your contact details.","Missing Prospa license",MB_OK | MB_ICONASTERISK);

// Tidy up
   delete [] adrs;
   delete [] buf;
   delete [] shuffleBuf;
   return(true);
}

/****************************************************************************************
   Check if we have a license for command cmd - can be called from DLLs
****************************************************************************************/

EXPORT bool HaveLicense(char *cmd)
{
   for(int i = 0; i < nrLicensedItems; i++)
   {
      if(!strcmp(licenseList[i],cmd))
         return(true);
   }

   return(false);
}

/***************************************************************************************
  Save a license request to the desktop
****************************************************************************************/

int RequestLicense(Interface* itfc ,char args[])
{
   CopyLicenseRequestToDeskTopV2();
   return(OK);
}

/***************************************************************************************
  Get information about the current licenses
****************************************************************************************/

int GetLicenseInfo(Interface* itfc ,char args[])
{
   short r;
   CText cmd;
   int i,j;

   char **lst = NULL;

   for(i = 0, j = 0; i < nrLicensedItems; i++)
   {
      if(strcmp(licenseList[i],"Prospa"))
      {
         cmd.Format("temp = %s(description)",licenseList[i]);

         r = ProcessMacroStr(itfc, false, cmd.Str()); 
         if(r == 0 && itfc->retVar[1].GetType() == UNQUOTED_STRING)
         {
		       AppendStringToList(itfc->retVar[1].GetString(),&lst,j);
             j++;
         }
      }     
   }

   if(nrLicensedItems == 0 && gReadOnly)
      itfc->retVar[1].MakeAndSetString("readonly");
   else
      itfc->retVar[1].MakeAndSetDouble(serialNumber);

   itfc->retVar[2].MakeAndSetList(lst,j);

   SYSTEMTIME loctime;
   GetLocalTime(&loctime);

   int curDay = loctime.wDay;
   int curMonth = loctime.wMonth;
   int curYear = loctime.wYear;

   int curTotalDays = dateToDay(curDay,curMonth,curYear);

   if(gExpiryDay > 0)
   {
      int diff = gExpiryDay-curTotalDays;
      itfc->retVar[3].MakeAndSetFloat(diff);
   }
   else
      itfc->retVar[3].MakeAndSetFloat(0.0);

   itfc->retVar[4].MakeAndSetFloat(gLicenseVersion);

   itfc->nrRetValues = 4;
    
    FreeList(lst,j);
    return(OK);
}

/****************************************************************************************
   CLI interface to get MAC address
****************************************************************************************/

int GetMacAdress(Interface* itfc ,char args[])
{
   int length;
   int nr;
   unsigned short* mac = GetMACAddresses(length,nr);
   float** macMat = MakeMatrix2D(length,nr);
   for(int j = 0; j < nr; j++)
   {
      for(int i = 0; i < length; i++)
         macMat[j][i] = (float)mac[j*length+i];
   }

   itfc->retVar[1].AssignMatrix2D(macMat,length,nr);
   itfc->nrRetValues = 1;
   delete [] mac;
   return(OK);
}

/****************************************************************************************
   Core routine to get the first MAC address for the ethernet board
****************************************************************************************/

unsigned short* GetFirstMACAddress(int &length)
{
   DWORD dwBufLen = 0;
   DWORD dwStatus = GetAdaptersInfo(NULL,&dwBufLen); 
   if(dwStatus == ERROR_NO_DATA)
     return(NULL);

   IP_ADAPTER_INFO *AdapterInfo = new IP_ADAPTER_INFO[dwBufLen];
   IP_ADAPTER_INFO *Adapter;

   if((dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen)) == NO_ERROR)
   {
      Adapter = AdapterInfo;
      while(Adapter)
      {
         if(Adapter->Type == MIB_IF_TYPE_ETHERNET)
            break;
         Adapter = Adapter->Next;
      }
   }
   else
   {
      delete [] AdapterInfo;
      return(NULL);
   }

   if(!Adapter || Adapter->Type != MIB_IF_TYPE_ETHERNET)
   {
      delete [] AdapterInfo;
      return(NULL);
   }

  if(dwStatus == NO_ERROR)
  {
     unsigned short *address = new unsigned short[Adapter->AddressLength];

     for(int i = 0; i < Adapter->AddressLength; i++)
     {
        address[i] = Adapter->Address[i];
     }

     length = Adapter->AddressLength;
     delete [] AdapterInfo;
     return(address);
  }
  else
  {
     delete [] AdapterInfo;
     return(NULL);
  }

  delete [] AdapterInfo;
  return(NULL);
}

/****************************************************************************************
   Core routine to get the MAC address for the ethernet board
   This version returns all suitable addresses not just the first
****************************************************************************************/

unsigned short* GetMACAddresses(int &length, int &nr)
{
   DWORD dwBufLen = 0;
   DWORD dwStatus = GetAdaptersInfo(NULL,&dwBufLen); 
   if(dwStatus == ERROR_NO_DATA)
     return(NULL);

   IP_ADAPTER_INFO *AdapterInfo = new IP_ADAPTER_INFO[dwBufLen];
   IP_ADAPTER_INFO *Adapter;

// Count the number of ethernet adapters
   int nrAdapters = 0;
   int adrsLength;
   if((dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen)) == NO_ERROR)
   {
      Adapter = AdapterInfo;
      while(Adapter)
      {
         if(Adapter->Type == MIB_IF_TYPE_ETHERNET)
         {
            nrAdapters++;
            adrsLength = Adapter->AddressLength;
         }
         Adapter = Adapter->Next;
      }
   }
   else
   {
      delete [] AdapterInfo;
      return(NULL);
   }

   if(nrAdapters == 0)
   {
      delete [] AdapterInfo;
      return(NULL);
   }

// Allocate space for all adapter addresses
   unsigned short *address = new unsigned short[adrsLength*nrAdapters];

// Loop again this time recording MAC addresses
   dwStatus = GetAdaptersInfo(NULL,&dwBufLen); 
   int cnt = 0;
   if((dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen)) == NO_ERROR)
   {
      Adapter = AdapterInfo;
      while(Adapter)
      {
         if(Adapter->Type == MIB_IF_TYPE_ETHERNET)
         {
             if(Adapter->AddressLength == adrsLength)
             {
                for(int i = 0; i < Adapter->AddressLength; i++)
                {
                   address[i+cnt*adrsLength] = Adapter->Address[i];
                }
                cnt++;
             }
         }
         Adapter = Adapter->Next;
      }
   }
   else
   {
      delete [] AdapterInfo;
      return(NULL);
   }


   delete [] AdapterInfo;
   length = adrsLength;
   nr = nrAdapters;
   return(address);
}

/****************************************************************************************
 Compare macAdrsToCheck with ethernet adapters in computer
 Returns
 0 => match found
 1 => no match
 2 => no ethernet adapter
 3 => internal error in GetAdaptersInfo
 4 => shouldn't happen
****************************************************************************************/


short CheckMacAddress(USHORT *macAdrsToCheck, USHORT macSz)
{
   bool ethernetFound = false;

   DWORD dwBufLen = 0;
   DWORD dwStatus = GetAdaptersInfo(NULL,&dwBufLen); 
   if(dwStatus == ERROR_NO_DATA) // No adapters
     return(MAC_NO_ETHERNET);

   IP_ADAPTER_INFO *AdapterInfo = new IP_ADAPTER_INFO[dwBufLen];
   IP_ADAPTER_INFO *Adapter;

   if((dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen)) == NO_ERROR)
   {
      Adapter = AdapterInfo;
      while(Adapter)
      {
         if(Adapter->Type == MIB_IF_TYPE_ETHERNET)
         {
            ethernetFound = true;
            USHORT address;
            int i;
            if(Adapter->AddressLength != macSz)
            {
               delete [] AdapterInfo;
               return(MAC_INTERNAL_ERROR);
            }
            for(i = 0; i < Adapter->AddressLength; i++)
            {
                if(macAdrsToCheck[i] != Adapter->Address[i])
                {
                   break;
                }
            }
            if(i == Adapter->AddressLength)  // Success match found
            {
               delete [] AdapterInfo;
               return(MAC_MATCH_FOUND);
            }
         }
         Adapter = Adapter->Next;
      }
   }
   else // Internal error
   {
      delete [] AdapterInfo;
      return(MAC_INTERNAL_ERROR);
   }

   if(!Adapter) // Can't find adapter
   {
      delete [] AdapterInfo;

      if(ethernetFound)
          return(MAC_NO_MATCH);

      return(MAC_NO_ETHERNET);
   }

  delete [] AdapterInfo;
  return(MAC_FAULT); // Internal error shouldn't happen
}

/****************************************************************************************
 Compare macAdrsToCheck with ethernet adapters in computer. In this version macAdrsToCheck
 is an array of addresses.
 Returns
 0 => match found
 1 => no match
 2 => no ethernet adapter
 3 => internal error in GetAdaptersInfo
 4 => shouldn't happen
****************************************************************************************/


short CheckMultipleMacAddress(USHORT **macAdrsToCheck, USHORT macSz, USHORT nrMacs)
{
   bool ethernetFound = false;

   DWORD dwBufLen = 0;
   DWORD dwStatus = GetAdaptersInfo(NULL,&dwBufLen); 
   if(dwStatus == ERROR_NO_DATA) // No adapters
     return(MAC_NO_ETHERNET);

   IP_ADAPTER_INFO *AdapterInfo = new IP_ADAPTER_INFO[dwBufLen];
   IP_ADAPTER_INFO *Adapter;

   if((dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen)) == NO_ERROR)
   {
      Adapter = AdapterInfo;
      while(Adapter)
      {
         if(Adapter->Type == MIB_IF_TYPE_ETHERNET)
         {
            ethernetFound = true;
            USHORT address;
            int i,j;
            if(Adapter->AddressLength != macSz)
            {
               delete [] AdapterInfo;
               return(MAC_INTERNAL_ERROR);
            }
            for(j = 0; j < nrMacs; j++) // Search through all MAC addresses
            {
               for(i = 0; i < Adapter->AddressLength; i++)
               {
                   if(macAdrsToCheck[j][i] != Adapter->Address[i])
                   {
                      break;
                   }
               }
               if(i == Adapter->AddressLength)  // Success match found
               {
                  delete [] AdapterInfo;
                  return(MAC_MATCH_FOUND);
               }
            }
         }
         Adapter = Adapter->Next;
      }
   }
   else // Internal error
   {
      delete [] AdapterInfo;
      return(MAC_INTERNAL_ERROR);
   }

   if(!Adapter) // Can't find adapter
   {
      delete [] AdapterInfo;

      if(ethernetFound)
          return(MAC_NO_MATCH);

      return(MAC_NO_ETHERNET);
   }

  delete [] AdapterInfo;
  return(MAC_FAULT); // Internal error shouldn't happen
}

//#define KEY_SID_PLACES	_T("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList")
//
//// Get User Security identifiers and user home directory - a work in progress.
//
//int GetSID(Interface* itfc ,char args[])
//{
//   HKEY hSubKey;
//
//   long result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_SID_PLACES, 0, KEY_READ, &hSubKey);
//   TextMessage("\n\n");
//   if(result == ERROR_SUCCESS)
//   {
//      for (DWORD Index=0; ; Index++)
//      {
//          char SubKeyName[255];
//          DWORD cName = 255;
//          LONG res = RegEnumKeyEx(hSubKey, Index, SubKeyName, &cName, 
//              NULL, NULL, NULL, NULL);
//          if (res != ERROR_SUCCESS)
//              break;
//
//          char Value[MAX_PATH];
//          DWORD cbData = MAX_PATH;
//          res = RegGetValue(hSubKey, SubKeyName, "ProfileImagePath",
//                                 RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, NULL, Value, &cbData);
//
//          if(res == ERROR_SUCCESS)
//             TextMessage("%s : %s\n",SubKeyName, Value);
//          else
//             TextMessage("Can't read profile path\n");
//      }
//
//      RegCloseKey(hSubKey);
//   }
//   return(0);
//}

/****************************************************************************************
   CLI interface to generate a hash for a vector
****************************************************************************************/

int HashFunction(Interface* itfc ,char args[])
{
   Variable var;
   short nrArgs;

   if((nrArgs = ArgScan(itfc,args,1,"str","e","v",&var)) < 0)
      return(nrArgs); 

   if(var.GetType() == MATRIX2D && var.GetDimY() == 1)
   {
      int sz = var.GetDimX();

      UCHAR* data = new UCHAR[sz];
      for(int i = 0; i < sz; i++)
      {
         data[i] = var.GetMatrix2D()[0][i];
      }
      size_t hash = GetHash(data,sz);
    
      itfc->retVar[1].MakeAndSetDouble(hash);
      itfc->nrRetValues = 1;
   }
   else
   {
      ErrorMessage("invalid data type for hashing");
      return(ERR);
   }

   return(OK);
}

/****************************************************************************************
   Core routine to calculate hash
****************************************************************************************/

size_t GetHash(UCHAR *data, int length)
{
   size_t hash;
   int bufNr = length/20;

   size_t hashTot = 0;
   int cnt = 0;
   for(int j = 0; j < bufNr; j++)
   {
      hash = 0;
      for(int i = 0; i < 20; i++)
      {
         hash = hash << 1 ^ data[cnt++];
      }
      hashTot += hash;
   }

   int remainder = length - bufNr*20;

   hash = 0;
   for(int i = 0; i < remainder; i++)
   {
      hash = hash << 1 ^ data[cnt++];
   }
   hashTot += hash;

   return(hashTot);
}


// Old attempt at using microsoft code to encrypt/decrypt license

//#include "wincrypt.h"
//#include "tchar.h"

//int TestCode(Interface* itfc ,char args[]);
//
//
//HCRYPTPROV hProv;
//HCRYPTKEY hKey;
//HCRYPTKEY hSessionKey;
//
//// Get a license key
//int GetKey(Interface* itfc ,char args[])
//{
//   DWORD dwResult;
//   short r;
//	DWORD cbBlob = 0;
//	BYTE *pbBlob = NULL;
//   char fileName[MAX_PATH];
//
//  // Extract filename  ************************   
//   if((r = ArgScan(itfc,args,0,"filename","e","s",fileName)) < 0)
//      return(r);
//
//  // Get Context 
//	if(!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0))
//	{
//		dwResult = GetLastError();
//		if(dwResult == NTE_BAD_KEYSET)
//		{
//			if (!CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET))
//			{
//				dwResult = GetLastError();
//				ErrorMessage("CryptAcquireContext() failed (Error [0x%x])", dwResult);
//				return(ERR);
//			}
//		}
//      else
//      {
//			dwResult = GetLastError();
//			ErrorMessage("CryptAcquireContext() failed (Error [0x%x])", dwResult);
//			return(ERR);
//		}
//	}
//
//  // Filename provided?
//   if(r == 1)
//   {
//	   FILE *fp = fopen(fileName, "rb");
//	   if (fp)
//      {
//         cbBlob = GetFileLength(fp);
//         pbBlob = new BYTE[cbBlob];
//		   fread(pbBlob, 1, cbBlob, fp);
//		   fclose(fp);
//	   } 
//      else
//      {
//			ErrorMessage("Can't read file '%s'", fileName);
//			return(ERR);
//	   }
//
//	   if(pbBlob)
//      {
//		   if (!CryptImportKey(hProv, pbBlob, cbBlob, 0, 0, &hSessionKey))
//		   {
//			   dwResult = GetLastError();
//		      ErrorMessage("CryptImportKey() failed (Err: [0x%x])",dwResult);
//		      return(ERR);
//		   }
//         delete [] pbBlob;
//         pbBlob = NULL;
//	   }
//      else
//      {
//			ErrorMessage("Can't read file '%s'", fileName);
//			return(ERR);
//	   }
//   }
//   else
//   {
//      if (!CryptImportKey(hProv, PrivateKeyWithExponentOfOne, sizeof(PrivateKeyWithExponentOfOne), 0, 0, &hKey))
//	   {
//		   dwResult = GetLastError();
//		   ErrorMessage("CryptImportKey() failed");
//		   return(ERR);
//	   }
//
//	   if(!CryptGenKey(hProv, CALG_RC4, CRYPT_EXPORTABLE, &hSessionKey))
//	   {
//		   dwResult = GetLastError();
//		   ErrorMessage("CryptGenKey() failed");
//		   return(ERR);
//	   }	
//   }
//   itfc->nrRetValues = 0;
//   return(OK);
//}
//
//int SaveKey(Interface* itfc ,char args[])
//{
//   char file[MAX_PATH];
//   short r;
//   DWORD dwResult;
//	DWORD cbBlob;
//	BYTE *pbBlob;
//
//// Extract filename  ************************   
//   if((r = ArgScan(itfc,args,1,"filename","e","s",file)) < 0)
//      return(r);
//
//	if (!CryptExportKey(hSessionKey, hKey, SIMPLEBLOB, 0, NULL, &cbBlob))
//	{
//		dwResult = GetLastError();
//		ErrorMessage("CryptExportKey() failed (Err: [0x%x])",dwResult);
//		return(ERR);
//	}
//
//	if ((pbBlob = (BYTE *) LocalAlloc(LMEM_ZEROINIT,cbBlob)) == NULL)
//	{
//		dwResult = ERROR_NOT_ENOUGH_MEMORY;
//		ErrorMessage("LocalAlloc() failed (Err: [0x%x])",dwResult);
//		return(ERR);
//	}
//	if (!CryptExportKey(hSessionKey, hKey, SIMPLEBLOB, 0, pbBlob, &cbBlob))
//	{
//		dwResult = GetLastError();
//		ErrorMessage("CryptExportKey() failed (Err: [0x%x])",dwResult);
//		return(ERR);
//	}
//
//	//if(hSessionKey != 0) 
//	//	CryptDestroyKey(hSessionKey);
//
//	//hSessionKey = 0;
//
//	FILE *fp = fopen(file, "w+b");
//	if (fp)
//   {
//		fwrite(pbBlob, 1, cbBlob, fp);
//		fclose(fp);
//	}
//
//	if (pbBlob)
//		LocalFree(pbBlob);
//
//	//if(hKey != 0) CryptDestroyKey(hKey);
//	//if(hProv != 0) CryptReleaseContext(hProv, 0);
//   return(OK);
//}
//
//// Encrypt a string
//int Encrypt(Interface* itfc ,char args[])
//{
//   char str[MAX_STR];
//   DWORD dwResult;
//   short r;
//
//// Extract data  ************************   
//   if((r = ArgScan(itfc,args,1,"string to encrypt","e","s",str)) < 0)
//      return(r);
//
//   DWORD length = strlen(str);
//
//   if (!CryptEncrypt(hSessionKey, 0, TRUE, 0, (BYTE*)str, &length, length))
//	{
//		dwResult = GetLastError();
//		ErrorMessage("CryptEncrypt() failed (Err: [0x%x]",dwResult);
//		return(ERR);
//	}
//   itfc->retVar[1].MakeAndSetString(str);
//   itfc->nrRetValues = 1;
//   return(OK);
//}
//
//// Decrypt a string
//int Decrypt(Interface* itfc ,char args[])
//{
//   char str[MAX_STR];
//   DWORD dwResult;
//   short r;
//
//// Extract data  ************************   
//   if((r = ArgScan(itfc,args,1,"string to decrypt","e","s",str)) < 0)
//      return(r);
//
//   DWORD length = strlen(str);
//
//	if (!CryptDecrypt(hSessionKey, 0, TRUE, 0, (BYTE*)str, &length))
//	{
//		dwResult = GetLastError();
//		ErrorMessage("CryptDecrypt() failed (Err: [0x%x]",dwResult);
//		return(ERR);
//	}
//
//   itfc->retVar[1].MakeAndSetString(str);
//   itfc->nrRetValues = 1;
//   return(OK);
//}
//
//
//// FUNCTIONS
//int KeysTest(char* strPublicKeyFile, char* strPrivateKeyFile);
//int EncryptTest(char* strPublicKeyFile, char* strPlainFile, char* strEncryptedFile);
//int DecryptTest(char* strPrivateKeyFile, char* strEncryptedFile, char* strPlainFile);
//
//// Main
//int TestCode(Interface* itfc ,char args[])
//{
//   short r;
//   CText mode,txt1,txt2,txt3;
//   int iResult = 0;
//
//     // Extract filename  ************************   
//   if((r = ArgScan(itfc,args,0,"filename","eeee","tttt",&mode,&txt1,&txt2,&txt3)) < 0)
//      return(r);
//
//      if ((r == 3) && (mode == "k"))
//      {
//            // Generate a new key pair
//            iResult = KeysTest(txt1.Str(),txt2.Str());
//      }
//      else if ((r == 4) && (mode == "e"))
//      {
//            // Encrypt
//            iResult = EncryptTest(txt1.Str(),txt2.Str(),txt3.Str());
//      }
//      else if ((r == 4) && (mode == "d"))
//      {
//            // Decrypt
//            iResult = DecryptTest(txt1.Str(),txt2.Str(),txt3.Str());
//      }
//      else 
//      {
//            // Show usage
//            TextMessage(("\n\nUsage:\n"));
//            TextMessage(("   - New key pair: EncryptDecrypt k public_key_file private_key_file\n"));
//            TextMessage(("   - Encrypt:      EncryptDecrypt e public_key_file plain_file encrypted_file\n"));
//            TextMessage(("   - Decrypt:      EncryptDecrypt d private_key_file encrypted_file plain_file\n"));
//            iResult = 1;
//      }
//
//      return(OK);
//}
//// End of Main
//
//// Keys
//int KeysTest(char* strPublicKeyFile, char* strPrivateKeyFile)
//{
//      // Variables
//      HCRYPTPROV hCryptProv = NULL;
//      HCRYPTKEY hKey = NULL;
//      DWORD dwPublicKeyLen = 0;
//      DWORD dwPrivateKeyLen = 0;
//      BYTE* pbPublicKey = NULL;
//      BYTE* pbPrivateKey = NULL;
//      HANDLE hPublicKeyFile = NULL;
//      HANDLE hPrivateKeyFile = NULL;
//      DWORD lpNumberOfBytesWritten = 0;
//
//      __try 
//      {
//            // Acquire access to key container
//            TextMessage(("CryptAcquireContext...\n"));
//            if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) 
//            {
//                  // Error
//                  TextMessage(("CryptAcquireContext error 0x%x\n"), GetLastError());
//
//                  // Try to create a new key container
//                  if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET))
//                  {
//                        // Error
//                        TextMessage(("CryptAcquireContext error 0x%x\n"), GetLastError());
//                        return 1;
//                  }
//            }
//
//            // Generate new key pair
//            TextMessage(("CryptGenKey...\n"));
//            if (!CryptGenKey(hCryptProv, AT_KEYEXCHANGE,  CRYPT_ARCHIVABLE | CRYPT_EXPORTABLE, &hKey))
//            {
//                  // Error
//                  TextMessage(("CryptGenKey error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get public key size
//            TextMessage(("CryptExportKey...\n"));
//            if (!CryptExportKey(hKey, NULL, PUBLICKEYBLOB, 0, NULL, &dwPublicKeyLen))
//            {
//                  // Error
//                  TextMessage(("CryptExportKey error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a buffer for the public key
//            TextMessage(("malloc...\n"));
//            if (!(pbPublicKey = (BYTE *)malloc(dwPublicKeyLen)))
//            {
//                  // Error
//                  TextMessage(("malloc error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get public key
//            TextMessage(("CryptExportKey...\n"));
//            if (!CryptExportKey(hKey, NULL, PUBLICKEYBLOB, 0, pbPublicKey, &dwPublicKeyLen))
//            {
//                  // Error
//                  TextMessage(("CryptExportKey error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get private key size
//            TextMessage(("CryptExportKey...\n"));
//            if (!CryptExportKey(hKey, NULL, PRIVATEKEYBLOB, 0, NULL, &dwPrivateKeyLen))
//            {
//                  // Error
//                  TextMessage(("CryptExportKey error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a buffer for the private key
//            TextMessage(("malloc...\n"));
//            if (!(pbPrivateKey = (BYTE *)malloc(dwPrivateKeyLen)))
//            {
//                  // Error
//                  TextMessage(("malloc error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get private key
//            TextMessage(("CryptExportKey...\n"));
//            if (!CryptExportKey(hKey, NULL, PRIVATEKEYBLOB, 0, pbPrivateKey, &dwPrivateKeyLen))
//            {
//                  // Error
//                  TextMessage(("CryptExportKey error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a file to save the public key
//            TextMessage(("CreateFile...\n"));
//            if ((hPublicKeyFile = CreateFile(
//            strPublicKeyFile,
//            GENERIC_WRITE,
//            0,
//            NULL,
//            CREATE_ALWAYS,
//            FILE_ATTRIBUTE_NORMAL,
//            NULL
//            )) == INVALID_HANDLE_VALUE)
//            {
//                  // Error
//                  TextMessage(("CreateFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Write the public key to the file
//            TextMessage(("WriteFile...\n"));
//            if (!WriteFile(
//                  hPublicKeyFile, 
//                  (LPCVOID)pbPublicKey, 
//                  dwPublicKeyLen, 
//                  &lpNumberOfBytesWritten, 
//                  NULL
//            )) 
//            {
//                  // Error
//                  TextMessage(("WriteFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a file to save the private key
//            TextMessage(("CreateFile...\n"));
//            if ((hPrivateKeyFile = CreateFile(
//            strPrivateKeyFile,
//            GENERIC_WRITE,
//            0,
//            NULL,
//            CREATE_ALWAYS,
//            FILE_ATTRIBUTE_NORMAL,
//            NULL
//            )) == INVALID_HANDLE_VALUE)
//            {
//                  // Error
//                  TextMessage(("CreateFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Write the private key to the file
//            TextMessage(("WriteFile...\n"));
//            if (!WriteFile(
//                  hPrivateKeyFile, 
//                  (LPCVOID)pbPrivateKey, 
//                  dwPrivateKeyLen, 
//                  &lpNumberOfBytesWritten, 
//                  NULL
//            )) 
//            {
//                  // Error
//                  TextMessage(("WriteFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            return 0;
//      }
//      __finally
//      {
//            // Clean up       
//            if (!pbPublicKey) {
//                  TextMessage(("free...\n"));
//                  free(pbPublicKey);
//            }           
//            if (!pbPrivateKey) {
//                  TextMessage(("free...\n"));
//                  free(pbPrivateKey);
//            }           
//            if (hPublicKeyFile) {
//                  TextMessage(("CloseHandle...\n"));
//                  CloseHandle(hPublicKeyFile);
//            }           
//            if (hPrivateKeyFile) {
//                  TextMessage(("CloseHandle...\n"));
//                  CloseHandle(hPrivateKeyFile);
//            }
//            if (hKey) {
//                  TextMessage(("CryptDestroyKey...\n"));
//                  CryptDestroyKey(hKey);
//            }           
//            if (hCryptProv) {
//                  TextMessage(("CryptReleaseContext...\n"));
//                  CryptReleaseContext(hCryptProv, 0);
//            }
//      }
//}
//// End of Keys
//
//// Encrypt
//int EncryptTest(char* strPublicKeyFile, char* strPlainFile, char* strEncryptedFile)
//{
//      // Variables
//      HCRYPTPROV hCryptProv = NULL;
//      HCRYPTKEY hKey = NULL;
//      DWORD dwPublicKeyLen = 0;
//      DWORD dwDataLen = 0;
//      DWORD dwEncryptedLen = 0;
//      BYTE* pbPublicKey = NULL;
//      BYTE* pbData = NULL;
//      HANDLE hPublicKeyFile = NULL;
//      HANDLE hEncryptedFile = NULL;
//      HANDLE hPlainFile = NULL;
//      DWORD lpNumberOfBytesWritten = 0;
//
//      __try
//      {
//            // Acquire access to key container
//            TextMessage(("CryptAcquireContext...\n"));
//            if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) 
//            {
//                  // Error
//                  TextMessage(("CryptAcquireContext error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Open public key file
//            TextMessage(("CreateFile...\n"));
//            if ((hPublicKeyFile = CreateFile(
//            strPublicKeyFile,
//            GENERIC_READ,
//            FILE_SHARE_READ,
//            NULL,
//            OPEN_EXISTING,
//            FILE_FLAG_SEQUENTIAL_SCAN,
//            NULL
//            )) == INVALID_HANDLE_VALUE)
//            {
//                  // Error
//                  TextMessage(("CreateFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get file size
//            TextMessage(("GetFileSize...\n"));
//            if ((dwPublicKeyLen = GetFileSize(hPublicKeyFile, NULL)) == INVALID_FILE_SIZE)
//            {
//                  // Error
//                  TextMessage(("GetFileSize error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a buffer for the public key
//            TextMessage(("malloc...\n"));
//            if (!(pbPublicKey = (BYTE *)malloc(dwPublicKeyLen)))
//            {
//                  // Error
//                  TextMessage(("malloc error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Read public key
//            TextMessage(("ReadFile...\n"));
//            if (!ReadFile(hPublicKeyFile, pbPublicKey, dwPublicKeyLen, &dwPublicKeyLen, NULL))
//            {
//                  // Error
//                  TextMessage(("ReadFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Import public key
//            TextMessage(("CryptImportKey...\n"));
//            if (!CryptImportKey(hCryptProv, pbPublicKey, dwPublicKeyLen, 0, 0, &hKey))
//            {
//                  // Error
//                  TextMessage(("CryptImportKey error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Open plain text file
//            TextMessage(("CreateFile...\n"));
//            if ((hPlainFile = CreateFile(
//            strPlainFile,
//            GENERIC_READ,
//            FILE_SHARE_READ,
//            NULL,
//            OPEN_EXISTING,
//            FILE_FLAG_SEQUENTIAL_SCAN,
//            NULL
//            )) == INVALID_HANDLE_VALUE)
//            {
//                  // Error
//                  TextMessage(("CreateFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get file size
//            TextMessage(("GetFileSize...\n"));
//            if ((dwDataLen = GetFileSize(hPlainFile, NULL)) == INVALID_FILE_SIZE)
//            {
//                  // Error
//                  TextMessage(("GetFileSize error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a buffer for the plain text
//            TextMessage(("malloc...\n"));
//            if (!(pbData = (BYTE *)malloc(dwDataLen)))
//            {
//                  // Error
//                  TextMessage(("malloc error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Read plain text
//            TextMessage(("ReadFile...\n"));
//            if (!ReadFile(hPlainFile, pbData, dwDataLen, &dwDataLen, NULL))
//            {
//                  // Error
//                  TextMessage(("ReadFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get lenght for encrypted data
//            if (!CryptEncrypt(hKey, NULL, TRUE, 0, NULL, &dwEncryptedLen, 0))
//            {
//                  // Error
//                  TextMessage(("CryptEncrypt error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a buffer for encrypted data
//            TextMessage(("malloc...\n"));
//            if (!(pbData = (BYTE *)realloc(pbData, dwEncryptedLen)))
//            {
//                  // Error
//                  TextMessage(("malloc error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Encrypt data
//            if (!CryptEncrypt(hKey, NULL, TRUE, 0, pbData, &dwDataLen, dwEncryptedLen))
//            {
//                  // Error
//                  TextMessage(("CryptEncrypt error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a file to save the encrypted data
//            TextMessage(("CreateFile...\n"));
//            if ((hEncryptedFile = CreateFile(
//            strEncryptedFile,
//            GENERIC_WRITE,
//            0,
//            NULL,
//            CREATE_ALWAYS,
//            FILE_ATTRIBUTE_NORMAL,
//            NULL
//            )) == INVALID_HANDLE_VALUE)
//            {
//                  // Error
//                  TextMessage(("CreateFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Write the public key to the file
//            TextMessage(("WriteFile...\n"));
//            if (!WriteFile(
//                  hEncryptedFile, 
//                  (LPCVOID)pbData, 
//                  dwDataLen, 
//                  &lpNumberOfBytesWritten, 
//                  NULL
//            )) 
//            {
//                  // Error
//                  TextMessage(("WriteFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            return 0;
//      }
//      __finally
//      {
//            // Clean up
//            if (!pbPublicKey) {
//                  TextMessage(("free...\n"));
//                  free(pbPublicKey);
//            }
//            if (!pbData) {
//                  TextMessage(("free...\n"));
//                  free(pbData);
//            }
//            if (hPublicKeyFile) {
//                  TextMessage(("CloseHandle...\n"));
//                  CloseHandle(hPublicKeyFile);
//            }
//            if (hPlainFile) {
//                  TextMessage(("CloseHandle...\n"));
//                  CloseHandle(hPlainFile);
//            }
//            if (hEncryptedFile) {
//                  TextMessage(("CloseHandle...\n"));
//                  CloseHandle(hEncryptedFile);
//            }
//            if (hKey) {
//                  TextMessage(("CryptDestroyKey...\n"));
//                  CryptDestroyKey(hKey);
//            }           
//            if (hCryptProv) {
//                  TextMessage(("CryptReleaseContext...\n"));
//                  CryptReleaseContext(hCryptProv, 0);
//            }
//      }
//}
//// End of Encrypt
//
//// Decrypt
//int DecryptTest(char* strPrivateKeyFile, char* strEncryptedFile, char* strPlainFile)
//{
//      // Variables
//      HCRYPTPROV hCryptProv = NULL;
//      HCRYPTKEY hKey = NULL;
//      DWORD dwPrivateKeyLen = 0;
//      DWORD dwDataLen = 0;
//      BYTE* pbPrivateKey = NULL;
//      BYTE* pbData = NULL;
//      HANDLE hPrivateKeyFile = NULL;
//      HANDLE hEncryptedFile = NULL;
//      HANDLE hPlainFile = NULL;
//      DWORD lpNumberOfBytesWritten = 0;
//
//      __try 
//      {
//            // Acquire access to key container
//            TextMessage(("CryptAcquireContext...\n"));
//
//            if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) 
//            {
//                  // Error
//                  TextMessage(("CryptAcquireContext error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Open private key file
//            TextMessage(("CreateFile...\n"));
//            if ((hPrivateKeyFile = CreateFile(
//            strPrivateKeyFile,
//            GENERIC_READ,
//            FILE_SHARE_READ,
//            NULL,
//            OPEN_EXISTING,
//            FILE_FLAG_SEQUENTIAL_SCAN,
//            NULL
//            )) == INVALID_HANDLE_VALUE)
//            {
//                  // Error
//                  TextMessage(("CreateFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get file size
//            TextMessage(("GetFileSize...\n"));
//            if ((dwPrivateKeyLen = GetFileSize(hPrivateKeyFile, NULL)) == INVALID_FILE_SIZE)
//            {
//                  // Error
//                  TextMessage(("GetFileSize error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a buffer for the private key
//            TextMessage(("malloc...\n"));
//            if (!(pbPrivateKey = (BYTE *)malloc(dwPrivateKeyLen)))
//            {
//                  // Error
//                  TextMessage(("malloc error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Read private key
//            TextMessage(("ReadFile...\n"));
//            if (!ReadFile(hPrivateKeyFile, pbPrivateKey, dwPrivateKeyLen, &dwPrivateKeyLen, NULL))
//            {
//                  // Error
//                  TextMessage(("ReadFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Import private key
//            TextMessage(("CryptImportKey...\n"));
//            if (!CryptImportKey(hCryptProv, pbPrivateKey, dwPrivateKeyLen, 0, 0, &hKey))
//            {
//                  // Error
//                  TextMessage(("CryptImportKey error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Open encrypted file
//            TextMessage(("CreateFile...\n"));
//            if ((hEncryptedFile = CreateFile(
//            strEncryptedFile,
//            GENERIC_READ,
//            FILE_SHARE_READ,
//            NULL,
//            OPEN_EXISTING,
//            FILE_FLAG_SEQUENTIAL_SCAN,
//            NULL
//            )) == INVALID_HANDLE_VALUE)
//            {
//                  // Error
//                  TextMessage(("CreateFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get file size
//            TextMessage(("GetFileSize...\n"));
//            if ((dwDataLen = GetFileSize(hEncryptedFile, NULL)) == INVALID_FILE_SIZE)
//            {
//                  // Error
//                  TextMessage(("GetFileSize error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a buffer for the encrypted data
//            TextMessage(("malloc...\n"));
//            if (!(pbData = (BYTE *)malloc(dwDataLen)))
//            {
//                  // Error
//                  TextMessage(("malloc error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Read encrypted data
//            TextMessage(("ReadFile...\n"));
//            if (!ReadFile(hEncryptedFile, pbData, dwDataLen, &dwDataLen, NULL))
//            {
//                  // Error
//                  TextMessage(("ReadFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Get lenght for plain text
//            if (!CryptDecrypt(hKey, NULL, TRUE, 0, pbData, &dwDataLen))
//            {
//                  // Error
//                  TextMessage(("CryptDecrypt error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Create a file to save the plain text
//            TextMessage(("CreateFile...\n"));
//            if ((hPlainFile = CreateFile(
//            strPlainFile,
//            GENERIC_WRITE,
//            0,
//            NULL,
//            CREATE_ALWAYS,
//            FILE_ATTRIBUTE_NORMAL,
//            NULL
//            )) == INVALID_HANDLE_VALUE)
//            {
//                  // Error
//                  TextMessage(("CreateFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            // Write the plain text the file
//            TextMessage(("WriteFile...\n"));
//            if (!WriteFile(
//                  hPlainFile, 
//                  (LPCVOID)pbData, 
//                  dwDataLen, 
//                  &lpNumberOfBytesWritten, 
//                  NULL
//            )) 
//            {
//                  // Error
//                  TextMessage(("WriteFile error 0x%x\n"), GetLastError());
//                  return 1;
//            }
//
//            return 0;
//      }
//      __finally
//      {
//            // Clean up       
//            if (!pbPrivateKey) {
//                  TextMessage(("free...\n"));
//                  free(pbPrivateKey);
//            }                       
//            if (!pbData) {
//                  TextMessage(("free...\n"));
//                  free(pbData);
//            }           
//            if (hPrivateKeyFile) {
//                  TextMessage(("CloseHandle...\n"));
//                  CloseHandle(hPrivateKeyFile);
//            }
//            if (hEncryptedFile) {
//                  TextMessage(("CloseHandle...\n"));
//                  CloseHandle(hEncryptedFile);
//            }
//            if (hPlainFile) {
//                  TextMessage(("CloseHandle...\n"));
//                  CloseHandle(hPlainFile);
//            }
//            if (hKey) {
//                  TextMessage(("CryptDestroyKey...\n"));
//                  CryptDestroyKey(hKey);
//            }           
//            if (hCryptProv) {
//                  TextMessage(("CryptReleaseContext...\n"));
//                  CryptReleaseContext(hCryptProv, 0);
//            }
//      }
//}
//// End of Decrypt
