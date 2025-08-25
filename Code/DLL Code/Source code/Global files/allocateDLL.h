// Numerical recipes allocation prototypes.
// These functions allow the lower and upper matrix bound to be specified,
// useful if starting from index 1, and may be used in Numerical Recipes
// algorithms without problems.
IMPORT extern float*        MakeVectorNR(long,long);
IMPORT extern void          FreeVectorNR(float*,long,long);
IMPORT extern complex*      MakeCVectorNR(long,long);
IMPORT extern void          FreeCVectorNR(complex*, long , long );
IMPORT extern long*         MakeIVectorNR(long,long);
IMPORT extern void          FreeIVectorNR(long*,long,long);
IMPORT extern double*       MakeDVectorNR(long,long);
IMPORT extern void          FreeDVectorNR(double*,long,long);
IMPORT extern float**       MakeMatrix2DNR(long,long,long,long);
IMPORT extern void          FreeMatrix2DNR(float **m, long, long, long, long);
IMPORT extern double**      MakeDMatrix2DNR(long,long,long,long);
IMPORT extern void          FreeDMatrix2DNR(double **m, long, long, long, long);
IMPORT extern complex**     MakeCMatrix2DNR(long,long,long,long);
IMPORT extern void          FreeCMatrix2DNR(complex **m, long, long, long, long);
 
// Vector (1D) zero indexed allocation function for general storage
IMPORT extern float*      MakeVector(long dim1);
IMPORT extern void        FreeVector(float *v);
IMPORT extern complex*    MakeCVector(long dim1);
IMPORT extern void        FreeCVector(complex *v);

// Memory allocation routines for Prospa Variables
IMPORT extern float**     MakeMatrix2D(long dim1, long dim2);
IMPORT extern void        FreeMatrix2D(float **m);
IMPORT extern double**    MakeDMatrix2D(long dim1, long dim2);
IMPORT extern void        FreeDMatrix2D(double **m);
IMPORT extern complex**   MakeCMatrix2D(long dim1, long dim2);
IMPORT extern void        FreeCMatrix2D(complex **m);
IMPORT extern float***    MakeMatrix3D(long dim1 ,long dim2 ,long dim3);
IMPORT extern void        FreeMatrix3D(float ***m);
IMPORT extern complex***  MakeCMatrix3D(long dim1 ,long dim2 ,long dim3);
IMPORT extern void        FreeCMatrix3D(complex ***m);
IMPORT extern float****   MakeMatrix4D(long dim1 ,long dim2 ,long dim3, long dim4);
IMPORT extern void        FreeMatrix4D(float ****m);
IMPORT extern complex**** MakeCMatrix4D(long dim1 ,long dim2 ,long dim3, long dim4);
IMPORT extern void        FreeCMatrix4D(complex ****m);

