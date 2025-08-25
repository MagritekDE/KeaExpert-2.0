#include "stdafx.h"
#include "allocate.h"
#include "defines.h"
#include "memoryLeak.h"

/******************************************************************************

   Numerical recipes allocation functions.

   These functions allocate memory which can only be indexed from any number

******************************************************************************/


/*****************************************************************************
    Allocate space for a float one dimensional array
*****************************************************************************/

EXPORT long* MakeIVectorNR(long nl, long nh)
{
   long *v;
   
   v = (long*)GlobalAlloc(GMEM_FIXED,(nh-nl+1)*sizeof(long));
   if(!v) 
   {
      return((long*)0);
   }
   return(v-nl);
}


/*****************************************************************************
    Free space allocated for a float one dimensional array 
*****************************************************************************/


EXPORT void FreeIVectorNR(long *v, long nl, long nh)
{
   if(v != (long*)0)
   {
      GlobalFree(v+nl);
      v = (long*)0;
   }
}





/*****************************************************************************
     Allocate space for a float one dimensional array
*****************************************************************************/

EXPORT float* MakeVectorNR(long nl, long nh)
{
   float *v;
   
   v = (float*)GlobalAlloc(GMEM_FIXED,(nh-nl+1)*sizeof(float));
   if(!v) 
   {
      return((float*)0);
   }
   return(v-nl);
}


/*****************************************************************************
    Free space allocated for a float one dimensional array 
*****************************************************************************/

EXPORT void FreeVectorNR(float *v, long nl, long nh)
{
   if(v != (float*)0)
   {
      GlobalFree(v+nl);
      v = (float*)0;
   }
}

/*****************************************************************************
    Allocate space for a double one dimensional array 
*****************************************************************************/

EXPORT double* MakeDVectorNR(long nl, long nh)
{
   double *v;

   v = (double*)GlobalAlloc(GMEM_FIXED,(nh-nl+1)*sizeof(double));
   if(!v) 
   {
      return((double*)0);
   }
   return(v-nl);
}


/*****************************************************************************
   Free space allocated for a double one dimensional array
*****************************************************************************/


EXPORT void FreeDVectorNR(double *v, long nl, long nh)
{
   if(v != (double*)0)
   {
      GlobalFree(v+nl);
      v = (double*)0;
   }
}

/*****************************************************************************
   Allocate space for a complex one dimensional array   
*****************************************************************************/

EXPORT complex* MakeCVectorNR(long nl, long nh)
{
   complex *v;
   
   v = (complex*)GlobalAlloc(GMEM_FIXED,(nh-nl+1)*sizeof(complex));
   if(!v) 
   {
      return((complex*)0);
   }
   return(v-nl);
}


/*****************************************************************************
    Free space allocated for a complex one dimensional array
*****************************************************************************/

EXPORT void FreeCVectorNR(complex *v, long nl, long nh)
{
   if(v != (complex*)0)
   {
      GlobalFree(v+nl);
      v = (complex*)0;
   }
}

/*****************************************************************************
    Allocate space for a float two dimensional array 
    row start, row end , col start col end 
*****************************************************************************/

EXPORT float** MakeMatrix2DNR(long nrl,long nrh,long ncl,long nch)
{
   long i;
   long nrow = nrh-nrl+1;
   long ncol = nch-ncl+1;
   float **m;
   
   if(nrow <= 0 || ncol <= 0)
      return((float**)0);

   m = (float**)GlobalAlloc(GMEM_FIXED,nrow*sizeof(float*));
   
   if(!m)
   {
      return((float**)0); 
   }
   m -= nrl;

   m[nrl] = (float*)GlobalAlloc(GMEM_FIXED,nrow*ncol*sizeof(float));
   
   if(!m[nrl])
   {
      GlobalFree(m+nrl);
      return((float**)0); 
   }
   m[nrl] -= ncl;
   
   for(i = nrl+1; i <= nrh; i++)
      m[i] = m[i-1]+ncol;
      
   return m;
}


/*****************************************************************************
      Free space allocated for a float two dimensional array 
*****************************************************************************/

