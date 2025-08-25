#include "stdafx.h"
#include "PlotLegendFile.h"
#include "Inset.h"
#include "PlotFile.h"
#include "memoryLeak.h"

PlotLegendFile* PlotLegendFile::makeLoader(int version, FILE* fp)
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
		return 0;	
	case PLOTFILE_VERSION_3_0:
	case PLOTFILE_VERSION_3_1:
	case PLOTFILE_VERSION_3_2:
	default:
		return new PlotLegendFile_V3_0(version, fp);
	}
}

PlotLegendFile::PlotLegendFile(int version, FILE* fp)
{
	version_ = version;
	fp_ = fp;
}

PlotLegendFile::PlotLegendFile(const PlotLegendFile& copyMe)
{
	version_ = copyMe.version_;
	fp_ = copyMe.fp_;
}

PlotLegendFile::~PlotLegendFile()
{}

PlotLegendFile_V3_0::PlotLegendFile_V3_0(int version, FILE* fp)
: PlotLegendFile(version,fp)
{}

PlotLegendFile_V3_0::PlotLegendFile_V3_0(const PlotLegendFile_V3_0& copyMe)
: PlotLegendFile(copyMe)
{}

PlotLegendFile* PlotLegendFile_V3_0::clone() const
{
	return new PlotLegendFile_V3_0(*this);
}

PlotLegendFile_V3_0::~PlotLegendFile_V3_0()
{}

PlotLegend* PlotLegendFile_V3_0::load()
{
	PlotLegend* legend = new PlotLegend();
	bool visible;
	fread(&visible, sizeof(bool),1, fp_);
	legend->setVisible(visible);
	legend->setVisibleLocked(visible);

	int legend_relative_x;
	fread(&legend_relative_x, sizeof(int), 1, fp_);
	legend->setRelativeX(legend_relative_x);

	int legend_relative_y;
	fread(&legend_relative_y, sizeof(int), 1, fp_);
	legend->setRelativeY(legend_relative_y);

	InsetCorner legend_corner;
	fread(&legend_corner, sizeof(InsetCorner), 1, fp_);
	legend->setCorner(legend_corner);

	return legend;
}
