#include "stdafx.h"
#include "command_other.h"
#include "allocate.h"
#include "cache.h"
#include "cArg.h"
#include "cli_events.h"
#include "cli_files.h"
#include "command.h"
#include "command_other.h"
#include "CmdInfo.h"
#include "CommandRegistry.h"
#include "control.h"
#include "debug.h"
#include "defineWindows.h"
#include "dialogs.h"
#include "dll.h"
#include "drawvector.h"
#include "edit_class.h"
#include "edit_files.h"
#include "edit_utilities.h"
#include "environment.h"
#include "evaluate.h"
#include "evaluate_complex.h"
#include "events_edit.h"
#include "export_data_1d.h"
#include "export_data_2d.h"
#include "export_data_3d.h"
#include "files.h"
#include "fourier.h"
#include "globals.h"
#include "gridcontrol.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiMakeObjects.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "import_export_utilities.h"
#include "import_data_1d.h"
#include "import_data_2d.h"
#include "import_data_3d.h"
#include "integration.h"
#include "interface.h"
#include "license.h"
#include "list_functions.h"
#include "load_save_data.h"
#include "macro_class.h"
#include "main.h"
#include "message.h"
#include "mymath.h"
#include "mytime.h"
#include "phase.h"
#include "plotText.h"
#include "plot.h"
#include "plot1dCLI.h"
#include "plot1dEvents.h"
#include "plot2dCLI.h"
#include "plot3dEvents.h"
#include "plot3dSurface.h"
#include "plotCLI.h"
#include "preferences.h"
#include "process.h"
#include "scanstrings.h"
#include "sounds.h"
#include "string_utilities.h"
#include "structure.h"
#include "thread.h"
#include "trie.h"
#include "utilities.h"
#include "variablesOther.h"
#include <direct.h>
#include <errno.h>
#include <htmlhelp.h>
#include <shellapi.h>
#include <process.h>
#include <string>
#include <vector>
#include <algorithm>
#include "memoryLeak.h"

using std::string;
using std::vector;

#pragma warning (disable: 4996) // Ignore deprecated library functions

#define VERSION 3.128 // Version number used by getversion() command
#define VERSION_DATE "24-February-2025"

// Define run mode

#define EXECUTE_COMMAND 1
#define PROMPT_USER     2

// Functions called by Prospa commands
int AdjustObjects(Interface *itfc, char *args);
int AddPrefix(Interface* itfc ,char args[]);
int Ceiling(Interface* itfc ,char args[]);
int CopyAFile(Interface* itfc ,char args[]);
int ExecuteFile(Interface* itfc, char args[]);
int GetCWD(Interface *itfc, char args[]);
int GetProspaVersion(Interface* itfc ,char args[]);
int GetMainWindow(Interface* itfc ,char args[]);
int Histogram(Interface* itfc ,char args[]);
int JoinFiles(Interface* itfc ,char args[]);
int Floor(Interface* itfc ,char arg[]);
int ListCommands(Interface* itfc ,char arg[]);
int ListFiles(Interface* itfc ,char argIn[]);
int MakeDirectory(Interface* itfc ,char args[]);
int MoveAFile(Interface* itfc ,char args[]);
int OffDiagonalMatrix(Interface* itfc ,char args[]);
int RemoveDirectory(Interface* itfc ,char args[]);
int RemoveFile(Interface* itfc ,char args[]);
int ReturnFromProcedure(Interface* itfc ,char args[]);
int SetCWD(Interface *itfc, char args[]);
int SetFonts(Interface* itfc, char args[]);
int SoundBell(Interface* itfc ,char[]);
int SortMatrixRows(Interface* itfc ,char arg[]);
int StartProcedure(Interface* itfc ,char args[]);
int UseQuotedStrings(Interface* itfc, char args[]);
int GetFontList(Interface* itfc ,char args[]);
int SetStructureValues(Interface* itfc ,char args[]);
int SetPlotScaleFactor(Interface* itfc ,char args[]);
int RenameListEntry(Interface* itfc ,char args[]);
int SplitParameterString(Interface* itfc ,char args[]);
int DrawCone(Interface* itfc ,char args[]);
int MakeMovie(Interface* itfc , char args[]);
int AddMovieFrame(Interface* itfc , char args[]);
int EndMovie(Interface* itfc , char args[]);
int AddLineToMatrix(Interface* itfc , char args[]);
int PseudoLogBin(Interface* itfc ,char args[]);
int PseudoLogBinD(Interface* itfc ,char args[]);
int PseudoLogVector(Interface* itfc ,char args[]);
int PseudoLogVectorD(Interface* itfc ,char args[]);
int LogVector(Interface* itfc ,char args[]);
int GetLicenseInfo(Interface* itfc ,char args[]);
short CheckForMacroProcedure(Interface *itfc, char *command, char *args);
short CheckForVariableProcedure(Interface *itfc, char *command, char *arg);
short CheckForClassProcedure(Interface *itfc, char *command, char *arg);
short RunClassProc(Interface *itfc, Variable *var, char *func, char *args);
int StringToAscii(Interface* itfc ,char args[]);
int AsciiToString(Interface* itfc ,char args[]);
int LawsonHansonNNLS2D(Interface* itfc ,char args[]);
int ThrowException(Interface* itfc ,char args[]);
int FISTA2D(Interface* itfc ,char args[]);
int testFunc(Interface *itfc, char *args);
int ExclusiveOr(Interface *itfc, char *args);
char gCommand[MAX_STR]; // Copy of current command, Externally accessible
char helpviewer[MAX_PATH] = "iexplore"; // Externally accessible
extern int DrawWaterFallPlot(Interface* itfc ,char args[]);
extern int TestCode(Interface* itfc ,char args[]);
extern int GetSID(Interface* itfc ,char args[]);
int LogBin(Interface *itfc, char args[]);
int MatrixMean(Interface* itfc ,char args[]);
int MatrixStandardDeviation(Interface* itfc ,char args[]);
extern int CopyPlot(Interface* itfc, char args[]);
extern int PastePlot(Interface* itfc, char args[]);
extern int PasteIntoPlot(Interface* itfc, char args[]);
int RemoveFolder(Interface *itfc, char args[]);
int IsObject(Interface* itfc ,char args[]);
int SaveWaveFile(Interface* itfc, char args[]);
int UpdateEditorParentTitle(Interface* itfc ,char args[]);
int IsIntegerCLI(Interface* itfc ,char args[]);
int IsFloatCLI(Interface* itfc ,char args[]);
int IsDoubleCLI(Interface* itfc ,char args[]);
int SetParameterListValues(Interface* itfc ,char arg[]);
int IsKeyPressed(Interface* itfc ,char arg[]);
int HTMLReplaceString(Interface* itfc ,char arg[]);
int HTMLCountSubString(Interface* itfc ,char arg[]);
int MakeControl(Interface* itfc ,char arg[]);
int PlotViewVersion(Interface* itfc ,char arg[]);
int WaitforThread(Interface* itfc ,char arg[]);
int CleanUpThread(Interface* itfc ,char arg[]);
int Convert1DListTo2D(Interface* itfc ,char arg[]);
int Convert2DListTo1D(Interface* itfc ,char arg[]);
int MakeDirectoryCore(char *newDir);
int GetRegistryInfo(Interface* itfc ,char arg[]);
int PrintProcedureStack(Interface* itfc, char arg[]);
int GetCurrentMacroLineNr(Interface *itfc, char *args);
int SelectEditorLine(Interface *itfc, char *args);
int SetEditorRenderMode(Interface *itfc, char *args);
int SearchDLLs(Interface *itfc, char *args);
int CopyAFolder(Interface* itfc ,char args[]);
int RunRemoteMacro(Interface* itfc, char args[]);

extern int IsPlotIdle(Interface *itfc, char *args);
extern int ComplexToReal(Interface *itfc, char arg[]);
extern int IgnoreDLL(Interface* itfc ,char args[]);
extern int IgnoreDLLs(Interface* itfc, char args[]);
extern int RemoveCachedMacro(Interface *itfc,  char args[]);
extern int RemoveCachedMacros(Interface *itfc,  char args[]);
extern int ListCachedProcedures(Interface *itfc,  char args[]);
extern int ReturnSign(Interface *itfc,  char args[]);
extern int GetProcedureNames(Interface* itfc, char args[]);
extern int ShowErrors(Interface* itfc, char args[]);
//int ShowMacroProcedures(Interface* itfc ,char args[]);
int AssignTest(Interface *itfc, char args[]);
int ExecuteAndWait(Interface* itfc, char args[]);
int SendToExistingProspa(Interface* itfc, char args[]);
extern int PrintToString(Interface* itfc, char args[]);
int GetStructureItem(Interface *itfc, char arg[]);
int MakeClass(Interface *itfc, char arg[]);
int DealiasVariable(Interface *itfc, char arg[]);
int ImportMacro(Interface *itfc, char arg[]);
int ConvertToDecimal(Interface *itfc, char arg[]);
int GetArgumentNames(Interface *itfc, char arg[]);
int FindXValue(Interface *itfc, char arg[]);
extern int MakeLink(Interface *itfc, char args[]);
int GetStructureItemIndex(Interface *itfc, char arg[]);
extern int MatrixRMS(Interface *itfc, char arg[]);
//int CheckStructure(Interface *itfc, char arg[]);
int IsProcedureAvailable(Interface *itfc, char arg[]);
int MakeStructureArray(Interface *itfc, char arg[]);
extern int MakeParameterStructure(Interface* itfc ,char arg[]);
int IsApplicationOpen(Interface *itfc, char arg[]);
int TrimString(Interface *itfc, char arg[]);
int ValidFileName(Interface *itfc, char args[]);
int SetClassInfo(Interface *itfc, char args[]);
int AppendToStructure(Interface *itfc, char args[]);
extern int FindExactIndex(Interface *itfc, char args[]);
extern int TextEditFunctions(Interface *itfc, char args[]);
extern int FixListFaults(Interface *itfc, char args[]);
extern int ReplaceSubExpressions(Interface *itfc, char args[]);
extern int SetOrGetCurrentEditor(Interface *itfc, char args[]);
extern int FindMacro(Interface *itfc, char args[]);
extern int SetOrGetCurrentCLI(Interface *itfc, char args[]);
extern int EvaluateHexString(Interface *itfc, char args[]);
extern int IsFileCached(Interface* itfc, char args[]);
extern int DisplayProcInfo(Interface* itfc, char args[]);
extern int GetArgument(Interface* itfc, char args[]);
extern int FindProspaWindows(Interface* itfc, char args[]);
extern int DefinePlotCallback(Interface* itfc, char args[]);

int CopyToClipBoard(Interface *itfc, char args[]);

int DoNothing(Interface *itfc, char args[]);
int SetUISkin(Interface *itfc, char arg[]);

bool searchDLLs = true;

int nProc(Interface* itfc ,char args[])
{
   return(OK);
}

/**************************************************************************************
* Store command names and associated functions                                        *
**************************************************************************************/
 
CommandRegistry *prospaCommandRegistry;


/**************************************************************************************
*           Load up commands																			  *
**************************************************************************************/