EXPORT void FreeMatrix2DNR(float **m,long nrl,long nrh,long ncl,long nch)
{
   if(m == NULL || nrh == -1 || nch == -1) // Null matrix
      return;
   GlobalFree(m[nrl]+ncl);
   GlobalFree(m+nrl);
   m = NULL;
}

/*****************************************************************************
    Allocate space for a double two dimensional array 
    row start, row end , col start col end 
*****************************************************************************/

EXPORT double** MakeDMatrix2DNR(long nrl,long nrh,long ncl,long nch)
{
   long i;
   long nrow = nrh-nrl+1;
   long ncol = nch-ncl+1;
   double **m;

   m = (double**)GlobalAlloc(GMEM_FIXED,nrow*sizeof(double*));

   if(!m)
   {
      return((double**)0); 
   }
   m -= nrl;

   m[nrl] = (double*)GlobalAlloc(GMEM_FIXED,nrow*ncol*sizeof(double));

   if(!m[nrl])
   {
      GlobalFree(m+nrl);
      return((double**)0); 
   }
   m[nrl] -= ncl;

   for(i = nrl+1; i <= nrh; i++)
      m[i] = m[i-1]+ncol;

   return m;
}


/*****************************************************************************
    Free space allocated for a float two dimensional array
*****************************************************************************/

EXPORT void FreeDMatrix2DNR(double **m,long nrl,long nrh,long ncl,long nch)
{
   if(m == NULL || nrh == -1 || nch == -1) // Null matrix
      return;
   GlobalFree(m[nrl]+ncl);
   GlobalFree(m+nrl);
   m  = NULL;
}


/*****************************************************************************
     Allocate space for a complex float two dimensional array 
     row start, row end , col start col end 
*****************************************************************************/

EXPORT complex** MakeCMatrix2DNR(long nrl,long nrh,long ncl,long nch)
{
   long i;
   long nrow = nrh-nrl+1;
   long ncol = nch-ncl+1;
   complex **m;
   
   m = (complex**)GlobalAlloc(GMEM_FIXED,nrow*sizeof(complex*));
   
   if(!m)
   {
      return((complex**)0); 
   }
   m -= nrl;

   m[nrl] = (complex*)GlobalAlloc(GMEM_FIXED,nrow*ncol*sizeof(complex));
   
   if(!m[nrl])
   {
      GlobalFree(m+nrl);
      return((complex**)0); 
   }
   m[nrl] -= ncl;
   
   for(i = nrl+1; i <= nrh; i++)
      m[i] = m[i-1]+ncol;
      
   return m;
}

/*****************************************************************************
     Free space allocated for a complex float two dimensional array 
*****************************************************************************/

EXPORT void FreeCMatrix2DNR(complex **m,long nrl,long nrh,long ncl,long nch)
{
   if(m == NULL || nrh == -1 || nch == -1) // Null matrix
      return;
   GlobalFree(m[nrl]+ncl);
   GlobalFree(m+nrl);
   m = NULL;
}

/******************************************************************************

   Prospa allocation functions.

   These functions allocate memory which can only be indexed from 0

******************************************************************************/

/*****************************************************************************
    Allocate space for a float one dimensional array
*****************************************************************************/

EXPORT long* MakeIVector(long size)
{
   long *v;
   
   v = (long*)GlobalAlloc(GMEM_FIXED,size*sizeof(long));
   if(!v) 
   {
      return((long*)0);
   }
   return(v);
}


/*****************************************************************************
    Free space allocated for a float one dimensional array 
*****************************************************************************/


EXPORT void FreeIVector(long *v)
{
   if(v != (long*)0)
   {
      GlobalFree(v);
      v = (long*)0;
   }
}

/*****************************************************************************
     Allocate space for a real one dimensional array             
*****************************************************************************/

EXPORT float* MakeVector(long nrX)
{
   float *v;
   
   v = (float*)GlobalAlloc(GMEM_FIXED,nrX*sizeof(float));
   if(!v) 
   {
      return((float*)0);
   }
   return(v);
}

/*****************************************************************************
     Allocate space for a double one dimensional array             
*****************************************************************************/

EXPORT double* MakeDVector(long nrX)
{
   double *v;
   
   v = (double*)GlobalAlloc(GMEM_FIXED,nrX*sizeof(double));
   if(!v) 
   {
      return((double*)0);
   }
   return(v);
}


