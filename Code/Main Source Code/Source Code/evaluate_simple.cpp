#include "stdafx.h"
#include "evaluate_simple.h"
#include "allocate.h"
#include "cArg.h"
#include "command_other.h"
#include "evaluate.h"
#include "evaluate_complex.h"
#include "globals.h"
#include "interface.h"
#include "list_functions.h"
#include "mymath.h"
#include "scanstrings.h"
#include <string.h>
#include "structure.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include <assert.h>
#include "memoryLeak.h"

#define RANGE     0
#define SPECIFIED 1

// Experimental code to get new thread-safe evaluate routines working
short EvaluateRealMatrix(Interface *itfc, char *operand, short dataType, Variable *result);
short EvaluateDoubleMatrix(Interface *itfc, char *operand, short dataType, Variable *result);

short EvaluateStringArray(Interface *itfc, char *operand, short dataType, Variable *result);
short EvaluateUnquotedStringArray(Interface *itfc, char *operand, short dataType, Variable *result);
short EvaluateComplexMatrix(Interface *itfc, char *operand, short dataType, Variable *result);

short CheckMatrixRow(Interface *itfc, char *row, CArg &cRow, long &nrMatrixCols, float &start, float &step, float &end, short &rowType);
short CheckMatrixRow(Interface *itfc, char *row, CArg &cRow, long &nrMatrixCols, double &start, double &step, double &end, short &rowType);
short EvaluateArrayIndexStr(Interface *itfc, char *indexStr, long maxDim, Variable *result);

bool ProcessIndex(Interface *itfc, short whichDim, long maxDim, CArg *innerArg, long &outIndex, long **outList);

#pragma warning (disable: 4996) // Ignore deprecated library functions

short Evaluate(Interface *itfc,short mode ,char *expression, Variable *result)
{
   short r;

   r = CheckForSimpleEval(expression);

   if(r == 1)
      r = EvaluateSimpleExpression(itfc,mode,expression,result);
   else if(r == 0)
      r = EvaluateComplexExpression(itfc,expression,result);

   return(r);
}

short Evaluate(void *par, short mode ,char *expression, Variable *result)
{
   short r;
   Interface *itfc = (Interface*)par;

   r = CheckForSimpleEval(expression);

   if(r == 1)
      r = EvaluateSimpleExpression(itfc,mode,expression,result);
   else if(r == 0)
      r = EvaluateComplexExpression(itfc,expression,result);

   return(r);
}

/**********************************************************************
  Take 'expression' and evaluate it as if it was a mathematical       
  string. The result is returned in variable "result" while the          
  data type is returned directly.            

  Note a 'simple expression' should only contain a single operand 
  and no operators.
**********************************************************************/


short EvaluateSimpleExpression(Interface *itfc, short mode ,char *expression, Variable *result)
{
   short resultType,retType;

   if(itfc)
      itfc->nrRetValues = 1; // Normally only 1 result is returned from evaluate
   
// Do a quick check to see if we are just evaluating a variable
	Variable *var;
	if((var = GetVariable(itfc,ALL_VAR|DO_NOT_RESOLVE,expression,resultType)) != NULL)
   {  
//	   if(var == ansVar) // Don't bother copying if var already equals ansVar
//	      return(ansVar->GetType());

	   switch(mode)
	   {
		   case(FULL_COPY): // Copy the variable to ansvar
		   {
		      if(CopyVariable(result,var,FULL_COPY) == ERR)
               return(ERR);
		      return(resultType);
	      }
	      case(RESPECT_ALIAS): // Just copy as an alias
	      {		      
		      if(CopyVariable(result,var,RESPECT_ALIAS) == ERR)
               return(ERR);
		      return(resultType);
	      }	
	   }      	 
   }
   
// Checking for strings which are too short or too long
   if(strlen(expression) == 0)
   {
      ErrorMessage("empty expression");
      return(ERR);
   }

// Work our way through 'expression' evaluating as we go.
// i points to the character we are currently interpreting
   short i = 0;   
          
// Skip any white space
    SkipWhiteSpace(expression,i);

// Evaluate bracketed expression e.g. (2+4/5) 
    if(expression[i] == '(') 
    {
       retType = EvaluateBracketedExpression(itfc,expression,i,result);
    }
    
// Extract next operand e.g. '-12.3' or 'v1'   
    else  
    {
       short datType;
       CText operand;

      // Get next operand e.g. 3, 2.12, var, v1 ...
       datType = GetNextOperandCT(i,expression,operand);

       if(datType == ERR)
       {
          return(ERR);
       }

     // Check and evaluate array operand here e.g. v1[23] or m1[1,2] or m1[1,~]
       if(expression[i] == '[')
       {
          retType = EvaluateArrayOperand(itfc, operand.Str(),expression,i,result); 
	    }  
            		            
     // Check and evaluate function here e.g. log(y)     
	    else if(expression[i] == '(') 
	    {
          retType = EvaluateFunctionOperand(itfc, operand.Str(),expression,i,result); 
       }
       
     // No brackets so must be a simple operand e.g. 12.3, "Hi", var       
       else 
       {
          retType = EvaluateSimpleOperand(itfc, operand.Str(),datType,result);
       }
    }

    return(retType);
} 

/************************************************************************
     Evaluate the contents of a bracket, returning in 'result'.   
************************************************************************/

short EvaluateBracketedExpression(Interface *itfc, char *expression, short &i, Variable *result)
{   
   char *innerStr = new char[strlen(expression)+1];
   short dataType;
   
// Extract the expression enclosed by round brackets '()'. Place in 'innerStr'
       
	if(ExtractSubExpression(expression,innerStr,i,'(',')') < 0)
	{
		delete [] innerStr;
	   return(ERR);
	}

// Evaluate the expression 'innerStr' and return in answer variable

	if((dataType = Evaluate(itfc, RESPECT_ALIAS,innerStr,result)) == -1)
	{
		delete [] innerStr;
	   return(ERR);
	}

	delete [] innerStr;
   return(dataType);
}


/********************************************************************
  If the current part of the expression is a function call this 
  routine calls the function. Any results are returned.   
********************************************************************/

short EvaluateFunctionOperand(Interface *itfc, char *function, char *expression, short &i, Variable *result)
{
   char *arguments;
   int r = CMD_NOT_FOUND;

   arguments = new char[strlen(expression)+1];
   
// Extract the expression enclosed by round brackets '()'. Place in 'arguments'
	if(ExtractSubExpression(expression,arguments,i,'(',')') < 0)
   {
      delete [] arguments;
	   return(ERR);
   }

// Run the function returning results in ansVar & retVar[]
//   ansVar->MakeNullVar();
//   itfc->nrRetValues = 0;
   if(!itfc)
   {
      assert(false);
   }
   else
   {
	   r = RunCommand(itfc,function,arguments,3);
   }

   if(r == CMD_NOT_FOUND)
      r = ERR;

// Free allocated memmory
   delete [] arguments;

// Check for command errors
	if(r == INVALID_ARG)
	{
	   ErrorMessage("invalid argument to function '%s'",function);
	   return(ERR);
	}
	else if(r == ERR) // Error
	{
	   return(ERR);
	}			    
	else if(r == ABORT) // Abort
	{
	   return(ABORT);
	}
	else if(r == THROW) // throw exception
	{
	   return(THROW);
	}

   if(itfc && &(itfc->retVar[1]) != result)
   {
      if(result->CopyWithAlias(&itfc->retVar[1]) == ERR)
         return(ERR);
      result->SetScope(itfc->retVar[1].GetScope());
   }

   return(result->GetType());
}


/********************************************************************
    Evaluate string 'operand' (of type dataType)  
    and return result in variable 'result'.
********************************************************************/

