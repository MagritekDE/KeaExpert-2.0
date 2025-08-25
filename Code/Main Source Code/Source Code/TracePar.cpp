#include "stdafx.h"
#include "TracePar.h"
#include "plot.h"
#include "prospaResource.h"
#include "StringPairs.h"
#include "string_utilities.h"
#include <string>
#include <utility>
#include <vector>
#include "memoryLeak.h"

using std::pair;
using std::string;
using std::vector;
using std::make_pair;
/************************************************************************
* TracePar
*
* Represents display properties of a 1D trace.
*
* (c) Magritek 2011
*************************************************************************/


/************************************************************************
Mapping between symbolic constants and strings, for trace style and
data point symbols.
************************************************************************/
namespace TraceSymbol
{
	vector<pair<char*,int>> symbolType;
	vector<pair<char*,int>> traceType;

	void init()
	{

		TraceSymbol::symbolType.push_back(pair<char*, int>("opensquare",PLOT_SYMBOL_OPEN_SQUARE));
		TraceSymbol::symbolType.push_back(pair<char*, int>("square",PLOT_SYMBOL_SQUARE));
		TraceSymbol::symbolType.push_back(pair<char*, int>("opencircle",PLOT_SYMBOL_OPEN_CIRCLE));
		TraceSymbol::symbolType.push_back(pair<char*, int>("circle",PLOT_SYMBOL_CIRCLE));
		TraceSymbol::symbolType.push_back(pair<char*, int>("opentriangle",PLOT_SYMBOL_OPEN_TRIANGLE));
		TraceSymbol::symbolType.push_back(pair<char*, int>("triangle",PLOT_SYMBOL_TRIANGLE));
		TraceSymbol::symbolType.push_back(pair<char*, int>("openinvtriangle",PLOT_SYMBOL_OPEN_INV_TRIANGLE));
		TraceSymbol::symbolType.push_back(pair<char*, int>("invtriangle",PLOT_SYMBOL_INV_TRIANGLE));
		TraceSymbol::symbolType.push_back(pair<char*, int>("opendiamond",PLOT_SYMBOL_OPEN_DIAMOND));
		TraceSymbol::symbolType.push_back(pair<char*, int>("diamond",PLOT_SYMBOL_DIAMOND));
		TraceSymbol::symbolType.push_back(pair<char*, int>("cross",PLOT_SYMBOL_CROSS));
		TraceSymbol::symbolType.push_back(pair<char*, int>("plus",PLOT_SYMBOL_PLUS));
		TraceSymbol::symbolType.push_back(pair<char*, int>("none",PLOT_SYMBOL_NO_SYMBOL));

		TraceSymbol::traceType.push_back(pair<char*, int>("none",PLOT_TRACE_NONE));
		TraceSymbol::traceType.push_back(pair<char*, int>("lines",PLOT_TRACE_LINES));
		TraceSymbol::traceType.push_back(pair<char*, int>("stairs",PLOT_TRACE_STAIRS));
		TraceSymbol::traceType.push_back(pair<char*, int>("dots",PLOT_TRACE_DOTS));
	}
};

TracePar::TracePar()
{
	this->showErrorBars = false;
	this->errorBarsStored = false;
	this->barFixedHeight = 0;
	this->fixedErrorBars = true;	
	this->traceWidth = 0;
	this->realStyle = 0;
	this->imagStyle = 0;
	this->traceType = PLOT_TRACE_LINES;
	this->traceWidth = PS_SOLID;
	this->symbolType = TD_NO_SYMBOL;
	this->symbolSize = 3;
	this->xOffset = 0;
	this->yOffset = 0;
	this->realSymbolColor = RGB(255,0,0);
	this->imagSymbolColor = RGB(128,0,0);
	this->barColor = RGB(255,0,0);
	this->inLegend_ = true;
   this->whiteWash = false;
}
TracePar::~TracePar()
{
}

