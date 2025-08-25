#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h>
#include "stdafx.h"
#include "Inset.h"
#include "cArg.h"
#include "drawing.h"
#include "error.h"
#include "evaluate_simple.h"
#include "globals.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "Interface.h"
#include "mymath.h"
#include "Plot.h"
#include "PlotFile.h"
#include "Plot1dcli.h"
#include "PlotWindow.h"
#include "print.h"
#include "rgbColors.h"
#include "ShortRect.h"
#include "string_utilities.h"
#include "trace.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include <boost\tokenizer.hpp>
#include <string>
//#include "memoryLeak.h"

using std::string;
using namespace Gdiplus;

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
boost::char_separator<char> sep(" \n");

Inset::Inset(string& description, int width /* = 0*/, int left /*= 0*/, int top /*= 0*/)
: font_(DEFAULT_FONT_NAME, RGB_BLACK, 10, 0)
{		
	plot_ = 0;
	backgroundColor_ = RGB_WHITE; 
	corner_ = TOP_LEFT;
	required_height_ = 0;
	required_width_ = 0;
	visible_ = true;
	visibleLocked_ = false;
	relativePosition_.x = left? left : INSET_PADDING_LEFT;
	relativePosition_.y = top? top: INSET_PADDING_TOP;
	position_.set(relativePosition_.x,relativePosition_.y,0,0);
	beingMoved_ = false;
	user_specified_width_ = width;
	userScaling_ = 1;
	description_ = description;
	layoutHasChanged();
}

Inset::Inset(const Inset& copyMe)
: font_(copyMe.font_)
{
	corner_ = copyMe.corner_;
	position_ = copyMe.position_;
	backgroundColor_ = copyMe.backgroundColor_;
	visible_ = copyMe.visible_;
	visibleLocked_ = copyMe.visibleLocked_;
	relativePosition_.x = copyMe.relativePosition_.x;
	relativePosition_.y = copyMe.relativePosition_.y;
	required_width_ = copyMe.required_width_;
	required_height_ = copyMe.required_height_;
	beingMoved_ = false;
	user_specified_width_ = copyMe.user_specified_width_;
	description_ = copyMe.description_;
	layoutHasChanged();
}

Inset::~Inset()
{
}

void Inset::draw(HDC hdc)
{
	if (!visible())
	{
		return;
	}
	// Setup HDCs
	SaveDC(hdc);

	// Set up and select drawing objects
	selectDrawingObjects(hdc);

	// Check for need to recalc layout; recalc if needed
	if (need_to_recalculate_layout_)
	{
		recalculateLayout(hdc);
	}
	// Check we're not too big to draw
	if (!fits_in_parent(hdc))
	{
		deleteDrawingObjects();
		RestoreDC(hdc,-1);
		return;
	}	
   if (need_to_recalculate_layout_)
	   resize();
	// draw background and border
	drawBackground(hdc);
	drawBorder(hdc);
	// draw the type-specific insides
	drawContents(hdc);
	// clean up.
	deleteDrawingObjects();
	RestoreDC(hdc,-1);
	need_to_recalculate_layout_ = false;
}

void Inset::scale_(double x, double y)
{
	if (!visible_)
	{
		return;
	}
	position_.scale(x,y);
	font_.scale(x,y);
	updateRelativePosition();
	layoutHasChanged();
}

void Inset::unscale_()
{
	if (!visible_)
	{
		return;
	}
	position_.unscale();
	font_.unscale();
	updateRelativePosition();
	layoutHasChanged();
}

short Inset::left_inner_padding()
{
	return INSET_PADDING_LEFT * x_scaling_factor();
}

short Inset::left_outer_padding() const
{
	return INSET_PADDING_LEFT * x_scaling_factor();
}

short Inset::right_padding() const
{
	return INSET_PADDING_RIGHT * x_scaling_factor();
}

short Inset::top_padding() const
{
	return INSET_PADDING_TOP * y_scaling_factor();
}

short Inset::bottom_padding() const
{
	return INSET_PADDING_BOTTOM * y_scaling_factor();
}

