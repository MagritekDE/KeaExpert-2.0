#include "stdafx.h"

#include "../Global files/includesDLL.h"
//#include "stdlib.h"

#define VERSION 1.0

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short GetLicenseInfo(DLLParameters* par,char arg[]);
short GetMACAdrs(DLLParameters* par, char arg[]);

// Encrypted MAC address for this computer
unsigned short macAdrs[] = {0xC3EF , 0xA6EE,  0x89BB, 0x5F06,  0xF8C6,  0xC7BF };
// List of encrypted licensed commands
unsigned short licenseLst[][6] = {{0x77A1,0x4DA9,0x5100,0x4C90,0x4FE1,0x6184},
                                 {0x5469,0x5CA4,0x4731,0x4A64,0x4C90,0x0000}};
                      /*"lexus",
                      "lawsonhansonil2d",
                      "maxentropyil2d"};*/

const double serialNumber = 123456789;

short nrItems = sizeof(licenseLst)/sizeof(float);

// Extension procedure to add commands to Prospa 

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

   if(!strcmp(command,"license"))   r = GetLicenseInfo(dpar,parameters);      

   return(r);
}

// Don't list any commands here - they are hidden

EXPORT void  ListCommands(void)
{

}

/*******************************************************************************
    Extension procedure to return syntax in DLL 
********************************************************************************/


EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(syntax[0] == '\0')
      return(false);
   return(true);
}

short GetLicenseInfo(DLLParameters* par, char arg[])
{
   int sz = sizeof(macAdrs)/sizeof(short);
   float *temp = new float[sz];
   for(int i = 0; i < sz; i++)
      temp[i] = macAdrs[i];
   par->retVar[1].MakeMatrix2DFromVector(temp,sz,1);
   par->retVar[2].MakeAndSetDouble(serialNumber);
   par->retVar[3].MakeAndSetList(licenseLst,nrItems);
   par->nrRetVar = 3;
   delete [] temp;
   return(OK);
}