void InitializeProspaCommandList()
{
   prospaCommandRegistry = new CommandRegistry(COMMAND);

   prospaCommandRegistry->add("aa3d",               AntiAlias3D,             THREED_CMD,                           "aa3d(STR on/off)");
   prospaCommandRegistry->add("abort",              Abort,                   CONTROL_COMMAND | MACRO_CMD,          "abort(STR message)");
   prospaCommandRegistry->add("abs",                Magnitude,               MATH_CMD,                             "NUM y = abs(NUM x)");
   prospaCommandRegistry->add("abortonerror",       AbortOnError,            CONTROL_COMMAND | MACRO_CMD,          "abortonerror(STR \"true/false\")");
   prospaCommandRegistry->add("acos",               ArcCosine,               TRIG_CMD | MATH_CMD,                  "NUM y = acos(NUM x)");
   prospaCommandRegistry->add("activatewindow",     ActivateWindow,          GUI_CMD,                              "activatewindow(STR \"true/false\")");
   prospaCommandRegistry->add("addprefix",          AddPrefix,               LIST_CMD,                             "LIST result = addprefix(VARIABLE list, STR prefix)");
   prospaCommandRegistry->add("adjustctrls",        AdjustObjects,           GUI_CMD,                              "adjustobjs(INT window_nr)");
   prospaCommandRegistry->add("addframe",           AddMovieFrame,           GENERAL_COMMAND,                      "addframe(STR window)");
   prospaCommandRegistry->add("alias",              MakeVariableAlias,       VAR_CMD,                              "VAR y = alias(VAR x, STR \"eval/noeval\")");
   prospaCommandRegistry->add("alignobj",           AlignObjects,            GUI_CMD,                              "alignobj(STR align_mode)");
   prospaCommandRegistry->add("allowvariables",     AllowNonLocalVariables,  VAR_CMD,                              "AllowNonLocalVariables([STR type1, [STR type2]])");
   prospaCommandRegistry->add("asin",               ArcSine,                 TRIG_CMD | MATH_CMD,                  "NUM y = asin(NUM x)");
   prospaCommandRegistry->add("assign",             AssignSpecialVariable,   VAR_CMD,                              "assign(STR variable, CONST-STR expression, [STR scope])");
   prospaCommandRegistry->add("assignlock",         AssignWithLock,          VAR_CMD,                              "assignlock(STR variable, CONST-STR expression, [STR scope])");
   prospaCommandRegistry->add("asciitostr",         AsciiToString,           STRING_CMD,                           "STR result asciitostr(FLOAT/VEC input)");
   prospaCommandRegistry->add("assignlist",         AssignParameterList,     LIST_CMD,                             "assignlist(LIST/STRUCT/STR variable)");
   prospaCommandRegistry->add("assignctrls",        AssignControlObjects,    GUI_CMD,                              "assignctrls(INT window_nr)");
   prospaCommandRegistry->add("assignstruct",       AssignStructure,         STRUCT_CMD,                           "assignstruct(STRUCT variable)");
   prospaCommandRegistry->add("atan",               ArcTangent,              TRIG_CMD | MATH_CMD,                  "NUM y = atan(NUM x)");
   prospaCommandRegistry->add("attachobj",          AttachObjects,           GUI_CMD,                              "attachobj(STR attach_mode)");
   prospaCommandRegistry->add("autophase",          AutoPhase,               MATH_CMD | MATRIX_CMD,                "FLOAT phase = autophase(VEC y, INT left, INT right, [STR method])");
   prospaCommandRegistry->add("autorange",          AutoRange,               TWOD_CMD,                             "autorange(STR \"on/off\")");
   prospaCommandRegistry->add("avg",                MatrixMean,              MATH_CMD,                             "float result = avg(MAT/VEC input)");
   prospaCommandRegistry->add("axis3d",             Draw3DAxis,              THREED_CMD,                           "axis3d(STR direction, VEC plot-range, FLOAT intercept1, FLOAT intercept VEC label-range, STR label, STR label-position, STR axes-direction)");
   prospaCommandRegistry->add("axispar3d",          Set3DLabelSizes,         THREED_CMD,                           "axispar3d(FLOAT short_tick, FLOAT long_tick, FLOAT number_size, FLOAT label_size)"); 
   prospaCommandRegistry->add("axes3d",             Draw3DAxes,              THREED_CMD,                           "axes3d(VEC origin, FLOAT length, [RGB colour, [FLOAT label_size]])");
   prospaCommandRegistry->add("axes",               ModifyAxes,              ONED_CMD | TWOD_CMD,                  "axes(STR parameter, VARIANT value, ...)");

   prospaCommandRegistry->add("bell",               SoundBell,               SOUND_CMD,                            "bell()");
   prospaCommandRegistry->add("bordercolor",        SetBorderColor,          ONED_CMD | TWOD_CMD,                  "bordercolor(VEC rgb)");
   prospaCommandRegistry->add("bkcolor",            SetBkColor,              ONED_CMD | TWOD_CMD,                  "bkcolor(VEC rgb)");
   prospaCommandRegistry->add("bkgcolor",           SetBkColor,              ONED_CMD | TWOD_CMD,                  "bkcolor(VEC rgb)");
   prospaCommandRegistry->add("bkcolor3d",          Set3DBkColor,            THREED_CMD,                           "bkcolor3d(VEC rgb)");
   prospaCommandRegistry->add("bkgcolor3d",         Set3DBkColor,            THREED_CMD,                           "bkcolor3d(VEC rgb)");
   prospaCommandRegistry->add("box3d",              DrawBox,                 THREED_CMD,                           "box(FLOAT xmin,FLOAT xmax,FLOAT ymin,FLOAT ymax,FLOAT zmin,FLOAT zmax)");
   prospaCommandRegistry->add("button",             MakeButton,              GUI_CMD,                              "OBJ name = button(INT control_nr, INT x, INT y, INT width, INT height, STR caption, [LIST commands])");

   prospaCommandRegistry->add("cachemacro",         CacheMacro,              GENERAL_COMMAND,                      "cachemacro(STR macro_name, STR \"window/local\")");
   prospaCommandRegistry->add("cacheproc",          CacheProcedures,         GENERAL_COMMAND,                      "cacheproc(STR \"true\"/\"false\")");
   prospaCommandRegistry->add("callstack",          PrintProcedureStack,     DEBUG_CMD,                            "callstack()");
   prospaCommandRegistry->add("catch",              CatchError,              CONTROL_COMMAND,                      "catch");
   prospaCommandRegistry->add("caseset",            SetStringCase,           STRING_CMD,                           "STR out = caseset(STR in, STR mode)");
   prospaCommandRegistry->add("cd",                 SetCWD,                  FILE_CMD,                             "cd(STR directory)");
   prospaCommandRegistry->add("ceil",               Ceiling,                 MATH_CMD,                             "NUM y = ceil(NUM x)");   
   prospaCommandRegistry->add("checkbox",           MakeCheckBox,            GUI_CMD,                              "OBJ name = checkbox(INT control_nr, INT x, INT y, STR states, STR initial_value, [LIST commands])");
   prospaCommandRegistry->add("checkcontrols",      CheckControlValues,      GUI_CMD | LIST_CMD,                   "STR error = checkcontrols(INT window_nr, [STR method = \"list\"/\"prefix\"/\"all\", VAR variable])");
   prospaCommandRegistry->add("checkctrlvalues",    CheckControlValues,      GUI_CMD | LIST_CMD,                   "STR error = checkctrlvalues(INT window_nr, [STR method = \"list\"/\"prefix\"/\"all\", VAR variable])");
//   prospaCommandRegistry->add("checkstruct",        CheckStructure,          GENERAL_COMMAND,                      "INT sz = checkstruct(STRUCT variable)");
   prospaCommandRegistry->add("circle",             AddCircleToMatrix,       MATRIX_CMD,                           "MAT mout = circle(MAT min, INT x0, INT y0, INT r, FLOAT a0)");
   prospaCommandRegistry->add("class",              MakeClass,               CLASS_CMD,                            "CLASS obj = class(STR \"class_initialiser\")");
   prospaCommandRegistry->add("classinfo",          SetClassInfo,            CLASS_CMD,                            "classinfo(LIST class_info)");
   prospaCommandRegistry->add("clear",              Clear,                   GUI_CMD,                              "clear(STR \"cli/edit/plot\")");
   prospaCommandRegistry->add("clear1d",            Clear1D,                 ONED_CMD,                             "clear1d()");
   prospaCommandRegistry->add("clear2d",            Clear2D,                 TWOD_CMD,                             "clear2d()");
   prospaCommandRegistry->add("clear3d",            Clear3D,                 THREED_CMD,                           "clear3d()");
   prospaCommandRegistry->add("cli",                MakeCLI,                 GUI_CMD,                              "OBJ name = cli(INT control_nr, INT x, INT y, INT width, INT height)");
   prospaCommandRegistry->add("clip3d",             Set3DClippingPlane,      THREED_CMD,                           "clip3d(INT number, INT position, STR plane, STR side)");
   prospaCommandRegistry->add("clip3dstatus",       Set3DClippingStatus,     THREED_CMD,                           "clip3dstatus(STR \"on/off\")");
   prospaCommandRegistry->add("clonearray",         CloneArray,              MATRIX_CMD,                           "MAT m = clonearray(VEC v, INT w, int h)");
   prospaCommandRegistry->add("copytoclipboard",    CopyToClipBoard,         GENERAL_COMMAND,                      "copytoclipboard(STR value)");
   prospaCommandRegistry->add("closedialog",        CloseDialog,             GUI_CMD,                              "closedialog(var1, var2 ...)");  
   prospaCommandRegistry->add("closeprint",         ClosePrintFile,          FILE_CMD,                             "closeprint()");  
   prospaCommandRegistry->add("closewindow",        DestroyMyWindow,         GUI_CMD,                              "closewindow(INT window_nr)");                           
   prospaCommandRegistry->add("cmap",               SetColorMap,             TWOD_CMD,                             "cmap(STR colormap)");                           
   prospaCommandRegistry->add("cmap3d",             Draw3DColorScale,        THREED_CMD,                           "cmap3d(MAT scale, VEC range, VEC font-color, FLOAT font-size)");
   prospaCommandRegistry->add("cmatrix",            NewComplexMatrix,        MATRIX_CMD,                           "CMAT m = cmatrix(INT width [,INT height [,INT depth [,INT hyperdepth]])");
   prospaCommandRegistry->add("cmpstr",             CompareStrings,          STRING_CMD,                           "INT result = cmpstr(STR s1,STR s2)");
   prospaCommandRegistry->add("color3d",            Set3DColor,              THREED_CMD,                           "color3d(VEC rgb)");
   prospaCommandRegistry->add("colorbox",           MakeColorBox,            GUI_CMD,                              "OBJ name = colorbox(INT control_nr, INT x, INT y, INT width, INT height, VEC default_colours, [LIST commands])");
   prospaCommandRegistry->add("cone",               DrawCone,                THREED_CMD,                           "cone(VEC base,VEC top,FLOAT radius,RGB colour)"),                           
   prospaCommandRegistry->add("conj",               ComplexConjugate,        MATH_CMD,                             "COMPLEX y = conj(COMPLEX x) or CMAT y = conj(CMAT x)");
   prospaCommandRegistry->add("contour",            ContourPlot,             TWOD_CMD,                             "contour(INT number/VEC levels, [INT mode(0/1/2/3/4) [,VEC fixed_color, INT use_fixed_color(0/1)]])");
   prospaCommandRegistry->add("copydir",            CopyAFolder,             FILE_CMD,                             "copydir(STR source, STR destination)");
   prospaCommandRegistry->add("copyfile",           CopyAFile,               FILE_CMD,                             "copyfile(STR source, STR destination [, STR check])");
   //prospaCommandRegistry->add("copyfolder",         CopyAFolder,             FILE_CMD,                             "copyfolder(STR source, STR destination)");
   prospaCommandRegistry->add("copyplot",           CopyPlot,                ONED_CMD | TWOD_CMD,                  "copyplot([CLASS plot_region])");
   prospaCommandRegistry->add("convolve",           Convolve,                MATRIX_CMD,                           "VEC dataIn = convolve(VEC dataIn, VEC kernel [, STR symmetrical = \"true\"/\"false\"])");
   prospaCommandRegistry->add("convertcolor",       ConvertColor,            COLOR_CMD,                            "VEC colOut = convertcolor(VEC colIn)");
   prospaCommandRegistry->add("cos",                Cosine,                  MATH_CMD | TRIG_CMD,                  "NUM y = cos(NUM x)");
   prospaCommandRegistry->add("cosh",               HyperbolicCosine,        MATH_CMD | TRIG_CMD,                  "NUM y = cosh(NUM x)");
   prospaCommandRegistry->add("ctor",               ComplexToReal,           MATH_CMD,                             "VEC y = ctor(CVEC x)");
   prospaCommandRegistry->add("cumsum",             CumulativeSum,           MATRIX_CMD,                           "VEC y = cumsum(VEC x [, INT dim])");
   prospaCommandRegistry->add("curplot",            SetOrGetCurrentPlot,     ONED_CMD | TWOD_CMD,                  "CLASS rg = curplot(), curplot(CLASS rg), curplot(STR 1d/2d[, INT x, INT y])");
   prospaCommandRegistry->add("currentaxis",        SetOrGetCurrentAxis,     ONED_CMD | TWOD_CMD,                  "STR s = currentaxis(STR \"left\", \"right\")");
   prospaCommandRegistry->add("curaxis",            SetOrGetCurrentAxis,     ONED_CMD | TWOD_CMD,                  "STR s = curaxis(STR \"left\", \"right\")");
   prospaCommandRegistry->add("curcli",             SetOrGetCurrentCLI,      GUI_CMD,                              "curcli(INT winNr, INT cliNr)");
   prospaCommandRegistry->add("cureditor",          SetOrGetCurrentEditor,   GUI_CMD,                              "cureditor(INT winNr, INT editNr)");
   prospaCommandRegistry->add("curtrace",           GetOrSetCurrentTrace,    ONED_CMD,                             "CLASS tc = curtrace(), curtrace(CLASS tc), curtrace(INT trace_id)");
   prospaCommandRegistry->add("curwin",             GetOrSetCurrentWindow,   GUI_CMD,                              "CLASS win = curwin(INT win_nr/ CLASS window)");
   prospaCommandRegistry->add("cylinder",           Draw3DCylinder,          THREED_CMD,                           "cylinder(VEC base,VEC top,FLOAT radius,RGB colour)");

   prospaCommandRegistry->add("datarange3d",        Set3DDataRange,          THREED_CMD,                           "datarange(FLOAT xmin,FLOAT xmax,FLOAT ymin,FLOAT ymax,FLOAT zmin,FLOAT zmax)");
   prospaCommandRegistry->add("dealias",            DealiasVariable,         GENERAL_COMMAND,                      "dealias()");
   prospaCommandRegistry->add("debug",              DebugInterface,          HIDDEN_COMMAND,                       "debug(STR command)");
   prospaCommandRegistry->add("dec",                ConvertToDecimal,        GENERAL_COMMAND,                      "INT decimal = dec(INT hex_number, INT bits)");
   prospaCommandRegistry->add("decimate",           DecimateVector,          MATRIX_CMD,                           "VEC y = decimate(VEC x, INT offset, INT step)");
//   prospaCommandRegistry->add("decrypt",            Decrypt,                 GENERAL_COMMAND,                      "STR output = decrypt(STR input)");
   prospaCommandRegistry->add("depthcue",           DepthCue,                THREED_CMD,                           "depthcue(STR \"on/off\")");
   prospaCommandRegistry->add("depthcuerange",      SetFogRange,             THREED_CMD,                           "fogrange(FLOAT start, FLOAT end)");
   prospaCommandRegistry->add("diag",               DiagonalMatrix,          THREED_CMD,                           "MAT out = diag(VEC in/MAT in)");
   prospaCommandRegistry->add("diff",               Difference,              THREED_CMD,                           "VEC out = diff(VEC in)");
   prospaCommandRegistry->add("divider",            MakeDivider,             GUI_CMD,                              "OBJ name = divider(INT control_nr, INT x, INT y, INT width, INT height, STR orientation)");
   prospaCommandRegistry->add("distobj",            DistributeObjects,       GUI_CMD,                              "distobj(STR distribute_mode)");
   prospaCommandRegistry->add("donothing",          DoNothing,               GENERAL_COMMAND,                      "donothing()");
   prospaCommandRegistry->add("double",             ConvertToDoublePrec,     VAR_CMD | MATH_CMD | MATRIX_CMD,      "NUM y = double(NUM x)");
   prospaCommandRegistry->add("dmatrix",            NewDMatrix,              MATH_CMD | MATRIX_CMD,                "MAT(2D) m = dmatrix(INT width [,INT height])");
   prospaCommandRegistry->add("dnoise",             NoiseDouble,             MATH_CMD | MATRIX_CMD,                "DMAT y = dnoise(INT width [,INT height [,INT depth [,INT hyperdepth]]])");
   prospaCommandRegistry->add("draw1d",             DrawPlot,                GUI_CMD | ONED_CMD,                   "draw1d(STR \"true/false\")");
   prospaCommandRegistry->add("draw2d",             DrawImage,               GUI_CMD | TWOD_CMD,                   "draw2d(STR \"true/false\")");
   prospaCommandRegistry->add("draw3d",             Draw3DPlot,              GUI_CMD | THREED_CMD,                 "draw3d(STR \"true/false\")");
   prospaCommandRegistry->add("drawimage",          DrawImage,               GUI_CMD | TWOD_CMD,                   "drawimage(STR \"true/false\")");
   prospaCommandRegistry->add("drawobj",            DrawObject,              GUI_CMD,                              "drawobj(INT window_nr, INT control_nr)");
   prospaCommandRegistry->add("drawplot",           DrawPlot,                GUI_CMD | ONED_CMD,                   "drawplot(STR \"true/false\")");
   prospaCommandRegistry->add("drawwin",            DrawWindow,              GUI_CMD,                              "drawwin(INT window_nr)");

   prospaCommandRegistry->add("ed",                 LoadEditor,              GUI_CMD,                              "ed(STR file_name)");
   prospaCommandRegistry->add("edit",               LoadEditor,              GUI_CMD,                              "edit(STR file_name)");
   prospaCommandRegistry->add("editobj",            EditObject,              GUI_CMD,                              "(INT winNr,VEC objNr(s)) = editobj(INT winNr, INT/VEC objNr(s))");
   prospaCommandRegistry->add("editor",             MakeTextEditor,          GUI_CMD,                              "OBJ name = editor(INT control_nr, INT x, INT y, INT width, INT height)");
   prospaCommandRegistry->add("else",               nProc,                   CONTROL_COMMAND,                      "else");
   prospaCommandRegistry->add("elseif",             ElseIf,                  CONTROL_COMMAND,                      "elseif (boolean_expression)");
//   prospaCommandRegistry->add("encrypt",            Encrypt,                 GENERAL_COMMAND,                    "STR output = encrypt(STR input)");
   prospaCommandRegistry->add("endif",              nProc,                   CONTROL_COMMAND,                      "endif");
   prospaCommandRegistry->add("endmovie",           EndMovie,                GENERAL_COMMAND,                      "endmovie");
   prospaCommandRegistry->add("endtry",             EndTry,                  CONTROL_COMMAND,                      "endtry");
   prospaCommandRegistry->add("endproc",            ReturnFromProcedure,     CONTROL_COMMAND,                      "endproc(argument_list)");
   prospaCommandRegistry->add("endwhile",           EndWhileLoop,            CONTROL_COMMAND,                      "endwhile");
   prospaCommandRegistry->add("errorstr",           MakeErrorString,         STRING_CMD,                           "STR result = errorstr(FLOAT number, FLOAT error, [INT resolution , [INT power [,STR destination]]])");
   prospaCommandRegistry->add("escapechar",         ProcessEscapeCharacters, STRING_CMD,                           "escapechar(STR \"true/false\"))");
   prospaCommandRegistry->add("eval",               EvaluateExpression,      STRING_CMD,                           "VARIANT result = eval(STR/LIST expression [, STR \"real/double/complex\"])");
   prospaCommandRegistry->add("evalhex",            EvaluateHexString,       STRING_CMD,                           "VARIANT result = evalhex(STR/LIST expression [, INT nrBits, STR \"signed/unisigned\"])");
   prospaCommandRegistry->add("evalsubexp",         EvaluateSubExpression,   STRING_CMD,                           "VARIANT result = evalsubexp(STR expression)");                           
   prospaCommandRegistry->add("ex",                 CloseApplication,        CONTROL_COMMAND,                      "ex([INT 0/1])");
   prospaCommandRegistry->add("exec",               ExecuteFile,             GENERAL_COMMAND,                      "exec(STR file_name, [STR arguments])");
   prospaCommandRegistry->add("execandwait",        ExecuteAndWait,          GENERAL_COMMAND,                      "execandwait(STR file_name, [STR arguments])");
   prospaCommandRegistry->add("exit",               CloseApplication,        CONTROL_COMMAND,                      "exit([INT 0/1])");
   prospaCommandRegistry->add("exitwhile",          ExitWhileLoop,           CONTROL_COMMAND,                      "exitwhile()");
   prospaCommandRegistry->add("exitfor",            ExitForLoop,             CONTROL_COMMAND,                      "exitfor()");
   prospaCommandRegistry->add("exp",                Exponential,             MATH_CMD | MATRIX_CMD,                "NUM y = exp(NUM x)");
   prospaCommandRegistry->add("export1d",           Export1DDataCLI,         FILE_CMD,                             "export1d(VARIABLE vector, STR filename) or export1d(VARIABLE x_vector, VARIABLE y_vector, STR filename)");
   prospaCommandRegistry->add("export2d",           Export2DDataCLI,         FILE_CMD,                             "export2d(VARIABLE matrix, STR filename)");
   prospaCommandRegistry->add("export3d",           Export3DDataCLI,         FILE_CMD,                             "export3d(VARIABLE matrix, STR filename)");
   prospaCommandRegistry->add("export1dpar",        Export1DDataParameters,  FILE_CMD,                             "export1dpar(STR parameter, VARIANT value, ...)");
   prospaCommandRegistry->add("export2dpar",        Export2DDataParameters,  FILE_CMD,                             "export2dpar(STR parameter, VARIANT value, ...)");
   prospaCommandRegistry->add("export3dpar",        Export3DDataParameters,  FILE_CMD,                             "export3dpar(STR parameter, VARIANT value, ...)");

   prospaCommandRegistry->add("factorial",          Factorial,               MATH_CMD,                             "INT y = factorial(INT x)");
   prospaCommandRegistry->add("findindex",          FindIndex,               MATH_CMD | MATRIX_CMD,                "(INT index1 ...) = findindex(VARIABLE vector, FLOAT x1, FLOAT x ...)");
   prospaCommandRegistry->add("findindex2",         FindIndex2,              MATH_CMD | MATRIX_CMD,                "(INT index1 ...) = findindex2(VARIABLE vector, INT startIndex, STR \"left/right\", FLOAT x1, FLOAT x2, ...)");
   prospaCommandRegistry->add("findmacro",          FindMacro,               FILE_CMD,                             "(STR path, STR full_name) = findmacro(STR macro_name)");
   prospaCommandRegistry->add("findstructitem",     GetStructureItemIndex,   STRUCT_CMD,                           "INT index = findstructitem(VARIABLE struct, STR itemName)");
   prospaCommandRegistry->add("findxvalue",         FindXValue,              MATH_CMD | MATRIX_CMD,                "(FLOAT index1, index2 ...) = findxvalue(VEC xAxis, VEC yAxis, INT startXIndex, STR \"left/right\", FLOAT y1, FLOAT y2, ...)");
   prospaCommandRegistry->add("fileinfo",           FileInfo,                FILE_CMD,                             "VARIANT info = fileinfo(STR filename, STR \"age/date/dateandtime/length/readonly\",[STR \"created/modified/accessed\"])");
   prospaCommandRegistry->add("fill",               FillVector,              MATRIX_CMD,                           "VEC y = fill(VEC x, INT new_size, STR \"start/sides/end\", CPLX fill_value)");
   prospaCommandRegistry->add("findobj",            FindObjectCLI,           GUI_CMD,                              "INT control_nr = findobj(INT window_nr, STR method, STR value)");
   prospaCommandRegistry->add("findwin",            FindWindowCLI,           GUI_CMD,                              "INT window_nr = findwin(STR method, STR value)");
   prospaCommandRegistry->add("findprospawin",      FindProspaWindows,       GUI_CMD,                              "ARRAY window_nrs = findprospawin()");
   prospaCommandRegistry->add("finishwindow",       CompleteWindow,          GUI_CMD,                              "finishwindow(INT window_nr, STR mode)"); 
   prospaCommandRegistry->add("fista",              FISTA2D,                 MATH_CMD | MATRIX_CMD,                "DMAT result = fista(DMAT nmr_data, DMAT kernel_x, DMAT kernel_y, DMAT spectral_guess, DOUBLE smoothing, INT nr_iterations[, INT mode, OBJ plot_region, OBJ progress_ctrl] )"); 
   prospaCommandRegistry->add("fixlist",            FixListFaults,           LIST_CMD,                             "LIST result = fixlist(LIST input, STR mode)"); 
   prospaCommandRegistry->add("floor",              Floor,                   MATH_CMD | MATRIX_CMD,                "NUM y = floor(NUM x)");   
   prospaCommandRegistry->add("for",                ForLoop,                 CONTROL_COMMAND,                      "for(VAR variable = INT start to INT end [step INT size])");
   prospaCommandRegistry->add("frac",               FindFractionalPart,      MATH_CMD | MATRIX_CMD,                "NUM y = frac(NUM x)");
   prospaCommandRegistry->add("func1d",             Plot1DFunctions,         GUI_CMD,                              "func1d(STR command)");                           
   prospaCommandRegistry->add("func2d",             Plot2DFunctions,         GUI_CMD,                              "func2d(STR command)");                           
   prospaCommandRegistry->add("func3d",             Plot3DFunctions,         GUI_CMD,                              "func3d(STR command)");
   prospaCommandRegistry->add("funccli",            CLIFunctions,            GUI_CMD,                              "funccli(STR command)");
   prospaCommandRegistry->add("funcedit",           EditorFunctions,         GUI_CMD,                              "funcedit(STR command)");
   prospaCommandRegistry->add("funcprospa",         ProspaFunctions,         GUI_CMD,                              "funcprospa(STR command)");
   prospaCommandRegistry->add("functextedit",       TextEditFunctions,       GUI_CMD,                              "functextedit(STR command)");
   prospaCommandRegistry->add("ft",                 FourierTransform,        FOURIER_CMD,                          "CVEC y = ft(CVEC x)");
   prospaCommandRegistry->add("ftshift",            FFTShift,                FOURIER_CMD,                          "ftshift(CVEC x)");

   prospaCommandRegistry->add("getargument",        GetArgument,             GENERAL_COMMAND,                      "VAR argument = getargument()");
   prospaCommandRegistry->add("getargnames",        GetArgumentNames,        GENERAL_COMMAND,                      "LIST names = getargnames()");
   prospaCommandRegistry->add("getbasedir",         GetBaseDirectory,        FILE_CMD,                             "STR dir = getbasedir(STR path)");
   prospaCommandRegistry->add("getbasepath",        GetBasePath,             FILE_CMD,                             "STR dir = getbasepath(STR path)");
   prospaCommandRegistry->add("getparentpath",      GetBasePath,             FILE_CMD,                             "STR dir = getparentpath(STR path)");
   prospaCommandRegistry->add("getcolor",           SelectColour,            COLOR_CMD | GUI_CMD,                   "VEC col = getcolor(VEC default)");
   prospaCommandRegistry->add("getcolormap",        GetColorMap,             COLOR_CMD | GUI_CMD | TWOD_CMD,       "MAT col = getcolormap()");
   prospaCommandRegistry->add("getctrlvalues",      GetControlValues,        GUI_CMD,                              "LIST/STRUCT result = getctrlvalues(INT window_nr [,STR output = \"list\"/\"struct\", [STR method = \"list\"/\"prefix\"/\"all\", STR variable]])");
   prospaCommandRegistry->add("getcurrentline",     GetCurrentMacroLineNr,   GENERAL_COMMAND,                      "INT lineNr = getcurrentline()");
   prospaCommandRegistry->add("getcwd",             GetCWD,                  FILE_CMD,                             "STR directory = getcwd()");
   prospaCommandRegistry->add("getdate",            GetDate,                 TIME_CMD,                             "STR date = getdate([STR format])");
   prospaCommandRegistry->add("getdatarange",       GetDataRange,            TWOD_CMD,                             "(FLOAT min,max) = getdatarange(STR mode)");
   prospaCommandRegistry->add("getdirlist",         GetDirectoryList,        FILE_CMD,                             "LIST dirlist = getdirlist(STR directory, STR dot_folders_to_include)");
   prospaCommandRegistry->add("geteditwin",         GetEditedWindow,         GUI_CMD,                              "INT window_nr = geteditwin()");
   prospaCommandRegistry->add("getenvvar",          GetEnvironmentVariable,  GENERAL_COMMAND,                      "STR result = getenvvar(STR name)");
   prospaCommandRegistry->add("getext",             GetFileExtension,        FILE_CMD,                             "STR ext = getext(STR filename)");
   prospaCommandRegistry->add("getfilelist",        GetFileList,             FILE_CMD,                             "LIST filelist = getfilelist(STR directory)");                        
   prospaCommandRegistry->add("getfilesize",        GetFileDimensions,       FILE_CMD,                             "(INT width, height, depth) = GetFileDimensions(STR file_name)");
   prospaCommandRegistry->add("getfilename",        GetFileName,             FILE_CMD,                             "STR name = getfilename(STR type, STR window_title, STR range_title, STR range,[STR default])");
   prospaCommandRegistry->add("getfolder",          SelectFolder,            FILE_CMD,                             "STR name = getfolder([STR start_directory[, STR title[, STR top_directory[, STR allow_relative]]]])");
   prospaCommandRegistry->add("getfoldernobuttons", SelectFolderNoButtons,   FILE_CMD,                             "STR name = getfoldernobuttons([STR start_directory[, STR title[, STR top_directory[, STR allow_relative]]]])");
   prospaCommandRegistry->add("getfontlist",        GetFontList,             GENERAL_COMMAND,                      "LIST names = getfontlist()");
   prospaCommandRegistry->add("getfocus",           GetCtrlFocus,            GENERAL_COMMAND,                      "(INT winnr, objnr) = getfocus()");
//   prospaCommandRegistry->add("getkey",             GetKey,                  GUI_CMD,                              "getkey()");
   prospaCommandRegistry->add("getline",            GetLineFromText,         STRING_CMD,                           "STR line = getline(STR text, INT lineNr [, INT startCharNr, INT startLineNr])");
   prospaCommandRegistry->add("getmenuname",        GetMenuName,             GUI_CMD,                              "STR name = getmenuname()");
   prospaCommandRegistry->add("getmacroname",       GetMacroName,            FILE_CMD,                             "STR name = getmacroname()");
   prospaCommandRegistry->add("getmacropath",       GetMacroPath,            FILE_CMD,                             "STR path = getmacropath([STR macroName])");
   prospaCommandRegistry->add("getmainwindow",      GetMainWindow,           GUI_CMD,                              "INT nr = getmainwindow()");
   prospaCommandRegistry->add("getparentdir",       GetParentDirectory,      FILE_CMD,                             "STR dir = getparentdir(STR path)");
   prospaCommandRegistry->add("getprocnames",       GetProcedureNames,       FILE_CMD,                             "(LIST names, LIST args) = getprocnames(STR path, STR macro)");  
   prospaCommandRegistry->add("getlistindex",       FindListIndex,           LIST_CMD,                             "INT index = getlistindex(LIST list, STR entry)");
   prospaCommandRegistry->add("getlistnames",       GetListNames,            LIST_CMD,                             "LIST names = getlistnames(LIST parameter_list)");
   prospaCommandRegistry->add("getlistvalue",       GetParameterListValue,   LIST_CMD,                             "STR value = getlistvalue(LIST assignment_list, STR parameter_name)");
   prospaCommandRegistry->add("getmessage",         MakeGetMessage,          GUI_CMD,                              "getmessage(INT control_nr, LIST command_list)");
   prospaCommandRegistry->add("getobj",             GetObject,                GUI_CMD,                              "CLASS parameter = getobj(INT window_nr, INT control_nr)");
   prospaCommandRegistry->add("getpar",             GetParameter,            GUI_CMD,                              "VARIANT parameter = getpar(INT window_nr, INT/STR control_id, STR parameter)");
   prospaCommandRegistry->add("getplotdata",        ExtractDataFromPlot,     GUI_CMD | ONED_CMD | TWOD_CMD,        "(VEC x, VEC y) = getplotdata(STR 1d, STR opt) or (MAT m) = getplotdata(STR 2d, STR opt) or (MAT m, VEC x, VEC y = getplotdata(STR 2d, STR opt)");                           
   prospaCommandRegistry->add("getplotstate",       GetPlotState,            GUI_CMD | ONED_CMD | TWOD_CMD,        "STR state = getplotstate()");                           
   prospaCommandRegistry->add("getrect",            GetSelectRectangle,      GUI_CMD | ONED_CMD | TWOD_CMD,        "(INT minx,miny,maxx,maxy) = getrect()");
   prospaCommandRegistry->add("getregistryinfo",    GetRegistryInfo,         GENERAL_COMMAND,                      "STR info = getregistryinfo(STR hive, STR key[, STR valueName, STR valueType])");
 //  prospaCommandRegistry->add("getsid",             GetSID,                 GENERAL_COMMAND,                     "STR result  = getsid()"); 
   prospaCommandRegistry->add("getstructitem",      GetStructureItem,        STRUCT_CMD,                            "(STR name, VAR value) = getstructitem(STRUCT src, INT index)");
   prospaCommandRegistry->add("getsublist",         GetSubParameterList,     LIST_CMD,                             "LIST result = getsublist(LIST src, LIST keys)");
   prospaCommandRegistry->add("getselection",       GetTextSelection,        GUI_CMD,                              "STR text = getselection()");
   prospaCommandRegistry->add("gettime",            GetTimeOfDay,            TIME_CMD,                             "STR time = gettime([STR format])");
   prospaCommandRegistry->add("getwindowpar",       GetWindowParameter,      GUI_CMD,                              "VARIANT parameter_value = getwindowpar(INT window_nr, STR parameter_name)");
   prospaCommandRegistry->add("getx",               Get1DCoordinate,         GUI_CMD | ONED_CMD,                   "FLOAT x = getx([\"value\"]) or INT x = getx(\"index\" or (INT idx, FLOAT xpos) = getx(\"both\")");
   prospaCommandRegistry->add("getxy",              Get2DCoordinate,         GUI_CMD | TWOD_CMD,                   "(INT x, INT y) = getxy(STR \"vert/horiz/cross\", STR \"index/value\")");
   prospaCommandRegistry->add("grid",               ModifyGrid,              GUI_CMD | TWOD_CMD,                   "grid(STR parameter, VARIANT value, ...)");
   prospaCommandRegistry->add("gridctrl",           MakeGridCtrl,            GUI_CMD,                              "OBJ name = gridctrl(INT control_nr, INT x, INT y, INT width, INT height, INT cols, INT rows)");
   prospaCommandRegistry->add("groupbox",           MakeGroupBox,            GUI_CMD,                              "OBJ name = groupbox(INT control_nr, STR label, INT x, INT y, INT width, INT height)");
   prospaCommandRegistry->add("guiwinnr",           GUIWindowNumber,         GUI_CMD,                              "guiwinnr(INT win_nr)");

   prospaCommandRegistry->add("hash",               HashFunction,            STRING_CMD,                           "STR output = hash(VEC/STR input)");
   prospaCommandRegistry->add("hex",                HexConversion,           MATRIX_CMD | MATH_CMD,                "STR y = hex(INT decimal_number[, number_of_bits[, \"signed/unsigned/s/u\"]])");
   prospaCommandRegistry->add("help",               OpenHelpFile,            HELP_CMD,                             "help([STR documentation_folder, STR html_file])");
   prospaCommandRegistry->add("hft",                HilbertTransform,        FOURIER_CMD,                          "CVEC y = hft(RVEC x)");
   prospaCommandRegistry->add("hidewindow",         HideMyWindow,            GUI_CMD,                              "hidewindow(INT window_nr)");
   prospaCommandRegistry->add("histogram",          Histogram,               MATRIX_CMD,                           "(VEC histx, VEC histy) = histogram(VEC data, INT bins, VEC range)");
   prospaCommandRegistry->add("hold",               SetPlotHoldMode,         GUI_CMD | TWOD_CMD,                   "hold(STR \"on/off\")");
   prospaCommandRegistry->add("htmlbox",            MakeHTMLBox,             GUI_CMD,                              "OBJ name = htmlbox(INT control_nr, INT x, INT y, INT width, INT height)");
   prospaCommandRegistry->add("htmlcountsubstr",    HTMLCountSubString,      GUI_CMD,                              "STR result = htmlcountsubstr(STR text, STR substring_to_find [,casesensitivity])");   
   prospaCommandRegistry->add("htmlreplacestr",     HTMLReplaceString,       GUI_CMD,                              "(STR result, INT count) = htmlreplacestr(STR text, STR substring_to_replace, STR replacement_substring [,casesensitivity])");   

   prospaCommandRegistry->add("identity",           IdentityMatrix,          MATRIX_CMD,                           "MAT out = identity(INT dimension)");
   prospaCommandRegistry->add("if",                 IfStatement,             CONTROL_COMMAND,                      "if(boolean_expression)");
   prospaCommandRegistry->add("ignoredll",          IgnoreDLL,               DLL_CMD,                              "ignoredll(STR dll_name)");
   prospaCommandRegistry->add("ignoredlls",         IgnoreDLLs,              DLL_CMD,                              "ignoredlls(STR dll_name1, STR dll_name2, ...)");
   prospaCommandRegistry->add("info",               DisplayProcInfo,         HELP_CMD,                             "info(STR procedureName)");
   prospaCommandRegistry->add("ift",                InverseTransform,        FOURIER_CMD,                          "CVEC y = ift(CVEC x)");
   prospaCommandRegistry->add("inheritclass",       AppendToStructure,       STRUCT_CMD,                           "inheritclass(CLASS child, CLASS base)");
   prospaCommandRegistry->add("insertinlist",       InsertStringIntoListCLI, LIST_CMD,                             "LIST result = insertinlist(VARIABLE list, STR value, INT position)");
   prospaCommandRegistry->add("insert",             InsertIntoMatrix,        MATRIX_CMD,                           "MAT result = insert(MAT mA, MAT mB, INT x0[, INT y0[, INT z0]])");
   prospaCommandRegistry->add("integfunc",          SimpsonsIntegration,     MATH_CMD,                             "FLOAT result = integfunc(CONST-STR func, FLOAT left_limit, FLOAT right_limit)");
   prospaCommandRegistry->add("integvector",        TrapezoidalIntegration,  MATH_CMD,                             "integvector(VARIABLE x,VARIABLE y [, FLOAT left_limit ,FLOAT right_limit])");
   prospaCommandRegistry->add("interp",             InterpolateMatrix,       MATRIX_CMD,                           "MATRIX out = interp(MATRIX in, INT sx[, INT sy[, INT sz]])");
   prospaCommandRegistry->add("inv",                InvOperator,             MATRIX_CMD,                           "VARIANT y = inv(VARIANT x)");
   prospaCommandRegistry->add("imag",               ImaginaryPart,           MATH_CMD | MATRIX_CMD,                "MAT y = imag(CMAT x)");
   prospaCommandRegistry->add("image",              DisplayMatrixAsImage,    MATRIX_CMD | GUI_CMD | TWOD_CMD,      "image(MAT m, [STR colormap]) or image(MAT m, [VEC xscale, VEC yscale, [STR colormap]])");
   prospaCommandRegistry->add("imagerange",         SetImageRange,           GUI_CMD | TWOD_CMD,                   "imagerange(FLOAT min, FLOAT max)");
   prospaCommandRegistry->add("imagefileversion",   ImageFileSaveVersion,    FILE_CMD | TWOD_CMD,                  "imagefileversion(FLOAT version)");

   prospaCommandRegistry->add("import",             ImportMacro,             FILE_CMD,                             "import(macroName[, macroPath[, mode=\"window\"/\"local\"]");
   prospaCommandRegistry->add("import1d",           Import1DDataCLI,         FILE_CMD,                             "VEC y = import1d(STR filename,[INT length]) or (VEC x,y) = import1d(STR filename,[INT length])");
   prospaCommandRegistry->add("import2d",           Import2DDataCLI,         FILE_CMD,                             "MAT m = import2d(STR filename, INT width, INT height)");
   prospaCommandRegistry->add("import3d",           Import3DDataCLI,         FILE_CMD,                             "MAT m = import3d(STR filename, INT width, INT height, INT depth)");
   prospaCommandRegistry->add("import1dpar",        Import1DDataParameters,  FILE_CMD,                             "import1dpar(STR parameter, VARIANT value, ...)");
   prospaCommandRegistry->add("import2dpar",        Import2DDataParameters,  FILE_CMD,                             "import2dpar(STR parameter, VARIANT value, ...)");
   prospaCommandRegistry->add("import3dpar",        Import3DDataParameters,  FILE_CMD,                             "import3dpar(STR parameter, VARIANT value, ...)");
   prospaCommandRegistry->add("isappopen",          IsApplicationOpen,       FILE_CMD,                             "INT result = isappopen(STR app_name)");
   prospaCommandRegistry->add("iscached",           IsFileCached,            FILE_CMD,                             "INT result = iscached(STR file_name, STR dir_name, STR \"window\"/\"global\")");
   prospaCommandRegistry->add("isdir",              DoesDirectoryExist,      FILE_CMD,                             "INT result = isdir(STR dir_name)");
   prospaCommandRegistry->add("isdouble",           IsDoubleCLI,             MATH_CMD,                             "INT result = isdouble(VAR input)");
   prospaCommandRegistry->add("isfile",             DoesFileExist,           FILE_CMD,                             "INT result = isfile(STR file_name)");
   prospaCommandRegistry->add("isfloat",            IsFloatCLI,              MATH_CMD,                             "INT result = isfloat(VAR input)");   
   prospaCommandRegistry->add("index",              FindExactIndex,          MATH_CMD,                             "INT index  = index(ARRAY/STR input, VAR value)");   
   prospaCommandRegistry->add("isinteger",          IsIntegerCLI,            MATH_CMD,                             "INT result = isinteger(VAR input)");   
   prospaCommandRegistry->add("iskeypressed",       IsKeyPressed,            GENERAL_COMMAND,                      "INT result = iskeypressed(STR key)");
   prospaCommandRegistry->add("isobj",              IsObject,                GUI_CMD,                              "INT result = isobj(INT window_nr, INT control_nr)");   
   prospaCommandRegistry->add("isplotidle",         IsPlotIdle,              GUI_CMD,                              "INT result = isplotidle([CLASS plot_region])");
   prospaCommandRegistry->add("isproc",             IsProcedureAvailable,    FILE_CMD,                             "INT result = isproc(STR, macroPath, STR macroName, STR procName)");

   prospaCommandRegistry->add("ispar",              DoesParameterExist,      LIST_CMD,                             "INT result = ispar(LIST list_name, STR parameter_name)");
   prospaCommandRegistry->add("issubstr",           IsSubString,             STRING_CMD,                           "INT result = issubstr(STR string/LIST list, STR substring)");
   prospaCommandRegistry->add("isosurf",            Display3dSurface,        THREED_CMD,                           "isosurf(MAT3D matrix, FLOAT level, RGB colour, VEC range)");
   prospaCommandRegistry->add("isvar",              IsAVariable,             VAR_CMD,                              "INT result = isvar(variable)");
   prospaCommandRegistry->add("isvalidfilename",    ValidFileName,           FILE_CMD,                             "INT result = isvalidfilename(STR file/folder-name)");
   
   prospaCommandRegistry->add("j0",                 ZerothOrderBessel,       MATRIX_CMD | MATH_CMD,                "NUM y = j0(NUM x)");
   prospaCommandRegistry->add("j1",                 FirstOrderBessel,        MATRIX_CMD | MATH_CMD,                "NUM y = j1(NUM x)");
   prospaCommandRegistry->add("jn",                 NthOrderBessel,          MATRIX_CMD | MATH_CMD,                "NUM y = jn(INT order, NUM x)");
   prospaCommandRegistry->add("join",               JoinMatrices,            MATRIX_CMD,                           "MAT result = join(MAT m1, MAT m [STR edge])");
   prospaCommandRegistry->add("joinfiles",          JoinFiles,               FILE_CMD,                             "joinfiles(STR file1, STR file STR result)");

   prospaCommandRegistry->add("keepontop",          KeepWindowOnTop,         GUI_CMD,                              "keepontop(INT window_nr)");
   prospaCommandRegistry->add("keepfocus",          KeepFocus,               GUI_CMD,                              "keepfocus(STR \"true\"/\"false\")");
   prospaCommandRegistry->add("keepsubplot",        KeepSubPlot,             GUI_CMD | TWOD_CMD,                   "keepsubplot(STR 1d/2d, INT x, INT y)");

   prospaCommandRegistry->add("lasterror",          GetLastError,            GENERAL_COMMAND,                      "STRUC errorstring = lasterror()");
//   prospaCommandRegistry->add("line2d",             AddLineToMatrix,         GENERAL_COMMAND,                      "line2d(INT x0, INT y0, INT x1, INT y1, FLOAT value)");
   prospaCommandRegistry->add("lhil2d",             LawsonHansonNNLS2D,      MATRIX_CMD,                           "lhil2d(MAT data, MAT Ex, MAT Ey, VEC specX, VEC specY, FLOAT smoothing, STR callback)");
   prospaCommandRegistry->add("licenseinfo",        GetLicenseInfo,          LICENSE_CMD,                          "licenseinfo()");
   prospaCommandRegistry->add("lines3d",            Draw3DLines,             THREED_CMD,                           "line3d(MAT lines)");
   prospaCommandRegistry->add("linewidth3d",        LineWidth3d,             THREED_CMD,                           "linewidth3d(FLOAT width)");
   prospaCommandRegistry->add("linspace",           LinearVector,            MATRIX_CMD,                           "VEC result = linspace(FLOAT first, FLOAT last, FLOAT number)");
   prospaCommandRegistry->add("linvec",             LinearVector,            MATRIX_CMD,                           "VEC result = linvec(FLOAT first, FLOAT last, FLOAT number)");
   prospaCommandRegistry->add("light3d",            Set3DLight,              THREED_CMD,                           "light3d(FLOAT xPos, FLOAT yPos, FLOAT zPos)");
   prospaCommandRegistry->add("list",               NewList,                 LIST_CMD,                             "LIST y = list(INT number_of_entries), LIST2D y = list(INT number_cols, number_rows)");
   prospaCommandRegistry->add("listbox",            MakeListBox,             GUI_CMD,                              "OBJ name = listbox(INT control_nr, INT x, INT y, INT width, INT height, [LIST commands])");
   prospaCommandRegistry->add("listcom",            ListCommands,            HELP_CMD,                             "LIST com = listcom(CONST-STR start_characters [, STR category_filter])");
   prospaCommandRegistry->add("listdlls",           ListDLLCommands,         HELP_CMD | DLL_CMD,                   "listdlls([STR \"names\",dll_name])");
   prospaCommandRegistry->add("listwindows",        ListWindows,             GUI_CMD | HELP_CMD,                   "listwindows()");
   prospaCommandRegistry->add("listcachedprocs",    ListCachedProcedures,    GENERAL_COMMAND,                      "listcachedprocs(STR mode [,STR macro])");
   prospaCommandRegistry->add("listcontrols",       ListControls,            GUI_CMD,                              "listcontrols(INT winNr)");
   prospaCommandRegistry->add("listto1d",           Convert2DListTo1D,       LIST_CMD,                             "LIST1D out = listto1d(LIST2D in)");
   prospaCommandRegistry->add("listto2d",           Convert1DListTo2D,       LIST_CMD,                             "LIST2D out = listto2d(LIST1D in)");
   prospaCommandRegistry->add("load",               Load,                    FILE_CMD,                             "load(STR filename, [STR option[, STR display]])");
   prospaCommandRegistry->add("loaddlls",           LoadDLLs,                FILE_CMD,                             "loaddlls()");
   prospaCommandRegistry->add("loadplotmode",       LoadPlotMode,            ONED_CMD | GUI_CMD,                   "loadplotmode(STR plot, STR mode)");
   prospaCommandRegistry->add("loadpix",            LoadPictureTo3DMatrix,   FILE_CMD | MATRIX_CMD,                "loadpix(STR file)");
   prospaCommandRegistry->add("log10",              Log10,                   MATRIX_CMD | MATH_CMD,                "NUM y = log10(NUM x)");
   prospaCommandRegistry->add("loge",               Loge,                    MATRIX_CMD | MATH_CMD,                "NUM y = loge(NUM x)");
   prospaCommandRegistry->add("log2",               Log2,                    MATRIX_CMD | MATH_CMD,                "NUM y = log2(NUM x)");
   prospaCommandRegistry->add("logbin",             LogBin,                  MATRIX_CMD,                           "(VEC xOut, VEC yOut) = logbin(VEC xIn, VEC yIn, INT bin_size)");
   prospaCommandRegistry->add("logspace",           LogVector,               MATRIX_CMD,                           "VEC result = logspace(FLOAT first, FLOAT last, FLOAT number)");
   prospaCommandRegistry->add("logvec",             LogVector,               MATRIX_CMD,                           "VEC result = logvec(FLOAT first, FLOAT last, FLOAT number)");
   prospaCommandRegistry->add("ls",                 ListFiles,               FILE_CMD,                             "LIST files = ls([STR range [,STR which[, STR print]]])");

   prospaCommandRegistry->add("macadrs",            GetMacAdress,            GENERAL_COMMAND,                      "VEC adrs = macadrs()");
   prospaCommandRegistry->add("makelink",           MakeLink,                FILE_CMD,                              "makelink(STR linkPath, STR executablePath, STR quotedExecuableArguments, STR workingDirectory)");
   prospaCommandRegistry->add("mag",                Magnitude,               MATRIX_CMD | MATH_CMD,                "MAT y = mag(CMAT x )");
   prospaCommandRegistry->add("marginspref",        DefaultWindowMargins,    GENERAL_COMMAND,                      "marginspref(INT left, INT right, INT top, INT base)");
   prospaCommandRegistry->add("matrix",             NewMatrix,               MATRIX_CMD,                           "MAT(3D) m = matrix(INT width [,INT height [,INT depth [,INT hyperdepth]]])");
   prospaCommandRegistry->add("matrixlist",         GetMatrixList,           MATRIX_CMD,                           "LIST result = matrixlist(INT which_type_to_list)");
   prospaCommandRegistry->add("matrixdim",          GetMatrixDimension,      MATRIX_CMD,                           "INT result = matrixdim(MAT m)");
   prospaCommandRegistry->add("max",                Maximum,                 MATRIX_CMD,                           "[val,x] = max(MAT m) or [val,x,y] = max(MAT m) or [val,x,y,z] = max(MAT3D m)");
   prospaCommandRegistry->add("mean",               MatrixMean,              MATRIX_CMD,                           "float result = mean(MAT/VEC input)");
   prospaCommandRegistry->add("menu",               MakeMenu,                GENERAL_COMMAND,                      "OBJ name = menu(INT nr, STR name, {STR item, STR command})");
   prospaCommandRegistry->add("menuk",              MakeMenuWithKeys,        GUI_CMD,                              "OBJ name = menuk(INT nr, STR name, {STR item, STR key, STR command})");
   prospaCommandRegistry->add("memory",             MemoryStatus,            GENERAL_COMMAND,                      "INT result = memory()");
   prospaCommandRegistry->add("mergelists",         MergeParameterLists,     LIST_CMD,                             "LIST newlist = mergelists(LIST list1, LIST list2)");
   prospaCommandRegistry->add("message",            DisplayMessage,          GUI_CMD,                              "message(STR caption, STR text_to_display [, STR icon])");
   prospaCommandRegistry->add("min",                Minimum,                 MATRIX_CMD,                           "[val,x] = min(MAT m) or [val,x,y] = min(MAT m) or [val,x,y,z] = min(MAT3D m)");
   prospaCommandRegistry->add("mkdir",              MakeDirectory,           FILE_CMD,                             "mkdir(STR new_directory)");
   prospaCommandRegistry->add("mkobj",              MakeObjectInteractively, GUI_CMD,                              "mkobj(CONST-STR object_command)");
   prospaCommandRegistry->add("mkparlist",          MakeParameterList,       LIST_CMD,                             "LIST parList = mkparlist([LIST list_names])");
   prospaCommandRegistry->add("mkparstruct",        MakeParameterStructure,  STRUCT_CMD,                           "STRUCT par = mkparstruct([LIST list_names])");

   
   prospaCommandRegistry->add("movie",              MakeMovie,               GENERAL_COMMAND,                      "movie(STR codec, STR fileName, STR fps)");
   prospaCommandRegistry->add("movefile",           MoveAFile,               FILE_CMD,                             "movefile(STR source, STR destination)");
   prospaCommandRegistry->add("movewindow",         MoveAppWindow,           GUI_CMD,                              "movewindow(STR name, INT x, INT y, INT width, INT height)");
   prospaCommandRegistry->add("multiplot",          Multiplot,               GUI_CMD | TWOD_CMD,                   "multiplot(STR \"1d/2d\", INT nrx, INT nry)");

   prospaCommandRegistry->add("next",               NextLoop,                CONTROL_COMMAND,                      "next(for_loop_variable)");
   prospaCommandRegistry->add("noise",              Noise,                   MATRIX_CMD,                           "MAT y = noise(INT width [,INT height, [INT depth, [INT hyperdepth]]])");
   prospaCommandRegistry->add("not",                NotOperator,             MATH_CMD,                             "FLOAT/STR y = not(FLOAT/STR x)");
                           
   prospaCommandRegistry->add("onerror",            OnError,                 GENERAL_COMMAND,                      "onerror(STR commands)");
   prospaCommandRegistry->add("outer",              TensorProduct,           MATRIX_CMD,                           "MAT result = outer(MAT A, MAT B)");
   prospaCommandRegistry->add("offdiag",            OffDiagonalMatrix,       MATRIX_CMD,                           "MAT out = offdiag(MAT in)");

   prospaCommandRegistry->add("pasteplot",          PastePlot,               ONED_CMD | TWOD_CMD,                  "pasteplot([CLASS plot_region])");
   prospaCommandRegistry->add("pasteintoplot",      PasteIntoPlot,           ONED_CMD | TWOD_CMD,                  "pasteintoplot([CLASS plot_region])");
   prospaCommandRegistry->add("pathnames",          SetPathNames,            FILE_CMD,                             "pathnames(STR folder_name, STR path)");
   prospaCommandRegistry->add("panel",              MakePanel,               GUI_CMD,                              "OBJ name = panel(INT control_nr, INT x, INT y, INT width, INT height)");
   prospaCommandRegistry->add("parentctrl",         GetParentObject,         GUI_CMD,                              "CLASS obj = parentobj()");
   prospaCommandRegistry->add("parse",              ParseString,             STRING_CMD,                           "LIST result = parse(STR string, STR delimiter)");                          
   prospaCommandRegistry->add("pause",              Pause,                   TIME_CMD,                             "pause(FLOAT time_in_seconds[, STR sleep/nosleep, STR events/noevents])");                          
   prospaCommandRegistry->add("phase",              Phase,                   MATRIX_CMD | MATH_CMD,                "MAT y = phase(CPLX/CMAT x)");                           
   prospaCommandRegistry->add("picture",            MakePicture,             GUI_CMD,                              "OBJ name = picture(INT control_nr, INT x, INT y, INT width, INT height)");                           
   prospaCommandRegistry->add("plane3d",            Draw3DPlane,             THREED_CMD,                           "plane3d(FLOAT position, STR direction, FLOAT xmin, FLOAT xmax, FLOAT ymin, FLOAT ymax, VEC color)");
   prospaCommandRegistry->add("plot",               PlotXY,                  TWOD_CMD,                             "INT traceID = plot([VEC x],VEC y,[parameter_list])");
   prospaCommandRegistry->add("plot1d",             MakePlotWindow,          ONED_CMD | GUI_CMD,                   "OBJ name = plot1d(INT nr, INT x, INT y, INT w, INT h)");
   prospaCommandRegistry->add("plot2d",             MakeImageWindow,         TWOD_CMD | GUI_CMD,                   "OBJ name = plot2d(INT nr, INT x, INT y, INT w, INT h)");
   prospaCommandRegistry->add("plot3d",             Make3DWindow,            THREED_CMD | GUI_CMD,                 "OBJ name = plot3d(INT nr, INT x, INT y, INT w, INT h)");
   prospaCommandRegistry->add("plotcallback",       DefinePlotCallback,      ONED_CMD,                             "plotcallback(STR function_to_replace, STR callback_macro)");
   prospaCommandRegistry->add("plotpref",           PlotPreferences,         ONED_CMD | TWOD_CMD,                  "plotpref(STR parameter, VARIANT value, ...)");                           
   prospaCommandRegistry->add("plotscale",          SetPlotScaleFactor,      ONED_CMD | TWOD_CMD,                  "plotscale(FLOAT scale_factor)");
   prospaCommandRegistry->add("plotfileversion",    PlotFileSaveVersion,     ONED_CMD | TWOD_CMD,                  "plotfileversion(FLOAT version)");
   prospaCommandRegistry->add("plotviewversion",    PlotViewVersion,         ONED_CMD | TWOD_CMD,                  "plotviewversion(INT version)");
   prospaCommandRegistry->add("pref3d",             Set3DControlPrefs,       THREED_CMD,                           "pref3d(FLOAT rotation, FLOAT xyzShift, FLOAT scale, FLOAT distance)");                  
   prospaCommandRegistry->add("pr",                 PrintString,             GENERAL_COMMAND,                      "pr(VARIANT data_to_print)");
   prospaCommandRegistry->add("print",              PrintString,             GENERAL_COMMAND,                      "print(VARIANT data_to_print)");
   prospaCommandRegistry->add("printtofile",        PrintToFile,             FILE_CMD,                              "printtofile(STR file_name [, \"overwrite\"/\"append\"])");
   prospaCommandRegistry->add("printtostring",      PrintToString,           STRING_CMD,                           "printtostring(STR string_name)");
   prospaCommandRegistry->add("procedure",          StartProcedure,          CONTROL_COMMAND,                      "procedure(CONST-STR procedure_name, [argument_list])");
   prospaCommandRegistry->add("progressbar",        MakeProgressBar,         GUI_CMD,                              "OBJ name = progressbar(INT control_nr, INT x, INT y, INT width, INT height, STR \"horizontal/vertical\")");
   prospaCommandRegistry->add("pseudologvec",       PseudoLogVector,         MATRIX_CMD,                           "(VEC xOut, VEC yOut) = pseudologvec(FLOAT start, FLOAT end, INT number_of_points)");
   prospaCommandRegistry->add("pseudologbin",       PseudoLogBin,            MATRIX_CMD,                           "(VEC xOut, VEC yOut) = pseudologbin(VEC xIn, VEC yIn, INT bin_size)");
   prospaCommandRegistry->add("pwd",                GetCWD,                  GENERAL_COMMAND,                      "STR directory = pwd()");
  
   prospaCommandRegistry->add("query",              YesNoMessage,            GUI_CMD,                              "STR result = query(STR caption, STR message, [STR default, [STR mode]])");

   prospaCommandRegistry->add("rand",               Random,                  MATH_CMD,                             "INT y = rand(INT max_value) or MAT y = rand(INT width, INT height, max_value)");   
   prospaCommandRegistry->add("radiobuttons",       MakeRadioButtons,        GUI_CMD,                              "OBJ name = radiobuttons(INT control_nr, INT x, INT y, INT spacing, STR orientation, STR states, STR initial, [LIST commands])");   
   prospaCommandRegistry->add("real",               RealPart,                MATH_CMD,                             "FLOAT y = real(CPLX x)");   
   prospaCommandRegistry->add("realtostr",          RealToStr,               STRING_CMD,                           "STR y = realtostr(FLOAT x)");
   prospaCommandRegistry->add("rtoc",               RealToComplex,           MATH_CMD,                             "VEC y = realtocomplex(CVEC x)");
   prospaCommandRegistry->add("rectangle",          AddRectangleToMatrix,    GENERAL_COMMAND,                      "MAT mout = rectangle(MAT min, INT x0, INT y0, INT w/2, INT h/2, FLOAT a)");
   prospaCommandRegistry->add("relpath",            RelativePath,            FILE_CMD,                             "STR result = relpath(STR path, VARIABLE var)");   
   prospaCommandRegistry->add("requestlicense",     RequestLicense,          LICENSE_CMD,                          "requestlicense()");   
   prospaCommandRegistry->add("replacestr",         ReplaceString,           GENERAL_COMMAND,                      "STR result = replacestr(STR text, STR substring_to_replace, STR replacement_substring");   
   prospaCommandRegistry->add("replacesubexp",      ReplaceSubExpressions,   GENERAL_COMMAND,                      "replacesubexp(STR \"true/false\"))");
   prospaCommandRegistry->add("resizeobj",          ResizeObjects,           GUI_CMD,                              "resizeobj(STR resize_mode)");
   prospaCommandRegistry->add("resolvelink",        ResolveLink,             FILE_CMD,                             "STR result = resolvelink(STR link_path)");
   prospaCommandRegistry->add("reshape",            ReshapeMatrix,           MATRIX_CMD,                           "MAT out = reshape(MAT in, INT new_width [, INT new_height [, INT new_depth [, INT new_hyperdepth]]])");
   prospaCommandRegistry->add("reflect",            ReflectMatrix,           MATRIX_CMD,                           "MAT y = relect(MAT x, [STR \"horiz\"/\"vert\"])");
   prospaCommandRegistry->add("regexp",             RegexMatch,              STRING_CMD,                           "VEC groups = regexp(STR text, STR regex)");
   prospaCommandRegistry->add("renamelistentry",    RenameListEntry,         LIST_CMD,                             "renamelistentry(LIST lst, STR oldKey, STR newKey)");
   prospaCommandRegistry->add("runremote",          RunRemoteMacro,          FILE_CMD,                             "STR response = runremote(STR cmd)");
   prospaCommandRegistry->add("retvar",             GetReturnValue,          MACRO_CMD,                            "VAR result = retvar(COMMAND/PROCEDURE,INT n)");
   prospaCommandRegistry->add("return",             ReturnFromProcedure,     MACRO_CMD,                            "return([argument_list])");
   prospaCommandRegistry->add("rft",                RealFourierTransform,    MATRIX_CMD | FOURIER_CMD,             "CVEC y = rft(CVEC x)");   
   prospaCommandRegistry->add("rmdir",              RemoveDirectory,         FILE_CMD,                             "rmdir(STR directory)");   
   prospaCommandRegistry->add("rmcachedmacro",      RemoveCachedMacro,       MACRO_CMD,                            "rmcachedmacro(STR path, STR macro_name, STR mode, STR verbose)");   
   prospaCommandRegistry->add("rmcachedmacros",     RemoveCachedMacros,      MACRO_CMD,                            "rmcachedmacros(STR mode)");   
   prospaCommandRegistry->add("rmext",              RemoveFileExtension,     STRING_CMD | FILE_CMD,                "STR/LIST filename = rmext(STR/LIST filename.ext)");   
   prospaCommandRegistry->add("rmobj",              RemoveObject,            GUI_CMD,                              "rmobj(INT window_nr, INT control_nr)");   
   prospaCommandRegistry->add("rmfile",             RemoveFile,              FILE_CMD,                             "rmfile(STR file_name)");   
   prospaCommandRegistry->add("rmfolder",           RemoveFolder,            FILE_CMD,                             "rmfolder(STR folder_name)");   
   prospaCommandRegistry->add("rmfromlist",         RemoveStringFromList,    LIST_CMD,                             "LIST result = rmfromlist(VARIABLE list, INT position)");   
   prospaCommandRegistry->add("rmprefix",           RemovePrefix,            LIST_CMD,                             "LIST result = rmprefix(VARIABLE list, STR prefix)");   
   prospaCommandRegistry->add("rmrect",             RemoveSelectionRect,     GENERAL_COMMAND,                      "rmrect()");   
   prospaCommandRegistry->add("rms",                MatrixRMS,               MATRIX_CMD,                           "float result = rms(MAT/VEC input)");
   prospaCommandRegistry->add("rmsubstr",           RemoveSubString,         STRING_CMD,                           "STR result = rmsubstr(STR str, STR substr)");   
   prospaCommandRegistry->add("rmvar",              RemoveVariableByName,    VAR_CMD,                              "rmvar(VARIABLE v1, VARIABLE v ...)");   
   prospaCommandRegistry->add("rotate",             RotateMatrix,            MATRIX_CMD,                           "MAT y = rotate(MAT x, INT x_rot[, INT y_rot[, INT z_rot]])");   
   prospaCommandRegistry->add("rotate3d",           Rotate3DPlot,            THREED_CMD,                           "rotate3d(FLOAT x_angle, FLOAT y_angle, FLOAT z_angle)");   
   prospaCommandRegistry->add("round",              RoundToNearestInteger,   MATH_CMD,                             "NUM y = round(NUM x)");   
   prospaCommandRegistry->add("run",                EvaluateExpression,      MACRO_CMD,                            "run(STR macro_call)");   
                           
   prospaCommandRegistry->add("save",               Save,                    FILE_CMD,                             "save(STR file_name [, VARIABLE var])");
   prospaCommandRegistry->add("savewav",            SaveWaveFile,            FILE_CMD,                             "savewav(STR file_name , VEC var)");
   prospaCommandRegistry->add("savelayout",         SaveWindowLayout,        FILE_CMD,                             "savelayout(STR file_name [, STR mode1[, STR mode2]])");
//   prospaCommandRegistry->add("savekey",            SaveKey,                 GENERAL_COMMAND,                      "savekey(STR file_name)");
   prospaCommandRegistry->add("savewindow",         SaveGUIWindowImage,      GENERAL_COMMAND,                      "savewindow(INT window_nr, STR file_name, [VEC cliprect [, STR mode]])");
   prospaCommandRegistry->add("scale3d",            Scale3DPlot,             GENERAL_COMMAND,                      "scale3d(FLOAT x_scale, FLOAT y_scale, FLOAT z_scale)");
   prospaCommandRegistry->add("searchdlls",         SearchDLLs,              GENERAL_COMMAND,                      "searchdlls(STR \"true\"/\"false\")");
   prospaCommandRegistry->add("segments3d",         Draw3DSegments,          GENERAL_COMMAND,                      "segments3d(MAT segments)");

   prospaCommandRegistry->add("scanstr",           ScanString,               STRING_CMD,                           "STR result = scanstr(STR text, STR regex)");   
   prospaCommandRegistry->add("sd",                MatrixStandardDeviation,  MATRIX_CMD,                           "float result = sd(MAT/VEC input)");
   prospaCommandRegistry->add("selectobj",         SelectObjInteractively,   GENERAL_COMMAND,                      "(INT winNr, INT objNr, OBJ name) = selectobj()");
   prospaCommandRegistry->add("sendmessage",       SendGUIMessage,           GENERAL_COMMAND,                      "sendmessage(STR source, STR message)");
   prospaCommandRegistry->add("sendtoprospa",      SendToExistingProspa,     FILE_CMD,                             "int result = sendtoprospa(STR window_title, STR file_path, STR file_to_open)");
   prospaCommandRegistry->add("setctrlvalues",     SetControlValues,         GENERAL_COMMAND,                      "setctrlvalues(INT window_nr, LIST control_list)");
   prospaCommandRegistry->add("setcwd",            SetCWD,                   FILE_CMD,                             "setcwd(STR directory)");
   prospaCommandRegistry->add("seteditwin",        SetEditableWindow,        GENERAL_COMMAND,                      "seteditwin(INT window_nr)");
   prospaCommandRegistry->add("seteditrendermode", SetEditorRenderMode,      GENERAL_COMMAND,                      "seteditrendermode(STR \"current/parent\")");
   prospaCommandRegistry->add("selecteditline",    SelectEditorLine,         GENERAL_COMMAND,                      "selecteditline(INT line_nr)");
   prospaCommandRegistry->add("setfolder",         SetCurrentFolder,         FILE_CMD,                             "setfolder(STR folder)");
   prospaCommandRegistry->add("setfont",           SetFonts,                 GUI_CMD,                              "setfont(STR prospa_font, STR font_name, INT font_size)");
   prospaCommandRegistry->add("setfocus",          SetWindowFocus,           GUI_CMD,                              "setfocus(INT window_nr [,INT ctrl_nr])");
   prospaCommandRegistry->add("setlistvalue",      SetParameterListValue,    LIST_CMD,                             "setlistvalue(LIST assignment_list, STR name, STR new_value)");
   prospaCommandRegistry->add("setlistvalues",     SetParameterListValues,   LIST_CMD,                             "setlistvalues(LIST main_list, LIST new_values)");
   prospaCommandRegistry->add("setpar",            SetParameter,             GUI_CMD,                              "setpar(INT window_nr, INT/STR control_id, STR parameter, VARIANT value)");
   prospaCommandRegistry->add("setplotstate",      SetPlotState,             GENERAL_COMMAND,                      "setplotstate(STR state)");                           
   prospaCommandRegistry->add("setstruct",         SetStructureValues,       GENERAL_COMMAND,                      "setstruct(STRUCT a, VARIANT value1, VARIANT value2 ...)");                           
   prospaCommandRegistry->add("setuiskin",         SetUISkin,                GENERAL_COMMAND,                      "setuiskin(STR \"win7/default\")");                           
   prospaCommandRegistry->add("setwindowpar",      SetWindowParameter,       GENERAL_COMMAND,                      "setwindowpar(INT window_nr, STR parameter, CONST-STR value)");
   prospaCommandRegistry->add("shift",             ShiftMatrix,              MATRIX_CMD,                           "MAT y = shift(MAT x, INT x_shift[, INT y_shift[, INT z_shift]])");
   prospaCommandRegistry->add("shift3d",           Shift3DPlot,              GENERAL_COMMAND,                      "shift3d(FLOAT x_offset, FLOAT y_offset, FLOAT z_offset)");
   prospaCommandRegistry->add("sign",              ReturnSign,               GENERAL_COMMAND,                      "result = sign(VAR input)");
   prospaCommandRegistry->add("showcmap",          ShowColorScale,           GENERAL_COMMAND,                      "showcmap(STR \"true/false\")");
   prospaCommandRegistry->add("showcmap3d",        ShowColorScale3D,         GENERAL_COMMAND,                      "showcmap3d(STR \"yes/no\")");
   prospaCommandRegistry->add("showcursor",        TrackCursor,              GENERAL_COMMAND,                      "showcursor(STR \"1D/2D\", STR \"cross/col/row\", CONST-STR procedure)");
   prospaCommandRegistry->add("showdialog",        ShowDialog,               GUI_CMD,                              "showdialog(INT window_nr)");
   prospaCommandRegistry->add("showerrors",        ShowErrors,               DEBUG_CMD,                            "showerrors(INT level (0/1/2)");
   prospaCommandRegistry->add("showobjects",       ShowObjects,              GUI_CMD,                              "showobject(INT window_nr)");
   prospaCommandRegistry->add("showborder",        ShowPlotBorder,           ONED_CMD  |TWOD_CMD,                  "showborder(STR \"true\"/\"false\")");
   //prospaCommandRegistry->add("showprocedures",    ShowMacroProcedures,      MACRO_CMD,                            "showprocedures()");
   prospaCommandRegistry->add("showwindow",        ShowMyWindow,             GUI_CMD,                              "showwindow(INT window_nr, STR mode)");
   prospaCommandRegistry->add("shownextwindow",    ShowNextWindow,           GUI_CMD,                              "shownextwindow(INT window_nr)");
   prospaCommandRegistry->add("showlastwindow",    ShowLastWindow,           GUI_CMD,                              "showlastwindow(INT window_nr)");
   prospaCommandRegistry->add("shuffle",           ShuffleItem,              MATRIX_CMD,                           "LIST/MAT result = shuffle(LIST/MAT shuffleMe)");

   prospaCommandRegistry->add("simplifydir",       SimplifyDirectory,        FILE_CMD,                             "STR simple_dir = simplifydir(STR complex_dir)");
   prospaCommandRegistry->add("sin",               Sine,                     MATH_CMD |TRIG_CMD,                   "NUM y = sin(NUM x)");
   prospaCommandRegistry->add("single",            ConvertToSinglePrec,      MATH_CMD,                             "NUM y = single(NUM x)");
   prospaCommandRegistry->add("sinh",              HyperbolicSine,           MATH_CMD |TRIG_CMD,                   "NUM y = sinh(NUM x)");
   prospaCommandRegistry->add("size",              Size,                     VAR_CMD | MATRIX_CMD,                 "INT w = size(LIST l) or (INT w,INT h) = size(MAT m) or (INT w, INT h, INT d) = size(MAT3D m) or (INT w, INT h, INT d, INT hd) = size(MAT4D m)");
   prospaCommandRegistry->add("sized",             SizeD,                    VAR_CMD | MATRIX_CMD,                 "LINT w = size(LIST l) or (LINT w,LINT h) = size(MAT m) or (LINT w, LINT h, LINT d) = size(MAT3D m) or (LINT w, LINT h, LINT d, LINT hd) = size(MAT4D m)");
   prospaCommandRegistry->add("slider",            MakeSlider,               GUI_CMD,                              "OBJ name = slider(INT control_nr, INT x, INT y, INT width, INT height, STR \"horizontal/vertical\", [LIST commands])");                           
   prospaCommandRegistry->add("sortlist",          SortList,                 LIST_CMD,                             "LIST out = sortlist(LIST in, [forward/reverse/forward_numeric/reverse_numeric/forward_embedded_numeric/reverse_embedded_numeric]) or sortlist(LIST in, VEC sort_indices)");
   prospaCommandRegistry->add("sortlist2",         SortList2,                LIST_CMD,                             "(LIST out, VEC indices) = sortlist2(LIST in, [forward/reverse/forward_numeric/reverse_numeric/forward_embedded_numeric/reverse_embedded_numeric]) or sortlist(LIST in, VEC sort_indices)");
   prospaCommandRegistry->add("sortrows",          SortMatrixRows,           MATRIX_CMD,                           "LIST out = sortrows(MAT in, [INT row, [STR \"ascending\"/\"descending\"]])");
   prospaCommandRegistry->add("sound",             ProduceSound,             MATRIX_CMD | SOUND_CMD,               "sound(VECTOR sound, [INT sampling_rate, [STR \"scale\"/\"fixed\")");
   prospaCommandRegistry->add("sphere",            DrawSphere,               THREED_CMD,                           "sphere(VECTOR pos, FLOAT radius, RGB colour, INT steps)");
   prospaCommandRegistry->add("splitpar",          SplitParameterString,     LIST_CMD,                             "(STR name, STR value) = splitpar(parameter_string)");
   prospaCommandRegistry->add("sqrt",              SquareRoot,               MATH_CMD,                             "NUM y = sqrt(NUM x)");
   prospaCommandRegistry->add("statictext",        MakeStaticText,           GUI_CMD,                              "OBJ name = statictext(INT control_nr, INT x, INT y, STR \"left/right\", STR text_to_display)");
   prospaCommandRegistry->add("statusbox",         MakeStatusBox,            GUI_CMD,                              "OBJ name = statusbox(INT control_nr)");
   prospaCommandRegistry->add("step",              nProc,                    NOP_CONTROL_CMD,                      "step");
   prospaCommandRegistry->add("strtoascii",        StringToAscii,            STRING_CMD,                           "VEC result = strtoascii(STR input)");
   prospaCommandRegistry->add("struct",            MakeStructure,            STRUCT_CMD,                           "STRUC result = struct(STR \"local\"/\"winvar\")  or STRUC result = struct(LIST parameter_list) or STRUC result = struct(member1=value1,  member2=value2, ... memberN=valueN) or STRUC result = struct(VAR member1, VAR member2, ... VAR memberN)");
   prospaCommandRegistry->add("structarray",       MakeStructureArray,       STRUCT_CMD,                           "STRUCARRAY result = struct(INT size)");
   prospaCommandRegistry->add("structappend",      AppendToStructure,        STRUCT_CMD,                           "structappend(STRUCT to_modify, STRUCT to_append)");
   prospaCommandRegistry->add("submatrix",         ExtractSubMatrix,         MATRIX_CMD,                           "MAT s = submatrix(MAT m, INT x1, INT x [INT y1, INT y [INT z1, INT z [INT q1, INT q2]]]");
   prospaCommandRegistry->add("sum",               Sum,                      MATRIX_CMD,                           "FLOAT s = sum(MAT m, [STR \"x/y/xy\"]) )");
   prospaCommandRegistry->add("surf2d",            Display2DSurface,         THREED_CMD,                           "surf2d(MAT/CMAT matrix, VEC colour-scale, VEC data-range, VEC map-range, STR xDirection, STR yDirection)");
   prospaCommandRegistry->add("swapvar",           SwapVariables,            VAR_CMD,                              "swapvar(CONST-STR variable1, CONST-STR variable2)");
   prospaCommandRegistry->add("syncaxes",          SetOrGetSyncAxes,         ONED_CMD,                             "STR s = syncaxes(STR \"true\", \"false\")");

   prospaCommandRegistry->add("tab",               MakeTabCtrl,              GUI_CMD,                              "tab(INT control_nr, INT x, INT y, INT width, INT height, [LIST commands])");
   prospaCommandRegistry->add("tan",               Tangent,                  MATH_CMD |TRIG_CMD,                   "NUM y = tan(NUM x)");
   prospaCommandRegistry->add("tanh",              HyperbolicTangent,        MATH_CMD |TRIG_CMD,                   "NUM y = tanh(NUM x)");
   prospaCommandRegistry->add("textbox",           MakeTextBox,              GUI_CMD,                              "OBJ name = textbox(INT control_nr, INT x, INT y, INT width, [LIST commands])");
   prospaCommandRegistry->add("textmenu",          MakeComboBox,             GUI_CMD,                              "OBJ name = textmenu(INT control_nr, INT x, INT y, INT width, INT height, [LIST commands])");
   prospaCommandRegistry->add("textsize3d",        Set3DTextSize,            THREED_CMD,                           "textsize3d(FLOAT size)");
   prospaCommandRegistry->add("testit",            AssignTest,               GENERAL_COMMAND,                      "testit(dst,src)");
   prospaCommandRegistry->add("thread",            Thread,                   THREAD_CMD,                           "INT threadID = thread(STR command[, STR args])");
   prospaCommandRegistry->add("threadabort",       AbortThread,              THREAD_CMD,                           "INT status = threadabort(INT threadID)");
   prospaCommandRegistry->add("threadcleanup",     CleanUpThread,            THREAD_CMD,                           "threadcleanup(INT threadID)");
   prospaCommandRegistry->add("threadwait",        WaitforThread,            THREAD_CMD,                           "threadwait(INT threadID)");
   prospaCommandRegistry->add("threadrunning",     ThreadStatus,             THREAD_CMD,                           "INT status = threadrunning(INT threadID)");
   prospaCommandRegistry->add("throw",             ThrowException,           CONTROL_COMMAND,                      "throw(STR message)");
   prospaCommandRegistry->add("time",              GetorSetElapsedTime,      TIME_CMD,                             "INT t = time() or time(INT initial_time [,float/double])");
   prospaCommandRegistry->add("title",             SetTitleParameters,       ONED_CMD | TWOD_CMD,                  "title(STR caption_text)");
   prospaCommandRegistry->add("to",                nProc,                    NOP_CONTROL_CMD,                      "to");
   prospaCommandRegistry->add("toolbar",           MakeToolBar,              GUI_CMD,                              "OBJ name = toolbar(INT control_nr, STR bitmap_name, {STR label, STR command})");
   prospaCommandRegistry->add("toolbark",          MakeToolBarWithKeys,      GUI_CMD,                              "OBJ name = toolbark(INT control_nr, STR bitmap_name, {STR label, STR key, STR command})");
   prospaCommandRegistry->add("trackcursor",       TrackCursor,              GUI_CMD,                              "trackcursor(STR \"1D/2D\", STR \"cross/col/row\", CONST-STR procedure)");
   prospaCommandRegistry->add("trans",             TransposeMatrix,          MATRIX_CMD,                           "MAT y = trans(MAT x, [STR axes])");
   prospaCommandRegistry->add("trac",              MatrixTrace,              MATRIX_CMD,                           "NUM y = trac(MAT x)");
   prospaCommandRegistry->add("trimstr",           TrimString,               STRING_CMD,                           "STR output = TrimString(STR input [[, STR \"front/back/both\"], STR char])");
   prospaCommandRegistry->add("trunc",             TruncateToInteger,        GENERAL_COMMAND,                      "NUM y = trunc(NUM x)");
   prospaCommandRegistry->add("try",               TryForAnError,            CONTROL_COMMAND,                      "try");
   prospaCommandRegistry->add("trace",             ModifyTrace,              ONED_CMD,                             "trace(STR parameter, VARIANT value, ...)");
   prospaCommandRegistry->add("tracepref",         ModifyTraceDefault,       ONED_CMD,                             "tracepref(STR parameter, VARIANT value, ...)");

   prospaCommandRegistry->add("uniquename",        GetUniqueVariableName,    VAR_CMD,                              "STR name = uniquename(STR base)");
   prospaCommandRegistry->add("unloaddlls",        UnloadDLLs,               DLL_CMD,                              "unloaddlls()");
   prospaCommandRegistry->add("updateedittitle",   UpdateEditorParentTitle,  GUI_CMD,                              "updateedittitle()");
   prospaCommandRegistry->add("updown",            MakeUpDown,               GUI_CMD,                              "updown(INT control_nr, INT x, INT y, INT width, INT height, STR orientation, [LIST commands])");
   prospaCommandRegistry->add("updatemainmenu",    UpdateMainMenu,           GUI_CMD,                              "updatemainmenu()");
   prospaCommandRegistry->add("usedll",            UseDLL,                   DLL_CMD,                              "usedll(STR DLL_name)");
   prospaCommandRegistry->add("usequotedstrings",  UseQuotedStrings,         STRING_CMD,                           "unquotedstrings(STR \"true/false\")");
   
   prospaCommandRegistry->add("vartype",           ReturnVariableType,       VAR_CMD,                              "STR type = vartype(CONST-STR variable)");
   prospaCommandRegistry->add("varlist",           GetVariableList,          LIST_CMD | VAR_CMD,                   "LIST result = varlist([STR/INT type[,STR/INT scope]])");
   prospaCommandRegistry->add("varstatus",         SetVariableStatus,        VAR_CMD,                              "varstatus(STR name, STR visibility, STR readstatus, STR lifetime)");
   prospaCommandRegistry->add("viewdistance",      SetViewDistance,          THREED_CMD,                           "viewdistance(FLOAT z_distance)");
   prospaCommandRegistry->add("viewsubplot",       ViewSubPlot,              ONED_CMD | TWOD_CMD,                  "viewsubplot(STR 1d/2d, INT x, INT y)");
   prospaCommandRegistry->add("viewfullplot",      ViewFullPlot,             ONED_CMD | TWOD_CMD,                  "viewfullplot(STR 1d/2d)");
   prospaCommandRegistry->add("vectorplot",        DrawVectors,              TWOD_CMD,                             "vectorplot(MAT x, MAT y, FLOAT length [, INT xstep, INT ystep])");
   prospaCommandRegistry->add("version",           GetProspaVersion,         GENERAL_COMMAND,                      "INT v = version()");
   
//   prospaCommandRegistry->add("waterfall2d",       DrawWaterFallPlot,        TWOD_CMD,                             "waterfall2d(MAT/CMAT matrix, VEC colour-scale, VEC data-range, VEC map-range, STR xDirection, STR yDirection)");
   prospaCommandRegistry->add("waterfall",         Waterfall,                THREED_CMD,                           "waterfall(MAT/CMAT matrix, VEC colour-scale, VEC data-range, VEC map-range, STR xDirection, STR yDirection)");
   prospaCommandRegistry->add("window",            MakeWindow,               GUI_CMD,                              "(INT number, OBJ name) = window(STR title, INT x, INT y, INT width, INT height[, STR \"resizeable\"])");
   prospaCommandRegistry->add("windowvar",         DefineWindowVariables,    VAR_CMD,                              "windowvar(CONST-STR var_a, var_b, ...)");
   prospaCommandRegistry->add("winnamespace",      GUIWindowNumber,          GUI_CMD,                              "winnamespace(INT win_nr)");
   prospaCommandRegistry->add("windowmargins",     SetWindowMargins,         GUI_CMD,                              "windowmargins(INT left, INT right, INT top, INT base)");
   prospaCommandRegistry->add("while",             WhileStatement,           CONTROL_COMMAND,                      "while (boolean_expression)");
   prospaCommandRegistry->add("xlabel",            SetXLabelParameters,      ONED_CMD | TWOD_CMD,                  "xlabel(STR caption_text)");
   prospaCommandRegistry->add("xor",               ExclusiveOr,              MATH_CMD,                             "FLOAT result = xor(FLOAT arg1, FLOAT arg2)");

   prospaCommandRegistry->add("ylabel",            SetYLabelParameters,      ONED_CMD | TWOD_CMD,                  "ylabel(STR caption_text)");
   prospaCommandRegistry->add("ylabelleft",        SetLeftYLabelParameters,  ONED_CMD,                             "ylabelleft(STR caption_text)");
   prospaCommandRegistry->add("ylabelright",       SetRightYLabelParameters, ONED_CMD,                             "ylabelright(STR caption_text)");


   prospaCommandRegistry->add("zerofill",          FillVector,               MATRIX_CMD,                           "VEC y = zerofill(VEC x, INT new_size, STR \"start/sides/end\", [CPLX fill_value])");
   prospaCommandRegistry->add("zoom1d",            Zoom1D,                   ONED_CMD,                             "zoom1d(FLOAT minx, FLOAT maxx [,FLOAT miny, FLOAT  maxy])");
   prospaCommandRegistry->add("zoom2d",            Zoom2D,                   TWOD_CMD,                             "zoom2d(INT left, INT right, INT bottom, INT top)");
}