TracePar::TracePar(const TracePar &tp)
{
	this->realColor = tp.realColor;
	this->imagColor = tp.imagColor;
	this->realSymbolColor = tp.realSymbolColor;
	this->imagSymbolColor = tp.imagSymbolColor;
	this->barColor = tp.barColor;
	this->barFixedHeight = tp.barFixedHeight;
	this->fixedErrorBars = tp.fixedErrorBars;
	this->showErrorBars = tp.showErrorBars;
	this->errorBarsStored = tp.errorBarsStored;
	this->realStyle = tp.realStyle;
	this->imagStyle = tp.imagStyle;
	this->traceWidth = tp.traceWidth;
	this->traceType = tp.traceType;
	this->symbolType = tp.symbolType;
	this->symbolSize = tp.symbolSize;
	this->xOffset = tp.xOffset;
	this->yOffset = tp.yOffset;
	this->inLegend_ = tp.inLegend_;
   this->whiteWash = tp.whiteWash;
}

TracePar& TracePar::operator=(const TracePar &rhs) 
{
	this->realColor = rhs.realColor;
	this->imagColor = rhs.imagColor;
	this->realSymbolColor = rhs.realSymbolColor;
	this->imagSymbolColor = rhs.imagSymbolColor;
	this->barColor = rhs.barColor;
	this->barFixedHeight = rhs.barFixedHeight;
	this->fixedErrorBars = rhs.fixedErrorBars;
	this->showErrorBars = rhs.showErrorBars;
	this->errorBarsStored = rhs.errorBarsStored;
	this->realStyle = rhs.realStyle;
	this->imagStyle = rhs.imagStyle;
	this->traceWidth = rhs.traceWidth;
	this->traceType = rhs.traceType;
	this->symbolType = rhs.symbolType;
	this->symbolSize = rhs.symbolSize;
	this->xOffset = rhs.xOffset;
	this->yOffset = rhs.yOffset;
	this->inLegend_ = rhs.inLegend_;
	this->whiteWash = rhs.whiteWash;
	return *this;
}

