#include "stdafx.h"
#include "mymath.h"
#include <float.h>
#include <math.h>
#include "allocate.h"
#include "cArg.h"
#include "command_other.h"
#include "evaluate.h"
#include "fourier.h"
#include "list_functions.h"
#include "globals.h"
#include "interface.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include <time.h>
#include <assert.h>         // needed for assert command
#include "memoryLeak.h"

#pragma warning (disable: 4996) // Ignore deprecated library functions


int Round(Interface* itfc ,char arg[]);
float signf(float num);
double signd(double num);

// Local commands
float MakeRandomGaussianNum(long &seed);
double MakeRandomGaussianDNum(long &seed);
float pfunc(float num);
float ran0(long &seed);
double ran0d(long &seed);
short EvaluateSingleArgumentFunction(Interface* itfc, char *arg, float (*func)(float), double (*funcD)(double));
void RotateVector(complex *vIn, complex *vOut, long step, long N, long rot);
void RotateVector(float *vIn, float *vOut, long step, long N, long rot);
void ShiftVector(complex *vIn, complex *vOut, long step, long N, long shift);
void ShiftVector(float *vIn, float *vOut, long step, long N, long shift);
int Histogram(Interface* itfc ,char args[]);
int AddLineToMatrix(Interface* itfc , char args[]);
int PseudoLogBin(Interface* itfc ,char args[]);
int LogVector(Interface* itfc ,char args[]);
int PseudoLogVector(Interface* itfc ,char args[]);
float sumVec(float* vec, long N, long start, long end);
double sumVec(double* vec, long N, long start, long end);
int ExclusiveOr(Interface *itfc, char *args);
int LogBin(Interface *itfc, char args[]);
void LinearFit(float *xdata, float *ydata, long num,
               float *slope,float *intercept);
int Ceiling(Interface* itfc ,char args[]);
int Floor(Interface* itfc ,char arg[]);
int ComplexToReal(Interface *itfc, char arg[]);
int ReturnSign(Interface* itfc, char arg[]);
int FindXValue(Interface *itfc, char arg[]);
int MatrixRMS(Interface* itfc ,char args[]);
int FindExactIndex(Interface *itfc, char args[]);

/*****************************************************************************
   Check to see if a number or a string is a float (32 bit real)
*****************************************************************************/

int IsFloatCLI(Interface* itfc ,char args[])
{
   int r;
   Variable inputVar;

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,1,"input","e","v",&inputVar)) < 0)
      return(r);
   if(inputVar.GetType() == UNQUOTED_STRING)
   {
      char *txt = inputVar.GetString();
      if(IsNumber(txt))
      {
         itfc->retVar[1].MakeAndSetFloat(1);
      }
      else
      {
         itfc->retVar[1].MakeAndSetFloat(0);
      }
   }
   else if(inputVar.GetType() == FLOAT32)
   {
      itfc->retVar[1].MakeAndSetFloat(1);
   }
   else
   {
      itfc->retVar[1].MakeAndSetFloat(0);
   }

   return(OK);
}

/*****************************************************************************
   Check to see if a number or a string is a double (64 bit real)
*****************************************************************************/

int IsDoubleCLI(Interface* itfc ,char args[])
{
   int r;
   Variable inputVar;

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,1,"input","e","v",&inputVar)) < 0)
      return(r);
   if(inputVar.GetType() == UNQUOTED_STRING)
   {
      char *txt = inputVar.GetString();
      if(IsNumber(txt))
      {
         itfc->retVar[1].MakeAndSetFloat(1);
      }
      else
      {
         itfc->retVar[1].MakeAndSetFloat(0);
      }
   }
   else if(inputVar.GetType() == FLOAT64)
   {
      itfc->retVar[1].MakeAndSetFloat(1);
   }
   else
   {
      itfc->retVar[1].MakeAndSetFloat(0);
   }

   return(OK);
}

/*****************************************************************************
   Check to see if a number or a string is an integer (32 bit)
*****************************************************************************/

int IsIntegerCLI(Interface* itfc ,char args[])
{
   int r;
   Variable inputVar;

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,1,"input","e","v",&inputVar)) < 0)
      return(r);
   if(inputVar.GetType() == UNQUOTED_STRING)
   {
      char *txt = inputVar.GetString();
      if(IsNumber(txt))
      {
	      double number;
         char *end;
         number = strtod(txt,&end);
         if((int)number == number)
            itfc->retVar[1].MakeAndSetFloat(1);
         else
            itfc->retVar[1].MakeAndSetFloat(0);
      }
      else
      {
         itfc->retVar[1].MakeAndSetFloat(0);
      }
   }
   else if(inputVar.GetType() == FLOAT32)
   {
      float num = inputVar.GetReal();
      if((int)num == num)
         itfc->retVar[1].MakeAndSetFloat(1);
      else
         itfc->retVar[1].MakeAndSetFloat(0);
   }
   else if(inputVar.GetType() == FLOAT64)
   {
      float num = inputVar.GetDouble();
      if((int)num == num)
         itfc->retVar[1].MakeAndSetFloat(1);
      else
         itfc->retVar[1].MakeAndSetFloat(0);
   }
   else
   {
      itfc->retVar[1].MakeAndSetFloat(0);
   }

   return(OK);
}


/*****************************************************************************
   Calculate a histogram from the supplied data
*****************************************************************************/

int Histogram(Interface* itfc ,char args[])
{
   int r;
   Variable data,range;
   long N;

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,3,"matrix, bins, range","eee","vlv",&data,&N,&range)) < 0)
      return(r);

   if(data.GetType() == MATRIX2D && range.GetType() == MATRIX2D)
   {
      if(range.GetDimX() != 2 || range.GetDimY() != 1)
      {
         ErrorMessage("Invalid range - should be 2 by 1 vector");
         return(ERR);
      }

      long width = data.GetDimX();
      long height = data.GetDimY();
      if(height == 1)
      {
         float *matIn = data.GetMatrix2D()[0];
         float **histx = MakeMatrix2D(N,1);
         float **histy = MakeMatrix2D(N,1);
         float *rng = range.GetMatrix2D()[0];

         float lw,up;
         for(int k = 0; k < N; k++)
         {
            lw = k*(rng[1]-rng[0])/(float)N + rng[0];
            up = (k+1)*(rng[1]-rng[0])/(float)N + rng[0];
            histx[0][k] = lw;
            histy[0][k] = 0;
            for(int q = 0; q < width; q++)
            {
               if(matIn[q] >= lw & matIn[q] < up)
                  histy[0][k]++;
            }
         }
         itfc->retVar[1].AssignMatrix2D(histx,N,1);
         itfc->retVar[2].AssignMatrix2D(histy,N,1);
         itfc->nrRetValues = 2;
         return(OK);
      }
   }
   else
   {
      ErrorMessage("Invalid data or range type - should be real vectors");
      return(ERR);
   }
  

   return(OK);
}


/*****************************************************************************
 Convert a 32 bit float or 2D float matrix to 64 bit precision
*****************************************************************************/

int ConvertToDoublePrec(Interface* itfc ,char args[])
{
   Variable var;
   short r;

// Extract data  ************************   
   if((r = ArgScan(itfc,args,1,"expression","e","v",&var)) < 0)
      return(r);

   switch(var.GetType())
   {
      case(FLOAT32):
      {
         double data = (double)var.GetReal();
         itfc->retVar[1].MakeAndSetDouble(data);
         itfc->nrRetValues = 1;
         break;
      }
		case(FLOAT64):
		{
			double data = var.GetDouble();
			itfc->retVar[1].MakeAndSetDouble(data);
         itfc->nrRetValues = 1;
         break;
		}
      case(MATRIX2D):
      {
         long x,y;
         long width = var.GetDimX();
         long height = var.GetDimY();
         float **matIn = var.GetMatrix2D();
         double **matOut = MakeDMatrix2D(width,height);

         for(y = 0; y < height; y++)
         {
            for(x = 0; x < width; x++)
            {
               matOut[y][x] = matIn[y][x];
            }
         }
         itfc->retVar[1].AssignDMatrix2D(matOut,width,height);
         itfc->nrRetValues = 1;
         break;
      }
      case(DMATRIX2D):
      {
         long x,y;
         long width = var.GetDimX();
         long height = var.GetDimY();
         double **matIn = var.GetDMatrix2D();
         double **matOut = MakeDMatrix2D(width,height);

         for(y = 0; y < height; y++)
         {
            for(x = 0; x < width; x++)
            {
               matOut[y][x] = matIn[y][x];
            }
         }
         itfc->retVar[1].AssignDMatrix2D(matOut,width,height);
         itfc->nrRetValues = 1;
         break;
      }
      default:
         ErrorMessage("invalid type for conversion to double precision");
         return(ERR);
   }

   return(OK);
}

/*****************************************************************************
 Convert a 64 bit float or 2D float matrix to 32 bit precision
*****************************************************************************/

int ConvertToSinglePrec(Interface* itfc, char args[])
{
   Variable var;
   short r;

// Extract data  ************************   
   if((r = ArgScan(itfc,args,1,"expression","e","v",&var)) < 0)
      return(r);

   switch(var.GetType())
   {
		case(FLOAT32):
		{
			float data = (float)var.GetReal();
         itfc->retVar[1].MakeAndSetFloat(data);
         itfc->nrRetValues = 1;
         break;
		}
      case(FLOAT64):
      {
         float data = (float)var.GetDouble();
         itfc->retVar[1].MakeAndSetFloat(data);
         itfc->nrRetValues = 1;
         break;
      }
      case(MATRIX2D):
      {
         long x,y;
         long width = var.GetDimX();
         long height = var.GetDimY();
         float **matIn = var.GetMatrix2D();
         float **matOut = MakeMatrix2D(width,height);

         for(y = 0; y < height; y++)
         {
            for(x = 0; x < width; x++)
            {
               matOut[y][x] = matIn[y][x];
            }
         }
         itfc->retVar[1].AssignMatrix2D(matOut,width,height);
         itfc->nrRetValues = 1;
         break;
      }
      case(DMATRIX2D):
      {
         long x,y;
         long width = var.GetDimX();
         long height = var.GetDimY();
         double **matIn = var.GetDMatrix2D();
         float **matOut = MakeMatrix2D(width,height);

         for(y = 0; y < height; y++)
         {
            for(x = 0; x < width; x++)
            {
               matOut[y][x] = matIn[y][x];
            }
         }
         itfc->retVar[1].AssignMatrix2D(matOut,width,height);
         itfc->nrRetValues = 1;
         break;
      }
      default:
         ErrorMessage("invalid type for conversion to single");
         return(ERR);
   }

   return(OK);
}

// Return the cumulative sum of a 1D or 2D matrix
int CumulativeSum(Interface* itfc, char args[])
{
   Variable var;
   CText dim = "x";
   long x,y;
   short r;
   float sum;

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,1,"matrix, [dim]","ee","vt",&var,&dim)) < 0)
      return(r);

   if(var.GetType() == MATRIX2D)
   {
      long width = var.GetDimX();
      long height = var.GetDimY();
      float **matIn = var.GetMatrix2D();
      float **matOut = MakeMatrix2D(width,height);

      if(dim == "x")
      {
         for(y = 0; y < height; y++)
         {
            sum = 0;
            for(x = 0; x < width; x++)
            {
               sum += matIn[y][x];
               matOut[y][x] = sum;
            }
         }
      }
      else if(dim == "y")
      {
         for(x = 0; x < width; x++)
         {
            sum = 0;
            for(y = 0; y < height; y++)
            {
               sum += matIn[y][x];
               matOut[y][x] = sum;
            }
         }
      }   
      else
      {
         FreeMatrix2D(matOut);
         ErrorMessage("invalid dimension");
         return(ERR);
      }

      itfc->retVar[1].AssignMatrix2D(matOut,width,height);
      itfc->nrRetValues = 1;
 
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   return(OK);
}


/**************************************************************************
            Convolve a 1D or 2D matrix with a kernel
**************************************************************************/

int Convolve(Interface* itfc, char args[])
{
   Variable mat,kern;
   short r;
   int xoff,yoff;
   int i,j,x,y;
   int mwidth,mheight;
   int kwidth,kheight;
   CText symmetrical = "false";


// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,2,"matrix, kernel, center","eee","vvt",&mat,&kern,&symmetrical)) < 0)
      return(r);

   if(mat.GetType() == MATRIX2D && kern.GetType() == MATRIX2D)
   {
      float **km,**matIn,**matOut;
      float sum;

      mwidth = mat.GetDimX();
      mheight = mat.GetDimY();
      matIn = mat.GetMatrix2D();
      matOut = MakeMatrix2D(mwidth,mheight);

      kwidth = kern.GetDimX();
      kheight = kern.GetDimY();

      if((kwidth-1) % 2 != 0)
      {
         ErrorMessage("Kernel matrix width should be an odd number");
         return(ERR);
      }

      if((kheight-1) % 2 != 0)
      {
         ErrorMessage("Kernel matrix height should be an odd number");
         return(ERR);
      }

      if(kwidth > mwidth)
      {
         ErrorMessage("The Kernel matrix width should be less than or equal to the input matrix width");
         return(ERR);
      }

      if(kheight > mheight)
      {
         ErrorMessage("The Kernel matrix height should be less than or equal to the input matrix height");
         return(ERR);
      }

   // Vector convolve
      if(mheight == 1 && kheight == 1 && kwidth < mwidth)
      {
         if(symmetrical == "true") //Centered convolution
         {
            float **km = kern.GetMatrix2D();
            xoff = (kwidth-1)/2;

            for(x = 0; x < xoff; x++)
            {
               matOut[0][x] = matIn[0][x];
            }
            for(x = xoff; x < mwidth-xoff; x++)
            {
               sum = 0;
               for(i = 0; i < kwidth; i++)
                  sum += km[0][i]*matIn[0][x+i-xoff];
               matOut[0][x] =sum;
            }
            for(x = mwidth-xoff; x < mwidth; x++)
            {
               matOut[0][x] = matIn[0][x];
            }
         }
         else // Conventional convolution
         {
            float *km = kern.GetMatrix2D()[0];
            float *mIn = matIn[0];
            float *mOut = matOut[0];

            for(i = 0; i < mwidth; i++)
            {
		        float sum = 0;
                for(j = 0; j < kwidth; j++)
			    {
				    if(i-j >= 0)
                       sum += km[j]*mIn[i-j];
			    } 
			    mOut[i] = sum;
            }
         }
      }

   // Matrix convolve (centered only)
      else if(mheight > 1 && kwidth < mwidth && kheight < mheight)
      {
         km = kern.GetMatrix2D();
         xoff = (kwidth-1)/2;
         yoff = (kheight-1)/2;

      // Copy complete matrix
         for(y = 0; y < mheight; y++)
            for(x = 0; x < mwidth; x++)
               matOut[y][x] = matIn[y][x];

      // Convolve with kernel (edges are missed)
         for(y = yoff; y < mheight-yoff; y++)
         {
            for(x = xoff; x < mwidth-xoff; x++)
            {
               sum = 0;
               for(j = 0; j < kheight; j++)
                  for(i = 0; i < kwidth; i++)
                     sum += km[j][i]*matIn[y+j-yoff][x+i-xoff];
               matOut[y][x] =sum;
            }
         }
      }
      else
      {
         ErrorMessage("No support for supplied matrices");
         return(ERR);
      }

      itfc->retVar[1].AssignMatrix2D(matOut,mwidth,mheight);
      itfc->nrRetValues = 1;
 
   }
   else if(mat.GetType() == CMATRIX2D && kern.GetType() == MATRIX2D)
   {
      float **km;
      complex **matIn,**matOut;
      complex sum;

      mwidth = mat.GetDimX();
      mheight = mat.GetDimY();
      matIn = mat.GetCMatrix2D();
      matOut = MakeCMatrix2D(mwidth,mheight);

      kwidth = kern.GetDimX();
      kheight = kern.GetDimY();

      if((kwidth-1) % 2 != 0)
      {
         ErrorMessage("Kernel matrix width should be an odd number");
         return(ERR);
      }

      if((kheight-1) % 2 != 0)
      {
         ErrorMessage("Kernel matrix height should be an odd number");
         return(ERR);
      }

   // Vector convolve
      if(mheight == 1 && kheight == 1 && kwidth < mwidth)
      {
         if(symmetrical == "true") //Centered convolution
         {
            float **km = kern.GetMatrix2D();
            xoff = (kwidth-1)/2;

            for(x = 0; x < xoff; x++)
            {
               matOut[0][x] = matIn[0][x];
            }
            for(x = xoff; x < mwidth-xoff; x++)
            {
               sum.r = sum.i = 0;
               for(i = 0; i < kwidth; i++)
               {
                  sum.r += km[0][i]*matIn[0][x+i-xoff].r;
                  sum.i += km[0][i]*matIn[0][x+i-xoff].i;
               }
               matOut[0][x] = sum;
            }
            for(x = mwidth-xoff; x < mwidth; x++)
            {
               matOut[0][x] = matIn[0][x];
            }
         }
         else // Conventional convolution
         {
		      float *km = kern.GetMatrix2D()[0];
            complex *mIn = matIn[0];
            complex *mOut = matOut[0];

            for(i = 0; i < mwidth; i++)
            {
		         sum.r = sum.i = 0;
               for(j = 0; j < kwidth; j++)
			      {
				      if(i-j >= 0)
				      {
                     sum.r += km[j]*mIn[i-j].r;
                     sum.i += km[j]*mIn[i-j].i;
				      }
			      } 
			      mOut[i] = sum;
            }
         }
      }
   // Matrix convolve (centered only)
      else if(mheight > 1 && kwidth < mwidth && kheight < mheight)
      {
         km = kern.GetMatrix2D();
         xoff = (kwidth-1)/2;
         yoff = (kheight-1)/2;

      // Copy complete matrix
         for(y = 0; y < mheight; y++)
            for(x = 0; x < mwidth; x++)
               matOut[y][x] = matIn[y][x];

      // Convolve with kernel (edges are missed)
         for(y = yoff; y < mheight-yoff; y++)
         {
            for(x = xoff; x < mwidth-xoff; x++)
            {
               sum.r = sum.i = 0;
               for(j = 0; j < kheight; j++)
               {
                  for(i = 0; i < kwidth; i++)
                  {
                     sum.r += km[j][i]*matIn[y+j-yoff][x+i-xoff].r;
                     sum.i += km[j][i]*matIn[y+j-yoff][x+i-xoff].i;
                  }
               }
               matOut[y][x] =sum;
            }
         }
      }
      else
      {
         ErrorMessage("No support for supplied matrices");
         return(ERR);
      }

      itfc->retVar[1].AssignCMatrix2D(matOut,mwidth,mheight);
      itfc->nrRetValues = 1;
 
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   return(OK);
}


int AddCircleToMatrix(Interface* itfc, char args[])
{
   Variable var;
   long x0,y0,r0;
   long x,y;
   float a0;
   short r;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,5,"matrix, x0, y0, r0, a0","eeeee","vlllf",&var,&x0,&y0, &r0, &a0)) < 0)
      return(r);

   if(var.GetType() == MATRIX2D)
   {
      long width = var.GetDimX();
      long height = var.GetDimY();
      float **matIn = var.GetMatrix2D();

      ans->MakeAndLoadMatrix2D(NULL,width,height);
      float **matOut = ans->GetMatrix2D();

      for(y = 0; y < height; y++)
      {
         for(x = 0; x < width; x++)
         {
            if(((x-x0)*(x-x0) + (y-y0)*(y-y0)) <= r0*r0)
               matOut[y][x] = a0;
            else
               matOut[y][x] = matIn[y][x];
         }
      }
      itfc->nrRetValues = 1;
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   return(OK);
}

int AddRectangleToMatrix(Interface* itfc , char args[])
{
   Variable var;
   long x0,y0,w,h;
   long x,y;
   float a0;
   short r;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,6,"matrix, x0, y0, w, h, a0","eeeeee","vllllf",&var,&x0,&y0, &w, &h, &a0)) < 0)
      return(r);

   if(var.GetType() == MATRIX2D)
   {
     long width = var.GetDimX();
     long height = var.GetDimY();
     float **matIn = var.GetMatrix2D();

     ans->MakeAndLoadMatrix2D(NULL,width,height);
     float **matOut = ans->GetMatrix2D();

     for(y = 0; y < height; y++)
     {
        for(x = 0; x < width; x++)
        {
           matOut[y][x] = matIn[y][x];
        }
     }
     
     for(y = y0-h; y < y0+h; y++)
     {
        if(y < 0 || y >= height) 
           continue;
        for(x = x0-w; x < x0+w; x++)
        {
           if(x < 0 || x >= width)
              continue;
           matOut[y][x] = a0;
        }
     }
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   itfc->nrRetValues = 1;
   return(OK);
}

// Not finished yet - needs other octants
//int AddLineToMatrix(Interface* itfc , char args[])
//{
//   Variable var;
//   int x0,y0,x1,y1;
//   int x,y;
//   float a0;
//   short r;
//
//   Variable* retVar = itfc->retVar;
//   Variable* ans = &retVar[1];
//
//// Extract matrix  ************************   
//   if((r = ArgScan(itfc,args,6,"matrix, x0, y0, x1, y1, a0","eeeeee","vllllf", &var, &x0, &y0, &x1, &y1, &a0)) < 0)
//      return(r);
//
//   if(var.GetType() == MATRIX2D)
//   {
//     long width = var.GetDimX();
//     long height = var.GetDimY();
//     float **matIn = var.GetMatrix2D();
//
//     ans->MakeAndLoadMatrix2D(NULL,width,height);
//     float **matOut = ans->GetMatrix2D();
//
//     for(y = 0; y < height; y++)
//     {
//        for(x = 0; x < width; x++)
//        {
//           matOut[y][x] = matIn[y][x];
//        }
//     }
//
//     int deltax = x1 - x0;
//     int deltay = y1 - y0;
//     float error = 0;
//     if(deltax > 0)
//     {
//        float deltaerr =  fabs ((float)deltay / (float)deltax); 
//        int y = y0;
//        int x;
//        for (x = x0; x <= x1; x++)
//        {
//            matOut[y][x] = a0;
//            error = error + deltaerr;
//            if(error >= 0.5)
//            {
//                y = y + 1;
//                error = error - 1.0;
//            }
//         }
//      }
//     else if(deltax < 0)
//     {
//        float deltaerr = fabs ((float)deltay / (float)deltax); 
//        int y = y0;
//        int x;
//        for (x = x1; x >= x0; x--)
//        {
//            matOut[y][x] = a0;
//            error = error + deltaerr;
//            if(error >= 0.5)
//            {
//                y = y + 1;
//                error = error - 1.0;
//            }
//         }
//      }
//     else if(deltax == 0)
//     {
//        if(deltay > 0)
//        {
//           for (y = y0; y <= y1; y++)
//           {
//               matOut[y][x0] = a0;
//           }
//         }
//        else
//        {
//           for (y = y1; y >= y0; y--)
//           {
//               matOut[y][x0] = a0;
//           }
//         }
//      }
//   }
//   else
//   {
//      ErrorMessage("invalid type");
//      return(ERR);
//   }
//   itfc->nrRetValues = 1;
//   return(OK);
//}


// Apply a linear interpolation to a 2D or 3D real matrix

int InterpolateMatrix(Interface* itfc , char args[])
{
   short r;
   long nx,ny,nz;
   Variable mat;
   long ow,oh,od;
   long x,y,z;
   long xa,ya,za,xb,yb,zb;
   float dx1,dx2;
   float fx,fy,fz;
   float sx,sy,sz;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

   nx = ny = nz = 1;

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,2,"matrix,nx,ny,nz","eeee","vlll",&mat,&nx,&ny,&nz)) < 0)
      return(r);

   switch(mat.GetType())
   {
       case(MATRIX2D):
       {
          if(nx <= 1 || ny <= 0)
          {
             ErrorMessage("invalid interpolate dimensions");
             return(ERR);
          }
          ow = mat.GetDimX();
          oh = mat.GetDimY();
          float **m = mat.GetMatrix2D();

          sx = (nx-1.0)/(ow-1.0);
          sy = (ny-1.0)/(oh-1.0);

          float **interp = MakeMatrix2D(nx,ny);
          if(interp)
          {
             if(ny == 1) // 1D interpolation
             {
                for(x = 0; x < nx; x++)
                {
                   xa = x*(ow-1)/(nx-1);
                   fx = (x-xa*sx)/sx;
                   (x < nx-1) ? (xb = xa+1) : (xb = xa);
                   interp[0][x] = (m[0][xb]-m[0][xa])*fx + m[0][xa];
                }
             }
             else // 2D interpolation
             {
               for(y = 0; y < ny; y++)
               {
                  ya = y*(oh-1)/(ny-1);
                  fy = (y-ya*sy)/sy;

                  for(x = 0; x < nx; x++)
                  {
                     xa = x*(ow-1)/(nx-1);
                     fx = (x-xa*sx)/sx;

                     (x < nx-1) ? (xb = xa+1) : (xb = xa);
                     (y < ny-1) ? (yb = ya+1) : (yb = ya);

                     dx1 = (m[ya][xb]-m[ya][xa])*fx + m[ya][xa];
                     dx2 = (m[yb][xb]-m[yb][xa])*fx + m[yb][xa];
                     interp[y][x] = (dx2-dx1)*fy + dx1;
                  }
               }
             }
             ans->AssignMatrix2D(interp,nx,ny);
          }
          else
          {
             ErrorMessage("unable to allocate matrix memory");
             return(ERR);
          }
          break;
       }
       case(MATRIX3D):
       {
          if(nx <= 1 || ny <= 1 || nz <= 0)
          {
             ErrorMessage("invalid interpolate dimensions");
             return(ERR);
          }

          float dxy1,dxy2;
          ow = mat.GetDimX();
          oh = mat.GetDimY();
          od = mat.GetDimZ();
          float ***m = mat.GetMatrix3D();

          sx = (nx-1.0)/(ow-1.0);
          sy = (ny-1.0)/(oh-1.0);
          sz = (nz-1.0)/(od-1.0);

          float ***interp = MakeMatrix3D(nx,ny,nz);
          if(interp)
          {
             for(z = 0; z < nz; z++)
             {
                 za = z*(od-1)/(nz-1);
                 fz = (z-za*sz)/sz;

                  for(y = 0; y < ny; y++)
                  {
                     ya = y*(oh-1)/(ny-1);
                     fy = (y-ya*sy)/sy;

                     for(x = 0; x < nx; x++)
                     {
                        xa = x*(ow-1)/(nx-1);
                        fx = (x-xa*sx)/sx;

                        (x < nx-1) ? (xb = xa+1) : (xb = xa);
                        (y < ny-1) ? (yb = ya+1) : (yb = ya);
                        (z < nz-1) ? (zb = za+1) : (zb = za);

                        dx1 = (m[za][ya][xb]-m[za][ya][xa])*fx + m[za][ya][xa];
                        dx2 = (m[za][yb][xb]-m[za][yb][xa])*fx + m[za][yb][xa];
                        dxy1 = (dx2-dx1)*fy + dx1;

                        dx1 = (m[zb][ya][xb]-m[zb][ya][xa])*fx + m[zb][ya][xa];
                        dx2 = (m[zb][yb][xb]-m[zb][yb][xa])*fx + m[zb][yb][xa];
                        dxy2 = (dx2-dx1)*fy + dx1;
                        interp[z][y][x] = (dxy2-dxy1)*fz + dxy1;
                     }
                  }
             }
             ans->AssignMatrix3D(interp,nx,ny,nz);
          }
          else
          {
             ErrorMessage("unable to allocate matrix memory");
             return(ERR);
          }
          break;
      }
      default:
      {
         ErrorMessage("data type not supported for interpolation");
         return(ERR);
      }
   }

   itfc->nrRetValues = 1;

   return(OK);
}

/************************************************************************************************
             Rotate a matrix 'm' about its horizonal or vertical centre by by (xShift, yShift)

    Syntax:   mOut = rotate(mIn, xshift, yshift)

************************************************************************************************/

int RotateMatrix(Interface* itfc ,char args[])
{
   Variable var;
   long xrot = 0;
   long yrot = 0;
   long zrot = 0;
   long x,y,z;
   short r;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,2,"matrix,xrot[,yrot[,zrot]","eeee","vlll",&var,&xrot,&yrot,&zrot)) < 0)
      return(r);

   if(var.GetType() == MATRIX2D)
   {
      long width = var.GetDimX();
      long height = var.GetDimY();
      float **matIn = var.GetMatrix2D();

      ans->MakeAndLoadMatrix2D(NULL,width,height);
      float **matOut = ans->GetMatrix2D();

   // Check for valid matrix
      if(width == 0 || height == 0)
      {
         ErrorMessage("invalid matrix dimensions");
         return(ERR);
      }
   // Row matrix - so rotate horizontally
      if(width > 1 && height == 1 && yrot == 0)
      {
         RotateVector(matIn[0],matOut[0],1,width,xrot);
      }

   // Column matrix - so rotate vertically
      else if(width == 1 && height > 1 && xrot == 0)
      {
         RotateVector(matIn[0],matOut[0],1,height,yrot); 
      }

   // General matrix to shift
      else if(width > 1 && height > 1)
      {
         if(xrot != 0 && yrot == 0)
         {
            for(y = 0; y < height; y++)
               RotateVector(matIn[y],matOut[y],1,width,xrot);
         }
         else if(xrot == 0 && yrot != 0)
         {
            for(x = 0; x < width; x++)
               RotateVector(&matIn[0][x],&matOut[0][x],width,height,yrot);
         }
         else if(xrot != 0 && yrot != 0)
         {
            for(y = 0; y < height; y++)
               RotateVector(matIn[y],matOut[y],1,width,xrot);
            for(x = 0; x < width; x++)
               RotateVector(&matOut[0][x],&matIn[0][x],width,height,yrot);
            memcpy(&matOut[0][0],&matIn[0][0],width*height*sizeof(float));
         }
         else
            memcpy(&matOut[0][0],&matIn[0][0],width*height*sizeof(float));
      }
   }
   else if(var.GetType() == CMATRIX2D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      complex **matIn = VarComplexMatrix(&var);

      ans->MakeAndLoadCMatrix2D(NULL,width,height);
      complex **matOut = VarComplexMatrix(ans);

   // Check for valid matrix
      if(width == 0 || height == 0)
      {
         ErrorMessage("invalid matrix dimensions");
         return(ERR);
      }
   // Row matrix - so rotate horizontally
      if(width > 1 && height == 1 && yrot == 0)
      {
         RotateVector(matIn[0],matOut[0],1,width,xrot);
      }

   // Column matrix - so rotate vertically
      else if(width == 1 && height > 1 && xrot == 0)
      {
         RotateVector(matIn[0],matOut[0],1,height,yrot); 
      }

   // General matrix to shift
      else if(width > 1 && height > 1)
      {
         if(xrot != 0 && yrot == 0)
         {
            for(y = 0; y < height; y++)
               RotateVector(matIn[y],matOut[y],1,width,xrot);
         }
         else if(xrot == 0 && yrot != 0)
         {
            for(x = 0; x < width; x++)
               RotateVector(&matIn[0][x],&matOut[0][x],width,height,yrot);
         }
         else if(xrot != 0 && yrot != 0)
         {
            for(y = 0; y < height; y++)
               RotateVector(matIn[y],matOut[y],1,width,xrot);
            for(x = 0; x < width; x++)
               RotateVector(&matOut[0][x],&matIn[0][x],width,height,yrot);
            memcpy(&matOut[0][0],&matIn[0][0],width*height*sizeof(complex));
         }
         else
            memcpy(&matOut[0][0],&matIn[0][0],width*height*sizeof(complex));
      }
   }
   else if(var.GetType() == MATRIX3D)
   {
      long xn,yn,zn;
   // Get the input and output matrix
      long width  = var.GetDimX();
      long height = var.GetDimY();
      long depth  = var.GetDimZ();
      float ***matIn = var.GetMatrix3D();
      ans->MakeAndLoadMatrix3D(NULL,width,height,depth);
      float ***matOut = VarReal3DMatrix(ans);

   // Check for valid matrix
      if(width <= 0 || height <= 0 || depth <= 0)
      {
         ErrorMessage("invalid matrix dimensions");
         return(ERR);
      }

   // Correct for large or negative rotations
      xrot = xrot%width;
      if(xrot < 0) xrot = width+xrot;
      yrot = yrot%height;
      if(yrot < 0) yrot = height+yrot;
      zrot = zrot%depth;
      if(zrot < 0) zrot = depth+zrot;

   // Rotate the data
      for(z = 0; z < depth; z++)
      {
         zn = (z + zrot)%depth;
         for(y = 0; y < height; y++)
         {
            yn = (y + yrot)%height;
            for(x = 0; x < width; x++)
            {
               xn = (x + xrot)%width;
               if(xn < width && yn < height && zn < depth)
               {
                  matOut[zn][yn][xn] = matIn[z][y][x];
               }
            }
         }
      }
   }
   else if(var.GetType() == CMATRIX3D)
   {
      long xn,yn,zn;
   // Get the input and output matrix
      long width  = VarWidth(&var);
      long height = VarHeight(&var);
      long depth  = VarDepth(&var);
      complex ***matIn = VarComplex3DMatrix(&var);
      ans->MakeAndLoadCMatrix3D(NULL,width,height,depth);
      complex ***matOut = VarComplex3DMatrix(ans);

   // Check for valid matrix
      if(width <= 0 || height <= 0 || depth <= 0)
      {
         ErrorMessage("invalid matrix dimensions");
         return(ERR);
      }

   // Correct for large or negative rotations
      xrot = xrot%width;
      if(xrot < 0) xrot = width+xrot;
      yrot = yrot%height;
      if(yrot < 0) yrot = height+yrot;
      zrot = zrot%depth;
      if(zrot < 0) zrot = depth+zrot;

   // Shift the data
      for(z = 0; z < depth; z++)
      {
         zn = (z + zrot)%depth;
         for(y = 0; y < height; y++)
         {
            yn = (y + yrot)%height;
            for(x = 0; x < width; x++)
            {
               xn = (x + xrot)%width;
               if(xn < width && yn < height && zn < depth)
               {
                  matOut[zn][yn][xn] = matIn[z][y][x];
               }
            }
         }
      }
   }
	else if(var.GetType() == LIST)
   {
      long N = var.GetDimX();
      char **lstIn = var.GetList();
		char **lstOut = new char*[N];
		int j;
		 if(xrot >= 0)
		 {
			for(int i = 0; i < N; i++)
			{
				j = (i+xrot)%N;
				lstOut[j] = new char[strlen(lstIn[i])+1];
				strcpy(lstOut[j],lstIn[i]);
			}
      }
		else
		{
			for(int i = 0; i < N; i++)
			{
				j = (i-xrot)%N;
				lstOut[i] = new char[strlen(lstIn[j])+1];
				strcpy(lstOut[i],lstIn[j]);
			}
		}
	   itfc->retVar[1].AssignList(lstOut,N);

	}
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   
   itfc->nrRetValues = 1;
   return(OK);
}

