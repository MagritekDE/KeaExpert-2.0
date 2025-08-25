#include "stdafx.h"
#include "array.h"
#include "4DFunctions.h"
#include "evaluate_simple.h"
#include "globals.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

short ProcessMatrix(Variable*, Variable*,long,long,long,long*,long*,long*);
short ProcessDMatrix(Variable*, Variable*,long,long,long,long*,long*,long*);
short ProcessComplexMatrix(Variable*, Variable*,long,long,long,long*,long*,long*);
short Process3DMatrix(Variable*, Variable*,long,long,long,long*,long*,long*);
short Process3DComplexMatrix(Variable*, Variable*,long,long,long,long*,long*,long*);
short ProcessList(Variable*, Variable*, long, long*);
short Process2DList(Variable *dstVar, Variable *srcVar, long x, long y, long *xa, long *ya);
short ProcessString(Variable*, Variable*, long, long*);
void FreeIndexLists(long *xa, long *ya, long *za, long *qa);

/*********************************************************************************
*  Assign current values in variable 'srcVar' to an array or elements of an array. 
* 'expression' contains the array or array element string                               
* 'arrayName' is the name of the array and 'start' is the start of the array     
*  index.                                                                            
*********************************************************************************/


short AssignArray(Interface *itfc, char *expression, short start, char *arrayName, Variable *srcVar)
{
   long x,y,z,q;                            // Array indices (fixed)
   long *xa = 0,*ya = 0,*za = 0,*qa=0;      // Array indices (list)
   Variable *dstVar = 0;                    // Array variable referred to by 'arrayName'
   short dstType;                           // Array type of 'arrayName'

   
// Extract array type, variable and index (or indices)
   if(ExtractArrayAddress(itfc, arrayName,expression,x,y,z,q,&xa,&ya,&za,&qa,&dstVar,dstType,start) < 0)
   {
      return(ERR);
   }

	return(AssignArrayCore(itfc, arrayName,expression,x,y,z,q,xa,ya,za,qa,dstVar,dstType,srcVar,start));

}

short AssignArray(Interface *itfc, char *expression, short &start, char *arrayName, Variable *dstVar,  short dstType, Variable *srcVar)
{
   long x,y,z,q;                            // Array indices (fixed)
   long *xa = 0,*ya = 0,*za = 0,*qa=0;      // Array indices (list)

   
// Extract array type, variable and index (or indices)
   if(ExtractArrayAddress(itfc, arrayName,expression,x,y,z,q,&xa,&ya,&za,&qa,&dstVar,dstType,start) < 0)
   {
      return(ERR);
   }

	return(AssignArrayCore(itfc, arrayName,expression,x,y,z,q,xa,ya,za,qa,dstVar,dstType,srcVar,start));
}

