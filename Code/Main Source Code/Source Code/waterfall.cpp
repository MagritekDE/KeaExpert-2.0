#include "stdafx.h"
#include <math.h>
#include "allocate.h"
#include "globals.h"
#include "interface.h"
#include "mymath.h"
#include "plot.h"
#include "plot2dCLI.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

int DrawWaterFallPlot(Interface* itfc ,char args[]);


int DrawWaterFallPlot(Interface* itfc ,char args[])
{
   short na;                   // Number of arguments passed by user
   Variable vMat;
   Variable vColor;
   int i,j;
   float alpha,beta;
   

	Plot2D* cp = Plot2D::curPlot();

// Get the matrix names and curdor length (and optionally sampling spacing)
   if((na = ArgScan(itfc,args,4,"matrix","eeee","vvff",&vMat,&vColor,&alpha,&beta)) < 0)
    return(na);

// Delete any existing data in current plot   
   cp->setDrawMode(DISPLAY_WATERFALL);

// Is a matrix is being imaged? ******************
   if(vColor.GetType() != MATRIX2D || vColor.GetDimX() != 3 ||  vColor.GetDimY() != 1)
   {
      ErrorMessage("invalid waterfall color");
      return(ERR);
   }

   float *cArray = vColor.GetMatrix2D()[0];
   cp->wfColor = RGB(cArray[0],cArray[1],cArray[2]);

// Is a matrix is being imaged? ******************
   if(vMat.GetType() == MATRIX2D)
   {
      float **mat = VarRealMatrix(&vMat);
      long w = vMat.GetDimX();
      long h = vMat.GetDimY();

   // Copy matrix to the 2D plot matrix ********** 
      cp->clearData();
	   cp->clearColorMap();
      cp->setMatWidth(w);
      cp->setMatHeight(h);
      cp->setMat(MakeMatrix2D(w,h));
      if(!cp->mat())
      {
         ErrorMessage("out of memory!");
         return(ERR);
      }
      
      for(i = 0; i < h; i++)
         for(j = 0; j < w; j++)
            cp->mat()[i][j] = mat[i][j];
      
		cp->resetDataView();

		cp->curXAxis()->initialiseRange();
		cp->curYAxis()->initialiseRange();

      cp->setColorMap(CopyMatrix(gDefaultColorMap,3,gDefaultColorMapLength));
      cp->setColorMapLength(gDefaultColorMapLength);

      cp->setAlpha(alpha);
      cp->setBeta(beta);

   }

	cp->resetDataView();

// Display the image if desired ****************
   cp->DisplayAll(false);
	      
   itfc->nrRetValues = 0;

   return(OK);

}