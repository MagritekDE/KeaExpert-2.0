/**
* @file Inset.h
* @brief Inset header
*/

#ifndef INSET_H
#define INSET_H

#include "Plot.h"
#include "font.h"
#include "Scalable.h"
#include "ShortRect.h"

namespace Gdiplus {
	class Bitmap;
}

typedef enum
{
	/**
	* @enum InsetType represents the type of the Inset for, eg, saving
	*/
	NO_INSET,
	ANNOTATION,
	IMAGE,
	LEGEND
}  InsetType;


/***********************************************************************
/* Constants: pixel widths of various things in an Inset.					*
/************************************************************************/
#define INSET_PADDING_LEFT 10
#define INSET_PADDING_RIGHT INSET_PADDING_LEFT
#define INSET_PADDING_TOP INSET_PADDING_LEFT
#define INSET_PADDING_BOTTOM INSET_PADDING_LEFT

/************************************************************************
/* Corner of enclosing plot to which the inset may be considered closest
/************************************************************************/
typedef enum {
	/**
	*	@enum InsetCorner the plot corner to which this Inset is closest
	*/
	NO_CORNER,				/**< Not close to any corner. */
	TOP_LEFT,		/**< Close to top left corner. */
	BOTTOM_LEFT,	/**< Close to bottom left corner. */
	TOP_RIGHT,		/**< Close to top right corner. */
	BOTTOM_RIGHT	/**< Close to bottom right corner. */
} InsetCorner;

/************************************************************************
/* Relative distance from the closest LegendCorner.							*
/************************************************************************/
typedef struct 
{
	int x;
	int y;
} RelativePosition;


/************************************************************************
/* Insets provide these as descriptions of themselves, eg for command line 
/* displays of existing Inserts.
/***********************************************************************/
class InsetDescription
{
public:
	/************************************************************************
	/* Constructor for InsetDescription.
	/* @param index the index by which the Inset can be addressed
	/* @param contents the contents of the Inset
	/* @param position a description of the Inset's position (eg, "x,y")
	/* @param visibility the visibility state of the Inset	
	/***********************************************************************/
	InsetDescription(int index, std::string& contents, std::string& position, bool visibility);
	/************************************************************************
	/* Destructor for InsetDescription.
	/***********************************************************************/
	~InsetDescription(){}
	/************************************************************************
	/* Copy constructor for InsetDescription.
	/* @param copyMe the InsetDescription to copy
	/***********************************************************************/
	InsetDescription(const InsetDescription& copyMe);
	/************************************************************************
	/* The index by which the Inset can be addressed.
	/* @return the index by which the Inset can be addressed
	/***********************************************************************/
	int index() const {return index_;}
	/************************************************************************
	/* The contents of the Inset.
	/* @return the contents of the Inset
	/***********************************************************************/
	const std::string& contents() const {return contents_;}
	/************************************************************************
	/* The position of the Inset.
	/* @return the position of the Inset
	/***********************************************************************/
	const std::string& position() const {return position_;}
	/************************************************************************
	/* The visibility state of the Inset.
	/* @return the visibility state of the Inset
	/***********************************************************************/
	bool visibility() const {return visibility_;}

private:
	int index_;					///< Index of the described Inset.
	std::string contents_;			///< Contents of the described Inset.
	std::string position_;			///< x,y position of the described Inset.
	bool visibility_;			///< visibility state of the described Inset.
};