/**************************************************************************************
*                                        Do nothing                                   *
**************************************************************************************/

// For 10,000 iterations nop(argument)
// with null function time = 0.26s (i.e. no body to function)
// with evaluate and argument = "1" time = 0.72s (+0.03 for each digit)
// with evaluate and argument = "b" time = 0.76 (b = 12.34)
// with evaluate and argument = "a" time = 4.2s (a = [0:1:1000]) increases with number of elements (170 s for 10000!)

// GetNextOperand takes 0.095 s


int NoOperation(char arg[])
{
   return(0);
}

/****************************************************************************
* Run a command (either internal, variable or DLL) or macro
****************************************************************************/

int RunCommand(Interface *itfc, char *nameIn, char *arg, short mode)
{
   int r;
   char name[MAX_STR];

   itfc->nrRetValues = 0;
   itfc->defaultVar = 1;

// Convert all letters to lower case
   StrNCopy(name,nameIn,MAX_STR-1);
   ToLowerCase(name);

// If its not a procedure so see if it is a command, DLL or a variable **************
   if(name[0] != ':') 
   {   
		int result = prospaCommandRegistry->call(name, itfc, arg);
      switch (result)
	   {
			case(-2):
				return 0;
			case(CMD_NOT_FOUND):
	         if(mode == 1) return(-4);
				   break;
			default:
				return result;
      }
	
	// Can't find command so check DLLs ***************************************	
      if (searchDLLs)
      {
         r = ProcessContinue(itfc, name, arg);
         if (r == OK)
            return(OK);
         if (r != RETURN_FROM_DLL)
            return(r);
      }
	  
  // See if it's a class procedure **********************
      if((r = CheckForClassProcedure(itfc,nameIn,arg)) != CMD_NOT_FOUND)
         return(r);

  // See if it's a variable procedure **********************
      if((r = CheckForVariableProcedure(itfc,nameIn,arg)) != CMD_NOT_FOUND)
         return(r);
	}

   //   CText expression;

   //Variable result;
   //if(arg[0] == '\0')
   //   expression = nameIn;
   //else
   //  expression.Format("%s(%s)",nameIn,arg);
   //short err;
   //r = Evaluate(itfc, RESPECT_ALIAS, expression.Str(), &result);
   //if(r == OK)
   //   return(OK);
 

// See if it's a macro procedure **************************
   r = CheckForMacroProcedure(itfc,nameIn,arg);

// If command not found and is not a valid filename then 
// assume its an expression and print it out

   if(r == CMD_NOT_FOUND)
   {
      static short infinCheck = 0; // Prevent inifinite looping
      if(name[0] != '\0' && infinCheck == 0)
      {
         if(itfc->inCLI)
         {
            infinCheck++;
            if(arg[0] == '\0')
            {
               r = PrintString(itfc,nameIn);
               gCheckForEvents = false;
               infinCheck = 0;
               return(r);
            }
            else
            {
               CText cmd;
               cmd.Format("%s(%s)",nameIn,arg);
     //          r = PrintRetVar(itfc,cmd.Str());

               r = PrintString(itfc,cmd.Str());
               gCheckForEvents = false;
               infinCheck = 0;
               return(r);
            }
         }
      }
      infinCheck = 0;

      if(mode != 2)
         ErrorMessage("invalid or unknown expression (command = '%s' argument = '%s')",name,arg);

   }

   return(r);
}