void RotateVector(float *vIn, float *vOut, long step, long N, long rot)
{
   long i,j;

   if(rot >= 0)
   {
      for(i = 0; i < N; i++)
      {
         j = (i+rot)%N;
         vOut[j*step] = vIn[i*step];
      }
   }
   else
   {
      for(i = 0; i < N; i++)
      {
         j = (i-rot)%N;
         vOut[i*step] = vIn[j*step];
      }
   }
}

void RotateVector(complex *vIn, complex *vOut, long step, long N, long rot)
{
   long i,j;

   if(rot >= 0)
   {
      for(i = 0; i < N; i++)
      {
         j = (i+rot)%N;
         vOut[j*step] = vIn[i*step];
      }
   }
   else
   {
      for(i = 0; i < N; i++)
      {
         j = (i-rot)%N;
         vOut[i*step] = vIn[j*step];
      }
   }
}



/******************************************************************************
*  Sort a matrix by row or column
* 
* Syntax: result = sort(list)
******************************************************************************/


int SortMatrixRows(Interface* itfc ,char arg[])
{ 
   short r;               // Function return flag
   Variable var;          // Variable containing matrix
   long row = 0;
   CText mode = "ascending";
   extern void SortMatrix(float **mat, int width, int height, int row, CText mode);

// Get list name, string to insert and position to insert it in   
   if((r = ArgScan(itfc,arg,1,"matrix","eee","vlt",&var,&row,&mode)) < 0)
      return(r); 


// Check for errors 
   if(VarType(&var) == MATRIX2D)
   {
      float **mat = var.GetMatrix2D();
      long width = var.GetDimX();
      long height = var.GetDimY();

      if(row < 0 || row >= width)
      {
         ErrorMessage("Row to sort on it out of bound for this matrix");
         return(ERR);
      }

      if(mode != "ascending" && mode != "descending")
      {
         ErrorMessage("Mode should be 'ascending' or 'descending'");
         return(ERR);
      }

      itfc->retVar[1].MakeAndLoadMatrix2D(mat,width,height);
      SortMatrix(itfc->retVar[1].GetMatrix2D(),width,height,row,mode);
      itfc->nrRetValues = 1;
   }
   else
   {
      ErrorMessage("Object to sort should be a real matrix");
      return(ERR);
   }

   return(OK);
}

void Swap(float &s1, float &s2)
{
   float temp = s1;
   s1 = s2;
   s2 = temp;
}


void SortMatrix(float **mat, int width, int height, int row, CText mode)
{
   long i,j,k;
   float temp;
   if(mode == "ascending")
   {
      for(i = 1; i < height; i++)
      {
         j = i;
         while(j > 0 && mat[j-1][row] > mat[j][row])
         {
            for(k = 0; k < width; k++)
               Swap(mat[j-1][k],mat[j][k]);
            j--;
         }
      }
   }
   else // Descending
   {
      for(i = 1; i < height; i++)
      {
         j = i;
         while(j > 0 && mat[j-1][row] < mat[j][row])
         {
            for(k = 0; k < width; k++)
               Swap(mat[j-1][k],mat[j][k]);
            j--;
         }
      }
   }
}



/************************************************************************************************
             Shift a matrix 'mIn' about its horizonal or vertical centre by (xShift, yShift)

    Syntax:   mOut = shift(mIn, xshift, yshift)

************************************************************************************************/

int ShiftMatrix(Interface* itfc, char args[])
{
   Variable var;
   long xshift = 0;
   long yshift = 0;
   long zshift = 0;
   long x,y,z;
   short r;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,2,"matrix, xshift[, yshift[, zshift]]","eeee","vlll",&var,&xshift,&yshift,&zshift)) < 0)
      return(r);

   if(var.GetType() == MATRIX2D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      float **matIn = VarRealMatrix(&var);
      ans->MakeAndLoadMatrix2D(NULL,width,height);
      float **matOut = ans->GetMatrix2D();
   // Check for valid matrix
      if(width == 0 || height == 0)
      {
         ErrorMessage("invalid matrix dimensions");
         return(ERR);
      }
   // Row matrix - so shift horizontally
      if(width > 1 && height == 1 && yshift == 0)
      {
         ShiftVector(matIn[0],matOut[0],1,width,xshift);
      }

   // Column matrix - so shift vertically
      else if(width == 1 && height > 1 && xshift == 0)
      {
         ShiftVector(matIn[0],matOut[0],1,height,yshift); 
      }

   // General matrix to shift
      else if(width > 1 && height > 1)
      {
         if(xshift != 0 && yshift == 0)
         {
            for(y = 0; y < height; y++)
               ShiftVector(matIn[y],matOut[y],1,width,xshift);
         }
         else if(xshift == 0 && yshift != 0)
         {
            for(x = 0; x < width; x++)
               ShiftVector(&matIn[0][x],&matOut[0][x],width,height,yshift);
         }
         else if(xshift != 0 && yshift != 0)
         {
            for(y = 0; y < height; y++)
               ShiftVector(matIn[y],matOut[y],1,width,xshift);
            memset(&matIn[0][0],0,width*height*sizeof(float));
            for(x = 0; x < width; x++)
               ShiftVector(&matOut[0][x],&matIn[0][x],width,height,yshift);
            memcpy(&matOut[0][0],&matIn[0][0],width*height*sizeof(float));
         }
         else
            memcpy(&matOut[0][0],&matIn[0][0],width*height*sizeof(float));

      }
   }
   else if(var.GetType() == CMATRIX2D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      complex **matIn = VarComplexMatrix(&var);
      ans->MakeAndLoadCMatrix2D(NULL,width,height);
      complex **matOut = VarComplexMatrix(ans);

   // Check for valid matrix
      if(width == 0 || height == 0)
      {
         ErrorMessage("invalid matrix dimensions");
         return(ERR);
      }
   // Row matrix - so shift horizontally
      if(width > 1 && height == 1 && yshift == 0)
      {
         ShiftVector(matIn[0],matOut[0],1,width,xshift);
      }

   // Column matrix - so shift vertically
      else if(width == 1 && height > 1 && xshift == 0)
      {
         ShiftVector(matIn[0],matOut[0],1,height,yshift); 
      }

   // General matrix to shift
      else if(width > 1 && height > 1)
      {
         if(xshift != 0 && yshift == 0)
         {
            for(y = 0; y < height; y++)
               ShiftVector(matIn[y],matOut[y],1,width,xshift);
         }
         else if(xshift == 0 && yshift != 0)
         {
            for(x = 0; x < width; x++)
               ShiftVector(&matIn[0][x],&matOut[0][x],width,height,yshift);
         }
         else if(xshift != 0 && yshift != 0)
         {
            for(y = 0; y < height; y++)
               ShiftVector(matIn[y],matOut[y],1,width,xshift);
            memset(&matIn[0][0],0,width*height*sizeof(complex));
            for(x = 0; x < width; x++)
               ShiftVector(&matOut[0][x],&matIn[0][x],width,height,yshift);
            memcpy(&matOut[0][0],&matIn[0][0],width*height*sizeof(complex));
         }
         else
            memcpy(&matOut[0][0],&matIn[0][0],width*height*sizeof(complex));
      }
   }
   else if(var.GetType() == MATRIX3D)
   {
      long xn,yn,zn;
   // Get the input and output matrix
      long width  = VarWidth(&var);
      long height = VarHeight(&var);
      long depth  = VarDepth(&var);
      float ***matIn = VarReal3DMatrix(&var);
      ans->MakeAndLoadMatrix3D(NULL,width,height,depth);
      float ***matOut = VarReal3DMatrix(ans);

   // Check for valid matrix
      if(width <= 0 || height <= 0 || depth <= 0)
      {
         ErrorMessage("invalid matrix dimensions");
         return(ERR);
      }

   // Shift the data
      for(z = 0; z < depth; z++)
      {
         zn = z + zshift;
         for(y = 0; y < height; y++)
         {
            yn = y + yshift;
            for(x = 0; x < width; x++)
            {
               xn = x + xshift;
               if(xn >= 0 && yn >= 0 && zn >= 0 &&
                  xn < width && yn < height && zn < depth)
               {
                  matOut[zn][yn][xn] = matIn[z][y][x];
               }
            }
         }
      }
   }
   else if(var.GetType() == CMATRIX3D)
   {
      long xn,yn,zn;
   // Get the input and output matrix
      long width  = VarWidth(&var);
      long height = VarHeight(&var);
      long depth  = VarDepth(&var);
      complex ***matIn = VarComplex3DMatrix(&var);
      ans->MakeAndLoadCMatrix3D(NULL,width,height,depth);
      complex ***matOut = VarComplex3DMatrix(ans);

   // Check for valid matrix
      if(width <= 0 || height <= 0 || depth <= 0)
      {
         ErrorMessage("invalid matrix dimensions");
         return(ERR);
      }

   // Shift the data
      for(z = 0; z < depth; z++)
      {
         zn = z + zshift;
         for(y = 0; y < height; y++)
         {
            yn = y + yshift;
            for(x = 0; x < width; x++)
            {
               xn = x + xshift;
               if(xn >= 0 && yn >= 0 && zn >= 0 &&
                  xn < width && yn < height && zn < depth)
               {
                  matOut[zn][yn][xn] = matIn[z][y][x];
               }
            }
         }
      }
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   itfc->nrRetValues = 1;
   return(OK);
}
  
// Shift contents of vIn by shift points. vIn should be zeroed first

void ShiftVector(float *vIn, float *vOut, long step, long N, long shift)
{
   long i,j;

   if(shift >= 0)
   {
      for(i = 0; i < N; i++)
      {
         j = (i+shift);
         if(j >= 0 && j < N)
            vOut[j*step] = vIn[i*step];
      }
   }
   else
   {
      for(i = 0; i < N; i++)
      {
         j = (i-shift);
         if(j >= 0 && j < N)
            vOut[i*step] = vIn[j*step];
      }
   }
}

void ShiftVector(complex *vIn, complex *vOut, long step, long N, long shift)
{
   long i,j;

   if(shift >= 0)
   {
      for(i = 0; i < N; i++)
      {
         j = (i+shift);
         if(j >= 0 && j < N)
            vOut[j*step] = vIn[i*step];
      }
   }
   else
   {
      for(i = 0; i < N; i++)
      {
         j = (i-shift);
         if(j >= 0 && j < N)
            vOut[i*step] = vIn[j*step];
      }
   }
}

/************************************************************************************************
                       Reflect a matrix about its horizonal or vertical centre  

    Syntax:   result = reflect(m,[axis])

************************************************************************************************/

