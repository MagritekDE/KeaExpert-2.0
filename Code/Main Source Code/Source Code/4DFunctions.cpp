#include "stdafx.h"
#include "4DFunctions.h"
#include <math.h>
#include "allocate.h"
#include "evaluate.h"
#include "globals.h"
#include "mymath.h"
#include "operators.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/******************************************************************************************
   Evaluate a 4D MATRIX operating on a 4D COMPLEX MATRIX - result is a 4D COMPLEX matrix
******************************************************************************************/

short Matrix4DCMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z,q;
   long width = dataRight->GetDimX();
   long height = dataRight->GetDimY();
   long depth = dataRight->GetDimZ();
   long hypers = dataRight->GetDimQ();
   float ****rmat = dataLeft->GetMatrix4D();
   complex ****cmat = dataRight->GetCMatrix4D();
   complex ****result = 0;
   
// Make sure matrix dimensions are the same if not matrix multiple
	if(operation != '*')
	{
	   if(height != dataLeft->GetDimY() || width != dataLeft->GetDimX() || 
         depth != dataLeft->GetDimZ()  || hypers != dataLeft->GetDimQ())
	   {
	      ErrorMessage("matrix dimensions don't match");
	      return(ERR);
	   }
	  
		// Allocate space for result which will be complex            
		result = MakeCMatrix4D(width,height,depth,hypers);
		if(!result) 
		{
			ErrorMessage("can't allocate memory for 4D matrix - 4D cmatrix operation"); 
			return(ERR);
		}      
	}

// Evaluate expression   
   switch(operation)
   { 
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     result[q][z][y][x].r = rmat[q][z][y][x] + cmat[q][z][y][x].r;
                     result[q][z][y][x].i = cmat[q][z][y][x].i;
                  }
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     result[q][z][y][x].r = rmat[q][z][y][x] - cmat[q][z][y][x].r;
                     result[q][z][y][x].i = -cmat[q][z][y][x].i;
                  }
               }
            }
         }
         break;
      }

      case (MATMUL) : // .*
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     result[q][z][y][x].r = rmat[q][z][y][x]*cmat[q][z][y][x].r;
                     result[q][z][y][x].i = rmat[q][z][y][x]*cmat[q][z][y][x].i;
                  }
               }
            }
         }
         break;
       }
      
      case ('/') : 
      { 
         float denom;
          

         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     denom = cmat[q][z][y][x].r*cmat[q][z][y][x].r + cmat[q][z][y][x].i*cmat[q][z][y][x].i;
                     if(denom == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     }
	                  result[q][z][y][x].r = (rmat[q][z][y][x]*cmat[q][z][y][x].r)/denom;
	                  result[q][z][y][x].i = (-rmat[q][z][y][x]*cmat[q][z][y][x].i)/denom;	
                  }
               }
            } 
         }
         break;
      }       

      default: 
         ErrorMessage("'%s' is an undefined operation\n           between real and complex 4D matricies",GetOpString(operation));
         return(ERR); 
   }

   dataLeft->AssignCMatrix4D(result,width,height,depth,hypers);
   return(OK);
}

/******************************************************************************************
   Evaluate a 4D COMPLEX MATRIX operating on a 4DMATRIX - result is a 4D COMPLEX MATRIX
******************************************************************************************/

short CMatrix4DMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z,q;
   long width = dataRight->GetDimX();
   long height = dataRight->GetDimY();
   long depth = dataRight->GetDimZ();
   long hypers = dataRight->GetDimQ();
   float ****rmat = dataRight->GetMatrix4D();
   complex ****cmat = dataLeft->GetCMatrix4D();
   
// Make sure matrix dimensions are the same if not matrix multiple
	if(operation != '*')
	{
	   if(height != dataLeft->GetDimY() || width != dataLeft->GetDimX() || 
         depth != dataLeft->GetDimZ()  || hypers != dataLeft->GetDimQ())
	   {
	      ErrorMessage("matrix dimensions don't match");
	      return(ERR);
	   }
   }

// Evaluate expression
   switch(operation)
   { 
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
                     cmat[q][z][y][x].r = cmat[q][z][y][x].r + rmat[q][z][y][x];
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
                     cmat[q][z][y][x].r = cmat[q][z][y][x].r - rmat[q][z][y][x];
         break;
      }
      case (MATMUL) : // .*
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     cmat[q][z][y][x].r = cmat[q][z][y][x].r * rmat[q][z][y][x];
                     cmat[q][z][y][x].i = cmat[q][z][y][x].i * rmat[q][z][y][x];
                  }
               }
            }
         }
         break;
      }
     
      case ('/') : 
      {
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     if(rmat[q][z][y][x] == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     } 
                     cmat[q][z][y][x].r = cmat[q][z][y][x].r/rmat[q][z][y][x];
                     cmat[q][z][y][x].i = cmat[q][z][y][x].i/rmat[q][z][y][x];
                  }
               }
            }
         }
         break;
      }

      default: 
         ErrorMessage("'%s' is an undefined operation\n           between real and complex matricies",GetOpString(operation));
         return(ERR); 
    }
    return(OK);
}


/******************************************************************************************
   Evaluate a REAL 4D MATRIX operating on a REAL 4D MATRIX - result is a REAL 4D MATRIX
******************************************************************************************/

short Matrix4DMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z,q;
   long width = dataLeft->GetDimX();
   long height = dataLeft->GetDimY();
   long depth = dataLeft->GetDimZ();
   long hypers = dataLeft->GetDimQ();
   float ****src = dataRight->GetMatrix4D();
   float ****dst = dataLeft->GetMatrix4D();
   
   if(operation != '*' && (dataRight->GetDimY() != height || dataRight->GetDimX() != width || 
                           dataRight->GetDimZ() != depth  || dataRight->GetDimQ() != hypers))
   {
      ErrorMessage("matrix sizes don't agree in '%s' operation",GetOpString(operation));
      return(ERR);
   }
   
   switch(operation)
   { 
      case (EQ) :
      case ('=') :
      {
         long neq = 0;
	       
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
	                  neq += nint(FloatNotEqual(dst[q][z][y][x],src[q][z][y][x]));
         dataLeft->MakeAndSetFloat(neq == 0);
         return(OK);
      }	
      case (NEQ) :
      {
         long neq = 0;
	       
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
                     neq += nint(FloatNotEqual(dst[q][z][y][x],src[q][z][y][x]));
         dataLeft->MakeAndSetFloat(neq != 0);
	      return(OK);
      }	         
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
                     dst[q][z][y][x] = dst[q][z][y][x] + src[q][z][y][x];
         break;
      }
      case ('-') :
      {
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
                     dst[q][z][y][x] = dst[q][z][y][x] - src[q][z][y][x];
         break;
      }
      case (MATMUL) :
      { 
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
                     dst[q][z][y][x] = dst[q][z][y][x] * src[q][z][y][x];
         break;
      } 
      case ('/') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     if(src[q][z][y][x] == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     } 
                     dst[q][z][y][x] = dst[q][z][y][x]/src[q][z][y][x];
                  }
               }
            }
         }
         break;
      }
      case ('^') :
      {
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
                  dst[q][z][y][x] = pow(dst[q][z][y][x],src[q][z][y][x]);
         break;
      }
      default: 
         ErrorMessage("'%s' is an undefined operation\n           between 4D matrices",GetOpString(operation));
         return(ERR); 
    }
	 return(OK);
}





/******************************************************************************************
Evaluate a 4D COMPLEX MATRIX operating on a 4D COMPLEX MATRIX - result is a 4D COMPLEX MATRIX
******************************************************************************************/