/************************************************************************
/* Insets are boxes drawn over top of a plot. Abstract class.
/************************************************************************/
class Inset : public Scalable
{
public:
	/************************************************************************
	/* Constructor for an abstract Inset.
	/* @param description text description of this Inset
	/* @param width the width of this Annotation. If not set, a default width is
	/*   used.
	/* @param left the x coordinate of the top left corner of this Annotation
	/* @param top the y coordinate of the top left corner of this Annotation
	/************************************************************************/
	Inset(std::string& description, int width = 0, int left = 0, int top = 0);
	/************************************************************************
	/* Copy constructor for an abstract Inset.                                
	/* @param copyMe the Inset to be copied.
	/************************************************************************/
	Inset(const Inset& copyMe);
	/************************************************************************
	/* Polymorphic clone for an abstract Inset.                                                                     
	/************************************************************************/
	virtual Inset* clone() = 0;
	/************************************************************************
	/* Destructor for an abstract Inset.                                                                     
	/************************************************************************/
	virtual ~Inset();
	/************************************************************************
	/* Template method to draw this Inset over its parent.
	/* @param hdc the device context to draw on
	/************************************************************************/
	void draw(HDC hdc);
	/************************************************************************
	/* Is this Inset visible?                  
	/*	@return whether this Inset is visible
	/************************************************************************/
	virtual bool visible() const {return visible_;}
	/************************************************************************
	/* Is this Inset's visibility state locked to its current value?
	/* @return true if visibility locked to current value, otherwise false
	/************************************************************************/
	bool visibleLocked() const {return visibleLocked_;}
	/************************************************************************
	/* Get the current background color of this Inset.                    
	/* @return background color of this Inset.
	/************************************************************************/
	COLORREF backgroundColor() const {return backgroundColor_;}
	/************************************************************************
	/* Move this Inset within its parent.
	/* @param dx the change in x position
	/* @param dy the change in y position
	/************************************************************************/
	virtual void shift(float dx, float dy);
	/************************************************************************
	/* Return the current position of this Inset relative to the top left of
	/*  its parent.
	/* @return the current position of this Inset.
	/************************************************************************/
	ShortRect& position() {return position_;}
	/************************************************************************
	/* Return the current x position of this Inset relative to the closest corner
	/*  of its parent.
	/* @return the current x position of this Inset relative to the closest corner
	/*  of its parent.
	/************************************************************************/
	int relativeX() const {return relativePosition_.x;}
	/************************************************************************
	/* Return the current y position of this Inset relative to the closest corner
	/*  of its parent.
	/* @return the current y position of this Inset relative to the closest corner
	/*  of its parent.
	/************************************************************************/
	int relativeY() const {return relativePosition_.y;}
	/************************************************************************
	/* Return the current x position of this Inset relative to the closest corner
	/*  of its parent.
	/* @return the current x position of this Inset relative to the closest corner
	/*  of its parent.
	/************************************************************************/
	void setRelativeX(int relativeX) {relativePosition_.x = relativeX;}
	/************************************************************************
	/* Return the current y position of this Inset relative to the closest corner
	/*  of its parent.
	/* @return the current y position of this Inset relative to the closest corner
	/*  of its parent.
	/************************************************************************/
	void setRelativeY(int relativeY) {relativePosition_.y = relativeY;}
	/************************************************************************
	/* Set the visibility state of this Inset. Forbid Prospa from changing its
	/*  visibility programmatically.
	/* @param visible the new visibility state of this Inset
	/************************************************************************/
	void setVisible(bool visible){visible_ = visible; visibleLocked_ = true;}
	/************************************************************************
	/* Set or release lock on the visibility state of this Inset. 
	/* @param locked if true, Prospa will not unilaterally change the visibility
	/*  state of this Inset without user intervention. If false, Prospa will
	/*  decide when to display the Inset.
	/************************************************************************/
	void setVisibleLocked(bool locked){visibleLocked_ = locked;}
	/************************************************************************
	/* Set the background color of this Inset.
	/* @param color the new background color of this Inset.
	/************************************************************************/
	void setBackgroundColor(COLORREF color) {backgroundColor_ = color;}
	/************************************************************************
	/* Set the position of this Inset.
	/* @param position the new position for this Inset.
	/************************************************************************/
	void setPosition(ShortRect& position) {position_ = position;}
	void setPosition(short x, short y) 
   {
      short w = position_.getx1()-position_.getx0();
      short h = position_.gety1()-position_.gety0();
      position_.setx0(x);
      position_.setx1(x+w);
      position_.sety0(y);
      position_.sety1(y+h);
   }
	/************************************************************************
	/* Supply a string from which this Inset can derive its contents.
	/* @param text a string from which this Inset can derive its contents
	/* @return OK if successful, ERR otherwise.
	/************************************************************************/
	virtual int setContents(std::string& text) = 0;
	/************************************************************************
	/*	Set the name of the double-click callback for this Inset.
	/* @param cback the name of the double-click callback for this Inset
	/************************************************************************/
	void setCallback(const std::string& cback) {callback_ = cback;}
	/************************************************************************
	/* Return the name of the double-click callback for this Inset.
	/*	@return the name of the double-click callback for this Inset
	/************************************************************************/
	const std::string& callback() const {return callback_;}
	/************************************************************************
	/* Set the width of this Inset.
	/* @param width the new width for this Inset.
	/* @return OK if successful, ERR otherwise.
	/************************************************************************/
	virtual int setWidth(int width);
	/************************************************************************
	/* Set the position of this Inset relative to its nearest corner.
	/************************************************************************/
	virtual void updateRelativePosition();
	/************************************************************************
	/* Save a description of this Inset to file, using the current protocol.
	/* @param fp the file to save to; will write at the current position.
	/* @return OK if the operation was successful, ERR otherwise
	/************************************************************************/
	virtual short save(FILE* fp) const;
	/************************************************************************
	/* Set the plot parent for this Inset.
	/* @param parent the new plot parent for this Inset.
	/************************************************************************/
	virtual void setParent(Plot* parent) {plot_ = parent;}
	/************************************************************************
	/* Is this Inset currently being moved by the mouse?
	/* @return true if this Inset is currently being moved by the mouse; false
	/*    otherwise
	/************************************************************************/
	bool beingMoved() const {return this->beingMoved_;}
	/************************************************************************
	/* Set whether this Inset is currently being moved by the mouse.
	/* @param isBeingMoved set true if this Inset is currently being moved by 
	/*   the mouse; false otherwise
	/************************************************************************/
	void beingMoved(bool isBeingMoved) {this->beingMoved_ = isBeingMoved;}
	/************************************************************************
	/* Call this to notify this Inset that it must recalculate its layout.                                                                     
	/************************************************************************/
	virtual void layoutHasChanged() {need_to_recalculate_layout_ = true;}
	/************************************************************************
	/* Get an InsetDescription describing this Inset, for, eg, displaying an
	/*  inventory of existing Insets.
	/* @param index the value by which the client expects to be able to address
	/*  this Inset
	/* @return a pointer to description of this Inset. The caller is responsible
	/*  for this object.
	/************************************************************************/
	virtual InsetDescription* description(int index);
	/************************************************************************
	/* Handle command line messages to this Inset.
	/* @param itfc the Interface being used to communicate back to the CLI
	/* @param args the arguments sent to this Inset from the CLI
	/* @return OK if the operation was successful, otherwise ERR.
	/************************************************************************/
	int ProcessInsetParameters(Interface* itfc, char args[]);
	/************************************************************************
	/* Format state of this Inset in a human readable way.
	/* @return state of this Inset in a human readable way.
	/************************************************************************/
	std::string FormatState();
	/************************************************************************
	/* Indicates whether this Inset is saveable (in a Plot file)
	/* @return true if this Inset is saveable; otherwise, false
	/************************************************************************/
	virtual bool isSaveable() {return true;}
	/************************************************************************
	/* The type of this Inset.
	/* @return the type of this Inset.
	/***********************************************************************/
	virtual InsetType type() const = 0;
	/************************************************************************
	/* Factory method for Insets.
	/* @param type string representing type of Inset to be instantiated.
	/* @param width user-specified width of the Inset to create
	/* @param left user-specified left position of this Inset
	/* @param top user-specified top position of this Inset
	/* @return a new Inset per the parameters' description, or NULL if one
   /*   cannot be made
	/************************************************************************/
	static Inset* makeInset(InsetType type, std::string& text, int width = 0, int left = 0, int top = 0);
	/************************************************************************
	/* Return the closest corner of this Inset.
	/* @return the closest corner of this Inset.
	/************************************************************************/
	InsetCorner corner() const {return corner_;}
	/************************************************************************
	/* Set the closest corner of this Inset.
	/* @param the new closest corner of this Inset.
	/************************************************************************/
	void setCorner(InsetCorner corner) {corner_ = corner;}
protected:
	/************************************************************************
	/* Paint the background of this Inset with its background brush.
	/* @param hdc the device context to use
	/************************************************************************/
	virtual void drawBackground(HDC hdc);
	/************************************************************************
	/* Get this Inset's left inner padding width in pixels.
	/* @return this Inset's left inner padding width in pixels.
	/************************************************************************/
	virtual short left_inner_padding();
	/************************************************************************
	/* Get this Inset's left outer padding width in pixels.
	/* @return this Inset's left outer padding width in pixels.
	/************************************************************************/
	virtual short left_outer_padding() const;
	/************************************************************************
	/* Get this Inset's right padding width in pixels.
	/* @return this Inset's right padding width in pixels.
	/************************************************************************/
	virtual short right_padding() const;
	/************************************************************************
	/* Get this Inset's top padding height in pixels.
	/* @return this Inset's top padding height in pixels.
	/************************************************************************/
	virtual short top_padding() const;
	/************************************************************************
	/* Get this Inset's bottom padding height in pixels.
	/* @return this Inset's bottom padding height in pixels.
	/************************************************************************/
	virtual short bottom_padding() const;
	/************************************************************************
	/* Resize this Inset to fit a required width and height. Reposition myself
	/*  if this Inset now goes outside of one or more of the plot's edges.
	/* @param required_width new width of this Inset
	/* @param required_height new height of this Inset
	/************************************************************************/
	virtual void resize();
	/************************************************************************
	/* Draw the border around this Inset using its current border brush.
	/* @param hdc the device context to use
	/************************************************************************/
	virtual void drawBorder(HDC hdc);
	/************************************************************************
	/* Scalability interface function. Scales this Inset by specified x,y factors.
	/* @param x amount to scale x by
	/* @param y amount to scale y by
	/************************************************************************/
	void scale_(double x, double y);
	/************************************************************************
	/* Scalability interface function. Scales this Inset back to 100%.
	/************************************************************************/
	void unscale_();
	/************************************************************************
	/* Figure out the required height and width for this Inset.
	/* @param hdc the device context to use
	/************************************************************************/
	virtual void recalculateLayout(HDC hdc);