short EvaluateSimpleOperand(Interface *itfc, char *operand, short dataType, Variable *result)
{
   Variable *var;
   short varType;
   CText txt;
      
// Process 'operand' based on its data type ****************************   
   switch(dataType)
   {
      case(FLOAT32): // It's a number so evaluate e.g. 1.23 or 12.3e-05
      {
         short type;
         double num = StringToNumber(operand,type);
         if(type == FLOAT32)
            result->MakeAndSetFloat((float)num);
         else
            result->MakeAndSetDouble(num);
         break;
      }
      case(FLOAT64): // It's a long float so evaluate e.g. 1.23 or 12.3e-05
      {
         result->MakeAndSetDouble(StringToDouble(operand));
         break;
      }
      case(COMPLEX): // It's an imaginary number such as 2.3j or 3i
      {
         result->MakeAndSetComplex(StrtoComplex(operand));
         break;
      }
      case(QUOTED_STRING): // It's a quoted string "Hi there"
      {
         RemoveQuotes(operand);
         if(itfc->processEscapes)
            ReplaceEscapedCharacters(operand);
         else
            ReplaceEscapedQuotes(operand);
         txt.Assign(operand);
         if(ReplaceVarInString(itfc,txt) == ERR)
            return(ERR);
         result->MakeAndSetString(txt.Str());
         break;
      }
      case(MATRIX2D): // It's an array of the form [start:step:end] or [a,b,c,d, ...] or [a,b...;c,d...]
      {
         short type;
         char c;
         CArg carg;
         short r;
         
      // Look for a delimiter
         r = FirstCharInString(operand,",;:",c);

         if(r >= 0) // Delimiter found
         {
				char * item = new char[strlen(operand)+1];
            carg.Init(c);
            carg.GetNext(operand,item);
		      type = Evaluate(itfc,RESPECT_ALIAS,item,result);
				delete [] item;
         }
         else // Delimiter not found
         {
		      type = Evaluate(itfc,RESPECT_ALIAS,operand,result);
         }

         if(type == ERR) return(ERR);

         if(type == FLOAT32)
         {
            if(EvaluateRealMatrix(itfc, operand, dataType, result) == ERR)
               return(ERR);
         }
         else if(type == FLOAT64)
         {
            if(EvaluateDoubleMatrix(itfc, operand, dataType, result) == ERR)
               return(ERR);
         }
         else if(type == UNQUOTED_STRING)
         {
            if(EvaluateStringArray(itfc, operand, dataType, result) == ERR)
            {
              // ErrorMessage("error in string");
               return(ERR);
            } 
         }             
         else
         {
            ErrorMessage("invalid type for an array");
            return(ERR);
         }
         break;
      }
      case(CMATRIX2D): // Its an array of the form {start:step:end} or {a,b,c,d, ...} or {{};{}}
      {
         if(EvaluateComplexMatrix(itfc, operand, dataType, result) == ERR)
            return(ERR);
         break;
      } 
      case(CLASS): // Its a class chain member
      {
      //   if(EvaluateClassChain(itfc,operand,result) == ERR)
      //      return(ERR);
         break;
      } 
      
 // Not an identified type so must be a variable ***************************                   
      case(UNQUOTED_STRING):
      {
         var = GetVariable(itfc,ALL_VAR,operand,varType); // Is it a variable?

         if(var) // Yes, it is a variable so copy to 'result'
         {
            if(result->CopyWithAlias(var) == ERR)
               return(ERR);
         }
         else // An undefined variable
         {

            if(!strcmp(operand,"~"))
            {
               result->MakeAndSetString("~");
            }
            else
            {
               if(itfc->inCLI || !useQuotedStrings) // Allow non-quoted strings in CLI only
               {
                  if(itfc->processEscapes)
                      ReplaceEscapedCharacters(operand);
                  result->MakeAndSetString(operand);
               }
               else
               {
                  ErrorMessage("undefined variable '%s'",operand);
                  return(ERR);
               }
            }
         }
         break;
      }
// A list without quoted strings (i.e. <...>)
      case(QUOTELESS_LIST):
      {
         if(EvaluateUnquotedStringArray(itfc, operand, dataType, result) == ERR)
         {
            return(ERR);
         }
         break;
      } 
   }
   itfc->nrRetValues = 1;
   return(result->GetType());
} 
/***************************************************************************************
* An operand of the form [a,b,c,d, ...] has been specified where the
* terms a, b c etc are strings of variable length.
***************************************************************************************/

short EvaluateStringArray(Interface *itfc, char *operand, short dataType, Variable *result)
{
	CArg carg;

   carg.Init(';');
   int nrRows = carg.Count(operand);
	if(nrRows > 1)
	{
		long  width,height; 
		List2DData *list;
		list = Make2DListFromEvaluatedText(itfc, operand);
		if(list)
		{
			result->Assign2DList(list);
			return(OK);
		}
		return(ERR);
	}
	else
	{
		long  width; 
		char **list;
		list = MakeListFromEvaluatedText(itfc, operand, &width);
		if(list)
		{
			result->MakeAndSetList(list,width);
			FreeList(list,width);
			return(OK);
		}
		return(ERR);
	}
}

/***************************************************************************************
* An operand of the form [a,b,c,d, ...] has been specified where the
* terms a, b c etc are strings of variable length.
***************************************************************************************/

short EvaluateUnquotedStringArray(Interface *itfc, char *operand, short dataType, Variable *result)
{
	CArg carg;

   carg.Init(';');
   int nrRows = carg.Count(operand);
	if(nrRows > 1)
	{
		long  width,height; 
		List2DData *list;
		list = Make2DListFromUnquotedText(itfc, operand);
		if(list)
		{
			result->Assign2DList(list);
			return(OK);
		}
		return(ERR);
	}
	else
	{
		long  width; 
		char **list;
		list = MakeListFromUnquotedText(itfc, operand, &width);
		if(list)
		{
			result->MakeAndSetList(list,width);
			FreeList(list,width);
			return(OK);
		}
		return(ERR);
	}
}




/***************************************************************************************
* An operand of the form [start:step:end] or [a,b,c,d, ...] or [a,b...;c,d...]
* has been specified. Generate a matrix from this specification and store in data[pos]
* All terms can be expressions.
***************************************************************************************/



short EvaluateRealMatrix(Interface *itfc, char *operand, short dataType, Variable *result)
{
   long  nrMatrixRows;       // Number of rows in matrix
   long  nrMatrixCols = 0;   // Number of columns in matrix
   CText  row;               // The row descriptor in text form
   CText  element;           // A particular element extract from row string
   float **mat;              // Pointer to matrix to store data
   short mode;               // How the matrix has been specified
   long i,j;
   float start,end,step;     // Range specifiers
   CArg cRows(';'),cRow;     // Delimited lists
   
// Work out the number of rows in the matrix by searching for semi-colon delimiters
   nrMatrixRows = cRows.Count(operand);
   if(nrMatrixRows == 0)
   {
      ErrorMessage("null matrix");
      return(ERR);
   }

// Extract the first row in text form from 'operand'
	row = cRows.Extract(1);

// Figure out row type and count the number of columns in the first row
   if(CheckMatrixRow(itfc, row.Str(), cRow, nrMatrixCols, start, step, end, mode) == ERR)
       return(ERR);

// Now allocate memory for new matrix, assuming rest of description is valid
	if(!(mat = MakeMatrix2D(nrMatrixCols,nrMatrixRows)))
   {
      ErrorMessage("insufficient memory to allocate matrix");
      return(ERR);
   }	

// Set the elements of each row in the newly formed matrix
   for(i = 0; i < nrMatrixRows; i++)
   {
      if(i > 0) // Extract row info (first one already extracted)
      {
	      row = cRows.Extract(i+1);
         if(CheckMatrixRow(itfc, row.Str(), cRow, nrMatrixCols, start, step, end, mode) == ERR)
         {
             FreeMatrix2D(mat);
             return(ERR);
         }
      }
      
      if(mode == RANGE) // [start:step:end]
      {	        
	      for(j = 0; j < nrMatrixCols; j++)
	      { 
	         mat[i][j] = (start + j*step);
	      }   
      }
      else // [a,b,c,d, ...]
      {
	      for(j = 0; j < nrMatrixCols; j++)
	      { 
	         element = cRow.Extract(j+1);
		      if(Evaluate(itfc,RESPECT_ALIAS,element.Str(),result) != FLOAT32)
		      {
		         FreeMatrix2D(mat);
		         ErrorMessage("array index '%s' is not a number",element.Str());
		         return(ERR);
		      } 
		      mat[i][j] = VarReal(result);
		   }
	   }
   }

// Store result in result
   result->AssignMatrix2D(mat, nrMatrixCols, nrMatrixRows);
   return(OK);
} 

/***************************************************************************************
* An operand of the form [start:step:end] or [a,b,c,d, ...] or [a,b...;c,d...]
* has been specified. Generate a matrix from this specification and store in data[pos]
* All terms can be expressions.
***************************************************************************************/

short EvaluateDoubleMatrix(Interface *itfc, char *operand, short dataType, Variable *result)
{
   long  nrMatrixRows;       // Number of rows in matrix
   long  nrMatrixCols = 0;   // Number of columns in matrix
   CText  row;               // The row descriptor in text form
   CText  element;           // A particular element extract from row string
   double **mat;             // Pointer to matrix to store data
   short mode;               // How the matrix has been specified
   long i,j;
   double start,end,step;     // Range specifiers
   CArg cRows(';'),cRow;     // Delimited lists
   
// Work out the number of rows in the matrix by searching for semi-colon delimiters
   nrMatrixRows = cRows.Count(operand);
   if(nrMatrixRows == 0)
   {
      ErrorMessage("null matrix");
      return(ERR);
   }

// Extract the first row in text form from 'operand'
	row = cRows.Extract(1);

// Figure out row type and count the number of columns in the first row
   if(CheckMatrixRow(itfc, row.Str(), cRow, nrMatrixCols, start, step, end, mode) == ERR)
       return(ERR);

// Now allocate memory for new matrix, assuming rest of description is valid
	if(!(mat = MakeDMatrix2D(nrMatrixCols,nrMatrixRows)))
   {
      ErrorMessage("insufficient memory to allocate matrix");
      return(ERR);
   }	

// Set the elements of each row in the newly formed matrix
   for(i = 0; i < nrMatrixRows; i++)
   {
      if(i > 0) // Extract row info (first one already extracted)
      {
	      row = cRows.Extract(i+1);
         if(CheckMatrixRow(itfc, row.Str(), cRow, nrMatrixCols, start, step, end, mode) == ERR)
         {
             FreeDMatrix2D(mat);
             return(ERR);
         }
      }
      
      if(mode == RANGE) // [start:step:end]
      {	        
	      for(j = 0; j < nrMatrixCols; j++)
	      { 
	         mat[i][j] = (start + j*step);
	      }   
      }
      else // [a,b,c,d, ...]
      {
	      for(j = 0; j < nrMatrixCols; j++)
	      { 
	         element = cRow.Extract(j+1);
		      short type = Evaluate(itfc,RESPECT_ALIAS,element.Str(),result);
            if(type == FLOAT32)
		         mat[i][j] = result->GetReal();
            else if(type == FLOAT64)
		         mat[i][j] = result->GetDouble();
            else
		      {
		         FreeDMatrix2D(mat);
		         ErrorMessage("array index '%s' is not a number",element.Str());
		         return(ERR);
		      } 

		   }
	   }
   }

// Store result in result
   result->AssignDMatrix2D(mat, nrMatrixCols, nrMatrixRows);
   return(OK);
} 


/***************************************************************************************
  An operand of the form {start:step:end} or {a,b,c,d, ...} or {a,b...;c,d...}
  has been specified. Generate a complex matrix from this specification and store in
  result.
***************************************************************************************/

