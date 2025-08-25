#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include "stdlib.h"

#define VERSION 3.0

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
extern short LawsonHansonNNLS(DLLParameters* par,char arg[]);
extern short Soften(DLLParameters* par,char arg[]);
extern short NoFunc(DLLParameters* par,char arg[]);
extern short NNLS_Version(DLLParameters* par,char arg[]);
short GetHelpFolder(DLLParameters* par,char arg[]);
extern short LawsonHansonNNLSFast(DLLParameters* par, char arg[]);

// Extension procedure to add commands to Prospa 

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

   if(!strcmp(command,"lhil1d"))               r = LawsonHansonNNLS(dpar,parameters);      
   else if(!strcmp(command,"nnlsversion"))     r = NNLS_Version(dpar,parameters);
   else if(!strcmp(command,"helpfolder"))      r = GetHelpFolder(dpar,parameters);  

   return(r);
}

// Extension procedure to list commands in DLL 

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   NNLS DLL module (V3.0)\n\n");
   TextMessage("   lhil1d ..... 1D Lawson and Hanson inverse laplace\n");

}

/*******************************************************************************
    Extension procedure to return syntax in DLL 
********************************************************************************/


EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"lhil1d"))  strcpy(syntax,"VEC spectrum = lhil1d(VEC data, MAT kernel, MAT/VEC/FLOAT smoothing [, STR callback])");


   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short GetHelpFolder(DLLParameters* par, char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\NNLS");
   par->nrRetVar = 1;
   return(OK);
}


/*******************************************************************************
   Return the version number 
*******************************************************************************/

short NNLS_Version(DLLParameters* par, char *args)
{
   par->retVar[1].MakeAndSetFloat((long)(VERSION*100+0.5));
   par->nrRetVar = 1;
   return(OK);
}
