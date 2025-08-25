#include "stdafx.h"
#include "../Global files/includesDLL.h"

#define VERSION 2.3 // 26-Sept-2024

// Prospa interface functions
EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);

// Locally defined functions
short PolyFit(DLLParameters*,char*);
extern short T1Fit(DLLParameters*,char*);
extern short T2Fit(DLLParameters*,char*);
extern short DiffFit(DLLParameters*,char*);
extern short CapillaryFit(DLLParameters*,char*);
extern short FindLinearRegion(DLLParameters*,char*);
extern short TriExpFit(DLLParameters*,char *parameters);
extern short GaussFit(DLLParameters*,char *parameters);
extern short LorentzianFit(DLLParameters*,char *parameters);
extern short LorentziansFit(DLLParameters*,char *parameters);
extern short BiExpFit(DLLParameters*,char *parameters);
extern short ExpFitWithOffset(DLLParameters*,char*);
extern short NonLinearFit(DLLParameters*, char*);
extern short PhaseCorrection(DLLParameters*, char*);

int HelpFolder(DLLParameters*,char*);
short SVD(DLLParameters*,char*);
short InverseGJ(DLLParameters*,char*);
short InverseGJ2(DLLParameters*,char*);
extern short svdcmp_dbl(double **a, long m, long n, double *w, double **v, long mode);
extern short LogNormalFit(DLLParameters* par, char *parameters);

/*******************************************************************************
   Extension procedure to add commands to Prospa  
*******************************************************************************/

EXPORT short AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;
      
   if(!strcmp(command,"polyfit"))                r = PolyFit(dpar,parameters);      
   else if(!strcmp(command,"invert"))            r = InverseGJ(dpar,parameters);      
   else if(!strcmp(command,"invertd"))           r = InverseGJ2(dpar,parameters);      
   else if(!strcmp(command,"expfit"))            r = T2Fit(dpar,parameters);      
   else if(!strcmp(command,"t2fit"))             r = T2Fit(dpar,parameters);      
   else if(!strcmp(command,"t1fit"))             r = T1Fit(dpar,parameters);      
   else if(!strcmp(command,"expofffit"))         r = ExpFitWithOffset(dpar,parameters);      
   else if(!strcmp(command,"diffit"))            r = DiffFit(dpar,parameters);         
   else if(!strcmp(command,"capfit"))            r = CapillaryFit(dpar,parameters);         
   else if(!strcmp(command,"svd"))               r = SVD(dpar,parameters);  
   else if(!strcmp(command,"lognormalfit"))      r = LogNormalFit(dpar,parameters);  
   else if(!strcmp(command,"biexpfit"))          r = BiExpFit(dpar,parameters);  
   else if(!strcmp(command,"triexpfit"))         r = TriExpFit(dpar,parameters);  
   else if(!strcmp(command,"gaussfit"))          r = GaussFit(dpar,parameters);  
   else if(!strcmp(command,"lorentzianfit"))     r = LorentzianFit(dpar,parameters);  
   else if(!strcmp(command,"peakfit"))           r = LorentziansFit(dpar,parameters);  
   else if(!strcmp(command,"nlinfit"))           r = NonLinearFit(dpar, parameters);
   else if(!strcmp(command,"phasecorrection"))   r = PhaseCorrection(dpar, parameters);
   else if(!strcmp(command,"helpfolder"))        r = HelpFolder(dpar,parameters);
           
   return(r);
}

/*******************************************************************************
   Extension procedure to list commands in DLL  
*******************************************************************************/

