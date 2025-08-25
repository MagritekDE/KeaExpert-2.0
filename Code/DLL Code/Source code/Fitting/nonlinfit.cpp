
/******************************************************************************
*                                                                             *
* Non-linear least squares fit. Uses Levenberg-Marquardt algorithm            *
*                                                                             *
*                                                                             *
*                                                                             *
*        Ref. Numerical Recipies p 545.           C Eccles JUNE 93            *
*                                                                             *
******************************************************************************/

/* Library includes */
#include "stdafx.h"
#include <float.h>
#include "../Global files/includesDLL.h"

/* Defines */

#define GENERAL_FIT     0
#define LORENTZIAN      1
#define GAUSSIAN		   2
#define T1_VD_FIT		   3
#define T1_IR_FIT		   4
#define T1_IR_BIEXP_FIT 5
#define T2_FIT			   6
#define DIFF_FIT1		   7
#define DIFF_FIT2		   8
#define DIFF_FIT3		   9
#define VEL_DIFF		   10
#define BI_EXP_FIT	   11
#define TRI_EXP_FIT	   12
#define T1_IR_FIT_ABS   13
#define EXP_OFF_FIT     14
#define LOG_NORMAL      15
#define CAP_FIT         16
#define PEAKFIT         17

/* Functions used defined in this file */

void function(float,float[],float*,float[],long);

short nlfit(float x[],float y[],float sig[],long ndata,float a[], long ia[], long ma,
      float **covar,float **alpha,float *chisq,
      void (*funcs)(float,float[],float*,float[],long),float *lamda);
      
void covsrt(float **covar,long ma,long ia[], long mfit);

void mrqcof(float x[],float y[],float sig[],long ndata,float a[],
       long ia[],long ma,float **alpha,float beta[],float *chisq,
       void (*funcs)(float,float[],float*,float[],long));
       

short gaussj(float **a, long n, float **b, long m);
short T1Fit(DLLParameters*,char*);
short T2Fit(DLLParameters*,char*);
short DiffFit(DLLParameters*,char*);
short GradCalc(DLLParameters*,Variable*,Variable*,float,char*);
short type = LORENTZIAN;
short ExpFit(long*,long*,float*,float*,float*,float**,float**,
             short,long,float*,float*,long,float,float*,float*,float*,float*);
short BiExpFit(DLLParameters*,char *parameters);
short GaussFit(DLLParameters*,char *parameters);
short LorentzianFit(DLLParameters*,char *parameters);
short ExpFitWithOffset(char*);
short LogNormalFit(DLLParameters* par, char *parameters);
extern short CapillaryFit(DLLParameters*,char*);
short LorentziansFit(DLLParameters* par, char *parameters);

static float grad,lgdel,smdel;
static CText *nonLinearfunction;
static void* callingInterface = 0;

// A general nonlinear fit. User must supply Prospa procedure which returns function and derivatives

short NonLinearFit(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varNoise;
   Variable varGuess;
   float *noiseVector;
   CText report;
   short err = OK;

	nonLinearfunction = new CText();
      
   type = GENERAL_FIT;

// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc, parameters,4,"x, y, function, initialGuess, [[noise], [report], [iterations]]","eeeeeee","vvtvvtl",&varX,&varY,nonLinearfunction,&varGuess,&varNoise,&report,&max_it)) < 0)
     return(nrArgs);

   callingInterface = par->itfc;

   // Check for input errors *************************************************
   if(varX.GetType() != MATRIX2D || varX.GetType() != MATRIX2D)
   {
      ErrorMessage("x & y variable should be row vector");
		delete nonLinearfunction;
      return(ERR);
   }

   if(varX.GetDimY() != 1 || varY.GetDimY() != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
		delete nonLinearfunction;
      return(ERR);
   }

   if(varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y variables should have same number of points");
		delete nonLinearfunction;
      return(ERR);
   }

   if(varGuess.GetType() != MATRIX2D || varGuess.GetDimY() != 1)
   {
      ErrorMessage("guess variable should be row vector");
		delete nonLinearfunction;
      return(ERR);
   }

// Evaluate noise
   if(nrArgs >= 5)
   {
      if(varNoise.GetType() == FLOAT32)
      {
         noiseLevel = varNoise.GetReal();

      // Check for errors *************************/   
         if(noiseLevel < 0)
         {
            ErrorMessage("noise level must be positive");
		      delete nonLinearfunction;
            return(ERR);
         }
      }
      else if(varNoise.GetType() == MATRIX2D)
      {
         if(varNoise.GetDimY() == 1 && varNoise.GetDimX() == varX.GetDimX())
         {
            noiseVector = varNoise.GetMatrix2D()[0];
         }
         else
         {
            ErrorMessage("invalid noise vector dimension");
		      delete nonLinearfunction;
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid data type for noise");
         delete nonLinearfunction;
         return(ERR);
      }
   }
   else
   {
      varNoise.MakeAndSetFloat(0.0);
   }


// Get sizes
   ndata = varX.GetDimX();
   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      delete nonLinearfunction;
      return(ERR);
   }
      
   nrPar = varGuess.GetDimX();
   if(nrPar == 0)
   {
      ErrorMessage("Zero parameters");
      delete nonLinearfunction;
      return(ERR);
   }
            
// Allocate memory ***************************/
   a    = MakeVectorNR(1L,nrPar);
   ia   = MakeIVectorNR(1L,nrPar);
   x    = MakeVectorNR(1L,ndata);
   y    = MakeVectorNR(1L,ndata);
   sig  = MakeVectorNR(1L,ndata);
 
// Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

// Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
// Estimate initial parameter values ***********/
   for(int i = 0; i < nrPar; i++)
      a[i+1] = varGuess.GetMatrix2D()[0][i];
         
// Set uncertainties to noise level ********/
   if(varNoise.GetType() == FLOAT32)
   {
      if(noiseLevel == 0)
      {
         calcNoise = 1;
         noiseLevel = 1;  
      }
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseLevel;
   }
   else
   {
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseVector[i-1];
   }

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.000001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }  
               
// Print out results of data ***************/
	if(calcNoise)
		NL = sqrt(chisq/(ndata-nrPar));
	else
		NL = 1;
		
   if(report == "yes")
   {
		if(calcNoise)
			TextMessage("\n\n     noise ............. %2.3f",NL);
		else
			TextMessage("\n");
		TextMessage("\n     Iterations ......... %ld",nit);
      for(int i = 1; i <= nrPar; i++)
		   TextMessage("\n     parameter %d ............... %2.3f +- %2.3f",i,a[i],sqrt(covar[i][i])*NL);
				
		TextMessage("\n     Normalised chi-squared .... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
   }

   // Return variables
   for(int i = 1; i <= nrPar; i++)
   {
      par->retVar[i].MakeAndSetFloat(a[i]);
   }   
   for(int i = 1; i <= nrPar; i++)
   {
      par->retVar[nrPar+i].MakeAndSetFloat(sqrt(covar[i][i])*NL);
   } 
   par->retVar[nrPar*2+1].MakeAndSetFloat(chisq/(ndata - nrPar)/sqr(NL));

   par->nrRetVar = nrPar*2+1;
   
// Free memory ********************************/
ex:
   FreeVectorNR(a,1L,nrPar); 
   FreeIVectorNR(ia,1L,nrPar);
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   // Run the Prospa macro func

	delete nonLinearfunction;

   return(OK);
}


short CapillaryFit(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   CText report;
   short err = OK;
   float pStart,kStart,sStart;
         
// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc,parameters,5,"x, y, Ps, ks, ss, [report], [max_iterations]","eeeeeee","vvffftl",&varX,&varY,&pStart,&kStart,&sStart,&report,&max_it)) < 0)
     return(nrArgs); 

   type = CAP_FIT;

// Check for input errors *************************************************
   if(VarRowSize(&varX) != 1 || VarRowSize(&varY) != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }
   
   ndata = VarColSize(&varX);

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }
      
   if(VarColSize(&varX) != VarColSize(&varY))
   {
      ErrorMessage("x & y vectors do not have same length");
      return(ERR);
   }
         
   nrPar = 3;
            
// Allocate memory ***************************/  
   ia = MakeIVectorNR(1L,nrPar);
   x = MakeVectorNR(1L,ndata);
   y = MakeVectorNR(1L,ndata);
   sig = MakeVectorNR(1L,ndata);
   a = MakeVectorNR(1L,nrPar);
 
// Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

// Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
// Estimate initial parameter values ***********/
   a[1] = pStart;
   a[2] = kStart;
   a[3] = sStart;
  
// Set uncertainties to noise level ********/
   if(noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;  
   }
   for(i = 1; i <= ndata; i++)
      sig[i] = noiseLevel;

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
	      err = ERR;
	      goto ex;   
	   }   
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;   
   }    
               
// Print out results of data ***************/
   NL = sqrt(chisq/(ndata-nrPar));
	
   if(report =="yes")
   {
		TextMessage("\n     Iterations .................. %ld",nit);
      TextMessage("\n\n     sqrt(fit_variance) .......... %2.3f",NL);
		TextMessage("\n     P ........................... %2.3f +- %2.3f",a[1],sqrt(covar[1][1])*NL);
		TextMessage("\n     k ........................... %2.3f +- %2.3f",a[2],sqrt(covar[2][2])*NL);
		TextMessage("\n     S ........................... %2.3f +- %2.3f",a[3],sqrt(covar[3][3])*NL);
  }
   
	float** bestFit = MakeMatrix2D(ndata,1);
	float val;
	float *dyda;
		
   dyda = MakeVectorNR(1L,ma);
		
   for(i = 1; i <= ndata; i++)
   {
	   function(x[i],a,&val,dyda,nrPar);
		bestFit[0][i-1] = val;
   }		
	FreeVectorNR(dyda,1L,ma);   

// Return to user (ans2 ... ans4)
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);
   
// Update other return variables *************
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(a[2]);
   par->retVar[4].MakeAndSetFloat(a[3]);
   par->retVar[5].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[6].MakeAndSetFloat(sqrt(covar[2][2])*NL);
   par->retVar[7].MakeAndSetFloat(sqrt(covar[3][3])*NL);
   par->nrRetVar = 7;
   
// Free memory ********************************
ex:
   FreeIVectorNR(ia,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   FreeVectorNR(a,1L,nrPar);   
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
          
   return(err);
}

// Try and fit 1 decaying exponential to the supplied xy data

short T1Fit(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   char exptype[50];
   Variable varX;
   Variable varY;
   char report[50];
   short err = OK;
         
// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc,parameters,3,"x, y, ir/vd, [[noise], [report], [max_iterations]","eeeeee","vvsfsl",&varX,&varY,exptype,&noiseLevel,report,&max_it)) < 0)
     return(nrArgs);  

   type = T1_VD_FIT;

// Check for errors *************************/   
   if(noiseLevel < 0)
   {
      ErrorMessage("noise level must be positive");
      return(ERR);
   }

   if(!strcmp(exptype,"vd"))  
      type = T1_VD_FIT;
   else if(!strcmp(exptype,"ir"))
      type = T1_IR_FIT;
   else if (!strcmp(exptype, "irdual"))
      type = T1_IR_BIEXP_FIT;
   else if(!strcmp(exptype,"irabs"))
      type = T1_IR_FIT_ABS;
   else
   {
      ErrorMessage("invalid experiment type");
      return(ERR);
   }
   
// Check for input errors *************************************************
   if(VarRowSize(&varX) != 1 || VarRowSize(&varY) != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }
   
   ndata = VarColSize(&varX);

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }
      
   if(VarColSize(&varX) != VarColSize(&varY))
   {
      ErrorMessage("x & y vectors do not have same length");
      return(ERR);
   }

   if (type == T1_IR_BIEXP_FIT)
      nrPar = 4;
   else
      nrPar = 2;

            
// Allocate memory ***************************/  
   ia = MakeIVectorNR(1L,nrPar);
   x = MakeVectorNR(1L,ndata);
   y = MakeVectorNR(1L,ndata);
   sig = MakeVectorNR(1L,ndata);
   a = MakeVectorNR(1L,nrPar);
 
// Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

// Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
// Estimate initial parameter values ***********/

   if(type == T1_IR_FIT)
   {
      a[1] = y[ndata];
      if(a[1] <= 0)
      {
         ErrorMessage("data set unsuited to T1 fit");
         err = ERR;
         goto ex;
      }
      
      for(i = 1; i <= ndata; i++)
      {
         if(y[i] > a[1]*0.632)
         {
            if(x[i] <= 0)
            {
               ErrorMessage("invalid x axis");
               err = ERR;
               goto ex;
            }
            a[2] = 1.0/x[i];
            break;
         }
      } 
      if(i > ndata)
         a[2] = 1/x[ndata];  
   }
   else if (type == T1_IR_BIEXP_FIT)
   {
      if (y[ndata] <= 0)
      {
         ErrorMessage("data set unsuited to T1 fit");
         err = ERR;
         goto ex;
      }

      a[1] = y[ndata] / 2;
      a[3] = y[ndata] / 2;

      for (i = 1; i <= ndata; i++)
      {
         if (y[i] > a[1] * 0.632 * 2)
         {
            if (x[i] <= 0)
            {
               ErrorMessage("invalid x axis");
               err = ERR;
               goto ex;
            }
            a[2] = 1.0 / x[i];
            a[4] = 1.0 / x[i];
            break;
         }
      }
      if (i > ndata)
      {
         a[2] = 1.0 / x[ndata];
         a[4] = 1.0 / x[ndata];
      }
   }
   else // Positive decay only
   {
      a[1] = (y[ndata] + y[1])/2;

      float ymin = 1e38;
      long mini;
      for(i = 1; i <= ndata; i++)
      {
         if(y[i] < ymin)
         {
            ymin = y[i];
            mini = i;
         }
      }
      a[2] = 0.693/x[mini];
   }


          
// Set uncertainties to noise level ********/
   if(noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;  
   }
   for(i = 1; i <= ndata; i++)
      sig[i] = noiseLevel;

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
	      err = ERR;
	      goto ex;   
	   }   
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;   
   }    
               
// Print out results of data ***************/
	if(calcNoise)
		NL = sqrt(chisq/(ndata-nrPar));
	else
		NL = 1;
		
   if(!strcmp(report,"yes"))
   {
      if (type != T1_IR_BIEXP_FIT)
      {
         if (calcNoise)
            TextMessage("\n\n     noise ............. %2.3f", NL);
         else
            TextMessage("\n");
         TextMessage("\n     Iterations ......... %ld", nit);
         TextMessage("\n     E(0) ............... %2.3f +- %2.3f", a[1], sqrt(covar[1][1]) * NL);
         TextMessage("\n     T1 ................. %2.3f +- %2.3f", 1.0 / a[2], sqrt(covar[2][2]) / sqr(a[2]) * NL);

         // Print out statistics ***********************/

         TextMessage("\n     Normalised chi-squared .... %2.2f\n", chisq / (ndata - nrPar) / sqr(NL));
      }
      else
      {
         if (calcNoise)
            TextMessage("\n\n     noise ............. %2.3f", NL);
         else
            TextMessage("\n");
         TextMessage("\n     Iterations ......... %ld", nit);
         TextMessage("\n     Ea(0) ............... %2.3f +- %2.3f", a[1], sqrt(covar[1][1]) * NL);
         TextMessage("\n     T1a ................. %2.3f +- %2.3f", 1.0 / a[2], sqrt(covar[2][2]) / sqr(a[2]) * NL);
         TextMessage("\n     Eb(0) ............... %2.3f +- %2.3f", a[1], sqrt(covar[3][3])* NL);
         TextMessage("\n     T1b ................. %2.3f +- %2.3f", 1.0 / a[2], sqrt(covar[4][4]) / sqr(a[4]) * NL);

         // Print out statistics ***********************/

         TextMessage("\n     Normalised chi-squared .... %2.2f\n", chisq / (ndata - nrPar) / sqr(NL));
      }
   }
   
// Make best fit vector (ans1 & ans)
	float** bestFit = MakeMatrix2D(ndata,1);
	float val;
	float *dyda;
		
   dyda = MakeVectorNR(1L,ma);
		
   for(i = 1; i <= ndata; i++)
   {
	   function(x[i],a,&val,dyda,nrPar);
		bestFit[0][i-1] = val;
   }		
	FreeVectorNR(dyda,1L,ma);   

// Return to user (ans2 ... ans4)
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);
   
// Update other return variables *************
   if (type != T1_IR_BIEXP_FIT)
   {
      par->retVar[2].MakeAndSetFloat(a[1]);
      par->retVar[3].MakeAndSetFloat(1.0 / a[2]);
      par->retVar[4].MakeAndSetFloat(sqrt(covar[1][1]) * NL);
      par->retVar[5].MakeAndSetFloat(sqrt(covar[2][2]) / sqr(a[2]) * NL);
      par->nrRetVar = 5;
   }
   else
   {
      par->retVar[2].MakeAndSetFloat(a[1]);
      par->retVar[3].MakeAndSetFloat(1.0 / a[2]);
      par->retVar[4].MakeAndSetFloat(a[3]);
      par->retVar[5].MakeAndSetFloat(1.0 / a[4]);
      par->retVar[6].MakeAndSetFloat(sqrt(covar[1][1]) * NL);
      par->retVar[7].MakeAndSetFloat(sqrt(covar[2][2]) / sqr(a[2]) * NL);
      par->retVar[8].MakeAndSetFloat(sqrt(covar[3][3]) * NL);
      par->retVar[9].MakeAndSetFloat(sqrt(covar[4][4]) / sqr(a[4]) * NL);
      par->nrRetVar = 9;

   }
   
// Free memory ********************************
ex:
   FreeIVectorNR(ia,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   FreeVectorNR(a,1L,nrPar);   
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
          
   return(err);
}

// Try and fit 1 decaying exponential with offset to the suppied xy data

short ExpFitWithOffset(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 20;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   char exptype[50];
   Variable varX;
   Variable varY;
   char report[50];
   short err = OK;
         
// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc,parameters,2,"x, y, [[noise], [report]","eeee","vvfs",&varX,&varY,&noiseLevel,report)) < 0)
     return(nrArgs);  

// Check for errors *************************/   
   if(noiseLevel < 0)
   {
      ErrorMessage("noise level must be positive");
      return(ERR);
   }

   type = EXP_OFF_FIT;

   
// Check for input errors *************************************************
   if(VarRowSize(&varX) != 1 || VarRowSize(&varY) != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }
   
   ndata = VarColSize(&varX);

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }
      
   if(VarColSize(&varX) != VarColSize(&varY))
   {
      ErrorMessage("x & y vectors do not have same length");
      return(ERR);
   }
         
   nrPar = 3;
            
// Allocate memory ***************************/  
   ia = MakeIVectorNR(1L,nrPar);
   x = MakeVectorNR(1L,ndata);
   y = MakeVectorNR(1L,ndata);
   sig = MakeVectorNR(1L,ndata);
   a = MakeVectorNR(1L,nrPar);
 
// Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

// Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
// Estimate initial parameter values ***********/
   a[1] = y[1];
   if(a[1] <= 0)
      return(-1);

   for(i = 1; i <= ndata; i++)
   {
      if(y[i] < a[1]*0.3679)
      {
         if(x[i] == 0) return(-1);
         a[2] = 1.0/x[i];
         break;
      }
   }
   if(i > ndata)
      a[2] = 1/x[ndata];

   a[3] = y[ndata];

	a[1] = a[1] - a[3];
          
// Set uncertainties to noise level ********/
   if(noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;  
   }
   for(i = 1; i <= ndata; i++)
      sig[i] = noiseLevel;

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
	      err = ERR;
	      goto ex;   
	   }   
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;   
   }    
               
// Print out results of data ***************/
	if(calcNoise)
		NL = sqrt(chisq/(ndata-nrPar));
	else
		NL = 1;
		
   if(!strcmp(report,"yes"))
   {
		if(calcNoise)
			TextMessage("\n\n     noise ............. %2.3f",NL);
		else
			TextMessage("\n");
		TextMessage("\n     Iterations ......... %ld",nit);
		TextMessage("\n     E(0) ............... %2.3f +- %2.3f",a[1],sqrt(covar[1][1])*NL);
		TextMessage("\n     Tau ................ %2.3f +- %2.3f",1.0/a[2],sqrt(covar[2][2])/sqr(a[2])*NL);
		TextMessage("\n     Offset ............. %2.3f +- %2.3f",a[3],sqrt(covar[3][3])*NL);
		
	// Print out statistics ***********************/
		
		TextMessage("\n     Normalised chi-squared .... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
   }
   
// Make best fit vector (ans1 & ans)
	float** bestFit = MakeMatrix2D(ndata,1);
	float val;
	float *dyda;
		
   dyda = MakeVectorNR(1L,ma);
		
   for(i = 1; i <= ndata; i++)
   {
	   function(x[i],a,&val,dyda,nrPar);
		bestFit[0][i-1] = val;
   }		
	FreeVectorNR(dyda,1L,ma);   

// Return to user (ans2 ... ans4)
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);
   
// Update other return variables *************
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(1.0/a[2]);
   par->retVar[4].MakeAndSetFloat(a[3]);
   par->retVar[5].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[6].MakeAndSetFloat(sqrt(covar[2][2])/sqr(a[2])*NL); 
   par->retVar[7].MakeAndSetFloat(sqrt(covar[3][3])*NL);
   par->nrRetVar = 7;
   
// Free memory ********************************
ex:
   FreeIVectorNR(ia,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   FreeVectorNR(a,1L,nrPar);   
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
          
   return(err);
}


// Try and fit 1 decaying exponential to the suppied xy data

long cnt = 0;
short T2Fit(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varNoise;
   float *noiseVector;
   char report[50];
   short err = OK;
   float e0Guess,t2Guess;

   cnt = 0;
      
   type = T2_FIT;
   
// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc, parameters,2,"x, y, [[noise], [report], [iterations], [e0Guess, t2Guess]","eeeeeee","vvvslff",&varX,&varY,&varNoise,report,&max_it,&e0Guess,&t2Guess)) < 0)
     return(nrArgs);  

// Check for input errors *************************************************
   if(varX.GetType() != MATRIX2D || varX.GetType() != MATRIX2D)
   {
      ErrorMessage("x & y variable should be row vector");
      return(ERR);
   }

   if(varX.GetDimY() != 1 || varY.GetDimY() != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }

   if(varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y variables should have same number of points");
      return(ERR);
   }

   if(nrArgs > 2)
   {
      if(varNoise.GetType() == FLOAT32)
      {
         noiseLevel = varNoise.GetReal();

      // Check for errors *************************/   
         if(noiseLevel < 0)
         {
            ErrorMessage("noise level must be positive");
            return(ERR);
         }
      }
      else if(varNoise.GetType() == MATRIX2D)
      {
         if(varNoise.GetDimY() == 1 && varNoise.GetDimX() == varX.GetDimX())
         {
            noiseVector = varNoise.GetMatrix2D()[0];
         }
         else
         {
            ErrorMessage("invalid noise vector dimension");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid data type for noise");
         return(ERR);
      }
   }
   else
   {
      varNoise.MakeAndSetFloat(0.0);
   }


   ndata = varX.GetDimX();

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }
      
   nrPar = 2;
            
// Allocate memory ***************************/
   a    = MakeVectorNR(1L,nrPar);
   ia   = MakeIVectorNR(1L,nrPar);
   x    = MakeVectorNR(1L,ndata);
   y    = MakeVectorNR(1L,ndata);
   sig  = MakeVectorNR(1L,ndata);
 
// Copy data to x,y arrays ******************************************
   float maxY = -1e39;
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
      if(y[i] > maxY) maxY = y[i];
   }

// Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
// Estimate initial parameter values ***********/
   if(nrArgs == 7)
   {
      a[1] = e0Guess;
      a[2] = 1.0/t2Guess;
   }
   else
   {
      a[1] = y[1];
      if(a[1] <= 0)
      {
         a[1] = maxY;
         a[2] = 1.0/x[ndata];
      }
      else
      {
         for(i = 1; i <= ndata; i++)
         {
            if(y[i] < a[1]*0.3679)
            {
               if(x[i] == 0)
               {
                  ErrorMessage("invalid x axis");
                  err = ERR;
                  goto ex;
               }
               a[2] = 1.0/x[i];
               break;
            }
         } 
         if(i > ndata)
            a[2] = 1/x[ndata];  
      }
   }
          
