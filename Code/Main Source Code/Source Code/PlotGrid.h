#ifndef PLOTGRID_H
#define PLOTGRID_H

#include "Axis.h"
#include "Scalable.h"

class PlotGrid : public Scalable
{
public:
	static PlotGrid* makePlotGrid(AxisOrientation ao,  bool drawGrid, bool drawFineGrid, COLORREF color, COLORREF fineColor);
	PlotGrid(Axis* axis, bool drawGrid, bool drawFineGrid, COLORREF color, COLORREF fineColor);
	PlotGrid(const PlotGrid& copyMe);
	virtual ~PlotGrid();

	bool drawGrid() {return drawGrid_;}
	bool drawFineGrid() {return drawFineGrid_;}
	COLORREF color() {return color_;}
	COLORREF fineColor() {return fineColor_;}

	void setAxis(Axis* axis) {axis_ = axis;}

	void setDrawGrid(bool draw) {drawGrid_ = draw;}
	void setDrawFineGrid(bool draw) {drawFineGrid_ = draw;}
	void setColor(COLORREF color) {color_ = color; setPenColors(false);}
	void setFineColor(COLORREF fineColor) {fineColor_ = fineColor;setPenColors(false);}

	void setPenColors(bool blackAndWhite = false);
	
	void draw(HDC hdc);

	virtual PlotGrid* clone() = 0;

	short save(FILE* fp);
	short load(FILE* fp);

protected:
	void drawFineLine(HDC hdc, int position);
	void drawThickLine(HDC hdc, int position);
	virtual void drawLine(HDC hdc, int position, COLORREF color) = 0;

	Axis* axis_;
	HPEN gridPen_;
	HPEN fineGridPen_;
   short gridLineWidth_;

private:
	void deleteAllPens();

	bool  drawGrid_;       // Draw a dotted grid between major ticks
	bool  drawFineGrid_;   // Draw a dotted grid between minor ticks
	COLORREF color_;
	COLORREF fineColor_;
	virtual void scale_(double x, double y) = 0;
	virtual void unscale_() = 0;
};

class PlotGridVertical : public PlotGrid
{
public:
	PlotGridVertical(Axis* axis, bool drawGrid, bool drawFineGrid, COLORREF color, COLORREF fineColor);
	PlotGridVertical(const PlotGridVertical& copyMe);
	virtual ~PlotGridVertical();
	PlotGrid* clone();

private:
	void drawLine(HDC hdc, int position, COLORREF color);
	void scale_(double x, double y);
	void unscale_();
};

class PlotGridHorizontal : public PlotGrid
{
public:
	PlotGridHorizontal(Axis* axis, bool drawGrid, bool drawFineGrid, COLORREF color, COLORREF fineColor);
	PlotGridHorizontal(const PlotGridHorizontal& copyMe);
	virtual ~PlotGridHorizontal();
	PlotGrid* clone();

private:
	void drawLine(HDC hdc, int position, COLORREF color);
	void scale_(double x, double y);
	void unscale_();
};


#endif // define PLOTGRID_H