EXPORT void ListCommands(void)
{
   TextMessage("\n\n   Curve fitting DLL module (V%1.2f)\n\n",VERSION);
   TextMessage("   diffit ............ gaussian fit for diffusion\n");
   TextMessage("   invert ............ return matrix inverse (G-J)\n");
   TextMessage("   invertd ........... return matrix inverse (G-J) - double precision\n");
   TextMessage("   polyfit ........... linear polynomial fit\n");
   TextMessage("   capfit ............ fit to capillary saturation curve\n");
   TextMessage("   peakfit ........... fit to spectral and peaklist\n");
   TextMessage("   svd ............... apply singular value decomposition to a matrix\n");
   TextMessage("   t2fit ............. single exponential fit for T2\n");
   TextMessage("   t1fit ............. single exponential fit for T1\n");
   TextMessage("   expfit ............ single exponential fit for T2\n");
   TextMessage("   expofffit ......... single exponential fit with offset\n");
   TextMessage("   biexpfit .......... double exponential fit\n");
   TextMessage("   triexpfit ......... triple exponential fit\n");
   TextMessage("   gaussfit .......... gaussian fit\n");
   TextMessage("   lorentzianfit ..... lorentzian fit\n");
   TextMessage("   lognormalfit ...... log-normal fit\n");
   TextMessage("   nlinfit ........... general nonlinear fit\n");
   TextMessage("   phasecorrection  .. corrects the phase of the input spectrum\n");

}


/*******************************************************************************
   Extension procedure to return syntax in DLL 
*******************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"diffit"))               strcpy(syntax,"(VEC fit, FLOAT e0, FLOAT d, FLOAT e0err, FLOAT derr) = diffit(STR type, VEC x, VEC y, FLOAT noise, FLOAT arg1, FLOAT arg2, STR report))");
   else if(!strcmp(cmd,"polyfit"))         strcpy(syntax,"(VEC fit, FLOAT p1, FLOAT p2, ... FLOAT e1, FLOAT e2, ... FLOAT chisq) = polyfit(VEC x, VEC y, INT order, INT step, [FLOAT sd, [STR print]])");
   else if(!strcmp(cmd,"svd"))             strcpy(syntax,"(MAT u, MAT v, VEC w) = svd(MAT a)");
   else if(!strcmp(cmd,"capfit"))          strcpy(syntax,"capfit(MAT sat, Mat cap, float P0, float k0, float S0)");
   else if(!strcmp(cmd,"peakfit"))         strcpy(syntax,"VEC fit = peakfit(VEC x, VEC y, MAT peakList)");
   else if(!strcmp(cmd,"expfit"))          strcpy(syntax,"(VEC fit, FLOAT e0, FLOAT t2, FLOAT e0err, FLOAT t2err) = expfit(VEC x, VEC y, [[FLOAT noise], [STR report], [INT maxIterations]])");
   else if(!strcmp(cmd,"t2fit"))           strcpy(syntax,"(VEC fit, FLOAT e0, FLOAT t2, FLOAT e0err, FLOAT t2err) = t2fit(VEC x, VEC y, [[FLOAT noise], [STR report], [INT maxIterations], [e0Guess, t2Guess])");
   else if(!strcmp(cmd,"t1fit"))           strcpy(syntax,"(VEC fit, FLOAT e0, FLOAT t1, FLOAT e0err, FLOAT t1err) = t1fit(VEC x, VEC y, STR type, [[FLOAT noise], [STR report]])");
   else if(!strcmp(cmd,"expofffit"))       strcpy(syntax,"(VEC fit, FLOAT e0, FLOAT tau, FLOAT offset, FLOAT e0err, FLOAT t2err, FLOAT offseterr) = expofffit(VEC x, VEC y, [[FLOAT noise], [STR report]])");
   else if(!strcmp(cmd,"invert"))          strcpy(syntax,"MAT mout = invert(MAT min)");
   else if(!strcmp(cmd,"invertd"))         strcpy(syntax,"DMAT mout = invertd(DMAT min)");
   else if(!strcmp(cmd,"biexpfit"))        strcpy(syntax,"(VEC fit, FLOAT ea, FLOAT ta, FLOAT eb, FLOAT tb, FLOAT ea_err, FLOAT ta_err, FLOAT eb_err, FLOAT tb_err) = biexpfit(VEC x, VEC y, [[FLOAT noise], [STR report], [INT maxIterations]])");
   else if(!strcmp(cmd,"triexpfit"))       strcpy(syntax,"(VEC fit, FLOAT ea, FLOAT ta, FLOAT eb, FLOAT tb, FLOAT ec, FLOAT tc, FLOAT ea_err, FLOAT ta_err, FLOAT eb_err, FLOAT tb_err, FLOAT ec_err, FLOAT tc_err) = triexpfit(VEC x, VEC y, [[FLOAT noise], [STR report], [INT maxIterations])");
   else if(!strcmp(cmd,"lognormalfit"))    strcpy(syntax,"(VEC fit, FLOAT e0, FLOAT t2, FLOAT sigma, FLOAT e0_err, FLOAT t2_err, FLOAT sigma_err) = lognormalfit(VEC x, VEC y, [[FLOAT noise], [STR report], [INT maxIterations])");
   else if(!strcmp(cmd,"gaussfit"))        strcpy(syntax,"(VEC fit, FLOAT a0, FLOAT a1, FLOAT a2, FLOAT a0_err, FLOAT a1_err, FLOAT a2_err) = gaussfit(VEC x, VEC y, [[FLOAT noise], [STR report], [INT maxIterations], [FLOAT a1_init, FLOAT a2_init, FLOAT a3_init])");
   else if(!strcmp(cmd,"lorentzianfit"))   strcpy(syntax,"(VEC fit, FLOAT a0, FLOAT a1, FLOAT a2, FLOAT a0_err, FLOAT a1_err, FLOAT a2_err) = lorentzianfit(VEC x, VEC y, [[FLOAT noise], [STR report], [INT maxIterations], [FLOAT a1_init, FLOAT a2_init, FLOAT a3_init])");
   else if(!strcmp(cmd,"nlinfit"))         strcpy(syntax,"(VEC fit, FLOATS results, FLOATS errors) = nlinfit(VEC x, VEC y, STR function, VEC init_values, [[FLOAT noise], [STR report], [INT maxIterations]])");
   else if(!strcmp(cmd,"phasecorrection")) strcpy(syntax, "(FLOAT ph0, FLOAT ph1, DOUBLE target) = phasecorrection(VEC spectrum, INT order (0/1), [p0Init, p1Init])");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

int HelpFolder(DLLParameters* par,char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\Fitting");
   par->nrRetVar = 1;
   return(OK);
}


/******************************************************************************
*                           Fit a polynomial to a data set                    *
******************************************************************************/

