#ifndef AXISFILE_H
#define AXISFILE_H

/**
*	@file AxisFile.h
*	@brief Axis loading. This file contains the interface to load Axes from 1D and 2D plot files. 
*	Clients get a loader by calling @code AxisFile* aFile = AxisFile::makeLoader(version, fp) @endcode, and then get an @c Axis* by calling @code Axis* a = aFile->load() @endcode. 
*  @author Mike Davidson, Magritek Ltd
*	@date 2012 Magritek Ltd.
*	@version 1.0 Initial version 
*/ 

#include "Axis.h"

class Plot;
class PlotGridFile;
class PlotLabelFile;
class TicksFile;

/**
*	Interface to classes responsible for loading Axis files.
*	Clients wanting to read plot files call @c makeLoader factory method to get a concrete 
*		AxisFile*.
*	Annoyingly, I'd made the positions of the axes within the plot file significant (ie, first
*		is X, second and third are Y axes...) in past versions, so AxisFile::load() needs extra 
*		parameters @c orientation, etc...
*/
class AxisFile
{
public:
	/**
	*	Create a concrete AxisFile.
	*	@param version version of the plot file
	*	@param fp the file containing the axis to read
	*	@return an AxisFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static AxisFile* makeLoader(int version, FILE* fp);
	/**
	*	Load an Axis from the file.
	*	@param orientation the AxisOrientation of the axis to be read
	*  @side the side of the axis to be read
	*	@return the next axis read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual Axis* load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent) = 0;
	/**
	*	Copy constructor.
	*	@param copyMe the AxisFile to copy
	*/
	AxisFile(const AxisFile& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this AxisFile
	*/
	virtual AxisFile* clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~AxisFile();

protected:
	/**
	*	Constructor for an abstract AxisFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	AxisFile(int version, FILE* fp);

	FILE* fp_;								///< the file from which the plot is to be read
	int version_;							///< plot file version
	TicksFile* ticksFile_;				///< axis file reader
	PlotGridFile* plotGridFile_;		///< plot grid file reader
	PlotLabelFile* plotLabelFile_;	///< plot label file reader
};



#endif // define AXISFILE_H