short CMatrix4DCMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z,q;
   complex temp;
   long width = dataRight->GetDimX();
   long height = dataRight->GetDimY();
   long depth = dataRight->GetDimZ();
   long hypers = dataRight->GetDimQ();
   complex ****src = dataRight->GetCMatrix4D();
   complex ****dst = dataLeft->GetCMatrix4D();
   
   if(operation != '*' && (dataLeft->GetDimY() != height || dataLeft->GetDimX() != width ||
                           dataLeft->GetDimZ() != depth  || dataLeft->GetDimQ() != hypers))
   {
      ErrorMessage("matrix sizes don't agree in '%s' operation",GetOpString(operation));
      return(ERR);
   }

   switch(operation)
   { 
      case (EQ) :
      case ('=') :
      {
         long neq = 0;
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
                     neq += (FloatNotEqual(dst[q][z][y][x].r,src[q][z][y][x].r) || FloatNotEqual(dst[q][z][y][x].i,src[q][z][y][x].i));
	       dataLeft->MakeAndSetFloat(neq == 0);
	       return(OK);
	       break;
      }	
      case (NEQ) :
      {
         long neq = 0;
         for(q = 0; q < hypers; q++)
            for(z = 0; z < depth; z++)
               for(y = 0; y < height; y++)
                  for(x = 0; x < width; x++)
                     neq += (FloatNotEqual(dst[q][z][y][x].r,src[q][z][y][x].r) || FloatNotEqual(dst[q][z][y][x].i,src[q][z][y][x].i));
	      dataLeft->MakeAndSetFloat(neq != 0);
         return(OK);
         break;
      }	
	       
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     dst[q][z][y][x].r = dst[q][z][y][x].r + src[q][z][y][x].r;
                     dst[q][z][y][x].i = dst[q][z][y][x].i + src[q][z][y][x].i;
                  }
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     dst[q][z][y][x].r = dst[q][z][y][x].r - src[q][z][y][x].r;
                     dst[q][z][y][x].i = dst[q][z][y][x].i - src[q][z][y][x].i;
                  }
               }
            }
         }
         break;
      }
      case (MATMUL) : // .*
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     dst[q][z][y][x].r = dst[q][z][y][x].r*src[q][z][y][x].r - dst[q][z][y][x].i*src[q][z][y][x].r;
                     dst[q][z][y][x].i = dst[q][z][y][x].r*src[q][z][y][x].i + dst[q][z][y][x].i*src[q][z][y][x].r;
                  }
               }
            }
         }
         break;
      }
 
      case ('/') : 
      { 
         float denom;
         
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     denom = src[q][z][y][x].r*src[q][z][y][x].r + src[q][z][y][x].i*src[q][z][y][x].i;
                     if(denom == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     }                
                     temp.r = (dst[q][z][y][x].r*src[q][z][y][x].r + dst[q][z][y][x].i*src[q][z][y][x].i)/denom;
                     temp.i = (dst[q][z][y][x].i*src[q][z][y][x].r - dst[q][z][y][x].r*src[q][z][y][x].i)/denom;
                     dst[q][z][y][x] = temp;
                  }
               }
            }
         }
         break;
      }
      default: 
         ErrorMessage("'%s' is an undefined operation\n           between 4D complex matricies",GetOpString(operation));
         return(ERR); 
    }
    return(OK);
}



/******************************************************************************************
   Evaluate a REAL 4D MATRIX operating on a REAL NUMBER - result is a REAL 4D MATRIX
******************************************************************************************/

short Matrix4DFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k,q;
   
   float ****mat = dataLeft->GetMatrix4D();   
   long cols = dataLeft->GetDimX();
   long rows = dataLeft->GetDimY();
   long tiers = dataLeft->GetDimZ();
   long hypers = dataLeft->GetDimQ();
   float result = dataRight->GetReal();
   
   switch(operation)
   { 
      case (EQ):
      { 
          for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatEqual(mat[q][k][j][i],result);
          break;
      }
      case ('>') :
      { 
         for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatGreater(mat[q][k][j][i],result);
          break;
      }
      case ('<') :
      { 
         for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatLess(mat[q][k][j][i],result);
          break;
      }
      case (GEQ) :
      { 
         for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatGreaterEqual(mat[q][k][j][i],result);
          break;
      }
      case (LEQ) :
      { 
          for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatLessEqual(mat[q][k][j][i],result);
          break;
      }
      case ('+') :
       { 
          for(q = 0; q < hypers; q++)
            for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                      mat[q][k][j][i] += result;
          break;
       }
       case ('-') :
       { 
          for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                     mat[q][k][j][i] -= result;
          break;
       }
       case ('*') :
       { 
          for(q = 0; q < hypers; q++)
            for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                     mat[q][k][j][i] *= result;
          break;
       }
       case ('/') :
       { 
          if(result == 0)
          {
             ErrorMessage("divide by zero");
             return(ERR);
          }
          for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                     mat[q][k][j][i] /= result;
          break;
       }
       case ('^') :
       { 
          for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                     mat[q][k][j][i] = pow(mat[q][k][j][i],result);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a 4D matrix and real number",GetOpString(operation));
          return(ERR); 
    }
    return(0);
}



/******************************************************************************************
  Evaluate a REAL NUMBER operating on a REAL 4D MATRIX - result is a REAL 4D MATRIX
******************************************************************************************/

short FloatMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k,q;

   float result = dataLeft->GetReal();
   long cols = dataRight->GetDimX();
   long rows = dataRight->GetDimY();
   long tiers = dataRight->GetDimZ();
   long hypers = dataRight->GetDimQ();
   float ****mat = dataRight->GetMatrix4D();


   switch(operation)
   { 
      case (EQ):
      { 
          for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatEqual(mat[q][k][j][i],result);
          break;
      }
      case ('>') :
      { 
         for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatGreater(mat[q][k][j][i],result);
          break;
      }
      case ('<') :
      { 
         for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatLess(mat[q][k][j][i],result);
          break;
      }
      case (GEQ) :
      { 
         for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatGreaterEqual(mat[q][k][j][i],result);
          break;
      }
      case (LEQ) :
      { 
          for(q = 0; q < hypers; q++)
             for(k = 0; k < tiers; k++)
                for(j = 0; j < rows; j++)
                   for(i = 0; i < cols; i++)
                      mat[q][k][j][i] = FloatLessEqual(mat[q][k][j][i],result);
          break;
      }
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
            for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                     mat[q][k][j][i] = result + mat[q][k][j][i];
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
            for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                     mat[q][k][j][i] = result - mat[q][k][j][i];
         break;
      }
      case ('*') :
      { 
         for(q = 0; q < hypers; q++)
            for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                     mat[q][k][j][i] = result * mat[q][k][j][i];
         break;
      }
      case ('/') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {
               for(j = 0; j < rows; j++)
               {
                  for(i = 0; i < cols; i++)
                  {
                     if(mat[q][k][j][i] == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     }
                     mat[q][k][j][i] = result / mat[q][k][j][i];
                  }
               }
            }
         }
         break;
      }
      case ('^') :
      { 
         for(q = 0; q < hypers; q++)
            for(k = 0; k < tiers; k++)
               for(j = 0; j < rows; j++)
                  for(i = 0; i < cols; i++)
                     mat[q][k][j][i] = pow(result,mat[q][k][j][i]);
         break;
      }
      default: 
         ErrorMessage("'%s' is an undefined operation\n           between a real number and a 4D matrix",GetOpString(operation));
         return(ERR); 
    }

    dataLeft->AssignMatrix4D(mat,cols,rows,tiers,hypers);
    dataRight->SetNull(); // Data right moved to data left
    return(OK);
}