short EvaluateComplexMatrix(Interface *itfc, char *operand, short dataType, Variable *result)
{
   long  nrMatrixRows;       // Number of rows in matrix
   long  nrMatrixCols = 0;   // Number of columns in matrix
   CText row;                // The row descriptor in text form
   CText element;            // A particular element extract from row string
   complex **cmat;           // Pointer to complex matrix to store data
   short mode;               // How the matrix has been specified
   long i,j;
   float start,end,step;     // Range specifiers
   CArg cRows(';'),cRow(','); // Delimited lists
   
// Work out the number of rows in the matrix by searching for semi-colon delimiters
   nrMatrixRows = cRows.Count(operand);
   if(nrMatrixRows == 0)
   {
      ErrorMessage("null matrix");
      return(ERR);
   }

// Extract the first row in text form from 'operand'
	row = cRows.Extract(1);

// Figure out row type and count the number of columns in the first row
   if(CheckMatrixRow(itfc, row.Str(), cRow, nrMatrixCols, start, step, end, mode) == ERR)
      return(ERR);

// Now allocate memory for new complex matrix, assuming rest of description is valid
	if(!(cmat = MakeCMatrix2D(nrMatrixCols,nrMatrixRows)))
   {
      ErrorMessage("out of memory");
      return(ERR);
   }	

// Set the elements of each row in the newly formed matrix
   for(i = 0; i < nrMatrixRows; i++)
   {
      if(i > 0)
      {
	      row = cRows.Extract(i+1);
         if(CheckMatrixRow(itfc, row.Str(), cRow, nrMatrixCols, start, step, end, mode) == ERR)
         {
             FreeCMatrix2D(cmat);
             return(ERR);
         }      
      }
      
      if(mode == RANGE) // {start:step:end}
      {	        
	      for(j = 0; j < nrMatrixCols; j++)
	      { 
	         cmat[i][j].r = (start + j*step);
	         cmat[i][j].i = cmat[i][j].r;
	      }   
      }
      else // {a,b,c,d, ...}
      {
	      for(j = 0; j < nrMatrixCols; j++)
	      {       
	         element = cRow.Extract(j+1);
	         short type = Evaluate(itfc,RESPECT_ALIAS,element.Str(),result);
	
	         if(type == COMPLEX)
	         {
	            cmat[i][j] = VarComplex(result);
	         }
	         else if(type == FLOAT32)
	         {
	             cmat[i][j].r = VarReal(result);
	             cmat[i][j].i = 0;
	         }  
	         else
	         {
	            FreeCMatrix2D(cmat);
	            ErrorMessage("array elements should be real or complex");
	            return(ERR);
	         }   
		   } 
	   }
   }

// Store result in result
   result->AssignCMatrix2D(cmat, nrMatrixCols, nrMatrixRows);

   return(OK);
}

/****************************************************************************
   User has entered an expression of the form m[x,y,z,q] (1-4 dimensions)
   Evaluate this expression and return in variable 'result'.
  
   x,y,z,q can be numerical indices or index arrays.
   'arrayName' is the name of the array (e.g. m)
   'expression' is the string expression to be evaluated (e.g. m[x,y,z,q])
   pos is the index position of the left bracket '[' in expression
*****************************************************************************/

short EvaluateArrayOperand(Interface *itfc, char *arrayName, char *expression, short &pos, Variable *result)
{
	Variable *var = 0;  // Variable representing arrayName
	long x = 0,y = 0, z = 0, q = 0;  // Array indices (-ve implies a range)
   long *xa = 0, *ya = 0, *za = 0, *qa = 0; // Array index ranges
	short type;  // Type of variable 'var'
	      
// Determine the array index or indicies
	if(ExtractArrayAddress(itfc, arrayName,expression,x,y,z,q,&xa,&ya,&za,&qa,&var,type,pos) < 0)
	   return(ERR);

// Extract the matrix elements specified by x,y ... from variable var and reurn in variable result
	if(EvaluateArrayCore(itfc,var,expression,x,y,z,q,xa,ya,za,qa,type,result) == ERR)
		return(ERR);

	return(type);

}

short EvaluateArrayOperand(Interface *itfc, char *arrayName, Variable *arrayVar, char *expression, Variable *result)
{
	long x = 0,y = 0, z = 0, q = 0;  // Array indices (-ve implies a range)
   long *xa = 0, *ya = 0, *za = 0, *qa = 0; // Array index ranges
	short type = arrayVar->GetType();
	short pos = 0;
	      
// Determine the array index or indicies
	if(ExtractArrayAddress(itfc, arrayName,expression,x,y,z,q,&xa,&ya,&za,&qa,&arrayVar,type,pos) < 0)
	   return(ERR);

// Extract the matrix elements specified by x,y ... from variable var and reurn in variable result
	if(EvaluateArrayCore(itfc,arrayVar,expression,x,y,z,q,xa,ya,za,qa,type,result) == ERR)
		return(ERR);

	return(type);

}

