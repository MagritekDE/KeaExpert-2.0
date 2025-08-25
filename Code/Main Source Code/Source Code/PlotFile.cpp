#include "stdafx.h"
#include "PlotFile.h"
#include "allocate.h"
#include "AxisFile.h"
#include "InsetFile.h"
#include "mymath.h"
#include "plotLine.h"
#include "Plot.h"
#include "PlotText.h"
#include "Plot2DCLI.h"
#include "PlotGrid.h"
#include "PlotLabelFile.h"
#include "PlotLegendFile.h"
#include "plotwindefaults.h"
#include "rgbColors.h"
#include "TraceFile.h"
#include <algorithm>
#include "memoryLeak.h"

using namespace std;

char PlotFile::currDataDirectory[MAX_STR] = {'\0'};
char PlotFile::currMacroDirectory[MAX_STR] = {'\0'};

char PlotFile1D::currPlotDirectory[MAX_STR] = {'\0'};
char PlotFile1D::currImportDataDirectory[MAX_STR] = {'\0'};
char PlotFile1D::currExportDataDirectory[MAX_STR] = {'\0'};

char PlotFile2D::currPlotDirectory[MAX_STR] = {'\0'};
char PlotFile2D::currImportDataDirectory[MAX_STR] = {'\0'};
char PlotFile2D::currExportDataDirectory[MAX_STR] = {'\0'};