void Inset::resize()
{	
	POINT corner = cornerPoint();

	position().setx0(corner.x + relativeX());
	position().sety0(corner.y + relativeY());

	int width = required_width_;
	int height = required_height_;
	if (user_specified_width_)
	{
		width *= userScaling_;
		height *= userScaling_;
	}

	position().setx1(width + position().getx0());
	position().sety1(height + position().gety0());

	// Did we just "grow" too far to the left?
	if (position().getx1() > plot_->getDimensions().right())
	{
		short howFarTooFar = position().getx1() - plot_->getDimensions().right() + 1;
		position().setx0(position().getx0() - howFarTooFar); 
		position().setx1(plot_->getDimensions().right() - 1); 
	}
	// .. or "grow" over the bottom edge?
	if (position().gety1() > plot_->getDimensions().bottom())
	{
		short howFarTooFar = position().gety1() - plot_->getDimensions().bottom() + 1;
		position().sety0(position().gety0() - howFarTooFar); 
		position().sety1(plot_->getDimensions().bottom() - 1); 
	}
	updateRelativePosition();
}

void Inset::drawBorder(HDC hdc)
{
	SaveDC(hdc);
	HPEN borderPen = CreatePen(PS_SOLID	,1,RGB_BLACK);
	HPEN oldpen = (HPEN)SelectObject(hdc,borderPen);

	MoveToEx(hdc, position().getx0(), position().gety0(),0);
	LineTo(hdc, position().getx1(), position().gety0());
	LineTo(hdc, position().getx1(), position().gety1());
	LineTo(hdc, position().getx0(), position().gety1());
	LineTo(hdc, position().getx0(), position().gety0());

	SelectObject(hdc, oldpen); 
	DeleteObject(borderPen);
	RestoreDC(hdc,-1);
}

InsetCorner Inset::updateClosestPlotCorner()
{
	int plotHorizontalMidpoint = plot_->getDimensions().left() + (plot_->getDimensions().width() / 2);
	int plotVerticalMidpoint = plot_->getDimensions().top() + (plot_->getDimensions().height() / 2);

	if (position().getx0() < plotHorizontalMidpoint) 
	{
		if (position().gety0() < plotVerticalMidpoint)
		{
			setCorner(TOP_LEFT);
		}
		else 
		{
			setCorner(BOTTOM_LEFT);
		}
	}
	else if (position().gety0() < plotVerticalMidpoint)
	{
		setCorner(TOP_RIGHT);
	}
	else
	{
		setCorner(BOTTOM_RIGHT);
	}
	return corner();
}

POINT Inset::cornerPoint()
{
	POINT point = {0,0};
	switch(corner())
	{
	case BOTTOM_LEFT:
		point.x = plot_->getDimensions().left();
		point.y = plot_->getDimensions().bottom();
		break;
	case TOP_RIGHT:
		point.x = plot_->getDimensions().right();
		point.y = plot_->getDimensions().top();
		break;
	case BOTTOM_RIGHT:
		point.x = plot_->getDimensions().right();
		point.y = plot_->getDimensions().bottom();
		break;
	case TOP_LEFT:
	default:
		point.x = plot_->getDimensions().left();
		point.y = plot_->getDimensions().top();
		break;
	}
	return point;
}

void Inset::updateRelativePosition()
{
	updateClosestPlotCorner();
	POINT corner = cornerPoint();
	relativePosition_.x = position().getx0() - corner.x;
	relativePosition_.y = position().gety0() - corner.y;
}


void Inset::shift(float dx, float dy)
{
	// Did we try to go off the top edge?
	// Did we try to go off the bottom edge?
	if ( (position().gety0() + dy > plot_->getDimensions().top()) &&
		(position().gety1() + dy < plot_->getDimensions().bottom()))
	{
		position().sety0(position().gety0() + dy);
		position().sety1(position().gety1() + dy);
	}
	// Did we try to go off the left edge?
	// Did we try to go off the right edge?
	if ( (position().getx0() + dx > plot_->getDimensions().left()) &&
		(position().getx1() + dx < plot_->getDimensions().right()))
	{
		position().setx0(position().getx0() + dx);
		position().setx1(position().getx1() + dx);
	}
	updateRelativePosition();
	layoutHasChanged();
}

void Inset::drawBackground(HDC hdc)
{
	RECT r;
	HBRUSH bkBrush = plot_->GetPlotBorderBrush();
	SaveDC(hdc);
	SetRect(&r, position().getx0(), position().gety0(), position().getx1(), position().gety1());
	FillRect(hdc,&r,bkBrush);
	DeleteObject(bkBrush);
	RestoreDC(hdc, -1);
}

bool Inset::fits_in_parent(HDC hdc)
{
	return ( (required_width_ < plot_->getDimensions().width()) &&
				(required_height_ < plot_->getDimensions().height()) 
			 );
}