// Set uncertainties to noise level ********/
   if(varNoise.GetType() == FLOAT32)
   {
      if(noiseLevel == 0)
      {
         calcNoise = 1;
         noiseLevel = 1;  
      }
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseLevel;
   }
   else
   {
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseVector[i-1];
   }

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }  
               
// Print out results of data ***************/
	if(calcNoise)
		NL = sqrt(chisq/(ndata-nrPar));
	else
		NL = 1;
		
   if(!strcmp(report,"yes"))
   {
		if(calcNoise)
			TextMessage("\n\n     noise ............. %2.3f",NL);
		else
			TextMessage("\n");
		TextMessage("\n     Iterations ......... %ld",nit);
		TextMessage("\n     E(0) ............... %2.3f +- %2.3f",a[1],sqrt(covar[1][1])*NL);
		TextMessage("\n     T2 ................. %2.3f +- %2.3f",1.0/a[2],sqrt(covar[2][2])/sqr(a[2])*NL);
		
	// Print out statistics ***********************/
		
		TextMessage("\n     Normalised chi-squared .... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
   }
   
// Make best fit vector (ans1 & ans)
	float** bestFit = MakeMatrix2D(ndata,1);
	float val;
	float *dyda;
		
   dyda = MakeVectorNR(1L,ma);
		
   for(i = 1; i <= ndata; i++)
   {
	   function(x[i],a,&val,dyda,nrPar);
		bestFit[0][i-1] = val;
   }		
	FreeVectorNR(dyda,1L,ma);   

// Return to user bestFit in ans and ans1 ****
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);
   
// Return other parameters in ans2 ... ans5
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(1.0/a[2]);
   par->retVar[4].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[5].MakeAndSetFloat(sqrt(covar[2][2])/sqr(a[2])*NL);   
   par->nrRetVar = 5;
   
// Free memory ********************************/
ex:
   FreeVectorNR(a,1L,nrPar); 
   FreeIVectorNR(ia,1L,nrPar);
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);

   return(err);
}

// Try and fit to a gaussian function using the supplied xy data

short GaussFit(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,j,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varNoise;
   float *noiseVector;
   char report[50];
   short err = OK;
   float par1 = 0, par2 = 0, par3 = 0;

   type = GAUSSIAN;

   // Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc, parameters,2,"x, y, [noise], [report], [p1, p2, p3]","eeeeeeee","vvvslfff",&varX,&varY,&varNoise,report,&max_it,&par1,&par2,&par3)) < 0)
      return(nrArgs);  

   // Check for input errors *************************************************
   if(varX.GetType() != MATRIX2D || varX.GetType() != MATRIX2D)
   {
      ErrorMessage("x & y variable should be row vector");
      return(ERR);
   }

   if(varX.GetDimY() != 1 || varY.GetDimY() != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }

   if(varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y variables should have same number of points");
      return(ERR);
   }

   if(nrArgs > 2)
   {
      if(varNoise.GetType() == FLOAT32)
      {
         noiseLevel = varNoise.GetReal();

         // Check for errors *************************/   
         if(noiseLevel < 0)
         {
            ErrorMessage("noise level must be positive");
            return(ERR);
         }
      }
      else if(varNoise.GetType() == MATRIX2D)
      {
         if(varNoise.GetDimY() == 1 && varNoise.GetDimX() == varX.GetDimX())
         {
            noiseVector = varNoise.GetMatrix2D()[0];
         }
         else
         {
            ErrorMessage("invalid noise vector dimension");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid data type for noise");
         return(ERR);
      }
   }
   else
   {
      varNoise.MakeAndSetFloat(0.0);
   }


   ndata = varX.GetDimX();

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }

   nrPar = 3;

   // Allocate memory ***************************/
   a    = MakeVectorNR(1L,nrPar);
   ia   = MakeIVectorNR(1L,nrPar);
   x    = MakeVectorNR(1L,ndata);
   y    = MakeVectorNR(1L,ndata);
   sig  = MakeVectorNR(1L,ndata);

   // Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

   // Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   for(i = 1; i <= nrPar; i++)
   {
      for(j = 1; j <= nrPar; j++)
      {
         covar[i][j] = 0;
         alpha[i][j] = 0;
      }
   }
      

   if(par1 == 0)
   {
      // Estimate initial parameter values ***********/
      float maxY = -1e39;
      int maxPos = 0;

      for(i = 1; i <= ndata; i++)
      {
         if(y[i] > maxY)
         {
            maxY = y[i];
            maxPos = i;
         }
      } 

      a[1] = maxY;
      a[2] = x[maxPos];

      for(i = maxPos; i <= ndata; i++)
      {
         if(y[i] < maxY/2)
         {
            a[3] = (x[i]-x[maxPos])/sqrt(-2*log(0.5));
            break;
         }
      }  
   }
   else
   {
      a[1] = par1;
      a[2] = par2;
      a[3] = par3;
   }
   
   if(a[3] == 0)
   {
      ErrorMessage("Invalid Gaussian width start value");
      goto ex;
   }

   // Set uncertainties to noise level ********/
   if(varNoise.GetType() == FLOAT32)
   {
      if(noiseLevel == 0)
      {
         calcNoise = 1;
         noiseLevel = 1;  
      }
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseLevel;
   }
   else
   {
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseVector[i-1];
   }

   // Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }  

   // Print out results of data ***************/
   if(calcNoise)
      NL = sqrt(chisq/(ndata-nrPar));
   else
      NL = 1;

   if(!strcmp(report,"yes"))
   {
      if(calcNoise)
         TextMessage("\n\n     Noise ..................... %2.3f",NL);
      else
         TextMessage("\n");
      TextMessage("\n     Iterations ................ %ld",nit);
      TextMessage("\n     Amplitude ................. %2.3f +- %2.3f",a[1],sqrt(covar[1][1])*NL);
      TextMessage("\n     Position .................. %2.3f +- %2.3f",a[2],sqrt(covar[2][2])*NL);
      TextMessage("\n     Standard deviation ........ %2.3f +- %2.3f",a[3],sqrt(covar[3][3])*NL);

      // Print out statistics ***********************/

      TextMessage("\n     Normalised chi-squared .... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
   }

   // Make best fit vector (ans1 & ans)
   float** bestFit = MakeMatrix2D(ndata,1);
   float val;
   float *dyda;

   dyda = MakeVectorNR(1L,ma);

   for(i = 1; i <= ndata; i++)
   {
      function(x[i],a,&val,dyda,nrPar);
      bestFit[0][i-1] = val;
   }		
   FreeVectorNR(dyda,1L,ma);   

   // Return to user bestFit in ans and ans1 ****
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);

   // Return other parameters in ans2 ... ans7
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(a[2]);
   par->retVar[4].MakeAndSetFloat(a[3]);
   par->retVar[5].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[6].MakeAndSetFloat(sqrt(covar[2][2])*NL);   
   par->retVar[7].MakeAndSetFloat(sqrt(covar[3][3])*NL);   
   par->nrRetVar = 7;

   // Free memory ********************************/
ex:
   FreeVectorNR(a,1L,nrPar); 
   FreeIVectorNR(ia,1L,nrPar);
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);

   return(err);
}



// Try and fit to a Lorentzian function using the supplied xy data

short LorentzianFit(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varNoise;
   float *noiseVector;
   char report[50];
   short err = OK;
   float par1 = 0, par2 = 0, par3 = 0;

   type = LORENTZIAN;

   // Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc, parameters,2,"x, y, [noise], [report], [p1, p2, p3]","eeeeeeee","vvvslfff",&varX,&varY,&varNoise,report,&max_it,&par1,&par2,&par3)) < 0)
      return(nrArgs);  

   // Check for input errors *************************************************
   if(varX.GetType() != MATRIX2D || varX.GetType() != MATRIX2D)
   {
      ErrorMessage("x & y variable should be row vector");
      return(ERR);
   }

   if(varX.GetDimY() != 1 || varY.GetDimY() != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }

   if(varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y variables should have same number of points");
      return(ERR);
   }

   if(nrArgs > 2)
   {
      if(varNoise.GetType() == FLOAT32)
      {
         noiseLevel = varNoise.GetReal();

         // Check for errors *************************/   
         if(noiseLevel < 0)
         {
            ErrorMessage("noise level must be positive");
            return(ERR);
         }
      }
      else if(varNoise.GetType() == MATRIX2D)
      {
         if(varNoise.GetDimY() == 1 && varNoise.GetDimX() == varX.GetDimX())
         {
            noiseVector = varNoise.GetMatrix2D()[0];
         }
         else
         {
            ErrorMessage("invalid noise vector dimension");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid data type for noise");
         return(ERR);
      }
   }
   else
   {
      varNoise.MakeAndSetFloat(0.0);
   }


   ndata = varX.GetDimX();

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }

   nrPar = 3;

   // Allocate memory ***************************/
   a    = MakeVectorNR(1L,nrPar);
   ia   = MakeIVectorNR(1L,nrPar);
   x    = MakeVectorNR(1L,ndata);
   y    = MakeVectorNR(1L,ndata);
   sig  = MakeVectorNR(1L,ndata);

   // Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

   // Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);

   if(par1 == 0)
   {
      // Estimate initial parameter values ***********/
      float maxY = -1e39;
      int maxPos = 0;

      for(i = 1; i <= ndata; i++)
      {
         if(y[i] > maxY)
         {
            maxY = y[i];
            maxPos = i;
         }
      } 

      a[1] = maxY;
      a[2] = x[maxPos];

      for(i = maxPos; i <= ndata; i++)
      {
         if(y[i] < maxY/2)
         {
            a[3] = 1.0/(sqr(x[i]-x[maxPos]));
            break;
         }
      }  
   }
   else
   {
      a[1] = par1;
      a[2] = par2;
      a[3] = 1.0/sqr(par3/2);
   }
   
   // Set uncertainties to noise level ********/
   if(varNoise.GetType() == FLOAT32)
   {
      if(noiseLevel == 0)
      {
         calcNoise = 1;
         noiseLevel = 1;  
      }
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseLevel;
   }
   else
   {
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseVector[i-1];
   }

   // Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }  

   // Print out results of data ***************/
   if(calcNoise)
      NL = sqrt(chisq/(ndata-nrPar));
   else
      NL = 1;

   if(!strcmp(report,"yes"))
   {
      if(calcNoise)
         TextMessage("\n\n     Noise ..................... %2.3f",NL);
      else
         TextMessage("\n");
      TextMessage("\n     Iterations ................ %ld",nit);
      TextMessage("\n     Amplitude ................. %2.3f +- %2.3f",a[1],sqrt(covar[1][1])*NL);
      TextMessage("\n     Position .................. %2.3f +- %2.3f",a[2],sqrt(covar[2][2])*NL);
      TextMessage("\n     FWHM ...................... %2.3f +- %2.3f",2/sqrt(a[3]),2/sqrt(a[3])*sqrt(covar[3][3])*NL/a[3]);

      // Print out statistics ***********************/

      TextMessage("\n     Normalised chi-squared .... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
   }

   // Make best fit vector (ans1 & ans)
   float** bestFit = MakeMatrix2D(ndata,1);
   float val;
   float *dyda;

   dyda = MakeVectorNR(1L,ma);

   for(i = 1; i <= ndata; i++)
   {
      function(x[i],a,&val,dyda,nrPar);
      bestFit[0][i-1] = val;
   }		
   FreeVectorNR(dyda,1L,ma);   

   // Return to user bestFit in ans and ans1 ****
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);

   // Return other parameters in ans2 ... ans7
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(a[2]);
   par->retVar[4].MakeAndSetFloat(a[3]);
   par->retVar[5].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[6].MakeAndSetFloat(sqrt(covar[2][2])*NL);   
   par->retVar[7].MakeAndSetFloat(sqrt(covar[3][3])*NL);   
   par->nrRetVar = 7;

   // Free memory ********************************/
ex:
   FreeVectorNR(a,1L,nrPar); 
   FreeIVectorNR(ia,1L,nrPar);
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);

   return(err);
}

// Try and fit to multiple Lorentzian functions using the supplied xy data

