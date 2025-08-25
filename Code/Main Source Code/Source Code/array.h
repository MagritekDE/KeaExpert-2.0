#ifndef ARRAY_H
#define ARRAY_H

class Interface;
class Variable;

short AssignArray(Interface *itfc, char*, short, char*);
short AssignArray(Interface *itfc, char *expression, short start, char *arrayName, Variable *srcVar);
short AssignArray(Interface *itfc, char *expression, short &start, char *arrayName,Variable *dstVar, short dstType,  Variable *srcVar);
short AssignArrayCore(Interface *itfc, char arrayName[], char expression[],
                          long x, long y, long z, long q,
                          long *xa, long *ya, long *za, long *qa,
                          Variable *dstVar, short &dstType, Variable *srcVar, short&);

#endif // define ARRAY_H