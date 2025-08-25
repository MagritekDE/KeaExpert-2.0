#include "stdafx.h"
#include "drawvector.h"
#include <math.h>
#include "allocate.h"
#include "globals.h"
#include "plot.h"
#include "PlotWindow.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/****************************************************************************
*                                  DrawVectors
*
* Using two matrices Vx and Vy construct a vector plot i.e. each data point
* Vx[u,v]*i + Vy[u,v]*j is represented by an arrow with a colour corresponding
* to its magnitude and a direction corresponding to its phase angle.
*
*****************************************************************************/

int DrawVectors(Interface* itfc ,char args[])
{
   short na;                   // Number of arguments passed by user
   Variable Vx,Vy;             // Matrix variables
   float vLength;              // Chosen length of vectors in pixels
   float **mx,**my;            // Matrices to plot
   short xstep = 1,ystep = 1;  // Determines spacing between data points which will be plotted

	Plot2D* cur2DPlot = Plot2D::curPlot();

// Get the matrix names and curdor length (and optionally sampling spacing)
   if((na = ArgScan(itfc,args,3,"x_matrix, y_matrix, length, [xstep, ystep], [","eeeee","vvfdd",&Vx,&Vy,&vLength,&xstep,&ystep)) < 0)
    return(na);

   if(!cur2DPlot)
   {
      ErrorMessage("2D plot is not defined");
      return(ERR);
   }

// Check to be sure the passed variables are matrices and have the same dimensions
   if(VarType(&Vx) != MATRIX2D || VarType(&Vy) != MATRIX2D)
   {
      ErrorMessage("data must be real 2D matrices");
      return(ERR);
   }
   
   if(VarRowSize(&Vx) != VarRowSize(&Vy) || VarColSize(&Vx) != VarColSize(&Vy))
   {
      ErrorMessage("Matrix sizes must match");
      return(ERR);
   }   

// Extract the matrices and check for valid step size   
   mx = VarRealMatrix(&Vx);
   my = VarRealMatrix(&Vy);

   if(xstep < 0 || xstep > VarWidth(&Vx))
   {
      ErrorMessage("invalid x-step size");
      return(ERR);
   }
   
   if(ystep < 0 || ystep > VarHeight(&Vx))
   {
      ErrorMessage("invalid y-step size");
      return(ERR);
   }      

// Delete any existing data in current plot   
   cur2DPlot->setDrawMode(DISPLAY_VECTORS);
   cur2DPlot->clearData();

// Add vector component matrices   
   cur2DPlot->setMatWidth(VarWidth(&Vx));
   cur2DPlot->setMatHeight(VarHeight(&Vx));
   cur2DPlot->setVX(MakeMatrix2D(cur2DPlot->matWidth(),cur2DPlot->matHeight()));
   cur2DPlot->setVY(MakeMatrix2D(cur2DPlot->matWidth(),cur2DPlot->matHeight()));
 
// Set the x and y axis range
	cur2DPlot->curXAxis()->initialiseRange();
	cur2DPlot->curYAxis()->initialiseRange();

// Copy input matrices to current plot buffers noting maximum and minimum plot lengths 
   float minVal = 1e9;
   float maxVal = -1e9;
   float length;   
   for(long i = 0; i < cur2DPlot->matHeight(); i++)
   {
      for(long j = 0; j < cur2DPlot->matWidth(); j++)
      {
         cur2DPlot->vx()[i][j] = mx[i][j];
         cur2DPlot->vy()[i][j] = my[i][j];
         length = sqrt(mx[i][j]*mx[i][j] + my[i][j]*my[i][j]);
         if(length > maxVal) maxVal = length;
         if(length < minVal) minVal = length;
      }
   }
   if(!cur2DPlot->getOverRideAutoRange())
   {
      cur2DPlot->setMinVal(minVal);
      cur2DPlot->setMaxVal(maxVal);
   }

// Update other relevant parameters           
   cur2DPlot->xVectorStep = xstep;
   cur2DPlot->yVectorStep = ystep;
   cur2DPlot->setVectorLength(vLength);

	cur2DPlot->resetDataView();

// Display the image if desired ****************
   cur2DPlot->DisplayAll(false);
	         
   return(OK);
}