	float userScaling_;					///< The ratio between the default width and user-requested width
	ShortRect position_;					///< top left and bottom right points relative to plot_
	ProspaFont font_;						///< Font used for all text in this Inset
	Plot* plot_;							///< Plot parent of this Inset
	bool need_to_recalculate_layout_; ///< Flag notes when layout must be recalc'd
	int required_width_;					///< The minimum width into which this Inset will fit.
	int required_height_;				///< The minimum height into which this Inset will fit.
	HFONT f_;								///< Temporary HFONT, used for figuring out text sizes
	bool visible_;							///< visibility state
	bool visibleLocked_;					///< visibility lock state
	int user_specified_width_; 		///< ...that width. 

	std::string description_;			///< Description of this inset (eg, filename, text contents...)

private:
	/************************************************************************
	/* Return the x,y position of this Inset's closest plot corner.
	/* @return the x,y position of this Inset's closest plot corner
	/************************************************************************/
	POINT cornerPoint();
	/************************************************************************
	/* Discover and set the currently closest plot corner.
	/* @return the currently closest plot corner.
	/************************************************************************/
	InsetCorner updateClosestPlotCorner();
	/************************************************************************
	/* Checks whether this Inset can fit inside its Plot.
	/* @param hdc the device context to use
	/* @return true if this Inset can fit inside its Plot; false otherwise
	/************************************************************************/
	bool fits_in_parent(HDC hdc);
	/************************************************************************
	/* Figures out and sets this Inset's required pixel width.
	/* @param hdc the device context to use
	/************************************************************************/
	virtual void evaluate_required_width(HDC hdc) = 0;
	/************************************************************************
	/* Figures out and sets this Inset's required pixel height.
	/* @param hdc the device context to use
	/************************************************************************/
	virtual void evaluate_required_height(HDC hdc) = 0;
	/************************************************************************
	/* Used by the draw() template method; derived classes define this to set
	/*  up the device context as required.
	/* @param hdc the device context to use
	/************************************************************************/
	virtual void selectDrawingObjects(HDC hdc) = 0;
	/************************************************************************
	/* Used by the draw() template method; derived classes define this to 
	/*  draw their contents within the Inset.
	/* @param hdc the device context to use
	/************************************************************************/
	virtual void drawContents(HDC hdc) = 0;
	/************************************************************************
	/* Used by the draw() template method; derived classes define this to 
	/*  delete device objects created in selectDrawingObjects().
	/* @param hdc the device context to use
	/************************************************************************/
	virtual void deleteDrawingObjects() = 0;

