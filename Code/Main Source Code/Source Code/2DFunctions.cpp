#include "stdafx.h"
#include "2DFunctions.h"
#include <math.h>
#include "allocate.h"
#include "evaluate.h"
#include "globals.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "mymath.h"
#include "operators.h"
#include "memoryLeak.h"

MatMulMode matMulMode = TRANSPOSE;

/*********************************************************************************
   Evaluate a REAL MATRIX operating on a REAL NUMBER - result is a REAL MATRIX
*********************************************************************************/

short MatrixFloatEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
   long i,j;
   float **mat  = dataLeft->GetMatrix2D();   
   long cols    = dataLeft->GetDimX();
   long rows    = dataLeft->GetDimY();
   float result = dataRight->GetReal();
   
   switch(operation)
   { 
       case (EQ):
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatEqual(mat[j][i],result);
          break;
       }
       case ('>') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatGreater(mat[j][i],result);
          break;
       }
       case ('<') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatLess(mat[j][i],result);
          break;
       }
       case (GEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatGreaterEqual(mat[j][i],result);
          break;
       }
       case (LEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatLessEqual(mat[j][i],result);
          break;
       }
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] += result;
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] -= result;
          break;
       }
       case ('*') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] *= result;
          break;
       }
       case ('/') :
       { 
          if(result == 0)
          {
             ErrorMessage("divide by zero");
             return(ERR);
          }
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] /= result;
          break;
       }
       case ('&') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(mat[j][i]) & nhint(result);
          break;
       }
       case ('|') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(mat[j][i]) | nhint(result);
          break;
       }
       case ('^') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = pow(mat[j][i],result);
          break;
       }
	   case ('%') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(mat[j][i]) % nhint(result);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a matrix and real number",GetOpString(operation));
          return(ERR); 
    }
    return(0);
}


/*********************************************************************************
   Evaluate a REAL NUMBER operating on a REAL MATRIX2D - result is a real matrix
*********************************************************************************/
 
short FloatMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;

   float result = dataLeft->GetReal();
   long cols    = dataRight->GetDimX();
   long rows    = dataRight->GetDimY();
   float **mat  = dataRight->GetMatrix2D();

   switch(operation)
   { 
      case (EQ):
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatEqual(mat[j][i],result);
          break;
       }
       case ('>') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatGreater(mat[j][i],result);
          break;
       }
       case ('<') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatLess(mat[j][i],result);
          break;
       }
       case (GEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatGreaterEqual(mat[j][i],result);
          break;
       }
       case (LEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = FloatLessEqual(mat[j][i],result);
          break;
       }
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result + mat[j][i];
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result - mat[j][i];
          break;
       }
       case ('*') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result * mat[j][i];
          break;
       }
       case ('/') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                if(mat[j][i] == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                }
                mat[j][i] = result / mat[j][i];
             }
          }
          break;
       }
       case ('^') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = pow(result,mat[j][i]);
          break;
       }
       case ('&') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(result) & nhint(mat[j][i]);
          break;
       }
       case ('|') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(result) | nhint(mat[j][i]);
          break;
       }
       case ('%') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(result) % nhint(mat[j][i]);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a real number and matrix",GetOpString(operation));
          return(ERR); 
    }

    dataLeft->AssignMatrix2D(mat,cols,rows);
    dataRight->SetNull(); // Data right moved to data left

    return(OK);
}

/*********************************************************************************
   Evaluate a COMPLEX NUMBER operating on a REAL MATRIX - result is a complex matrix
*********************************************************************************/
 
short CompMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long cols    = dataRight->GetDimX();
   long rows    = dataRight->GetDimY();
   float **mat  = dataRight->GetMatrix2D();
   complex comp = dataLeft->GetComplex();

   complex **cmat = MakeCMatrix2D(cols,rows);

   if(!cmat) 
   {
      ErrorMessage("can't allocate memory for 'complex op matrix' result");
      return(ERR);
   }
          
   switch(operation)
   { 
       case ('+') :
       { 
          for(long j = 0; j < rows; j++)
          {
             for(long i = 0; i < cols; i++)
             {
                cmat[j][i].r = comp.r + mat[j][i];
                cmat[j][i].i = comp.i;
             }
          }
          break;
       }
       case ('-') :
       { 
          for(long j = 0; j < rows; j++)
          {
             for(long i = 0; i < cols; i++)
             {
                cmat[j][i].r = comp.r - mat[j][i];
                cmat[j][i].i = comp.i;
             }
          }
          break;
       }
       case ('*') :
       { 
          for(long j = 0; j < rows; j++)
          {
             for(long i = 0; i < cols; i++)
             {
                cmat[j][i].r = comp.r * mat[j][i];
                cmat[j][i].i = comp.i * mat[j][i];
             }
          }
          break;
       }
       case ('/') :
       { 
          for(long j = 0; j < rows; j++)
          {
             for(long i = 0; i < cols; i++)
             {
                if(mat[j][i] == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                }
                cmat[j][i].r = comp.r/mat[j][i];
                cmat[j][i].i = comp.i/mat[j][i];
             }
          }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation between\n           real matrix and complex number",GetOpString(operation));
          return(ERR); 
    }
    
    dataRight->FreeData();
    dataLeft->AssignCMatrix2D(cmat,cols,rows);

    return(0);
}


/*********************************************************************************
   Evaluate a REAL MATRIX operating on a COMPLEX NUMBER - result is a COMPLEX MATRIX
*********************************************************************************/
 
short MatrixCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   float **mat  = dataLeft->GetMatrix2D();   
   long cols    = dataLeft->GetDimX();
   long rows    = dataLeft->GetDimY();
   complex comp = dataRight->GetComplex();
   
 // Make output matrix
   complex **cmat = MakeCMatrix2D(cols,rows);

   if(!cmat) 
   {
      ErrorMessage("can't allocate memory for 'matrix op complex' result");
      return(ERR);
   }
          
   switch(operation)
   { 
       case ('+') :
       { 
          for(long j = 0; j < rows; j++)
          {
             for(long i = 0; i < cols; i++)
             {
                cmat[j][i].r = mat[j][i] + comp.r;
                cmat[j][i].i = comp.i;
             }
          }
          break;
       }
       case ('-') :
       { 
          for(long j = 0; j < rows; j++)
          {
             for(long i = 0; i < cols; i++)
             {
                cmat[j][i].r = mat[j][i] - comp.r;
                cmat[j][i].i = -comp.i;
             }
          }
          break;
       }
       case ('*') :
       { 
          for(long j = 0; j < rows; j++)
          {
             for(long i = 0; i < cols; i++)
             {
                cmat[j][i].r = mat[j][i] * comp.r;
                cmat[j][i].i = mat[j][i] * comp.i;
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
          for(long j = 0; j < rows; j++)
          {
             for(long i = 0; i < cols; i++)
             {
                cmat[j][i].r = (mat[j][i] * comp.r)/denom;
                cmat[j][i].i = -(mat[j][i] * comp.i)/denom;
             }
          }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a real matrix and a complex number",GetOpString(operation));
          return(ERR); 
    }
    
    dataLeft->AssignCMatrix2D(cmat,cols,rows);
    return(OK);
}

/******************************************************************************************
  Evaluate a COMPLEX NUMBER operating on a COMPLEX MATRIX - result is a COMPLEX matrix
******************************************************************************************/

short CompCMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;
   complex comp    = dataLeft->GetComplex();
   long cols       = dataRight->GetDimX();
   long rows       = dataRight->GetDimY();
   complex **cmat  = dataRight->GetCMatrix2D();
   complex temp; 

   switch(operation)
   { 
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = comp.r + cmat[j][i].r;
                cmat[j][i].i = comp.i + cmat[j][i].i;
             }
          }
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = comp.r - cmat[j][i].r;
                cmat[j][i].i = comp.i - cmat[j][i].i;
             }
          }
          break;
       }
       case ('*') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                temp.r = comp.r*cmat[j][i].r - comp.i*cmat[j][i].i;
                temp.i = comp.r*cmat[j][i].i + comp.i*cmat[j][i].r;
                cmat[j][i] = temp;
             }
          }
          break;
       }
       case ('/') :
       { 
          float denom;
          complex temp;
          
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                denom = cmat[j][i].r*cmat[j][i].r + cmat[j][i].i*cmat[j][i].i;
                if(denom == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                }             
                temp.r = (comp.r*cmat[j][i].r + comp.i*cmat[j][i].i)/denom;
                temp.i = (comp.i*cmat[j][i].r - comp.r*cmat[j][i].i)/denom;
                cmat[j][i] = temp;              
             }
          }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation between\n           a complex number and a complex matrix",GetOpString(operation));
          return(ERR); 
    }

    dataLeft->AssignCMatrix2D(cmat,cols,rows);
    dataRight->SetNull(); // Data right moved to data left
    return(OK);
}


/******************************************************************************************
   Evaluate a COMPLEX MATRIX operating on a COMPLEX NUMBER - result is a COMPLEX matrix
******************************************************************************************/

short CMatrixCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;

   complex **cmat = dataLeft->GetCMatrix2D();   
   long cols      = dataLeft->GetDimX();
   long rows      = dataLeft->GetDimY();
   complex comp   = dataRight->GetComplex();
   complex temp;
          
   switch(operation)
   { 
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = cmat[j][i].r + comp.r;
                cmat[j][i].i = cmat[j][i].i + comp.i;
             }
          }
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = cmat[j][i].r - comp.r;
                cmat[j][i].i = cmat[j][i].i - comp.i;
             }
          }
          break;
       }
       case ('*') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                temp.r = cmat[j][i].r*comp.r - cmat[j][i].i*comp.i;
                temp.i = cmat[j][i].i*comp.r + cmat[j][i].r*comp.i;
                cmat[j][i] = temp;
             }
          }
          break;
       }
       case ('/') :
       { 
          float denom;
          
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                denom = comp.r*comp.r + comp.i*comp.i;
                if(denom == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                }            
                temp.r = (cmat[j][i].r*comp.r + cmat[j][i].i*comp.i)/denom;
                temp.i = (cmat[j][i].i*comp.r - cmat[j][i].r*comp.i)/denom;
                cmat[j][i] = temp;
             }
          }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation between\n           a complex number and a complex matrix",GetOpString(operation));
          return(ERR); 
    }
    return(OK);
}



/*********************************************************************************
   Evaluate a REAL NUMBER operating on a COMPLEX MATRIX  - result is a complex matrix
*********************************************************************************/
 
short FloatCMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;
   float realNr   = dataLeft->GetReal();
   long cols      = dataRight->GetDimX();
   long rows      = dataRight->GetDimY();
   complex **cmat = dataRight->GetCMatrix2D();

   switch(operation)
   { 
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = realNr + cmat[j][i].r;
             }
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = realNr - cmat[j][i].r;
                cmat[j][i].i = -cmat[j][i].i;
             }
          break;
       }
       case ('*') :
       case (MATMUL) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = realNr * cmat[j][i].r;
                cmat[j][i].i = realNr * cmat[j][i].i;
             }
          break;
       }
       case ('/') :
       { 
          float denom;
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
             {
                denom = cmat[j][i].r*cmat[j][i].r + cmat[j][i].i*cmat[j][i].i;
                if(denom == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                }
                cmat[j][i].r =  realNr*cmat[j][i].r/denom;
                cmat[j][i].i = -realNr*cmat[j][i].i/denom;
             }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a real number and a complex matrix",GetOpString(operation));
          return(ERR); 
    }

// Make sure data on top of stack is the result
    dataLeft->AssignCMatrix2D(cmat,cols,rows);
    dataRight->SetNull(); // Data right moved to data left
    
    return(OK);
}


/*********************************************************************************
   Evaluate a COMPLEX MATRIX operating on a REAL NUMBER - result is a complex matrix
*********************************************************************************/

short CMatrixFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;
   complex **cmat  = dataLeft->GetCMatrix2D();   
   long cols       = dataLeft->GetDimX();
   long rows       = dataLeft->GetDimY();
   float realNr    = dataRight->GetReal();

   switch(operation)
   { 
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = cmat[j][i].r + realNr;
             }
          }
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = cmat[j][i].r - realNr;
             }
          }
          break;
       }
       case ('*') :
       case (MATMUL) :       
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = cmat[j][i].r * realNr;
                cmat[j][i].i = cmat[j][i].i * realNr;
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
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = cmat[j][i].r / realNr;
                cmat[j][i].i = cmat[j][i].i / realNr;
             }
          }
          break;
       }
       case ('^') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                cmat[j][i].r = pow(cmat[j][i].r, realNr);
                cmat[j][i].i = pow(cmat[j][i].i, realNr);
             }
          }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation between\n           a complex matrix and a real number",GetOpString(operation));
          return(ERR); 
    }
    return(OK);
}


/******************************************************************************************
   Evaluate a REAL MATRIX operating on a REAL MATRIX - result is a REAL MATRIX

   Allowable operations

   A EQ B     Test for equality
   A NEQ B    Test for inequality
   A + B      Add element by element
   A - B      Subtract element by element
   A .* B     Multiply element by element
   A * B      Proper matrix multiply
   A / B      Divide element by element
   A ^ B      Raise to power - element by element

   Only B may be a row or column vector if A is a m by n matrix .

******************************************************************************************/

short MatrixMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;
   long colsL = dataLeft->GetDimX();
   long rowsL = dataLeft->GetDimY();
   long colsR = dataRight->GetDimX();
   long rowsR = dataRight->GetDimY();
   float **src = dataRight->GetMatrix2D();
   float **dst = dataLeft->GetMatrix2D();
   bool err = false;
   

   switch(operation)
   { 
	   case (EQ) :
	   case ('=') :
	   {
         if(rowsR == rowsL && colsR == colsL) // Testing for equality
         {
	         long neq = 0;
   	       
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 neq += nint(FloatNotEqual(dst[j][i],src[j][i]));
            dataLeft->MakeAndSetFloat(neq == 0);
            return(OK);
         }
         else
         {
            dataLeft->MakeAndSetFloat(0);
            return(OK);
         }
         break;
	   }	
	   case (NEQ) :
	   {
         if(rowsR == rowsL && colsR == colsL) // Testing for inequality
         {
            long neq = 0;

            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 neq += nint(FloatNotEqual(dst[j][i],src[j][i]));

            dataLeft->MakeAndSetFloat(neq != 0);
            return(OK);
         }
         else
         {
            dataLeft->MakeAndSetFloat(1);
            return(OK);
         }
         break;
	   }	
      case ('|') :
      { 
         if(rowsR == rowsL && colsR == colsL) // Oring two matrices
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = nint(dst[j][i]) | nint(src[j][i]);
         }
         else if(colsR == colsL && rowsR == 1) // Oring a row vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = nint(dst[j][i]) | nint(src[0][i]);
         }
         else if(rowsR == rowsL && colsR == 1) // Oring by a column vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = nint(dst[j][i]) | nint(src[j][0]);
         }
         else
         {
            err = true;
         }
         break;
      }
      case ('&') :
      { 
         if(rowsR == rowsL && colsR == colsL) // Anding two matrices
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = nint(dst[j][i]) & nint(src[j][i]);
         }
         else if(colsR == colsL && rowsR == 1) // Anding a row vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = nint(dst[j][i]) & nint(src[0][i]);
         }
         else if(rowsR == rowsL && colsR == 1) // Anding by a column vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = nint(dst[j][i]) & nint(src[j][0]);
         }
         else
         {
            err = true;
         }
         break;
      }
      case ('+') :
      { 
         if(rowsR == rowsL && colsR == colsL) // Adding two matrices
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = dst[j][i] + src[j][i];
         }
         else if(colsR == colsL && rowsR == 1) // Adding a row vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] + src[0][i];
         }
         else if(rowsR == rowsL && colsR == 1) // Adding by a column vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] + src[j][0];
         }
         else
         {
            err = true;
         }
         break;
      }
      case ('-') :
      { 
         if(rowsR == rowsL && colsR == colsL) //  Subtracting two matrices
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] - src[j][i];
         }
         else if(colsR == colsL && rowsR == 1) // Subtracting a row vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] - src[0][i];
         }
         else if(rowsR == rowsL && colsR == 1) // Subtracting by a column vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] - src[j][0];
         }
         else
         {
            err = true;
         }
         break;
      }
      case (MATMUL) : // Multiply matrix elements
      { 
         if(rowsR == rowsL && colsR == colsL) // Multiplying by a matrix
         {
            for(j = 0; j < rowsL; j++)
               for(i = 0; i < colsL; i++)
                  dst[j][i] = dst[j][i] * src[j][i];
         }
         else if(colsR == colsL && rowsR == 1) // Multiplying by a row vector
         {
             for(j = 0; j < rowsL; j++)
                for(i = 0; i < colsL; i++)
                   dst[j][i] = dst[j][i] * src[0][i];
         }
         else if(rowsR == rowsL && colsR == 1) // Multiplying by a column vector
         {
             for(j = 0; j < rowsL; j++)
                for(i = 0; i < colsL; i++)
                   dst[j][i] = dst[j][i] * src[j][0];
         }
         else
         {
            err = true;
         }
         break;
       } 
       case ('*') : // Proper matrix multiply
       { 
          if(rowsR != colsL)
          {
             ErrorMessage("inner matrix dimensions don't match for '*' operation");
             return(ERR);
          }
          float sum;
          long csize = rowsR;
          float **prod = MakeMatrix2D(colsR,rowsL);
          if(!prod) 
          {
             ErrorMessage("can't allocate memory for matrix product");
             return(ERR);
          }   
          if(matMulMode == NAIVE)
          {
             for(j = 0; j < rowsL; j++)
             {
                for(i = 0; i < colsR; i++)
                {
                   sum = 0.0;
                   for(long k = 0; k < csize; k++)
                   {
                      sum += dst[j][k]*src[k][i];
                   }
                   prod[j][i] = sum;
                }
             }
             if(colsR == 1 && rowsL == 1)
             {
                dataLeft->MakeAndSetFloat(prod[0][0]);
                FreeMatrix2D(prod);
             }
             else
             {
                dataLeft->FreeData();
                dataLeft->AssignMatrix2D(prod,colsR,rowsL);
             }
          }
          else // Transpose version
          {
             float **trans = MakeMatrix2D(rowsR,colsR);
             if(!trans) 
             {
                FreeMatrix2D(prod);
                ErrorMessage("can't allocate memory for matrix product (transpose step)");
                return(ERR);
             }
            // Transpose second matrix to speed up memory access
             for(j = 0; j < rowsR; j++)
             {
                for(i = 0; i < colsR; i++)
                {
                  trans[i][j] = src[j][i];
                }
             }
           // Perform multiplication  
             for(j = 0; j < rowsL; j++)
             {
                for(i = 0; i < colsR; i++)
                {
                   sum = 0.0;
                   for(long k = 0; k < csize; k++)
                   {
                      sum += dst[j][k]*trans[i][k];
                   }
                   prod[j][i] = sum;
                }
             }
             FreeMatrix2D(trans);
             if(colsR == 1 && rowsL == 1)
             {
                dataLeft->MakeAndSetFloat(prod[0][0]);
                FreeMatrix2D(prod);
             }
             else
             {
                dataLeft->FreeData();
                dataLeft->AssignMatrix2D(prod,colsR,rowsL);
             }
          }
		    return(OK);          
          break;
      }       
      case ('/') : // Divide elements
      { 
         if(rowsR == rowsL && colsR == colsL) // Dividing by a matrix
         {
            for(j = 0; j < rowsL; j++)
            {
               for(i = 0; i < colsL; i++)
               {
                  if(src[j][i] == 0)
                  {
                     ErrorMessage("divide by zero");
                     return(ERR);
                  } 
                  dst[j][i] = dst[j][i]/src[j][i];
               }
            }
         }
         else if(colsR == colsL && rowsR == 1) // Dividing by a row vector
         {
            for(i = 0; i < colsL; i++)
            {
               if(src[0][i] == 0)
               {
                  ErrorMessage("divide by zero");
                  return(ERR);
               }
               for(j = 0; j < rowsL; j++)
               {
                  dst[j][i] = dst[j][i] / src[0][i];
               }
            }
         }
         else if(rowsR == rowsL && colsR == 1) // Dividing by a column vector
         {
            for(j = 0; j < rowsL; j++)
            {
               if(src[j][0] == 0)
               {
                  ErrorMessage("divide by zero");
                  return(ERR);
               }
               for(i = 0; i < colsL; i++)
               {
                  dst[j][i] = dst[j][i] / src[j][0];
               }
            }
         }
         else
         {
            err = true;
         }
         break;
      }
      case ('^') : // Raise elements to a power
      { 
         if(rowsR == rowsL && colsR == colsL) // Raising to the power of a matrix
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = pow(dst[j][i],src[j][i]);
         }
         else if(colsR == colsL && rowsR == 1) // Raising to the power of a row vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = pow(dst[j][i],src[0][i]);
         }
         else if(rowsR == rowsL && colsR == 1) // Raising to the power of a column vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = pow(dst[j][i],src[j][0]);
         }
         else
         {
            err = true;
         }
         break;
      }
      default: 
      {
         ErrorMessage("'%s' is an undefined operation\n           between matrices",GetOpString(operation));
         return(ERR);
      }
   }

   if(err)
   {
      ErrorMessage("matrix sizes don't agree in '%s' operation",GetOpString(operation));
      return(ERR);
   }

   return(OK);
}