void Inset::recalculateLayout(HDC hdc)
{
	evaluate_required_width(hdc);
	evaluate_required_height(hdc);
}

InsetDescription* Inset::description(int index)
{
	char pos[64];
	sprintf(pos, "(%d,%d)", position_.getx0(), position_.gety0());
	return new InsetDescription(index, description_, string(pos), visible_);
}

string Inset::FormatState()
{
	StringPairs state;
	CText formatter;
	formatter.Format("(%d,%d)",plot_->plotParent->obj->winParent->nr,plot_->plotParent->obj->nr());
	state.add("parent",formatter.Str());
	state.add("contents",description_.c_str());
	formatter.Format("(%d,%d)",position_.getx0(), position_.gety0());
	state.add("position",formatter.Str());
	state.add("visible",visible_? "true":"false");
	return FormatStates(state);
}

int Inset::ProcessInsetParameters(Interface* itfc, char args[])
{
   CText  parameter;
   CText  value;
   CArg carg;
   Variable parVar;
   Variable valueVar;

   short nrArgs = carg.Count(args);

// Prompt user if no arguments supplied
   if(nrArgs == 0)
   {
      TextMessage(formatNoArgHeader().c_str());
      TextMessage(FormatState().c_str());
      itfc->nrRetValues = 0;
      return(0);
   }
	else
	{
	// Check for valid number of arguments
	   if(nrArgs%2 != 0)
	   {
	      ErrorMessage("number of arguments must be even");
	      return(ERR);
	   }

	// Extract arguments    
		short i = 1;
	   while(i < nrArgs)
	   {
      // Extract parameter name
         parameter = carg.Extract(i);
			short type;
         if((type = Evaluate(itfc,RESPECT_ALIAS,parameter.Str(),&parVar)) < 0)
            return(ERR); 
	      
	      if(type != UNQUOTED_STRING)
	      {
	         ErrorMessage("invalid data type for parameter '%s'",parameter.Str());
	         return(ERR);
	      }  
         parameter = parVar.GetString();

      // Extract parameter value
         value = carg.Extract(i+1);
         value.RemoveQuotes();

	   // Process different kinds of parameter    

			if (!strcmp(parameter.Str(), "visible"))
			{
				if (!strcmp(value.Str(), "true"))
				{
					this->visible_ = true;
				}
				else
				{
					this->visible_ = false;
				}
			}
			else if (!strcmp(parameter.Str(), "contents"))
			{
				string result(value.Str());
				try
				{
					setContents(result);
				}
				catch(int code)
				{
					char message[256];
					sprintf (message, "GDI error %d loading file (%s)", code, result.c_str());
					ErrorMessage(message);
					return ERR;
				}
			}
			else if (!strcmp(parameter.Str(), "width"))
			{
				if((type = Evaluate(itfc,RESPECT_ALIAS,value.Str(),&valueVar)) < 0)
					return(ERR); 
				short width = 0;
				if(ConvertAnsVar(&valueVar,"width", type, width) == ERR)
               return(ERR);
				setWidth(width);
			}
			else if (!strcmp(parameter.Str(), "corner"))
			{
				short corner;
				CText cornerStr = value.Str();
				if(cornerStr == "top left" || cornerStr == "1")
					corner = 1;
				else if(cornerStr == "bottom left" || cornerStr == "2")
					corner = 2;
				else if(cornerStr == "top right" || cornerStr == "3")
					corner = 3;
				else if(cornerStr == "bottom right" || cornerStr == "4")
					corner = 4;
				else
				{
					ErrorMessage("Invalid corner (top/bottom left/right) or (1/2/3/4)");
					return(ERR);
				}
			   setCorner(InsetCorner(corner));
			}
			else if (!strcmp(parameter.Str(), "position"))
			{
				if((type = Evaluate(itfc,RESPECT_ALIAS,value.Str(),&valueVar)) < 0)
					return(ERR); 
		      if(valueVar.GetType() == MATRIX2D && valueVar.GetDimX() == 2 &&  valueVar.GetDimY() == 1)
		      {
			      short xpos = nsint(valueVar.GetMatrix2D()[0][0]);
			      short ypos = nsint(valueVar.GetMatrix2D()[0][1]);
					if(xpos < 0)
						xpos = xpos - required_width_;
					if(ypos < 0)
						ypos = ypos - required_height_;
					setRelativeX(xpos);
					setRelativeY(ypos);
               layoutHasChanged();
		      }
            else
            {
					ErrorMessage("invalid position - should be a vector");
					return ERR;
            }
         }
         i+=2;
		}
	}
	MyInvalidateRect(Plot::curPlot()->win,NULL,false);
	itfc->nrRetValues = 0;
	return OK;
}

