#ifndef PLOTLEGENDFILE_H
#define PLOTLEGENDFILE_H

/**
*	@file PlotLegendFile.h
*	@brief PlotLegend loading. This file contains the interface to load PlotLegend from 1D and 2D plot files. 
*	Clients get a loader by calling @code PlotLegendFile* aFile = PlotLegendFile::makeLoader(version, fp) @endcode, and then get a @c PlotLegend* by calling @code PlotLegend* p = pFile->load() @endcode. 
*  @author Mike Davidson, Magritek Ltd
*	@date 2012 Magritek Ltd.
*	@version 1.0 Initial version 
*/ 

class PlotLegend;

/**
*	Interface to classes responsible for loading PlotLegend files.
*	Clients wanting to read PlotLegend files call @c makeLoader factory method to get a concrete 
*		PlotLegendFile*.
*/
class PlotLegendFile
{
public:
	/**
	*	Create a concrete PlotLegendFile.
	*	@param version version of the plot file
	*	@param fp the file containing the PlotLegend to read
	*	@return an PlotLegendFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static PlotLegendFile* makeLoader(int version, FILE* fp);
	/**
	*	Load an PlotLegend from the file.
	*	@return the next PlotLegend read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual PlotLegend* load() = 0;
	/**
	*	Copy constructor.
	*	@param copyMe the PlotLegendFile to copy
	*/
	PlotLegendFile(const PlotLegendFile& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotLegendFile
	*/
	virtual PlotLegendFile* clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~PlotLegendFile();

protected:
	/**
	*	Constructor for an abstract PlotLegendFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotLegendFile(int version, FILE* fp);

	FILE* fp_;		///< the file from which the plot is to be read
	int version_;	///< plot file version
};

class PlotLegendFile_V3_0 : public PlotLegendFile
{
public:
	PlotLegendFile_V3_0(int version, FILE* fp);
	PlotLegend* load();
	PlotLegendFile_V3_0(const PlotLegendFile_V3_0& copyMe);
	PlotLegendFile* clone() const;
	virtual ~PlotLegendFile_V3_0();

protected:
};

#endif // define TICKSFILE_H