int ReflectMatrix(Interface* itfc, char args[])
{
   Variable var;
   char direction[MAX_STR] = "horiz";
   long x,y;
   short r;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract matrix  ************************   
   if((r = ArgScan(itfc,args,1,"matrix, [horiz/vert]","ee","vs",&var,direction)) < 0)
      return(r);

   if(var.GetType() == MATRIX2D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      float **mat = VarRealMatrix(&var);
      float temp;

   // Row matrix - so reflect horizontally
      if(width > 1 && height == 1)
      {
         for(x = 0; x < width/2; x++)
         {
            temp = mat[0][x];
            mat[0][x] = mat[0][width-x-1];
            mat[0][width-x-1] = temp;
         }
      }

   // Column matrix - so reflect vertically
      else if(width == 1 && height > 1)
      {
         for(y = 0; y < height/2; y++)
         {
            temp = mat[y][0];
            mat[y][0] = mat[height-y-1][0];
            mat[height-y-1][0] = temp;
         }
      }

   // General matrix to reflect vertical axis
      else if(!strncmp(direction,"vertical",4))
      {
         for(y = 0; y < height; y++)
         {
            for(x = 0; x < width/2; x++)
            {
               temp = mat[y][x];
               mat[y][x] = mat[y][width-x-1];
               mat[y][width-x-1] = temp;
            }
         }
      }

   // General matrix to reflect horizontal axis
      else if(!strncmp(direction,"horizontal",4))
      {
         for(y = 0; y < height/2; y++)
         {
            for(x = 0; x < width; x++)
            {
               temp = mat[y][x];
               mat[y][x] = mat[height-y-1][x];
               mat[height-y-1][x] = temp;
            }
         }
      }
      ans->MakeAndLoadMatrix2D(mat,width,height);
      itfc->nrRetValues = 1;
   }
   else if(var.GetType() == CMATRIX2D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      complex **mat = VarComplexMatrix(&var);
      complex temp;

   // Row matrix - so reflect horizontally
      if(width > 1 && height == 1)
      {
         for(x = 0; x < width/2; x++)
         {
            temp = mat[0][x];
            mat[0][x] = mat[0][width-x-1];
            mat[0][width-x-1] = temp;
         }
      }

   // Column matrix - so reflect vertically
      else if(width == 1 && height > 1)
      {
         for(y = 0; y < height/2; y++)
         {
            temp = mat[y][0];
            mat[y][0] = mat[height-y-1][0];
            mat[height-y-1][0] = temp;
         }
      }

   // General matrix to reflect vertical axis
      else if(!strncmp(direction,"vertical",4))
      {
         for(y = 0; y < height; y++)
         {
            for(x = 0; x < width/2; x++)
            {
               temp = mat[y][x];
               mat[y][x] = mat[y][width-x-1];
               mat[y][width-x-1] = temp;
            }
         }
      }

   // General matrix to reflect about horizontal axis
      else if(!strncmp(direction,"horizontal",4))
      {
         for(y = 0; y < height/2; y++)
         {
            for(x = 0; x < width; x++)
            {
               temp = mat[y][x];
               mat[y][x] = mat[height-y-1][x];
               mat[height-y-1][x] = temp;
            }
         }
      }
      ans->MakeAndLoadCMatrix2D(mat,width,height);
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   itfc->nrRetValues = 1;
   return(OK);
}

/************************************************************************************************
 Change the dimension of a 1D, 2D or 3D matrix   

Syntax:   mOut = reshape(mIn,new_width,new_height,[new_depth])

************************************************************************************************/

int ReshapeMatrix(Interface* itfc, char args[])
{
   Variable var;
   long x,y,z,q;
   long newWidth, newHeight=1, newDepth=1, newHyper=1;
   short r;

// Extract matrix and new size parameters ********* 
   if((r = ArgScan(itfc,args,1,"matrix, new_width, new_height, new_depth, new_hyper","eeeee","vllll",&var, &newWidth, &newHeight, &newDepth, &newHyper)) < 0)
      return(r);


   if(newWidth == 0 || newHeight == 0 || newDepth == 0 || newHyper == 0)
   {
      ErrorMessage("invalid new dimension(s)");
      return(ERR);
   }

// Process real 1D or 2D matrix resizing *******************
   if(var.GetType() == MATRIX2D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      float *mat = &(VarRealMatrix(&var)[0][0]);

      if((newWidth > 1 || newHeight > 1) && newDepth == 1 && newHyper == 1) // Convert to 2D
      {
         float **newMat;

      // Check for valid resizing
         if(newWidth*newHeight != width*height)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix2D(newWidth,newHeight)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(y = 0; y < newHeight; y++)
            for(x = 0; x < newWidth; x++)
               newMat[y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix2D(newMat,newWidth,newHeight);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth > 1 && newHyper == 1) // Convert to 3D
      {
         float ***newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth != width*height)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix3D(newWidth,newHeight,newDepth)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(z = 0; z < newDepth; z++)
            for(y = 0; y < newHeight; y++)
               for(x = 0; x < newWidth; x++)
                  newMat[z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix3D(newMat,newWidth,newHeight,newDepth);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth >= 1 && newHyper > 1) // Convert to 4D
      {
         float ****newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth*newHyper != width*height)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix4D(newWidth,newHeight,newDepth,newHyper)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(q = 0; q < newHyper; q++)
            for(z = 0; z < newDepth; z++)
               for(y = 0; y < newHeight; y++)
                  for(x = 0; x < newWidth; x++)
                     newMat[q][z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix4D(newMat,newWidth,newHeight,newDepth,newHyper);
      }
      else
      {
         ErrorMessage("invalid depth specified");
         return(ERR);
      }
   }

// Process real 3D matrix resizing *******************
   else if(var.GetType() == MATRIX3D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      long depth = VarDepth(&var);
      float *mat = &(VarReal3DMatrix(&var)[0][0][0]);

      if((newWidth > 1 || newHeight > 1) && newDepth == 1 && newHyper == 1) // Convert to 2D
      {
         float **newMat;

      // Check for valid resizing
         if(newWidth*newHeight != width*height*depth)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix2D(newWidth,newHeight)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(y = 0; y < newHeight; y++)
            for(x = 0; x < newWidth; x++)
               newMat[y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix2D(newMat,newWidth,newHeight);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth > 1 && newHyper == 1) // Convert to 3D
      {
         float ***newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth != width*height*depth)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix3D(newWidth,newHeight,newDepth)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(z = 0; z < newDepth; z++)
            for(y = 0; y < newHeight; y++)
               for(x = 0; x < newWidth; x++)
                  newMat[z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix3D(newMat,newWidth,newHeight,newDepth);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth >= 1 && newHyper > 1) // Convert to 4D
      {
         float ****newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth*newHyper != width*height*depth)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix4D(newWidth,newHeight,newDepth,newHyper)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	 
         for(q = 0; q < newHyper; q++)
            for(z = 0; z < newDepth; z++)
               for(y = 0; y < newHeight; y++)
                  for(x = 0; x < newWidth; x++)
                     newMat[q][z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix4D(newMat,newWidth,newHeight,newDepth,newHyper);
      }

      else
      {
         ErrorMessage("invalid depth specified");
         return(ERR);
      }
   }
// Process complex 1D or 2D matrix resizing *******************
   else if(var.GetType() == CMATRIX2D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      complex *mat = &(VarComplexMatrix(&var)[0][0]);

      if((newWidth > 1 || newHeight > 1) && newDepth == 1 && newHyper == 1) // Convert to 2D
      {
         complex **newMat;

      // Check for valid resizing
         if(newWidth*newHeight != width*height)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix2D(newWidth,newHeight)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(y = 0; y < newHeight; y++)
            for(x = 0; x < newWidth; x++)
               newMat[y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix2D(newMat,newWidth,newHeight);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth > 1 && newHyper == 1) // Convert to 3D
      {
         complex ***newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth != width*height)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix3D(newWidth,newHeight,newDepth)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(z = 0; z < newDepth; z++)
            for(y = 0; y < newHeight; y++)
               for(x = 0; x < newWidth; x++)
                  newMat[z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix3D(newMat,newWidth,newHeight,newDepth);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth >= 1 && newHyper > 1) // Convert to 4D
      {
         complex ****newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth*newHyper != width*height)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix4D(newWidth,newHeight,newDepth,newHyper)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(q = 0; q < newHyper; q++)
            for(z = 0; z < newDepth; z++)
               for(y = 0; y < newHeight; y++)
                  for(x = 0; x < newWidth; x++)
                     newMat[q][z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix4D(newMat,newWidth,newHeight,newDepth,newHyper);
      }
      else
      {
         ErrorMessage("invalid depth specified");
         return(ERR);
      }
   }

// Process complex 3D matrix resizing *******************
   else if(var.GetType() == CMATRIX3D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      long depth = VarDepth(&var);
      complex *mat = &(VarComplex3DMatrix(&var)[0][0][0]);

      if((newWidth > 1 || newHeight > 1) && newDepth == 1 && newHyper == 1) // Convert to 2D
      {
         complex **newMat;

      // Check for valid resizing
         if(newWidth*newHeight != width*height*depth)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix2D(newWidth,newHeight)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(y = 0; y < newHeight; y++)
            for(x = 0; x < newWidth; x++)
               newMat[y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix2D(newMat,newWidth,newHeight);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth > 1 && newHyper == 1) // Convert to 3D
      {
         complex ***newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth != width*height*depth)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix3D(newWidth,newHeight,newDepth)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(z = 0; z < newDepth; z++)
            for(y = 0; y < newHeight; y++)
               for(x = 0; x < newWidth; x++)
                  newMat[z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix3D(newMat,newWidth,newHeight,newDepth);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth >= 1 && newHyper > 1) // Convert to 4D
      {
         complex ****newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth*newHyper != width*height*depth)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix4D(newWidth,newHeight,newDepth,newHyper)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(q = 0; q < newHyper; q++)
            for(z = 0; z < newDepth; z++)
               for(y = 0; y < newHeight; y++)
                  for(x = 0; x < newWidth; x++)
                     newMat[q][z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix4D(newMat,newWidth,newHeight,newDepth,newHyper);
      }
      else
      {
         ErrorMessage("invalid depth specified");
         return(ERR);
      }
   }
// Process real 4D matrix resizing *******************
   else if(var.GetType() == MATRIX4D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      long depth = VarDepth(&var);
      long hyper = VarHyper(&var);
      float *mat = &(VarReal4DMatrix(&var)[0][0][0][0]);

      if((newWidth > 1 || newHeight > 1) && newDepth == 1 && newHyper == 1) // Convert to 2D
      {
         float **newMat;

      // Check for valid resizing
         if(newWidth*newHeight != width*height*depth*hyper)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix2D(newWidth,newHeight)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(y = 0; y < newHeight; y++)
            for(x = 0; x < newWidth; x++)
               newMat[y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix2D(newMat,newWidth,newHeight);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth > 1 && newHyper == 1) // Convert to 3D
      {
         float ***newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth != width*height*depth*hyper)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix3D(newWidth,newHeight,newDepth)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(z = 0; z < newDepth; z++)
            for(y = 0; y < newHeight; y++)
               for(x = 0; x < newWidth; x++)
                  newMat[z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix3D(newMat,newWidth,newHeight,newDepth);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth >= 1 && newHyper > 1) // Convert to 4D
      {
         float ****newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth*newHyper != width*height*depth*hyper)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeMatrix4D(newWidth,newHeight,newDepth,newHyper)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(q = 0; q < newHyper; q++)
            for(z = 0; z < newDepth; z++)
               for(y = 0; y < newHeight; y++)
                  for(x = 0; x < newWidth; x++)
                     newMat[q][z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignMatrix4D(newMat,newWidth,newHeight,newDepth,newHyper);
      }
      else
      {
         ErrorMessage("invalid hyperdepth specified");
         return(ERR);
      }
   }
// Process complex 4D matrix resizing *******************
   else if(var.GetType() == CMATRIX4D)
   {
      long width = VarWidth(&var);
      long height = VarHeight(&var);
      long depth = VarDepth(&var);
      long hyper = VarHyper(&var);
      complex *mat = &(VarComplex4DMatrix(&var)[0][0][0][0]);

      if((newWidth > 1 || newHeight > 1) && newDepth == 1 && newHyper == 1) // Keep as 2D
      {
         complex **newMat;

      // Check for valid resizing
         if(newWidth*newHeight != width*height*depth*hyper)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix2D(newWidth,newHeight)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(y = 0; y < newHeight; y++)
            for(x = 0; x < newWidth; x++)
               newMat[y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix2D(newMat,newWidth,newHeight);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth > 1 && newHyper == 1) // Convert to 3D
      {
         complex ***newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth != width*height*depth*hyper)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix3D(newWidth,newHeight,newDepth)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(z = 0; z < newDepth; z++)
            for(y = 0; y < newHeight; y++)
               for(x = 0; x < newWidth; x++)
                  newMat[z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix3D(newMat,newWidth,newHeight,newDepth);
      }
      else if(newWidth >= 1 && newHeight >= 1 && newDepth >= 1 && newHyper > 1) // Convert to 4D
      {
         complex ****newMat;

      // Check for valid resizing
         if(newWidth*newHeight*newDepth*newHyper != width*height*depth*hyper)
         {
            ErrorMessage("number of elements in matrix must remain the same");
            return(ERR);
         }

      // Allocate memory for resized matrix	    
         if(!(newMat = MakeCMatrix4D(newWidth,newHeight,newDepth,newHyper)))
         {
            ErrorMessage("unable to allocate memory for reshaped matrix");
            return(ERR);
         }

      // Do the resizing	    
         for(q = 0; q < newHyper; q++)
            for(z = 0; z < newDepth; z++)
               for(y = 0; y < newHeight; y++)
                  for(x = 0; x < newWidth; x++)
                     newMat[q][z][y][x] = (*(mat++));

      // Update answer variable
         itfc->retVar[1].AssignCMatrix4D(newMat,newWidth,newHeight,newDepth,newHyper);
      }
      else
      {
         ErrorMessage("invalid hyperdepth specified");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }

   itfc->nrRetValues = 1;

   return(OK);
}

/************************************************************************************************
Insert matrix mB into mA at location (x0,y0) and place the result in mOut   

Syntax:   mOut = insert(mA,x0,y0,mB)

************************************************************************************************/

int InsertIntoMatrix(Interface* itfc, char args[])
{
   Variable varA,varB;
   long x,y,z;
   long x0 = 0;
   long y0 = 0;
   long z0 = 0;
   short r;

   Variable* ans = &itfc->retVar[1];

// Extract matrix and new size parameters ********* 
   if((r = ArgScan(itfc,args,3,"mA, mB, x0[, y0[, z0]]","eeeee","vvlll",&varA, &varB, &x0, &y0, &z0)) < 0)
      return(r);

// Process real matrix insertion *******************
   if(varA.GetType() == MATRIX2D && varB.GetType() == MATRIX2D)
   {
      long widthA = VarWidth(&varA);
      long heightA = VarHeight(&varA);
      float **matA = VarRealMatrix(&varA);
      long widthB = VarWidth(&varB);
      long heightB = VarHeight(&varB);
      float **matB = VarRealMatrix(&varB);
      float **mOut;

      // Check for valid resizing
      if(x0+widthB > widthA || x0 < 0 || y0+heightB > heightA || y0 < 0)
      {
         ErrorMessage("invalid insertion location or dimensions");
         return(ERR);
      }

      // Do the insertion	    
      for(y = y0; y < y0+heightB; y++)
         for(x = x0; x < x0+widthB; x++)
            matA[y][x] = matB[y-y0][x-x0];

      // Allocate memory for output matrix	    
      if(!(mOut = MakeMatrix2D(widthA,heightA)))
      {
         ErrorMessage("unable to allocate memory for reshaped matrix");
         return(ERR);
      }

      // Copy the results to the output matrix
      memcpy(&(mOut[0][0]),&(matA[0][0]),widthA*heightA*sizeof(float));

      // Update answer variable
      ans->AssignMatrix2D(mOut,widthA,heightA);
   }
// Process complex matrix insertion *******************
   else if(varA.GetType() == CMATRIX2D && varB.GetType() == CMATRIX2D)
   {
      long widthA = VarWidth(&varA);
      long heightA = VarHeight(&varA);
      complex **matA = VarComplexMatrix(&varA);
      long widthB = VarWidth(&varB);
      long heightB = VarHeight(&varB);
      complex **matB = VarComplexMatrix(&varB);
      complex **mOut;

      // Check for valid resizing
      if(x0+widthB > widthA || x0 < 0 || y0+heightB > heightA || y0 < 0)
      {
         ErrorMessage("invalid insertion location or dimensions");
         return(ERR);
      }

      // Do the insertion	    
      for(y = y0; y < y0+heightB; y++)
         for(x = x0; x < x0+widthB; x++)
            matA[y][x] = matB[y-y0][x-x0];

      // Allocate memory for output matrix	    
      if(!(mOut = MakeCMatrix2D(widthA,heightA)))
      {
         ErrorMessage("unable to allocate memory for reshaped matrix");
         return(ERR);
      }

      // Copy the results to the output matrix
      memcpy(&(mOut[0][0]),&(matA[0][0]),widthA*heightA*sizeof(complex));

      // Update answer variable
      ans->AssignCMatrix2D(mOut,widthA,heightA);
   }
// Process real 3D matrix insertion *******************
   else if(varA.GetType() == MATRIX3D && varB.GetType() == MATRIX3D)
   {
      long widthA   = VarWidth(&varA);
      long heightA  = VarHeight(&varA);
      long depthA   = VarDepth(&varA);
      float ***matA = VarReal3DMatrix(&varA);
      long widthB   = VarWidth(&varB);
      long heightB  = VarHeight(&varB);
      long depthB   = VarDepth(&varB);
      float ***matB = VarReal3DMatrix(&varB);
      float ***mOut;

      // Check for valid resizing
      if(x0+widthB  > widthA   || x0 < 0 || 
         y0+heightB > heightA  || y0 < 0 ||
         z0+depthB  > depthA   || z0 < 0)
      {
         ErrorMessage("invalid insertion location or dimensions");
         return(ERR);
      }

      // Do the insertion	    
      for(z = z0; z < z0+depthB; z++)
         for(y = y0; y < y0+heightB; y++)
            for(x = x0; x < x0+widthB; x++)
               matA[z][y][x] = matB[z-z0][y-y0][x-x0];

      // Allocate memory for output matrix	    
      if(!(mOut = MakeMatrix3D(widthA,heightA,depthA)))
      {
         ErrorMessage("unable to allocate memory for reshaped matrix");
         return(ERR);
      }

      // Copy the results to the output matrix
      memcpy(&(mOut[0][0][0]),&(matA[0][0][0]),widthA*heightA*depthA*sizeof(float));

      // Update answer variable
      ans->AssignMatrix3D(mOut,widthA,heightA,depthA);
   }
// Process real 3D matrix insertion *******************
   else if(varA.GetType() == CMATRIX3D && varB.GetType() == CMATRIX3D)
   {
      long widthA   = VarWidth(&varA);
      long heightA  = VarHeight(&varA);
      long depthA   = VarDepth(&varA);
      complex ***matA = VarComplex3DMatrix(&varA);
      long widthB   = VarWidth(&varB);
      long heightB  = VarHeight(&varB);
      long depthB   = VarDepth(&varB);
      complex ***matB = VarComplex3DMatrix(&varB);
      complex ***mOut;

      // Check for valid resizing
      if(x0+widthB  > widthA   || x0 < 0 || 
         y0+heightB > heightA || y0 < 0 ||
         z0+depthB  > depthA  || z0 < 0)
      {
         ErrorMessage("invalid insertion location or dimensions");
         return(ERR);
      }

      // Do the insertion	    
      for(z = z0; z < z0+depthB; z++)
         for(y = y0; y < y0+heightB; y++)
            for(x = x0; x < x0+widthB; x++)
               matA[z][y][x] = matB[z-z0][y-y0][x-x0];

      // Allocate memory for output matrix	    
      if(!(mOut = MakeCMatrix3D(widthA,heightA,depthA)))
      {
         ErrorMessage("unable to allocate memory for reshaped matrix");
         return(ERR);
      }

      // Copy the results to the output matrix
      memcpy(&(mOut[0][0][0]),&(matA[0][0][0]),widthA*heightA*depthA*sizeof(complex));

      // Update answer variable
      ans->AssignCMatrix3D(mOut,widthA,heightA,depthA);
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   itfc->nrRetValues = 1;
   return(OK);
}



/************************************************************************************************
                               Join 2 2D matrices together  

    Syntax:   result = join(m1,m2,[edge])

************************************************************************************************/

int JoinMatrices(Interface* itfc, char arg[])
{
   Variable var1,var2;
   long w1,w2,h1,h2,y;
   CText edge = "vert";
   short r;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract parameters ************************   
   if((r = ArgScan(itfc,arg,2,"obj1,obj2,[edge]","eee","vvt",&var1,&var2,&edge)) < 0)
      return(r);

   if(var1.GetType() == NULL_VARIABLE && var2.GetType() == MATRIX2D)
   {
	   float **m2 = VarRealMatrix(&var2);
	   w2 = VarWidth(&var2);
      h2 = VarHeight(&var2);
		float** mat = MakeMatrix2D(w2,h2);
	   long size = sizeof(float);
      for(y = 0; y < h2; y++)
          memcpy(mat[y],m2[y],w2*size);
      ans->AssignMatrix2D(mat,w2,h2);
	}
   else if(var1.GetType() == MATRIX2D && var2.GetType() == NULL_VARIABLE)
   {
	   float **m1 = VarRealMatrix(&var1);
	   w1 = VarWidth(&var1);
      h1 = VarHeight(&var1);
		float** mat = MakeMatrix2D(w1,h1);
	   long size = sizeof(float);
      for(y = 0; y < h1; y++)
          memcpy(mat[y],m1[y],w1*size);
      ans->AssignMatrix2D(mat,w1,h1);
	}
   else if(var1.GetType() == MATRIX2D && var2.GetType() == MATRIX2D)
   {
      float **mat;
      w1 = VarWidth(&var1);
      h1 = VarHeight(&var1);
      w2 = VarWidth(&var2);
      h2 = VarHeight(&var2);
      float **m1 = VarRealMatrix(&var1);
      float **m2 = VarRealMatrix(&var2);
      long size = sizeof(float);

   // Joining matrices with same height but different widths
      if(w1!=w2 && h1==h2)
      {
         mat = MakeMatrix2D(w1+w2,h1);
         for(y = 0; y < h1; y++)
            memcpy(mat[y],m1[y],w1*size);
         for(y = 0; y < h1; y++)
            memcpy(mat[y]+w1,m2[y],w2*size);
         ans->AssignMatrix2D(mat,w1+w2,h1);
      }
   // Joining matrices with same width but different heights
      else if(w1==w2 && h1!=h2)
      {
         mat = MakeMatrix2D(w1,h1+h2);
         memcpy(&mat[0][0],&m1[0][0],w1*h1*size);
         memcpy(&mat[0][0]+w1*h1,&m2[0][0],w2*h2*size);
         ans->AssignMatrix2D(mat,w1,h1+h2);
      }
   // Joining matrices with same width and same height
      else if(w1 == w1 && h1 == h2)
      {
         if(edge == "vert")
         {
            mat = MakeMatrix2D(w1+w2,h1);
            for(y = 0; y < h1; y++)
               memcpy(mat[y],m1[y],w1*size);
            for(y = 0; y < h2; y++)
               memcpy(mat[y]+w1,m2[y],w2*size);
            ans->AssignMatrix2D(mat,w1+w2,h1);
         }
         else if(edge == "horiz")
         {
            mat = MakeMatrix2D(w1,h1+h2);
            memcpy(&mat[0][0],&m1[0][0],w1*h1*size);
            memcpy(&mat[0][0]+w1*h1,&m2[0][0],w2*h2*size);
            ans->AssignMatrix2D(mat,w1,h1+h2);
         }
         else
         {
            ErrorMessage("invalid 'edge'");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("matrix dimensions are not compatible for joining");
         return(ERR);
      }
   }

   else if(var1.GetType() == CMATRIX2D && var2.GetType() == CMATRIX2D)
   {
      complex **mat;
      w1 = VarWidth(&var1);
      h1 = VarHeight(&var1);
      w2 = VarWidth(&var2);
      h2 = VarHeight(&var2);
      complex **m1 = VarComplexMatrix(&var1);
      complex **m2 = VarComplexMatrix(&var2);
      long size = sizeof(complex);

   // Joining matrices with same height but different widths
      if(w1!=w2 && h1==h2)
      {
         mat = MakeCMatrix2D(w1+w2,h1);
         for(y = 0; y < h1; y++)
            memcpy(mat[y],m1[y],w1*size);
         for(y = 0; y < h1; y++)
            memcpy(mat[y]+w1,m2[y],w2*size);
         ans->AssignCMatrix2D(mat,w1+w2,h1);
      }
   // Joining matricies with same width but different heights
      else if(w1==w2 && h1!=h2)
      {
         mat = MakeCMatrix2D(w1,h1+h2);
         memcpy(&mat[0][0],&m1[0][0],w1*h1*size);
         memcpy(&mat[0][0]+w1*h1,&m2[0][0],w2*h2*size);
         ans->AssignCMatrix2D(mat,w1,h1+h2);
      }
   // Joining matricies with same width and same height
      else if(w1==w2 && h1==h2)
      {
         if(edge == "vert")
         {
            mat = MakeCMatrix2D(w1+w2,h1);
            for(y = 0; y < h1; y++)
               memcpy(mat[y],m1[y],w1*size);
            for(y = 0; y < h2; y++)
               memcpy(mat[y]+w1,m2[y],w2*size);
            ans->AssignCMatrix2D(mat,w1+w2,h1);
         }
         else if( edge == "horiz")
         {
            mat = MakeCMatrix2D(w1,h1+h2);
            memcpy(&mat[0][0],&m1[0][0],w1*h1*size);
            memcpy(&mat[0][0]+w1*h1,&m2[0][0],w2*h2*size);
            ans->AssignCMatrix2D(mat,w1,h1+h2);
         }
         else
         {
            ErrorMessage("invalid 'edge'");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("matrix dimensions are not compatible for joining");
         return(ERR);
      }
   }

   else if(var1.GetType() == DMATRIX2D && var2.GetType() == DMATRIX2D)
   {
      double **mat;
      w1 = VarWidth(&var1);
      h1 = VarHeight(&var1);
      w2 = VarWidth(&var2);
      h2 = VarHeight(&var2);
      double **m1 = VarDoubleMatrix(&var1);
      double **m2 = VarDoubleMatrix(&var2);
      long size = sizeof(double);

   // Joining matrices with same height but different widths
      if(w1!=w2 && h1==h2)
      {
         mat = MakeDMatrix2D(w1+w2,h1);
         for(y = 0; y < h1; y++)
            memcpy(mat[y],m1[y],w1*size);
         for(y = 0; y < h1; y++)
            memcpy(mat[y]+w1,m2[y],w2*size);
         ans->AssignDMatrix2D(mat,w1+w2,h1);
      }
   // Joining matrices with same width but different heights
      else if(w1==w2 && h1!=h2)
      {
         mat = MakeDMatrix2D(w1,h1+h2);
         memcpy(&mat[0][0],&m1[0][0],w1*h1*size);
         memcpy(&mat[0][0]+w1*h1,&m2[0][0],w2*h2*size);
         ans->AssignDMatrix2D(mat,w1,h1+h2);
      }
   // Joining matrices with same width and same height
      else if(w1 == w1 && h1 == h2)
      {
         if(edge == "vert")
         {
            mat = MakeDMatrix2D(w1+w2,h1);
            for(y = 0; y < h1; y++)
               memcpy(mat[y],m1[y],w1*size);
            for(y = 0; y < h2; y++)
               memcpy(mat[y]+w1,m2[y],w2*size);
            ans->AssignDMatrix2D(mat,w1+w2,h1);
         }
         else if(edge == "horiz")
         {
            mat = MakeDMatrix2D(w1,h1+h2);
            memcpy(&mat[0][0],&m1[0][0],w1*h1*size);
            memcpy(&mat[0][0]+w1*h1,&m2[0][0],w2*h2*size);
            ans->AssignDMatrix2D(mat,w1,h1+h2);
         }
         else
         {
            ErrorMessage("invalid 'edge'");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("matrix dimensions are not compatible for joining");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("invalid type");
      return(ERR);
   }
   itfc->nrRetValues = 1;
   return(OK);
}

// Used by nnls1/2d
int Difference(Interface* itfc, char arg[])
{
   Variable var;
   short r;
   float *diff;
   
   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract parameter ************************   
   if((r = ArgScan(itfc,arg,1,"vector","e","v",&var)) < 0)
      return(r); 

// Check for invalid variable type ************
   if(VarType(&var) != MATRIX2D || VarHeight(&var) != 1)
   {
      ErrorMessage("data must be a real 1D matrix");
      return(ERR);
   }

// Get the matrix dimensions ******************
   float *v = VarRealMatrix(&var)[0];
   long w = VarWidth(&var);

// Allocate space for the difference-vector ***
   ans->MakeAndLoadMatrix2D(NULL,w-1,1);
   diff = ans->GetMatrix2D()[0];

// Calculate the difference
   for(long i = 0; i < w-1; i++)
   {
      diff[i] = v[i+1] - v[i];
   }

   itfc->nrRetValues = 1;

   return(OK);
}


/************************************************************************************************
  Extract a subvector from an input vector by taking every nth entry starting from entry 'offset'
************************************************************************************************/

int DecimateVector(Interface* itfc, char arg[])
{
   long offset,n;
   short r;
   Variable var;
   long w2;
   long i,j;

   Variable* ans = &itfc->retVar[1];

// Extract parameters ************************   
   if((r = ArgScan(itfc,arg,3,"vector,offset,step","eee","vll",&var,&offset,&n)) < 0)
      return(r); 

   if(var.GetDimY() != 1)
   {
      ErrorMessage("data must be a 1D matrix");
      return(ERR);
   }

// Check for invalid variable type ************
   if(var.GetType() == MATRIX2D)
   {
	// Get the matrix dimensions ******************
	   float* v = var.GetMatrix2D()[0];
	   long w = var.GetDimX();

	// Check for invalid parameters ***************
	   if(offset < 0 || offset > w)
	   {
		  ErrorMessage("invalid offset");
		  return(ERR);
	   }  
	   if(n <= 0 || n > w)
	   {
		  ErrorMessage("invalid step size");
		  return(ERR);
	   }       

	// Determine the number of value in the sub-vector ******
	   for(w2 = 0, i = offset; i < w; i+=n, w2++){;}

	// Allocate space for the sub-vector ********************
	   float **mOut = MakeMatrix2D(w2,1);
 
	// Extract sub-vector and place in ans ***************
	   for(j = 0, i = offset; i < w; i+=n, j++)
	   {
		  mOut[0][j] = v[i];
	   }
       itfc->retVar[1].AssignMatrix2D(mOut,w2,1);
   }
   else if(var.GetType() == CMATRIX2D)
   {
	// Get the matrix dimensions ******************
	   complex* v = var.GetCMatrix2D()[0];
	   long w = var.GetDimX();

	// Check for invalid parameters ***************
	   if(offset < 0 || offset > w)
	   {
		  ErrorMessage("invalid offset");
		  return(ERR);
	   }  
	   if(n <= 0 || n > w)
	   {
		  ErrorMessage("invalid step size");
		  return(ERR);
	   }       

	// Determine the number of value in the sub-vector ******
	   for(w2 = 0, i = offset; i < w; i+=n, w2++){;}

	// Allocate space for the sub-vector ********************
	   complex **mOut = MakeCMatrix2D(w2,1);
 
	// Extract sub-vector and place in ans ***************
	   for(j = 0, i = offset; i < w; i+=n, j++)
	   {
		  mOut[0][j].r = v[i].r;
		  mOut[0][j].i = v[i].i;
	   }
	   itfc->retVar[1].AssignCMatrix2D(mOut,w2,1);
   }
   else
   {
      ErrorMessage("data must be a real or complex 1D matrix");
      return(ERR);
   }
   itfc->nrRetValues = 1;            
   return(0);
}

/*************************************************************************************************
     Generate a matrix which contains identical rows or columns. The input vector is "cloned" so
     that it fills the output matrix. 

     Syntax: m = clonearray(v, w, h)

     m ... output matrix - dimensions w by h (real or complex)
     v ... array or vector to clone may be a row or column matrix (real or complex)
     w ... width of output matrix
     h ... height of output matrix

*************************************************************************************************/

int CloneArray(Interface* itfc, char arg[])
{
   short r;
   long widthOut,heightOut;
   Variable vec;
   long x,y;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

   if((r = ArgScan(itfc,arg,3,"vector, matrix-width, matrix-height","eee","vll",&vec,&widthOut,&heightOut)) < 0)
      return(r); 

   if(VarType(&vec) == MATRIX2D)
   {
      long widthIn = VarWidth(&vec);
      long heightIn = VarHeight(&vec);

      ans->MakeAndLoadMatrix2D(NULL,widthOut,heightOut);

      if(heightIn == 1 && widthIn == widthOut)
      {
	      for(y = 0; y < heightOut; y++)
	         for(x = 0; x < widthOut; x++)
	            ans->GetMatrix2D()[y][x] = VarRealMatrix(&vec)[0][x];
      }
      else if(heightIn == heightOut && widthIn == 1)
      {
	      for(y = 0; y < heightOut; y++)
	         for(x = 0; x < widthOut; x++)
	            ans->GetMatrix2D()[y][x] = VarRealMatrix(&vec)[y][0];
      }
      else
      {
         ErrorMessage("width or height of input vector does not match output matrix");
         return(ERR);
      }
   }
   else if(VarType(&vec) == CMATRIX2D)
   {
      long widthIn = VarWidth(&vec);
      long heightIn = VarHeight(&vec);

      ans->MakeAndLoadCMatrix2D(NULL,widthOut,heightOut);

      if(heightIn == 1 && widthIn == widthOut)
      {
	      for(y = 0; y < heightOut; y++)
	         for(x = 0; x < widthOut; x++)
	            VarComplexMatrix(ans)[y][x] = VarComplexMatrix(&vec)[0][x];
      }
      else if(heightIn == heightOut && widthIn == 1)
      {
	      for(y = 0; y < heightOut; y++)
	         for(x = 0; x < widthOut; x++)
	            VarComplexMatrix(ans)[y][x] = VarComplexMatrix(&vec)[y][0];
      }
      else
      {
         ErrorMessage("width or height of input vector does not match output matrix");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("1st argument should be a vector");
      return(ERR);
   }
   itfc->nrRetValues = 1;
   return(OK);
}

 
/***************************************************************
   Generate a linear row vector which ranges from 'first' to 
   'last' with N data points.
***************************************************************/

int LinearVector(Interface* itfc, char arg[])
{
   short r;
   long N,i;
   Variable firstVar,lastVar;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

   if((r = ArgScan(itfc,arg,3,"first, last, number","eee","vvl",&firstVar,&lastVar,&N)) < 0)
      return(r); 

   if(N <= 0)
   {
      ErrorMessage("number of data points should be positive");
      return(ERR);
   }

   if(firstVar.GetType() == FLOAT32 && lastVar.GetType() == FLOAT32)
   {
      float first = firstVar.GetReal();
      float last = lastVar.GetReal();
      ans->MakeAndLoadMatrix2D(NULL,N,1L);
      float **m = ans->GetMatrix2D();

      for(i = 0; i < N; i++)
      {
         ans->GetMatrix2D()[0][i] = first + i*(last-first)/(N-1);
      }
   }
   else if(firstVar.GetType() == FLOAT32 && lastVar.GetType() == FLOAT64)
   {
      double first = (double)firstVar.GetReal();
      double last = lastVar.GetDouble();
      ans->MakeAndLoadDMatrix2D(NULL,N,1L);

      double **m = ans->GetDMatrix2D();

      for(i = 0; i < N; i++)
      {
         m[0][i] = first + i*(last-first)/(N-1);
      }
   }
   else if(firstVar.GetType() == FLOAT64 && lastVar.GetType() == FLOAT32)
   {
      double first = firstVar.GetDouble();
      double last = (double)lastVar.GetReal();
      ans->MakeAndLoadDMatrix2D(NULL,N,1L);

      double **m = ans->GetDMatrix2D();

      for(i = 0; i < N; i++)
      {
         m[0][i] = first + i*(last-first)/(N-1);
      }
   }
   else if(firstVar.GetType() == FLOAT64 && lastVar.GetType() == FLOAT64)
   {
      double first = firstVar.GetDouble();
      double last = lastVar.GetDouble();
      ans->MakeAndLoadDMatrix2D(NULL,N,1L);

      double **m = ans->GetDMatrix2D();

      for(i = 0; i < N; i++)
      {
         m[0][i] = first + i*(last-first)/(N-1);
      }
   }
   itfc->nrRetValues = 1;

   return(OK);
}
 
/***************************************************************
      Generate a complex number from real and imaginary parts
***************************************************************/

int ComplexNumber(Interface* itfc, char arg[])
{
   float real,imag;
   short r;
   complex value;
   
   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

   if((r = ArgScan(itfc,arg,2,"real,imag","ee","ff",&real,&imag)) < 0)
      return(r); 
    
   value.r = real;
   value.i = imag;
     
   ans->MakeAndSetComplex(value);
   itfc->nrRetValues = 1;  
   return(0);
}

/***************************************************************
       Return the real part of a complex quantity
***************************************************************/

int RealPart(Interface* itfc ,char arg[])
{
   Variable var;
   short r,type;
   float result;
   
   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

   if((r = ArgScan(itfc,arg,1,"complex variable","e","v",&var)) < 0)
      return(r); 

   type = var.GetType();

   switch(type)
   {
      case(FLOAT32):
         result = var.GetReal();
         ans->MakeAndSetFloat(result);
         break;
      case(COMPLEX):
         result = var.GetComplex().r;
         ans->MakeAndSetFloat(result);
         break;
      case(MATRIX2D):
      {
         float **mat;
         long xsize = var.GetDimX();
         long ysize = var.GetDimY();

         if(!(mat = MakeMatrix2D(xsize,ysize)))
         {
            ErrorMessage("out of memory");
            return(ERR);
         }   
         for(long j = 0; j < ysize; j++)
         {
            for(long i = 0; i < xsize; i++)
            {
               mat[j][i] = var.GetMatrix2D()[j][i];
            }
         }
         ans->AssignMatrix2D(mat,xsize,ysize);
         break;
      }
      case(CMATRIX2D):
      {
         float **mat;
         long xsize = var.GetDimX();
         long ysize = var.GetDimY();
         
 	      if(!(mat = MakeMatrix2D(xsize,ysize)))
 	      {
            ErrorMessage("out of memory");
            return(ERR);
         }   
         for(long j = 0; j < ysize; j++)
         {
            for(long i = 0; i < xsize; i++)
            {
               mat[j][i] = var.GetCMatrix2D()[j][i].r;
            }
         }
         ans->AssignMatrix2D(mat,xsize,ysize);
         break;
      }
      case(MATRIX3D):
      {
         float ***mat;
         long xsize = var.GetDimX();
         long ysize = var.GetDimY();
         long zsize = var.GetDimZ();

         if(!(mat = MakeMatrix3D(xsize,ysize,zsize)))
         {
            ErrorMessage("out of memory");
            return(ERR);
         }   
         for(long k = 0; k < zsize; k++)
         {
            for(long j = 0; j < ysize; j++)
            {
               for(long i = 0; i < xsize; i++)
               {
                  mat[k][j][i] = var.GetMatrix3D()[k][j][i];
               }
            }
         }
         ans->AssignMatrix3D(mat,xsize,ysize,zsize);
         break;
      }
      case(CMATRIX3D):
      {
         float ***mat;
         long xsize = var.GetDimX();
         long ysize = var.GetDimY();
         long zsize = var.GetDimZ();
         
 	      if(!(mat = MakeMatrix3D(xsize,ysize,zsize)))
 	      {
            ErrorMessage("out of memory");
            return(ERR);
         }   
         for(long k = 0; k < zsize; k++)
         {
            for(long j = 0; j < ysize; j++)
            {
               for(long i = 0; i < xsize; i++)
               {
                  mat[k][j][i] = var.GetCMatrix3D()[k][j][i].r;
               }
            }
         }
         ans->AssignMatrix3D(mat,xsize,ysize,zsize);
         break;
      }
      default:
      {
         ErrorMessage("invalid data type");
         return(ERR);
      } 
   }
   itfc->nrRetValues = 1;         
   return(OK);
}


int ImaginaryPart(Interface* itfc ,char arg[])
{
   Variable var;
   short r,type;
   float result;
   
   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

   if((r = ArgScan(itfc,arg,1,"complex variable","e","v",&var)) < 0)
      return(r); 

   type = var.GetType();

   switch(type)
   {
      case(FLOAT32):
         ans->MakeAndSetFloat(0.0);
         break;
      case(COMPLEX):
         result = var.GetComplex().i;
         ans->MakeAndSetFloat(result);
         break;
      case(MATRIX2D):
      {
         float **mat;
         long xsize = var.GetDimX();
         long ysize = var.GetDimY();

         if(!(mat = MakeMatrix2D(xsize,ysize)))
         {
            ErrorMessage("out of memory");
            return(ERR);
         }   
         for(long j = 0; j < ysize; j++)
         {
            for(long i = 0; i < xsize; i++)
            {
               mat[j][i] = 0.0;
            }
         }
         ans->AssignMatrix2D(mat,xsize,ysize);
         break;
      }
      case(CMATRIX2D):
      {
         float **mat;
         long xsize = var.GetDimX();
         long ysize = var.GetDimY();
         
 	      if(!(mat = MakeMatrix2D(xsize,ysize)))
 	      {
            ErrorMessage("out of memory");
            return(ERR);
         }   
         for(long j = 0; j < ysize; j++)
         {
            for(long i = 0; i < xsize; i++)
            {
               mat[j][i] = var.GetCMatrix2D()[j][i].i;
            }
         }
         ans->AssignMatrix2D(mat,xsize,ysize);
         break;
      }
      case(MATRIX3D):
      {
         float ***mat;
         long xsize = var.GetDimX();
         long ysize = var.GetDimY();
         long zsize = var.GetDimZ();

         if(!(mat = MakeMatrix3D(xsize,ysize,zsize)))
         {
            ErrorMessage("out of memory");
            return(ERR);
         }   
         for(long k = 0; k < zsize; k++)
         {
            for(long j = 0; j < ysize; j++)
            {
               for(long i = 0; i < xsize; i++)
               {
                  mat[k][j][i] = 0.0;
               }
            }
         }
         ans->AssignMatrix3D(mat,xsize,ysize,zsize);
         break;
      }
      case(CMATRIX3D):
      {
         float ***mat;
         long xsize = var.GetDimX();
         long ysize = var.GetDimY();
         long zsize = var.GetDimZ();
         
 	      if(!(mat = MakeMatrix3D(xsize,ysize,zsize)))
 	      {
            ErrorMessage("out of memory");
            return(ERR);
         }   
         for(long k = 0; k < zsize; k++)
         {
            for(long j = 0; j < ysize; j++)
            {
               for(long i = 0; i < xsize; i++)
               {
                  mat[k][j][i] = var.GetCMatrix3D()[k][j][i].i;
               }
            }
         }
         ans->AssignMatrix3D(mat,xsize,ysize,zsize);
         break;
      }
      default:
      {
         ErrorMessage("invalid data type");
         return(ERR);
      } 
   }
   itfc->nrRetValues = 1;         
   return(OK);

}

// Return the sine of the argument

int Round(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,roundBase,roundBase));
}

// Return the square root of the argument

int SquareRoot(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,sqrtf,sqrt));
}

// Return the natural logarithm of the argument

int Loge(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,logf,log));
}

// Return the base 10 logarithm

int Log10(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,log10f,log10));
}

// Return the base 2 logarithm

int Log2(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,log2Base,log2Base));
}

// Return the sine of the argument

int Sine(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,sinf,sin));
}

// Return the cosine of the argument

int Cosine(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,cosf,cos));
}

// Return the tangent of the argument

int Tangent(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,tanf,tan));
}

// Return the sine of the argument

int ArcSine(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,asinf,asin));
}

// Return the cosine of the argument

int ArcCosine(Interface* itfc ,char args[])
{
   return(EvaluateSingleArgumentFunction(itfc,args,acosf,acos));
}

// Return the tangent of the argument

int ArcTangent(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,atanf,atan));
}

// Return the hyperbolic cosine of the argument

int HyperbolicCosine(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,coshf,cosh));
}

// Return the hyperbolic sine of the argument

int HyperbolicSine(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,sinhf,sinh));
}

// Return the hyperbolic tangent of the argument

int HyperbolicTangent(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,tanhf,tanh));
}

float j0sp(float x);
float j1sp(float x);

int ZerothOrderBessel(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,j0sp,j0));
}

int FirstOrderBessel(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,j1sp,j1));
}

float j0sp(float x)
{
   return((float)j0(float(x)));
}

float j1sp(float x)
{
   return((float)j1(float(x)));
}



// Return the absolute value of the argument

int AbsoluteValue(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,fabsf,fabs));
}

int RoundToNearestInteger(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,nintf,nintd));
}

int TruncateToInteger(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,truncf,truncd));
}

int Ceiling(Interface* itfc ,char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,ceilf,ceil));
}

int Floor(Interface* itfc ,char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,floorf,floor));
}

int ReturnSign(Interface* itfc ,char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,signf,signd));
}

int FindFractionalPart(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,fracf,fracd));
}