/************************************************************************
   Check to see if command is stored in a variable.
************************************************************************/
//TODO needs a lot of work
short CheckForVariableProcedure(Interface *itfc, char *command, char *arg)
{
	Variable* var = NULL;
	short type = UNQUOTED_STRING;
   Variable result;


   if(!var)
	   var = GetVariable(itfc,ALL_VAR,command,type); // Search for the variable

	if(var)
	{
		if(type == UNQUOTED_STRING) // A string has been passed - treat as
		{                           // a macro command(s)
			CText argument;
         CArg carg;

         if(!strcmp(var->GetString(),command))
         {
            ErrorMessage("Infinite recursion - aborting!");
            return(ERR);
         }

		// First evaluate and store any argument list in global "argVar" variables		
			RemoveQuotes(var->GetString());

         short nrArgs = carg.Count(arg);
         for(long i = 1; i <= nrArgs; i++)
         {
            argument = carg.Extract(i);
         
	 			if(Evaluate(itfc,RESPECT_ALIAS,argument.Str(),&result) < 0) 
					return(ERR);  

				if(CopyVariable(itfc->argVar[i].var,&result,RESPECT_ALIAS) == ERR)
					return(ERR);
			}  

         itfc->nrProcArgs = nrArgs;
         int oldLineNr = itfc->startLine;
         itfc->startLine = 0;
         bool oldInCLI = itfc->inCLI;
			itfc->inCLI = false;
         int oldParentLineNr = itfc->parentLineNr;
         itfc->parentLineNr = itfc->lineNr;
         short r;

         if(strchr(var->GetString(),';') || strchr(var->GetString(),'\n'))
         {
			   r = ProcessMacroStr(itfc,var->GetString()); // Macro procedure as a string
         }
         else
         {
            r = RunCommand(itfc, var->GetString(), arg, 2); // A variable representing a file name 
         }

         itfc->inCLI = oldInCLI;
         itfc->startLine = oldLineNr;
         itfc->parentLineNr = oldParentLineNr;

			return(r);
		}

		else if(type == LIST) // An argument list has been passed. Treat it
		{                     // as a list of commands and execute them
			char *cmd;
			long len = 0;
			char argument[MAX_STR];
         CArg carg;
         short e;

			if(var->GetData() == NULL)
				return(CMD_NOT_FOUND);

		// First simplify and store any argument list in global "arg" variable
			itfc->nrProcArgs = 0;
         do
         {
            if((e = carg.GetNext(arg,argument)) == ERR)
               break;
	 			if(Evaluate(itfc,RESPECT_ALIAS,argument,&result) < 0) 
					return(ERR);     
				if(CopyVariable(itfc->argVar[++itfc->nrProcArgs].var,&result,RESPECT_ALIAS) == ERR)
					return(ERR);
         }
         while(e != FINISH);

		// Determine the length of the commands by concatenating each string 
			for(long i = 0; i < var->GetDimX(); i++)
			{
				cmd = ((char**)var->GetString())[i];
				len += strlen(cmd)+2; // Add two for trailing \r\n
			}

		// Allocate space for the new macro and copy from list
         char* macro = new char[len+1]; // Add one for trailing NULL character
			macro[0] = '\0';

			for(long i = 0; i < var->GetDimX(); i++)
			{
				cmd = ((char**)var->GetString())[i];
				strcat(macro,cmd);
				strcat(macro,"\r\n");
			}

         bool oldInCLI = itfc->inCLI;
		// Run the resultant macro (note: should only have 1 procedure)
			itfc->inCLI = false;
         ProcessMacroStr(itfc,macro);
         itfc->inCLI = oldInCLI;
         delete [] macro;
			return(OK);
		}
      else if(type == CLASS) // If a class function is passed process it
      {
         short RunClassProc(Interface *itfc, Variable *var, char *func, char *args);

         if(arg[0] != '\0')
            return(RunClassProc(itfc, var, "default", arg));
      }
	}
   return(CMD_NOT_FOUND);
}

