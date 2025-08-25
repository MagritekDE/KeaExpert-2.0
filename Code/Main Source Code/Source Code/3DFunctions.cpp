#include "stdafx.h"
#include "3DFunctions.h"
#include <math.h>
#include "allocate.h"
#include "evaluate.h"
#include "globals.h"
#include "mymath.h"
#include "operators.h"
#include "variablesClass.h"
#include "memoryLeak.h"

/******************************************************************************************
  Evaluate a REAL NUMBER operating on a REAL 3D MATRIX - result is a REAL 3D MATRIX
******************************************************************************************/

short FloatMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k;

   float result = dataLeft->GetReal();
   long cols = dataRight->GetDimX();
   long rows = dataRight->GetDimY();
   long tiers = dataRight->GetDimZ();
   float ***mat = dataRight->GetMatrix3D();


   switch(operation)
   { 
       case (EQ):
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatEqual(mat[k][j][i],result);
          break;
       }
       case ('>') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatGreater(mat[k][j][i],result);
          break;
       }
       case ('<') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatLess(mat[k][j][i],result);
          break;
       }
       case (GEQ) :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatGreaterEqual(mat[k][j][i],result);
          break;
       }
       case (LEQ) :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatLessEqual(mat[k][j][i],result);
          break;
       }
       case ('+') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = result + mat[k][j][i];
          break;
       }
       case ('-') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                  mat[k][j][i] = result - mat[k][j][i];
          break;
       }
       case ('*') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = result * mat[k][j][i];
          break;
       }
       case ('/') :
       { 
          for(k = 0; k < tiers; k++)
          {
             for(j = 0; j < rows; j++)
             {
                for(i = 0; i < cols; i++)
                {
                   if(mat[k][j][i] == 0)
                   {
                      ErrorMessage("divide by zero");
                      return(ERR);
                   }
                   mat[k][j][i] = result / mat[k][j][i];
                }
             }
          }
          break;
       }
       case ('^') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = pow(result,mat[k][j][i]);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a real number and matrix",GetOpString(operation));
          return(ERR); 
    }

    dataLeft->AssignMatrix3D(mat,cols,rows,tiers);
    dataRight->SetNull(); // Data right moved to data left
    return(OK);
}


/******************************************************************************************
 Evaluate a COMPLEX NUMBER operating on a REAL 3D MATRIX - result is a COMPLEX 3D MATRIX
******************************************************************************************/

short CompMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z;
   long width = dataRight->GetDimX();
   long height = dataRight->GetDimY();
   long depth = dataRight->GetDimZ();
   float ***mat = dataRight->GetMatrix3D();
   complex comp = dataLeft->GetComplex();
   complex ***cmat = MakeCMatrix3D(width,height,depth);

   if(!cmat) 
   {
      ErrorMessage("can't allocate memory for 'complex op 3D matrix' result");
      return(ERR);
   }
          
   switch(operation)
   { 
       case ('+') :
       { 
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = comp.r + mat[z][y][x];
                  cmat[z][y][x].i = comp.i;
               }
            }
          }
          break;
       }
       case ('-') :
       { 
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = comp.r - mat[z][y][x];
                  cmat[z][y][x].i = comp.i;
               }
            }
          }
          break;
       }
       case ('*') :
       { 
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = comp.r * mat[z][y][x];
                  cmat[z][y][x].i = comp.i * mat[z][y][x];
               }
            }
          }
          break;
       }
       case ('/') :
       { 
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  if(mat[z][y][x] == 0)
                  {
                     ErrorMessage("divide by zero");
                     return(ERR);
                  }
                  cmat[z][y][x].r = comp.r / mat[z][y][x];
                  cmat[z][y][x].i = comp.i / mat[z][y][x];
               }
            }
          }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation between\n           real 3D matrix and complex number",GetOpString(operation));
          return(ERR); 
    }
    
    dataLeft->AssignCMatrix3D(cmat,width,height,depth);
    return(0);
}


/******************************************************************************************
   Evaluate a REAL 3D MATRIX operating on a REAL NUMBER - result is a REAL 3D MATRIX
******************************************************************************************/

