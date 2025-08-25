#include "stdafx.h"
#include "Translator.h"
#include "Axis.h"
#include <math.h>
#include "mymath.h"
#include "Plot.h"
#include "memoryLeak.h"

#define MAX_SCREEN_COORDINATE 32767
#define MIN_SCREEN_COORDINATE -32767

Translator::Translator()
{
	axis_ = 0;
	dim_ = 0;
}

Translator::Translator(const Translator& copyMe)
{
	axis_ = 0;
	dim_ = 0;
}

Translator* Translator::makeTranslator(int dimensions, short mapping)
{
	if (1 == dimensions)
	{
		if (mapping == PLOT_LINEAR_X)
		{
			return new TranslatorHorizontalLinear1D();
		}
		else if (mapping == PLOT_LINEAR_Y)
		{
			return new TranslatorVerticalLinear1D();
		}
		else if (mapping == PLOT_LOG_X)
		{
			return new TranslatorHorizontalLog1D();
		}
		else // (mapping == PLOT_LOG_Y)
		{
			return new TranslatorVerticalLog1D();
		}
	}
	else if (2 == dimensions)
	{
		if (mapping == PLOT_LINEAR_X)
		{
			return new TranslatorHorizontal2D();
		}
      else if(mapping == PLOT_LOG_X)
		{
			return new TranslatorHorizontalLog2D();
		}
		else if (mapping == PLOT_LINEAR_Y)
		{
			return new TranslatorVertical2D();
		}
      else if(mapping == PLOT_LOG_Y)
		{
			return new TranslatorVerticalLog2D();
		}
	}	
	return 0;
}

Translator::~Translator()
{
}

void Translator::setAxis(Axis* axis)
{
	axis_ = axis; 
	dim_ = axis->plotDimensions();
}

long Translator::scrnToIndex(long xs)
{
	// Should never be called.
	return 0;
}

long Translator::dataToScrn(float val)
{
	// Should never be called.
	return 0;
}

long Translator::dataToScrn(float val, long off)
{
	// Should never be called.
	return 0;
}

float Translator::dataToScrnF(float val, long off)
{
	// Should never be called.
	return 0;
}

long Translator::userToScrn(float val)
{
	// Should never be called.
	return 0;
}


long Translator::dataToScrnReverse(float val)
{
	// Should never be called.
	return 0;
}

long Translator::userToData(float val)
{
	// Should never be called.
	return 0;
}


////////////////////////////////////////////////////////////////////


TranslatorVerticalLinear1D::TranslatorVerticalLinear1D()
: Translator()
{
}

TranslatorVerticalLinear1D* TranslatorVerticalLinear1D::clone() const
{
	return new TranslatorVerticalLinear1D(*this);
}

float TranslatorVerticalLinear1D::scrnToData(long ys)
{
	float result;
	if (axis_->plotDirection() == PLT_FORWARD)
	{
	   result = (float)(dim_->bottom()-ys) * (axis_->Max() - axis_->Min()) / (float)dim_->height() + axis_->Min();
	}
	else
	{
	   result = (float)(ys-dim_->top()) * (axis_->Max() - axis_->Min()) / (float)dim_->height() + axis_->Min();
	}

	return result;
}


float TranslatorVerticalLinear1D::dataToUser(long ys)
{
	return ys;
}

float TranslatorVerticalLinear1D::userToData(long yu)
{
	return yu;
}

float TranslatorVerticalLinear1D::scrnToUser(long ys)
{
   return ys;
}

float TranslatorVerticalLinear1D::scrnToFraction(long ys)
{
   float result = (float)(ys - dim_->top() - dim_->height())/(float)dim_->height();
   return(result);
}

long TranslatorVerticalLinear1D::dataToScrn(float yd)
{
   extern float Inf;
	long result;
   if(yd == Inf)
      yd = axis_->Max();
   else if(yd == -Inf)
      yd = axis_->Min();


	if(axis_->plotDirection() == PLT_FORWARD)
   {
	   result = (long)(dim_->bottom() - (yd-axis_->Min())*(float)dim_->height() / (float)(axis_->Max()-axis_->Min())+0.5);
   }
   else
   {
	   result = (long)(dim_->top() + (yd-axis_->Min())*(float)dim_->height() / (float)(axis_->Max()-axis_->Min())+0.5);
   }
	if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}


