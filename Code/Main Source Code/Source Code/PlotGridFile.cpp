#include "stdafx.h"
#include "PlotGridFile.h"
#include "Axis.h"
#include "PlotFile.h"
#include "PlotGrid.h"
#include "memoryLeak.h"


class PlotGridFile_V3_0 : public PlotGridFile
{
public:
	PlotGridFile_V3_0(int version, FILE* fp);
	PlotGrid* load(AxisOrientation orientation);
	PlotGridFile_V3_0(const PlotGridFile_V3_0& copyMe);
	PlotGridFile* clone() const;
	virtual ~PlotGridFile_V3_0();

protected:
};

PlotGridFile* PlotGridFile::makeLoader(int version, FILE* fp)
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
		throw PlotFileException("Invalid plot grid version specified in plot file.");	
	case PLOTFILE_VERSION_3_0:
	case PLOTFILE_VERSION_3_1:
	case PLOTFILE_VERSION_3_2:
	case PLOTFILE_VERSION_3_3:
	default:
		return new PlotGridFile_V3_0(version, fp);
	}
}

PlotGridFile::PlotGridFile(const PlotGridFile& copyMe)
{
	version_ = copyMe.version_;
	fp_ = copyMe.fp_;
}

PlotGridFile::~PlotGridFile()
{}

PlotGridFile::PlotGridFile(int version, FILE* fp)
{
	version_ = version;
	fp_ = fp;
}

////////////////

PlotGridFile_V3_0::PlotGridFile_V3_0(int version, FILE* fp)
: PlotGridFile(version,fp)
{}

PlotGridFile_V3_0::PlotGridFile_V3_0(const PlotGridFile_V3_0& copyMe)
: PlotGridFile(copyMe)
{}

PlotGridFile* PlotGridFile_V3_0::clone() const
{
	return new PlotGridFile_V3_0(*this);
}

PlotGridFile_V3_0::~PlotGridFile_V3_0()
{}

PlotGrid* PlotGridFile_V3_0::load(AxisOrientation orientation)
{
	bool drawGrid, drawFineGrid;
	COLORREF color, fineColor;
	fread(&drawGrid, sizeof(bool), 1, fp_);
	fread(&drawFineGrid, sizeof(bool), 1, fp_);
	fread(&color, sizeof(COLORREF), 1, fp_);
	fread(&fineColor, sizeof(COLORREF), 1, fp_);
	return PlotGrid::makePlotGrid(orientation,drawGrid,drawFineGrid,color,fineColor);
}
