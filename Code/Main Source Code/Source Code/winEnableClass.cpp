#include "stdafx.h"
#include "winEnableClass.h"
#include "guiMakeObjects.h"
#include "memoryLeak.h"

// Disable all windows but not first if they are visible

void CWinEnable::Disable(HWND win)
{
   DisableAllWindows(win);

   // Check main windows
 //  prospaWinEnabled = IsWindowEnabled(prospaWin);
 //  plot1DWinEnabled = IsWindowEnabled(plot1DWin);
//   plot2DWinEnabled = IsWindowEnabled(plot2DWin);
//   plot3DWinEnabled = IsWindowEnabled(plot3DWin);
//   cliWinEnabled    = IsWindowEnabled(cliWin);
 //  editWinEnabled   = IsWindowEnabled(editWin);

//   EnableWindow(prospaWin,false);
 //  EnableWindow(plot1DWin,false);
 //  EnableWindow(plot2DWin,false);
 //  EnableWindow(plot3DWin,false);
 //  EnableWindow(cliWin,false);
//   EnableWindow(editWin,false);
}

// Enable all windows if they were previously enabled
// i.e. for call to Disable

void CWinEnable::Enable(HWND win)
{
   EnableAllWindows(win);

   // Enable main windows
 //  if(prospaWinEnabled) EnableWindow(prospaWin,true);
 //  if(plot1DWinEnabled) EnableWindow(plot1DWin,true);
//   if(plot2DWinEnabled) EnableWindow(plot2DWin,true);
  /// if(plot3DWinEnabled) EnableWindow(plot3DWin,true);
 //  if(cliWinEnabled)    EnableWindow(cliWin,true);
 //  if(editWinEnabled)   EnableWindow(editWin,true);

}