//short EvaluateClassChain(Interface *itfc, char *command, char *args)
//{


short CheckForClassProcedure(Interface *itfc, char *command, char *arg)
{
   Variable result;
   short r,i;

   long sz = strlen(command);

   for(i = 0; i < sz-1; i++)
   {
      if(i < sz-1 && command[i] == ';') // Ignore statements with multiple commands
         return(CMD_NOT_FOUND);
   }

   for(i = 0; i < sz-1; i++)
   {
      if(command[i] == '-' && command[i+1] == '>') //ARROW
         break;
   }
   if(i == sz-1)
      return(CMD_NOT_FOUND);

//	if(arg[0] != '\0')
//	{
		sz +=  strlen(arg) + 3;
		char *expression = new char[sz];
		strncpy_s(expression,sz,command,_TRUNCATE);
	   strcat(expression,"(");
	   strcat(expression,arg);
	   strcat(expression,")");

      r = EvaluateComplexExpression(itfc,expression,&result);
      delete [] expression;
//	}
//	else
//      r = EvaluateComplexExpression(itfc,command,&result);

   if(itfc->nrRetValues == 1)
   {
      if(CopyVariable(&(itfc->retVar[1]),&result,FULL_COPY) == ERR)
         return(ERR);
   }

		//result.NullData();

   return(r);
}

// If the variable is a class function then see if it can be processed
short RunClassProc(Interface *itfc, Variable *var, char *func, char *args)
{
   if(var->GetType() != CLASS)
   {
      ErrorMessage("variable is not a class");
      return(ERR);
   }

   ClassData *cData = (ClassData*)var->GetData();

   if(CheckClassValidity(cData,true) == ERR)
      return(ERR);

   ProcessClassProcedure(itfc, cData, "", args);

   return(OK);
}


/************************************************************************
   Check to see if command is a procedure and if so run it.
************************************************************************/

short CheckForMacroProcedure(Interface *itfc, char *command, char *arg)
{
   char *text = NULL;
   char macroPath[MAX_PATH];
   char macroName[MAX_PATH];
   char procName[MAX_PATH];
   Variable *procVar = NULL;
   long startNr = 0;

   macroPath[0] = '\0';
   macroName[0] = '\0';
   procName[0] = '\0';

	if(strlen(command) >= MAX_PATH)
		return(CMD_NOT_FOUND);

// If the current procedure has been originally called from a window object
// then it will be cached already. Alternatively it may be cached in the 
// calling macro if the macro has accessed this function before.
   if(itfc->macro)
   {
   // Determine the macroname and the procedurename
      if(command[0] == ':')
      {
	      strncpy_s(procName,MAX_PATH,command+1,_TRUNCATE);
	      strncpy_s(macroName,MAX_PATH,itfc->macro->macroName.Str(),_TRUNCATE);
         strncpy_s(macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE);
      }
      else // macro:proc or macro - path is not known
      {
         strncpy_s(macroName,MAX_PATH,command,_TRUNCATE);
         ExtractProcedureName(macroName,procName);
         GetCurrentDirectory(MAX_PATH,macroPath);
      }

		// Check for globally cached procedures
		if(gCachedProc.next)
		{
		//	TextMessage("Checking for global cache of %s %s %s\n",macroPath,macroName,procName);
         procVar = GetProcedure(macroPath,macroName,procName);
		}

		// Check for window cached procedures
		if(!procVar)
		{
			if(itfc->cacheProc || (itfc->win && itfc->win->cacheProc))
			{
				if(itfc->win)
				{
					procVar = itfc->win->GetProcedure(macroPath,macroName,procName);

					if(procVar)
					   ;//TextMessage("window cache loaded: %s\n",command);
					else // Not found so try base macro
					{
						if(itfc->baseMacro)
						{
							procVar = itfc->baseMacro->GetProcedure(macroPath,macroName,procName);
							if(procVar)
							{
							 ;//  TextMessage("macro cache loaded: %s:%s\n",macroN,procName);
							}
							else // Not found so restore path
								strncpy_s(macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE); 
						}
					}

				}
				else if(itfc->baseMacro) // See if the procedure has already be cached in the current macro
				{
					procVar = itfc->baseMacro->GetProcedure(macroPath,macroName,procName);
					if(procVar)
					{
					 ;//  TextMessage("macro cache loaded: %s:%s\n",macroName,procName);
					}
					else // Not found so restore path
						strncpy_s(macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE);  
				}
			}
			else
			  strncpy_s(macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE);  
		}
	}

   if(procVar) // Yes it has been cached
   {
      text = procVar->GetProcedureText(startNr);
     // TextMessage("%s cached\n",command);
   }
   else // No, must be a new one
   { 
 //     TextMessage("%s not cached\n",command);

	   if(command[0] == ':') // Command has form ':proc' - so search current macro
	   {    
	   // Load parent macro (either from file or text editor)   	         
	      if(macroPath[0] == '\0' && !strcmp(macroName,"current_text"))
         {
            if(!curEditor)
            {   
               ErrorMessage("No current editor selected");
               return(ERR);
            }
	         text = GetText(curEditor->edWin);
         }
	      else     
         {
	         text = LoadTextFileFromFolder(macroPath, macroName,".mac");
         //   TextMessage("Text allocated for %s:%s\n",macroName,command);
         }
			if(text && FindProcedure(text,procName,startNr) == ERR)
			{
	         delete[] text;		   
			   return(ERR); 
			} 
	   }
	   else // Command has form 'macro' or 'macro:proc' - search for macro in normal path
	   {
	      strncpy_s(macroName,MAX_PATH,command,_TRUNCATE);
	      ExtractProcedureName(macroName,procName); // Extract macro filename and procedure name (if any)
	      text = LoadTextFile(itfc,macroPath, macroName, procName,".mac",startNr);
			if(text && text[0] == '\0')
			{
				ErrorMessage("File '%s' in '%s' is empty!",macroName,macroPath);
				return(ERR);
			}
      //   TextMessage("Text allocated for %s:%s\n",macroName,procName);
	   }

   // Save the macro particulars to the procedure run list menu
      if(text)
         AddFilenameToList(procRunList,macroPath, macroName);
	}
   
// Process the macro text ******************************************
   if(text) 
   {
      CText argument;
      CArg carg;
      short i,nrArgs ;
      Variable result;
   // Save the current macro information
      CText oldMacroName = itfc->macroName;
      CText oldMacroPath = itfc->macroPath;
      CText oldProcName = itfc->procName;
      long oldStartLine = itfc->startLine;
      int oldParentLineNr = itfc->parentLineNr;

      nrArgs = carg.Count(arg);
      
   // Set the new macro name and location
      itfc->macroName = macroName;
      itfc->macroPath = macroPath;
      itfc->procName = procName;
      itfc->startLine = startNr;
      itfc->parentLineNr = itfc->lineNr;

   // Save the macro stack information
      MacroStackInfo* info = new MacroStackInfo;
      info->macroPath = oldMacroPath;
      info->macroName = oldMacroName;
      info->procName = oldProcName;
      info->lineNr = itfc->parentLineNr;
      itfc->macroStack.push_back(info);

   // Arguments will be stored in temp variables to prevent overwrite in case of 
   // embedded function calls.
      Variable *tempArgVar = new Variable[nrArgs+1];
      CText *tempArgNames = new CText[nrArgs+1];
      int foundNames = 0;

   // Evaluate and store any argument list in global "arg" variable
      for(i = 1; i <= nrArgs; i++)
      {
         argument = carg.Extract(i);
         char* name = new char[argument.Size()+1];
         char* value = new char[argument.Size()+1];

         // See if the argument includes an equals sign (if not a string
			if(ParseAssignmentString(argument.Str(),name,value) == ERR)
         {
            strcpy(value,name);
            name[0] = '\0';
         }
         else
         {
            foundNames++;
         }
			result.FreeName();
 	      if(Evaluate(itfc,RESPECT_ALIAS,value,&result) < 0) 
         {
            if(!procVar)
               delete[] text;
            delete [] tempArgVar;
            delete [] tempArgNames;
            delete [] name;
            delete [] value;
	         return(ERR); 
         }
         if(CopyVariable(&tempArgVar[i],&result,RESPECT_ALIAS) == ERR)
         {
            if(!procVar)
               delete[] text;
            delete [] tempArgVar;
            delete [] tempArgNames;
            delete [] name;
            delete [] value;
            return(ERR); 
         }

         tempArgNames[i] = name;
         delete [] name;
         delete [] value;
      }

   // Copy from temporary to final variables
      for(i = 1; i <= nrArgs; i++)
      {
			itfc->argVar[i].var->Remove();
         CopyVariable(itfc->argVar[i].var,&tempArgVar[i],RESPECT_ALIAS);
         itfc->argVar[i].name = tempArgNames[i];
      }

   // Note whether all names are passed or not
      if(foundNames == 0)
         itfc->namedArgumentMode = ArgMode::NONE_NAMED;
      else if(foundNames == nrArgs)
         itfc->namedArgumentMode = ArgMode::ALL_NAMED;
      else if(foundNames < nrArgs)
         itfc->namedArgumentMode = ArgMode::SOME_NAMED;

      itfc->nrProcArgs = nrArgs;

   // Remove temporary variables
      delete [] tempArgVar;
      delete [] tempArgNames;

   // Process the macro text
      bool oldInCLI = itfc->inCLI;
      itfc->inCLI = false;

   //   CText oldPath,oldName;
      bool edModified = false;

   // Load the macro into the editor
      if(itfc->debugging && command[0] != ':')
      {
         if(curEditor)
         {
            if(gDebug.mode == "stepinto")
            {
               LoadEditorGivenPath(itfc->macroPath.Str(),itfc->macroName.Str());
               UpdateDebugBreakPointList(curEditor);
               edModified = true;
            }
            else if(gDebug.mode == "runto")
            {
               UpdateDebugBreakPointList(itfc->macroPath.Str(),itfc->macroName.Str());
               edModified = true;
            }
         }
      }

      short r = ProcessMacroStr(itfc, 0, text);

  // Restore the original macro information
      itfc->macroName = oldMacroName;
      itfc->macroPath = oldMacroPath;
      itfc->procName = oldProcName;
      itfc->startLine = oldStartLine;
      itfc->parentLineNr = oldParentLineNr;

      if (itfc->macro)
      {
         gInTryBlock = itfc->macro->inTryBlock;
         //TextMessage("Exiting macro thread ID is %lX %d\n", GetCurrentThreadId(),(int)gInTryBlock);
      }

   // Restore previous macro
      if(edModified)
      {
         if(gDebug.mode == "stepinto" || gDebug.mode == "stepout")
         {
            LoadEditorGivenPath(itfc->macroPath.Str(), itfc->macroName.Str());
            UpdateDebugBreakPointList(curEditor);
         }
         else if(gDebug.mode == "runto")
         {
            UpdateDebugBreakPointList(itfc->macroPath.Str(), itfc->macroName.Str());
         }
      }

      itfc->inCLI = oldInCLI;
  
   // Free up text if not cached
      if(!procVar)
      {
         delete[] text;
     //   TextMessage("Text deleted #3: %Xl\n",(long)text);
      }
      return(r);
   }
   else
      return(CMD_NOT_FOUND); // Command not found
}


/******************************************************************************
   Code for return command
   It evaluates the argument list storing the result in the variables itfc->retVar
   and then returns from the current macro without error
******************************************************************************/

int ReturnFromProcedure(Interface* itfc ,char args[])
{
   CText expression;
   CArg carg;
   Variable result;
   short nrRet;
   long i;

// Count the number of returned variable
   nrRet = carg.Count(args);

// Check for a variable argument list
	if(nrRet == 2)
	{
	   expression = carg.Extract(1);
	   if(expression == "\"...\"")
		{
	      expression = carg.Extract(2);

			// Get the structure continaing the returned variables
			if(Evaluate(itfc,RESPECT_ALIAS,expression.Str(),&result) < 0) 
			{
				 return(ERR);
			}

			if(result.GetType() == STRUCTURE)
			{
            Variable *struc, *svar;

				struc = result.GetStruct();
				svar = struc->next;
			// Count variables in structure
			   nrRet = 0;
				while (svar != NULL)
				{
					nrRet++;
					svar = svar->next;
				}
				if(nrRet > MAX_RETURN_VARIABLES)
				{
					ErrorMessage("More than %d variables returned by endproc",MAX_RETURN_VARIABLES);
					return(ERR);
				}
			// Copy the structure variables to the return variables
				svar = struc->next;
				for(i = 1; i <= nrRet; i++)
				{
			      CopyVariable(&itfc->retVar[i],svar,FULL_COPY);
					svar = svar->next;
				}
			// Note number of returned variables
				itfc->nrRetValues = nrRet;

				return(RETURN_FROM_MACRO);
			}
			//else if(result.GetType() == LIST)
			//{


			//}
			else
			{
				ErrorMessage("Invalid second argument in 'endproc' argument list - should be structure or list");
				return(ERR);
			}
		}
	}

// Make a temporary return variable array
   Variable* retVar = new Variable[nrRet+1];

// Evaluate the returned variables
   for(i = 1; i <= nrRet; i++)
   {
      expression = carg.Extract(i);

	   if(Evaluate(itfc,RESPECT_ALIAS,expression.Str(),&result) < 0) 
      {
          delete [] retVar;
		    return(ERR);
      }
      CopyVariable(&retVar[i],&result,FULL_COPY);
	}

// Copy from temporary to final variables
   for(i = 1; i <= nrRet; i++)
   {
      CopyVariable(&itfc->retVar[i],&retVar[i],FULL_COPY);
   }

// Remove temporary variables
   delete [] retVar;

// Note number of returned variables
   itfc->nrRetValues = nrRet;

   return(RETURN_FROM_MACRO);
}

// Get the names of the arguments used in the calling procedure
int GetArgumentNames(Interface* itfc ,char args[])
{
	short nrArgs = itfc->nrProcArgs;

	if(nrArgs == 0)
	{
		itfc->retVar[1].MakeNullVar();
		itfc->nrRetValues = 1;
		return(OK);
	}

	
	itfc->retVar[1].MakeList(0);

	for(int i = 1; i <= nrArgs; i++)
	{
		char *name = itfc->argVar[i].var->GetName();
		if(name)
		   itfc->retVar[1].AddToList(name);
		else
		    itfc->retVar[1].AddToList("");

	}
	itfc->nrRetValues = 1;
	return(OK);
}


/******************************************************************************
   First command in a procedure - take the form
   procedure(name(arg1,arg2,...));
******************************************************************************/

int ProcedureStart(char arg[])
{
   short i;
   char argList[MAX_STR];
   short len = strlen(arg);
   
   for(i = 0; i < len; i++)
   {
      if(arg[i] == '(')
      {
      	if(ExtractSubExpression(arg,argList,i,'(',')') < 0)
	         return(ERR);
	         
	 //     StartProcedure(argList);
	   }
	}
   return(0);
}

/************************************************************************
Comparator used by ListCommands.

Returns true if a is a prefix of b, otherwise returns false.
************************************************************************/

bool compStr(char* a, char* b)
{
	return (strncmp(a,b,strlen(a)) < 0);
}

/**************************************************************************************
         List all available commands                        
**************************************************************************************/

int ListCommands(Interface* itfc ,char arg[])
{
   short r;

	vector<char*> comList = prospaCommandRegistry->allCommands();
	vector<char*> comListCopy;
	comListCopy.resize(comList.size());
	copy(comList.begin(),comList.end(),comListCopy.begin());
	sort(comListCopy.begin(),comListCopy.end(),compStr);

   CArg carg;
   short nrArgs = carg.Count(arg);

	long cnt = 0;
	char **cList = NULL; 
	vector<char*>::iterator low, high;
	if (nrArgs == 0)//  We want all commands that are not hidden.
   {
		low = comListCopy.begin();
		high = comListCopy.end();
	   for (vector<char*>::iterator it = low; it < high; ++it)
      {
		   if (HIDDEN_COMMAND != prospaCommandRegistry->typeOf(*it))
		   {
			   AppendStringToList(*it,&cList,cnt++);
         }
      }
	}
	else if(nrArgs == 1)
   { // We want a subset of commands.
      CText filter;
      if((r = ArgScan(itfc,arg,1,"filter","e","t",&filter)) < 0)
        return(r);  
      if(filter == "*")
      {
		   low = comListCopy.begin();
		   high = comListCopy.end();
      }
      else
      {
		   low = lower_bound(comListCopy.begin(),comListCopy.end(),filter.Str(),compStr);
		   high = upper_bound(comListCopy.begin(),comListCopy.end(),filter.Str(),compStr);
      }
	   for (vector<char*>::iterator it = low; it < high; ++it)
      {
		   if (HIDDEN_COMMAND != prospaCommandRegistry->typeOf(*it))
		   {
			   AppendStringToList(*it,&cList,cnt++);
         }
      }
	}
   else if(nrArgs == 2)
   {
      CText filter;
      CText categoryStr;
      INT64 category;
    
      if((r = ArgScan(itfc,arg,2,"filter, category","ee","tt",&filter,&categoryStr)) < 0)
        return(r); 

      if(categoryStr == "files")
         category = FILE_CMD;
      else if(categoryStr == "license")
         category = LICENSE_CMD;
      else if(categoryStr == "control")
         category = CONTROL_COMMAND;
      else if(categoryStr == "misc")
         category = GENERAL_COMMAND;
      else if(categoryStr == "math")
         category = MATH_CMD;
      else if(categoryStr == "trig")
         category = TRIG_CMD;
      else if(categoryStr == "lists")
         category = LIST_CMD;
      else if(categoryStr == "strings")
         category = STRING_CMD;
      else if(categoryStr == "struct")
         category = STRUCT_CMD;
      else if(categoryStr == "1d")
         category = ONED_CMD;
      else if(categoryStr == "2d")
         category = TWOD_CMD;
      else if(categoryStr == "3d")
         category = THREED_CMD;
      else if(categoryStr == "gui")
         category = GUI_CMD;
      else if(categoryStr == "dlls")
         category = DLL_CMD;
      else if(categoryStr == "time")
         category = TIME_CMD;
      else if(categoryStr == "matrix")
         category = MATRIX_CMD;
      else if(categoryStr == "fourier")
         category = FOURIER_CMD;
      else if(categoryStr == "variables")
         category = VAR_CMD;
      else if(categoryStr == "color")
         category = COLOR_CMD;
      else if(categoryStr == "sound")
         category = SOUND_CMD;
      else if(categoryStr == "help")
         category = HELP_CMD;
      else if(categoryStr == "macros")
         category = MACRO_CMD;
      else if (categoryStr == "classes")
         category = CLASS_CMD;
      else
         category = NOT_FOUND_CMD;

      if(filter == "*")
      {
		   low = comListCopy.begin();
		   high = comListCopy.end();
      }
      else
      {
		   low = lower_bound(comListCopy.begin(),comListCopy.end(),filter.Str(),compStr);
		   high = upper_bound(comListCopy.begin(),comListCopy.end(),filter.Str(),compStr);
      }

	   for (vector<char*>::iterator it = low; it < high; ++it)
      {
		   INT64 type = prospaCommandRegistry->typeOf(*it);
         if(type & category) 
		   {
			   AppendStringToList(*it,&cList,cnt++);
         }
      }
   }

// Return to user
   itfc->retVar[1].AssignList(cList,cnt);
   itfc->nrRetValues = 1;
   return(0);
}


