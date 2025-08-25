#ifndef PLOTFILE_H
#define PLOTFILE_H

/**
*	@file PlotFile.h
*	@brief 1D and 2D file loading. This file contains the interface to load 1D and 2D plot files. 
*	Clients get a loader by calling @code PlotFile* pFile = PlotFile::makeLoader(dimensions, fp) @endcode, and then get a @c Plot* by calling @code Plot* p = pFile->load() @endcode. 
*  @author Mike Davidson, Magritek Ltd
*	@date 2012 Magritek Ltd.
*	@version 1.0 Initial version 
*/ 

#include <stdexcept>
//#include <exception>
#include <string>
#include "defines.h"

class AxisFile;
class InsetFile;
class Plot;
class Plot1D;
class Plot2D;
class PlotLabelFile;
class PlotLegendFile;
class Trace;

#define PLOTFILE_VERSION_1_0 'V1.0'
#define PLOTFILE_VERSION_1_1 'V1.1'
#define PLOTFILE_VERSION_1_2 'V1.2'
#define PLOTFILE_VERSION_1_4 'V1.4'
#define PLOTFILE_VERSION_1_6 'V1.6'
#define PLOTFILE_VERSION_2_0 'V2.0'
#define PLOTFILE_VERSION_2_1 'V2.1'
#define PLOTFILE_VERSION_2_2 'V2.2'
#define PLOTFILE_VERSION_3_0 'V3.0'
#define PLOTFILE_VERSION_3_1 'V3.1'
#define PLOTFILE_VERSION_3_2 'V3.2'
#define PLOTFILE_VERSION_3_3 'V3.3'
#define PLOTFILE_VERSION_3_4 'V3.4'
#define PLOTFILE_VERSION_3_5 'V3.5'
#define PLOTFILE_VERSION_3_6 'V3.6'
#define PLOTFILE_VERSION_3_7 'V3.7'
#define PLOTFILE_VERSION_3_8 'V3.8'
#define PLOTFILE_VERSION_3_9 'V3.9'
#define PLOTFILE_VERSION_4_0 'V4.0'
#define PLOTFILE_VERSION_4_1 'V4.1'
#define PLOTFILE_VERSION_4_2 'V4.2'
#define PLOTFILE_VERSION_4_3 'V4.3'
#define PLOTFILE_VERSION_4_4 'V4.4'
#define PLOTFILE_VERSION_4_5 'V4.5'
#define PLOTFILE_VERSION_4_6 'V4.6'
#define PLOTFILE_VERSION_4_7 'V4.7'
#define PLOTFILE_VERSION_4_8 'V4.8'
#define PLOTFILE_VERSION_4_9 'V4.9'
#define PLOTFILE_VERSION_5_0 'V5.0'

#define PLOTFILE_CURRENT_VERSION_1D PLOTFILE_VERSION_3_8
#define PLOTFILE_CURRENT_VERSION_2D PLOTFILE_VERSION_3_7

/**
*	Exceptions thrown while loading a plot file.
*/
class PlotFileException : public std::runtime_error
{
public:
	/**
	*	Constructor for PlotFileException.
	*	@param ss the message of this PlotFileException
	*/
	PlotFileException(std::string const& ss) : std::runtime_error(ss) 
	{}
};

/**
*	Exceptions thrown while loading a bitmap file.
*/
class BitmapFileException : public std::runtime_error
{
public:
	/**
	*	Constructor for BitmapFileException.
	*	@param ss the message of this BitmapFileException
	*/
	BitmapFileException(std::string const& ss) : std::runtime_error(ss) 
	{}
};

