#include "stdafx.h"
#include "AxisFile.h"
#include "Axis.h"
#include "PlotFile.h"
#include "PlotGrid.h"
#include "PlotGridFile.h"
#include "PlotLabel.h"
#include "PlotLabelFile.h"
#include "TicksFile.h"
#include "memoryLeak.h"

using namespace std;


class AxisFile_V3_0 : public AxisFile
{
public:
	AxisFile_V3_0(int version, FILE* fp);
	Axis* load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent);
	AxisFile_V3_0(const AxisFile_V3_0& copyMe);
	AxisFile* clone() const;
	virtual ~AxisFile_V3_0();

protected:
};

class AxisFile_V3_2 : public AxisFile
{
public:
	AxisFile_V3_2(int version, FILE* fp);
	Axis* load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent);
	AxisFile_V3_2(const AxisFile_V3_2& copyMe);
	AxisFile* clone() const;
	virtual ~AxisFile_V3_2();

protected:
};

class AxisFile_V3_3 : public AxisFile
{
public:
	AxisFile_V3_3(int version, FILE* fp);
	Axis* load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent);
	AxisFile_V3_3(const AxisFile_V3_3& copyMe);
	AxisFile* clone() const;
	virtual ~AxisFile_V3_3();

protected:
};


class AxisFile_V3_4 : public AxisFile
{
public:
	AxisFile_V3_4(int version, FILE* fp);
	Axis* load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent);
	AxisFile_V3_4(const AxisFile_V3_4& copyMe);
	AxisFile* clone() const;
	virtual ~AxisFile_V3_4();

protected:
};


AxisFile* AxisFile::makeLoader(int version, FILE* fp)
{
	switch(version)
	{
	   case PLOTFILE_VERSION_3_0:
	   case PLOTFILE_VERSION_3_1:
		   return new AxisFile_V3_0(version, fp);
	   case PLOTFILE_VERSION_1_0:
	   case PLOTFILE_VERSION_1_1:
	   case PLOTFILE_VERSION_1_2:
	   case PLOTFILE_VERSION_1_4:
	   case PLOTFILE_VERSION_1_6:
	   case PLOTFILE_VERSION_2_0:
	   case PLOTFILE_VERSION_2_1:
	   case PLOTFILE_VERSION_2_2:
		   return 0;
	   case PLOTFILE_VERSION_3_2:
	   case PLOTFILE_VERSION_3_3:
	   case PLOTFILE_VERSION_3_4:
	   case PLOTFILE_VERSION_3_5:
         return new AxisFile_V3_2(version, fp);
	   case PLOTFILE_VERSION_3_6:
	      return new AxisFile_V3_3(version, fp);
      case PLOTFILE_VERSION_3_7:
		   return new AxisFile_V3_4(version, fp);
	   default:
		   return new AxisFile_V3_4(version, fp);
	}
}

AxisFile::AxisFile(const AxisFile& copyMe)
{
	version_ = copyMe.version_;
	fp_ = copyMe.fp_;
	ticksFile_ = copyMe.ticksFile_->clone();
	plotGridFile_ = copyMe.plotGridFile_->clone();
	plotLabelFile_ = copyMe.plotLabelFile_->clone();
}

AxisFile::~AxisFile()
{
	if (ticksFile_)
	{
		delete ticksFile_;	
	}
	if (plotGridFile_)
	{
		delete plotGridFile_;
	}
	if (plotLabelFile_)
	{
		delete plotLabelFile_;
	}
}

AxisFile::AxisFile(int version, FILE* fp)
{
	version_ = version;
	fp_ = fp;
	ticksFile_ = TicksFile::makeLoader(version,fp);
	plotGridFile_ = PlotGridFile::makeLoader(version,fp);
	plotLabelFile_ = PlotLabelFile::makeLoader(version,fp);
}

////////////////

AxisFile_V3_0::AxisFile_V3_0(int version, FILE* fp)
: AxisFile(version,fp)
{}