float TranslatorVerticalLinear1D::dataToScrnF(float yd)
{
   extern float Inf;
	float result;
   if(yd == Inf)
      yd = axis_->Max();
   else if(yd == -Inf)
      yd = axis_->Min();

	if(axis_->plotDirection() == PLT_FORWARD)
   {
	   result = dim_->bottom() - (yd-axis_->Min())*(float)dim_->height() / (float)(axis_->Max()-axis_->Min());
   }
   else
   {
	   result = dim_->top() + (yd-axis_->Min())*(float)dim_->height() / (float)(axis_->Max()-axis_->Min());
   }
	if (result > 0)	
		return min(result,(float)MAX_SCREEN_COORDINATE);
	return max(result, (float)MIN_SCREEN_COORDINATE);
}


long TranslatorVerticalLinear1D::dataToScrn(float yd, long yoff)
{
	long result;
   float fyoff;

   if(yoff == 0)
   {
	   if(axis_->plotDirection() == PLT_FORWARD)
      {
	      result = (long)(dim_->bottom() - (yd-axis_->Min())*(float)dim_->height() / (float)(axis_->Max()-axis_->Min())  + 0.5);
      }
      else
      {
	      result = (long)(dim_->top() + (yd-axis_->Min())*(float)dim_->height() / (float)(axis_->Max()-axis_->Min())  +0.5);
      }
   }
   else
   {
      if(axis_->origMapping() == PLOT_LOG_Y)
         fyoff = yoff - (log10(axis_->MinIndep())-log10(axis_->MinIndepOrig()))*0.9*(float)dim_->height() / ((float)log10(axis_->MaxIndep())-log10(axis_->MinIndep())) ;
      else
          fyoff = yoff - (axis_->MinIndep()-axis_->MinIndepOrig())*0.9*(float)dim_->height() / (float)(axis_->MaxIndep()-axis_->MinIndep());

	   if(axis_->plotDirection() == PLT_FORWARD)
      {
	      result = (long)(dim_->bottom() - (yd)*(float)dim_->height() / (float)(axis_->Max()-axis_->Min()) - fyoff + 0.5);
      }
      else
      {
	      result = (long)(dim_->top() + (yd)*(float)dim_->height() / (float)(axis_->Max()-axis_->Min()) + fyoff +0.5);
      }
   }

	if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}


float TranslatorVerticalLinear1D::dataToScrnF(float yd, long yoff)
{
	float result;
   float fyoff;

   if(yoff == 0)
   {
	   if(axis_->plotDirection() == PLT_FORWARD)
      {
	      result = dim_->bottom() - (yd-axis_->Min())*(float)dim_->height() / (float)(axis_->Max()-axis_->Min());
      }
      else
      {
	      result = dim_->top() + (yd-axis_->Min())*(float)dim_->height() / (float)(axis_->Max()-axis_->Min());
      }
   }
   else
   {
      if(axis_->origMapping() == PLOT_LOG_Y)
         fyoff = yoff - (log10(axis_->MinIndep())-log10(axis_->MinIndepOrig()))*0.9*(float)dim_->height() / ((float)log10(axis_->MaxIndep())-log10(axis_->MinIndep())) ;
      else
          fyoff = yoff - (axis_->MinIndep()-axis_->MinIndepOrig())*0.9*(float)dim_->height() / (float)(axis_->MaxIndep()-axis_->MinIndep());

	   if(axis_->plotDirection() == PLT_FORWARD)
      {
	      result = dim_->bottom() - (yd)*(float)dim_->height() / (float)(axis_->Max()-axis_->Min()) - fyoff;
      }
      else
      {
	      result = dim_->top() + (yd)*(float)dim_->height() / (float)(axis_->Max()-axis_->Min()) + fyoff;
      }
   }

	if (result > 0)	
		return min(result, (float)MAX_SCREEN_COORDINATE);
	return max(result, (float)MIN_SCREEN_COORDINATE);
}

////////////////////////////////////////////////////////////////////


TranslatorHorizontalLinear1D::TranslatorHorizontalLinear1D()
: Translator()
{
}

TranslatorHorizontalLinear1D* TranslatorHorizontalLinear1D::clone() const
{
	return new TranslatorHorizontalLinear1D(*this);
}

float TranslatorHorizontalLinear1D::scrnToData(long xs)
{
	float result;
	if (axis_->plotDirection() == PLT_FORWARD)
	{
		result = (float)(xs - dim_->left())*(axis_->Max()-axis_->Min())/(float)dim_->width() + axis_->Min();
	}
	else
	{
		result = (float)(dim_->right() - xs)*(axis_->Max()-axis_->Min())/(float)dim_->width() + axis_->Min();
	}
   return(result);
}