short EvaluateArrayCore(Interface *itfc, Variable *var, char expression[],
                          long x, long y, long z, long q,
                          long *xa, long *ya, long *za, long *qa,
								  short &type,
                          Variable *result)
{
	int i,j,k,l; // Array loop indices
   
// Extract data from array and store in result  
	switch(type)
	{
	   case(MATRIX2D):
	   {
	      if(x >= 0 && y >= 0) // One element
	      {
	         result->MakeAndSetFloat(var->GetMatrix2D()[y][x]);
	         type = FLOAT32;
	      }
	      else if(x < 0 && y >= 0) // // Part or all of a row
	      {
            result->MakeAndLoadMatrix2D(NULL,-x,1);
            for(i = 0; i < -x; i++)
               result->GetMatrix2D()[0][i] = var->GetMatrix2D()[y][xa[i]];
	         type = MATRIX2D;
            delete [] xa;
	      } 
	      else if(x >= 0 && y < 0) // // Part or all of a column
	      {
            result->MakeAndLoadMatrix2D(NULL,1,-y);
            for(i = 0; i < -y; i++)
	            result->GetMatrix2D()[0][i] = var->GetMatrix2D()[ya[i]][x];	
	         type = MATRIX2D;	 
            delete [] ya;
	      } 
	      else if(x < 0 && y < 0) // Part or all of the array
	      {
            result->MakeAndLoadMatrix2D(NULL,-x,-y);
            for(i = 0; i < -y; i++)
               for(j = 0; j < -x; j++)
	               result->GetMatrix2D()[i][j] = var->GetMatrix2D()[ya[i]][xa[j]];
	         type = MATRIX2D;
            delete [] xa;
            delete [] ya;
	      }	
         else
         {
	         ErrorMessage("invalid 2D matrix reference (%s)",expression);
	         return(ERR);
         }
	      break;
	   }

	   case(DMATRIX2D):
	   {
	      if(x >= 0 && y >= 0) // One element
	      {
	         result->MakeAndSetDouble(var->GetDMatrix2D()[y][x]);
	         type = FLOAT64;
	      }
	      else if(x < 0 && y >= 0) // // Part or all of a row
	      {
            result->MakeAndLoadDMatrix2D(NULL,-x,1);
            for(i = 0; i < -x; i++)
               result->GetDMatrix2D()[0][i] = var->GetDMatrix2D()[y][xa[i]];
	         type = DMATRIX2D;
            delete [] xa;
	      } 
	      else if(x >= 0 && y < 0) // // Part or all of a column
	      {
            result->MakeAndLoadDMatrix2D(NULL,1,-y);
            for(i = 0; i < -y; i++)
	            result->GetDMatrix2D()[0][i] = var->GetDMatrix2D()[ya[i]][x];	
	         type = DMATRIX2D;	 
            delete [] ya;
	      } 
	      else if(x < 0 && y < 0) // Part or all of the array
	      {
            result->MakeAndLoadDMatrix2D(NULL,-x,-y);
            for(i = 0; i < -y; i++)
               for(j = 0; j < -x; j++)
	               result->GetDMatrix2D()[i][j] = var->GetDMatrix2D()[ya[i]][xa[j]];
	         type = DMATRIX2D;
            delete [] xa;
            delete [] ya;
	      }	
         else
         {
	         ErrorMessage("invalid 2D matrix reference (%s)",expression);
	         return(ERR);
         }
	      break;
	   }

      case(CMATRIX2D):
	   {
	      if(x >= 0 && y >= 0) // Extract one element
	      {
	         result->MakeAndSetComplex(var->GetCMatrix2D()[y][x]);
	         type = COMPLEX;
	      }
	      else if(x < 0 && y >= 0) // Part or all of a row
	      {
            result->MakeAndLoadCMatrix2D(NULL,-x,1);
            for(i = 0; i < -x; i++)
               result->GetCMatrix2D()[0][i] = var->GetCMatrix2D()[y][xa[i]];
	         type = CMATRIX2D;
            delete [] xa;
	      } 
	      else if(x >= 0 && y < 0) // Part or all of a column
	      {
            result->MakeAndLoadCMatrix2D(NULL,1,-y);
            for(i = 0; i < -y; i++)
	            result->GetCMatrix2D()[0][i] = var->GetCMatrix2D()[ya[i]][x];	
	         type = CMATRIX2D;	 
            delete [] ya;
	      } 
	      else if(x < 0 && y < 0) // Part or all of the array
	      {
            result->MakeAndLoadCMatrix2D(NULL,-x,-y);
            for(i = 0; i < -y; i++)
               for(j = 0; j < -x; j++)
	               result->GetCMatrix2D()[i][j] = var->GetCMatrix2D()[ya[i]][xa[j]];
	         type = CMATRIX2D;
            delete [] xa;
            delete [] ya;
	      } 
         else
         {
	         ErrorMessage("invalid 2D matrix reference (%s)",expression);
	         return(ERR);
         }
	      break;
	   }	

	   case(MATRIX3D):
	   {
	      if(x >= 0 && y >= 0 && z >= 0) // One element
	      {
	         result->MakeAndSetFloat(var->GetMatrix3D()[z][y][x]);
	         type = FLOAT32;
	      }
	      else if(x < 0 && y >= 0 && z >= 0) // Part or all of a row
	      {
            result->MakeAndLoadMatrix2D(NULL,-x,1);
            for(i = 0; i < -x; i++)
               result->GetMatrix2D()[0][i] = var->GetMatrix3D()[z][y][xa[i]];
	         type = MATRIX2D;
            delete [] xa;
	      } 
	      else if(x >= 0 && y < 0 && z >= 0) // Part or all of a column
	      {
            result->MakeAndLoadMatrix2D(NULL,1,-y);
            for(i = 0; i < -y; i++)
	            result->GetMatrix2D()[i][0] = var->GetMatrix3D()[z][ya[i]][x];	
	         type = MATRIX2D;	 
            delete [] ya;
	      } 
	      else if(x >= 0 && y >= 0 && z < 0) //  Part or all of a tier
	      {
            result->MakeAndLoadMatrix2D(NULL,-z,1);
            for(i = 0; i < -z; i++)
	            result->GetMatrix2D()[0][i] = var->GetMatrix3D()[za[i]][y][x];	
	         type = MATRIX2D;	 
            delete [] za;
	      } 
	      else if(x < 0 && y < 0 && z >= 0) //  Part or all of an xy plane
	      {
            result->MakeAndLoadMatrix2D(NULL,-x,-y);
            for(i = 0; i < -y; i++)
               for(j = 0; j < -x; j++)
	                result->GetMatrix2D()[i][j] = var->GetMatrix3D()[z][ya[i]][xa[j]];	
	         type = MATRIX2D;	 
            delete [] xa;
            delete [] ya;
	      } 
	      else if(x >= 0 && y < 0 && z < 0) //  Part or all of an yz plane
	      {
            result->MakeAndLoadMatrix2D(NULL,-y,-z);
            for(i = 0; i < -z; i++)
               for(j = 0; j < -y; j++)
	                result->GetMatrix2D()[i][j] = var->GetMatrix3D()[za[i]][ya[j]][x];	
	         type = MATRIX2D;	 
            delete [] ya;
            delete [] za;
	      } 
	      else if(x < 0 && y >= 0 && z < 0) //  Part or all of an xz plane
	      {
            result->MakeAndLoadMatrix2D(NULL,-x,-z);
            for(i = 0; i < -z; i++)
               for(j = 0; j < -x; j++)
	                result->GetMatrix2D()[i][j] = var->GetMatrix3D()[za[i]][y][xa[j]];	
	         type = MATRIX2D;	 
            delete [] xa;
            delete [] za;
	      }  
	      else if(x < 0 && y < 0 && z < 0) // part or all of the whole array 
	      {
	         result->MakeAndLoadMatrix3D(NULL,-x,-y,-z);
	         for(i = 0; i < -z; i++)
	            for(j = 0; j < -y; j++)
	              for(k = 0; k < -x; k++)
	                result->GetMatrix3D()[i][j][k] = var->GetMatrix3D()[za[i]][ya[j]][xa[k]];		                
	         type = MATRIX3D;	
            delete [] xa;
            delete [] ya;
            delete [] za;
	      }
         else
         {
	         ErrorMessage("invalid 3D matrix reference (%s)",expression);
	         return(ERR);
         }
	      break;
	   }	   

	   case(CMATRIX3D):
	   {
	      if(x >= 0 && y >= 0 && z >= 0) // One element
	      {
	         result->MakeAndSetComplex(var->GetCMatrix3D()[z][y][x]);
	         type = FLOAT32;
	      }
	      else if(x < 0 && y >= 0 && z >= 0) // // Part or all of a row
	      {
            result->MakeAndLoadCMatrix2D(NULL,-x,1);
            for(i = 0; i < -x; i++)
               result->GetCMatrix2D()[0][i] = var->GetCMatrix3D()[z][y][xa[i]];
	         type = MATRIX2D;
            delete [] xa;
	      } 
	      else if(x >= 0 && y < 0 && z >= 0) // Part or all of a column
	      {
            result->MakeAndLoadCMatrix2D(NULL,1,-y);
            for(i = 0; i < -y; i++)
	            result->GetCMatrix2D()[0][i] = var->GetCMatrix3D()[z][ya[i]][x];	
	         type = MATRIX2D;	 
            delete [] ya;
	      } 
	      else if(x >= 0 && y >= 0 && z < 0) //  Part or all of a tier
	      {
            result->MakeAndLoadCMatrix2D(NULL,-z,1);
            for(i = 0; i < -z; i++)
	            result->GetCMatrix2D()[0][i] = var->GetCMatrix3D()[za[i]][y][x];	
	         type = MATRIX2D;	 
            delete [] za;
	      } 
	      else if(x < 0 && y < 0 && z >= 0) //  Part or all of an xy plane
	      {
            result->MakeAndLoadCMatrix2D(NULL,-x,-y);
            for(i = 0; i < -y; i++)
               for(j = 0; j < -x; j++)
	                result->GetCMatrix2D()[i][j] = var->GetCMatrix3D()[z][ya[i]][xa[j]];	
	         type = MATRIX2D;	 
            delete [] xa;
            delete [] ya;
	      } 
	      else if(x >= 0 && y < 0 && z < 0) //  Part or all of an yz plane
	      {
            result->MakeAndLoadCMatrix2D(NULL,-y,-z);
            for(i = 0; i < -z; i++)
               for(j = 0; j < -y; j++)
	                result->GetCMatrix2D()[i][j] = var->GetCMatrix3D()[za[i]][ya[j]][x];	
	         type = MATRIX2D;	 
            delete [] ya;
            delete [] za;
	      } 
	      else if(x < 0 && y >= 0 && z < 0) //  Part or all of an xz plane
	      {
            result->MakeAndLoadCMatrix2D(NULL,-x,-z);
            for(i = 0; i < -z; i++)
               for(j = 0; j < -x; j++)
	                result->GetCMatrix2D()[i][j] = var->GetCMatrix3D()[za[i]][y][xa[j]];	
	         type = MATRIX2D;	 
            delete [] xa;
            delete [] za;
	      }  
	      else if(x < 0 && y < 0 && z < 0) // part or all of the whole array 
	      {
	         result->MakeAndLoadCMatrix3D(NULL,-x,-y,-z);
	         for(i = 0; i < -z; i++)
	            for(j = 0; j < -y; j++)
	              for(k = 0; k < -x; k++)
	                result->GetCMatrix3D()[i][j][k] = var->GetCMatrix3D()[za[i]][ya[j]][xa[k]];		                
	         type = MATRIX3D;	
            delete [] xa;
            delete [] ya;
            delete [] za;
	      }
         else
         {
	         ErrorMessage("invalid 3D matrix reference (%s)",expression);
	         return(ERR);
         }
	      break;
	   }	

	   case(MATRIX4D):
	   {
	      if(x >= 0 && y >= 0 && z >= 0 && q >= 0) // One element
	      {
	         result->MakeAndSetFloat(VarReal4DMatrix(var)[q][z][y][x]);
	         type = FLOAT32;
	      }
	      else if(x < 0 && y >= 0 && z >= 0 && q >= 0) // Part or all of a row
	      {
	         result->MakeAndLoadMatrix2D(NULL,-x,1);
	         for(i = 0; i < -x; i++)
	            result->GetMatrix2D()[0][i] = var->GetMatrix4D()[q][z][y][xa[i]];	
	         type = MATRIX2D;	
            delete [] xa;
	      } 
	      else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // Part or all of a column
	      {
	         result->MakeAndLoadMatrix2D(NULL,1,-y);
	         for(i = 0; i < -y; i++)
	             result->GetMatrix2D()[i][0] = var->GetMatrix4D()[q][z][ya[i]][x];
	         type = MATRIX2D;	
            delete [] ya;
         }
	      else if(x >= 0 && y >= 0 && z < 0 && q >= 0) //  Part or all of a tier
	      {
	         result->MakeAndLoadMatrix2D(NULL,-z,1);
	         for(i = 0; i < -z; i++)
	            result->GetMatrix2D()[0][i] = var->GetMatrix4D()[q][za[i]][y][x];	
	         type = MATRIX2D;
            delete [] za;
	      }
	      else if(x >= 0 && y >= 0 && z >= 0 && q < 0) // A hyperline
	      {
	         result->MakeAndLoadMatrix2D(NULL,-q,1);
	         for(i = 0; i < -q; i++)
	            result->GetMatrix2D()[0][i] = var->GetMatrix4D()[qa[i]][z][y][x];	
	         type = MATRIX2D;	
            delete [] qa;
	      }
	      else if(x < 0 && y < 0 && z >= 0 && q >= 0) //  Part or all of an xy plane
	      {
	         result->MakeAndLoadMatrix2D(NULL,-x,-y); 
	         for(i = 0; i < -y; i++)
	            for(j = 0; j < -x; j++)
	               result->GetMatrix2D()[i][j] = var->GetMatrix4D()[q][z][ya[i]][xa[j]];
	         type = MATRIX2D;	
            delete [] xa;
            delete [] ya;
	      }
	      else if(x < 0 && y >= 0 && z >= 0 && q < 0) // Part or all of a plane from xq
	      {
	         result->MakeAndLoadMatrix2D(NULL,-x,-q);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -x; j++)
	               result->GetMatrix2D()[i][j] = var->GetMatrix4D()[qa[i]][z][y][xa[j]];	
	         type = MATRIX2D;	
            delete [] xa;
            delete [] qa;
	      }
	      else if(x < 0 && y >= 0 && z < 0 && q >= 0) // Part or all of a plane from xz
	      {
	         result->MakeAndLoadMatrix2D(NULL,-x,-z);
	         for(i = 0; i < -z; i++)
	            for(j = 0; j < -x; j++)
	               result->GetMatrix2D()[i][j] = var->GetMatrix4D()[q][za[i]][y][xa[j]];	
	         type = MATRIX2D;
            delete [] xa;
            delete [] za;
	      }
	      else if(x >= 0 && y < 0 && z < 0 && q >= 0) // Part or all of a plane from yz
	      {
	         result->MakeAndLoadMatrix2D(NULL,-y,-z);
	         for(i = 0; i < -z; i++)
	            for(j = 0; j < -y; j++)
	               result->GetMatrix2D()[i][j] = var->GetMatrix4D()[q][za[i]][ya[j]][x];	
	         type = MATRIX2D;
            delete [] ya;
            delete [] za;
	      } 
	      else if(x >= 0 && y < 0 && z >= 0 && q < 0) // Part or all of a plane from yq
	      {
	         result->MakeAndLoadMatrix2D(NULL,-q,-y);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -y; j++)
	               result->GetMatrix2D()[j][i] = var->GetMatrix4D()[qa[i]][z][ya[j]][x];	
	         type = MATRIX2D;	               	                
	      }
	      else if(x >= 0 && y >= 0 && z < 0 && q < 0) // Part or all of a plane from zq
	      {
	         result->MakeAndLoadMatrix2D(NULL,-q,-z);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -z; j++)
	               result->GetMatrix2D()[j][i] = var->GetMatrix4D()[qa[i]][za[j]][y][x];	
	         type = MATRIX2D;
            delete [] za;
            delete [] qa;
	      }
	      else if(x < 0 && y < 0 && z < 0 && q >= 0) // An xyz data set 
	      {
	         result->MakeAndLoadMatrix3D(NULL,-x,-y,-z);
	         for(i = 0; i < -z; i++)
	            for(j = 0; j < -y; j++)
	               for(k = 0; k < -x; k++)
	                     result->GetMatrix3D()[i][j][k] = var->GetMatrix4D()[q][za[i]][ya[j]][xa[k]];		                
	         type = MATRIX3D;
            delete [] xa;
            delete [] ya;
            delete [] za;
	      }
	      else if(x < 0 && y < 0 && z >= 0 && q < 0) // An xyq data set 
	      {
	         result->MakeAndLoadMatrix3D(NULL,-x,-y,-q);
	         for(i = 0; i < -x; i++)
	            for(j = 0; j < -y; j++)
	               for(k = 0; k < -z; k++)
	                     result->GetMatrix3D()[i][j][k] = var->GetMatrix4D()[q][za[k]][ya[j]][xa[i]];		                
	         type = MATRIX3D;	  
            delete [] xa;
            delete [] ya;
            delete [] qa;
	      }
         else if(x < 0 && y >= 0 && z < 0 && q < 0) // An xzq data set 
	      {
	         result->MakeAndLoadMatrix3D(NULL,-x,-z,-q);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -z; j++)
	               for(k = 0; k < -x; k++)
	                     result->GetMatrix3D()[i][j][k] = var->GetMatrix4D()[qa[i]][za[j]][y][xa[k]];		                
	         type = MATRIX3D;
            delete [] xa;
            delete [] za;
            delete [] qa;
	      }
         else if(x >= 0 && y < 0 && z < 0 && q < 0) // An yzq data set 
	      {
	         result->MakeAndLoadMatrix3D(NULL,-y,-z,-q);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -z; j++)
	               for(k = 0; k < -y; k++)
	                     result->GetMatrix3D()[i][j][k] = var->GetMatrix4D()[qa[i]][za[j]][ya[k]][x];		                
	         type = MATRIX3D;	
            delete [] ya;
            delete [] za;
            delete [] qa;
	      }
	      else if(x < 0 && y < 0 && z < 0 && q < 0) // The whole array 
	      {
	         result->MakeAndLoadMatrix4D(NULL,-x,-y,-z,-q);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -z; j++)
	               for(k = 0; k < -y; k++)
	                  for(l = 0; l < -x; l++)
	                     result->GetMatrix4D()[i][j][k][l] = var->GetMatrix4D()[qa[i]][za[j]][ya[k]][xa[l]];		                
	         type = MATRIX4D;	                            
	      }
         else
         {
	         ErrorMessage("invalid 4D matrix reference (%ld,%ld,%ld,%ld)",x,y,z,q);
	         return(ERR);
         }
	      break;
	   }	  
	   case(CMATRIX4D):
	   {
	      if(x >= 0 && y >= 0 && z >= 0 && q >= 0) // One element
	      {
	         result->MakeAndSetComplex(var->GetCMatrix4D()[q][z][y][x]);
	         type = COMPLEX;
	      }
	      else if(x < 0 && y >= 0 && z >= 0 && q >= 0) // Part or all of a row
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-x,1);
	         for(i = 0; i < -x; i++)
	            result->GetCMatrix2D()[0][i] = var->GetCMatrix4D()[q][z][y][xa[i]];	
	         type = CMATRIX2D;	
            delete [] xa;
	      } 
	      else if(x >= 0 && y < 0 && z >= 0 && q >= 0) // Part or all of a column
	      {
	         result->MakeAndLoadCMatrix2D(NULL,1,-y);
	         for(i = 0; i < -y; i++)
	             result->GetCMatrix2D()[i][0] = var->GetCMatrix4D()[q][z][ya[i]][x];
	         type = CMATRIX2D;	
            delete [] ya;
         }
	      else if(x >= 0 && y >= 0 && z < 0 && q >= 0) //  Part or all of a tier
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-z,1);
	         for(i = 0; i < -z; i++)
	            result->GetCMatrix2D()[0][i] = var->GetCMatrix4D()[q][za[i]][y][x];	
	         type = CMATRIX2D;
            delete [] za;
	      }
	      else if(x >= 0 && y >= 0 && z >= 0 && q < 0) // A hyperline
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-q,1);
	         for(i = 0; i < -q; i++)
	            result->GetCMatrix2D()[0][i] = var->GetCMatrix4D()[qa[i]][z][y][x];	
	         type = CMATRIX2D;	
            delete [] qa;
	      }
	      else if(x < 0 && y < 0 && z >= 0 && q >= 0) //  Part or all of an xy plane
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-x,-y); 
	         for(i = 0; i < -y; i++)
	            for(j = 0; j < -x; j++)
	               result->GetCMatrix2D()[i][j] = var->GetCMatrix4D()[q][z][ya[i]][xa[j]];
	         type = CMATRIX2D;	
            delete [] xa;
            delete [] ya;
	      }
	      else if(x < 0 && y >= 0 && z >= 0 && q < 0) // Part or all of a plane from xq
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-x,-q);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -x; j++)
	               result->GetCMatrix2D()[i][j] = var->GetCMatrix4D()[qa[i]][z][y][xa[j]];	
	         type = CMATRIX2D;	
            delete [] xa;
            delete [] qa;
	      }
	      else if(x < 0 && y >= 0 && z < 0 && q >= 0) // Part or all of a plane from xz
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-x,-z);
	         for(i = 0; i < -z; i++)
	            for(j = 0; j < -x; j++)
	               result->GetCMatrix2D()[i][j] = var->GetCMatrix4D()[q][za[i]][y][xa[j]];	
	         type = CMATRIX2D;
            delete [] xa;
            delete [] za;
	      }
	      else if(x >= 0 && y < 0 && z < 0 && q >= 0) // Part or all of a plane from yz
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-y,-z);
	         for(i = 0; i < -z; i++)
	            for(j = 0; j < -y; j++)
	               result->GetCMatrix2D()[i][j] = var->GetCMatrix4D()[q][za[i]][ya[j]][x];	
	         type = CMATRIX2D;
            delete [] ya;
            delete [] za;
	      } 
	      else if(x >= 0 && y < 0 && z >= 0 && q < 0) // Part or all of a plane from yq
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-q,-y);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -y; j++)
	               result->GetCMatrix2D()[j][i] = var->GetCMatrix4D()[qa[i]][z][ya[j]][x];	
	         type = CMATRIX2D;	               	                
	      }
	      else if(x >= 0 && y >= 0 && z < 0 && q < 0) // Part or all of a plane from zq
	      {
	         result->MakeAndLoadCMatrix2D(NULL,-q,-z);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -z; j++)
	               result->GetCMatrix2D()[j][i] = var->GetCMatrix4D()[qa[i]][za[j]][y][x];	
	         type = CMATRIX2D;
            delete [] za;
            delete [] qa;
	      }
	      else if(x < 0 && y < 0 && z < 0 && q >= 0) // An xyz data set 
	      {
	         result->MakeAndLoadCMatrix3D(NULL,-x,-y,-z);
	         for(i = 0; i < -z; i++)
	            for(j = 0; j < -y; j++)
	               for(k = 0; k < -x; k++)
	                     result->GetCMatrix3D()[i][j][k] = var->GetCMatrix4D()[q][za[i]][ya[j]][xa[k]];		                
	         type = CMATRIX3D;
            delete [] xa;
            delete [] ya;
            delete [] za;
	      }
	      else if(x < 0 && y < 0 && z >= 0 && q < 0) // An xyq data set 
	      {
	         result->MakeAndLoadCMatrix3D(NULL,-x,-y,-q);
	         for(i = 0; i < -x; i++)
	            for(j = 0; j < -y; j++)
	               for(k = 0; k < -z; k++)
	                     result->GetCMatrix3D()[i][j][k] = var->GetCMatrix4D()[q][za[k]][ya[j]][xa[i]];		                
	         type = CMATRIX3D;	  
            delete [] xa;
            delete [] ya;
            delete [] qa;
	      }
         else if(x < 0 && y >= 0 && z < 0 && q < 0) // An xzq data set 
	      {
	         result->MakeAndLoadCMatrix3D(NULL,-x,-z,-q);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -z; j++)
	               for(k = 0; k < -x; k++)
	                     result->GetCMatrix3D()[i][j][k] = var->GetCMatrix4D()[qa[i]][za[j]][y][xa[k]];		                
	         type = CMATRIX3D;
            delete [] xa;
            delete [] za;
            delete [] qa;
	      }
         else if(x >= 0 && y < 0 && z < 0 && q < 0) // An yzq data set 
	      {
	         result->MakeAndLoadCMatrix3D(NULL,-y,-z,-q);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -z; j++)
	               for(k = 0; k < -y; k++)
	                     result->GetCMatrix3D()[i][j][k] = var->GetCMatrix4D()[qa[i]][za[j]][ya[k]][x];		                
	         type = CMATRIX3D;	
            delete [] ya;
            delete [] za;
            delete [] qa;
	      }
	      else if(x < 0 && y < 0 && z < 0 && q < 0) // The whole array 
	      {
	         result->MakeAndLoadCMatrix4D(NULL,-x,-y,-z,-q);
	         for(i = 0; i < -q; i++)
	            for(j = 0; j < -z; j++)
	               for(k = 0; k < -y; k++)
	                  for(l = 0; l < -x; l++)
	                     result->GetCMatrix4D()[i][j][k][l] = var->GetCMatrix4D()[qa[i]][za[j]][ya[k]][xa[l]];		                
	         type = CMATRIX4D;	
            delete [] xa;
            delete [] ya;
            delete [] za;
            delete [] qa;
	      }
         else
         {
	         ErrorMessage("invalid 4D matrix reference (%ld,%ld,%ld,%ld)",x,y,z,q);
	         return(ERR);
         }
	      break;
	   }	 	  
	   case(LIST):
	   {
	      if(x < 0) // // Part or all of list
	      {
            result->MakeList(-x);
            for(i = 0; i < -x; i++)
               result->ReplaceListItem(var->GetList()[xa[i]],i);
	         type = LIST;
            delete [] xa;
	      } 
         else // One element of the list
         {
	         result->MakeAndSetString(VarList(var)[x]);
            type = UNQUOTED_STRING;
         }
	      break;
	   } 
	   case(LIST2D):
	   {
			List2DData *lst = var->GetList2D();

	      if(x >= 0 && y >= 0) // One element
	      {
				if(x < lst->rowSz[y] && y < lst->nrRows)
				{
					result->MakeAndSetString(lst->strings[y][x]);
					type = UNQUOTED_STRING;
				}
				else
				{
					ErrorMessage("2D list array index 1 out of bounds (%s)",expression);
					return(ERR);
				}
	      }
	      else if(x < 0 && y >= 0) // // Part or all of a row
	      {
				int width = lst->rowSz[y];
            result->MakeList(width);
            for(i = 0; i < width; i++)
               result->ReplaceListItem(lst->strings[y][xa[i]],i);
	         type = LIST;
            delete [] xa;
	      } 
	      else if(x >= 0 && y < 0) // // Part or all of a column
	      {
            for(i = 0; i < -y; i++)
				{
				   int width = lst->rowSz[i];
					if(x >= width) 
					{
						delete [] ya;
						ErrorMessage("2D list in (%s) has to few columns in row %d",expression,x);
						return(ERR);
					}
				}
            result->MakeList(-y);
            for(i = 0; i < -y; i++)
               result->ReplaceListItem(lst->strings[ya[i]][x],i);
	         type = LIST;
            delete [] ya;
	      } 
	      else if(x < 0 && y < 0) // Part or all of the array
	      {
            for(i = 0; i < -y; i++)
				{
				   int width = lst->rowSz[ya[i]];
				   for(j = 0; j < -x; j++)
					{
						if(xa[j] >= width) 
						{
							delete [] xa;
							delete [] ya;
							ErrorMessage("2D list in (%s) has to few columns in row %d",expression,i);
							return(ERR);
						}
					}
				}
            result->MakeAndSet2DList(-x,-y);
            for(i = 0; i < -y; i++)
               for(j = 0; j < -x; j++)
                  result->Replace2DListItem(lst->strings[ya[i]][xa[j]],j,i);
	         type = LIST2D;
            delete [] xa;
            delete [] ya;
	      }	
         else
         {
	         ErrorMessage("invalid 2D list reference (%s)",expression);
	         return(ERR);
         }
	      break;
	   }  
      case(STRUCTURE_ARRAY):
	   {
	      if(x < 0) // // part or all of structure
	      {
            Variable *arrayIn, *arrayOut ;
            Variable *strucIn,*strucOut;
            arrayIn = (Variable*)var->GetData();
            result->MakeStructArray(-x);
            arrayOut = (Variable*)result->GetData();

            for(i = 0; i < -x; i++)
            {
               strucIn = &(arrayIn[xa[i]]);
               strucOut = &(arrayOut[i]);
               if(CopyVariable(strucOut,strucIn,FULL_COPY) == ERR)
                  return(ERR);
            }
	         type = STRUCTURE_ARRAY;
            delete [] xa;
	      } 
         else // one element of the structure
         {
            Variable *strucArray = (Variable*)var->GetData();
            Variable *varBase = &(strucArray[x]);
            result->MakeStruct();
            if(CopyVariable(result,varBase,FULL_COPY) == ERR)
               return(ERR);
            CText name;
            name.Format("%s[%hd]",var->GetName(),x);
            result->SetName(name.Str());
            type = STRUCTURE;
         }
	      break;
	   }
	   case(UNQUOTED_STRING):
	   {
	      if(x < 0) // // Part or all of string
	      {
            char *s = new char[-x+1];
            for(i = 0; i < -x; i++)
               s[i] = var->GetString()[xa[i]];
            s[i] = '\0';
	         type = UNQUOTED_STRING;
	         result->AssignString(s);
            delete [] xa;
	      } 
         else // One element of the string
         {
            char s[2];
            s[0] = VarString(var)[x];
            s[1] = '\0';
	         result->MakeAndSetString(s);
            type = UNQUOTED_STRING;
         }
	      break;
	   } 
      default:
         ErrorMessage("invalid type to have an array index");
         return(ERR);
	}
 
	return(OK);         
}


