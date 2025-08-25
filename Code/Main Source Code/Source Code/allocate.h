#ifndef ALLOCATE_H
#define ALLOCATE_H

#include "defines.h"

// Numerical recipes allocation prototypes.
// These functions allow the lower and upper matrix bound to be specified,
// useful if starting from index 1, and may be used in Numerical Recipes
// algorithms without problems.
EXPORT extern float*        MakeVectorNR(long,long);
EXPORT extern void          FreeVectorNR(float*,long,long);
EXPORT extern complex*      MakeCVectorNR(long,long);
EXPORT extern void          FreeCVectorNR(complex*, long , long );
EXPORT extern long*         MakeIVectorNR(long,long);
EXPORT extern void          FreeIVectorNR(long*,long,long);
EXPORT extern double*       MakeDVectorNR(long,long);
EXPORT extern void          FreeDVectorNR(double*,long,long);
EXPORT extern float**       MakeMatrix2DNR(long,long,long,long);
EXPORT extern void          FreeMatrix2DNR(float **m, long, long, long, long);
EXPORT extern double**      MakeDMatrix2DNR(long,long,long,long);
EXPORT extern void          FreeDMatrix2DNR(double **m, long, long, long, long);
EXPORT extern complex**     MakeCMatrix2DNR(long,long,long,long);
EXPORT extern void          FreeCMatrix2DNR(complex **m, long, long, long, long);
 
// Vector (1D) zero indexed allocation function for general storage
EXPORT extern long*       MakeIVector(long dim1);
EXPORT extern void        FreeIVector(long *v);
EXPORT extern float*      MakeVector(long dim1);
EXPORT extern void        FreeVector(float *v);
EXPORT extern double*     MakeDVector(long dim1);
EXPORT extern void        FreeDVector(double *v);
EXPORT extern complex*    MakeCVector(long dim1);
EXPORT extern void        FreeCVector(complex *v);

// Memory allocation routines for Prospa Variables
EXPORT extern float**     MakeMatrix2D(long dim1, long dim2);
EXPORT extern void        FreeMatrix2D(float **m);
EXPORT extern long**      MakeIMatrix2D(long dim1, long dim2);
EXPORT extern void        FreeIMatrix2D(long **m);
EXPORT extern double**    MakeDMatrix2D(long dim1, long dim2);
EXPORT extern void        FreeDMatrix2D(double **m);
EXPORT extern complex**   MakeCMatrix2D(long dim1, long dim2);
EXPORT extern void        FreeCMatrix2D(complex **m);
EXPORT extern float***    MakeMatrix3D(long dim1 ,long dim2 ,long dim3);
EXPORT extern void        FreeMatrix3D(float ***m);
EXPORT extern complex***  MakeCMatrix3D(long dim1 ,long dim2 ,long dim3);
EXPORT extern void        FreeCMatrix3D(complex ***m);
EXPORT extern float****   MakeMatrix4D(long dim1 ,long dim2 ,long dim3, long dim4);
EXPORT extern void        FreeMatrix4D(float ****m);
EXPORT extern complex**** MakeCMatrix4D(long dim1 ,long dim2 ,long dim3, long dim4);
EXPORT extern void        FreeCMatrix4D(complex ****m);

#endif // define ALLOCATE_H