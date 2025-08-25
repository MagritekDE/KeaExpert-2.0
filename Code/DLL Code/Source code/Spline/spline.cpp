#include "stdafx.h"
#include "../Global files/includesDLL.h"

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short SplineFunction(DLLParameters*,char *parameters);
void spline(float x[], float y[], int n, float yp1, float ypn, float y2[]);
short splint(float xa[], float ya[], float y2a[], int n, float x, float *y);
short GetHelpFolder(DLLParameters* par, char *args);

Variable *ansVar;
Variable *returnVar;

/*******************************************************************************
    Extension procedure to add spline commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;
      
   if(!strcmp(command,"spline"))              r = SplineFunction(dpar,parameters);      
   else if(!strcmp(command,"helpfolder"))     r = GetHelpFolder(dpar,parameters);      


   return(r);
}

/*******************************************************************************
    Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   Spline DLL module V2.0\n\n");
   TextMessage("   spline ...... spline interpolation\n");
}

/*******************************************************************************
    Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"spline"))  strcpy(syntax,"VEC ynew = spline(VEC xold, VEC yold, VEC xnew)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short GetHelpFolder(DLLParameters* par, char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\Serial");
   par->nrRetVar = 1;
   return(OK);
}


/*********************************************************************************
      This function will interpolate a passed real or complex vector 
*********************************************************************************/

short SplineFunction(DLLParameters* par, char *parameters)
{
  short nrArgs;
   Variable varX,varY,varXNew;
   short type;
   long wn,i,w,h;

// Extract vector argument
   if((nrArgs = ArgScan(par->itfc,parameters,3,"vxold, vyold, vxnew","eee","vvv",&varX, &varY, &varXNew)) < 0)
     return(nrArgs);  
 
// See if its a float variable   
   if(varX.GetType() == MATRIX2D && varY.GetType() == MATRIX2D)
   {
     // Get some information from the variable         
      long wX = varX.GetDimX();
      long hX = varX.GetDimY();
      long wY = varY.GetDimX();
      long hY = varY.GetDimY();
      long wn = varXNew.GetDimX();
      long hn = varXNew.GetDimY();
       
      if(wX != wY | hX != hY)
      {
         ErrorMessage("x and y vectors must have same size");
         return(ERR);
      }

      if(hn != 1)
      {
         ErrorMessage("new x vector should be a row vector");
         return(ERR);
      }

      h = hX;
      w = wX;

      if(h == 1 && w > 1)
      {
         float* x = varX.GetMatrix2D()[0];
         float* y = varY.GetMatrix2D()[0];
         float** xn = varXNew.GetMatrix2D();
         float* y2 = MakeVectorNR(1,w);
         float yout;
      
         spline(x-1, y-1, w, 1e30, 1e30, y2);

         float** yn = MakeMatrix2D(wn,1);

         for(i = 0; i < wn; i++)
         {
            if(splint(x-1,y-1,y2,w,xn[0][i],&yout) == ERR)
            {
               FreeMatrix2D(yn);
               FreeVectorNR(y2,1,w);
               return(ERR);
            }
            yn[0][i] = yout;
         }

         par->retVar[1].MakeAndLoadMatrix2D(yn,wn,1);
         par->nrRetVar = 1;

         FreeMatrix2D(yn);
         FreeVectorNR(y2,1,w);
      }
      else
      {
         ErrorMessage("x and y must be row vectors");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Arguments to 'spline' should be a vectors");
      return(ERR);
   }

   return(OK);
}

/************************************************************************************************
                     Numerical Recipes routines for the spline fit
   Given arrays x[1..n] and y[1..n] containing a tabulated function, i.e., yi = f(xi), with
   x1 < x2 < .. . < xN, and given values yp1 and ypn for the first derivative of the interpolating
   function at points 1 and n, respectively, this routine returns an array y2[1..n] that contains
   the second derivatives of the interpolating function at the tabulated points xi. If yp1 and/or
   ypn are equal to 10^30 or larger, the routine is signaled to set the corresponding boundary
   condition for a natural spline, with zero second derivative on that boundary.
*************************************************************************************************/

void spline(float x[], float y[], int n, float yp1, float ypn, float y2[])
{
   int i,k;
   float p,qn,sig,un,*u;

   u = MakeVectorNR(1,n-1);
   if (yp1 > 0.99e30) // The lower boundary condition is set either to be “natural”
   {
      y2[1]=u[1]=0.0;
   }
   else
   { // or else to have a specified first derivative.
      y2[1] = -0.5;
      u[1]=(3.0/(x[2]-x[1]))*((y[2]-y[1])/(x[2]-x[1])-yp1);
   }
   for (i=2;i<=n-1;i++)
   { // This is the decomposition loop of the tridiagonal algorithm.
     // y2 and u are used for temporary
     // storage of the decomposed factors.
      sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
      p=sig*y2[i-1]+2.0;
      y2[i]=(sig-1.0)/p;
      u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
      u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
   }
   if (ypn > 0.99e30) // The upper boundary condition is set either to be “natural” 
      qn=un=0.0;
   else 
   {  // or else to have a specified first derivative.
      qn=0.5;
      un=(3.0/(x[n]-x[n-1]))*(ypn-(y[n]-y[n-1])/(x[n]-x[n-1]));
   }
   y2[n]=(un-qn*u[n-1])/(qn*y2[n-1]+1.0);
   for (k=n-1;k>=1;k--) // This is the backsubstitution loop of the tridiagonal algorithm.
      y2[k]=y2[k]*y2[k+1]+u[k];

   FreeVectorNR(u,1,n-1);
}

/**********************************************************************************
Given the arrays xa[1..n] and ya[1..n], which tabulate a function (with the xai’s in order),
and given the array y2a[1..n], which is the output from spline above, and given a value of
x, this routine returns a cubic-spline interpolated value y.
**************************************************************************************/

short splint(float xa[], float ya[], float y2a[], int n, float x, float *y)
{
   void nrerror(char error_text[]);
   int klo,khi,k;
   float h,b,a;
   klo=1; 
   /* We will find the right place in the table by means of
      bisection. This is optimal if sequential calls to this
      routine are at random values of x. If sequential calls
      are in order, and closely spaced, one would do better
      to store previous values of klo and khi and test if
      they remain appropriate on the next call. */
   khi=n;
   while (khi-klo > 1)
   {
      k=(khi+klo) >> 1;
      if (xa[k] > x)
         khi=k;
      else
         klo=k;
   } //klo and khi now bracket the input value of x.
   h=xa[khi]-xa[klo];
   if (h == 0.0) 
   {
      ErrorMessage("Bad xa input to routine splint"); //The xa’s must be distinct.
      return(ERR);
   }
   a=(xa[khi]-x)/h;
   b=(x-xa[klo])/h; //Cubic spline polynomial is now evaluated.
   *y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
   return(OK);
}