/******************************************************************************************
 Evaluate a COMPLEX NUMBER operating on a REAL 4D MATRIX - result is a COMPLEX 3D MATRIX
******************************************************************************************/

short ComplexMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z,q;
   long width = dataRight->GetDimX();
   long height = dataRight->GetDimY();
   long depth = dataRight->GetDimZ();
   long hypers = dataRight->GetDimQ();
   float ****mat = dataRight->GetMatrix4D();
   complex comp = dataLeft->GetComplex();
   complex ****cmat = MakeCMatrix4D(width,height,depth,hypers);

   if(!cmat) 
   {
      ErrorMessage("can't allocate memory for 'complex op 4D matrix' result");
      return(ERR);
   }
          
   switch(operation)
   { 
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     cmat[q][z][y][x].r = comp.r + mat[q][z][y][x];
                     cmat[q][z][y][x].i = comp.i;
                  }
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     cmat[q][z][y][x].r = comp.r - mat[q][z][y][x];
                     cmat[q][z][y][x].i = comp.i;
                  }
               }
            }
         }
         break;
      }
      case ('*') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     cmat[q][z][y][x].r = comp.r * mat[q][z][y][x];
                     cmat[q][z][y][x].i = comp.i * mat[q][z][y][x];
                  }
               }
            }
         }
         break;
       }
      case ('/') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     if(mat[q][z][y][x] == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     }
                     cmat[q][z][y][x].r = comp.r / mat[q][z][y][x];
                     cmat[q][z][y][x].i = comp.i / mat[q][z][y][x];
                  }
               }
            }
         }
         break;
      }
       default: 
          ErrorMessage("'%s' is an undefined operation between\n           real 4D matrix and complex number",GetOpString(operation));
          return(ERR); 
    }
    
    dataLeft->AssignCMatrix4D(cmat,width,height,depth,hypers);
    return(0);
}


/******************************************************************************************
 Evaluate a REAL 4D MATRIX operating on a COMPLEX NUMBER - result is a COMPLEX 3D MATRIX
******************************************************************************************/

short Matrix4DComplexEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z,q;
   long width = dataLeft->GetDimX();
   long height = dataLeft->GetDimY();
   long depth = dataLeft->GetDimZ();
   long hypers = dataLeft->GetDimQ();
   float ****mat = dataLeft->GetMatrix4D();
   complex comp = dataRight->GetComplex();
   
 // Make output matrix
   complex ****cmat = MakeCMatrix4D(width,height,depth,hypers);
   if(!cmat) 
   {
      ErrorMessage("can't allocate memory for '4D matrix op complex' result");
      return(ERR);
   }
          
   switch(operation)
   { 
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     cmat[q][z][y][x].r = mat[q][z][y][x] + comp.r;
                     cmat[q][z][y][x].i = comp.i;
                  }
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     cmat[q][z][y][x].r = mat[q][z][y][x] - comp.r;
                     cmat[q][z][y][x].i = -comp.i;
                  }
               }
            }
         }
         break;
      }
      case ('*') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     cmat[q][z][y][x].r = mat[q][z][y][x] * comp.r;
                     cmat[q][z][y][x].i = mat[q][z][y][x] * comp.i;
                  }
               }
            }
         }
         break;
      }
      case ('/') :
      { 
         float denom = comp.r*comp.r + comp.i*comp.i;
         if(denom == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         } 
         for(q = 0; q < hypers; q++)
         {
            for(z = 0; z < depth; z++)
            {
               for(y = 0; y < height; y++)
               {
                  for(x = 0; x < width; x++)
                  {
                     cmat[q][z][y][x].r = (mat[q][z][y][x] * comp.r)/denom;
                     cmat[q][z][y][x].i = -(mat[q][z][y][x] * comp.i)/denom;
                  }
               }
            }
         }
         break;
      }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a real 4D matrix and a complex number",GetOpString(operation));
          return(ERR); 
    }
    
    dataLeft->AssignCMatrix4D(cmat,width,height,depth,hypers);

    return(OK);
}




/******************************************************************************************
Evaluate a COMPLEX 4D MATRIX operating on a REAL NUMBER - result is a COMPLEX 4D MATRIX
******************************************************************************************/
//<-- check changes to hypers
short CMatrix4DFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k,q;
   long cols = dataLeft->GetDimX();
   long rows = dataLeft->GetDimY();
   long tiers = dataLeft->GetDimZ();
   long hypers = dataLeft->GetDimQ();
   complex ****cmat = dataLeft->GetCMatrix4D();
   float realNr = dataRight->GetReal();

   switch(operation)
   { 
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {
	            for(j = 0; j < rows; j++)
	            {
	               for(i = 0; i < cols; i++)
	               {
	                  cmat[q][k][j][i].r = cmat[q][k][j][i].r + realNr;
	               }
	            }
	         }
         }
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {       
	            for(long j = 0; j < rows; j++)
	            {
	               for(long i = 0; i < cols; i++)
	               {
	                  cmat[q][k][j][i].r = cmat[q][k][j][i].r - realNr;
	               }
	            }
	         }
         }
         break;
      }
      case ('*') :
      case (MATMUL) :       
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {        
	            for(long j = 0; j < rows; j++)
	            {
	               for(long i = 0; i < cols; i++)
	               {
	                  cmat[q][k][j][i].r = cmat[q][k][j][i].r * realNr;
	                  cmat[q][k][j][i].i = cmat[q][k][j][i].i * realNr;
	               }
	            }
	         }   
         }
         break;
      }
      case ('/') :
      { 
         if(realNr == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         }
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {         
	            for(j = 0; j < rows; j++)
	            {
	               for(i = 0; i < cols; i++)
	               {
	                  cmat[q][k][j][i].r = cmat[q][k][j][i].r / realNr;
	                  cmat[q][k][j][i].i = cmat[q][k][j][i].i / realNr;
	               }
	            }
	         }
         }
         break;
      }
      case ('^') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {     
	            for(j = 0; j < rows; j++)
	            {
	               for(i = 0; i < cols; i++)
	               {
	                  cmat[q][k][j][i].r = pow(cmat[q][k][j][i].r, realNr);
	                  cmat[q][k][j][i].i = pow(cmat[q][k][j][i].i, realNr);
	               }
	            }
	         }
         }
         break;
      }
      default: 
         ErrorMessage("'%s' is an undefined operation between\n           a complex 4D matrix and a real number",GetOpString(operation));
         return(ERR); 
    }
    return(OK);
}




/******************************************************************************************
   Evaluate a REAL NUMBER operating on a 4D COMPLEX MATRIX  - result is a COMPLEX 4D MATRIX
******************************************************************************************/

short FloatCMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k,q;
   long cols = dataRight->GetDimX();
   long rows = dataRight->GetDimY();
   long tiers = dataRight->GetDimZ();
   long hypers = dataRight->GetDimQ();
   complex ****cmat = dataRight->GetCMatrix4D();
   float realNr = dataLeft->GetReal();
   
   switch(operation)
   { 
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {        
	            for(j = 0; j < rows; j++)
	            {
	               for(i = 0; i < cols; i++)
	               {
	                  cmat[q][k][j][i].r = realNr + cmat[q][k][j][i].r;
	               }
	            } 
	         }
         }
			break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {         
               for(j = 0; j < rows; j++)
               {
	               for(i = 0; i < cols; i++)
	               {
	                  cmat[q][k][j][i].r = realNr - cmat[q][k][j][i].r;
	                  cmat[q][k][j][i].i = -cmat[q][k][j][i].i;
	               }
	            }
	         }
         }
         break;
      }
      case ('*') :
      case (MATMUL) :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {         
	            for(j = 0; j < rows; j++)
	            {
	               for(i = 0; i < cols; i++)
	               {
	                  cmat[q][k][j][i].r = realNr * cmat[q][k][j][i].r;
	                  cmat[q][k][j][i].i = realNr * cmat[q][k][j][i].i;
	               }
	            }
	         }
         }
         break;
      }
      case ('/') :
      { 
         float denom;
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {          
	            for(j = 0; j < rows; j++)
	            {
	               for(i = 0; i < cols; i++)
	               {
	                  denom = cmat[q][k][j][i].r*cmat[q][k][j][i].r + cmat[q][k][j][i].i*cmat[q][k][j][i].i;
                     if(denom == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     }
	                  cmat[q][k][j][i].r =  realNr*cmat[q][k][j][i].r/denom;
	                  cmat[q][k][j][i].i = -realNr*cmat[q][k][j][i].i/denom;
	               }
	            }
	         }
         }
         break;
      }
      default: 
         ErrorMessage("'%s' is an undefined operation\n           between a real number and a 3D complex matrix",GetOpString(operation));
         return(ERR); 
    }

// Make sure data on top of stack is the result
    dataLeft->AssignCMatrix4D(cmat,cols,rows,tiers,hypers);
    dataRight->SetNull(); // Data right moved to data left
    
    return(OK);
}



/******************************************************************************************
 Evaluate a COMPLEX NUMBER operating on a COMPLEX 4D MATRIX - result is a COMPLEX 4D MATRIX
******************************************************************************************/

short ComplexCMatrix4DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k,q;
   complex comp = dataLeft->GetComplex();
   long cols = dataRight->GetDimX();
   long rows = dataRight->GetDimY();
   long tiers = dataRight->GetDimZ();
   long hypers = dataRight->GetDimQ();
   complex ****cmat = dataRight->GetCMatrix4D();
   complex temp;
   float denom;

   switch(operation)
   { 
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {
               for(j = 0; j < rows; j++)
               {
                  for(i = 0; i < cols; i++)
                  {
                     cmat[q][k][j][i].r = comp.r + cmat[q][k][j][i].r;
                     cmat[q][k][j][i].i = comp.i + cmat[q][k][j][i].i;
                  }
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {       
               for(j = 0; j < rows; j++)
               {
                  for(i = 0; i < cols; i++)
                  {
                     cmat[q][k][j][i].r = comp.r - cmat[q][k][j][i].r;
                     cmat[q][k][j][i].i = comp.i - cmat[q][k][j][i].i;
                  }
               }
            }
         }
         break;
      }
      case ('*') :
      case (MATMUL) :       
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {        
               for(j = 0; j < rows; j++)
               {
                  for(i = 0; i < cols; i++)
                  {
                     temp.r = comp.r*cmat[q][k][j][i].r - comp.i*cmat[q][k][j][i].i;
                     temp.i = comp.r*cmat[q][k][j][i].i + comp.i*cmat[q][k][j][i].r;
                     cmat[q][k][j][i] = temp;
                  }
               }
            } 
         }
         break;
      }
      case ('/') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {         
               for(j = 0; j < rows; j++)
               {
                  for(i = 0; i < cols; i++)
                  {
                     denom = cmat[q][k][j][i].r*cmat[q][k][j][i].r + cmat[q][k][j][i].i*cmat[q][k][j][i].i;
                     if(denom == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     }            
                     temp.r = (comp.r*cmat[q][k][j][i].r + comp.i*cmat[q][k][j][i].i)/denom;
                     temp.i = (comp.i*cmat[q][k][j][i].r - comp.r*cmat[q][k][j][i].i)/denom;
                     cmat[q][k][j][i] = temp;
                  }
               }
            }
         }
         break;
      }
      default: 
      ErrorMessage("'%s' is an undefined operation between\n           a complex 4D matrix and a complex number",GetOpString(operation));
      return(ERR); 
   }
   dataLeft->AssignCMatrix4D(cmat,cols,rows,tiers,hypers);
   dataRight->SetNull(); // Data right moved to data left
   return(OK);
}


/******************************************************************************************
 Evaluate a COMPLEX 4D MATRIX operating on a COMPLEX NUMBER - result is a COMPLEX 4D MATRIX
******************************************************************************************/

short CMatrix4DComplexEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k,q;
   long cols = dataLeft->GetDimX();
   long rows = dataLeft->GetDimY();
   long tiers = dataLeft->GetDimZ();
   long hypers = dataLeft->GetDimQ();
   complex ****cmat = dataLeft->GetCMatrix4D();
   complex comp = dataRight->GetComplex();
   complex temp;
   float denom;

   switch(operation)
   { 
      case ('+') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {
               for(j = 0; j < rows; j++)
               {
                  for(i = 0; i < cols; i++)
                  {
                     cmat[q][k][j][i].r = cmat[q][k][j][i].r + comp.r;
                     cmat[q][k][j][i].i = cmat[q][k][j][i].i + comp.i;
                  }
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {       
               for(j = 0; j < rows; j++)
               {
                  for(i = 0; i < cols; i++)
                  {
                     cmat[q][k][j][i].r = cmat[q][k][j][i].r - comp.r;
                     cmat[q][k][j][i].i = cmat[q][k][j][i].i - comp.i;
                  }
               }
            }
         }
         break;
      }
      case ('*') :
      case (MATMUL) :       
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {        
               for(j = 0; j < rows; j++)
               {
                  for(i = 0; i < cols; i++)
                  {
                     temp.r = cmat[q][k][j][i].r*comp.r - cmat[q][k][j][i].i*comp.i;
                     temp.i = cmat[q][k][j][i].i*comp.r + cmat[q][k][j][i].r*comp.i;
                     cmat[q][k][j][i] = temp;
                  }
               }
            }
         }
         break;
      }
      case ('/') :
      { 
         for(q = 0; q < hypers; q++)
         {
            for(k = 0; k < tiers; k++)
            {         
               for(long j = 0; j < rows; j++)
               {
                  for(long i = 0; i < cols; i++)
                  {
                     denom = comp.r*comp.r + comp.i*comp.i;
                     if(denom == 0)
                     {
                        ErrorMessage("divide by zero");
                        return(ERR);
                     }            
                     temp.r = (cmat[q][k][j][i].r*comp.r + cmat[q][k][j][i].i*comp.i)/denom;
                     temp.i = (cmat[q][k][j][i].i*comp.r - cmat[q][k][j][i].r*comp.i)/denom;
                     cmat[q][k][j][i] = temp;
                  }
               }
            }
         }
         break;
      }
      default: 
         ErrorMessage("'%s' is an undefined operation between\n           a complex 4D matrix and a complex number",GetOpString(operation));
         return(ERR); 
   }
   return(OK);
}

/*********************************************************************************
Assign values to a point, a row or column, a plane or cube in a 4D real matrix
*********************************************************************************/

