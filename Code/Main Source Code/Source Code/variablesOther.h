#ifndef VARIABLESOTHER_H
#define VARIABLESOTHER_H

#include "defines.h"

class Interface;
class Variable;

// Global variables
extern Variable globalVariable; // Global variable list

// Function prototypes for variables_other.cp

EXPORT Variable* AddVariable(Interface *itfc, short, short, char[]);
Variable* GetVariable(Interface *itfc, short scope, short restrictions, char name[], short &type);
EXPORT Variable* GetVariable(Interface *itfc, short,char[], short&);
EXPORT Variable* AddLocalVariable(Interface*,short,char[]);
Variable* AddGlobalVariable(short type, char name[]);
EXPORT int  ReturnVariableType(Interface* itfc ,char arg[]);
EXPORT short CopyVariable(short, char*, char*, short);
short CopyVariable(Variable*, Variable*, short);
EXPORT  void GetVariableTypeAsString(short, char*);
EXPORT int MakeVariableAlias(Interface* itfc ,char arg[]);
EXPORT int DefineWindowVariables(Interface* itfc ,char arg[]);
EXPORT int IsAVariable(Interface* itfc ,char arg[]);
bool IsAVariableCore(Interface *itfc, char name[]);
long GetNextObjectValidationCode();
short CheckClassValidity(ClassData* cd, bool displayError);
int SetVariableStatus(Interface *itfc, char args[]);
int AllowNonLocalVariables(Interface* itfc ,char args[]);
int RemoveVariableByName(Interface* itfc ,char arg[]);
int SwapVariables(Interface* itfc ,char args[]);
int DealiasVariable(Interface *itfc, char arg[]);
void DeleteGlobalVariables(void);

// Variable types ***************
#define CHARACTER 			0
#define UNQUOTED_STRING 	1
#define QUOTED_STRING 		2

#define INTEGER 				3
#define FLOAT32   		   4
#define COMPLEX 				5

#define MATRIX2D				6
#define CMATRIX2D			   7
#define LIST    			   8
#define MATRIX3D				9
#define CMATRIX3D			   10
#define MATRIX4D				11
#define CMATRIX4D			   12
#define PROCEDURE          13
#define OPERATOR_TOKEN     14
#define NULL_VARIABLE      15
#define STRUCTURE          16
#define STRUCTMEMBER       17
#define CLASS              18
#define CLASSITEM          19
#define FLOAT64   		   20
#define BREAKPOINTINFO     21
#define DMATRIX2D				22
#define LIST2D    		   23
#define STRUCTURE_ARRAY    24
#define FILENAME           25
#define PATHNAME           26
#define BLANKSTR           27
#define FLOAT32ORBLANK     28
#define FLOAT64ORBLANK     29
#define QUOTELESS_LIST     30
#define HEX                31



// Variable scope
#define ALL   -1
#define LOCAL  0
#define GLOBAL 1
#define WINDOW 2

// Some macros to provide backward compatibility
#define VarColSize(var)          ((var)->GetDimX())
#define VarRowSize(var)          ((var)->GetDimY())
#define VarTierSize(var)         ((var)->GetDimZ())
#define VarWidth(var)            ((var)->GetDimX())
#define VarHeight(var)           ((var)->GetDimY())
#define VarDepth(var)            ((var)->GetDimZ())
#define VarHyper(var)            ((var)->GetDimQ())

#define VarType(var)             ((var)->GetType())
#define VarName(var)             ((var)->GetName())

#define VarString(var)           ((var)->GetString())
#define VarList(var)             ((var)->GetList())
#define VarList2D(var)           ((var)->GetList2D())
#define VarReal(var)             ((var)->GetReal())
#define VarInteger(var)          ((var)->GetLong())
#define VarComplex(var)          ((var)->GetComplex())

#define VarRealMatrix(var)       ((var)->GetMatrix2D())
#define VarDoubleMatrix(var)     ((var)->GetDMatrix2D())
#define VarComplexMatrix(var)    ((var)->GetCMatrix2D())

#define VarReal3DMatrix(var)     ((var)->GetMatrix3D())
#define VarComplex3DMatrix(var)  ((var)->GetCMatrix3D())
#define VarReal4DMatrix(var)     ((var)->GetMatrix4D())
#define VarComplex4DMatrix(var)  ((var)->GetCMatrix4D())

#endif // define VARIABLESOTHER_H