AxisFile_V3_0::AxisFile_V3_0(const AxisFile_V3_0& copyMe)
: AxisFile(copyMe)
{}
AxisFile* AxisFile_V3_0::clone() const
{
	return new AxisFile_V3_0(*this);
}

AxisFile_V3_0::~AxisFile_V3_0()
{}

Axis* AxisFile_V3_0::load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent)
{
	float min, max;
	fread(&min, sizeof(float), 1, fp_);
	fread(&max, sizeof(float), 1, fp_);
	short mapping;
	fread(&mapping, sizeof(short), 1, fp_);
	
	Axis* axis = Axis::makeAxis(orientation, mapping,parent,side);
	axis->setMin(min);
	axis->setMax(max);
	
	bool autoRange;
	fread(&autoRange, sizeof(bool), 1, fp_);
	axis->setAutorange(autoRange);

	Ticks* ticks = ticksFile_->load();
	axis->setTicks(*ticks);
	delete ticks;

	PlotGrid* plotGrid = plotGridFile_->load(orientation);
	plotGrid->setAxis(axis);
	axis->setGrid(plotGrid);
	
	PlotLabel* label;
	if (orientation == HORIZONTAL_AXIS)
	{
		label = plotLabelFile_->load(HORIZONTAL_AXIS_PLOT_LABEL);
	}
	else
	{
		label = plotLabelFile_->load(VERTICAL_AXIS_PLOT_LABEL);
	}
	axis->setLabel(label);
	static_cast<HorizontalAxisLabel*>(label)->setParent(axis);

	return axis;
}


AxisFile_V3_2::AxisFile_V3_2(int version, FILE* fp)
: AxisFile(version,fp)
{}

AxisFile_V3_2::AxisFile_V3_2(const AxisFile_V3_2& copyMe)
: AxisFile(copyMe)
{}

AxisFile* AxisFile_V3_2::clone() const
{
	return new AxisFile_V3_2(*this);
}

AxisFile_V3_2::~AxisFile_V3_2()
{}

Axis* AxisFile_V3_2::load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent)
{
	float min, max;
	fread(&min, sizeof(float), 1, fp_);
	fread(&max, sizeof(float), 1, fp_);
	short mapping;
	fread(&mapping, sizeof(short), 1, fp_);
	
	Axis* axis = Axis::makeAxis(orientation, mapping,parent,side);
	axis->setMin(min);
	axis->setMax(max);

	bool autoRange;
	fread(&autoRange, sizeof(bool), 1, fp_);
	axis->setAutorange(autoRange);

   PlotDirection direction;
	fread(&direction, sizeof(PlotDirection), 1, fp_);
   axis->setDirection(direction);

	Ticks* ticks = ticksFile_->load();
	axis->setTicks(*ticks);
	delete ticks;

	PlotGrid* plotGrid = plotGridFile_->load(orientation);
	plotGrid->setAxis(axis);
	axis->setGrid(plotGrid);
	
	PlotLabel* label;
	if (orientation == HORIZONTAL_AXIS)
	{
		label = plotLabelFile_->load(HORIZONTAL_AXIS_PLOT_LABEL);
	}
	else
	{
		label = plotLabelFile_->load(VERTICAL_AXIS_PLOT_LABEL);
	}
	axis->setLabel(label);
	static_cast<HorizontalAxisLabel*>(label)->setParent(axis);

	return axis;
}



AxisFile_V3_3::AxisFile_V3_3(int version, FILE* fp)
: AxisFile(version,fp)
{}

AxisFile_V3_3::AxisFile_V3_3(const AxisFile_V3_3& copyMe)
: AxisFile(copyMe)
{}

AxisFile* AxisFile_V3_3::clone() const
{
	return new AxisFile_V3_3(*this);
}

AxisFile_V3_3::~AxisFile_V3_3()
{}

