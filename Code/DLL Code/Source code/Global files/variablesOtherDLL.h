// Function prototypes for variables_other.cp


extern Variable* AddGlobalVariable(short type, char name[]);
EXPORT extern int  DefineLocalVariables(char arg[]);
EXPORT extern int  DefineGlobalVariables(char arg[]);
EXPORT extern int  IsNullVariable(char arg[]);
EXPORT extern short CopyVariable(short, char*, char*, short);
extern short CopyVariable(Variable*, Variable*, short);
EXPORT extern void GetVariableTypeAsString(short, char*);
EXPORT extern short AssignVariable(short, char*, char*);

// Global variables
extern Variable globalVariable; // Global variable list
//extern short variableScope;     // Scope for newly defined variables

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