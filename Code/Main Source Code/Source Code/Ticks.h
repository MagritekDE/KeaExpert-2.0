#ifndef MYTICKS_H
#define MYTICKS_H

#include "font.h"
#include "Scalable.h"

typedef enum {
	MAJOR,
	MINOR
} TickType;

// Data class for plot ticks.
class Ticks : public Scalable
{
public:
	Ticks(double majorLength, double minorLength, double spacing, double perLabel, ProspaFont* font, COLORREF fontColor);
	Ticks(const Ticks& copyMe);

	~Ticks();

	double majorLength() {return majorTickLength_;}
	double minorLength() {return minorTickLength_;}
	double spacing() {return tickSpacing_;}
	double perLabel() {return ticksPerLabel_;}
	double labelSpacing() {return labelSpacing_;}

	ProspaFont& pfont() {return font_;}
	LOGFONT font() {return font_.font();}

	COLORREF& fontColor() {return font_.color();}
	short fontSize() {return font_.size();}
	short fontStyle() {return font_.style();}
	char* fontName() {return font_.name();}

	void setMajorLength(double len){ majorTickLength_ = len;}
	void setMinorLength(double len){ minorTickLength_ = len;}
	void setSpacing(double spacing){ tickSpacing_ = spacing; calculateLabelSpacing();}
	void setPerLabel(double perLabel){ ticksPerLabel_ = perLabel; calculateLabelSpacing();}

	void setFont(ProspaFont& font) {font_ = font;}
	void setFontColor(COLORREF fontColor) {font_.setColor(fontColor);}
	void setFontSize(short fontSize) {font_.setSize(fontSize);}
	void setFontStyle(short fontStyle) {font_.setStyle(fontStyle);}
	void setFontName(const char* const name) {font_.setName(name);}

	short save(FILE* fp);

private:
	// Label spacing is calculated based on ticks per label and label spacing
	//  whenever either of those values changes.
	void calculateLabelSpacing();

	void scale_(double x, double y);
	void unscale_();

	double majorTickLength_;  // Length of short and long ticks in pixels
	double minorTickLength_;
	double tickSpacing_;    // Spacing in pixels between ticks
	double ticksPerLabel_;  // Spacing in pixels between labels
	double labelSpacing_;

	double scaleFactor_;

	ProspaFont font_;    // Font information for text
};



#endif // ifndef MYTICKS