short Matrix3DFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k;
   
   float ***mat = dataLeft->GetMatrix3D();   
   long cols = dataLeft->GetDimX();
   long rows = dataLeft->GetDimY();
   long tiers = dataLeft->GetDimZ();
   float result = dataRight->GetReal();
   
   switch(operation)
   { 
       case (EQ):
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatEqual(mat[k][j][i],result);
          break;
       }
       case ('>') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatGreater(mat[k][j][i],result);
          break;
       }
       case ('<') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatLess(mat[k][j][i],result);
          break;
       }
       case (GEQ) :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatGreaterEqual(mat[k][j][i],result);
          break;
       }
       case (LEQ) :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = FloatLessEqual(mat[k][j][i],result);
          break;
       }
       case ('+') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] += result;
          break;
       }
       case ('-') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] -= result;
          break;
       }
       case ('*') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] *= result;
          break;
       }
       case ('/') :
       { 
          if(result == 0)
          {
             ErrorMessage("divide by zero");
             return(ERR);
          }
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] /= result;
          break;
       }
       case ('^') :
       { 
          for(k = 0; k < tiers; k++)
             for(j = 0; j < rows; j++)
                for(i = 0; i < cols; i++)
                   mat[k][j][i] = pow(mat[k][j][i],result);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a 3D matrix and real number",GetOpString(operation));
          return(ERR); 
    }
    return(0);
}


/******************************************************************************************
 Evaluate a REAL 3D MATRIX operating on a COMPLEX NUMBER - result is a COMPLEX 3D MATRIX
******************************************************************************************/

short Matrix3DCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z;
   long width = dataLeft->GetDimX();
   long height = dataLeft->GetDimY();
   long depth = dataLeft->GetDimZ();
   float ***mat = dataLeft->GetMatrix3D();
   complex comp = dataRight->GetComplex();
   
 // Make output matrix
   complex ***cmat = MakeCMatrix3D(width,height,depth);
   if(!cmat) 
   {
      ErrorMessage("can't allocate memory for '3D matrix op complex' result");
      return(ERR);
   }
          
   switch(operation)
   { 
       case ('+') :
       { 
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = mat[z][y][x] + comp.r;
                  cmat[z][y][x].i = comp.i;
               }
            }
          }
          break;
       }
       case ('-') :
       { 
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = mat[z][y][x] - comp.r;
                  cmat[z][y][x].i = -comp.i;
               }
            }
          }
          break;
       }
       case ('*') :
       { 
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = mat[z][y][x] * comp.r;
                  cmat[z][y][x].i = mat[z][y][x] * comp.i;
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
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = (mat[z][y][x] * comp.r)/denom;
                  cmat[z][y][x].i = -(mat[z][y][x] * comp.i)/denom;
               }
            }
          }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a real 3D matrix and a complex number",GetOpString(operation));
          return(ERR); 
    }
    
    dataLeft->AssignCMatrix3D(cmat,width,height,depth);

    return(OK);
}


/******************************************************************************************
Evaluate a COMPLEX 3D MATRIX operating on a REAL NUMBER - result is a COMPLEX 3D MATRIX
******************************************************************************************/

