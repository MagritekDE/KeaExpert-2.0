
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


// 2D Inversion macro based on FISTA algorithm

char FISTAMacro[] =
"procedure(fista,Y,K2,K1,S,alpha,nrIter,update,plotObj,progressObj);\
   if(nrArgs < 7);\
      update=0;\
   endif;\
   KK1 = K1'*K1;\
   KK2 = K2'*K2;\
   KY12 = K1'*Y*K2;\
   L = 2*trac(KK1)*trac(KK2);\
   SY = S;\
   tt = 1d;\
   fac1 = (L-2d*alpha)/L;\
   fac2 = 2d/L;\
   for(iter = 1 to nrIter);\
     term2 = KY12-KK1*SY*KK2;\
     Snew = fac1*SY + fac2*term2;\
     Snew = (Snew >= 0d).*Snew;\
     ttnew = 0.5d*(1d + sqrt(1d+4d*tt^2d));\
     trat = (tt-1d)/ttnew;\
     SY = Snew + trat * (Snew-S);\
     tt = ttnew;\
     S = Snew;\
     if((iter % 100) == 0);\
         if((update&1)>0 & nrArgs >= 8);\
           plotObj->draw(\"false\");\
           plotObj->image(S);\
           plotObj->title(\"Iteration = $iter$\");\
           plotObj->draw(\"true\");\
         endif;\
         if((update&2)>0 & nrArgs == 9);\
           progressObj->value(100*iter/nrIter);\
         endif;\
      endif;\
    next(iter);\
;\
;\
endproc(S);";


int FISTA2D(Interface* itfc ,char args[]);


int FISTA2D(Interface* itfc ,char args[])
{
   Variable result;
   CArg carg;
   CText argument;

  if(!HaveLicense("fista"))
  {
      MessageBox(NULL,"You don't have a valid license for the FISTA module.\n\nPlease contact Magritek support to obtain one.\n\nsupport@magritek.com","Invalid license",MB_OK | MB_ICONEXCLAMATION);
      ErrorMessage("no license for the FISTA module");
      return(ERR);
  }

// Get arguments from user ************* 
   short nrArgs = carg.Count(args);

   if(nrArgs == 1)
   {
      if(!strcmp(carg.Extract(1),"description"))
      {
         itfc->nrRetValues = 1;
         itfc->retVar[1].MakeAndSetString("fista : V1.0 : FISTA 2D Inverse Laplace : Author Paul Teal, VUW");
         return(OK);
      }
   }

   if(nrArgs < 6)
   {
      ErrorMessage("expected 6-9 arguments: nmr_data, kernel_x, kernel_y, spectral_guess, smoothing, nr_iterations, display_mode, plot_region, progress_obj"); 
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
   short r = ProcessMacroStr(itfc, false, FISTAMacro);


   return(OK);
}