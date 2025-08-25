/* Library includes */
#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include "nelder_mead.h"

short PhaseCorrection(DLLParameters* par, char* args);


/********************************************************************
* Find the p0 and p1 phases which give an optimal spectrum
* Do this by adjust these parameter using a 2D simplex search
* The criterion for best phase is that the differential of the
* phased data will be symmetrical about zero.
* 
* Adapted from Cameron Dykstra's Eigen and cppoptlib code.
* Uses the Nelder-Mead simplex algorithm see header file for details
*********************************************************************/

static complex* dataIn;
static double* dataWork;
static long size;
static double gP1;

double p1TargetFunction(const std::array<double, 1>& x);
double p0TargetFunction(const std::array<double, 1>& x);

short PhaseCorrection(DLLParameters* par, char* args)
{
   long seg;
   short n;
   short order = 0;
   Variable varData;
   float initPhase0 = 0.0;
   float initPhase1 = 0.0;

   // Get parameters
   if ((n = ArgScan(par->itfc, args, 1, "data, order (0/1), initP0, initP1", "eeee", "vdff", &varData, &order, &initPhase0, &initPhase1)) < 0)
      return(n);

   if (varData.GetType() != CMATRIX2D || varData.GetDimY() != 1)
   {
      ErrorMessage("Input data should be a 1D complex vector");
      return(ERR);
   }

   dataIn = varData.GetCMatrix2D()[0];
   size = varData.GetDimX();
   dataWork = new double[size];

   if (order == 0)
   {
      std::array<double, 1> start = { initPhase0 };
      std::array<double, 1> step = { 0.1 };

      gP1 = 0;

      nelder_mead_result<double, 1> result = nelder_mead<double, 1>(p0TargetFunction, start, 1.0e-25, step);

      par->retVar[1].MakeAndSetFloat(result.xmin[0]);
      par->retVar[2].MakeAndSetDouble(result.ynewlo);

      par->nrRetVar = 2;

      delete[] dataWork;

      return(OK);
   }
   else
   {
      std::array<double, 1> start = { initPhase1 };
      std::array<double, 1> step = { 0.1 };

      nelder_mead_result<double, 1> result = nelder_mead<double, 1>(p1TargetFunction, start, 1.0e-25, step);

      gP1 = result.xmin[0];

      if (gP1 > 180)
         gP1 = -(360 - gP1);

      double p0Init; // Need this from the fid - another parameter? = atan(dataIn->i / dataIn->r) * 180.0 / PI;
      p0Init = 0;

      start[0] = initPhase0;
      step[0] = 0.1;

      result = nelder_mead<double, 1>(p0TargetFunction, start, 1.0e-25, step);

      par->retVar[1].MakeAndSetFloat(result.xmin[0]);
      par->retVar[2].MakeAndSetFloat(gP1);
      par->retVar[3].MakeAndSetDouble(result.ynewlo);

      par->nrRetVar = 3;

      delete[] dataWork;

      return(OK);
   }
}

double p1TargetFunction(const std::array<double, 1>& x)
{
   double target;

   double p0 = 0;
   double p1 = x[0];

   // Apply a phase shift to the data - keep only the real part
   for (int i = 0; i < size; i++)
   {
      double ph = (p0 + i * p1 / double(size)) * PI / 180;
      dataWork[i] = dataIn[i].r * cos(ph) - dataIn[i].i * sin(ph);
   }

   // Calculate the differential
   for (int i = 1; i < size; i++)
   {
      dataWork[i - 1] = dataWork[i] - dataWork[i - 1];
   }

   // Work out the target function which is just the average of the differential
   double sum = 0;
   for (int i = 0; i < size-1; i++)
   {
      sum += dataWork[i];
   }
   double result = fabs(sum / (size-1));
  // TextMessage("%g %g\n", p1, result);
   return(result); 
}


double p0TargetFunction(const std::array<double, 1>& x)
{
   double target;

   double p0 = x[0];
   double p1 = gP1;

   // Apply a phase shift to the data - keep only the real part
   for (int i = 0; i < size; i++)
   {
      double ph = (p0 + i * p1 / double(size)) * PI / 180;
      dataWork[i] = dataIn[i].r * cos(ph) - dataIn[i].i * sin(ph);
   }

   // Work out the differential
   for (int i = 1; i < size; i++)
   {
      dataWork[i - 1] = dataWork[i] - dataWork[i - 1];
   }

   // Work out the noise level of the differential
   double sum = 0;
   for (int i = 0; i < size-1; i++)
   {
      sum += dataWork[i] * dataWork[i];
   }
   double rms = sqrt(sum / (size-1));

   // Threshold the data
   double threshold = 3 * rms;
   double last = 0;
   for (int i = 0; i < size-1; i++)
   {
      double val = dataWork[i];
      if (val > threshold)
      {
         val -= threshold;
      }
      else if (val < -threshold)
      {
         val += threshold;
      }
      else
      {
         val = 0;
      }
      dataWork[i] = last + val;
      last = dataWork[i];
   }

   // Work out the target function which is just the average of the thresholded data
   sum = 0;
   for (int i = 0; i < size-1; i++)
   {
      sum += dataWork[i];
   }

   double result = fabs(sum / (size-1));
  // TextMessage("%g %g\n", p0, result);

   return(result);
}