/*****************************************************************************
    Free space allocated for a real one dimensional array       
*****************************************************************************/

EXPORT void FreeVector(float *v)
{
   if(v != (float*)0)
   {
      GlobalFree(v);
      v = (float*)0;
   }
}

/*****************************************************************************
    Free space allocated for a double one dimensional array       
*****************************************************************************/

EXPORT void FreeDVector(double *v)
{
   if(v != (double*)0)
   {
      GlobalFree(v);
      v = (double*)0;
   }
}

/*****************************************************************************
    Allocate space for a complex one dimensional array            
*****************************************************************************/

EXPORT complex* MakeCVector(long nrX)
{
   complex *v;
   
   v = (complex*)GlobalAlloc(GMEM_FIXED,nrX*sizeof(complex));
   if(!v) 
   {
      return((complex*)0);
   }
   return(v);
}


/*****************************************************************************
    Free space allocated for a complex one dimensional array       
*****************************************************************************/

EXPORT void FreeCVector(complex *v)
{
   if(v != (complex*)0)
   {
      GlobalFree(v);
      v = (complex*)0;
   }
}


/*****************************************************************************
   Allocate space for a float two dimensional array             
*****************************************************************************/

EXPORT long** MakeIMatrix2D(long nrX, long nrY)
{
   long y;
   long **m;
   LONG64 bytes;

// Allocate memory for 2D matrix and pointers to each row ******************* 
   bytes = (LONG64)nrY*sizeof(long*) + (LONG64)nrX*nrY*sizeof(long);
   if(bytes > MAXLONG)
      return((long**)0); 
 
   if(!(m = (long**)GlobalAlloc(GMEM_FIXED,(LONG64)bytes)))
      return((long**)0); 

// Update other pointers **********************************
   m[0] = (long*)(m + nrY);
   for(y = 1; y < nrY; y++)
      m[y] = m[y-1]+nrX;
      
   return(m);
}

/*****************************************************************************
   Allocate space for a float two dimensional array             
*****************************************************************************/

EXPORT float** MakeMatrix2D(long nrX, long nrY)
{
   long y;
   float **m;
   LONG64 bytes;

// Allocate memory for 2D matrix and pointers to each row ******************* 
   bytes = (LONG64)nrY*sizeof(float*) + (LONG64)nrX*nrY*sizeof(float);
   if(bytes > MAXLONG)
      return((float**)0); 
 
   if(!(m = (float**)GlobalAlloc(GMEM_FIXED,(LONG64)bytes)))
      return((float**)0); 

// Update other pointers **********************************
   m[0] = (float*)(m + nrY);
   for(y = 1; y < nrY; y++)
      m[y] = m[y-1]+nrX;
      
   return(m);
}

/*****************************************************************************
    Free space allocated for a float two dimensional array 
*****************************************************************************/

EXPORT void FreeMatrix2D(float **m)
{
   if(m == NULL) // Null matrix
      return;
   GlobalFree(m);
   m = NULL;
}

/*****************************************************************************
    Free space allocated for an integer two dimensional array 
*****************************************************************************/

EXPORT void FreeIMatrix2D(long **m)
{
   if(m == NULL) // Null matrix
      return;
   GlobalFree(m);
   m = NULL;
}

/*****************************************************************************
   Allocate space for a float two dimensional array             
*****************************************************************************/

EXPORT double** MakeDMatrix2D(long nrX, long nrY)
{
   long y;
   double **m;
   LONG64 bytes;

// Allocate memory for 2D matrix and pointers to each row ******************* 
   bytes = (LONG64)nrY*sizeof(double*) + (LONG64)nrX*nrY*sizeof(double);
   if(bytes > MAXLONG)
      return((double**)0); 
 
   if(!(m = (double**)GlobalAlloc(GMEM_FIXED,(LONG64)bytes)))
      return((double**)0); 

// Update other pointers **********************************
   m[0] = (double*)(m + nrY);
   for(y = 1; y < nrY; y++)
      m[y] = m[y-1]+nrX;
      
   return(m);
}

/*****************************************************************************
    Free space allocated for a float two dimensional array 
*****************************************************************************/