/********************************************************************************
   Returns the indices of an array reference of the form m[x,y,z,q] where
   x,y,z,q can be variables or arrays themselves. The special case is when
   they are '~' in which case the whole row,col etc is returned.
                            
 Passed Variables
  'arrayName' is the name of the array e.g. 'm'
  'expression' contains the array name and its indices in string form.
   i passes the position of the array left bracket '['

 Returned Variables
   indexi returns the index of the ith dimension - if negative it is the length 
   of the array rangei which are a list of indices to load.
   rangei returns an array of indices to load from the row col etc.          
  The actual array referred to by 'arrayName' is returned in 'var'. 
  The type of array is returned in varType.
     

  Note that this function will also return an element in a list or a character
  from a string.

  Last modified 23 June 2008
********************************************************************************/

short ExtractArrayAddress(Interface *itfc, char arrayName[], char expression[],
                          long &index1, long &index2, long &index3, long &index4,
                          long **range1, long **range2, long **range3, long **range4,
                          Variable **var, short &varType, short &i)
{
	int nrArgs,nrDim;
	char innerStr[MAX_STR];
   
   index1 = index2 = index3 = index4 = 0;

// Get the array variable and check it is a valid type  
   if(!(*var))
   {
	   (*var) = GetVariable(itfc,ALL_VAR,arrayName,varType);
      if((*var) == NULL)
	   {
	      ErrorMessage("Matrix '%s' is not defined",arrayName);
	      return(ERR);
	   } 
   }
      	
	if(varType != MATRIX2D   && varType != CMATRIX2D && 
	   varType != MATRIX3D   && varType != CMATRIX3D &&
	   varType != MATRIX4D   && varType != CMATRIX4D &&
      varType != DMATRIX2D  && varType != STRUCTURE_ARRAY &&
      varType != LIST       && varType != LIST2D &&
		varType != UNQUOTED_STRING) // Check for array type
	{
	   ErrorMessage("operand '%s' is not a matrix, list or string",arrayName);
	   return(ERR);
	}

// Get the index expression e.g. [x,y] ****************	
   if(ExtractSubExpression(expression,innerStr,i,'[',']') < 0) 
      return(ERR);
   
// Count the number of array dimensions ******************
   CArg innerArgs;
   innerArgs.Init(',');
   nrArgs = innerArgs.Count(innerStr);

   if(nrArgs == 0) // None given!
   {
     ErrorMessage(" no array index supplied");
     return(ERR);
   }
   if(nrArgs > 4) // 4 is the maximum currently
   {
     ErrorMessage("too many indicies passed (max 4)");
     return(ERR);
   }

// Check that the number of indices matches the array variable  **********
   if(varType == MATRIX2D || varType == CMATRIX2D || varType == DMATRIX2D || varType == LIST2D)
      nrDim = 2;
   else if(varType == MATRIX3D || varType == CMATRIX3D)
      nrDim = 3;
   else if(varType == MATRIX4D || varType == CMATRIX4D)
      nrDim = 4;
   else // List
      nrDim = 1;

   if(nrArgs > nrDim)
   {
     ErrorMessage("invalid number of indices passed for matrix '%s'",arrayName);
     return(ERR);
   }   

// Check for correct number of indices passed
   if(nrArgs == 1 && (*var)->GetDimX() > 1 && (*var)->GetDimY() > 1)
   {
      ErrorMessage("expected 2 indices");
      return(ERR);
   }
   else if(nrArgs == 2 && (*var)->GetDimX() > 1 && (*var)->GetDimY() > 1 && (*var)->GetDimZ() > 1)
   {
      ErrorMessage("expected 3 indices");
      return(ERR);
   }
   else if(nrArgs == 3 && (*var)->GetDimX() > 1 && (*var)->GetDimY() > 1 && (*var)->GetDimZ() > 1 &&  (*var)->GetDimQ() > 1)
   {
      ErrorMessage("expected 4 indices");
      return(ERR);
   }

// Process the first index for a string ****
   if(varType == UNQUOTED_STRING)
   {
      if(ProcessIndex(itfc, 1, strlen((*var)->GetString()), &innerArgs, index1, range1))
         return(ERR);
   }
   else // Process the first index for a vector or a matrix ****
   {
      if(ProcessIndex(itfc, 1, (*var)->GetDimX(), &innerArgs, index1, range1))
         return(ERR);
   }

// Process the second index for a vector or a matrix ****
   if(nrArgs >= 2)
   {
      if(ProcessIndex(itfc, 2, (*var)->GetDimY(), &innerArgs, index2, range2))
         return(ERR);
   }
	
// Process the third index for a vector or a matrix ****
   if(nrArgs >= 3)
   {
      if(ProcessIndex(itfc, 3, (*var)->GetDimZ(), &innerArgs, index3, range3))
         return(ERR);
   }

// Process the fourth index for a vector or a matrix ****
   if(nrArgs >= 4)
   {
      if(ProcessIndex(itfc, 4, (*var)->GetDimQ(), &innerArgs, index4, range4))
         return(ERR);
   }
  	   
   return(OK);
}

