#ifndef PLOTLABEL_H
#define PLOTLABEL_H

#include "defines.h"
#include "font.h"
#include "Plot.h"
#include "Scalable.h"
#include "ShortRect.h"

#define PLOT_STR_LEN 200

typedef enum 
{
	TITLE_PLOT_LABEL,
	VERTICAL_AXIS_PLOT_LABEL,
	HORIZONTAL_AXIS_PLOT_LABEL
} PlotLabelType;


/**********************************************************************************
PlotLabels are the result of a first pass of factoring out a lot of the plot title 
and axis label code from Plot itself, resulting in a generic Plot Label with 
subclasses TitleLabel, VerticalAxisLabel and HorizontalAxisLabel.
**********************************************************************************/

class PlotLabel : public Scalable
{
public:
	static PlotLabel* makePlotLabel(PlotLabelType type, ProspaFont& font, char* text);

	PlotLabel(ProspaFont& font, char* text);
	PlotLabel(const PlotLabel& copyMe);
	virtual PlotLabel* clone() = 0;
	virtual ~PlotLabel();

	char* text() {return text_;}
	void setFont(ProspaFont& font) {font_ = font;}
	void setFontColor(COLORREF col) {font_.setColor(col);}
	void setFontSize(short size) {font_.setSize(size);}
	void setFontStyle(short style) {font_.setStyle(style);}
	void setFontName(const char* const  name) {font_.setName(name);}

	LOGFONT& font() {return font_.font();}
	ProspaFont& pfont() {return font_;}
	ShortRect& rect() {return rect_;}
	bool isTextLocked() {return textLocked_;}
	
	void setText(const char* const text){strncpy_s(text_, PLOT_STR_LEN, text, _TRUNCATE);}
	void clearText();
	void userSetText(const char* const text){setText(text); userHasSetText(true);}
	void userHasSetText(bool hasSetText) {textLocked_ = hasSetText;}

	short fontSize() {return font_.size();}
	COLORREF fontColor() {return font_.color();}
	short fontStyle() {return font_.style();}

	virtual void draw(HDC hdc){};// = 0;

	short save(FILE* fp);
	
private:
	ProspaFont font_;    // Font information for text
	ShortRect rect_;
	char  text_[PLOT_STR_LEN];  // Plot title
	bool textLocked_;
	PlotLabel& operator=(const PlotLabel &tmp);

	void scale_(double x, double y) {font_.scale(x,y);}
	void unscale_() {font_.unscale();}
};

class TitleLabel : public PlotLabel
{
public:
	TitleLabel(Plot* parent, ProspaFont& font, char* text);
	TitleLabel(const TitleLabel& copyMe);
	PlotLabel* clone();
	~TitleLabel();
	
	void setParent(Plot* parent) {plot_ = parent;}
	void draw(HDC hdc);
	
private:
	TitleLabel& operator=(const TitleLabel &tmp);
	Plot* plot_;
};

class HorizontalAxisLabel : public PlotLabel
{
public:
	HorizontalAxisLabel(Axis* parent, ProspaFont& font, char* text);
	HorizontalAxisLabel(const HorizontalAxisLabel& copyMe);
	PlotLabel* clone();
	~HorizontalAxisLabel();
	
	void setParent(Axis* parent) {axis_ = parent;}
	void draw(HDC hdc);

private:
	HorizontalAxisLabel& operator=(const HorizontalAxisLabel &tmp);
	Axis* axis_;
};

class VerticalAxisLabel : public PlotLabel
{
public:
	VerticalAxisLabel(Axis* parent,ProspaFont& font, char* text);
	VerticalAxisLabel(const VerticalAxisLabel& copyMe);
	PlotLabel* clone();
	~VerticalAxisLabel();
	
	void setParent(Axis* parent) {axis_ = parent;}
	void draw(HDC hdc);

private:
	VerticalAxisLabel& operator=(const VerticalAxisLabel &tmp);
	Axis* axis_;
};


#endif // define PLOTLABEL_H