#define MAXPNTS 32768

extern short SingValueDecompFit(float[],float[],long,long,float*,float*,float*,float**,float);
extern void  fpoly(float,float[],long);

short PolyFit(DLLParameters* dllPar, char parameters[])
{
   float x[MAXPNTS];
   float y[MAXPNTS];
   long ord = 4;
   long step = 8;
   long i,j,N,nrPnts;
   short nrArgs;
   float *par,*err,chisq;
   float fac;
   float sd = 1.0; // SD of data - assume 1 
   float **cvm;
   float varDat,ym;
   Variable varX;
   Variable varY;
   char model[500];
   char str[50];
   bool findBest = false;
   CText print = "true";
   
// Get arguments      
   if((nrArgs = ArgScan(dllPar->itfc,parameters,4,"x,y,order,step,[sd, [print]]","eeeeee","vvllft",&varX,&varY,&ord,&step,&sd,&print)) < 0)
     return(nrArgs);  

// Check for valid data type
	if(VarType(&varX) != MATRIX2D || VarType(&varY) != MATRIX2D)
   {
      ErrorMessage("x and y variable should be a real single precision matrix");
      return(ERR);
   }

// Check for input errors *************************************************
   if(VarHeight(&varX) != 1 || VarHeight(&varY) != 1)
   {
      ErrorMessage("x or y variable is not a row vector");
      return(ERR);
   }
   
   nrPnts = VarWidth(&varX);

   if(nrPnts == 0)
   {
      ErrorMessage("Zero data width");
      return(ERR);
   }
      
   if(VarWidth(&varX) != VarWidth(&varY))
   {
      ErrorMessage("x & y vectors do not have same length");
      return(ERR);
   }

   if(ord < 0) 
   {
      findBest = true;
      ord = -ord;
   }
   
   if(ord < 0 || ord > 20)
   {
      ErrorMessage("Invalid order (0->20)");
      return(ERR);
   }
   
   if(step <= 0)
   {
      ErrorMessage("Step size must be > 0");
      return(ERR);
   }

   if(nrPnts/step < ord)
   {
      ErrorMessage("Step size is too large");
      return(ERR);
   }

   if(nrPnts/step > MAXPNTS)
   {
      ErrorMessage("Step size is too small");
      return(ERR);
   }
   
// Copy data to x,y arrays ******************************************
   for(N = 1, i = 0; i < nrPnts; i+= step, N++)
   {
      x[N] = VarRealMatrix(&varX)[0][i];
      y[N] = VarRealMatrix(&varY)[0][i];
   }
   N--;
   
// Fit function to points (xa,ya) using singular value decomposition *
   ord++;
 
   if(findBest)
   {
      float minchisq = 1e30;
      long bestord = 0;
      
      for(i = 1; i <= ord; i++)
      {  
		   par = MakeVectorNR(1L,i);
		   err = MakeVectorNR(1L,i);
		   cvm = MakeMatrix2DNR(1L,i,1L,i);
      
		   if(SingValueDecompFit(x,y,N,i,par,err,&chisq,cvm,sd) == -1)
		   {
		      FreeVectorNR(par,1L,i);
		      FreeVectorNR(err,1L,i);
	         FreeMatrix2DNR(cvm,1L,i,1L,i);
		      ErrorMessage("No convergence");
		      return(ERR);
		   }  
		   if(chisq < minchisq)
		   {
		      bestord = i;
		      minchisq = chisq;
		   }
		   
	      FreeVectorNR(par,1L,i);
	      FreeVectorNR(err,1L,i);
	      FreeMatrix2DNR(cvm,1L,i,1L,i);
		}
		ord = bestord;
   }

   par = MakeVectorNR(1L,ord);
   err = MakeVectorNR(1L,ord);
   cvm = MakeMatrix2DNR(1L,ord,1L,ord);
        
   if(SingValueDecompFit(x,y,N,ord,par,err,&chisq,cvm,sd) == -1)
   {
      FreeVectorNR(par,1L,ord);
      FreeVectorNR(err,1L,ord);
	   FreeMatrix2DNR(cvm,1L,i,1L,i);
      ErrorMessage("No convergence");
      return(ERR);
   }

// Work out variance of input data using chisquare
   for(varDat = 0.0, i = 1; i <= N; i++)
   {
      ym = 0.0;
      fac = 1.0;
      for(j = 1; j <= ord; j++)
      {
         ym += par[j]*fac;
         fac *= x[i];
      }
          
      varDat += (ym - y[i])*(ym - y[i]);
   }
   varDat /= (N-ord);
         
// Print out results **************************************************
   if(print == "true")
   {
      TextMessage("\n\n  Polynomial Fit\n");
      TextMessage("  --------------------\n\n");
   }
   
   strcpy(model,"a0");
   for(i = 2; i <= ord; i++)
   {
      if(i == 2)
         sprintf(str," + a%ld*x",i-1);
      else
         sprintf(str," + a%ld*x^%ld",i-1,i-1);
      strcat(model,str);
   }

   if(print == "true")
   {
      TextMessage("  Model function: %s\n\n",model);
      TextMessage("  Number of supplied data points = %ld\n",N);
      TextMessage("  Estimated standard deviation in data = %2.3g\n",sqrt(varDat));
   
      if(nrArgs == 5) 
      {
         varDat = 1; // Ignore this is correct data SD given
         TextMessage("  Specified standard deviation in data = %2.3g\n",sd);
         TextMessage("  Chi squared = %2.3g\n\n",chisq);      
         TextMessage("  Fit parameters +- SD\n");
         TextMessage("  --------------------\n");
         
      }
      else
      {
         TextMessage("  Chi squared = %2.3g\n\n",chisq);
         TextMessage("  Fit parameters +- estimated SD\n");
         TextMessage("  ------------------------------\n");
      }
   }

// Make best fit vector (returned in ans1 &ans)
	float** bestFit = MakeMatrix2D(N,1);
		
   for(i = 0; i < N; i++)
   {
      ym = 0.0;
      fac = 1.0;
      for(j = 1; j <= ord; j++)
      {
         ym += par[j]*fac;
         fac *= x[i+1];
      }
		bestFit[0][i] = ym;
   }		

// Return to user
   dllPar->retVar[1].MakeAndLoadMatrix2D(bestFit,N,1);
   FreeMatrix2D(bestFit);

   if(print == "true")
   {
      for(i = 1; i <= ord; i++)
      {
         TextMessage("  a%ld:\t%2.3g +- %2.2g\n",i-1,par[i],sqrt(cvm[i][i]*varDat));
      }
   }

// Return best fit parameters (ans2,ans3,...)     
   for(i = 1; i <= ord; i++)
	   dllPar->retVar[i+1].MakeAndSetFloat(par[i]);

   for(i = 1; i <= ord; i++)
	   dllPar->retVar[i+ord+1].MakeAndSetFloat(sqrt(cvm[i][i]*varDat));

   dllPar->retVar[2*ord+2].MakeAndSetFloat(chisq);

   dllPar->nrRetVar = 2*ord+3;
   
   FreeVectorNR(par,1L,ord);
   FreeVectorNR(err,1L,ord);
   FreeMatrix2DNR(cvm,1L,ord,1L,ord);

   return(OK);
}


