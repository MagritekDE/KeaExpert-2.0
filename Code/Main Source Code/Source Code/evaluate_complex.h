#ifndef EVALUATE_COMPLEX_H
#define EVALUATE_COMPLEX_H

#include "defines.h"

class Variable;
class Interface;

short EvaluateComplexExpression(char *expression, Variable *result);
short EvaluateComplexExpression(Interface *itfc, char *expression, Variable *resVar);
short ProcessClassProcedure(Interface *itfc, ClassData *cls, char *name, char *args);

#endif //define EVALUATE_COMPLEX_H