EXPORT void FreeDMatrix2D(double **m)
{
   if(m == NULL) // Null matrix
      return;
   GlobalFree(m);
   m = NULL;
}

/*****************************************************************************
      Allocate space for a complex two dimensional array  
*****************************************************************************/
//
//EXPORT complex** MakeCMatrix2D(long nrX, long nrY)
//{
//   long y;
//   complex **m;
//   LONG64 bytes;
//   float test;
//
//// Allocate pointers to the 2D matrix ******************* 
//   bytes = (LONG64)nrY*sizeof(complex*);
//   if(bytes > MAXLONG)
//   {
//      return((complex**)0); 
//   }
//   m = (complex**)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
//   if(!m)
//   {
//      return((complex**)0); 
//   }
//
//// Allocate space for 2D matrix (single block of floats)
//   bytes = (LONG64)nrX*nrY*sizeof(complex);
//   if(bytes > MAXLONG)
//   {
//      GlobalFree(m);
//      return((complex**)0); 
//   }
//
//   m[0] = (complex*)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);   
//   if(!m[0])
//   {
//      GlobalFree(m);
//      return((complex**)0); 
//   }
//
//// Update other pointers **********************************
//   for(y = 1; y < nrY; y++)
//      m[y] = m[y-1]+nrX;
//      
//   return(m);
//}

/*****************************************************************************
      Allocate space for a complex two dimensional array  
*****************************************************************************/

EXPORT complex** MakeCMatrix2D(long nrX, long nrY)
{
   long y;
   complex **m;
   LONG64 bytes;

// Allocate memory for 2D matrix and pointers to each row ******************* 
   bytes = (LONG64)nrY*sizeof(complex*) + (LONG64)nrX*nrY*sizeof(complex);
   if(bytes > MAXLONG)
      return((complex**)0); 

   if(!(m = (complex**)GlobalAlloc(GMEM_FIXED,(LONG64)bytes)))
      return((complex**)0); 

// Update other pointers **********************************
   m[0] = (complex*)(m + nrY);
   for(y = 1; y < nrY; y++)
      m[y] = m[y-1]+nrX;
      
   return(m);
}

/*****************************************************************************
    Free space allocated for a complex two dimensional array  
*****************************************************************************/

EXPORT void FreeCMatrix2D(complex **m)
{
   if(m == NULL) // Null matrix
      return;
   GlobalFree(m);
   m = NULL;
}

/*****************************************************************************
    Allocate space for a float three dimensional array
*****************************************************************************/

EXPORT float*** MakeMatrix3D(long nrX, long nrY, long nrZ)
{
   LONG64 bytes;
   long y,z;
   float ***m;
   
// Allocate a pointer to the 3D matrix ******************* 
   bytes = (LONG64)nrZ*sizeof(float**);
   if(bytes > MAXLONG)
      return((float***)0); 

   m = (float***)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m)
   {
      return((float***)0); 
   }

// Allocate pointers to a set of plane pointers **********
   bytes = (LONG64)nrZ*nrY*sizeof(float*);
   if(bytes > MAXLONG)
	{
	   GlobalFree(m);
	   return((float***)0); 
	}
	m[0] = (float**)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
	if(!m[0])
	{
	   GlobalFree(m);
	   return((float***)0); 
	}

// Update other pointers **********************************
   for(z = 1; z < nrZ; z++)
   {
      m[z] = m[z-1] + nrY;
   }

// Allocate memory for complete data set
   bytes = (LONG64)nrX*nrY*nrZ*sizeof(float);
   if(bytes > MAXLONG)
	{
	   GlobalFree(m[0]);
	   GlobalFree(m);
	   return((float***)0); 
	}
   m[0][0] = (float*)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m[0][0])
	{
	   GlobalFree(m[0]);
	   GlobalFree(m);
	   return((float***)0); 
	}

// Update pointers to each row in each plane **************
   float* off = m[0][0];
   for(z = 0; z < nrZ; z++)
   {
	   for(y = 0; y < nrY; y++)
	   {
	      m[z][y] = off;
         off += nrX;
		}
   }   

   return(m);
}

/*****************************************************************************
    Free space allocated for a float three dimensional array  
*****************************************************************************/

