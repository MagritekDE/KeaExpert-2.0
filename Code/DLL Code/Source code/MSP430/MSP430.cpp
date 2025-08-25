#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <stdlib.h>
#include <vector>

using namespace std;
using std::vector;


// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short ReadMSP430(DLLParameters*,char *args);


FILE *gFile = NULL;
CText gFileName="";

char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list

/*******************************************************************************
Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
  short r = RETURN_FROM_DLL;


  if(!strcmp(command,"readmsp430"))           r = ReadMSP430(dpar,parameters);      


  return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
  TextMessage("\n\n   MSP430 DLL module\n\n");
  TextMessage("   readmsp430 ........... reads the ascii flash file for msp430 and return matrices\n");
 }

/*******************************************************************************
Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
  syntax[0] = '\0';
  if(!strcmp(cmd,"readmsp430"))           strcpy(syntax,"(NUM adrs, MAT data)  = readmsp430(STR filename, INT offset)");
 
  if(syntax[0] == '\0')
    return(false);
  return(true);
}

/****************************************************************************
  
*****************************************************************************/

short ReadMSP430(DLLParameters* par, char *args)
{
   short nrArgs;
   long offset = 0;
   FILE* fp;
   CText fileName;
   CText mode = "start";
   CText adrsStr;
   CText hexStr;
   long adrs,value;
   bool generatingStr = false;
   vector<float> data;

   // Argument can be a constant float or matrix or a variable containing these
   if((nrArgs = ArgScan(par->itfc,args,1,"data to be opened","ee","td",&fileName, &offset)) < 0)
      return(nrArgs); 


   if (!(fp = fopen(fileName.Str(), "r")))
   {
      ErrorMessage("unable to open file %s", gFileName.Str());
      return(ERR);
   }

   if (offset > 0)
   {
      if (fseek(fp, offset, 0) != 0)
      {
         ErrorMessage("invalid offset %d", offset);
         return(ERR);
      }
   }

   data.clear();

   char c;
   while ((c = fgetc(fp)) != EOF)
   {
      if (mode == "start")
      {
         if (c == '@')
         {
            mode = "readAdrs";
            continue;
         }
      }

      else if (mode == "readAdrs")
      {
         if (c != ' ' && c != '\r' && c != '\n')
         {
            adrsStr.Append(c);
         }
         else
         {
            adrsStr.Append('\0');
            sscanf(adrsStr.Str(), "%lX", &adrs);
            adrsStr = "";
            mode = "readHex";
         }
      }

      else if (mode == "readHex")
      {
         if (c == '@' || c == 'q')
         {
            int sz = data.size();
            float** array = MakeMatrix2D(sz, 1);
            for (int i = 0; i < sz; i++)
               array[0][i] = data[i];
            long pos = ftell(fp);
            par->retVar[1].MakeAndSetFloat(adrs);
            par->retVar[2].AssignMatrix2D(array, sz, 1);
            if (c == 'q')
               par->retVar[3].MakeAndSetFloat(-1);
            else
               par->retVar[3].MakeAndSetFloat(pos-1);
            par->nrRetVar = 3;
            fclose(fp);
            return(OK);
         }
         else if (c != ' ' && c != '\r' && c != '\n')
         {
            hexStr.Append(c);
            generatingStr = true;
         }
         else
         {
            if (generatingStr)
            {
               hexStr.Append('\0');
               sscanf(hexStr.Str(), "%lX", &value);
               data.push_back(value);
               //  TextMessage("Data = %s\n", hexStr.Str());
               hexStr = "";
               generatingStr = false;
            }
         }
      }
   }

   ErrorMessage("invalid file format");
   fclose(fp);
   return(ERR);
}
    