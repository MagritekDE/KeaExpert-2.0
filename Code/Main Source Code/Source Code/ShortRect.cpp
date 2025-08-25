#include "ShortRect.h"
#include "memoryLeak.h"

ShortRect::ShortRect(void)
: Scalable()
{
	this->reset();
}

ShortRect::ShortRect(const ShortRect& copyMe)
: Scalable(copyMe)
{
	x0 = copyMe.x0;
	x1 = copyMe.x1;
	y0 = copyMe.y0;
	y1 = copyMe.y1;
}

ShortRect::~ShortRect(void)
{
}

void ShortRect::reset(void)
{
	this->set(MAX_RECT_VAL,MAX_RECT_VAL,MIN_RECT_VAL,MIN_RECT_VAL);
}

void ShortRect::zero(void)
{
	this->set(0,0,0,0);
}

// Update a rectangle to take in the maximum region
void ShortRect::SetAxisRect(short x0, short y0, short x1, short y1)
{
	if(x0 < this->x0) this->x0 = x0;
	if(x1 > this->x1) this->x1 = x1;
	if(y0 < this->y0) this->y0 = y0;
	if(y1 > this->y1) this->y1 = y1;
}  

void ShortRect::setx0(short x0)
{
	this->x0 = x0;
}

void ShortRect::sety0(short y0)
{
	this->y0 = y0;
}

void ShortRect::setx1(short x1)
{
	this->x1 = x1;
}

void ShortRect::sety1(short y1)
{
	this->y1 = y1;
}

short ShortRect::getx0(void) const
{
	return this->x0;
}

short ShortRect::getx1(void) const
{
	return this->x1;
}

short ShortRect::gety0(void) const
{
	return this->y0;
}

short ShortRect::gety1(void) const
{
	return this->y1;
}

bool ShortRect::PointInRect(short x, short y) const 
{
	if(x >= this->x0 && x <= this->x1 && y >= this->y0 && y <= this->y1)
		return(true);
	else
		return(false);
}

void ShortRect::set(short x0, short y0, short x1, short y1)
{
	this->x0 = x0;
	this->y0 = y0;
	this->x1 = x1;
	this->y1 = y1;
}


void ShortRect::scale_(double x, double y)
{
	x0 = x * x0 + 0.5;
	y0 = y * y0 + 0.5;
	x1 = x * x1 + 0.5;
	y1 = y * y1 + 0.5;
}

void ShortRect::unscale_()
{
	x0 = x_inverse_scaling_factor() * x0 + 0.5;
	y0 = y_inverse_scaling_factor() * y0 + 0.5;
	x1 = x_inverse_scaling_factor() * x1 + 0.5;
	y1 = y_inverse_scaling_factor() * y1 + 0.5;
}
