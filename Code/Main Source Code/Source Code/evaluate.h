#ifndef EVALUATE_H
#define EVALUATE_H

#include "defines.h"

class Interface;
class Variable;

int EvaluateExpression(Interface *itfc, char arg[]);
int EvaluateSubExpression(Interface* itfc, char args[]);
short GetNextOperandCT(short &pos, char str[], CText &operand);
int GetMatrixList(Interface* itfc ,char args[]);

short Evaluate(Interface* itfc,short mode ,char *expression, Variable *result);
char* GetOpString(unsigned char op);
int   NewComplexMatrix(Interface* itfc ,char[]);
int RealToStr(Interface* itfc ,char args[]);

short CheckForSimpleEval(char *str);
char* GetOperationString(short, unsigned char);
//short EvaluateComplexExpression(char *expression);
short ExtractSubExpression(char expression[],char subexpression[],short &i,char delleft,char delright);
void SetUpPrecedence(void);

EXPORT extern float StringToFloat(char *s);
double StringToDouble(char *s);
double StringToNumber(char *s, short &type);
short GetNextOperand(short &pos,char str[],char operand[]);
short GetNextOperator(short &pos,char str[],unsigned char &op);
void SkipWhiteSpace(char *expression, short &i);
bool IsOperandDelimiter(char *txt, long pos, long len); 
short ExtractArrayAddress2(char arrayName[], char expression[],
                           long **xa, long **ya, long **za, long **qa,
                           long &xs, long &ys, long &zs,
                           Variable **var, short &varType, short &i);
int NewMatrix(Interface* itfc ,char[]);
int NewDMatrix(Interface* itfc ,char[]);
char GetNumType(float number);
char GetNumType(double number);
short CheckForSign(char*,short&);

// (Environment) variables
int ListEnvironmentVariables (char args[]);
int GetUniqueVariableName(Interface* itfc, char[]);
int GetVariableList(Interface* itfc, char[]);
short ReplaceVarInString(Interface *itfc, CText&);
void ConvertVariableToText(Variable *var, CText &str, char *format, bool useFormat);
void ConvertListVariableToText(Variable *var, CText &str, char *format, bool useFormat);
void Convert2DListVariableToText(Variable *var, CText &str, char *format, bool useFormat);
void ConvertScalarVariableToText(Variable *var, CText &str, char *format, bool useFormat);		
void ConvertRealMatrixVariableToText(Variable *var, CText &str, char *format, bool useFormat);		
void ConvertComplexMatrixVariableToText(Variable *var, CText &str, char *format, bool useFormat);		

extern short precedence[]; 
#define NORMAL 0
#define ASSIGN 1

#endif // define EVALUATE_H