short CMatrix3DFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k;
   long cols = dataLeft->GetDimX();
   long rows = dataLeft->GetDimY();
   long tiers = dataLeft->GetDimZ();
   complex ***cmat = dataLeft->GetCMatrix3D();
   float realNr = dataRight->GetReal();

   switch(operation)
   { 
       case ('+') :
       { 
          for(k = 0; k < tiers; k++)
          {
	          for(j = 0; j < rows; j++)
	          {
	             for(i = 0; i < cols; i++)
	             {
	                cmat[k][j][i].r = cmat[k][j][i].r + realNr;
	             }
	          }
	       }
          break;
       }
       case ('-') :
       { 
          for(k = 0; k < tiers; k++)
          {       
	          for(long j = 0; j < rows; j++)
	          {
	             for(long i = 0; i < cols; i++)
	             {
	                cmat[k][j][i].r = cmat[k][j][i].r - realNr;
	             }
	          }
	       }
          break;
       }
       case ('*') :
       case (MATMUL) :       
       { 
          for(k = 0; k < tiers; k++)
          {        
	          for(long j = 0; j < rows; j++)
	          {
	             for(long i = 0; i < cols; i++)
	             {
	                cmat[k][j][i].r = cmat[k][j][i].r * realNr;
	                cmat[k][j][i].i = cmat[k][j][i].i * realNr;
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
          for(k = 0; k < tiers; k++)
          {         
	          for(long j = 0; j < rows; j++)
	          {
	             for(long i = 0; i < cols; i++)
	             {
	                cmat[k][j][i].r = cmat[k][j][i].r / realNr;
	                cmat[k][j][i].i = cmat[k][j][i].i / realNr;
	             }
	          }
	       }
          break;
       }
       case ('^') :
       { 
          for(k = 0; k < tiers; k++)
          {     
	          for(long j = 0; j < rows; j++)
	          {
	             for(long i = 0; i < cols; i++)
	             {
	                cmat[k][j][i].r = pow(cmat[k][j][i].r, realNr);
	                cmat[k][j][i].i = pow(cmat[k][j][i].i, realNr);
	             }
	          }
	       }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation between\n           a complex 3D matrix and a real number",GetOpString(operation));
          return(ERR); 
    }
    return(OK);
}


/******************************************************************************************
   Evaluate a REAL NUMBER operating on a 3D COMPLEX MATRIX  - result is a COMPLEX 3D MATRIX
******************************************************************************************/

short FloatCMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k;
   long cols = dataRight->GetDimX();
   long rows = dataRight->GetDimY();
   long tiers = dataRight->GetDimZ();
   complex ***cmat = dataRight->GetCMatrix3D();
   float realNr = dataLeft->GetReal();

   
   switch(operation)
   { 
       case ('+') :
       { 
          for(k = 0; k < tiers; k++)
          {        
	          for(j = 0; j < rows; j++)
	          {
	             for(i = 0; i < cols; i++)
	             {
	                cmat[k][j][i].r = realNr + cmat[k][j][i].r;
	             }
	          }
	       }
          break;
       }
       case ('-') :
       { 
          for(k = 0; k < tiers; k++)
          {         
             for(j = 0; j < rows; j++)
             {
	             for(i = 0; i < cols; i++)
	             {
	                cmat[k][j][i].r = realNr - cmat[k][j][i].r;
	                cmat[k][j][i].i = -cmat[k][j][i].i;
	             }
	          }
	       }
          break;
       }
       case ('*') :
       case (MATMUL) :
       { 
          for(k = 0; k < tiers; k++)
          {         
	          for(j = 0; j < rows; j++)
	          {
	             for(i = 0; i < cols; i++)
	             {
	                cmat[k][j][i].r = realNr * cmat[k][j][i].r;
	                cmat[k][j][i].i = realNr * cmat[k][j][i].i;
	             }
	          }
	       }
          break;
       }
       case ('/') :
       { 
          float denom;
          for(k = 0; k < tiers; k++)
          {          
	          for(j = 0; j < rows; j++)
	          {
	             for(i = 0; i < cols; i++)
	             {
	                denom = cmat[k][j][i].r*cmat[k][j][i].r + cmat[k][j][i].i*cmat[k][j][i].i;
                   if(denom == 0)
                   {
                      ErrorMessage("divide by zero");
                      return(ERR);
                   }
	                cmat[k][j][i].r =  realNr*cmat[k][j][i].r/denom;
	                cmat[k][j][i].i = -realNr*cmat[k][j][i].i/denom;
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
    dataLeft->AssignCMatrix3D(cmat,cols,rows,tiers);
    dataRight->SetNull(); // Data right moved to data left
    
    return(OK);
}


/******************************************************************************************
 Evaluate a COMPLEX NUMBER operating on a COMPLEX 3D MATRIX - result is a COMPLEX 3D MATRIX
******************************************************************************************/

short ComplexCMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k;
   complex comp = dataLeft->GetComplex();
   long cols = dataRight->GetDimX();
   long rows = dataRight->GetDimY();
   long tiers = dataRight->GetDimZ();
   complex ***cmat = dataRight->GetCMatrix3D();
   complex temp;
   float denom;

   switch(operation)
   { 
      case ('+') :
      { 
         for(k = 0; k < tiers; k++)
         {
            for(j = 0; j < rows; j++)
            {
               for(i = 0; i < cols; i++)
               {
                  cmat[k][j][i].r = comp.r + cmat[k][j][i].r;
                  cmat[k][j][i].i = comp.i + cmat[k][j][i].i;
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(k = 0; k < tiers; k++)
         {       
            for(long j = 0; j < rows; j++)
            {
               for(long i = 0; i < cols; i++)
               {
                  cmat[k][j][i].r = comp.r - cmat[k][j][i].r;
                  cmat[k][j][i].i = comp.i - cmat[k][j][i].i;
               }
            }
         }
         break;
      }
      case ('*') :
      case (MATMUL) :       
      { 
         for(k = 0; k < tiers; k++)
         {        
            for(long j = 0; j < rows; j++)
            {
               for(long i = 0; i < cols; i++)
               {
                  temp.r = comp.r*cmat[k][j][i].r - comp.i*cmat[k][j][i].i;
                  temp.i = comp.r*cmat[k][j][i].i + comp.i*cmat[k][j][i].r;
                  cmat[k][j][i] = temp;
               }
            }
         }   
         break;
      }
      case ('/') :
      { 
         for(k = 0; k < tiers; k++)
         {         
            for(long j = 0; j < rows; j++)
            {
               for(long i = 0; i < cols; i++)
               {
                 denom = cmat[k][j][i].r*cmat[k][j][i].r + cmat[k][j][i].i*cmat[k][j][i].i;
                  if(denom == 0)
                  {
                     ErrorMessage("divide by zero");
                     return(ERR);
                  }            
                  temp.r = (comp.r*cmat[k][j][i].r + comp.i*cmat[k][j][i].i)/denom;
                  temp.i = (comp.i*cmat[k][j][i].r - comp.r*cmat[k][j][i].i)/denom;
                  cmat[k][j][i] = temp;
               }
            }
         }
         break;
      }
      default: 
      ErrorMessage("'%s' is an undefined operation between\n           a complex 3D matrix and a complex number",GetOpString(operation));
      return(ERR); 
   }
   dataLeft->AssignCMatrix3D(cmat,cols,rows,tiers);
   dataRight->SetNull(); // Data right moved to data left
   return(OK);
}


/******************************************************************************************
 Evaluate a COMPLEX 3D MATRIX operating on a COMPLEX NUMBER - result is a COMPLEX 3D MATRIX
******************************************************************************************/

short CMatrix3DComplexEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j,k;
   long cols = dataLeft->GetDimX();
   long rows = dataLeft->GetDimY();
   long tiers = dataLeft->GetDimZ();
   complex ***cmat = dataLeft->GetCMatrix3D();
   complex comp = dataRight->GetComplex();
   complex temp;
   float denom;

   switch(operation)
   { 
      case ('+') :
      { 
         for(k = 0; k < tiers; k++)
         {
            for(j = 0; j < rows; j++)
            {
               for(i = 0; i < cols; i++)
               {
                  cmat[k][j][i].r = cmat[k][j][i].r + comp.r;
                  cmat[k][j][i].i = cmat[k][j][i].i + comp.i;
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(k = 0; k < tiers; k++)
         {       
            for(long j = 0; j < rows; j++)
            {
               for(long i = 0; i < cols; i++)
               {
                  cmat[k][j][i].r = cmat[k][j][i].r - comp.r;
                  cmat[k][j][i].i = cmat[k][j][i].i - comp.i;
               }
            }
         }
         break;
      }
      case ('*') :
      case (MATMUL) :       
      { 
         for(k = 0; k < tiers; k++)
         {        
            for(long j = 0; j < rows; j++)
            {
               for(long i = 0; i < cols; i++)
               {
                  temp.r = cmat[k][j][i].r*comp.r - cmat[k][j][i].i*comp.i;
                  temp.i = cmat[k][j][i].i*comp.r + cmat[k][j][i].r*comp.i;
                  cmat[k][j][i] = temp;
               }
            }
         }   
         break;
      }
      case ('/') :
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
                  temp.r = (cmat[k][j][i].r*comp.r + cmat[k][j][i].i*comp.i)/denom;
                  temp.i = (cmat[k][j][i].i*comp.r - cmat[k][j][i].r*comp.i)/denom;
                  cmat[k][j][i] = temp;
               }
            }
         }
         break;
      }
      default: 
         ErrorMessage("'%s' is an undefined operation between\n           a complex 3D matrix and a complex number",GetOpString(operation));
         return(ERR); 
   }
   return(OK);
}


// Evaluate a 3D MATRIX operating on a 3D COMPLEX MATRIX - result is a 3D COMPLEX matrix

short Matrix3DCMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z;
   long width = dataRight->GetDimX();
   long height = dataRight->GetDimY();
   long depth = dataRight->GetDimZ();
   float ***rmat = dataLeft->GetMatrix3D();
   complex ***cmat = dataRight->GetCMatrix3D();
   complex ***result = 0;
   
// Make sure matrix dimensions are the same if not matrix multiple
	if(operation != '*')
	{
	   if((dataLeft->GetDimY() != height) || (dataLeft->GetDimX() != width) || (dataLeft->GetDimZ() != depth))
	   {
	      ErrorMessage("matrix dimensions don't match");
	      return(ERR);
	   }
	   
   // Allocate space for result which will be complex            
      result = MakeCMatrix3D(width,height,depth);
		if(!result) 
		{
		   ErrorMessage("can't allocate memory for 3D matrix - 3D cmatrix operation");
		   return(ERR);
		}      
   }


// Evaluate expression
   
   switch(operation)
   { 
      case ('+') :
      { 
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  result[z][y][x].r = rmat[z][y][x] + cmat[z][y][x].r;
                  result[z][y][x].i = cmat[z][y][x].i;
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  result[z][y][x].r = rmat[z][y][x] - cmat[z][y][x].r;
                  result[z][y][x].i = -cmat[z][y][x].i;
               }
            }
         }
         break;
      }

      case (MATMUL) : // .*
      { 
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  result[z][y][x].r = rmat[z][y][x]*cmat[z][y][x].r;
                  result[z][y][x].i = rmat[z][y][x]*cmat[z][y][x].i;
               }
            }
         }
         break;
       }
      
      case ('/') : 
      { 
         float denom;
          
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  denom = cmat[z][y][x].r*cmat[z][y][x].r + cmat[z][y][x].i*cmat[z][y][x].i;
                  if(denom == 0)
                  {
                     ErrorMessage("divide by zero");
                     return(ERR);
                  }
	               result[z][y][x].r = (rmat[z][y][x]*cmat[z][y][x].r)/denom;
	               result[z][y][x].i = (-rmat[z][y][x]*cmat[z][y][x].i)/denom;	
               }
            }
         } 
         break;
      }       

      default: 
         ErrorMessage("'%s' is an undefined operation\n           between real and complex matricies",GetOpString(operation));
         return(ERR); 
   }

   dataLeft->AssignCMatrix3D(result,width,height,depth);
   return(OK);
}