/**************************************************************************************
   See if 'str' is a valid Prospa command. 
**************************************************************************************/

bool IsACommand(char name[])
{
	return (prospaCommandRegistry->has(name));
}

/**************************************************************************************
   Get the type of a Prospa command.
**************************************************************************************/

LONG64 GetCommandType(char name[])
{  
	return (prospaCommandRegistry->typeOf(name));
}

/**************************************************************************************
   Return the syntax for command name - NULL is not a command                      
**************************************************************************************/

char* GetCommandSyntax(char name[])
{
   char *syntax = NULL;
	const string *st;
   
	if (prospaCommandRegistry->has(name))
   {
		LONG64 t = prospaCommandRegistry->typeOf(name);

		if(strlen(prospaCommandRegistry->syntaxFor(name)->c_str()) > 500)
		{
			TextMessage("Error: syntax string is too long (> 500 characters)\n");
			return(NULL);
		}
		if(!(t & CONTROL_COMMAND))
      {
			st = prospaCommandRegistry->syntaxFor(name);
			if(!st)
			  return(NULL);
	      syntax = new char[strlen(st->c_str())+50];
		   sprintf(syntax, "GENERAL COMMAND:  %s", st->c_str());
		   return(syntax);
      }
      else if(t | CONTROL_COMMAND)
      {
			st = prospaCommandRegistry->syntaxFor(name);
			if(!st)
			  return(NULL);
	      syntax = new char[strlen(st->c_str())+50];
		   sprintf(syntax, "CONTROL COMMAND:  %s", st->c_str());
		   return(syntax);
      }
      else
      {
		   // Command is neither GENERAL nor CONTROL, and hence has no reportable syntax.
		   return NULL;
		}
	}
	else
	{
	// Command does not exist.
		return NULL;
	}
}


/**************************************************************************************
    Print the current working directory                      
**************************************************************************************/

int PrintDirectory(char newDir[])
{
   char currentwd[MAX_PATH];
   
   GetCurrentDirectory(MAX_PATH,currentwd);
   
   TextMessage("\n\n  dir = ");
   TextMessage(currentwd);
   TextMessage("\n");
   return(0);
}


/**************************************************************************************
    Remove the specified file                      
**************************************************************************************/

int RemoveFile(Interface* itfc ,char args[])
{
   short nrArgs;
   CText fileName;

// Get the directory      
   if((nrArgs = ArgScan(itfc,args,1,"file to remove","e","t",&fileName)) < 0)
      return(nrArgs); 

// Check for errors
   if(!DeleteFile(fileName.Str()))
   {
      DWORD err = GetLastError();
      if(err == ERROR_ACCESS_DENIED)
		  itfc->retVar[1].MakeAndSetString("file is read-only");
      else if(err == ERROR_FILE_NOT_FOUND)
         itfc->retVar[1].MakeAndSetString("file not found");     
      else
         itfc->retVar[1].MakeAndSetString("error");     
   }
   else
      itfc->retVar[1].MakeAndSetString("ok");                 
   
   itfc->nrRetValues = 1;

   return(OK);
   
}


/**************************************************************************************
    Copy the specified file src to dst. Return "ok" if completed "error" if failed                     
**************************************************************************************/

int CopyAFile(Interface* itfc ,char args[])
{
   short nrArgs;
   CText src,dst;
   CText check = "false";
   short r;

// Get the directory      
   if((nrArgs = ArgScan(itfc,args,2,"src, dst, [check]","eee","ttt",&src,&dst,&check)) < 0)
      return(nrArgs);

   if(check == "false")
	{
      r = CopyFile(src.Str(),dst.Str(),false);
	   SetFileAttributes( dst.Str(),  GetFileAttributes(dst.Str()) & ~FILE_ATTRIBUTE_READONLY);
	}
   else
	{
      r = CopyFile(src.Str(),dst.Str(),true);
	}

// Check for errors
   if(!r)
      itfc->retVar[1].MakeAndSetString("error");     
   else
      itfc->retVar[1].MakeAndSetString("ok");                 
   
   itfc->nrRetValues = 1;

   return(OK);  
}

/**************************************************************************************
    Copy the selected folders  (Has problems if you copy twice over existing
	 folder it puts it inside the existing folder)
**************************************************************************************/

int CopyAFolder(Interface* itfc ,char args[])
{
   short nrArgs;
   CText src,dst;
	char *source, *destination;
   short r;
	// Get the directories     
   if((nrArgs = ArgScan(itfc,args,2,"src, dst","ee","tt",&src,&dst)) < 0)
      return(nrArgs);

	if(src == dst)
	{
		ErrorMessage("Destination folder should be different from source");
		return(ERR);
	}

	int srcSz = src.Size();
	source = new char[srcSz+2];
	strcpy(source,src.Str());
	source[srcSz+1] = '\0';

	int dstSz = dst.Size();
	destination = new char[dstSz+2];
	strcpy(destination,dst.Str());
	destination[dstSz+1] = '\0';

	SHFILEOPSTRUCT s = { 0 };
	s.hwnd = NULL;
	s.wFunc = FO_COPY;
	s.fFlags = FOF_NO_UI;
 	s.pFrom = source;
	s.pTo = destination;

	int ret = SHFileOperation(&s);

	delete [] source;
	delete [] destination;

	itfc->nrRetValues = 0;

	if(ret == 0)
	{
		return(OK);
	}
	else
	{
		ErrorMessage("Copy folder/dir failed");
		return(ERR);
	}
}
/**************************************************************************************
    Move the specified file src to dst. Return "ok" if completed "error" if failed                     
**************************************************************************************/

int MoveAFile(Interface* itfc ,char args[])
{
   short nrArgs;
   CText src,dst;
   short r;

// Get the directory      
   if((nrArgs = ArgScan(itfc,args,2,"src, dst","ee","tt",&src,&dst)) < 0)
      return(nrArgs);

   r = MoveFileEx(src.Str(),dst.Str(),MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);

// Check for errors
   if(!r)
      itfc->retVar[1].MakeAndSetString("error");     
   else
      itfc->retVar[1].MakeAndSetString("ok");  

   itfc->nrRetValues = 1;
   
   return(OK);  
}

/**************************************************************************************
    Join files src1 and src2 and write to file dst                     
**************************************************************************************/

int JoinFiles(Interface* itfc ,char args[])
{
   short nrArgs;
   CText src1,src2,dst;
   char *data1 = NULL;
	char *data2 = NULL;
   FILE *fp;

// Get the directory      
   if((nrArgs = ArgScan(itfc,args,3,"src1, src2, dst","eee","ttt",&src1, &src2, &dst)) < 0)
      return(nrArgs);

// Open file 1
   if(!(fp = fopen(src1.Str(),"rb")))
	{
	   ErrorMessage("Can't open file '%s'",src1.Str());
	   return(ERR);
	}
	long size1 = GetFileLength(fp);

// Allocate memory for file 1
	if((data1 = new char[size1]) == (char*)0)
	{
	   fclose(fp);
	   ErrorMessage("JoinFiles -> out of memory");
	   return(ERR);
	}

// Read in file 1
	if(fread(data1,1,size1,fp) != size1)
	{
	   fclose(fp);
      delete [] data1;
	   ErrorMessage("Can't read file '%s'",src1.Str());
	   return(ERR);
	}
	fclose(fp);

// Open file 2
   if(!(fp = fopen(src2.Str(),"rb")))
	{
		if (data1)
		{
			delete[] data1;
		}
	   ErrorMessage("Can't open file '%s'",src2.Str());
	   return(ERR);
	}
	long size2 = GetFileLength(fp);

// Allocate memory for file 1
	if((data2 = new char[size2]) == (char*)0)
	{
      delete [] data1;
	   fclose(fp);
	   ErrorMessage("JoinFiles -> out of memory");
	   return(ERR);
	}

// Read in file 2
	if(fread(data2,1,size2,fp) != size2)
	{
	   fclose(fp);
      delete [] data1;
      delete [] data2;
	   ErrorMessage("Can't read file '%s'",src2.Str());
	   return(ERR);
	}
	fclose(fp);

// Write out the total file
   if(!(fp = fopen(dst.Str(),"wb")))
	{
      delete [] data1;
      delete [] data2;
	   ErrorMessage("Can't write file '%s'",dst.Str());
	   return(ERR);
	}

   if(fwrite(data1,1,size1,fp)!= size1)
   {
	   fclose(fp);
      delete [] data1;
      delete [] data2;
	   ErrorMessage("Can't write file '%s'",dst.Str());
	   return(ERR);
	}

   if(fwrite(data2,1,size2,fp)!= size2)
   {
	   fclose(fp);
      delete [] data1;
      delete [] data2;
	   ErrorMessage("Can't write file '%s'",dst.Str());
	   return(ERR);
	}

   delete [] data1;
   delete [] data2;
	fclose(fp);

   return(OK);  
}

/*******************************************************************
  Make a new directory. Syntax mkdir("directory_name")
  Returns : "ok" directory was successfully made
          : "directory exists" but no abort occurs.
          : Other errors cause an abort.
*******************************************************************/

int MakeDirectory(Interface* itfc ,char args[])
{
   short nrArgs;
   long i,j;
   int len;
   int err = 0;
   CText curDir;
   char subDir[MAX_PATH];
   char newDir[MAX_PATH];
   errno = 0;

// Get the directory      
   if((nrArgs = ArgScan(itfc,args,1,"new directory","e","s",newDir)) < 0)
      return(nrArgs); 

// Record current directory
   GetDirectory(curDir);

	err = MakeDirectoryCore(newDir);
//
//// Loop over each sub-directory, making it if necessay
//   len = strlen(newDir);
//   j = 0;
//   for(i = 0; i <= len; i++)
//   {
//      if(newDir[i] == '\\' || i == len)
//      {
//         subDir[j] = '\\';
//         subDir[j+1] = '\0';
//      // Try to move into this directory
//         if(!SetCurrentDirectory(subDir))
//         { // If can't then try and make it
//            err = _mkdir(subDir);
//            if(err == -1 && errno != EEXIST)
//            {
//               if(errno == EACCES)
//                  ErrorMessage("can't make directory '%s' - permission denied",newDir);
//               else
//                  ErrorMessage("can't make directory '%s' - unknown error",newDir);  
//               SetDirectory(curDir);
//               return(ERR);
//            }
//            if(!SetCurrentDirectory(subDir))
//            {
//               ErrorMessage("can't move into directory '%s'",subDir);
//               SetDirectory(curDir);
//               return(ERR);
//            }
//         }
//         else
//           errno = EEXIST; 
//
//         j = 0; 
//      }
//      else
//         subDir[j++] = newDir[i];
//   }

// Check for errors
   if(err == 2)
   {
       itfc->retVar[1].MakeAndSetString("directory exists");
   }
   else if(err == 0)
   {
      ErrorMessage("can't make directory '%s'",newDir);
      SetDirectory(curDir);
      return(ERR);
   }   
   else
	{
     itfc->retVar[1].MakeAndSetString("ok");
	}

   itfc->nrRetValues = 1;
   
// Restore directory
   SetDirectory(curDir);

   return(OK);
   
}

// Make a directory recursively
// Return 1 if ok 0 if error

int MakeDirectoryCore(char *newDir)
{
   short nrArgs;
   long i,j;
   int len;
   int err = 0;
   CText curDir;
   char subDir[MAX_PATH];
	errno = 0;

// Record current directory
   GetDirectory(curDir);

// Loop over each sub-directory, making it if necessay
   len = strlen(newDir);
   j = 0;
   for(i = 0; i <= len; i++)
   {
      if(newDir[i] == '\\' || i == len)
      {
         subDir[j] = '\\';
         subDir[j+1] = '\0';
      // Try to move into this directory
         if(!SetCurrentDirectory(subDir))
         { // If can't then try and make it
            err = _mkdir(subDir);
            if(err == -1 && errno != EEXIST)
            {
               if(errno == EACCES)
                  ErrorMessage("can't make directory '%s' - permission denied",newDir);
               else
                  ErrorMessage("can't make directory '%s' - unknown error",newDir);  
               SetDirectory(curDir);
               return(ERR);
            }
            if(!SetCurrentDirectory(subDir))
            {
               ErrorMessage("can't move into directory '%s'",subDir);
               SetDirectory(curDir);
               return(ERR);
            }
         }
         else
           errno = EEXIST; 

         j = 0; 
      }
      else
         subDir[j++] = newDir[i];
   }

	if(errno != 0 || err == -1)
   {
      if(errno == EEXIST)
      {
         return(2); // Directory exists
      }
      else
      {
         return(0); // Can't make
      } 
	}
	return(1); // Ok

}

/*******************************************************************
  Removes a directory. Syntax rmdir("directory_name")
  Returns : "ok" directory was successfully deleted
          : Errors cause an abort.
*******************************************************************/

int RemoveDirectory(Interface* itfc ,char args[])
{
   short nrArgs;
   char curDir[MAX_PATH];
   char dir[MAX_PATH];

// Get the directory      
   if((nrArgs = ArgScan(itfc,args,1,"directory to remove","e","s",dir)) < 0)
      return(nrArgs); 

// Record current directory
   GetCurrentDirectory(MAX_PATH,curDir);

// If current directory has been deleted then move up one
   if(!strcmp(curDir,dir))
      SetCurrentDirectory("..");

// Try and remove the directory
   errno = 0;
   int err = _rmdir(dir);

// Check for errors
   if(err == -1)
   {
 //     SetCurrentDirectory(dir);

      if(errno == ENOENT)
      {
         ErrorMessage("directory '%s' doesn't exist",dir);
         return(ERR);
      }
      else if(errno == EACCES)
      {
         ErrorMessage("directory '%s' is being used by this or another program",dir);
         return(ERR);
      }
      else if(errno == ENOTEMPTY)
      {
         ErrorMessage("directory '%s' may not be deleted or is not empty",dir);
         return(ERR);
      }
      else
      {
         ErrorMessage("unknown error");
         return(ERR);
      }
   }

	itfc->nrRetValues = 0;
   return(OK);
}



/********************************************************************************
   Change the current directory
********************************************************************************/

int SetCWD(Interface *itfc, char args[])
{
   CText newDir;
   CText currentwd;
   CText verbose = "true";
   short nrArgs;
   short r;

// Prompt user
   GetDirectory(newDir);
   if((nrArgs = ArgScan(itfc,args,1,"new directory, verbose","ee","tt",&newDir,&verbose)) < 0)
      return(nrArgs); 

// Change to the new directory
   r = SetDirectory(newDir);

 // Check to see if the directory change was successful
   if(r == 0)
   {
      if(itfc->inCLI)
      {  
         ErrorMessage("Invalid path specified : '%s'",newDir.Str());
         return(ERR);
      }
      else
      {
         itfc->retVar[1].MakeAndSetFloat(0);
         itfc->nrRetValues = 1;
         return(OK);
      }
   }

   GetDirectory(currentwd);
   
   if(itfc->inCLI && verbose == "true")
   { 
	   TextMessage("\n\n  dir = %s\n",currentwd);
	}
	
	strncpy_s(gCurrentDir,MAX_PATH,currentwd.Str(),_TRUNCATE); // Make sure CLI current directory is updated
 //  TextMessage("\n\n  (cd) dir = %s\n",gCurrentDir);

	if(!itfc->inCLI)
	{
		 itfc->retVar[1].MakeAndSetFloat(1);
		 itfc->nrRetValues = 1;
	}
	else
	{
		 itfc->nrRetValues = 0;
	}

   return(OK);
}

/********************************************************************************
   Returns the current working directory
********************************************************************************/

int GetCWD(Interface *itfc, char args[])
{
   CText currentwd;
   GetDirectory(currentwd);

/*   if(itfc->inCLI)
   {
	   TextMessage("\n\n  dir = %s\n",currentwd.Str());
   }  */ 

   itfc->retVar[1].MakeAndSetString(currentwd.Str());
   itfc->nrRetValues = 1;

   return(OK);
}


/********************************************************************************
   Lists directories and then files (with sizes) in the current folder
   print to CLI and to a list.

   LIST result = ls([[STR range [,STR which[, STR print]]])

   range ... which files/folders to print e.g. p* or *.txt
   which ... one of "all"/"files"/"folders"/"dirs"/"directories" 9default "all")
   print ... one of "yes"/"no" (print to CLI default "yes")

********************************************************************************/

int ListFiles(Interface* itfc ,char argIn[])
{
   WIN32_FIND_DATA findData;
   HANDLE h;
 	char **fileList; 
   long cnt = 0;
   char arg[MAX_STR];
   char range[MAX_STR] = "*";
   CText which = "all";
   CText print = "yes";
   bool pr = false;
   short nrArgs;
	CArg carg;

// Make a local copy of the argument since it shouldn't be modified
   strncpy_s(arg,MAX_STR,argIn,_TRUNCATE);

// Allow user to enter file restriction without quotes if there is only one argument
   nrArgs = carg.Count(arg);
   if(nrArgs == 1)
   {
      if(!isStr(arg)) AddQuotes(arg);
   }

// Get range of file to list and file/dir restriction
   if((nrArgs = ArgScan(itfc,arg,0,"[range [,which, print]]","eee","stt",range,&which,&print)) < 0)
     return(nrArgs);  

// Check the parameters
   if(print != "yes" && print != "no")
   {
      ErrorMessage("invalid 'print' parameter - one of: yes/no");
      return(ERR);
   }

   if(which != "files" && which != "all" && which != "dirs" && which != "directories" && which != "folders")
   {
      ErrorMessage("invalid 'which' parameter - one of: files/dirs/all");
      return(ERR);
   }

// Should we print to cli?
   if(print == "yes")
      pr = true;

   pr = false;

// Intialise the file list 
	fileList = NULL;

// List files *************************
   h = FindFirstFile(range,&findData);

   if(h == INVALID_HANDLE_VALUE)
   {
      itfc->retVar[1].MakeNullVar();
      itfc->nrRetValues = 1;
	//   FreeList(fileList,1);
      if(itfc->inCLI)
         ErrorMessage("no files found");
      return(OK);
   }
   
   if(pr) TextMessage("\n\n   TYPE    %-25s\t%-8s\n","NAME","SIZE");

   if(which == "files" || which == "all")
   {
      if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         if(pr) TextMessage("\n   FILE    %-25s\t%-8ld",findData.cFileName,findData.nFileSizeLow);
	      AppendStringToList(findData.cFileName,&fileList,cnt++);
      }
      while(1)
      {
         if(!FindNextFile(h,&findData)) break;
	      if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
         {
            if(pr) TextMessage("\n   FILE    %-25s\t%-8ld",findData.cFileName,findData.nFileSizeLow);
	         AppendStringToList(findData.cFileName,&fileList,cnt++);
         }
      }
      FindClose(h);
   }


   if(which == "folders" || which == "directories" || which == "dirs" || which == "all")
   {   
   // List directories *************************
      h = FindFirstFile(range,&findData);

      if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         if(pr) TextMessage("\n\n   DIR     %-25s",findData.cFileName);
         AppendStringToList(findData.cFileName,&fileList,cnt++);
      }

      while(1)
      {
         if(!FindNextFile(h,&findData)) break;
	      if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            if(pr) TextMessage("\n   DIR     %-25s",findData.cFileName);
            AppendStringToList(findData.cFileName,&fileList,cnt++);
         }
      }
      if(pr)
         TextMessage("\n");
   
      FindClose(h);
   }

// Return to user
   if(cnt > 0)
   {
      itfc->retVar[1].AssignList(fileList,cnt);
      itfc->nrRetValues = 1;
   }
   else
   {
	   itfc->retVar[1].MakeNullVar();
	   itfc->nrRetValues = 1;
	//   FreeList(fileList,1);
   }

   return(OK);
}


/********************************************************************************
   Search through the current directory and through all sub directories for a file
   Returns the directory where file was found in 'path'. Also modifies file if
   it should contain an extension.

   May need modification to use UNICODE as can get stuck looking through folders
   with non ACSII characters in name
********************************************************************************/
  
short RecursiveSearch(char *file, char extension[], CText &path)
{
   WIN32_FIND_DATAW findData;
   HANDLE h;
   short r;
   CText fileTemp;
   wchar_t tempDir[MAX_PATH];
   wchar_t tempDir2[MAX_PATH];
   static int cnt = 0;
   extern bool wideNarrowStringEqual(bool caseInsensitive, wchar_t* wideStr, char* narrowStr);

   GetCurrentDirectoryW(MAX_PATH,tempDir2); // Save current

// Find first file in current directory
   h = FindFirstFileW(L"*",&findData);
   
   if(h == INVALID_HANDLE_VALUE)
   {
      ErrorMessage("no files found");
      return(ERR);
   }
  
// Look at each file in turn
   do
   {
	   if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	   {
	      if(!(!wcscmp(findData.cFileName,L".") || !wcscmp(findData.cFileName,L"..")))
	      {
	         GetCurrentDirectoryW(MAX_PATH,tempDir); // Save current
	         SetCurrentDirectoryW(findData.cFileName); // Drop down

            // Check for too many nested searches
            if(++cnt == 500)
            {	 
               FindClose(h);
               ErrorMessage("too many directories to search for file/command '%s'",file);
	            return(ERR);
            }
		      r = RecursiveSearch(file,extension,path);
            cnt--;

		      if(r == FOUND)
		      {
	            FindClose(h);
	            return(FOUND);
	         }
	         else if(r == ERR)
	         {
	            FindClose(h);
	            return(ERR);
	         }
	         SetCurrentDirectoryW(tempDir); // Not found so go up
         }
	   }
	   else
	   {
	      if(wideNarrowStringEqual(1,findData.cFileName,file)) // Compare with raw filename
	      {
            GetDirectory(path); // Save path
            FindClose(h);
            wcstombs(file,findData.cFileName, MAX_PATH);
	         return(FOUND);
	      }
	      else // Compare with name and extension
	      {
	         fileTemp.Assign(file);
	         fileTemp.Concat(extension);
	         if(wideNarrowStringEqual(1,findData.cFileName,fileTemp.Str()))
	         {
               GetDirectory(path); // Save path
               wcstombs(file,findData.cFileName, MAX_PATH);  // Make sure the case is correct
               FindClose(h);
	            return(FOUND);
	         }	  
	      }       
	   }
   }
   while(FindNextFileW(h,&findData));

   FindClose(h);
   return(NOT_FOUND);
}

bool wideNarrowStringEqual(bool caseInsensitive, wchar_t* wideStr, char* narrowStr)
{
    const size_t sz = strlen(narrowStr)+1;
    wchar_t* wStr = new wchar_t[sz];
    mbstowcs(wStr, narrowStr, sz);

    if(caseInsensitive)
    {
       if(!wcsicmp(wStr,wideStr))
       {
          delete [] wStr;
          return(true);
       }
    }
    else
    {
       if(!wcscmp(wStr,wideStr))
       {
          delete [] wStr;
          return(true);
       }
    }

    delete [] wStr;
    return(false);
}

void CopyStrWideToNarrow(char* narrowStr, wchar_t* wideStr)
{
    const size_t sz = wcslen(wideStr)+1;
    wcstombs(narrowStr, wideStr, sz);
}


// Search for argName in the calling macro parameter list

short GetArgFromCallingMacro(Interface *itfc, int nrArgs, char* procName, char* argName, int argIdx, Variable **var)
{
   int i = 0;


   if(itfc->namedArgumentMode == ArgMode::ALL_NAMED) // Arguments can be in any order
   {
      for(i = 1; i <= itfc->nrProcArgs; i++) // Loop over passed variables
      {
         if(!strcmp(argName,itfc->argVar[i].name.Str()))
         {
            *var = itfc->argVar[i].var;
            return(OK);
         }
      }
      itfc->argVar[i].name = "";
      *var = 0;
      return(OK);    
   }
   else if(itfc->namedArgumentMode == ArgMode::NONE_NAMED) // Arguments must be in order
   {   
      if(argIdx <= itfc->nrProcArgs)
         *var = itfc->argVar[argIdx].var;
      else
         *var = 0;
      return(OK);
   }
   else // Arguments must be in order
   {
     char* srcName = itfc->argVar[argIdx].name.Str();
      if(srcName[0] == '\0' || !strcmp(argName,srcName))
      {
         if(argIdx <= itfc->nrProcArgs)
         {
            *var = itfc->argVar[argIdx].var;
            return(OK);
         }
         else // Argument not found
         {
            *var = 0;
            return(OK);
         }
      }

      ErrorMessage("Parameter '%s' not found in procedure '%s' argument list or is out of order",argName,procName);
      return(ERR); 
   }

   return(ERR);
}

short macroArgs; // Argument list passed to macro

/******************************************************************
* In a procedure extract the nth argument passed from the call
* Used with varArgs
*******************************************************************/

