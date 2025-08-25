#ifndef THREEDFUNCTIONS_H
#define THREEDFUNCTIONS_H 1

class Variable;

short Matrix3DMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short Matrix3DCMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short Matrix3DCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short Matrix3DFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CompMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short FloatMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short FloatCMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short ComplexCMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrix3DComplexEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrix3DCMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrix3DMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrix3DFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);

#endif // define THREEDFUNCTIONS_H