EXPORT void FreeMatrix3D(float ***m)
{   
   if(m == NULL) return;

   GlobalFree(m[0][0]); 
   GlobalFree(m[0]); 
   GlobalFree(m); 
   m = NULL;
} 

/*****************************************************************************
    Allocate space for a complex three dimensional array
*****************************************************************************/

EXPORT complex*** MakeCMatrix3D(long nrX, long nrY, long nrZ)
{
   long y,z;
   complex ***m;
   LONG64 bytes;

// Allocate a pointer to the 3D matrix *******************  
   bytes = (LONG64)nrZ*sizeof(complex**);
   if(bytes > MAXLONG)
      return((complex***)0); 

   m = (complex***)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m)
   {
      return((complex***)0); 
   }

// Allocate pointers to a set of plane pointers **********
   bytes = (LONG64)nrZ*nrY*sizeof(complex*);
   if(bytes > MAXLONG)
	{
	   GlobalFree(m);
	   return((complex***)0); 
	}
	m[0] = (complex**)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);	
	if(!m[0])
	{
	   GlobalFree(m);
	   return((complex***)0); 
	}

// Update other pointers **********************************
   for(z = 1; z < nrZ; z++)
   {
      m[z] = m[z-1] + nrY;
   }
	
// Allocate memory for complete data set
   bytes = (LONG64)nrX*nrY*nrZ*sizeof(complex);
   if(bytes > MAXLONG)
	{
	   GlobalFree(m[0]);
	   GlobalFree(m);
	   return((complex***)0); 
	}
   m[0][0] = (complex*)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m[0][0])
	{
	   GlobalFree(m[0]);
	   GlobalFree(m);
	   return((complex***)0); 
	}

// Update pointers to each row in each plane **************
   complex* off = m[0][0];
   for(z = 0; z < nrZ; z++)
   {
	   for(y = 0; y < nrY; y++)
	   {
	      m[z][y] = off;
         off += nrX;
		}
   }   

   return(m);
}

/*****************************************************************************
     Free space allocated for a complex three dimensional array 
*****************************************************************************/

EXPORT void FreeCMatrix3D(complex ***m)
{   
   if(m == NULL) return;

   GlobalFree(m[0][0]); 
   GlobalFree(m[0]); 
   GlobalFree(m); 
   m = NULL;
}

   

/*****************************************************************************
      Allocate space for a float four dimensional array  
*****************************************************************************/

EXPORT float**** MakeMatrix4D(long nrX, long nrY, long nrZ, long nrQ)
{
   long y,z,q;
   float ****m;
   LONG64 bytes;
   
// Allocate a pointer to the 4D matrix *******************  
   bytes = (LONG64)nrQ*sizeof(float***);
   if(bytes > MAXLONG)
      return((float****)0); 

   m = (float****)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m)
   {
      return((float****)0); 
   }

// Allocate pointers to a set of 3D matrices *******************  
   bytes = (LONG64)nrQ*nrZ*sizeof(float**);
   if(bytes > MAXLONG)
   {
      GlobalFree(m);
      return((float****)0); 
   }
   m[0] = (float***)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m[0])
   {
      GlobalFree(m);
      return((float****)0); 
   }

// Update other pointers **********************************
   for(q = 1; q < nrQ; q++)
   {
      m[q] = m[q-1] + nrZ;
   }

// Allocate pointers to a set of 2D matrices *******************  
   bytes = (LONG64)nrQ*nrZ*nrY*sizeof(float*);
   if(bytes > MAXLONG)
   {
	   GlobalFree(m[0]);
	   GlobalFree(m);
      return((float****)0); 
   }
   m[0][0] = (float**)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m[0][0])
   {
	   GlobalFree(m[0]);
	   GlobalFree(m);
      return((float****)0); 
   }

// Update other pointers **********************************
   float **off2 = m[0][0];
   for(q = 0; q < nrQ; q++)
   {
      for(z = 0; z < nrZ; z++)
      {
         m[q][z] = off2;
         off2 += nrY;
      }
   }