/******************************************************************************************
   Evaluate a 3D COMPLEX MATRIX operating on a 3DMATRIX - result is a 3D COMPLEX MATRIX
******************************************************************************************/

short CMatrix3DMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z;
   long width = dataRight->GetDimX();
   long height = dataRight->GetDimY();
   long depth = dataRight->GetDimZ();
   float ***rmat = dataRight->GetMatrix3D();
   complex ***cmat = dataLeft->GetCMatrix3D();
   
// Make sure matrix dimensions are the same if not matrix multiple
	if(operation != '*')
	{
	   if((height != dataLeft->GetDimY()) || (width != dataLeft->GetDimX()) || (depth != dataLeft->GetDimZ()))
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
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = cmat[z][y][x].r + rmat[z][y][x];
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = cmat[z][y][x].r - rmat[z][y][x];
               }
            }
         }
         break;
      }
      case (MATMUL) : // .*
      { 
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  cmat[z][y][x].r = cmat[z][y][x].r * rmat[z][y][x];
                  cmat[z][y][x].i = cmat[z][y][x].i * rmat[z][y][x];
               }
            }
         }
         break;
      }
     
      case ('/') : 
      {
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  if(rmat[z][y][x] == 0)
                  {
                     ErrorMessage("divide by zero");
                     return(ERR);
                  } 
                  cmat[z][y][x].r = cmat[z][y][x].r/rmat[z][y][x];
                  cmat[z][y][x].i = cmat[z][y][x].i/rmat[z][y][x];
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
   Evaluate a REAL 3D MATRIX operating on a REAL 3D MATRIX - result is a REAL 3D MATRIX
******************************************************************************************/

short Matrix3DMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z;
   long width = dataLeft->GetDimX();
   long height = dataLeft->GetDimY();
   long depth = dataLeft->GetDimZ();
   float ***src = dataRight->GetMatrix3D();
   float ***dst = dataLeft->GetMatrix3D();
   
   if(operation != '*' && (dataRight->GetDimY() != height || dataRight->GetDimX() != width || dataRight->GetDimZ() != depth))
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
	       
         for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
	               neq += nint(FloatNotEqual(dst[z][y][x],src[z][y][x]));
         dataLeft->MakeAndSetFloat(neq == 0);
         return(OK);
      }	
      case (NEQ) :
      {
         long neq = 0;
	       
         for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
                  neq += nint(FloatNotEqual(dst[z][y][x],src[z][y][x]));
         dataLeft->MakeAndSetFloat(neq != 0);
	      return(OK);
      }	         
       case ('+') :
       { 
          for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
                  dst[z][y][x] = dst[z][y][x] + src[z][y][x];
          break;
       }
       case ('-') :
       { 
          for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
                  dst[z][y][x] = dst[z][y][x] - src[z][y][x];
          break;
       }
       case (MATMUL) :
       { 
          for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
                  dst[z][y][x] = dst[z][y][x] * src[z][y][x];
          break;
       } 
       case ('/') :
       { 
          for(z = 0; z < depth; z++)
          {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  if(src[z][y][x] == 0)
                  {
                     ErrorMessage("divide by zero");
                     return(ERR);
                  } 
                  dst[z][y][x] = dst[z][y][x]/src[z][y][x];
               }
            }
          }
          break;
       }
       case ('^') :
       { 
          for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
                  dst[z][y][x] = pow(dst[z][y][x],src[z][y][x]);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between 3D matrices",GetOpString(operation));
          return(ERR); 
    }
	 return(OK);
}