Axis* AxisFile_V3_3::load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent)
{
	float min, max;
	float minInd, maxInd;
	fread(&min, sizeof(float), 1, fp_);
	fread(&max, sizeof(float), 1, fp_);
	fread(&minInd, sizeof(float), 1, fp_);
	fread(&maxInd, sizeof(float), 1, fp_);
	short mapping;
	fread(&mapping, sizeof(short), 1, fp_);
	
	Axis* axis = Axis::makeAxis(orientation, mapping,parent,side);
	axis->setMin(min);
	axis->setMax(max);

   axis->setMinIndepOrig(minInd);
   axis->setMinIndep(minInd);
	axis->setMaxIndep(maxInd);

	bool autoRange;
	fread(&autoRange, sizeof(bool), 1, fp_);
	axis->setAutorange(autoRange);

   PlotDirection direction;
	fread(&direction, sizeof(PlotDirection), 1, fp_);
   axis->setDirection(direction);

	Ticks* ticks = ticksFile_->load();
	axis->setTicks(*ticks);
	delete ticks;

	PlotGrid* plotGrid = plotGridFile_->load(orientation);
	plotGrid->setAxis(axis);
	axis->setGrid(plotGrid);
	
	PlotLabel* label;
	if (orientation == HORIZONTAL_AXIS)
	{
		label = plotLabelFile_->load(HORIZONTAL_AXIS_PLOT_LABEL);
	}
	else
	{
		label = plotLabelFile_->load(VERTICAL_AXIS_PLOT_LABEL);
	}
	axis->setLabel(label);
	static_cast<HorizontalAxisLabel*>(label)->setParent(axis);

	return axis;
}


AxisFile_V3_4::AxisFile_V3_4(int version, FILE* fp)
: AxisFile(version,fp)
{}

AxisFile_V3_4::AxisFile_V3_4(const AxisFile_V3_4& copyMe)
: AxisFile(copyMe)
{}

AxisFile* AxisFile_V3_4::clone() const
{
	return new AxisFile_V3_4(*this);
}

AxisFile_V3_4::~AxisFile_V3_4()
{}

Axis* AxisFile_V3_4::load(AxisOrientation orientation, VerticalAxisSide side, Plot* parent)
{
	float min, max;
	float minInd, maxInd;
	fread(&min, sizeof(float), 1, fp_);
	fread(&max, sizeof(float), 1, fp_);
	fread(&minInd, sizeof(float), 1, fp_);
	fread(&maxInd, sizeof(float), 1, fp_);
	short mapping;
	fread(&mapping, sizeof(short), 1, fp_);
	
	Axis* axis = Axis::makeAxis(orientation, mapping,parent,side);
	axis->setMin(min);
	axis->setMax(max);

   axis->setMinIndepOrig(minInd);
   axis->setMinIndep(minInd);
	axis->setMaxIndep(maxInd);

	bool autoRange;
	fread(&autoRange, sizeof(bool), 1, fp_);
	axis->setAutorange(autoRange);

	bool ppm;
	fread(&ppm, sizeof(bool), 1, fp_);
   axis->setPPMScale(ppm);

   PlotDirection direction;
	fread(&direction, sizeof(PlotDirection), 1, fp_);
   axis->setDirection(direction);

	Ticks* ticks = ticksFile_->load();
	axis->setTicks(*ticks);
	delete ticks;

	PlotGrid* plotGrid = plotGridFile_->load(orientation);
	plotGrid->setAxis(axis);
	axis->setGrid(plotGrid);
	
	PlotLabel* label;
	if (orientation == HORIZONTAL_AXIS)
	{
		label = plotLabelFile_->load(HORIZONTAL_AXIS_PLOT_LABEL);
	}
	else
	{
		label = plotLabelFile_->load(VERTICAL_AXIS_PLOT_LABEL);
	}
	axis->setLabel(label);
	static_cast<HorizontalAxisLabel*>(label)->setParent(axis);

	return axis;
}

