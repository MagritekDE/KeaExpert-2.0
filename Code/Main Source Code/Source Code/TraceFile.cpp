#include "stdafx.h"
#include "TraceFile.h"
#include "defines.h"
#include "PlotFile.h"
#include "PlotLabel.h"
#include "trace.h"
#include "memoryLeak.h"

/**
*	Interface to classes responsible for loading TracePar files. Nothing outside of TraceFile should want to do that.
*	Clients wanting to read TracePar files call @c makeLoader factory method to get a concrete 
*		TracePar* of the desired dimensionality and version. 
*/
class TraceParFile
{
public:
	/**
	*	Create a concrete TraceParFile.
	*	@param version version of the plot file
	*	@param fp the file containing the TracePar to read
	*	@return an TraceParFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static TraceParFile* makeLoader(int version, FILE* fp);
	/**
	*	Load an TracePar from the file.
	*	@return the next TracePar read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual TracePar* load() = 0;
	/**
	*	Copy constructor.
	*	@param copyMe the TraceParFile to copy
	*/
	TraceParFile(const TraceParFile& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this TraceParFile
	*/
	virtual TraceParFile* clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~TraceParFile();
protected:
	/**
	*	Constructor for an abstract TraceParFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	TraceParFile(int version, FILE* fp);
	
	FILE* fp_;		///< the file from which the plot is to be read
	int version_;	///< plot file version
};

class TraceParFile_V3_1 : public TraceParFile
{
public:
	TraceParFile_V3_1(int version, FILE* fp);
	TraceParFile_V3_1(const TraceParFile_V3_1& copyMe);
	TraceParFile* clone() const;
	virtual ~TraceParFile_V3_1();
	TracePar* load();
};

class TraceParFile_V3_2 : public TraceParFile
{
public:
	TraceParFile_V3_2(int version, FILE* fp);
	TraceParFile_V3_2(const TraceParFile_V3_2& copyMe);
	TraceParFile* clone() const;
	virtual ~TraceParFile_V3_2();
	TracePar* load();
};

class TraceParFile_V3_3 : public TraceParFile
{
public:
	TraceParFile_V3_3(int version, FILE* fp);
	TraceParFile_V3_3(const TraceParFile_V3_3& copyMe);
	TraceParFile* clone() const;
	virtual ~TraceParFile_V3_3();
	TracePar* load();
};

class TraceFile_V1_0 : public TraceFile
{
public:
	Trace* load();
	TraceFile_V1_0(int version, FILE* fp);
	TraceFile_V1_0(const TraceFile_V1_0& copyMe);
	TraceFile* clone() const;
	virtual ~TraceFile_V1_0();
};

class TraceFile_V1_4 : public TraceFile
{
public:
	Trace* load();
	TraceFile_V1_4(int version, FILE* fp);
	TraceFile_V1_4(const TraceFile_V1_4& copyMe);
	TraceFile* clone() const;
	virtual ~TraceFile_V1_4();
};

class TraceFile_V2_0 : public TraceFile
{
public:
	Trace* load();
	TraceFile_V2_0(int version, FILE* fp);
	TraceFile_V2_0(const TraceFile_V2_0& copyMe);
	TraceFile* clone() const;
	virtual ~TraceFile_V2_0();
};

class TraceFile_V2_1 : public TraceFile
{
public:
	Trace* load();
	TraceFile_V2_1(int version, FILE* fp);
	TraceFile_V2_1(const TraceFile_V2_1& copyMe);
	TraceFile* clone() const;
	virtual ~TraceFile_V2_1();
};

class TraceFile_V3_0 : public TraceFile
{
public:
	Trace* load();
	TraceFile_V3_0(int version, FILE* fp);
	TraceFile_V3_0(const TraceFile_V3_0& copyMe);
	TraceFile* clone() const;
	virtual ~TraceFile_V3_0();
};


class TraceFile_V3_1 : public TraceFile
{
public:
	Trace* load();
	TraceFile_V3_1(int version, FILE* fp);
	TraceFile_V3_1(const TraceFile_V3_1& copyMe);
	TraceFile* clone() const;
	virtual ~TraceFile_V3_1();
};

class TraceFile_V3_2 : public TraceFile
{
public:
	Trace* load();
	TraceFile_V3_2(int version, FILE* fp);
	TraceFile_V3_2(const TraceFile_V3_2& copyMe);
	TraceFile* clone() const;
	virtual ~TraceFile_V3_2();
};


TraceParFile* TraceParFile::makeLoader(int version, FILE* fp)
{
	switch(version)
	{
	   case PLOTFILE_VERSION_1_0:
	   case PLOTFILE_VERSION_1_1:
	   case PLOTFILE_VERSION_1_2:
	   case PLOTFILE_VERSION_1_4:
	   case PLOTFILE_VERSION_1_6:
	   case PLOTFILE_VERSION_2_0:
	   case PLOTFILE_VERSION_2_1:
	   case PLOTFILE_VERSION_2_2:
	   case PLOTFILE_VERSION_3_0:
         return 0;
      case PLOTFILE_VERSION_3_1:
   //		return new TraceParFile_V3_1(version, fp);
	   case PLOTFILE_VERSION_3_2:
	   case PLOTFILE_VERSION_3_3:
	   case PLOTFILE_VERSION_3_4:
	   case PLOTFILE_VERSION_3_5:
		   return new TraceParFile_V3_1(version, fp);

	   case PLOTFILE_VERSION_3_6:
		   return new TraceParFile_V3_2(version, fp);
	   case PLOTFILE_VERSION_3_7:
	   default:
		   return new TraceParFile_V3_3(version, fp);
	}
}

TraceParFile::TraceParFile(int version, FILE* fp)
{
	version_ = version;
	fp_ = fp;
}

TraceParFile::TraceParFile(const TraceParFile& copyMe)
{
	version_ = copyMe.version_;
	fp_ = copyMe.fp_;
}

TraceParFile::~TraceParFile()
{}

TraceParFile_V3_1::TraceParFile_V3_1(int version, FILE* fp)
: TraceParFile(version, fp)
{}

TraceParFile_V3_1::TraceParFile_V3_1(const TraceParFile_V3_1& copyMe)
: TraceParFile(copyMe)
{}

TraceParFile* TraceParFile_V3_1::clone() const
{
	return new TraceParFile_V3_1(*this);
}

TraceParFile_V3_1::~TraceParFile_V3_1()
{}

TracePar* TraceParFile_V3_1::load()
{	
	TracePar* par = new TracePar();
	
	short temp_short;
	fread(&temp_short, sizeof(short), 1, fp_);
	par->setTraceType(temp_short);

	COLORREF temp_colorref;
	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setRealColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setRealStyle(temp_short);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setImagColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setImagStyle(temp_short);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setRealSymbolColor(temp_colorref);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setImagSymbolColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setSymbolType(temp_short);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setSymbolSize(temp_short);

	bool temp_bool;
	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setShowErrorBars(temp_bool);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setFixedErrorBars(temp_bool);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setErrorBarsStored(temp_bool);

	float temp_float;
	fread(&temp_float, sizeof(float), 1, fp_);
	par->setBarFixedHeight(temp_float);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setBarColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setTraceWidth(temp_short);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setInLegend(temp_bool);
	
	return par;
}


TraceParFile_V3_2::TraceParFile_V3_2(int version, FILE* fp)
: TraceParFile(version, fp)
{}

TraceParFile_V3_2::TraceParFile_V3_2(const TraceParFile_V3_2& copyMe)
: TraceParFile(copyMe)
{}

TraceParFile* TraceParFile_V3_2::clone() const
{
	return new TraceParFile_V3_2(*this);
}

TraceParFile_V3_2::~TraceParFile_V3_2()
{}

TracePar* TraceParFile_V3_2::load()
{	
	TracePar* par = new TracePar();
	
	short temp_short;
	fread(&temp_short, sizeof(short), 1, fp_);
	par->setTraceType(temp_short);

	COLORREF temp_colorref;
	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setRealColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setRealStyle(temp_short);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setImagColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setImagStyle(temp_short);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setRealSymbolColor(temp_colorref);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setImagSymbolColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setSymbolType(temp_short);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setSymbolSize(temp_short);

	bool temp_bool;
	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setShowErrorBars(temp_bool);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setFixedErrorBars(temp_bool);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setErrorBarsStored(temp_bool);

	float temp_float;
	fread(&temp_float, sizeof(float), 1, fp_);
	par->setBarFixedHeight(temp_float);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setBarColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setTraceWidth(temp_short);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setInLegend(temp_bool);
	
	fread(&temp_float, sizeof(float), 1, fp_);
	par->setYOffset(temp_float);

	return par;
}


TraceParFile_V3_3::TraceParFile_V3_3(int version, FILE* fp)
: TraceParFile(version, fp)
{}

TraceParFile_V3_3::TraceParFile_V3_3(const TraceParFile_V3_3& copyMe)
: TraceParFile(copyMe)
{}

TraceParFile* TraceParFile_V3_3::clone() const
{
	return new TraceParFile_V3_3(*this);
}

TraceParFile_V3_3::~TraceParFile_V3_3()
{}

TracePar* TraceParFile_V3_3::load()
{	
	TracePar* par = new TracePar();
	
	short temp_short;
	fread(&temp_short, sizeof(short), 1, fp_);
	par->setTraceType(temp_short);

	COLORREF temp_colorref;
	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setRealColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setRealStyle(temp_short);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setImagColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setImagStyle(temp_short);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setRealSymbolColor(temp_colorref);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setImagSymbolColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setSymbolType(temp_short);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setSymbolSize(temp_short);

	bool temp_bool;
	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setShowErrorBars(temp_bool);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setFixedErrorBars(temp_bool);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setErrorBarsStored(temp_bool);

	float temp_float;
	fread(&temp_float, sizeof(float), 1, fp_);
	par->setBarFixedHeight(temp_float);

	fread(&temp_colorref, sizeof(COLORREF), 1, fp_);
	par->setBarColor(temp_colorref);

	fread(&temp_short, sizeof(short), 1, fp_);
	par->setTraceWidth(temp_short);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setInLegend(temp_bool);
	
	fread(&temp_float, sizeof(float), 1, fp_);
	par->setXOffset(temp_float);

	fread(&temp_float, sizeof(float), 1, fp_);
	par->setYOffset(temp_float);

	fread(&temp_bool, sizeof(bool), 1, fp_);
	par->setWhiteWash(temp_bool);

	return par;
}


TraceFile* TraceFile::makeLoader(FILE* fp, int version)
{
   // Ignored since incorrect in some older versions
	long versionOld;
	fread(&versionOld,sizeof(long),1,fp);

	switch(version)
	{
	   case PLOTFILE_VERSION_1_0:
		   return new TraceFile_V1_0(version,fp);
	   case PLOTFILE_VERSION_1_4:
		   return new TraceFile_V1_4(version,fp);
	   case PLOTFILE_VERSION_2_0:	
		   return new TraceFile_V2_0(version,fp);
	   case PLOTFILE_VERSION_2_1:
		   return new TraceFile_V2_1(version,fp);
	   case PLOTFILE_VERSION_3_0:
		   return new TraceFile_V3_0(version,fp);	
	   case PLOTFILE_VERSION_3_1:
		   return new TraceFile_V3_1(version,fp);	
	   case PLOTFILE_VERSION_1_1:
	   case PLOTFILE_VERSION_1_2:
	   case PLOTFILE_VERSION_1_6:
	   case PLOTFILE_VERSION_2_2:
		   return 0;
	   case PLOTFILE_VERSION_3_2:
	   case PLOTFILE_VERSION_3_3:
	   default:
		   return new TraceFile_V3_2(version,fp);
	}
}

TraceFile::TraceFile(const TraceFile& copyMe)
{
	fp_ = copyMe.fp_;
	version_ = copyMe.version_;
	traceParFile_ = copyMe.traceParFile_->clone();
}

TraceFile::~TraceFile()
{
	if (traceParFile_)
	{
		delete traceParFile_;
	}
}

TraceFile::TraceFile(int version, FILE* fp)
{
	fp_ = fp;
	version_ = version;
	traceParFile_ = TraceParFile::makeLoader(version,fp);
}

void TraceFile::setOldVersionDefaults(Trace* t)
{
	t->tracePar.setShowErrorBars(false);
	t->tracePar.setFixedErrorBars(false);
	t->tracePar.setErrorBarsStored(false);
	t->tracePar.setBarFixedHeight(0);
	t->tracePar.setBarColor(RGB(0,0,0));
	t->tracePar.setRealStyle(0);
	t->tracePar.setImagStyle(0);
	t->tracePar.setTraceWidth(1);
}

////

TraceFile_V1_0::TraceFile_V1_0(int version, FILE* fp)
: TraceFile(version, fp)
{}

TraceFile_V1_0::TraceFile_V1_0(const TraceFile_V1_0& copyMe)
: TraceFile(copyMe)
{}

TraceFile_V1_0::~TraceFile_V1_0()
{}

TraceFile* TraceFile_V1_0::clone() const
{
	return new TraceFile_V1_0(*this);
}

Trace* TraceFile_V1_0::load()
{

	Trace* t = new TraceReal(0);
	setOldVersionDefaults(t);
	long size;
	float minx, maxx, miny, maxy;

   fread(&size,sizeof(long),1,fp_);
	t->setSize(size);
   fread(&minx,sizeof(float),1,fp_);
	t->setMinX(minx);
   fread(&maxx,sizeof(float),1,fp_);
	t->setMaxX(maxx);
   fread(&miny,sizeof(float),1,fp_);
	t->setMinY(miny);
   fread(&maxy,sizeof(float),1,fp_);
	t->setMaxY(maxy);
	
	COLORREF realColor, symbolColor;
	short traceType, symbolType, symbolSize;
		
   fread(&realColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealColor(realColor);
   fread(&symbolColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealSymbolColor(symbolColor);
	t->tracePar.setImagSymbolColor(symbolColor);
   fread(&traceType,sizeof(short),1,fp_);
	t->tracePar.setTraceType(traceType);
   fread(&symbolType,sizeof(short),1,fp_);
   t->tracePar.setSymbolType(symbolType);
	fread(&symbolSize,sizeof(short),1,fp_);
   t->tracePar.setSymbolSize(symbolSize);
	char name[MAX_STR];
	fread(name,PLOT_STR_LEN,1,fp_);
	// Ignore the name of the trace in this version.
	// The plot label should be set at the plot level.
	//t->setName(name);
   t->tracePar.setHalftoneImagColor();
	
	if (ERR == t->readData(fp_))
	{
		delete t;
		throw PlotFileException("Failed to read trace data from plot file.");	
	}
	return t;
}


TraceFile_V1_4::TraceFile_V1_4(int version, FILE* fp)
: TraceFile(version, fp)
{}

TraceFile_V1_4::TraceFile_V1_4(const TraceFile_V1_4& copyMe)
: TraceFile(copyMe)
{}

TraceFile_V1_4::~TraceFile_V1_4()
{}

TraceFile* TraceFile_V1_4::clone() const
{
	return new TraceFile_V1_4(*this);
}

Trace* TraceFile_V1_4::load()
{

	short type;
	fread(&type,sizeof(short),1,fp_);

	Trace* t;
	if (type)
	{
		t = new TraceComplex(0);
	}
	else
	{
		t = new TraceReal(0);
	}

	setOldVersionDefaults(t);
	long size;
	float minx, maxx, miny, maxy;

   fread(&size,sizeof(long),1,fp_);
	t->setSize(size);
   fread(&minx,sizeof(float),1,fp_);
	t->setMinX(minx);
   fread(&maxx,sizeof(float),1,fp_);
	t->setMaxX(maxx);
   fread(&miny,sizeof(float),1,fp_);
	t->setMinY(miny);
   fread(&maxy,sizeof(float),1,fp_);
	t->setMaxY(maxy);
	
	COLORREF realColor, symbolColor;
	short traceType, symbolType, symbolSize;
		
   fread(&realColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealColor(realColor);
   fread(&symbolColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealSymbolColor(symbolColor);
	t->tracePar.setImagSymbolColor(symbolColor);
   fread(&traceType,sizeof(short),1,fp_);
	t->tracePar.setTraceType(traceType);
   fread(&symbolType,sizeof(short),1,fp_);
   t->tracePar.setSymbolType(symbolType);
	fread(&symbolSize,sizeof(short),1,fp_);
   t->tracePar.setSymbolSize(symbolSize);
	char name[MAX_STR];
	fread(name,PLOT_STR_LEN,1,fp_);
	// Ignore the name of the trace in this version.
	// The plot label should be set at the plot level.
	//t->setName(name);
   t->tracePar.setHalftoneImagColor();
	
	if (ERR == t->readData(fp_))
	{
		delete t;
		throw PlotFileException("Failed to read trace data from plot file.");	
	}
	return t;
}



TraceFile_V2_0::TraceFile_V2_0(int version, FILE* fp)
: TraceFile(version, fp)
{}

TraceFile_V2_0::TraceFile_V2_0(const TraceFile_V2_0& copyMe)
: TraceFile(copyMe)
{}

TraceFile_V2_0::~TraceFile_V2_0()
{}

TraceFile* TraceFile_V2_0::clone() const
{
	return new TraceFile_V2_0(*this);
}

Trace* TraceFile_V2_0::load()
{

	short type;
	fread(&type,sizeof(short),1,fp_);

	Trace* t;
	if (type)
	{
		t = new TraceComplex(0);
	}
	else
	{
		t = new TraceReal(0);
	}
	setOldVersionDefaults(t);

	long size;
	float minx, maxx, miny, maxy;

	fread(&size,sizeof(long),1,fp_);
	t->setSize(size);
	fread(&minx,sizeof(float),1,fp_);
	t->setMinX(minx);
	fread(&maxx,sizeof(float),1,fp_);
	t->setMaxX(maxx);
	fread(&miny,sizeof(float),1,fp_);
	t->setMinY(miny);
	fread(&maxy,sizeof(float),1,fp_);
	t->setMaxY(maxy);

	COLORREF realColor, symbolColor, barColor;
	short traceType, symbolType, symbolSize;
	bool showErrorBars, fixedErrorBars, errorBarsStored;
	float barFixedHeight;

	fread(&realColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealColor(realColor);
	fread(&symbolColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealSymbolColor(symbolColor);
	t->tracePar.setImagSymbolColor(symbolColor);
	fread(&traceType,sizeof(short),1,fp_);
	t->tracePar.setTraceType(traceType);
	fread(&symbolType,sizeof(short),1,fp_);
	t->tracePar.setSymbolType(symbolType);
	fread(&symbolSize,sizeof(short),1,fp_);
	t->tracePar.setSymbolSize(symbolSize);
	fread(&showErrorBars,sizeof(bool),1,fp_);
	t->tracePar.setShowErrorBars(showErrorBars);
	fread(&fixedErrorBars,sizeof(bool),1,fp_);
	t->tracePar.setFixedErrorBars(fixedErrorBars);
	fread(&errorBarsStored,sizeof(bool),1,fp_);
	t->tracePar.setErrorBarsStored(errorBarsStored);
	fread(&barFixedHeight,sizeof(float),1,fp_);
	t->tracePar.setBarFixedHeight(barFixedHeight);
	fread(&barColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setBarColor(barColor);
	if(t->tracePar.isErrorBarsStored())
	{
		if (ERR == t->readBars(fp_))
		{
			delete t;
			throw PlotFileException("Failed to read error bars from plot file.");	
		}
	}
			
	char name[MAX_STR];
	fread(name,PLOT_STR_LEN,1,fp_);
	// Ignore the name of the trace in this version.
	// The plot label should be set at the plot level.
	//data->setName(name);
	t->tracePar.setHalftoneImagColor();
	
	if (ERR == t->readData(fp_))
	{
		delete t;
		throw PlotFileException("Failed to read trace data from plot file.");	
	}
	return t;
}

TraceFile_V2_1::TraceFile_V2_1(int version, FILE* fp)
: TraceFile(version, fp)
{}

TraceFile_V2_1::TraceFile_V2_1(const TraceFile_V2_1& copyMe)
: TraceFile(copyMe)
{}

TraceFile_V2_1::~TraceFile_V2_1()
{}

TraceFile* TraceFile_V2_1::clone() const
{
	return new TraceFile_V2_1(*this);
}

Trace* TraceFile_V2_1::load()
{
	short type;
	fread(&type,sizeof(short),1,fp_);

	Trace* t;
	if (type)
	{
		t = new TraceComplex(0);
	}
	else
	{
		t = new TraceReal(0);
	}
	setOldVersionDefaults(t);

	long size;
	float minx, maxx, miny, maxy;

   fread(&size,sizeof(long),1,fp_);
	t->setSize(size);
   fread(&minx,sizeof(float),1,fp_);
	t->setMinX(minx);
   fread(&maxx,sizeof(float),1,fp_);
	t->setMaxX(maxx);
   fread(&miny,sizeof(float),1,fp_);
	t->setMinY(miny);
   fread(&maxy,sizeof(float),1,fp_);
	t->setMaxY(maxy);

	COLORREF realColor, imagColor, symbolColor, barColor;
	short realStyle, imagStyle, traceType, symbolType, symbolSize;
	bool showErrorBars, fixedErrorBars, errorBarsStored;
	float barFixedHeight;

	fread(&realColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealColor(realColor);
   fread(&imagColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setImagColor(imagColor);
   fread(&realStyle,sizeof(short),1,fp_);
   t->tracePar.setRealStyle(realStyle);
	fread(&imagStyle,sizeof(short),1,fp_);
	t->tracePar.setImagStyle(imagStyle);
	fread(&symbolColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealSymbolColor(symbolColor);
	t->tracePar.setImagSymbolColor(symbolColor);
	fread(&traceType,sizeof(short),1,fp_);
	t->tracePar.setTraceType(traceType);
	fread(&symbolType,sizeof(short),1,fp_);
	t->tracePar.setSymbolType(symbolType);
	fread(&symbolSize,sizeof(short),1,fp_);
	t->tracePar.setSymbolSize(symbolSize);
   fread(&showErrorBars,sizeof(bool),1,fp_);
   t->tracePar.setShowErrorBars(showErrorBars);
	fread(&fixedErrorBars,sizeof(bool),1,fp_);
   t->tracePar.setFixedErrorBars(fixedErrorBars);
	fread(&errorBarsStored,sizeof(bool),1,fp_);
   t->tracePar.setErrorBarsStored(errorBarsStored);
	fread(&barFixedHeight,sizeof(float),1,fp_);
   t->tracePar.setBarFixedHeight(barFixedHeight);
	fread(&barColor,sizeof(COLORREF),1,fp_);
   t->tracePar.setBarColor(barColor);
	if(t->tracePar.isErrorBarsStored())
   {
		if (ERR == t->readBars(fp_))
		{
			delete t;
			throw PlotFileException("Failed to read error bars from plot file.");	
		}
   }
	char name[MAX_STR];
	fread(name,PLOT_STR_LEN,1,fp_);
	// Ignore the name of the trace in this version.
	// The plot label should be set at the plot level.
	//t->setName(name);
	
	if (ERR == t->readData(fp_))
	{
		delete t;
		throw PlotFileException("Failed to read trace data from plot file.");	
	}
	return t;
}


TraceFile_V3_0::TraceFile_V3_0(int version, FILE* fp)
: TraceFile(version, fp)
{}

TraceFile_V3_0::TraceFile_V3_0(const TraceFile_V3_0& copyMe)
: TraceFile(copyMe)
{}

TraceFile_V3_0::~TraceFile_V3_0()
{}

TraceFile* TraceFile_V3_0::clone() const
{
	return new TraceFile_V3_0(*this);
}

Trace* TraceFile_V3_0::load()
{
	// set these:
      //if (side == LEFT_VAXIS_SIDE)
      //   data->setYAxis(data->getParent()->yAxisLeft());
      //else
	   //   data->setYAxis(data->getParent()->yAxisRight());
		//data->getParent()->setCurYAxis(side);
	short type;
	fread(&type,sizeof(short),1,fp_);

	Trace* t;
	if (type)
	{
		t = new TraceComplex(0);
	}
	else
	{
		t = new TraceReal(0);
	}
	setOldVersionDefaults(t);


	long size;
	float minx, maxx, miny, maxy;

   VerticalAxisSide side;
   fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	t->setSide(side);
	
   fread(&size,sizeof(long),1,fp_);
	t->setSize(size);
   fread(&minx,sizeof(float),1,fp_);
	t->setMinX(minx);
   fread(&maxx,sizeof(float),1,fp_);
	t->setMaxX(maxx);
   fread(&miny,sizeof(float),1,fp_);
	t->setMinY(miny);
   fread(&maxy,sizeof(float),1,fp_);
	t->setMaxY(maxy);

	COLORREF realColor, imagColor, symbolColor, barColor;
	short realStyle, imagStyle, traceType, symbolType, symbolSize,traceWidth;
	bool showErrorBars, fixedErrorBars, errorBarsStored,inLegend;
	float barFixedHeight;

   fread(&traceType, sizeof(short), 1, fp_);
   t->tracePar.setTraceType(traceType);
	fread(&realColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealColor(realColor);
   fread(&realStyle,sizeof(short),1,fp_);
   t->tracePar.setRealStyle(realStyle);
   fread(&imagColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setImagColor(imagColor);
	fread(&imagStyle,sizeof(short),1,fp_);
	t->tracePar.setImagStyle(imagStyle);
	fread(&symbolColor,sizeof(COLORREF),1,fp_);
	t->tracePar.setRealSymbolColor(symbolColor);
	t->tracePar.setImagSymbolColor(symbolColor);
	fread(&symbolType,sizeof(short),1,fp_);
	t->tracePar.setSymbolType(symbolType);
	fread(&symbolSize,sizeof(short),1,fp_);
	t->tracePar.setSymbolSize(symbolSize);
   fread(&showErrorBars,sizeof(bool),1,fp_);
   t->tracePar.setShowErrorBars(showErrorBars);
	fread(&fixedErrorBars,sizeof(bool),1,fp_);
   t->tracePar.setFixedErrorBars(fixedErrorBars);
	fread(&errorBarsStored,sizeof(bool),1,fp_);
   t->tracePar.setErrorBarsStored(errorBarsStored);
	fread(&barFixedHeight,sizeof(float),1,fp_);
   t->tracePar.setBarFixedHeight(barFixedHeight);
	fread(&barColor,sizeof(COLORREF),1,fp_);
   t->tracePar.setBarColor(barColor);
   fread(&traceWidth, sizeof(short), 1, fp_);
   t->tracePar.setTraceWidth(traceWidth);
   fread(&inLegend, sizeof(bool), 1, fp_);
   t->tracePar.setInLegend(inLegend);

	if(t->tracePar.isErrorBarsStored())
   {
		if (ERR == t->readBars(fp_))
		{
			delete t;
			throw PlotFileException("Failed to read error bars from plot file.");	
		}
   }
	char name[MAX_STR];
   fread(name, sizeof(char), MAX_STR, fp_);
   t->setName(name);
		
	if (ERR == t->readData(fp_))
	{
		delete t;
		throw PlotFileException("Failed to read trace data from plot file.");	
	}
	return t;
}

TraceFile_V3_1::TraceFile_V3_1(int version, FILE* fp)
: TraceFile(version, fp)
{}

TraceFile_V3_1::TraceFile_V3_1(const TraceFile_V3_1& copyMe)
: TraceFile(copyMe)
{}

TraceFile_V3_1::~TraceFile_V3_1()
{}

TraceFile* TraceFile_V3_1::clone() const
{
	return new TraceFile_V3_1(*this);
}

Trace* TraceFile_V3_1::load()
{
	short type;
	fread(&type,sizeof(short),1,fp_);

	Trace* t;
	if (type)
	{
		t = new TraceComplex(0);
	}
	else
	{
		t = new TraceReal(0);
	}
	setOldVersionDefaults(t);

	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	t->setSide(side);
	long size;
	fread(&size, sizeof(long), 1, fp_);
	t->setSize(size);
	float temp;
	fread(&temp, sizeof(float), 1, fp_);
	t->setMinX(temp);
	fread(&temp, sizeof(float), 1, fp_);
	t->setMaxX(temp);
	fread(&temp, sizeof(float), 1, fp_);
	t->setMinY(temp);
	fread(&temp, sizeof(float), 1, fp_);
	t->setMaxY(temp);

	TracePar* tp = traceParFile_->load();
	t->tracePar = *tp;
	delete(tp);
	
	/* TODO the "bar reading" should also be from an abstract factory */
	if (t->tracePar.isErrorBarsStored())
	{
		if (ERR == t->readBars(fp_))
		{
			delete t;
			throw PlotFileException("Failed to read error bars from plot file.");	
		}
	}
	char name[MAX_STR] = {'\0'};
	fread(name, sizeof(char), MAX_STR, fp_);
	if (ERR == t->readData(fp_))
	{
		delete t;
		throw PlotFileException("Failed to read trace data from plot file.");
	}
	t->setName(name);
	return t;
}



TraceFile_V3_2::TraceFile_V3_2(int version, FILE* fp)
: TraceFile(version, fp)
{}

TraceFile_V3_2::TraceFile_V3_2(const TraceFile_V3_2& copyMe)
: TraceFile(copyMe)
{}

TraceFile_V3_2::~TraceFile_V3_2()
{}

TraceFile* TraceFile_V3_2::clone() const
{
	return new TraceFile_V3_2(*this);
}

Trace* TraceFile_V3_2::load()
{
	short type;
	fread(&type,sizeof(short),1,fp_);

	Trace* t;
	if (type)
	{
		t = new TraceComplex(0);
	}
	else
	{
		t = new TraceReal(0);
	}
	setOldVersionDefaults(t);

	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	t->setSide(side);
	long size;
	fread(&size, sizeof(long), 1, fp_);
	t->setSize(size);
	float temp;
	fread(&temp, sizeof(float), 1, fp_);
	t->setMinX(temp);
	fread(&temp, sizeof(float), 1, fp_);
	t->setMaxX(temp);
	fread(&temp, sizeof(float), 1, fp_);
	t->setMinY(temp);
	fread(&temp, sizeof(float), 1, fp_);
	t->setMaxY(temp);

	TracePar* tp = traceParFile_->load();
	t->tracePar = *tp;
	delete(tp);
	
	/* TODO the "bar reading" should also be from an abstract factory */
	if (t->tracePar.isErrorBarsStored())
	{
		if (ERR == t->readBars(fp_))
		{
			delete t;
			throw PlotFileException("Failed to read error bars from plot file.");	
		}
	}
	char name[MAX_STR] = {'\0'};
	fread(name, sizeof(char), MAX_STR, fp_);
	if (ERR == t->readData(fp_))
	{
		delete t;
		throw PlotFileException("Failed to read trace data from plot file.");
	}
	t->setName(name);
	return t;
}

