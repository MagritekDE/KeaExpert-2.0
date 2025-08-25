#include "stdafx.h"
#include "opengl.h"
#include <math.h>
#include <gl/gl.h> 
#include <gl/glu.h>
#include "memoryLeak.h"

#define GLOBE    1 
#define CYLINDER 2 
#define CONE     3 

BOOL SetupPrintPixelFormat(HDC hdc);

GLfloat latitude, longitude, latinc, longinc; 
GLdouble radius; 

bool setFormat = false;


// Reduces a normal vector specified as a set of three coordinates,
// to a unit normal vector of length one.
void ReduceToUnit(float vector[3])
{
	float length;
	
	// Calculate the length of the vector		
	length = (float)sqrt((vector[0]*vector[0]) + 
						(vector[1]*vector[1]) +
						(vector[2]*vector[2]));

	// Keep the program from blowing up by providing an exceptable
	// value for vectors that may calculated too close to zero.
	if(length == 0.0f)
		length = 1.0f;

	// Dividing each element by the length will result in a
	// unit normal vector.
	vector[0] /= length;
	vector[1] /= length;
	vector[2] /= length;
}


// Points a, b, & c specified in counter clock-wise order
// represent a triangular surface. Return the normal to 
// this surface in out

void CalcNormal(float *a, float *b, float *c, float *out)
{
	float v1[3],v2[3];
	static const int x = 0;
	static const int y = 1;
	static const int z = 2;

	// Calculate two vectors from the three points
	v1[x] = a[x] - b[x];
	v1[y] = a[y] - b[y];
	v1[z] = a[z] - b[z];

	v2[x] = b[x] - c[x];
	v2[y] = b[y] - c[y];
	v2[z] = b[z] - c[z];

	// Take the cross product of the two vectors to get
	// the normal vector which will be stored in out
	out[x] = v1[y]*v2[z] - v1[z]*v2[y];
	out[y] = v1[z]*v2[x] - v1[x]*v2[z];
	out[z] = v1[x]*v2[y] - v1[y]*v2[x];

	// Normalize the vector (shorten length to one)
//	ReduceToUnit(out);
}
	