float TranslatorHorizontalLinear1D::dataToUser(long xs)
{
	return xs;
}

long TranslatorHorizontalLinear1D::userToData(float xu)
{
	return xu;
}

float TranslatorHorizontalLinear1D::scrnToUser(long xs)
{
	return xs;
}

float TranslatorHorizontalLinear1D::scrnToFraction(long xs)
{
   float result = (float)(xs - dim_->left())/(float)dim_->width();
   return result;
}

long TranslatorHorizontalLinear1D::scrnToIndex(long xs)
{
   long result = 0;

	if(!(dynamic_cast<Plot1D*>(axis_->plot())->hasNoCurTrace()))
   {
		Trace* t = dynamic_cast<Plot1D*>(axis_->plot())->curTrace();
      float size = t->getSize();
      float minData  = t->getMinX();
      float maxData  = t->getMaxX();
      float offset;

      if(axis_->plotDirection() == PLT_FORWARD)
         offset = (float)(xs - dim_->left())*(axis_->Max()-axis_->Min())/(float)dim_->width() + axis_->Min();
      else
         offset = (float)(dim_->right() - xs)*(axis_->Max()-axis_->Min())/(float)dim_->width() + axis_->Min();

      result = nint(size*(offset-minData)/(maxData-minData));
   }
   
	return(result);
}


long TranslatorHorizontalLinear1D::dataToScrn(float xd)
{
   extern float Inf;
   if(xd == Inf)
      xd = axis_->Max();
   else if(xd == -Inf)
      xd = axis_->Min();
   long result;

	if(axis_->plotDirection() == PLT_FORWARD)
   {
      result = (long)(dim_->left() + (xd-axis_->Min())*(float)dim_->width()/(float)(axis_->Max()-axis_->Min())+0.5);
   }
   else
   {
      result = (long)(dim_->right() - (xd-axis_->Min())*(float)dim_->width()/(float)(axis_->Max()-axis_->Min())+0.5);
   }
   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
   return(result);
}

long TranslatorHorizontalLinear1D::dataToScrn(float xd, long xoffset)
{
   extern float Inf;
   if(xd == Inf)
      xd = axis_->Max();
   else if(xd == -Inf)
      xd = axis_->Min();
   long result;

	if(axis_->plotDirection() == PLT_FORWARD)
   {
      result = (long)(dim_->left() + (xd-axis_->Min())*(float)dim_->width()/(float)(axis_->Max()-axis_->Min())+0.5)+xoffset;
   }
   else
   {
      result = (long)(dim_->right() - (xd-axis_->Min())*(float)dim_->width()/(float)(axis_->Max()-axis_->Min())+0.5)+xoffset;
   }
   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
   return(result);
}

float TranslatorHorizontalLinear1D::dataToScrnF(float xd, long xoffset)
{
   extern float Inf;
   if(xd == Inf)
      xd = axis_->Max();
   else if(xd == -Inf)
      xd = axis_->Min();
   float result;

	if(axis_->plotDirection() == PLT_FORWARD)
   {
      result = (dim_->left() + (xd-axis_->Min())*(float)dim_->width()/(float)(axis_->Max()-axis_->Min()))+xoffset;
   }
   else
   {
      result = (dim_->right() - (xd-axis_->Min())*(float)dim_->width()/(float)(axis_->Max()-axis_->Min()))+xoffset;
   }
   if (result > 0)	
		return min(result,(float)MAX_SCREEN_COORDINATE);
	return max(result, (float)MIN_SCREEN_COORDINATE);
   return(result);
}


long TranslatorHorizontalLinear1D::dataToScrnReverse(float xd)
{
   long result;
	result = (long)(dim_->right() - (xd-axis_->Min())*(float)dim_->width()/(float)(axis_->Max()-axis_->Min())+0.5);
   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}

float TranslatorHorizontalLinear1D::dataToScrnReverseF(float xd)
{
   float result;
	result = dim_->right() - (xd-axis_->Min())*(float)dim_->width()/(float)(axis_->Max()-axis_->Min());
   if (result > 0)	
		return min(result,(float)MAX_SCREEN_COORDINATE);
	return max(result, (float)MIN_SCREEN_COORDINATE);
}


////////////////////////////////////////////////////////////////////

TranslatorVerticalLog1D::TranslatorVerticalLog1D()
: Translator()
{
}

