#ifndef PLOTGRIDFILE_H
#define PLOTGRIDFILE_H

/**
*	@file PlotGridFile.h
*	@brief PlotGrid loading. This file contains the interface to load PlotGrid from 1D and 2D plot files. 
*	Clients get a loader by calling @code PlotGridFile* aFile = PlotGridFile::makeLoader(version, fp) @endcode, and then get a @c PlotGrid* by calling @code PlotGrid* p = pFile->load() @endcode. 
*  @author Mike Davidson, Magritek Ltd
*	@date 2012 Magritek Ltd.
*	@version 1.0 Initial version 
*/ 

#include "Axis.h"

class PlotGrid;

/**
*	Interface to classes responsible for loading PlotGrid files.
*	Clients wanting to read PlotGrid files call @c makeLoader factory method to get a concrete 
*		PlotGridFile* of the desired dimensionality and version.
*	Annoyingly, I'd made the positions of the axes within the plot file significant (ie, first
*		is X, second and third are Y axes...) in past versions, so PlotGridFile::load() needs extra 
*		parameters @c orientation, etc...
*/
class PlotGridFile
{
public:
	/**
	*	Create a concrete PlotGridFile.
	*	@param version version of the plot file
	*	@param fp the file containing the PlotGrid to read
	*	@return an PlotGridFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static PlotGridFile* makeLoader(int version, FILE* fp);
	/**
	*	Load an PlotGrid from the file.
	*	@param orientation the AxisOrientation of the PlotGrid to be read
	*	@return the next PlotGrid read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual PlotGrid* load(AxisOrientation orientation) = 0;
	/**
	*	Copy constructor.
	*	@param copyMe the PlotGridFile to copy
	*/
	PlotGridFile(const PlotGridFile& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotGridFile
	*/
	virtual PlotGridFile* clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~PlotGridFile();

protected:
	/**
	*	Constructor for an abstract PlotGridFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotGridFile(int version, FILE* fp);

	FILE* fp_;		///< the file from which the plot is to be read
	int version_;	///< plot file version
};

#endif // define PLOTGRIDFILE_H