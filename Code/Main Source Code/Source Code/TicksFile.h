#ifndef TICKSFILE_H
#define TICKSFILE_H

/**
*	@file TicksFile.h
*	@brief Ticks loading. This file contains the interface to load Ticks from 1D and 2D plot files. 
*	Clients get a loader by calling @code TicksFile* aFile = TicksFile::makeLoader(version, fp) @endcode, and then get a @c Ticks* by calling @code Ticks* p = pFile->load() @endcode. 
*  @author Mike Davidson, Magritek Ltd
*	@date 2012 Magritek Ltd.
*	@version 1.0 Initial version 
*/ 

class Ticks;

/**
*	Interface to classes responsible for loading Ticks files.
*	Clients wanting to read Ticks files call @c makeLoader factory method to get a concrete 
*		TicksFile*.
*/
class TicksFile
{
public:
	/**
	*	Create a concrete TicksFile.
	*	@param version version of the plot file
	*	@param fp the file containing the Ticks to read
	*	@return an TicksFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static TicksFile* makeLoader(int version, FILE* fp);
	/**
	*	Load an Ticks from the file.
	*	@return the next Ticks read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual Ticks* load() = 0;
	/**
	*	Copy constructor.
	*	@param copyMe the TicksFile to copy
	*/
	TicksFile(const TicksFile& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this TicksFile
	*/
	virtual TicksFile* clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~TicksFile();

protected:
	/**
	*	Constructor for an abstract TicksFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	TicksFile(int version, FILE* fp);

	FILE* fp_;		///< the file from which the plot is to be read
	int version_;	///< plot file version
};





#endif // define TICKSFILE_H