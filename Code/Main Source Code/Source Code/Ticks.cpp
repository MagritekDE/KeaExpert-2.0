#include "stdafx.h"
#include "Ticks.h"
#include "plotwindefaults.h"
#include "memoryLeak.h"

Ticks::Ticks(double majorLength, double minorLength, double spacing, double perLabel, 
				 ProspaFont* font, COLORREF fontColor)
: font_(*font), Scalable()
{
	font_.setColor(fontColor);
	majorTickLength_ = majorLength;
	minorTickLength_ = minorLength;
	tickSpacing_ = spacing;
	ticksPerLabel_ = perLabel;
	calculateLabelSpacing();
	scaleFactor_ = 1;
}


Ticks::Ticks(const Ticks& copyMe)
: font_(copyMe.font_), Scalable(copyMe)
{
	majorTickLength_ = copyMe.majorTickLength_;
	minorTickLength_ = copyMe.minorTickLength_;
	tickSpacing_ = copyMe.tickSpacing_;
	ticksPerLabel_ = copyMe.ticksPerLabel_;
	calculateLabelSpacing();
	scaleFactor_ = copyMe.scaleFactor_;
}

Ticks::~Ticks()
{
}

void Ticks::calculateLabelSpacing()
{
	labelSpacing_ = tickSpacing_*ticksPerLabel_;
}

short Ticks::save(FILE* fp)
{
	if (!fp)
		return ERR;

	LOGFONT f = font_.font();
	COLORREF c = font_.color();

	float temp = majorTickLength_;
	fwrite(&temp, sizeof(float), 1, fp);
	temp = minorTickLength_;
	fwrite(&temp, sizeof(float), 1, fp);
	temp = tickSpacing_;
	fwrite(&temp, sizeof(float), 1, fp);
	temp = ticksPerLabel_;
	fwrite(&temp, sizeof(float), 1, fp);
	fwrite(&f, sizeof(LOGFONT), 1, fp);
	fwrite(&c, sizeof(COLORREF), 1, fp);
	
	return OK;	
}

void Ticks::scale_(double x, double y)
{
	majorTickLength_ *= x; // double majorTickLength_
	minorTickLength_ *= x; // double minorTickLength_
	font_.scale(x, y/2);
}

void Ticks::unscale_()
{
	majorTickLength_ *= x_inverse_scaling_factor(); // double majorTickLength_
	minorTickLength_ *= x_inverse_scaling_factor(); // double minorTickLength_
	font_.unscale();
}