	InsetCorner corner_;		///< the closest corner to this Inset
	RelativePosition relativePosition_;		///< relative position WRT that closest corner
	COLORREF backgroundColor_;			///< background color of this Inset
	bool beingMoved_;						///< true if the mouse is moving this inset, false otherwise	
	std::string callback_;				///< name of CLI-level macro to run on double-click
};

#define ANNOTATION_DEFAULT_SIZE_FACTOR 8
#define ANNOTATION_MINIMUM_HEIGHT 30
#define ANNOTATION_MINIMUM_WIDTH 30
#define ANNOTATION_LINE_SPACING 8

/************************************************************************
/* Annotations are user-created text boxes superimposed on plots.
/************************************************************************/
class Annotation : public Inset
{
public:
	/************************************************************************
	/* Constructor for an Annotation. 
	/* @param text the text to be displayed in this Annotation.
	/* @param width the width of this Annotation. If not set, a default width is
	/*   used.
	/* @param left the x coordinate of the top left corner of this Annotation
	/* @param top the y coordinate of the top left corner of this Annotation
	/************************************************************************/
	Annotation(std::string& text, int width = 0, int left = 0, int top = 0);
	/************************************************************************
	/* Copy constructor for an Annotation.                                
	/* @param copyMe the Annotation to be copied.
	/************************************************************************/
	Annotation(const Annotation& copyMe);
	/************************************************************************
	/* Polymorphic clone for an Annotation.
	/* @return a deep polymorphic clone of this Annotation.
	/************************************************************************/
	Inset* clone();
	/************************************************************************
	/* Destructor                                                                     
	/************************************************************************/
	virtual ~Annotation(){}
	/************************************************************************
	/* Set the text to be displayed in this Annotation.
	/* @param text the text to be displayed in this Annotation.
	/* @return OK if successful, ERR otherwise.
	/************************************************************************/
	int setContents(std::string& text);
	/************************************************************************
	/* Set the width of the Annotation.
	/* @param width the width of the Annotation
	/* @return OK if successful, ERR otherwise
	/************************************************************************/
	int setWidth(int width);
	/************************************************************************
	/* The type of this Inset.
	/* @return the type of this Inset.
	/***********************************************************************/
	InsetType type() const {return ANNOTATION;}
	/************************************************************************
	/* Set the plot parent for this Inset.
	/* @param parent the new plot parent for this Inset.
	/************************************************************************/
	void setParent(Plot* parent);

protected:
private:

