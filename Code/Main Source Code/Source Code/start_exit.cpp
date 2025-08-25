#include "stdafx.h"
#include <stdio.h>
#include "defineWindows.h"
#include "memoryLeak.h"

/***************************************************************************************
   Save window positions and sizes as a fraction of screen size to the parameter file
   (Note that the screen size does not include the taskbar.)
****************************************************************************************/

void SaveWinParameters(FILE *fp)
{

// Hide all windows ****************************
 //  fprintf(fp,"hidewindow(\"plot1d\")\n");
 //  fprintf(fp,"hidewindow(\"plot2d\")\n");
   fprintf(fp,"hidewindow(\"plot3d\")\n");
//   fprintf(fp,"hidewindow(\"editor\")\n");
   fprintf(fp,"hidewindow(\"cli\")\n");

// Save position and size of windows ***********
   fprintf(fp,"movewindow(\"prospa\",%1.4f,%1.4f,%1.4f,%1.4f)\n",
           prospaRect.x,prospaRect.y,prospaRect.w,prospaRect.h);
//   fprintf(fp,"movewindow(\"plot1d\",%1.4f,%1.4f,%1.4f,%1.4f)\n",
//           plot1DRect.x,plot1DRect.y,plot1DRect.w,plot1DRect.h);
//   fprintf(fp,"movewindow(\"plot2d\",%1.4f,%1.4f,%1.4f,%1.4f)\n",
//           plot2DRect.x,plot2DRect.y,plot2DRect.w,plot2DRect.h);
 //  fprintf(fp,"movewindow(\"plot3d\",%1.4f,%1.4f,%1.4f,%1.4f)\n",
 //          plot3DRect.x,plot3DRect.y,plot3DRect.w,plot3DRect.h);
 //  fprintf(fp,"movewindow(\"cli\",%1.4f,%1.4f,%1.4f,%1.4f)\n",
 //          cliRect.x,cliRect.y,cliRect.w,cliRect.h);   
 //  fprintf(fp,"movewindow(\"editor\",%1.4f,%1.4f,%1.4f,%1.4f)\n",
 //          editRect.x,editRect.y,editRect.w,editRect.h);

// Show windows (if visible) *********************
//   if(GetWindowLong(plot1DWin,GWL_STYLE) & WS_VISIBLE)
 //     fprintf(fp,"showwindow(\"plot1d\")\n");
 //  if(GetWindowLong(plot2DWin,GWL_STYLE) & WS_VISIBLE)
 //     fprintf(fp,"showwindow(\"plot2d\")\n");
//   if(GetWindowLong(plot3DWin,GWL_STYLE) & WS_VISIBLE)
//      fprintf(fp,"showwindow(\"plot3d\")\n");      
 //  if(GetWindowLong(editWin,GWL_STYLE) & WS_VISIBLE)
 //     fprintf(fp,"showwindow(\"editor\")\n");
 //  if(GetWindowLong(cliWin,GWL_STYLE) & WS_VISIBLE)
//      fprintf(fp,"showwindow(\"cli\")\n");
}

