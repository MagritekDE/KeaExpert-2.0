/******************************************************************************
*                                                                             *
*  svd.c : Implements the singular-value-decomposition routine from Numerical *
* Recipes. Used by the baseline correction routines.                          *
*                                                                             *
*                                           Craig Eccles 1992                 *
*                                                                             *
*                                                                             *
******************************************************************************/

#include "stdafx.h"
#include "../Global files/includesDLL.h"

static float at,bt,ct;
#define PYTHAG(a,b) ((at=fabs(a)) > (bt=fabs(b)) ? \
(ct=bt/at,at*sqrt(1.0+ct*ct)) : (bt ? (ct=at/bt,bt*sqrt(1.0+ct*ct)): 0.0))

static float maxarg1,maxarg2;
#define MAXOF(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
   (maxarg1) : (maxarg2))
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define NPOL 5
#define MAX_ARRAY 2048

// Locally defined and used functions 

int SVD(char args[]);
short SingValueDecompFit(float[],float[],long,long,float*,float*,float*,float**,float);
short svdcmp(float**,long,long,float*,float**,long);
short svdfit(float[],float[],float[],long,float[],long,float**,float**,float[],float*,void(*)(float,float[],long) );
void  svbksb(float**,float[],float**,long,long,float[],float[]);
void  svdvar(float**, int, float[], float**);
void  fpoly(float,float[],long);
void  ftrig(float,float[],long);
void  fhyp(float,float[],long);
float sqr(float num);
void Swap(float &s1, float &s2);
void Swap(double &s1, double &s2);
float NUM_PNTS; 
short svdcmp_dbl(double **a, long m, long n, double *w, double **v, long mode);

#define POLY 1
#define TRIG 2

/**********************************************************************************
   Perform a linear fit to data in (x,y) using a polynomial or a trig function
***********************************************************************************/

short SingValueDecompFit(float x[],float y[],
                        long N,long ma,float *a,
                         float *err,float *chisq, float **cvm, float sd)
{
   float *sig;
   float *w;
   float **u;
   float **v;
   long i;
   short r;
   float fac;
   short method = POLY;
   
   if(method == TRIG)
      ma = ma*2+1;

   if(N < ma) 
   {
      TextMessage("\n\n Not enough points defined\n\n");
      return(-2);
   }

// Allocate space for doing calculation
   sig = MakeVectorNR(1L,N);
   w = MakeVectorNR(1L,ma);
   u = MakeMatrix2DNR(1L,N,1L,ma);
   v = MakeMatrix2DNR(1L,ma,1L,ma);
   NUM_PNTS = x[N]-x[1];

// Scale x axis 0->1 to minimise rounding errors
   for(i = 1; i <= N; i++)
      x[i] /= NUM_PNTS;
         
// Initialize standard deviations 
   for(i = 1; i <= N; i++)
      sig[i] = sd; 

// Fit data 
   if(method == POLY)
      r = svdfit(x,y,sig,N,a,ma,u,v,w,chisq,fpoly);
   if(method == TRIG)
      r = svdfit(x,y,sig,N,a,ma,u,v,w,chisq,ftrig);

// Get covariance matrix
   svdvar(v,ma,w,cvm);

// Scale parameters and restore x axis 
   fac = 1;
   for(i = 1; i <= ma; i++)
   {
      a[i] /= fac;
      cvm[i][i] /= (fac*fac);
      fac *= NUM_PNTS;
   }

   for(i = 1; i <= N; i++)
      x[i] *= NUM_PNTS;
         
// Free memory      
   FreeMatrix2DNR(u,1L,N,1L,ma);
   FreeMatrix2DNR(v,1L,ma,1L,ma);
   FreeVectorNR(w,1L,ma);
   FreeVectorNR(sig,1L,N);
   return(r);
}


/*******************************************************************************
  Given an m by n matrix a this routine (from Press et. al. p 67) returns the
  matrices u, w and v where a = u w trans(v). a is replaced by u.
********************************************************************************/