short AssignArrayCore(Interface *itfc, char arrayName[], char expression[],
                          long x, long y, long z, long q,
                          long *xa, long *ya, long *za, long *qa,
                          Variable *dstVar, short &dstType, Variable *srcVar, short &start)
{

   switch(dstType)
   {
      case(UNQUOTED_STRING): // Resulting array is a string
	   {
	      if(ProcessString(dstVar, srcVar, x, xa) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
	      break;
	   }

      case(LIST): // Resulting array is a list
	   {
	      if(ProcessList(dstVar, srcVar, x, xa) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
	      break;
	   }

      case(LIST2D): // Resulting array is a 2D list
	   {
	      if(Process2DList(dstVar, srcVar, x, y, xa, ya) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
	      break;
	   }

      case(STRUCTURE_ARRAY):
      {
         extern short ProcessStructArrayAssignment(char* expression, short start, Variable *dstVar, Variable *srcVar, long x, long *xa);
         if(ProcessStructArrayAssignment(expression, start, dstVar, srcVar, x, xa) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
         break;
      } 

   // Assign array element is part/all of a matrix e.g. m[2,3] = 10 or m[~,3] = 10,
   //  m[~,3] = [1:1:5] or m[[1:5]] = [1,2,3,4,5]
      case(MATRIX2D):
	   {
	      if(ProcessMatrix(dstVar, srcVar, x, y, z, xa, ya, za) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
	      break;
	   }
   
   // Assign array element is part/all of a matrix e.g. m[2,3] = 10 or m[~,3] = 10,
   //  m[~,3] = [1:1:5] or m[[1:5]] = [1,2,3,4,5]
      case(DMATRIX2D):
	   {
	      if(ProcessDMatrix(dstVar, srcVar, x, y, z, xa, ya, za) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
	      break;
	   }
   

   // Assign array element is part/all of a matrix e.g. m[2,3] = 10 or m[~,3] = 10
   // or m[~,3] = [1:1:5]
      case(CMATRIX2D):
	   {
	      if(ProcessComplexMatrix(dstVar, srcVar, x, y, z, xa, ya, za) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
	      break;
	   }   
  

   // Assign array element is part/all of a matrix e.g. m[2,3,5] = 10 or m[~,3,5] = [1,2,3]
   // or m[~,~,1] = [1,2;3,4] or m[~,~,~] = 2
      case(MATRIX3D):
	   {
	      if(Process3DMatrix(dstVar, srcVar, x, y, z, xa, ya, za) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
	      break;
	   }

   
   // Assign array element is part/all of a matrix e.g. m[2,3,5] = 10 or m[~,3,5] = [1,2,3]
   // or m[~,~,1] = [1,2;3,4] or m[~,~,~] = 2
      case(CMATRIX3D):
      {
         if(Process3DComplexMatrix(dstVar, srcVar, x, y, z, xa, ya, za) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
         break;
      }  

   // Assign array element is part/all of a matrix e.g. m[2,3,5,3] = 10 or m[~,3,5,1] = [1,2,3]
   // or m[~,~,1,2] = [1,2;3,4] or m[~,~,~,~] = 2
      case(MATRIX4D):
	   {
	      if(Process4DMatrixAssignment(dstVar, srcVar, x, y, z, q, xa, ya, za, qa) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
	      break;
	   }

   
   // Assign array element is part/all of a matrix e.g. m[2,3,5,3] = 10 or m[~,3,5,1] = [1,2,3]
   // or m[~,~,1,2] = [1,2;3,4] or m[~,~,~,~] = 2
      case(CMATRIX4D):
      {
         if(Process4DCMatrixAssignment(dstVar, srcVar, x, y, z, q, xa, ya, za, qa) == ERR)
	      {
            FreeIndexLists(xa,ya,za,qa);
	         return(ERR);
	      }
         break;
      } 

   } 

// Delete index lists
   FreeIndexLists(xa,ya,za,qa);
   
   return(OK);
} 

// Free up any memory used by the array index lists

void FreeIndexLists(long *xa, long *ya, long *za, long *qa)
{  
   if(xa) delete [] xa;
   if(ya) delete [] ya;
   if(za) delete [] za;
   if(qa) delete [] qa;
}

/*********************************************************************
*   Assign element in string from another string                     *
*********************************************************************/

short ProcessString(Variable *dstVar, Variable *srcVar, long x, long *xa)
{
   long i;
   
   char *srcStr = srcVar->GetString();
   char *dstStr =dstVar->GetString();

   switch(srcVar->GetType()) // Check out value type
   {
      case(UNQUOTED_STRING):
      {
         if(x >= 0)
         {
            dstStr[x] = srcStr[0];
         }
         else
         {
            if(strlen(srcStr) != -x)
            {
               ErrorMessage("source string has a different size from destination index range");
               return(ERR);
            }
            for(i = 0; i < -x; i++)
            {
               dstStr[xa[i]] = srcStr[i];
            }
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

/*********************************************************************
*       Assign element in list from a string or from another list    *
*********************************************************************/

short ProcessList(Variable *dstVar, Variable *srcVar, long x, long *xa)
{
   long i;

   
   switch(srcVar->GetType()) // Check out value type
   {
      case(UNQUOTED_STRING):
      {
         char *srcStr = srcVar->GetString();

         if(x < 0) // a[~] = "string"
         {
            for(i = 0; i < -x; i++)
               dstVar->ReplaceListItem(srcStr,xa[i]);
         }
         else
         {
            dstVar->ReplaceListItem(srcStr,x);
         }
         break;
      }
      case(LIST):
      {
         long srcWidth = srcVar->GetDimX();
         char **srcList = srcVar->GetList();
         
         if(x < 0) // a[~] = ??
         {
            if(srcWidth == -x) // a[[2:4]] = b (where b is smaller or equal to a)
            {
               for(i = 0; i < -x; i++)
                  dstVar->ReplaceListItem(srcList[i],xa[i]);
            }
	         else // a[[2:4]] = b (where b is greater or equal to [2:4])
	         {
	            ErrorMessage("source string has a different size from destination index range");
	            return(ERR);
	         }  
	      }           
         else // a[x] = ??
         {
            dstVar->ReplaceListItem(srcVar->GetList()[0],x);     
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


/*********************************************************************
*    Assign element in 2D list from a string or from another list    *
*********************************************************************/

short Process2DList(Variable *dstVar, Variable *srcVar, long x, long y, long *xa, long*ya)
{
   long i;
   
   switch(srcVar->GetType()) // Check out value type
   {
      case(UNQUOTED_STRING):
      {
         char *srcStr = srcVar->GetString();

         if(x < 0) // a[~,j] = "string"
         {
            for(i = 0; i < -x; i++)
               dstVar->Replace2DListItem(srcStr,xa[i],y);
         }
         else if(y < 0) // a[i,~] = "string"
         {
            for(i = 0; i < -x; i++)
               dstVar->Replace2DListItem(srcStr,x,ya[i]);
         }
         else // a[i,j] = "string"
         {
            dstVar->Replace2DListItem(srcStr,x,y);
         }
         break;
      }
      case(LIST):
      {
         long srcWidth = srcVar->GetDimX();
         char **srcList = srcVar->GetList();
         
         if(x < 0)
         {
            if(srcWidth == -x) // a[[2:4],y] = b (where b is smaller or equal to a)
            {
               for(i = 0; i < -x; i++)
                  dstVar->Replace2DListItem(srcList[i],xa[i],y);
            }
	         else // a[[2:4]] = b (e.g. where b is greater or equal to [2:4])
	         {
	            ErrorMessage("source string has a different size from destination index range");
	            return(ERR);
	         }  
	      }           
         else 
         {
            if(srcWidth == -y) // a[x,[2:4]] = b (where b is smaller or equal to a)
            {
               for(i = 0; i < -y; i++)
                  dstVar->Replace2DListItem(srcList[i],x,ya[i]);
            }
	         else // a[[2:4]] = b (e.g. where b is greater or equal to [2:4])
	         {
	            ErrorMessage("source string has a different size from destination index range");
	            return(ERR);
	         } 
			}         
         break; 
      } 
      case(LIST2D):
      {
         long srcXSize = srcVar->GetDimX();
         long srcYSize = srcVar->GetDimY();
         List2DData *srcList = srcVar->GetList2D();
         List2DData *dstList = dstVar->GetList2D();

         if(y < 0 && x < 0) // Assign all or part of the source matrix to the destination matrix e.g. m[[2:4],[4:8]] = noise(3,5)
         {
				// 
            if(srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
				// Check for invalid 
            for(int i = 0; i < srcYSize; i++)
				{
					int srcWidth = srcList->rowSz[i];
					int dstWidth = dstList->rowSz[i];
               for(int j = 0; j < srcWidth; j++)
					{
						if(xa[j] >= dstWidth) 
						{
							ErrorMessage("Invalid index %d in row %d of destination 2D list",xa[j],i);
							return(ERR);
						}
					}
				} 
				// Do the replacement
            for(int i = 0; i < srcYSize; i++)
				{
					int srcWidth = srcList->rowSz[i]; 
               for(int j = 0; j < srcWidth; j++)
						 dstVar->Replace2DListItem(srcList->strings[i][j],xa[j],ya[i]);
				}
         }            
         else
         {
            ErrorMessage("can only assign 2D list to a 2D list");
            return(ERR);
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
  Assign 1 or more elements of a real source matrix to a real destination  matrix
**********************************************************************************/

short ProcessMatrix(Variable *dstVar, Variable *srcVar,  // Destination and source matrices
                    long x, long y, long z,              // Array indices - if < 0 then there is an array
                    long *xa, long *ya, long *za)        // Array of indices only present if x,y, or z are -ve.
{
   long i,j;
   float **dstMat = dstVar->GetMatrix2D();
   float **srcMat = srcVar->GetMatrix2D();
   long srcXSize  = srcVar->GetDimX();
   long srcYSize  = srcVar->GetDimY();
   float fnum     = srcVar->GetReal();

// Check out source variable type
   switch(srcVar->GetType()) 
   {
      case(FLOAT64):
         fnum = (float)srcVar->GetDouble();
      case(FLOAT32): // Source is a number i.e. m[23] = 1.234
      case(INTEGER):
      {
         if(x < 0 && y >= 0) // Assign 'fnum' to all or part of row 'y' e.g. m[[1:5],y] = [1,2,3,4,5]
         {
            for(i = 0; i < -x; i++)
            {
               dstMat[y][xa[i]] = fnum;
            }
         }
         else if(y < 0 && x >= 0) // Assign 'fnum' to all or part of column 'x' e.g. m[x,[1:5]] = [1,2,3,4,5]
         {
            for(i = 0; i < -y; i++)
            {
               dstMat[ya[i]][x] = fnum;
            }
         }
         else if(y < 0 && x < 0) // Assign 'fnum' to all or some elements e.g. m[[1:3],[1:5]] = noise(3,4)
         {
            for(j = 0; j < -y; j++)
            {
               for(i = 0; i < -x; i++)
               {
                  dstMat[ya[j]][xa[i]] = fnum;
               }
            }
         }            
         else // Assign 'result' to row y and column x e.g. m[1,2] = 1.234
         {
            dstMat[y][x] = fnum;
         }
         break;
      }
      case(MATRIX2D): // Source is a vector or matrix
      {
         if(x < 0 && y >= 0 && srcYSize == 1) // Assign source vector to row 'y' of destination matrix e.g. m[[4:8],y] = [4:8]
         {
            if(srcXSize != -x)
            {
                ErrorMessage("source vector has a different size from destination index range");
                return(ERR);
            }
            for(i = 0; i < srcXSize; i++)
               dstMat[y][xa[i]] = srcMat[0][i];
         }
         else if(y < 0 && x >= 0  && srcXSize == 1) // Assign source vector to column 'x' of destination matrix e.g. m[y,[4:8]] = [4:8]'
         {
            if(srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(j = 0; j < srcYSize; j++)
               dstMat[ya[j]][x] = srcMat[j][0];
         }
         else if(y < 0 && x < 0) // Assign all or part of the source matrix to the destination matrix e.g. m[[2:4],[4:8]] = noise(3,5)
         {
            if(srcXSize != -x || srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(i = 0; i < srcYSize; i++)
               for(j = 0; j < srcXSize; j++)
                  dstMat[ya[i]][xa[j]] = srcMat[i][j];
         }            
         else
         {
            ErrorMessage("can only assign a vector to a matrix row or column");
            return(ERR);
         }
         break;
      }
      default:
      {
	      ErrorMessage("can only assign a real number or matrix to a real matrix");
         return(ERR);
      }
	}
	return(OK);				      
}


/*********************************************************************************
  Assign 1 or more elements of a real source matrix to a real destination  matrix
**********************************************************************************/

short ProcessDMatrix(Variable *dstVar, Variable *srcVar,  // Destination and source matrices
                    long x, long y, long z,              // Array indices - if < 0 then there is an array
                    long *xa, long *ya, long *za)        // Array of indices only present if x,y, or z are -ve.
{
   long i,j;
   double **dstMat = dstVar->GetDMatrix2D();
   long srcXSize  = srcVar->GetDimX();
   long srcYSize  = srcVar->GetDimY();

// Check out source variable type
   switch(srcVar->GetType()) 
   {
      case(FLOAT64): // Source is a number i.e. m[23] = 1.234
      case(FLOAT32):
      case(INTEGER):
      {
         double fnum;

         if(srcVar->GetType() == FLOAT64)
            fnum = srcVar->GetDouble();
         else
            fnum = (double)srcVar->GetReal();

         if(x < 0 && y >= 0) // Assign 'fnum' to all or part of row 'y' e.g. m[[1:5],y] = [1,2,3,4,5]
         {
            for(i = 0; i < -x; i++)
            {
               dstMat[y][xa[i]] = fnum;
            }
         }
         else if(y < 0 && x >= 0) // Assign 'fnum' to all or part of column 'x' e.g. m[x,[1:5]] = [1,2,3,4,5]
         {
            for(i = 0; i < -y; i++)
            {
               dstMat[ya[i]][x] = fnum;
            }
         }
         else if(y < 0 && x < 0) // Assign 'fnum' to all or some elements e.g. m[[1:3],[1:5]] = noise(3,4)
         {
            for(j = 0; j < -y; j++)
            {
               for(i = 0; i < -x; i++)
               {
                  dstMat[ya[j]][xa[i]] = fnum;
               }
            }
         }            
         else // Assign 'result' to row y and column x e.g. m[1,2] = 1.234
         {
            dstMat[y][x] = fnum;
         }
         break;
      }
      case(MATRIX2D): // Source is a vector or matrix
      {
         float **srcMat = srcVar->GetMatrix2D();

         if(x < 0 && y >= 0 && srcYSize == 1) // Assign source vector to row 'y' of destination matrix e.g. m[[4:8],y] = [4:8]
         {
            if(srcXSize != -x)
            {
                ErrorMessage("source vector has a different size from destination index range");
                return(ERR);
            }
            for(i = 0; i < srcXSize; i++)
               dstMat[y][xa[i]] = (double)srcMat[0][i];
         }
         else if(y < 0 && x >= 0  && srcXSize == 1) // Assign source vector to column 'x' of destination matrix e.g. m[y,[4:8]] = [4:8]'
         {
            if(srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(j = 0; j < srcYSize; j++)
               dstMat[ya[j]][x] = (double)srcMat[j][0];
         }
         else if(y < 0 && x < 0) // Assign all or part of the source matrix to the destination matrix e.g. m[[2:4],[4:8]] = noise(3,5)
         {
            if(srcXSize != -x || srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(i = 0; i < srcYSize; i++)
               for(j = 0; j < srcXSize; j++)
                  dstMat[ya[i]][xa[j]] = (double)srcMat[i][j];
         }            
         else
         {
            ErrorMessage("can only assign a vector to a matrix row or column");
            return(ERR);
         }
         break;
      }
      case(DMATRIX2D): // Source is a vector or matrix
      {
         double **srcMat = srcVar->GetDMatrix2D();

         if(x < 0 && y >= 0 && srcYSize == 1) // Assign source vector to row 'y' of destination matrix e.g. m[[4:8],y] = [4:8]
         {
            if(srcXSize != -x)
            {
                ErrorMessage("source vector has a different size from destination index range");
                return(ERR);
            }
            for(i = 0; i < srcXSize; i++)
               dstMat[y][xa[i]] = srcMat[0][i];
         }
         else if(y < 0 && x >= 0  && srcXSize == 1) // Assign source vector to column 'x' of destination matrix e.g. m[y,[4:8]] = [4:8]'
         {
            if(srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(j = 0; j < srcYSize; j++)
               dstMat[ya[j]][x] = srcMat[j][0];
         }
         else if(y < 0 && x < 0) // Assign all or part of the source matrix to the destination matrix e.g. m[[2:4],[4:8]] = noise(3,5)
         {
            if(srcXSize != -x || srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(i = 0; i < srcYSize; i++)
               for(j = 0; j < srcXSize; j++)
                  dstMat[ya[i]][xa[j]] = srcMat[i][j];
         }            
         else
         {
            ErrorMessage("can only assign a vector to a matrix row or column");
            return(ERR);
         }
         break;
      }
      default:
      {
	      ErrorMessage("can only assign a real number or matrix to a real matrix");
         return(ERR);
      }
	}
	return(OK);				      
}


/*********************************************************************************
       Assign a number or elements of a matrix to a complex matrix
**********************************************************************************/

short ProcessComplexMatrix(Variable *dstVar, Variable *srcVar, // Source and destination variables
                           long x,   long y,   long z,         // Individual indices - if -ve then this is the index list size
                           long *xa, long *ya, long *za)       // Index arrays or lists
{
   long i,j;
   complex **dstMat = dstVar->GetCMatrix2D();
   complex **srcMat = srcVar->GetCMatrix2D();
   float **srcMatr  = srcVar->GetMatrix2D();
   long srcXSize    = srcVar->GetDimX();
   long srcYSize    = srcVar->GetDimY();
   float fnum       = srcVar->GetReal();
   complex cnum     = srcVar->GetComplex();
	
	switch(srcVar->GetType()) // Check out value type
	{
	   case(FLOAT32):
	   case(INTEGER):
	   {
         if(x < 0 && y >= 0) // Assign 'fnum' to all or part of row 'y' e.g. m[[1:5],y] = [1,2,3,4,5]
         {
	         for(i = 0; i < -x; i++)
            {
               dstMat[y][xa[i]].r = fnum; 
	            dstMat[y][xa[i]].i = 0; 
            }
	      }
         else if(y < 0 && x >= 0) // Assign 'fnum' to all or part of column 'x' e.g. m[x,[1:5]] = [1,2,3,4,5]
	      {
	         for(j = 0; j < -y; j++)
            {
	            dstMat[ya[j]][x].r = fnum; 
	            dstMat[ya[j]][x].i = 0; 
            }
	      }
         else if(y < 0 && x < 0) // Assign 'fnum' to all or some elements e.g. m[[1:3],[1:5]] = noise(3,4)
	      {
            for(j = 0; j < -y; j++)
            {
               for(i = 0; i < -x; i++)
               {
                  dstMat[ya[j]][xa[i]].r = fnum;
                  dstMat[ya[j]][xa[i]].i = 0.0;
               }
            }
	      }            
	      else // Assign 'result' to row y and column x 
	      {
	         dstMat[y][x].r = fnum;
	         dstMat[y][x].i = 0;
	      }
	      break;
	   }
	   case(COMPLEX):
	   {
	      if(x < 0 && y >= 0) // Assign 'result' to row 'y'
	      {
	         for(i = 0; i < -x; i++)
	           dstMat[y][xa[i]] = cnum; 
	      }
	      else if(y < 0 && x >= 0) // Assign 'result' to column 'x'
	      {
	         for(j = 0; j < -y; j++)
	           dstMat[ya[j]][x] = cnum; 
	      }
	      else if(y < 0 && x < 0) // Assign 'cnum' to some or all of the elements
	      {
	         for(j = 0; j < -y; j++)
	            for(i = 0; i < -x; i++)
	              dstMat[ya[j]][xa[i]] = cnum; 
	      }            
	      else // Assign 'result' to row y and column x 
	      {
	         dstMat[y][x] = cnum;
	      }
	      break;
	   }   
	   case(MATRIX2D):
	   {
         if(x < 0 && y >= 0 && srcYSize == 1) // Assign source vector to row 'y' of destination matrix e.g. m[[4:8],y] = [4:8]
	      {
            if(srcXSize != -x)
            {
                ErrorMessage("source vector has a different size from destination index range");
                return(ERR);
            }
            for(i = 0; i < srcXSize; i++)
            {
               dstMat[y][xa[i]].r = srcMatr[0][i];
               dstMat[y][xa[i]].i = 0;
            }
	      }
         else if(y < 0 && x >= 0  && srcXSize == 1) // Assign source vector to column 'x' of destination matrix e.g. m[y,[4:8]] = [4:8]'
	      {
            if(srcYSize != -y)
            {
                ErrorMessage("source vector has a different size from destination index range");
                return(ERR);
            }
            for(j = 0; j < srcYSize; j++)
            {
               dstMat[ya[j]][x].r = srcMatr[j][0];
               dstMat[ya[j]][x].i = 0;
            }
         }
         else if(y < 0 && x < 0) // Assign all or part of the source matrix to the destination matrix e.g. m[[2:4],[4:8]] = noise(3,5)
         {
            if(srcXSize != -x || srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(j = 0; j < srcYSize; j++)
            {
               for(i = 0; i < srcXSize; i++)
               {
                  dstMat[ya[j]][xa[i]].r = srcMatr[j][i];
                  dstMat[ya[j]][xa[i]].i = 0;
               }
            }
         }            
         else
         {
            ErrorMessage("can only assign a vector to a matrix row or column");
            return(ERR);
         }
         break;
      }
	   case(CMATRIX2D):
	   {
         if(x < 0 && y >= 0 && srcYSize == 1) // Assign source vector to row 'y' of destination matrix e.g. m[[4:8],y] = [4:8]
	      {
            if(srcXSize != -x)
            {
                ErrorMessage("source vector has a different size from destination index range");
                return(ERR);
            }
            for(i = 0; i < srcXSize; i++)
               dstMat[y][xa[i]] = srcMat[0][i];
	      }
         else if(y < 0 && x >= 0  && srcXSize == 1) // Assign source vector to column 'x' of destination matrix e.g. m[y,[4:8]] = [4:8]'
	      {
            if(srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(j = 0; j < srcYSize; j++)
            {
               dstMat[ya[j]][x] = srcMat[j][0];
            }
         }
         else if(y < 0 && x < 0) // Assign all or part of the source matrix to the destination matrix e.g. m[[2:4],[4:8]] = noise(3,5)
         {
            if(srcXSize != -x || srcYSize != -y)
            {
               ErrorMessage("source vector has a different size from destination index range");
               return(ERR);
            }
            for(j = 0; j < srcYSize; j++)
               for(i = 0; i < srcXSize; i++)
                  dstMat[ya[j]][xa[i]] = srcMat[j][i];
         }            
         else
         {
            ErrorMessage("can only assign a vector to a matrix row or column");
            return(ERR);
         }
         break;
      }
	   default:
      {
	      ErrorMessage("can only assign a complex number or matrix to a complex matrix");
         return(ERR);
      }
	}
	return(OK);
}	

/*********************************************************************************
    Assign values to a point, a row or column, or a plane in a 3D real matrix
*********************************************************************************/

short Process3DMatrix(Variable *dstVar, Variable *srcVar, 
                      long x,   long y,   long z,
                      long *xa, long *ya, long *za)
{
   long i,j,k;
	float ***dstMat   = dstVar->GetMatrix3D();
	float **srcMat    = srcVar->GetMatrix2D();
	long dstZSize     = dstVar->GetDimZ();
	long srcXSize     = srcVar->GetDimX();
	long srcYSize     = srcVar->GetDimY();
   float fnum        = srcVar->GetReal();
	
	switch(srcVar->GetType()) // Check out value type
	{
	   case(FLOAT32):
	   case(INTEGER):
	   {
	      if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,3,5] = 5
	      {
	         for(i = 0; i < -x; i++)
	           dstMat[z][y][xa[i]] = fnum; 
	      }
	      else if(x >= 0 && y < 0 && z >= 0) // e.g. m[1,~,5] = 5
	      {
	         for(j = 0; j < -y; j++)
	           dstMat[z][ya[j]][x] = fnum; 
	      }
	      else if(x >= 0 && y >= 0 && z < 0) // e.g. m[1,3,~] = 5
	      {
	         for(k = 0; k < -z; k++)
	           dstMat[za[k]][y][x] = fnum; 
	      } 
	      else if(x >= 0 && y < 0 && z < 0) // e.g. m[1,~,~] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
	            for(j = 0; j < -y; j++)
	                dstMat[za[k]][ya[j]][x] = fnum; 
	      }   
	      else if(x < 0 && y >= 0 && z < 0) // e.g. m[~,3,~] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
	            for(i = 0; i < -x; i++)
	                dstMat[za[k]][y][xa[i]] = fnum; 
	      } 
	      else if(x < 0 && y < 0 && z >= 0) // e.g. m[~,~,5] = 5
	      {                                     
	         for(j = 0; j < -y; j++)
	            for(i = 0; i < -x; i++)
	                dstMat[z][ya[j]][xa[i]] = fnum; 
	      }                                             
	      else if(x < 0 && y < 0 && z < 0) // e.g. m[~,~,~] = 5
	      {
	         for(k = 0; k < -z; k++)
	            for(j = 0; j < -y; j++)
	               for(i = 0; i < -x; i++)
	                   dstMat[za[k]][ya[j]][xa[i]] = fnum; 
	      }            
	      else // e.g. m[2,3,5] = 5
	      {
	         dstMat[z][y][x] = fnum;
	      }
	      break;
	   }

      case(COMPLEX):
      {
         ErrorMessage("Can't assign a complex number to a real matrix");
         return(ERR);
      }

	   case(MATRIX2D):
	   {
	      if(srcXSize == 1) // Column vector
	      {
				if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,1,0] = [1,2]'
				{
               if(srcYSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[z][y][xa[i]] = srcMat[i][0];
				}
				else if(x >= 0 && y < 0 && z >= 0) // e.g. m[1,~,0] = [1,2]'
				{
              if(srcYSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[z][ya[j]][x] = srcMat[j][0];
				}                 
				else if(x >= 0 && y >= 0 && z < 0) // e.g. m[1,1,~] = [1,2]'
				{
               if(srcYSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[za[k]][y][x] = srcMat[k][0];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else if(srcYSize == 1) // Row vector
	      {
				if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,1,0] = [1,2]
				{
               if(srcXSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[z][y][xa[i]] = srcMat[0][i];
				}
				else if(x >= 0 && y < 0 && z >= 0) // e.g. m[1,~,0] = [1,2]
				{
               if(srcXSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[z][ya[j]][x] = srcMat[0][j];
				}                 
				else if(x >= 0 && y >= 0 && z < 0 && srcXSize == dstZSize) // e.g. m[1,1,~] = [1,2]
				{
               if(srcXSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[za[k]][y][x] = srcMat[0][k];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else // Plane matrix
	      {
				if(x < 0 && y < 0 && z >= 0)
				{
               if(srcXSize != -x || srcYSize != -y)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(j = 0; j < -y; j++)     // e.g. m[~,~,1] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[z][ya[j]][xa[i]] = srcMat[j][i];
				} 
				else if(x >= 0 && y < 0 && z < 0)
				{
               if(srcXSize != -y || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)     // e.g. m[0,~,~] = [1,2;3,4]
			 	      for(j = 0; j < -y; j++)
				         dstMat[za[k]][ya[j]][x] = srcMat[k][j];
				}                  
				else if(x < 0 && y >= 0 && z < 0)
				{
               if(srcXSize != -x || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)    // e.g. m[~,1,~] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[za[k]][y][xa[i]] = srcMat[k][i];
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
         ErrorMessage("Can't assign a complex matrix to a real matrix");
         return(ERR);
      }

      default:
      {
         ErrorMessage("unsupported data type for matrix assignment");
         return(ERR);
      } 
	}
	return(OK);
}

/*********************************************************************************
    Assign values to a point, a row or column, or a plane in a 3D complex matrix
*********************************************************************************/

short Process3DComplexMatrix(Variable *dstVar, Variable *srcVar,
                             long x, long y, long z,
                             long *xa, long *ya, long *za)
{
   long i,j,k;
	complex ***dstMat = dstVar->GetCMatrix3D();
	complex **srcMat  = srcVar->GetCMatrix2D();
	long srcXSize     = srcVar->GetDimX();
	long srcYSize     = srcVar->GetDimY();
   float fnum        = srcVar->GetReal();
   complex cnum      = srcVar->GetComplex();
	
	switch(srcVar->GetType()) // Check out value type
	{
	   case(FLOAT32):
	   case(INTEGER):
	   {
	      if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,3,5] = 5
	      {
	         for(i = 0; i < -x; i++)
            {
	            dstMat[z][y][xa[i]].r = fnum; 
	            dstMat[z][y][xa[i]].i = 0; 
            }
	      }
	      else if(x >= 0 && y < 0 && z >= 0) // e.g. m[1,~,5] = 5
	      {
	         for(j = 0; j < -y; j++)
            {
	            dstMat[z][ya[j]][x].r = fnum; 
	            dstMat[z][ya[j]][x].i = 0; 
            }
	      }
	      else if(x >= 0 && y >= 0 && z < 0) // e.g. m[1,3,~] = 5
	      {
	         for(k = 0; k < -z; k++)
            {
	            dstMat[za[k]][y][x].r = fnum; 
	            dstMat[za[k]][y][x].i = 0; 
            }
	      } 
	      else if(x >= 0 && y < 0 && z < 0) // e.g. m[1,~,~] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
            {
	            for(j = 0; j < -y; j++)
               {
	                dstMat[za[k]][ya[j]][x].r = fnum; 
	                dstMat[za[k]][ya[j]][x].i = 0; 
               }
            }
	      }   
	      else if(x < 0 && y >= 0 && z < 0) // e.g. m[~,3,~] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
            {
	            for(i = 0; i < -x; i++)
               {
	                dstMat[za[k]][y][xa[i]].r = fnum; 
	                dstMat[za[k]][y][xa[i]].i = 0; 
               }
            }
	      } 
	      else if(x < 0 && y < 0 && z >= 0) // e.g. m[~,~,5] = 5
	      {                                     
	         for(j = 0; j < -y; j++)
            {
	            for(i = 0; i < -x; i++)
               {
	                dstMat[z][ya[j]][xa[i]].r = fnum; 
	                dstMat[z][ya[j]][xa[i]].i = 0; 
               }
            }
	      }                                             
	      else if(x < 0 && y < 0 && z < 0) // e.g. m[~,~,~] = 5
	      {
	         for(k = 0; k < -z; k++)
            {
	            for(j = 0; j < -y; j++)
               {
	               for(i = 0; i < -x; i++)
                  {
	                   dstMat[za[k]][ya[j]][xa[i]].r = fnum; 
	                   dstMat[za[k]][ya[j]][xa[i]].i = 0; 
                  }
               }
            }
	      }            
	      else // e.g. m[2,3,5] = 5
	      {
	         dstMat[z][y][x].r = fnum;
	         dstMat[z][y][x].i = 0;
	      }
	      break;
	   }


	   case(COMPLEX):
	   {
	      if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,3,5] = 5
	      {
	         for(i = 0; i < -x; i++)
	           dstMat[z][y][xa[i]] = cnum; 
	      }
	      else if(x >= 0 && y < 0 && z >= 0) // e.g. m[1,~,5] = 5
	      {
	         for(j = 0; j < -y; j++)
	           dstMat[z][ya[j]][x] = cnum; 
	      }
	      else if(x >= 0 && y >= 0 && z < 0) // e.g. m[1,3,~] = 5
	      {
	         for(k = 0; k < -z; k++)
	           dstMat[za[k]][y][x] = cnum; 
	      } 
	      else if(x >= 0 && y < 0 && z < 0) // e.g. m[1,~,~] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
	            for(j = 0; j < -y; j++)
	                dstMat[za[k]][ya[j]][x] = cnum; 
	      }   
	      else if(x < 0 && y >= 0 && z < 0) // e.g. m[~,3,~] = 5
	      {                                     
	         for(k = 0; k < -z; k++)
	            for(i = 0; i < -x; i++)
	                dstMat[za[k]][y][xa[i]] = cnum; 
	      } 
	      else if(x < 0 && y < 0 && z >= 0) // e.g. m[~,~,5] = 5
	      {                                     
	         for(j = 0; j < -y; j++)
	            for(i = 0; i < -x; i++)
	                dstMat[z][ya[j]][xa[i]] = cnum; 
	      }                                             
	      else if(x < 0 && y < 0 && z < 0) // e.g. m[~,~,~] = 5
	      {
	         for(k = 0; k < -z; k++)
	            for(j = 0; j < -y; j++)
	               for(i = 0; i < -x; i++)
	                   dstMat[za[k]][ya[j]][xa[i]] = cnum; 
	      }            
	      else // e.g. m[2,3,5] = 5
	      {
	         dstMat[z][y][x] = cnum;
	      }
	      break;
	   }         
	   case(MATRIX2D):
	   {
         float **srcMat  = srcVar->GetMatrix2D();
	   
	      if(srcXSize == 1) // Column vector
	      {
				if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,1,0] = [1,2]'
				{
               if(srcYSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[z][y][xa[i]].r = srcMat[i][0];
				}
				else if(x >= 0 && y < 0 && z >= 0) // e.g. m[1,~,0] = [1,2]'
				{
               if(srcYSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[z][ya[j]][x].r = srcMat[j][0];
				}                 
				else if(x >= 0 && y >= 0 && z < 0) // e.g. m[1,1,~] = [1,2]'
				{
               if(srcYSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[za[k]][y][x].r = srcMat[k][0];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else if(srcYSize == 1) // Row vector
	      {
				if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,1,0] = [1,2]
				{
               if(srcXSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[z][y][xa[i]].r = srcMat[0][i];
				}
				else if(x >= 0 && y < 0 && z >= 0) // e.g. m[1,~,0] = [1,2]
				{
               if(srcXSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				      dstMat[z][ya[j]][x].r = srcMat[0][j];
				}                 
				else if(x >= 0 && y >= 0 && z < 0) // e.g. m[1,1,~] = [1,2]
				{
               if(srcXSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				      dstMat[za[k]][y][x].r = srcMat[0][k];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else // Plane matrix
	      {
				if(x < 0 && y < 0 && z >= 0)
				{
               if(srcXSize != -x || srcYSize != -y)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(j = 0; j < -y; j++)     // e.g. m[~,~,1] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[z][ya[j]][xa[i]].r = srcMat[j][i];
				} 
				else if(x >= 0 && y < 0 && z < 0)
				{
               if(srcXSize != -y || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)     // e.g. m[0,~,~] = [1,2;3,4]
			 	      for(j = 0; j < -y; j++)
				         dstMat[za[k]][ya[j]][x].r = srcMat[k][j];
				}                  
				else if(x < 0 && y >= 0 && z < 0)
				{
               if(srcXSize != -x || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)    // e.g. m[~,1,~] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[za[k]][y][xa[i]].r = srcMat[k][i];
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
	      if(srcXSize == 1) // Column vector
	      {
				if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,1,0] = [1,2]
				{
               if(srcYSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[z][y][xa[i]] = srcMat[i][0];
				}
				else if(x >= 0 && y < 0 && z >= 0) // e.g. m[1,~,0] = [1,2]
				{
               if(srcYSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				     dstMat[z][ya[j]][x] = srcMat[j][0];
				}                 
				else if(x >= 0 && y >= 0 && z < 0) // e.g. m[1,1,~] = [1,2]
				{
               if(srcYSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				     dstMat[za[k]][y][x] = srcMat[k][0];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else if(srcYSize == 1) // Row vector
	      {
				if(x < 0 && y >= 0 && z >= 0) // e.g. m[~,1,0] = [1,2]
				{
               if(srcXSize != -x)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
			 	   for(i = 0; i < -x; i++)
				      dstMat[z][y][xa[i]] = srcMat[0][i];
				}
				else if(x >= 0 && y < 0 && z >= 0 ) // e.g. m[1,~,0] = [1,2]
				{
               if(srcXSize != -y)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(j = 0; j < -y; j++)
				     dstMat[z][ya[j]][x] = srcMat[0][j];
				}                 
				else if(x >= 0 && y >= 0 && z < 0) // e.g. m[1,1,~] = [1,2]
				{
               if(srcXSize != -z)
               {
                   ErrorMessage("source vector has a different size from destination index range");
                   return(ERR);
               }
				   for(k = 0; k < -z; k++)
				     dstMat[za[k]][y][x] = srcMat[0][k];
				} 
	         else
	         {
	            ErrorMessage("source vector wrong size");
	            return(ERR);
	         } 
	      }
	      else // Plane matrix
	      {
				if(x < 0 && y < 0 && z >= 0)
				{
               if(srcXSize != -x || srcYSize != -y)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(j = 0; j < -y; j++)     // e.g. m[~,~,1] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[z][ya[j]][xa[i]] = srcMat[j][i];
				} 
				else if(x >= 0 && y < 0 && z < 0)
				{
               if(srcXSize != -y || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)     // e.g. m[0,~,~] = [1,2;3,4]
			 	      for(j = 0; j < -y; j++)
				         dstMat[za[k]][ya[j]][x] = srcMat[k][j];
				}                  
				else if(x < 0 && y >= 0 && z < 0)
				{
               if(srcXSize != -x || srcYSize != -z)
               {
                   ErrorMessage("source matrix has a different size from destination index range");
                   return(ERR);
               }
			 	   for(k = 0; k < -z; k++)    // e.g. m[~,1,~] = [1,2;3,4]
			 	      for(i = 0; i < -x; i++)
				         dstMat[za[k]][y][xa[i]] = srcMat[k][i];
				}
	         else
	         {
	            ErrorMessage("source matrix wrong size");
	            return(ERR);
	         } 
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