int Inset::setWidth(int width)
{
	user_specified_width_ = width;
	layoutHasChanged();
	return OK;
}

Inset* Inset::makeInset(InsetType type,std::string& text, int width /* = 0 */, int left /* = 0 */, int top /* = 0 */)
{
	if (type == ANNOTATION)
	{
		return new Annotation(text,width,left,top);
	}
	else if (type == IMAGE)
	{
		return new ImageInset(text, width, left, top);
	}
	return 0;
}

short Inset::save(FILE* fp) const
{
	InsetType insetType = this->type();
	fwrite(&insetType,sizeof(InsetType),1,fp);
	bool show_legend = visible();
	fwrite(&show_legend,sizeof(bool),1,fp);
	int legend_relative_x = relativeX();
	fwrite(&legend_relative_x, sizeof(int), 1, fp);
	int legend_relative_y = relativeY();
	fwrite(&legend_relative_y, sizeof(int), 1, fp);
	InsetCorner legend_corner = corner();
	fwrite(&legend_corner, sizeof(InsetCorner), 1, fp);
	fwrite(&user_specified_width_, sizeof(int), 1, fp);
	int description_length = description_.length();
	fwrite(&description_length, sizeof(int), 1, fp);
	fwrite(description_.c_str(), sizeof(char), description_length, fp);
	return OK;
}

////////////////////////////////////////////////////////////////////////////////////////////

Annotation::Annotation(string& text, int width /* = 0 */, int left /* = 0 */, int top /* = 0 */)
: Inset(text,width,left,top)
{
	widestWordWidth_ = 0;
	user_specified_width_ = width;
	layoutHasChanged();

}

Annotation::Annotation(const Annotation& copyMe)
: Inset(copyMe)
{
	widestWordWidth_ = 0;
	user_specified_width_ = copyMe.user_specified_width_;
	required_width_ = copyMe.required_width_;
}

void Annotation::setParent(Plot* parent)
{
	Inset::setParent(parent);
	layoutHasChanged();
}

Inset* Annotation::clone()
{
	return new Annotation(*this);
}

int Annotation::defaultWidth() const
{
	return int(plot_->getDimensions().width() / ANNOTATION_DEFAULT_SIZE_FACTOR);
}

// FIXME: Should not recalc this value; should just return it.
void Annotation::evaluate_required_height(HDC hdc)
{
	makeTextAsLines(hdc);
	required_height_ = top_padding();
	for(LinePosition* s: textLines_)
	{
		required_height_ += textHeightInPixels(hdc,s->str());
	}
}

int Annotation::setContents(string& text)
{
	description_ = text;
	clearTextAsLines();
	required_width_ = 0;
	required_height_ = 0;
	layoutHasChanged();
	return OK;
}

int Annotation::setWidth(int width)
{
	Inset::setWidth(width);
	required_width_ = required_height_ = 0;
	HDC hdc = GetDC(plot_->win);
	evaluate_required_width(hdc);
	ReleaseDC(plot_->win,hdc);
	return OK;
}

void Annotation::clearTextAsLines()
{
	std::for_each(textLines_.begin(), textLines_.end(), delete_object());
	textLines_.clear();
	layoutHasChanged();
}

int Annotation::widthOfWidestWord(HDC hdc)
{
	int max_width = 0;
	// Tokenize the text
	tokenizer tok(this->description_, sep);
	for(tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg){
		max_width = max(textLengthInPixels(hdc,*beg),max_width);
	}
	return max_width;
}

int Annotation::widthOfFirstNWords(HDC hdc, int n)
{
	int width = 0;
	tokenizer tok(this->description_, sep);
	// how many words in tok?
	int wordcount = 0;
	for(tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg){
		wordcount++;
	}
	n = min(n, wordcount);
	tokenizer::iterator beg = tok.begin();
	for (int i = 0; i < n; i++)
	{
		width += textLengthInPixels(hdc,*beg);
		//width += textLengthInPixels(hdc," ");
		++beg;
	}
	return width;
}