short LorentziansFit(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,j,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varPeakList;
   Variable fixedParVar;
   short err = OK;

   type = PEAKFIT;

   // Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc, parameters,3,"x, y, peakList, whichPar","eeee","vvvv",&varX,&varY,&varPeakList,&fixedParVar)) < 0)
      return(nrArgs);  

   // Check for input errors *************************************************
   if(varX.GetType() != MATRIX2D || varX.GetType() != MATRIX2D)
   {
      ErrorMessage("x & y variable should be row vector");
      return(ERR);
   }

   if(varPeakList.GetType() != MATRIX2D || varPeakList.GetDimX() != 3)
   {
      ErrorMessage("peak list should be a 3 by N matrix");
      return(ERR);
   }

   if(varX.GetDimY() != 1 || varY.GetDimY() != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }

   if(varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y variables should have same number of points");
      return(ERR);
   }

   if(nrArgs == 4)
   {
      if(fixedParVar.GetType() != MATRIX2D || fixedParVar.GetDimX() != varPeakList.GetDimX() || fixedParVar.GetDimY() != 1)
      {
         ErrorMessage("fixed par should have same number of entried as peak list");
         return(ERR);
      }
   }


   ndata = varX.GetDimX();

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }

   // Get peak list info
   float **peakList = varPeakList.GetMatrix2D();
   int nrPeaks = varPeakList.GetDimY();

   nrPar = 3*nrPeaks;

   // Allocate memory ***************************/
   a    = MakeVectorNR(1L,nrPar);
   ia   = MakeIVectorNR(1L,nrPar);
   x    = MakeVectorNR(1L,ndata);
   y    = MakeVectorNR(1L,ndata);
   sig  = MakeVectorNR(1L,ndata);

   // Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
      sig[i] = 1;
   }

   // Initialize some data values ****************/
   ma = nrPar;
   if(nrArgs ==3)
   {
      for(i = 1; i <= nrPar; i++)
         ia[i] = 1;
   }
   else
   {
      float *fixedPar = fixedParVar.GetMatrix2D()[0];
      for(i = 1; i <= nrPar; i++)
         ia[i] = (int)(fixedPar[(i-1)%3]==1);
   }

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);


  // Put in intial values of position, amplitude and width
   for(i = 1, j = 0; i <= nrPar; i+=3, j++)
   {
      a[i] = peakList[j][0];
      a[i+1] = peakList[j][1];
      a[i+2] = peakList[j][2];
   }

   // Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.0000001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }  

   // Make best fit vector (ans1 & ans)
   float** bestFit = MakeMatrix2D(ndata,1);
   float val;
   float *dyda;

   dyda = MakeVectorNR(1L,ma);

   for(i = 1; i <= ndata; i++)
   {
      function(x[i],a,&val,dyda,nrPar);
      bestFit[0][i-1] = val;
   }		
   FreeVectorNR(dyda,1L,ma);   

   // Return to user bestFit ****
   par->retVar[1].AssignMatrix2D(bestFit,ndata,1);

   float** parOut = MakeMatrix2D(3,nrPeaks);
   for(int i = 1, j = 0; i < nrPar; i+=3,j++)
   {
      parOut[j][0] = a[i];
      parOut[j][1] = a[i+1];
      parOut[j][2] = a[i+2];
   }

   par->retVar[2].AssignMatrix2D(parOut,3,nrPeaks);  
   par->nrRetVar = 2;

   // Free memory ********************************/
ex:
   FreeVectorNR(a,1L,nrPar); 
   FreeIVectorNR(ia,1L,nrPar);
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);

   return(err);
}

// Try and fit to a lognormal function using the supplied xy data

short LogNormalFit(DLLParameters* par, char *parameters)
{
   long *ia;
   float *x,*y,*sig,*a;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varNoise;
   float *noiseVector;
   char report[50];
   short err = OK;

   type = LOG_NORMAL;

   // Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc, parameters,2,"x, y, [[noise], [report]","eeeee","vvvsl",&varX,&varY,&varNoise,report,&max_it)) < 0)
      return(nrArgs);  

   // Check for input errors *************************************************
   if(varX.GetType() != MATRIX2D || varX.GetType() != MATRIX2D)
   {
      ErrorMessage("x & y variable should be row vector");
      return(ERR);
   }

   if(varX.GetDimY() != 1 || varY.GetDimY() != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }

   if(varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y variables should have same number of points");
      return(ERR);
   }

   if(nrArgs > 2)
   {
      if(varNoise.GetType() == FLOAT32)
      {
         noiseLevel = varNoise.GetReal();

         // Check for errors *************************/   
         if(noiseLevel < 0)
         {
            ErrorMessage("noise level must be positive");
            return(ERR);
         }
      }
      else if(varNoise.GetType() == MATRIX2D)
      {
         if(varNoise.GetDimY() == 1 && varNoise.GetDimX() == varX.GetDimX())
         {
            noiseVector = varNoise.GetMatrix2D()[0];
         }
         else
         {
            ErrorMessage("invalid noise vector dimension");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid data type for noise");
         return(ERR);
      }
   }
   else
   {
      varNoise.MakeAndSetFloat(0.0);
   }


   ndata = varX.GetDimX();

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }

   nrPar = 3;

   // Allocate memory ***************************/
   a    = MakeVectorNR(1L,nrPar);
   ia   = MakeIVectorNR(1L,nrPar);
   x    = MakeVectorNR(1L,ndata);
   y    = MakeVectorNR(1L,ndata);
   sig  = MakeVectorNR(1L,ndata);

   // Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

   // Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);

   // Estimate initial parameter values ***********/
   float maxY = -1e39;
   int maxPos = 0;

   for(i = 1; i <= ndata; i++)
   {
      if(y[i] > maxY)
      {
         maxY = y[i];
         maxPos = i;
      }
   } 

   a[1] = maxY;
   a[2] = x[maxPos];

   for(i = maxPos; i <= ndata; i++)
   {
      if(y[i] < maxY/2)
      {
         a[3] = log(x[i]/a[2])/sqrt(-2*log(0.5));
         break;
      }
   }    
   
   // Set uncertainties to noise level ********/
   if(varNoise.GetType() == FLOAT32)
   {
      if(noiseLevel == 0)
      {
         calcNoise = 1;
         noiseLevel = 1;  
      }
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseLevel;
   }
   else
   {
      for(i = 1; i <= ndata; i++)
         sig[i] = noiseVector[i-1];
   }

   // Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }  

   // Print out results of data ***************/
   if(calcNoise)
      NL = sqrt(chisq/(ndata-nrPar));
   else
      NL = 1;

   if(!strcmp(report,"yes"))
   {
      if(calcNoise)
         TextMessage("\n\n     noise ............. %2.3f",NL);
      else
         TextMessage("\n");
      TextMessage("\n     Iterations ......... %ld",nit);
      TextMessage("\n     E(0) ............... %2.3f +- %2.3f",a[1],sqrt(covar[1][1])*NL);
      TextMessage("\n     T2 ................. %2.3f +- %2.3f",a[2],sqrt(covar[2][2])*NL);
      TextMessage("\n     Sigma .............. %2.3f +- %2.3f",a[3],sqrt(covar[3][3])*NL);

      // Print out statistics ***********************/

      TextMessage("\n     Normalised chi-squared .... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
   }

   // Make best fit vector (ans1 & ans)
   float** bestFit = MakeMatrix2D(ndata,1);
   float val;
   float *dyda;

   dyda = MakeVectorNR(1L,ma);

   for(i = 1; i <= ndata; i++)
   {
      function(x[i],a,&val,dyda,nrPar);
      bestFit[0][i-1] = val;
   }		
   FreeVectorNR(dyda,1L,ma);   

   // Return to user bestFit in ans and ans1 ****
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);

   // Return other parameters in ans2 ... ans7
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(a[2]);
   par->retVar[4].MakeAndSetFloat(a[3]);
   par->retVar[5].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[6].MakeAndSetFloat(sqrt(covar[2][2])*NL);   
   par->retVar[7].MakeAndSetFloat(sqrt(covar[3][3])*NL);   
   par->nrRetVar = 7;

   // Free memory ********************************/
ex:
   FreeVectorNR(a,1L,nrPar); 
   FreeIVectorNR(ia,1L,nrPar);
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);

   return(err);
}

#ifdef UNDEFINED
       
/* Test program for lfit */

short PeakFit(char arg[])
{
   float *x;
   float *y;
   float *sig;
   float *a;
   long *ia;
   float **covar;
   float **alpha;
   float chisq;
   float lamda;
   long *fix;
   float *temp;
   long ma;
   long i,j;
   long ndata;
   static float min_chisq;
   static long max_it = 10;
   long nit;
   long nrPar;
   char str[MAX_STR];
   float pos;
   complex *data;
   static float peakWidth;
   long left,right,cnt;
   static float noiseLevel;
   short nrArgs;
   extern short nr_picked_peaks;
   extern PeakData peak[];
   long *pNr;
   char update = 'n';
   extern short ListPeaks(char arg[]);

   type = LORENZIAN;
   
/* Get info from user ******************************/
   if(arg[0] == '\0')
   {
      sprintf(arg,"\n\n   width noise iter ... %2.3f %g %ld %c",peakWidth,noiseLevel,max_it,update);
      if(GetStringFromUser(arg,26) == -1) return(0);
   }     

   if((nrArgs = str_scan(arg,"%vf %vf &vld %vc",&peakWidth,&noiseLevel,&max_it,&update)) == -1)
   {
      return(-1);
   }
   else if(nrArgs != 4)
   { 
      return(-2);
   }

/* Determine number of parameters by counting peaks */
   cnt = 0;

   left = (**plot1).dataRect.left;
   right = (**plot1).dataRect.right;
   min_chisq = right-left+1;
   for(i = 1; i <= nr_picked_peaks; i++)
   {
      pos = PPMToPoints(peak[i].x);
      if((pos < left) || (pos > right)) 
         continue;
      cnt++;
   }
   if(cnt == 0)
   {
      ErrorMessage("No visible peaks");
      return(-1);
   }   
   
   nrPar = cnt*3;
   ndata = right-left+1;
            
/* Allocate memory ***************************/
   pNr = MakeIVectorNR(1L,ndata);
   ia = MakeIVectorNR(1L,ndata);
   fix = MakeIVectorNR(1L,ndata);
   temp = MakeVectorNR(1L,ndata);
   x = MakeVectorNR(1L,ndata);
   y = MakeVectorNR(1L,ndata);
   sig = MakeVectorNR(1L,ndata);
   a = MakeVectorNR(1L,ndata);
 
/* Load data from buffer 0 *******************/
   data = *data1DHandle;
   for(i = 1; i <= ndata; i++)
   {
      x[i] = i+left-1;
      y[i] = data[i+left-1].r;
   }
   
/* Initialize some data values ****************/
   ma = nrPar;

   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
/* Enter initial parameter values ***********/
  cnt = 1;
  for(i = 1; i <= nr_picked_peaks; i++)
  {
     pos = PPMToPoints(peak[i].x);

     if((pos < left) || (pos > right)) 
         continue;
     pNr[cnt] = i;
     a[cnt++] = peak[i].y; /* amplitude */
     a[cnt++] = pos; /* position  */
     if(peak[i].width > 0) peakWidth = peak[i].width;
     if(type == LORENZIAN)
        a[cnt++] = SQR(2.0/peakWidth);
     else
        a[cnt++] = peakWidth; /* width */
   }
   
/* Set uncertainties to noise level ********/
   for(i = 1; i <= ndata; i++)
      sig[i] = noiseLevel;

/* Fit data ********************************/
   TextMessage("\n\n");
   nit = 0;
   lamda = -1.0;
   do
   {
      nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda);
      nit++;
      sprintf(str,"Interation #%ld",nit);
      SetPlotLabel(plot1,str);
   }
   while(chisq > min_chisq && nit < max_it);
   lamda = 0.0;
   nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda);

/* Print out results of data ***************/
   TextMessage("  Peak  x (ppm)  Amplitude  Width (Hz)  Integral\n\n");

   for(i = 1; i <= nrPar; i+=3)
   {
      TextMessage("%5ld% 9.3f%12.3e%9.3f%13.3e\n",i/3+1,PointsToPPM(a[i+1]),a[i],2.0/sqrt(a[i+2]),PI*a[i]/sqrt(a[i+2]));
   }
   
/* Print out statistics ***********************/
   TextMessage("\n   Normalised chi-squared ... %2.2f\n",chisq/(right-left+1));

