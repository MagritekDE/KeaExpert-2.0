#include "stdafx.h"
#include "PlotLabelFile.h"
#include "PlotFile.h"
#include "PlotLabel.h"
#include "memoryLeak.h"


class PlotLabelFile_V3_0 : public PlotLabelFile
{
public:
	PlotLabelFile_V3_0(int version, FILE* fp);
	PlotLabel* load(PlotLabelType type);
	PlotLabelFile_V3_0(const PlotLabelFile_V3_0& copyMe);
	PlotLabelFile* clone() const;
	virtual ~PlotLabelFile_V3_0();

protected:
};


PlotLabelFile* PlotLabelFile::makeLoader(int version, FILE* fp)
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
	case PLOTFILE_VERSION_3_3:
	default:
		return new PlotLabelFile_V3_0(version, fp);
	}
}

PlotLabelFile::PlotLabelFile(const PlotLabelFile& copyMe)
{
	version_ = copyMe.version_;
	fp_ = copyMe.fp_;
}

PlotLabelFile::~PlotLabelFile()
{}

PlotLabelFile::PlotLabelFile(int version, FILE* fp)
{
	version_ = version;
	fp_ = fp;
}

////////////////

PlotLabelFile_V3_0::PlotLabelFile_V3_0(int version, FILE* fp)
: PlotLabelFile(version,fp)
{}

PlotLabelFile_V3_0::PlotLabelFile_V3_0(const PlotLabelFile_V3_0& copyMe)
: PlotLabelFile(copyMe)
{}

PlotLabelFile* PlotLabelFile_V3_0::clone() const
{
	return new PlotLabelFile_V3_0(*this);
}

PlotLabelFile_V3_0::~PlotLabelFile_V3_0()
{}

PlotLabel* PlotLabelFile_V3_0::load(PlotLabelType type)
{
	char buf[PLOT_STR_LEN];
	fread(buf, sizeof(char), PLOT_STR_LEN, fp_);
	LOGFONT font;
	fread(&font, sizeof(LOGFONT), 1, fp_);
	COLORREF col;
	fread(&col, sizeof(COLORREF), 1, fp_);
	
	ProspaFont pf(font);
	pf.setColor(col);

	PlotLabel* returnMe = PlotLabel::makePlotLabel(type,pf,buf);
	if (strlen(buf) > 0)
	{
		returnMe->userHasSetText(true);
	}
	return returnMe;
}