/*****************************************************************************
  Process the index for dimension 'whichDim' e.g. if input is m[[2:4],3]
  and whichDim = 1 it will return [2:4] in outList and -3 for outIndex.
  If whichDim = 2 it will return 3 for outindex adn outList is unchanged
******************************************************************************/

bool ProcessIndex(Interface *itfc,
                  short whichDim,    // Which array dimension are we processing
                  long maxDim,       // Size of output matrix in current dimension
                  CArg *indexExp,    // String holding index expression
                  long &outIndex,    // Which index to extract (-ve if from list)
                  long **outList)    // List of indices to extract

{
   Variable result; // Index or list of indices    
   short datType;
   long i,j;
	char indexStr[MAX_STR];
	char boundary[MAX_STR];

// Extract index expression for current dimension
   char *s = indexExp->Extract(whichDim); 	
   long len = strlen(s);

// Check to see if this is a range expression 
// by searching for the : operator closest to one end
// This allows statements like m[:2:20] or m[2:3:]
   for(i = 0; i < len; i++)
   {
      if(s[i] == ':')
        break;
   }

   for(j = len-1; j >= 0; j--)
   {
      if(s[j] == ':')
        break;
   }

	if(len-1-j < i)
		i = j;

// If it is surround with array brackets (if not already)
   if(s[0] != '[') // No brackets
   {
	// Check for partial range in expression e.g. m[:20] or m[20:]
		if(i == len)
		{
			strcpy(indexStr,s);
		}
		else if(i == 0 && len == 1) // e.g. m[:]
		{
			strcpy(indexStr,"~");
		}
		else if(i == len-1) // e.g. m[20:]
		{
			indexStr[0] = '[';
			itoa(maxDim-1,boundary,10);
			strcpy(indexStr+1,s);
			strcat(indexStr,boundary);
			strcat(indexStr,"]");
		}
		else if(i == 0) //  e.g. m[:5]
		{
			strcpy(indexStr,"[0");
			strcat(indexStr,s);
			strcat(indexStr,"]");
		}
		else // e.g. m[10:20]
		{
			indexStr[0] = '[';
			strcpy(indexStr+1,s);
			strcat(indexStr,"]");
		}
	}
	else // Brackets already
	{
	// Check for partial range in expression e.g. m[[:20]] or m[[20:]]
		if(i == len)
		{
			strcpy(indexStr,s);
		}
		else if(i == 1 && len == 3) // e.g. m[[:]]
		{
			strcpy(indexStr,"~");
		}
		else if(i == len-2)// e.g. m[[20:]]
		{
			itoa(maxDim-1,boundary,10);
			strcpy(indexStr,s);
			strcpy(indexStr+len-1,boundary);
			strcat(indexStr,"]");
		}
		else if(i == 1) //  e.g. m[[:5]]
		{
			strcpy(indexStr,"[0");
			strcat(indexStr,s+1);
		}
		else // e.g. m[10:20]
		{
			strcpy(indexStr,s);
		}
   }

// Evaluate index expression returning an index or array of indicies
   datType = EvaluateArrayIndexStr(itfc,indexStr,maxDim,&result);

//	datType = Evaluate(itfc,RESPECT_ALIAS,indexStr,&result);

   if(datType == ERR)
   {
      return(1);
   }

// Check for invalid indices (out of bounds or non integer).
   switch(result.GetType())
   {
      case(FLOAT32):
      {
         float indexf = result.GetReal();
         if(IsInteger(indexf))  // Make sure its an integer
         {
            outIndex = nint(indexf);
            if(outIndex < 0 || outIndex >= maxDim) // Make sure its a valid index
            {
               ErrorMessage("array index %hd is out of bounds",whichDim);
               return(1);
            }
         }
         else
         {
            ErrorMessage("array index %hd (%s) should be an integer",whichDim,indexStr);
            return(1);
         }
         break;
      }
      case(MATRIX2D): // The passed index is a row vector
      {
         float **inList = result.GetMatrix2D();
         long s = result.GetDimX();
         (*outList) = new long[s];
         for(int x = 0; x < s; x++)
         {
            if(IsInteger(inList[0][x]))
            {
               (*outList)[x] = nint(inList[0][x]);
               if((*outList)[x] < 0 || (*outList)[x] >= maxDim)
               {
                  delete [] (*outList);
                  *outList = NULL;
                  ErrorMessage("matrix index %d in '%s' is out of range",x,indexStr); 
                  return(1);
               }
            }
            else
            {
               delete [] (*outList);
               *outList = NULL;
               ErrorMessage("matrix index %d in '%s' is not an integer",x,indexStr);
               return(1);
            }
         }
         outIndex = -s;
         break;
      }
      case(UNQUOTED_STRING):
      {
			if(!strcmp(result.GetString(),"~"))
			{
				long s = maxDim;
				(*outList) = new long[s];
				for(int x = 0; x < s; x++)
					(*outList)[x] = x;
				outIndex = -s;
				break;
			}
			else
			{
				ErrorMessage("matrix index '%s' should be an integer, an array or '~'",indexStr);
				return(1);
			}
      }
      default:
      {
         ErrorMessage("matrix index '%s' should be an integer, an array or '~'",indexStr);
         return(1);
      }
   }
   return(0);
}

