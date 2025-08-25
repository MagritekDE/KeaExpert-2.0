#ifndef TWODFUNCTIONS_H
#define TWODFUNCTIONS_H 1

class Variable; 

// Single precision
short FloatMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short MatrixFloatEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short CMatrixFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short FloatCMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CompCMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrixCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short MatrixMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrixMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short MatrixCMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CMatrixCMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short MatrixCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CompMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);

// Double precision
short FloatDMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short DMatrixFloatEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short DoubleDMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short DMatrixDoubleEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short DMatrixDMatrixEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);

#endif // define TWODFUNCTIONS_H