TranslatorVerticalLog1D* TranslatorVerticalLog1D::clone() const
{
	return new TranslatorVerticalLog1D(*this);
}

float TranslatorVerticalLog1D::scrnToData(long ys)
{
   float min_y = log10(axis_->Min());
   float max_y = log10(axis_->Max());
   float result = (float)(ys - dim_->top() - dim_->height())*(min_y-max_y)/(float)dim_->height() + min_y;
   return((float)pow((float)10.0,(float)result));
}

float TranslatorVerticalLog1D::dataToUser(long ys)
{
   return(ys);
}

long TranslatorVerticalLog1D::userToData(float yu)
{
   return(yu);
}

float TranslatorVerticalLog1D::scrnToUser(long ys)
{
   return(ys);
}

float TranslatorVerticalLog1D::scrnToFraction(long ys)
{
   float result = (float)(ys - dim_->top() - dim_->height())/(float)dim_->height();
   return(result);
}

long TranslatorVerticalLog1D::dataToScrn(float yd)
{
   long result;
   float lyd;
   float min_y = log10(axis_->Min());
   float max_y = log10(axis_->Max());
   extern float Inf;
   if(yd == Inf)
      yd = max_y;
   else if(yd == -Inf)
      yd = min_y;
   else if(yd < 0)
      lyd = -50;
   else
      lyd = log10(yd);

   result = (long)(dim_->top() + dim_->height() - (lyd-min_y)*(float)dim_->height()/(float)(max_y-min_y)+0.5);
   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}

float TranslatorVerticalLog1D::dataToScrnF(float yd)
{
   float result;
   float lyd;
   float min_y = log10(axis_->Min());
   float max_y = log10(axis_->Max());
   extern float Inf;
   if(yd == Inf)
      yd = max_y;
   else if(yd == -Inf)
      yd = min_y;
   else if(yd < 0)
      lyd = -50;
   else
      lyd = log10(yd);

   result = (dim_->top() + dim_->height() - (lyd-min_y)*(float)dim_->height()/(float)(max_y-min_y));
   if (result > 0)	
		return (min(result,(float)MAX_SCREEN_COORDINATE));
	return (max(result, (float)MIN_SCREEN_COORDINATE));
}



long TranslatorVerticalLog1D::dataToScrn(float yd, long yoff)
{
   long result;
   float lyd;
   float fyoff;
   float min_y = log10(axis_->Min());
   float max_y = log10(axis_->Max());
   extern float Inf;
   if(yd == Inf)
      yd = max_y;
   else if(yd == -Inf)
      yd = min_y;
   else if(yd < 0)
      lyd = -50;
   else
      lyd = log10(yd);


   if(yoff == 0)
   {
      result = (long)(dim_->top() + dim_->height() - (lyd-min_y)*(float)dim_->height()/(float)(max_y-min_y)+0.5) ;
   }
   else
   {
      fyoff = yoff - (axis_->MinIndep()-axis_->MinIndepOrig())*0.9*(float)dim_->height() / (float)(axis_->MaxIndep()-axis_->MinIndep());
      result = (long)(dim_->top() + dim_->height() - (lyd-min_y)*(float)dim_->height()/(float)(max_y-min_y)+0.5+fyoff) ;
   }

   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}


float TranslatorVerticalLog1D::dataToScrnF(float yd, long yoff)
{
   float result;
   float lyd;
   float fyoff;
   float min_y = log10(axis_->Min());
   float max_y = log10(axis_->Max());
   extern float Inf;
   if(yd == Inf)
      yd = max_y;
   else if(yd == -Inf)
      yd = min_y;
   else if(yd < 0)
      lyd = -50;
   else
      lyd = log10(yd);


   if(yoff == 0)
   {
      result = (dim_->top() + dim_->height() - (lyd-min_y)*(float)dim_->height()/(float)(max_y-min_y)) ;
   }
   else
   {
      fyoff = yoff - (axis_->MinIndep()-axis_->MinIndepOrig())*0.9*(float)dim_->height() / (float)(axis_->MaxIndep()-axis_->MinIndep());
      result = (dim_->top() + dim_->height() - (lyd-min_y)*(float)dim_->height()/(float)(max_y-min_y)+fyoff) ;
   }

   if (result > 0)	
		return min(result,(float)MAX_SCREEN_COORDINATE);
	return max(result, (float)MIN_SCREEN_COORDINATE);
}


////////////////////////////////////////////////////////////////////