int Annotation::textLengthInPixels(HDC hdc, const std::string& text) const
{
	SIZE te;
	GetTextExtentPoint32(hdc,text.c_str(), text.length(), &te);
	return te.cx;	
}

int Annotation::textHeightInPixels(HDC hdc, const std::string& text) const
{
	SIZE te;
	GetTextExtentPoint32(hdc,text.c_str(), text.length(), &te);
	return te.cy;	
}

void Annotation::makeTextAsLines(HDC hdc)
{
	// We know the width coming in, but not the height.
	// Grab successive blocks of words that fit that width.
	SaveDC(hdc);
	HFONT f = GetFont(hdc, font_.font(),0,0);
	SelectObject(hdc,f);
	// Tokenize the text  
	tokenizer tok(this->description_, sep);
	string current_line = "";
	string extended_current_line = "";
	bool first_word = true;
	clearTextAsLines();
	for (tokenizer::iterator beg=tok.begin(); beg!=tok.end();++beg)
	{
		if (!first_word)
		{
			// If we are not adding a line's first word, add a space.
			extended_current_line.append(" ");
		}
		// Add the next word.
		extended_current_line.append(*beg);
		//Check whether we've gone over the end.
		if (textLengthInPixels(hdc,extended_current_line) > required_width_)
		{
			// Yes -- use the current_line, and start a new line.
			// Abort if the current line is empty (ie, we will never fit the current word in this width)
			if (current_line.length() == 0)
			{
				clearTextAsLines();
				DeleteObject(f);
				RestoreDC(hdc,-1);
				return;
			}
			textLines_.push_back(new Annotation::LinePosition(current_line, positionOfNextLine(hdc)));
			current_line = *beg;
			extended_current_line = *beg;
		}
		else
		{
			//  No  -- Set current_line = extended_current_line, and continue.
			current_line = extended_current_line;
		}
		first_word = false;
	}
	if (current_line.length() > 0)
	{
		textLines_.push_back(new Annotation::LinePosition(current_line, positionOfNextLine(hdc)));		
	}
	DeleteObject(f);
	RestoreDC(hdc,-1);
}

void Annotation::evaluate_required_width(HDC hdc)
{
	widestWordWidth_ = widthOfWidestWord(hdc);
	if (user_specified_width_)
	{
		required_width_ = max(user_specified_width_, widestWordWidth_) + left_inner_padding() + right_padding();
		
	}
	else
	{
		required_width_ = max(widestWordWidth_, widthOfFirstNWords(hdc,4)) + left_inner_padding() + right_padding();
	}
}

void Annotation::selectDrawingObjects(HDC hdc)
{
	f_ = GetFont(hdc, font_.font(),0,0); 
	SelectObject(hdc,f_);
}

void Annotation::drawContents(HDC hdc)
{
	SaveDC(hdc);
	SetBkMode(hdc,TRANSPARENT);
	recalculateLayout(hdc);
	ShortRect rect;
	for(Annotation::LinePosition* line: textLines_)
	{
		WriteText(hdc,position().getx0() + line->pt().x, position().gety0() + line->pt().y,WindowLayout::LEFT_ALIGN,WindowLayout::CENTRE_ALIGN,
			WindowLayout::HORIZONTAL,font_.font(),RGB_BLACK,RGB_WHITE,line->str().c_str(),rect);
	}
	RestoreDC(hdc, -1);
}

void Annotation::deleteDrawingObjects()
{
	DeleteObject(f_);
}


POINT* Annotation::positionOfNextLine(HDC hdc)
{
	POINT* p = new POINT();
	p->x = left_inner_padding();
	p->y = top_padding();
	for(LinePosition* line: textLines_)
	{
		p->y += textHeightInPixels(hdc,line->str());
	}
	return p;
}

////////////////////////////////////////////////////////////////////////////////////////////

Annotation::LinePosition::LinePosition(string& s, POINT* pos)
{
	this->s_ = s;
	this->p_ = pos;
}

Annotation::LinePosition::LinePosition(const LinePosition& copyMe)
{
	this->s_ = copyMe.s_;
	this->p_ = copyMe.p_;
}

////////////////////////////////////////////////////////////////////////////////////////////

PlotLegend::PlotLegend()
: Inset(string("Legend"))
{
	visible_ = false;	
	visibleLocked_ = false;
	longest_trace_name_width_ = 0;
	avg_trace_name_height_ = 0;
	total_trace_name_height_ = 0;
}

