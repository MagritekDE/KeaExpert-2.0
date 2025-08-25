/* Library includes */
#include "stdafx.h"
#include "../Global files/includesDLL.h"

short FindLinearRegion(char*);
void LinearFit(float *x,float *y,long,float*,float*,float*,float*);

/********************************************************************
* Find the largest stretch of linear data in an x-y data set
*
* Parameters are: the smallest segment size to consider
*               : the largest slope uncertainty to consider
*********************************************************************/

short FindLinearRegion(DLLParameters* par, char* args)
{
   long seg;
   short n;
   long i,size;
   Variable varX,varY;
   float slope,slopeErr,intercept,interceptErr;
   
// Get parameters
   if((n = ArgScan(par->itfc,args,3,"x,y,segment","eee","vvl",&varX,&varY,&seg)) < 0)
      return(n);

// Check for errors
   if(VarType(&varX) != MATRIX2D || VarRowSize(&varX) != 1)
   {
      ErrorMessage("first argument should be row vector x");
      return(ERR);
   }
   
   if(VarType(&varY) != MATRIX2D || VarRowSize(&varY) != 1)
   {
      ErrorMessage("second argument should be row vector y");
      return(ERR);
   }

   if(VarColSize(&varX) != VarColSize(&varY))
   {
      ErrorMessage("x & y vector size mismatch");
      return(ERR);
   } 

   if(seg <= 0)
   {
      ErrorMessage("segment size greater than zero");
      return(ERR);
   }

         
   size = VarColSize(&varX);

   if(size < seg)
   {
      ErrorMessage("segment size should be less than or equal to x,y vector length");
      return(ERR);
   }

	float **xMat = VarRealMatrix(&varX);
	float **yMat = VarRealMatrix(&varY);

// Find the most linear region ****************
   float minqf = 1e10;
   float x0,x1,y0;
   float qf;
   short minpos;
   
   for(i = 0; i < size-seg; i++)
   {
      x0 = xMat[0][i];
      x1 = xMat[0][i+seg-1];
      y0 = yMat[0][i];
      LinearFit(&xMat[0][i],&yMat[0][i],seg,&slope,&slopeErr,&intercept,&interceptErr);
      qf = fabs(slopeErr*100/(slope*sqrt(x1-x0)));
      if(qf < minqf) 
      {
         minqf = qf;
         minpos = i;
      }
  //    TextMessage("\ni = %ld x0 = %g slope = %g err = %g qf = %g",i,xMat[0][i],slope,slopeErr,qf);
      
   }

   LinearFit(&xMat[0][minpos],&yMat[0][minpos],seg,&slope,&slopeErr,&intercept,&interceptErr);
   qf = fabs(slopeErr*100/(slope*sqrt(xMat[0][minpos+seg-1]-xMat[0][minpos])));
//   TextMessage("\ni = %ld x0 = %g slope = %g err = %g qf = %g\n\n",minpos,xMat[0][minpos],slope,slopeErr,qf);

// Now allow this region to grow
/*
   long s;
   long m = minpos;
   long minlen = seg;
   for(s = 0;; s++)
   {
      if(m-s < 0 || m+seg-1+s >= size)
         break;
      x0 = xMat[0][m-s];
      x1 = xMat[0][m+seg-1+s];
      y0 = yMat[0][m-s];
      LinearFit(&xMat[0][m-s],&yMat[0][m-s],seg+2*s,&slope,&slopeErr,&intercept,&interceptErr);
      qf = fabs(slopeErr*100/(slope*sqrt(x1-x0)));
      if(qf < minqf) 
      {
         minqf = qf;
         minpos = m-s;
         minlen = seg+2*s;
      }      
   }

   LinearFit(&xMat[0][minpos],&yMat[0][minpos],minlen,&slope,&slopeErr,&intercept,&interceptErr);
   qf = fabs(slopeErr*100/(slope*sqrt(xMat[0][minpos+minlen-1]-xMat[0][minpos])));
//   TextMessage("\nl = %ld x0,x1 = %g,%g slope = %g err = %g qf = %g",minlen,xMat[0][minpos],xMat[0][minpos+minlen-1],slope,slopeErr,qf);
*/

// Return slope and intercept found
   par->retVar[1].MakeAndSetFloat(slope);
   par->retVar[2].MakeAndSetFloat(intercept);
   par->nrRetVar = 2;   
                
   return(OK);
}

void LinearFit(float *xdata, float *ydata, long num, float *slope,
               float *slopeErr, float *intercept,float *interceptErr)

{
   float delta,x,y,m,c,sdat;
   float sx,sy,sxx,sxy,syy;
   long i;
   
// Perform linear fit on short segment
   sx = sy = sxx = sxy = syy = 0.0;

   for(i = 0; i < num; i++)
   {
	   x = xdata[i];
	   y = ydata[i];
	   sx += x;
	   sy += y;
	   sxx += x*x;
	   sxy += y*x;
      syy += y*y;
   }
  
   delta = sxx*num - sx*sx;
   c = *intercept = (sxx*sy - sx*sxy)/delta;
   m = *slope = (sxy*num - sx*sy)/delta;
   sdat = fabs((syy - 2*m*sxy - 2*c*sy + 2*m*c*sx + m*m*sxx + num*c*c)/(num-2)); // Take abs to allow for rounding errors

   *slopeErr = sqrt(num*sdat/delta);
   *interceptErr = sqrt(sdat*(1.0/num + sx*sx/(num*delta))); 
}