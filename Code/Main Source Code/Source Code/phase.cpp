#include "stdafx.h"
#include "phase.h"
#include <math.h>
#include "globals.h"
#include "interface.h"
#include "scanstrings.h"
#include "utilities.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"


int AutoPhase2(char args[]);
short GetBestFit(complex*, float, long,long,float*,float*);
float AutoPhaseCoreMin(complex *data, long left,long right,long n);
float AutoPhaseCoreMax(complex *data, long left,long right);

/*******************************************************************************
           Automatically determine the phase shift required to bring
           a 1D complex vector to in-phase form.            
*******************************************************************************/

int AutoPhase(Interface* itfc, char args[])
{
   short r;                     // Error flag and number of arguments passed
   long rightIndex,leftIndex;   // Range of indicies for phasing over
   long datWidth;               // Width of data set to phase
   float phase;                 // Returned phase value
   Variable x;                  // x axis vector for data set to phase
   Variable y;                  // Data set to phase
   long i;                      // Data index
   CText method = "maximise";  // Autophase method {maximise, minimise}
   CText centerMode = "recenter";    // Center the data {recenter, uselimits}


// Get arguments
	if((r = ArgScan(itfc,args,1,"y,left,right,[method1, [method2]]","eeeee","vlltt",&y,&leftIndex,&rightIndex,&method,&centerMode)) < 0)
	   return(r); 

// Check for a valid vector
   if(VarType(&y) != CMATRIX2D && VarHeight(&y) != 1)
   {
      ErrorMessage("invalid vector for phasing");
      return(ERR);
   }
   datWidth = VarWidth(&y);

// Work out the index range if none given
   if(r == 1)
   {
      leftIndex = 0;
      rightIndex = datWidth-1;
   }

// Make sure the selected region for phasing is valid  
   if(rightIndex == leftIndex)
   {
      ErrorMessage("peak region must be greater than zero");
      return(ERR);
   }   
   
   if(rightIndex < 0 || leftIndex < 0 || rightIndex >= datWidth || leftIndex >= datWidth)
   {
      ErrorMessage("invalid peak");
      return(ERR);
   }
      
   if(rightIndex < leftIndex)
      Swap(rightIndex,leftIndex);

// Search for the index of peak
   complex *data = VarComplexMatrix(&y)[0];
   long maxIndex = -1;
   float mag,max = -1;
   for(i = leftIndex; i <= rightIndex; i++)
   {
      mag = data[i].r*data[i].r + data[i].i*data[i].i;
      if(mag > max)
      {
         max = mag;
         maxIndex = i;
      }
   }

   if(maxIndex == -1)
   {
      ErrorMessage("peak not found in specified region");
      return(ERR);
   }

// Re-position left and right indices
   if(centerMode == "recenter")
   {
      long width = rightIndex-leftIndex;
      leftIndex = maxIndex - width/2;
      rightIndex = maxIndex + width/2;   
   }

// Work out the phase shift for this region 
   if(method == "maximise")
   {
   // Make sure left and right indices are valid
      if(leftIndex < 0) 
         leftIndex = 0;
      if(rightIndex > datWidth) 
         rightIndex = datWidth - 1;
      phase = AutoPhaseCoreMax(data, leftIndex, rightIndex);
   }
   else if(method == "minimise")
   {
   // Make sure left and right indices are valid
      if(leftIndex < 6) 
         leftIndex = 5;
      if(rightIndex > datWidth-6) 
         rightIndex = datWidth - 5;
      phase = AutoPhaseCoreMin(data, leftIndex, rightIndex, 5);
   }
   else
   {
      ErrorMessage("Invalid autophase method");
      return(ERR);
   }

   itfc->retVar[1].MakeAndSetFloat(phase);
   itfc->nrRetValues = 1;
   
   return(0);
} 


/*******************************************************************************
   Core routine for AutoPhase - adjusts the phase shift in 20 degree steps then 
   1 degree steps looking to minimise the area under the real part of the spectrum
   between indices left and right. Some averaging is done around these points
   to minimise errors.

   Applied function is Dr*cos(phi) + Di*sin(phi) this comes
   from the real part of data*exp(-i*phi)= (Dr+i*Di)*(cos(phi)-i*sin(phi)
*******************************************************************************/