PlotLegend::PlotLegend(const PlotLegend& copyMe) 
: Inset(copyMe)
{
	plot_ = 0;
	traceList_ = 0;
	longest_trace_name_width_ = 0;
	avg_trace_name_height_ = 0;
	total_trace_name_height_ = 0;
}

Inset* PlotLegend::clone()
{
	return new PlotLegend(*this);
}

PlotLegend::~PlotLegend()
{
}

bool PlotLegend::drawingCurTraceIndicator()
{
	return ((gPlotMode == DISPLAY || gPlotMode == CLIPBOARD) && 
	       	displayCount() > 1);
}

short PlotLegend::left_inner_padding()
{
	if (drawingCurTraceIndicator()) // No point drawing the curtrace indicator with only a single trace
		return Inset::left_inner_padding() + (PLOT_LEGEND_CURTRACE_INDICATOR_SPACE * x_scaling_factor());
	return Inset::left_inner_padding();
}

short PlotLegend::curTrace_indicator_xpos()
{
	return (position().getx0() + PLOT_LEGEND_CURTRACE_INDICATOR_POS) * x_scaling_factor();
}

short PlotLegend::segment_width()
{
	return PLOT_LEGEND_SEGMENT_WIDTH * x_scaling_factor();
}

short PlotLegend::name_spacing()
{
	return PLOT_LEGEND_SEG_NAME_SPACING * x_scaling_factor();
}

short PlotLegend::interline_spacing()  
{
	return PLOT_LEGEND_INTERLINE_SPACING * y_scaling_factor();
}

SIZE PlotLegend::text_extents(const char* const name, HDC hdc)
{
	SIZE te;
	int nameLength = strlen(name);

	if (0 == nameLength)
	{
		nameLength = strlen(PLOT_LEGEND_MINIMUM_DISPLAYED_TRACE_NAME);
		GetTextExtentPoint32(hdc, PLOT_LEGEND_MINIMUM_DISPLAYED_TRACE_NAME, nameLength, &te);
	}
	else if (nameLength > PLOT_LEGEND_MAXIMUM_DISPLAYED_TRACE_NAME)
	{
		nameLength = PLOT_LEGEND_MAXIMUM_DISPLAYED_TRACE_NAME;
		GetTextExtentPoint32(hdc, name, nameLength, &te);
	}
	else
	{
		GetTextExtentPoint32(hdc, name, nameLength, &te);
	}
	return te;
}

short PlotLegend::name_width(const char* const name, HDC hdc)
{
	return text_extents(name, hdc).cx;
}

void PlotLegend::evaluate_longest_trace_name_width(HDC hdc)
{
	longest_trace_name_width_ = 0;
	for(Trace* t: *traceList_)
	{
		short candidate;
		if (!t->appearsInLegend())
		{
			continue;
		}
		if ((candidate = name_width(t->getName(), hdc)) > longest_trace_name_width_)
		{
			longest_trace_name_width_ = candidate;
		}
	}
}

short PlotLegend::name_height(const char* const name, HDC hdc)
{
	return text_extents(name, hdc).cy;
}

int PlotLegend::displayCount() const
{
	int count = 0;
	for(Trace* t: *traceList_)
	{
		if (t->appearsInLegend())
		{
			count++;
		}
	}
	return count;
}


void PlotLegend::evaluate_total_trace_name_height(HDC hdc)
{
	total_trace_name_height_ = 0;

   for(Trace* t: *traceList_)
	{
		if (!t->appearsInLegend())
		{
			continue;
		}
		total_trace_name_height_ += name_height(t->getName(), hdc);
	}
}

void PlotLegend::evaluate_avg_trace_name_height(HDC hdc)
{
	int display_count = displayCount();
	avg_trace_name_height_ = display_count ? total_trace_name_height_ / displayCount() : 0;
}

void PlotLegend::evaluate_required_width(HDC hdc)
{
	evaluate_longest_trace_name_width(hdc);

	// Required width consists of:
	required_width_ =  
	// left padding, plus
	left_inner_padding() +
	// width of a line segment, plus
	segment_width() +
	// spacing, plus
	2 * name_spacing() +
	// width of the longest trace name, plus
	longest_trace_name_width_ + 
	// right padding.
	right_padding();
}

