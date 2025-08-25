#include "stdafx.h"
#include "include.h"

int BiotSavart(char arg[]);

int
BiotSavart(char arg[])
{
// Some variables

   float dlx,dly;
   float drx,dry,drz;
   float dBz;
   float theta;
   long i,j;
   long xi,yi,zi;
   float x,y,z;
   
// Some constants

	float pi     = 3.1415927;  // 
	float R      = 0.127;      // Coil radius in metres
	float maxz   = 0.2;        // maximum z value for calculation
	float minz   = -0.2;       // minimum z value for calculation
	float stepz  = (maxz-minz)/50;       // step size of z value
	float dtheta = pi/50;      // step size for theta
	float len    = 0.399;      // length of coil in metres
	float N      = 100;         // Number of turns in simulation
	float NT     = 372;        // Total number of turns
	float I      = 5;          // Current in coil
   float minx   = -0.06;
   float maxx   = 0.06;
   float stepx  = (maxx-minx)/50;
   float miny   = -0.06;
   float maxy   = 0.06;
   float stepy  = (maxy-miny)/50;
      
// Initialize segment vectors *****************

	long st = nint(N*((2*pi)/dtheta));
	float *lx1 = Vector(0,st-1);
	float *lx2 = Vector(0,st-1);
	float *ly1 = Vector(0,st-1);
	float *ly2 = Vector(0,st-1);
	float *lz1 = Vector(0,st-1);
	float *lz2 = Vector(0,st-1);

// Calculate end coordinates of segments ******

   z = -len/2+len/N/2;
   
   for(i = 0; i < st; i++)
   { 
      lx1[i] = R*cos(theta); 
      lx2[i] = R*cos(theta+dtheta);
      ly1[i] = R*sin(theta);
      ly2[i] = R*sin(theta+dtheta);
      lz1[i] = z;
      lz2[i] = z; 
      theta += dtheta;
      z += len/N/(2*pi/dtheta);
      if(theta >= 2*pi) theta = 0;
   }

// Calculate field B along the z axis for each segment

	long sx = nint((maxx-minx)/stepx);
	long sy = nint((maxy-miny)/stepy);
	long sz = nint((maxz-minz)/stepz);

   float ***B = Matrix3D(sx,sy,sz);
 
   z = minz;
   for(zi = 0; zi < sz; zi++)
   {   
      y = miny; // Initial y value
	   for(yi = 0; yi < sy; yi++)
	   {
	      x = minx; // Initial x value
		   for(xi = 0; xi < sx; xi++)  // Step along z axis
		   {
		      B[zi][yi][xi] = 0;
		      for(j = 0; j < st; j++) // Loop over coil segments
		      {
			      dlx = lx2[j]-lx1[j];
			      dly = ly2[j]-ly1[j];
			      drx = x - 0.5*(lx1[j]+lx2[j]);
			      dry = y - 0.5*(ly1[j]+ly2[j]);
			      drz = z - 0.5*(lz1[j]+lz2[j]);
			      R = sqrt(drx*drx+dry*dry+drz*drz);
			      dBz = (dlx*dry-dly*drx)/(R*R*R);
			      B[zi][yi][xi] += I*NT/N*1e-7*dBz;
			   }
			   x += stepx; // Increment x
			}
			y += stepy;
	   }
	   TextMessage("\n   zi = %ld",zi);
	   z += stepz;
	}
   

// Return field

//	Variable *var = AddVariable(GLOBAL, MATRIX3D,"ans");  
	ansVar->MakeAndLoad3DMatrix(B, sx, sy, sz);
   	
	FreeVector(lx1,0,st-1);
	FreeVector(lx2,0,st-1);
	FreeVector(ly1,0,st-1);
	FreeVector(ly2,0,st-1);
	FreeVector(lz1,0,st-1);
	FreeVector(lz2,0,st-1);
	Free3DMatrix(B,sx,sy,sz);
	
	return(OK);
}