TranslatorHorizontalLog1D::TranslatorHorizontalLog1D()
: Translator()
{
}

TranslatorHorizontalLog1D* TranslatorHorizontalLog1D::clone() const
{
	return new TranslatorHorizontalLog1D(*this);
}

float TranslatorHorizontalLog1D::scrnToData(long xs)
{
   if(axis_->plotDirection() == PLT_FORWARD)
   {
      float min_x = log10(axis_->Min());
      float max_x = log10(axis_->Max());
      float result = (float)(xs - dim_->left())*(max_x-min_x)/(float)dim_->width() + min_x;
      return((float)pow((float)10.0,(float)result));
   }
   else
   {
	   float result = xs - dim_->left();
		return(result);
	}
}

float TranslatorHorizontalLog1D::dataToUser(long xs)
{
   //float right = log10(axis_->length() + axis_->base());
   //float left = log10(axis_->base());
   //float result = xs*(right-left)/(float)dynamic_cast<Plot2D*>(axis_->plot())->matWidth()+left;
   //result = pow((float)10.0,(float)result);
   return(xs);
}

long TranslatorHorizontalLog1D::userToData(float xu)
{
	return(xu);
}

float TranslatorHorizontalLog1D::scrnToUser(long xs)
{
   return(xs);
}

float TranslatorHorizontalLog1D::scrnToFraction(long xs)
{ 
   float result = (float)(xs - dim_->left())/(float)dim_->width();
   return(result);
}

long TranslatorHorizontalLog1D::scrnToIndex(long xs)
{
   long result = 0;

	if(!(dynamic_cast<Plot1D*>(axis_->plot())->hasNoCurTrace()))
   {
		Trace* currentTrace = dynamic_cast<Plot1D*>(axis_->plot())->curTrace();
      float size = currentTrace->getSize();
      float minData  = currentTrace->getMinX();
      float maxData  = currentTrace->getMaxX();
      float offset = (axis_->Max()-axis_->Min())*(float)(xs - dim_->left())/(float)dim_->width() + axis_->Min();
      result = nint(size*(offset-minData)/(maxData-minData));
   }
	return(result);
}


long TranslatorHorizontalLog1D::dataToScrn(float xd)
{
   long result = 0;

   if(axis_->plotDirection() == PLT_FORWARD)
   {
		float min_x = log10(axis_->Min());
      float max_x = log10(axis_->Max());
      result = (long)((log10(xd)-min_x)*(float)dim_->width()/(float)(max_x-min_x)+dim_->left()+0.5);
   }
   else
   {
		// TODO: throw an exception? We are returning junk for non-linear-reversed.
		result = 0;
   }
   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}

float TranslatorHorizontalLog1D::dataToScrnF(float xd)
{
   long result = 0;

   if(axis_->plotDirection() == PLT_FORWARD)
   {
		float min_x = log10(axis_->Min());
      float max_x = log10(axis_->Max());
      result = ((log10(xd)-min_x)*(float)dim_->width()/(float)(max_x-min_x)+dim_->left());
   }
   else
   {
		// TODO: throw an exception? We are returning junk for non-linear-reversed.
		result = 0;
   }
   if (result > 0)	
		return min(result,(float)MAX_SCREEN_COORDINATE);
	return max(result, (float)MIN_SCREEN_COORDINATE);
}

long TranslatorHorizontalLog1D::dataToScrn(float xd, long xoffset)
{
   long result = 0;

   if(axis_->plotDirection() == PLT_FORWARD)
   {
		float min_x = log10(axis_->Min());
      float max_x = log10(axis_->Max());
      result = (long)((log10(xd)-min_x)*(float)dim_->width()/(float)(max_x-min_x)+dim_->left()+0.5)+xoffset;
   }
   else
   {
		// TODO: throw an exception? We are returning junk for non-linear-reversed.
		result = 0;
   }
   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}


float TranslatorHorizontalLog1D::dataToScrnF(float xd, long xoffset)
{
   long result = 0;

   if(axis_->plotDirection() == PLT_FORWARD)
   {
		float min_x = log10(axis_->Min());
      float max_x = log10(axis_->Max());
      result = ((log10(xd)-min_x)*(float)dim_->width()/(float)(max_x-min_x)+dim_->left())+xoffset;
   }
   else
   {
		// TODO: throw an exception? We are returning junk for non-linear-reversed.
		result = 0;
   }
   if (result > 0)	
		return min(result,(float)MAX_SCREEN_COORDINATE);
	return max(result, (float)MIN_SCREEN_COORDINATE);
}