/******************************************************************************************
   Evaluate a REAL DMATRIX operating on a REAL DMATRIX - result is a REAL DMATRIX

   Allowable operations

   A EQ B     Test for equality
   A NEQ B    Test for inequality
   A + B      Add element by element
   A - B      Subtract element by element
   A .* B     Multiply element by element
   A * B      Proper matrix multiply
   A / B      Divide element by element
   A ^ B      Raise to power - element by element

   Only B may be a row or column vector if A is a m by n matrix .

******************************************************************************************/

short DMatrixDMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;
   long colsL = dataLeft->GetDimX();
   long rowsL = dataLeft->GetDimY();
   long colsR = dataRight->GetDimX();
   long rowsR = dataRight->GetDimY();
   double **src = dataRight->GetDMatrix2D();
   double **dst = dataLeft->GetDMatrix2D();
   bool err = false;
   

   switch(operation)
   { 
	   case (EQ) :
	   case ('=') :
	   {
         if(rowsR == rowsL && colsR == colsL) // Testing for equality
         {
	         long neq = 0;
   	       
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 neq += nint(DoubleNotEqual(dst[j][i],src[j][i]));
            dataLeft->MakeAndSetDouble(neq == 0);
            return(OK);
         }
         else
         {
            err = true;
         }
         break;
	   }	
	   case (NEQ) :
	   {
         if(rowsR == rowsL && colsR == colsL) // Testing for inequality
         {
            long neq = 0;

            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 neq += nint(DoubleNotEqual(dst[j][i],src[j][i]));

            dataLeft->MakeAndSetDouble(neq != 0);
            return(OK);
         }
         else
         {
            err = true;
         }
         break;
	   }	         
      case ('+') :
      { 
         if(rowsR == rowsL && colsR == colsL) // Adding two matrices
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = dst[j][i] + src[j][i];
         }
         else if(colsR == colsL && rowsR == 1) // Adding a row vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] + src[0][i];
         }
         else if(rowsR == rowsL && colsR == 1) // Adding by a column vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] + src[j][0];
         }
         else
         {
            err = true;
         }
         break;
      }
      case ('-') :
      { 
         if(rowsR == rowsL && colsR == colsL) //  Subtracting two matrices
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] - src[j][i];
         }
         else if(colsR == colsL && rowsR == 1) // Subtracting a row vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] - src[0][i];
         }
         else if(rowsR == rowsL && colsR == 1) // Subtracting by a column vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                dst[j][i] = dst[j][i] - src[j][0];
         }
         else
         {
            err = true;
         }
         break;
      }
      case (MATMUL) : // Multiply matrix elements
      { 
         if(rowsR == rowsL && colsR == colsL) // Multiplying by a matrix
         {
            for(j = 0; j < rowsL; j++)
               for(i = 0; i < colsL; i++)
                  dst[j][i] = dst[j][i] * src[j][i];
         }
         else if(colsR == colsL && rowsR == 1) // Multiplying by a row vector
         {
             for(j = 0; j < rowsL; j++)
                for(i = 0; i < colsL; i++)
                   dst[j][i] = dst[j][i] * src[0][i];
         }
         else if(rowsR == rowsL && colsR == 1) // Multiplying by a column vector
         {
             for(j = 0; j < rowsL; j++)
                for(i = 0; i < colsL; i++)
                   dst[j][i] = dst[j][i] * src[j][0];
         }
         else
         {
            err = true;
         }
         break;
       } 
       case ('*') : // Proper matrix multiply
       { 
          if(rowsR != colsL)
          {
             ErrorMessage("inner matrix dimensions don't match for '*' operation");
             return(ERR);
          }
          double sum;
          long csize = rowsR;
          double **prod = MakeDMatrix2D(colsR,rowsL);
          if(!prod) 
          {
             ErrorMessage("can't allocate memory for matrix product");
             return(ERR);
          }   
          if(matMulMode == NAIVE)
          {
             for(j = 0; j < rowsL; j++)
             {
                for(i = 0; i < colsR; i++)
                {
                   sum = 0.0;
                   for(long k = 0; k < csize; k++)
                   {
                      sum += dst[j][k]*src[k][i];
                   }
                   prod[j][i] = sum;
                }
             }
             if(colsR == 1 && rowsL == 1)
             {
                dataLeft->MakeAndSetDouble(prod[0][0]);
                FreeDMatrix2D(prod);
             }
             else
             {
                dataLeft->FreeData();
                dataLeft->AssignDMatrix2D(prod,colsR,rowsL);
             }
          }
          else // Transpose version
          {
             double **trans = MakeDMatrix2D(rowsR,colsR);
             if(!trans) 
             {
                FreeDMatrix2D(prod);
                ErrorMessage("can't allocate memory for matrix product (transpose step)");
                return(ERR);
             }
            // Transpose second matrix to speed up memory access
             for(j = 0; j < rowsR; j++)
             {
                for(i = 0; i < colsR; i++)
                {
                  trans[i][j] = src[j][i];
                }
             }
           // Perform multiplication  
             for(j = 0; j < rowsL; j++)
             {
                for(i = 0; i < colsR; i++)
                {
                   sum = 0.0;
                   for(long k = 0; k < csize; k++)
                   {
                      sum += dst[j][k]*trans[i][k];
                   }
                   prod[j][i] = sum;
                }
             }
             FreeDMatrix2D(trans);
             if(colsR == 1 && rowsL == 1)
             {
                dataLeft->MakeAndSetDouble(prod[0][0]);
                FreeDMatrix2D(prod);
             }
             else
             {
                dataLeft->FreeData();
                dataLeft->AssignDMatrix2D(prod,colsR,rowsL);
             }
          }
		    return(OK);          
          break;
      }       
      case ('/') : // Divide elements
      { 
         if(rowsR == rowsL && colsR == colsL) // Dividing by a matrix
         {
            for(j = 0; j < rowsL; j++)
            {
               for(i = 0; i < colsL; i++)
               {
                  if(src[j][i] == 0)
                  {
                     ErrorMessage("divide by zero");
                     return(ERR);
                  } 
                  dst[j][i] = dst[j][i]/src[j][i];
               }
            }
         }
         else if(colsR == colsL && rowsR == 1) // Dividing by a row vector
         {
            for(i = 0; i < colsL; i++)
            {
               if(src[0][i] == 0)
               {
                  ErrorMessage("divide by zero");
                  return(ERR);
               }
               for(j = 0; j < rowsL; j++)
               {
                  dst[j][i] = dst[j][i] / src[0][i];
               }
            }
         }
         else if(rowsR == rowsL && colsR == 1) // Dividing by a column vector
         {
            for(j = 0; j < rowsL; j++)
            {
               if(src[j][0] == 0)
               {
                  ErrorMessage("divide by zero");
                  return(ERR);
               }
               for(i = 0; i < colsL; i++)
               {
                  dst[j][i] = dst[j][i] / src[j][0];
               }
            }
         }
         else
         {
            err = true;
         }
         break;
      }
      case ('^') : // Raise elements to a power
      { 
         if(rowsR == rowsL && colsR == colsL) // Raising to the power of a matrix
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = pow(dst[j][i],src[j][i]);
         }
         else if(colsR == colsL && rowsR == 1) // Raising to the power of a row vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = pow(dst[j][i],src[0][i]);
         }
         else if(rowsR == rowsL && colsR == 1) // Raising to the power of a column vector
         {
            for(j = 0; j < rowsL; j++)
              for(i = 0; i < colsL; i++)
                 dst[j][i] = pow(dst[j][i],src[j][0]);
         }
         else
         {
            err = true;
         }
         break;
      }
      default: 
      {
         ErrorMessage("'%s' is an undefined operation\n           between matrices",GetOpString(operation));
         return(ERR);
      }
   }

   if(err)
   {
      ErrorMessage("matrix sizes don't agree in '%s' operation",GetOpString(operation));
      return(ERR);
   }

   return(OK);
}