/*****************************************************************************************************************
   Helper function for EvaluateArrayIndexStr
   For an array specification of the form [a:b:c] or [a:c] this returns a, b and c as floating point numbers.
   Note that a or c can be negative in which case they are references from the end of the array.
******************************************************************************************************************/

short GetArraySpecs(Interface *itfc, char *row, long maxDim, long &nrMatrixCols, float &start, float &step, float &end)
{
   int nrTerms;
   Variable result;
   char *str;
   CArg cRow(':');

   nrTerms = cRow.Count(row);

// Extract arguments - they can be expressions so evaluate them     
   if(nrTerms == 3)
   {        
   // Extract the start value
      str = cRow.Extract(1);
      if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
      {
         ErrorMessage("start index for array should be float");
         return(ERR);
      }   
      start = result.GetReal(); 
      if(start < 0) // Allow for negative start
         start = maxDim+start;
   // Extract the step value
      str = cRow.Extract(2);

      if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
      {
         ErrorMessage("step size value for array should be float");
         return(ERR);
      } 
      step = result.GetReal(); 
      if(step == 0)
      {
         ErrorMessage("zero step size");
         return(ERR);
      }
   // Extract the end value
      str = cRow.Extract(3);

      if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
      {
         ErrorMessage("end index for array should be float");
         return(ERR);
      }  
      end = result.GetReal();   
      if(end < 0) // Allow for negative start
         end = maxDim+end;

   // Work out number of columns in matrix from the array definition
   //   long temp = int(1.000001*(end-start)/step)+1;
      long temp = (int)(((double)end-(double)start)/(double)step+1.50);

      if(temp < 0)
      {
         ErrorMessage("invalid array step sign for array limits");
         return(ERR);
      }
   // If desired check to see if the number of columns has changed
      if(nrMatrixCols > 0 && temp != nrMatrixCols)
      {
         ErrorMessage("number of columns is not constant");
         return(ERR);
      } 

   // Update the number of columns
      nrMatrixCols = temp;         
   }
   else if(nrTerms == 2)
   {
   // Extract the start value
      str = cRow.Extract(1);
      if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
      {
         ErrorMessage("start value for array should be float");
         return(ERR);
      }   
      start = result.GetReal();        
      if(start < 0) // Allow for negative end.s
         start = maxDim+start;
   // Set the step value to 1
      step = 1.0; 
   // Extract the end value
      str = cRow.Extract(2);

      if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
      {
         ErrorMessage("end value for array should be float");
         return(ERR);
      }  
      end = result.GetReal();  
      if(end < 0) // Allow for negative end.s
         end = maxDim+end;
      if(end < start)
      {
         ErrorMessage("end value must be greater or equal to start value");
         return(ERR);
      }
   // Work out number of columns in matrix from the array definition
   // The term 1.000001 ensures that rounding errors don't occur in the
   // calculation 
   //   long temp = int(1.000001*(end-start)/step)+1;
      long temp = (int)(((double)end-(double)start)/(double)step+1.50);

   // If desired check to see if the number of columns has changed
      if(nrMatrixCols > 0 && temp != nrMatrixCols)
      {
         ErrorMessage("number of columns is not constant");
         return(ERR);
      }  

   // Update the number of columns
      nrMatrixCols = temp;         
   }
   else
   {
      ErrorMessage("invalid array definition [start:step:end] or [start:end]");
      return(ERR);
   }
  
   return(OK);
}