// Allocate memory for complete data set
   bytes = (LONG64)nrQ*nrZ*nrY*nrX*sizeof(float);
   if(bytes > MAXLONG)
	{
	   GlobalFree(m[0][0]);
	   GlobalFree(m[0]);
	   GlobalFree(m);
	   return((float****)0); 
	}
   m[0][0][0] = (float*)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m[0][0][0])
	{
	   GlobalFree(m[0][0]);
	   GlobalFree(m[0]);
	   GlobalFree(m);
	   return((float****)0); 
	}

// Update pointers to each row in each plane **************
   float* off1 = m[0][0][0];
   for(q = 0; q < nrQ; q++)
   {
      for(z = 0; z < nrZ; z++)
      {
	      for(y = 0; y < nrY; y++)
	      {
	         m[q][z][y] = off1;
            off1 += nrX;
		   }
      } 
   }

   return(m);
}

/*****************************************************************************
     Free space allocated for a float four dimensional array 
*****************************************************************************/

EXPORT void FreeMatrix4D(float ****m)
{
   if(m == NULL) return;
   GlobalFree(m[0][0][0]); 
   GlobalFree(m[0][0]); 
   GlobalFree(m[0]); 
   GlobalFree(m); 
   m = NULL;
} 


/*****************************************************************************
   Allocate space for a float four dimensional array   
*****************************************************************************/

EXPORT complex**** MakeCMatrix4D(long nrX, long nrY, long nrZ, long nrQ)
{
   long y,z,q;
   complex ****m;
   LONG64 bytes;
   
// Allocate a pointer to the 4D matrix *******************  
   bytes = (LONG64)nrQ*sizeof(complex***);
   if(bytes > MAXLONG)
   {
      return((complex****)0); 
   }
   m = (complex****)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m)
   {
      return((complex****)0); 
   }

// Allocate pointers to a set of 3D matrices *******************  
   bytes = (LONG64)nrQ*nrZ*sizeof(complex**);
   if(bytes > MAXLONG)
   {
      GlobalFree(m);
      return((complex****)0); 
   }
   m[0] = (complex***)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m[0])
   {
      GlobalFree(m);
      return((complex****)0); 
   }

// Update other pointers **********************************
   for(q = 1; q < nrQ; q++)
   {
      m[q] = m[q-1] + nrZ;
   }

// Allocate pointers to a set of 2D matrices *******************  
   bytes = (LONG64)nrQ*nrZ*nrY*sizeof(complex*);
   if(bytes > MAXLONG)
   {
	   GlobalFree(m[0]);
	   GlobalFree(m);
      return((complex****)0); 
   }
   m[0][0] = (complex**)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m[0][0])
   {
	   GlobalFree(m[0]);
	   GlobalFree(m);
      return((complex****)0); 
   }

// Update other pointers **********************************
   complex **off2 = m[0][0];
   for(q = 0; q < nrQ; q++)
   {
      for(z = 0; z < nrZ; z++)
      {
         m[q][z] = off2;
         off2 += nrY;
      }
   }

// Allocate memory for complete data set
   bytes = (LONG64)nrQ*nrZ*nrY*nrX*sizeof(complex);
   if(bytes > MAXLONG)
	{
	   GlobalFree(m[0][0]);
	   GlobalFree(m[0]);
	   GlobalFree(m);
	   return((complex****)0); 
	}
   m[0][0][0] = (complex*)GlobalAlloc(GMEM_FIXED,(ULONG)bytes);
   if(!m[0][0][0])
	{
	   GlobalFree(m[0][0]);
	   GlobalFree(m[0]);
	   GlobalFree(m);
	   return((complex****)0); 
	}

// Update pointers to each row in each plane **************
   complex* off1 = m[0][0][0];
   for(q = 0; q < nrQ; q++)
   {
      for(z = 0; z < nrZ; z++)
      {
	      for(y = 0; y < nrY; y++)
	      {
	         m[q][z][y] = off1;
            off1 += nrX;
		   }
      } 
   }

   return(m);
}

/*****************************************************************************
      Free space allocated for a float four dimensional array 
*****************************************************************************/

EXPORT void FreeCMatrix4D(complex ****m)
{
   if(m == NULL) return;
   GlobalFree(m[0][0][0]); 
   GlobalFree(m[0][0]); 
   GlobalFree(m[0]); 
   GlobalFree(m); 
   m = NULL;
} 