void PlotLegend::evaluate_required_height(HDC hdc)
{
	evaluate_total_trace_name_height(hdc);
	evaluate_avg_trace_name_height(hdc);

	required_height_ = 
	// Required height with n traces consists of:
	// top and bottom padding, plus
	avg_trace_name_height_ + 
	// total trace name heights, plus
	total_trace_name_height_ +
	// n - 1 times inter-line spacing, plus
	(displayCount() - 1) * interline_spacing() ;
	// bottom padding.
}

float PlotLegend::drawingScalingFactor(HDC hdc)
{
	SIZE te;
	GetTextExtentPoint32(hdc, PLOT_LEGEND_MINIMUM_DISPLAYED_TRACE_NAME, 1, &te);
	return te.cy/15.0;
}

void PlotLegend::selectDrawingObjects(HDC hdc)
{
	f_ = GetFont(hdc, font_.font(),0,0); 
	SelectObject(hdc,f_);
}

void PlotLegend::drawContents(HDC hdc)
{
	// Tell each trace where to draw its line segment.
	int i = 0; // Trace index
	for(Trace* t: *traceList_)
	{
		if (!t->appearsInLegend())
		{
			continue;
		}
		POINT start; 
		POINT end;
		segmentEndsForTrace(i, start, end, hdc);
		t->drawSegment(hdc, &start, &end);
		t->drawSegmentTick(hdc, drawingScalingFactor(hdc), &start, &end);
		namePositionForTrace(i, start, hdc);
		t->drawName(hdc, font_, &start);
		if (plot_->curTrace() == t)
		{
			if (drawingCurTraceIndicator())
			{
				// ...then draw the "current trace" symbol.
				HBRUSH symbolBrush = (HBRUSH) GetStockObject(WHITE_BRUSH);
				HPEN symbolPen = (HPEN)CreatePen(PS_SOLID,0,RGB_BLACK);
				HBRUSH bkgBrush = plot_->GetPlotBackgroundBrush();
				HPEN bkgPen = CreatePen(PS_SOLID,0,plot_->bkColor);

				end.x = curTrace_indicator_xpos();
				plot_->DrawSymbol(hdc, PLOT_SYMBOL_CIRCLE, symbolBrush, symbolPen, bkgBrush, bkgPen,end.x, end.y, 3,3);

				DeleteObject(symbolBrush);
				DeleteObject(symbolPen);
				DeleteObject(bkgBrush);
				DeleteObject(bkgPen);
			}
		}
		i++;
	}
}

void PlotLegend::deleteDrawingObjects()
{
	DeleteObject(f_);
}


// FIXME: Should only recalc when there is a change.
void PlotLegend::segmentEndsForTrace(int index, POINT& start, POINT& end, HDC hdc)
{
	start.x = position().getx0() + left_inner_padding();
	start.y = position().gety0() + avg_trace_name_height_
		+ index * (avg_trace_name_height_ + interline_spacing());

	end.x = start.x + segment_width();
	end.y = start.y;
}

// FIXME: Should only recalc when there is a change.
void PlotLegend::namePositionForTrace(int index, POINT& pos, HDC hdc)
{
	pos.x = position().getx0() + left_inner_padding() + segment_width() + (2 * name_spacing());
	pos.y = position().gety0() + avg_trace_name_height_ + 
		index * (avg_trace_name_height_ + interline_spacing());
}

short PlotLegend::save(FILE* fp) const
{
	bool show_legend = visible();
	fwrite(&show_legend,sizeof(bool),1,fp);
	int legend_relative_x = relativeX();
	fwrite(&legend_relative_x, sizeof(int), 1, fp);
	int legend_relative_y = relativeY();
	fwrite(&legend_relative_y, sizeof(int), 1, fp);
	InsetCorner legend_corner = corner();
	fwrite(&legend_corner, sizeof(InsetCorner), 1, fp);
	
	return OK;
}

void PlotLegend::drawBackground(HDC hdc)
{
	Inset::drawBackground(hdc);
	RECT r;
	HBRUSH plotBkBrush = plot_->GetPlotBackgroundBrush();
	SaveDC(hdc);
	SetRect(&r, position().getx0() + 2, 
		position().gety0() + 2, 
		position().getx0() + left_inner_padding() + segment_width() + name_spacing(), 
		position().gety1() - 1);
	FillRect(hdc, &r, plotBkBrush);
	DeleteObject(plotBkBrush);
	RestoreDC(hdc, -1);
}