	/************************************************************************
	/* Assume the appropriate size for this Annotation, given its contents.
	/************************************************************************/
	void resolveDimensions();
	/************************************************************************
	/* Returns the default width of a Annotation.
	/* @return the default width of a Annotation.
	/************************************************************************/
	int defaultWidth() const;
	/************************************************************************
	/* Returns the text, split up into lines that fit into this Annotation. 
	/* @param hdc the device context to use
	/*  Lines are broken at word boundaries. 
	/************************************************************************/
	void makeTextAsLines(HDC hdc);
	/************************************************************************
	/* Use the current width to figure out the required height for this Annotation. 
	/* @param hdc the device context to use                                                                    
	/* @return the required height for this Annotation.
	/************************************************************************/
	void evaluate_required_height(HDC hdc);
	/************************************************************************
	/* Returns the pixel width of the widest word in this Annotation. 
	/* @param hdc the device context to use
	/* @return the pixel width of the widest word in this Annotation. 
	/************************************************************************/
	int widthOfWidestWord(HDC hdc);
	/************************************************************************
	/* Returns the pixel width of the first n words in this Annotation. 
	/* @param hdc the device context to use
	/* @param hdc the number of words to use
	/* @return the pixel width of the first n words, plus spaces, in this 
	/*   Annotation. 
	/************************************************************************/
	int widthOfFirstNWords(HDC hdc, int n);
	/************************************************************************
	/* Returns the pixel width of the supplied text in this Annotation. 
	/* @param hdc the device context to use
	/* @param text the text to measure
	/* @return the pixel width of the supplied word in this Annotation. 
	/************************************************************************/
	int textLengthInPixels(HDC hdc, const std::string& text) const;
	/************************************************************************
	/* Returns the pixel height of the supplied text in this Annotation. 
	/* @param hdc the device context to use
	/* @param text the text to measure
	/* @return the pixel height of the supplied word in this Annotation. 
	/************************************************************************/
	int textHeightInPixels(HDC hdc, const std::string& text) const;
	/************************************************************************
	/* Empty the list of text broken into lines.
	/************************************************************************/
	void clearTextAsLines();
	/************************************************************************
	/* The top left point in the index'th line in this Annotation.
	/* @param hdc the device context to use
	/* @return the x,y position of the index'th line in this Annotation.
	/************************************************************************/
	POINT* positionOfNextLine(HDC hdc);
	/************************************************************************
	/* Figures out and sets this Annotation's required pixel height.
	/* @param hdc the device context to use
	/************************************************************************/
	void evaluate_required_width(HDC hdc);
	/************************************************************************
	/* Used by the draw() template method; sets up device context for drawing
	/*  Annotation.
	/* @param hdc the device context to use
	/************************************************************************/	
	void selectDrawingObjects(HDC hdc);
	/************************************************************************
	/* Used by the draw() template method; draws Annotation contents.
	/* @param hdc the device context to use
	/************************************************************************/
	void drawContents(HDC hdc);
	/************************************************************************
	/* Used by the draw() template method; deletes device objects created in 
	/*  selectDrawingObjects().
	/************************************************************************/	
	void deleteDrawingObjects();
	/************************************************************************
	/* Encapsulates a line of text and its position.
	/************************************************************************/
	class LinePosition
	{
	public:
		/************************************************************************
		/* Constructor
		/* @param s the string in this line
		/* @param pos the point associated with this line
		/************************************************************************/
		LinePosition(std::string& s, POINT* pos);
		/************************************************************************
		/* Copy constructor
		/* @param copyMe the LinePosition to copy
		/************************************************************************/
		LinePosition(const LinePosition& copyMe);
		/************************************************************************
		/* Destructor for LinePosition                                                                     
		/************************************************************************/
		~LinePosition(){delete p_;}
		/************************************************************************
		/* Returns the point associated with this line
		/* @return the point associated with this line
		/************************************************************************/
		POINT& pt(){return *p_;}
		/************************************************************************
		/* Returns the string associated with this line
		/* @return the string associated with this line
		/************************************************************************/
		std::string& str(){return s_;}

