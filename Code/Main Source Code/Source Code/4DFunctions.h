#ifndef FOURDFUNCTIONS_H
#define FOURDFUNCTIONS_H

class Variable;

short Matrix4DMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrix4DCMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrix4DMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short Matrix4DCMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrix4DFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short FloatCMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrix4DComplexEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short ComplexCMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short ComplexMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short Matrix4DComplexEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short Matrix4DFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short FloatMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);

short Process4DCMatrixAssignment(Variable *dstVar, Variable *srcVar,
											long x, long y, long z, long q,
											long *xa, long *ya, long *za, long *qa);

short Process4DMatrixAssignment(Variable *dstVar, Variable *srcVar,
										  long x, long y, long z, long q,
										  long *xa, long *ya, long *za, long *qa);


#endif //define FOURDFUNCTIONS_H