long TranslatorHorizontalLog1D::dataToScrnReverse(float xd)
{
   long result = 0;
	// TODO: throw an exception? We are returning junk for non-linear-reversed.
   return(result);
}


////////////////////////////////////////////////////////////////////


TranslatorVertical2D::TranslatorVertical2D()
: Translator()
{
}

TranslatorVertical2D* TranslatorVertical2D::clone() const
{
	return new TranslatorVertical2D(*this);
}

float TranslatorVertical2D::scrnToData(long ys)
{
   float result;
   long visTop = dynamic_cast<Plot2D*>(axis_->plot())->visibleTop();
   long visHeight = dynamic_cast<Plot2D*>(axis_->plot())->visibleHeight();

	if(axis_->plotDirection() == PLT_FORWARD)
   {
	    result = visTop - (ys - dim_->top() - dim_->height() + 1)*visHeight/dim_->height();
   }
   else
   {
	    result = visTop + (ys - dim_->top())*visHeight/dim_->height();
   }

// Check for limits
   if(result >= visHeight+visTop)
      result = visHeight+visTop-1;

   if(result < visTop)
      result = visTop;

	return(result);      
}


float TranslatorVertical2D::dataToUser(long ys)
{
 	float result = ys*axis_->length()/(float)dynamic_cast<Plot2D*>(axis_->plot())->matHeight() + axis_->base();
   return(result);
}

long TranslatorVertical2D::userToData(float yu)
{
   float result = (long)((yu-axis_->base())*dynamic_cast<Plot2D*>(axis_->plot())->matHeight()/axis_->length());
   return(result);
}

long TranslatorVertical2D::userToScrn(float yu)
{
   float result = (yu-axis_->base())*dynamic_cast<Plot2D*>(axis_->plot())->matHeight()/axis_->length();
   return(dataToScrn(result));
}

float TranslatorVertical2D::scrnToUser(long ys)
{
	if(axis_->plotDirection() == PLT_FORWARD)
   {
      float index = dynamic_cast<Plot2D*>(axis_->plot())->visibleTop() - (ys - dim_->top() - dim_->height() + 1)*dynamic_cast<Plot2D*>(axis_->plot())->visibleHeight()/(float)dim_->height();
      float result = index*axis_->length()/(float)dynamic_cast<Plot2D*>(axis_->plot())->matHeight()+axis_->base();
      return(result);
   }
   else
   {
      float index = dynamic_cast<Plot2D*>(axis_->plot())->visibleTop() + (ys - dim_->top())*dynamic_cast<Plot2D*>(axis_->plot())->visibleHeight()/(float)dim_->height();
      float result = index*axis_->length()/(float)dynamic_cast<Plot2D*>(axis_->plot())->matHeight()+axis_->base();
      return(result);
   }
}

float TranslatorVertical2D::scrnToFraction(long ys)
{
   float result = (float)(ys - dim_->top() - dim_->height() + 1)/(float)dim_->height();
   return(result);      
}

long TranslatorVertical2D::dataToScrn(float yd)
{
   long result = nint(dim_->height() + dim_->top() - 1 - (yd-dynamic_cast<Plot2D*>(axis_->plot())->visibleTop())*dim_->height()/(float)dynamic_cast<Plot2D*>(axis_->plot())->visibleHeight());
	if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}



////////////////////////////////////////////////////////////////////


TranslatorHorizontal2D::TranslatorHorizontal2D()
: Translator()
{
}

TranslatorHorizontal2D* TranslatorHorizontal2D::clone() const
{
	return new TranslatorHorizontal2D(*this);
}

float TranslatorHorizontal2D::scrnToData(long xs)
{
   float result;
   long visLeft = dynamic_cast<Plot2D*>(axis_->plot())->visibleLeft();
   long visWidth = dynamic_cast<Plot2D*>(axis_->plot())->visibleWidth();

	if(axis_->plotDirection() == PLT_FORWARD)
   {
	   result = (xs - dim_->left())*visWidth/dim_->width() + visLeft;
   }
   else
   {
   	result = (dim_->right() - xs)*visWidth/dim_->width() + visLeft;
   }

// Check for limits
   if(result >= (visWidth+visLeft))
      result = visWidth+visLeft-1;

   if(result < visLeft)
      result = visLeft;

   return(result);
}

float TranslatorHorizontal2D::dataToUser(long xs)
{
   float result = xs*axis_->length()/(float)dynamic_cast<Plot2D*>(axis_->plot())->matWidth()+axis_->base();
   return(result);
}

