#include "stdafx.h"
#include "globals.h"
#include "CArg.h"
#include "CText.h"
#include "license.h"
#include "process.h"
#include "evaluate.h"
#include "interface.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"


// 2D Inversion macro
// Last update 26 Sept 13
// 1. Introduced TSVD to factor of 0.001 max singular values (typically 10)
// 2. Separated out individual matrices in IL call.
// 3. Replace diagonal smoothing matrix with single value to save memory
//
char LH2DMacro[] =
"procedure(invert2d,data,Ex,Ey,taux,tauy,smoothing);\
   tauXSize = size(taux);\
   tauYSize = size(tauy);\
   tauSize = tauXSize*tauYSize;\
   (dataXSize,dataYSize) = size(data);\
   (Ux,Vx,Sx) = svd(Ex);\
   (Uy,Vy,Sy) = svd(Ey);\
    tFac = 1e-3;\
    q1 = size(Sx);\
    for(k = 0 to size(Sx)-1);\
        if(Sx[k,k] < Sx[0,0]*tFac) ;\
           q1 = k;\
           exitfor;\
        endif;\
    next(k);\
    Ux = Ux[[0:q1-1],:];\
    Sx = Sx[[0:q1-1],[0:q1-1]];\
    q2 = size(Sy);\
    for(k = 0 to size(Sy)-1);\
        if(Sy[k,k] < Sy[0,0]*tFac) ;\
           q2 = k;\
           exitfor;\
        endif;\
    next(k);\
    Uy = Uy[[0:q2-1],:];\
    Sy = Sy[[0:q2-1],[0:q2-1]];\
    ExRed = -Ux'*Ex;\
    EyRed = -Uy'*Ey;\
    dataRed = Uy'*data*Ux;\
    for(direction = 0 to 1);\
       if(direction == 0);\
          data = reshape(dataRed,1,q1*q2);\
          E = outer(EyRed,ExRed);\
       else;\
          data = reshape(dataRed',1,q1*q2);\
          E = outer(ExRed,EyRed);\
       endif;\
       if(direction == 0);\
          spectrum = lhil1d(data',E,smoothing,\":callBackNNLSx\");\
          spectrumh = reshape(spectrum,size(taux),size(tauy));\
       else;\
          spectrum = lhil1d(data',E,smoothing,\":callBackNNLSy\");\
          spectrumv = reshape(spectrum,size(tauy),size(taux));\
       endif;\
   next(k);\
   spectrum = trans(sqrt(spectrumh' .* spectrumv));\
endproc(spectrumh);";

int LawsonHansonNNLS2D(Interface* itfc ,char args[]);


int LawsonHansonNNLS2D(Interface* itfc ,char args[])
{
   Variable result;
   CArg carg;
   CText argument;

  if(!HaveLicense("lhil2d"))
  {
      MessageBox(NULL,"You don't have a valid license for the 2D Lawson and Hanson module.\n\nPlease contact Magritek support to obtain one.\n\nsupport@magritek.com","Invalid license",MB_OK | MB_ICONEXCLAMATION);
      ErrorMessage("no license for the 2D Lawson and Hanson module");
      return(ERR);
  }

// Get arguments from user ************* 
   short nrArgs = carg.Count(args);

   if(nrArgs == 1)
   {
      if(!strcmp(carg.Extract(1),"description"))
      {
         itfc->nrRetValues = 1;
         itfc->retVar[1].MakeAndSetString("lhil2d : V1.0 : Lawson and Hanson 2D Inverse Laplace : U.S. Pat. No. 6,462,542");
         return(OK);
      }
   }

   if(nrArgs != 6)
   {
      ErrorMessage("expected 6 arguments: data,Ex,Ey,taux,tauy,weighting"); 
      return(ERR);
   }

   for(long i = 1; i <= nrArgs; i++)
   {
      argument = carg.Extract(i);
         
	 	if(Evaluate(itfc,RESPECT_ALIAS,argument.Str(),&result) < 0) 
			return(ERR);  

		if(CopyVariable(itfc->argVar[i].var,&result,RESPECT_ALIAS) == ERR)
			return(ERR);
	}  
   itfc->nrProcArgs = nrArgs;

// Run the procedure and return any results
   short r = ProcessMacroStr(itfc, false, LH2DMacro);


   return(OK);
}