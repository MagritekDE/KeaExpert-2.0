#ifndef SCALABLE_H
#define SCALABLE_H
#include <deque>

class Scalable
{
public:
	Scalable() {sfX = sfY = 1;}
	Scalable(double x, double y) {sfX = x; sfY = y;}
	Scalable(const Scalable& copyMe) {sfX = copyMe.sfX; sfY = copyMe.sfY;}
	virtual ~Scalable() {};

	double x_scaling_factor() const {return sfX;}
	double y_scaling_factor() const {return sfY;}
	double x_inverse_scaling_factor() const { 
		return (0 != sfX) ? 1.0/sfX : 1;
	}
	double y_inverse_scaling_factor() const {
		return (0 != sfY) ? 1.0/sfY : 1;
	}

	void scale(double x, double y = 0){
		if (0 == y) 
		{y=x;} 
		sfX *= x; 
		sfY *= y; 
		scale_(x,y);
	}
	virtual void unscale(){
		unscale_();
		sfX = sfY = 1;
	}

private:
	double sfX;
	double sfY;

	virtual void scale_(double x, double y) = 0;
	virtual void unscale_() = 0;
};

typedef std::deque<Scalable*> ScalableList;


#endif// ifndef SCALABLE_H