int GetArgument(Interface* itfc, char args[])
{
   short nrArgs;
   short argNr;

   if ((nrArgs = ArgScan(itfc, args, 1, "argument number", "e", "d", &argNr)) < 0)
      return(nrArgs);

   if (argNr < 0 || argNr >= itfc->nrProcArgs)
   {
      ErrorMessage("Invalid argument number - should be between 0 and %d\n", itfc->nrProcArgs - 1);
      return(ERR);
   }

   itfc->nrRetValues = 1;
   itfc->retVar[1].CopyWithAlias(itfc->argVar[argNr+1].var);
   return(OK);
}


/******************************************************************
* Read arguments to procedure and assign values to local variables 
*******************************************************************/

int StartProcedure(Interface* itfc ,char args[])
{
   short nrArgs;
   CText srcName;
   CText dstName;
   CText procName;
   Variable *dstVar;
   CArg carg;
   long i,j;
   Variable result;
   bool varArgs = false;
   
   if(!itfc || !itfc->macro)
   {
      ErrorMessage("no current macro");
      return(ERR);
   }

   // Grab the argument list
   nrArgs = carg.Count(args) - 1;

   // Extract procedure name
   procName = carg.Extract(1);
   if (procName[0] == '\0')
   {
      ErrorMessage("no procedure name defined");
      return(ERR);
   }

   // Check for variable argument list
   if (nrArgs == 1)
   {
      CText arg1 = carg.Extract(2);
      if (arg1 == "varArgs")
         varArgs = true;
   }
   if(!varArgs)
   {
      if (nrArgs < itfc->nrProcArgs)
      {
         ErrorMessage("more arguments passed to procedure than expected");
         return(ERR);
      }
   }

// Make all variable assigments LOCAL
   itfc->varScope = LOCAL;

   if (varArgs)
   {
      // Save number of arguments as a local variable
      Variable* var = AddLocalVariable(itfc, FLOAT32, "nrArgs");
      var->MakeAndSetFloat((float)itfc->nrProcArgs);
      var->SetPermanent(false);
      var->SetReadOnly(true);

      // Save the parent control if is exists as a local variable
      if (itfc->obj)
      {
         var = AddLocalVariable(itfc, FLOAT32, "parentCtrl");
         var->MakeClass(OBJECT_CLASS, (void*)itfc->obj);
         var->SetPermanent(false);
         var->SetReadOnly(true);
      }
   }
   else // Add any default arguments
   {
      char* name = 0;
      char* value = 0;
      char* argument = 0;
      for (i = 1; i <= nrArgs; i++) // Loop over local argument variables
      {
         argument = carg.Extract(i + 1);
         name = new char[strlen(argument) + 1];
         value = new char[strlen(argument) + 1];
         bool defaultAssignment = false;

         if (strchr(argument, '='))
         {
            if (ParseAssignmentString(argument, name, value) == ERR)
            {
               ErrorMessage("Argument '%s' in procedure '%s' has an invalid default assignment", argument, procName.Str());
               goto err;
            }

            defaultAssignment = true;
         }
         else
         {
            strcpy(name, argument);
         }
         Variable* var = 0;
         if (itfc->nrProcArgs)
         {
            if (GetArgFromCallingMacro(itfc, nrArgs, procName.Str(), name, i, &var) == ERR)
            {
               goto err;
            }
         }

         if (var)
         {
            dstVar = AddLocalVariable(itfc, FLOAT32, name);
            if (CopyVariable(dstVar, var, RESPECT_ALIAS) == ERR)
               return(ERR);
         }
         else
         {
            if (defaultAssignment)
            {
               if (Evaluate(itfc, RESPECT_ALIAS, value, &result) < 0)
               {
                  goto err;
               }

               dstVar = AddLocalVariable(itfc, FLOAT32, name);
               if (CopyVariable(dstVar, &result, RESPECT_ALIAS) == ERR)
               {
                  goto err;
               }
            }
         }
         delete[] name;
         delete[] value;
      }

      // Ensure that we reset the named argument flag
      itfc->namedArgumentMode = ArgMode::NONE_NAMED;

      // Ensure that all named arguments are nulled
      for (i = 1; i <= nrArgs; i++)
      {
         if (itfc->argVar[i].name != "")
            itfc->argVar[i].name = "";
      }

      // Save number of arguments as a variable
      Variable* var = AddLocalVariable(itfc, FLOAT32, "nrArgs");

      var->MakeAndSetFloat((float)itfc->nrProcArgs);
      var->SetPermanent(false);
      var->SetReadOnly(true);

      if (itfc->obj)
      {
         var = AddLocalVariable(itfc, FLOAT32, "parentCtrl");
         var->MakeClass(OBJECT_CLASS, (void*)itfc->obj);
         var->SetPermanent(false);
         var->SetReadOnly(true);
      }
      return(0);

   err:

      delete[] name;
      delete[] value;
   }
   return(ERR);
}



// Make a beep to alert the user
int SoundBell(Interface* itfc ,char[])
{
   MessageBeep(MB_OK);
   return(OK);
}

// Execute a windows program "file" with arguments "args"

int ExecuteFile(Interface* itfc, char args[])
{
   short nrArgs;
   CText fileName;
   CText fileArgs;
   
   if((nrArgs = ArgScan(itfc,args,1,"file args","ee","tt",&fileName,&fileArgs)) < 0)
      return(nrArgs); 


   ShellExecute(prospaWin,"open",fileName.Str(),fileArgs.Str(),"",SW_SHOWNORMAL);
   itfc->nrRetValues = 0;
   return(OK);

}

// Execute a Windows command and wait until it finishes
// File to execute should be the cwd or a full path must be given
// The argument list can be a Prospa list or a string with space delimited arguments

STARTUPINFO startupinfo;
PROCESS_INFORMATION processinfo;

int ExecuteAndWait(Interface* itfc, char args[])
{
   CText cmdline;
   CText file;
   Variable argumentVar;
   CText arguments;
   short nrArgs;
   DWORD exitCode;

   if((nrArgs = ArgScan(itfc,args,1,"file,arguments","ee","qv",&file,&argumentVar)) < 0)
      return(nrArgs);

   if (argumentVar.GetType() == UNQUOTED_STRING)
   {
      arguments = argumentVar.GetString();
    //  arguments.RemoveQuotes();
   }
   else if (argumentVar.GetType() == LIST)
   {
      int sz = argumentVar.GetDimX();
      char** list = argumentVar.GetList();

      for (int i = 0; i < sz; i++)
      {
         arguments.Concat(list[i]);
         arguments.Append(' ');
      }
   }
   else
   {
      ErrorMessage("Invalid argument type");
      return(ERR);
   }

   cmdline.Format("\"%s\" %s",file.Str(),arguments.Str());
   startupinfo.cb = sizeof(STARTUPINFO);
   bool st = CreateProcess(file.Str(),cmdline.Str(),NULL,NULL,false,NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,NULL,NULL,&startupinfo,&processinfo);
   long errNr =  GetLastError();

   WaitForSingleObject(processinfo.hProcess, INFINITE);
   bool result = GetExitCodeProcess(processinfo.hProcess, &exitCode);

   CloseHandle(processinfo.hProcess);

   if(!st)
   {
      ErrorMessage("Execute and wait failed: error number %d",errNr);
      return(ERR);
   }

   itfc->retVar[1].MakeAndSetFloat(exitCode);
   itfc->nrRetValues = 1;

   return(OK);
}

int SetHelpViewer(Interface* itfc ,char args[])
{
   short nrArgs;

   if((nrArgs = ArgScan(itfc,args,1,"program","e","s",helpviewer)) < 0)
      return(nrArgs); 

   itfc->nrRetValues = 0;
   return(OK);
}

int GetProspaVersion(Interface* itfc ,char args[])
{
   short nrArgs;
   CText type = "number";

   if((nrArgs = ArgScan(itfc,args,0,"string/number","e","t",&type)) < 0)
      return(nrArgs); 

   if(type == "string")
   {
      CText txt;
      GetVersionTxt(txt);
      itfc->retVar[1].MakeAndSetString(txt.Str()); 
      itfc->nrRetValues = 1;
   }
   else if(type == "date")
   {
      itfc->retVar[1].MakeAndSetString(VERSION_DATE); 
      itfc->nrRetValues = 1;
   }
	else if(type == "special")
   {
      itfc->retVar[1].MakeAndSetString(VERSION_SPECIAL); 
      itfc->nrRetValues = 1;
   }
   else
   {
      itfc->retVar[1].MakeAndSetFloat(VERSION);
      itfc->retVar[2].MakeAndSetFloat((int)VERSION);
      itfc->retVar[3].MakeAndSetFloat(nint((VERSION - (int)VERSION) * 1000));
      itfc->nrRetValues = 3;
   }

   return(OK);
}

void GetVersionTxt(CText &txt)
{
   int version = (int)VERSION;
   float frac = VERSION-version;
   int subversion = (int)(frac*1000+0.5);
//   frac = frac*10-subversion;
 //  int subsubversion = nint(frac*1000);
 //  txt.Format("%d.%d.%d",version,subversion,subsubversion);
   txt.Format("%d.%d",version,subversion);
}

bool useQuotedStrings = true; // Externally visible; Default is to use quote strings as command arguments

int UseQuotedStrings(Interface* itfc, char args[])
{
   short nrArgs;
   CText response = "false";

   if((nrArgs = ArgScan(itfc,args,1,"program","e","t",&response)) < 0)
      return(nrArgs); 

   if(response == "true")
      useQuotedStrings = true;
   else if(response == "false")
      useQuotedStrings = false;
   else
   {
      ErrorMessage("invalid response (should be true or false)");
      return(ERR);
   }

   return(OK);
}

// Open a specific html or chm file stored in the documentation folder
HWND helpWin;

int OpenHelpFile(Interface* itfc ,char args[])
{
   short nrArgs;
   CText fileName,group;
   CText msg;
   CText viewerArgs;
   CText viewerExe;
   CText reference = ""; 
 //  CText viewer = "internal"; 
   CText dllHelpFolder;
   CText name1,name2;
   short type;
   CText command;
   void GetClassAsString(short type, CText &name);
   void GetObjectTypeAsString(short type, CText &name);
	CArg carg;


	WinData *win1 = rootWin->FindWinByTitle("Command and Macro Help");
	WinData *win2 = rootWin->FindWinByTitle("Prospa Help Viewer");

   nrArgs = carg.Count(args);

   if(nrArgs == 1) // Help requested for an internal command or DLL command
   {
      Variable nameVar;
      if((nrArgs = ArgScan(itfc,args,1,"name1","e","v",&nameVar)) < 0)
         return(nrArgs); 

      if(nameVar.GetType() == CLASS)
      {
         CText name;
         ClassData *cdata = (ClassData*)nameVar.GetData();
         if(CheckClassValidity(cdata,true) == ERR)
            return(ERR);
         type = cdata->type;
         GetClassAsString(type, name);
         if(name == "object")
         {
            ObjectData *obj = (ObjectData*)cdata->data;
            GetObjectTypeAsString(obj->type,name);
         }
         if(win1 || win2)
         {
            CText msg;
            msg.Format("HelpViewer,Classes\\Windows:%s",name.Str());
            SendMessageToGUI(msg.Str(),-1);
            ShowWindow((win1 ? win1->hWnd : win2->hWnd),SW_SHOW);
         }
         else
         {
            command.Format("ProspaHelpViewer(\"Classes\\Windows\",\"%s\")",name.Str());
            ProcessMacroStr(0,NULL,NULL,command.Str(),"","","ProspaHelpViewer.mac","");
         }
         itfc->nrRetValues = 0;
         return(OK);
      }
      else if(nameVar.GetType() == UNQUOTED_STRING)
      {
         name1 = nameVar.GetString();
      }
      else
      {
         ErrorMessage("invalid data type for help");
         return(ERR);
      }

      if(name1.FindSubStr(0,".")  == -1)
      {
         fileName = name1;

         if(IsACommand(fileName.Str()))
         {
            command.Format("ProspaHelpViewer(\"Commands\",\"%s\")",fileName.Str());
            msg.Format("HelpViewer,Commands:%s",fileName.Str());
         }
         else if(GetDLLHelpFolderName(itfc,fileName.Str(), dllHelpFolder))
         {
            command.Format("ProspaHelpViewer(\"%s\",\"%s\")",dllHelpFolder.Str(),fileName.Str());
            msg.Format("HelpViewer,%s:%s",dllHelpFolder.Str(),fileName.Str());
         }
         else
         {
            // Run a command to check for current editor macro
            if(curEditor && IsProcedure(curEditor->edPath,curEditor->edName,"showHelp"))
            {
               command.Format(":showHelp(\"%s\")",fileName);
               if(ProcessMacroStr(0,NULL,NULL,command.Str(),"","",curEditor->edName,curEditor->edPath) == ERR)
               {
                  msg.Format("No help available for '%s'",fileName);
                  MessageDialog(prospaWin,MB_ICONERROR,"Error",msg.Str());
                  ErrorMessage("no help available for '%s'",fileName);
                  return(ERR);
               }
               return(OK);
            }
            // Run a command to check for help procedure in the controlling macro
            else if(IsProcedure(itfc->macroPath.Str(),itfc->macroName.Str(),"showHelp"))
            {
               command.Format(":showHelp(\"%s\")",fileName);
               if(ProcessMacroStr(0,NULL,NULL,command.Str(),"","",itfc->macroName.Str(),itfc->macroPath.Str()) == ERR)
               {
                  msg.Format("No help available for '%s'",fileName);
                  MessageDialog(prospaWin,MB_ICONERROR,"Error",msg.Str());
                  ErrorMessage("no help available for '%s'",fileName);
                  return(ERR);
               }
               return(OK);
            }
            else
            {
               msg.Format("Can't find help for '%s'",fileName);
               MessageDialog(prospaWin,MB_ICONERROR,"Error",msg.Str());
               ErrorMessage("Can't find help for '%s'",fileName);
               return(ERR);
            }
         }

         if(win1 || win2)
         {
            SendMessageToGUI(msg.Str(),-1);
            ShowWindow((win1 ? win1->hWnd : win2->hWnd),SW_SHOW);
         }
         else
         {
            ProcessMacroStr(0,NULL,NULL,command.Str(),"","","ProspaHelpViewer.mac","");
         }
         itfc->nrRetValues = 0;
         return(OK);
      }
      else
      {
         ErrorMessage("invalid argument");
         return(ERR);
      }
   }
   else if(nrArgs == 2) // Help requested for a macro, or a subfile in a CHM file or a book mark in an HTML file  
   {
      if((nrArgs = ArgScan(itfc,args,2,"name1,name2","ee","tt",&name1,&name2)) < 0)
         return(nrArgs); 
      if(name1.FindSubStr(0,".") > 0 && name2.FindSubStr(0,".") == -1)
      {
         fileName = name1;
         reference = name2;
      }
      else if(name1.FindSubStr(0,".") == -1 && name2.FindSubStr(0,".") > 0)
      {
         group = name1;
         fileName = name2;

         fileName.LowerCase(); 

         command.Format("ProspaHelpViewer(\"%s\",\"%s\")",group.Str(),fileName.Str());

			if(win1 || win2)
        {
            CText msg;
            msg.Format("HelpViewer,%s:%s",group.Str(),fileName.Str());
            SendMessageToGUI(msg.Str(),-1);
            ShowWindow((win1 ? win1->hWnd : win2->hWnd),SW_SHOW);
         }
         else
            ProcessMacroStr(0,NULL,NULL,command.Str(),"","","ProspaHelpViewer.mac","");

         itfc->nrRetValues = 0;
         return(OK);
      }
      else
      {
         ErrorMessage("invalid arguments");
         return(ERR);
      }
   }
   else if(nrArgs == 3)
   {
      if((nrArgs = ArgScan(itfc,args,3,"group,file,reference","eee","ttt",&group,&fileName,&reference)) < 0)
         return(nrArgs); 

   }

   fileName.LowerCase(); 

   if(!strcmp(GetExtension(fileName.Str()),"chm"))
   {
      command.Format("%s\\documentation\\%s",applicationHomeDir,fileName.Str());

      if(!IsFile(command.Str()))
      {
         MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","Help file '%s' not found",fileName.Str());
         itfc->nrRetValues = 0;
         return(OK);
      }
      command.Format("%s\\documentation\\%s::/Documents\\%s.htm",applicationHomeDir,fileName.Str(),reference.Str());

      HtmlHelp(GetDesktopWindow(),command.Str(),HH_DISPLAY_TOPIC, 0);
      gKeepCurrentFocus = true;
      itfc->nrRetValues = 0;
      return(OK);
   }
   else if(!strcmp(GetExtension(fileName.Str()),"htm"))
   {
      command.Format("%s\\documentation\\%s",applicationHomeDir,fileName.Str());

      if(!IsFile(command.Str()))
      {
         MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","Help file '%s' not found",fileName.Str());
         return(OK);
      }
      viewerExe.Format("%s.exe",helpviewer);
      viewerArgs.Format("file:///%s\\documentation\\%s#%s",applicationHomeDir,fileName.Str(),reference.Str());

      ShellExecute(prospaWin,"open",viewerExe.Str(),viewerArgs.Str(),"",SW_SHOW);
      return(OK);
   }
   else if(!strcmp(GetExtension(fileName.Str()),"") && nrArgs == 2) // Backward compatibility
   {
      command.Format("%s\\documentation\\%s.htm",applicationHomeDir,fileName.Str());

      if(!IsFile(command.Str()))
      {
         MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","Help file '%s' not found",fileName.Str());
         return(OK);
      }
      viewerExe.Format("%s.exe",helpviewer);
      viewerArgs.Format("file:///%s\\documentation\\%s.htm#%s",applicationHomeDir,fileName.Str(),reference.Str());

      ShellExecute(prospaWin,"open",viewerExe.Str(),viewerArgs.Str(),"",SW_SHOW);
      return(OK);
   }

	else if (0 == strlen(fileName.Str()))
	{
		if(win1 || win2)
		{
			ShowWindow((win1 ? win1->hWnd : win2->hWnd),SW_SHOW);
		}
		else
		{
			command.Format("ProspaHelpViewer(\"General Information\\Introduction\",\"Introduction.htm\")");
			ProcessMacroStr(0,NULL,NULL,command.Str(),"","","ProspaHelpViewer.mac","");
		}
		itfc->nrRetValues = 0;
	}

   else
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","Help for '%s' not found",fileName.Str());
   }
   
                                            
   return(OK);
}

int DisplayProcInfo(Interface* itfc, char args[])
{
   short nrArgs;
   CText name = "";
   CText macroName;
   CText procName;
   char cwd[MAX_PATH];
   char calledMacro[MAX_STR];
   char calledPath[MAX_STR];
   char calledProcedure[MAX_STR];
   char fullMacroName[MAX_STR];
   FILE* fp;
   extern short FindProcedureComment(char* text, char* funcName, long& lineNr, bool verbose);

   if ((nrArgs = ArgScan(itfc, args, 1, "procedure name", "e", "t", &name)) < 0)
      return(nrArgs);

   GetCurrentDirectory(MAX_PATH, cwd);

   strcpy(calledPath, cwd);

   long pos = name.FindSubStr(0,":");
   if (pos != -1)
   {
      macroName = name.Start(pos - 1);
      procName = name.End(pos + 1);
   }
   else
   {
      macroName = name;
      procName = name;
   }

   strcpy(calledMacro, macroName.Str());

   if (!(fp = FindFolder(itfc, calledPath, calledMacro, ".mac")))
   {
      if (!(fp = FindFolder(itfc, calledPath, calledMacro, ".pex")))
      {
         ErrorMessage("Can't find macro '%s'", calledMacro);
         return(ERR);
      }
   }
   fclose(fp);

   // Load the file text
   char* text = LoadTextFileFromFolder(calledPath, calledMacro, ".mac");
   long lineNr = 0;
   if (text)
   {
      int pos = 0;
      // Find the procedure
      if (FindProcedureComment(text, procName.Str(), lineNr, false) == ERR)
      {
         delete[] text;
         ErrorMessage("Can't find procedure '%s'", procName.Str());
         return(ERR);
      }
      TextMessage("\n\n%s\n", text);
      delete[] text;
      itfc->nrRetValues = 0;
      return(OK);
   }
   else
   {
      ErrorMessage("Can't find text for macro '%s'", calledMacro);
      return(ERR);
   }
}


void GetClassAsString(short type, CText &name)
{
   switch(type)
   {
      case(OBJECT_CLASS):
	      name = "object"; break;
      case(WINDOW_CLASS):
	      name = "window"; break;
      case(PLOT_CLASS):
	      name = "plotRegion"; break;
      case(XLABEL_CLASS):
	      name = "xlabel"; break;
      case(YLABEL_CLASS):
	      name = "ylabel"; break;
      case(AXES_CLASS):
	      name = "axes"; break;
      case(GRID_CLASS):
	      name = "grid"; break;
      case(TITLE_CLASS):
	      name = "title"; break;
      case(TRACE_CLASS):
	      name = "trace"; break;
		case(INSET_CLASS):
			name = "inset"; break;
      default:
	      name = "unknown"; break;
   }
}


void GetObjectTypeAsString(short type, CText &name)
{
   switch(type)
   {
      case(RADIO_BUTTON):
	      name = "radioButton"; break;
      case(CHECKBOX):
	      name = "checkBox"; break;
      case(STATICTEXT):
	      name = "staticText"; break;
      case(TEXTBOX):
	      name = "textBox"; break;
      case(TEXTMENU):
	      name = "textMenu"; break;
      case(LISTBOX):
	      name = "listBox"; break;
      case(SLIDER):
	      name = "slider"; break;
      case(GETMESSAGE):
	      name = "getMessage"; break;
      case(PROGRESSBAR):
	      name = "progressBar"; break;
      case(STATUSBOX):
	      name = "statusBox"; break;	         	         	         
      case(COLORSCALE):
	      name = "colorScale"; break;
      case(GROUP_BOX):
	      name = "groupBox"; break;
      case(BUTTON):
	      name = "button"; break;	  
      case(COLORBOX):
	      name = "colorBox"; break;
      case(UPDOWN):
	      name = "updownControl"; break;	
      case(DIVIDER):
	      name = "divider"; break;	
      case(TEXTEDITOR):
	      name = "textEditor"; break;	
      case(CLIWINDOW):
	      name = "cli"; break;	
      case(PLOTWINDOW):
	      name = "plot1d"; break;	
      case(IMAGEWINDOW):
	      name = "plot2d"; break;
      case(OPENGLWINDOW):
	      name = "plot3d"; break;
		case(GRIDCTRL):
			name = "gridControl"; break;
      default:
	      name = "unknown"; break;
   }
}

/******************************************************************************
  Return the current macro line number. That is the line which is 
  currently being executed in the macro file. (Zero based).
  By passing the argument "parent" it will return the line number in the 
  calling procedure.
******************************************************************************/

int GetCurrentMacroLineNr(Interface *itfc, char *args)
{
   CText mode = "current";
   short nrArgs;

   if((nrArgs = ArgScan(itfc,args,0,"mode","e","t",&mode)) < 0)
	   return(nrArgs);

   if(mode == "current") 
      itfc->retVar[1].MakeAndSetFloat(itfc->lineNr);
   else
      itfc->retVar[1].MakeAndSetFloat(itfc->parentLineNr);

   itfc->nrRetValues = 1;

   return(OK);
}

/******************************************************************************
  Select the specified line number in the current editor. This will also 
  cause the editor to scroll to make this line visible
******************************************************************************/

int SelectEditorLine(Interface *itfc, char *args)
{
   short nrArgs;
   long lineNr;

   if((nrArgs = ArgScan(itfc,args,0,"lineNr","e","l",&lineNr)) < 0)
	   return(nrArgs);

	if(curEditor)
      curEditor->SelectEditorLine(lineNr);
   itfc->nrRetValues = 0;

   return(OK);
}

/******************************************************************************
  Select which method is used to set the editor focus before colored text 
  rendering (on most computer one method will be faster than the other)
******************************************************************************/

int SetEditorRenderMode(Interface *itfc, char *args)
{
   short nrArgs;
   CText mode;

   if(gFocusForRendering == CURRENTEDITOR)
      mode = "current";
   else
      mode = "parent";

   if((nrArgs = ArgScan(itfc,args,1,"mode","e","t",&mode)) < 0)
	   return(nrArgs);

   if(mode == "current")
      gFocusForRendering = CURRENTEDITOR;
   else
      gFocusForRendering = PARENTWINDOW;

   itfc->nrRetValues = 0;

   return(OK);
}


//declare the function
int CALLBACK EnumFontFamProc(
ENUMLOGFONTEX *lpelf, // pointer to logical-font data
NEWTEXTMETRICEX *lpntm, // pointer to physical-font data
int FontType, // type of font
LPARAM lParam // pointer to application-defined data
);

