#ifndef PROTOTYPES_H
#define PROTOTYPES_H

// allocate.cp

extern float** matrix(long,long,long,long);
extern void free_matrix(float **m, long, long, long, long);
extern float *vector(long,long);
extern void free_vector(float*,long,long);
extern long *ivector(long,long);
extern void free_ivector(long*,long,long);
extern int MemoryStatus(Interface* itfc ,char args[]);
extern complex *cvector(long,long);
extern void free_cvector(complex*, long , long );
extern complex** cmatrix(long,long,long,long);
extern void free_cmatrix(complex **m, long, long, long, long);

// command_other.cp


// variables_other.cp


#endif // define PROTOTYPES_H