/* Write to buffer 1 using fit parameters */
   {
      float *dyda,val;
      complex *buf;
      
      buf = *bufHandle[1];
      dyda = MakeVectorNR(1L,ma);

      for(i = 1; i <= ndata; i++)
      {
         function(x[i],a,&val,dyda,nrPar);
         buf[i+left-1].r = val;
      }

      FreeVectorNR(dyda,1L,ma);   
   }

/* Update peak parameters if required *********/
   if(update == 'y')
   {
      for(i = 1; i <= nrPar; i+=3)
      {
         j = pNr[i];
         peak[j].x = PointsToPPM(a[i+1]);
         peak[j].y = a[i];
         peak[j].width = 2.0/sqrt(a[i+2]);
         peak[j].integral = PI*a[i]/sqrt(a[i+2]);
      }
   } 

/* Free memory */
   FreeIVectorNR(pNr,1L,ndata);
   FreeIVectorNR(ia,1L,ndata);
   FreeIVectorNR(fix,1L,ndata);
   FreeVectorNR(temp,1L,ndata);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   FreeVectorNR(a,1L,ndata);   
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   
   onedModified = 1;
   return(0);
}

short
VelDifFit(char arg[])

{
   static float noiseLevel;
   long *ia,*fix;
   float *x,*y,*sig,*a,*temp;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 50;
   complex *data;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   float amp,freq,phase,decay,offset;
         
   type = VEL_DIFF;
   
/* Get info from user ******************************/

   if(arg[0] == '\0')
   {
      sprintf(arg,"\n\n   maxit noise amp freq phase decay offset ... ");
      if(GetStringFromUser(arg,49) == -1) return(0);
   }     

   if((nrArgs = str_scan(arg,"%ld %vf %vf %vf %vf %vf %vf",
                         &max_it,&noiseLevel,&amp,&freq,&phase,&decay,&offset)) == -1)
   {
      return(-1);
   }
   else if(nrArgs < 4)
   { 
      return(-2);
   }

   if(noiseLevel < 0)
   {
      ErrorMessage("noise level must be positive");
      return(-1);
   }
         
   nrPar = 5;
   ndata = (**plot1).maxRect.right;
            
/* Allocate memory ***************************/
   
   ia = MakeIVectorNR(1L,ndata);
   fix = MakeIVectorNR(1L,ndata);
   temp = MakeVectorNR(1L,ndata);
   x = MakeVectorNR(1L,ndata);
   y = MakeVectorNR(1L,ndata);
   sig = MakeVectorNR(1L,ndata);
   a = MakeVectorNR(1L,ndata);
 
/* Load data from buffer 0 *******************/

   data = *data1DHandle;
   for(i = 1; i <= ndata; i++)
   {
      x[i] = data[i-1].r;
      y[i] = data[i-1].i;
   }
   
/* Initialize some data values ****************/

   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
/* Estimate initial parameter values ***********/

   a[1] = amp;
   a[2] = freq;
   a[3] = phase;
   a[4] = decay;
   a[5] = offset;

/* Set uncertainties to noise level ********/

   if(noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;  
   }
   for(i = 1; i <= ndata; i++)
        sig[i] = noiseLevel;

/* Fit data ********************************/

   nit = 0;
   lamda = -1.0;
   chisq = 1e9;
   do
   {
      chisqold = chisq;
      nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda);
      nit++;
      TextMessage("\n\n     lamda chisq ............ %g %g",lamda,chisq);      
   }
   while(nit <= max_it);
   lamda = 0.0;
   nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda);
               
/* Print out results of data ***************/

   if(calcNoise)
   {
      NL = sqrt(chisq/(ndata-nrPar));
      TextMessage("\n\n     noise ............ %2.3f",NL);
   }
   else
   {
      NL = 1;
      TextMessage("\n");
   }
   TextMessage("\n     Iterations ........ %ld",nit);
   TextMessage("\n     amp .............. %2.3e ± %2.3e",a[1],sqrt(covar[1][1])*NL);
   TextMessage("\n     freq ................. %2.3e ± %2.3e",a[2],sqrt(covar[2][2])*NL);
   TextMessage("\n     phase ................. %2.3e ± %2.3e",a[3],sqrt(covar[3][3])*NL); 
   TextMessage("\n     decay ................. %2.3e ± %2.3e",a[4],sqrt(covar[4][4])*NL); 
         
/* Print out statistics ***********************/

   TextMessage("\n     Normalised chi-squared ... %2.2f\n",chisq/(ndata-nrPar)/SQR(NL));

/* Write to buffer 1 using fit parameters *****/

   {
      float *dyda,val;
      complex *buf;
      
      buf = *bufHandle[1];
      dyda = MakeVectorNR(1L,ma);

      for(i = 1; i <= ndata; i++)
      {
         function(x[i],a,&val,dyda,nrPar);
         buf[i-1].r = x[i];
         buf[i-1].i = val;
      }

      FreeVectorNR(dyda,1L,ma);   
   }

/* Free memory ********************************/

   FreeIVectorNR(ia,1L,ndata);
   FreeIVectorNR(fix,1L,ndata);
   FreeVectorNR(temp,1L,ndata);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   FreeVectorNR(a,1L,ndata);   
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);

   onedModified = 1;
   return(0);
}

#endif

/****************************************************************************************************
* Calculate the diffusion coefficient from PGSE data for the case where the gradient (g) is varied  *
****************************************************************************************************/

short DiffFit(DLLParameters* par, char *arg)
{
   static float noiseLevel;
   static char diffStr[50],differrStr[50],E0Str[50],E0errStr[50];
   short r;
   float var1,var2;
   Variable varX;
   Variable varY;
   bool err = false;
   char report[50];
   char typeStr[50];
         
 
// Get parameters from user ******************************

  if((r = ArgScan(par->itfc, arg,7,"type, x, y, noise, var1, var2","eeeeeee","svvfffs",typeStr, &varX,&varY,&noiseLevel,&var1,&var2,report)) < 0)
     return(r);
  
// Check for valid y array *******************
   if(varY.GetType() == MATRIX2D)
   {
      float temp;
      for(int i = 0; i < varY.GetDimX(); i++)
      {
         temp = VarRealMatrix(&varY)[0][i-1];
         if(temp <= 0)
         {
            err = true;
            break;
         }
      }
   }
   else
   {
      ErrorMessage("y value should be an array");
      return(ERR);
   } 


// Choose type

   if(!strcmp(typeStr,"grad"))
   {
      smdel = var1;
      lgdel = var2;
      if(smdel <= 0 || lgdel <= 0)
      {
         ErrorMessage("small or large delta <= 0");
         return(ERR);
      } 
      if(err) 
      {
         TextMessage("Diffusive attenuation array has some values <= 0\n");
       //  return(ERR);
      }
      type = DIFF_FIT1;
   }
   else if(!strcmp(typeStr,"sdel"))
   {
      lgdel = var1;
      grad = var2;  
      if(lgdel <= 0 || grad <= 0)
      {
         ErrorMessage("large delta or gradient <= 0");
         return(ERR);
      } 
      if(err) 
      {
         ErrorMessage("Small delta is <= 0");
         return(ERR);
      }
      type = DIFF_FIT2;
   }
   else if(!strcmp(typeStr,"ldel"))
   {
      grad = var1;
      smdel = var2;  
      if(smdel <= 0 || grad <= 0)
      {
         ErrorMessage("small delta or gradient <= 0");
         return(ERR);
      } 
      if(err) 
      {
         ErrorMessage("Large delta is <= 0");
         return(ERR);
      }
      type = DIFF_FIT3;
   }
   else
   {
      ErrorMessage("invalid fit type");
      return(ERR);
   }   
   
// Calculate diffusion coefficient **********************

   return(GradCalc(par,&varX,&varY,noiseLevel,report));
}


/********************************************************************************************************
* A non-linear fitting routines which can handle PGSE data. Output is diffusion coefficient inital      *
* echo amplitude and errors                                                                             *
********************************************************************************************************/
      
short GradCalc(DLLParameters* par, Variable *varX, Variable *varY, float noiseLevel, char *report)
{
   long *ia,*fix;
   float *x,*y,*sig,*a,*temp;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar=2;
   float min_chisq = 1;
   long max_it = 50;
   long min_it = 5;
   float NL;
   short calcNoise = 0;
   short err = OK;
   
   if(noiseLevel < 0)
   {
      ErrorMessage("noise level must be positive");
      return(ERR);
   }

// Check for input errors *************************************************
   if(VarRowSize(varX) != 1 || VarRowSize(varY) != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }
   
   ndata = VarColSize(varX);

   if(ndata < nrPar)
   {
      ErrorMessage("Should be more data points than parameters");
      return(ERR);
   }
      
   if(VarColSize(varX) != VarColSize(varY))
   {
      ErrorMessage("x & y vectors do not have same length");
      return(ERR);
   }
            
// Allocate memory ***************************  
   ia   = MakeIVectorNR(1L,ndata);
   fix  = MakeIVectorNR(1L,ndata);
   temp = MakeVectorNR(1L,ndata);
   x    = MakeVectorNR(1L,ndata);
   y    = MakeVectorNR(1L,ndata);
   sig  = MakeVectorNR(1L,ndata);
   a    = MakeVectorNR(1L,ndata);

// Copy data to x,y arrays *******************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(varX)[0][i-1];
      y[i] = VarRealMatrix(varY)[0][i-1];
   }

// Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
// Estimate initial parameter values ***********/
   a[1] = y[1];
   if(a[1] <= 0)
   {
      ErrorMessage("data set unsuited to diff fit");
      err = ERR;
      goto ex;;
   }

   int midPos;

// Find the midpoint
   midPos = ndata/2+1;

// Estimate diffusion coefficient
   if(type == DIFF_FIT1) // Grad
   {
      double gamma = 2.6752e+008;
      if(y[midPos] <= 0)
      {
         ErrorMessage("Can't estimate diffusion coeff. since middle data point is negative");
         err = ERR;
         goto ex;
      }
      if((lgdel-smdel)/3 <= 0)
      {
         ErrorMessage("Can't estimate diffusion coeff. since (lgdel-smdel)/3 <= 0");
         err = ERR;
         goto ex;
      }
      a[2] = -log((double)y[midPos]/(double)a[1])/(gamma*gamma*x[midPos]*smdel*smdel*(lgdel-smdel/3));
   }
   else if(type == DIFF_FIT2) // Small delta
   {
      double gamma = 2.6752e+008;
      if(y[midPos] <= 0)
      {
         ErrorMessage("Can't estimate diffusion coeff. since middle data point is negative");
         err = ERR;
         goto ex;
      }
      if((lgdel-x[midPos]/3) <= 0)
      {
         ErrorMessage("Can't estimate diffusion coeff. since (lgdel-smdel)/3 <= 0");
         err = ERR;
         goto ex;
      }
      a[2] = -log((double)y[midPos]/(double)a[1])/(gamma*gamma*grad*x[midPos]*x[midPos]*(lgdel-x[midPos]/3));
   }
   else if(type == DIFF_FIT3) // Large delta
   {
      double gamma = 2.6752e+008;
      if(y[midPos] <= 0)
      {
         ErrorMessage("Can't estimate diffusion coeff. since middle data point is negative");
         err = ERR;
         goto ex;
      }
      if((x[midPos]-smdel/3) <= 0)
      {
         ErrorMessage("Can't estimate diffusion coeff. since (lgdel-smdel)/3 <= 0");
         err = ERR;
         goto ex;
      }
      a[2] = -log((double)y[midPos]/(double)a[1])/(gamma*gamma*grad*smdel*(x[midPos]-smdel/3));
   }
   
   
// Set uncertainties to noise level ********/
   if(noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;  
   }
   for(i = 1; i <= ndata; i++)
     sig[i] = noiseLevel;

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 1e9;
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
	      err = ERR;
	      goto ex;   
	   }   
      nit++;
   }
   while(nit <  min_it || (fabs(chisq-chisqold) > chisqold/100.0 && nit <= max_it));
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;   
   }
                  
