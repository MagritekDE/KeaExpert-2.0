#ifndef TRACE_PAR_H
#define TRACE_PAR_H

#include <string>

/************************************************************************
* TracePar
*
* Represents display properties of a 1D trace.
*
* (c) Magritek 2011
*************************************************************************/

// 1D trace type -  don't change these, they are used in the plot files

#define PLOT_TRACE_LINES   470
#define PLOT_TRACE_STAIRS  471
#define PLOT_TRACE_DOTS    472
#define PLOT_TRACE_NONE    473

// 1D symbol type -  don't change these, they are used in the plot files

#define PLOT_SYMBOL_DIAMOND    			474
#define PLOT_SYMBOL_TRIANGLE	  	 		475
#define PLOT_SYMBOL_INV_TRIANGLE			476
#define PLOT_SYMBOL_SQUARE					477
#define PLOT_SYMBOL_CIRCLE					478
#define PLOT_SYMBOL_OPEN_DIAMOND    	479
#define PLOT_SYMBOL_OPEN_TRIANGLE	   480
#define PLOT_SYMBOL_OPEN_INV_TRIANGLE	481
#define PLOT_SYMBOL_OPEN_SQUARE			482
#define PLOT_SYMBOL_OPEN_CIRCLE			483
#define PLOT_SYMBOL_PLUS         		484
#define PLOT_SYMBOL_CROSS         		485
#define PLOT_SYMBOL_NO_SYMBOL				486
#define PLOT_SYMBOL_SYMBOL_SIZE_CTRL   487
#define PLOT_SYMBOL_SYMBOL_SIZE_TEXT   489


namespace TraceSymbol
{
	void init();
}

class TracePar
{
public:
	TracePar();
	~TracePar();
	TracePar(const TracePar &tp);
	TracePar& operator=(const TracePar &tmp);
	/************************************************************************
	getters/setters.                                                                     
	************************************************************************/
	COLORREF getRealColor() const;         // Real line colour
	COLORREF getImagColor() const;			// Imaginary line colour
	COLORREF getRealSymbolColor() const;  // Real symbol colour
	COLORREF getImagSymbolColor() const;  // Imaginary symbol colour
	COLORREF getBarColor() const;     // Error-bar colour
	float getBarFixedHeight() const;  // Fixed errorbar height
	bool isFixedErrorBars() const;   // Use a fixed size for error bar?
	bool isShowErrorBars() const;    // Display error bars?
	bool isErrorBarsStored() const;  // Are variable error bars stored?
	short getRealStyle() const;       // Style of real line (one of PS_SOLID, PS_DASH ...)  
	short getImagStyle() const;       // Style of imag line (one of PS_SOLID, PS_DASH ...)  
	short getTraceWidth() const;      // trace line width in pixels
	short getTraceType() const;       // PLOT_TRACE_LINES, PLOT_TRACE_STAIRS, PLOT_TRACE_DOTS
	short getSymbolType() const;      // Type of symbol
	short getSymbolSize() const;      // 1/2 size of symbol in pixels
	float getXOffset() const;         // Return trace yoffset
	float getYOffset() const;         // Return trace yoffset
	bool getWhiteWash() const;         // Return trace yoffset
	bool inLegend() const {return inLegend_;}

	TracePar* setRealColor(COLORREF);    // Real line colour
	TracePar* setImagColor(COLORREF);    // Imaginary line colour
	TracePar* setRealSymbolColor(COLORREF);  // Real symbol colour
	TracePar* setImagSymbolColor(COLORREF);  // Imaginary symbol colour
	TracePar* setBarColor(COLORREF);     // Error-bar colour
	TracePar* setBarFixedHeight(float);  // Fixed errorbar height
	TracePar* setFixedErrorBars(bool);   // Use a fixed size for error bar?
	TracePar* setShowErrorBars(bool);    // Display error bars?
	TracePar* setErrorBarsStored(bool);  // Are variable error bars stored?
	TracePar* setRealStyle(short);       // Style of real line (one of PS_SOLID, PS_DASH ...)  
	TracePar* setImagStyle(short);       // Style of imag line (one of PS_SOLID, PS_DASH ...)  
	TracePar* setTraceWidth(short);      // trace line width in pixels
	TracePar* setTraceType(short);       // PLOT_TRACE_LINES, PLOT_TRACE_STAIRS, PLOT_TRACE_DOTS
	TracePar* setSymbolType(short);      // Type of symbol
	TracePar* setSymbolSize(short);      // 1/2 size of symbol in pixels
	TracePar* setXOffset(float);         // Horizontal shift of trace
	TracePar* setYOffset(float);         // Vertical shift of trace
	TracePar* setWhiteWash(bool);         // Set whether to display this trace in whitewash mode
	TracePar* setInLegend(bool);         // Set whether to display this trace in legend.

	/************************************************************************
	Set the colour of the imaginary component of the trace to 1/2 the colour 
		of its real component.
	************************************************************************/
	TracePar* setHalftoneImagColor();
	/************************************************************************
	Set all colours of the trace.
	************************************************************************/
	TracePar* setColors(COLORREF traceCol, COLORREF symbolCol, COLORREF barCol);
	TracePar* setColorsBlackNWhite();

	/************************************************************************
	Translate back and forth between symbolic constants and strings.
	************************************************************************/
	static const char* const GetSymbolTypeStr(int type);
	static int GetSymbolTypeCode(char *value);
	static const char* const GetTraceTypeStr(int type);
	static int GetTraceTypeCode(char *value);
	
	// Generate a string representing the state of this object's parameter-
	//  accessible attributes.
	const std::string FormatState();

	short save(FILE* fp, int version);
	short load(FILE* fp);

private:
	COLORREF realColor;    // Real line colour
	COLORREF imagColor;    // Imaginary line colour
	COLORREF realSymbolColor;  // Real symbol colour
	COLORREF imagSymbolColor;  // Imag symbol colour
	COLORREF barColor;     // Error-bar colour
	float barFixedHeight;  // Fixed errorbar height
	bool fixedErrorBars;   // Use a fixed size for error bar?
	bool showErrorBars;    // Display error bars?
	bool errorBarsStored;  // Are variable error bars stored?
	short realStyle;       // Style of real line (one of PS_SOLID, PS_DASH ...)  
	short imagStyle;       // Style of imag line (one of PS_SOLID, PS_DASH ...)  
	short traceWidth;      // trace line width in pixels
	short traceType;       // PLOT_TRACE_LINES, PLOT_TRACE_STAIRS, PLOT_TRACE_DOTS
	short symbolType;      // Type of symbol
	short symbolSize;      // 1/2 size of symbol in pixels
	float xOffset;         // x-offset of trace as a % of window width
	float yOffset;         // y-offset of trace as a % of window height
	bool inLegend_;        // Whether this trace should appear in the legend.
	bool whiteWash;		  // Whether this trace should be whitewashed.
};

#endif //define TRACE_PAR_H