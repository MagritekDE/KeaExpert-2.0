#ifndef TRACEFILE_H
#define TRACEFILE_H

/**
*	@file TraceFile.h
*	@brief Trace loading. This file contains the interface to load Trace from 1D and 2D plot files. 
*	Clients get a loader by calling @code TraceFile* aFile = TraceFile::makeLoader(version, fp) @endcode, and then get a @c Trace* by calling @code Trace* p = pFile->load() @endcode. 
*  @author Mike Davidson, Magritek Ltd
*	@date 2012 Magritek Ltd.
*	@version 1.0 Initial version 
*/ 

class Trace;
class TraceParFile;

/**
*	Interface to classes responsible for loading Trace files.
*	Clients wanting to read Trace files call @c makeLoader factory method to get a concrete 
*		TraceFile* 
*	Annoyingly, I'd made the positions of the axes within the plot file significant (ie, first
*		is X, second and third are Y axes...) in past versions, so TraceFile::load() needs extra 
*		parameters @c orientation, etc...
*/
class TraceFile
{
public:
	/**
	*	Create a concrete TraceFile.
	*	@param version version of the plot file
	*	@param fp the file containing the Trace to read
	*	@return an TraceFile that knows how to read from the specified type of file
	*	@throw PlotFileException on error
	*/
	static TraceFile* makeLoader(FILE* fp, int version);
	/**
	*	Load an Trace from the file.
	*	@param orientation the AxisOrientation of the Trace to be read
	*	@return the next Trace read from @c fp
	*	@throw PlotFileException on error
	*/
	virtual Trace* load() = 0;
	/**
	*	Copy constructor.
	*	@param copyMe the TraceFile to copy
	*/
	TraceFile(const TraceFile& copyMe);
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this TraceFile
	*/
	virtual TraceFile* clone() const = 0;
	/**
	*	Destructor.
	*/
	virtual ~TraceFile();

protected:
	/**
	*	Constructor for an abstract TraceFile
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	TraceFile(int version, FILE* fp);
	/**
	*	Set some old version defaults. This was being done in the old "Load1D" functions;
	*	keeping it since I'm not sure where it's needed and it's harmless anyways
	*	@param t the Trace to modify
	*/
	void setOldVersionDefaults(Trace* t);

	FILE* fp_;								///< the file from which the plot is to be read
	int version_;							///< plot file version
	TraceParFile* traceParFile_;		///< TracePar file reader
};

#endif // ifndef TRACEFILE_H