	private:
		std::string s_; ///< The string associated with this line
		POINT* p_; ///< The point associated with this line
	};

	int widestWordWidth_; ///< The width of the widest word. Updated when the text changes.
	std::vector<LinePosition*> textLines_; ///< The text split into lines that fit the current width	
};

/************************************************************************
/* ImageInsets are user-created image boxes superimposed on plots.                                                                      
/************************************************************************/
class ImageInset : public Inset
{
public:
	/************************************************************************
	/* Constructor for an ImageInset.
	/* @param filename the filename of the image the bitmap was loaded from
	/* @param width the width to draw the image at
	/************************************************************************/
	ImageInset(std::string& filename, int width = 0, int left = 0, int top = 0);
	/************************************************************************
	/* Copy constructor for an ImageInset.                                
	/* @param copyMe the ImageInset to be copied.
	/************************************************************************/
	ImageInset(const ImageInset& copyMe);
	/************************************************************************
	/* Polymorphic clone for an ImageInset.      
	/* @return a deep polymorphic clone of this ImageInset.
	/************************************************************************/
	Inset* clone();
	/************************************************************************
	/* Destructor                                                                     
	/************************************************************************/
	virtual ~ImageInset();
	/************************************************************************
	/* Set the image to be displayed in this ImageInset.
	/* @param filename the filename of the image to be displayed in this Annotation.
	/* @return OK if successful, ERR otherwise.
	/************************************************************************/
	int setContents(std::string& filename);
	/************************************************************************
	/* Set the width of the image to be displayed in this ImageInset.
	/* @param width the width of the image to be displayed in this ImageInset.
	/* @return OK if successful, ERR otherwise.
	/************************************************************************/
	int setWidth(int width);
	/************************************************************************
	/* The type of this Inset.
	/* @return the type of this Inset.
	/***********************************************************************/
	InsetType type() const {return IMAGE;}

protected:
	/************************************************************************
	/* Override Inset's drawBackground to do nothing.                                                                     
	/************************************************************************/
	void drawBackground(HDC hdc) {return;}	
	/************************************************************************
	/* Override Inset's drawBorder to do nothing.
	/************************************************************************/
	void drawBorder(HDC hdc) {return;}
private:
	/************************************************************************
	/* Override Inset's evaluate_required_width to do nothing. We get the 
	/*  dimensions from the image itself, or the user's requested size.
	/* @param hdc the device context to use
	/************************************************************************/
	void evaluate_required_width(HDC hdc) {return;}
	/************************************************************************
	/* Override Inset's evaluate_required_height to do nothing. We get the 
	/*  dimensions from the image itself, or the user's requested size.
	/* @param hdc the device context to use
	/************************************************************************/
	void evaluate_required_height(HDC hdc) {return;}
	/************************************************************************
	/* Used by the draw() template method; sets up device context for drawing
	/*  ImageInset.
	/* @param hdc the device context to use
	/************************************************************************/	
	void selectDrawingObjects(HDC hdc);
	/************************************************************************
	/* Used by the draw() template method; draws ImageInset contents.
	/* @param hdc the device context to use
	/************************************************************************/
	void drawContents(HDC hdc);
	/************************************************************************
	/* Used by the draw() template method; deletes device objects created in 
	/*  selectDrawingObjects().
	/* @param hdc the device context to use
	/************************************************************************/	
	void deleteDrawingObjects();
	/************************************************************************
	/* Load the specified image file into a Gdiplus::Bitmap
	/* @param filename the file to load.
	/***********************************************************************/
	Gdiplus::Bitmap* loadImageFile(std::string& filename) throw(int);

	Gdiplus::Bitmap* gdi_bitmap_;   ///< latest bitmap loaded from image file
	HBITMAP image_;	///< The image to be drawn
	std::string filename_; ///< The filename of that image
	HDC hdcMem_;      ///< mem device context for drawing this image
};


/***********************************************************************
/* Constants: pixel widths of various things in a PlotLegend.
/************************************************************************/
#define PLOT_LEGEND_SEGMENT_WIDTH 50
#define PLOT_LEGEND_SEG_NAME_SPACING 8
#define PLOT_LEGEND_INTERLINE_SPACING PLOT_LEGEND_SEG_NAME_SPACING
#define PLOT_LEGEND_CURTRACE_INDICATOR_SPACE 12
#define PLOT_LEGEND_CURTRACE_INDICATOR_POS ((PLOT_LEGEND_CURTRACE_INDICATOR_SPACE + INSET_PADDING_LEFT) / 2)
#define PLOT_LEGEND_TRACEAXIS_INDICATOR_LENGTH (PLOT_LEGEND_INTERLINE_SPACING / 2)

