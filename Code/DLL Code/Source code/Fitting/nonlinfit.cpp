
/******************************************************************************
*                                                                             *
* Non-linear least squares fit. Uses Levenberg-Marquardt algorithm            *
*                                                                             *
*                                                                             *
*                                                                             *
*        Ref. Numerical Recipies p 545.           C Eccles JUNE 1993 - 2026   *
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

void function(float, float[], float*, float[], long);
short function_dbl(double, double[], double*, double[], long);

short nlfit(float x[],float y[],float sig[],long ndata,float a[], long ia[], long ma,
      float **covar,float **alpha,float *chisq,
      void (*funcs)(float,float[],float*,float[],long),float *lamda, CText method="gj");
short nlfit_dbl(double x[], double y[], double sig[], long ndata, double a[], long ia[], long ma,
   double** covar, double** alpha, double* chisq,
   short (*funcs_dbl)(double, double[], double*, double[], long), double* lamda, CText method="gj");

void covsrt(float** covar, long ma, long ia[], long mfit);
void covsrt_dbl(double** covar, long ma, long ia[], long mfit);

void mrqcof(float x[],float y[],float sig[],long ndata,float a[],
       long ia[],long ma,float **alpha,float beta[],float *chisq,
       void (*funcs)(float,float[],float*,float[],long));
short mrqcof_dbl(double x[], double y[], double sig[], long ndata, double a[],
   long ia[], long ma, double** alpha, double beta[], double* chisq,
   short (*funcs_dbl)(double, double[], double*, double[], long));

short gaussj(float **a, long n, float **b, long m);
short  gaussj_dbl(double** a, long n, double** b, long m);


short T1Fit(DLLParameters*,char*);
short T2Fit(DLLParameters*,char*);
short T2FitD(DLLParameters* par, char* parameters);

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

void print(const char* const text, ...);

bool debug = false; // Controls print outs of various vector and matrices

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
   CText returnMode = "float";
   short err = OK;
   CText fitMethod = "gj";

   nonLinearfunction = new CText;

   type = GENERAL_FIT;

// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc, parameters,4,"x, y, function, method, initialGuess, [[noise], [report], [returnMode], [iterations]]","eeeeeeeee","vvttvvtlt",&varX,&varY,nonLinearfunction,&fitMethod,&varGuess,&varNoise,&report,&max_it,&returnMode)) < 0)
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
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda, fitMethod) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
      print("Iteration %d\n", nit);
   }
   while(lamda > 1e-6f && nit <= max_it);
   lamda = 0.0;
   print("Final Iteration %d\n", nit);

   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda, fitMethod) == ERR)
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
            
      TextMessage("\n     Chi-squared .... %2.2f\n",chisq/(ndata-nrPar));
   }

   // Return variables

   if (returnMode == "struct0") // Zero based named variables p0, p0Err etc in a structure
   {
      par->retVar[1].MakeStruct();
      Variable* s = par->retVar[1].GetStruct();
      CText name;

      for (int i = 0; i < nrPar; i++)
      {
         name.Format("p%d", i);
         s->AddToStructure(name.Str(), a[i + 1]);
         name.Format("p%derr", i);
         s->AddToStructure(name.Str(), (float)sqrt(covar[i + 1][i + 1]) * NL);
      }
      par->retVar[2].MakeAndSetFloat(chisq / (ndata - nrPar));
      par->nrRetVar = 2;
   }
   else if (returnMode == "struct1") // One based named variables p1, p1Err etc in a structure
   {
      par->retVar[1].MakeStruct();
      Variable* s = par->retVar[1].GetStruct();
      CText name;

      for (int i = 1; i <= nrPar; i++)
      {
         name.Format("p%d", i);
         s->AddToStructure(name.Str(), a[i]);
         name.Format("p%derr", i);
         s->AddToStructure(name.Str(), (float)sqrt(covar[i][i]) * NL);
      }
      par->retVar[2].MakeAndSetFloat(chisq / (ndata - nrPar));
      par->nrRetVar = 2;
   }
   else // Separate float variables returned
   {
      for (int i = 1; i <= nrPar; i++)
      {
         par->retVar[i].MakeAndSetFloat(a[i]);
      }
      for (int i = 1; i <= nrPar; i++)
      {
         par->retVar[nrPar + i].MakeAndSetFloat(sqrt(covar[i][i]) * NL);
      }
      par->retVar[nrPar * 2 + 1].MakeAndSetFloat(chisq / (ndata - nrPar));

      par->nrRetVar = nrPar * 2 + 1;
   }
   
// Free memory ********************************/
ex:
   FreeVectorNR(a,1L,nrPar); 
   FreeIVectorNR(ia,1L,nrPar);
   FreeMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   FreeVectorNR(x,1L,ndata);
   FreeVectorNR(y,1L,ndata);
   FreeVectorNR(sig,1L,ndata);
   delete nonLinearfunction;

   return(OK);
}

// A general nonlinear fit using double precision. User must supply Prospa procedure which returns function and derivatives

short NonLinearFitD(DLLParameters* par, char* parameters)
{
   long* ia;
   double* x, * y, * sig, * a;
   double** covar, ** alpha;
   double chisq, lamda, chisqold;
   long ma, i, ndata, nit, nrPar;
   double min_chisq = 1;
   long max_it = 100;
   double noiseLevel = 0;
   short nrArgs;
   double NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varNoise;
   Variable varGuess;
   float* noiseVector;
   CText report;
   CText returnMode = "float";
   short err = OK;
   CText fitMethod = "gj";

   nonLinearfunction = new CText;

   type = GENERAL_FIT;

   // Get info from user ******************************/
   if ((nrArgs = ArgScan(par->itfc, parameters, 4, "x, y, function, method, initialGuess, [[noise], [report], [returnMode], [iterations]]", "eeeeeeeee", "vvttvvtlt", &varX, &varY, nonLinearfunction, &fitMethod , &varGuess, &varNoise, &report, &max_it, &returnMode)) < 0)
      return(nrArgs);

   callingInterface = par->itfc;

   // Check for input errors *************************************************
   if (varX.GetType() != MATRIX2D || varX.GetType() != MATRIX2D)
   {
      ErrorMessage("x & y variable should be row vector");
      delete nonLinearfunction;
      return(ERR);
   }

   if (varX.GetDimY() != 1 || varY.GetDimY() != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      delete nonLinearfunction;
      return(ERR);
   }

   if (varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y variables should have same number of points");
      delete nonLinearfunction;
      return(ERR);
   }

   if (varGuess.GetType() != MATRIX2D || varGuess.GetDimY() != 1)
   {
      ErrorMessage("guess variable should be row vector");
      delete nonLinearfunction;
      return(ERR);
   }

   // Evaluate noise
   if (nrArgs >= 5)
   {
      if (varNoise.GetType() == FLOAT32)
      {
         noiseLevel = varNoise.GetReal();

         // Check for errors *************************/   
         if (noiseLevel < 0)
         {
            ErrorMessage("noise level must be positive");
            delete nonLinearfunction;
            return(ERR);
         }
      }
      else if (varNoise.GetType() == MATRIX2D)
      {
         if (varNoise.GetDimY() == 1 && varNoise.GetDimX() == varX.GetDimX())
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
   if (ndata == 0)
   {
      ErrorMessage("Zero data width");
      delete nonLinearfunction;
      return(ERR);
   }

   nrPar = varGuess.GetDimX();
   if (nrPar == 0)
   {
      ErrorMessage("Zero parameters");
      delete nonLinearfunction;
      return(ERR);
   }

   // Allocate memory ***************************/
   a = MakeDVectorNR(1L, nrPar);
   ia = MakeIVectorNR(1L, nrPar);
   x = MakeDVectorNR(1L, ndata);
   y = MakeDVectorNR(1L, ndata);
   sig = MakeDVectorNR(1L, ndata);

   // Copy data to x,y arrays ******************************************
   for (i = 1; i <= ndata; i++)
   {
      x[i] = (double)VarRealMatrix(&varX)[0][i - 1];
      y[i] = (double)VarRealMatrix(&varY)[0][i - 1];
   }

   // Initialize some data values ****************/
   ma = nrPar;
   for (i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeDMatrix2DNR(1L, nrPar, 1L, nrPar);
   alpha = MakeDMatrix2DNR(1L, nrPar, 1L, nrPar);

   // Estimate initial parameter values ***********/
   for (int i = 0; i < nrPar; i++)
      a[i + 1] = varGuess.GetMatrix2D()[0][i];

   // Set uncertainties to noise level ********/
   if (varNoise.GetType() == FLOAT32)
   {
      if (noiseLevel == 0)
      {
         calcNoise = 1;
         noiseLevel = 1;
      }
      for (i = 1; i <= ndata; i++)
         sig[i] = noiseLevel;
   }
   else
   {
      for (i = 1; i <= ndata; i++)
         sig[i] = noiseVector[i - 1];
   }

   // Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if (nlfit_dbl(x, y, sig, ndata, a, ia, ma, covar, alpha, &chisq, function_dbl, &lamda, fitMethod) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
      print("Iteration %d\n", nit);

   } while (lamda > 1.0e-12 && nit <= max_it);
   lamda = 0.0;
   print("Final Iteration %d\n", nit);
   if (nlfit_dbl(x, y, sig, ndata, a, ia, ma, covar, alpha, &chisq, function_dbl, &lamda, fitMethod) == ERR)
   {
      err = ERR;
      goto ex;
   }

   // Print out results of data ***************/
   if (calcNoise)
      NL = sqrt(chisq / (ndata - nrPar));
   else
      NL = 1;

   if (report == "yes")
   {
      if (calcNoise)
         TextMessage("\n\n     noise ............. %2.3f", NL);
      else
         TextMessage("\n");
      TextMessage("\n     Iterations ......... %ld", nit);
      for (int i = 1; i <= nrPar; i++)
         TextMessage("\n     parameter %d ............... %2.3f +- %2.3f", i, a[i], sqrt(covar[i][i]) * NL);

      TextMessage("\n     Chi-squared .... %2.2f\n", chisq / (ndata - nrPar));
   }

   // Return variables

   if (returnMode == "struct0") // Zero based named variables p0, p0Err etc in a structure
   {
      par->retVar[1].MakeStruct();
      Variable* s = par->retVar[1].GetStruct();
      CText name;

      for (int i = 0; i < nrPar; i++)
      {
         name.Format("p%d", i);
         s->AddToStructure(name.Str(), (float)a[i + 1]);
         name.Format("p%derr", i);
         s->AddToStructure(name.Str(), (float)sqrt(covar[i + 1][i + 1]) * NL);
      }
      par->retVar[2].MakeAndSetFloat(chisq / (ndata - nrPar));
      par->nrRetVar = 2;
   }
   else if (returnMode == "struct1") // One based named variables p1, p1Err etc in a structure
   {
      par->retVar[1].MakeStruct();
      Variable* s = par->retVar[1].GetStruct();
      CText name;

      for (int i = 1; i <= nrPar; i++)
      {
         name.Format("p%d", i);
         s->AddToStructure(name.Str(), (float)a[i]);
         name.Format("p%derr", i);
         s->AddToStructure(name.Str(), (float)sqrt(covar[i][i]) * NL);
      }
      par->retVar[2].MakeAndSetFloat(chisq / (ndata - nrPar));
      par->nrRetVar = 2;
   }
   else // Separate float variables returned
   {
      for (int i = 1; i <= nrPar; i++)
      {
         par->retVar[i].MakeAndSetFloat(a[i]);
      }
      for (int i = 1; i <= nrPar; i++)
      {
         par->retVar[nrPar + i].MakeAndSetFloat((float)sqrt(covar[i][i]) * NL);
      }
      par->retVar[nrPar * 2 + 1].MakeAndSetFloat((float)chisq / (ndata - nrPar));

      par->nrRetVar = nrPar * 2 + 1;
   }

   // Free memory ********************************/
ex:
   FreeDVectorNR(a, 1L, nrPar);
   FreeIVectorNR(ia, 1L, nrPar);
   FreeDMatrix2DNR(covar, 1L, nrPar, 1L, nrPar);
   FreeDMatrix2DNR(alpha, 1L, nrPar, 1L, nrPar);
   FreeDVectorNR(x, 1L, ndata);
   FreeDVectorNR(y, 1L, ndata);
   FreeDVectorNR(sig, 1L, ndata);
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
   CText fitMethod = "GJ";
         
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
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda,fitMethod) == ERR)
      {
         err = ERR;
         goto ex;   
      }   
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda, fitMethod) == ERR)
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
  // TextMessage("T2Fit\n");

// Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
     // TextMessage("--- Iteration %d ---\n", nit);
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
 //  TextMessage("--- Final Iteration ---\n");

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


short T2FitD(DLLParameters* par, char* parameters)
{
   long* ia;
   double* x, * y, * sig, * a;
   double** covar, ** alpha;
   double chisq, lamda, chisqold;
   long ma, i, ndata, nit, nrPar;
   float min_chisq = 1;
   long max_it = 100;
   float noiseLevel = 0;
   short nrArgs;
   float NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varNoise;
   float* noiseVector;
   char report[50];
   short err = OK;
   float e0Guess, t2Guess;

   cnt = 0;

   type = T2_FIT;

   // Get info from user ******************************/
   if ((nrArgs = ArgScan(par->itfc, parameters, 2, "x, y, [[noise], [report], [iterations], [e0Guess, t2Guess]", "eeeeeee", "vvvslff", &varX, &varY, &varNoise, report, &max_it, &e0Guess, &t2Guess)) < 0)
      return(nrArgs);

   // Check for input errors *************************************************
   if (varX.GetType() != MATRIX2D || varX.GetType() != MATRIX2D)
   {
      ErrorMessage("x & y variable should be row vector");
      return(ERR);
   }

   if (varX.GetDimY() != 1 || varY.GetDimY() != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }

   if (varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y variables should have same number of points");
      return(ERR);
   }

   if (nrArgs > 2)
   {
      if (varNoise.GetType() == FLOAT32)
      {
         noiseLevel = varNoise.GetReal();

         // Check for errors *************************/   
         if (noiseLevel < 0)
         {
            ErrorMessage("noise level must be positive");
            return(ERR);
         }
      }
      else if (varNoise.GetType() == MATRIX2D)
      {
         if (varNoise.GetDimY() == 1 && varNoise.GetDimX() == varX.GetDimX())
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

   if (ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }

   nrPar = 2;

   // Allocate memory ***************************/
   a = MakeDVectorNR(1L, nrPar);
   ia = MakeIVectorNR(1L, nrPar);
   x = MakeDVectorNR(1L, ndata);
   y = MakeDVectorNR(1L, ndata);
   sig = MakeDVectorNR(1L, ndata);

   // Copy data to x,y arrays ******************************************
   float maxY = -1e39;
   for (i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i - 1];
      y[i] = VarRealMatrix(&varY)[0][i - 1];
      if (y[i] > maxY) maxY = y[i];
   }

   // Initialize some data values ****************/
   ma = nrPar;
   for (i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeDMatrix2DNR(1L, nrPar, 1L, nrPar);
   alpha = MakeDMatrix2DNR(1L, nrPar, 1L, nrPar);

   // Estimate initial parameter values ***********/
   if (nrArgs == 7)
   {
      a[1] = e0Guess;
      a[2] = 1.0 / t2Guess;
   }
   else
   {
      a[1] = y[1];
      if (a[1] <= 0)
      {
         a[1] = maxY;
         a[2] = 1.0 / x[ndata];
      }
      else
      {
         for (i = 1; i <= ndata; i++)
         {
            if (y[i] < a[1] * 0.3679)
            {
               if (x[i] == 0)
               {
                  ErrorMessage("invalid x axis");
                  err = ERR;
                  goto ex;
               }
               a[2] = 1.0 / x[i];
               break;
            }
         }
         if (i > ndata)
            a[2] = 1 / x[ndata];
      }
   }

   // Set uncertainties to noise level ********/
   if (varNoise.GetType() == FLOAT32)
   {
      if (noiseLevel == 0)
      {
         calcNoise = 1;
         noiseLevel = 1;
      }
      for (i = 1; i <= ndata; i++)
         sig[i] = noiseLevel;
   }
   else
   {
      for (i = 1; i <= ndata; i++)
         sig[i] = noiseVector[i - 1];
   }
 //  TextMessage("T2DFit\n");

   // Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added

   do
   {
      chisqold = chisq;
    //  TextMessage("--- Iteration %d ---\n", nit);
      if (nlfit_dbl(x, y, sig, ndata, a, ia, ma, covar, alpha, &chisq, function_dbl, &lamda) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;

   } while (lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
  // TextMessage("--- Final Iteration ---\n");
   if (nlfit_dbl(x, y, sig, ndata, a, ia, ma, covar, alpha, &chisq, function_dbl, &lamda) == ERR)
   {
      err = ERR;
      goto ex;
   }

   // Print out results of data ***************/
   if (calcNoise)
      NL = sqrt(chisq / (ndata - nrPar));
   else
      NL = 1;

   if (!strcmp(report, "yes"))
   {
      if (calcNoise)
         TextMessage("\n\n     noise ............. %2.3f", NL);
      else
         TextMessage("\n");
      TextMessage("\n     Iterations ......... %ld", nit);
      TextMessage("\n     E(0) ............... %2.3f +- %2.3f", a[1], sqrt(covar[1][1]) * NL);
      TextMessage("\n     T2 ................. %2.3f +- %2.3f", 1.0 / a[2], sqrt(covar[2][2]) / sqr(a[2]) * NL);

      // Print out statistics ***********************/

      TextMessage("\n     Normalised chi-squared .... %2.2f\n", chisq / (ndata - nrPar) / sqr(NL));
   }

   // Make best fit vector (ans1 & ans)
   float** bestFit = MakeMatrix2D(ndata, 1);
   double val;
   double* dyda;

   dyda = MakeDVectorNR(1L, ma);

   for (i = 1; i <= ndata; i++)
   {
      function_dbl(x[i], a, &val, dyda, nrPar);
      bestFit[0][i - 1] = val;
   }
   FreeDVectorNR(dyda, 1L, ma);

   // Return to user bestFit in ans and ans1 ****
   par->retVar[1].MakeAndLoadMatrix2D(bestFit, ndata, 1);
   FreeMatrix2D(bestFit);

   // Return other parameters in ans2 ... ans5
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(1.0 / a[2]);
   par->retVar[4].MakeAndSetFloat(sqrt(covar[1][1]) * NL);
   par->retVar[5].MakeAndSetFloat(sqrt(covar[2][2]) / sqr(a[2]) * NL);
   par->nrRetVar = 5;

   // Free memory ********************************/
ex:
   FreeDVectorNR(a, 1L, nrPar);
   FreeIVectorNR(ia, 1L, nrPar);
   FreeDMatrix2DNR(covar, 1L, nrPar, 1L, nrPar);
   FreeDMatrix2DNR(alpha, 1L, nrPar, 1L, nrPar);
   FreeDVectorNR(x, 1L, ndata);
   FreeDVectorNR(y, 1L, ndata);
   FreeDVectorNR(sig, 1L, ndata);

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
   double *x,*y,*sig,*a;
   double**covar,**alpha;
   double chisq,lamda,chisqold;
   long ma,i,j,ndata,nit,nrPar;
   double min_chisq = 1;
   long max_it = 5;
   double noiseLevel = 0;
   short nrArgs;
   double NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   Variable varPeakList;
   Variable fixedParVar;
   short err = OK;
   CText fitMethod = "svd";

   type = PEAKFIT;

   // Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc, parameters,3,"x, y, peakList, whichPar, iterations, fitMethod","eeeeee","vvvvlt",&varX,&varY,&varPeakList,&fixedParVar,&max_it,&fitMethod)) < 0)
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
   a    = MakeDVectorNR(1L,nrPar);
   ia   = MakeIVectorNR(1L,nrPar);
   x    = MakeDVectorNR(1L,ndata);
   y    = MakeDVectorNR(1L,ndata);
   sig  = MakeDVectorNR(1L,ndata);

   // Copy data to x,y arrays ******************************************
   for(i = 1; i <= ndata; i++)
   {
      x[i] = (double)VarRealMatrix(&varX)[0][i-1];
      y[i] = (double)VarRealMatrix(&varY)[0][i-1];
      sig[i] = 1.0;
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

   covar = MakeDMatrix2DNR(1L,nrPar,1L,nrPar);
   alpha = MakeDMatrix2DNR(1L,nrPar,1L,nrPar);


  // Put in intial values of position, amplitude and width
   for(i = 1, j = 0; i <= nrPar; i+=3, j++)
   {
      a[i] = (double)peakList[j][0];
      a[i+1] = (double)peakList[j][1];
      a[i+2] = (double)peakList[j][2];
   }

   // Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      //TextMessage("\n--- Iteration: %ld ---\n", nit);
      chisqold = chisq;
      if(nlfit_dbl(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function_dbl,&lamda,fitMethod) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while (lamda > 1.0e-10 && nit <= max_it);
   TextMessage("\n   Number of iterations: %ld\n", nit);
   lamda = 0.0;
   if(nlfit_dbl(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function_dbl,&lamda,fitMethod) == ERR)
   {
      err = ERR;
      goto ex;
   }  

   // Make best fit vector (ans1 & ans)
   float** bestFit = MakeMatrix2D(ndata,1);
   double val;
   double *dyda;

   dyda = MakeDVectorNR(1L,ma);

   for(i = 1; i <= ndata; i++)
   {
      function_dbl(x[i],a,&val,dyda,nrPar);
      bestFit[0][i-1] = (float)val;
   }		
   FreeDVectorNR(dyda,1L,ma);   

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
   FreeDVectorNR(a,1L,nrPar); 
   FreeIVectorNR(ia,1L,nrPar);
   FreeDMatrix2DNR(covar,1L,nrPar,1L,nrPar);
   FreeDMatrix2DNR(alpha,1L,nrPar,1L,nrPar);
   FreeDVectorNR(x,1L,ndata);
   FreeDVectorNR(y,1L,ndata);
   FreeDVectorNR(sig,1L,ndata);

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

extern short svdcmp(float** a, long m, long n, float* w, float** v, long mode);
extern void svbksb(float** u, float w[], float** v, long m, long n, float b[], float x[]);
extern short svdcmp_dbl(double** a, long m, long n, double* w, double** v, long mode);
extern void svbksb_dbl(double** u, double w[], double** v, long m, long n, double b[], double x[]);


void printMatrix(char* title, float** mat, long rows, long cols);
void printVector(char* title, float* vec, long entries);
void printDMatrix(char* title, double** mat, long rows, long cols);
void printDVector(char* title, double* vec, long entries);

/****************************************************************************
*                    Levenberg-Marquardt nonlinear fit                      *
****************************************************************************/

short nlfit(float x[],float y[],float sig[],long ndata,float a[], long ia[], long ma,
            float **covar,float **alpha,float *chisq,
            void (*funcs)(float,float[],float*,float[],long),float *alamda, CText fitMethod)
{
   long j,k,l,m;
   static long mfit;
   static float ochisq,*atry,*beta,*da,**oneda;
   short svdSolver(float** covar, long ma, float* da);

  printVector("x:", x, ndata);
  printVector("y:", y, ndata);

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

      printMatrix("alpha (1):", alpha, ma, ma);
      printVector("beta (1):", beta, ma);
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


   for (j = 1; j <= mfit; j++)
      da[j] = oneda[j][1];

   if (fitMethod == "svd" || fitMethod == "SVD")
   {
      print("\n--------------- SVD solver --------------\n");
      // Print out matrices
      printMatrix("covar (init):", covar, ma, ma);
      printVector("oneda (init):", da, ma);

      if (svdSolver(covar, mfit, da) == -1)
      {
         FreeMatrix2DNR(oneda, 1L, mfit, 1L, 1L);
         FreeVectorNR(da, 1L, ma);
         FreeVectorNR(beta, 1L, ma);
         FreeVectorNR(atry, 1L, ma);
         return(-1);
      }
   } 
   else
   {
   
       print("\n--------------- Gauss-Jordan --------------\n");
       printMatrix("covar (init):", covar, ma, ma);
       printVector("oneda (init):", da, ma);

      // covar -> inverse(covar)
      // oneda -> x Where A.x = oneda
      if (gaussj(covar, mfit, oneda, 1L))
      {
         FreeMatrix2DNR(oneda, 1L, mfit, 1L, 1L);
         FreeVectorNR(da, 1L, ma);
         FreeVectorNR(beta, 1L, ma);
         FreeVectorNR(atry, 1L, ma);
         return(-1);
      }
      for (j = 1; j <= mfit; j++)
         da[j] = oneda[j][1];
   }

   printMatrix("Inv covar:", covar, ma, ma);
   printVector("da :", da, ma);

   if(*alamda == 0.0)
   {
      covsrt(covar,ma,ia,mfit);
      //TextMessage("\nFinal chisq = %g\n", *chisq);

      FreeMatrix2DNR(oneda, 1L, mfit, 1L, 1L);
      FreeVectorNR(da, 1L, ma);
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

   print("\nchisq = %g\n", *chisq);
   print("\nlamda = %g\n", *alamda);

   return(0);
}


/****************************************************************************
*                    Levenberg-Marquardt nonlinear fit                      *
****************************************************************************/

short nlfit_dbl(double x[], double y[], double sig[], long ndata, double a[], long ia[], long ma,
   double** covar, double** alpha, double* chisq,
   short (*funcs_dbl)(double, double[], double*, double[], long), double* alamda, CText fitMethod)
{
   long j, k, l, m;
   static long mfit;
   static double ochisq, * atry, * beta, * da, ** oneda;
   short svdSolver_dbl(double** covar, long ma, double* da);

   printDVector("x:", x, ndata);
   printDVector("y:", y, ndata);

     // Check for invalid values
   for (j = 1; j <= ndata; j++)
   {
      if (isnan(x[j]) || !_finite(x[j]))
      {
         ErrorMessage("invalid value in x matrix");
         return(-1);
      }
      if (isnan(y[j]) || !_finite(y[j]))
      {
         ErrorMessage("invalid value in y matrix");
         return(-1);
      }
   }

   // Intialisation condition
   if (*alamda < 0.0)
   {
      atry = MakeDVectorNR(1L, ma);
      beta = MakeDVectorNR(1L, ma);
      da = MakeDVectorNR(1L, ma);
      for (mfit = 0, j = 1; j <= ma; j++)
         if (ia[j]) mfit++;
      oneda = MakeDMatrix2DNR(1L, mfit, 1L, 1L);
      *alamda = 0.001;
      if (mrqcof_dbl(x, y, sig, ndata, a, ia, ma, alpha, beta, chisq, funcs_dbl))
      {
         FreeDMatrix2DNR(oneda, 1L, mfit, 1L, 1L);
         FreeDVectorNR(da, 1L, ma);
         FreeDVectorNR(beta, 1L, ma);
         FreeDVectorNR(atry, 1L, ma);
         return(-1);
      }
      ochisq = (*chisq);
      for (j = 1; j <= ma; j++)
         atry[j] = a[j];

      printDMatrix("alpha (1):", alpha, ma, ma);
      printDVector("beta (1):", beta, ma);
   }

   for (j = 0, l = 1; l <= ma; l++)
   {
      if (ia[l])
      {
         for (j++, k = 0, m = 1; m <= ma; m++)
         {
            if (ia[m])
            {
               k++;
               covar[j][k] = alpha[j][k];
            }
         }
         covar[j][j] = alpha[j][j] * (1.0 + (*alamda));
         oneda[j][1] = beta[j];
      }
   }

   for (j = 1; j <= mfit; j++)
      da[j] = oneda[j][1];


   if (fitMethod == "svd" || fitMethod == "SVD")
   {
      print("\n--------------- SVD solver --------------\n");

      // Print out matrices
      printDMatrix("covar (init):", covar, ma, ma);
      printDVector("oneda (init):", da, ma);

      // covar -> inverse(covar)
      // da -> x Where A.x = oneda
      if (svdSolver_dbl(covar, mfit, da) == -1)
      {
         FreeDMatrix2DNR(oneda, 1L, mfit, 1L, 1L);
         FreeDVectorNR(da, 1L, ma);
         FreeDVectorNR(beta, 1L, ma);
         FreeDVectorNR(atry, 1L, ma);
         return(-1);
      }
   }
   else
   {
      print("\n--------------- Gauss-Jordan --------------\n");
      printDMatrix("covar (init):", covar, ma, ma);
      printDVector("oneda (init):", da, ma);

      // covar -> inverse(covar)
   // oneda -> x Where A.x = oneda
      if (gaussj_dbl(covar, mfit, oneda, 1L))
      {
         FreeDMatrix2DNR(oneda, 1L, mfit, 1L, 1L);
         FreeDVectorNR(da, 1L, ma);
         FreeDVectorNR(beta, 1L, ma);
         FreeDVectorNR(atry, 1L, ma);
         return(-1);
      }

      for (j = 1; j <= mfit; j++)
         da[j] = oneda[j][1];

      printDMatrix("Inv covar (G-J):", covar, ma, ma);
      printDVector("da (G-J):", da, ma);
   }
   
   if (*alamda == 0.0)
   {
      covsrt_dbl(covar, ma, ia, mfit);
      //TextMessage("\nFinal chisq = %g\n", *chisq);

      FreeDMatrix2DNR(oneda, 1L, mfit, 1L, 1L);
      FreeDVectorNR(da, 1L, ma);
      FreeDVectorNR(beta, 1L, ma);
      FreeDVectorNR(atry, 1L, ma);
      return(0);
   }

   for (j = 0, l = 1; l <= ma; l++)
   {
      if (ia[l])
         atry[l] = a[l] + da[++j];
   }

   if(mrqcof_dbl(x, y, sig, ndata, atry, ia, ma, covar, da, chisq, funcs_dbl))
   {
      FreeDMatrix2DNR(oneda, 1L, mfit, 1L, 1L);
      FreeDVectorNR(da, 1L, ma);
      FreeDVectorNR(beta, 1L, ma);
      FreeDVectorNR(atry, 1L, ma);
      return(-1);
   }

   if (*chisq < ochisq)
   {
      *alamda *= 0.1;
      ochisq = (*chisq);
      for (j = 0, l = 1; l <= ma; l++)
      {
         if (ia[l])
         {
            for (j++, k = 0, m = 1; m <= ma; m++)
            {
               if (ia[m])
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

   print("\nchisq = %g\n", *chisq);
   print("\nlamda = %g\n", *alamda);

   return(0);
}

// Allows for format specifiers in the print statement
char* vssprintf(const char* format, va_list argptr)
{
   int n = _vscprintf(format, argptr) + 1;
   char* buffer = new char[n];
   _vsnprintf(buffer, n, format, argptr);
   return (buffer);
}

// A general print statement for use when debugging
void print(const char* const text, ...)
{
   if (!debug) return;

   va_list ap;

   va_start(ap, text);

   char* output = vssprintf(text, ap);

   TextMessage("%s", output);

   va_end(ap);

   delete[] output;
}

void printMatrix(char* title, float** mat, long rows, long cols)
{
   if (!debug) return;
   TextMessage("%s\n", title);
   for (int i = 1; i <= rows; i++)
   {
      for (int j = 1; j <= cols; j++)
         TextMessage("%g ", mat[i][j]);
      TextMessage("\n");
   }
   TextMessage("\n");

}

void printVector(char *title, float* vec, long entries)
{
   if (!debug) return;
   TextMessage("%s\n", title);
   for (int i = 1; i <= entries; i++)
   {
      TextMessage("%g ", vec[i]);
   }
   TextMessage("\n\n");
}

short svdSolver(float** covar, long ma, float* da)
{
   long i, j;
   void findInverse(float** u, float w[], float** v, long n, float** inv);

    float* w = MakeVectorNR(1L, ma);
    float** v = MakeMatrix2DNR(1L, ma, 1L, ma);
    float** u = MakeMatrix2DNR(1L, ma, 1L, ma);
    float* xA = MakeVectorNR(1L, ma);

    for (int i = 1; i <= ma; i++)
       for (int j = 1; j <= ma; j++)
          u[i][j] = covar[i][j];

    printMatrix("U:", u, ma, ma);

    if (svdcmp(u, ma, ma, w, v, 0) == -1)
    {
       FreeMatrix2DNR(u, 1L, ma, 1L, ma);
       FreeMatrix2DNR(v, 1L, ma, 1L, ma);
       FreeVectorNR(w, 1L, ma);
       FreeVectorNR(xA, 1L, ma);
       return(-1);
    }
 
    // Print out matrices
    printMatrix("U:", u, ma, ma);
    printMatrix("V:", u, ma, ma);
    printVector("W:", w, ma);
 
     // Calculate 1/W
    float wmax = 0.0;
    for (j = 1; j <= ma; j++)
       if (w[j] > wmax) wmax = w[j];
    float thresh = 1e-16 * wmax;
    for (j = 1; j <= ma; j++)
    {
       if (w[j] < thresh)
          w[j] = 0.0;
    }

    printVector("1/W:", w, ma);

    // Calculate the inverse of covar
    findInverse(u, w, v, ma, covar);
 
    printMatrix("covar inverse (SVD):", covar, ma, ma);

  // Solves covar.xA = da
    svbksb(u, w, v, ma, ma, da, xA);
 
    printVector("xA (SVD):", xA, ma);

   // Return xA in da
    for (j = 1; j <= ma; j++)
       da[j] = xA[j];

    FreeMatrix2DNR(u, 1L, ma, 1L, ma);
    FreeMatrix2DNR(v, 1L, ma, 1L, ma);
    FreeVectorNR(w, 1L, ma);
    FreeVectorNR(xA, 1L, ma);

    return(0);
}

void printDMatrix(char* title, double** mat, long rows, long cols)
{
   if (!debug) return;
   TextMessage("%s\n", title);
   for (int i = 1; i <= rows; i++)
   {
      for (int j = 1; j <= cols; j++)
         TextMessage("%g ", mat[i][j]);
      TextMessage("\n");
   }
   TextMessage("\n");

}

void printDVector(char* title, double* vec, long entries)
{
   if (!debug) return;
   TextMessage("%s\n", title);
   for (int i = 1; i <= entries; i++)
   {
      TextMessage("%g ", vec[i]);
   }
   TextMessage("\n\n");
}

short svdSolver_dbl(double** covar, long ma, double* da)
{
   long i, j;
   void findInverse_dbl(double** u, double w[], double** v, long n, double** inv);

   double* w = MakeDVectorNR(1L, ma);
   double** v = MakeDMatrix2DNR(1L, ma, 1L, ma);
   double** u = MakeDMatrix2DNR(1L, ma, 1L, ma);
   double* xA = MakeDVectorNR(1L, ma);

   for (int i = 1; i <= ma; i++)
      for (int j = 1; j <= ma; j++)
         u[i][j] = covar[i][j];

   printDMatrix("U:", u, ma, ma);

   if (svdcmp_dbl(u, ma, ma, w, v, 0) == -1)
   {
      FreeDMatrix2DNR(u, 1L, ma, 1L, ma);
      FreeDMatrix2DNR(v, 1L, ma, 1L, ma);
      FreeDVectorNR(w, 1L, ma);
      FreeDVectorNR(xA, 1L, ma);
      return(-1);
   }

   // Print out matrices
   printDMatrix("U:", u, ma, ma);
   printDMatrix("V:", v, ma, ma);
   printDVector("W:", w, ma);

   // Threshold w[j]
   float wmax = 0.0;
   for (j = 1; j <= ma; j++)
      if (w[j] > wmax) wmax = w[j];
   float thresh = 1e-16 * wmax;
   for (j = 1; j <= ma; j++)
   {
      if (w[j] < thresh)
         w[j] = 0.0;
   }

   // Calculate the inverse of covar
   findInverse_dbl(u, w, v, ma, covar);

   printDVector("1/W:", w, ma);

   printDMatrix("covar inverse (SVD):", covar, ma, ma);

   // Solves covar.xA = da
   svbksb_dbl(u, w, v, ma, ma, da, xA);

   printDVector("xA (SVD):", xA, ma);

   // Return xA in da
   for (j = 1; j <= ma; j++)
      da[j] = (float)xA[j];

   FreeDMatrix2DNR(u, 1L, ma, 1L, ma);
   FreeDMatrix2DNR(v, 1L, ma, 1L, ma);
   FreeDVectorNR(w, 1L, ma);
   FreeDVectorNR(xA, 1L, ma);

   return(0);
}


// Use the SVD matricies u,w,v to find the inverse inv = V*1/W*UT
void findInverse(float** u, float w[], float** v, long n, float **inv)
{
   long j, i;
   float s, **tmp;

   if (n == 0) return;
   tmp = MakeMatrix2DNR(1L, n, 1L, n);
   // V*1/W
   for (j = 1; j <= n; j++) // Row
   {
      for (i = 1; i <= n; i++) // Col
      {
         s = 0.0;
         for (long k = 1; k <= n; k++)
         {
            if(k == i)
               s += v[j][k]/w[k];
         }
         tmp[j][i] = s;
      }
   }
   // V*1/W*UT
   for (j = 1; j <= n; j++)
   {
      for (i = 1; i <= n; i++)
      {
         s = 0.0;
         for (long k = 1; k <= n; k++)
         {
            s += tmp[j][k] * u[i][k];
         }
         inv[j][i] = s;
      }
   }

   FreeMatrix2DNR(tmp, 1L, n, 1L, n);
}


// Use the SVD matricies u,w,v to find the inverse inv = V*1/W*UT
void findInverse_dbl(double** u, double w[], double** v, long n, double** inv)
{
   long j, i;
   double s, ** tmp;

   if (n == 0) return;
   tmp = MakeDMatrix2DNR(1L, n, 1L, n);
   // V*1/W
   for (j = 1; j <= n; j++) // Row
   {
      for (i = 1; i <= n; i++) // Col
      {
         s = 0.0;
         for (long k = 1; k <= n; k++)
         {
            if (k == i)
               s += v[j][k] / w[k];
         }
         tmp[j][i] = s;
      }
   }
   // V*1/W*UT
   for (j = 1; j <= n; j++)
   {
      for (i = 1; i <= n; i++)
      {
         s = 0.0;
         for (long k = 1; k <= n; k++)
         {
            s += tmp[j][k] * u[i][k];
         }
         inv[j][i] = s;
      }
   }

   FreeDMatrix2DNR(tmp, 1L, n, 1L, n);
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
  // TextMessage("a1: %f: a2 %f a3: %f a4: %f\n", a[1], a[2], a[3], a[4]);

   *chisq = 0.0;
   for(i = 1; i <= ndata; i++)
   {

      (*funcs)(x[i],a,&ymod,dyda,ma);
     // TextMessage("i: %ld ymod: %f dyda1: %f dyda2: %f dyda3: %f dyda4: %f sigi: %f\n", i, ymod, dyda[1], dyda[2], dyda[3], dyda[4], sig[i]);

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
*           Calculate matricies alpha and beta and chisq number             *
****************************************************************************/

short mrqcof_dbl(double x[], double y[], double sig[], long ndata, double a[],
   long ia[], long ma, double** alpha, double beta[], double* chisq,
   short (*funcs)(double, double[], double*, double[], long))
{
   long i, j, k, l, m, mfit = 0;
   double ymod, wt, sig2i, dy, *dyda;

   dyda = MakeDVectorNR(1L, ma);

   for (j = 1; j <= ma; j++)
   {
      if (ia[j]) mfit++;
   }
   for (j = 1; j <= mfit; j++)
   {
      for (k = 1; k <= j; k++)
         alpha[j][k] = 0.0;
      beta[j] = 0.0;
   }

  // TextMessage("a1: %lf: a2 %lf a3: %lf a4: %lf\n", a[1], a[2], a[3], a[4]);

   *chisq = 0.0;
   for (i = 1; i <= ndata; i++)
   {

      if ((*funcs)(x[i], a, &ymod, dyda, ma))
      {
         ErrorMessage("Error calling fitting function - aborting");
         FreeDVectorNR(dyda, 1L, ma);
         return(1);
      }
     // TextMessage("i: %ld ymod: %lf dyda1: %lf dyda2: %lf dyda3: %lf dyda4: %lf sigi %lf\n", i, ymod, dyda[1], dyda[2], dyda[3], dyda[4], sig[i]);

      sig2i = 1.0 / (sig[i] * sig[i]);
      dy = y[i] - ymod;
      for (j = 0, l = 1; l <= ma; l++)
      {
         if (ia[l])
         {
            wt = dyda[l] * sig2i;
            for (j++, k = 0, m = 1; m <= l; m++)
            {
               if (ia[m])
                  alpha[j][++k] += wt * dyda[m];
            }
            beta[j] += dy * wt;
         }
      }
      (*chisq) += dy * dy * sig2i;
   }
   for (j = 2; j <= mfit; j++)
      for (k = 1; k < j; k++)
         alpha[k][j] = alpha[j][k];
   FreeDVectorNR(dyda, 1L, ma);
   return(0);
}
    
/****************************************************************************
*             Reorder covariance matrix so we can extract errors            *  
****************************************************************************/

    
void covsrt_dbl(double **covar,long ma,long ia[], long mfit)
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
*             Reorder covariance matrix so we can extract errors            *
****************************************************************************/

void covsrt(float** covar, long ma, long ia[], long mfit)
{
   long i, j, k;

   for (i = mfit + 1; i <= ma; i++)
      for (j = 1; j <= i; j++)
         covar[i][j] = covar[j][i] = 0.0;

   k = mfit;
   for (j = ma; j >= 1; j--)
   {
      if (ia[j])
      {
         for (i = 1; i <= ma; i++) Swap(covar[i][k], covar[i][j]);
         for (i = 1; i <= ma; i++) Swap(covar[k][i], covar[j][i]);
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
         if (v[1].GetType() == FLOAT32)
            *y = v[1].GetReal();
         for (int i = 1; i < nrRetArgs; i++)
         {
            if (v[i + 1].GetType() == FLOAT32)
               dydx[i] = v[i + 1].GetReal();
         }
         break;
      }
   }
}


/****************************************************************************
*   Routine to evaluate a function and its derivatives   (double version)   *
****************************************************************************/

short function_dbl(double x, double a[], double* y, double dydx[], long na)
{
   long i;
   double fac, ex, arg;

   *y = 0.0;

   switch (type)
   {
      case(LORENTZIAN):   // Lorentzian fit y = a/(1+c*(x-b)^2)
      {
         for (i = 1; i <= na - 1; i += 3)
         {
            fac = (x - a[i + 1]);
            arg = (1 + a[i + 2] * fac * fac);
            *y += a[i] / arg;
            dydx[i] = 1 / arg;
            dydx[i + 1] = a[i] / (arg * arg) * 2 * a[i + 2] * fac;
            dydx[i + 2] = -a[i] / (arg * arg) * fac * fac;
         }
         break;
      }
      case(PEAKFIT):   // Lorentzian fit y = a/(1+c^2*(x-b)^2)
      {
         for (i = 1; i <= na - 1; i += 3)
         {
            fac = (x - a[i + 1]);
            arg = (1 + a[i + 2] * a[i + 2] * fac * fac);
            *y += a[i] / arg;
            dydx[i] = 1 / arg;
            dydx[i + 1] = a[i] / (arg * arg) * 2 * a[i + 2] * a[i + 2] * fac;
            dydx[i + 2] = -a[i] / (arg * arg) * fac * fac * 2 * a[i + 2];
         }
         break;
      }
      case(GAUSSIAN):   // Gaussian fit  y = a*exp(-(x-b)^2/(2*c^2))
      {
         for (i = 1; i <= na - 1; i += 3)
         {
            arg = (x - a[i + 1]) / a[i + 2];
            ex = exp(-arg * arg / 2);
            fac = a[i] * ex * arg;
            *y += a[i] * ex;
            dydx[i] = ex;
            dydx[i + 1] = fac / a[i + 2];
            dydx[i + 2] = fac * arg / a[i + 2];
         }
         break;
      }
      case(T1_VD_FIT):   // T1 variable delay fit y = a[1]*(1.0 - exp(-x*a[2]))
      {
         ex = exp(-x * a[2]);
         fac = (1.0 - ex);
         *y = a[1] * fac;
         dydx[1] = fac;
         dydx[2] = a[1] * x * ex;
         break;
      }
      case(T1_IR_FIT): // T1 inversion recovery fit y = a[1]*(1.0 - 2.0*exp(-x*a[2]))
      {
         ex = exp(-x * a[2]);
         fac = (1.0 - 2.0 * ex);
         *y = a[1] * fac;
         dydx[1] = fac;
         dydx[2] = a[1] * x * 2.0 * ex;
         break;
      }
      case(T1_IR_BIEXP_FIT): // Bi exponential T1 inversion recovery fit y = a[1]*(1.0 - 2.0*exp(-x*a[2]) + a[3]*(1.0 - 2.0*exp(-x*a[4]))
      {
         *y = a[1] * (1.0 - 2.0 * exp(-x * a[2])) + a[3] * (1.0 - 2.0 * exp(-x * a[4]));
         dydx[1] = (1.0 - 2.0 * exp(-x * a[2]));
         dydx[2] = a[1] * x * 2.0 * exp(-x * a[2]);
         dydx[3] = (1.0 - 2.0 * exp(-x * a[4]));
         dydx[4] = a[3] * x * 2.0 * exp(-x * a[4]);
         break;
      }
      case(T1_IR_FIT_ABS): // T1 inversion recovery fit y = |a[1]*(1.0 - 2.0*exp(-x*a[2]))|
      {
         ex = exp(-x * a[2]);
         fac = (1.0 - 2.0 * ex);
         *y = a[1] * fac;
         if (*y >= 0)
         {
            dydx[1] = fac;
            dydx[2] = a[1] * x * 2.0 * ex;
         }
         else
         {
            *y = -*y;
            dydx[1] = -fac;
            dydx[2] = -a[1] * x * 2.0 * ex;
         }
         break;
      }
      case(T2_FIT): // Single exponential fit  y = a[1]*exp(-x*a[2])
      {
         ex = exp(-x * a[2]);
         *y = a[1] * ex;
         dydx[1] = ex;
         dydx[2] = -a[1] * x * ex;
         cnt++;
         break;
      }
      case(EXP_OFF_FIT): // Exp fit with offset y = a[1]*exp(-x*a[2])) + a[3]
      {
         ex = exp(-x * a[2]);
         *y = a[1] * ex + a[3];
         dydx[1] = ex;
         dydx[2] = -a[1] * x * ex;
         dydx[3] = 1;
         break;
      }
      case(DIFF_FIT1): // Vary gradient i.e.  y = a[1]*exp(-gamma^2*x^2*sdel^2*a[2]*(lgdel-smdel/3))
      {
         fac = sqr(smdel * 2.69e8 * x) * (lgdel - smdel / 3.0);
         ex = exp(-fac * a[2]);
         *y = a[1] * ex;
         dydx[1] = ex;
         dydx[2] = -fac * a[1] * ex;
         break;
      }
      case(DIFF_FIT2): // Vary smdel  i.e.  y = a[1]*exp(-gamma^2*grad^2*x^2*a[2]*(lgdel-x/3))
      {
         fac = sqr(x * 2.69e8 * grad) * (lgdel - x / 3.0);
         ex = exp(-fac * a[2]);
         *y = a[1] * ex;
         dydx[1] = ex;
         dydx[2] = -fac * a[1] * ex;
         break;
      }
      case(DIFF_FIT3):  // Vary lgdel  i.e.   y = a[1]*exp(-gamma^2*grad^2*sdel^2*a[2]*(x-smdel/3))
      {
         fac = sqr(smdel * 2.69e8 * grad) * (x - smdel / 3.0);
         ex = exp(-fac * a[2]);
         *y = a[1] * ex;
         dydx[1] = ex;
         dydx[2] = -fac * a[1] * ex;
         break;
      }
      case(VEL_DIFF):  // Vary V and D 
      {
         *y = a[1] * cos(2 * PI * x * a[2] + a[3]) * exp(-x * x * a[4]) + a[5];
         dydx[1] = cos(2 * PI * x * a[2] + a[3]) * exp(-x * x * a[4]);
         dydx[2] = -2 * PI * x * a[1] * sin(2 * PI * x * a[2] + a[3]) * exp(-x * x * a[4]);
         dydx[3] = -a[1] * sin(2 * PI * x * a[2] + a[3]) * exp(-x * x * a[4]);
         dydx[4] = -x * x * a[1] * cos(2 * PI * x * a[2] + a[3]) * exp(-x * x * a[4]);
         dydx[5] = 1;
         break;
      }
      case(BI_EXP_FIT): // Bi-exponential fit 
      {
         *y = a[1] * exp(-x * a[2]) + a[3] * exp(-x * a[4]);
         dydx[1] = exp(-x * a[2]);
         dydx[2] = -x * a[1] * exp(-x * a[2]);
         dydx[3] = exp(-x * a[4]);
         dydx[4] = -x * a[3] * exp(-x * a[4]);
         break;
      }
      case(TRI_EXP_FIT): // Tri-exponential fit 
      {
         *y = a[1] * exp(-x * a[2]) + a[3] * exp(-x * a[4]) + a[5] * exp(-x * a[6]);
         dydx[1] = exp(-x * a[2]);
         dydx[2] = -x * a[1] * exp(-x * a[2]);
         dydx[3] = exp(-x * a[4]);
         dydx[4] = -x * a[3] * exp(-x * a[4]);
         dydx[5] = exp(-x * a[6]);
         dydx[6] = -x * a[5] * exp(-x * a[6]);
         break;
      }
      case(LOG_NORMAL): // Log-normal fit
      {
         fac = exp(-0.5 * sqr(log(x / a[2]) / a[3]));
         *y = a[1] * fac;
         dydx[1] = fac;
         dydx[2] = a[1] * log(x / a[2]) * fac / (a[3] * a[3] * a[2]);
         dydx[3] = a[1] * sqr(log(x / a[2])) * fac / (a[3] * a[3] * a[3]);
         break;
      }
      case(CAP_FIT): // Capillary pressure fit
      {
         float P = a[1];
         float k = a[2];
         float S = a[3];

         *y = exp(log(x) / k - log(P) / k + log(1 - S)) + S;
         dydx[1] = 1 / k * (S - 1) * pow(x, 1 / k) * pow(P, -(k + 1) / k);
         dydx[2] = 1 / k * (S - 1) * pow(P, -1 / k) * pow(x, 1 / k) * (log(P) - log(x));
         dydx[3] = 1 - pow(P, -1 / k) * pow(x, 1 / k);
         break;
      }
      case(GENERAL_FIT): // General fit - call Prospa function
      {
         CText arg, temp;
         arg.Format("%s(%0.16fd,", nonLinearfunction->Str(), (double)x);
         for (int i = 1; i < na; i++)
         {
            temp.Format("%0.16fd,", (double)a[i]);
            arg.Concat(temp.Str());
         }
         temp.Format("%0.16fd)", (double)a[na]);
         arg.Concat(temp.Str());
         short r = ProcessMacroStr(callingInterface, 1, arg.Str());
         if (r != 0)
            return(1);
         int nrRetArgs = *(int*)((char*)callingInterface + 76);     // Interface offset for nrRetValues
         Variable* v = *(Variable**)((char*)callingInterface + 88); // Interface offset for retVar
         *y = (double)(v[1].GetDouble());
         for (int i = 1; i < nrRetArgs; i++)
            dydx[i] = (double)(v[i + 1].GetDouble());
         break;
      }
   }
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

short  gaussj_dbl(double **a, long n, double **b, long m)
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
   CText fitMethod = "gj";
      
   type = BI_EXP_FIT;
   
// Get info from user ******************************/
   if((nrArgs = ArgScan(par->itfc,parameters,2,"x, y, [[noise], [report]","eeeeee","vvfslt",&varX,&varY,&noiseLevel,report,&max_it, &fitMethod)) < 0)
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
      if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda,fitMethod) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   }
   while(lamda > 0.00001 && nit <= max_it);
   lamda = 0.0;
   if(nlfit(x,y,sig,ndata,a,ia,ma,covar,alpha,&chisq,function,&lamda, fitMethod) == ERR)
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
      
      TextMessage("\n     chi-squared .... %2.2f\n",chisq/(ndata-nrPar));
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
   par->retVar[10].MakeAndSetFloat(chisq / (ndata - nrPar));
   par->nrRetVar = 10;
   
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



short BiExpFitD(DLLParameters* par, char* parameters)
{
   long* ia, * fix;
   double* x, * y, * sig, * a, * temp;
   double** covar, ** alpha;
   double chisq, lamda, chisqold;
   long ma, i, ndata, nit, nrPar;
   double min_chisq = 1;
   long max_it = 100;
   double noiseLevel = 0;
   short nrArgs;
   double NL;
   short calcNoise = 0;
   Variable varX;
   Variable varY;
   char report[50];
   short err = OK;
   CText fitMethod = "GJ";

   type = BI_EXP_FIT;

   // Get info from user ******************************/
   if ((nrArgs = ArgScan(par->itfc, parameters, 2, "x, y, [[noise], [report], [fitMethod]", "eeeeee", "vvfslt", &varX, &varY, &noiseLevel, report, &max_it, &fitMethod)) < 0)
      return(nrArgs);

   // Check for errors *************************/   
   if (noiseLevel < 0)
   {
      ErrorMessage("noise level must be positive");
      return(ERR);
   }

   // Check for input errors *************************************************
   if (VarRowSize(&varX) != 1 || VarRowSize(&varY) != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }

   ndata = VarColSize(&varX);

   if (ndata == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }

   if (VarColSize(&varX) != VarColSize(&varY))
   {
      ErrorMessage("x & y vectors do not have same length");
      return(ERR);
   }

   nrPar = 4;

   // Allocate memory ***************************/
   ia = MakeIVectorNR(1L, ndata);
   fix = MakeIVectorNR(1L, ndata);
   temp = MakeDVectorNR(1L, ndata);
   x = MakeDVectorNR(1L, ndata);
   y = MakeDVectorNR(1L, ndata);
   sig = MakeDVectorNR(1L, ndata);
   a = MakeDVectorNR(1L, ndata);

   // Copy data to x,y arrays ******************************************
   for (i = 1; i <= ndata; i++)
   {
      x[i] = VarRealMatrix(&varX)[0][i - 1];
      y[i] = VarRealMatrix(&varY)[0][i - 1];
   }

   // Initialize some data values ****************/
   ma = nrPar;
   for (i = 1; i <= nrPar; i++)
      ia[i] = i;

   covar = MakeDMatrix2DNR(1L, nrPar, 1L, nrPar);
   alpha = MakeDMatrix2DNR(1L, nrPar, 1L, nrPar);

   // Estimate initial parameter values ***********/
   a[1] = y[1] / 2;
   a[3] = y[1] / 2;
   if (a[1] <= 0)
   {
      ErrorMessage("data set unsuited to bi-exp fit");
      err = ERR;
      goto ex;
   }

   for (i = 1; i <= ndata; i++)
   {
      if (y[i] < a[1] * 0.3679)
      {
         if (x[i] == 0)
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
      a[2] = 1 / x[ndata];
      a[4] = 1 / x[ndata];
   }

   // Set uncertainties to noise level ********/
   if (noiseLevel == 0)
   {
      calcNoise = 1;
      noiseLevel = 1;
   }
   for (i = 1; i <= ndata; i++)
      sig[i] = noiseLevel;

   // Fit data ********************************/
   nit = 0;
   lamda = -1.0;
   chisq = 0; // Added
   do
   {
      chisqold = chisq;
      if (nlfit_dbl(x, y, sig, ndata, a, ia, ma, covar, alpha, &chisq, function_dbl, &lamda, fitMethod) == ERR)
      {
         err = ERR;
         goto ex;
      }
      nit++;
   } while (lamda > 1e-10 && nit <= max_it);
   lamda = 0.0;
   if (nlfit_dbl(x, y, sig, ndata, a, ia, ma, covar, alpha, &chisq, function_dbl, &lamda, fitMethod) == ERR)
   {
      err = ERR;
      goto ex;
   }

   // Print out results of data ***************/
   if (calcNoise)
      NL = sqrt(chisq / (ndata - nrPar));
   else
      NL = 1;

   if (!strcmp(report, "yes"))
   {
      if (calcNoise)
         TextMessage("\n\n     noise ............. %2.3f", NL);
      else
         TextMessage("\n");
      TextMessage("\n     Iterations ......... %ld", nit);
      TextMessage("\n     Ea(0) ............... %2.3f +- %2.3f", a[1], sqrt(covar[1][1]) * NL);
      TextMessage("\n     Ta .................. %2.3f +- %2.3f", 1.0 / a[2], sqrt(covar[2][2]) / sqr(a[2]) * NL);
      TextMessage("\n     Eb(0) ............... %2.3f +- %2.3f", a[3], sqrt(covar[3][3]) * NL);
      TextMessage("\n     Tb .................. %2.3f +- %2.3f", 1.0 / a[4], sqrt(covar[4][4]) / sqr(a[4]) * NL);

      // Print out statistics ***********************/

      TextMessage("\n     chi-squared .... %2.2f\n", chisq / (ndata - nrPar));
   }

   // Make best fit vector (ans1 & ans)
   double** bestFit = MakeDMatrix2D(ndata, 1);
   double val;
   double* dyda;

   dyda = MakeDVectorNR(1L, ma);

   for (i = 1; i <= ndata; i++)
   {
      function_dbl(x[i], a, &val, dyda, nrPar);
      bestFit[0][i - 1] = val;
   }
   FreeDVectorNR(dyda, 1L, ma);

   // Return to user bestFit in ans and ans1 ****
   par->retVar[1].MakeAndLoadDMatrix2D(bestFit, ndata, 1);
   FreeDMatrix2D(bestFit);

   // Return other parameters in ans2 ... ans5
   par->retVar[2].MakeAndSetFloat(a[1]);
   par->retVar[3].MakeAndSetFloat(1.0 / a[2]);
   par->retVar[4].MakeAndSetFloat(a[3]);
   par->retVar[5].MakeAndSetFloat(1.0 / a[4]);
   par->retVar[6].MakeAndSetFloat(sqrt(covar[1][1]) * NL);
   par->retVar[7].MakeAndSetFloat(sqrt(covar[2][2]) / sqr(a[2]) * NL);
   par->retVar[8].MakeAndSetFloat(sqrt(covar[3][3]) * NL);
   par->retVar[9].MakeAndSetFloat(sqrt(covar[4][4]) / sqr(a[4]) * NL);
   par->retVar[10].MakeAndSetFloat(chisq / (ndata - nrPar));
   par->nrRetVar = 10;

   // Free memory ********************************/
ex:
   FreeIVectorNR(ia, 1L, ndata);
   FreeIVectorNR(fix, 1L, ndata);
   FreeDVectorNR(temp, 1L, ndata);
   FreeDVectorNR(x, 1L, ndata);
   FreeDVectorNR(y, 1L, ndata);
   FreeDVectorNR(sig, 1L, ndata);
   FreeDVectorNR(a, 1L, ndata);
   FreeDMatrix2DNR(covar, 1L, nrPar, 1L, nrPar);
   FreeDMatrix2DNR(alpha, 1L, nrPar, 1L, nrPar);

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
//
//void test()
//{
//   float pythag(float a, float b);
//   int flag, i, its, j, jj, k, l, nm;
//   float anorm, c, f, g, h, s, scale, x, y, z, * rv1;
//   rv1 = vector(1, n);
//   g = scale = anorm = 0.0;
//   for (i = 1; i <= n; i++)
//   {
//      l = i + 1;
//      rv1[i] = scale * g;
//      g = s = scale = 0.0;
//      if (i <= m)
//      {
//         for (k = i; k <= m; k++)
//            scale += fabs(a[k][i]);
//         if (scale)
//         {
//            for (k = i; k <= m; k++)
//            {
//               a[k][i] /= scale;
//               s += a[k][i] * a[k][i];
//            }
//            f = a[i][i];
//            g = -SIGN(sqrt(s), f);
//            h = f * g - s;
//            a[i][i] = f - g;
//            for (j = l; j <= n; j++)
//            {
//               for (s = 0.0, k = i; k <= m; k++)
//                  s += a[k][i] * a[k][j];
//               f = s / h;
//               for (k = i; k <= m; k++)
//                  a[k][j] += f * a[k][i];
//            }
//            for (k = i; k <= m; k++)
//               a[k][i] *= scale;
//         }
//      }
//      w[i] = scale * g;
//      g = s = scale = 0.0;
//      if (i <= m && i != n)
//      {
//         for (k = l; k <= n; k++)
//            scale += fabs(a[i][k]);
//         if (scale)
//         {
//            for (k = l; k <= n; k++)
//            {
//               a[i][k] /= scale;
//               s += a[i][k] * a[i][k];
//            }
//            f = a[i][l];
//            g = -SIGN(sqrt(s), f);
//            h = f * g - s;
//            a[i][l] = f - g;
//            for (k = l; k <= n; k++)
//               rv1[k] = a[i][k] / h;
//            for (j = l; j <= m; j++)
//            {
//               for (s = 0.0, k = l; k <= n; k++)
//                  s += a[j][k] * a[i][k];
//               for (k = l; k <= n; k++)
//                  a[j][k] += s * rv1[k];
//            }
//            for (k = l; k <= n; k++)
//               a[i][k] *= scale;
//         }
//      }
//      anorm = FMAX(anorm, (fabs(w[i]) + fabs(rv1[i])));
//   }
//   for (i = n; i >= 1; i--)
//   {
//      if (i < n)
//      {
//         if (g)
//         {
//            for (j = l; j <= n; j++)
//               v[j][i] = (a[i][j] / a[i][l]) / g;
//            for (j = l; j <= n; j++)
//            {
//               for (s = 0.0, k = l; k <= n; k++)
//                  s += a[i][k] * v[k][j];
//               for (k = l; k <= n; k++)
//                  v[k][j] += s * v[k][i];
//            }
//         }
//         for (j = l; j <= n; j++)
//            v[i][j] = v[j][i] = 0.0;
//      }
//      v[i][i] = 1.0;
//      g = rv1[i];
//      l = i;
//   }
//   for (i = IMIN(m, n); i >= 1; i--)
//   {
//      l = i + 1;
//      g = w[i];
//      for (j = l; j <= n; j++)
//         a[i][j] = 0.0;
//      if (g)
//      {
//         g = 1.0 / g;
//         for (j = l; j <= n; j++)
//         {
//            for (s = 0.0, k = l; k <= m; k++)
//               s += a[k][i] * a[k][j];
//            f = (s / a[i][i]) * g;
//            for (k = i; k <= m; k++)
//               a[k][j] += f * a[k][i];
//         }
//         for (j = i; j <= m; j++)
//            a[j][i] *= g;
//      }
//      else
//         for (j = i; j <= m; j++)
//            a[j][i] = 0.0;
//      ++a[i][i];
//   }
//   for (k = n; k >= 1; k--)
//   {
//      for (its = 1; its <= 30; its++)
//      {
//         flag = 1;
//         for (l = k; l >= 1; l--)
//         {
//            nm = l - 1;
//            if ((float)(fabs(rv1[l]) + anorm) == anorm)
//            {
//               flag = 0;
//               break;
//            }
//            if ((float)(fabs(w[nm]) + anorm) == anorm)
//               break;
//         }
//         if (flag)
//         {
//            c = 0.0;
//            s = 1.0;
//            for (i = l; i <= k; i++)
//            {
//               f = s * rv1[i];
//               rv1[i] = c * rv1[i];
//               if ((float)(fabs(f) + anorm) == anorm)
//                  break;
//               g = w[i];
//               h = pythag(f, g);
//               w[i] = h;
//               h = 1.0 / h;
//               c = g * h;
//               s = -f * h;
//               for (j = 1; j <= m; j++)
//               {
//                  y = a[j][nm];
//                  z = a[j][i];
//                  a[j][nm] = y * c + z * s;
//                  a[j][i] = z * c - y * s;
//               }
//            }
//         }
//         z = w[k];
//         if (l == k)
//         {
//            if (z < 0.0)
//            {
//               w[k] = -z;
//               for (j = 1; j <= n; j++)
//                  v[j][k] = -v[j][k];
//            }
//            break;
//         }
//         if (its == 30)
//            nrerror("no convergence in 30 svdcmp iterations");
//         x = w[l];
//         nm = k - 1;
//         y = w[nm];
//         g = rv1[nm];
//         h = rv1[k];
//         f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
//         g = pythag(f, 1.0);
//         f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
//         c = s = 1.0;
//         for (j = l; j <= nm; j++)
//         {
//            i = j + 1;
//            g = rv1[i];
//            y = w[i];
//            h = s * g;
//            g = c * g;
//            z = pythag(f, h);
//            rv1[j] = z;
//            c = f / z;
//            s = h / z;
//            f = x * c + g * s;
//            g = g * c - x * s;
//            h = y * s;
//            y *= c;
//            for (jj = 1; jj <= n; jj++)
//            {
//               x = v[jj][j];
//               z = v[jj][i];
//               v[jj][j] = x * c + z * s;
//               v[jj][i] = z * c - x * s;
//            }
//            z = pythag(f, h);
//            w[j] = z;
//            if (z)
//            {
//               z = 1.0 / z;
//               c = f * z;
//               s = h * z;
//            }
//            f = c * g + s * y;
//            x = c * y - s * g;
//            for (jj = 1; jj <= m; jj++)
//            {
//               y = a[jj][j];
//               z = a[jj][i];
//               a[jj][j] = y * c + z * s;
//               a[jj][i] = z * c - y * s;
//            }
//         }
//         rv1[l] = 0.0;
//         rv1[k] = f;
//         w[k] = x;
//      }
//   }
//}
//free_vector(rv1, 1, n);
//}
// From https://pages.astro.umd.edu/~ricotti/NEWWEB/teaching/ASTR415/InClassExamples/NR3/legacy/nr2/CPP_211/progs.htm
//bool flag;
//int i, its, j, jj, k, l, nm;
//DP anorm, c, f, g, h, s, scale, x, y, z;
//
//int m = a.nrows();
//int n = a.ncols();
//Vec_DP rv1(n);
//g = scale = anorm = 0.0;
//for (i = 0; i < n; i++)
//{
//	l = i + 2;
//	rv1[i] = scale * g;
//	g = s = scale = 0.0;
//	if (i < m)
//	{
//		for (k = i; k < m; k++)
//         scale += fabs(a[k][i]);
//		if (scale != 0.0)
//		{
//			for (k = i; k < m; k++)
//			{
//				a[k][i] /= scale;
//				s += a[k][i] * a[k][i];
//			}
//			f = a[i][i];
//			g = -SIGN(sqrt(s), f);
//			h = f * g - s;
//			a[i][i] = f - g;
//			for (j = l - 1; j < n; j++)
//			{
//				for (s = 0.0, k = i; k < m; k++)
//               s += a[k][i] * a[k][j];
//				f = s / h;
//				for (k = i; k < m; k++) 
//               a[k][j] += f * a[k][i];
//			}
//			for (k = i; k < m; k++)
//            a[k][i] *= scale;
//		}
//	}
//	w[i] = scale * g;
//	g = s = scale = 0.0;
//	if (i + 1 <= m && i + 1 != n)
//	{
//		for (k = l - 1; k < n; k++) scale += fabs(a[i][k]);
//		if (scale != 0.0)
//		{
//			for (k = l - 1; k < n; k++)
//			{
//				a[i][k] /= scale;
//				s += a[i][k] * a[i][k];
//			}
//			f = a[i][l - 1];
//			g = -SIGN(sqrt(s), f);
//			h = f * g - s;
//			a[i][l - 1] = f - g;
//			for (k = l - 1; k < n; k++)
//            rv1[k] = a[i][k] / h;
//			for (j = l - 1; j < m; j++)
//			{
//				for (s = 0.0, k = l - 1; k < n; k++)
//               s += a[j][k] * a[i][k];
//				for (k = l - 1; k < n; k++)
//               a[j][k] += s * rv1[k];
//			}
//			for (k = l - 1; k < n; k++)
//            a[i][k] *= scale;
//		}
//	}
//	anorm = MAX(anorm, (fabs(w[i]) + fabs(rv1[i])));
//}
//for (i = n - 1; i >= 0; i--)
//{
//	if (i < n - 1)
//	{
//		if (g != 0.0)
//		{
//			for (j = l; j < n; j++)
//				v[j][i] = (a[i][j] / a[i][l]) / g;
//			for (j = l; j < n; j++)
//			{
//				for (s = 0.0, k = l; k < n; k++) 
//               s += a[i][k] * v[k][j];
//				for (k = l; k < n; k++) 
//               v[k][j] += s * v[k][i];
//			}
//		}
//		for (j = l; j < n; j++) 
//         v[i][j] = v[j][i] = 0.0;
//	}
//	v[i][i] = 1.0;
//	g = rv1[i];
//	l = i;
//}
//for (i = MIN(m, n) - 1; i >= 0; i--)
//{
//	l = i + 1;
//	g = w[i];
//	for (j = l; j < n; j++) 
//      a[i][j] = 0.0;
//	if (g != 0.0)
//	{
//		g = 1.0 / g;
//		for (j = l; j < n; j++)
//		{
//			for (s = 0.0, k = l; k < m; k++) s += a[k][i] * a[k][j];
//			f = (s / a[i][i]) * g;
//			for (k = i; k < m; k++) a[k][j] += f * a[k][i];
//		}
//		for (j = i; j < m; j++) a[j][i] *= g;
//	}
//	else for (j = i; j < m; j++) a[j][i] = 0.0;
//	++a[i][i];
//}
//for (k = n - 1; k >= 0; k--)
//{
//	for (its = 0; its < 30; its++)
//	{
//		flag = true;
//		for (l = k; l >= 0; l--)
//		{
//			nm = l - 1;
//			if (fabs(rv1[l]) + anorm == anorm)
//			{
//				flag = false;
//				break;
//			}
//			if (fabs(w[nm]) + anorm == anorm) break;
//		}
//		if (flag)
//		{
//			c = 0.0;
//			s = 1.0;
//			for (i = l; i < k + 1; i++)
//			{
//				f = s * rv1[i];
//				rv1[i] = c * rv1[i];
//				if (fabs(f) + anorm == anorm) break;
//				g = w[i];
//				h = pythag(f, g);
//				w[i] = h;
//				h = 1.0 / h;
//				c = g * h;
//				s = -f * h;
//				for (j = 0; j < m; j++)
//				{
//					y = a[j][nm];
//					z = a[j][i];
//					a[j][nm] = y * c + z * s;
//					a[j][i] = z * c - y * s;
//				}
//			}
//		}
//		z = w[k];
//		if (l == k)
//		{
//			if (z < 0.0)
//			{
//				w[k] = -z;
//				for (j = 0; j < n; j++) v[j][k] = -v[j][k];
//			}
//			break;
//		}
//		if (its == 29) nrerror("no convergence in 30 svdcmp iterations");
//		x = w[l];
//		nm = k - 1;
//		y = w[nm];
//		g = rv1[nm];
//		h = rv1[k];
//		f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
//		g = pythag(f, 1.0);
//		f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
//		c = s = 1.0;
//		for (j = l; j <= nm; j++)
//		{
//			i = j + 1;
//			g = rv1[i];
//			y = w[i];
//			h = s * g;
//			g = c * g;
//			z = pythag(f, h);
//			rv1[j] = z;
//			c = f / z;
//			s = h / z;
//			f = x * c + g * s;
//			g = g * c - x * s;
//			h = y * s;
//			y *= c;
//			for (jj = 0; jj < n; jj++)
//			{
//				x = v[jj][j];
//				z = v[jj][i];
//				v[jj][j] = x * c + z * s;
//				v[jj][i] = z * c - x * s;
//			}
//			z = pythag(f, h);
//			w[j] = z;
//			if (z)
//			{
//				z = 1.0 / z;
//				c = f * z;
//				s = h * z;
//			}
//			f = c * g + s * y;
//			x = c * y - s * g;
//			for (jj = 0; jj < m; jj++)
//			{
//				y = a[jj][j];
//				z = a[jj][i];
//				a[jj][j] = y * c + z * s;
//				a[jj][i] = z * c - y * s;
//			}
//		}
//		rv1[l] = 0.0;
//		rv1[k] = f;
//		w[k] = x;
//	}
//}
//}