/************************************************************************
Consists entirely of getters/setters.                                                                     
************************************************************************/
COLORREF TracePar::getRealColor() const {return this->realColor;}
COLORREF TracePar::getImagColor() const {return this->imagColor;}
COLORREF TracePar::getRealSymbolColor() const {return this->realSymbolColor;}
COLORREF TracePar::getImagSymbolColor() const {return this->imagSymbolColor;}
COLORREF TracePar::getBarColor() const {return this->barColor;}     // Error-bar colour
float TracePar::getBarFixedHeight() const {return this->barFixedHeight;}  // Fixed errorbar height
bool TracePar::isFixedErrorBars() const {return this->fixedErrorBars;}   // Use a fixed size for error bar?
bool TracePar::isShowErrorBars() const {return this->showErrorBars;}    // Display error bars?
bool TracePar::isErrorBarsStored() const {return this->errorBarsStored;}  // Are variable error bars stored?
short TracePar::getRealStyle() const {return this->realStyle;}       // Style of real line (one of PS_SOLID, PS_DASH ...)  
short TracePar::getImagStyle() const {return this->imagStyle;}       // Style of imag line (one of PS_SOLID, PS_DASH ...)  
short TracePar::getTraceWidth() const {return this->traceWidth;}      // trace line width in pixels
short TracePar::getTraceType() const {return this->traceType;}       // PLOT_TRACE_LINES, PLOT_TRACE_STAIRS, PLOT_TRACE_DOTS
short TracePar::getSymbolType() const {return this->symbolType;}     // Type of symbol
short TracePar::getSymbolSize() const {return this->symbolSize;}      // 1/2 size of symbol in pixels
float TracePar::getXOffset() const {return this->xOffset;}          // Return trace y offset
float TracePar::getYOffset() const {return this->yOffset;}          // Return trace y offset
bool TracePar::getWhiteWash() const {return this->whiteWash;}          // Return trace whitewash state
TracePar* TracePar::setRealColor(COLORREF col){this->realColor = col; return this;}
TracePar* TracePar::setImagColor(COLORREF col){this->imagColor = col; return this;}
TracePar* TracePar::setRealSymbolColor(COLORREF col){this->realSymbolColor = col; return this;}  // Symbol colour
TracePar* TracePar::setImagSymbolColor(COLORREF col){this->imagSymbolColor = col; return this;}  // Symbol colour
TracePar* TracePar::setBarColor(COLORREF col){this->barColor = col; return this;}     // Error-bar colour
TracePar* TracePar::setBarFixedHeight(float height){this->barFixedHeight = height; return this;}  // Fixed errorbar height
TracePar* TracePar::setFixedErrorBars(bool fixedErrorBars){this->fixedErrorBars = fixedErrorBars; return this;}   // Use a fixed size for error bar?
TracePar* TracePar::setShowErrorBars(bool showErrorBars){this->showErrorBars = showErrorBars; return this;}    // Display error bars?
TracePar* TracePar::setErrorBarsStored(bool errorBarsStored){this->errorBarsStored = errorBarsStored; return this;}  // Are variable error bars stored?
TracePar* TracePar::setRealStyle(short style){this->realStyle = style; return this;}       // Style of real line (one of PS_SOLID, PS_DASH ...)  
TracePar* TracePar::setImagStyle(short style){this->imagStyle = style; return this;}       // Style of imag line (one of PS_SOLID, PS_DASH ...)  
TracePar* TracePar::setTraceWidth(short width){this->traceWidth = width; return this;}      // trace line width in pixels
TracePar* TracePar::setTraceType(short type){this->traceType = type; return this;}       // PLOT_TRACE_LINES, PLOT_TRACE_STAIRS, PLOT_TRACE_DOTS
TracePar* TracePar::setSymbolType(short type){this->symbolType = type; return this;}      // Type of symbol
TracePar* TracePar::setSymbolSize(short size){this->symbolSize = size; return this;}      // 1/2 size of symbol in pixels
TracePar* TracePar::setXOffset(float off) {this->xOffset = off; return this;}             // Vertical shift of trace
TracePar* TracePar::setYOffset(float off) {this->yOffset = off; return this;}             // Vertical shift of trace
TracePar* TracePar::setInLegend(bool in)  {this->inLegend_ = in; return this;}  
TracePar* TracePar::setWhiteWash(bool in)  {this->whiteWash = in; return this;}  

/************************************************************************
Set the colour of the imaginary component of the trace to 1/2 the colour 
	of its real component.
************************************************************************/
TracePar* TracePar::setHalftoneImagColor()
{
	return setImagColor(RGB(GetRValue(realColor)/2,GetGValue(realColor)/2,GetBValue(realColor)/2)); 
}

/************************************************************************
Set all colours of the trace.
************************************************************************/
TracePar* TracePar::setColors(COLORREF traceCol, COLORREF symbolCol, COLORREF barCol)
{
	return this->setRealColor(traceCol)->setRealSymbolColor(symbolCol)->setImagSymbolColor(symbolCol)->setBarColor(barCol)->setHalftoneImagColor();
}

TracePar* TracePar::setColorsBlackNWhite()
{
	return this->setRealColor(RGB(0,0,0))->setImagColor(RGB(0,0,0))
		->setRealStyle(0)->setImagStyle(2)->setRealSymbolColor(RGB(0,0,0))->setImagSymbolColor(RGB(0,0,0))->setBarColor(RGB(0,0,0));
}

/************************************************************************
Translate back and forth between symbolic constants and strings.
************************************************************************/
const char* const TracePar::GetSymbolTypeStr(int type)
{
	vector<pair<char*,int>>::const_iterator it;
	for (it = TraceSymbol::symbolType.begin();it != TraceSymbol::symbolType.end();++it)
	{
		if (type == (*it).second)
		{
			return (*it).first;
		}
	}
	return NULL;
}

int TracePar::GetSymbolTypeCode(char *value)
{
	vector<pair<char*,int>>::const_iterator it;
	for (it = TraceSymbol::symbolType.begin();it != TraceSymbol::symbolType.end();++it)
	{
		if (!strcmp((char*)((*it).first),value))
		{
			return (*it).second;
		}
	}
	return -1;
}