/************************************************************************
/* Constants: Maximum and minimum displayable trace name lengths.			*
/************************************************************************/
#define PLOT_LEGEND_MAXIMUM_DISPLAYED_TRACE_NAME 64
#define PLOT_LEGEND_MINIMUM_DISPLAYED_TRACE_NAME "[    ]" 

/************************************************************************
/* PlotLegends are Insets whose contents and visibility are determined by
/* the traces on their parent 1D plot. By default, traces are added to a
/* a Legend, but may be removed by the user. The user can show and hide
/* a Legend. 
/************************************************************************/
class PlotLegend : public Inset
{
public:
	/************************************************************************
	/* Constructor for PlotLegend.
	/************************************************************************/
	PlotLegend();
	/************************************************************************
	/* Copy constructor for a PlotLegend.                                
	/* @param copyMe the PlotLegend to be copied.
	/************************************************************************/
	PlotLegend(const PlotLegend& copyMe);
	/************************************************************************
	/* Polymorphic clone for a PlotLegend.
	/************************************************************************/
	Inset* clone();
	/************************************************************************
	/* Destructor for a PlotLegend.           
	/************************************************************************/
	virtual ~PlotLegend();
	/************************************************************************
	/* Set the plot parent for this PlotLegend.
	/* @param parent the new plot parent for this PlotLegend.
	/************************************************************************/
	void setParent(Plot1D* parent);
	/************************************************************************
	/* Set the trace list for this PlotLegend.
	/* @param list the new trace list for this PlotLegend.
	/************************************************************************/
	void setTraceList(TraceList* list) {traceList_ = list;}
	/************************************************************************
	/* Save a description of this PlotLegend to file, using the current protocol.
	/* @param fp the file to save to; will write at the current position.
	/* @return OK if the operation was successful, ERR otherwise
	/************************************************************************/
	short save(FILE* fp) const;
	/************************************************************************
	/* Is this PlotLegend visible?                                                                     
	/************************************************************************/
	bool visible() const;
	/************************************************************************
	/* PlotLegends don't bother describing themselves. Override this Index fn.
	/************************************************************************/
	//InsetDescription* description(int index) {return 0;}
	/************************************************************************
	/* Override to disallow setting contents.
	/* @param text the text to be displayed in this Annotation (ignored).
	/* @return OK.
	/************************************************************************/
	int setContents(std::string& text) {return OK;}
	/************************************************************************
	/* Indicates whether this Inset is saveable (in a Plot file). Overrides.
	/* @return true if this Inset is saveable; otherwise, false
	/************************************************************************/
	bool isSaveable() {return false;}
	/************************************************************************
	/* The type of this Inset.
	/* @return the type of this Inset.
	/***********************************************************************/
	InsetType type() const {return LEGEND;}
private:
	/************************************************************************
	/* Paint the background of this PlotLegend with its background brush.
	/* @param hdc the device context to use
	/************************************************************************/
	void drawBackground(HDC hdc);
	/************************************************************************
	/* Return the x position of the "current trace" indicator.
	/* @return the x position of the "current trace" indicator.
	/************************************************************************/
	short curTrace_indicator_xpos();
	/************************************************************************
	/* Get this PlotLegend's left inner padding width in pixels.
	/* @return this PlotLegend's left inner padding width in pixels.
	/************************************************************************/
	short left_inner_padding();
	/************************************************************************
	/* Return the scaled width of a trace segment in this Legend. A segment 
	/*  is the short sample of the trace in its entry in a Legend.
	/* @return the width of a trace segment in this Legend.
	/************************************************************************/
	short segment_width();
	/************************************************************************
	/* Return the scaled spacing in pixels between a trace name and its segment.
	/* @return the scaled spacing in pixels between a trace name and its segment
	/************************************************************************/
	short name_spacing();
	/************************************************************************
	/* Return the scaled number of pixels between legend entries.                                                                     
	/* @return the scaled number of pixels between legend entries
	/************************************************************************/
	short interline_spacing();
	float drawingScalingFactor(HDC hdc);
	/************************************************************************
	/* Return the height and width of specified text within the context of 
	/*  this Legend. 
	/* @param name the text to measure
	/* @param hdc the device context to use
	/* @return if name is empty, the minimum size of a text area in a line of
	/*   this PlotLegend. If name is longer than will fit into the maximum size of
	/*   a text area in a line of this Legend, return the maximum size of a 
	/*   text area in a line of this Legend. Otherwise, return the size of string
	/*	  name in this PlotLegend.
	/************************************************************************/
	SIZE text_extents(const char* const name, HDC hdc);
	/************************************************************************
	/* Return the height of specified text within the context of 
	/*  this Legend. 
	/* @param name the text to measure
	/* @param hdc the device context to use
	/* @return if name is empty, the minimum height of a text area in a line of
	/*   this PlotLegend. If name is taller than will fit into the maximum height of
	/*   a text area in a line of this Legend, return the maximum height of a 
	/*   text area in a line of this Legend. Otherwise, return the height of string
	/*	  name in this PlotLegend.
	/************************************************************************/
	short name_height(const char* const name, HDC hdc);
	/************************************************************************
	/* Return the total pixel height of all trace names in this PlotLegend.
	/* @param hdc the device context to use
	/* @return the total pixel height of all trace names in this PlotLegend.
	/************************************************************************/
	void evaluate_total_trace_name_height(HDC hdc);
	/************************************************************************
	/* Return the average pixel height of trace names in this PlotLegend.
	/* @param hdc the device context to use
	/* @return the average pixel height of trace names in this PlotLegend.
	/************************************************************************/
	void evaluate_avg_trace_name_height(HDC hdc);
	/************************************************************************
	/* Return the minimum pixel height required to display all trace names.
	/* @param hdc the device context to use
	/* @return the minimum pixel height required to display all trace names
	/************************************************************************/
	void evaluate_required_height(HDC hdc);
	/************************************************************************
	/* Return the width of specified text within the context of 
	/*  this Legend. 
	/* @param name the text to measure
	/* @param hdc the device context to use
	/* @return if name is empty, the minimum width of a text area in a line of
	/*   this PlotLegend. If name is wider than will fit into the maximum width of
	/*   a text area in a line of this Legend, return the maximum width of a 
	/*   text area in a line of this Legend. Otherwise, return the width of string
	/*	  name in this PlotLegend.
	/************************************************************************/
	short name_width(const char* const name, HDC hdc);
	/************************************************************************
	/* Return the length in pixels of the longest trace name.
	/* @param hdc the device context to use
	/* @return the length in pixels of the longest trace name
	/************************************************************************/
	void evaluate_longest_trace_name_width(HDC hdc);
	/************************************************************************ 
	/* Return the minimum pixel width required to display all trace names.
	/* @param hdc the device context to use
	/* @return the minimum pixel width required to display all trace names       
	/************************************************************************/
	void evaluate_required_width(HDC hdc);	
	/************************************************************************
	/* Set the start and end points of a segment in a trace's entry in 
	/*  this PlotLegend.
	/* @param index the index of the trace's entry 
	/* @param start the start point of the segment (written to)
	/* @param end the end point of the segment (written to)
	/* @param hdc the device context to use
	/************************************************************************/
	void segmentEndsForTrace(int index, POINT& start, POINT& end, HDC hdc);
	/************************************************************************
	/* Set the top left point of a trace name display in this PlotLegend.
	/* @param index the index of the trace's entry
	/* @param pos the top left point of the trace name display (written to)
	/* @param hdc the device context to use
	/************************************************************************/
	void namePositionForTrace(int index, POINT& pos, HDC hdc);
	/************************************************************************
	/* Returns true if the "current trace" indicator should be drawn in this
	/*  PlotLegend; false otherwise.
	/* @return true if the "current trace" indicator should be drawn in this
	/*  PlotLegend; false otherwise
	/************************************************************************/
	bool drawingCurTraceIndicator();
	/************************************************************************
	/* Returns the number of traces currently drawn in this PlotLegend.
	/* @return the number of traces currently drawn in this PlotLegend
	/************************************************************************/
	int displayCount() const;
	/************************************************************************
	/* Used by the draw() template method; sets up device context for drawing
	/*  PlotLegend.
	/* @param hdc the device context to use
	/************************************************************************/	
	void selectDrawingObjects(HDC hdc);
	/************************************************************************
	/* Used by the draw() template method; draws PlotLegend contents.
	/* @param hdc the device context to use
	/************************************************************************/
	void drawContents(HDC hdc);
	/************************************************************************
	/* Used by the draw() template method; deletes device objects created in 
	/*  selectDrawingObjects().
	/* @param hdc the device context to use
	/************************************************************************/	
	void deleteDrawingObjects();
	
	TraceList* traceList_; ///< The traces currently drawn in this PlotLegend
	int longest_trace_name_width_;  ///< Length in pixels of the longest trace name being displayed
	int avg_trace_name_height_; ///< The average height of a displayed trace name.
	int total_trace_name_height_; ///< The total height of all trace names.
	Plot1D* plot_; ///< Plot parent of this PlotLegend.
};

#endif // ifndef INSET_H