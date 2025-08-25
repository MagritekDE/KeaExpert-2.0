#ifndef PLOTLABELFILE_H
#define PLOTLABELFILE_H

/**
*	@file PlotLabelFile.h
*	@brief PlotLabel loading. This file contains the interface to load PlotLabel from 1D and 2D plot files. 
*	Clients get a loader by calling @code PlotLabelFile* aFile = PlotLabelFile::makeLoader(version, fp) @endcode, and then get a @c PlotLabel* by calling @code PlotLabel* p = pFile->load() @endcode. 
*  @author Mike Davidson, Magritek Ltd
*	@date 2012 Magritek Ltd.
*	@version 1.0 Initial version 
*/ 

#include "PlotLabel.h"

/**
*	Interface to classes responsible for loading PlotLabel files.
*	Clients wanting to read PlotLabel files call @c makeLoader factory method to get a concrete 
*		PlotLabelFile*.
*/
class PlotLabelFile
{
public:
	/**
	*	Create a concrete PlotLabelFile.
	*	@param version version of the plot file
	*	@param fp the file containing the PlotLabel to read
	*	@return an PlotLabelFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static PlotLabelFile* makeLoader(int version, FILE* fp);
	/**
	*	Load an PlotLabel from the file.
	*	@param type the PlotLabelType of the PlotLabel to be read
	*	@return the next PlotLabel read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual PlotLabel* load(PlotLabelType type) = 0;
	/**
	*	Copy constructor.
	*	@param copyMe the PlotLabelFile to copy
	*/
	PlotLabelFile(const PlotLabelFile& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotLabelFile
	*/
	virtual PlotLabelFile*  clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~PlotLabelFile();

protected:
	/**
	*	Constructor for an abstract PlotLabelFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotLabelFile(int version, FILE* fp);

	FILE* fp_;		///< the file from which the plot is to be read
	int version_;	///< plot file version
};

#endif // define PLOTLABELFILE_H