short Process4DMatrixAssignment(Variable *dstVar, Variable *srcVar,
                      long x, long y, long z, long q,
                      long *xa, long *ya, long *za, long *qa)
{
   long i,j,k,l;
	float ****dstMat = dstVar->GetMatrix4D();

	long srcXSize     = srcVar->GetDimX();
	long srcYSize     = srcVar->GetDimY();
	long srcZSize     = srcVar->GetDimZ();


	
	switch(srcVar->GetType()) // Check out value type
	{
	   case(FLOAT32):
	   case(INTEGER):
	   {
         float fnum  = srcVar->GetReal();

         // Assign a complex number to 1 row, col, tier or hyper
	      if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,3,5,4] = 5
	      {
	         for(i = 0; i < -x; i++)
	            dstMat[q][z][y][xa[i]] = fnum; 
	      }
	      else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,5,4] = 5
	      {
	         for(j = 0; j < -y; j++)
	            dstMat[q][z][ya[j]][x] = fnum; 

	      }
	      else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,3,~,4] = 5
	      {
	         for(k = 0; k < -z; k++)
	            dstMat[q][za[k]][y][x] = fnum; 

	      }
	      else if(x >= 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[1,3,5,~] = 5
	      {
	         for(k = 0; k < -q; k++)
	            dstMat[qa[k]][z][y][x] = fnum; 

	      } 
      // Assign complex number to 1 plane
	      else if(x >= 0 && y < 0 && z < 0 && q >= 0) // e.g. m[1,~,~,4] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
	            for(j = 0; j < -y; j++)
	                dstMat[q][za[k]][ya[j]][x] = fnum; 
	      }   
	      else if(x < 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[~,3,~,4] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
	            for(i = 0; i < -x; i++)
	                dstMat[q][za[k]][y][xa[i]] = fnum; 
	      } 
	      else if(x < 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[~,~,5,4] = 5
	      {                                     
	         for(j = 0; j < -y; j++)
	            for(i = 0; i < -x; i++)
	                dstMat[q][z][ya[j]][xa[i]] = fnum; 

	      } 
	      else if(x < 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[~,3,5,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
	            for(i = 0; i < -x; i++)
	                dstMat[qa[j]][z][y][xa[i]] = fnum; 

	      } 
	      else if(x >= 0 && y < 0 && z >= 0 && q < 0) // e.g. m[1,~,5,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
	            for(i = 0; i < -y; i++)
	                dstMat[qa[j]][z][ya[i]][x] = fnum; 

	      }
	      else if(x >= 0 && y < 0 && z >= 0 && q < 0) // e.g. m[1,3,~,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
	            for(i = 0; i < -z; i++)
	                dstMat[qa[j]][za[i]][y][x] = fnum; 
	      }
      // Assign complex number to 1 cube
	      else if(x >= 0 && y < 0 && z < 0 && q < 0) // e.g. m[1,~,~,~] = 5
	      {
	         for(k = 0; k < -q; k++)
	            for(j = 0; j < -z; j++)
	               for(i = 0; i < -y; i++)
	                   dstMat[qa[k]][za[j]][ya[i]][x] = fnum; 

	      }
	      else if(x < 0 && y < 0 && z < 0 && q >= 0) // e.g. m[~,3,~,~] = 5
	      {
	         for(k = 0; k < -q; k++)
	            for(j = 0; j < -z; j++)
	               for(i = 0; i < -x; i++)
	                   dstMat[qa[k]][za[j]][y][xa[i]] = fnum; 

	      } 
	      else if(x < 0 && y < 0 && z >= 0 && q < 0) // e.g. m[~,~,5,~] = 5
	      {
	         for(k = 0; k < -q; k++)
	            for(j = 0; j < -y; j++)
	               for(i = 0; i < -x; i++)
	                   dstMat[qa[k]][z][ya[j]][xa[i]] = fnum; 

	      } 
	      else if(x < 0 && y < 0 && z < 0 && q >= 0) // e.g. m[~,~,~,4] = 5
	      {
	         for(k = 0; k < -z; k++)
	            for(j = 0; j < -y; j++)
	               for(i = 0; i < -x; i++)
	                   dstMat[q][za[k]][ya[j]][xa[i]] = fnum; 

	      } 
	      else if(x < 0 && y < 0 && z < 0 && q < 0) // e.g. m[~,~,~,~] = 5
	      {
	         for(l = 0; l < -q; l++)
	            for(k = 0; k < -z; k++)
	               for(j = 0; j < -y; j++)
	                  for(i = 0; i < -x; i++)
	                     dstMat[qa[l]][za[k]][ya[j]][xa[i]] = fnum; 

	      } 
	      else // e.g. m[2,3,5,4] = 5
	      {
	         dstMat[q][z][y][x] = fnum;
	      }
	      break;
	   }        