// Print out results of data ***************/
	if(calcNoise)
		NL = sqrt(chisq/(ndata-nrPar));
	else
		NL = 1;

   if(!strcmp(report,"yes"))
   {
	   if(calcNoise)
	      TextMessage("\n\n     noise ............ %2.3f",NL);
	   else
	      TextMessage("\n");

	   TextMessage("\n     Iterations ........ %ld",nit);
	   TextMessage("\n     E(0) .............. %2.3e ± %2.3e",a[1],sqrt(covar[1][1])*NL);
	   TextMessage("\n     D ................. %2.3e ± %2.3e",a[2],sqrt(covar[2][2])*NL);
	   
	  // Print out statistics ***********************/
	   TextMessage("\n     Normalised chi-squared ... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
	}
	   
// Make best fit vector (ans1/ans)
	float** bestFit = MakeMatrix2D(ndata,1);
	float val;
	float *dyda;
		
   dyda = MakeVectorNR(1L,ma);
		
   for(i = 1; i <= ndata; i++)
   {
	   function(x[i],a,&val,dyda,nrPar);
		bestFit[0][i-1] = val;
   }		
	FreeVectorNR(dyda,1L,ma);   

// Return to user
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);

// Update variables ***************************/
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(a[2]);
   par->retVar[4].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[5].MakeAndSetFloat(sqrt(covar[2][2])*NL);   
   par->nrRetVar = 5;

// Free memory ********************************/
ex:
   FreeIVectorNR(ia,1L,ndata);
   FreeIVectorNR(fix,1L,ndata);
   FreeVectorNR(temp,1L,ndata);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   FreeVectorNR(a,1L,ndata);   
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
              
   return(OK);
}
  

short ExpFit(long *ia,long *fix,float *temp,float *sig,float *a,float **covar,float **alpha,
             short t, long max_it,float *x, float *y,long ndata,float noiseLevel,
             float *E0,float *E0err,float *decay,float *decayErr)


{
   float chisq,lamda,chisqold;
   long ma,i,nit,nrPar;
   static float min_chisq = 1;
   short calcNoise = 0;
   float NL;
         
   nrPar = 2;
   
   if(t == 0) type = T2_FIT;
   if(t == 1) type = T1_VD_FIT;
   if(t == 2) type = T1_IR_FIT;
   if(t == 3) type = T1_IR_FIT_ABS;

/* Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

  
/* Estimate initial parameter values ***********/ 
   if(type == T2_FIT)
   { 
      a[1] = y[1];
      if(a[1] <= 0)
         return(-1);

      for(i = 1; i <= ndata; i++)
      {
         if(y[i] < a[1]*0.3679)
         {
            if(x[i] == 0) return(-1);
            a[2] = 1.0/x[i];
            break;
         }
      }
      if(i > ndata)
         a[2] = 1/x[ndata];
   } 
   else /* T1 fits */
   {
      a[1] = y[ndata];
      if(a[1] <= 0)
         return(-1);
    
      for(i = 1; i <= ndata; i++)
      {
         if(y[i] > a[1]*0.632)
         {
            if(x[i] == 0)
               return(-1);
            a[2] = 1.0/x[i];
            break;
         }
      }
      if(i > ndata)
         a[2] = 1/x[ndata];
   }

/* Allow for zero noise input **************/    
   if(noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;  
   }
          
/* Set uncertainties to noise level ********/
   for(i = 1; i <= ndata; i++)
      sig[i] = noiseLevel;

/* Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; //Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == -1)
         return(-2);
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == -1)
      return(-2);   

   if(calcNoise)
   {
      NL = sqrt(chisq/(ndata-nrPar));
   }
   else
   {
      NL = 1;
   }

/* Return parameters ***********************/      
   *E0 = a[1];
   *E0err = sqrt(covar[1][1])*NL;
   *decay = a[2];
   *decayErr = sqrt(covar[2][2])*NL;
      
   return(0);
}

/****************************************************************************
*                    Levenberg-Marquardt nonlinear fit                      *
****************************************************************************/

short nlfit(float x[],float y[],float sig[],long ndata,float a[], long ia[], long ma,
            float **covar,float **alpha,float *chisq,
            void (*funcs)(float,float[],float*,float[],long),float *alamda)
{
   long j,k,l,m;
   static long mfit;
   static float ochisq,*atry,*beta,*da,**oneda;
   
// Check for invalid values
   for(j = 1; j <= ndata; j++)
   {
      if(isnan(x[j]) || !_finite(x[j]))
      {
         ErrorMessage("invalid value in x matrix");
         return(-1);
      }
      if(isnan(y[j]) || !_finite(y[j]))
      {
         ErrorMessage("invalid value in y matrix");
         return(-1);
      }
   }

   // Intialisation condition
   if(*alamda < 0.0)
   {
      atry = MakeVectorNR(1L,ma);
      beta = MakeVectorNR(1L,ma);
      da = MakeVectorNR(1L,ma);
      for(mfit = 0, j = 1; j <= ma; j++)
         if(ia[j]) mfit++;
      oneda = MakeMatrix2DNR(1L,mfit,1L,1L);
      *alamda = 0.001;
      mrqcof(x,y,sig,ndata,a,ia,ma,alpha,beta,chisq,funcs);
      ochisq = (*chisq);
      for(j = 1; j <= ma; j++)
         atry[j] = a[j];
   }

   for(j = 0, l = 1; l <= ma; l++)
   {
      if(ia[l])
      {
         for(j++,k = 0, m = 1; m <= ma; m++)
         {
            if(ia[m])
            {
               k++;
               covar[j][k] = alpha[j][k];
            }
         }
         covar[j][j] = alpha[j][j]*(1.0+(*alamda));
         oneda[j][1] = beta[j];
      }
   }

   if(gaussj(covar,mfit,oneda,1L))
   {
      FreeMatrix2DNR(oneda,1L,mfit,1L,1L);
      FreeVectorNR(da,1L,ma);
      FreeVectorNR(beta,1L,ma);
      FreeVectorNR(atry,1L,ma); 
      return(-1);
   }
   
   for(j = 1; j <= mfit; j++)
      da[j] = oneda[j][1];


   if(*alamda == 0.0)
   {
      covsrt(covar,ma,ia,mfit);
      FreeMatrix2DNR(oneda,1L,mfit,1L,1L);
      FreeVectorNR(da,1L,ma);
      FreeVectorNR(beta,1L,ma);
      FreeVectorNR(atry,1L,ma); 
      return(0);
   }

   for(j = 0, l = 1; l <= ma; l++) 
   {
      if(ia[l]) 
         atry[l] = a[l] + da[++j];
   }

   mrqcof(x,y,sig,ndata,atry,ia,ma,covar,da,chisq,funcs);

   if(*chisq < ochisq)
   {
      *alamda *= 0.1;
      ochisq = (*chisq);
      for(j = 0, l = 1; l <= ma; l++)
      {
         if(ia[l])
         {
            for(j++, k = 0, m = 1; m <= ma; m++)
            {
               if(ia[m])
               {
                  k++;
                  alpha[j][k] = covar[j][k];
               }
            }
            beta[j] = da[j];
            a[l] = atry[l];
         }
      }
   }
   else
   {
      *alamda *= 10.0;
      *chisq = ochisq;
   }
   return(0);
}

/****************************************************************************
*           Calculate matricies alpha and beta and chisq number             *
****************************************************************************/

void mrqcof(float x[],float y[],float sig[],long ndata,float a[],
            long ia[],long ma,float **alpha,float beta[],float *chisq,
            void (*funcs)(float,float[],float*,float[],long))
{
   long i,j,k,l,m,mfit = 0;
   float ymod,wt,sig2i,dy,*dyda;

   dyda = MakeVectorNR(1L,ma);

   for(j = 1; j <= ma; j++)
   {
      if(ia[j]) mfit++;
   }
   for(j = 1; j <= mfit; j++)
   {
      for(k = 1; k <= j; k++)
         alpha[j][k] = 0.0;
      beta[j] = 0.0;
   }

   *chisq = 0.0;
   for(i = 1; i <= ndata; i++)
   {
      (*funcs)(x[i],a,&ymod,dyda,ma);
      sig2i = 1.0/(sig[i]*sig[i]);
      dy = y[i] - ymod;
      for(j = 0, l = 1; l <= ma; l++)
      {
         if(ia[l])
         {
            wt = dyda[l]*sig2i;
            for(j++, k = 0, m = 1; m <= l; m++)
            {
               if(ia[m])
                 alpha[j][++k] += wt*dyda[m];
            }
            beta[j] += dy*wt;
         }
      }
      (*chisq) += dy*dy*sig2i;
   }
   for(j = 2; j <= mfit; j++)
      for(k = 1; k < j; k++)
         alpha[k][j] = alpha[j][k];
   FreeVectorNR(dyda,1L,ma);
}
    
/****************************************************************************
*             Reorder covariance matrix so we can extract errors            *  
****************************************************************************/

    
void covsrt(float **covar,long ma,long ia[], long mfit)
{
   long i,j,k;

   for(i = mfit+1; i <= ma; i++)
      for(j = 1; j <= i; j++)
         covar[i][j] = covar[j][i] = 0.0;

   k = mfit;
   for(j = ma; j >= 1; j--)
   {
      if(ia[j])
      {
         for(i = 1; i <= ma; i++) Swap(covar[i][k],covar[i][j]);
         for(i = 1; i <= ma; i++) Swap(covar[k][i],covar[j][i]);
         k--;
      }
   }
}
   

/****************************************************************************
*   Routine to evaluate a function and its derivatives                      *  
****************************************************************************/

