#ifndef EVALUATE_SIMPLE_H
#define EVALUATE_SIMPLE_H

#include "defines.h"

class Interface;
class Variable;

EXPORT short Evaluate(void *par ,short mode ,char *expression, Variable *result);
short EvaluateSimpleExpression(Interface *itfc, short mode ,char *expression, Variable *result);
short EvaluateSimpleOperand(Interface *itfc, char * operand, short dataType, Variable *result);
short EvaluateFunctionOperand(Interface *itfc, char *function, char *expression, short &i, Variable *result);
short EvaluateArrayOperand(Interface *itfc, char *arrayName, char *expression, short &pos, Variable *result);
short EvaluateBracketedExpression(Interface *itfc, char *expression, short &i, Variable *result);
short EvaluateArrayCore(Interface *itfc, Variable *var, char expression[],
                          long x, long y, long z, long q,
                          long *xa, long *ya, long *za, long *qa,
								  short &type,
                          Variable *result);
short EvaluateArrayOperand(Interface *itfc, char *arrayName, Variable *arrayVar, char *expression, Variable *result);
short ExtractArrayAddress(Interface*, char[], char[], long&, long&, long&, long&,
											long**, long**, long**, long**,
											Variable**, short&, short&);
#endif //define EVALUATE_SIMPLE_H