float AutoPhaseCoreMin(complex *data, long left,long right, long n)
{ 
   long i;
   float maxAng,minSum;
   float rightVal,leftVal;
   float cosVal,sinVal,t,sum;
   float theta;
   float K,m = 0,c = 0,liney,real;    
   float midAng;

// Phase this point for maximum area using 20 degree steps 
   K = 2*PI/360.0;
   minSum = 1e30;
  
   for(maxAng = 0, theta = 0; theta < 360; theta += 20)
   {
      t = K*theta;
      cosVal = cos(t);
      sinVal = sin(t);
   
  // Work out right and left baseline values by averaging a few points either side
      for(leftVal = 0.0, i = left-n; i <= left+n; i++)
         leftVal += cosVal*data[i].r + sinVal*data[i].i;
      leftVal /= n*2+1;
      for(rightVal = 0.0, i = right-n; i <= right+n; i++)
         rightVal += cosVal*data[i].r + sinVal*data[i].i;
      rightVal /= n*2+1;


   //   leftVal = cosVal*data[left].r + sinVal*data[left].i;
   //   rightVal = cosVal*data[right].r + sinVal*data[right].i;

   // Determine line through these points 
      m = (rightVal - leftVal)/(right-left);
      c = rightVal - m*right;
            
   // Calculate integral of spectrum below line for each angle   
   // Cube signal to minmise influence of noise
      sum = 0.0;
      for(i = left; i <= right; i++)
      {
         real = cosVal*data[i].r + sinVal*data[i].i;
         if(real < 0) 
            real = -(real*real);
         else
            real = (real*real);
         liney = m * i + c;
         if(liney < 0) 
            liney = -(liney*liney);
         else
            liney = (liney*liney);
         if(real < liney) sum += (liney - real);
      }
      if(sum < minSum)
      {
         minSum = sum;
         maxAng = theta;
      }
   }

//  Repeat using 1 degree steps 
   K = 2*PI/360.0;
   midAng = maxAng;
   
   for(theta = midAng-10; theta <= midAng+10; theta++)
   {
      t = K*theta;
      cosVal = cos(t);
      sinVal = sin(t);
            
   // Determine line parameters       
      rightVal = cosVal*data[right].r + sinVal*data[right].i;
      leftVal = cosVal*data[left].r + sinVal*data[left].i;

      for(leftVal = 0.0, i = left-n; i <= left+n; i++)
         leftVal += cosVal*data[i].r + sinVal*data[i].i;
      leftVal /= n*2+1;
      for(rightVal = 0.0, i = right-n; i <= right+n; i++)
         rightVal += cosVal*data[i].r + sinVal*data[i].i;
      rightVal /= n*2+1;
      
   //   m = (rightVal - leftVal)/(right-left);
   //   c = rightVal - m*right;

   // Calculate integral of spectrum below line for each angle             
      sum = 0.0;
      for(i = left; i <= right; i++)
      {
         real = cosVal*data[i].r + sinVal*data[i].i;
         if(real < 0) 
            real = -(real*real);
         else
            real = (real*real);
         liney = m * i + c;
         if(liney < 0) 
            liney = -(liney*liney);
         else
            liney = (liney*liney);
         if(real < liney) sum += (liney - real);
      }
      if(sum < minSum)
      {
         minSum = sum;
         maxAng = theta;
      }
   }

   return(maxAng);
}



/*******************************************************************************
   Core routine for AutoPhase - adjusts the phase shift in 20 degree steps then 
   1 degree steps looking to maximise the area under the real part of the spectrum
   between indices left and right.

   Applied function is Dr*cos(phi) + Di*sin(phi) this comes
   from the real part of data*exp(-i*phi)= (Dr+i*Di)*(cos(phi)-i*sin(phi)
*******************************************************************************/