long TranslatorHorizontal2D::userToData(float xu)
{
   float result = (long)((xu-axis_->base())*dynamic_cast<Plot2D*>(axis_->plot())->matWidth()/axis_->length());
   return(result);
}

long TranslatorHorizontal2D::userToScrn(float xu)
{
   float result = (xu-axis_->base())*dynamic_cast<Plot2D*>(axis_->plot())->matWidth()/axis_->length();
   return(dataToScrn(result));
}

float TranslatorHorizontal2D::scrnToUser(long xs)
{
	if(axis_->plotDirection() == PLT_FORWARD)
   {
      float index = (xs - dim_->left())*dynamic_cast<Plot2D*>(axis_->plot())->visibleWidth()/(float)dim_->width() + dynamic_cast<Plot2D*>(axis_->plot())->visibleLeft();
      float result = index*axis_->length()/(float)dynamic_cast<Plot2D*>(axis_->plot())->matWidth()+axis_->base();
      return(result);
   }
   else
   {
      float index = (dim_->right() - xs)*dynamic_cast<Plot2D*>(axis_->plot())->visibleWidth()/(float)dim_->width() + dynamic_cast<Plot2D*>(axis_->plot())->visibleLeft();
      float result = index*axis_->length()/(float)dynamic_cast<Plot2D*>(axis_->plot())->matWidth()+axis_->base();
      return(result);
   }
}

float TranslatorHorizontal2D::scrnToFraction(long xs)
{
   float result = (float)(xs - dim_->left())/(float)dim_->width();
   return(result);
}

long TranslatorHorizontal2D::dataToScrn(float xd)
{
   long result;
   long visLeft = dynamic_cast<Plot2D*>(axis_->plot())->visibleLeft();
   long visWidth = dynamic_cast<Plot2D*>(axis_->plot())->visibleWidth();

	if(axis_->plotDirection() == PLT_FORWARD)
	   result = nint(dim_->left() + (xd-visLeft)*dim_->width()/(float)visWidth);
   else
	   result = nint(dim_->right() - (xd-visLeft)*dim_->width()/(float)visWidth);

   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}



long TranslatorHorizontal2D::dataToScrnReverse(float xd)
{
   long result = 0;
	// TODO: Throw exception if called for 2D?
   return(result);
}



// TranslatorHorizontalLog2D ////////////////////////


TranslatorHorizontalLog2D::TranslatorHorizontalLog2D()
: Translator()
{
}

TranslatorHorizontalLog2D* TranslatorHorizontalLog2D::clone() const
{
	return new TranslatorHorizontalLog2D(*this);
}

float TranslatorHorizontalLog2D::scrnToData(long xs)
{
   float result;
   long visLeft = dynamic_cast<Plot2D*>(axis_->plot())->visibleLeft();
   long visWidth = dynamic_cast<Plot2D*>(axis_->plot())->visibleWidth();

	if(axis_->plotDirection() == PLT_FORWARD)
   {
	   result = (xs - dim_->left())*visWidth/dim_->width() + visLeft;
   }
   else
   {
   	result = (dim_->right() - xs)*visWidth/dim_->width() + visLeft;
   }
   return(result);
}

float TranslatorHorizontalLog2D::dataToUser(long xs)
{
   float right = log10(axis_->length() + axis_->base());
   float left = log10(axis_->base());
   float result = xs*(right-left)/(float)dynamic_cast<Plot2D*>(axis_->plot())->matWidth()+left;
   result = pow((float)10.0,(float)result);
   return(result);
}

long TranslatorHorizontalLog2D::userToData(float xu)
{
   float right = log10(axis_->length() + axis_->base());
   float left = log10(axis_->base());
   xu = log10(xu);
   long result = nint((xu-left)*dynamic_cast<Plot2D*>(axis_->plot())->matWidth()/(right-left));
   return(result);
}

float TranslatorHorizontalLog2D::scrnToUser(long xs)
{
   float index = (xs - dim_->left())*dynamic_cast<Plot2D*>(axis_->plot())->visibleWidth()/(float)dim_->width() + dynamic_cast<Plot2D*>(axis_->plot())->visibleLeft();
   float result = index*axis_->length()/(float)dynamic_cast<Plot2D*>(axis_->plot())->matWidth()+axis_->base();
   return(result);
}