short svdcmp(float **a,long m,long n,float *w,float **v,long mode)
{
   long flag,i,its,j,jj,k,l,nm;
   float c,f,h,s,x,y,z;
   float anorm=0.0,g=0.0,scale=0.0;
   float *rv1;
   char str[100];

   rv1 = MakeVectorNR(1L,n);

/* Householder reduction to bidiagonal form */

   if(mode == 1)
   {
      TextMessage("\r  Reduction to bidiagonal form ...");
   }

   for(i = 1; i <= n; i++)
   {
      if(mode == 1)
      {
         sprintf(str," i = %4d of %4d<",i,n);
         TextMessage(str);
      }

      l = i+1;
      rv1[i] = scale*g;
      g=s=scale=0.0;
      if(i <= m)
      {
         for(k = i; k <= m; k++)
            scale += fabs(a[k][i]);
         if(scale)
         {
            for(k = i; k <= m; k++)
            {
               a[k][i] /= scale;
               s += a[k][i]*a[k][i];
            }
            f = a[i][i];
            g = -SIGN(sqrt(s),f);
            h = f*g - s;
            a[i][i] = f - g;
            if(i != n)
            {
               for(j = l; j <= n; j++)
               {
                  for(s = 0.0, k = i; k <= m ; k++)
                     s += a[k][i]*a[k][j];
                  f = s/h;
                  for(k = i; k <= m; k++)
                     a[k][j] += f*a[k][i];
               }
            }
            for(k = i ; k <= m; k++)
               a[k][i] *= scale;
         }
      }
      w[i] = scale*g;
      g = s = scale = 0.0;
      if(i <= m && i != n)
      {
         for(k = l; k <= n; k++)
           scale += fabs(a[i][k]);
         if(scale)
         {
            for(k = l; k <= n ; k++)
            {
                a[i][k] /= scale;
                s += a[i][k]*a[i][k];
            }
            f = a[i][l];
            g = -SIGN(sqrt(s),f);
            h = f*g-s;
            a[i][l] = f - g;
            for(k = l; k <= n; k++)
               rv1[k] = a[i][k]/h;
            if(i != m)
            {
               for(j = l; j <= m; j++)
               {
                  for(s = 0.0, k = l; k <= n; k++)
                     s += a[j][k]*a[i][k];
                  for(k = l; k <= n; k++)
                     a[j][k] += s*rv1[k];
               }
            }
            for(k = l ; k <= n; k++)
               a[i][k] *= scale;
         }
      }
      anorm = MAXOF(anorm,(fabs(w[i])+fabs(rv1[i])));
   }
         
/* Accumulation of right-hand transforms */

   if(mode == 1)
   {
       TextMessage("\r  Accumulation of rh transforms  ...");
   }

   for(i = n; i >= 1; i--)
   {
      if(mode == 1)
      {
         sprintf(str," i = %4d<",i);
         TextMessage(str);
      }

      if(i < n)
      {
         if(g)
         {
            for(j = l; j <= n; j++)
               v[j][i] = (a[i][j]/a[i][l])/g;
            for(j = l; j <= n; j++)
            {
                for(s = 0.0, k = l; k <= n; k++)
                   s += a[i][k]*v[k][j];
                for(k = l; k <= n; k++)
                   v[k][j] += s*v[k][i];
             }
          }
          for(j = l; j <= n; j++)
             v[i][j] = v[j][i] = 0.0;
      }
      v[i][i] = 1.0;
      g = rv1[i];
      l = i;
   }

/* Accumulation of left-hand transforms */

   if(mode == 1)
   {
       TextMessage("\r  Accumulation of lh transforms  ...");
   }

   for(i = min(n,m); i >= 1; i--)
   {
      if(mode == 1)
      {
         sprintf(str," i = %4d<",i);
         TextMessage(str);
      }

      l = i+1;
      g = w[i];
      if(i < n)
      {
         for(j = l; j <= n; j++)
            a[i][j] = 0.0;
      }
      if(g)
      {
         g = 1.0/g;
         if(i != n)
         {
            for(j = l; j <= n; j++)
            {
               for(s = 0.0, k = l; k <= m; k++)
                   s += a[k][i]*a[k][j];
               f = (s/a[i][i])*g;
               for(k = i; k <= m; k++)
                  a[k][j] += f*a[k][i];
            }
         }
         for(j = i; j <= m; j++)
            a[j][i] *= g;
      }
      else
      {
         for(j = i; j <= m; j++)
           a[j][i] = 0.0;
      }
      ++a[i][i];
   }

/* Diagonalization of the bidiagonal form */

   if(mode == 1)
   {
       TextMessage("\r  Diagonalization ... ");
   }

   for(k = n; k >= 1; k--)
   {
      if(mode == 1)
      {
         sprintf(str," k = %4d<",k);
         TextMessage(str);
      }

      for(its = 1;;its++) /* Loop until convergence condition satisfied */
      {
         flag = 1;
         for(l = k; l >= 1; l--)
         {
            nm = l - 1 ;
            if(fabs(rv1[l])+anorm == anorm)
            {
               flag = 0.0;
               break;
            }
            if(fabs(w[nm]) + anorm == anorm) break;
         }
         if(flag)
         {
            c = 0.0;
            s = 1.0;
            for(i = l; i <=k ;i++)
            {
               f = s*rv1[i];
               if(fabs(f)+anorm != anorm)
               {
                  g = w[i];
                  h = PYTHAG(f,g);
                  w[i] = h;
                  h = 1.0/h;
                  c = g*h;
                  s = (-f*h);
                  for(j = 1; j <= m;j++)
                  {
                     y = a[j][nm];
                     z = a[j][i];
                     a[j][nm] = y*c+z*s;
                     a[j][i] = z*c-y*s;
                  }
               }
	         }
         }
	      z = w[k];
	      if(l == k)
	      {
	         if(z < 0.0)
	         {
	            w[k] = -z;
	            for(j = 1; j <= n; j++)
		         v[j][k] = (-v[j][k]);
	         }
	         break;
	      }
	      if(its == 1000)
	      {
	        if(mode == 1)
	           ErrorMessage("No convergence in 1000 SVD iterations");
	        FreeVectorNR(rv1,1L,n);
	        return(-1);
	      }
	      x = w[l];
	      nm = k - 1;
	      y = w[nm];
	      g = rv1[nm];
	      h = rv1[k];
	      f = ((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
	      g = PYTHAG(f,1.0);
	      f = ((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
	      c=s=1.0;
	      for(j=l; j <= nm; j++)
	      {
	         i = j+1;
	         g = rv1[i];
	         y = w[i]; 
	         h = s*g;
	         g = c*g;
	         z = PYTHAG(f,h);
	         rv1[j] = z;
	         c = f/z;
	         s = h/z;
	         f = x*c+g*s;
	         g = g*c-x*s;
	         h=y*s;
	         y=y*c;
	         for(jj=1; jj <= n; jj++)
	         {
	            x = v[jj][j];
	            z = v[jj][i];
	            v[jj][j] = x*c+z*s;
	            v[jj][i] = z*c-x*s;
	         }
	         z = PYTHAG(f,h);
	         w[j] = z;
	         if(z)
	         {
	            z = 1.0/z;
	            c = f*z;
	            s = h*z;
	         }
	         f = (c*g) + (s*y);
	         x = (c*y) - (s*g);
	         for(jj = 1; jj <= m; jj++)
	         {
	            y = a[jj][j];
	            z = a[jj][i];
	            a[jj][j] = y*c + z*s;       
	            a[jj][i] = z*c-y*s;
	         }
	      }
	      rv1[l] = 0.0;
	      rv1[k] = f;
	      w[k] = x;
      }
   }
   FreeVectorNR(rv1,1L,n);
   return(0);
}

// To evaluate the covariance matrix cvm[1.ma][1..ma] of the fit for ma parameters
// obtained by svdfit, call this routine with matricies v[1..ma][1..ma] and w[1..ma]
// as returned by svdfit.
             
void
svdvar(float **v, int ma, float w[], float **cvm)
{
   int i,j,k;
   float sum,*wti;
   
   wti = MakeVectorNR(1,ma);
   for(i = 1; i <= ma; i++)
   {
      wti[i] = 0.0;
      if(w[i]) wti[i] = 1.0/(w[i]*w[i]);
   }
   for(i = 1; i <= ma; i++)
   {
      for(j = 1; j <= i; j++)
      {
         for(sum = 0.0, k = 1; k <= ma; k++)
            sum += v[i][k]*v[j][k]*wti[k];
         cvm[j][i] = cvm[i][j] = sum;
      }
   }
   FreeVectorNR(wti,1,ma);
}

      
#define TOL 1e-10


short 
svdfit(float x[],float y[],float sig[],long ndata,
       float a[],long ma,float **u,float **v,float w[],float *chisq,
       void (*funcs)(float,float[],long))

{
   long j,i;
   float wmax,tmp,thresh,sum,*b,*afunc;

   b = MakeVectorNR(1L,ndata);
   afunc = MakeVectorNR(1L,ma);
   for(i = 1; i <= ndata; i++)
   {
      (*funcs)(x[i],afunc,ma);
      tmp = 1.0/sig[i];
      for(j = 1; j <= ma; j++)
         u[i][j] = afunc[j]*tmp;
      b[i]=y[i]*tmp;
   }
   if(svdcmp(u,ndata,ma,w,v,0) == -1)
   {
      FreeVectorNR(afunc,1L,ma);
      FreeVectorNR(b,1L,ndata);
      return(-1);
   }
   wmax = 0.0;
   for(j = 1; j <= ma; j++)
      if(w[j] > wmax) wmax = w[j];
   thresh = TOL*wmax;
   for(j = 1; j <= ma; j++)
      if(w[j] < thresh) w[j] = 0.0;

   svbksb(u,w,v,ndata,ma,b,a);
   *chisq = 0.0;
   for(i = 1; i <= ndata; i++)
   {
      (*funcs)(x[i],afunc,ma);
      for(sum = 0.0, j = 1; j <= ma; j++)
         sum += a[j]*afunc[j];
      *chisq += (tmp=(y[i]-sum)/sig[i],tmp*tmp);
   }
   FreeVectorNR(afunc,1L,ma);
   FreeVectorNR(b,1L,ndata);
   return(0);
}
   

void
svbksb(float **u,float w[],float **v,long m,long n,float b[],float x[])
{
   long jj,j,i;
   float s,*tmp;
  
   if(n == 0) return;
   tmp = MakeVectorNR(1L,n);
   for(j = 1; j <= n; j++)
   {
      s = 0.0;
      if(w[j])
      {
         for(i = 1; i <= m; i++)
            s += u[i][j]*b[i];
         s /= w[j];
      }
      tmp[j] = s;
   }
   for(j = 1; j <= n; j++)
   {
      s = 0.0;
      for(jj = 1; jj <= n; jj++)
         s += v[j][jj]*tmp[jj];
      x[j] = s;
   }
   FreeVectorNR(tmp,1L,n);
}


void
fpoly(float x,float p[],long np)
{
   long j;
 
   p[1] = 1.0;
   for(j = 2; j <= np; j++)
      p[j] = p[j-1]*x;
}

void
ftrig(float x,float p[],long np)
{
   long j;

   p[1] = 1.0;
   for(j = 2; j < np; j+=2)
   {
      p[j] = cos(PI*x*(j-1)/NUM_PNTS);
      p[j+1] = sin(PI*x*(j-1)/NUM_PNTS);
   }
}

void
fhyp(float x,float p[],long np)
{
   p[1] = 1.0;
   p[2] = x;
   p[3] = 1.0/(x+1e-5);
}

float
sqr(float num)
{
   return(num*num);
}

void
Swap(float &s1, float &s2)
{
   float temp = s1;
   s1 = s2;
   s2 = temp;
}

void
Swap(double &s1, double &s2)
{
   double temp = s1;
   s1 = s2;
   s2 = temp;
}


/*******************************************************************************
  Given an m by n matrix a this routine (from Press et. al. p 67) returns the
  matrices u, w and v where a = u w trans(v). a is replaced by u.
********************************************************************************/

short svdcmp_dbl(double **a, long m, long n, double *w, double **v, long mode)
{
   long flag,i,its,j,jj,k,l,nm;
   double c,f,h,s,x,y,z;
   double anorm=0.0,g=0.0,scale=0.0;
   double *rv1;
   char str[100];

   rv1 = MakeDVectorNR(1L,n);

/* Householder reduction to bidiagonal form */

   if(mode == 1)
   {
      TextMessage("\r  Reduction to bidiagonal form ...");
   }

   for(i = 1; i <= n; i++)
   {
      if(mode == 1)
      {
         sprintf(str," i = %4d of %4d<",i,n);
         TextMessage(str);
      }

      l = i+1;
      rv1[i] = scale*g;
      g=s=scale=0.0;
      if(i <= m)
      {
         for(k = i; k <= m; k++)
            scale += fabs(a[k][i]);
         if(scale)
         {
            for(k = i; k <= m; k++)
            {
               a[k][i] /= scale;
               s += a[k][i]*a[k][i];
            }
            f = a[i][i];
            g = -SIGN(sqrt(s),f);
            h = f*g - s;
            a[i][i] = f - g;
            if(i != n)
            {
               for(j = l; j <= n; j++)
               {
                  for(s = 0.0, k = i; k <= m ; k++)
                     s += a[k][i]*a[k][j];
                  f = s/h;
                  for(k = i; k <= m; k++)
                     a[k][j] += f*a[k][i];
               }
            }
            for(k = i ; k <= m; k++)
               a[k][i] *= scale;
         }
      }
      w[i] = scale*g;
      g = s = scale = 0.0;
      if(i <= m && i != n)
      {
         for(k = l; k <= n; k++)
           scale += fabs(a[i][k]);
         if(scale)
         {
            for(k = l; k <= n ; k++)
            {
                a[i][k] /= scale;
                s += a[i][k]*a[i][k];
            }
            f = a[i][l];
            g = -SIGN(sqrt(s),f);
            h = f*g-s;
            a[i][l] = f - g;
            for(k = l; k <= n; k++)
               rv1[k] = a[i][k]/h;
            if(i != m)
            {
               for(j = l; j <= m; j++)
               {
                  for(s = 0.0, k = l; k <= n; k++)
                     s += a[j][k]*a[i][k];
                  for(k = l; k <= n; k++)
                     a[j][k] += s*rv1[k];
               }
            }
            for(k = l ; k <= n; k++)
               a[i][k] *= scale;
         }
      }
      anorm = MAXOF(anorm,(fabs(w[i])+fabs(rv1[i])));
   }
         
/* Accumulation of right-hand transforms */

   if(mode == 1)
   {
       TextMessage("\r  Accumulation of rh transforms  ...");
   }

   for(i = n; i >= 1; i--)
   {
      if(mode == 1)
      {
         sprintf(str," i = %4d<",i);
         TextMessage(str);
      }

      if(i < n)
      {
         if(g)
         {
            for(j = l; j <= n; j++)
               v[j][i] = (a[i][j]/a[i][l])/g;
            for(j = l; j <= n; j++)
            {
                for(s = 0.0, k = l; k <= n; k++)
                   s += a[i][k]*v[k][j];
                for(k = l; k <= n; k++)
                   v[k][j] += s*v[k][i];
             }
          }
          for(j = l; j <= n; j++)
             v[i][j] = v[j][i] = 0.0;
      }
      v[i][i] = 1.0;
      g = rv1[i];
      l = i;
   }

/* Accumulation of left-hand transforms */

   if(mode == 1)
   {
       TextMessage("\r  Accumulation of lh transforms  ...");
   }

   for(i = min(n,m); i >= 1; i--)
   {
      if(mode == 1)
      {
         sprintf(str," i = %4d<",i);
         TextMessage(str);
      }

      l = i+1;
      g = w[i];
      if(i < n)
      {
         for(j = l; j <= n; j++)
            a[i][j] = 0.0;
      }
      if(g)
      {
         g = 1.0/g;
         if(i != n)
         {
            for(j = l; j <= n; j++)
            {
               for(s = 0.0, k = l; k <= m; k++)
                   s += a[k][i]*a[k][j];
               f = (s/a[i][i])*g;
               for(k = i; k <= m; k++)
                  a[k][j] += f*a[k][i];
            }
         }
         for(j = i; j <= m; j++)
            a[j][i] *= g;
      }
      else
      {
         for(j = i; j <= m; j++)
           a[j][i] = 0.0;
      }
      ++a[i][i];
   }

/* Diagonalization of the bidiagonal form */

   if(mode == 1)
   {
       TextMessage("\r  Diagonalization ... ");
   }

   for(k = n; k >= 1; k--)
   {
      if(mode == 1)
      {
         sprintf(str," k = %4d<",k);
         TextMessage(str);
      }

      for(its = 1;;its++) /* Loop until convergence condition satisfied */
      {
         flag = 1;
         for(l = k; l >= 1; l--)
         {
            nm = l - 1 ;
            if(fabs(rv1[l])+anorm == anorm)
            {
               flag = 0.0;
               break;
            }
            if(fabs(w[nm]) + anorm == anorm) break;
         }
         if(flag)
         {
            c = 0.0;
            s = 1.0;
            for(i = l; i <=k ;i++)
            {
               f = s*rv1[i];
               if(fabs(f)+anorm != anorm)
               {
                  g = w[i];
                  h = PYTHAG(f,g);
                  w[i] = h;
                  h = 1.0/h;
                  c = g*h;
                  s = (-f*h);
                  for(j = 1; j <= m;j++)
                  {
                     y = a[j][nm];
                     z = a[j][i];
                     a[j][nm] = y*c+z*s;
                     a[j][i] = z*c-y*s;
                  }
               }
	         }
         }
	      z = w[k];
	      if(l == k)
	      {
	         if(z < 0.0)
	         {
	            w[k] = -z;
	            for(j = 1; j <= n; j++)
		         v[j][k] = (-v[j][k]);
	         }
	         break;
	      }
	      if(its == 100)
	      {
	        if(mode == 1)
	           ErrorMessage("No convergence in 100 SVD iterations");
	        FreeDVectorNR(rv1,1L,n);
	        return(-1);
	      }
	      x = w[l];
	      nm = k - 1;
	      y = w[nm];
	      g = rv1[nm];
	      h = rv1[k];
	      f = ((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
	      g = PYTHAG(f,1.0);
	      f = ((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
	      c=s=1.0;
	      for(j=l; j <= nm; j++)
	      {
	         i = j+1;
	         g = rv1[i];
	         y = w[i]; 
	         h = s*g;
	         g = c*g;
	         z = PYTHAG(f,h);
	         rv1[j] = z;
	         c = f/z;
	         s = h/z;
	         f = x*c+g*s;
	         g = g*c-x*s;
	         h=y*s;
	         y=y*c;
	         for(jj=1; jj <= n; jj++)
	         {
	            x = v[jj][j];
	            z = v[jj][i];
	            v[jj][j] = x*c+z*s;
	            v[jj][i] = z*c-x*s;
	         }
	         z = PYTHAG(f,h);
	         w[j] = z;
	         if(z)
	         {
	            z = 1.0/z;
	            c = f*z;
	            s = h*z;
	         }
	         f = (c*g) + (s*y);
	         x = (c*y) - (s*g);
	         for(jj = 1; jj <= m; jj++)
	         {
	            y = a[jj][j];
	            z = a[jj][i];
	            a[jj][j] = y*c + z*s;       
	            a[jj][i] = z*c-y*s;
	         }
	      }
	      rv1[l] = 0.0;
	      rv1[k] = f;
	      w[k] = x;
      }
   }
   FreeDVectorNR(rv1,1L,n);
   return(0);
}

// To evaluate the covariance matrix cvm[1.ma][1..ma] of the fit for ma parameters
// obtained by svdfit, call this routine with matricies v[1..ma][1..ma] and w[1..ma]
// as returned by svdfit.
             
void svdvar_dbl(double **v, int ma, double w[], double **cvm)
{
   int i,j,k;
   double sum,*wti;
   
   wti = MakeDVectorNR(1,ma);
   for(i = 1; i <= ma; i++)
   {
      wti[i] = 0.0;
      if(w[i]) wti[i] = 1.0/(w[i]*w[i]);
   }
   for(i = 1; i <= ma; i++)
   {
      for(j = 1; j <= i; j++)
      {
         for(sum = 0.0, k = 1; k <= ma; k++)
            sum += v[i][k]*v[j][k]*wti[k];
         cvm[j][i] = cvm[i][j] = sum;
      }
   }
   FreeDVectorNR(wti,1,ma);
}