// Take a string of the form a:b:c or a:b and return an approproate array
// maxDim is used to check for limits for resolve negative indicies
// i.e. -n means maxDim-n

short EvaluateArrayIndexStr(Interface *itfc, char *indexStr, long maxDim, Variable *result)
{
   float **mat;
   char subStr[MAX_STR];

// Figure out what form the index expression will take
   if(strchr(indexStr,':'))
   {    
       int len = strlen(indexStr);
       if(indexStr[0] == '[')
       {
          strcpy(subStr,indexStr+1);
          subStr[len-2] = '\0';
       }
		 float start = 0;
		 float end = 0;
		 float step = 0;
		 long nrMatrixCols = 0;
       if(GetArraySpecs(itfc, subStr, maxDim, nrMatrixCols, start, step, end) == ERR)
          return(ERR);

       if(!(mat = MakeMatrix2D(nrMatrixCols,1)))
       {
          ErrorMessage("insufficient memory to allocate matrix");
          return(ERR);
       }	

       // Set the elements of each row in the newly formed matrix
       for(long i = 0; i < nrMatrixCols; i++)
       { 
          mat[0][i] = (start + i*step);
       } 
       result->AssignMatrix2D(mat,nrMatrixCols,1);
       return(result->GetType());
    }
    else // A single index e.g. 2 or 'idx' or an explicit array [1,2,3]
    {
      if(Evaluate(itfc,RESPECT_ALIAS,indexStr,result) == ERR)
         return(ERR);

      // Check for negative indicies and correct

      switch(result->GetType())
      {
         case(FLOAT32): // A single index
         {
            float inxf = result->GetReal();
            if(inxf < 0)
               result->SetReal(maxDim+inxf);   
            break;
         }

         case(MATRIX2D): // A defined array
         {
            long w = result->GetDimX();
            long h = result->GetDimY();
            long d = result->GetDimZ();
            long q = result->GetDimQ();
            float **m = result->GetMatrix2D();
            if(h != 1 || d != 1 || q != 1)
            {
               ErrorMessage("invalid index array - should a row vector");
               return(ERR);
            }
            for(long i = 0; i < w; i++)
            {
               if(m[0][i] < 0) 
                  m[0][i] = maxDim+m[0][i];
            }
            break;
         }

         case(UNQUOTED_STRING): // Special case '~' or ':'
            break;

         default:
         {
            ErrorMessage("invalid index specified - should be an integer or an array");
            return(ERR);
         }

      }
      return(result->GetType());
    }
}

/*****************************************************************************
* Interrogates a string which is an expression for the row of a matrix
* This can be an explicit matrix e.g. [1,2,3,4] or it can be specified
* by a range [1:0.2:10]. This routine returns the type of specification 
* and in the case of a range, the start, step and end values. The number
* of columns is returned in both cases. Checks are made to see if the 
* number of columns is consistent with a previous row. If they are not
* then an error is returned.
*
* The type of array definition is returned as either:
* SPECIFIED or RANGE
*
******************************************************************************/

short CheckMatrixRow(Interface *itfc, char *row, CArg &cRow, long &nrMatrixCols, float &start, float &step, float &end, short &rowType)
{
   int nrTerms;
   Variable result;

   cRow.Init(':');
   nrTerms = cRow.Count(row);

// The row has been specified using the syntax [start:step:end]  
   if(nrTerms > 1)
   {
      char *str;

   // Extract arguments - they can be expressions so evaluate them     
      if(nrTerms == 3)
      {        
      // Extract the start value
         str = cRow.Extract(1);
     //    strcpy(str,cRow.Extract(1));
         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
         {
            ErrorMessage("start value for array should be float");
            return(ERR);
         }   
         start = result.GetReal(); 
      // Extract the step value
         str = cRow.Extract(2);

         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
         {
            ErrorMessage("step size value for array should be float");
            return(ERR);
         } 
         step = result.GetReal(); 
         if(step == 0)
         {
            ErrorMessage("zero step size");
            return(ERR);
         }
      // Extract the end value
         str = cRow.Extract(3);

         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
         {
            ErrorMessage("end value for array should be float");
            return(ERR);
         }  
         end = result.GetReal();      

      // Work out number of columns in matrix from the array definition
         long temp = int(1.000001*(end-start)/step)+1;
         if(temp < 0)
         {
            ErrorMessage("invalid array step sign");
            return(ERR);
         }
      // If desired check to see if the number of columns has changed
         if(nrMatrixCols > 0 && temp != nrMatrixCols)
         {
            ErrorMessage("number of columns is not constant");
            return(ERR);
         } 

      // Update the number of columns
         nrMatrixCols = temp;         
         rowType = RANGE; 
      }
      else if(nrTerms == 2)
      {
      // Extract the start value
         str = cRow.Extract(1);
         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
         {
            ErrorMessage("start value for array should be float");
            return(ERR);
         }   
         start = result.GetReal();        

      // Set the step value to 1
         step = 1.0; 
      // Extract the end value
         str = cRow.Extract(2);

         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT32)
         {
            ErrorMessage("end value for array should be float");
            return(ERR);
         }  
         end = result.GetReal();  
         if(end < start)
         {
            ErrorMessage("end value must be greater or equal to start value");
            return(ERR);
         }
      // Work out number of columns in matrix from the array definition
      // The term 1.000001 ensures that rounding errors don't occur in the
      // calculation 
       //  long temp = int(1.000001*(end-start)/step)+1;
      long temp = (int)(((double)end-(double)start)/(double)step+1.50);

      // If desired check to see if the number of columns has changed
         if(nrMatrixCols > 0 && temp != nrMatrixCols)
         {
            ErrorMessage("number of columns is not constant");
            return(ERR);
         }  

      // Update the number of columns
         nrMatrixCols = temp;         

         rowType = RANGE; 
      }
      else
      {
         ErrorMessage("invalid array definition [start:step:end] or [start:end]");
         return(ERR);
      }
          
   }
// The row must include all the elements so count them to find number of columns  
   else
   {
      cRow.Init(',');
      long temp = cRow.Count(row);
      if(nrMatrixCols > 0 && temp != nrMatrixCols) // Check to see that nrCols is const.
      {
         ErrorMessage("number of columns is not constant");
         return(ERR);
      }
      nrMatrixCols = temp;         
      rowType = SPECIFIED; 
   }
   return(OK);
}


short CheckMatrixRow(Interface *itfc, char *row, CArg &cRow, long &nrMatrixCols, double &start, double &step, double &end, short &rowType)
{
   int nrTerms;
   Variable result;

   cRow.Init(':');
   nrTerms = cRow.Count(row);

// The row has been specified using the syntax [start:step:end]  
   if(nrTerms > 1)
   {
      char *str;

   // Extract arguments - they can be expressions so evaluate them     
      if(nrTerms == 3)
      {        
      // Extract the start value
         str = cRow.Extract(1);
         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT64)
         {
            ErrorMessage("start value for array should be double precision");
            return(ERR);
         }   
         start = result.GetDouble(); 
      // Extract the step value
         str = cRow.Extract(2);

         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT64)
         {
            ErrorMessage("step size value for array should be double precision");
            return(ERR);
         } 
         step = result.GetDouble(); 
         if(step == 0)
         {
            ErrorMessage("zero step size");
            return(ERR);
         }
      // Extract the end value
         str = cRow.Extract(3);

         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT64)
         {
            ErrorMessage("end value for array should be double precision");
            return(ERR);
         }  
         end = result.GetDouble();      

      // Work out number of columns in matrix from the array definition
    //     long temp = int(1.000001*(end-start)/step)+1;
      long temp = (int)(((double)end-(double)start)/(double)step+1.50);

         if(temp < 0)
         {
            ErrorMessage("invalid array step sign");
            return(ERR);
         }
      // If desired check to see if the number of columns has changed
         if(nrMatrixCols > 0 && temp != nrMatrixCols)
         {
            ErrorMessage("number of columns is not constant");
            return(ERR);
         } 

      // Update the number of columns
         nrMatrixCols = temp;         
         rowType = RANGE; 
      }
      else if(nrTerms == 2)
      {
      // Extract the start value
         str = cRow.Extract(1);
         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT64)
         {
            ErrorMessage("start value for array should be double precision");
            return(ERR);
         }   
         start = result.GetDouble();        

      // Set the step value to 1
         step = 1.0; 
      // Extract the end value
         str = cRow.Extract(2);

         if(Evaluate(itfc,RESPECT_ALIAS,str,&result) != FLOAT64)
         {
            ErrorMessage("end value for array should be double precision");
            return(ERR);
         }  
         end = result.GetDouble();        
         if(end < start)
         {
            ErrorMessage("end value must be greater than or equal to start value");
            return(ERR);
         }
      // Work out number of columns in matrix from the array definition
      // The term 1.000001 ensures that rounding errors don't occur in the
      // calculation 
       //  long temp = int(1.000001*(end-start)/step)+1;
      long temp = (int)(((double)end-(double)start)/(double)step+1.50);

      // If desired check to see if the number of columns has changed
         if(nrMatrixCols > 0 && temp != nrMatrixCols)
         {
            ErrorMessage("number of columns is not constant");
            return(ERR);
         }  

      // Update the number of columns
         nrMatrixCols = temp;         

         rowType = RANGE; 
      }
      else
      {
         ErrorMessage("invalid array definition [start:step:end] or [start:end]");
         return(ERR);
      }
          
   }
// The row must include all the elements so count them to find number of columns  
   else
   {
      cRow.Init(',');
      long temp = cRow.Count(row);
      if(nrMatrixCols > 0 && temp != nrMatrixCols) // Check to see that nrCols is const.
      {
         ErrorMessage("number of columns is not constant");
         return(ERR);
      }
      nrMatrixCols = temp;         
      rowType = SPECIFIED; 
   }
   return(OK);
}