/**
*	Interface to classes responsible for loading plot files.
*	This is an abstract factory http://en.wikipedia.org/wiki/Abstract_factory_pattern
*	Clients wanting to read plot files call @c makeLoader to get a concrete 
*		PlotFile* of the desired dimensionality and version. 
*/
class PlotFile
{
public:
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile(const PlotFile& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	virtual PlotFile* clone() const = 0;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual Plot* load() = 0;
	
	/**
	*	Create a concrete PlotFile.
	*	@param dimensions 1 to read a 1D file, 2 to read a 2D file
	*	@param fp the file containing the plot to read
	*	@return a PlotFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static PlotFile* makeLoader(int dimensions,  FILE* fp);
	/**
	*	Get the current data directory.
	*	@return the current data directory.
	*/
	static char* getCurrDataDirectory() {return currDataDirectory;}
	/**
	*	Get the current macro directory.
	*	@return the current macro directory.
	*/
	static char* getCurrMacroDirectory() {return currMacroDirectory;}
	/**
	*	Set the current data directory.
	*	@param dir the new current data directory
	*/
	static void setCurrDataDirectory(const char* const dir){strncpy_s(currDataDirectory,MAX_STR,dir,_TRUNCATE);} 
	/**
	*	Set the current macro directory.
	*	@param dir the new current macro directory
	*/
	static void setCurrMacroDirectory(const char* const dir){strncpy_s(currMacroDirectory,MAX_STR,dir,_TRUNCATE);}

protected:
	/**
	*	Constructor for an abstract PlotFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile(int version, FILE* fp);

	int version_;								///< plot file version
	FILE* fp_;									///< the file from which the plot is to be read
	AxisFile* axisFile_;						///< axis file reader
	PlotLabelFile* plotLabelFile_;		///< plot label file reader
	PlotLegendFile* plotLegendFile_;		///< plot legend file reader
	InsetFile* insetFile_;					///< inset file reader

private:
	static char currDataDirectory[MAX_STR];	///< the current data directory
	static char currMacroDirectory[MAX_STR];	///< the current macro directory
};

/**
*	Interface to classes responsible for loading 1D plot files.
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile1D: public PlotFile
{
public:
	/**
	*	Constructor for an abstract PlotFile1D
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D(const PlotFile1D& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	virtual PlotFile* clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D();
	/**
	*	Create a concrete PlotFile1D.
	*	@param version the plot file version
	*	@param fp the file containing the plot to read.
	*	@return a PlotFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static PlotFile* makeLoader(FILE* fp);	
	/**
	*	Get the current 1D plot directory
	*	@return the current 1D plot directory
	*/
	static char* getCurrPlotDirectory() {return currPlotDirectory;}
	/**
	*	Get the current 1D import directory
	*	@return the current 1D import directory
	*/
	static char* getCurrImportDataDirectory() {return currImportDataDirectory;}
	/**
	*	Get the current 1D export directory
	*	@return the current 1D export directory
	*/
	static char* getCurrExportDataDirectory() {return currExportDataDirectory;}
	/**
	*	Set the current 1D plot directory
	*	@param dir the new current 1D plot directory
	*/
	static void setCurrPlotDirectory(const char* const dir){strncpy_s(currPlotDirectory,MAX_STR,dir,_TRUNCATE);}
	/**
	*	Set the current 1D import directory
	*	@param dir the new current 1D import directory
	*/
	static void setCurrImportDataDirectory(const char* const dir){strncpy_s(currImportDataDirectory,MAX_STR,dir,_TRUNCATE);}
	/**
	*	Set the current 1D export directory
	*	@param dir the new current 1D export directory
	*/
	static void setCurrExportDataDirectory(const char* const dir){strncpy_s(currExportDataDirectory,MAX_STR,dir,_TRUNCATE);}

	/**
	*	Translate a file version to long format
	*	@Translate a file version to long format
	*/
   static long translateFileVersion(long version);

private:
	static char currPlotDirectory[MAX_STR];			///< the current 1D plot directory
	static char currImportDataDirectory[MAX_STR];	///< the current 1D import directory
	static char currExportDataDirectory[MAX_STR];	///< the current 1D export directory
};

/**
*	Interface to classes responsible for loading 2D plot files.
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D: public PlotFile
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D(const PlotFile2D& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D();
	/**
	*	Create a concrete PlotFile2D.
	*	@param version the plot file version
	*	@param fp the file containing the plot to read.
	*	@return a PlotFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static PlotFile* makeLoader(FILE* fp);	

	/**
	*	Translate a file version to long format
	*	@Translate a file version to long format
	*/
   static long translateFileVersion(long version);

