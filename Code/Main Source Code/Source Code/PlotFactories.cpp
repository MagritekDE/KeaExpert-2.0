#include "stdafx.h"
#include "plot.h"
#include "memoryLeak.h"

Plot1D* Plot1DFactory::makePlot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col)
{
	return new Plot1D(pd, hWnd, pp, row, col, defaultMargins1D);
}

Plot2D* Plot2DFactory::makePlot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col)
{
	return new Plot2D(pd, hWnd, pp, row, col, defaultMargins2D);
}
