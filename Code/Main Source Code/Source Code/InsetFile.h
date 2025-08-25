#ifndef INSETFILE_H
#define INSETFILE_H

/**
*	@file InsetFile.h
*	@brief Inset loading. This file contains the interface to load Insets from 1D and 2D plot files. 
*	Clients get a loader by calling @code InsetFile* iFile = InsetFile::makeLoader(version, fp) @endcode, and then get a @c PlotGrid* by calling @code Inset* p = iFile->load() @endcode. 
*  @author Mike Davidson, Magritek Ltd
*	@date 2012 Magritek Ltd.
*	@version 1.0 Initial version 
*/ 

#include "Inset.h"

class Inset;

/**
*	Interface to classes responsible for loading Inset files.
*	Clients wanting to read Inset files call @c makeLoader factory method to get a concrete 
*		InsetFile*. 
*/
class InsetFile
{
public:
	/**
	*	Create a concrete InsetFile.
	*	@param version version of the plot file
	*	@param fp the file containing the Inset to read
	*	@return an InsetFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static InsetFile* makeLoader(int version, FILE* fp);
	/**
	*	Load an Inset from the file.
	*	@return the next Inset read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual Inset* load() = 0;
	/**
	*	Copy constructor.
	*	@param copyMe the InsetFile to copy
	*/
	InsetFile(const InsetFile& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this InsetFile
	*/
	virtual InsetFile* clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~InsetFile();

protected:
	/**
	*	Constructor for an abstract InsetFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	InsetFile(int version, FILE* fp);
	
	FILE* fp_;								///< the file from which the plot is to be read
	int version_;							///< plot file version
};

#endif // ifndef INSETFILE_H
