#ifndef FONT_H
#define FONT_H

#include "Scalable.h"

#define PROSPA_FONT_NAME_LEN 200
#define DEFAULT_FONT_NAME "Times New Roman"

class ProspaFont : public Scalable
{
public:
	ProspaFont(const char* const name, COLORREF color, short size, short style);
	ProspaFont(LOGFONT font);
	ProspaFont(const ProspaFont& copyMe);
	~ProspaFont();

	LOGFONT& font() {return font_;}
	COLORREF& color() {return color_;}
	short size() {return size_;}
	short style() {return style_;}
	char* name() {return name_;}

	void setColor(COLORREF color) {color_ = color; update();}
	void setStyle(short style) {style_ = style; update();}
	void setSize(short size) {size_ = size; update();}
	void setName(const char* const name) {strncpy_s(name_,PROSPA_FONT_NAME_LEN,name,_TRUNCATE); update();}
	void setFont(LOGFONT font);

private:
	LOGFONT font_;
	COLORREF color_;
	short size_;
	short style_;
	char name_[PROSPA_FONT_NAME_LEN];
	
	short lfHeight();
	short lfHeightInverse();
	int pixelsperinch(HDC hDC) {return GetDeviceCaps(hDC, LOGPIXELSY);} 
	void update();

	void scale_(double x, double y);
	void unscale_();
};

HFONT MakeFont(HDC, char *, short, short, bool, short);
HFONT GetFont(HDC, LOGFONT, short, short);
HFONT GetNewFont(HDC, LOGFONT, short,char[]);

char *GetFontStyleStr(short style); // defines
int ParseFontStyleStr(char* styleStr); // defines

#endif //define FONT_H