/*******************************************************************************
CLI interface to SVD. Supply a matrix A and the matrices U, W and V are returned
********************************************************************************/

short SVD(DLLParameters* par, char *args)
{
   Variable vA;
   double **u;
   double *wv;
   double **v;
   long r,c;
   long m,n;
   short nrArgs;
   extern short svdcmp(float **a,long m,long n,float *w,float **v,long mode);

   // Get user parameters 
   if((nrArgs = ArgScan(par->itfc,args,1,"matrix to decompose","e","v",&vA)) < 0)
      return(nrArgs);

   m = VarRowSize(&vA);
   n = VarColSize(&vA);

   u = MakeDMatrix2DNR(1L,m,1L,n);
   wv = MakeDVectorNR(1L,n);
   v = MakeDMatrix2DNR(1L,n,1L,n);

   if(vA.GetType() == MATRIX2D)
   {
      for(r = 1; r <= m; r++)
         for(c = 1; c <= n; c++)
            u[r][c] = (double)VarRealMatrix(&vA)[r-1][c-1];
   }
   else if(vA.GetType() == DMATRIX2D)
   {
      for(r = 1; r <= m; r++)
         for(c = 1; c <= n; c++)
            u[r][c] = VarDoubleMatrix(&vA)[r-1][c-1];
   }
   else
   {
      ErrorMessage("invalid argument to SVD");
      return(ERR);
   }

   if(svdcmp_dbl(u,m,n,wv,v,0) != ERR)
   {
      if(vA.GetType() == MATRIX2D)
      {
         par->retVar[1].MakeAndLoadMatrix2D(NULL,n,m);
         for(r = 1; r <= m; r++)
            for(c = 1; c <= n; c++)
               VarRealMatrix(&par->retVar[1])[r-1][c-1] = (float)u[r][c];

         par->retVar[2].MakeAndLoadMatrix2D(NULL,n,n);
         for(r = 1; r <= n; r++)
            for(c = 1; c <= n; c++)
               VarRealMatrix(&par->retVar[2])[r-1][c-1] = (float)v[r][c];

         par->retVar[3].MakeAndLoadMatrix2D(NULL,n,n);
         for(r = 1; r <= n; r++)
            for(c = 1; c <= n; c++)
               VarRealMatrix(&par->retVar[3])[r-1][c-1] = 0;
         for(c = 1; c <= n; c++)
               VarRealMatrix(&par->retVar[3])[c-1][c-1] = (float)wv[c];

         par->retVar[4].MakeAndLoadMatrix2D(NULL,n,1);
         for(c = 1; c <= n; c++)
            VarRealMatrix(&par->retVar[4])[0][c-1] = (float)wv[c];
      }
      else
      {
         par->retVar[1].MakeAndLoadDMatrix2D(NULL,n,m);
         for(r = 1; r <= m; r++)
            for(c = 1; c <= n; c++)
               VarDoubleMatrix(&par->retVar[1])[r-1][c-1] = u[r][c];

         par->retVar[2].MakeAndLoadDMatrix2D(NULL,n,n);
         for(r = 1; r <= n; r++)
            for(c = 1; c <= n; c++)
               VarDoubleMatrix(&par->retVar[2])[r-1][c-1] = v[r][c];

         par->retVar[3].MakeAndLoadDMatrix2D(NULL,n,n);
         for(r = 1; r <= n; r++)
            for(c = 1; c <= n; c++)
               VarDoubleMatrix(&par->retVar[3])[r-1][c-1] = 0;
         for(c = 1; c <= n; c++)
               VarDoubleMatrix(&par->retVar[3])[c-1][c-1] = wv[c];

         par->retVar[4].MakeAndLoadDMatrix2D(NULL,n,1);
         for(c = 1; c <= n; c++)
            VarDoubleMatrix(&par->retVar[4])[0][c-1] = (float)wv[c];
      }

      par->nrRetVar = 4;
   }

   FreeDMatrix2DNR(u,1L,m,1L,n);
   FreeDMatrix2DNR(v,1L,n,1L,n);
   FreeDVectorNR(wv,1L,n);

   return(OK);
}