void function(float x,float a[], float *y,float dydx[], long na)
{
   long i;
   float fac,ex,arg;

   *y = 0.0;
   
   switch(type)
   {
	   case(LORENTZIAN):   // Lorentzian fit y = a/(1+c*(x-b)^2)
	   {
	      for(i = 1; i <= na-1; i+=3)
	      {
	         fac = (x-a[i+1]);
	         arg = (1+a[i+2]*fac*fac);
	         *y += a[i]/arg;
	         dydx[i] = 1/arg;
	         dydx[i+1] = a[i]/(arg*arg)*2*a[i+2]*fac;
	         dydx[i+2] = -a[i]/(arg*arg)*fac*fac;
	      }  
	      break; 
	   }
	   case(PEAKFIT):   // Lorentzian fit y = a/(1+c^2*(x-b)^2)
	   {
	      for(i = 1; i <= na-1; i+=3)
	      {
	         fac = (x-a[i+1]);
	         arg = (1+a[i+2]*a[i+2]*fac*fac);
	         *y += a[i]/arg;
	         dydx[i] = 1/arg;
	         dydx[i+1] = a[i]/(arg*arg)*2*a[i+2]*a[i+2]*fac;
	         dydx[i+2] = -a[i]/(arg*arg)*fac*fac*2*a[i+2];
	      }  
	      break; 
	   }
	   case(GAUSSIAN):   // Gaussian fit  y = a*exp(-(x-b)^2/(2*c^2))
	   {
	      for(i = 1; i <= na-1; i+=3)
	      {
	         arg = (x-a[i+1])/a[i+2];
	         ex = exp(-arg*arg/2);
	         fac = a[i]*ex*arg;
	         *y += a[i]*ex;
	         dydx[i] = ex;
	         dydx[i+1] = fac/a[i+2];
	         dydx[i+2] = fac*arg/a[i+2];
	      }
	      break;
	   }
	   case(T1_VD_FIT):   // T1 variable delay fit y = a[1]*(1.0 - exp(-x*a[2]))
	   {
	      ex = exp(-x*a[2]);
	      fac = (1.0-ex);
	      *y = a[1]*fac;
	      dydx[1] = fac;
	      dydx[2] = a[1]*x*ex;
	      break;
	   }
	   case(T1_IR_FIT): // T1 inversion recovery fit y = a[1]*(1.0 - 2.0*exp(-x*a[2]))
	   {
	      ex = exp(-x*a[2]);
	      fac = (1.0-2.0*ex);
	      *y = a[1]*fac;
	      dydx[1] = fac;
	      dydx[2] = a[1]*x*2.0*ex;
	      break;
	   }
      case(T1_IR_BIEXP_FIT): // Bi exponential T1 inversion recovery fit y = a[1]*(1.0 - 2.0*exp(-x*a[2]) + a[3]*(1.0 - 2.0*exp(-x*a[4]))
      {
         *y = a[1]*(1.0 - 2.0 * exp(-x * a[2])) + a[3]*(1.0 - 2.0 * exp(-x * a[4]));
         dydx[1] = (1.0 - 2.0 * exp(-x * a[2]));
         dydx[2] = a[1] * x * 2.0 * exp(-x * a[2]);
         dydx[3] = (1.0 - 2.0 * exp(-x * a[4]));
         dydx[4] = a[3] * x * 2.0 * exp(-x * a[4]);
         break;
      }
	   case(T1_IR_FIT_ABS): // T1 inversion recovery fit y = |a[1]*(1.0 - 2.0*exp(-x*a[2]))|
	   {
	      ex = exp(-x*a[2]);
	      fac = (1.0-2.0*ex);
	      *y = a[1]*fac;
         if(*y >= 0)
         {
	         dydx[1] = fac;
	         dydx[2] = a[1]*x*2.0*ex;
         }
         else
         {
            *y = -*y;
	         dydx[1] = -fac;
	         dydx[2] = -a[1]*x*2.0*ex;
         }
	      break;
	   }
	   case(T2_FIT): // Single exponential fit  y = a[1]*exp(-x*a[2])
	   {
	      ex = exp(-x*a[2]);
	      *y = a[1]*ex;
	      dydx[1] = ex;
	      dydx[2] = -a[1]*x*ex;
         cnt++;
	      break;
	   }
	   case(EXP_OFF_FIT): // Exp fit with offset y = a[1]*exp(-x*a[2])) + a[3]
	   {
	      ex = exp(-x*a[2]);
	      *y = a[1]*ex + a[3];
	      dydx[1] = ex;
	      dydx[2] = -a[1]*x*ex;
	      dydx[3] = 1;
	      break;
	   }
	   case(DIFF_FIT1): // Vary gradient i.e.  y = a[1]*exp(-gamma^2*x^2*sdel^2*a[2]*(lgdel-smdel/3))
	   {
	      fac = sqr(smdel*2.69e8*x)*(lgdel-smdel/3.0);
	      ex = exp(-fac*a[2]);
	      *y = a[1]*ex;
	      dydx[1] = ex;
	      dydx[2] = -fac*a[1]*ex;
	      break;	      
	   }
      case(DIFF_FIT2): // Vary smdel  i.e.  y = a[1]*exp(-gamma^2*grad^2*x^2*a[2]*(lgdel-x/3))
	   {
	      fac = sqr(x*2.69e8*grad)*(lgdel-x/3.0);
	      ex = exp(-fac*a[2]);
	      *y = a[1]*ex;
	      dydx[1] = ex;
	      dydx[2] = -fac*a[1]*ex;
	      break;	      
	   }
      case(DIFF_FIT3):  // Vary lgdel  i.e.   y = a[1]*exp(-gamma^2*grad^2*sdel^2*a[2]*(x-smdel/3))
	   {
	      fac = sqr(smdel*2.69e8*grad)*(x-smdel/3.0);
	      ex = exp(-fac*a[2]);
	      *y = a[1]*ex;
	      dydx[1] = ex;
	      dydx[2] = -fac*a[1]*ex;
	      break;	      
	   }
      case(VEL_DIFF):  // Vary V and D 
	   {
	      *y = a[1]*cos(2*PI*x*a[2] + a[3])*exp(-x*x*a[4]) + a[5];
	      dydx[1] = cos(2*PI*x*a[2] + a[3])*exp(-x*x*a[4]);
	      dydx[2] = -2*PI*x*a[1]*sin(2*PI*x*a[2] + a[3])*exp(-x*x*a[4]);
	      dydx[3] = -a[1]*sin(2*PI*x*a[2] + a[3])*exp(-x*x*a[4]);
	      dydx[4] = -x*x*a[1]*cos(2*PI*x*a[2] + a[3])*exp(-x*x*a[4]);
	      dydx[5] = 1;
	      break;	      
	   }
      case(BI_EXP_FIT): // Bi-exponential fit 
	   {
	      *y = a[1]*exp(-x*a[2]) + a[3]*exp(-x*a[4]);
	      dydx[1] = exp(-x*a[2]);
	      dydx[2] = -x*a[1]*exp(-x*a[2]);
	      dydx[3] = exp(-x*a[4]);
	      dydx[4] = -x*a[3]*exp(-x*a[4]);
	      break;	      
	   }
      case(TRI_EXP_FIT): // Tri-exponential fit 
	   {
	      *y = a[1]*exp(-x*a[2]) + a[3]*exp(-x*a[4]) + a[5]*exp(-x*a[6]);
	      dydx[1] = exp(-x*a[2]);
	      dydx[2] = -x*a[1]*exp(-x*a[2]);
	      dydx[3] = exp(-x*a[4]);
	      dydx[4] = -x*a[3]*exp(-x*a[4]);
	      dydx[5] = exp(-x*a[6]);
	      dydx[6] = -x*a[5]*exp(-x*a[6]);
	      break;	      
	   }
      case(LOG_NORMAL): // Log-normal fit
      {
         fac = exp(-0.5*sqr(log(x/a[2])/a[3]));
         *y = a[1]*fac;
         dydx[1] = fac;
         dydx[2] = a[1]*log(x/a[2])*fac/(a[3]*a[3]*a[2]);
         dydx[3] = a[1]*sqr(log(x/a[2]))*fac/(a[3]*a[3]*a[3]);
         break;
      }
      case(CAP_FIT): // Capillary pressure fit
      {
         float P = a[1];
         float k = a[2];
         float S = a[3];

         *y = exp(log(x)/k - log(P)/k + log(1-S)) + S;
         dydx[1] = 1/k*(S-1)*pow(x,1/k)*pow(P,-(k+1)/k);
         dydx[2] = 1/k*(S-1)*pow(P,-1/k)*pow(x,1/k)*(log(P)-log(x));
         dydx[3] = 1-pow(P,-1/k)*pow(x,1/k);
         break;
      }
      case(GENERAL_FIT): // General fit - call Prospa function
      {
         CText arg, temp;
         arg.Format("%s(%f,",nonLinearfunction->Str(),x);
         for(int i = 1; i < na; i++)
         {
            temp.Format("%f,",a[i]);
            arg.Concat(temp.Str());
         }
         temp.Format("%f)",a[na]);
         arg.Concat(temp.Str());
         ProcessMacroStr(callingInterface,1,arg.Str());
         int nrRetArgs = *(int*)((char*)callingInterface+76);     // Interface offset for nrRetValues
         Variable *v = *(Variable**)((char*)callingInterface+88); // Interface offset for retVar
         *y = v[1].GetReal();
         for(int i = 1; i < nrRetArgs; i++)
            dydx[i] = v[i+1].GetReal();
         break;
      }
	}
}


/****************************************************************************
*                                                                           *
*                                gaussj.c                                   *
*                                                                           *
*                                                                           *
*   Gauss-Jordan elimination. (see Numerical Recipies, p. 24)               *
*                                                                           *
*                                                Craig Eccles JULY 93       *
*                                                                           *
****************************************************************************/

short  gaussj(float **a, long n, float **b, long m)
{
	long *ipiv,*indxr,*indxc;
	long j,i,k,irow=1,icol=1,l,ll;
	float big,dum,pivinv;

	indxc = MakeIVectorNR(1L,n);
	indxr = MakeIVectorNR(1L,n);
	ipiv = MakeIVectorNR(1L,n);
   icol = 0;

 //  TextMessage("n = %ld\n",n);

   for(j = 1; j <= n; j++) /* initialize pivots */
	   ipiv[j] = 0;

   for(i = 1; i <= n; i++)
   {
      big = 0.0;
      for(j = 1; j <= n; j++) /* Find largest element in a[][]     */
      {                       /* in a row and column which has not */
         if(ipiv[j] != 1)     /* already been used as a pivot.     */
         {
	         for(k = 1; k <= n; k++)
            {
	            if(ipiv[k] == 0)
		         {
		            if(fabs(a[j][k]) >= big)
		            {
		               big = fabs(a[j][k]);
		               irow = j;
		               icol = k;
		            }
	            }
		         else if(ipiv[k] > 1)
		         {
		             ErrorMessage("Singular matrix in Gauss-Jordan elimination");
                   FreeIVectorNR(ipiv,1L,n);
                   FreeIVectorNR(indxr,1L,n);
                   FreeIVectorNR(indxc,1L,n);
		             return(1);
		         }
            }
         }
      }

      if(icol == 0.0)
      {
         ErrorMessage("Gauss-Jordan elimination error");
         FreeIVectorNR(ipiv,1L,n);
         FreeIVectorNR(indxr,1L,n);
         FreeIVectorNR(indxc,1L,n);
         return(1);
      }

      ++(ipiv[icol]); /* Note this pivot */

      if(irow != icol) /* Not diagonal pivot? then Swap rows */
      {
	     for(l = 1; l <= n; l++) 
           Swap(a[irow][l],a[icol][l]);
	     for(l = 1; l <= m; l++)
           Swap(b[irow][l],b[icol][l]);
      }
      indxr[i] = irow;
      indxc[i] = icol;
      if(a[icol][icol] == 0.0)
      {
         ErrorMessage("Singular matrix in Gauss-Jordan elimination");
         FreeIVectorNR(ipiv,1L,n);
         FreeIVectorNR(indxr,1L,n);
         FreeIVectorNR(indxc,1L,n);
         return(1);
      }
      pivinv = 1.0/a[icol][icol];
      a[icol][icol] = 1.0;

      for(l = 1; l <= n; l++) a[icol][l] *= pivinv;
      for(l = 1; l <= m; l++) b[icol][l] *= pivinv;

      for(ll = 1; ll <= n; ll++)
      {
         if(ll != icol)
	      {
	         dum = a[ll][icol];
	         a[ll][icol] = 0.0;
	         for(l = 1; l <= n; l++) a[ll][l] -= a[icol][l] * dum;
	         for(l = 1; l <= m; l++) b[ll][l] -= b[icol][l] * dum;
	      }
      }
   }
   for(l = n; l >= 1; l--)
   {
       if(indxr[l] != indxc[l])
       {
	       for(k = 1; k <= n; k++)
	       Swap(a[k][indxr[l]],a[k][indxc[l]]);
       }
   }
   FreeIVectorNR(ipiv,1L,n);
   FreeIVectorNR(indxr,1L,n);
   FreeIVectorNR(indxc,1L,n);
   return(0);
}



/****************************************************************************
*                                                                           *
*                                gaussj.c                                   *
*                                                                           *
*                                                                           *
*   Gauss-Jordan elimination. (see Numerical Recipies, p. 24)               *
*                                                                           *
*                                                Craig Eccles JULY 93       *
*                                                                           *
****************************************************************************/

short  dgaussj(double **a, long n, double **b, long m)
{
	long *ipiv,*indxr,*indxc;
	long j,i,k,irow=1,icol=1,l,ll;
	double big,dum,pivinv;

	indxc = MakeIVectorNR(1L,n);
	indxr = MakeIVectorNR(1L,n);
	ipiv = MakeIVectorNR(1L,n);
   icol = 0;

 //  TextMessage("n = %ld\n",n);

   for(j = 1; j <= n; j++) /* initialize pivots */
	   ipiv[j] = 0;

   for(i = 1; i <= n; i++)
   {
      big = 0.0;
      for(j = 1; j <= n; j++) /* Find largest element in a[][]     */
      {                       /* in a row and column which has not */
         if(ipiv[j] != 1)     /* already been used as a pivot.     */
         {
	         for(k = 1; k <= n; k++)
            {
	            if(ipiv[k] == 0)
		         {
		            if(fabs(a[j][k]) >= big)
		            {
		               big = fabs(a[j][k]);
		               irow = j;
		               icol = k;
		            }
	            }
		         else if(ipiv[k] > 1)
		         {
		             ErrorMessage("Singular matrix in Gauss-Jordan elimination");
                   FreeIVectorNR(ipiv,1L,n);
                   FreeIVectorNR(indxr,1L,n);
                   FreeIVectorNR(indxc,1L,n);
		             return(1);
		         }
            }
         }
      }

      if(icol == 0.0)
      {
         ErrorMessage("Gauss-Jordan elimination error");
         FreeIVectorNR(ipiv,1L,n);
         FreeIVectorNR(indxr,1L,n);
         FreeIVectorNR(indxc,1L,n);
         return(1);
      }

      ++(ipiv[icol]); /* Note this pivot */

      if(irow != icol) /* Not diagonal pivot? then Swap rows */
      {
	     for(l = 1; l <= n; l++) 
           Swap(a[irow][l],a[icol][l]);
	     for(l = 1; l <= m; l++)
           Swap(b[irow][l],b[icol][l]);
      }
      indxr[i] = irow;
      indxc[i] = icol;
      if(a[icol][icol] == 0.0)
      {
         ErrorMessage("Singular matrix in Gauss-Jordan elimination");
         FreeIVectorNR(ipiv,1L,n);
         FreeIVectorNR(indxr,1L,n);
         FreeIVectorNR(indxc,1L,n);
         return(1);
      }
      pivinv = 1.0/a[icol][icol];
      a[icol][icol] = 1.0;

      for(l = 1; l <= n; l++) a[icol][l] *= pivinv;
      for(l = 1; l <= m; l++) b[icol][l] *= pivinv;

      for(ll = 1; ll <= n; ll++)
      {
         if(ll != icol)
	      {
	         dum = a[ll][icol];
	         a[ll][icol] = 0.0;
	         for(l = 1; l <= n; l++) a[ll][l] -= a[icol][l] * dum;
	         for(l = 1; l <= m; l++) b[ll][l] -= b[icol][l] * dum;
	      }
      }
   }
   for(l = n; l >= 1; l--)
   {
       if(indxr[l] != indxc[l])
       {
	       for(k = 1; k <= n; k++)
	          Swap(a[k][indxr[l]],a[k][indxc[l]]);
       }
   }
   FreeIVectorNR(ipiv,1L,n);
   FreeIVectorNR(indxr,1L,n);
   FreeIVectorNR(indxc,1L,n);
   return(0);
}