/******************************************************************************************
   Evaluate a MATRIX operating on a COMPLEX MATRIX - result is a COMPLEX matrix
******************************************************************************************/

short MatrixCMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;
   long colsL = dataLeft->GetDimX();
   long rowsL = dataLeft->GetDimY();
   long colsR = dataRight->GetDimX();
   long rowsR = dataRight->GetDimY();
   complex **cmat = dataRight->GetCMatrix2D();
   float **rmat = dataLeft->GetMatrix2D();
   complex **result = 0;
   
// Make sure matrix dimensions are the same if not matrix multiple
   
	if(operation != '*')
	{
	   if(rowsR != rowsL || colsR != colsL)
	   {
	      ErrorMessage("matrix dimensions don't match");
	      return(ERR);
	   }
	   
   // Allocate space for result which will be complex
             
      result = MakeCMatrix2D(colsL,rowsL);
		if(!result) 
		{
		   ErrorMessage("can't allocate memory for matrix - cmatrix operation");
		   return(ERR);
		}      
   }


// Evaluate expression
   
   switch(operation)
	{ 
		case ('+') :
		{ 
			if (result)
			{
				for(j = 0; j < rowsL; j++)
				{
					for(i = 0; i < colsL; i++)
					{
						result[j][i].r = rmat[j][i] + cmat[j][i].r;
						result[j][i].i = cmat[j][i].i;
					}
				}
			}
			break;
		}
		case ('-') :
		{ 
			if (result)
			{
				for(j = 0; j < rowsL; j++)
				{
					for(i = 0; i < colsL; i++)
					{
						result[j][i].r = rmat[j][i] - cmat[j][i].r;
						result[j][i].i = -cmat[j][i].i;
					}
				}
			}
			break;
		}
       case (MATMUL) : // .*
       { 
			if (result)
			{ 
				for(j = 0; j < rowsL; j++)
				{
					for(i = 0; i < colsL; i++)
					{
						result[j][i].r = rmat[j][i]*cmat[j][i].r;
						result[j][i].i = rmat[j][i]*cmat[j][i].i;
					}
				}
			}
			break;
       }
       case ('*') : // Complex matrix multiply
       { 
          if(rowsR != colsL)
          {
             ErrorMessage("inner matrix dimensions don't match for '*' operation");
             return(ERR);
          }
          complex sum;
          long csize = rowsR;
          complex **prod = MakeCMatrix2D(colsR,rowsL);
          if(!prod) 
          {
             ErrorMessage("can't allocate memory for complex matrix product");
             return(ERR);
          }          
          for(j = 0; j < rowsL; j++)
          {
             for(i = 0; i < colsR; i++)
             {
                sum.r = sum.i = 0.0;
                for(long k = 0; k < csize; k++)
                {
                   sum.r += rmat[j][k]*cmat[k][i].r;
                   sum.i += rmat[j][k]*cmat[k][i].i;
                }
                prod[j][i] = sum;
             }
          }
          if(colsR == 1 && rowsL == 1)
          {
             dataLeft->MakeAndSetComplex(prod[0][0]);
             FreeCMatrix2D(prod);
          }
          else
          {
             dataLeft->FreeData();
             dataLeft->AssignCMatrix2D(prod,colsR,rowsL);
          }
		    return(OK);          
          break;
       }       
       case ('/') : 
       { 
			 if (result)
			 {
				float denom; 
				for(j = 0; j < rowsL; j++)
				{
					for(i = 0; i < colsL; i++)
					{
						denom = cmat[j][i].r*cmat[j][i].r + cmat[j][i].i*cmat[j][i].i;
						if(denom == 0)
						{
							FreeCMatrix2D(result);
							ErrorMessage("divide by zero");
							return(ERR);
						}
						result[j][i].r = (rmat[j][i]*cmat[j][i].r)/denom;
						result[j][i].i = (-rmat[j][i]*cmat[j][i].i)/denom;	
					}
             }
          } 
          break;
       }       

       default: 
          ErrorMessage("'%s' is an undefined operation\n           between real and complex matricies",GetOpString(operation));
          return(ERR); 
    }
    dataLeft->AssignCMatrix2D(result,colsL,rowsL);
    return(OK);
}


