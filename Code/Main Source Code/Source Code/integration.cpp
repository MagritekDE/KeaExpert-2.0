#include "stdafx.h"
#include "integration.h"
#include <math.h>
#include "allocate.h"
#include "globals.h"
#include "interface.h"
#include "macro_class.h"
#include "mymath.h"
#include "process.h"
#include "scanstrings.h"
#include "utilities.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

float Function(float x, short &err);
char funcStr[MAX_STR];  // String containing function to evaluate
float trapzd(float (*func)(float,short&), float a, float b, int n, short &err);
float  qsimp(float (*func)(float,short&), float a, float b, short &err);

/*******************************************************************
* Computes the nth stage of refinement of an extended trapezoidal
* rule. func is input as a pointer to a function to be integrated
* between the limits (a,b).
*
* From Numerical Recipes in C by Press et al. p136
*******************************************************************/

#define FUNC(x,e) ((*func)(x,e))

float trapzd(float (*func)(float,short&), float a, float b, int n, short &err)
{
   static float s;
   
   if(n == 1)
   {
      s = 0.5*(b-a)*(FUNC(a,err)+FUNC(b,err));
      return(s);
   }
   else
   {
		int it = 1;
		int j = 1;
      for(it = 1, j = 1; j < n-1; j++)
         it <<= 1;
         
      float tnm = it;
      float del = (b-a)/tnm;
      float x = a + 0.5*del;
		float sum;
      for(sum = 0.0, j = 1; j <= it; j++, x += del)
      {
         sum += FUNC(x,err);
         if(err < 0) break;
      }
      
      s = 0.5*(s+(b-a)*sum/tnm);
      return(s);
   }
}

#define EPS 1.0e-6
#define JMAX 15

float  qsimp(float (*func)(float,short&), float a, float b, short &err)
{
   int j;
   float s =0;
	float st,ost,os;
   
   ost = os = -1.0e30;
   
   for(j = 1; j <= JMAX; j++)
   {
      st = trapzd(func,a,b,j,err);
      if(err < 0) return(0);
      s = (4.0*st - ost)/3.0;
      if(fabs(s-os) < EPS*fabs(os))
         return(s);
      os = s;
      ost = st;
   }
   TextMessage("\n\n   Warning! Incomplete convergence\n   try changing the limits\n   or offsetting the data.");
   err = -1;
   return(s);
}

/*******************************************************************************
*            Integrate a user supplied function using Simpson's rule.          *
*******************************************************************************/

int SimpsonsIntegration(Interface* itfc ,char args[])
{
   short r;
   float lim1,lim2;
   float ans;
   short err;
   
	if((r = ArgScan(itfc,args,3,"func,a,b","cee","sff",funcStr,&lim1,&lim2)) < 0)
	   return(r); 

// Integrate
   ans = qsimp(Function,lim1,lim2, err);

// Return answer
   itfc->retVar[1].MakeAndSetFloat(ans);
   itfc->nrRetValues = 1;
      
   return(OK);
}

/*******************************************************************************
*                       Evaluate the function at position x                    *
*******************************************************************************/

float Function(float x, short &err)
{
   char macro[MAX_STR];
   Interface itfc;
   
   sprintf(macro,"procedure(func); x = %f; endproc(%s);",x,funcStr);
   macroArgs = 0;
   err = ProcessMacroStr(&itfc, 0, macro);
   return(itfc.retVar[1].GetReal());
}

/*******************************************************************************
*           Integrate an x,y vector using trapezoidal integration.             *
*
* Modified to handle reversed x axis data - 19/2/07
*******************************************************************************/

int TrapezoidalIntegration(Interface* itfc ,char args[])
{
   short r;
   float lim1,lim2;
   Variable xVar,yVar;
   float *x,*y;
   long xsize,ysize;
   long lim1Index,lim2Index;
   long i,j;

// Get x and y vectors and integration limits  
	if((r = ArgScan(itfc,args,2,"x,y[,a,b]","eeee","vvff",&xVar,&yVar,&lim1,&lim2)) < 0)
	   return(r); 

   if(xVar.GetType() == MATRIX2D && xVar.GetDimY() == 1)
   {
	   xsize = xVar.GetDimX();
      x = CopyArray(xVar.GetMatrix2D()[0],xsize);
	}
	else
	{
	   ErrorMessage("expecting a real row vector for argument 1");
	   return(ERR);
	}

   if(yVar.GetType() == MATRIX2D && yVar.GetDimY() == 1)
   {
	   ysize = yVar.GetDimX();   
	   y = CopyArray(yVar.GetMatrix2D()[0],ysize);
	}
	else
	{
	   FreeVector(x);	
	   ErrorMessage("expecting a real row vector for argument 2");
	   return(ERR);
	}

// Check for size match
   if(xsize != ysize)
	{
	   ErrorMessage("x and y vectors must have same size");
	   FreeVector(x);	
	   FreeVector(y);	
	   return(ERR);
	}   	

// Select limits for integration
   if(r == 2)
   {	
	   lim1 = x[0];
	   lim2 = x[xsize-1];
	}
	else if(r == 4)
	{
	   if(lim2 < lim1)
	      Swap(lim2,lim1);	   
	}
	else
	{
	   FreeVector(x);	
	   FreeVector(y);   
	   ErrorMessage("invalid number of arguments - 4 expected");
	   return(ERR);
	}	
	   	      
// Find limits in terms of indices
   if(x[xsize-1] > x[0]) // Ascending abscissa
   {
      for(i = 0; i < xsize; i++)
      {
         if(x[i] > lim1) break;
      }
      lim1Index = i-1;
      
      for(j = i; j < xsize; j++)
      {
         if(x[j] > lim2) break;
      }
      lim2Index = j;
   }
   else // Descending abscissa
   {
      for(i = xsize-1; i >= 0; i--)
      {
         if(x[i] > lim1) break;
      }
      lim2Index = i-1;
      
      for(j = i; j >= 0; j--)
      {
         if(x[j] > lim2) break;
      }
      lim1Index = j;
   }

// Integrate using simple trapezoidal method
   float sum = 0;
   for(long i = lim1Index; i < lim2Index-1; i++)
   {
      sum += 0.5 * (y[i+1]+y[i]) * (x[i+1]-x[i]);
   }
      
// Return answer
   itfc->retVar[1].MakeAndSetFloat(sum);
   itfc->nrRetValues = 1;

// Free memory
	FreeVector(x);	
   FreeVector(y);

   return(OK);
}   