// Assign a 2D real matrix to part of a 4D complex matrix
	   case(MATRIX2D):
	   {
         float **srcMat  = srcVar->GetMatrix2D();
	   
	      if(srcXSize == 1) // Column vector
	      {
				if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,1,0,4] = [1,2]'
				{
               if(srcYSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[q][z][y][xa[i]] = srcMat[i][0];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,0,4] = [1,2]'
				{
               if(srcYSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[q][z][ya[j]][x] = srcMat[j][0];
				}                 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,~,4] = [1,2]'
				{
               if(srcYSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[q][za[k]][y][x] = srcMat[k][0];
				} 
				else if(x >= 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[1,1,0,~] = [1,2]'
				{
               if(srcYSize != -q)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -q; k++)
				      dstMat[qa[k]][z][y][x] = srcMat[k][0];
				}
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else if(srcYSize == 1) // Row vector
	      {
				if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,1,0,4] = [1,2]
				{
               if(srcXSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[q][z][y][xa[i]] = srcMat[0][i];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,0,4] = [1,2]
				{
               if(srcXSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[q][z][ya[j]][x] = srcMat[0][j];
				}                 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,~,4] = [1,2]
				{
               if(srcXSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[q][za[k]][y][x] = srcMat[0][k];
				} 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,0,~] = [1,2]
				{
               if(srcXSize != -q)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -q; k++)
				      dstMat[qa[k]][z][y][x] = srcMat[0][k];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else // Plane matrix
	      {
				if(x < 0 && y < 0 && z >= 0 && q >= 0)
				{
               if(srcXSize != -x || srcYSize != -y)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(j = 0; j < -y; j++)     // e.g. m[~,~,3,4] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][z][ya[j]][xa[i]] = srcMat[j][i];
				} 
				else if(x >= 0 && y < 0 && z < 0 && q >= 0)
				{
               if(srcXSize != -y || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)     // e.g. m[1,~,~,4] = [1,2;3,4]
			 	      for(j = 0; j < -y; j++)
				         dstMat[q][za[k]][ya[j]][x] = srcMat[k][j];
				}                  
				else if(x < 0 && y >= 0 && z < 0 && q >= 0)
				{
               if(srcXSize != -x || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)    // e.g. m[~,2,~,4] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][za[k]][y][xa[i]] = srcMat[k][i];
				}
				else if(x < 0 && y >= 0 && z >= 0 && q < 0)
				{
               if(srcXSize != -x || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[~,2,3,~] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][z][y][xa[i]] = srcMat[k][i];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q < 0)
				{
               if(srcXSize != -y || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[1,~,3,~] = [1,2;3,4]
			 	      for(i = 0; i < -y; i++)
				         dstMat[qa[k]][z][ya[i]][x] = srcMat[k][i];
				}
				else if(x >= 0 && y >= 0 && z < 0 && q < 0)
				{
               if(srcXSize != -z || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[1,2,~,~] = [1,2;3,4]
			 	      for(i = 0; i < -z; i++)
				         dstMat[qa[k]][za[i]][y][x] = srcMat[k][i];
				}
	         else
	         {
	            ErrorMessage("source matrix wrong size");
	            return(ERR);
	         } 
	      }	                                       
	      break;
	   }   

// Assign a 3D real matrix to part of a 4D complex matrix
	   case(MATRIX3D):
	   {
         float ***srcMat  = srcVar->GetMatrix3D();
	   
			if(x < 0 && y < 0 && z < 0 && q >= 0)   // e.g. m[~,~,~,4] = n3D
			{
            if(srcXSize != -x || srcYSize != -y|| srcZSize != -z)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -z; k++) 
			 	   for(j = 0; j < -y; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][za[k]][ya[j]][xa[i]] = srcMat[k][j][i];
			}    
			else if(x < 0 && y < 0 && z >= 0 && q < 0)   // e.g. m[~,~,3,~] = n3D
			{
            if(srcXSize != -x || srcYSize != -y || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -y; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][z][ya[j]][xa[i]] = srcMat[k][j][i];
			} 
			else if(x >= 0 && y < 0 && z < 0 && q < 0)   // e.g. m[1,~,~,~] = n3D
			{
            if(srcXSize != -y || srcYSize != -z || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -z; j++) 
			 	      for(i = 0; i < -y; i++)
				         dstMat[qa[k]][za[j]][ya[i]][x] = srcMat[k][j][i];
			} 
			else if(x < 0 && y >= 0 && z < 0 && q < 0)   // e.g. m[1,~,~,~] = n3D
			{
            if(srcXSize != -x || srcYSize != -z || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -z; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][za[j]][y][xa[i]] = srcMat[k][j][i];
			} 
	      break;
	   }  
	   default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}
	return(OK);	
}

/*********************************************************************************
Assign values to a point, a row or column, a plane or cube in a 4D complex matrix
*********************************************************************************/

short Process4DCMatrixAssignment(Variable *dstVar, Variable *srcVar,
                             long x, long y, long z, long q,
                             long *xa, long *ya, long *za, long *qa)
{
   long i,j,k,l;
	complex ****dstMat = dstVar->GetCMatrix4D();

	long srcXSize     = srcVar->GetDimX();
	long srcYSize     = srcVar->GetDimY();
	long srcZSize     = srcVar->GetDimZ();
	
	switch(srcVar->GetType()) // Check out value type
	{
	   case(FLOAT32):
	   case(INTEGER):
	   {
         float fnum  = srcVar->GetReal();

      // Assign a real number to 1 row, col, tier or hyper
	      if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,3,5,4] = 5
	      {
	         for(i = 0; i < -x; i++)
            {
	            dstMat[q][z][y][xa[i]].r = fnum; 
	            dstMat[q][z][y][xa[i]].i = 0; 
            }
	      }
	      else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,5,4] = 5
	      {
	         for(j = 0; j < -y; j++)
            {
	            dstMat[q][z][ya[j]][x].r = fnum; 
	            dstMat[q][z][ya[j]][x].i = 0; 
            }
	      }
	      else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,3,~,4] = 5
	      {
	         for(k = 0; k < -z; k++)
            {
	            dstMat[q][za[k]][y][x].r = fnum; 
	            dstMat[q][za[k]][y][x].i = 0; 
            }
	      }
	      else if(x >= 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[1,3,5,~] = 5
	      {
	         for(k = 0; k < -q; k++)
            {
	            dstMat[qa[k]][z][y][x].r = fnum; 
	            dstMat[qa[k]][z][y][x].i = 0; 
            }
	      } 
      // Assign real number to 1 plane
	      else if(x >= 0 && y < 0 && z < 0 && q >= 0) // e.g. m[1,~,~,4] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
            {
	            for(j = 0; j < -y; j++)
               {
	                dstMat[q][za[k]][ya[j]][x].r = fnum; 
	                dstMat[q][za[k]][ya[j]][x].i = 0; 
               }
            }
	      }   
	      else if(x < 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[~,3,~,4] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
            {
	            for(i = 0; i < -x; i++)
               {
	                dstMat[q][za[k]][y][xa[i]].r = fnum; 
	                dstMat[q][za[k]][y][xa[i]].i = 0; 
               }
            }
	      } 
	      else if(x < 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[~,~,5,4] = 5
	      {                                     
	         for(j = 0; j < -y; j++)
            {
	            for(i = 0; i < -x; i++)
               {
	                dstMat[q][z][ya[j]][xa[i]].r = fnum; 
	                dstMat[q][z][ya[j]][xa[i]].i = 0; 
               }
            }
	      } 
	      else if(x < 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[~,3,5,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
            {
	            for(i = 0; i < -x; i++)
               {
	                dstMat[qa[j]][z][y][xa[i]].r = fnum; 
	                dstMat[qa[j]][z][y][xa[i]].i = 0; 
               }
            }
	      } 
	      else if(x >= 0 && y < 0 && z >= 0 && q < 0) // e.g. m[1,~,5,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
            {
	            for(i = 0; i < -y; i++)
               {
	                dstMat[qa[j]][z][ya[i]][x].r = fnum; 
	                dstMat[qa[j]][z][ya[i]][x].i = 0; 
               }
            }
	      }
	      else if(x >= 0 && y < 0 && z >= 0 && q < 0) // e.g. m[1,3,~,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
            {
	            for(i = 0; i < -z; i++)
               {
	                dstMat[qa[j]][za[i]][y][x].r = fnum; 
	                dstMat[qa[j]][za[i]][y][x].i = 0; 
               }
            }
	      }
      // Assign real number to 1 cube
	      else if(x >= 0 && y < 0 && z < 0 && q < 0) // e.g. m[1,~,~,~] = 5
	      {
	         for(k = 0; k < -q; k++)
            {
	            for(j = 0; j < -z; j++)
               {
	               for(i = 0; i < -y; i++)
                  {
	                   dstMat[qa[k]][za[j]][ya[i]][x].r = fnum; 
	                   dstMat[qa[k]][za[j]][ya[i]][x].i = 0; 
                  }
               }
            }
	      }
	      else if(x < 0 && y < 0 && z < 0 && q >= 0) // e.g. m[~,3,~,~] = 5
	      {
	         for(k = 0; k < -q; k++)
            {
	            for(j = 0; j < -z; j++)
               {
	               for(i = 0; i < -x; i++)
                  {
	                   dstMat[qa[k]][za[j]][y][xa[i]].r = fnum; 
	                   dstMat[qa[k]][za[j]][y][xa[i]].i = 0; 
                  }
               }
            }
	      } 
	      else if(x < 0 && y < 0 && z >= 0 && q < 0) // e.g. m[~,~,5,~] = 5
	      {
	         for(k = 0; k < -q; k++)
            {
	            for(j = 0; j < -y; j++)
               {
	               for(i = 0; i < -x; i++)
                  {
	                   dstMat[qa[k]][z][ya[j]][xa[i]].r = fnum; 
	                   dstMat[qa[k]][z][ya[j]][xa[i]].i = 0; 
                  }
               }
            }
	      } 
	      else if(x < 0 && y < 0 && z < 0 && q >= 0) // e.g. m[~,~,~,4] = 5
	      {
	         for(k = 0; k < -z; k++)
            {
	            for(j = 0; j < -y; j++)
               {
	               for(i = 0; i < -x; i++)
                  {
	                   dstMat[q][za[k]][ya[j]][xa[i]].r = fnum; 
	                   dstMat[q][za[k]][ya[j]][xa[i]].i = 0; 
                  }
               }
            }
	      } 
	      else if(x < 0 && y < 0 && z < 0 && q < 0) // e.g. m[~,~,~,~] = 5
	      {
	         for(l = 0; l < -q; l++)
            {
	            for(k = 0; k < -z; k++)
               {
	               for(j = 0; j < -y; j++)
                  {
	                  for(i = 0; i < -x; i++)
                     {
	                     dstMat[qa[l]][za[k]][ya[j]][xa[i]].r = fnum; 
	                     dstMat[qa[l]][za[k]][ya[j]][xa[i]].i = 0; 
                     }
                  }
               }
            }
	      } 
	      else // e.g. m[2,3,5,4] = 5
	      {
	         dstMat[q][z][y][x].r = fnum;
	         dstMat[q][z][y][x].i = 0;
	      }
	      break;
	   }


	   case(COMPLEX):
	   {
         complex cnum  = srcVar->GetComplex();

      // Assign a complex number to 1 row, col, tier or hyper
	      if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,3,5,4] = 5
	      {
	         for(i = 0; i < -x; i++)
	            dstMat[q][z][y][xa[i]] = cnum; 
	      }
	      else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,5,4] = 5
	      {
	         for(j = 0; j < -y; j++)
	            dstMat[q][z][ya[j]][x] = cnum; 

	      }
	      else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,3,~,4] = 5
	      {
	         for(k = 0; k < -z; k++)
	            dstMat[q][za[k]][y][x] = cnum; 

	      }
	      else if(x >= 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[1,3,5,~] = 5
	      {
	         for(k = 0; k < -q; k++)
	            dstMat[qa[k]][z][y][x] = cnum; 

	      } 
      // Assign complex number to 1 plane
	      else if(x >= 0 && y < 0 && z < 0 && q >= 0) // e.g. m[1,~,~,4] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
	            for(j = 0; j < -y; j++)
	                dstMat[q][za[k]][ya[j]][x] = cnum; 
	      }   
	      else if(x < 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[~,3,~,4] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
	            for(i = 0; i < -x; i++)
	                dstMat[q][za[k]][y][xa[i]] = cnum; 
	      } 
	      else if(x < 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[~,~,5,4] = 5
	      {                                     
	         for(j = 0; j < -y; j++)
	            for(i = 0; i < -x; i++)
	                dstMat[q][z][ya[j]][xa[i]] = cnum; 

	      } 
	      else if(x < 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[~,3,5,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
	            for(i = 0; i < -x; i++)
	                dstMat[qa[j]][z][y][xa[i]] = cnum; 

	      } 
	      else if(x >= 0 && y < 0 && z >= 0 && q < 0) // e.g. m[1,~,5,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
	            for(i = 0; i < -y; i++)
	                dstMat[qa[j]][z][ya[i]][x] = cnum; 

	      }
	      else if(x >= 0 && y < 0 && z >= 0 && q < 0) // e.g. m[1,3,~,~] = 5
	      {                                     
	         for(j = 0; j < -q; j++)
	            for(i = 0; i < -z; i++)
	                dstMat[qa[j]][za[i]][y][x] = cnum; 
	      }
      // Assign complex number to 1 cube
	      else if(x >= 0 && y < 0 && z < 0 && q < 0) // e.g. m[1,~,~,~] = 5
	      {
	         for(k = 0; k < -q; k++)
	            for(j = 0; j < -z; j++)
	               for(i = 0; i < -y; i++)
	                   dstMat[qa[k]][za[j]][ya[i]][x] = cnum; 

	      }
	      else if(x < 0 && y < 0 && z < 0 && q >= 0) // e.g. m[~,3,~,~] = 5
	      {
	         for(k = 0; k < -q; k++)
	            for(j = 0; j < -z; j++)
	               for(i = 0; i < -x; i++)
	                   dstMat[qa[k]][za[j]][y][xa[i]] = cnum; 

	      } 
	      else if(x < 0 && y < 0 && z >= 0 && q < 0) // e.g. m[~,~,5,~] = 5
	      {
	         for(k = 0; k < -q; k++)
	            for(j = 0; j < -y; j++)
	               for(i = 0; i < -x; i++)
	                   dstMat[qa[k]][z][ya[j]][xa[i]] = cnum; 

	      } 
	      else if(x < 0 && y < 0 && z < 0 && q >= 0) // e.g. m[~,~,~,4] = 5
	      {
	         for(k = 0; k < -z; k++)
	            for(j = 0; j < -y; j++)
	               for(i = 0; i < -x; i++)
	                   dstMat[q][za[k]][ya[j]][xa[i]] = cnum; 

	      } 
	      else if(x < 0 && y < 0 && z < 0 && q < 0) // e.g. m[~,~,~,~] = 5
	      {
	         for(l = 0; l < -q; l++)
	            for(k = 0; k < -z; k++)
	               for(j = 0; j < -y; j++)
	                  for(i = 0; i < -x; i++)
	                     dstMat[qa[l]][za[k]][ya[j]][xa[i]] = cnum; 

	      }
	      else // e.g. m[2,3,5,4] = 5
	      {
	         dstMat[q][z][y][x] = cnum;
	      }
	      break;
	   }        