/******************************************************************************************
   Evaluate a COMPLEX MATRIX operating on a MATRIX - result is a COMPLEX matrix
******************************************************************************************/

short CMatrixMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;
   long colsL = dataLeft->GetDimX();
   long rowsL = dataLeft->GetDimY();
   long colsR = dataRight->GetDimX();
   long rowsR = dataRight->GetDimY();
   complex **cmat = dataLeft->GetCMatrix2D();
   float **rmat = dataRight->GetMatrix2D();


// Make sure matrix dimensions are the same if not matrix multiple
   
	if(operation != '*')
	{
	   if(rowsR != rowsL || colsR != colsL)
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
          for(j = 0; j < rowsL; j++)
          {
             for(i = 0; i < colsL; i++)
             {
                cmat[j][i].r = cmat[j][i].r + rmat[j][i];
             }
          }
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rowsL; j++)
          {
             for(i = 0; i < colsL; i++)
             {
                cmat[j][i].r = cmat[j][i].r - rmat[j][i];
             }
          }
          break;
       }
       case (MATMUL) : // .*
       { 
          for(j = 0; j < rowsL; j++)
          {
             for(i = 0; i < colsL; i++)
             {
                cmat[j][i].r = cmat[j][i].r * rmat[j][i];
                cmat[j][i].i = cmat[j][i].i * rmat[j][i];
             }
          }
          break;
       }
       case ('*') : // Complex matrix multiply
       { 
          if(rowsR != rowsL)
          {
             ErrorMessage("inner matrix dimensions don't match for '*' operation");
             return(ERR);
          }
          complex sum;
          long csize = rowsR;
          complex **prod = MakeCMatrix2D(colsR,rowsL);
          if(!prod) 
          {
             ErrorMessage("can't allocate memory for complex matrix product");
             return(ERR);
          }          
          for(j = 0; j < rowsL; j++)
          {
             for(i = 0; i < colsR; i++)
             {
                sum.r = sum.i = 0.0;
                for(long k = 0; k < csize; k++)
                {
                   sum.r += cmat[j][k].r*rmat[k][i];
                   sum.i += cmat[j][k].i*rmat[k][i];
                }
                prod[j][i] = sum;
             }
          }
          if(colsR == 1 && rowsL == 1)
          {
             dataLeft->MakeAndSetComplex(prod[0][0]);
             FreeCMatrix2D(prod);
          }
          else
          {
             dataLeft->FreeData();
             dataLeft->AssignCMatrix2D(prod,colsR,rowsL);
          }
		    return(OK);           
          break;
       }       
       case ('/') : 
       {
          for(j = 0; j < rowsL; j++)
          {
             for(i = 0; i < colsL; i++)
             {
                if(rmat[j][i] == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                } 
                cmat[j][i].r = cmat[j][i].r/rmat[j][i];
                cmat[j][i].i = cmat[j][i].i/rmat[j][i];
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
   Evaluate a COMPLEX MATRIX operating on a COMPLEX MATRIX - result is a COMPLEX matrix
******************************************************************************************/

short CMatrixCMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;
   complex temp;
   long colsL = dataLeft->GetDimX();
   long rowsL = dataLeft->GetDimY();
   long colsR = dataRight->GetDimX();
   long rowsR = dataRight->GetDimY();
   complex **src = dataRight->GetCMatrix2D();
   complex **dst = dataLeft->GetCMatrix2D();

   if((operation != '*' && operation != MATMUL) && (rowsL != rowsR || colsL != colsR))
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
	       
          for(j = 0; j < rowsL; j++)
             for(i = 0; i < colsL; i++)
	             neq += (FloatNotEqual(dst[j][i].r,src[j][i].r) || FloatNotEqual(dst[j][i].i,src[j][i].i));
	       dataLeft->MakeAndSetFloat(neq == 0);
	       return(OK);
	       break;
	    }	
	    case (NEQ) :
	    {
	       long neq = 0;
	       
          for(j = 0; j < rowsL; j++)
             for(i = 0; i < colsL; i++)
	             neq += (FloatNotEqual(dst[j][i].r,src[j][i].r) || FloatNotEqual(dst[j][i].i,src[j][i].i));
	       dataLeft->MakeAndSetFloat(neq != 0);
	       return(OK);
	       break;
	    }	
	       
      case ('+') :
      { 
         for(j = 0; j < rowsL; j++)
         {
            for(i = 0; i < colsL; i++)
            {
               dst[j][i].r = dst[j][i].r + src[j][i].r;
               dst[j][i].i = dst[j][i].i + src[j][i].i;
            }
         }
         break;
      }
      case ('-') :
      { 
         for(j = 0; j < rowsL; j++)
         {
            for(i = 0; i < colsL; i++)
            {
               dst[j][i].r = dst[j][i].r - src[j][i].r;
               dst[j][i].i = dst[j][i].i - src[j][i].i;
            }
         }
          break;
       }
       case (MATMUL) : // .*
       { 
         if(rowsL == rowsR && colsL == colsR) // Matrices are same size
         {
            for(j = 0; j < rowsL; j++)
            {
               for(i = 0; i < colsL; i++)
               {
                  temp.r = dst[j][i].r*src[j][i].r - dst[j][i].i*src[j][i].i;
                  temp.i = dst[j][i].r*src[j][i].i + dst[j][i].i*src[j][i].r;
                  dst[j][i] = temp;
               }
            }
         }
         else if(rowsL == rowsR && colsR == 1) // Second matrix is a column vector
         {
            for(j = 0; j < rowsR; j++) //y
            {
               for(i = 0; i < colsL; i++) //x
               {
                  temp.r = dst[j][i].r*src[j][0].r - dst[j][i].i*src[j][0].i;
                  temp.i = dst[j][i].r*src[j][0].i + dst[j][i].i*src[j][0].r;
                  dst[j][i] = temp;
               }
            }
         }
         else if(rowsR == 1 && colsL == colsR) // Second matrix is a row vector
         {
            for(j = 0; j < rowsL; j++) //y
            {
               for(i = 0; i < colsR; i++) //x
               {
                  temp.r = dst[j][i].r*src[0][i].r - dst[j][i].i*src[0][i].i;
                  temp.i = dst[j][i].r*src[0][i].i + dst[j][i].i*src[0][i].r;
                  dst[j][i] = temp;
               }
            }
         }
         else
         {
            ErrorMessage("matrix sizes don't agree in '%s' operation",GetOpString(operation));
            return(ERR);
         }
         break;
      }
      case ('*') : // Complex matrix multiply
      { 
          if(rowsR != colsL)
          {
             ErrorMessage("inner matrix dimensions don't match for '*' operation");
             return(ERR);
          }
          complex sum;
          long csize = rowsR;
          complex **prod = MakeCMatrix2D(colsR, rowsL);
          if(!prod) 
          {
             ErrorMessage("can't allocate memory for complex matrix product");
             return(ERR);
          }   
          for(long j = 0; j < rowsL; j++)
          {
             for(long i = 0; i < colsR; i++)
             {
                sum.r = sum.i = 0.0;
                for(long k = 0; k < csize; k++)
                {
                   sum.r += dst[j][k].r*src[k][i].r - dst[j][k].i*src[k][i].i;
                   sum.i += dst[j][k].i*src[k][i].r + dst[j][k].r*src[k][i].i;
                }
                prod[j][i] = sum;
             }
          }
          if(colsR == 1 && rowsL == 1)
          {
             dataLeft->MakeAndSetComplex(prod[0][0]);
             FreeCMatrix2D(prod);
          }
          else
          {
             dataLeft->FreeData();
             dataLeft->AssignCMatrix2D(prod,colsR,rowsL);
          }
		    return(OK);          
          break;
       }       
       case ('/') : 
       { 
          float denom;
          
          for(j = 0; j < rowsL; j++)
          {
             for(i = 0; i < colsL; i++)
             {
                denom = src[j][i].r*src[j][i].r + src[j][i].i*src[j][i].i;
                if(denom == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                }                
                temp.r = (dst[j][i].r*src[j][i].r + dst[j][i].i*src[j][i].i)/denom;
                temp.i = (dst[j][i].i*src[j][i].r - dst[j][i].r*src[j][i].i)/denom;
                dst[j][i] = temp;
             }
          }
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between complex matricies",GetOpString(operation));
          return(ERR); 
    }
    return(OK);
}


/*********************************************************************************
   Evaluate a REAL NUMBER operating on a REAL DMATRIX2D - result is a d-matrix
*********************************************************************************/
 
short FloatDMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;

   double result = (double)dataLeft->GetReal();
   long cols     = dataRight->GetDimX();
   long rows     = dataRight->GetDimY();
   double **mat  = dataRight->GetDMatrix2D();

   switch(operation)
   { 
      case (EQ):
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleEqual(mat[j][i],result);
          break;
       }
       case ('>') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleGreater(mat[j][i],result);
          break;
       }
       case ('<') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleLess(mat[j][i],result);
          break;
       }
       case (GEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleGreaterEqual(mat[j][i],result);
          break;
       }
       case (LEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleLessEqual(mat[j][i],result);
          break;
       }
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result + mat[j][i];
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result - mat[j][i];
          break;
       }
       case ('*') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result * mat[j][i];
          break;
       }
       case ('/') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                if(mat[j][i] == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                }
                mat[j][i] = result / mat[j][i];
             }
          }
          break;
       }
       case ('^') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = pow(result,mat[j][i]);
          break;
       }
       case ('&') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(result) & nhint(mat[j][i]);
          break;
       }
       case ('|') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(result) | nhint(mat[j][i]);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a real number and dmatrix",GetOpString(operation));
          return(ERR); 
    }

    dataLeft->AssignDMatrix2D(mat,cols,rows);
    dataRight->SetNull(); // Data right moved to data left

    return(OK);
}