short InverseGJ(DLLParameters* par,char *args)
{
   Variable vA,vB;
   float **A;
   float **B;
   long r,c;
   long m,n;
   short nrArgs;
   extern short  gaussj(float **a, long n, float **b, long m);

   // Get user parameters 
   if((nrArgs = ArgScan(par->itfc,args,1,"matrix to invert","ee","vv",&vA,&vB)) < 0)
      return(nrArgs);

   if(vA.GetType() == FLOAT32)
   {
      par->retVar[1].MakeAndSetFloat(1.0/vA.GetReal());
      par->nrRetVar = 1;
      return(OK);
   }
   if(vA.GetType() != MATRIX2D)
   {
      ErrorMessage("argument must be a float or matrix");
      return(ERR);
   }

   if(VarRowSize(&vA) != VarColSize(&vA))
   {
      ErrorMessage("matrix must be square (N by N)");
      return(ERR);
   }

   n = VarRowSize(&vA);

   if(nrArgs == 2)
   {
      if(vB.GetType() != MATRIX2D)
      {
         ErrorMessage("second argument must be matrix");
         return(ERR);
      }

      if(VarRowSize(&vB) != VarRowSize(&vA) || VarColSize(&vB) != 1)
      {
         ErrorMessage("second matrix must be N by 1");
         return(ERR);
      }
   }

   n = VarRowSize(&vA);

   A = MakeMatrix2DNR(1L,n,1L,n);
   for(r = 1; r <= n; r++)
      for(c = 1; c <= n; c++)
         A[r][c] = VarRealMatrix(&vA)[r-1][c-1];

   B = MakeMatrix2DNR(1L,n,1L,1L);
   if(nrArgs == 1)
   {
      for(c = 1; c <= n; c++)
         B[1][c] = 0.0;
   }
   else
   {
      for(c = 1; c <= n; c++)
         B[1][c] = VarRealMatrix(&vB)[0][c-1];
   }

   if(gaussj(A,n,B,1))
   {
      FreeMatrix2DNR(A,1L,n,1L,n);
      FreeMatrix2DNR(B,1L,n,1L,1L);
      return(ERR);
   }

   par->retVar[1].MakeAndLoadMatrix2D(NULL,n,n);
   for(r = 1; r <= n; r++)
      for(c = 1; c <= n; c++)
         VarRealMatrix(&par->retVar[1])[r-1][c-1] = A[r][c];

   par->retVar[2].MakeAndLoadMatrix2D(NULL,1,n);
   for(c = 1; c <= n; c++)
      VarRealMatrix(&par->retVar[2])[0][c-1] = B[1][c];

   par->nrRetVar = 2;

   FreeMatrix2DNR(A,1L,n,1L,n);
   FreeMatrix2DNR(B,1L,n,1L,1L);

   return(OK);
}