// Try and fit 2 decaying exponentials to the suppied xy data

short BiExpFit(DLLParameters* par, char *parameters)
{
   long *ia,*fix;
   float *x,*y,*sig,*a,*temp;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   char report[50];
   short err = OK;
      
   type = BI_EXP_FIT;
   
// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc,parameters,2,"x, y, [[noise], [report]","eeeee","vvfsl",&varX,&varY,&noiseLevel,report,&max_it)) < 0)
     return(nrArgs);  

// Check for errors *************************/   
   if(noiseLevel < 0)
   {
      ErrorMessage("noise level must be positive");
      return(ERR);
   }

// Check for input errors *************************************************
   if(VarRowSize(&varX) != 1 || VarRowSize(&varY) != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }
   
   ndata = VarColSize(&varX);

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }
      
   if(VarColSize(&varX) != VarColSize(&varY))
   {
      ErrorMessage("x & y vectors do not have same length");
      return(ERR);
   }
         
   nrPar = 4;
            
// Allocate memory ***************************/
   ia = MakeIVectorNR(1L,ndata);
   fix = MakeIVectorNR(1L,ndata);
   temp = MakeVectorNR(1L,ndata);
   x = MakeVectorNR(1L,ndata);
   y = MakeVectorNR(1L,ndata);
   sig = MakeVectorNR(1L,ndata);
   a = MakeVectorNR(1L,ndata);
 
// Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

// Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
// Estimate initial parameter values ***********/
   a[1] = y[1]/2;
   a[3] = y[1]/2;
   if(a[1] <= 0)
   {
      ErrorMessage("data set unsuited to bi-exp fit");
      err = ERR;
      goto ex;
   }
   
   for(i = 1; i <= ndata; i++)
   {
      if(y[i] < a[1]*0.3679)
      {
         if(x[i] == 0)
         {
            ErrorMessage("invalid x axis");
            err = ERR;
            goto ex;
         }
         a[2] = 1.0/x[i];
         a[4] = 1.0/x[i];
         break;
      }
   } 
   if(i > ndata)
   {
      a[2] = 1/x[ndata];  
      a[4] = 1/x[ndata];  
   }
          
// Set uncertainties to noise level ********/
   if(noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;  
   }
   for(i = 1; i <= ndata; i++)
      sig[i] = noiseLevel;

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }  
               
// Print out results of data ***************/
	if(calcNoise)
		NL = sqrt(chisq/(ndata-nrPar));
	else
		NL = 1;
		
   if(!strcmp(report,"yes"))
   {
		if(calcNoise)
			TextMessage("\n\n     noise ............. %2.3f",NL);
		else
			TextMessage("\n");
		TextMessage("\n     Iterations ......... %ld",nit);
		TextMessage("\n     Ea(0) ............... %2.3f +- %2.3f",a[1],sqrt(covar[1][1])*NL);
		TextMessage("\n     Ta .................. %2.3f +- %2.3f",1.0/a[2],sqrt(covar[2][2])/sqr(a[2])*NL);
		TextMessage("\n     Eb(0) ............... %2.3f +- %2.3f",a[3],sqrt(covar[3][3])*NL);
		TextMessage("\n     Tb .................. %2.3f +- %2.3f",1.0/a[4],sqrt(covar[4][4])/sqr(a[4])*NL);
		
	// Print out statistics ***********************/
		
		TextMessage("\n     Normalised chi-squared .... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
   }
   
// Make best fit vector (ans1 & ans)
	float** bestFit = MakeMatrix2D(ndata,1);
	float val;
	float *dyda;
		
   dyda = MakeVectorNR(1L,ma);
		
   for(i = 1; i <= ndata; i++)
   {
	   function(x[i],a,&val,dyda,nrPar);
		bestFit[0][i-1] = val;
   }		
	FreeVectorNR(dyda,1L,ma);   

// Return to user bestFit in ans and ans1 ****
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);
   
// Return other parameters in ans2 ... ans5
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(1.0/a[2]);
   par->retVar[4].MakeAndSetFloat(a[3]);
   par->retVar[5].MakeAndSetFloat(1.0/a[4]);
   par->retVar[6].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[7].MakeAndSetFloat(sqrt(covar[2][2])/sqr(a[2])*NL);   
   par->retVar[8].MakeAndSetFloat(sqrt(covar[3][3])*NL);
   par->retVar[9].MakeAndSetFloat(sqrt(covar[4][4])/sqr(a[4])*NL);   
   par->nrRetVar = 9;
   
// Free memory ********************************/
ex:
   FreeIVectorNR(ia,1L,ndata);
   FreeIVectorNR(fix,1L,ndata);
   FreeVectorNR(temp,1L,ndata);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   FreeVectorNR(a,1L,ndata);   
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
          
   return(err);
}



// Try and fit 3 decaying exponentials to the suppied xy data

short TriExpFit(DLLParameters* par, char *parameters)
{
   long *ia,*fix;
   float *x,*y,*sig,*a,*temp;
   float **covar,**alpha;
   float chisq,lamda,chisqold;
   long ma,i,ndata,nit,nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   char report[50];
   short err = OK;
      
   type = TRI_EXP_FIT;
   
// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc,parameters,2,"x, y, [noise], [report], [maxIter]","eeeee","vvfsl",&varX,&varY,&noiseLevel,report,&max_it)) < 0)
     return(nrArgs);  

// Check for errors *************************/   
   if(noiseLevel < 0)
   {
      ErrorMessage("noise level must be positive");
      return(ERR);
   }

// Check for input errors *************************************************
   if(VarRowSize(&varX) != 1 || VarRowSize(&varY) != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }
   
   ndata = VarColSize(&varX);

   if(ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }
      
   if(VarColSize(&varX) != VarColSize(&varY))
   {
      ErrorMessage("x & y vectors do not have same length");
      return(ERR);
   }
         
   nrPar = 6;
            
// Allocate memory ***************************/
   ia = MakeIVectorNR(1L,ndata);
   fix = MakeIVectorNR(1L,ndata);
   temp = MakeVectorNR(1L,ndata);
   x = MakeVectorNR(1L,ndata);
   y = MakeVectorNR(1L,ndata);
   sig = MakeVectorNR(1L,ndata);
   a = MakeVectorNR(1L,ndata);
 
// Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i-1];
      y[i] = VarRealMatrix(&varY)[0][i-1];
   }

// Initialize some data values ****************/
   ma = nrPar;
   for(i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeMatrix2DNR(1L,nrPar,1L,nrPar);
  
// Estimate initial parameter values ***********/
   a[1] = y[1]/2;
   a[3] = y[1]/2;
   a[5] = y[1]/2;
   if(a[1] <= 0)
   {
      ErrorMessage("data set unsuited to tri-exp fit");
      err = ERR;
      goto ex;
   }
   
   for(i = 1; i <= ndata; i++)
   {
      if(y[i] < a[1]*0.3679)
      {
         if(x[i] == 0)
         {
            ErrorMessage("invalid x axis");
            err = ERR;
            goto ex;
         }
         a[2] = 1.0/x[i];
         a[4] = 1.0/x[i];
         a[6] = 1.0/x[i];
         break;
      }
   } 
   if(i > ndata)
   {
      a[2] = 1/x[ndata];  
      a[4] = 1/x[ndata];  
      a[6] = 1/x[ndata];  
   }
          
// Set uncertainties to noise level ********/
   if(noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;  
   }
   for(i = 1; i <= ndata; i++)
      sig[i] = noiseLevel;

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }  
               
// Print out results of data ***************/
	if(calcNoise)
		NL = sqrt(chisq/(ndata-nrPar));
	else
		NL = 1;
		
   if(!strcmp(report,"yes"))
   {
		if(calcNoise)
			TextMessage("\n\n     noise ............... %2.3f",NL);
		else
			TextMessage("\n");
		TextMessage("\n     Iterations .......... %ld",nit);
		TextMessage("\n     Ea(0) ............... %2.3f +- %2.3f",a[1],sqrt(covar[1][1])*NL);
		TextMessage("\n     Ta .................. %2.3f +- %2.3f",1.0/a[2],sqrt(covar[2][2])/sqr(a[2])*NL);
		TextMessage("\n     Eb(0) ............... %2.3f +- %2.3f",a[3],sqrt(covar[3][3])*NL);
		TextMessage("\n     Tb .................. %2.3f +- %2.3f",1.0/a[4],sqrt(covar[4][4])/sqr(a[4])*NL);
		TextMessage("\n     Ec(0) ............... %2.3f +- %2.3f",a[5],sqrt(covar[5][5])*NL);
		TextMessage("\n     Tc .................. %2.3f +- %2.3f",1.0/a[6],sqrt(covar[6][6])/sqr(a[6])*NL);
		
	// Print out statistics ***********************/
		
		TextMessage("\n     Normalised chi-squared .... %2.2f\n",chisq/(ndata-nrPar)/sqr(NL));
   }
   
// Make best fit vector (ans1 & ans)
	float** bestFit = MakeMatrix2D(ndata,1);
	float val;
	float *dyda;
		
   dyda = MakeVectorNR(1L,ma);
		
   for(i = 1; i <= ndata; i++)
   {
	   function(x[i],a,&val,dyda,nrPar);
		bestFit[0][i-1] = val;
   }		
	FreeVectorNR(dyda,1L,ma);   

// Return to user bestFit in ans and ans1 ****
   par->retVar[1].MakeAndLoadMatrix2D(bestFit,ndata,1);
   FreeMatrix2D(bestFit);
   
// Return other parameters in ans2 ... ans5
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(1.0/a[2]);
   par->retVar[4].MakeAndSetFloat(a[3]);
   par->retVar[5].MakeAndSetFloat(1.0/a[4]);
   par->retVar[6].MakeAndSetFloat(a[5]);
   par->retVar[7].MakeAndSetFloat(1.0/a[6]);
   par->retVar[8].MakeAndSetFloat(sqrt(covar[1][1])*NL);
   par->retVar[9].MakeAndSetFloat(sqrt(covar[2][2])/sqr(a[2])*NL);   
   par->retVar[10].MakeAndSetFloat(sqrt(covar[3][3])*NL);
   par->retVar[11].MakeAndSetFloat(sqrt(covar[4][4])/sqr(a[4])*NL);
   par->retVar[12].MakeAndSetFloat(sqrt(covar[5][5])*NL);
   par->retVar[13].MakeAndSetFloat(sqrt(covar[6][6])/sqr(a[6])*NL);   
   par->nrRetVar = 13;
   
// Free memory ********************************/
ex:
   FreeIVectorNR(ia,1L,ndata);
   FreeIVectorNR(fix,1L,ndata);
   FreeVectorNR(temp,1L,ndata);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   FreeVectorNR(a,1L,ndata);   
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
          
   return(err);
}