/**
*	Load version 1.0 1D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile1D_V1_0 : public PlotFile1D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D_V1_0(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D_V1_0(const PlotFile1D_V1_0& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D_V1_0();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.0 1D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile1D_V3_0 : public PlotFile1D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D_V3_0(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D_V3_0(const PlotFile1D_V3_0& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D_V3_0();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.2 1D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile1D_V3_2 : public PlotFile1D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D_V3_2(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D_V3_2(const PlotFile1D_V3_2& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D_V3_2();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};



/**
*	Load version 3.3 1D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*	Version 3.3 adds a loop of Insets.
*/
class PlotFile1D_V3_3 : public PlotFile1D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D_V3_3(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D_V3_3(const PlotFile1D_V3_3& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D_V3_3();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.4 1D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*	Version 3.4 adds overlay line drawing.
*/
class PlotFile1D_V3_4 : public PlotFile1D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D_V3_4(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D_V3_4(const PlotFile1D_V3_4& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D_V3_4();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.5 1D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*	Version 3.5 adds overlay line drawing.
*/
class PlotFile1D_V3_5 : public PlotFile1D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D_V3_5(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D_V3_5(const PlotFile1D_V3_5& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D_V3_5();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};


/**
*	Load version 3.6 1D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*	Version 3.6 adds overlay line drawing.
*/
class PlotFile1D_V3_6 : public PlotFile1D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D_V3_6(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D_V3_6(const PlotFile1D_V3_6& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D_V3_6();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};



/**
*	Load version 3.8 1D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*	Version 3.6 adds overlay line drawing.
*/
class PlotFile1D_V3_8 : public PlotFile1D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile1D_V3_8(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile1D_V3_8(const PlotFile1D_V3_6& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile1D_V3_8();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};




/**
*	Load version 1.0 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V1_0 : public PlotFile2D
{
public:	
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V1_0(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V1_0(const PlotFile2D_V1_0& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V1_0();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 1.6 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V1_6 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V1_6(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V1_6(const PlotFile2D_V1_6& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V1_6();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 2.0 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V2_0 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V2_0(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V2_0(const PlotFile2D_V2_0& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V2_0();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 2.1 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V2_1 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V2_1(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V2_1(const PlotFile2D_V2_1& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V2_1();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.0 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V3_0 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V3_0(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V3_0(const PlotFile2D_V3_0& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V3_0();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.2 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V3_2 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V3_2(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V3_2(const PlotFile2D_V3_2& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V3_2();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.3 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V3_3 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V3_3(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V3_3(const PlotFile2D_V3_3& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V3_3();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.4 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V3_4 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V3_4(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V3_4(const PlotFile2D_V3_4& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V3_4();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.5 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V3_5 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V3_5(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V3_5(const PlotFile2D_V3_5& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V3_5();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

/**
*	Load version 3.6 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V3_6 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V3_6(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V3_6(const PlotFile2D_V3_6& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V3_6();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};


/**
*	Load version 3.7 2D plots 
*	Clients should never directly talk to this class. All instances should appear
*		transparently as PlotFiles.
*/
class PlotFile2D_V3_7 : public PlotFile2D
{
public:
	/**
	*	Constructor 
	*	@param version the plot file version
	*	@param fp the file from which the plot is to be read
	*/
	PlotFile2D_V3_7(int version, FILE* fp);
	/**
	*	Copy constructor.
	*	@param copyMe the PlotFile to copy
	*/
	PlotFile2D_V3_7(const PlotFile2D_V3_7& copyMe);
	/**
	*	Destructor.
	*/
	virtual ~PlotFile2D_V3_7();
	/**
	*	Polymorphic clone
	*	@return polymorphic clone of this PlotFile
	*/
	PlotFile* clone() const;
	/**
	*	Load a Plot from the file.
	*	@return the next plot read from @c fp
	*	@throw PlotFileException on error
	*/
	Plot* load();
};

PlotFile* PlotFile::makeLoader(int dimensions, FILE* fp)
{
	switch(dimensions)
	{
	   case 1:
		   return PlotFile1D::makeLoader(fp);
	   case 2:
		   return PlotFile2D::makeLoader(fp);
	   default:
		   throw PlotFileException("Invalid dimensions specified in plot file.");
   }
}

PlotFile::PlotFile(int version, FILE* fp)
{
	version_ = version;
	fp_ = fp;
	axisFile_ = 0;
	plotLabelFile_ = 0;
	plotLegendFile_ = 0;
	insetFile_ = 0;
}

PlotFile::~PlotFile()
{
	if (axisFile_)
	{
		delete axisFile_;
	}
	if (plotLabelFile_)
	{
		delete plotLabelFile_;
	}
	if (plotLegendFile_)
	{
		delete plotLegendFile_;
	}
	if (insetFile_)
	{
		delete insetFile_;
	}
}

PlotFile::PlotFile(const PlotFile& copyMe)
{
	version_ = copyMe.version_;
	fp_ = copyMe.fp_;
	axisFile_ = copyMe.axisFile_->clone();
	plotLabelFile_ = copyMe.plotLabelFile_->clone();
	plotLegendFile_ = copyMe.plotLegendFile_->clone();
	insetFile_ = copyMe.insetFile_->clone();
}

////////////////

PlotFile* PlotFile1D::makeLoader(FILE* fp)
{
// Read version
   long version;
   fread(&version,sizeof(long),1,fp);

	switch(version)
	{
	   case PLOTFILE_VERSION_1_0:
	   case PLOTFILE_VERSION_1_4:
	   case PLOTFILE_VERSION_1_6:
	   case PLOTFILE_VERSION_2_0:
	   case PLOTFILE_VERSION_2_1:
	   case PLOTFILE_VERSION_2_2:
		   return new PlotFile1D_V1_0(version, fp);
	   case PLOTFILE_VERSION_3_0:
	   case PLOTFILE_VERSION_3_1:
		   return new PlotFile1D_V3_0(version, fp);
	   case PLOTFILE_VERSION_3_2:
		   return new PlotFile1D_V3_2(version,fp);	
	   case PLOTFILE_VERSION_3_3:
		   return new PlotFile1D_V3_3(version,fp);
	   case PLOTFILE_VERSION_3_4:
		   return new PlotFile1D_V3_4(version,fp);
	   case PLOTFILE_VERSION_3_5:
		   return new PlotFile1D_V3_5(version,fp);
	   case PLOTFILE_VERSION_3_6:
      case PLOTFILE_VERSION_3_7:
		   return new PlotFile1D_V3_6(version,fp);
      case PLOTFILE_VERSION_3_8:
		   return new PlotFile1D_V3_8(version,fp);
      default:
         CText txt;
         txt.Format("Unsupported plot version: %ld",translateFileVersion(version));
         throw PlotFileException(txt.Str());
   }
}


long PlotFile1D::translateFileVersion(long fileVersion)
{
   switch(fileVersion)
   {
      case(PLOTFILE_VERSION_1_0):
         return(100);
      case(PLOTFILE_VERSION_1_1):
         return(110);
      case(PLOTFILE_VERSION_1_2):
         return(120);
      case(PLOTFILE_VERSION_1_4):
         return(140);
      case(PLOTFILE_VERSION_1_6):
         return(160);
      case(PLOTFILE_VERSION_2_0):
         return(200);
      case(PLOTFILE_VERSION_2_1):
         return(210);
      case(PLOTFILE_VERSION_2_2):
         return(220);
      case(PLOTFILE_VERSION_3_0):
         return(300);
      case(PLOTFILE_VERSION_3_1):
         return(310);
      case(PLOTFILE_VERSION_3_2):
         return(320);
      case(PLOTFILE_VERSION_3_3):
         return(330);
      case(PLOTFILE_VERSION_3_4):
         return(340);
      case(PLOTFILE_VERSION_3_5):
         return(350);
      case(PLOTFILE_VERSION_3_6):
         return(360);
      case(PLOTFILE_VERSION_3_7):
         return(370);
      case(PLOTFILE_VERSION_3_8):
         return(380);
      case(PLOTFILE_VERSION_3_9):
         return(390);
      case(PLOTFILE_VERSION_4_0):
         return(400);
      case(PLOTFILE_VERSION_4_1):
         return(410);
      case(PLOTFILE_VERSION_4_2):
         return(420);
      case(PLOTFILE_VERSION_4_3):
         return(430);
      case(PLOTFILE_VERSION_4_4):
         return(440);
      case(PLOTFILE_VERSION_4_5):
         return(450);
      case(PLOTFILE_VERSION_4_6):
         return(460);
      case(PLOTFILE_VERSION_4_7):
         return(470);
      case(PLOTFILE_VERSION_4_8):
         return(480);
      case(PLOTFILE_VERSION_4_9):
         return(490);
      case(PLOTFILE_VERSION_5_0):
         return(500);

   }
   return(0);
}

PlotFile1D::PlotFile1D(int version, FILE* fp)
: PlotFile(version, fp)
{
	axisFile_ = AxisFile::makeLoader(version, fp);
	plotLabelFile_ = PlotLabelFile::makeLoader(version,fp);
	plotLegendFile_ = PlotLegendFile::makeLoader(version,fp);
	insetFile_ = InsetFile::makeLoader(version,fp);
}

PlotFile1D::PlotFile1D(const PlotFile1D& copyMe)
: PlotFile(copyMe)
{
	if (copyMe.axisFile_)
	{
		axisFile_ = copyMe.axisFile_->clone();
   }
	if (copyMe.plotLabelFile_)
	{
		plotLabelFile_ = copyMe.plotLabelFile_->clone();
	}
	if (copyMe.plotLegendFile_)
	{
		plotLegendFile_ = copyMe.plotLegendFile_->clone();
	}
}

PlotFile1D::~PlotFile1D()
{  
}

////////////////////

PlotFile1D_V1_0::PlotFile1D_V1_0(int version, FILE* fp)
: PlotFile1D(version, fp)
{
}

PlotFile1D_V1_0::PlotFile1D_V1_0(const PlotFile1D_V1_0& copyMe)
: PlotFile1D(copyMe)
{}

PlotFile* PlotFile1D_V1_0::clone() const
{
	return new PlotFile1D_V1_0(*this);
}

PlotFile1D_V1_0::~PlotFile1D_V1_0()
{
}

Plot* PlotFile1D_V1_0::load()
{
	Plot1D* plot = new Plot1D(pwd,0,0,0,0,defaultMargins1D);

   plot->setFileVersion(version_);

	/////
	// loadTicks(fp, p);

	float majorTickLength = 0;
   fread(&majorTickLength,sizeof(float),1,fp_);
	float minorTickLength = 0;
   fread(&minorTickLength,sizeof(float),1,fp_);
   float xTickSpacing = 0;
	fread(&xTickSpacing,sizeof(float),1,fp_);
	float yTickSpacing = 0;
   fread(&yTickSpacing,sizeof(float),1,fp_);
	float xTicksPerLabel = 0;
   fread(&xTicksPerLabel,sizeof(float),1,fp_);
	float yTicksPerLabel = 0;
   fread(&yTicksPerLabel,sizeof(float),1,fp_);

	ProspaFont ticksFont(pwd->ticks->fontName(), pwd->ticks->fontColor(), pwd->ticks->fontSize(), pwd->ticks->fontStyle());
	Ticks xTicks(majorTickLength, minorTickLength, xTickSpacing, xTicksPerLabel, &ticksFont, RGB_BLACK);
	Ticks yTicks(majorTickLength, minorTickLength, yTickSpacing, yTicksPerLabel, &ticksFont, RGB_BLACK);
	plot->setXTicks(xTicks);
	plot->setYTicks(yTicks);

	/////
	// loadLabelFontParams(fp, p);

	LOGFONT labelFont;

   fread(&labelFont,sizeof(LOGFONT),1,fp_);
	ProspaFont pLabelFont(labelFont);
	for(Axis* axis : plot->axisList())
	{
		axis->ticks().setFont(pLabelFont);
	}
	
	fread(&labelFont,sizeof(LOGFONT),1,fp_);
	ProspaFont pLFont(labelFont);
	for(Axis* axis : plot->axisList())
	{
		axis->label().setFont(pLFont);
	}

	fread(&labelFont,sizeof(LOGFONT),1,fp_);
	ProspaFont titleFont(labelFont);
	plot->title().setFont(titleFont);

	COLORREF col;
   fread(&col,sizeof(COLORREF),1,fp_);
	for(Axis* axis : plot->axisList())
	{
		axis->ticks().setFontColor(col);
	}

	fread(&col,sizeof(COLORREF),1,fp_);
	for(Axis* axis : plot->axisList())
	{
		axis->label().setFontColor(col);
	}

   fread(&col,sizeof(COLORREF),1,fp_);
	plot->title().setFontColor(col);


	fread(&plot->yLabelVert,sizeof(bool),1,fp_);
   fread(&plot->axesMode,sizeof(short),1,fp_);


	/////
	// load2AxesMappings(fp,p);
	short mapping;
	// Read x mapping
   fread(&mapping,sizeof(short),1,fp_);
	plot->setAxisMapping(mapping);
	// Read y mapping
   fread(&mapping,sizeof(short),1,fp_);
	plot->setAxisMapping(mapping);

	/////
	//	loadDrawGrid(fp, p);

	bool draw = false;
   fread(&draw,sizeof(bool),1,fp_);
	plot->curXAxis()->grid()->setDrawGrid(draw);
   fread(&draw,sizeof(bool),1,fp_);
	plot->curYAxis()->grid()->setDrawGrid(draw);
   fread(&draw,sizeof(bool),1,fp_);
	plot->curXAxis()->grid()->setDrawFineGrid(draw);
   fread(&draw,sizeof(bool),1,fp_);
	plot->curYAxis()->grid()->setDrawFineGrid(draw);


   /////
	//	loadLabels(fp, p);

	char title[PLOT_STR_LEN];

	fread(title,PLOT_STR_LEN,1,fp_);
	plot->title().setText(title);
   fread(title,PLOT_STR_LEN,1,fp_);
	plot->curXAxis()->label().setText(title);
   fread(title,PLOT_STR_LEN,1,fp_);
	plot->curYAxis()->label().setText(title);
   fread(&plot->axesColor,sizeof(COLORREF),1,fp_);
   fread(&plot->bkColor,sizeof(COLORREF),1,fp_);
   fread(&plot->plotColor,sizeof(COLORREF),1,fp_);
   fread(&plot->borderColor,sizeof(COLORREF),1,fp_);

   /////
	//	loadGridColors(fp, p);

	fread(&col,sizeof(COLORREF),1,fp_);
	plot->curXAxis()->grid()->setColor(col);
	plot->curYAxis()->grid()->setColor(col);
   fread(&col,sizeof(COLORREF),1,fp_);
	plot->curXAxis()->grid()->setFineColor(col);
	plot->curYAxis()->grid()->setFineColor(col);


   /////
	//	loadMinMaxXY(fp,p);

	float val;
   fread(&val,sizeof(float),1,fp_);
	plot->curXAxis()->setMin(val);
   fread(&val,sizeof(float),1,fp_);
	plot->curXAxis()->setMax(val);
   fread(&val,sizeof(float),1,fp_);
	plot->curYAxis()->setMin(val);
   fread(&val,sizeof(float),1,fp_);
	plot->curYAxis()->setMax(val);

//
	bool autoxrange;
   fread(&autoxrange,sizeof(bool),1,fp_);   
	plot->curXAxis()->setAutorange(autoxrange);

// Read number of data sets
	short nrDataSets;
   fread(&nrDataSets,2,1,fp_); 
   
   for(short i = 0; i < nrDataSets; i++)
   {
		TraceFile* traceFile = TraceFile::makeLoader(fp_,version_);
		Trace* t = traceFile->load();
		if (!t)
		{
			delete traceFile;
			delete plot;
			throw PlotFileException("Failed to load trace from plot file.");		
		}			
		plot->appendTrace(t);	
		delete traceFile;
	}
	return plot;
}



PlotFile1D_V3_0::PlotFile1D_V3_0(int version, FILE* fp)
: PlotFile1D(version, fp)
{}

PlotFile1D_V3_0::PlotFile1D_V3_0(const PlotFile1D_V3_0& copyMe)
: PlotFile1D(copyMe)
{}


PlotFile* PlotFile1D_V3_0::clone() const
{
	return new PlotFile1D_V3_0(*this);
}

PlotFile1D_V3_0::~PlotFile1D_V3_0()
{
}	   	   

Plot* PlotFile1D_V3_0::load()
{
	Plot1D* plot = new Plot1D(pwd, defaultMargins1D);
   plot->setFileVersion(version_);
	plot->setTitle(plotLabelFile_->load(TITLE_PLOT_LABEL));
	fread(&plot->axesMode, sizeof(short), 1, fp_);
	fread(&plot->yLabelVert, sizeof(bool), 1, fp_);

	COLORREF axesColor, bkColor, plotColor, borderColor;
	fread(&axesColor, sizeof(COLORREF), 1, fp_);
	fread(&bkColor, sizeof(COLORREF), 1, fp_);
	fread(&plotColor, sizeof(COLORREF), 1, fp_);
	fread(&borderColor, sizeof(COLORREF), 1, fp_);
	plot->setColors(axesColor, bkColor, plotColor, borderColor);

	bool aa = false;
	fread(&aa, sizeof(bool),1,fp_);
	plot->setAntiAliasing(aa);

	plot->setLegend(plotLegendFile_->load());
 
	int axisCount;
	fread(&axisCount, sizeof(int), 1, fp_);

	// X axis must exist, and is the first in the file.
	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	Axis* axis = axisFile_->load(HORIZONTAL_AXIS, side, plot);
	plot->setXAxis(axis);
	plot->setCurXAxis(axis);

	// There's at least one Y axis, but which one?
	for (int i = 1; i < axisCount; i++)
   {
		fread(&side, sizeof(VerticalAxisSide), 1, fp_);
		axis = axisFile_->load(VERTICAL_AXIS, side, plot);
		switch(side)
      {
		   case LEFT_VAXIS_SIDE:
			   plot->setYAxisL(axis);
	         plot->setCurYAxis(axis);
			   break;
		   case RIGHT_VAXIS_SIDE:
			   plot->setYAxisR(axis);
	         plot->setCurYAxis(axis);
			   break;
		   default:
			   // error
			   delete plot;
			   throw PlotFileException("Trace specifies invalid axis reference in plot file.");		
      }
   }
// Read number of data sets
	short nrDataSets;
   fread(&nrDataSets,2,1,fp_); 
	for (short i = 0; i < nrDataSets; i++)
   {
		TraceFile* traceFile = TraceFile::makeLoader(fp_,version_);
		Trace* t = traceFile->load();
		if (!t)
      {
			delete traceFile;
			delete (plot);
			throw PlotFileException("Failed to load trace from plot file.");		
      } 
		plot->setCurYAxis(t->getSide());
		plot->appendTrace(t);	
		if (t->getSide() == LEFT_VAXIS_SIDE)
      {
			t->setYAxis(plot->yAxisLeft());
	  }
      else
      {
			t->setYAxis(plot->yAxisRight());
      }
		delete traceFile;
	}
	return plot;
}
   

     
PlotFile1D_V3_2::PlotFile1D_V3_2(int version, FILE* fp)
: PlotFile1D(version, fp)
{}

PlotFile1D_V3_2::PlotFile1D_V3_2(const PlotFile1D_V3_2& copyMe)
: PlotFile1D(copyMe)
{}


PlotFile* PlotFile1D_V3_2::clone() const
{
	return new PlotFile1D_V3_2(*this);
}

PlotFile1D_V3_2::~PlotFile1D_V3_2()
{
}

Plot* PlotFile1D_V3_2::load()
{
	Plot1D* plot = new Plot1D(pwd, defaultMargins1D);
   plot->setFileVersion(version_);
	plot->setTitle(plotLabelFile_->load(TITLE_PLOT_LABEL));
	fread(&plot->axesMode, sizeof(short), 1, fp_);
	fread(&plot->yLabelVert, sizeof(bool), 1, fp_);

	COLORREF axesColor, bkColor, plotColor, borderColor;
	fread(&axesColor, sizeof(COLORREF), 1, fp_);
	fread(&bkColor, sizeof(COLORREF), 1, fp_);
	fread(&plotColor, sizeof(COLORREF), 1, fp_);
	fread(&borderColor, sizeof(COLORREF), 1, fp_);
	plot->setColors(axesColor, bkColor, plotColor, borderColor);

	// Introduced in v3.2
	short display1DComplex;
	fread(&display1DComplex, sizeof(short), 1, fp_);
	plot->display1DComplex = display1DComplex;
	// end comment

	bool aa = false;
	fread(&aa, sizeof(bool),1,fp_);
	plot->setAntiAliasing(aa);

	plot->setLegend(plotLegendFile_->load());

	int axisCount;
	fread(&axisCount, sizeof(int), 1, fp_);
	
	// X axis must exist, and is the first in the file.
	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	Axis* axis = axisFile_->load(HORIZONTAL_AXIS, side, plot);
	plot->setXAxis(axis);
	plot->setCurXAxis(axis);

	// There's at least one Y axis, but which one?
	for (int i = 1; i < axisCount; i++)
   {
		fread(&side, sizeof(VerticalAxisSide), 1, fp_);
		axis = axisFile_->load(VERTICAL_AXIS, side, plot);
		switch(side)
		{
		   case LEFT_VAXIS_SIDE:
			   plot->setYAxisL(axis);
			   plot->setCurYAxis(axis);			
			   break;
		   case RIGHT_VAXIS_SIDE:
			   plot->setYAxisR(axis);
			   plot->setCurYAxis(axis);
			   break;
		   default:
			   // error
			   delete plot;
			   return 0;
		}      
   }
// Read number of data sets
	short nrDataSets;
   fread(&nrDataSets,2,1,fp_); 
	for (short i = 0; i < nrDataSets; i++)
   {
		TraceFile* traceFile = TraceFile::makeLoader(fp_,version_);
		Trace* t = traceFile->load();
		if (!t)
		{
			delete traceFile;
			delete (plot);
			throw PlotFileException("Trace specifies invalid axis reference in plot file.");	
		}
		plot->setCurYAxis(t->getSide());
		plot->appendTrace(t);	
		if (t->getSide() == LEFT_VAXIS_SIDE)
      {
			t->setYAxis(plot->yAxisLeft());
		}
		else
      {
			t->setYAxis(plot->yAxisRight());
		}
		delete traceFile;
	}
	return plot;
}


PlotFile1D_V3_3::PlotFile1D_V3_3(int version, FILE* fp)
: PlotFile1D(version, fp)
{}

PlotFile1D_V3_3::PlotFile1D_V3_3(const PlotFile1D_V3_3& copyMe)
: PlotFile1D(copyMe)
{}


PlotFile* PlotFile1D_V3_3::clone() const
{
	return new PlotFile1D_V3_3(*this);
}

PlotFile1D_V3_3::~PlotFile1D_V3_3()
{
}

Plot* PlotFile1D_V3_3::load()
{
	Plot1D* plot = new Plot1D(pwd, defaultMargins1D);
   plot->setFileVersion(version_);
	plot->setTitle(plotLabelFile_->load(TITLE_PLOT_LABEL));
	fread(&plot->axesMode, sizeof(short), 1, fp_);
	fread(&plot->yLabelVert, sizeof(bool), 1, fp_);

	COLORREF axesColor, bkColor, plotColor, borderColor;
	fread(&axesColor, sizeof(COLORREF), 1, fp_);
	fread(&bkColor, sizeof(COLORREF), 1, fp_);
	fread(&plotColor, sizeof(COLORREF), 1, fp_);
	fread(&borderColor, sizeof(COLORREF), 1, fp_);
	plot->setColors(axesColor, bkColor, plotColor, borderColor);

	// Introduced in v3.2
	short display1DComplex;
	fread(&display1DComplex, sizeof(short), 1, fp_);
	plot->display1DComplex = display1DComplex;
	// end comment

	bool aa = false;
	fread(&aa, sizeof(bool),1,fp_);
	plot->setAntiAliasing(aa);

	plot->setLegend(plotLegendFile_->load());

	/* New in version 3.3, the loop of Insets. Yeah, it should be generalized to include the legend: maybe in the next version */
	int insetCount;
	fread(&insetCount, sizeof(int), 1, fp_);
	for (int i = 0; i < insetCount; i++)
	{
		try 
		{
			plot->addInset(insetFile_->load());
		}
		catch (std::runtime_error& e)
		{
			ErrorMessage(e.what());
		}
	}

	int axisCount;
	fread(&axisCount, sizeof(int), 1, fp_);
	
	// X axis must exist, and is the first in the file.
	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	Axis* axis = axisFile_->load(HORIZONTAL_AXIS, side, plot);
	plot->setXAxis(axis);
	plot->setCurXAxis(axis);

//	 There's at least one Y axis, but which one?
	for (int i = 1; i < axisCount; i++)
   {
		fread(&side, sizeof(VerticalAxisSide), 1, fp_);
		axis = axisFile_->load(VERTICAL_AXIS, side, plot);
		switch(side)
		{
		   case LEFT_VAXIS_SIDE:
			   plot->setYAxisL(axis);
			   plot->setCurYAxis(axis);			
			   break;
		   case RIGHT_VAXIS_SIDE:
			   plot->setYAxisR(axis);
			   plot->setCurYAxis(axis);
			   break;
		   default:
			   // error
			   delete plot;
			   return 0;
		}      
   }
// Read number of data sets
	short nrDataSets;
   fread(&nrDataSets,2,1,fp_); 
	for (short i = 0; i < nrDataSets; i++)
   {
		TraceFile* traceFile = TraceFile::makeLoader(fp_,version_);
		Trace* t = traceFile->load();
		if (!t)
		{
			delete traceFile;
			delete (plot);
			throw PlotFileException("Trace specifies invalid axis reference in plot file.");	
		}
		plot->setCurYAxis(t->getSide());
		plot->appendTrace(t);	
		if (t->getSide() == LEFT_VAXIS_SIDE)
      {
			t->setYAxis(plot->yAxisLeft());
		}
		else
      {
			t->setYAxis(plot->yAxisRight());
		}
		delete traceFile;
	}

	return plot;
}


PlotFile1D_V3_4::PlotFile1D_V3_4(int version, FILE* fp)
: PlotFile1D(version, fp)
{}

PlotFile1D_V3_4::PlotFile1D_V3_4(const PlotFile1D_V3_4& copyMe)
: PlotFile1D(copyMe)
{}


PlotFile* PlotFile1D_V3_4::clone() const
{
	return new PlotFile1D_V3_4(*this);
}

PlotFile1D_V3_4::~PlotFile1D_V3_4()
{
}

Plot* PlotFile1D_V3_4::load()
{
	Plot1D* plot = new Plot1D(pwd, defaultMargins1D);
   plot->setFileVersion(version_);
	plot->setTitle(plotLabelFile_->load(TITLE_PLOT_LABEL));
	fread(&plot->axesMode, sizeof(short), 1, fp_);
	fread(&plot->yLabelVert, sizeof(bool), 1, fp_);

	COLORREF axesColor, bkColor, plotColor, borderColor;
	fread(&axesColor, sizeof(COLORREF), 1, fp_);
	fread(&bkColor, sizeof(COLORREF), 1, fp_);
	fread(&plotColor, sizeof(COLORREF), 1, fp_);
	fread(&borderColor, sizeof(COLORREF), 1, fp_);
	plot->setColors(axesColor, bkColor, plotColor, borderColor);

	// Introduced in v3.2
	short display1DComplex;
	fread(&display1DComplex, sizeof(short), 1, fp_);
	plot->display1DComplex = display1DComplex;
	// end comment

	bool aa = false;
	fread(&aa, sizeof(bool),1,fp_);
	plot->setAntiAliasing(aa);

	plot->setLegend(plotLegendFile_->load());

	/* New in version 3.3, the loop of Insets. Yeah, it should be generalized to include the legend: maybe in the next version */
	int insetCount;
	fread(&insetCount, sizeof(int), 1, fp_);
	for (int i = 0; i < insetCount; i++)
	{
		try 
		{
			plot->addInset(insetFile_->load());
		}
		catch (std::runtime_error& e)
		{
			ErrorMessage(e.what());
		}
	}

   // New in V3.4 line overlay
// load lines
   long nrLines;
	fread(&nrLines, sizeof(long), 1, fp_);
   for(long i = 0; i < nrLines; i++)
   {
      PlotLine* line = new PlotLine();
      line->Load(plot, fp_);
      plot->lines_.push_back(line);
   }

	int axisCount;
	fread(&axisCount, sizeof(int), 1, fp_);
	
	// X axis must exist, and is the first in the file.
	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	Axis* axis = axisFile_->load(HORIZONTAL_AXIS, side, plot);
	plot->setXAxis(axis);
	plot->setCurXAxis(axis);

//	 There's at least one Y axis, but which one?
	for (int i = 1; i < axisCount; i++)
   {
		fread(&side, sizeof(VerticalAxisSide), 1, fp_);
		axis = axisFile_->load(VERTICAL_AXIS, side, plot);
		switch(side)
		{
		   case LEFT_VAXIS_SIDE:
			   plot->setYAxisL(axis);
			   plot->setCurYAxis(axis);			
			   break;
		   case RIGHT_VAXIS_SIDE:
			   plot->setYAxisR(axis);
			   plot->setCurYAxis(axis);
			   break;
		   default:
			   // error
			   delete plot;
			   return 0;
		}      
   }
// Read number of data sets
	short nrDataSets;
   fread(&nrDataSets,2,1,fp_); 
	for (short i = 0; i < nrDataSets; i++)
   {
		TraceFile* traceFile = TraceFile::makeLoader(fp_,version_);
		Trace* t = traceFile->load();
		if (!t)
		{
			delete traceFile;
			delete (plot);
			throw PlotFileException("Trace specifies invalid axis reference in plot file.");	
		}
		plot->setCurYAxis(t->getSide());
		plot->appendTrace(t);	
		if (t->getSide() == LEFT_VAXIS_SIDE)
      {
			t->setYAxis(plot->yAxisLeft());
		}
		else
      {
			t->setYAxis(plot->yAxisRight());
		}
		delete traceFile;
	}

	return plot;
}



PlotFile1D_V3_5::PlotFile1D_V3_5(int version, FILE* fp)
: PlotFile1D(version, fp)
{}

PlotFile1D_V3_5::PlotFile1D_V3_5(const PlotFile1D_V3_5& copyMe)
: PlotFile1D(copyMe)
{}


PlotFile* PlotFile1D_V3_5::clone() const
{
	return new PlotFile1D_V3_5(*this);
}

PlotFile1D_V3_5::~PlotFile1D_V3_5()
{
}

Plot* PlotFile1D_V3_5::load()
{
	Plot1D* plot = new Plot1D(pwd, defaultMargins1D);
   plot->setFileVersion(version_);
	plot->setTitle(plotLabelFile_->load(TITLE_PLOT_LABEL));
	fread(&plot->axesMode, sizeof(short), 1, fp_);
	fread(&plot->yLabelVert, sizeof(bool), 1, fp_);

	COLORREF axesColor, bkColor, plotColor, borderColor;
	fread(&axesColor, sizeof(COLORREF), 1, fp_);
	fread(&bkColor, sizeof(COLORREF), 1, fp_);
	fread(&plotColor, sizeof(COLORREF), 1, fp_);
	fread(&borderColor, sizeof(COLORREF), 1, fp_);
	plot->setColors(axesColor, bkColor, plotColor, borderColor);

	// Introduced in v3.2
	short display1DComplex;
	fread(&display1DComplex, sizeof(short), 1, fp_);
	plot->display1DComplex = display1DComplex;
	// end comment

	bool aa = false;
	fread(&aa, sizeof(bool),1,fp_);
	plot->setAntiAliasing(aa);

	plot->setLegend(plotLegendFile_->load());

	/* New in version 3.3, the loop of Insets. Yeah, it should be generalized to include the legend: maybe in the next version */
	int insetCount;
	fread(&insetCount, sizeof(int), 1, fp_);
	for (int i = 0; i < insetCount; i++)
	{
		try 
		{
			plot->addInset(insetFile_->load());
		}
		catch (std::runtime_error& e)
		{
			ErrorMessage(e.what());
		}
	}

// load lines
   long nrLines;
	fread(&nrLines, sizeof(long), 1, fp_);
   for(long i = 0; i < nrLines; i++)
   {
      PlotLine* line = new PlotLine();
      line->Load(plot, fp_);
      plot->lines_.push_back(line);
   }

// New in V3.5 - text overlay
// load text
   long nrString;
	fread(&nrString, sizeof(long), 1, fp_);
   for(long i = 0; i < nrString; i++)
   {
      PlotText* txt = new PlotText();
      txt->Load(plot, fp_);
      plot->text_.push_back(txt);
   }

	int axisCount;
	fread(&axisCount, sizeof(int), 1, fp_);
	
	// X axis must exist, and is the first in the file.
	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	Axis* axis = axisFile_->load(HORIZONTAL_AXIS, side, plot);
	plot->setXAxis(axis);
	plot->setCurXAxis(axis);

//	 There's at least one Y axis, but which one?
	for (int i = 1; i < axisCount; i++)
   {
		fread(&side, sizeof(VerticalAxisSide), 1, fp_);
		axis = axisFile_->load(VERTICAL_AXIS, side, plot);
		switch(side)
		{
		   case LEFT_VAXIS_SIDE:
			   plot->setYAxisL(axis);
			   plot->setCurYAxis(axis);			
			   break;
		   case RIGHT_VAXIS_SIDE:
			   plot->setYAxisR(axis);
			   plot->setCurYAxis(axis);
			   break;
		   default:
			   // error
			   delete plot;
			   return 0;
		}      
   }
// Read number of data sets
	short nrDataSets;
   fread(&nrDataSets,2,1,fp_); 
	for (short i = 0; i < nrDataSets; i++)
   {
		TraceFile* traceFile = TraceFile::makeLoader(fp_,version_);
		Trace* t = traceFile->load();
		if (!t)
		{
			delete traceFile;
			delete (plot);
			throw PlotFileException("Trace specifies invalid axis reference in plot file.");	
		}
		plot->setCurYAxis(t->getSide());
		plot->appendTrace(t);	
		if (t->getSide() == LEFT_VAXIS_SIDE)
      {
			t->setYAxis(plot->yAxisLeft());
		}
		else
      {
			t->setYAxis(plot->yAxisRight());
		}
		delete traceFile;
	}

	return plot;
}



PlotFile1D_V3_6::PlotFile1D_V3_6(int version, FILE* fp)
: PlotFile1D(version, fp)
{}

PlotFile1D_V3_6::PlotFile1D_V3_6(const PlotFile1D_V3_6& copyMe)
: PlotFile1D(copyMe)
{}


PlotFile* PlotFile1D_V3_6::clone() const
{
	return new PlotFile1D_V3_6(*this);
}

PlotFile1D_V3_6::~PlotFile1D_V3_6()
{
}

Plot* PlotFile1D_V3_6::load()
{
	Plot1D* plot = new Plot1D(pwd, defaultMargins1D);
   plot->setFileVersion(version_);
	plot->setTitle(plotLabelFile_->load(TITLE_PLOT_LABEL));
	fread(&plot->axesMode, sizeof(short), 1, fp_);
	fread(&plot->yLabelVert, sizeof(bool), 1, fp_);

	COLORREF axesColor, bkColor, plotColor, borderColor;
	fread(&axesColor, sizeof(COLORREF), 1, fp_);
	fread(&bkColor, sizeof(COLORREF), 1, fp_);
	fread(&plotColor, sizeof(COLORREF), 1, fp_);
	fread(&borderColor, sizeof(COLORREF), 1, fp_);
	plot->setColors(axesColor, bkColor, plotColor, borderColor);

	// Introduced in v3.2
	short display1DComplex;
	fread(&display1DComplex, sizeof(short), 1, fp_);
	plot->display1DComplex = display1DComplex;
	// end comment

	bool aa = false;
	fread(&aa, sizeof(bool),1,fp_);
	plot->setAntiAliasing(aa);

	plot->setLegend(plotLegendFile_->load());

	/* New in version 3.3, the loop of Insets. Yeah, it should be generalized to include the legend: maybe in the next version */
	int insetCount;
	fread(&insetCount, sizeof(int), 1, fp_);
	for (int i = 0; i < insetCount; i++)
	{
		try 
		{
			plot->addInset(insetFile_->load());
		}
		catch (std::runtime_error& e)
		{
			ErrorMessage(e.what());
		}
	}

// load lines
   long nrLines;
	fread(&nrLines, sizeof(long), 1, fp_);
   for(long i = 0; i < nrLines; i++)
   {
      PlotLine* line = new PlotLine();
      line->Load(plot, fp_);
      plot->lines_.push_back(line);
   }

// New in V3.5 - text overlay
// load text
   long nrString;
	fread(&nrString, sizeof(long), 1, fp_);
   for(long i = 0; i < nrString; i++)
   {
      PlotText* txt = new PlotText();
      txt->Load(plot, fp_);
      plot->text_.push_back(txt);
   }

	int axisCount;
	fread(&axisCount, sizeof(int), 1, fp_);
	
	// X axis must exist, and is the first in the file.
	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	Axis* axis = axisFile_->load(HORIZONTAL_AXIS, side, plot);
	plot->setXAxis(axis);
	plot->setCurXAxis(axis);

//	 There's at least one Y axis, but which one?
	for (int i = 1; i < axisCount; i++)
   {
		fread(&side, sizeof(VerticalAxisSide), 1, fp_);
		axis = axisFile_->load(VERTICAL_AXIS, side, plot);
		switch(side)
		{
		   case LEFT_VAXIS_SIDE:
			   plot->setYAxisL(axis);
			   plot->setCurYAxis(axis);			
			   break;
		   case RIGHT_VAXIS_SIDE:
			   plot->setYAxisR(axis);
			   plot->setCurYAxis(axis);
			   break;
		   default:
			   // error
			   delete plot;
			   return 0;
		}      
   }
// Read number of data sets
	short nrDataSets;
   fread(&nrDataSets,2,1,fp_); 
	for (short i = 0; i < nrDataSets; i++)
   {
		TraceFile* traceFile = TraceFile::makeLoader(fp_,version_);
		Trace* t = traceFile->load();
		if (!t)
		{
			delete traceFile;
			delete (plot);
			throw PlotFileException("Trace specifies invalid axis reference in plot file.");	
		}
		plot->setCurYAxis(t->getSide());
		plot->appendTrace(t);	
		if (t->getSide() == LEFT_VAXIS_SIDE)
      {
			t->setYAxis(plot->yAxisLeft());
		}
		else
      {
			t->setYAxis(plot->yAxisRight());
		}
		delete traceFile;
	}

	return plot;
}


PlotFile1D_V3_8::PlotFile1D_V3_8(int version, FILE* fp)
: PlotFile1D(version, fp)
{}

PlotFile1D_V3_8::PlotFile1D_V3_8(const PlotFile1D_V3_6& copyMe)
: PlotFile1D(copyMe)
{}


PlotFile* PlotFile1D_V3_8::clone() const
{
	return new PlotFile1D_V3_8(*this);
}

PlotFile1D_V3_8::~PlotFile1D_V3_8()
{
}

Plot* PlotFile1D_V3_8::load()
{
	Plot1D* plot = new Plot1D(pwd, defaultMargins1D);
   plot->setFileVersion(version_);
	plot->setTitle(plotLabelFile_->load(TITLE_PLOT_LABEL));
	fread(&plot->axesMode, sizeof(short), 1, fp_);
	fread(&plot->yLabelVert, sizeof(bool), 1, fp_);

	COLORREF axesColor, bkColor, plotColor, borderColor;
	fread(&axesColor, sizeof(COLORREF), 1, fp_);
	fread(&bkColor, sizeof(COLORREF), 1, fp_);
	fread(&plotColor, sizeof(COLORREF), 1, fp_);
	fread(&borderColor, sizeof(COLORREF), 1, fp_);
	plot->setColors(axesColor, bkColor, plotColor, borderColor);

	// Introduced in v3.2
	short display1DComplex;
	fread(&display1DComplex, sizeof(short), 1, fp_);
	plot->display1DComplex = display1DComplex;
	// end comment

	bool aa = false;
	fread(&aa, sizeof(bool),1,fp_);
	plot->setAntiAliasing(aa);

	plot->setLegend(plotLegendFile_->load());

	/* New in version 3.3, the loop of Insets. Yeah, it should be generalized to include the legend: maybe in the next version */
	int insetCount;
	fread(&insetCount, sizeof(int), 1, fp_);
	for (int i = 0; i < insetCount; i++)
	{
		try 
		{
			plot->addInset(insetFile_->load());
		}
		catch (std::runtime_error& e)
		{
			ErrorMessage(e.what());
		}
	}

// load lines
   long nrLines;
	fread(&nrLines, sizeof(long), 1, fp_);
   for(long i = 0; i < nrLines; i++)
   {
      PlotLine* line = new PlotLine();
      line->Load3_8(plot, fp_);
      plot->lines_.push_back(line);
   }

// New in V3.5 - text overlay
// load text
   long nrString;
	fread(&nrString, sizeof(long), 1, fp_);
   for(long i = 0; i < nrString; i++)
   {
      PlotText* txt = new PlotText();
      txt->Load(plot, fp_);
      plot->text_.push_back(txt);
   }

	int axisCount;
	fread(&axisCount, sizeof(int), 1, fp_);
	
	// X axis must exist, and is the first in the file.
	VerticalAxisSide side;
	fread(&side, sizeof(VerticalAxisSide), 1, fp_);
	Axis* axis = axisFile_->load(HORIZONTAL_AXIS, side, plot);
	plot->setXAxis(axis);
	plot->setCurXAxis(axis);

//	 There's at least one Y axis, but which one?
	for (int i = 1; i < axisCount; i++)
   {
		fread(&side, sizeof(VerticalAxisSide), 1, fp_);
		axis = axisFile_->load(VERTICAL_AXIS, side, plot);
		switch(side)
		{
		   case LEFT_VAXIS_SIDE:
			   plot->setYAxisL(axis);
			   plot->setCurYAxis(axis);			
			   break;
		   case RIGHT_VAXIS_SIDE:
			   plot->setYAxisR(axis);
			   plot->setCurYAxis(axis);
			   break;
		   default:
			   // error
			   delete plot;
			   return 0;
		}      
   }
// Read number of data sets
	short nrDataSets;
   fread(&nrDataSets,2,1,fp_); 
	for (short i = 0; i < nrDataSets; i++)
   {
		TraceFile* traceFile = TraceFile::makeLoader(fp_,version_);
		Trace* t = traceFile->load();
		if (!t)
		{
			delete traceFile;
			delete (plot);
			throw PlotFileException("Trace specifies invalid axis reference in plot file.");	
		}
		plot->setCurYAxis(t->getSide());
		plot->appendTrace(t);	
		if (t->getSide() == LEFT_VAXIS_SIDE)
      {
			t->setYAxis(plot->yAxisLeft());
		}
		else
      {
			t->setYAxis(plot->yAxisRight());
		}
		delete traceFile;
	}

	
// Check for additional information
	if (!feof(fp_))
	{
		  long version;
		  plot->setFileVersion(version_);
        fread(&version,sizeof(long),1,fp_);
		  if(version == 380)
		  {
			// load lines
				long nrLines;
				plot->lines_.clear(); // Remove previous version
				fread(&nrLines, sizeof(long), 1, fp_);
				for(long i = 0; i < nrLines; i++)
				{
					PlotLine* line = new PlotLine();
					line->Load3_8(plot, fp_);
					plot->lines_.push_back(line);
				}
		  }
	}

	return plot;
}



////////////////

PlotFile* PlotFile2D::makeLoader(FILE* fp)
{

// Read version
   long version;
   fread(&version,sizeof(long),1,fp);

	switch(version)
	{
	   case PLOTFILE_VERSION_1_0:
		   return new PlotFile2D_V1_0(version, fp);
	   case PLOTFILE_VERSION_1_6:
		   return new PlotFile2D_V1_6(version, fp);
	   case PLOTFILE_VERSION_2_0:
		   return new PlotFile2D_V2_0(version, fp);
	   case PLOTFILE_VERSION_2_1:
		   return new PlotFile2D_V2_1(version, fp);
	   case PLOTFILE_VERSION_3_0:
	   case PLOTFILE_VERSION_3_1:
		   return new PlotFile2D_V3_0(version, fp);
	   case PLOTFILE_VERSION_3_2:
         return new PlotFile2D_V3_2(version, fp);
	   case PLOTFILE_VERSION_3_3:
         return new PlotFile2D_V3_3(version, fp);
	   case PLOTFILE_VERSION_3_4:
         return new PlotFile2D_V3_4(version, fp);
	   case PLOTFILE_VERSION_3_5:
         return new PlotFile2D_V3_5(version, fp);
	   case PLOTFILE_VERSION_3_6:
         return new PlotFile2D_V3_6(version, fp);
	   case PLOTFILE_VERSION_3_7:
         return new PlotFile2D_V3_7(version, fp);
	   default:
         CText txt;
         txt.Format("Unsupported plot version: %ld",translateFileVersion(version));
         throw PlotFileException(txt.Str());
	}
}

long PlotFile2D::translateFileVersion(long fileVersion)
{
   switch(fileVersion)
   {
      case(PLOTFILE_VERSION_1_0):
         return(100);
      case(PLOTFILE_VERSION_1_1):
         return(110);
      case(PLOTFILE_VERSION_1_2):
         return(120);
      case(PLOTFILE_VERSION_1_4):
         return(140);
      case(PLOTFILE_VERSION_1_6):
         return(160);
      case(PLOTFILE_VERSION_2_0):
         return(200);
      case(PLOTFILE_VERSION_2_1):
         return(210);
      case(PLOTFILE_VERSION_2_2):
         return(220);
      case(PLOTFILE_VERSION_3_0):
         return(300);
      case(PLOTFILE_VERSION_3_1):
         return(310);
      case(PLOTFILE_VERSION_3_2):
         return(320);
      case(PLOTFILE_VERSION_3_3):
         return(330);
      case(PLOTFILE_VERSION_3_4):
         return(340);
      case(PLOTFILE_VERSION_3_5):
         return(350);
      case(PLOTFILE_VERSION_3_6):
         return(360);
      case(PLOTFILE_VERSION_3_7):
         return(370);
      case(PLOTFILE_VERSION_3_8):
         return(380);
      case(PLOTFILE_VERSION_3_9):
         return(390);
      case(PLOTFILE_VERSION_4_0):
         return(400);
      case(PLOTFILE_VERSION_4_1):
         return(410);
      case(PLOTFILE_VERSION_4_2):
         return(420);
      case(PLOTFILE_VERSION_4_3):
         return(430);
      case(PLOTFILE_VERSION_4_4):
         return(440);
      case(PLOTFILE_VERSION_4_5):
         return(450);
      case(PLOTFILE_VERSION_4_6):
         return(460);
      case(PLOTFILE_VERSION_4_7):
         return(470);
      case(PLOTFILE_VERSION_4_8):
         return(480);
      case(PLOTFILE_VERSION_4_9):
         return(490);
      case(PLOTFILE_VERSION_5_0):
         return(500);

   }
   return(0);
}


PlotFile2D::PlotFile2D(int version, FILE* fp)
: PlotFile(version, fp)
{
}

PlotFile2D::PlotFile2D(const PlotFile2D& copyMe)
: PlotFile(copyMe)
{
}

PlotFile2D::~PlotFile2D()
{
}

void PlotFile2D::load_legacy_V1_0(Plot2D* plot)
{
	// Inifialise x axis range to see if this information is loaded from plot file
   plot->curXAxis()->setLength(0);
	plot->curXAxis()->setBase(0);

   plot->setVisibleLeft(-1);
   plot->setVisibleTop(-1);
   plot->setVisibleWidth(-1);
   plot->setVisibleHeight(-1);

	//loadTicks();
	float majorTickLength = 0;
   fread(&majorTickLength,sizeof(float),1,fp_);
	float minorTickLength = 0;
   fread(&minorTickLength,sizeof(float),1,fp_);
   float xTickSpacing = 0;
	fread(&xTickSpacing,sizeof(float),1,fp_);
	float yTickSpacing = 0;
   fread(&yTickSpacing,sizeof(float),1,fp_);
	float xTicksPerLabel = 0;
   fread(&xTicksPerLabel,sizeof(float),1,fp_);
	float yTicksPerLabel = 0;
   fread(&yTicksPerLabel,sizeof(float),1,fp_);
	ProspaFont ticksFont(pwd->ticks->fontName(), pwd->ticks->fontColor(), pwd->ticks->fontSize(), pwd->ticks->fontStyle());
	Ticks xTicks(majorTickLength, minorTickLength, xTickSpacing, xTicksPerLabel, &ticksFont, RGB_BLACK);
	Ticks yTicks(majorTickLength, minorTickLength, yTickSpacing, yTicksPerLabel, &ticksFont, RGB_BLACK);
	plot->setXTicks(xTicks);
	plot->setYTicks(yTicks);

	// loadLabelFontParams()
	LOGFONT labelFont;
   fread(&labelFont,sizeof(LOGFONT),1,fp_);
	ProspaFont pLabelFont(labelFont);
	for(Axis* axis : plot->axisList())
   {
		axis->ticks().setFont(pLabelFont);
	}
	fread(&labelFont,sizeof(LOGFONT),1,fp_);
	ProspaFont pLFont(labelFont);
	for(Axis* axis : plot->axisList())
	{
		axis->label().setFont(pLFont);
	}
	fread(&labelFont,sizeof(LOGFONT),1,fp_);
	ProspaFont titleFont(labelFont);
	plot->title().setFont(titleFont);
	COLORREF col;
   fread(&col,sizeof(COLORREF),1,fp_);
	for(Axis* axis : plot->axisList())
	{
		axis->ticks().setFontColor(col);
	}
	fread(&col,sizeof(COLORREF),1,fp_);
	for(Axis* axis : plot->axisList())
	{
		axis->label().setFontColor(col);
	}
   fread(&col,sizeof(COLORREF),1,fp_);
	plot->title().setFontColor(col);

	fread(&plot->yLabelVert,sizeof(bool),1,fp_);
	fread(&plot->axesMode,sizeof(short),1,fp_);
		
	//loadDrawGrid
	bool draw = false;
   fread(&draw,sizeof(bool),1,fp_);
	plot->curXAxis()->grid()->setDrawGrid(draw);
   fread(&draw,sizeof(bool),1,fp_);
	plot->curYAxis()->grid()->setDrawGrid(draw);
   fread(&draw,sizeof(bool),1,fp_);
	plot->curXAxis()->grid()->setDrawFineGrid(draw);
   fread(&draw,sizeof(bool),1,fp_);
	plot->curYAxis()->grid()->setDrawFineGrid(draw);
		
	//loadLabels
	char title[PLOT_STR_LEN];
	fread(title,PLOT_STR_LEN,1,fp_);
	plot->title().setText(title);
   fread(title,PLOT_STR_LEN,1,fp_);
	plot->curXAxis()->label().setText(title);
   fread(title,PLOT_STR_LEN,1,fp_);
	plot->curYAxis()->label().setText(title);
		
	COLORREF axes, bkgd, plotColor, border;
	fread(&axes,sizeof(COLORREF),1,fp_);
   fread(&bkgd,sizeof(COLORREF),1,fp_);
   fread(&plotColor,sizeof(COLORREF),1,fp_);
   fread(&border,sizeof(COLORREF),1,fp_);
	plot->setColors(axes, bkgd, plotColor, border);

	// loadGridColors
	fread(&col,sizeof(COLORREF),1,fp_);
	plot->curXAxis()->grid()->setColor(col);
	plot->curYAxis()->grid()->setColor(col);
   fread(&col,sizeof(COLORREF),1,fp_);
	plot->curXAxis()->grid()->setFineColor(col);
	plot->curYAxis()->grid()->setFineColor(col);

   short scale;
	fread(&scale,sizeof(short),1,fp_);
	plot->setColorScale(scale);
}

void PlotFile2D::load_legacy_V1_6_delta(Plot2D *plot)
{
	// Everything following introduced in v1.6.
	float dim;
   fread(&dim,sizeof(float),1,fp_);
	plot->setVectorLength(dim);
   fread(&plot->xVectorStep,sizeof(short),1,fp_);
   fread(&plot->yVectorStep,sizeof(short),1,fp_);
	short mode;
   fread(&mode,sizeof(short),1,fp_);
	plot->setDrawMode(mode);
   if(gDefaultColorMap != NULL && gDefaultColorMapLength > 0)
   {
		plot->setColorMap(CopyMatrix(gDefaultColorMap,3,gDefaultColorMapLength));
      plot->setColorMapLength(gDefaultColorMapLength);
	}
	// End new stuff.
}


void PlotFile2D::load_legacy_V2_0_delta(Plot2D *plot)
{	
	// Everything following introduced in v2.0
   long length;
   fread(&length,sizeof(long),1,fp_);
   if(length)
   {
   	plot->setColorMapLength(length);
      if(plot->colorMap()) FreeMatrix2D(plot->colorMap());
      plot->setColorMap(MakeMatrix2D(3,plot->colorMapLength()));
      fread(&(plot->colorMap()[0][0]),sizeof(float),3*plot->colorMapLength(),fp_);
   }
	// end new bits
}

void PlotFile2D::load_legacy_V2_1_delta(Plot2D *plot)
{	
// Everything following introduced in v2.1
	bool display;
   fread(&display,sizeof(bool),1,fp_);
   plot->setDisplayColorScale(display);

//load2AxesDimensions()
	float dim;
   fread(&dim,sizeof(float),1,fp_);
	plot->curXAxis()->setBase(dim);
   fread(&dim,sizeof(float),1,fp_);
	plot->curXAxis()->setLength(dim);
   fread(&dim,sizeof(float),1,fp_);
	plot->curYAxis()->setBase(dim);
   fread(&dim,sizeof(float),1,fp_);
	plot->curYAxis()->setLength(dim);

// end new bits
}

void PlotFile2D::load_legacy_V3_0_delta(Plot2D *plot)
{
// Everything following introduced in v2.1
//loadDisplayedExtremes
	long val;
   fread(&val,sizeof(long),1,fp_);
	plot->setVisibleLeft(val);
   fread(&val,sizeof(long),1,fp_);
	plot->setVisibleTop(val);
   fread(&val,sizeof(long),1,fp_);
	plot->setVisibleWidth(val);
   fread(&val,sizeof(long),1,fp_);
	plot->setVisibleHeight(val);
// end new bits
}

void PlotFile2D::load_legacy_V3_2_delta(Plot2D *plot)
{
// Everything following introduced in v3.2
//loadAxesDirections
   PlotDirection direction;
	fread(&direction, sizeof(PlotDirection), 1, fp_);
   plot->curXAxis()->setDirection(direction);
	fread(&direction, sizeof(PlotDirection), 1, fp_);
   plot->curYAxis()->setDirection(direction);
// end new bits
}

void PlotFile2D::load_legacy_V3_3_delta(Plot2D *plot)
{
// Everything following introduced in v3.3
// load mapping
   short mapping;
	fread(&mapping, sizeof(short), 1, fp_);
   plot->curXAxis()->setMapping(mapping);
	fread(&mapping, sizeof(short), 1, fp_);
   plot->curYAxis()->setMapping(mapping);
// load lines
   long nrLines;
	fread(&nrLines, sizeof(long), 1, fp_);
   for(long i = 0; i < nrLines; i++)
   {
      PlotLine* line = new PlotLine();
      line->Load(plot, fp_);
      plot->lines_.push_back(line);
   }
// end new bits
}

void PlotFile2D::load_legacy_V3_4_delta(Plot2D *plot)
{
// load fixed contour color
   bool useFixedColor;
   bool useFixedLevels;
   short nrLevels;
   COLORREF col;

   fread(&useFixedColor,sizeof(bool),1,fp_);
   fread(&nrLevels,sizeof(short),1,fp_);
   fread(&col,sizeof(COLORREF),1,fp_);
   fread(&useFixedColor,sizeof(bool),1,fp_);
   fread(&useFixedLevels,sizeof(bool),1,fp_);
   if(useFixedLevels && nrLevels>0)
   {
      if(plot->contourLevels)
         delete [] plot->contourLevels;
      plot->contourLevels = new float[nrLevels];
      fread(plot->contourLevels,sizeof(float),nrLevels,fp_);
   }
   plot->nrContourLevels = nrLevels; 
   plot->setUseFixedContourColor(useFixedColor);
   plot->setFixedContourColor(col);

// end new bits
}

void PlotFile2D::load_legacy_V3_5_delta(Plot2D *plot)
{
// Load axes information

   bool xCalibrated = false;
   long width = 0;

   fread(&xCalibrated,sizeof(bool),1,fp_);
   plot->SetXCalibration(xCalibrated);
   fread(&width,sizeof(long),1,fp_);

   if(width > 0)
   {
      plot->xAxis = new float[width];
      fread(&plot->xAxis,sizeof(float),width,fp_);
   }

   bool yCalibrated = false;
   long height = 0;

   fread(&yCalibrated,sizeof(bool),1,fp_);
   plot->SetYCalibration(yCalibrated);
   fread(&height,sizeof(long),1,fp_);

   if(height > 0)
   {
      plot->yAxis = new float[height];
      fread(&plot->yAxis,sizeof(float),height,fp_);
   }

}


void PlotFile2D::load_legacy_V3_6_delta(Plot2D *plot)
{
// Load data mapping information

   float minV,maxV;
	bool overRide;
   fread(&minV,sizeof(float),1,fp_);
   fread(&maxV,sizeof(float),1,fp_);
   fread(&overRide,sizeof(bool),1,fp_);
   fread(&plot->dataMapping,sizeof(int),1,fp_);

   plot->setMinVal(minV);
   plot->setMaxVal(maxV);
	plot->setOverRideAutoRange(overRide);

}


void PlotFile2D::load_legacy_V3_7_delta(Plot2D *plot)
{
// Load daxes ppm flags information

   bool xppm,yppm;
   fread(&xppm,sizeof(bool),1,fp_);
   fread(&yppm,sizeof(bool),1,fp_);
 
   plot->curXAxis()->setPPMScale(xppm);
   plot->curYAxis()->setPPMScale(yppm);

}

void PlotFile2D::load_legacy_data(Plot2D* plot)
{
// Read in matrix x size
	long xSize = 0;
   fread(&xSize,sizeof(long),1,fp_);
   if(xSize < 0)
   {
		throw PlotFileException("Matrix must have positive x size");
   }

// Read in matrix y size
	long ySize = 0;
   fread(&ySize,sizeof(long),1,fp_);
   if(ySize < 0)
   {
	   throw PlotFileException("Matrix must have positive y size");
	}

   plot->setMatWidth(xSize);
   plot->setMatHeight(ySize);
		
// Read in matrix number type and matrix
	ReadData(plot);

// Initialize region displayed       
   if(plot->visibleWidth() == -1)
   {
      plot->selectRect.left = 0;
      plot->selectRect.top = 0;
      plot->selectRect.right = plot->matWidth();
      plot->selectRect.bottom = plot->matHeight(); 

		plot->resetDataView();
   }
     
// Initialize axis range if not loaded
	if(plot->curXAxis()->length() == plot->curXAxis()->base())
   {
		plot->curXAxis()->initialiseRange();
      plot->curYAxis()->initialiseRange();
	}

// Initialise region history list
	plot->resetZoomCount();			
}


void PlotFile2D::ReadData(Plot2D* plot)
{
	long type = 0;
   fread(&type,sizeof(long),1,fp_);
	plot->setMat(NULL);
	plot->setCMat(NULL);
	plot->setVX(NULL);
	plot->setVY(NULL);
	switch(type)
	{
		case 'REAL':
			ReadReal(plot);
			break;
		case 'COMP':
			ReadComplex(plot);
			break;
		case 'VEC2':
			ReadVec2(plot);
			break;
		case 'NONE':
			break;
		default:
			throw PlotFileException("Invalid/unknown 2D data type.");
   }
}

void PlotFile2D::ReadReal(Plot2D* plot)
{
	plot->setMat(MakeMatrix2D(plot->matWidth(),plot->matHeight()));
	long nrPoints = plot->matWidth() * plot->matHeight();
	if(fread(plot->mat()[0],sizeof(float),nrPoints,fp_) != nrPoints)
	{
		throw PlotFileException("Unable to read real matrix data from plot file.");
	}
}

void PlotFile2D::ReadComplex(Plot2D* plot)
{
   plot->setCMat(MakeCMatrix2D(plot->matWidth(),plot->matHeight()));
	long nrPoints = plot->matWidth() * plot->matHeight();
	if(fread(plot->cmat()[0],sizeof(complex),nrPoints,fp_) != nrPoints)
	{
      throw PlotFileException("Unable to read complex matrix data from plot file.");
   }
}

void PlotFile2D::ReadVec2(Plot2D* plot)
{
	long xSize = plot->matWidth();
	long ySize = plot->matHeight();

   plot->setVX(MakeMatrix2D(xSize,ySize));
   plot->setVY(MakeMatrix2D(xSize,ySize));
   long nrPoints = xSize*ySize;
	if(fread(plot->vx()[0],sizeof(float),nrPoints,fp_) != nrPoints)
   {
      throw PlotFileException("Unable to read x matrix data from plot file.");
   }
	if(fread(plot->vy()[0],sizeof(float),nrPoints,fp_) != nrPoints)
	{
      throw PlotFileException("Unable to read y matrix data from plot file.");
   }	   	   
}


PlotFile2D_V1_0::PlotFile2D_V1_0(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V1_0::PlotFile2D_V1_0(const PlotFile2D_V1_0& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V1_0::~PlotFile2D_V1_0()
{}

PlotFile* PlotFile2D_V1_0::clone() const
{
	return new PlotFile2D_V1_0(*this);
}

Plot* PlotFile2D_V1_0::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
	load_legacy_V1_0(plot);
	load_legacy_data(plot);
	return plot;
}


PlotFile2D_V1_6::PlotFile2D_V1_6(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V1_6::PlotFile2D_V1_6(const PlotFile2D_V1_6& copyMe)
: PlotFile2D(copyMe)
{}
		
PlotFile2D_V1_6::~PlotFile2D_V1_6()
{}
			
PlotFile* PlotFile2D_V1_6::clone() const
{
	return new PlotFile2D_V1_6(*this);
}

Plot* PlotFile2D_V1_6::load()
{
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_data(plot);
	return plot;
}



PlotFile2D_V2_0::PlotFile2D_V2_0(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V2_0::PlotFile2D_V2_0(const PlotFile2D_V2_0& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V2_0::~PlotFile2D_V2_0()
{}

PlotFile* PlotFile2D_V2_0::clone() const
{
	return new PlotFile2D_V2_0(*this);
}

Plot* PlotFile2D_V2_0::load()
{
// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_data(plot);
	return plot;
}

   	
PlotFile2D_V2_1::PlotFile2D_V2_1(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V2_1::PlotFile2D_V2_1(const PlotFile2D_V2_1& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V2_1::~PlotFile2D_V2_1()
{}

PlotFile* PlotFile2D_V2_1::clone() const
{
	return new PlotFile2D_V2_1(*this);
}

Plot* PlotFile2D_V2_1::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_V2_1_delta(plot);
	load_legacy_data(plot);
	return plot;
}

PlotFile2D_V3_0::PlotFile2D_V3_0(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V3_0::PlotFile2D_V3_0(const PlotFile2D_V3_0& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V3_0::~PlotFile2D_V3_0()
{}

PlotFile* PlotFile2D_V3_0::clone() const
{
	return new PlotFile2D_V3_0(*this);
}

Plot* PlotFile2D_V3_0::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_V2_1_delta(plot);
	load_legacy_V3_0_delta(plot);
	load_legacy_data(plot);

	return plot;
}

PlotFile2D_V3_2::PlotFile2D_V3_2(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V3_2::PlotFile2D_V3_2(const PlotFile2D_V3_2& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V3_2::~PlotFile2D_V3_2()
{}

PlotFile* PlotFile2D_V3_2::clone() const
{
	return new PlotFile2D_V3_2(*this);
}

Plot* PlotFile2D_V3_2::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_V2_1_delta(plot);
	load_legacy_V3_0_delta(plot);
	load_legacy_V3_2_delta(plot);
	load_legacy_data(plot);

	return plot;
}


PlotFile2D_V3_3::PlotFile2D_V3_3(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V3_3::PlotFile2D_V3_3(const PlotFile2D_V3_3& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V3_3::~PlotFile2D_V3_3()
{}

PlotFile* PlotFile2D_V3_3::clone() const
{
	return new PlotFile2D_V3_3(*this);
}


Plot* PlotFile2D_V3_3::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_V2_1_delta(plot);
	load_legacy_V3_0_delta(plot);
	load_legacy_V3_2_delta(plot);
	load_legacy_V3_3_delta(plot);
	load_legacy_data(plot);

	return plot;
}


PlotFile2D_V3_4::PlotFile2D_V3_4(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V3_4::PlotFile2D_V3_4(const PlotFile2D_V3_4& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V3_4::~PlotFile2D_V3_4()
{}

PlotFile* PlotFile2D_V3_4::clone() const
{
	return new PlotFile2D_V3_4(*this);
}


Plot* PlotFile2D_V3_4::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_V2_1_delta(plot);
	load_legacy_V3_0_delta(plot);
	load_legacy_V3_2_delta(plot);
	load_legacy_V3_3_delta(plot);
	load_legacy_V3_4_delta(plot);
	load_legacy_data(plot);

	return plot;
}


PlotFile2D_V3_5::PlotFile2D_V3_5(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V3_5::PlotFile2D_V3_5(const PlotFile2D_V3_5& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V3_5::~PlotFile2D_V3_5()
{}

PlotFile* PlotFile2D_V3_5::clone() const
{
	return new PlotFile2D_V3_5(*this);
}


Plot* PlotFile2D_V3_5::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_V2_1_delta(plot);
	load_legacy_V3_0_delta(plot);
	load_legacy_V3_2_delta(plot);
	load_legacy_V3_3_delta(plot);
	load_legacy_V3_4_delta(plot);
	load_legacy_V3_5_delta(plot);
	load_legacy_data(plot);

	return plot;
}


PlotFile2D_V3_6::PlotFile2D_V3_6(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V3_6::PlotFile2D_V3_6(const PlotFile2D_V3_6& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V3_6::~PlotFile2D_V3_6()
{}

PlotFile* PlotFile2D_V3_6::clone() const
{
	return new PlotFile2D_V3_6(*this);
}


Plot* PlotFile2D_V3_6::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_V2_1_delta(plot);
	load_legacy_V3_0_delta(plot);
	load_legacy_V3_2_delta(plot);
	load_legacy_V3_3_delta(plot);
	load_legacy_V3_4_delta(plot);
	load_legacy_V3_5_delta(plot);
	load_legacy_V3_6_delta(plot);
	load_legacy_data(plot);

	return plot;
}


PlotFile2D_V3_7::PlotFile2D_V3_7(int version, FILE* fp)
: PlotFile2D(version,fp)
{}

PlotFile2D_V3_7::PlotFile2D_V3_7(const PlotFile2D_V3_7& copyMe)
: PlotFile2D(copyMe)
{}

PlotFile2D_V3_7::~PlotFile2D_V3_7()
{}

PlotFile* PlotFile2D_V3_7::clone() const
{
	return new PlotFile2D_V3_7(*this);
}


Plot* PlotFile2D_V3_7::load()
{
	// TODO Need to set hwnd, pp, row, col.
	Plot2D* plot = new Plot2D(pwd,0,0,0,0,defaultMargins2D);
   plot->setFileVersion(version_);
   load_legacy_V1_0(plot);
	load_legacy_V1_6_delta(plot);
	load_legacy_V2_0_delta(plot);
	load_legacy_V2_1_delta(plot);
	load_legacy_V3_0_delta(plot);
	load_legacy_V3_2_delta(plot);
	load_legacy_V3_3_delta(plot);
	load_legacy_V3_4_delta(plot);
	load_legacy_V3_5_delta(plot);
	load_legacy_V3_6_delta(plot);
	load_legacy_V3_7_delta(plot);
	load_legacy_data(plot);

	return plot;
}