short InverseGJ2(DLLParameters* par,char *args)
{
   Variable vA,vB;
   double **A;
   double **B;
   long r,c;
   long m,n;
   short nrArgs;
   extern short  dgaussj(double **a, long n, double **b, long m);

   // Get user parameters 
   if((nrArgs = ArgScan(par->itfc,args,1,"matrix to invert","ee","vv",&vA,&vB)) < 0)
      return(nrArgs);


   if(vA.GetType() != DMATRIX2D)
   {
      ErrorMessage("argument must be a  or dmatrix");
      return(ERR);
   }

   if(VarRowSize(&vA) != VarColSize(&vA))
   {
      ErrorMessage("matrix must be square (N by N)");
      return(ERR);
   }

   n = VarRowSize(&vA);

   if(nrArgs == 2)
   {
      if(vB.GetType() != DMATRIX2D)
      {
         ErrorMessage("second argument must be dmatrix");
         return(ERR);
      }

      if(VarRowSize(&vB) != VarRowSize(&vA) || VarColSize(&vB) != 1)
      {
         ErrorMessage("second matrix must be N by 1");
         return(ERR);
      }
   }

   n = VarRowSize(&vA);

   A = MakeDMatrix2DNR(1L,n,1L,n);
   for(r = 1; r <= n; r++)
      for(c = 1; c <= n; c++)
         A[r][c] = VarDoubleMatrix(&vA)[r-1][c-1];

   B = MakeDMatrix2DNR(1L,n,1L,1L);
   if(nrArgs == 1)
   {
      for(c = 1; c <= n; c++)
         B[1][c] = 0.0;
   }
   else
   {
      for(c = 1; c <= n; c++)
         B[1][c] = VarDoubleMatrix(&vB)[0][c-1];
   }

   if(dgaussj(A,n,B,1))
   {
      FreeDMatrix2DNR(A,1L,n,1L,n);
      FreeDMatrix2DNR(B,1L,n,1L,1L);
      return(ERR);
   }

   par->retVar[1].MakeAndLoadDMatrix2D(NULL,n,n);
   for(r = 1; r <= n; r++)
      for(c = 1; c <= n; c++)
         VarDoubleMatrix(&par->retVar[1])[r-1][c-1] = A[r][c];

   par->retVar[2].MakeAndLoadDMatrix2D(NULL,1,n);
   for(c = 1; c <= n; c++)
      VarDoubleMatrix(&par->retVar[2])[0][c-1] = B[1][c];

   par->nrRetVar = 2;

   FreeDMatrix2DNR(A,1L,n,1L,n);
   FreeDMatrix2DNR(B,1L,n,1L,1L);

   return(OK);
}