struct FontInfo
{
   char **list;
   long cnt;
   CText script;
};

int CALLBACK EnumFontFamProc(ENUMLOGFONTEX *lpelf, NEWTEXTMETRICEX *lpntm, int FontType, LPARAM lParam )
{
   FontInfo* fontInfo = (FontInfo*)lParam;  

   if(fontInfo->script == (char*)(lpelf->elfScript) && lpelf->elfLogFont.lfFaceName[0] != '@')
   {
      int sz = strlen(lpelf->elfLogFont.lfFaceName);
      fontInfo->list[fontInfo->cnt] = new char[sz+1];
      strcpy(fontInfo->list[fontInfo->cnt],lpelf->elfLogFont.lfFaceName);
      fontInfo->cnt++;
   }
   return(true);
}

// Get the number of font entries
int CALLBACK EnumFontFamProcSz(ENUMLOGFONTEX *lpelf, NEWTEXTMETRICEX *lpntm, int FontType, LPARAM lParam )
{
   FontInfo* fontInfo = (FontInfo*)lParam;  

   if(fontInfo->script == (char*)(lpelf->elfScript))
   {
      fontInfo->cnt++;
   }
   return(true);
}


int GetFontList(Interface* itfc ,char args[])
{
   FontInfo fontInfo;
   CText script = "Western";
   short nrArgs;

   if((nrArgs = ArgScan(itfc,args,0,"script","e","t",&script)) < 0)
	   return(nrArgs);

   fontInfo.list = NULL;
   fontInfo.cnt = 0;
   fontInfo.script = script;

   HDC hdc = GetDC(prospaWin);

 //call the EnumFontFamiliesEx API function
   LOGFONT lf;
   lf.lfFaceName[0] = '\0';
   lf.lfCharSet = DEFAULT_CHARSET;
   lf.lfPitchAndFamily = 0;
   EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROC)EnumFontFamProcSz,(LPARAM)&fontInfo,0);

   fontInfo.list = new char*[fontInfo.cnt];
   fontInfo.cnt = 0;
   EnumFontFamiliesEx(hdc,&lf,(FONTENUMPROC)EnumFontFamProc,(LPARAM)&fontInfo,0);

   ReleaseDC(prospaWin,hdc);

   itfc->retVar[1].AssignList(fontInfo.list,fontInfo.cnt);
   itfc->nrRetValues = 1;

   return(OK);
}

#include <malloc.h>

// Extract information from the registry. Currently only support values of string type REG_SZ and REG_EXPAND_SZ
// Useful for extracting locations of applications.

int GetRegistryInfo(Interface* itfc ,char arg[])
{
	int nrArgs;
	CText key;
	CText valueName = "";
	CText valueType = "REG_SZ";
	CText hiveStr = "HKCR";
	HKEY hSubKey;
	HKEY hive;

	if((nrArgs = ArgScan(itfc,arg,2,"hive, key, [value, type]","eeee","tttt",&hiveStr,&key,&valueName,&valueType)) < 0)
	   return(nrArgs);

	if(nrArgs == 1 || nrArgs == 3)
	{
		ErrorMessage("invalid number of arguments (2 or 4)");
		return(ERR);
	}

	if(hiveStr == "HKEY_CLASSES_ROOT")
		hive = HKEY_CLASSES_ROOT;
	else if(hiveStr == "HKEY_CURRENT_USER")
		hive = HKEY_CURRENT_USER;
	else if(hiveStr == "HKEY_LOCAL_MACHINE")
		hive = HKEY_LOCAL_MACHINE;
	else if(hiveStr == "HKEY_USERS")
		hive = HKEY_USERS;
	else if(hiveStr == "HKEY_CURRENT_CONFIG")
		hive = HKEY_CURRENT_CONFIG;
	else
	{
		ErrorMessage("invalid or unsupported registry hive name");
		return(ERR);
	}

	long result = RegOpenKey(hive, key.Str(), &hSubKey);

	if(result == ERROR_SUCCESS)
	{
     DWORD bufferSize = MAX_PATH;
	  DWORD type;

		if(valueType == "REG_SZ")
		{
			if(RegQueryValueEx(hSubKey,valueName.Str(),NULL,&type,NULL,&bufferSize) == ERROR_SUCCESS)
			{
				char *value = new char[bufferSize+1];
				if(!value)
				{
					ErrorMessage("Can't allocate memory for registry value");
					itfc->nrRetValues = 1;
					RegCloseKey(hSubKey);
					return(ERR);
				}
			   if(RegQueryValueEx(hSubKey,valueName.Str(),NULL,&type,(LPBYTE)value,&bufferSize) == ERROR_SUCCESS)
				{
					if(type == REG_SZ)
					{
						itfc->retVar[1].MakeAndSetString(value);
					}
					else
					{
						itfc->retVar[1].MakeNullVar();			 
					}
					delete [] value;
					itfc->nrRetValues = 1;
					RegCloseKey(hSubKey);
					return(OK);
				}
			}
		}
		
		else if(valueType == "REG_EXPAND_SZ")
		{
			if(RegQueryValueEx(hSubKey,valueName.Str(),NULL,&type,NULL,&bufferSize) == ERROR_SUCCESS)
			{
				char *value = new char[bufferSize+1]; 
				if(!value)
				{
					ErrorMessage("Can't allocate memory for registry value");
					itfc->nrRetValues = 1;
					RegCloseKey(hSubKey);
					return(ERR);
				}
				if(RegQueryValueEx(hSubKey,valueName.Str(),NULL,&type,(LPBYTE)value,&bufferSize) == ERROR_SUCCESS)
				{
					if(type == REG_EXPAND_SZ)
					{
						bufferSize = ExpandEnvironmentStrings(value, 0, 0);
						char *valueOut = new char[bufferSize+2]; 
						if(!valueOut)
						{
							ErrorMessage("Can't allocate memory for registry value");
							itfc->nrRetValues = 1;
							delete [] value;
							RegCloseKey(hSubKey);
							return(ERR);
						}
						bufferSize = ExpandEnvironmentStrings(value, valueOut, MAX_PATH);
						itfc->retVar[1].MakeAndSetString(valueOut);
						delete [] valueOut;
					}
					else
					{
						itfc->retVar[1].MakeNullVar();			 
					}
					delete [] value;
					itfc->nrRetValues = 1;
					RegCloseKey(hSubKey);
					return(OK);
				}
			}
		}
	}
   RegCloseKey(hSubKey);
	itfc->retVar[1].MakeNullVar();
	itfc->nrRetValues = 1;
	return(OK);
}

// Print the current macro call stack
int PrintProcedureStack(Interface* itfc, char arg[])
{
   TextMessage("\n--- Stack trace ----\n");
   int sz = itfc->macroStack.size();
   for (int i = 0; i < sz; i++)
   {
      TextMessage("%*s   Macro: '%s', Procedure: '%s', LineNr: %d\n", i * 3, " ", itfc->macroStack[i]->macroName.Str(), itfc->macroStack[i]->procName.Str(), itfc->macroStack[i]->lineNr+1);
   }
   TextMessage("%*s   Macro: '%s', Procedure: '%s', LineNr: %d\n", sz * 3, " ", itfc->macroName.Str(), itfc->procName.Str(), itfc->lineNr+1);

   return(OK);
}


//
//short ProcessPlotClassReferences2(Interface *itfc, Plot *pd, char* name, char *args)
//{
//   short r;
//   CArg carg;
//   short nrArgs = carg.Count(args);
//
//   if(!pd)
//   {
//      ErrorMessage("Plot not defined");
//      return(ERR);
//   }
//
//
//
//   // Remove all text from a plot
//	if (!strcmp(name,"rmtext"))
//	{
//      for(int i = 0; i < pd->text_.size(); i++)
//      {
//         PlotText *txt = pd->text_[i];
//         delete txt;
//      }
//      pd->text_.clear();
//		pd->DisplayAll(false);
//		itfc->nrRetValues = 0;
//		return(OK);	
//    
//   }
//
//	else
//   {
//      ErrorMessage("Unknown or invalid plot function '%s'",name);
//      return(ERR);
//   }
//}

int testFunc(Interface *itfc, char *args)
{
   Plot *pd;

   pd = Plot1D::curPlot();

   for(int i = 0; i < pd->text_.size(); i++)
   {
      PlotText *txt = pd->text_[i];
      delete txt;
   }

   pd->text_.clear();
	//pd->DisplayAll(false);
	//itfc->nrRetValues = 0;
	return(OK);	

  // int  r = ProcessPlotClassReferences2(itfc, pd, "rmtext", "");


   return(OK);
}

void RemoveLines(Plot *pd)
{
   std::for_each(pd->lines_.begin(), pd->lines_.end(), delete_object());
   pd->lines_.clear();

}

// Display a blocking popup window showing all the procedure in the selected macro
// in either the CLI or the current editor
//
//int ShowMacroProcedures(Interface* itfc ,char args[])
//{
//	long startSel,endSel;
//   CText source = "editor";
//   short nrArgs;
//   extern void UpDateEditSyntax(WinData *win, EditRegion *edReg, long startSel, long endSel, bool showProcs);
//   extern void UpDateCLISyntax(WinData *win, HWND edWin, long startSel, long endSel, bool showProcs);
//
//	if((nrArgs = ArgScan(itfc,args,0,"cli/editor","e","t",&source)) < 0)
//	   return(nrArgs);
//
//   if(source == "editor")
//   {
//		extern CText *gClassName, *gClassMacro;
//      extern int gNrClassDeclarations;
//      long startWord,endWord;
//      char *name;
//		bool classCmd=false;
//		bool funcCmd=false;
//		short userClassCmd=0;
//		char **list = 0;
//		int cnt = 0;
//      GetEditSelection(curEditor->edWin,startWord,endWord);	      
//      name = ExpandToFullWord(curEditor->edWin," +-*/%^=(,[$\t<>:"," ()\n",startWord,endWord,classCmd,userClassCmd,funcCmd);
//		if(classCmd)
//		{
//			for(int i = 0; i < endWord-startWord; i++)
//			{
//				if(name[i] == '-' && name[i+1] == '>')
//				{
//					name[i] = '\0'; // The class name
//					Variable *procList = &gCachedProc;
//					CText macroName = name;
//					int n = gNrClassDeclarations;
//					for(int i = 0; i < n; i++)
//					{
//						if(gClassName[i] == name)
//						{
//							macroName = gClassMacro[i] + ".mac";
//						   cnt = 0;
//							// Search in global cache for this macro and count number of procedures
//							for(Variable *var = procList->next; var != NULL; var = var->next)
//							{
//								ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();
//								if(!strcmp(procInfo->macroName,macroName.Str()))
//				               cnt++;
//							}
//							if(cnt > 0)
//							{
//								// Make a list which can hold the names
//								list = MakeList(cnt);
//								cnt = 0;
//								// Now add the names to the list
//								for(Variable *var = procList->next; var != NULL; var = var->next)
//								{
//									ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();
//									if(!strcmp(procInfo->macroName,macroName.Str()))
//									{
//										AppendStringToList(procInfo->procName, &list, cnt++);
//									}
//								}
//							}
//							else // Can't find the macro in global cache so now try file system
//							{
//
//							}
//							break;
//						}
//					}
//					itfc->retVar[1].MakeAndSetString("class");
//					itfc->retVar[2].MakeAndSetString(name);
//					itfc->retVar[3].MakeAndSetString(macroName.Str());
//					itfc->retVar[4].AssignList(list,cnt);
//					itfc->nrRetValues = 4;
//					break;
//				}
//			}
//		}
//		else if(funcCmd)
//		{
//			for(int i = 0; i < endWord-startWord; i++)
//			{
//				if(name[i] == ':')
//				{
//					name[i] = '\0'; // The macro name
//					Variable *procList = &gCachedProc;
//					CText macroName = name;
//					macroName = macroName + ".mac";
//					cnt = 0;
//					for(Variable *var = procList->next; var != NULL; var = var->next)
//					{
//						ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();
//						if(!strcmp(procInfo->macroName,macroName.Str()))
//				         cnt++;
//					}
//					if(cnt == 0)
//					{
//						macroName = name;
//						macroName = macroName + ".pex";
//						cnt = 0;
//						for(Variable *var = procList->next; var != NULL; var = var->next)
//						{
//							ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();
//							if(!strcmp(procInfo->macroName,macroName.Str()))
//								cnt++;
//						}
//					}
//               list = MakeList(cnt);
//					cnt = 0;
//					for(Variable *var = procList->next; var != NULL; var = var->next)
//					{
//						ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();
//						if(!strcmp(procInfo->macroName,macroName.Str()))
//						{
//                     AppendStringToList(procInfo->procName, &list, cnt++);
//						}
//					}
//
//					itfc->retVar[1].MakeAndSetString("macro");
//					itfc->retVar[2].MakeAndSetString(name);
//					itfc->retVar[3].MakeAndSetString(macroName.Str());
//					itfc->retVar[4].AssignList(list,cnt);
//					itfc->nrRetValues = 4;
//					break;
//				}
//			}
//		}
//		delete [] name;
//
//   }
//   return(OK);
//}



#include "evaluate_simple.h"

short GetStructArrayVariable(Interface *itfc, char *expression, short &start, char *arrayName, Variable **arrayVar)
{
   long x,y,z,q;                            // Array indices (fixed)
   long *xa = 0,*ya = 0,*za = 0,*qa=0;      // Array indices (list)
   Variable *dstVar = *arrayVar;                        // Array variable referred to by 'arrayName'
   short dstType = (*arrayVar)->GetType();                           // Array type of 'arrayName'

   
// Extract array type, variable and index (or indices)
   if(ExtractArrayAddress(itfc, arrayName,expression,x,y,z,q,&xa,&ya,&za,&qa,&dstVar,dstType,start) < 0)
   {
      return(ERR);
   }

   if(dstType != STRUCTURE_ARRAY)
   {
      ErrorMessage("Expecting a structure array");
      return(ERR);
   }

   Variable *var = (Variable*)(*arrayVar)->GetData();
   *arrayVar = &(var[x]);

   return(OK);

}

// A fill command - does nothing
int DoNothing(Interface *itfc, char args[])
{
	itfc->nrRetValues = 0;
	return(OK);
}

/********************************************************************************
* Assign expVar to varName. 
*
* varName = expVar
*********************************************************************************/

int AssignTest(Interface *itfc, char args[])
{

 //  CText macroName,procName;
	short nrArgs;
   long value;

 // // extern  short LoadAndSelectProcedure(char *macroName, char *procName, bool showErrors);

   if((nrArgs = ArgScan(itfc,args,1,"value","e","l",&value)) < 0)
	   return(nrArgs);

   DWORD_PTR  processAffinityMask = value;

   BOOL success = SetProcessAffinityMask(GetCurrentProcess(), processAffinityMask);


   if (value)
      TextMessage("\nResult is true\n");
   else
      TextMessage("\nResult is false\n");


 //  SetFocus(curEditor->edWin);
 //  SetFocus(curEditor->edParent->parent->hwndParent);

 //  curEditor->LoadAndSelectProcedure(macroName.Str(), procName.Str(), TRUE);
 
   return(OK);
}

/********************************************************************************
* Determines whether DLLs will be searched for a command or not. Useful if a
* file command has the same name as a DLL
*********************************************************************************/

int SearchDLLs(Interface *itfc, char *args)
{
   short n;
   CText search;

   if ((n = ArgScan(itfc, args, 1, "true/false", "e", "t", &search)) < 0)
      return(n);

   if (search == "true")
   {
      searchDLLs = true;
   }
   else if (search == "false")
   {
      searchDLLs = false;
   }
   return(OK);
}


/********************************************************************************
* Search for a running Prospa application with the specificed title
* and then send it a file to open 
*********************************************************************************/

int SendToExistingProspa(Interface* itfc, char args[])
{
   short nrArgs;
   CText windowTitle;
   CText pathToFile;
   CText fileToOpen;

   if((nrArgs = ArgScan(itfc,args,3,"window_title, path_to_file, file_name","eee","ttt",&windowTitle,&pathToFile, &fileToOpen)) < 0)
	   return(nrArgs);

   HWND win = FindWindow(NULL,windowTitle.Str());

   if(!win)
   {
      itfc->retVar[1].MakeAndSetFloat(0);
      itfc->nrRetValues = 1;
      return(OK);
   }

   long sizeDir = strlen(pathToFile.Str());
   long sizePath = strlen(fileToOpen.Str());

   // Send the path
   SendMessage(win,WM_USER_LOADDATA,(WPARAM)sizeDir,0);
   for(int i = 0; i < sizeDir; i++)
   {
      SendMessage(win,WM_USER_LOADDATA,(WPARAM)(long)pathToFile.Str()[i],0);
   }

   // Send the file
   SendMessage(win,WM_USER_LOADDATA,(WPARAM)sizePath,0);
   for(int i = 0; i < sizePath; i++)
   {
      SendMessage(win,WM_USER_LOADDATA,(WPARAM)(long)fileToOpen.Str()[i],0);
   }

   itfc->retVar[1].MakeAndSetFloat(1);
   itfc->nrRetValues = 1;
   return(OK);
}



class RemoteCom
{
public:
   RemoteCom()
   {
      winList.clear();
   }
   static BOOL CALLBACK EnumWindowsCallBack(HWND hWnd, LPARAM lParam);
   static BOOL CALLBACK EnumWindowsCallBack2(HWND hWnd, LPARAM lParam);
   vector<long> winList;
   CText reply;
};

typedef struct 
{
   COPYDATASTRUCT* pcds;
   RemoteCom* rc;
} RemoteInfo;

// Run the commands specified in lParam in other instances of Prospa
BOOL CALLBACK RemoteCom::EnumWindowsCallBack(HWND hWnd, LPARAM info)
{
   char name[MAX_STR];
   extern bool gReceivedCopyDataMessage;
   extern CText gCopyDataRetMessage;

   GetClassName(hWnd, name, MAX_STR);

   if (!strcmp(name, "MAIN_PROSPAWIN") && hWnd != prospaWin)
   {
      gReceivedCopyDataMessage = false;

      SendMessage(hWnd, WM_COPYDATA, (WPARAM)prospaWin, (LPARAM)(((RemoteInfo*)info)->pcds));

      while (!gReceivedCopyDataMessage)
      {
         Sleep(1);
      }

      RemoteCom* com = ((RemoteInfo*)info)->rc;
      com->reply = gCopyDataRetMessage;

   }

   return(true);
}

// Run the commands specified in lParam in other instances of Prospa
BOOL CALLBACK RemoteCom::EnumWindowsCallBack2(HWND hWnd, LPARAM info)
{
   char name[MAX_STR];
   extern bool gReceivedCopyDataMessage;
   extern CText gCopyDataRetMessage;

   GetClassName(hWnd, name, MAX_STR);


   if (!strcmp(name, "MAIN_PROSPAWIN") && hWnd != prospaWin)
   {
     // HWND parent = GetParent(hWnd);
     // WinInfo* win = (WinInfo*)GetWindowLong(hWnd, GWL_USERDATA);


      RemoteCom* com = ((RemoteInfo*)info)->rc;
      com->winList.push_back((long)hWnd);
   }

   return(true);
}
int FindProspaWindows(Interface* itfc, char args[])
{
   RemoteInfo remoteInfo;
   RemoteCom com;

   remoteInfo.rc = &com;

   EnumWindows(com.EnumWindowsCallBack2, (LPARAM)&remoteInfo);

   int sz = com.winList.size();
   double **winList = MakeDMatrix2D(sz, 1);
   for (int i = 0; i < sz; i++)
      winList[0][i] = (double)com.winList[i];
   itfc->retVar[1].AssignDMatrix2D(winList,sz,1);
   itfc->nrRetValues = 1;

   return(OK);
}

/********************************************************************************
* Run a macro in other instances of Prospa
********************************************************************************/



int RunRemoteMacro(Interface* itfc, char args[])
{
   short nrArgs;
   CText macroToRun;
   COPYDATASTRUCT pcds;
   RemoteCom com;
   Variable winVar;
   unsigned long windowNr = 0;

   if ((nrArgs = ArgScan(itfc, args, 1, "Macro to run, [window]", "ee", "tv", &macroToRun, &winVar)) < 0)
      return(nrArgs);

   if (nrArgs == 2)
   {
      if (winVar.GetType() == FLOAT64)
         windowNr = (long)winVar.GetDouble();
      if (winVar.GetType() == FLOAT32)
         windowNr = (long)winVar.GetReal();
   }

   HWND win = FindWindow("MAIN_PROSPAWIN", NULL);

   if (!win)
   {
      itfc->retVar[1].MakeAndSetFloat(0);
      itfc->nrRetValues = 1;
      return(OK);
   }

   long szMessage = macroToRun.Size();

   RemoteInfo remoteInfo;

   pcds.dwData = 2;
   pcds.cbData = szMessage;
   pcds.lpData = macroToRun.Str();
   remoteInfo.pcds = &pcds;
   remoteInfo.rc = &com;

   if (windowNr == 0)
   {
      EnumWindows(com.EnumWindowsCallBack, (LPARAM)&remoteInfo);
      itfc->retVar[1].MakeAndSetString(com.reply.Str());
      itfc->nrRetValues = 1;
      return(OK);
   }
   else
   {
      extern bool gReceivedCopyDataMessage;
      extern CText gCopyDataRetMessage;

      gReceivedCopyDataMessage = false;

      SendMessage((HWND)windowNr, WM_COPYDATA, (WPARAM)prospaWin, (LPARAM)&pcds);

      while (!gReceivedCopyDataMessage)
      {
         Sleep(1);
      }
      itfc->retVar[1].MakeAndSetString(gCopyDataRetMessage.Str());
      itfc->nrRetValues = 1;
      return(OK);
   }
}



// Allow the user to change some of the user interface design. At present this only affects buttons which can be displayed in windows 7 style.
// Hopefully with time this will also apply to other controls like checkboxes, scrollbars, textboxes etc.

int SetUISkin(Interface *itfc, char arg[])
{
   short nrArgs;
   extern bool win7Mode;
   CText mode = "default";

   if(win7Mode)
      mode = "win7";

  if((nrArgs = ArgScan(itfc,arg,1,"mode","e","t",&mode)) < 0)
	   return(nrArgs);

  if(mode == "win7")
     win7Mode = true;
  else
     win7Mode = false;

   return(OK);
}

int CopyToClipBoard(Interface *itfc, char args[])
{
	short nrArgs;
	CText value;
	LPTSTR  lptstrCopy; 
   HGLOBAL hglbCopy;

   if((nrArgs = ArgScan(itfc,args,1,"string","e","t",&value)) < 0)
	   return(nrArgs);

	itfc->nrRetValues = 0;
	int sz = value.Size();
	if(sz == 0)
		return(OK);

// Prepare the global clipboard
	if (!OpenClipboard(prospaWin)) 
		return(OK); 
	EmptyClipboard(); 
// Allocate a global memory buffer for the text. 
   hglbCopy = GlobalAlloc(GHND | GMEM_SHARE, sz+1); 
// Lock the handle and copy the selected text to the buffer.  
   lptstrCopy = (LPSTR)GlobalLock(hglbCopy); 
   memcpy(lptstrCopy, value.Str(), sz); 
	lptstrCopy[sz] = '\0';
   GlobalUnlock(hglbCopy); 

// Place the handle on the clipboard. 
   SetClipboardData(CF_TEXT, hglbCopy); 
   CloseClipboard(); 


	return(OK);

}

int EvaluateHexString(Interface *itfc, char args[])
{
	CText mode = "signed";
	CText data;
	short nrArgs;
	char temp1[100];
	char temp2[100];
	int result;
	bool isNeg = false;
	int off = 0;
	int bits = 16;

   if((nrArgs = ArgScan(itfc,args,1,"string, bits, mode","eee","tdt",&data, &bits, &mode)) < 0)
	   return(nrArgs);

	strcpy(temp1,data.Str());

	if(temp1[0] == '0' && temp1[1] == 'x')
	{
		off = 2;
		strcpy(temp2,"0x");
	}

	char c = temp1[off];
	if(c == '8' || c == '9' || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' ||
   	c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f')
		 isNeg = true;

	int sz = strlen(temp1);

	if(isNeg && sz <= 8+off)
	{
		int i,j;

		for(i = off; i < 8-sz+off+off; i++)
		{
			temp2[i] = 'F';
		}
		for(j = 0; j < sz; j++)
		{
			temp2[j+i] = temp1[j+off];
		}
		temp2[j+i ] = '\0';
	}
	else
	{
		strcpy(temp2,temp1);
	}

	if(off == 0)
	{
		strcpy(temp1,"0x");
		strcat(temp1,temp2);
	}
	else
		strcpy(temp1,temp2);

	 sscanf(temp1,"%x",&result);

   itfc->retVar[1].MakeAndSetFloat((float)result);
   itfc->nrRetValues = 1;

   return(OK);

}
