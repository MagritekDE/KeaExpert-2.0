// math.cpp

#ifndef MYMATH_H
#define MYMATH_H

#include "defines.h"

class Variable;
class Interface;

short MatricesSameSize(Variable*, Variable*);
int FillVector(Interface *itfc, char*);

bool FloatEqual(float f1, float f2);
bool FloatNotEqual(float f1, float f2);
bool FloatGreater(float f1, float f2);
bool FloatGreaterEqual(float f1, float f2);
bool FloatLess(float f1, float f2);
bool FloatLessEqual(float f1, float f2);
void FloatSplit(float,char[],char[],short);

bool IsInteger(float num);
bool IsInteger(double num);
bool IsNumber(char *str);
bool IsDouble(char *str);
bool IsDouble(char *str, double &number);
bool DoubleEqual(double f1, double f2);
bool DoubleNotEqual(double f1, double f2);
bool DoubleGreater(double f1, double f2);
bool DoubleGreaterEqual(double f1, double f2);
bool DoubleLess(double f1, double f2);
bool DoubleLessEqual(double f1, double f2);
float* CopyArray(float*, long);
double* CopyDArray(double*, long);
complex* CopyCArray(complex*, long);
float* CopyDtoFArray(double *in, long size);
void SplitComplexArray(complex*, long, float**, float**);
float* MakeLinearArray(long);
float** CopyMatrix(float **in, long width, long height);
complex cmult(complex a, complex b);
complex cmult(float a, complex b);
complex cmult(complex a, float b);
complex cdiv(complex a, complex b);
float log2Base(float dat);
double log2Base(double dat);
float roundBase(float dat);
double roundBase(double dat);

bool IsInteger(float num);
bool IsNumber(char *str);
bool IsNumber(char *str, double &number);

double notd(double num);
double invd(double num);
//float truncf(float num);
double truncd(double num);
float nintf(float num);
float notf(float num);
double nintd(double num);
long  hint(float num);
float invf(float num);
unsigned __int64 nushint(double num);
unsigned __int64 nushint(float num);
signed __int64 nhint(float num);
signed __int64 nhint(double num);
long  nint(float);
float sqr(float num); 
float fracf(float num);
double fracd(double num);
short nsint(float num);

long FindIndexCore(float *v, long N, float x);
long FindIndexCore(double *v, long N, float x);

enum MatMulMode {NAIVE, TRANSPOSE};
// CLI interface
int AddCircleToMatrix(Interface* itfc ,char[]);
int AddRectangleToMatrix(Interface* itfc ,char[]);
int ArcCosine(Interface* itfc ,char args[]);
int ArcSine(Interface *itfc, char arg[]);
int ArcTangent(Interface *itfc, char arg[]);
int CloneArray(Interface* itfc ,char args[]);
int ComplexConjugate(Interface* itfc ,char arg[]);
int ConvertToDoublePrec(Interface* itfc ,char args[]);
int ConvertToSinglePrec(Interface* itfc ,char args[]);
int Convolve(Interface* itfc, char args[]);
int Cosine(Interface *itfc, char arg[]);
int DecimateVector(Interface* itfc ,char arg[]);
int DiagonalMatrix(Interface *itfc, char arg[]);
int OffDiagonalMatrix(Interface *itfc, char arg[]);
int Difference(Interface* itfc ,char arg[]);
int Exponential(Interface *itfc, char arg[]);
int ExtractSubMatrix(Interface* itfc ,char args[]);
int Factorial(Interface* itfc ,char args[]);
int FFTShift(Interface* itfc ,char arg[]);
int FindFractionalPart(Interface* itfc ,char arg[]);
int FindIndex(Interface* itfc ,char arg[]);
int FindIndex2(Interface* itfc ,char arg[]);
int FirstOrderBessel(Interface* itfc ,char args[]);
int GetMatrixDimension(Interface* itfc ,char args[]);
int HexConversion(Interface* itfc ,char args[]);
int HyperbolicCosine(Interface *itfc, char arg[]);
int HyperbolicSine(Interface *itfc, char arg[]);
int HyperbolicTangent(Interface *itfc, char arg[]);
int IdentityMatrix(Interface *itfc, char arg[]);
int ImaginaryPart(Interface* itfc ,char arg[]);
int InsertIntoMatrix(Interface* itfc ,char args[]);
int InterpolateMatrix(Interface* itfc ,char args[]);
int InvOperator(Interface* itfc ,char arg[]);
int JoinMatrices(Interface* itfc, char arg[]);
int LinearVector(Interface* itfc, char arg[]);
int Loge(Interface *itfc, char arg[]);
int Log10(Interface *itfc, char arg[]);
int Log2(Interface *itfc, char arg[]);
int Magnitude(Interface* itfc ,char args[]);
int MatrixTrace(Interface* itfc ,char arg[]);
int Maximum(Interface *itfc, char arg[]);
int Minimum(Interface *itfc, char arg[]);
int Noise(Interface *itfc, char arg[]);
int NoiseDouble(Interface *itfc, char arg[]);
int NotOperator(Interface* itfc ,char arg[]);
int NthOrderBessel(Interface* itfc ,char args[]);
int Phase(Interface *itfc, char[]);
int Random(Interface* itfc ,char arg[]);
int RealPart(Interface *itfc, char arg[]);
int RealToComplex(Interface* itfc ,char arg[]);
int ReflectMatrix(Interface* itfc ,char args[]);
int ReshapeMatrix(Interface* itfc, char args[]);
int RotateMatrix(Interface* itfc ,char args[]);
int RoundToNearestInteger(Interface* itfc ,char arg[]);
int ShiftMatrix(Interface* itfc ,char args[]);
int Sine(Interface *itfc, char arg[]);
int Size(Interface* itfc ,char args[]);
int SizeD(Interface* itfc ,char args[]);
int SquareRoot(Interface *itfc, char arg[]);
int Sum(Interface *itfc, char arg[]);
int Tangent(Interface *itfc, char arg[]);
int TensorProduct(Interface* itfc ,char arg[]);
int TransposeMatrix(Interface* itfc ,char arg[]);
int TruncateToInteger(Interface* itfc ,char arg[]);
int ZerothOrderBessel(Interface* itfc ,char args[]);
int CumulativeSum(Interface* itfc ,char[]);

#endif // define MYMATH_H