/*********************************************************************************
   Evaluate a REAL DOUBLE NUMBER operating on a REAL DMATRIX2D - result is a d-matrix
*********************************************************************************/
 
short DoubleDMatrixEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   long i,j;

   double result = dataLeft->GetDouble();
   long cols     = dataRight->GetDimX();
   long rows     = dataRight->GetDimY();
   double **mat  = dataRight->GetDMatrix2D();

   switch(operation)
   { 
       case (EQ):
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleEqual(mat[j][i],result);
          break;
       }
       case ('>') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleGreater(mat[j][i],result);
          break;
       }
       case ('<') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleLess(mat[j][i],result);
          break;
       }
       case (GEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleGreaterEqual(mat[j][i],result);
          break;
       }
       case (LEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleLessEqual(mat[j][i],result);
          break;
       }
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result + mat[j][i];
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result - mat[j][i];
          break;
       }
       case ('*') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = result * mat[j][i];
          break;
       }
       case ('/') :
       { 
          for(j = 0; j < rows; j++)
          {
             for(i = 0; i < cols; i++)
             {
                if(mat[j][i] == 0)
                {
                   ErrorMessage("divide by zero");
                   return(ERR);
                }
                mat[j][i] = result / mat[j][i];
             }
          }
          break;
       }
       case ('^') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = pow(result,mat[j][i]);
          break;
       }
       case ('&') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(result) & nhint(mat[j][i]);
          break;
       }
       case ('|') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(result) | nhint(mat[j][i]);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a real number and dmatrix",GetOpString(operation));
          return(ERR); 
    }

    dataLeft->AssignDMatrix2D(mat,cols,rows);
    dataRight->SetNull(); // Data right moved to data left

    return(OK);
}