/*
 #define TINY 1.0e-20;
 
 void ludcmp(float ** a, int n, int *indx, float *d)
{
   int i,imax,j,k;
   float big,dum,sum,temp;
   float *vv;
  
   vv=vector(1,n);
   *d=1.0;
   for (i=1;i<=n;i++) 
   {
      big = 0.0;
      for(j = 1; j <= n;j++)
         if ((temp = fabs(a[i][j])) > big)
            big=temp;
      if (big == 0.0)
         nrerror("Singular matrix in routine ludcmp");
      vv[i]=1.0/big;
   }
   for (j=1;j<=n;j++)
   {
      for (i=1;i<j;i++)
      {
         sum=a[i][j];
         for (k=1;k<i;k++)
            sum -= a[i][k]*a[k][j];
         a[i][j]=sum;
      }
      big=0.0;
      for (i=j;i<=n;i++)
      {
         sum=a[i][j];
         for (k=1;k<j;k++)
            sum -= a[i][k]*a[k][j];
         a[i][j]=sum;
         if ( (dum=vv[i]*fabs(sum)) >= big) 
         {
            big=dum;
            imax=i;
         }
      }
      if (j != imax)
      {
         for (k=1;k<=n;k++)
         {
            dum=a[imax][k];
            a[imax][k]=a[j][k];
            a[j][k]=dum;
         }
         *d = -(*d);
         vv[imax]=vv[j];
      }
      indx[j]=imax;
      if (a[j][j] == 0.0) 
         a[j][j]=TINY;
      if (j != n)
      {
         dum=1.0/(a[j][j]);
         for (i=j+1;i<=n;i++) a[i][j] *= dum;
      }
   }
   free_vector(vv,1,n);
}

void lubksb(float **a, int n, int *indx, float b[])
{
   int i,ii=0,ip,j;
   float sum;
    
   for (i=1;i<=n;i++)
   {
      ip=indx[i];
      sum=b[ip];
      b[ip]=b[i];
      if(ii)
      {
         for (j=ii;j<=i-1;j++)
            sum -= a[i][j]*b[j];
      }
      else if(sum)
      {
         ii=i;
      }
      b[i]=sum;
   }
   for (i=n;i>=1;i--)
   {
      sum=b[i];
      for (j=i+1;j<=n;j++)
         sum -= a[i][j]*b[j];
      b[i]=sum/a[i][i];
   }
}
*/