float TranslatorHorizontalLog2D::scrnToFraction(long xs)
{
   float result = (float)(xs - dim_->left())/(float)dim_->width();
   return(result);
}

long TranslatorHorizontalLog2D::userToScrn(float xu)
{
   float right = log10(axis_->length() + axis_->base());
   float left = log10(axis_->base());
   xu = log10(xu);
   float result = (xu-left)*dynamic_cast<Plot2D*>(axis_->plot())->matWidth()/(right-left);
   return(dataToScrn(result));
}

long TranslatorHorizontalLog2D::dataToScrn(float xd)
{
   long result;
   long visLeft = dynamic_cast<Plot2D*>(axis_->plot())->visibleLeft();
   long visWidth = dynamic_cast<Plot2D*>(axis_->plot())->visibleWidth();

	if(axis_->plotDirection() == PLT_FORWARD)
	   result = nint(dim_->left() + (xd-visLeft)*dim_->width()/(float)visWidth);
   else
	   result = nint(dim_->right() - (xd-visLeft)*dim_->width()/(float)visWidth);

   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}



long TranslatorHorizontalLog2D::dataToScrnReverse(float xd)
{
   long result = 0;
	// TODO: Throw exception if called for 2D?
   return(result);
}


// TranslatorVerticalLog2D /////////////////////////////////


TranslatorVerticalLog2D::TranslatorVerticalLog2D()
: Translator()
{
}

TranslatorVerticalLog2D* TranslatorVerticalLog2D::clone() const
{
	return new TranslatorVerticalLog2D(*this);
}

float TranslatorVerticalLog2D::scrnToData(long ys)
{
   float result;
   long visTop = dynamic_cast<Plot2D*>(axis_->plot())->visibleTop();
   long visHeight = dynamic_cast<Plot2D*>(axis_->plot())->visibleHeight();

	if(axis_->plotDirection() == PLT_FORWARD)
   {
	    result = visTop - (ys - dim_->top() - dim_->height() + 1)*visHeight/dim_->height();
   }
   else
   {
	    result = visTop + (ys - dim_->top())*visHeight/dim_->height();
   }

	return(result);      
}


float TranslatorVerticalLog2D::dataToUser(long ys)
{
   float top = log10(axis_->length() + axis_->base());
   float base = log10(axis_->base());
   float result = ys*(top-base)/(float)dynamic_cast<Plot2D*>(axis_->plot())->matHeight()+base;
   result = pow((float)10.0,(float)result);
   return(result);
}

long TranslatorVerticalLog2D::userToData(float yu)
{
   float top = log10(axis_->length() + axis_->base());
   float base = log10(axis_->base());
   yu = log10(yu);
   long result = nint((yu-base)*dynamic_cast<Plot2D*>(axis_->plot())->matHeight()/(top-base));

   return(result);
}

long TranslatorVerticalLog2D::userToScrn(float yu)
{
   float top = log10(axis_->length() + axis_->base());
   float base = log10(axis_->base());
   yu = log10(yu);
   float result = (yu-base)*dynamic_cast<Plot2D*>(axis_->plot())->matHeight()/(top-base);
   return(dataToScrn(result));
}


float TranslatorVerticalLog2D::scrnToUser(long ys)
{
   float index = dynamic_cast<Plot2D*>(axis_->plot())->visibleTop() - (ys - dim_->top() - dim_->height() + 1)*dynamic_cast<Plot2D*>(axis_->plot())->visibleHeight()/(float)dim_->height();
   float result = index*axis_->length()/(float)dynamic_cast<Plot2D*>(axis_->plot())->matHeight()+axis_->base();
   return(result);
}

float TranslatorVerticalLog2D::scrnToFraction(long ys)
{
   float result = (float)(ys - dim_->top() - dim_->height() + 1)/(float)dim_->height();
   return(result);      
}

long TranslatorVerticalLog2D::dataToScrn(float yd)
{
   long result;
   long visTop = dynamic_cast<Plot2D*>(axis_->plot())->visibleTop();
   long visHeight = dynamic_cast<Plot2D*>(axis_->plot())->visibleHeight();

    if(axis_->plotDirection() == PLT_FORWARD)
       result = nint(dim_->height() + dim_->top() - 1 - (yd-visTop)*dim_->height()/(float)visHeight);
    else
       result = nint(dim_->top() + (yd-visTop)*dim_->height()/(float)visHeight);

   if (result > 0)	
		return min(result,MAX_SCREEN_COORDINATE);
	return max(result, MIN_SCREEN_COORDINATE);
}
