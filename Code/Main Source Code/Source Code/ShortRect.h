#ifndef SHORTRECT_H
#define SHORTRECT_H

#include "Scalable.h"

#define MAX_RECT_VAL 1e4
#define MIN_RECT_VAL -1e4
#define X0_DEFAULT 1e4
#define Y0_DEFAULT 1e4
#define X1_DEFAULT -1e4
#define Y1_DEFAULT -1e4

class ShortRect : public Scalable
{
public:
	ShortRect(void);
	ShortRect(const ShortRect& copyMe);
	~ShortRect(void);

	// Update a rectangle to take in the maximum region
	void SetAxisRect(short x0, short y0, short x1, short y1);
	bool PointInRect(short,short) const;

	short getx0(void) const;
	short getx1(void) const;
	short gety0(void) const;
	short gety1(void) const;

	void zero(void);
	void reset(void);
	void set(short,short,short,short);
	void setx0(short x0);
	void sety0(short y0);
	void setx1(short x1);
	void sety1(short y1);

private:
	short x0;
	short y0;
	short x1;
	short y1;

	void scale_(double x, double y);
	void unscale_();
};

#endif // define SHORTRECT_H