	/**
	*	Get the current 2D plot directory
	*	@return the current 2D plot directory
	*/
	static char* getCurrPlotDirectory() {return currPlotDirectory;}
	/**
	*	Get the current 2D import directory
	*	@return the current 2D import directory
	*/
	static char* getCurrImportDataDirectory() {return currImportDataDirectory;}
	/**
	*	Get the current 2D export directory
	*	@return the current 2D export directory
	*/
	static char* getCurrExportDataDirectory() {return currExportDataDirectory;}
	/**
	*	Set the current 2D plot directory
	*	@param dir the new current 2D plot directory
	*/
	static void setCurrPlotDirectory(const char* const dir){strncpy_s(currPlotDirectory,MAX_STR,dir,_TRUNCATE);}
	/**
	*	Set the current 2D import directory
	*	@param dir the new current 2D import directory
	*/
	static void setCurrImportDataDirectory(const char* const dir){strncpy_s(currImportDataDirectory,MAX_STR,dir,_TRUNCATE);}
	/**
	*	Set the current 2D export directory
	*	@param dir the new current 2D export directory
	*/
	static void setCurrExportDataDirectory(const char* const dir){strncpy_s(currExportDataDirectory,MAX_STR,dir,_TRUNCATE);}

protected:
	/**
	*	Each of the successive 2D plot file versions up to 3.0 just appends new stuff to
	*	the end of the previous format. For versions up to at least 3.0, each version of PlotFile can just call
	*	@c load_legacy_V1_0(plot) to set the version 1.0 fields, then in turn call @c load_legacy_V1_6_delta(plot),
	*	and so on, to load the fields that are new in that version. At the end of this, each PlotFile version must
	*	call @c load_legacy_data(plot) to get the actual plot data.
	*  
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V1_0(Plot2D* plot);
	/**
	*	Read in the fields that were new in version 1.6 of the 2D plot file format
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V1_6_delta(Plot2D* plot);
	/**
	*	Read in the fields that were new in version 2.0 of the 2D plot file format
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V2_0_delta(Plot2D* plot);
	/**
	*	Read in the fields that were new in version 2.1 of the 2D plot file format
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V2_1_delta(Plot2D* plot);
	/**
	*	Read in the fields that were new in version 3.0 of the 2D plot file format
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V3_0_delta(Plot2D* plot);
	/**
	*	Read in the fields that were new in version 3.2 of the 2D plot file format
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V3_2_delta(Plot2D* plot);
	/**
	*	Read in the fields that were new in version 3.3 of the 2D plot file format
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V3_3_delta(Plot2D* plot);
	/**
	*	Read in the fields that were new in version 3.4 of the 2D plot file format
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V3_4_delta(Plot2D* plot);
	/**
	*	Read in the fields that were new in version 3.5 of the 2D plot file format
	*	@param plot the plot whose fields are to be set by this loader
	*/
	void load_legacy_V3_5_delta(Plot2D* plot);
	/**
	*	Read in plot data.
	*	@param plot the plot whose data is to be set by this loader
	*/
	void load_legacy_V3_6_delta(Plot2D* plot);
	/**
	*	Read in plot data.
	*	@param plot the plot whose data is to be set by this loader
	*/
	void load_legacy_V3_7_delta(Plot2D* plot);
	/**
	*	Read in plot data.
	*	@param plot the plot whose data is to be set by this loader
	*/
	void load_legacy_data(Plot2D* plot);

private:
	static char currPlotDirectory[MAX_STR];			///< the current 2D plot directory
	static char currImportDataDirectory[MAX_STR];	///< the current 2D data import directory
	static char currExportDataDirectory[MAX_STR];	///< the current 2D data export directory
	/** 
	*	Read 2D plot data (as opposed to plot parameters)
	*	@param plot the plot whose data is to be set by this loader
	*/
	void ReadData(Plot2D* plot);
	/** 
	*	Read real 2D plot data from the current file position
	*	@param plot the plot whose data is to be set by this loader
	*/
	void ReadReal(Plot2D* plot);
	/** 
	*	Read complex 2D plot data from the current file position
	*	@param plot the plot whose data is to be set by this loader
	*/
	void ReadComplex(Plot2D* plot);
	/** 
	*	Read 2D vector plot data from the current file position
	*	@param plot the plot whose data is to be set by this loader
	*/
	void ReadVec2(Plot2D* plot);
};

///////////////////////////////////

#endif // define PLOTFILE_H 