float AutoPhaseCoreMax(complex *data, long left,long right)
{ 
   long i;
   float maxAng = 0;
	float maxSum;
   float cosVal,sinVal,t,sum;
   float theta;
   float K;    
   float midAng;

// Phase this point for maximum area using 20 degree steps 
   K = 2*PI/360.0;
   maxSum = -1e30;
   
   for(midAng = 0, theta = 0; theta < 360; theta += 20)
   {
      t = K*theta;
      cosVal = cos(t);
      sinVal = sin(t);
      
   // Calculate integral of spectrum above line for each angle            
      sum = 0.0;
      for(i = left; i <= right; i++)
      {
         sum += cosVal*data[i].r + sinVal*data[i].i;
      }
      if(sum > maxSum)
      {
         maxSum = sum;
         maxAng = theta;
      }
   }

//  Repeat using 1 degree steps 
   K = 2*PI/360.0;
   midAng = maxAng;
   
   for(theta = midAng-10; theta <= midAng+10; theta++)
   {
      t = K*theta;
      cosVal = cos(t);
      sinVal = sin(t);

   // Calculate integral of spectrum below line for each angle             
      sum = 0.0;
      for(i = left; i <= right; i++)
      {
         sum += cosVal*data[i].r + sinVal*data[i].i;
      }
      if(sum > maxSum)
      {
         maxSum = sum;
         maxAng = theta;
      }
   }

   return(maxAng);
}

/* Alternative routine which averages points left and right to find best baseline
float AutoPhaseCore(complex *data, long left,long right)
{ 
   long i;
   float maxAng,minSum;
     
   
//  Phase this point for maximum area using 20 degree steps 

   {
      float cosVal,sinVal,t,sum;
      float theta;
      float K,m,c,liney,real;

      K = 2*PI/360.0;
      minSum = 1e30;
      
      for(theta = 0; theta < 360; theta += 20)
      {
         t = K*theta;
         cosVal = cos(t);
         sinVal = sin(t);
      
      // Determine line parameters 
      
         GetBestFit(data,t,right,left,&m,&c);
      
      // Calculate integral of spectrum below line for each angle 
            
         sum = 0.0;
         for(i = left; i <= right; i++)
         {
            real = cosVal*data[i].r + sinVal*data[i].i;
            liney = m * i + c;
            
            if(real < liney) sum += (liney - real);
         }
         if(sum < minSum)
         {
            minSum = sum;
            maxAng = theta;
         }
      }
   }
   
//  Repeat using 1 degree steps 

   {
      float cosVal,sinVal,t,sum;
      float theta;
      float K,m,c,liney,real;
      float midAng;
      
      K = 2*PI/360.0;
      midAng = maxAng;
      
      for(theta = midAng-10; theta <= midAng+10; theta++)
      {
         t = K*theta;
         cosVal = cos(t);
         sinVal = sin(t);
               
      // Determine line parameters

         GetBestFit(data,t,right,left,&m,&c);

      // Calculate integral of spectrum below line for each angle 
            
         sum = 0.0;
         for(i = left; i <= right; i++)
         {
            real = cosVal*data[i].r + sinVal*data[i].i;
            liney = m * i + c;
            
            if(real < liney) sum += (liney - real);
         }
         if(sum < minSum)
         {
            minSum = sum;
            maxAng = theta;
         }
      }
   }
   return(maxAng);
}
*/
short GetBestFit(complex *data, float theta, long right,long left,float *m,float *c)
{
   long x;
   float rightSum = 0,leftSum = 0;
   float cosVal,sinVal;
   
   cosVal = cos(theta);
   sinVal = sin(theta);  
    
   for(x = left - 4; x <= left + 4; x++)
   {
      leftSum += cosVal*data[x].r + sinVal*data[x].i;
   }

   for(x = right - 4; x <= right + 4; x++)
   {
      rightSum += cosVal*data[x].r + sinVal*data[x].i;
   }
      
   rightSum /= 9.0;
   leftSum /= 9.0;
      
   *m = (rightSum - leftSum)/(right-left);
   *c = rightSum - (*m)*right;  

   return(OK);
}  
 