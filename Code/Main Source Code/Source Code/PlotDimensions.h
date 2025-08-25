#ifndef PLOTDIMENSIONS_H
#define PLOTDIMENSIONS_H

#include "Scalable.h"

class Plot;

class PlotDimensions : public Scalable
{
	public:
		PlotDimensions(void);
		PlotDimensions(long top, long left, long width, long height, long rightFudge = 0, long bottomFudge = 0, double xScale = 1.0, double yScale = 1.0);
		PlotDimensions(const PlotDimensions& copyMe);

		long width(void) {return width_;}
		long height(void) {return height_;}
		long left(void) {return left_;}
		long top(void) {return top_;}
		long right(void) {return right_;}
		long bottom(void) {return bottom_;}

      void SetLeft(int left) {left_ = left;}
      void SetTop(int top) {top_ = top;}
      void SetWidth(int width) {width_ = width;}
      void SetHeight(int height) {height_ = height;}
				
	private:

		void scale_(double x, double y);
		void unscale_();

		long width_;
		long height_;
		long top_;
		long bottom_;
		long left_;
		long right_;

		long rightFudge_;
		long bottomFudge_;
};

#endif // #define PLOTDIMENSIONS_H