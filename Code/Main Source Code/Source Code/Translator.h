#ifndef ATRANSLATOR_H
#define ATRANSLATOR_H

#include "Observer.h"
#include "PlotDimensions.h"

class Axis;
class Plot;
class TracePar;

/**********************************************************************************
Translators are responsible for translating back and forth between data points and 
screen coordinates. A (one-dimensional) Axis is configured with a (one-dimensional) 
translator, and consults it to figure out where to draw a data point on its axis, 
or which data point corresponds to a given point on its axis on the screen. For 
example, changing a 1D linear plot to a 1D log plot means changing the affected 
Plot's Axes' Translators accordingly.

Translators come in vertical, horizontal, log, linear, 1D, or 2D flavours.
**********************************************************************************/

class Translator
{
public:
	static Translator* makeTranslator(int dimensions, short mapping);
	Translator(const Translator& copyMe);
	virtual ~Translator();
	virtual Translator* clone() const = 0;

	void setAxis(Axis* axis); 

	// Translating from a mouse position.
	virtual float scrnToData(long) = 0;
//	virtual float scrnToData(long,long) = 0;
	virtual float scrnToUser(long) = 0;
	virtual float scrnToFraction(long) = 0;
	virtual long scrnToIndex(long xs);
	// Translating from data.
	virtual float dataToUser(long) = 0;
	virtual long dataToScrn(float);
	virtual long dataToScrn(float,long);
	virtual float dataToScrnF(float,long);
	virtual long userToScrn(float);
	virtual long dataToScrnReverse(float);

	// Translating from ? to data.
	virtual long userToData(float);

protected:
	Translator();

	Axis* axis_;
	PlotDimensions* dim_;
};


class TranslatorVerticalLinear1D : public Translator
{
public:
	TranslatorVerticalLinear1D* clone() const;
	// virtual inheritance
	float scrnToData(long);
	float dataToUser(long);
	float userToData(long);
	float scrnToUser(long);
	float scrnToFraction(long);
	long dataToScrn(float);
	float dataToScrnF(float);
	long dataToScrn(float,long);
	float dataToScrnF(float,long);
	TranslatorVerticalLinear1D();
protected:

};

//////////////////////////////////////////////////////////////////////

class TranslatorHorizontalLinear1D : public Translator
{
public:
	TranslatorHorizontalLinear1D* clone() const;
	float scrnToData(long xs);
	float dataToUser(long xs);
	long userToData(float xu);
	float scrnToUser(long xs);
	float scrnToFraction(long xs);
	long scrnToIndex(long xs);
	long dataToScrn(float xd);
	float dataToScrnF(float xd);
	long dataToScrn(float xd, long xoff);
	float dataToScrnF(float xd, long xoff);
	long dataToScrnReverse(float xd);
	float dataToScrnReverseF(float xd);
	TranslatorHorizontalLinear1D();
protected:
};

//////////////////////////////////////

class TranslatorVerticalLog1D : public Translator
{
public:
	TranslatorVerticalLog1D* clone() const;
	float scrnToData(long ys);
	float dataToUser(long ys);
	long userToData(float yu);
	float scrnToUser(long ys);
	float scrnToFraction(long ys);
	long dataToScrn(float yd);
	float dataToScrnF(float yd);
	long dataToScrn(float yd, long yoff);
	float dataToScrnF(float yd, long yoff);
	TranslatorVerticalLog1D();
protected:
};

//////////////////////////////////////
class TranslatorHorizontalLog1D : public Translator
{
public:
	TranslatorHorizontalLog1D* clone() const;
	float scrnToData(long xs);
	float dataToUser(long xs);
	long userToData(float xu);
	float scrnToUser(long xs);
	float scrnToFraction(long xs);
	long scrnToIndex(long xs);
	long dataToScrn(float xd);
	float dataToScrnF(float xd);
	long dataToScrn(float xd, long xoff);
	float dataToScrnF(float xd, long xoff);
	long dataToScrnReverse(float xd);
	TranslatorHorizontalLog1D();
protected:
};


//////////////////////////////////////
class TranslatorVertical2D : public Translator
{
public:
	TranslatorVertical2D* clone() const;
	float scrnToData(long ys);
	float dataToUser(long ys);
	long userToData(float yu);
	long userToScrn(float yu);
	float scrnToUser(long ys);
	float scrnToFraction(long ys);
	long dataToScrn(float yd);
	TranslatorVertical2D();
protected:
};

//////////////////////////////////////
class TranslatorHorizontal2D : public Translator
{
public:
	TranslatorHorizontal2D* clone() const;
	float scrnToData(long xs);
	float dataToUser(long xs);
	long userToData(float xu);
	long userToScrn(float xu);
	float scrnToUser(long xs);
	float scrnToFraction(long xs);
	long dataToScrn(float xd);
	long dataToScrnReverse(float xd);
	TranslatorHorizontal2D();
protected:
};


//////////////////////////////////////
class TranslatorVerticalLog2D : public Translator
{
public:
	TranslatorVerticalLog2D* clone() const;
	float scrnToData(long ys);
	float dataToUser(long ys);
	long userToData(float yu);
   long userToScrn(float xu);
	float scrnToUser(long ys);
	float scrnToFraction(long ys);
	long dataToScrn(float yd);
	TranslatorVerticalLog2D();
protected:
};

//////////////////////////////////////
class TranslatorHorizontalLog2D : public Translator
{
public:
	TranslatorHorizontalLog2D* clone() const;
	float scrnToData(long xs);
	float dataToUser(long xs);
	long userToData(float xu);
   long userToScrn(float xu);
	float scrnToUser(long xs);
	float scrnToFraction(long xs);
	long dataToScrn(float xd);
	long dataToScrnReverse(float xd);
	TranslatorHorizontalLog2D();
protected:
};


#endif // ifndef ATRANSLATOR_H