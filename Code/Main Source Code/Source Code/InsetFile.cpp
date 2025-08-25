#include "stdafx.h"
#include "InsetFile.h"
#include "PlotFile.h"
#include "Inset.h"
#include <string>
#include "memoryLeak.h"

using namespace std;

class InsetFile_V3_3 : public InsetFile
{
public:
	InsetFile_V3_3(int version, FILE* fp);
	Inset* load();
	InsetFile_V3_3(const InsetFile_V3_3& copyMe);
	InsetFile* clone() const;
	virtual ~InsetFile_V3_3();
};


InsetFile* InsetFile::makeLoader(int version, FILE* fp)
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
	case PLOTFILE_VERSION_3_0:
	case PLOTFILE_VERSION_3_1:
	case PLOTFILE_VERSION_3_2:
		return 0;
	case PLOTFILE_VERSION_3_3:
	default:
		return new InsetFile_V3_3(version,fp);
	}
}

InsetFile::InsetFile(const InsetFile& copyMe)
{
	version_ = copyMe.version_;
	fp_ = copyMe.fp_;
}

InsetFile::~InsetFile()
{}

InsetFile::InsetFile(int version, FILE* fp)
{
	version_ = version;
	fp_ = fp;
}

////////////////

InsetFile_V3_3::InsetFile_V3_3(int version, FILE* fp)
: InsetFile(version,fp)
{}

InsetFile_V3_3::InsetFile_V3_3(const InsetFile_V3_3& copyMe)
: InsetFile(copyMe)
{}

InsetFile* InsetFile_V3_3::clone() const
{
	return new InsetFile_V3_3(*this);
}

InsetFile_V3_3::~InsetFile_V3_3()
{}

Inset* InsetFile_V3_3::load()
{
	InsetType t;
	fread(&t, sizeof(InsetType),1,fp_);
	bool visible = false;
	fread(&visible, sizeof(bool),1,fp_);
	int relative_x = 0;
	int relative_y = 0;
	fread(&relative_x, sizeof(int), 1, fp_);
	fread(&relative_y, sizeof(int), 1, fp_);
	InsetCorner corner;
	fread(&corner, sizeof(InsetCorner),1,fp_);
	int user_width = 0;
	fread(&user_width,sizeof(int),1,fp_);
	int description_length = 0;
	fread(&description_length,sizeof(int),1,fp_);
	char* description = new char[description_length+1];
	fread(description,sizeof(char),description_length,fp_);
	description[description_length] = '\0';
	Inset* inset = Inset::makeInset(t,string(description),user_width);
	delete[] description;
	
	inset->setCorner(corner);
	inset->setRelativeX(relative_x);
	inset->setRelativeY(relative_y);
	inset->setVisible(visible);
	
	return inset;
}
