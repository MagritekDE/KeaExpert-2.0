#ifndef FOURIER_H
#define FOURIER_H

#include "defines.h"

class Interface;

int FourierTransform(Interface* itfc ,char args[]);
void DataReorder(complex *data, long size);
void DataReorder(float *data, long size);
void DataReorder(double *data, long size);
int InverseTransform(Interface* itfc ,char args[]);
int HilbertTransform(Interface* itfc ,char[]);
int RealFourierTransform(Interface* itfc ,char args[]);

#endif // define FOURIER_H
