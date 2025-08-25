#include "stdafx.h"
#include "../Global files/includesDLL.h"


// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);
IMPORT bool HaveLicense(char *cmd);

// Extension to test ...
short Lexus(DLLParameters*, char* parameters);
short LexusSQR(DLLParameters*, char* parameters);
short LexusSQC(DLLParameters*, char* parameters);

#include <sstream>

/*******************************************************************************
Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
  short r = RETURN_FROM_DLL;

  if(!strcmp(command,"lexus")) 
	  r = Lexus(dpar,parameters);
  else if (!strcmp(command,"lexus_sqr")) 
	  r = LexusSQR(dpar,parameters);
  else if (!strcmp(command,"lexus_sqc")) 
	  r = LexusSQC(dpar,parameters);

  return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
  TextMessage("\n\n   Lexus module V1.1\n\n");

  // Peter's extension
  TextMessage("   lexus     ............ Lexus solver K.f = g \n");
  TextMessage("   lexus_sqr ............ Lexus Set/Query Real parameter \n");
  TextMessage("   lexus_sqc ............ Lexus Set/Query Character parameter \n");
}

/*******************************************************************************
Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
  syntax[0] = '\0';

  if (!strcmp(cmd,"lexus"))  
	  strcpy(syntax,"Vector f = Lexus(Matrix K, Vector g)");
  else if (!strcmp(cmd,"lexus_sq"))  
	  strcpy(syntax,"lexus_sqr(sq, name, real value)");
  else if (!strcmp(cmd,"lexus_sq"))  
	  strcpy(syntax,"lexus_sqc(sq, name, char value)");

  if(syntax[0] == '\0')
    return(false);

  return(true);
}

// ================================================== PETER CODE ===================================
// The following should be in a header file onces we are sorted

// This is the 'C' way of specifying the same function as the Fortran 90 underneath

// deprecated
IMPORT void lxBRDw_sp_dllc(int ng, int nf, float* K, float* g, float* f, float* w, float* zeta, float* zetad, int* status);

// new one
IMPORT void lexus(int ng, int nf, float* K, float* g, float* f, int* status);

IMPORT void lexus_sqr(char*, char* , float*, int*);
IMPORT void lexus_sqc(char*, char* , char*, int*, int*);

// This is the interface that Prospa uses
int WrappedFortranBRD_SP(int ng, int nf, float** K, float**g, float** f);

// This is the Prospa entry point

short Lexus(DLLParameters* par, char *parameters)
{
  short nrArgs;
  Variable varK, varg;

  // Matrix and vector sizes and pointers to values         
  long nrows;
  long ncols;
  float** pK;
  float** pg;
  CArg carg;

  if(!HaveLicense("lexus"))
  {
      MessageBox(NULL,"You don't have a valid license for the Lexus module.\n\nPlease contact Magritek support to obtain one.\n\nsupport@magritek.com","Invalid license",MB_OK | MB_ICONEXCLAMATION);
      ErrorMessage("no license for the Lexus module");
      return(ERR);
  }

// Get arguments from user ************* 
   nrArgs = carg.Count(parameters);

   if(nrArgs == 1)
   {
      if(!strcmp(carg.Extract(1),"description"))
      {
         par->nrRetVar = 1;
         par->retVar[1].MakeAndSetString("lexus : V1.0 : 1D BRD Inverse Laplace : Author - Peter Aptaker, Laplacian");
         return(OK);
      }
   }

  // Extract arguments
  if((nrArgs = ArgScan(par->itfc,parameters,2,"K,g","ee","vv",&varK,&varg)) < 0)
    return(nrArgs);  

  // Test for correct variable type
  if(VarType(&varK) != MATRIX2D )
  {
    ErrorMessage("invalid data type passed - should be Matrix");
    return(ERR);
  }
  else
  {
    nrows = VarRowSize(&varK);
    ncols = VarColSize(&varK);
    pK = VarRealMatrix(&varK);
  }

  // Test for correct variable type
  if(VarType(&varg) != MATRIX2D )
  {
    ErrorMessage("invalid data type passed (should be column vector)");
    return(ERR);
  }
  else
  {
    if (nrows != VarRowSize(&varg) ) 
    {
      ErrorMessage("Vector nrows: %d != Matrix nrows: %d",VarRowSize(&varg),nrows);
      return (ERR);
    }
    if ( VarColSize(&varg) != 1 )
    {
      ErrorMessage("Vector has ncols: %d != 1",VarColSize(&varg));
      return (ERR);
    }
    pg = VarRealMatrix(&varg);
  }

  // Allocate space for the output vector - Matrix with 1 column   
  float** pResult = MakeMatrix2D(1,ncols);

  int status = WrappedFortranBRD_SP(nrows, ncols, pK, pg, pResult);
  if ( status != 0 )
  {
    ErrorMessage("BRD failed with error status = %d",status);
    return(ERR);
  }

  // Return result to the user - matrix with 1 column and nrows
  par->retVar[1].MakeAndLoadMatrix2D(pResult,1,ncols);

  // Free matrix
  FreeMatrix2D(pResult);

  par->nrRetVar = 1;

  return(OK);  	 
}

int WrappedFortranBRD_SP(int ng, int nf, float** k, float**g, float** f)
{
  // Ok - make a 'double precision' copy of all the data
  float ZETA  = 0.0;
  float ZETAD = 0.0;
  float W     = 0.01;

  // G first
  float* G = new float[ng];
  float* F = new float[nf];
  for( int i=0; i!=ng; ++i )
    G[i] = g[i][0];

  // Copy K
  float* K = new float[ ng* nf ];
  for( int r = 0; r!=ng; ++r )
    for( int c = 0; c!=nf; ++c )
      K[ c*ng + r ] = k[r][c];      // Note in C - K is numbered by column & then row, fotran - other way round

  // Call the Fortran
  int STATUS;
//  lxBRDw_sp_dllc(ng, nf, K, G, F, &W, &ZETA, &ZETAD, &STATUS);
  lexus(ng, nf, K, G, F, &STATUS);

  // Check the status
  if ( STATUS != 0 )
  {
     delete [] G;
     delete [] F;
     delete [] K;
     return STATUS;
  }

  // Copy result back into floats for Prospa
  for( int i=0; i!=nf; ++i )
    f[i][0] = F[i];

   delete [] G;
   delete [] F;
   delete [] K;

   return 0;
}

short LexusSQR(DLLParameters* par, char *parameters)
{
  short nrArgs;  
  Variable sq,name,rval;
  long i;

  // Copy passed array into a variable var
  if((nrArgs = ArgScan(par->itfc, parameters,3,"sq,name,rval","eee","vvv",&sq,&name,&rval)) < 0)
    return(nrArgs); 

  bool bad_args = false;
  if ( VarType(&sq) != UNQUOTED_STRING )
  {
	  bad_args = true;
	  TextMessage("\nFirst argument must be a string with values 'S' Set,'Q' Query or 'I' Initialise to default.");
  }
  if ( VarType(&name) != UNQUOTED_STRING )
  {
	  bad_args = true;
	  TextMessage("\nSecond argument must be a string name of the variable to set, query or initialise to default.");
  }
  if ( VarType(&rval) != FLOAT32 )
  {
	  bad_args = true;
	  TextMessage("\nThird argument must be a real value to be associated with the variable.");
  }
  if ( bad_args )
	  return false;

  char* pSQ   = sq.GetString();
  char* pName = name.GetString();
  float RVal  = rval.GetReal();

  char SQ[2]; SQ[0] = pSQ[0]; SQ[1] = 0;
  if ( SQ[0] != 'S' && SQ[0] != 'Q' && SQ[0] != 'I' )
  {
	  TextMessage("The first parameter must be either S - set, Q - query or I - initialize.\n");
	  return false;
  }
  char Name[50]; memset(Name,0,50); strcpy(Name,pName);

  int status;
  lexus_sqr(SQ,Name,&RVal,&status);

  // Return value - set may fail?
  par->retVar[1].MakeAndSetFloat(RVal);
  par->nrRetVar = 1;

  return (status == 0);
}

short LexusSQC(DLLParameters* par, char *parameters)
{
  short nrArgs;  
  Variable sq,name,cval;
  long i;

  // Copy passed array into a variable var
  if((nrArgs = ArgScan(par->itfc,parameters,3,"sq,name,cval","eee","vvv",&sq,&name,&cval)) < 0)
    return(nrArgs); 

  bool bad_args = false;
  if ( VarType(&sq) != UNQUOTED_STRING )
  {
	  bad_args = true; 
	  TextMessage("\nFirst argument must be a string with values 'S' Set,'Q' Query or 'I' Initialise to default.");
  }
  if ( VarType(&name) != UNQUOTED_STRING )
  {
	  bad_args = true;
	  TextMessage("\nSecond argument must be a string name of the variable to set, query or initialise to default.");
  }
  if ( VarType(&cval) != UNQUOTED_STRING )
  {
	  bad_args = true;
	  TextMessage("\nThird argument must be a string with the character value to be associated with the variable.");
  }
  if ( bad_args )
	  return false;

  char* pSQ   = sq.GetString();
  char* pName = name.GetString();
  char* pCVal = cval.GetString();

  char SQ[2]; SQ[0] = pSQ[0]; SQ[1] = 0;
  if ( SQ[0] != 'S' && SQ[0] != 'Q' && SQ[0] != 'I' )
  {
	  TextMessage("The first parameter must be either S - set, Q - query or I - initialize.\n");
	  return false;
  }

  // Always expand name to 4 characters
  char Name[4] ; memset(Name,0,4) ; strcpy(Name,pName);

  int status;
  int clen = strlen(pCVal);
  lexus_sqc(SQ,Name,pCVal,&clen,&status);

  par->retVar[1].MakeAndSetString(pCVal);
  par->nrRetVar = 1;

  return (status == 0);
}
