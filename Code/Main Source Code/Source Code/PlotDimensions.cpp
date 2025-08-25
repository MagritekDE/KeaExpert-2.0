#include "stdafx.h"
#include "PlotDimensions.h"
#include "Plot.h"
#include "Scalable.h"
#include "memoryLeak.h"

PlotDimensions::PlotDimensions(void)
: Scalable()
{
	width_ = 0;
	height_ = 0;
	top_ = 0;
	bottom_ = 0;
	left_ = 0;
	right_ = 0;
	rightFudge_ = 0;
	bottomFudge_ = 0;
}

PlotDimensions::PlotDimensions(const PlotDimensions& copyMe)
: Scalable(copyMe)
{
	width_ = copyMe.width_;
	height_ = copyMe.height_; 
	top_ = copyMe.top_;
	bottom_ = copyMe.bottom_;
	left_ = copyMe.left_;
	right_ = copyMe.right_;
	rightFudge_ = copyMe.rightFudge_;
	bottomFudge_ = copyMe.bottomFudge_;
}


PlotDimensions::PlotDimensions(long top, long left, long width, long height, long rightFudge, long bottomFudge, double xScale, double yScale)
: Scalable(xScale, yScale)
{
	this->top_ = top;
	this->left_ = left;
	this->width_ = width;
	this->height_ = height;
	this->right_ = left + width + rightFudge;
	this->bottom_ = top + height + bottomFudge;

	this->rightFudge_ = rightFudge;
	this->bottomFudge_ = bottomFudge;
}


void PlotDimensions::scale_(double x, double y)
{
	top_ = y * top_ + 0.5;
	left_ = x * left_ + 0.5;
	width_ = x * width_ + 0.5;
	height_ = y * height_ + 0.5;
	rightFudge_ = x * rightFudge_ + 0.5;
	bottomFudge_ = y * bottomFudge_ + 0.5;

	right_ = left_ + width_ + rightFudge_;
	bottom_ = top_ + height_ + bottomFudge_;
}

void PlotDimensions::unscale_()
{
	top_ = y_inverse_scaling_factor() * top_ + 0.5;
	left_ = x_inverse_scaling_factor() * left_ + 0.5;
	width_ = x_inverse_scaling_factor() * width_ + 0.5;
	height_ = y_inverse_scaling_factor() * height_ + 0.5;
	rightFudge_ = x_inverse_scaling_factor() * rightFudge_ + 0.5;
	bottomFudge_ = y_inverse_scaling_factor() * bottomFudge_ + 0.5;
	
	right_ = left_ + width_ + rightFudge_;
	bottom_ = top_ + height_ + bottomFudge_;
}