int NotOperator(Interface* itfc, char arg[])
{
   Variable var;
   short r;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract value to 'not'  
	if((r = ArgScan(itfc,arg,1,"binary result","e","v",&var)) < 0)
	  return(r); 

   if(VarType(&var) == UNQUOTED_STRING)
   {
      char *text = VarString(&var);
      if(!strcmp(text,"1"))
         ans->MakeAndSetString("0");
      else if(!strcmp(text,"0"))
         ans->MakeAndSetString("1");
      else if(!strcmp(text,"true"))
         ans->MakeAndSetString("false");
      else if(!strcmp(text,"false"))
         ans->MakeAndSetString("true");
      else if(!strcmp(text,"yes"))
         ans->MakeAndSetString("no");
      else if(!strcmp(text,"no"))
         ans->MakeAndSetString("yes");
      else
      {
         ErrorMessage("invalid argument");
         return(ERR);
      }
   }
   else if(VarType(&var) == FLOAT32)
   {
      float data = VarReal(&var);

      if(fabs(data - 0) < 1e-6)
         ans->MakeAndSetFloat(1);
      else if(fabs(data - 1) < 1e-6)
         ans->MakeAndSetFloat(0);
      else
      {
         ErrorMessage("invalid argument");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("invalid argument data type");
      return(ERR);
   }
   itfc->nrRetValues = 1;
   return(OK);
}

int InvOperator(Interface *itfc, char arg[])
{
   return(EvaluateSingleArgumentFunction(itfc,arg,invf,invd));
}

int NthOrderBessel(Interface *itfc, char args[])
{
   Variable x;
   long order;
   long i,j;
   short r;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract order and argument  
	if((r = ArgScan(itfc,args,2,"order, x","ee","lv",&order,&x)) < 0)
	  return(r); 

   switch(x.GetType())
   {
      case(FLOAT32):
	   {  
	      ans->MakeAndSetFloat(jn(order,x.GetReal()));
	      break;
	   } 
      case(FLOAT64):
	   {  
	      ans->MakeAndSetDouble(jn(order,x.GetDouble()));
	      break;
	   } 
	   case(MATRIX2D):
	   {
	      float **mat = x.GetMatrix2D();
         long w = x.GetDimX();
         long h = x.GetDimY();
	   
	      for(j = 0; j < h; j++)
	         for(i = 0; i < w; i++)
	            mat[j][i] = jn(order,mat[j][i]);

         ans->MakeAndLoadMatrix2D(mat,w,h);
	      break;
	   } 
      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}
   itfc->nrRetValues = 1;      
   return(OK);
}

/*************************************************************
*  Return the exponential of the argument 
*  Valid arguments types:
*
* real/complex scalar, 1D, 2D, 3D, 4D matrices.
**************************************************************/

int Exponential(Interface *itfc, char arg[])
{
   short type;
   long i,j,k,l;
   CArg carg;
   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Make sure there is only a single argument ******
   if(carg.Count(arg) != 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
   
// Process passed argument ************************
   if((type = Evaluate(itfc, RESPECT_ALIAS,arg,ans)) == ERR)
      return(ERR); 

// Calculate result *******************************
   switch(type)
   {
      case(FLOAT32):
	   {  
	      ans->MakeAndSetFloat(exp(ans->GetReal()));
	      break;
	   }
      case(FLOAT64):
	   {  
	      ans->MakeAndSetDouble(exp(ans->GetDouble()));
	      break;
	   }
	   case(COMPLEX): // calc exp(a+ib) = exp(a) * exp(ib)
      {              //                = exp(a)*(cos(b) + i*sin(b))
	      float real = cos(ans->GetComplex().i);
	      float imag = sin(ans->GetComplex().i);
	      float efac = exp(ans->GetComplex().r);
	      ans->MakeAndSetComplex(real*efac,imag*efac);
	      break;
      }   
	   case(MATRIX2D):
	   {
	      float **mat = ans->GetMatrix2D();
	   
	      for(j = 0; j < ans->GetDimY(); j++)
	         for(i = 0; i < ans->GetDimX(); i++)
	            mat[j][i] = exp(mat[j][i]);

	      break;
	   }
	   case(DMATRIX2D):
	   {
	      double **mat = ans->GetDMatrix2D();
	   
	      for(j = 0; j < ans->GetDimY(); j++)
	         for(i = 0; i < ans->GetDimX(); i++)
	            mat[j][i] = exp(mat[j][i]);

	      break;
	   }
	   case(MATRIX3D):
	   {
	      float ***mat = ans->GetMatrix3D();
	   
	      for(k = 0; k < ans->GetDimZ(); k++)
	         for(j = 0; j < ans->GetDimY(); j++)
	            for(i = 0; i < ans->GetDimX(); i++)
	               mat[k][j][i] = exp(mat[k][j][i]);
	      break;
	   }
	   case(MATRIX4D):
	   {
	      float ****mat = ans->GetMatrix4D();
	   
	      for(l = 0; l < ans->GetDimQ(); l++)
	         for(k = 0; k < ans->GetDimZ(); k++)
	            for(j = 0; j < ans->GetDimY(); j++)
	               for(i = 0; i < ans->GetDimX(); i++)
	                  mat[l][k][j][i] = exp(mat[l][k][j][i]);
	      break;
	   }
	   case(CMATRIX2D):
	   {
	      complex **cmat = ans->GetCMatrix2D();
	      complex temp;
	      
	      for(j = 0; j < ans->GetDimY(); j++)
	      {
	         for(i = 0; i < ans->GetDimX(); i++)
	         {
	            temp.r = exp(cmat[j][i].r) * cos(cmat[j][i].i);
	            temp.i = exp(cmat[j][i].r) * sin(cmat[j][i].i);
	            cmat[j][i] = temp;
	         }
	      }
	      break;
	   }	
	   case(CMATRIX3D):
	   {
	      complex ***cmat = ans->GetCMatrix3D();
	      complex temp;
	      
	      for(k = 0; k < ans->GetDimZ(); k++)
         {
	         for(j = 0; j < ans->GetDimY(); j++)
	         {
	            for(i = 0; i < ans->GetDimX(); i++)
	            {
	               temp.r = exp(cmat[k][j][i].r) * cos(cmat[k][j][i].i);
	               temp.i = exp(cmat[k][j][i].r) * sin(cmat[k][j][i].i);
	               cmat[k][j][i] = temp;
	            }
	         }
         }
	      break;
	   }
	   case(CMATRIX4D):
	   {
	      complex ****cmat = ans->GetCMatrix4D();
	      complex temp;
	      
	      for(l = 0; l < ans->GetDimQ(); l++)
         {
	         for(k = 0; k < ans->GetDimZ(); k++)
            {
	            for(j = 0; j < ans->GetDimY(); j++)
	            {
	               for(i = 0; i < ans->GetDimX(); i++)
	               {
	                  temp.r = exp(cmat[l][k][j][i].r) * cos(cmat[l][k][j][i].i);
	                  temp.i = exp(cmat[l][k][j][i].r) * sin(cmat[l][k][j][i].i);
	                  cmat[l][k][j][i] = temp;
	               }
	            }
            }
         }
	      break;
	   }
      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}

   itfc->nrRetValues = 1;
      
   return(OK);
}


/***************************************************************
*  Shifts the zero in a data set from the origin to the centre *  
****************************************************************/

int FFTShift(Interface *itfc, char arg[])
{
   short type;
   long i;
   Variable result;
	CArg carg;

// Make sure there is only a single argument ******
   if(carg.Count(arg) != 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
   
// Process passed argument ************************
   if((type = Evaluate(itfc,RESPECT_ALIAS,arg,&result)) == ERR)
      return(ERR); 

// Calculate result *******************************
   if(type == MATRIX2D)
   {
      float **mat = result.GetMatrix2D();
      float *v;
      
      long rows = result.GetDimY();
      long cols = result.GetDimX();
      if(rows == 1)
         DataReorder(mat[0],cols);
      else if(cols == 1)
      {
         v = MakeVector(rows);
         for(i = 0; i < rows; i++)
            v[i] = mat[i][0];
         DataReorder(v,rows);
         for(i = 0; i < rows; i++)
            mat[i][0] = v[i];
         FreeVector(v);
      }
      else
      {
         ErrorMessage("argument must be a row or column matrix");
         return(ERR);
      }	
      itfc->retVar[1].AssignMatrix2D(mat,cols,rows);
      result.SetNull();
   }
   else if(type == DMATRIX2D)
   {
      double **mat = result.GetDMatrix2D();
      double *v;
      
      long rows = result.GetDimY();
      long cols = result.GetDimX();
      if(rows == 1)
         DataReorder(mat[0],cols);
      else if(cols == 1)
      {
         v = MakeDVector(rows);
         for(i = 0; i < rows; i++)
            v[i] = mat[i][0];
         DataReorder(v,rows);
         for(i = 0; i < rows; i++)
            mat[i][0] = v[i];
         FreeDVector(v);
      }
      else
      {
         ErrorMessage("argument must be a row or column matrix");
         return(ERR);
      }	
      itfc->retVar[1].AssignDMatrix2D(mat,cols,rows);
      result.SetNull();
   }
   else if(type == CMATRIX2D)
   {
      complex **cmat = result.GetCMatrix2D();
      complex *v;
      
      long rows = result.GetDimY();
      long cols = result.GetDimX();
      if(rows == 1)
      {
         DataReorder(cmat[0],cols);
      }
      else if(cols == 1)
      {
         v = MakeCVector(rows);
         for(i = 0; i < rows; i++)
            v[i] = cmat[i][0];
         DataReorder(v,rows);
         for(i = 0; i < rows; i++)
            cmat[i][0] = v[i];
         FreeCVector(v);
      }
      else
      {
         ErrorMessage("argument must be a row or column matrix");
         return(ERR);
      }
      itfc->retVar[1].AssignCMatrix2D(cmat,cols,rows);
      result.SetNull();
   }	   
   else
   {
      ErrorMessage("argument must be a matrix");
      return(ERR);
   }
      
   return(OK);
}

/***********************************************************************************
   Sum the elements of a matrix

   Syntax:   s = sum(matrix/cmatrix,[mode]) 
   mode = "x","y","xy" (2D only)
 **********************************************************************************/

int Sum(Interface *itfc, char args[])
{
   Variable matVar;
   char mode[50] = "xy";
   short r;
   
   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract matrix and mode  
	if((r = ArgScan(itfc,args,1,"matrix, [sum direction]","ee","vs",&matVar,mode)) < 0)
	  return(r); 

// Sum data in matrices ************************
   switch(VarType(&matVar))
   {
	   case(MATRIX2D):
	   {
	      float sum = 0;
	      float **mat = VarRealMatrix(&matVar);
	      long xsize = VarColSize(&matVar);
	      long ysize = VarRowSize(&matVar);
	   
         if(!strcmp(mode,"xy"))
         {
	         for(long j = 0; j < ysize; j++)
	            for(long i = 0; i < xsize; i++)
	               sum += mat[j][i];
   
            ans->MakeAndSetFloat(sum);	
         }
         else if(!strcmp(mode,"x"))
         {
            float** out = MakeMatrix2D(1,ysize);
	         for(long j = 0; j < ysize; j++)
	         {
               sum = 0;
	            for(long i = 0; i < xsize; i++)
	               sum += mat[j][i];
               out[j][0] = sum;
	         }
            ans->AssignMatrix2D(out,1,ysize);
         }
         else if(!strcmp(mode,"y"))
         {
            float** out = MakeMatrix2D(xsize,1);
	         for(long i = 0; i < xsize; i++)
	         {
               sum = 0;
	            for(long j = 0; j < ysize; j++)
	               sum += mat[j][i];
               out[0][i] = sum;
	         }
            ans->AssignMatrix2D(out,xsize,1);
         }
	      break;          
	   }
	   case(DMATRIX2D):
	   {
	      double sum = 0;
         double **mat = matVar.GetDMatrix2D();
         long xsize = matVar.GetDimX();
	      long ysize = matVar.GetDimY();
	   
	      for(long j = 0; j < ysize; j++)
	         for(long i = 0; i < xsize; i++)
	            sum += mat[j][i];
   
         ans->MakeAndSetDouble(sum);	
	      break;         
	   }
	   case(MATRIX3D):
	   {
	      float sum = 0;
	      float ***mat = VarReal3DMatrix(&matVar);
	      long xsize = VarColSize(&matVar);
	      long ysize = VarRowSize(&matVar);
	      long zsize = VarTierSize(&matVar);
	   
	      for(long k = 0; k < zsize; k++)
	         for(long j = 0; j < ysize; j++)
	            for(long i = 0; i < xsize; i++)
	               sum += mat[k][j][i];
   
         ans->MakeAndSetFloat(sum);	
	      break;
	   }	   
	   case(CMATRIX2D):
	   {
	      complex sum;
	      complex **cmat = VarComplexMatrix(&matVar);
	      long xsize = VarColSize(&matVar);
	      long ysize = VarRowSize(&matVar);
	   
         if(!strcmp(mode,"xy"))
         {
            sum.r = sum.i = 0;
	         for(long j = 0; j < ysize; j++)
	         {
	            for(long i = 0; i < xsize; i++)
	            {
	               sum.r += cmat[j][i].r;
	               sum.i += cmat[j][i].i;
	            }
	         }
            ans->MakeAndSetComplex(sum);	
         }
         else if(!strcmp(mode,"x"))
         {
            complex** out = MakeCMatrix2D(1,ysize);
	         for(long j = 0; j < ysize; j++)
	         {
               sum.r = sum.i = 0;
	            for(long i = 0; i < xsize; i++)
	            {
	               sum.r += cmat[j][i].r;
	               sum.i += cmat[j][i].i;
	            }
               out[j][0] = sum;
	         }
            ans->AssignCMatrix2D(out,1,ysize);
         }
         else if(!strcmp(mode,"y"))
         {
            complex** out = MakeCMatrix2D(xsize,1);
	         for(long i = 0; i < xsize; i++)
	         {
               sum.r = sum.i = 0;
	            for(long j = 0; j < ysize; j++)
	            {
	               sum.r += cmat[j][i].r;
	               sum.i += cmat[j][i].i;
	            }
               out[0][i] = sum;
	         }
            ans->AssignCMatrix2D(out,xsize,1);
         }
	      break;
	   }	
	   case(CMATRIX3D):
	   {
	      complex sum = {0,0};
	      complex ***cmat = VarComplex3DMatrix(&matVar);
	      long xsize = VarColSize(&matVar);
	      long ysize = VarRowSize(&matVar);
	      long zsize = VarTierSize(&matVar);
	   
	      for(long k = 0; k < zsize; k++)
         {
	         for(long j = 0; j < ysize; j++)
            {
	            for(long i = 0; i < xsize; i++)
		         {
		            sum.r += cmat[k][j][i].r;
		            sum.i += cmat[k][j][i].i;
		         }
            }
         }
   
         ans->MakeAndSetComplex(sum);	
	      break;        
	   }		      
      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}
     
   return(0);
}


int Size(Interface* itfc ,char args[])
{
   short type,r,nArg;
   Variable var,*pvar = 0;
   long dim = -1;
   CArg carg;
   Variable* retVar;

   retVar = itfc->retVar;

// Extract arguments - but do it explicitly to save time since 
// don't actually need the matrix info, just the dimensions
   nArg = carg.Count(args);

   if(nArg == 0)
   {
   // Prompt the user  
	   if((r = ArgScan(itfc,args,1,"matrix, [axis]","ee","vl",&var,&dim)) < 0)
	      return(r); 
   }
   if(nArg >= 1)
   {
      char *arg1 = carg.Extract(1);
	   if(!(pvar = GetVariable(itfc,ALL_VAR,arg1,type)))
      {
	      if((r = ArgScan(itfc,args,1,"matrix, [axis]","ee","vl",&var,&dim)) < 0)
	         return(r); 
         pvar = &var;
      }
      else if(nArg == 2)
      {
         char *arg2 = carg.Extract(2);
	      if((r = ArgScan(itfc,arg2,1,"axis","e","l",&dim)) < 0)
	         return(r); 
      }
   }
   if (!pvar)
	{
	   ErrorMessage("No matrix variable supplied");
		return ERR;
	}
// Get size according to data type ****************
   switch(pvar->GetType())
   {
      case(UNQUOTED_STRING):
	      retVar[1].MakeAndSetFloat(strlen(VarString(pvar)));
	      itfc->nrRetValues = 1;
         break;
      case(MATRIX2D):
      case(DMATRIX2D):
      case(CMATRIX2D):
	   {
         if(dim == -1)
         {
            retVar[1].MakeAndSetFloat(VarWidth(pvar));
            retVar[2].MakeAndSetFloat(VarHeight(pvar));
            retVar[3].MakeAndSetFloat(VarDepth(pvar));
            retVar[4].MakeAndSetFloat(VarHyper(pvar));
	         itfc->nrRetValues = 4;
         }
         else
         {
            if(dim == 1)
            {
               retVar[1].MakeAndSetFloat(VarWidth(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 2)
            {
               retVar[1].MakeAndSetFloat(VarHeight(pvar));
	            itfc->nrRetValues = 1;
            }
            else
            {
               retVar[1].MakeAndSetFloat(1.0);
	            itfc->nrRetValues = 1;
            }
         }
	      break;
	   }	
      case(MATRIX3D):
      case(CMATRIX3D):
	   {
        if(dim == -1)
         {
            retVar[1].MakeAndSetFloat(VarWidth(pvar));
            retVar[2].MakeAndSetFloat(VarHeight(pvar));
            retVar[3].MakeAndSetFloat(VarDepth(pvar));
            retVar[4].MakeAndSetFloat(VarHyper(pvar));
	         itfc->nrRetValues = 4;
         }
         else
         {
            if(dim == 1)
            {
               retVar[1].MakeAndSetFloat(VarWidth(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 2)
            {
               retVar[1].MakeAndSetFloat(VarHeight(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 3)
            {
               retVar[1].MakeAndSetFloat(VarDepth(pvar));
	            itfc->nrRetValues = 1;
            }
            else
            {
               retVar[1].MakeAndSetFloat(1.0);
	            itfc->nrRetValues = 1;
            }
         }
	      break;
	   }	
      case(MATRIX4D):
      case(CMATRIX4D):
	   {
         if(dim == -1)
         {
            retVar[1].MakeAndSetFloat(VarWidth(pvar));
            retVar[2].MakeAndSetFloat(VarHeight(pvar));
            retVar[3].MakeAndSetFloat(VarDepth(pvar));
            retVar[4].MakeAndSetFloat(VarHyper(pvar));
	         itfc->nrRetValues = 4;
         }
         else
         {
            if(dim == 1)
            {
               retVar[1].MakeAndSetFloat(VarWidth(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 2)
            {
               retVar[1].MakeAndSetFloat(VarHeight(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 3)
            {
               retVar[1].MakeAndSetFloat(VarDepth(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 4)
            {
               retVar[1].MakeAndSetFloat(VarHyper(pvar));
	            itfc->nrRetValues = 1;
            }
            else
            {
               retVar[1].MakeAndSetFloat(1.0);
	            itfc->nrRetValues = 1;
            }
         }
	      break;
	   }	
	   case(LIST):
	      retVar[1].MakeAndSetFloat(VarWidth(pvar));
	      itfc->nrRetValues = 1;
         break;
	   case(LIST2D):
		{
			List2DData *lst = VarList2D(pvar);
			float *rows = new float[lst->nrRows];
			for(int i = 0; i < lst->nrRows; i++)
				rows[i] = lst->rowSz[i];
	      retVar[1].MakeMatrix2DFromVector(rows,lst->nrRows,1);
	      retVar[2].MakeAndSetFloat(lst->nrRows);
	      itfc->nrRetValues = 2;
			delete [] rows;
         break;
		}
      case(STRUCTURE):
      {
         Variable *struc, *svar;
         struc = pvar->GetStruct();
         svar = struc->next;
         // Count entries in structure
         int width = 0;
         while (svar != NULL)
         {
            width++;
            svar = svar->next;
         }
         retVar[1].MakeAndSetFloat(width);
         itfc->nrRetValues = 1;
         break;
      }
	   case(STRUCTURE_ARRAY):
	      retVar[1].MakeAndSetFloat(VarWidth(pvar));
	      itfc->nrRetValues = 1;
         break;
	   case(NULL_VARIABLE):
	      retVar[1].MakeAndSetFloat(0);
	      itfc->nrRetValues = 1;
         break; 
      default:
	   {
	      ErrorMessage("invalid argument type for 'size' command");
	      return(ERR);
	   }
	}	
            
   return(0);
}

int SizeD(Interface* itfc ,char args[])
{
   short type,r,nArg;
   Variable var,*pvar = 0;
   long dim = -1;
   CArg carg;
   Variable* retVar;

   retVar = itfc->retVar;

// Extract arguments - but do it explicitly to save time since 
// don't actually need the matrix info, just the dimensions
   nArg = carg.Count(args);

   if(nArg == 0)
   {
   // Prompt the user  
	   if((r = ArgScan(itfc,args,1,"matrix, [axis]","ee","vl",&var,&dim)) < 0)
	      return(r); 
   }
   if(nArg >= 1)
   {
      char *arg1 = carg.Extract(1);
	   if(!(pvar = GetVariable(itfc,ALL_VAR,arg1,type)))
      {
	      if((r = ArgScan(itfc,args,1,"matrix, [axis]","ee","vl",&var,&dim)) < 0)
	         return(r); 
         pvar = &var;
      }
      else if(nArg == 2)
      {
         char *arg2 = carg.Extract(2);
	      if((r = ArgScan(itfc,arg2,1,"axis","e","l",&dim)) < 0)
	         return(r); 
      }
   }
   if (!pvar)
	{
	   ErrorMessage("No matrix variable supplied");
		return ERR;
	}
// Get size according to data type ****************
   switch(pvar->GetType())
   {
      case(UNQUOTED_STRING):
	      retVar[1].MakeAndSetDouble(strlen(VarString(pvar)));
	      itfc->nrRetValues = 1;
         break;
      case(MATRIX2D):
      case(DMATRIX2D):
      case(CMATRIX2D):
	   {
         if(dim == -1)
         {
            retVar[1].MakeAndSetDouble(VarWidth(pvar));
            retVar[2].MakeAndSetDouble(VarHeight(pvar));
            retVar[3].MakeAndSetDouble(VarDepth(pvar));
            retVar[4].MakeAndSetDouble(VarHyper(pvar));
	         itfc->nrRetValues = 4;
         }
         else
         {
            if(dim == 1)
            {
               retVar[1].MakeAndSetDouble(VarWidth(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 2)
            {
               retVar[1].MakeAndSetDouble(VarHeight(pvar));
	            itfc->nrRetValues = 1;
            }
            else
            {
               retVar[1].MakeAndSetDouble(1.0);
	            itfc->nrRetValues = 1;
            }
         }
	      break;
	   }	
      case(MATRIX3D):
      case(CMATRIX3D):
	   {
        if(dim == -1)
         {
            retVar[1].MakeAndSetDouble(VarWidth(pvar));
            retVar[2].MakeAndSetDouble(VarHeight(pvar));
            retVar[3].MakeAndSetDouble(VarDepth(pvar));
            retVar[4].MakeAndSetDouble(VarHyper(pvar));
	         itfc->nrRetValues = 4;
         }
         else
         {
            if(dim == 1)
            {
               retVar[1].MakeAndSetDouble(VarWidth(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 2)
            {
               retVar[1].MakeAndSetDouble(VarHeight(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 3)
            {
               retVar[1].MakeAndSetDouble(VarDepth(pvar));
	            itfc->nrRetValues = 1;
            }
            else
            {
               retVar[1].MakeAndSetDouble(1.0);
	            itfc->nrRetValues = 1;
            }
         }
	      break;
	   }	
      case(MATRIX4D):
      case(CMATRIX4D):
	   {
         if(dim == -1)
         {
            retVar[1].MakeAndSetDouble(VarWidth(pvar));
            retVar[2].MakeAndSetDouble(VarHeight(pvar));
            retVar[3].MakeAndSetDouble(VarDepth(pvar));
            retVar[4].MakeAndSetDouble(VarHyper(pvar));
	         itfc->nrRetValues = 4;
         }
         else
         {
            if(dim == 1)
            {
               retVar[1].MakeAndSetDouble(VarWidth(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 2)
            {
               retVar[1].MakeAndSetDouble(VarHeight(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 3)
            {
               retVar[1].MakeAndSetDouble(VarDepth(pvar));
	            itfc->nrRetValues = 1;
            }
            else if(dim == 4)
            {
               retVar[1].MakeAndSetDouble(VarHyper(pvar));
	            itfc->nrRetValues = 1;
            }
            else
            {
               retVar[1].MakeAndSetDouble(1.0);
	            itfc->nrRetValues = 1;
            }
         }
	      break;
	   }	
	   case(LIST):
	      retVar[1].MakeAndSetDouble(VarWidth(pvar));
	      itfc->nrRetValues = 1;
         break;
	   case(NULL_VARIABLE):
	      retVar[1].MakeAndSetDouble(0);
	      itfc->nrRetValues = 1;
         break; 
      default:
	   {
	      ErrorMessage("invalid argument type for 'size' command");
	      return(ERR);
	   }
	}	
            
   return(0);
}


/************************************************************************
           Check to see if two matrices are the same size 
 
  Returns 1 is same size 0 if not and -1 if types are different
************************************************************************/

short MatricesSameSize(Variable *var1, Variable *var2)
{
   short type1 = VarType(var1);
   short type2 = VarType(var2);
   long xdim1,xdim2;
   long ydim1,ydim2;
   long zdim1,zdim2;
   
   if(type1 != MATRIX2D && type1 != CMATRIX2D && type1 != MATRIX3D && type1 != CMATRIX3D && type1 != LIST &&
      type2 != MATRIX2D && type2 != CMATRIX2D && type2 != MATRIX3D && type2 != CMATRIX3D && type2 != LIST)
   {
      return(-1);
   }
   
   xdim1 = VarWidth(var1);
   ydim1 = VarHeight(var1);
   zdim1 = VarDepth(var1);
   xdim2 = VarWidth(var2);
   ydim2 = VarHeight(var2);
   zdim2 = VarDepth(var2);
      
   if(xdim1 == xdim2 && ydim1 == ydim2 && zdim1 == zdim2)
     return(1);
   else
     return(0);
}


/************************************************************************
    Return the minimum value (v) in a matrix and its location (x,y,z) 

  Syntax: (v,x,y) = min(matrix)  or
          (v,x,y,z) = min(maxtrix3d)

************************************************************************/

int Minimum(Interface *itfc, char arg[])
{
   short type;
   long minX = 0,minY = 0,minZ = 0;
   Variable result;
   long i,j,k;
	CArg carg;

// Make sure there is only a single argument
   if(carg.Count(arg) != 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
      
// Process passed argument
   if((type = Evaluate(itfc,RESPECT_ALIAS,arg,&result)) == ERR)
      return(ERR);

// Find minimum   
   if(type == MATRIX2D)
   {
      float minVal = 1e30;
      float **mat = result.GetMatrix2D();
      
      for(j = 0; j < result.GetDimY(); j++)
      {   
         for(i = 0; i < result.GetDimX(); i++)
         {
            if (mat[j][i] < minVal) 
            {
               minVal = mat[j][i]; 
               minX = i;
               minY = j;
            }
         }
      }
      itfc->retVar[1].MakeAndSetFloat(minVal);
      itfc->retVar[2].MakeAndSetFloat(minX);
      itfc->retVar[3].MakeAndSetFloat(minY);
	   itfc->nrRetValues = 3;  
   }
   else if(type == DMATRIX2D)
   {
      double minVal = 1e30;
      double **mat = result.GetDMatrix2D();
      
      for(j = 0; j < result.GetDimY(); j++)
      {   
         for(i = 0; i < result.GetDimX(); i++)
         {
            if(mat[j][i] < minVal) 
            {
               minVal = mat[j][i];
               minX = i;
               minY = j;
            }
         }
      }
      itfc->retVar[1].MakeAndSetDouble(minVal);
      itfc->retVar[2].MakeAndSetFloat(minX);
      itfc->retVar[3].MakeAndSetFloat(minY);
	   itfc->nrRetValues = 3;  
   }
   else if(type == MATRIX3D)
   {
      float minVal = 1e30;
      float ***mat = result.GetMatrix3D();
   
      for(k = 0; k < result.GetDimZ(); k++)
      {      
         for(j = 0; j < result.GetDimY(); j++)
         {   
            for(i = 0; i < result.GetDimX(); i++)
            {
               if(mat[k][j][i] < minVal)
               {
                  minVal = mat[k][j][i];
                  minX = i;
                  minY = j;                  
                  minZ = k;
               }               
            }
         }
      }
      itfc->retVar[1].MakeAndSetFloat(minVal);
      itfc->retVar[2].MakeAndSetFloat(minX);
      itfc->retVar[3].MakeAndSetFloat(minY);
      itfc->retVar[4].MakeAndSetFloat(minZ);
	   itfc->nrRetValues = 4;  
   }     
   else
   {
      ErrorMessage("invalid data type");
      return(ERR);
   }
         
   return(0);
}


/************************************************************************
    Return the maximum value (v) in a matrix and its location (x,y,z) 

  Syntax: (v,x,y) = max(matrix)  or
          (v,x,y,z) = max(maxtrix3d)

************************************************************************/

int Maximum(Interface *itfc, char arg[])
{
   short type;
   long maxX = 0,maxY = 0,maxZ = 0;
   Variable result;
   long i,j,k;
	CArg carg;

// Make sure there is only a single argument
   if(carg.Count(arg) != 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
      
// Process passed argument
   if((type = Evaluate(itfc,RESPECT_ALIAS,arg,&result)) == ERR)
      return(ERR);

// Find maximum   
   if(type == MATRIX2D)
   {
      float maxVal = -1e30;
      float **mat = result.GetMatrix2D();
 
      for(j = 0; j < result.GetDimY(); j++)
      {   
         for(i = 0; i < result.GetDimX(); i++)
         {
            if (mat[j][i] > maxVal)
            {
               maxVal = mat[j][i];
               maxX = i;
               maxY = j;
            }
         }
      }

      itfc->retVar[1].MakeAndSetFloat(maxVal);
      itfc->retVar[2].MakeAndSetFloat(maxX);
      itfc->retVar[3].MakeAndSetFloat(maxY);
	   itfc->nrRetValues = 3;
   }
   else if(type == DMATRIX2D)
   {
      double maxVal = -1e30;
      double **mat = result.GetDMatrix2D();
   
      for(j = 0; j < result.GetDimY(); j++)
      {   
         for(i = 0; i < result.GetDimX(); i++)
         {
            if(mat[j][i] > maxVal) 
            {
               maxVal = mat[j][i];
               maxX = i;
               maxY = j;
            }
         }
      }
      itfc->retVar[1].MakeAndSetDouble(maxVal);
      itfc->retVar[2].MakeAndSetFloat(maxX);
      itfc->retVar[3].MakeAndSetFloat(maxY);
	   itfc->nrRetValues = 3;
   }
   else if(type == MATRIX3D)
   {
      float maxVal = -1e30;
      float ***mat = result.GetMatrix3D();
   
      for(k = 0; k < result.GetDimZ(); k++)
      {      
         for(j = 0; j < result.GetDimY(); j++)
         {   
            for(i = 0; i < result.GetDimX(); i++)
            {
               if(mat[k][j][i] > maxVal) 
               {
                  maxVal = mat[k][j][i];
                  maxX = i;
                  maxY = j;                  
                  maxZ = k;
               }                  
            }
         }
      }
      itfc->retVar[1].MakeAndSetFloat(maxVal);
      itfc->retVar[2].MakeAndSetFloat(maxX);
      itfc->retVar[3].MakeAndSetFloat(maxY);
      itfc->retVar[4].MakeAndSetFloat(maxZ);
	   itfc->nrRetValues = 4;         
   }   
   else
   {
      ErrorMessage("invalid data type");
      return(ERR);
   }
         
   return(0);
}

/************************************************************************
    Generate a random integer between 0 and max

    Syntax: n = rand(max)

 ***********************************************************************/

int Random(Interface *itfc, char arg[])
{
   short nrArgs;
   Variable maxVar;
   long num,width,height = 1;
   CArg carg;
   
   nrArgs = carg.Count(arg);

   if(nrArgs == 1)
   {
      if((nrArgs = ArgScan(itfc,arg,1,"max","e","v",&maxVar)) < 0)
         return(nrArgs); 
   }

   if(nrArgs >= 2)
   {
      if((nrArgs = ArgScan(itfc,arg,2,"width, height, max","eee","llv",&width,&height,&maxVar)) < 0)
         return(nrArgs); 
   }


   if(nrArgs == 1)
   {
      if(maxVar.GetType() == FLOAT32)
      {
         float max = maxVar.GetReal()+1;
         ULONG32 num = (ULONG32)(rand()*(float)max/(float)RAND_MAX);
         if(num == max) num = (ULONG32)(max-1);
         itfc->retVar[1].MakeAndSetFloat((float)num);
         itfc->nrRetValues = 1;
         return(OK);
      }
      else if(maxVar.GetType() == FLOAT64)
      {
         double max = maxVar.GetDouble()+1;
         ULONG64 num = (ULONG64)(rand()*(double)max/(double)RAND_MAX);
         if(num >= max) num = (ULONG64)(max-1);
         itfc->retVar[1].MakeAndSetDouble(num);
         itfc->nrRetValues = 1;
         return(OK);
      }
   }

   else if(nrArgs >= 2)
   {
      if(maxVar.GetType() == FLOAT32)
      {
         float max = maxVar.GetReal()+1;
         float **outMat = MakeMatrix2D(width,height);
         for(int y = 0; y < height; y++)
         {
            for(int x = 0; x < width; x++)
            {
               num = (int)(rand()*(float)max/(float)RAND_MAX);
               if(num >= max) num = (ULONG32)(max-1);
               outMat[y][x] = num;
            }
         }
         itfc->retVar[1].AssignMatrix2D(outMat,width,height);
         itfc->nrRetValues = 1;
      }
      else if(maxVar.GetType() == FLOAT64)
      {
         double max = maxVar.GetDouble()+1;
         double **outMat = MakeDMatrix2D(width,height);
         for(int y = 0; y < height; y++)
         {
            for(int x = 0; x < width; x++)
            {
               num = (int)(rand()*(double)max/(double)RAND_MAX);
               if(num >= max) num = (ULONG64)(max-1);
               outMat[y][x] = num;
            }
         }
         itfc->retVar[1].AssignDMatrix2D(outMat,width,height);
         itfc->nrRetValues = 1;
      }
   } 
   return(0);
} 



/************************************************************************
   Generate a scalar or matrix filled with Gaussian random numbers with
   specified standard deviation.

   Syntax m = noise(width,[height,[depth]]
 ***********************************************************************/

int Noise(Interface *itfc, char arg[])
{
   short r;
   long cols = 1,rows = 1,tiers = 1,hypers = 1;
   long i,j,k,q;
   static long seed = 0;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract the dimensions of the matrix to be filled **************
   if((r = ArgScan(itfc,arg,1,"nrx,[nry],[nrz],[nrq]","eeee","llll",&cols,&rows,&tiers,&hypers)) < 0)
      return(r); 

// Make sure we get a new seed for the random number generator ****
	//int cnt = 0;
 //  while((seed = -abs((long)GetTickCount())) == oldSeed) {;}

   
// Fill a vector or scalar with Gaussian noise ********************
   if(r == 1) 
   {
      if(cols == 1) // Just a scalar
      {
         ans->MakeAndSetFloat(MakeRandomGaussianNum(seed));      
      }
      else // A vector
      {
         ans->MakeAndLoadMatrix2D(NULL,cols,1);
	      for(i = 0 ; i < cols; i++)
	         ans->GetMatrix2D()[0][i] = MakeRandomGaussianNum(seed);
	   }
   }
// Fill a 2D matrix with Gaussian noise ****************************
   else if(r == 2) 
   {
      ans->MakeAndLoadMatrix2D(NULL,cols,rows);

	   for(j = 0 ; j < rows; j++)
	      for(i = 0 ; i < cols; i++)
	         ans->GetMatrix2D()[j][i] = MakeRandomGaussianNum(seed);
   }
// Fill a 3D matrix with Gaussian noise ****************************
   else if(r == 3) 
   {
      ans->MakeAndLoadMatrix3D(NULL,cols,rows,tiers);

	   for(k = 0 ; k < tiers; k++)
	      for(j = 0 ; j < rows; j++)
	         for(i = 0 ; i < cols; i++)
	            VarReal3DMatrix(ans)[k][j][i] = MakeRandomGaussianNum(seed);
   } 
// Fill a 4D matrix with Gaussian noise ****************************
   else if(r == 4) 
   {
      ans->MakeAndLoadMatrix4D(NULL,cols,rows,tiers,hypers);

	   for(q = 0 ; q < hypers; q++)
	      for(k = 0 ; k < tiers; k++)
	         for(j = 0 ; j < rows; j++)
	            for(i = 0 ; i < cols; i++)
	               VarReal4DMatrix(ans)[q][k][j][i] = MakeRandomGaussianNum(seed);
   }
   else
   {
      ErrorMessage("invalid number of indices");
      return(ERR);
   }
	seed++;
   return(OK);
} 


/************************************************************************
   Generate a scalar or matrix filled with Gaussian random numbers with
   specified standard deviation. (Double precision version)

   Syntax m = noised(width[,height[,depth[,hyperdepth]]])
 ***********************************************************************/

int NoiseDouble(Interface *itfc, char arg[])
{
   short r;
   long cols = 1,rows = 1,tiers = 1,hypers = 1;
   long i,j;
   long seed;
   static long oldSeed;
 
   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract the dimensions of the matrix to be filled **************
   if((r = ArgScan(itfc,arg,1,"nrx,[nry],[nrz],[nrq]","eeee","llll",&cols,&rows,&tiers,&hypers)) < 0)
      return(r); 

// Make sure we get a new seed for the random number generator ****
   while((seed = -abs((long)GetTickCount())) == oldSeed) {;}
   oldSeed = seed;

// Fill a vector or scalar with Gaussian noise ********************
   if(r == 1) 
   {
      if(cols == 1) // Just a scalar
      {
         ans->MakeAndSetDouble(MakeRandomGaussianDNum(seed));      
      }
      else // A vector
      {
         ans->MakeAndLoadDMatrix2D(NULL,cols,1);
	      for(i = 0 ; i < cols; i++)
	         ans->GetDMatrix2D()[0][i] = MakeRandomGaussianDNum(seed);
	   }
   }
// Fill a 2D matrix with Gaussian noise ****************************
   else if(r == 2) 
   {
      ans->MakeAndLoadDMatrix2D(NULL,cols,rows);

	   for(j = 0 ; j < rows; j++)
	      for(i = 0 ; i < cols; i++)
	         ans->GetDMatrix2D()[j][i] = MakeRandomGaussianDNum(seed);
   }
//// Fill a 3D matrix with Gaussian noise ****************************
//   else if(r == 3) 
//   {
//      ans->MakeAndLoadMatrix3D(NULL,cols,rows,tiers);
//
//	   for(k = 0 ; k < tiers; k++)
//	      for(j = 0 ; j < rows; j++)
//	         for(i = 0 ; i < cols; i++)
//	            VarReal3DMatrix(ans)[k][j][i] = MakeRandomGaussianNum(seed);
//   } 
//// Fill a 4D matrix with Gaussian noise ****************************
//   else if(r == 4) 
//   {
//      ans->MakeAndLoadMatrix4D(NULL,cols,rows,tiers,hypers);
//
//	   for(q = 0 ; q < hypers; q++)
//	      for(k = 0 ; k < tiers; k++)
//	         for(j = 0 ; j < rows; j++)
//	            for(i = 0 ; i < cols; i++)
//	               VarReal4DMatrix(ans)[q][k][j][i] = MakeRandomGaussianNum(seed);
//   }
   else
   {
      ErrorMessage("invalid number of indices");
      return(ERR);
   }
   return(OK);
} 

float MakeRandomGaussianNum(long &seed)
{
   static short iset = 0;
   static float gset;
   
   if(iset == 0) 
	{
		float fac,r,v1,v2;
      do
      {
         v1 = 2.0*ran0(seed)-1.0;
         v2 = 2.0*ran0(seed)-1.0;
         r = v1*v1 + v2*v2;
      }
      while(r >= 1.0 || r == 0);

      fac = sqrt(-2.0*log(r)/r);

      gset = v1*fac;
      iset = 1;
      return(v2*fac);	      
   }
   else
   {
      iset = 0;
      return(gset);	
   }
}


double MakeRandomGaussianDNum(long &seed)
{
   static short iset = 0;
   static double gset;
   
   if(iset == 0) 
   {
		double fac,r,v1,v2;
      do
      {
         v1 = 2.0*ran0d(seed)-1.0;
         v2 = 2.0*ran0d(seed)-1.0;
         r = v1*v1 + v2*v2;
      }
      while(r >= 1.0 || r == 0);

      fac = sqrt(-2.0*log(r)/r);

      gset = v1*fac;
      iset = 1;
      return(v2*fac);	      
   }
   else
   {
      iset = 0;
      return(gset);	
   }
}
        
/****************************************************************************   
*                 Returns random double between 0.0 and 1.0                 *
*                 Numerical recipes p.207                                   *
****************************************************************************/

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876

float ran0(long &seed)
{
   long k;
   float ans;
   
   seed ^= MASK;
   k = (seed)/IQ;
   seed = IA*(seed - k*IQ) - IR*k;
   if(seed < 0) seed += IM;
   ans = AM*(seed);
   seed ^= MASK;
   return(ans);
}

        
/****************************************************************************   
*                 Returns random double between 0.0 and 1.0                 *
*                 Numerical recipes p.207                                   *
****************************************************************************/

#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876

double ran0d(long &seed)
{
   long k;
   double ans;
   
   seed ^= MASK;
   k = (seed)/IQ;
   seed = IA*(seed - k*IQ) - IR*k;
   if(seed < 0) seed += IM;
   ans = AM*(seed);
   seed ^= MASK;
   return(ans);
}


short EvaluateSingleArgumentFunction(Interface* itfc, char *arg, float (*func)(float), double (*funcD)(double))
{
   short type,nrArgs;
   long i,j,k;
   long width,height,depth;
   CArg carg;
   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];
   Variable result;

// Make sure there is only a single argument
   nrArgs = carg.Count(arg);
   
   if(nrArgs == 0)
   {
      TextMessage("\n\n   argument: expression  (e:f/c/m/cm)\n");
      return(OK);
   }
   
   if(nrArgs > 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
      
// Process passed argument
   if((type = Evaluate(itfc,RESPECT_ALIAS,arg,&result)) == ERR)
      return(ERR);
      
// Apply function to argument ****************
   switch(type)
   {
      case(FLOAT32):
      {  
         float data = result.GetReal();
         ans->MakeAndSetFloat(func(data));
         break;
      }
      case(FLOAT64):
      {  
         double data = result.GetDouble();
         ans->MakeAndSetDouble(funcD(data));
         break;
      }
      case(COMPLEX):
      {  
         complex data = result.GetComplex();
         ans->MakeAndSetComplex(func(data.r),func(data.i));
         break;
      } 
		case(STRUCTURE): // Process valid data types ignore others
		{
         Variable *struc,*svar;
         struc = result.GetStruct();
			if(!struc)
			{
				ErrorMessage("Null structure");
				return(ERR);
			}
         svar = struc->next;

         while(svar != NULL)
         {
				if(svar->GetType() == FLOAT32)
					svar->SetReal(func(svar->GetReal()));
				else if(svar->GetType() == FLOAT64)
					svar->SetDouble(func(svar->GetDouble()));
				else if(svar->GetType() == COMPLEX)
				{
				   complex data = svar->GetComplex();
					complex z;
					z.r = func(data.r);
					z.i = func(data.i);
				 	svar->SetComplex(z);
				}
				else if(svar->GetType() == MATRIX2D)
				{
					float **mat = svar->GetMatrix2D();
					width = svar->GetDimX();
					height = svar->GetDimY();
	   
					for(j = 0; j < height; j++)
						for(i = 0; i < width; i++)
							mat[j][i] = func(mat[j][i]);
				}
				else if(svar->GetType() == DMATRIX2D)
				{
					double **mat = svar->GetDMatrix2D();
					width = svar->GetDimX();
					height = svar->GetDimY();
	   
					for(j = 0; j < height; j++)
						for(i = 0; i < width; i++)
							mat[j][i] = func(mat[j][i]);
				}
				else if(svar->GetType() == CMATRIX2D)
				{
					complex **cmat = svar->GetCMatrix2D();
					width = svar->GetDimX();
					height = svar->GetDimY();
	   
					for(j = 0; j < height; j++)
					{
						for(i = 0; i < width; i++)
						{
							cmat[j][i].r = func(cmat[j][i].r);
							cmat[j][i].i = func(cmat[j][i].i);
						}
					}
				}
				else if(svar->GetType() == MATRIX3D)
				{
					float ***mat = svar->GetMatrix3D();
					width  = result.GetDimX();
					height = result.GetDimY();
					depth  = result.GetDimZ();

					for(k = 0; k < depth; k++)   
						for(j = 0; j < height; j++)
							for(i = 0; i < width; i++)
								mat[k][j][i] = func(mat[k][j][i]);
				}
				else if(svar->GetType() == CMATRIX3D)
				{
					complex ***cmat = svar->GetCMatrix3D();
					width  = result.GetDimX();
					height = result.GetDimY();
					depth  = result.GetDimZ();

					for(k = 0; k < depth; k++)
					{	   
						for(j = 0; j < height; j++)
						{
							for(i = 0; i < width; i++)
							{
								cmat[k][j][i].r = func(cmat[k][j][i].r);
								cmat[k][j][i].i = func(cmat[k][j][i].i);		            
							}
						}
					}
				}
            svar = svar->next;
         }
         ans->Assign(&result);
         result.SetNull();
			break;
		}
	   case(MATRIX2D):
	   {
	      float **mat = result.GetMatrix2D();
         width = result.GetDimX();
         height = result.GetDimY();
	   
	      for(j = 0; j < height; j++)
	         for(i = 0; i < width; i++)
	            mat[j][i] = func(mat[j][i]);

         ans->Assign(&result);
         result.SetNull();
	      break;
	   }
	   case(DMATRIX2D):
	   {
	      double **mat = result.GetDMatrix2D();
         width = result.GetDimX();
         height = result.GetDimY();
	   
	      for(j = 0; j < height; j++)
	         for(i = 0; i < width; i++)
	            mat[j][i] = funcD(mat[j][i]);

         ans->Assign(&result);
         result.SetNull();
	      break;
	   }
	   case(CMATRIX2D):
	   {
	      complex **cmat = result.GetCMatrix2D();
         width = result.GetDimX();
         height = result.GetDimY();

	      for(j = 0; j < height; j++)
         {
	         for(i = 0; i < width; i++)
            {
	            cmat[j][i].r = func(cmat[j][i].r);
	            cmat[j][i].i = func(cmat[j][i].i);
	         }
	      }
         ans->Assign(&result);
         result.SetNull();
	      break;
	   }	
	   case(MATRIX3D):
	   {
	      float ***mat = result.GetMatrix3D();
         width  = result.GetDimX();
         height = result.GetDimY();
         depth  = result.GetDimZ();

	      for(k = 0; k < depth; k++)   
		      for(j = 0; j < height; j++)
		         for(i = 0; i < width; i++)
		            mat[k][j][i] = func(mat[k][j][i]);

         ans->Assign(&result);
         result.SetNull();
	      break;
	   }
	   case(CMATRIX3D):
	   {
	      complex ***mat = result.GetCMatrix3D();
         width  = result.GetDimX();
         height = result.GetDimY();
         depth  = result.GetDimZ();

	      for(k = 0; k < depth; k++)
	      {	   
		      for(j = 0; j < height; j++)
		      {
		         for(i = 0; i < width; i++)
		         {
		            mat[k][j][i].r = func(mat[k][j][i].r);
		            mat[k][j][i].i = func(mat[k][j][i].i);		            
		         }
		      }
		   }
         ans->Assign(&result);
         result.SetNull();
	      break;
	   }	   	      	   	   
      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}
   itfc->nrRetValues = 1;
	return(OK);
}


/******************************************************
 Take the factorial of an integer
******************************************************/

int Factorial(Interface *itfc, char args[])
{
   short r;
   Variable var;

   if((r = ArgScan(itfc,args,1,"number","e","v",&var)) < 0)
      return(r); 

   if(var.GetType() == FLOAT32)
   {  
      float data = var.GetReal();
      float fac = 1;
	   long num = nint(data);
	   if((fabs(data - num) > 0.000001) || num < 0 || num > 34)
	   {
		   ErrorMessage("can't take factorial of %g",data);
		   return(ERR);
	   }

      for(short i = 2; i <= num; i++)
         fac = fac*i;
 
      itfc->retVar[1].MakeAndSetFloat(fac);
      itfc->nrRetValues = 1;
   }
   else if(var.GetType() == FLOAT64)
   {  
      double data = var.GetDouble();
      double fac = 1;
	   long num = nintd(data);
	   if((fabs(data - num) > 0.000001) || num < 0 || num > 170)
	   {
		   ErrorMessage("can't take factorial of %g",data);
		   return(ERR);
	   }

      for(short i = 2; i <= num; i++)
         fac = fac*i;
 
      itfc->retVar[1].MakeAndSetDouble(fac);
      itfc->nrRetValues = 1;
   }  
   return(OK);
}

/******************************************************
 Takes the transpose of a matrix
******************************************************/

int TransposeMatrix(Interface *itfc, char args[])
{
   short r;
   long x,y,z;
   long xsize,ysize,zsize;
   CText transposeWhich = "xy";
   Variable var;

   Variable* ans = &itfc->retVar[1];

// Get the arguments
   if((r = ArgScan(itfc,args,1,"matrix, [axes to swap]","ee","vt",&var,&transposeWhich)) < 0)
      return(r); 

   switch(var.GetType())
   {
      case(MATRIX2D):
      {
         float **mat = VarRealMatrix(&var);
         float **trans;

         xsize = VarWidth(&var);
         ysize = VarHeight(&var);

         if(xsize <= 0 || ysize <= 0)
         {
            ErrorMessage("matrix dimensions are invalid");
            return(ERR);
         }
   	
	   // Allocate memory for transposed matrix
         if(!(trans = MakeMatrix2D(ysize, xsize)))
         {
            ErrorMessage("unable to allocate memory for transposed matrix");
            return(ERR);
         }

      // Do the transposition
         for(y = 0; y < ysize; y++)
	         for(x = 0; x < xsize; x++)
	            trans[x][y] = mat[y][x];
    
   // Update answer variable
         ans->AssignMatrix2D(trans,ysize,xsize);
         break;
      }
      
      case(DMATRIX2D):
      {
         double **mat = var.GetDMatrix2D();
         double **trans;

         xsize = VarWidth(&var);
         ysize = VarHeight(&var);

         if(xsize <= 0 || ysize <= 0)
         {
            ErrorMessage("matrix dimensions are invalid");
            return(ERR);
         }
   	
	   // Allocate memory for transposed matrix
         if(!(trans = MakeDMatrix2D(ysize, xsize)))
         {
            ErrorMessage("unable to allocate memory for transposed matrix");
            return(ERR);
         }

      // Do the transposition
         for(y = 0; y < ysize; y++)
	         for(x = 0; x < xsize; x++)
	            trans[x][y] = mat[y][x];
    
   // Update answer variable
         ans->AssignDMatrix2D(trans,ysize,xsize);
         break;
      }
      case(CMATRIX2D):
      {
         complex **mat = VarComplexMatrix(&var);
         complex **trans;

         xsize = VarWidth(&var);
         ysize = VarHeight(&var);
   	
         if(xsize <= 0 || ysize <= 0)
         {
            ErrorMessage("matrix dimensions are invalid");
            return(ERR);
         }

	   // Allocate memory for transposed matrix	    
         if(!(trans = MakeCMatrix2D(ysize, xsize)))
         {
	         ErrorMessage("unable to allocate memory for transposed matrix");
	         return(ERR);
         }

      // Do the transposition	    
	      for(y = 0; y < ysize; y++)
	         for(x = 0; x < xsize; x++)
	            trans[x][y] = mat[y][x];

   // Update answer variable
         ans->AssignCMatrix2D(trans,ysize,xsize);
         break;
      }
      case(MATRIX3D):
      {
         float ***mat = VarReal3DMatrix(&var);
         float ***trans;
   	    
         xsize = VarWidth(&var);
         ysize = VarHeight(&var);
         zsize = VarDepth(&var);

         if(xsize <= 0 || ysize <= 0 || zsize <= 0)
         {
            ErrorMessage("matrix dimensions are invalid");
            return(ERR);
         }

      // Determine transpose type
         if(transposeWhich == "xy" || transposeWhich == "yx")
         {
	      // Allocate memory for transposed matrix
            if(!(trans = MakeMatrix3D(ysize, xsize, zsize)))
	         {
	            ErrorMessage("unable to allocate memory for transposed matrix");
	            return(ERR);
	         }
         // Do the transposition	    
	         for(z = 0; z < zsize; z++)
	            for(y = 0; y < ysize; y++)
	               for(x = 0; x < xsize; x++)
	                  trans[z][x][y] = mat[z][y][x];

      // Update answer variable
            ans->AssignMatrix3D(trans,ysize,xsize,zsize);
         }
         else if(transposeWhich == "xz" || transposeWhich == "zx")
         {
	      // Allocate memory for transposed matrix
            if(!(trans = MakeMatrix3D(zsize, ysize, xsize)))
	         {
	            ErrorMessage("unable to allocate memory for transposed matrix");
	            return(ERR);
	         }
         // Do the transposition	    
	         for(z = 0; z < zsize; z++)
	            for(y = 0; y < ysize; y++)
	               for(x = 0; x < xsize; x++)
	                  trans[x][y][z] = mat[z][y][x];

      // Update answer variable
            ans->AssignMatrix3D(trans,zsize,ysize,xsize);
         }
         else if(transposeWhich == "yz" || transposeWhich == "zy")
         {
	      // Allocate memory for transposed matrix
            if(!(trans = MakeMatrix3D(xsize, zsize, ysize)))
	         {
	            ErrorMessage("unable to allocate memory for transposed matrix");
	            return(ERR);
	         }
         // Do the transposition	    
	         for(z = 0; z < zsize; z++)
	            for(y = 0; y < ysize; y++)
	               for(x = 0; x < xsize; x++)
	                  trans[y][z][x] = mat[z][y][x];

         // Update answer variable
            ans->AssignMatrix3D(trans,xsize,zsize,ysize);
         }
         else
         {
            ErrorMessage("invalid transposition");
            return(ERR);
         }
         break;
      }
      case(CMATRIX3D):
      {
         complex ***mat = VarComplex3DMatrix(&var);
         complex ***trans;
   	    
         xsize = VarWidth(&var);
         ysize = VarHeight(&var);
         zsize = VarDepth(&var);

         if(xsize <= 0 || ysize <= 0 || zsize <= 0)
         {
            ErrorMessage("matrix dimensions are invalid");
            return(ERR);
         }

      // Determine transpose type

         if(transposeWhich == "xy" || transposeWhich == "yx")
         {
	      // Allocate memory for transposed matrix
            if(!(trans = MakeCMatrix3D(ysize, xsize, zsize)))
	         {
	            ErrorMessage("unable to allocate memory for transposed matrix");
	            return(ERR);
	         }
         // Do the transposition	    
	         for(z = 0; z < zsize; z++)
	            for(y = 0; y < ysize; y++)
	               for(x = 0; x < xsize; x++)
	                  trans[z][x][y] = mat[z][y][x];

         // Update answer variable
            ans->AssignCMatrix3D(trans,ysize,xsize,zsize);
         }
         else if(transposeWhich == "xz" || transposeWhich == "zx")
         {
	      // Allocate memory for transposed matrix
            if(!(trans = MakeCMatrix3D(zsize, ysize, xsize)))
	         {
	            ErrorMessage("unable to allocate memory for transposed matrix");
	            return(ERR);
	         }
         // Do the transposition	    
	         for(z = 0; z < zsize; z++)
	            for(y = 0; y < ysize; y++)
	               for(x = 0; x < xsize; x++)
	                  trans[x][y][z] = mat[z][y][x];

         // Update answer variable
            ans->AssignCMatrix3D(trans,zsize,ysize,xsize);
         }
         else if(transposeWhich == "yz" || transposeWhich == "zy")
         {
	      // Allocate memory for transposed matrix
            if(!(trans = MakeCMatrix3D(xsize, zsize, ysize)))
	         {
	            ErrorMessage("unable to allocate memory for transposed matrix");
	            return(ERR);
	         }
         // Do the transposition	    
	         for(z = 0; z < zsize; z++)
	            for(y = 0; y < ysize; y++)
	               for(x = 0; x < xsize; x++)
	                  trans[y][z][x] = mat[z][y][x];

      // Update answer variable
            ans->AssignCMatrix3D(trans,xsize,zsize,ysize);
         }
         else
         {
            ErrorMessage("invalid transposition");
            return(ERR);
         }
         break;
      }
      default:
      {
         ErrorMessage("invalid data type");
         return(ERR);
      }
   }

   itfc->nrRetValues = 1;

   return(OK);
}

/******************************************************
* Calculates the complex conjugate of a complex variable
******************************************************/

int ComplexConjugate(Interface *itfc, char arg[])
{
   short type;
   long i,j;
   Variable result;
	CArg carg;

// Make sure there is only a single argument
   if(carg.Count(arg) != 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
     
// Process passed argument *************
   if((type = Evaluate(itfc,RESPECT_ALIAS,arg,&result)) == ERR)
      return(ERR);
      
// Take the complex conjugate of the argument *
   switch(type)
   {
      case(FLOAT32):
      {  
         itfc->retVar[1].Assign(&result);
         result.SetNull();
         break;
      }
      case(FLOAT64):
      {  
         itfc->retVar[1].Assign(&result);
         result.SetNull();
         break;
      }
      case(COMPLEX):
      {  
         complex a = result.GetComplex();
         itfc->retVar[1].MakeAndSetComplex(a.r,-a.i);
         break;
      }      
	   case(MATRIX2D):
	   {
         itfc->retVar[1].Assign(&result);
         result.SetNull();
	      break;
	   }
	   case(DMATRIX2D):
	   {
         itfc->retVar[1].Assign(&result);
         result.SetNull();
	      break;
	   }
	   case(CMATRIX2D):
	   {
         complex **cmat = result.GetCMatrix2D();
	   
         for(j = 0; j < result.GetDimY(); j++)
	      {
	         for(i = 0; i < result.GetDimX(); i++)
	         {
	            cmat[j][i].r = cmat[j][i].r;
	            cmat[j][i].i = -cmat[j][i].i;
	         }
	      }

         itfc->retVar[1].Assign(&result);
         result.SetNull();
	      break;
	   }	   	   	   
      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}
   return(OK);
}

int ExclusiveOr(Interface *itfc, char *args)
{
   short r;
   Variable var1,var2;

   if((r = ArgScan(itfc,args,2,"arg1, arg2","ee","vv",&var1,&var2)) < 0)
     return(r); 

   if(var1.GetType() == FLOAT32 && var2.GetType() == FLOAT32)
   {
      ULONG32 arg1 = nint(var1.GetReal());
      ULONG32 arg2 = nint(var2.GetReal());
      ULONG32 result = arg1 ^ arg2;
      itfc->retVar[1].MakeAndSetFloat(result);
      itfc->nrRetValues = 1;
   }
   else if(var1.GetType() == FLOAT64 && var2.GetType() == FLOAT64)
   {
      ULONG64 arg1 = nushint(var1.GetDouble());
      ULONG64  arg2 = nushint(var2.GetDouble());
      ULONG64  result = arg1 ^ arg2;
      itfc->retVar[1].MakeAndSetDouble(result);
      itfc->nrRetValues = 1;
   }
   else if(var1.GetType() == MATRIX2D && var2.GetType() == FLOAT32)
   {
      float** arg1 = var1.GetMatrix2D();
      ULONG32  arg2 = nint(var2.GetReal());
      int w = var1.GetDimX();
      int h = var1.GetDimY();
      float** result = MakeMatrix2D(w,h);

      for(int y = 0; y < h; y++)
         for(int x = 0; x < w; x++)
            result[y][x] = nint(arg1[y][x]) ^ arg2;
               
      itfc->retVar[1].AssignMatrix2D(result,w,h);
      itfc->nrRetValues = 1;
   }
   else if(var1.GetType() == DMATRIX2D && var2.GetType() == FLOAT64)
   {
      double** arg1 = var1.GetDMatrix2D();
      ULONG64  arg2 = nint(var2.GetDouble());
      int w = var1.GetDimX();
      int h = var1.GetDimY();
      double** result = MakeDMatrix2D(w,h);

      for(int y = 0; y < h; y++)
         for(int x = 0; x < w; x++)
            result[y][x] = nushint(arg1[y][x]) ^ arg2;
               
      itfc->retVar[1].AssignDMatrix2D(result,w,h);
      itfc->nrRetValues = 1;
   }
   else if(var1.GetType() == MATRIX2D && var2.GetType() == MATRIX2D)
   {
      float** arg1 = var1.GetMatrix2D();
      float** arg2 = var2.GetMatrix2D();
      int w1 = var1.GetDimX();
      int h1 = var1.GetDimY();
      int w2 = var2.GetDimX();
      int h2 = var2.GetDimY();
      if(w1 == w2 && h1 == h1)
      {
         float** result = MakeMatrix2D(w1,h1);

         for(int y = 0; y < h1; y++)
            for(int x = 0; x < w1; x++)
               result[y][x] = nint(arg1[y][x]) ^ nint(arg2[y][x]);
               
         itfc->retVar[1].AssignMatrix2D(result,w1,h1);
         itfc->nrRetValues = 1;
      }
      else
      {
         ErrorMessage("Matrix dimensions must match");
         return(ERR);
      }
   }
   else if(var1.GetType() == DMATRIX2D && var2.GetType() == DMATRIX2D)
   {
      double** arg1 = var1.GetDMatrix2D();
      double** arg2 = var2.GetDMatrix2D();
      int w1 = var1.GetDimX();
      int h1 = var1.GetDimY();
      int w2 = var2.GetDimX();
      int h2 = var2.GetDimY();
      if(w1 == w2 && h1 == h1)
      {
         double** result = MakeDMatrix2D(w1,h1);

         for(int y = 0; y < h1; y++)
            for(int x = 0; x < w1; x++)
               result[y][x] = nushint(arg1[y][x]) ^ nushint(arg2[y][x]);
               
         itfc->retVar[1].AssignDMatrix2D(result,w1,h1);
         itfc->nrRetValues = 1;
      }
      else
      {
         ErrorMessage("Matrix dimensions must match");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Invalid data types");
      return(ERR);
   }

  return(OK);

}


/******************************************************
 Converts a hexidecimal expression to its decimal format
 taking account of the number of bits

 Format:
		dec(number/vector[, bits])

******************************************************/

int ConvertToDecimal(Interface *itfc, char args[])
{
	short r;
	short bits = 32;
	CText data;
   signed __int64 result;
   short HexToNumber(char *s, short bits, signed __int64 &result);

	// Get the parameters
   if((r = ArgScan(itfc, args, 1, "hex number[, bits]", "ce", "td", &data, &bits)) < 0)
      return(r); 

   if(HexToNumber(data.Str(), bits, result) == ERR)
		return(ERR);

	itfc->retVar[1].MakeAndSetDouble(result);
	itfc->nrRetValues = 1;

	return(OK);
}


/*********************************************************************
            Converts a string in hex format to a number with 
				the sign based on the number of bits passed.
         
*********************************************************************/
  
short HexToNumber(char *s, short bits, signed __int64 &out)
{
   short len = strlen(s);
   long m;
   bool fac = false;
   long start = 0;
   short mantissaDigits = 0;
   short exponentDigits = 0;
   unsigned __int64 p16,p1,result;  

// Check to see if its a hexadecimal number 
   if(s[0] == '0' && len > 1 && (s[1] == 'x' || s[1] == 'X'))
   {
      result = 0;
      p16 = 1;
      p1 = 1;
      
      for(short i = len-1; i >= 2; i--)
      { 
         if(s[i] >= '0' && s[i] <= '9')
         {
            result += p16*(s[i] - '0');
         }
         else if(s[i] >= 'A' && s[i] <= 'F')
         {
            result += p16*(s[i] - 'A' + 10);
         }
         else if(s[i] >= 'a' && s[i] <= 'f')
         {
            result += p16*(s[i] - 'a' + 10);
         }  
			else
			{
				ErrorMessage("%s is not a valid hex number",s);
		      return(ERR);
			}
         p16 *= 16;
         mantissaDigits++;
      }

		if(result > p1<<bits)
		{
			ErrorMessage("Number is larger than 2^%d",bits);
		   return(ERR);
		}
		int n = bits/4;
		if(mantissaDigits >= n && s[len-n] >= '8')
			out = signed __int64(result) - signed __int64(p1<<bits);
		else
			out = result;

      return(OK);
	}
	else
	{
		ErrorMessage("Not a hex number");
		return(ERR);
	}
}

/******************************************************
 Converts an integer expression to its hex format
 as a string or string list (for a vector)

 Format:
		hex(number/vector[, bits[, signed/unsigned/s/u]])

 The default is 16 bits unsigned

******************************************************/

int HexConversion(Interface *itfc, char args[])
{
   short r;
   Variable var;
	CText output;
	CText sign = "unsigned";
   signed __int64 numIn;
   short bits = 16;
	bool _signed;
   int HexConversionCore(signed __int64 numIn,  short &bits, bool &_signed, CText &output);

	// Get the parameters
   if((r = ArgScan(itfc,args,1,"decimal number[, bits]","eee","vdt",&var, &bits,&sign)) < 0)
      return(r); 

	// Convert a number
	if(var.GetType() == FLOAT32 || var.GetType() == FLOAT64)
	{
		if(var.GetType() == FLOAT32)
		   numIn = nhint(var.GetReal());

		if(var.GetType() == FLOAT64)
			numIn = nhint(var.GetDouble());

		_signed = (sign == "signed" || sign == "s");

		if(HexConversionCore(numIn, bits, _signed, output) == ERR)
			return(ERR);

		 itfc->retVar[1].MakeAndSetString(output.Str());
	}
	// Convert a vector
   else if(var.GetType() == MATRIX2D)
   {
      if(var.GetDimY() == 1) //Row vector
      {
         long dimx = var.GetDimX();
         char **list = NULL;
         float **mat = var.GetMatrix2D();

         for(int i = 0; i < dimx; i++)
         {
				numIn = mat[0][i];
				_signed = (sign == "signed" || sign == "s");
				if(HexConversionCore(numIn, bits, _signed, output) == ERR)
					return(ERR);
            InsertStringIntoList(output.Str(),&list,i,i);
         }
         itfc->retVar[1].MakeAndSetList(list,dimx);
         itfc->nrRetValues = 1;
	      FreeList(list,dimx);
      }
      else if(var.GetDimX() == 1) // Column vector
      {
         long dimy = var.GetDimY();
         char **list = NULL;
         float **mat = var.GetMatrix2D();

         for(int i = 0; i < dimy; i++)
         {
				numIn = mat[i][0];
				_signed = (sign == "signed" || sign == "s");
				if(HexConversionCore(numIn, bits, _signed, output) == ERR)
					return(ERR);
            InsertStringIntoList(output.Str(),&list,i,i);
         }
         itfc->retVar[1].MakeAndSetList(list,dimy);
         itfc->nrRetValues = 1;
	      FreeList(list,dimy);
      }
		else
		{
			long dimx = var.GetDimX();
			long dimy = var.GetDimY();
         List2DData* list = Make2DList(dimx, dimy);
         float **mat = var.GetMatrix2D();
         for(int j = 0; j < dimy; j++)
         {
				for(int i = 0; i < dimx; i++)
				{
				    numIn = mat[j][i];
					_signed = (sign == "signed" || sign == "s");
				   if(HexConversionCore(numIn, bits, _signed, output) == ERR)
					   return(ERR);
               list->AddEntry(output.Str(),i,j);
				}
			}
         itfc->retVar[1].Assign2DList(list);
      }
   }
   else if (var.GetType() == DMATRIX2D)
   {
      if (var.GetDimY() == 1) //Row vector
      {
         long dimx = var.GetDimX();
         char** list = NULL;
         double** mat = var.GetDMatrix2D();

         for (int i = 0; i < dimx; i++)
         {
            numIn = mat[0][i];
            _signed = (sign == "signed" || sign == "s");
            if (HexConversionCore(numIn, bits, _signed, output) == ERR)
               return(ERR);
            InsertStringIntoList(output.Str(), &list, i, i);
         }
         itfc->retVar[1].MakeAndSetList(list, dimx);
         itfc->nrRetValues = 1;
         FreeList(list, dimx);
      }
      else if (var.GetDimX() == 1) // Column vector
      {
         long dimy = var.GetDimY();
         char** list = NULL;
         double** mat = var.GetDMatrix2D();

         for (int i = 0; i < dimy; i++)
         {
            numIn = mat[i][0];
            _signed = (sign == "signed" || sign == "s");
            if (HexConversionCore(numIn, bits, _signed, output) == ERR)
               return(ERR);
            InsertStringIntoList(output.Str(), &list, i, i);
         }
         itfc->retVar[1].MakeAndSetList(list, dimy);
         itfc->nrRetValues = 1;
         FreeList(list, dimy);
      }
      else
      {
         long dimx = var.GetDimX();
         long dimy = var.GetDimY();
         List2DData* list = Make2DList(dimx, dimy);
         double** mat = var.GetDMatrix2D();
         for (int j = 0; j < dimy; j++)
         {
            for (int i = 0; i < dimx; i++)
            {
               numIn = mat[j][i];
               _signed = (sign == "signed" || sign == "s");
               if (HexConversionCore(numIn, bits, _signed, output) == ERR)
                  return(ERR);
               list->AddEntry(output.Str(), i, j);
            }
         }
         itfc->retVar[1].Assign2DList(list);
      }
   }
   else
   {
      ErrorMessage("invalid type for hex conversion");
      return(ERR);
   }

//	itfc->retVar[2].MakeAndSetFloat(bits);
	itfc->nrRetValues = 1;
   return(OK);
}

int HexConversionCore(signed __int64 numIn,  short &bits, bool &_signed, CText &output)
{
	if(bits%4 != 0)
	{
		ErrorMessage("Number of bits must be divisible by 4");
		return(ERR);
	}

	unsigned __int64 num, one, base16;
	one = 1;
	base16 = 16;
	int value;

	if(numIn < 0)
		_signed = 1;

	if(_signed == 0)
	{
		while(abs(numIn) > (one<<bits)-1)
		{
			bits = bits*2;
			if(bits > 64)
			{
				ErrorMessage("More than 64 bits in number ");
				return(ERR);
			}
		//	TextMessage("\n   Number of bits used in hex conversion increased from %d to %d\n",bits/2,bits);
		}
	}
	else
	{
		while(abs(numIn) > one<<(bits-1))
		{
			bits = bits*2;
			if(bits > 64)
			{
				ErrorMessage("More than 64 bits in number ");
				return(ERR);
			}
			//TextMessage("\n   Number of bits used in hex conversion increased from %d to %d\n",bits/2,bits);
		}
	}

   if(numIn < 0)
	{
		if(!_signed)
		{
			ErrorMessage("Data should be unsigned");
			return(ERR);
		}
		if(numIn < (-one<<(bits-1)))
		{
			ErrorMessage("Data to be converted is more negative than -2^%d",bits-1);
			return(ERR);
		}
	   num = (unsigned __int64)((signed __int64)(one<<bits) + numIn);
	}
	else
	{
		num = numIn;
		if(_signed)
		{
			if(num >= one<<(bits-1))
			{
				ErrorMessage("Data to be converted is larger than 2^%d-1",bits-1);
				return(ERR);
			}
		}
		else
		{
			if(num > (one<<bits)-1)
			{
				ErrorMessage("Data to be converted is larger than 2^%d-1",bits);
				return(ERR);
			}
		}
	}
	char str[20];
	int steps = bits/4-1;
	output = "";
	unsigned __int64 factor = 1;
   factor = factor<<(steps*4);
	for(int i = steps; i >= 0; i--)
	{
		value = num / factor;
		num = num - value*factor;
		sprintf(str,"%X",value);
		output = output + str;
		factor = factor>>4;
	}

	return(OK);
}

/******************************************************
   Extract some part of a 1D or 2D matrix or a string

   Syntax: result = submatrix(str,lx,rx)
           result = submatrix(vec,lx,rx)
           result = submatrix(mat2d,lx,rx,ly,ry)
           result = submatrix(mat3d,lx,rx,ly,ry,lz,rz)
           result = submatrix(mat4d,lx,rx,ly,ry,lz,rz,lq,rq)

   lx, rx etc are left and right (start and end) matrix indices.
   Returns an error if indices are out of range.

   Last modified 6/3/06 CDE
******************************************************/

int ExtractSubMatrix(Interface *itfc, char args[])
{
   short r;
   long lx,ux,ly,uy,lz,uz,lq,uq;
   long x,y,z,q;
   Variable data;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Initialise limits
   lx = ux = ly = uy = lz = uz = lq = uq = 0;  

// Get arguments from user
   if((r = ArgScan(itfc,args,3,"matrix, lx, ux,[ly, uy, [lz, uz, [lq, uq]]]","eeeeeeeee","vllllllll",&data,&lx,&ux,&ly,&uy,&lz,&uz,&lq,&uq)) < 0)
      return(r); 

// Check for invalid submatrix range
   if(lx > ux || ly > uy || lz > uz || lq > uq || lx < 0 || ly < 0 || lz < 0 || lq < 0 || uq < 0)
   {
      ErrorMessage("invalid submatrix range");
      return(ERR);
   }

// Calculate new submatrix dimensions
   long wx = ux-lx+1;
   long wy = uy-ly+1;
   long wz = uz-lz+1;
   long wq = uq-lq+1;
   
// Extract submatrix based on data type     
   switch(VarType(&data))
   {
      case(UNQUOTED_STRING):
      {
         char *str,*subStr;
			str = data.GetString();
			if(ux >= strlen(str))
			{
			   ErrorMessage("invalid submatrix range");
			   return(ERR);
			}

			subStr = new char[wx+1];

			for(x = lx; x <= ux; x++)
			   subStr[x-lx] = str[x];
         subStr[x-lx] = '\0';

			ans->MakeAndSetString(subStr);
         delete [] subStr;
			break;
      }

      case(MATRIX2D):
      {
         float **mat,**subMat;
			mat = data.GetMatrix2D();
			if(ux >= data.GetDimX() || uy >= data.GetDimY())
			{
			   ErrorMessage("invalid submatrix range");
			   return(ERR);
			}

			subMat = MakeMatrix2D(wx,wy);

			for(y = ly; y <= uy; y++)
			   for(x = lx; x <= ux; x++)
			      subMat[y-ly][x-lx] = mat[y][x];

			ans->AssignMatrix2D(subMat,wx,wy);
			break;
      }


      case(DMATRIX2D):
      {
         double **mat,**subMat;
			mat = data.GetDMatrix2D();
			if(ux >= data.GetDimX() || uy >= data.GetDimY())
			{
			   ErrorMessage("invalid submatrix range");
			   return(ERR);
			}

			subMat = MakeDMatrix2D(wx,wy);

			for(y = ly; y <= uy; y++)
			   for(x = lx; x <= ux; x++)
			      subMat[y-ly][x-lx] = mat[y][x];

			ans->AssignDMatrix2D(subMat,wx,wy);
			break;
      }

      case(CMATRIX2D):
      {
         complex **cMat,**cSubMat;
			cMat = data.GetCMatrix2D();
			if(ux >= data.GetDimX() || uy >= data.GetDimY())
			{
			   ErrorMessage("invalid submatrix range");
			   return(ERR);
			}

			cSubMat = MakeCMatrix2D(wx,wy);
			
			for(y = ly; y <= uy; y++)
			   for(x = lx; x <= ux; x++)
			      cSubMat[y-ly][x-lx] = cMat[y][x];

	      ans->AssignCMatrix2D(cSubMat,wx,wy);
	      break;
      }

      case(MATRIX3D):
      {
         float ***mat,***subMat;

			mat = data.GetMatrix3D();
			if(ux >= data.GetDimX() || uy >= data.GetDimY() || uz >= data.GetDimZ())
			{
			   ErrorMessage("invalid submatrix range");
			   return(ERR);
			}

			subMat = MakeMatrix3D(wx,wy,wz);

			for(z = lz; z <= uz; z++)
			   for(y = ly; y <= uy; y++)
			      for(x = lx; x <= ux; x++)
			         subMat[z-lz][y-ly][x-lx] = mat[z][y][x];

			ans->AssignMatrix3D(subMat,wx,wy,wz);
			break;
      }

      case(CMATRIX3D):
      {
         complex ***mat,***subMat;   
			mat = data.GetCMatrix3D();
			if(ux >= data.GetDimX() || uy >= data.GetDimY() || uz >= data.GetDimZ())
			{
			   ErrorMessage("invalid submatrix range");
			   return(ERR);
			}

			subMat = MakeCMatrix3D(wx,wy,wz);
			
			for(z = lz; z <= uz; z++)
			   for(y = ly; y <= uy; y++)
			      for(x = lx; x <= ux; x++)
			         subMat[z-lz][y-ly][x-lx] = mat[z][y][x];

	      ans->AssignCMatrix3D(subMat,wx,wy,wz);
	      break;
      }

      case(MATRIX4D):
      {
         float ****mat,****subMat;

			mat = data.GetMatrix4D();
			if(ux >= data.GetDimX() || uy >= data.GetDimY() || uz >= data.GetDimZ() || uq >= data.GetDimQ())
			{
			   ErrorMessage("invalid submatrix range");
			   return(ERR);
			}

			subMat = MakeMatrix4D(wx,wy,wz,wq);

			for(q = lq; q <= uq; q++)
			   for(z = lz; z <= uz; z++)
			      for(y = ly; y <= uy; y++)
			         for(x = lx; x <= ux; x++)
			            subMat[q-lq][z-lz][y-ly][x-lx] = mat[q][z][y][x];

			ans->AssignMatrix4D(subMat,wx,wy,wz,wq);
			break;
      }

      case(CMATRIX4D):
      {
         complex ****mat,****subMat;

			mat = data.GetCMatrix4D();
			if(ux >= data.GetDimX() || uy >= data.GetDimY() || uz >= data.GetDimZ() || uq >= data.GetDimQ())
			{
			   ErrorMessage("invalid submatrix range");
			   return(ERR);
			}

			subMat = MakeCMatrix4D(wx,wy,wz,wq);

			for(q = lq; q <= uq; q++)
			   for(z = lz; z <= uz; z++)
			      for(y = ly; y <= uy; y++)
			         for(x = lx; x <= ux; x++)
			            subMat[q-lq][z-lz][y-ly][x-lx] = mat[q][z][y][x];

			ans->AssignCMatrix4D(subMat,wx,wy,wz,wq);
			break;
      }

      default:
      {
         ErrorMessage("invalid argument type");
         return(ERR);
      }
   } 
   return(OK);    
}


/******************************************************
   Round to dat to nearest integer
******************************************************/

float roundBase(float dat)
{
   if(dat > 0)
      return((long)(dat+0.5));
   else
      return((long)(dat-0.5));
}


double roundBase(double dat)
{
   if(dat > 0)
      return((long)(dat+0.5));
   else
      return((long)(dat-0.5));
}

/******************************************************
   Take base 2 logarithm of dat
******************************************************/

float log2Base(float dat)
{
    return(log10(dat)/0.30102999566);
}

double log2Base(double dat)
{
    return(log10(dat)/0.30102999566);
}
/*****************************************************************************************
*                           Return the nearest integer to the float num                  *
*****************************************************************************************/

long nint(float num)
{
   if(num > 0)
      return((long)(num+0.5));
   else
      return((long)(num-0.5));
   
}


/*******************************************************************************
                       Return nearest huge long unsigned integer
*******************************************************************************/

unsigned __int64 nushint(float num)
{
   if(num > 0)
      return((unsigned __int64)(num+0.5));
   else
      return(0);   
}

unsigned __int64 nushint(double num)
{
   if(num > 0)
      return((unsigned __int64)(num+0.5));
   else
      return(0);   
}


/*******************************************************************************
                       Return nearest huge long signed integer
*******************************************************************************/

signed __int64 nhint(float num)
{
   if(num > 0)
      return((signed __int64)(num+0.5));
   else
      return((signed __int64)(num-0.5));
}

/*******************************************************************************
                       Return nearest huge long signed integer
*******************************************************************************/

signed __int64 nhint(double num)
{
   if(num > 0)
      return((signed __int64)(num+0.5));
   else
      return((signed __int64)(num-0.5));
}


/*****************************************************************************************
*             Return the nearest integer with magnitude not less than the float num      *
*****************************************************************************************/

long hint(float num)
{
   if(num > 0)
      return((long)(num+0.9999));
   else
      return((long)(num-0.9999));
}


short nsint(float num)
{
   if(num > 0)
      return((short)(num+0.5));
   else
      return((short)(num-0.5));
   
}

float nintf(float num)
{
   if(num > 0)
      return((long)(num+0.5));
   else
      return((long)(num-0.5));
   
}


double nintd(double num)
{
   if(num > 0)
      return((__int64)(num+0.5));
   else
      return((__int64)(num-0.5));
   
}

//float truncf(float num)
//{
//   return((long)(num));
//}

double truncd(double num)
{
   return((__int64)(num));
}

float fracf(float num)
{
   if(num >= 0)
      return(num - (long)(num));
   else
      return(-num + (long)(num));
}

double fracd(double num)
{
   if(num >= 0)
      return(num - (long)(num));
   else
      return(-num + (long)(num));
}

float notf(float num)
{
   return(!(long)(num));
}

double notd(double num)
{
   return(!(long)(num));
}

float invf(float num)
{
   return(~(long)(num));
}

double invd(double num)
{
   return(~(long)(num));
}


float sqr(float num)
{
   return(num*num);
}

float signf(float num)
{
   if(num >= 0)
      return(1.0);
   else
      return(-1.0);   
}


double signd(double num)
{
   if(num >= 0)
      return((__int64)(1.0));
   else
      return((__int64)(-1.0));
   
}

/*******************************************************************************
  Zero fill a 1D real or complex data set - either at the start, end or the sides
  The input data can be a row or column vector - the output will be the same.
********************************************************************************/

short FillVectorSides(Variable *var, long fill, complex value, Variable *ans);
short FillVectorStart(Variable *var, long fill, complex value, Variable *ans);
short FillVectorEnd(Variable *var, long fill, complex value, Variable *ans);

int FillVector(Interface *itfc, char *arg)
{
   short r;          // Number of arguments
   CText mode;       // Type of filling
   long fill;        // Length of new matrix
   complex value;    // Imaginary value to FillVector with
   Variable var;     // Variable containing matrix to fill
   Variable fillVar; // Variable holding fill value

// Initialize value
   value.r = 0;
   value.i = 0;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Extract variable to fill, fill length and mode ("side" or "end")  
	if((r = ArgScan(itfc,arg,3,"vector,size,mode,[value]","eeee","vltv",&var,&fill,&mode,&fillVar)) < 0)
	  return(r); 

// Extract fill value if present
   if(r == 4)
   {
      if(VarType(&fillVar) == COMPLEX)
      {
         value = VarComplex(&fillVar);
      }
      else if(VarType(&fillVar) == FLOAT32)
      {
         value.r = VarReal(&fillVar);
         value.i = 0;
      }
      else
      {
         ErrorMessage("invalid fill value");
         return(ERR);
      }
   }

// Add zeros to start, end or sides depending on mode
   if(mode == "sides") 
      FillVectorSides(&var,fill,value,ans);	
   else if(mode == "end") 
      FillVectorEnd(&var,fill,value,ans); 
   else if(mode == "start") 
      FillVectorStart(&var,fill,value,ans);
   else
   {
      ErrorMessage("invalid mode");
      return(ERR);
   }
 
   return(OK);
}


/*******************************************************************************
  Add zeros to either side of matrix var so that its total length is "fill"
********************************************************************************/

short FillVectorSides(Variable *var, long fill, complex value, Variable *ans)
{
   long start,end,x;
   long width = VarWidth(var);
   long height = VarHeight(var);

// Real matrix fill ****************
	if(VarType(var) == MATRIX2D)
	{
	   if(width > 1 && height == 1) // Row matrix
	   {
		   float **inmat = VarRealMatrix(var);
	      if(fill < width)
	      {
	         ErrorMessage("fill size must be greater than matrix size");
	         return(ERR);
	      }		   
		   float **outmat = MakeMatrix2D(fill,1L);

	      start = (fill-width)/2;
	      end = start+width;
   	   
	      for(x = 0; x < start; x++)
	         outmat[0][x] = value.r;
   
	      for(x = start; x < end; x++)
	         outmat[0][x] = inmat[0][x-start];
   	   
	      for(x = end; x < fill; x++)
	         outmat[0][x] = value.r;
   	   
	      ans->AssignMatrix2D(outmat,fill,1L);  
      }
      else if(width == 1 && height > 1) // Column matrix
	   {
		   float **inmat = VarRealMatrix(var);
	      if(fill < height)
	      {
	         ErrorMessage("fill size must be greater than matrix size");
	         return(ERR);
	      }		   
		   float **outmat = MakeMatrix2D(1L,fill);

	      start = (fill-height)/2;
	      end = start+height;
   	   
	      for(x = 0; x < start; x++)
	         outmat[x][0] = value.r;
   
	      for(x = start; x < end; x++)
	         outmat[x][0] = inmat[x-start][0];
   	   
	      for(x = end; x < fill; x++)
	         outmat[x][0] = value.r;
   	   
	      ans->AssignMatrix2D(outmat,1,fill);  
      }
		else
		{
		   ErrorMessage("must be a row or column matrix");
		   return(ERR);
		}
	}
	
// Complex matrix fill ****************
	else if(VarType(var) == CMATRIX2D)
	{
	   if(width > 1 && height == 1) // Row matrix
	   {
		   complex **inmat = VarComplexMatrix(var);
	      if(fill < width)
	      {
	         ErrorMessage("fill size must be greater than matrix size");
	         return(ERR);
	      }		   
		   complex **outmat = MakeCMatrix2D(fill,1);
   	   
	      start = (fill-width)/2;
	      end = start+width;
   	   
	      for(x = 0; x < start; x++)
	      {
	         outmat[0][x].r = value.r;
	         outmat[0][x].i = value.i;
	      }
	      for(x = start; x < end; x++)
	      {
	         outmat[0][x].r = inmat[0][x-start].r;
	         outmat[0][x].i = inmat[0][x-start].i;
	      }
	      for(x = end; x < fill; x++)
	      {
	         outmat[0][x].r = value.r;
	         outmat[0][x].i = value.i;
	      } 
	      ans->AssignCMatrix2D(outmat,fill,1);  
      }
      else if(width == 1 && height > 1) // Column matrix
	   {
		   complex **inmat = VarComplexMatrix(var);
	      if(fill < height)
	      {
	         ErrorMessage("fill size must be greater than matrix size");
	         return(ERR);
	      }		   
		   complex **outmat = MakeCMatrix2D(1,fill);
   	   
	      start = (fill-height)/2;
	      end = start+height;
   	   
	      for(x = 0; x < start; x++)
	      {
	         outmat[x][0].r = value.r;
	         outmat[x][0].i = value.i;
	      }
	      for(x = start; x < end; x++)
	      {
	         outmat[x][0] = inmat[x-start][0];
	      }  	   
	      for(x = end; x < fill; x++)
	      {
	         outmat[x][0].r = value.r;
	         outmat[x][0].i = value.i;
	      } 
	      ans->AssignCMatrix2D(outmat,1,fill); 
      }
		else
		{
		   ErrorMessage("must be a row or column matrix");
		   return(ERR);
		}
	}
   else
	{
		ErrorMessage("must be a row or column matrix");
		return(ERR);
	}
   return(OK);
}

/*******************************************************************************
  Add zeros to the end of matrix var so that its total length is "fill"
********************************************************************************/

short FillVectorEnd(Variable *var, long fill, complex value, Variable *ans)
{
   long x;
   long width = VarWidth(var);
   long height = VarHeight(var);

	if(VarType(var) == MATRIX2D)
	{
	   if(width > 1 && height == 1)
	   {
			float **inmat = VarRealMatrix(var);
		   if(fill < width)
		   {
		      ErrorMessage("fill size must be greater than matrix size");
		      return(ERR);
		   }		   
			float **outmat = MakeMatrix2D(fill,1L);
			   
		   for(x = 0; x < width; x++)
		      outmat[0][x] = inmat[0][x];
	
		   for(x = width; x < fill; x++)
		      outmat[0][x] = value.r;

		   ans->AssignMatrix2D(outmat,fill,1L);  
		}
		else if(width == 1 && height > 1)
		{
			float **inmat = VarRealMatrix(var);
		   if(fill < height)
		   {
		      ErrorMessage("fill size must be greater than matrix size");
		      return(ERR);
		   }		   
			float **outmat = MakeMatrix2D(1L,fill);
			   
		   for(x = 0; x < height; x++)
		      outmat[x][0] = inmat[x][0];
	
		   for(x = height; x < fill; x++)
		      outmat[x][0] = value.r;

		   ans->AssignMatrix2D(outmat,1,fill);  
		}
		else
		{
		   ErrorMessage("must be a row or column matrix");
		   return(ERR);
		}		   
	}
		
	else if(VarType(var) == CMATRIX2D)
	{
	   if(width > 1 && height == 1)
	   {	   
			complex **inmat = VarComplexMatrix(var);

		   if(fill < width)
		   {
		      ErrorMessage("fill size must be greater than matrix size");
		      return(ERR);
		   }		   
			complex **outmat = MakeCMatrix2D(fill,1);
		   
		   for(x = 0; x < width; x++)
		      outmat[0][x] = inmat[0][x];
	
		   for(x = width; x < fill; x++)
		   {
		      outmat[0][x].r = value.r;
		      outmat[0][x].i = value.i;
		   }
		   ans->AssignCMatrix2D(outmat,fill,1);  
		}
		else if(width == 1 && height > 1)
	   {	   
			complex **inmat = VarComplexMatrix(var);
		   if(fill < height)
		   {
		      ErrorMessage("fill size must be greater than matrix size");
		      return(ERR);
		   }		   
			complex **outmat = MakeCMatrix2D(1,fill);
		   
		   for(x = 0; x < height; x++)
		      outmat[x][0] = inmat[x][0];
	
		   for(x = height; x < fill; x++)
		   {
		      outmat[x][0].r = value.r;
		      outmat[x][0].i = value.i;
		   }
		   ans->AssignCMatrix2D(outmat,1,fill);  
		}	
		else
		{
		   ErrorMessage("must be a row or column matrix");
		   return(ERR);
		}
	}
   return(OK);
}

	 
/*******************************************************************************
  Add zeros to the start of matrix "var" so that its total length is "fill"
********************************************************************************/

short FillVectorStart(Variable *var, long fill, complex value, Variable *ans)
{
   long x;
   long width = VarWidth(var);
   long height = VarHeight(var);

	if(VarType(var) == MATRIX2D)
	{
	   if(width > 1 && height == 1)
	   {	
		   float **inmat  = VarRealMatrix(var);
	      if(fill < width)
	      {
	         ErrorMessage("fill size must be greater than matrix size");
	         return(ERR);
	      }		   
		   float **outmat = MakeMatrix2D(fill,1L);
   		   
	      for(x = 0; x < fill-width; x++)
	      {
	         outmat[0][x] = value.r;
	      }

	      for(x = fill-width; x < fill; x++)
	      {
	         outmat[0][x] = inmat[0][x-fill+width];
	      }
	      ans->AssignMatrix2D(outmat,fill,1L);  
      }
		else if(width == 1 && height > 1)
	   {
		   float **inmat  = VarRealMatrix(var);
	      if(fill < height)
	      {
	         ErrorMessage("fill size must be greater than matrix size");
	         return(ERR);
	      }		   
		   float **outmat = MakeMatrix2D(1L,fill);
   		   
	      for(x = 0; x < fill-height; x++)
	      {
	         outmat[x][0] = value.r;
	      }

	      for(x = fill-height; x < fill; x++)
	      {
	         outmat[x][0] = inmat[x-fill+height][0];
	      }
	      ans->AssignMatrix2D(outmat,1L,fill);  
      }
	}
		
	else if(VarType(var) == CMATRIX2D)
	{
	   if(width > 1 && height == 1)
	   {	
		   complex **inmat = VarComplexMatrix(var);
	      if(fill < width)
	      {
	         ErrorMessage("fill size must be greater than matrix size");
	         return(ERR);
	      }		   
		   complex **outmat = MakeCMatrix2D(fill,1);
   	   
	      for(x = 0; x < fill-width; x++)
	      {
	         outmat[0][x].r = value.r;
	         outmat[0][x].i = value.i;
	      }

	      for(x = fill-width; x < fill; x++)
	      {
	         outmat[0][x] = inmat[0][x-fill+width];
	      }
	      ans->AssignCMatrix2D(outmat,fill,1);  
      }
      else if(width == 1 && height > 1)
      {
		   complex **inmat = VarComplexMatrix(var);
	      if(fill < height)
	      {
	         ErrorMessage("fill size must be greater than matrix size");
	         return(ERR);
	      }		   
		   complex **outmat = MakeCMatrix2D(1,fill);
   	   
	      for(x = 0; x < fill-height; x++)
	      {
	         outmat[x][0].r = value.r;
	         outmat[x][0].i = value.i;
	      }

	      for(x = fill-height; x < fill; x++)
	      {
	         outmat[x][0] = inmat[x-fill+height][0];
	      }
	      ans->AssignCMatrix2D(outmat,1,fill); 
      }
	}
   return(OK);
}

/*******************************************************************************
                   Take the magnitude of a scalar or matrix    
********************************************************************************/

int Magnitude(Interface* itfc ,char args[])
{
   long x,y,z,q;
   short r;
   Variable var;

   Variable* retVar = itfc->retVar;

// Get argument *****************************
	if((r = ArgScan(itfc,args,1,"number/matrix","e","v",&var)) < 0)
	   return(r); 
      
// Take magnitude *************************
   switch(var.GetType())
   {
   	case(FLOAT32):
	   {
	      retVar[1].MakeAndSetFloat(fabs(var.GetReal())); 
	      break;
	   }

   	case(FLOAT64):
	   {
	      retVar[1].MakeAndSetDouble(fabs(var.GetDouble())); 
	      break;
	   }

   	case(COMPLEX):
	   {
	      complex cnum = var.GetComplex();
	      float magnitude = sqrt(cnum.r*cnum.r + cnum.i*cnum.i);
	      retVar[1].MakeAndSetFloat(magnitude); 
	      break;
	   }
	     
	   case(MATRIX2D):
	   {
	      float **mat = var.GetMatrix2D();
	      long width = var.GetDimX();
	      long height = var.GetDimY();
		  
	      for(y = 0; y < height; y++)
	         for(x = 0; x < width; x++)
	            mat[y][x] = fabs(mat[y][x]);

	      retVar[1].MakeAndLoadMatrix2D(mat,width,height);

	      break;
	   }

	   case(DMATRIX2D):
	   {
	      double **mat = var.GetDMatrix2D();
	      long width = var.GetDimX();
	      long height = var.GetDimY();
		  
	      for(y = 0; y < height; y++)
	         for(x = 0; x < width; x++)
	            mat[y][x] = fabs(mat[y][x]);

	      retVar[1].MakeAndLoadDMatrix2D(mat,width,height);

	      break;
	   }

	   case(CMATRIX2D):
	   {
	      complex **cmat = var.GetCMatrix2D();
	      long width =  var.GetDimX();
	      long height = var.GetDimY();
		 
		   float **mat = MakeMatrix2D(width,height);
		   if(!mat) 
		   {
		      ErrorMessage("can't allocate memory for 'magnitude' result");
		      return(ERR);
		   }
		   	      
	      for(y = 0; y < height; y++)
	         for(x = 0; x < width; x++)
	            mat[y][x] = sqrt(cmat[y][x].r*cmat[y][x].r + cmat[y][x].i*cmat[y][x].i);

	      retVar[1].AssignMatrix2D(mat,width,height);

	      break;
	   }	 

	   case(MATRIX3D):
	   {
	      float ***mat = var.GetMatrix3D();
	      long width  = var.GetDimX();
	      long height = var.GetDimY();
	      long depth  = var.GetDimZ();
		  
	      for(z = 0; z < depth; z++)
	         for(y = 0; y < height; y++)
	            for(x = 0; x < width; x++)
	               mat[z][y][x] = fabs(mat[z][y][x]);

	      retVar[1].MakeAndLoadMatrix3D(mat,width,height,depth);

	      break;
	   }

	   case(CMATRIX3D):
	   {
	      complex ***cmat = var.GetCMatrix3D();
	      long width  = var.GetDimX();
	      long height = var.GetDimY();
	      long depth  = var.GetDimZ();
		  
		   float ***mat = MakeMatrix3D(width,height,depth);
		   if(!mat) 
		   {
		      ErrorMessage("can't allocate memory for 'magnitude' result");
		      return(ERR);
		   }

	      for(z = 0; z < depth; z++)
	         for(y = 0; y < height; y++)
	            for(x = 0; x < width; x++)
	               mat[z][y][x] = sqrt(cmat[z][y][x].r*cmat[z][y][x].r + cmat[z][y][x].i*cmat[z][y][x].i);

	      retVar[1].AssignMatrix3D(mat,width,height,depth);

	      break;
	   }

      case(MATRIX4D):
	   {
	      float ****mat = var.GetMatrix4D();
	      long width  = var.GetDimX();
	      long height = var.GetDimY();
	      long depth  = var.GetDimZ();
	      long hyper  = var.GetDimQ();
		  
	      for(q = 0; q < hyper; q++)
	         for(z = 0; z < depth; z++)
	            for(y = 0; y < height; y++)
	               for(x = 0; x < width; x++)
	                  mat[q][z][y][x] = fabs(mat[q][z][y][x]);

	      retVar[1].MakeAndLoadMatrix4D(mat,width,height,depth,hyper);

	      break;
	   }

	   case(CMATRIX4D):
	   {
	      complex ****cmat = var.GetCMatrix4D();
	      long width  = var.GetDimX();
	      long height = var.GetDimY();
	      long depth  = var.GetDimZ();
	      long hyper  = var.GetDimQ();
		  
		   float ****mat = MakeMatrix4D(width,height,depth,hyper);
		   if(!mat) 
		   {
		      ErrorMessage("can't allocate memory for 'magnitude' result");
		      return(ERR);
		   }

	      for(q = 0; q < hyper; q++)
	         for(z = 0; z < depth; z++)
	            for(y = 0; y < height; y++)
	               for(x = 0; x < width; x++)
	                  mat[q][z][y][x] = sqrt(cmat[q][z][y][x].r*cmat[q][z][y][x].r + cmat[q][z][y][x].i*cmat[q][z][y][x].i);

	      retVar[1].AssignMatrix4D(mat,width,height,depth,hyper);

	      break;
	   }

      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}

   itfc->nrRetValues = 1;
   itfc->nrRetValues = 1;

	return(OK);
}


/*******************************************************************************
                   Unpack a real data set and amke it complex    
********************************************************************************/

int RealToComplex(Interface *itfc, char arg[])
{
   long x;
   short r;
   Variable var;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get argument *****************************
	if((r = ArgScan(itfc,arg,1,"matrix","e","v",&var)) < 0)
	   return(r); 
      
// Take magnitude *************************
   switch(var.GetType())
   {
	   case(MATRIX2D):
	   {
	      float **mat = var.GetMatrix2D();

	      long width = var.GetDimX();
	      long height = var.GetDimY();

         if(height > 1)
         {
            ErrorMessage("row vector expected");
            return(ERR);
         }

         if(width%2 != 0)
         {
            ErrorMessage("even number of vector elements expected");
            return(ERR);
         }
		
		   complex **cmat = MakeCMatrix2D(width/2,height);

	      for(x = 0; x < width; x+=2)
         {
	         cmat[0][x/2].r = mat[0][x];
	         cmat[0][x/2].i = mat[0][x+1];
         }

	      ans->AssignCMatrix2D(cmat,width/2,height);

	      break;
	   }

      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}

   itfc->nrRetValues = 1;

	return(OK);
}




/*******************************************************************************
                   Pack a complex data set into a real one    
********************************************************************************/

int ComplexToReal(Interface *itfc, char arg[])
{
   long x;
   short r;
   Variable var;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get argument *****************************
	if((r = ArgScan(itfc,arg,1,"complex matrix","e","v",&var)) < 0)
	   return(r); 
      
// Take magnitude *************************
   switch(var.GetType())
   {
	   case(CMATRIX2D):
	   {
	      complex **cmat = var.GetCMatrix2D();

	      long width = var.GetDimX();
	      long height = var.GetDimY();

         if(height > 1)
         {
            ErrorMessage("row vector expected");
            return(ERR);
         }

         //if(width%2 != 0)
         //{
         //   ErrorMessage("even number of vector elements expected");
         //   return(ERR);
         //}
		
		   float **mat = MakeMatrix2D(width*2,height);

	      for(x = 0; x < width; x++)
         {
	         mat[0][x*2] = cmat[0][x].r;
	         mat[0][x*2+1] = cmat[0][x].i;
         }

	      ans->AssignMatrix2D(mat,width*2,height);

	      break;
	   }

      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}

   itfc->nrRetValues = 1;

	return(OK);
}

/*******************************************************************************
*        Calculate the phase of a complex number or complex matrix             *
********************************************************************************/

float GetPhase(complex cnum);

int Phase(Interface *itfc, char *arg)
{
   short r;
   Variable var;
   complex cnum;

// Get complex number or matrix *****************************
	if((r = ArgScan(itfc,arg,1,"cnum/cmatrix","e","v",&var)) < 0)
	   return(r); 

// Calculate the phase angle ***************

   switch(VarType(&var))
   {
   	case(COMPLEX):
	   {
	      cnum = VarComplex(&var);
	      itfc->retVar[1].MakeAndSetFloat(GetPhase(cnum)); 
         itfc->nrRetValues = 1;
	      break;
	   }
	      
	   case(CMATRIX2D):
	   {
	      complex **cmat = VarComplexMatrix(&var);
	      long rows = VarRowSize(&var);
	      long cols = VarColSize(&var);
		 
		   float **mat = MakeMatrix2D(cols,rows);
		   if(!mat) 
		   {
		      ErrorMessage("can't allocate memory for 'phase' result");
		      return(ERR);
		   }
		   	      
	      for(long j = 0; j < rows; j++)
	         for(long i = 0; i < cols; i++)
               mat[j][i] = GetPhase(cmat[j][i]);
	      
	      itfc->retVar[1].AssignMatrix2D(mat,cols,rows);
         itfc->nrRetValues = 1;
	      break;
	   }


	   case(CMATRIX3D):
	   {
         complex ***cmat = var.GetCMatrix3D();
	      long width = var.GetDimX();
	      long height = var.GetDimY();
	      long depth = var.GetDimZ();
		 
		   float ***mat = MakeMatrix3D(width,height,depth);
		   if(!mat) 
		   {
		      ErrorMessage("can't allocate memory for 'phase' result");
		      return(ERR);
		   }

	      for(long k = 0; k < depth; k++)
	         for(long j = 0; j < height; j++)
	            for(long i = 0; i < width; i++)
                  mat[k][j][i] = GetPhase(cmat[k][j][i]);
	
	      
	      itfc->retVar[1].AssignMatrix3D(mat,width,height,depth);
         itfc->nrRetValues = 1;
	      break;
	   }
	   	 
      case(CMATRIX4D):
	   {
         complex ****cmat = var.GetCMatrix4D();
	      long width = var.GetDimX();
	      long height = var.GetDimY();
	      long depth = var.GetDimZ();
	      long hypers = var.GetDimQ();
		 
		   float ****mat = MakeMatrix4D(width,height,depth,hypers);
		   if(!mat) 
		   {
		      ErrorMessage("can't allocate memory for 'phase' result");
		      return(ERR);
		   }

	      for(long q = 0; q < hypers; q++)
	         for(long k = 0; k < depth; k++)
	            for(long j = 0; j < height; j++)
	               for(long i = 0; i < width; i++)
                     mat[q][k][j][i] = GetPhase(cmat[q][k][j][i]);

	      
	      itfc->retVar[1].AssignMatrix4D(mat,width,height,depth,hypers);
         itfc->nrRetValues = 1;
	      break;
	   }	   	   	   	   
      default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}
	return(OK);
}


/**************************************************************

   Calculate the RMS  of a matrix

   i.e. result = rms(v[i,i])
 
***************************************************************/

int MatrixRMS(Interface* itfc ,char args[])
{
   Variable v;
   short r;
   long i,j;
   long width,height;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get matrix
	if((r = ArgScan(itfc,args,1,"matrix","e","v",&v)) < 0)
	   return(r); 

// Get dimensions
   if(VarType(&v) == MATRIX2D || VarType(&v) == CMATRIX2D || VarType(&v) == DMATRIX2D)
   {
      width = VarWidth(&v);
      height = VarHeight(&v);
   }
   else
   {
      ErrorMessage("SD command not defined for this data type");
      return(ERR);
   }

// Work out mean for a real matrix
   if(VarType(&v) == MATRIX2D)
   {
      double N = width*height;

   // Get pointer to data
      float** A = VarRealMatrix(&v);

   // Take sum of elements
      double sum = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sum += A[j][i];

   // RMS
      double sumRes = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sumRes += (A[j][i]*A[j][i])/N;

      double rms = sqrt(sumRes);

   // Return the result
      ans->MakeAndSetFloat((float)rms);
   }

// Work out mean for a real matrix
   else if(VarType(&v) == DMATRIX2D)
   {
      double N = width*height;

   // Get pointer to data
      double** A = VarDoubleMatrix(&v);

   // RMS
      double sum = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sum += (A[j][i]*A[j][i])/N;

      double rms = sqrt(sum);

   // Return the result
      ans->MakeAndSetDouble(rms);
   }

// Work out trace for a complex matrix
   else if(VarType(&v) == CMATRIX2D)
   {
      double N = width*height;

   // Get pointer to data
      complex** A = VarComplexMatrix(&v);

   // RMS
      double sum = 0;
      for(j = 0; j< height; j++)
      {
         for(i = 0; i < width; i++)
         {
             sum += (A[j][i].r*A[j][i].r + A[j][i].i*A[j][i].i)/N;
         }
      }

      float rms = sqrt(sum);

   // Return the result
		ans->MakeAndSetFloat(rms);
   }

   itfc->nrRetValues = 1;
   return(OK);
}


/**************************************************************

   Calculate the standard deviation of a matrix

   i.e. result = sd(v[i,i])
 
***************************************************************/

int MatrixStandardDeviation(Interface* itfc ,char args[])
{
   Variable v;
   short r;
   long i,j;
   long width,height;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get matrix
	if((r = ArgScan(itfc,args,1,"matrix","e","v",&v)) < 0)
	   return(r); 

// Get dimensions
   if(VarType(&v) == MATRIX2D || VarType(&v) == CMATRIX2D || VarType(&v) == DMATRIX2D)
   {
      width = VarWidth(&v);
      height = VarHeight(&v);
   }
   else
   {
      ErrorMessage("SD command not defined for this data type");
      return(ERR);
   }

// Work out mean for a real matrix
   if(VarType(&v) == MATRIX2D)
   {
      double N = width*height;

   // Get pointer to data
      float** A = VarRealMatrix(&v);

   // Take sum of elements
      double sum = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sum += A[j][i];

   // Average
      double avg = sum/N;

   // Standard deviation
      double sumRes = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sumRes += ((A[j][i] - avg)*(A[j][i] - avg))/N;

      double sd = sqrt(sumRes);

   // Return the result
      ans->MakeAndSetFloat((float)sd);
   }

// Work out mean for a real matrix
   else if(VarType(&v) == DMATRIX2D)
   {
      double N = width*height;

   // Get pointer to data
      double** A = VarDoubleMatrix(&v);

   // Take sum of elements
      double sum = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sum += A[j][i];

   // Average
      double avg = sum/N;

   // Standard deviation
      double sumRes = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sumRes += ((A[j][i] - avg)*(A[j][i] - avg))/N;

      double sd = sqrt(sumRes);

   // Return the result
      ans->MakeAndSetDouble(sd);
   }

// Work out trace for a complex matrix
   else if(VarType(&v) == CMATRIX2D)
   {
      double N = width*height;

   // Get pointer to data
      complex** A = VarComplexMatrix(&v);

   // Take sum of elements
      double sumR = 0;
      double sumI = 0;
      for(j = 0; j< height; j++)
      {
         for(i = 0; i < width; i++)
         {
             sumR += A[j][i].r;
             sumI += A[j][i].i;
         }
      }

   // Average
      double avgR = sumR/N;
      double avgI = sumI/N;

   // Standard deviation
      double sumResR = 0;
      double sumResI = 0;
      for(j = 0; j< height; j++)
      {
         for(i = 0; i < width; i++)
         {
             sumResR += ((A[j][i].r - avgR)*(A[j][i].r - avgR))/N;
             sumResI += ((A[j][i].i - avgI)*(A[j][i].i - avgI))/N;
         }
      }

      complex sd;
      sd.r = sqrt(sumResR);
      sd.i = sqrt(sumResI);

   // Return the result
      ans->MakeAndSetComplex(sd);
   }

   itfc->nrRetValues = 1;
   return(OK);
}


/**************************************************************

   Calculate the mean of a matrix

   i.e. result = mean(A) or result = avg(A)
 
***************************************************************/

int MatrixMean(Interface* itfc ,char args[])
{
   Variable v;
   short r;
   long i,j;
   long width,height;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get matrix
	if((r = ArgScan(itfc,args,1,"matrix","e","v",&v)) < 0)
	   return(r); 

// Get dimensions
   if(VarType(&v) == MATRIX2D || VarType(&v) == CMATRIX2D || VarType(&v) == DMATRIX2D)
   {
      width = VarWidth(&v);
      height = VarHeight(&v);
   }
   else
   {
      ErrorMessage("Mean/Avg command not defined for this data type");
      return(ERR);
   }

// Work out mean for a real matrix
   if(VarType(&v) == MATRIX2D)
   {
   // Get pointer to data
      float** A = VarRealMatrix(&v);

   // Sum the diagonal elements
      float sum = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sum += A[j][i];

   // Return the result
      ans->MakeAndSetFloat(sum/(width*height));
   }

// Work out mean for a real matrix
   else if(VarType(&v) == DMATRIX2D)
   {
   // Get pointer to data
      double** A = VarDoubleMatrix(&v);

   // Sum the diagonal elements
      double sum = 0;
      for(j = 0; j< height; j++)
         for(i = 0; i < width; i++)
             sum += A[j][i];

   // Return the result
      ans->MakeAndSetDouble(sum/(width*height));
   }

// Work out trace for a complex matrix
   else if(VarType(&v) == CMATRIX2D)
   {
   // Get pointer to data
      complex** A = VarComplexMatrix(&v);

   // Sum the diagonal elements
      complex sum = {0,0};
      for(j = 0; j < height; j++)
      {
         for(i = 0; i < width; i++)
         {
            sum.r += A[j][i].r;
            sum.i += A[j][i].i;
         }
      }
      sum.r /= (width*height);
      sum.i /= (width*height);

   // Return the result
      ans->MakeAndSetComplex(sum);
   }

   itfc->nrRetValues = 1;
   return(OK);
}

/**************************************************************

   Calculate the trace of a matrix

   i.e. result = sum(v[i,i])
 
***************************************************************/

int MatrixTrace(Interface *itfc, char arg[])
{
   Variable v;
   short r;
   long i;
   long width,height;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get matrix
	if((r = ArgScan(itfc,arg,1,"matrix","e","v",&v)) < 0)
	   return(r); 

// Get dimensions
   if(VarType(&v) == MATRIX2D || VarType(&v) == CMATRIX2D || VarType(&v) == DMATRIX2D)
   {
      width = VarWidth(&v);
      height = VarHeight(&v);

   // Check for square matrix
      if(width != height)
      {
         ErrorMessage("The trace is only defined for a square matrix");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Trace command not defined for this data type");
      return(ERR);
   }

// Work out trace for a real matrix
   if(VarType(&v) == MATRIX2D)
   {
   // Get pointer to data
      float** A = VarRealMatrix(&v);

   // Sum the diagonal elements
      float trace = 0;
      for(i = 0; i < width; i++)
          trace += A[i][i];

   // Return the result
      ans->MakeAndSetFloat(trace);
   }

// Work out trace for a complex matrix
   else if(VarType(&v) == CMATRIX2D)
   {
   // Get pointer to data
      complex** A = VarComplexMatrix(&v);

   // Sum the diagonal elements
      complex trace = {0,0};
      for(i = 0; i < height; i++)
      {
         trace.r += A[i][i].r;
         trace.i += A[i][i].i;
      }

   // Return the result
      ans->MakeAndSetComplex(trace);
   }

// Work out trace for a real matrix
   else if(VarType(&v) == DMATRIX2D)
   {
   // Get pointer to data
      double** A = VarDoubleMatrix(&v);

   // Sum the diagonal elements
      double trace = 0;
      for(i = 0; i < width; i++)
          trace += A[i][i];

   // Return the result
      ans->MakeAndSetDouble(trace);
   }

   itfc->nrRetValues = 1;
   return(OK);
}

/**************************************************************

   Return an identity matrix

   result = identity(dim)
 
***************************************************************/

int IdentityMatrix(Interface *itfc, char arg[])
{
  short r;
  long size;
  long x,y;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get dimension
	if((r = ArgScan(itfc,arg,1,"dimension","e","l",&size)) < 0)
	   return(r); 

// Check it
   if(size <= 0)
   {
      ErrorMessage("matrix size must be greater than zero");
      return(ERR);
   }

// Allocate space for result matrix
   float **result = MakeMatrix2D(size,size);
   if(!result)
   {
      ErrorMessage("can't allocate memory for identity matrix");
      return(ERR);
   }

// Zero the matrix
   for(y = 0; y < size; y++)
      for(x = 0; x < size; x++)
         result[y][x] = 0;

// Add dialognal ones
   for(x = 0; x < size; x++)
      result[x][x] = 1;

// Return result
   ans->AssignMatrix2D(result,size,size);
   itfc->nrRetValues = 1;

   return(OK);
}

/**************************************************************

   Return a diagonal matrix based on a vector or another matrix

   result = diag(vector)
 
***************************************************************/

int DiagonalMatrix(Interface *itfc, char arg[])
{
  short r;
  long size;
  long x,y;
  Variable v;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get dimension
	if((r = ArgScan(itfc,arg,1,"dialognal","e","v",&v)) < 0)
	   return(r); 

// Check it
   if(VarType(&v) == MATRIX2D)
   {
      if(VarWidth(&v) > 0 && VarHeight(&v) == 1)
      {
         size = VarWidth(&v);
         float **diag = VarRealMatrix(&v);

      // Allocate space for result matrix
         float **result = MakeMatrix2D(size,size);
         if(!result)
         {
            ErrorMessage("can't allocate memory for identity matrix");
            return(ERR);
         }

      // Zero the matrix
         for(y = 0; y < size; y++)
            for(x = 0; x < size; x++)
               result[y][x] = 0;

      // Add dialognal values
         for(x = 0; x < size; x++)
            result[x][x] = diag[0][x];

      // Return result
         ans->AssignMatrix2D(result,size,size);
      }
      else if(VarHeight(&v) == VarWidth(&v))
      {
         size = VarWidth(&v);
         float **diag = VarRealMatrix(&v);

      // Allocate space for result matrix
         float **result = MakeMatrix2D(size,size);
         if(!result)
         {
            ErrorMessage("can't allocate memory for identity matrix");
            return(ERR);
         }

      // Zero the matrix
         for(y = 0; y < size; y++)
            for(x = 0; x < size; x++)
               result[y][x] = 0;

      // Add dialognal values
         for(x = 0; x < size; x++)
            result[x][x] = diag[x][x];

      // Return result
         ans->AssignMatrix2D(result,size,size);
      }
   }
   else if(VarType(&v) == CMATRIX2D && VarWidth(&v) > 0 && VarHeight(&v) == 1)
   {
      size = VarWidth(&v);
      complex **diag = VarComplexMatrix(&v);

   // Allocate space for result matrix
      complex **result = MakeCMatrix2D(size,size);
      if(!result)
      {
         ErrorMessage("can't allocate memory for identity matrix");
         return(ERR);
      }

   // Zero the matrix
      for(y = 0; y < size; y++)
      {
         for(x = 0; x < size; x++)
         {
            result[y][x].r = 0;
            result[y][x].i = 0;
         }
      }

   // Add dialognal values
      for(x = 0; x < size; x++)
         result[x][x] = diag[0][x];

   // Return result
      ans->AssignCMatrix2D(result,size,size);
   }
   else
   {
      ErrorMessage("argument must be a row vector or square matrix");
      return(ERR);
   }
   itfc->nrRetValues = 1;

   return(OK);
}


/**************************************************************

   Return a square matrix based on off-diagonal elements of 
   another matrix

   result = offdiag(matrix)
 
***************************************************************/

int OffDiagonalMatrix(Interface *itfc, char arg[])
{
  short r;
  long size;
  long x,y;
  Variable v;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get dimension
	if((r = ArgScan(itfc,arg,1,"dialognal","e","v",&v)) < 0)
	   return(r); 

// Check it
   if(VarType(&v) == MATRIX2D && VarHeight(&v) == VarWidth(&v))
   {
      size = VarWidth(&v);
      float **offdiag = VarRealMatrix(&v);

   // Allocate space for result matrix
      float **result = MakeMatrix2D(size,size);
      if(!result)
      {
         ErrorMessage("can't allocate memory for identity matrix");
         return(ERR);
      }

   // Add off-dialognal values zero others
      for(y = 0; y < size; y++)
      {
         for(x = 0; x < size; x++)
         {
            if(x != y)
               result[y][x] = offdiag[y][x];
            else
               result[y][x] = 0.0;
         }
      }

   // Return result
      ans->AssignMatrix2D(result,size,size);
   }
   else
   {
      ErrorMessage("argument must be a square real matrix");
      return(ERR);
   }
   itfc->nrRetValues = 1;

   return(OK);
}

/**************************************************************

   Calculate the outer or tensor product of two matrices

   A is an m by n real or complex matrix and 
   B is a  p by q real or complex matrix
  
    | A11*B  A12*B  A13*B ... A1n*B |
    | A21*B  A22*B  A23*B ... A2n*B |
    | A21*B  A22*B  A23*B ... A3n*B |
    |                               |
    | Am1*B  Am2*B  Am3*B ... Amn*B |
 
***************************************************************/

int TensorProduct(Interface *itfc, char arg[])
{
   Variable vA, vB;
   short r;
   long x,y,j,k;

   Variable* retVar = itfc->retVar;
   Variable* ans = &retVar[1];

// Get matrices
	if((r = ArgScan(itfc,arg,2,"A, B","ee","vv",&vA,&vB)) < 0)
	   return(r); 

// Work out product based on data type
   if(VarType(&vA) == MATRIX2D && VarType(&vB) == MATRIX2D)
   {
   // Get dimensions
      long n = VarWidth(&vA);
      long m = VarHeight(&vA);
      long q = VarWidth(&vB);
      long p = VarHeight(&vB);

   // Get pointers to data
      float** A = VarRealMatrix(&vA);
      float** B = VarRealMatrix(&vB);

   // Allocate space for result matrix
      float **result = MakeMatrix2D(n*q,m*p);
      if(!result)
      {
         ErrorMessage("can't allocate memory for outer product");
         return(ERR);
      }

      for(j = 0; j < m; j++)
      {
         for(k = 0; k < n; k++)
         {
            for(y = 0; y < p; y++)
            {
               for(x = 0; x < q; x++)
               {
                  result[j*p+y][k*q+x] = A[j][k]*B[y][x];
               }
            }
         }
      }
      ans->AssignMatrix2D(result,n*q,m*p);
   }
   else if(VarType(&vA) == CMATRIX2D && VarType(&vB) == CMATRIX2D)
   {
   // Get dimensions
      long n = VarWidth(&vA);
      long m = VarHeight(&vA);
      long q = VarWidth(&vB);
      long p = VarHeight(&vB);

   // Get pointers to data
      complex** A = VarComplexMatrix(&vA);
      complex** B = VarComplexMatrix(&vB);

   // Allocate space for result matrix
      complex **result = MakeCMatrix2D(n*q,m*p);
      if(!result)
      {
         ErrorMessage("can't allocate memory for outer product");
         return(ERR);
      }

      for(j = 0; j < m; j++)
      {
         for(k = 0; k < n; k++)
         {
            for(y = 0; y < p; y++)
            {
               for(x = 0; x < q; x++)
               {
                  result[j*p+y][k*q+x] = cmult(A[j][k],B[y][x]);
               }
            }
         }
      }
      ans->AssignCMatrix2D(result,n*q,m*p);
   }
   else if(VarType(&vA) == MATRIX2D && VarType(&vB) == CMATRIX2D)
   {
   // Get dimensions
      long n = VarWidth(&vA);
      long m = VarHeight(&vA);
      long q = VarWidth(&vB);
      long p = VarHeight(&vB);

   // Get pointers to data
      float** A = VarRealMatrix(&vA);
      complex** B = VarComplexMatrix(&vB);

   // Allocate space for result matrix
      complex **result = MakeCMatrix2D(n*q,m*p);
      if(!result)
      {
         ErrorMessage("can't allocate memory for outer product");
         return(ERR);
      }

      for(j = 0; j < m; j++)
      {
         for(k = 0; k < n; k++)
         {
            for(y = 0; y < p; y++)
            {
               for(x = 0; x < q; x++)
               {
                  result[j*p+y][k*q+x] = cmult(A[j][k],B[y][x]);
               }
            }
         }
      }
      ans->AssignCMatrix2D(result,n*q,m*p);
   }
   else if(VarType(&vA) == CMATRIX2D && VarType(&vB) == MATRIX2D)
   {
   // Get dimensions
      long n = VarWidth(&vA);
      long m = VarHeight(&vA);
      long q = VarWidth(&vB);
      long p = VarHeight(&vB);

   // Get pointers to data
      complex** A = VarComplexMatrix(&vA);
      float** B = VarRealMatrix(&vB);

   // Allocate space for result matrix
      complex **result = MakeCMatrix2D(n*q,m*p);
      if(!result)
      {
         ErrorMessage("can't allocate memory for outer product");
         return(ERR);
      }

      for(j = 0; j < m; j++)
      {
         for(k = 0; k < n; k++)
         {
            for(y = 0; y < p; y++)
            {
               for(x = 0; x < q; x++)
               {
                  result[j*p+y][k*q+x] = cmult(A[j][k],B[y][x]);
               }
            }
         }
      }
      ans->AssignCMatrix2D(result,n*q,m*p);
   }
   else
   {
      ErrorMessage("invalid data types for outer product");
      return(ERR);
   }


   return(OK);
}

// Returns the phase of the complex number cnum

float GetPhase(complex cnum)
{
   float phase = 0;

   if(cnum.r >= 0 && cnum.i >= 0) // Quad 1
   {
      if(cnum.r == 0 && cnum.i > 0)
         phase = PI/2;
      else if(cnum.r == 0 && cnum.i == 0)
         phase = 0;
      else
	      phase = atan(cnum.i/cnum.r);
   }
   else if(cnum.r < 0 && cnum.i >= 0) // Quad 2
   {
      if(cnum.i == 0)
         phase = PI;
      else
	      phase = atan(-cnum.r/cnum.i) + PI/2;
   }
   else if(cnum.r < 0 && cnum.i < 0) // Quad 3
   {
      if(cnum.r == 0)
         phase = 3*PI/2;
      else
	      phase = atan(cnum.i/cnum.r)+PI;
   }
   else if(cnum.r >= 0 && cnum.i < 0) // Quad 4
   {
      if(cnum.i == 0)
         phase = 0;
      else
	      phase = atan(-cnum.r/cnum.i) + 3*PI/2;
   }  
   return(phase);
}



/*********************************************************************
  Routines to compare two single precision floating point numbers
**********************************************************************/

#define epsilon 1e-7 // 2^-23 


bool FloatEqual(float f1, float f2)
{
   if(isnan(f1))
      return(isnan(f2));
   else if(isnan(f2))
      return(isnan(f1));
   else if(!_finite(f1))
      return(!_finite(f2));
   else if(!_finite(f2))
      return(!_finite(f1));

   return(fabs(f1-f2) <= fabs(f1+f2)*epsilon);
}

bool FloatNotEqual(float f1, float f2)
{
   if(isnan(f1))
      return(!isnan(f2));
   else if(isnan(f2))
      return(!isnan(f1));
   else if(!_finite(f1))
      return(_finite(f2));
   else if(!_finite(f2))
      return(_finite(f1));

   if(fabs(f1-f2) <= fabs(f1+f2)*epsilon)
      return(false);
   return(true);
}

bool FloatGreater(float f1, float f2)
{
   if((f1-f2) > fabs(f1+f2)*epsilon)
      return(true);
   return(false);
}

bool FloatGreaterEqual(float f1, float f2)
{
   float small = fabs(f1+f2)*epsilon;
   if((f1-f2) > small || fabs(f1-f2) <= small)
      return(true);
   return(false);
}

bool FloatLess(float f1, float f2)
{
   if((f2-f1) > fabs(f1+f2)*epsilon)
      return(true);
   return(false);
}

bool FloatLessEqual(float f1, float f2)
{
   float small = fabs(f1+f2)*epsilon;

   if((f2-f1) > small  || fabs(f1-f2) <= small)
      return(true);
   return(false);
}


#define epsilond 2e-16 // 2^-52

bool DoubleEqual(double f1, double f2)
{
   if(isnan(f1))
      return(isnan(f2));
   else if(isnan(f2))
      return(isnan(f1));
   else if(!_finite(f1))
      return(!_finite(f2));
   else if(!_finite(f2))
      return(!_finite(f1));

   return(fabs(f1-f2) <= fabs(f1+f2)*epsilond);
}

bool DoubleNotEqual(double f1, double f2)
{
   if(isnan(f1))
      return(!isnan(f2));
   else if(isnan(f2))
      return(!isnan(f1));
   else if(!_finite(f1))
      return(_finite(f2));
   else if(!_finite(f2))
      return(_finite(f1));

   if(fabs(f1-f2) <= fabs(f1+f2)*epsilond)
      return(false);
   return(true);
}

bool DoubleGreater(double f1, double f2)
{
   if((f1-f2) > fabs(f1+f2)*epsilond)
      return(true);
   return(false);
}

bool DoubleGreaterEqual(double f1, double f2)
{
   double small = fabs(f1+f2)*epsilond;
   if((f1-f2) > small || fabs(f1-f2) <= small)
      return(true);
   return(false);
}

bool DoubleLess(double f1, double f2)
{
   if((f2-f1) > fabs(f1+f2)*epsilond)
      return(true);
   return(false);
}

bool DoubleLessEqual(double f1, double f2)
{
   double small = fabs(f1+f2)*epsilond;

   if((f2-f1) > small  || fabs(f1-f2) <= small)
      return(true);
   return(false);
}


/************************************************************************************
   Make a copy of float array "in" of length size and return pointer to caller
    Memory should be eventually cleared using FreeVector
************************************************************************************/

float* CopyArray(float *in, long size)
{
   float *out;
   
   out = MakeVector(size);
   
   for(long i = 0; i < size; i++)
   {
      out[i] = in[i];
   }
   
   return(out);
}


/************************************************************************************
   Make a copy of double array "in" of length size and return pointer to caller
    Memory should be eventually cleared using FreeDVector
************************************************************************************/

double* CopyDArray(double *in, long size)
{
   double *out;
   
   out = MakeDVector(size);
   
   for(long i = 0; i < size; i++)
   {
      out[i] = in[i];
   }

   return(out);
}


/************************************************************************************
   Make a copy of float array "in" of length size and return pointer to caller
    Memory should be eventually cleared using FreeVector
************************************************************************************/

float* CopyDtoFArray(double *in, long size)
{
   float *out;
   
   out = MakeVector(size);
   
   for(long i = 0; i < size; i++)
   {
      out[i] = (float)in[i];
   }
   
   return(out);
}


/************************************************************************************
  Make a copy of complex array "in" of length size and return pointer to caller
  Memory should be eventually cleared using FreeCVector
************************************************************************************/

complex* CopyCArray(complex *in, long size)
{
   complex *out;
   
   out = MakeCVector(size);
   
   for(long i = 0; i < size; i++)
   {
      out[i] = in[i];
   }
   
   return(out);
}

/************************************************************************************
  Return a copy of the contents of matrix 'in' 
  Memory should later be freed using FreeMatrix2D
************************************************************************************/

float** CopyMatrix(float **in, long width, long height)
{
   float **out;
   
   out = MakeMatrix2D(width,height);
   if(!out)
   {
      ErrorMessage("can't allocate memory in CopyMatrix");
      return(NULL);
   }
   
   for(long y = 0; y < height; y++)
   {
      for(long x = 0; x < width; x++)
      {   
         out[y][x] = in[y][x];
      }
   }
   return(out);
}


/************************************************************************************
    Split a complex array into real and imaginary parts
************************************************************************************/

void SplitComplexArray(complex *in, long size, float **real, float **imag)
{ 
   *real = MakeVector(size);
   *imag = MakeVector(size);
   
   for(long index = 0; index < size; index++)
   {
      (*real)[index] = in[index].r;
      (*imag)[index] = in[index].i;
   }
}


/************************************************************************************
   Make a linear array of length "size" and fill with array indices. Return
  array to caller. Memory should be eventually cleared using FreeVector
************************************************************************************/

float* MakeLinearArray(long size)
{
   float *out;
   
   out = MakeVector(size);
   
   for(long i = 0; i < size; i++)
   {
      out[i] = i;
   }
   
   return(out);
}


/************************************************************************************
   Returns the complex product of numbers a and b
************************************************************************************/

complex cmult(complex a, complex b)
{
   complex result;

   result.r = (a.r*b.r - a.i*b.i);
   result.i = (a.i*b.r + a.r*b.i);

   return(result);
}

complex cmult(float a, complex b)
{
   complex result;

   result.r = a*b.r;
   result.i = a*b.i;

   return(result);
}

complex cmult(complex a, float b)
{
   complex result;

   result.r = a.r*b;
   result.i = a.i*b;

   return(result);
}

/************************************************************************************
   Returns the complex quotient of numbers a and b
************************************************************************************/

complex cdiv(complex a, complex b)
{
   complex result;
   float denom;

   denom = a.r*a.r + b.r*b.r;

   result.r = (a.r*b.r + a.i*b.i)/denom;
   result.i = (a.i*b.r - a.r*b.i)/denom;

   return(result);
}

/*************************************************************************
    See if str represents a valid floating point number
*************************************************************************/

bool IsNumber(char *str)
{
	double number;
   char *end;
   if(str[0] == '\0') // Empty string
       return(false);
   number = strtod(str,&end);
   if(end == str + strlen(str))
      return(true);
   return(false);
}

bool IsNumber(char *str, double &number)
{
   char *end;
   if(str[0] == '\0') // Empty string
       return(false);
   number = strtod(str,&end);
   if(end == str + strlen(str))
      return(true);
   return(false);
}


/*************************************************************************
    See if str represents a valid double precision floating point number
*************************************************************************/

bool IsDouble(char *str)
{
	double number;
   char *end;
   if(str[0] == '\0') // Empty string
      return(false);

   int len = strlen(str);
   if(str[len-1] == 'd')
   {
      str[len-1] = '\0';
      number = strtod(str,&end);
      if(end == str + strlen(str))
      {
         str[len-1] = 'd';
         return(true);
      }
      str[len-1] = 'd';
   }
   return(false);
}

bool IsDouble(char *str, double &number)
{
   char *end;
   if(str[0] == '\0') // Empty string
      return(false);

   int len = strlen(str);
   if(str[len-1] == 'd')
   {
      str[len-1] = '\0';
      number = strtod(str,&end);
      if(end == str + strlen(str))
      {
         str[len-1] = 'd';
         return(true);
      }
      str[len-1] = 'd';
   }
   return(false);
}

#define SINGLE_SIGNIFICANT_FIGURES   6

bool IsDoubleByPrecision(char *str)
{
    int i,j,k;
    int start;
    int len = strlen(str);
    int end = len-1;
    int sig = 0;

// Handle scientific notation by ignoring exponent
// Ignore hexadecimal numbers (assume 32 bit)
   for(i = len-1; i >= 0; i--)
   {
      if(str[i] == 'e' || str[i] == 'E')
         break;
      if(str[i] == 'x' || str[i] == 'X')
         return(false);
   }

   if(i != -1)
      end = i-1;

// Count significant digits in mantissa
    for(int i = 0; i < end; i++)
    {
       if(str[i] != '0' && str[i] != '.' && str[i] != '-' && str[i] != '+')
       {
          int start = i;
          for(j = end; j >= 0; j--)
          {
             if(str[j] != '0' && str[j] != '.')
             {
                for(k = start; k <= j; k++)
                {
                   if(str[k] != '.')
                      sig++;
                }
                if(sig > SINGLE_SIGNIFICANT_FIGURES)
                   return(true);
                else
                   return(false);
             }
          }
       }
    }

    return(false);
}








  

/*************************************************************************
    See if num is an integer
*************************************************************************/

bool IsInteger(float num)
{
   if(num != 0)
      return(fabs((num - nint(num))/num) < 1e-6);
   else
      return(fabs(num - nint(num)) < 1e-6);
}

bool IsInteger(double num)
{
   if(num != 0)
      return(fabs((num - nhint(num))/num) < 1e-14);
   else
      return(fabs(num - nhint(num)) < 1e-14);
}

/*****************************************************************************************
*          Take a floating point number and return mantissa and exponent (base10)        *
*****************************************************************************************/

void FloatSplit(float data,
					 char mantissa[],
					 char exponent[],
					 short ac)
{
	short i;
	char format[20];
	char str[20];

	sprintf(format,"%%+1.%hde",ac);
	sprintf(str,format,data);
	if(ac == 0) ac--;

	for(i = 0; i < ac+3; i++)
		mantissa[i] = str[i];
	mantissa[i] = '\0';

	// Need different code here since MW uses 2 digits 
	// in the exponent and VC++ 3.
#ifdef __MWERKS__
	for(i = ac+4; i < ac+7; i++)
		exponent[i-ac-4] = str[i];
	exponent[3] = '\0';
#else
	for(i = ac+4; i < ac+8; i++)
		exponent[i-ac-4] = str[i];
	exponent[4] = '\0';
#endif


}


/*************************************************************************
  Given a vector v of length N return the index closest to vector value x

  Modfied to allow searching in both directions 19 Feb 07
**************************************************************************/

long FindIndexCore(float *v, long N, float x)
{
   long j;
   long index;

   if(v[0] < v[N-1]) // Ascending data
   {
      for(j = 0; j < N; j++)
      {
         if(v[j] >= x) break;
      }
		if (j == N)
			index = j-1;
      else if(fabs(v[j]-x) < fabs(v[j-1]-x))
         index = j;
      else
         index = j-1;  
   }
   else // Descending data
   {
      for(j = N-1; j >= 0; j--)
      {
         if(v[j] >= x) break;
      }
		if (j == -1)
			index = 0;
      else if(fabs(v[j]-x) < fabs(v[j+1]-x))
         index = j;
      else
         index = j+1; 

   }

   return(index);
}

long FindIndexCore(double *v, long N, float x)
{
   long j;
   long index;

   if(v[0] < v[N-1]) // Ascending data
   {
      for(j = 0; j < N; j++)
      {
         if(v[j] >= x) break;
      }
		if (j == N)
			index = j-1;
      else if(fabs(v[j]-x) < fabs(v[j-1]-x))
         index = j;
      else
         index = j-1;  
   }
   else // Descending data
   {
      for(j = N-1; j >= 0; j--)
      {
         if(v[j] >= x) break;
      }
		if (j == -1)
			index = 0;
      else if(fabs(v[j]-x) < fabs(v[j+1]-x))
         index = j;
      else
         index = (j+1); 

   }

   return(index);
}

/*************************************************************************
  Given a vector v of length N return the index closest to vector value x

**************************************************************************/

long FindIndexCore2(long start, bool searchRight, float *v, long N, float x)
{
   long j;
   long index;

   if(searchRight) // Search to the right
   {
      if(v[start] < x) // Ascending data
      {
         for(j = start; j < N; j++)
         {
            if(v[j] >= x) break;
         }
         if(fabs(v[j]-x) < fabs(v[j-1]-x))
            index = (float)j;
         else
            index = (float)(j-1);  
      }
      else // Descending data
      {
         for(j = start; j < N; j++)
         {
            if(v[j] <= x) break;
         }
         if(fabs(v[j]-x) < fabs(v[j-1]-x))
            index = (float)j;
         else
            index = (float)(j-1); 
       }
   }
   else // Search to the left
   {
      if(v[start] < x) // Ascending data
      {
         for(j = start; j >= 0; j--)
         {
            if(v[j] >= x) break;
         }
         if(fabs(v[j]-x) < fabs(v[j+1]-x))
            index = (float)j;
         else
            index = (float)(j+1);  
      }
      else // Descending data
      {
         for(j = start; j >= 0; j--)
         {
            if(v[j] <= x) break;
         }
         if(fabs(v[j]-x) < fabs(v[j+1]-x))
            index = (float)j;
         else
            index = (float)(j+1); 
       }
   }

   return(index);
}


/********************************************************************
  Return the linearly interpolated x axis value for given y values
*********************************************************************/

int FindXValue(Interface* itfc ,char args[])
{
   long r,index;
   Variable varX,varY,result;
   long xsize,ysize;
   float** vX, **vY;
   CText txt;
   CArg carg;
	CText direction = "left";
	long startIdx = 0;
	long i,j;
    
// Get vector *****************************
	if((r = ArgScan(itfc,args,4,"xAxis,yAxis,x0,direction,y1,y2,...","eeee","vvlt",&varX,&varY,&startIdx,&direction)) < 0)
	   return(r); 

// Get x vector
   if(varX.GetType() == MATRIX2D)
   {
      vX = varX.GetMatrix2D();
      xsize = varX.GetDimX();
      ysize = varX.GetDimY();
      if(ysize != 1)
	   {
	      ErrorMessage("invalid vector");
	      return(ERR);
	   }       
   }
   else
   {
      ErrorMessage("invalid x axis vector");
      return(ERR);
   }         

// Get y vector
   if(varY.GetType() == MATRIX2D)
   {
      vY = varY.GetMatrix2D();
      xsize = varY.GetDimX();
      ysize = varY.GetDimY();
      if(ysize != 1)
	   {
	      ErrorMessage("invalid vector");
	      return(ERR);
	   }       
   }
   else
   {
      ErrorMessage("invalid y axis vector");
      return(ERR);
   }   

// Check vector sizes are the same
	if(varX.GetDimX() != varY.GetDimX())
   {
      ErrorMessage("x and y vectors should have the same dimensions");
      return(ERR);
   }

// Check for valid start index
	if(startIdx >= xsize || startIdx < 0)
   {
      ErrorMessage("invalid x0 value- should be 0 -> %d",xsize-1);
      return(ERR);
   }

// Get variables *******************
   r = carg.Count(args);
   for(i = 5; i <= r; i++)
   {
      txt = carg.Extract(i);
      
      if(Evaluate(itfc,RESPECT_ALIAS,txt.Str(),&result) == ERR)
      {
         ErrorMessage("invalid coordinate");
         return(ERR);
      }
    // Search for closest array value  
      if(result.GetType() == FLOAT32)
      {
			float y = result.GetReal();
			if(direction == "right")
			{
				for(j = startIdx; j < xsize; j++)
				{
					if(vY[0][j] <= y)
					{
						float xpos = (y-vY[0][j-1])/(vY[0][j]-vY[0][j-1])*(vX[0][j]-vX[0][j-1]) + vX[0][j-1];
						itfc->retVar[i-4].MakeAndSetFloat(xpos); 
						break;
					}
				}
				if(j == xsize)
				{
					TextMessage("Warning - findxindex couldn't find y value %f - returning max x\n",y);
					itfc->retVar[i-4].MakeAndSetFloat(vX[0][xsize-1]);
				}
			}
			else
			{
				for(j = startIdx; j >= 1; j--)
				{
					if(vY[0][j] <= y)
					{
						float xpos = (y-vY[0][j])/(vY[0][j]-vY[0][j+1])*(vX[0][j]-vX[0][j+1]) + vX[0][j];
						itfc->retVar[i-4].MakeAndSetFloat(xpos); 
						break;
					}
				}
				if(j == 0)
				{
					TextMessage("Warning - findxindex couldn't find y value %f - returning min x\n",y);
					itfc->retVar[i-4].MakeAndSetFloat(vX[0][0]);
				}
			}
		}
      else
      {
         ErrorMessage("invalid data type for x value");
         return(ERR);
      }
   }
   itfc->nrRetValues = r-4;

   return(OK);
}

/********************************************************************
  Return the indices for the values in the 1D float vector which are
  equal the numbers x1, x2, x3 .... doubles or floats supported
*********************************************************************/

int FindExactIndex(Interface *itfc, char args[])
{
   long r,index;
   Variable var,result;
   long xsize,ysize;
   float** v;
   CText txt;
   CArg carg;
    
// Get vector *****************************
	if((r = ArgScan(itfc,args,1,"vector/list,x1,x2,...","e","v",&var)) < 0)
	   return(r); 

// Make sure var is a vector
   if(var.GetType() == MATRIX2D)
   {
      v = var.GetMatrix2D();
      xsize = var.GetDimX();
      ysize = var.GetDimY();
      if(ysize != 1)
	   {
	      ErrorMessage("array must be 1D");
	      return(ERR);
	   }       
      
         
	// Get variables *******************
		r = carg.Count(args);
		for(short i = 2; i <= r; i++)
		{
			txt = carg.Extract(i);
      
			if(Evaluate(itfc,RESPECT_ALIAS,txt.Str(),&result) == ERR)
			{
				ErrorMessage("invalid array value");
				return(ERR);
			}
		 // Search for exact array value  
			if(result.GetType() == FLOAT32)
			{
				float value = result.GetReal();
				for(index = 0; index < xsize; index++)
				{
					if(v[0][index] == value)
					{
						itfc->retVar[i-1].MakeAndSetFloat((float)index); 
						break;
					}
				}
				if(index == xsize)
					itfc->retVar[i-1].MakeAndSetFloat(-1.0); 
			}
			else if(result.GetType() == FLOAT64)
			{
				double value = result.GetDouble();
				for(index = 0; index < xsize; index++)
				{
					if(v[0][index] == value)
					{
						itfc->retVar[i-1].MakeAndSetFloat((float)index); 
						break;
					}
				}
				if(index == xsize)
					itfc->retVar[i-1].MakeAndSetFloat(-1.0); 	   }
			else
			{
				ErrorMessage("array value must be float or double");
				return(ERR);
			}
		}
		itfc->nrRetValues = r-1;
		return(OK);
	}
   else if(var.GetType() == LIST)
   {
		char **lst = var.GetList();
		int xsize = var.GetDimX();

		r = carg.Count(args);
		for(short i = 2; i <= r; i++)
		{
			txt = carg.Extract(i);
      
			if(Evaluate(itfc,RESPECT_ALIAS,txt.Str(),&result) == ERR)
			{
				ErrorMessage("invalid compare value");
				return(ERR);
			}
		 // Search for exact array value  
			if(result.GetType() == UNQUOTED_STRING)
			{
				char *testStr = result.GetString();

				for(index = 0; index < xsize; index++)
				{
					if(!strcmp(lst[index],testStr))
					{
						itfc->retVar[i-1].MakeAndSetFloat((float)index); 
						break;
					}
				}
				if(index == xsize)
					itfc->retVar[i-1].MakeAndSetFloat(-1.0); 

			}
			else
			{
				ErrorMessage("compare value must be a string");
				return(ERR);
			}
		}
		itfc->nrRetValues = r-1;
		return(OK);
	}
	else
	{
      ErrorMessage("array must be 1D and numeric or a list");
      return(ERR);
   }   
}

/********************************************************************
  Return the indices for the values in the vector which are closest
  to the numbers x1, x2, x3 ...
*********************************************************************/

int FindIndex(Interface* itfc ,char args[])
{
   long r,index;
   Variable var,result;
   long xsize,ysize;
   float** v;
   CText txt;
   CArg carg;
    
// Get vector *****************************
	if((r = ArgScan(itfc,args,1,"vector,x1,x2,...","e","v",&var)) < 0)
	   return(r); 

// Make sure var is a vector
   if(var.GetType() == MATRIX2D)
   {
      v = var.GetMatrix2D();
      xsize = var.GetDimX();
      ysize = var.GetDimY();
      if(ysize != 1)
	   {
	      ErrorMessage("invalid vector");
	      return(ERR);
	   }       
   }
   else
   {
      ErrorMessage("invalid vector");
      return(ERR);
   }         
         
// Get variables *******************
   r = carg.Count(args);
   for(short i = 2; i <= r; i++)
   {
      txt = carg.Extract(i);
      
      if(Evaluate(itfc,RESPECT_ALIAS,txt.Str(),&result) == ERR)
      {
         ErrorMessage("invalid coordinate");
         return(ERR);
      }
    // Search for closest array value  
      if(result.GetType() == FLOAT32)
      {
         index = FindIndexCore(v[0],xsize,result.GetReal());

         if(index < 0 || index > xsize)
         {
            ErrorMessage("invalid index");
            return(ERR);
         }
       // Return this as a variable
         itfc->retVar[i-1].MakeAndSetFloat((float)nint(index)); 
	   }
      else if(result.GetType() == FLOAT64)
      {
         index = FindIndexCore(v[0],xsize,(float)result.GetDouble());

         if(index < 0 || index > xsize)
         {
            ErrorMessage("invalid index");
            return(ERR);
         }
       // Return this as a variable
         itfc->retVar[i-1].MakeAndSetFloat((float)nint(index)); 
	   }
      else
      {
         ErrorMessage("invalid data type for x value");
         return(ERR);
      }
   }
   itfc->nrRetValues = r-1;

   return(OK);
}


/********************************************************************
  Return the indices for the values in the vector which are closest
  to the numbers x1, x2, x3 ... starting from position 'start' and moving
  in the specified direction.
*********************************************************************/

int FindIndex2(Interface* itfc ,char args[])
{
   long r,index;
   Variable vec,result;
   long xsize,ysize;
   long start = 0;
   CText dir = "right";
   float** v;
   CText txt;
   CArg carg;
   long dirN;
    
// Get vector *****************************
	if((r = ArgScan(itfc,args,3,"vector,start,direction,x1,x2,...","eee","vlt",&vec,&start,&dir)) < 0)
	   return(r); 

// Make sure var is a vector
   if(VarType(&vec) == MATRIX2D)
   {
      v = VarRealMatrix(&vec);
      xsize = VarColSize(&vec);
      ysize = VarRowSize(&vec);
      if(ysize != 1)
	   {
	      ErrorMessage("invalid vector");
	      return(ERR);
	   }       
   }
   else
   {
      ErrorMessage("invalid vector");
      return(ERR);
   }  

   if(start < 0 || start >= xsize)
   {
      ErrorMessage("start point outside vector");
      return(ERR);
   } 

   if(dir == "left")
   {
      dirN = 0;
   }
   else if(dir == "right")
   {
      dirN = 1;
   }
   else
   {
      ErrorMessage("invalid direction");
      return(ERR);
   } 

         
// Get variables *******************
   r = carg.Count(args);
   for(short i = 4; i <= r; i++)
   {
      txt = carg.Extract(i);
      
      if(Evaluate(itfc,RESPECT_ALIAS,txt.Str(),&result) == ERR)
      {
         ErrorMessage("invalid coordinate");
         return(ERR);
      }
    // Search for closest array value  
      if(result.GetType() == FLOAT32)
      {
         index = FindIndexCore2(start,dirN,v[0],xsize,result.GetReal());

         if(index < 0 || index > xsize)
         {
            ErrorMessage("invalid index");
            return(ERR);
         }
       // Return this as a variable
	      itfc->retVar[i-3].MakeAndSetFloat((float)nint(index)); 
	   }  
   }
   itfc->nrRetValues = r-3;

   return(OK);
}

/********************************************************************
          Return the dimension of the matrix (1,2,3,4)
*********************************************************************/

int GetMatrixDimension(Interface* itfc ,char args[])
{
   Variable var,*pvar;
   CArg carg;
   short nArg,r,type;

// Extract arguments - but do it explicitly to save time since 
// don't actually need the matrix info, just the dimensions

   nArg = carg.Count(args);

   if(nArg == 0)
   {
   // Prompt the user  
	   if((r = ArgScan(itfc,args,1,"matrix name","e","v",&var)) < 0)
	      return(r); 
   }
   if(nArg == 1)
   {
      char *arg1 = carg.Extract(1);
	   if(!(pvar = GetVariable(itfc,ALL_VAR,arg1,type)))
      {
	      if((r = ArgScan(itfc,args,1,"matrix name","e","v",&var)) < 0)
	         return(r); 
         pvar = &var;
      }
   }
   else
   {
      ErrorMessage("Only 1 argument expected");
      return(ERR);
   }

   type = pvar->GetType();

   if(type != MATRIX2D && type != CMATRIX2D &&
      type != MATRIX3D && type != CMATRIX3D &&
      type != MATRIX4D && type != CMATRIX4D)
   {
      ErrorMessage("not a matrix");
      return(ERR);
   }

   long dimX = pvar->GetDimX();
   long dimY = pvar->GetDimY();
   long dimZ = pvar->GetDimZ();
   long dimQ = pvar->GetDimQ();

   float dim = (((dimX > 1  && dimY == 1) || (dimX == 1  && dimY > 1)) && dimZ == 1 && dimQ == 1) +
               (dimX > 1   && dimY > 1   && dimZ == 1 && dimQ == 1)*2 + 
               (dimX >= 1  && dimY >= 1  && dimZ > 1  && dimQ == 1)*3 +
               (dimX >= 1  && dimY >= 1  && dimZ >= 1 && dimQ > 1)*4;

   itfc->retVar[1].MakeAndSetFloat(dim);
   itfc->nrRetValues = 1;

   return(OK);
}


/***************************************************************************************
    Generate a vector which bins input data using a log spacing.
***************************************************************************************/

int LogBin(Interface *itfc, char args[])
{
   short r;
   long N = 50;
   Variable xInVar,yInVar;
   float **xOut = 0,**yOut = 0;

   if((r = ArgScan(itfc,args,2,"xIn, yIn ,binnedWidth","eee","vvl",
                    &xInVar, &yInVar, &N)) < 0)
        return(r);

   if(xInVar.GetType() == MATRIX2D && yInVar.GetType() == MATRIX2D)
   {
      int widthX = xInVar.GetDimX();
      int widthY = yInVar.GetDimX();

      if(xInVar.GetDimY() != 1)
      {
         ErrorMessage("xIn vector should be a row vector");
         return(ERR);
      }

      if(yInVar.GetDimY() != 1)
      {
         ErrorMessage("yIn vector should be a row vector");
         return(ERR);
      }

      if(widthX != widthY)
      {
         ErrorMessage("xIn & yIn vectors should have same number of elements");
         return(ERR);
      }

      int M = widthX;

      float *xIn = xInVar.GetMatrix2D()[0];
      float *yIn = yInVar.GetMatrix2D()[0];

      if(N > M)
      {
         ErrorMessage("Reduce number of binned points");
         return(ERR);
      }

      // X axis range
      float minT = xIn[0];
      float maxT = xIn[M-1];

      if(minT <= 0)
      {
         ErrorMessage("All xaxis values should be > 0");
         return(ERR);
      }

      // Make a log space vector over this range
      float **logspace = MakeMatrix2D(N,1);
      float *logspc = logspace[0];
      float step = (log10(maxT)-log10(minT))/(N-1);
      float x = log10(minT);
      for(int i = 0; i < N; i++)
      {
         logspc[i] = pow(10.0,x);
         x += step;
      }

      // Result allocation
      float **resultx = MakeMatrix2D(N,1);
      float **resulty = MakeMatrix2D(N,1);
      resultx[0][0] = logspc[0];
      resulty[0][0] = yIn[0];

      // Loop to determine binning
      int n = 0;
      int cnt;
      float sx = 0;
      float sy = 0;
      float left,right,pos;
      int posq;

      for(int k = 1; k <= N-2; k++)
      {
         sx = 0;
         sy = 0;
         cnt = 0;
         left = (logspc[k-1] + logspc[k])/2;
         right = (logspc[k] + logspc[k+1])/2;

         for(int q = n; q <= M-2; q++)
         {
            pos = xIn[q];

            if(pos >= left & pos <= right)
            {
               sy = sy + yIn[q];
               sx = sx + xIn[q];
               cnt = cnt + 1;
            }
            else if(pos > right)
            {
               posq = q;
               break;
            }
         }
 
         n = posq;
         if(cnt == 0)
         {
            resulty[0][k] = (logspc[k]-xIn[posq-1])/(xIn[posq]-xIn[posq-1])*(yIn[posq]-yIn[posq-1])+yIn[posq-1];
            resultx[0][k] = logspc[k];
         }
         else 
         {
            resultx[0][k] = sx/cnt;
            resulty[0][k] = sy/cnt;
         }
      }
      float *finalx = new float[M-posq];
      float *finaly = new float[M-posq];
      for(int k = posq; k < M; k++)
      {
         finalx[k-posq] = xIn[k];
         finaly[k-posq] = yIn[k];
      }
      float slope,intercept;
      LinearFit(finalx,finaly,M-posq,&slope, &intercept);

      // Determine final point by linear fit
      resultx[0][N-1] = logspc[N-1];
      resulty[0][N-1] = xIn[M-1]*slope + intercept;

      itfc->retVar[1].AssignMatrix2D(resultx,N,1);
      itfc->retVar[2].AssignMatrix2D(resulty,N,1);
      itfc->nrRetValues = 2;
   }
   else
   {
      ErrorMessage("Invalid input data types");
      return(ERR);
   }

   return(OK);
}

/***************************************************************************************
    Perform a linear fit to data (xdata,ydata) which have num points. Return slope
    and intercept.
***************************************************************************************/

void LinearFit(float *xdata, float *ydata, long num,
               float *slope,float *intercept)

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
}

/***************************************************************************************
    Generate a matrix which has its rows or columns binning using the pseudo-log spacing
    in which the first gap is at least as large as the first. Useful for reducing the
    size of CPMG data sets before applying the Maximum Entropy inversion
***************************************************************************************/
float sumVec(float* vec, long N, long start, long end);

int PseudoLogBin(Interface* itfc ,char args[])
{
   short r;
   long N = 50;
   Variable xInVar,yInVar;
   float **xOut = 0,**yOut = 0;

   if((r = ArgScan(itfc,args,3,"xIn, yIn ,binnedWidth","eee","vvl",
                    &xInVar, &yInVar, &N)) < 0)
        return(r);

   if(xInVar.GetType() == MATRIX2D && yInVar.GetType() == MATRIX2D)
   {
      int widthX = xInVar.GetDimX();
      int widthY = yInVar.GetDimX();
      int x,y;

      if(xInVar.GetDimY() != 1)
      {
         ErrorMessage("xIn vector should be a row vector");
         return(ERR);
      }

      if(yInVar.GetDimY() != 1)
      {
         ErrorMessage("yIn vector should be a row vector");
         return(ERR);
      }

      if(widthX != widthY)
      {
         ErrorMessage("xIn & yIn vectors should have same number of elements");
         return(ERR);
      }

      int sz = widthX;

      float *xIn = xInVar.GetMatrix2D()[0];
      float *yIn = yInVar.GetMatrix2D()[0];

      if(N > sz)
      {
         ErrorMessage("Reduce number of binned points");
         return(ERR);
      }
      
      double n = 1.999;
      double del = 1;
      double q = xIn[sz-1]/xIn[0];
      double p = N;
      double m,alpha,z;

      while(1)
      {
         m = (pow(n,p)-1)/(n-1);
         if(m < q)
            n = n + del;
         else
            n = n -del;
            
         del = del/2;
         if(del < 1e-8)
         {
            alpha = 1/(n-1);
            z = log(alpha)/log(n);
            break;
         }
      }

      float* r = new float[N+1];

      for(int i = 0; i <= N; i++)
      {
         r[i] = (pow(n,z)*(pow(n,i)-1)+1)*xIn[0];
      }

      xOut = MakeMatrix2D(N,1);
      yOut = MakeMatrix2D(N,1);
      int indx1,indx2;
      for(int i = 0; i < N; i++)
      {
         indx1 = FindIndexCore(xIn,sz,r[i]);
         indx2 = FindIndexCore(xIn,sz,r[i+1]);
         if(indx2 == indx1)
         {
            xOut[0][i] = xIn[indx1];
            yOut[0][i] = yIn[indx1];
         }
         else
         {
            xOut[0][i] = sumVec(xIn,sz,indx1,indx2-1)/(indx2-indx1);
            yOut[0][i] = sumVec(yIn,sz,indx1,indx2-1)/(indx2-indx1);
         }
      }
   }
   else
   {
      ErrorMessage("Invalid input matrix");
      return(ERR);
   }

// Return solution to Prospa
   itfc->retVar[1].AssignMatrix2D(xOut,N,1);
   itfc->retVar[2].AssignMatrix2D(yOut,N,1);
   itfc->nrRetValues = 2;

   return(r); 

}


/***************************************************************************************
    Generate a vector which uses pseudo-log spacing in which the first gap
    is at least as large as the first value. Useful for reducing 
    the size of CPMG data sets before applying the Maximum Entropy inversion
***************************************************************************************/

int PseudoLogVector(Interface* itfc ,char args[])
{
   short r;
   long N;
   float maxT, minT;

   if((r = ArgScan(itfc,args,3,"start, end, points","eee","ffl",
                    &minT, &maxT, &N)) < 0)
        return(r);

 
   if(minT <= 0)
   {
      ErrorMessage("start point should be > 0");
      return(ERR);
   }

   double q = maxT/minT;

   if(N >= q)
   {
      ErrorMessage("(end_point/start_point) should be > number_of_points");
      return(ERR);
   }

   double* x = new double[N+1];
   double* y = new double[N+1];

      
   double n = 1.999;
   double del = 1;
   double p = N;
   double m,alpha,z;

   while(1)
   {
      m = (pow(n,p)-1)/(n-1);
      if(m < q)
         n = n + del;
      else
         n = n -del;
            
      del = del/2;
      if(del < 1e-8)
      {
         alpha = 1/(n-1);
         z = log(alpha)/log(n);
         break;
      }
   }

   for(int i = 0; i <= N; i++)
   {
      y[i] = (pow(n,z)*(pow(n,i)-1))*minT;
   }

// Copy to output vector
   float **yOut = MakeMatrix2D(N,1);
   for(int i = 0; i < N; i++)
   {
      yOut[0][i] = y[i+1];
   }

   delete [] x;
   delete [] y;

// Return solution to Prospa
   itfc->retVar[1].AssignMatrix2D(yOut,N,1);
   itfc->nrRetValues = 1;

   return(r); 

}

/*************************************************************************
             Sum elements of a vector from start to end
**************************************************************************/

float sumVec(float* vec, long N, long start, long end)
{
   float result = 0;

   for(int i = start; i <= end; i++)
   {
      assert(i >= 0 && i < N);
      result += vec[i];
   }
   return(result);
}

/*************************************************************************
             Sum elements of a vector from start to end
**************************************************************************/

double sumVec(double* vec, long N, long start, long end)
{
   double result = 0;

   for(int i = start; i <= end; i++)
   {
      assert(i >= 0 && i < N);
      result += vec[i];
   }
   return(result);
}


/*************************************************************************
Make a log spaced vector of N elements starting at minT and ending at maxT
**************************************************************************/

int LogVector(Interface* itfc ,char args[])
{
   short r;
   long N;
   Variable maxTVar, minTVar;
   double step,x;

   if((r = ArgScan(itfc,args,3,"start, end, points","eee","vvl",
                    &minTVar, &maxTVar, &N)) < 0)
        return(r);

   if(minTVar.GetType() == FLOAT32 && minTVar.GetType() == FLOAT32)
   {
      float minT = minTVar.GetReal();
      float maxT = maxTVar.GetReal();

      if(minT <= 0)
      {
         ErrorMessage("start point should be > 0");
         return(ERR);
      }

      if(maxT < minT)
      {
         ErrorMessage("end point should be > start point");
         return(ERR);
      }

      float **xOut = MakeMatrix2D(N,1);

      step = (log10(maxT)-log10(minT))/(N-1);
      x = log10(minT);
      for(int i = 0; i < N; i++)
      {
         xOut[0][i] = pow(10.0,x);
         x += step;
      }

   // Return solution to Prospa
      itfc->retVar[1].AssignMatrix2D(xOut,N,1);
      itfc->nrRetValues = 1;

      return(r); 

   }
   else if(minTVar.GetType() == FLOAT64 && minTVar.GetType() == FLOAT64)
   {
      double minT = minTVar.GetDouble();
      double maxT = maxTVar.GetDouble();

      if(minT <= 0)
      {
         ErrorMessage("start point should be > 0");
         return(ERR);
      }

      if(maxT < minT)
      {
         ErrorMessage("end point should be > start point");
         return(ERR);
      }

      double **xOut = MakeDMatrix2D(N,1);

      step = (log10(maxT)-log10(minT))/(N-1);
      x = log10(minT);
      for(int i = 0; i < N; i++)
      {
         xOut[0][i] = pow(10.0,x);
         x += step;
      }

   // Return solution to Prospa
      itfc->retVar[1].AssignDMatrix2D(xOut,N,1);
      itfc->nrRetValues = 1;

      return(r); 
   }
   else
   {
      ErrorMessage("start and end parameters should both be float or double");
      return(ERR);
   }
}