/******************************************************************************************
Evaluate a 3D COMPLEX MATRIX operating on a 3D COMPLEX MATRIX - result is a 3D COMPLEX MATRIX
******************************************************************************************/

short CMatrix3DCMatrix3DEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long x,y,z;
   complex temp;
   long width = dataRight->GetDimX();
   long height = dataRight->GetDimY();
   long depth = dataRight->GetDimZ();
   complex ***src = dataRight->GetCMatrix3D();
   complex ***dst = dataLeft->GetCMatrix3D();
   
   if(operation != '*' && (dataLeft->GetDimY() != height || dataLeft->GetDimX() != width || dataLeft->GetDimZ() != depth))
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
         for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
                  neq += (FloatNotEqual(dst[z][y][x].r,src[z][y][x].r) || FloatNotEqual(dst[z][y][x].i,src[z][y][x].i));
	       dataLeft->MakeAndSetFloat(neq == 0);
	       return(OK);
	       break;
      }	
      case (NEQ) :
      {
         long neq = 0;
         for(z = 0; z < depth; z++)
            for(y = 0; y < height; y++)
               for(x = 0; x < width; x++)
                  neq += (FloatNotEqual(dst[z][y][x].r,src[z][y][x].r) || FloatNotEqual(dst[z][y][x].i,src[z][y][x].i));
	      dataLeft->MakeAndSetFloat(neq != 0);
         return(OK);
         break;
      }	
	       
      case ('+') :
      { 
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  dst[z][y][x].r = dst[z][y][x].r + src[z][y][x].r;
                  dst[z][y][x].i = dst[z][y][x].i + src[z][y][x].i;
               }
            }
         }
         break;
      }
      case ('-') :
      { 
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  dst[z][y][x].r = dst[z][y][x].r - src[z][y][x].r;
                  dst[z][y][x].i = dst[z][y][x].i - src[z][y][x].i;
               }
            }
         }
         break;
      }
      case (MATMUL) : // .*
      { 
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  dst[z][y][x].r = dst[z][y][x].r*src[z][y][x].r - dst[z][y][x].i*src[z][y][x].r;
                  dst[z][y][x].i = dst[z][y][x].r*src[z][y][x].i + dst[z][y][x].i*src[z][y][x].r;
               }
            }
         }
         break;
      }
 
      case ('/') : 
      { 
         float denom;
          
         for(z = 0; z < depth; z++)
         {
            for(y = 0; y < height; y++)
            {
               for(x = 0; x < width; x++)
               {
                  denom = src[z][y][x].r*src[z][y][x].r + src[z][y][x].i*src[z][y][x].i;
                  if(denom == 0)
                  {
                     ErrorMessage("divide by zero");
                     return(ERR);
                  }                
                  temp.r = (dst[z][y][x].r*src[z][y][x].r + dst[z][y][x].i*src[z][y][x].i)/denom;
                  temp.i = (dst[z][y][x].i*src[z][y][x].r - dst[z][y][x].r*src[z][y][x].i)/denom;
                  dst[z][y][x] = temp;
               }
            }
         }
         break;
      }
      default: 
         ErrorMessage("'%s' is an undefined operation\n           between 3D complex matricies",GetOpString(operation));
         return(ERR); 
    }
    return(OK);
}
