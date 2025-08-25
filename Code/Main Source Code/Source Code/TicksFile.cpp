#include "stdafx.h"
#include "TicksFile.h"
#include "PlotFile.h"
#include "Ticks.h"
#include "memoryLeak.h"

class TicksFile_V3_0 : public TicksFile
{
public:
	TicksFile_V3_0(int version, FILE* fp);
	Ticks* load();
	TicksFile_V3_0(const TicksFile_V3_0& copyMe);
	TicksFile* clone() const;
	virtual ~TicksFile_V3_0();
};


TicksFile* TicksFile::makeLoader(int version, FILE* fp)
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
		throw PlotFileException("Invalid ticks version specified in plot file.");	
	case PLOTFILE_VERSION_3_0:
	case PLOTFILE_VERSION_3_1:
	case PLOTFILE_VERSION_3_2:
	case PLOTFILE_VERSION_3_3:
	default:
		return new TicksFile_V3_0(version, fp);
	}
}

TicksFile::TicksFile(const TicksFile& copyMe)
{
	version_ = copyMe.version_;
	fp_ = copyMe.fp_;
}

TicksFile::~TicksFile()
{}

TicksFile::TicksFile(int version, FILE* fp)
{
	version_ = version;
	fp_ = fp;
}

////////////////

TicksFile_V3_0::TicksFile_V3_0(int version, FILE* fp)
: TicksFile(version,fp)
{}

TicksFile_V3_0::TicksFile_V3_0(const TicksFile_V3_0& copyMe)
: TicksFile(copyMe)
{}

TicksFile* TicksFile_V3_0::clone() const
{
	return new TicksFile_V3_0(*this);
}

TicksFile_V3_0::~TicksFile_V3_0()
{}

Ticks* TicksFile_V3_0::load()
{
	LOGFONT f;
	COLORREF c;
	float majorLength, minorLength, spacing, perLabel;
	fread(&majorLength, sizeof(float), 1, fp_);
	fread(&minorLength, sizeof(float), 1, fp_);
	fread(&spacing, sizeof(float), 1, fp_);
	fread(&perLabel, sizeof(float), 1, fp_);
	fread(&f, sizeof(LOGFONT), 1, fp_);
	fread(&c, sizeof(COLORREF), 1, fp_);

	ProspaFont pf(f);
	return new Ticks(majorLength, minorLength, spacing, perLabel, &pf, c);
}