// Assign a 2D real matrix to part of a 4D complex matrix
	   case(MATRIX2D):
	   {
         float **srcMat  = srcVar->GetMatrix2D();
	   
	      if(srcXSize == 1) // Column vector
	      {
				if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,1,0,4] = [1,2]'
				{
               if(srcYSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[q][z][y][xa[i]].r = srcMat[i][0];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,0,4] = [1,2]'
				{
               if(srcYSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[q][z][ya[j]][x].r = srcMat[j][0];
				}                 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,~,4] = [1,2]'
				{
               if(srcYSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[q][za[k]][y][x].r = srcMat[k][0];
				} 
				else if(x >= 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[1,1,0,~] = [1,2]'
				{
               if(srcYSize != -q)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -q; k++)
				      dstMat[qa[k]][z][y][x].r = srcMat[k][0];
				}
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else if(srcYSize == 1) // Row vector
	      {
				if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,1,0,4] = [1,2]
				{
               if(srcXSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[q][z][y][xa[i]].r = srcMat[0][i];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,0,4] = [1,2]
				{
               if(srcXSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[q][z][ya[j]][x].r = srcMat[0][j];
				}                 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,~,4] = [1,2]
				{
               if(srcXSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[q][za[k]][y][x].r = srcMat[0][k];
				} 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,0,~] = [1,2]
				{
               if(srcXSize != -q)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -q; k++)
				      dstMat[qa[k]][z][y][x].r = srcMat[0][k];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else // Plane matrix
	      {
				if(x < 0 && y < 0 && z >= 0 && q >= 0)
				{
               if(srcXSize != -x || srcYSize != -y)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(j = 0; j < -y; j++)     // e.g. m[~,~,3,4] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][z][ya[j]][xa[i]].r = srcMat[j][i];
				} 
				else if(x >= 0 && y < 0 && z < 0 && q >= 0)
				{
               if(srcXSize != -y || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)     // e.g. m[1,~,~,4] = [1,2;3,4]
			 	      for(j = 0; j < -y; j++)
				         dstMat[q][za[k]][ya[j]][x].r = srcMat[k][j];
				}                  
				else if(x < 0 && y >= 0 && z < 0 && q >= 0)
				{
               if(srcXSize != -x || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)    // e.g. m[~,2,~,4] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][za[k]][y][xa[i]].r = srcMat[k][i];
				}
				else if(x < 0 && y >= 0 && z >= 0 && q < 0)
				{
               if(srcXSize != -x || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[~,2,3,~] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][z][y][xa[i]].r = srcMat[k][i];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q < 0)
				{
               if(srcXSize != -y || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[1,~,3,~] = [1,2;3,4]
			 	      for(i = 0; i < -y; i++)
				         dstMat[qa[k]][z][ya[i]][x].r = srcMat[k][i];
				}
				else if(x >= 0 && y >= 0 && z < 0 && q < 0)
				{
               if(srcXSize != -z || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[1,2,~,~] = [1,2;3,4]
			 	      for(i = 0; i < -z; i++)
				         dstMat[qa[k]][za[i]][y][x].r = srcMat[k][i];
				}
	         else
	         {
	            ErrorMessage("source matrix wrong size");
	            return(ERR);
	         } 
	      }	                                       
	      break;
	   }   
	   case(CMATRIX2D):
	   {
         complex **srcMat  = srcVar->GetCMatrix2D();
	   
	      if(srcXSize == 1) // Column vector
	      {
				if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,1,0,4] = [1,2]'
				{
               if(srcYSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[q][z][y][xa[i]] = srcMat[i][0];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,0,4] = [1,2]'
				{
               if(srcYSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[q][z][ya[j]][x] = srcMat[j][0];
				}                 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,~,4] = [1,2]'
				{
               if(srcYSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[q][za[k]][y][x] = srcMat[k][0];
				} 
				else if(x >= 0 && y >= 0 && z >= 0 && q < 0) // e.g. m[1,1,0,~] = [1,2]'
				{
               if(srcYSize != -q)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -q; k++)
				      dstMat[qa[k]][z][y][x] = srcMat[k][0];
				}
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else if(srcYSize == 1) // Row vector
	      {
				if(x < 0 && y >= 0 && z >= 0 && q >= 0) // e.g. m[~,1,0,4] = [1,2]
				{
               if(srcXSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[q][z][y][xa[i]] = srcMat[0][i];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // e.g. m[1,~,0,4] = [1,2]
				{
               if(srcXSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[q][z][ya[j]][x] = srcMat[0][j];
				}                 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,~,4] = [1,2]
				{
               if(srcXSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[q][za[k]][y][x] = srcMat[0][k];
				} 
				else if(x >= 0 && y >= 0 && z < 0 && q >= 0) // e.g. m[1,1,0,~] = [1,2]
				{
               if(srcXSize != -q)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -q; k++)
				      dstMat[qa[k]][z][y][x] = srcMat[0][k];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else // Plane matrix
	      {
				if(x < 0 && y < 0 && z >= 0 && q >= 0)
				{
               if(srcXSize != -x || srcYSize != -y)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(j = 0; j < -y; j++)     // e.g. m[~,~,3,4] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][z][ya[j]][xa[i]] = srcMat[j][i];
				} 
				else if(x >= 0 && y < 0 && z < 0 && q >= 0)
				{
               if(srcXSize != -y || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)     // e.g. m[1,~,~,4] = [1,2;3,4]
			 	      for(j = 0; j < -y; j++)
				         dstMat[q][za[k]][ya[j]][x] = srcMat[k][j];
				}                  
				else if(x < 0 && y >= 0 && z < 0 && q >= 0)
				{
               if(srcXSize != -x || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)    // e.g. m[~,2,~,4] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][za[k]][y][xa[i]] = srcMat[k][i];
				}
				else if(x < 0 && y >= 0 && z >= 0 && q < 0)
				{
               if(srcXSize != -x || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[~,2,3,~] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][z][y][xa[i]] = srcMat[k][i];
				}
				else if(x >= 0 && y < 0 && z >= 0 && q < 0)
				{
               if(srcXSize != -y || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[1,~,3,~] = [1,2;3,4]
			 	      for(i = 0; i < -y; i++)
				         dstMat[qa[k]][z][ya[i]][x] = srcMat[k][i];
				}
				else if(x >= 0 && y >= 0 && z < 0 && q < 0)
				{
               if(srcXSize != -z || srcYSize != -q)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -q; k++)    // e.g. m[1,2,~,~] = [1,2;3,4]
			 	      for(i = 0; i < -z; i++)
				         dstMat[qa[k]][za[i]][y][x] = srcMat[k][i];
				}
	         else
	         {
	            ErrorMessage("source matrix wrong size");
	            return(ERR);
	         } 
	      }	                                       
	      break;
	   }  
// Assign a 3D real matrix to part of a 4D complex matrix
	   case(MATRIX3D):
	   {
         float ***srcMat  = srcVar->GetMatrix3D();
	   
			if(x < 0 && y < 0 && z < 0 && q >= 0)   // e.g. m[~,~,~,4] = n3D
			{
            if(srcXSize != -x || srcYSize != -y|| srcZSize != -z)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -z; k++) 
			 	   for(j = 0; j < -y; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][za[k]][ya[j]][xa[i]].r = srcMat[k][j][i];
			}    
			else if(x < 0 && y < 0 && z >= 0 && q < 0)   // e.g. m[~,~,3,~] = n3D
			{
            if(srcXSize != -x || srcYSize != -y || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -y; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][z][ya[j]][xa[i]].r = srcMat[k][j][i];
			} 
			else if(x >= 0 && y < 0 && z < 0 && q < 0)   // e.g. m[1,~,~,~] = n3D
			{
            if(srcXSize != -y || srcYSize != -z || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -z; j++) 
			 	      for(i = 0; i < -y; i++)
				         dstMat[qa[k]][za[j]][ya[i]][x].r = srcMat[k][j][i];
			} 
			else if(x < 0 && y >= 0 && z < 0 && q < 0)   // e.g. m[1,~,~,~] = n3D
			{
            if(srcXSize != -x || srcYSize != -z || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -z; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][za[j]][y][xa[i]].r = srcMat[k][j][i];
			} 
	      break;
	   }  
// Assign a 3D complex matrix to part of a 4D complex matrix
	   case(CMATRIX3D):
	   {
         complex ***srcMat  = srcVar->GetCMatrix3D();
	   
			if(x < 0 && y < 0 && z < 0 && q >= 0)   // e.g. m[~,~,~,4] = n3D
			{
            if(srcXSize != -x || srcYSize != -y|| srcZSize != -z)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -z; k++) 
			 	   for(j = 0; j < -y; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[q][za[k]][ya[j]][xa[i]] = srcMat[k][j][i];
			}    
			else if(x < 0 && y < 0 && z >= 0 && q < 0)   // e.g. m[~,~,3,~] = n3D
			{
            if(srcXSize != -x || srcYSize != -y || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -y; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][z][ya[j]][xa[i]] = srcMat[k][j][i];
			} 
			else if(x >= 0 && y < 0 && z < 0 && q < 0)   // e.g. m[1,~,~,~] = n3D
			{
            if(srcXSize != -y || srcYSize != -z || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -z; j++) 
			 	      for(i = 0; i < -y; i++)
				         dstMat[qa[k]][za[j]][ya[i]][x] = srcMat[k][j][i];
			} 
			else if(x < 0 && y >= 0 && z < 0 && q < 0)   // e.g. m[1,~,~,~] = n3D
			{
            if(srcXSize != -x || srcYSize != -z || srcZSize != -q)
            {
                  ErrorMessage("source matrix has a different size from destination index range");
                  return(ERR);
            }
			 	for(k = 0; k < -q; k++) 
			 	   for(j = 0; j < -z; j++) 
			 	      for(i = 0; i < -x; i++)
				         dstMat[qa[k]][za[j]][y][xa[i]] = srcMat[k][j][i];
			} 
	      break;
      }
	   default:
	   {
	      ErrorMessage("invalid data type");
	      return(ERR);
	   }
	}
	return(OK);	
}