bool PlotLegend::visible() const
{
	int count = displayCount();
	if (Inset::visible() && (count >= MINIMUM_TRACES_BEFORE_DISPLAYING_LEGEND || (count > 0 && visibleLocked())))
	{
		return true;
	}
	return false; 
}

void PlotLegend::setParent(Plot1D* parent)
{
	Inset::setParent(parent);
	this->plot_ = parent;
}

ImageInset::ImageInset(string& filename, int width /* = 0 */, int left /* = 0 */, int top /* = 0 */)
: Inset(filename,width,left,top)
{
	filename_ = filename;
	gdi_bitmap_ = 0;
	gdi_bitmap_ = loadImageFile(filename);
	required_height_ = gdi_bitmap_->GetHeight();
	required_width_ = gdi_bitmap_->GetWidth();
	position_.setx1(position_.getx0() + required_width_);
	position_.sety1(position_.gety0() + required_height_);
	gdi_bitmap_->GetHBITMAP(0,&image_);
	hdcMem_ = 0;
	
	/* Work out the scaling factor, if the user has specified a display width. */
	userScaling_ = width ? (float)((float)width/(float)required_width_) : 1;
}

ImageInset::ImageInset(const ImageInset& copyMe)
: Inset(copyMe)
{
	image_ = copyMe.image_;
	filename_ = copyMe.filename_;
	userScaling_ = copyMe.userScaling_;
	Gdiplus::Bitmap* srcBMP = copyMe.gdi_bitmap_;
	gdi_bitmap_ = srcBMP->Clone(0,0,srcBMP->GetWidth(), srcBMP->GetHeight(), PixelFormatDontCare);
	required_height_ = gdi_bitmap_->GetHeight();
	required_width_ = gdi_bitmap_->GetWidth();
	gdi_bitmap_->GetHBITMAP(0,&image_);
	hdcMem_ = 0;
}

Inset* ImageInset::clone()
{
	return new ImageInset(*this);
}

ImageInset::~ImageInset()
{
	DeleteObject(image_);
	DeleteObject(hdcMem_);
	if (gdi_bitmap_)
	{
		delete gdi_bitmap_;
	}
}

void ImageInset::selectDrawingObjects(HDC hdc)
{
	hdcMem_ = CreateCompatibleDC(hdc);
	SelectObject(hdcMem_, image_);
}

void ImageInset::drawContents(HDC hdc)
{
	int destination_width = required_width_ * x_scaling_factor();
	int destination_height = required_height_ * y_scaling_factor(); 
	StretchBlt(hdc,position().getx0(),position().gety0(),destination_width,destination_height,hdcMem_,0,0,required_width_,required_height_,SRCCOPY);
}

void ImageInset::deleteDrawingObjects()
{
	DeleteObject(hdcMem_);
}

Bitmap* ImageInset::loadImageFile(string& filename) throw (Status)
{
	wchar_t* fileNameWide = CharToWChar(filename.c_str());
//	Bitmap* bp = ::new Bitmap(fileNameWide);
	Bitmap* bp = new Bitmap(fileNameWide);
	Status errorStatus = Ok;
	if (errorStatus = bp->GetLastStatus()) 
	{
		delete bp;
		//::delete bp;
		char error_message[512];
		sprintf(error_message, "GDI error %d opening image file %s.", (int)errorStatus, filename.c_str());
		throw BitmapFileException(string(error_message));
	}
	return bp;
}

int ImageInset::setContents(string& filename)
{
	if (gdi_bitmap_)
	{
		delete gdi_bitmap_;
	}
	gdi_bitmap_ = this->loadImageFile(filename);
	description_ = filename;
	filename_ = filename;
	required_height_ = gdi_bitmap_->GetHeight();
	required_width_ = gdi_bitmap_->GetWidth();
	DeleteObject(image_);
	gdi_bitmap_->GetHBITMAP(0,&image_);
	/* Work out the scaling factor, if the user has specified a display width. */
	userScaling_ = user_specified_width_ ? (float)((float)user_specified_width_/(float)required_width_) : 1;
	return(OK);
}

int ImageInset::setWidth(int width)
{
	Inset::setWidth(width);
	userScaling_ = (float)((float)width/(float)required_width_);
	return(OK);
}

//////////////////////////////////////////////////////////////////////////////////////

InsetDescription::InsetDescription(int index, string& contents, string& position, bool visibility)
{
	index_ = index;
	contents_ = contents;
	position_ = position;
	visibility_ = visibility;
}