/*********************************************************************************
   Evaluate a REAL DMATRIX operating on a REAL NUMBER - result is a REAL DMATRIX
*********************************************************************************/

short DMatrixFloatEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
   long i,j;
   double **mat  = dataLeft->GetDMatrix2D();   
   long cols     = dataLeft->GetDimX();
   long rows     = dataLeft->GetDimY();
   double result  = (double)dataRight->GetReal();
   
   switch(operation)
   { 
       case (EQ):
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleEqual(mat[j][i],result);
          break;
       }
       case ('>') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleGreater(mat[j][i],result);
          break;
       }
       case ('<') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleLess(mat[j][i],result);
          break;
       }
       case (GEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleGreaterEqual(mat[j][i],result);
          break;
       }
       case (LEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleLessEqual(mat[j][i],result);
          break;
       }
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] += result;
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] -= result;
          break;
       }
       case ('*') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] *= result;
          break;
       }
       case ('/') :
       { 
          if(result == 0)
          {
             ErrorMessage("divide by zero");
             return(ERR);
          }
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] /= result;
          break;
       }
       case ('^') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = pow(mat[j][i],result);
          break;
       }
       case ('&') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(mat[j][i]) & nhint(result);
          break;
       }
       case ('|') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(mat[j][i]) | nhint(result);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a dmatrix and real number",GetOpString(operation));
          return(ERR); 
    }
    return(0);
}


/*********************************************************************************
   Evaluate a REAL DMATRIX operating on a REAL NUMBER - result is a REAL DMATRIX
*********************************************************************************/

short DMatrixDoubleEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
   long i,j;
   double **mat  = dataLeft->GetDMatrix2D();   
   long cols     = dataLeft->GetDimX();
   long rows     = dataLeft->GetDimY();
   double result  = dataRight->GetDouble();
   
   switch(operation)
   { 
       case (EQ):
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleEqual(mat[j][i],result);
          break;
       }
       case ('>') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleGreater(mat[j][i],result);
          break;
       }
       case ('<') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleLess(mat[j][i],result);
          break;
       }
       case (GEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleGreaterEqual(mat[j][i],result);
          break;
       }
       case (LEQ) :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = DoubleLessEqual(mat[j][i],result);
          break;
       }
       case ('+') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] += result;
          break;
       }
       case ('-') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] -= result;
          break;
       }
       case ('*') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] *= result;
          break;
       }
       case ('/') :
       { 
          if(result == 0)
          {
             ErrorMessage("divide by zero");
             return(ERR);
          }
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] /= result;
          break;
       }
       case ('^') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = pow(mat[j][i],result);
          break;
       }
       case ('&') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(mat[j][i]) & nhint(result);
          break;
       }
       case ('|') :
       { 
          for(j = 0; j < rows; j++)
             for(i = 0; i < cols; i++)
                mat[j][i] = nhint(mat[j][i]) | nhint(result);
          break;
       }
       default: 
          ErrorMessage("'%s' is an undefined operation\n           between a dmatrix and real number",GetOpString(operation));
          return(ERR); 
    }
    return(0);
}