const char* const  TracePar::GetTraceTypeStr(int type)
{
	vector<pair<char*,int>>::const_iterator it;
	for (it = TraceSymbol::traceType.begin();it != TraceSymbol::traceType.end();++it)
	{
		if (type == (*it).second)
		{
			return (*it).first;
		}
	}
	return NULL;
}


int TracePar::GetTraceTypeCode(char *value)
{
	vector<pair<char*,int>>::const_iterator it;
	for (it = TraceSymbol::traceType.begin();it != TraceSymbol::traceType.end();++it)
	{
		if (!strcmp (value,(char*)((*it).first)))
		{
			return (*it).second;
		}
	}
	return -1;
}

/************************************************************************
Generate a string representing the state of this object's parameter-
accessible attributes.
************************************************************************/
const string TracePar::FormatState()
{
	StringPairs state;
	state.add("color",Plot::GetColorStr(getRealColor()));
	state.add("ebarcolor",Plot::GetColorStr(this->getBarColor()));
	state.add("ebarshow",toTrueFalse(this->isShowErrorBars()));
	state.add("ebarfixed",toTrueFalse(this->isFixedErrorBars()));
	state.add("ebarsize", stringifyFloat(this->getBarFixedHeight()).c_str());
	state.add("imagcolor",Plot::GetColorStr(this->getImagColor()));
	state.add("imagstyle",stringifyInt(this->getImagStyle()).c_str());
	state.add("realcolor", Plot::GetColorStr(this->getRealColor()));
	state.add("realstyle", stringifyInt(this->getRealStyle()).c_str());
	state.add("realsymbolcolor", Plot::GetColorStr(this->getRealSymbolColor()));
	state.add("imagsymbolcolor", Plot::GetColorStr(this->getImagSymbolColor()));
	state.add("symbolshape", TracePar::GetSymbolTypeStr(this->getSymbolType()));
	state.add("symbolsize", stringifyInt(this->getSymbolSize()).c_str());
	state.add("tracetype", TracePar::GetTraceTypeStr(this->getTraceType()));
	state.add("tracewidth", stringifyFloat(this->getTraceWidth()/2.0+0.5).c_str());
	state.add("whitewash", stringifyInt(this->getWhiteWash()).c_str());
	state.add("xoffset", stringifyFloat(this->getXOffset()).c_str());
	state.add("yoffset", stringifyFloat(this->getYOffset()).c_str());
	
	return FormatStates(state);
}

short TracePar::save(FILE* fp, int version)
{
	if (!fp)
		return ERR;

	fwrite(&traceType, sizeof(short), 1, fp);
	fwrite(&realColor, sizeof(COLORREF), 1, fp);
	fwrite(&realStyle, sizeof(short), 1, fp);
	fwrite(&imagColor, sizeof(COLORREF), 1, fp);
	fwrite(&imagStyle, sizeof(short), 1, fp);
	fwrite(&realSymbolColor, sizeof(COLORREF), 1, fp);
	fwrite(&imagSymbolColor, sizeof(COLORREF), 1, fp);
	fwrite(&symbolType, sizeof(short), 1, fp);
	fwrite(&symbolSize, sizeof(short), 1, fp);
	fwrite(&showErrorBars, sizeof(bool), 1, fp);
	fwrite(&fixedErrorBars, sizeof(bool), 1, fp);
	fwrite(&errorBarsStored, sizeof(bool), 1, fp);
	fwrite(&barFixedHeight, sizeof(float), 1, fp);
	fwrite(&barColor, sizeof(COLORREF), 1, fp);
	fwrite(&traceWidth, sizeof(short), 1, fp);
	fwrite(&inLegend_, sizeof(bool), 1, fp);
   if(version == 360)
   {
      fwrite(&yOffset, sizeof(float), 1, fp);
   }
   if(version > 360)
   {
      fwrite(&xOffset, sizeof(float), 1, fp);
      fwrite(&yOffset, sizeof(float), 1, fp);
      fwrite(&whiteWash, sizeof(bool), 1, fp);
   }

	return OK;
}
