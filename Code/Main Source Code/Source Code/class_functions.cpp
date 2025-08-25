#include "stdafx.h"
#include "classFunctions.h"
#include "CommandRegistry.h"
#include "string_utilities.h"
#include <string>
#include "memoryLeak.h"

using std::string;

/************************************************************************/
/*                                                                      */
/************************************************************************/


CommandRegistry *widgetCommandRegistry;

void InitializeWidgetCommandList(){

   widgetCommandRegistry = new CommandRegistry(WIDGET);

	// Generic control 
	widgetCommandRegistry->add("active",     "control",       "active(\"true\"/\"false\")");
	widgetCommandRegistry->add("ctrlnr",     "control",       "ctrlnr(INT n)");
	widgetCommandRegistry->add("ctrlid",     "control",       "ctrlid(STR name)");
	widgetCommandRegistry->add("datatype",   "control",       "datatype(STR \"string\", \"integer\", \"float\", \"double\")");
   widgetCommandRegistry->add("draw",       "control",       "draw(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("drawobj",    "control",       "drawobj(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("enable",     "control",       "enable(\"true\"/\"false\")");
	widgetCommandRegistry->add("event",      "control",       "event(STR eventName)");
	widgetCommandRegistry->add("focus",      "control",       "STR \"true\"/\"false\" = focus()");
	widgetCommandRegistry->add("fgcolor",    "control",       "fgcolor(VEC [R,G,B])");
	widgetCommandRegistry->add("bgcolor",    "control",       "bgcolor(VEC [R,G,B])",true);
	widgetCommandRegistry->add("bkgcolor",   "control",       "bkgcolor(VEC [R,G,B])",true);
	widgetCommandRegistry->add("label",      "control",       "label(STR txt)");
	widgetCommandRegistry->add("menubar",    "control",       "menubar(VEC menu_nrs)");
	widgetCommandRegistry->add("valueid",    "control",       "valueid(STR valuename)");	
	widgetCommandRegistry->add("objnr",      "control",       "INT n = objnr()");
	widgetCommandRegistry->add("permanent",  "control",       "permanent(STR \"true\"/\"false\")",true);

	widgetCommandRegistry->add("procedure",  "control",       "procedure(STR callback procedure)");
	widgetCommandRegistry->add("range",      "control",       "range(VEC [min,max]");
	widgetCommandRegistry->add("region",     "control",       "region(VEC [left,right,top,bottom])");
	widgetCommandRegistry->add("readonly",   "control",       "readonly(\"true\"/\"false\")");
	widgetCommandRegistry->add("statusbox",  "control",       "statusbox(INT ctrlNr)");
	widgetCommandRegistry->add("tag",        "control",       "tag(STR user-definable tag)");
	widgetCommandRegistry->add("tabnr",      "control",       "tabnr(INT1 tab position)");
	widgetCommandRegistry->add("tabparent",  "control",       "tabparent([INT winNr, INT tabNr])");
   widgetCommandRegistry->add("controltype","control",       "STR type = controltype()");
	widgetCommandRegistry->add("toolbar",    "control",       "toolbar(INT objNr)");
	widgetCommandRegistry->add("tooltip",    "control",       "tooltip(STR tip)");
	widgetCommandRegistry->add("rmtooltip",  "control",       "rmtooltip()");
	widgetCommandRegistry->add("visible",    "control",       "visible(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("winnr",      "control",       "INT n = winnr()");
	widgetCommandRegistry->add("x",          "control",       "FLOAT value = x(STR expression/FLOAT value)",true);
	widgetCommandRegistry->add("y",          "control",       "FLOAT value = y(STR expression/FLOAT value)",true);
	widgetCommandRegistry->add("width",      "control",       "FLOAT value = width(STR expression/FLOAT value)",true); 
	widgetCommandRegistry->add("height",     "control",       "FLOAT value = height(STR expression/FLOAT value)",true);
	widgetCommandRegistry->add("wexp",       "control",       "STR expression = wexp(STR expression/FLOAT value)");
	widgetCommandRegistry->add("hexp",       "control",       "STR expression = hexp(STR expression/FLOAT value)");
	widgetCommandRegistry->add("xexp",       "control",       "STR expression = xexp(STR expression/FLOAT value)");
	widgetCommandRegistry->add("yexp",       "control",       "STR expression = yexp(STR expression/FLOAT value)");
	widgetCommandRegistry->add("uservar",    "control",       "uservar(VAR variable)");

	// Button
	widgetCommandRegistry->add("mode",       "button",        "mode(\"default\"/\"abort\"/\"panic\"/\"normal\"/\"cancel\")");
	widgetCommandRegistry->add("icon",       "button",        "icon(STR filename)");

	// Colorbox
	widgetCommandRegistry->add("color",      "control",       "color(VEC [R,G,B])",true);

   // Editor
	widgetCommandRegistry->add("text",                    "editor",         "text(STR txt)",true);
	widgetCommandRegistry->add("filename",                "editor",         "filename(STR name)",true);
	widgetCommandRegistry->add("getcurline",              "editor",         "INT nr = getcurline()",true);
	widgetCommandRegistry->add("getcursorpos",            "editor",         "INT pos = getcursorpos()",true);
	widgetCommandRegistry->add("getmacroname",            "editor",         "STR name = getmacroname()",true);
	widgetCommandRegistry->add("gettopline",              "editor",         "INT nr = gettopline()",true);
	widgetCommandRegistry->add("insert",                  "editor",         "insert(STR name)",true);
	widgetCommandRegistry->add("insertoffset",            "editor",         "insertoffset(INT offset)",true);
	widgetCommandRegistry->add("inserttext",              "editor",         "inserttext(STR name)",true);
	widgetCommandRegistry->add("pathname",                "editor",         "pathname(STR name)",true);
	widgetCommandRegistry->add("scrolltoline",            "editor",         "scrolltoline(INT line)",true);
	widgetCommandRegistry->add("setcursorpos",            "editor",         "setcursorpos(INT position)",true);
	widgetCommandRegistry->add("showcontextualmenu",      "editor",         "showcontextualmenu(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("showsyntaxcoloring",      "editor",         "showsyntaxcoloring(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("showsyntaxdescription",   "editor",         "showsyntaxdescription(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("syntaxcoloringstyle",     "editor",         "syntaxcoloringstyle(\"macro\"/\"asm\"/\"none\")",true);
	widgetCommandRegistry->add("wordwrap",                "editor",         "wordwrap(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("modified",                "editor",         "modified(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("readonlytext",            "editor",         "readonlytext(\"true\"/\"false\")",true);
   
	// HTML viewer											 
	widgetCommandRegistry->add("url",             "html",     "url(STR address");
	widgetCommandRegistry->add("goback",          "html",     "goback()");
   widgetCommandRegistry->add("text",            "html",     "text(STR text_to_add)");
   widgetCommandRegistry->add("clear",           "html",     "clear()");
   widgetCommandRegistry->add("javascript",      "html",     "javascript(STR script)");
	widgetCommandRegistry->add("goforward",       "html",     "goforward()");
	widgetCommandRegistry->add("copyselection",   "html",     "copyselection()");
	widgetCommandRegistry->add("runselection",    "html",     "runselection()");

	// Menu
	widgetCommandRegistry->add("nritems",    "menu",          "INT nr = nritems(N.A.)");
	widgetCommandRegistry->add("menu",       "menu",          "LIST n = menu(N.A.)");
	widgetCommandRegistry->add("menuname",   "menu",          "STR name = menuname(N.A.)");

	// Picture ctrl 
	widgetCommandRegistry->add("clearpicture",   "picture",       "clearpicture()");
	widgetCommandRegistry->add("file",           "picture",       "file(STR fileName)");
	widgetCommandRegistry->add("showframe",      "picture",       "showframe(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("resizetoframe",  "picture",       "resizetoframe(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("usebluescreen",  "picture",       "usebluescreen(STR \"true\"/\"false\")");

	// Picture statusbox 
	widgetCommandRegistry->add("syntaxwindow",  "statusbox",  "syntaxwindow(INT winNr)");
	widgetCommandRegistry->add("text",          "statusbox",  "text(STR txt)",true);
	widgetCommandRegistry->add("textregion",    "statusbox",  "textregion(STR \"INT region, STR txt\")",true);

	// Plot1d
	widgetCommandRegistry->add("subplot",       "plot1d",         "subplot(INT1 x, y)");
	widgetCommandRegistry->add("save",          "plot1d",         "save(STR fileName.pt1)");
	widgetCommandRegistry->add("tracemenu",     "plot1d",         "tracemenu(INT menuNr)");
	widgetCommandRegistry->add("bkgmenu",       "plot1d",         "bkgmenu(INT menuNr)");
	widgetCommandRegistry->add("axesmenu",      "plot1d",         "axesmenu(INT menuNr)");
   widgetCommandRegistry->add("keepsubplot",   "plot1d",         "keepsubplot(INT1 x, INT1 y)");
	widgetCommandRegistry->add("titlemenu",     "plot1d",         "titlemenu(INT menuNr)");
	widgetCommandRegistry->add("labelmenu",     "plot1d",         "labelmenu(INT menuNr)");
	widgetCommandRegistry->add("usedefaults",   "plot1d",         "usedefaults(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("multiplot",     "plot1d",         "multiplot(INT1 nrx, INT1 nry)");
	widgetCommandRegistry->add("tracexborder",  "plot1d",         "tracexborder(FLOAT borderFactor (0:1))");
	widgetCommandRegistry->add("traceyborder",  "plot1d",         "traceyborder(FLOAT borderFactor (0:1))");
	widgetCommandRegistry->add("filename",      "plot1d",         "STR name = filename()");
	widgetCommandRegistry->add("pathname",      "plot1d",         "STR name = pathname()");
	widgetCommandRegistry->add("indicatorsize", "plot1d",         "indicatorsize(sz/-1)");

   	// Plot2d
	widgetCommandRegistry->add("subplot",       "plot2d",         "subplot(INT1 x, y)");
	widgetCommandRegistry->add("load",          "plot2d",         "load(STR fileName.pt2)");
	widgetCommandRegistry->add("save",          "plot2d",         "save(STR fileName.pt2)");
	widgetCommandRegistry->add("tracemenu",     "plot2d",         "tracemenu(INT menuNr)");
	widgetCommandRegistry->add("bkgmenu",       "plot2d",         "bkgmenu(INT menuNr)");
	widgetCommandRegistry->add("axesmenu",      "plot2d",         "axesmenu(INT menuNr)");
   widgetCommandRegistry->add("keepsubplot",   "plot2d",         "keepsubplot(INT1 x, INT1 y)");
	widgetCommandRegistry->add("titlemenu",     "plot2d",         "titlemenu(INT menuNr)");
	widgetCommandRegistry->add("labelmenu",     "plot2d",         "labelmenu(INT menuNr)");
	widgetCommandRegistry->add("usedefaults",   "plot2d",         "usedefaults(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("multiplot",     "plot2d",         "multiplot(INT1 nrx, INT1 nry)");
	widgetCommandRegistry->add("filename",      "plot2d",          "STR name = filename()");
	widgetCommandRegistry->add("pathname",      "plot2d",          "STR name = pathname()");

	// Progressbar    
	widgetCommandRegistry->add("stepit",        "progbar",     "stepit()");
	widgetCommandRegistry->add("minvalue",      "progbar",     "minvalue(FLOAT min)");
	widgetCommandRegistry->add("maxvalue",      "progbar",     "maxvalue(FLOAT max)");

	// Radiobuttons
	widgetCommandRegistry->add("init",          "radiobutton",     "init(FLOAT/STR value)",true);
	widgetCommandRegistry->add("option_list",   "radiobutton",     "radiobutton",     "LIST lst = option_list()");
	widgetCommandRegistry->add("option_string", "radiobutton",     "radiobutton",     "STR txt = option_string()");
	widgetCommandRegistry->add("options",       "radiobutton",     "option(STR/LIST value)",true);
	widgetCommandRegistry->add("orientation",   "radiobutton",     "INT n = orientation(\"horizontal\"/\"vertical\")",true);
	widgetCommandRegistry->add("spacing",       "radiobutton",     "spacing(FLOAT number)",true);	

	// Slider
	widgetCommandRegistry->add("range",           "slider",     "range(FLOAT min, max)",true);
	widgetCommandRegistry->add("pagestep",        "slider",     "FLOAT number = pagestep()");
	widgetCommandRegistry->add("tickstep",        "slider",     "FLOAT number = tickstep()");
	widgetCommandRegistry->add("arrowstep",       "slider",     "FLOAT number = arrowstep()");

	// Static text
	widgetCommandRegistry->add("multiline",          "statictext",     "multiline(STR \"yes\"/\"no\")");
	widgetCommandRegistry->add("justification",      "statictext",     "justification(STR \"left\"/\"center\"/\"right\")");
	widgetCommandRegistry->add("text",               "statictext",     "text(STR txt)",true);

	// Text box 
	widgetCommandRegistry->add("password",        "textbox",     "password(STR \"on\"/\"off\")");
	widgetCommandRegistry->add("debug",           "textbox",     "debug(STR \"true\"/\"false\")",true);
	widgetCommandRegistry->add("text",          "textbox",         "text(STR txt)",true);
	widgetCommandRegistry->add("value",         "textbox",         "value(FLOAT number)",true);

	// Tab ctrl
	widgetCommandRegistry->add("inittabs",        "tab",     "inittabs(LIST tab_names)");
	widgetCommandRegistry->add("currenttab",      "tab",     "INT n = currenttab(INT0 tabNr/STR name)");
	widgetCommandRegistry->add("currentpage",     "tab",     "INT n = currentpage(INT0 tabNr/STR name)");
	widgetCommandRegistry->add("tablist",         "tab",     "LIST lst = tablist(LIST tab_names)");
	widgetCommandRegistry->add("tabname",         "tab",     "STR name = tabname()");
	widgetCommandRegistry->add("renametabs",      "tab",     "renametabs(LIST tab_names)");
	widgetCommandRegistry->add("index",          "tab",      "index(INT1 tab_number)");
	widgetCommandRegistry->add("zindex",          "tab",     "zindex(INT0 tab_number)");

	// Textmenu
	widgetCommandRegistry->add("index",          "textmenu",     "index(INT1 idx)");
	widgetCommandRegistry->add("menu",           "textmenu",     "menu(VEC/LIST items)");
	widgetCommandRegistry->add("zindex",         "textmenu",     "zindex(INT0 menu_number)");

	// List
	widgetCommandRegistry->add("index",          "listbox",     "index(INT1 list_index)");
	widgetCommandRegistry->add("list",           "listbox",     "list(LIST items)");
	widgetCommandRegistry->add("icons",          "listbox",     "icons(VEC icon_numbers/LIST icon_names)");
	widgetCommandRegistry->add("zindex",         "listbox",     "zindex(INT0 list_index)");

	// Updown ctrl
	widgetCommandRegistry->add("base",            "updown",     "base(FLOAT number)");
	widgetCommandRegistry->add("stepsize",        "updown",     "stepsize(FLOAT number)",true);
	widgetCommandRegistry->add("nrsteps",         "updown",     "nrSteps(INT number)");

	// 3D ctrl 
	widgetCommandRegistry->add("initialise",      "plot3d",     "initialise()");
	widgetCommandRegistry->add("titleupdate",     "plot3d",     "titleupdate(STR \"true\"/\"false\")");

  // title
	widgetCommandRegistry->add("name",            "title",      "font(STR name)");
	widgetCommandRegistry->add("style",           "title",      "style(STR style)");
	widgetCommandRegistry->add("color",           "title",      "color(VEC [r,g,b])");
	widgetCommandRegistry->add("size",            "title",      "size(INT point_size)");
	widgetCommandRegistry->add("font",            "title",      "font(STR name)");

  // xlabel
	widgetCommandRegistry->add("name",            "xlabel",      "font(STR name)");
	widgetCommandRegistry->add("style",           "xlabel",      "style(STR style)");
	widgetCommandRegistry->add("color",           "xlabel",      "color(VEC [r,g,b])");
	widgetCommandRegistry->add("size",            "xlabel",      "size(INT point_size)");
	widgetCommandRegistry->add("font",            "xlabel",      "font(STR name)");

   // ylabel
	widgetCommandRegistry->add("name",            "ylabel",      "font(STR name)");
	widgetCommandRegistry->add("style",           "ylabel",      "style(STR style)");
	widgetCommandRegistry->add("color",           "ylabel",      "color(VEC [r,g,b])");
	widgetCommandRegistry->add("size",            "ylabel",      "size(INT point_size)");
	widgetCommandRegistry->add("font",            "ylabel",      "font(STR name)");

	// Axes
	widgetCommandRegistry->add("autoscale",      "axes",      "autoscale(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("xppmscale",      "axes",      "xppmscale(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("yppmscale",      "axes",      "yppmscale(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("type",           "axes",       "type(STR type)");
	widgetCommandRegistry->add("fontname",       "axes",      "fontname(STR name)");
	widgetCommandRegistry->add("fontstyle",      "axes",      "fontstyle(STR style)");
	widgetCommandRegistry->add("axescolor",      "axes",      "axescolor(VEC [r,g,b])");
	widgetCommandRegistry->add("fontcolor",      "axes",      "fontcolor(VEC [r,g,b])");
	widgetCommandRegistry->add("fontsize",       "axes",      "fontsize(INT point_size)");
	widgetCommandRegistry->add("minaxisvalue",   "axes",      "minaxisvalue(FLOAT minimum_axis_value)");
	widgetCommandRegistry->add("maxaxisvalue",   "axes",      "maxaxisvalue(FLOAT maximum_axis_value)");
	widgetCommandRegistry->add("origminaxisvalue",   "axes",   "origminaxisvalue()");

   
	widgetCommandRegistry->add("xtickspacing",   "axes",      "xtickspacing(INT pixel_spacing)");
	widgetCommandRegistry->add("ytickspacing",   "axes",      "ytickspacing(INT pixel_spacing)");
	widgetCommandRegistry->add("xticksperlabel", "axes",      "xticksperlabel(INT number)");
	widgetCommandRegistry->add("yticksperlabel", "axes",      "yticksperlabel(INT number)");
	widgetCommandRegistry->add("smticksize",     "axes",      "smticksize(INT pixel_length)");
	widgetCommandRegistry->add("lgticksize",     "axes",      "lgticksize(INT pixel_length)");
	widgetCommandRegistry->add("xdirection",     "axes",      "xdirection(STR \"forward\"/\"reversed\")");
   widgetCommandRegistry->add("ydirection",     "axes",      "ydirection(STR \"forward\"/\"reversed\")");
	widgetCommandRegistry->add("xmapping",       "axes",      "xmapping(STR \"log\"/\"lin\")");
	widgetCommandRegistry->add("ymapping",       "axes",      "ymapping(STR \"log\"/\"lin\")");
	widgetCommandRegistry->add("xrange",         "axes",      "xrange([FLOAT minx, maxy])");
	widgetCommandRegistry->add("yrange",         "axes",      "yrange([FLOAT minx, maxy])");

	// Grid
	widgetCommandRegistry->add("xgrid",        "grid",      "xgrid(STR \"on\"/\"off\")");
	widgetCommandRegistry->add("ygrid",        "grid",      "ygrid(STR \"on\"/\"off\")");
	widgetCommandRegistry->add("finexgrid",    "grid",      "finexgrid(STR \"on\"/\"off\")");
	widgetCommandRegistry->add("fineygrid",    "grid",      "fineygrid(STR \"on\"/\"off\")");
	widgetCommandRegistry->add("gridcolor",    "grid",      "gridcolor(VEC  [r,g,b])");
	widgetCommandRegistry->add("finecolor",    "grid",      "finecolor(VEC  [r,g,b])");

	// Trace
	widgetCommandRegistry->add("axis",	          "trace",      "axis(STR \"left\", \"right\")");
	widgetCommandRegistry->add("ebararray",        "trace",      "ebararray(VEC array[~,2])");
	widgetCommandRegistry->add("ebarcolor",        "trace",      "ebarcolor(VEC  [r,g,b])");
	widgetCommandRegistry->add("ebarshow",         "trace",      "ebarshow(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("ebarfixed",        "trace",      "ebarfixed(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("ebarsize",         "trace",      "ebarsize(FLOAT user_units)");
   widgetCommandRegistry->add("getdata",          "trace",      "(VEC x,y) = getdata()");
	widgetCommandRegistry->add("id",               "trace",      "INT number = id()");
	widgetCommandRegistry->add("imagcolor",        "trace",      "imagcolor(VEC  [r,g,b])");
	widgetCommandRegistry->add("imagstyle",        "trace",      "imagstyle(STR style)");
	widgetCommandRegistry->add("inlegend",         "trace",      "inlegend(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("ignorexrange",     "trace",      "ignorexrange(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("ignoreyrange",     "trace",      "ignoreyrange(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("realcolor",        "trace",      "realcolor(VEC  [r,g,b])");
	widgetCommandRegistry->add("realstyle",        "trace",      "realstyle(STR style)");
	widgetCommandRegistry->add("realsymbolcolor",  "trace",      "realsymbolcolor(VEC  [r,g,b])");
	widgetCommandRegistry->add("imagsymbolcolor",  "trace",      "imagsymbolcolor(VEC  [r,g,b])");
	widgetCommandRegistry->add("setdata",          "trace",      "setdata(struct(VEC x, VEC y))");
	widgetCommandRegistry->add("symbolcolor",      "trace",      "symbolcolor(VEC  [r,g,b])");
	widgetCommandRegistry->add("symbolshape",      "trace",      "symbolshape(STR shape)");
	widgetCommandRegistry->add("symbolsize",       "trace",      "symbolsize(INT pixel_size)");
	widgetCommandRegistry->add("tracetype",        "trace",      "tracetype(STR type)");
	widgetCommandRegistry->add("tracewidth",       "trace",      "tracewidth(INT0 pixel_size)");
 	widgetCommandRegistry->add("uservar",          "trace",      "uservar(VAR variable)");
 	widgetCommandRegistry->add("whitewash",        "trace",      "whitewash(STR \"true\"/\"false\")");
 	widgetCommandRegistry->add("xoffset",          "trace",      "xoffset(FLOAT offset (0-1))");
 	widgetCommandRegistry->add("yoffset",          "trace",      "yoffset(FLOAT offset (0-1))");

	// Window
	widgetCommandRegistry->add("winnr",           "window",      "INT window_nr = winnr()");
	widgetCommandRegistry->add("ctrllist",        "window",      "VEC lst = ctrllist()");
	widgetCommandRegistry->add("macroname",       "window",      "STR name = macroname()");
	widgetCommandRegistry->add("macropath",       "window",      "STR path = macropath()");
	widgetCommandRegistry->add("nr",              "window",      "INT window_nr = nr()");
	widgetCommandRegistry->add("focus",           "window",      "focus(\"\"/INT objNr)");
	widgetCommandRegistry->add("hide",            "window",      "hide(\"\")");
	widgetCommandRegistry->add("show",            "window",      "show(\"\"/STR mode)");
	widgetCommandRegistry->add("nrctrls",         "window",      "INT nr = nrctrls()");
	widgetCommandRegistry->add("titlebarheight",  "window",      "INT pixel_height = titlebarheight()");
	widgetCommandRegistry->add("bordersize",      "window",      "INT pixel_size = bordersize()");
	widgetCommandRegistry->add("title",           "window",      "INT STR = name()");
	widgetCommandRegistry->add("name",            "window",      "INT STR = name()");
	widgetCommandRegistry->add("snaptogrid",      "window",      "snaptogrid(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("showgrid",        "window",      "showgrid(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("mainwindow",      "window",      "mainwindow(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("menubar",         "window",      "menubar(VEC menu_numbers)");
	widgetCommandRegistry->add("resizable",       "window",      "resizable(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("position",        "window",      "VEC pos = position()");
	widgetCommandRegistry->add("gridspacing",     "window",      "gridspacing(INT pixel_spacing)");
	widgetCommandRegistry->add("sizelimits",      "window",      "sizelimits(INT minWidth, maxWidth, minHeight, maxHeight)");
	widgetCommandRegistry->add("dimensions",      "window",      "dimensions(INT x, y, w, h)");
	widgetCommandRegistry->add("dimensionstxt",   "window",      "LIST lst = dimensionstxt()");

	// Plotregion
	widgetCommandRegistry->add("addannotation",    "plotregion",    "addannotation(STR text[, INT width, INT left, INT top])");
	widgetCommandRegistry->add("addimageinset",    "plotregion",    "addimageinset(STR filename[, INT width, INT left, INT top])");
	widgetCommandRegistry->add("addimageannotation","plotregion",    "addimageannotation(STR filename[, INT width, INT left, INT top])");
	widgetCommandRegistry->add("addline",          "plotregion",    "addline(INT x0, y0, x1, y1[, VEC color, INT width, STR style[, INT units]])");
	widgetCommandRegistry->add("addlines",         "plotregion",    "addlines(MAT vectors, VEC color, INT width, STR style])");
	widgetCommandRegistry->add("addtext",          "plotregion",    "addtext(MAT x_y_position, LIST/STR text, [MAT shift, STR fontName, INT fontSize, FLOAT fontAngle, STR  fontStyle, VEC color [, STR units]])");
 //  widgetCommandRegistry->add("allowtobecurrent", "plotregion",   "allowtobecurrent(\"true\", \"false\")");
   widgetCommandRegistry->add("antialiasing1d",   "plotregion",    "antialiasing1d(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("autorange",        "plotregion",    "autorange(\"true\"/\"false\")");
	widgetCommandRegistry->add("axes",             "plotregion",    "obj ax = axes() or  axes(STR parameter, VARIANT value)");
	widgetCommandRegistry->add("border",           "plotregion",    "STR vis = border()");
	widgetCommandRegistry->add("bordercolor",      "plotregion",    "bordercolor(VEC [R,G,B])",true);
	widgetCommandRegistry->add("bkgcolor",         "plotregion",    "bgkcolor(VEC [R,G,B])",true);
	widgetCommandRegistry->add("cmap",             "plotregion",    "cmap(VAR color_map])");
	widgetCommandRegistry->add("contour",          "plotregion",    "contour(INT number_levels/VEC levels, [INT mode])");
	widgetCommandRegistry->add("contourlinewidth", "plotregion",    "contourlinewidth(FLOAT lineWidth)");
   widgetCommandRegistry->add("copy",             "plotregion",    "copy()");
   widgetCommandRegistry->add("copytoclipboard",  "plotregion",    "copytoclipboard([STR \"current\"/\"all\")");
	widgetCommandRegistry->add("clear",            "plotregion",    "clear()");
	widgetCommandRegistry->add("currentaxis",      "plotregion",    "AXIS ax = currentaxis(\"left\"/\"right\")");
	widgetCommandRegistry->add("curtrace",         "plotregion",    "TRACE tc = currenttrace()");
	widgetCommandRegistry->add("dim",              "plotregion",    "INT d = dim()");
	widgetCommandRegistry->add("datamapping",      "plotregion",    "datamapping(\"log\"/\"linear\")");
	widgetCommandRegistry->add("datarange",        "plotregion",    "(FLOAT minValue, maxValue) = datarange()");
	widgetCommandRegistry->add("draw",             "plotregion",    "draw(\"true\"/\"false\")",true);
	widgetCommandRegistry->add("filtertrace",      "plotregion",    "filtertrace(\"true\", \"false\")");
	widgetCommandRegistry->add("filename",         "plotregion",    "filename()");
	widgetCommandRegistry->add("filepath",         "plotregion",    "filepath()");
	widgetCommandRegistry->add("fileversion",      "plotregion",    "INT version = fileversion()");
	widgetCommandRegistry->add("fullregion",       "plotregion",    "fullregion()");
	widgetCommandRegistry->add("getdata",          "plotregion",    "(VEC x,y) = getdata() or (MAT m, VEC x,y) = getdata(\"current\"/\"all\")");
	widgetCommandRegistry->add("getlines",         "plotregion",    "STRUCTARRAY result = getlines()");
	widgetCommandRegistry->add("gettext",          "plotregion",    "STRUCTARRAY result = gettext([float x [,float y]])");
	widgetCommandRegistry->add("grid",             "plotregion",    "OBJ gd = grid() or  grid(STR parameter, VARIANT value)");
	widgetCommandRegistry->add("hold",             "plotregion",    "hold(STR \"on\"/\"off\")");
	widgetCommandRegistry->add("inset",            "plotregion",    "inset(INT insetID)");
	widgetCommandRegistry->add("annotation",       "plotregion",    "annotation(INT annotationID)");
	widgetCommandRegistry->add("insets",           "plotregion",    "insets()");
	widgetCommandRegistry->add("annotations",      "plotregion",    "annotations()");
   widgetCommandRegistry->add("load",             "plotregion",    "load(STR fileName.pt1/2)");
	widgetCommandRegistry->add("save",             "plotregion",    "save(STR fileName.pt1/2)");
	widgetCommandRegistry->add("lockgrid",         "plotregion",    "lockgrid(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("parent",           "plotregion",    "OBJ plt = parent()",true);
	widgetCommandRegistry->add("paste",            "plotregion",    "paste()");
	widgetCommandRegistry->add("pasteinto",        "plotregion",    "pasteinto()");
	widgetCommandRegistry->add("plot",             "plotregion",    "INT tc = plot([VEC x],VEC y,[parameter_list])");
	widgetCommandRegistry->add("position",         "plotregion",    "(INT x,y) = position");
	widgetCommandRegistry->add("image",            "plotregion",    "image(MAT m, [STR colormap]) or image(MAT m, [VEC xscale, VEC yscale, [STR colormap]])");
	widgetCommandRegistry->add("imagerange",       "plotregion",    "imagerange(FLOAT minValue, maxValue)");
	widgetCommandRegistry->add("margins",          "plotregion",    "margins(INT left,top,right,base)");
	widgetCommandRegistry->add("rightyaxis",	    "plotregion",    "rightyaxis(\"true\", \"false\")");
	widgetCommandRegistry->add("showborder",       "plotregion",    "showborder(\"true\"/\"false\")");
	widgetCommandRegistry->add("rmannotations",    "plotregion",    "rmannotations(INT0 annotationID)");
	widgetCommandRegistry->add("rminset",          "plotregion",    "rminset(INT0 insetID)");
	widgetCommandRegistry->add("rmlines",          "plotregion",    "rmlines([x[,[y]])");
	widgetCommandRegistry->add("rmtext",           "plotregion",    "rmtext([x[,[y]])");
	widgetCommandRegistry->add("rmtrace",          "plotregion",    "rmtrace(INT0 traceID)");
 	widgetCommandRegistry->add("showcmap",         "plotregion",    "showcmap(\"true\"/\"false\")");
   widgetCommandRegistry->add("showlegend",       "plotregion",    "showlegend(\"true\", \"false\")");
   widgetCommandRegistry->add("showreal",         "plotregion",    "showreal(\"true\", \"false\")");
   widgetCommandRegistry->add("showimag",         "plotregion",    "showimag(\"true\", \"false\")");
	widgetCommandRegistry->add("syncaxes",		    "plotregion",    "syncaxes(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("title",            "plotregion",    "OBJ lbl = title() or title(STR txt) or title(STR parameter, VARIANT value)",true);
	widgetCommandRegistry->add("titleupdate",      "plotregion",    "titleupdate(INT menuNr)",true);
   widgetCommandRegistry->add("trace",            "plotregion",    "OBJ trc = trace(INT0 id) or  trace(STR parameter, VARIANT value)");
	widgetCommandRegistry->add("tracepref",        "plotregion",    "tracepref(STR parameter, VARIANT value})");
	widgetCommandRegistry->add("tracelist",        "plotregion",    "VEC ids = tracelist()");
	widgetCommandRegistry->add("xlabel",           "plotregion",    "OBJ lbl = xlabel() or xlabel(STR txt) or xlabel(STR parameter, VARIANT value)");
	widgetCommandRegistry->add("ylabel",           "plotregion",    "OBJ lbl = ylabel() or ylabel(STR txt) or ylabel(STR parameter, VARIANT value)");
	widgetCommandRegistry->add("ylabelleft",       "plotregion",    "OBJ lbl = ylabelleft() or ylabelleft(STR txt) or ylabelleft(STR parameter, VARIANT value)");
	widgetCommandRegistry->add("ylabelright",      "plotregion",    "OBJ lbl = ylabelright() or ylabelright(STR txt) or ylabelright(STR parameter, VARIANT value)");
	widgetCommandRegistry->add("ylabelvert",       "plotregion",    "ylabelvert(STR \"true\"/\"false\")");
	widgetCommandRegistry->add("zoom",             "plotregion",    "zoom(FLOAT minx, maxx, miny, maxy)");
	widgetCommandRegistry->add("zoombkgcolor",     "plotregion",    "zoombkgcolor(VEC [R,G,B,A])");
	widgetCommandRegistry->add("zoombordercolor",  "plotregion",    "zoombordercolor(VEC [R,G,B,A])");
	widgetCommandRegistry->add("zoomrectmode",     "plotregion",    "zoomrectmode(STR \"dottedrect\"/\"solidrect\")");

	// Grid control
	widgetCommandRegistry->add("set",					"gridctrl",		"set(INT col, INT row, VAR value)");
	widgetCommandRegistry->add("get",					"gridctrl",		"STR result = get(VAR col, VAR row)");
	widgetCommandRegistry->add("clear",				"gridctrl",		"clear()");
	widgetCommandRegistry->add("label",				"gridctrl",		"label(VAR collabel, VAR rowlabel)");
	widgetCommandRegistry->add("protect",			"gridctrl",		"protect(VAR column, VAR row, [true | false])");
	widgetCommandRegistry->add("colwidth",			"gridctrl",		"colwidth(INT column, INT width)");
	widgetCommandRegistry->add("columns",			"gridctrl",		"INT cols = columns() or columns(INT cols)");
	widgetCommandRegistry->add("rows",				"gridctrl",    "INT rows = rows() or rows(INT rows)");
	widgetCommandRegistry->add("colheaderheight",	"gridctrl",    "INT height = colheaderheight() or colheaderheight(INT height)");
	widgetCommandRegistry->add("rowheaderwidth",	"gridctrl",    "INT width = rowheaderwidth() or rowheaderwidth(INT width)");
	widgetCommandRegistry->add("showcollabels",	"gridctrl",    "BOOL show = showcollabels() or showcollabels(true | false)");
	widgetCommandRegistry->add("showrowlabels",	"gridctrl",    "BOOL show = showrowlabels() or showrowlabels(true | false)");

	// Inset control
	widgetCommandRegistry->add("hide",            "inset",       "hide(\"true\"/\"false\")");
	widgetCommandRegistry->add("setcontents",     "inset",       "setcontents(STR contents)");
	widgetCommandRegistry->add("setwidth",        "inset",       "setwidth(INT width)");
	widgetCommandRegistry->add("show",            "inset",       "show(\"true\"/\"false\")");
	widgetCommandRegistry->add("position",        "inset",       "position(VEC x_y)");
	widgetCommandRegistry->add("corner",          "inset",       "corner(INT 1/2/3/4) or corner(STR \"top/bottom left/right\")");

   // Divider
   widgetCommandRegistry->add("limits",           "divider",       "limits(VEC [min,max]) or limits(LIST [min,max])");
   widgetCommandRegistry->add("orientation",      "divider",       "orientation(\"horizontal\"/\"vertical\")");

}

bool IsProcessClassFunction(char name[])
{   
   ToLowerCase(name);
	return widgetCommandRegistry->has(name);
}

// Returns the syntax string for an internal class or null string
void GetClassCommandSyntax(char name[], char **syntax)
{   
   ToLowerCase(name);
	const string *st = (widgetCommandRegistry->syntaxFor(name));

	if (!st)
	{
		*syntax = 0;
		return;
	}

	const char* s = st->c_str();
	*syntax = new char[strlen(s)+50];
	sprintf(*syntax, "CLASS FUNCTION SYNTAX: %s", s);  
}

const char* GetClassCommandHelpType(char name[])
{
	static char result[100];
   ToLowerCase(name);

	if (!widgetCommandRegistry->has(name))
	{
		return NULL;
	}

	sprintf(result,"%s",widgetCommandRegistry->associatedWidgetNameFor(name)->c_str());
   return(result);
}

bool GetClassCommandVariousness(char name[])
{
   ToLowerCase(name);
	return widgetCommandRegistry->isVarious(name);
   
}