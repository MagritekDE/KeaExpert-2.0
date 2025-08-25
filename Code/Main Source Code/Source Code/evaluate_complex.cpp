#include "stdafx.h"
#include "evaluate_complex.h"
#include "2DFunctions.h"
#include "3DFunctions.h"
#include "4DFunctions.h"
#include "allocate.h"
#include "cArg.h"
#include "command.h"
#include "trace.h"
#include "evaluate.h"
#include "evaluate_simple.h"
#include "globals.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "inset.h"
#include "interface.h"
#include "list_functions.h"
#include "mymath.h"
#include "operators.h"
#include "structure.h"
#include "plot.h"
#include "plot1dCLI.h"
#include "PlotWindow.h"
#include "TraceParCLI.h"
#include "variablesOther.h"
#include <assert.h>
#include <math.h>
#include "memoryLeak.h"

short TransposeMatrixVar(Variable *data);
short TransposeDMatrixVar(Variable *data);
short TransposeCMatrixVar(Variable *data);
short ProcessPostUnitaryOperation(Interface *itfc,Variable *data, unsigned char operation, char *operand);
short ProcessUnitaryOperation(Interface *itfc, Variable *var, unsigned char operation);
short ProcessBinaryOperation(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation);
short GetFunctionOperand(Interface *itfc, char *function, char *expression, short &i, Variable *result);
short EvaluateStructureArrayOperand(Interface *itfc, char *function, char *expression, short &i, Variable *result);
short GetArrayOperand(Interface *itfc, char *function, char *expression, short &i, Variable *result);
short GetSimpleOperand(Interface *itfc, char *name, char *expression, short &i, Variable *result);

short NullNullEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short ListNullEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short NullListEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short FloatFloatEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short CompCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short CompFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short FloatCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short StrStrEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short ListListEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short ListStrEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short StrListEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation);
short ClassClassItemEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation);
short StructureClassItemEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation);
short StructureStructureEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation);
//short StructureStringEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation);


short ClassClassEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation);

short DoubleDoubleEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short DoubleFloatEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);
short FloatDoubleEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation);

/****************
   Operation availablility table (ok=present no=not done inv=invalid)
  
              float   double   complex   matrix   dmatrix   cmatrix   matrix3d   cmatrix3d   matrix4d   cmatrix4d structure  class
    float      ok      ok        ok        ok       ok         ok         ok         ok          ok         ok       inv      inv
    double     ok      ok        no        no       ok         no         no         no          no         no       inv      inv
    complex    ok      no        ok        ok       no         ok         ok         ok          ok         ok       inv      inv
    matrix     ok      no        ok        ok       no         ok         inv        inv         inv        inv      inv      inv
    dmatrix    ok      ok        no        no       ok         ok         ok         ok          ok         ok       inv      inv
    cmatrix    ok      no        ok        ok       no         ok         inv        inv         inv        inv      inv      inv
    matrix3d   ok      no        ok        inv      inv        ok         ok         inv         inv        inv      inv      inv
    cmatrix3d  ok      no        ok        inv      inv        ok         ok         inv         inv        inv      inv      inv
    matrix4d   ok      no        ok        inv      inv        inv        inv        ok          ok         inv      inv      inv
    cmatrix4d  ok      no        ok        inv      inv        inv        inv        ok          ok         inv      inv      inv
    structure  inv     inv       inv       inv      inv        inv        inv        inv         inv        inv       no      no
    class      inv     inv       inv       inv      inv        inv        inv        inv         inv        inv       no      ok
****************/

typedef struct
{
   unsigned char   op;         // Current operator (+, -, *, / ...)
   char            type;       // Type of operator (u/b)
}
OpStk; // Operation stack - used by Evaluate

#define STACK_SIZE 10

short EvaluateComplexExpression(Interface *itfc, char *expression, Variable *resVar)
{
   Variable data[STACK_SIZE];
   OpStk  opStk[STACK_SIZE];
   short opCnt = -1;  // Operation pointer
   short datCnt = -1; // Data pointer
   short i,len;
   short result = OK;
   char* operand;

// All interfaces must be defined
   if(!itfc)
     assert(false);

// Check for an empty 'expression'
   len = strlen(expression);
   if(len == 0)
   {
      ErrorMessage("empty expression");
      return(ERR);
   }

// Initialise operator stacks
   for(i = 0; i < STACK_SIZE-1; i++)
   { 
      opStk[i].op = '&';
      opStk[i].type = BINARY_OP;
   }

// Make space for operand
   operand = new char[len+1];

// Work our way through 'expression' evaluating as we go.
// i points to the character we are currently interpreting
   i = 0;   
   while(true)
   {
   // Check for too complex an expression
      if(opCnt+2 == STACK_SIZE || datCnt+2 == STACK_SIZE)
      {
         ErrorMessage("expression stack overflow");
         result = ERR;
         goto ex;
      }
   // Get the next operator and place the result on the operator stack **********       
       SkipWhiteSpace(expression,i);
       if(CheckForSign(expression,i) == -1)
       {
          opStk[++opCnt].op = NEG;
          opStk[opCnt].type = UNITARY_OP;
       }

   // Get next operand and place on data stack ***********************************
   // Check for bracketed expression e.g. (2+4/5) 
       SkipWhiteSpace(expression,i);                
       if(expression[i] == '(') 
       {
          result = EvaluateBracketedExpression(itfc,expression,i,&data[++datCnt]);
       }
       
   // Extract next operand e.g. '-12.3', 'v1', a[23], (2+3), sin(12*3)     
       else  
       {
          short datType;

          datType = GetNextOperand(i,expression,operand);
          if(datType == OPERATOR_TOKEN)
          {
             ErrorMessage("invalid expression: %s",expression);
             result = ERR;
             goto ex;
          }
        
        // Check and evaluate array operand here e.g. v1[23] or m1[1,2] or m1[1,~]
          if(expression[i] == '[')
          {
				 if(opCnt >= 0 && opStk[opCnt].op == ARROW)
					 result = EvaluateStructureArrayOperand(itfc,operand,expression,i,&data[++datCnt]);
				 else
				    result = EvaluateArrayOperand(itfc,operand,expression,i,&data[++datCnt]);
		    }  

        // Check and evaluate function here e.g. log(y)      
		    else if(expression[i] == '(') 
		    {
             if(opCnt >= 0 && opStk[opCnt].op == ARROW)
                result = GetFunctionOperand(itfc,operand,expression,i,&data[++datCnt]); 
             else
                result = EvaluateFunctionOperand(itfc,operand,expression,i,&data[++datCnt]); 
          }
          
        // No brackets so must be a simple operand e.g. 12.3, "Hi", var
          else 
          {
             if(opCnt >= 0 && opStk[opCnt].op == ARROW)
                result = GetSimpleOperand(itfc,operand,expression,i,&data[++datCnt]); 
             else
                result = EvaluateSimpleOperand(itfc,operand,datType,&data[++datCnt]);   
          }
       }
	    if(result == ERR || result == ABORT) goto ex;

   // Check for an operator
       result = GetNextOperator(i,expression,opStk[++opCnt].op);
	    if(result == ERR) goto ex;

   // Check for and if found apply post unitary operator to top of stack (e.g. transpose)
       result = ProcessPostUnitaryOperation(itfc,&data[datCnt],opStk[opCnt].op,operand);
	    if(result == ERR) goto ex;

   // Get the next operator if unitary operator found
       if(result == FOUND)
          result = GetNextOperator(i,expression,opStk[opCnt].op);
	    if(result == ERR) goto ex;
	    
       
   // Process accumulated expression; either (a op b) or (-a) ***********************
       for(;;)
       {
	       if(datCnt >= 0 && opCnt >= 1)
	       {
	          if((precedence[opStk[opCnt-1].op] >= precedence[opStk[opCnt].op]))
	          {
	             if(opStk[opCnt-1].type == UNITARY_OP)
	             {  
	                result = ProcessUnitaryOperation(itfc, &data[datCnt],opStk[opCnt-1].op);
	             }
	             else if(opStk[opCnt-1].type == BINARY_OP)
	             {
	                result = ProcessBinaryOperation(itfc, &data[datCnt-1],&data[datCnt],opStk[opCnt-1].op);
		             datCnt--;
		          }
		          if(result == ERR) goto ex;
                opStk[opCnt-1] = opStk[opCnt];
                opStk[opCnt] = opStk[opCnt+1];		// ?? Necessary             
	             opCnt--;		          
		       }
		       else
		       {
		          break;
		       }
	       } 
	       else
	       {
	          break;
	       }
       }
       if(i >= len) break;
    }

// Check for dangling operator
   if(opStk[0].op != '&')
   {
      ErrorMessage("invalid expression: %s",expression);
      result = ERR;
      goto ex;
   }

// Absorb top of stack into return variable   
   resVar->Assign(&data[0]);
   data[0].NullData();

// Only 1 value can be returned by a complex expression
//   if(itfc->nrRetValues == 0)
//      itfc->nrRetValues = 1; 

// Free operand memory
ex: delete [] operand;
   
// Return error or data type
   if(result < 0)
      return(result);
   else 
      return(resVar->GetType());
}

short ProcessPostUnitaryOperation(Interface *itfc, Variable *data, unsigned char operation, char *operand)
{
   if(operation == '\'') // Transpose operator
   {
      itfc->nrRetValues = 1;
      switch(data->GetType())
      {
         case(MATRIX2D):
         {
	         TransposeMatrixVar(data);
	         return(FOUND);
         }  
         case(DMATRIX2D):
         {
	         TransposeDMatrixVar(data);
	         return(FOUND);
         }
         case(CMATRIX2D):
         {
	         TransposeCMatrixVar(data);
	         return(FOUND);
         } 
         default:
         {
	         ErrorMessage("can't take the transpose of '%s'",operand);
	         return(ERR);
         } 
      }
   }
   return(NOT_FOUND);
}


short TransposeMatrixVar(Variable *data)
{
    float **trans;
    float **mat;
    
    long cols = data->GetDimX();
    long rows = data->GetDimY();
    
    if(!(trans = MakeMatrix2D(rows,cols)))
    {
       ErrorMessage("unable to allocate memory for transposed matrix");
       return(ERR);
    }

    mat = data->GetMatrix2D();
    
    for(long y = 0; y < rows; y++)
    {
       for(long x = 0; x < cols; x++)
       {
          trans[x][y] = mat[y][x];
       }
    }
    
    data->AssignMatrix2D(trans,rows,cols);

    return(OK);
 }


short TransposeDMatrixVar(Variable *data)
{
    double **trans;
    double **mat;
    
    long cols = data->GetDimX();
    long rows = data->GetDimY();
    
    if(!(trans = MakeDMatrix2D(rows,cols)))
    {
       ErrorMessage("unable to allocate memory for transposed matrix");
       return(ERR);
    }

    mat = data->GetDMatrix2D();
    
    for(long y = 0; y < rows; y++)
    {
       for(long x = 0; x < cols; x++)
       {
          trans[x][y] = mat[y][x];
       }
    }
    
    data->AssignDMatrix2D(trans,rows,cols);

    return(OK);
 }
 
short TransposeCMatrixVar(Variable *data)
{
    complex **trans;
    complex **cmat;
    
    long cols = data->GetDimX();
    long rows = data->GetDimY();

    
    if(!(trans = MakeCMatrix2D(rows,cols)))
    {
       ErrorMessage("unable to allocate memory for transposed matrix");
       return(ERR);
    }
    
    cmat = data->GetCMatrix2D();

    for(long y = 0; y < rows; y++)
    {
       for(long x = 0; x < cols; x++)
       {
          trans[x][y] = cmat[y][x];
       }
    }
    
    data->AssignCMatrix2D(trans,rows,cols);
 
    return(OK);
 }

      
short ProcessUnitaryOperation(Interface *itfc, Variable *var, unsigned char operation)
{  
   short result;
   char* data = var->GetData();
   
   if(operation == NEG)
   {      
      itfc->nrRetValues = 1;

		switch(var->GetType())
		{
			 case(FLOAT32):
          {
             float *fdata = (float*)data;
			    *fdata *= -1;
			    break;
          }
			 case(FLOAT64):
          {
             double *fdata = (double*)data;
			    *fdata *= -1;
			    break;
          }
			 case(COMPLEX):
          {
             complex* cdata = (complex*)data;
			    cdata->r *= -1;
			    cdata->i *= -1;
			    break;
          }
			 case(MATRIX2D):
          {
             long sx = var->GetDimX();
             long sy = var->GetDimY();
             float **mdata = (float**)data;
			    for(long y = 0; y < sy; y++) 
			       for(long x = 0; x < sx; x++) 
			          mdata[y][x] *= -1;
			    break;
          }
			 case(DMATRIX2D):
          {
             long sx = var->GetDimX();
             long sy = var->GetDimY();
             double **mdata = (double**)data;
			    for(long y = 0; y < sy; y++) 
			       for(long x = 0; x < sx; x++) 
			          mdata[y][x] *= -1;
			    break;
          }
			 case(CMATRIX2D):
			 {
             long sx = var->GetDimX();
             long sy = var->GetDimY();
             complex **cdata = (complex**)data;

			    for(long y = 0; y < sy; y++) 
			    {
			       for(long x = 0; x < sx; x++) 
			       {
			          cdata[y][x].r *= -1;
			          cdata[y][x].i *= -1;
			       }
			    }
			    break;
			 }
			 case(MATRIX3D):
			 {
             long sx = var->GetDimX();
             long sy = var->GetDimY();
             long sz = var->GetDimZ();
             float ***mdata = (float***)data;
			    for(long z = 0; z < sz; z++) 
             {
			       for(long y = 0; y < sy; y++) 
                {
			          for(long x = 0; x < sx; x++)
                   {
			             mdata[z][y][x] *= -1;
                   }
                }
             }
			    break;
			 }	
			 case(CMATRIX3D):
			 {
             long sx = var->GetDimX();
             long sy = var->GetDimY();
             long sz = var->GetDimZ();
             complex ***cdata = (complex***)data;

			    for(long z = 0; z < sz; z++) 
			    {
			       for(long y = 0; y < sy; y++) 
			       {
			          for(long x = 0; x < sx; x++) 
			          {
			             cdata[z][y][x].r *= -1;
			             cdata[z][y][x].i *= -1;
			          }
			       }
			    }
			    break;	
			 }                                           		                      	                      
		}
		result = OK;
   }
   else
   {
   	ErrorMessage("'%s' is an undefined operation",GetOperationString(var->GetType(),operation));
	   result = ERR;
	}
   return(result);
}     


/*********************************************************************************
   Perform a binary operation between two variables dataLeft, dataRight
*********************************************************************************/

short ProcessBinaryOperation(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   short result;
   short type = dataLeft->GetType() + ((dataRight->GetType())<<8);
 
   itfc->nrRetValues = 1;

	switch(type)
	{
     case(NULL_VARIABLE + (NULL_VARIABLE<<8)):
        result = NullNullEvaluate(dataLeft,dataRight,operation);
        break; 
     case(NULL_VARIABLE + (LIST<<8)):
        result = NullListEvaluate(dataLeft,dataRight,operation);
        break; 
     case(LIST + (NULL_VARIABLE<<8)):
        result = ListNullEvaluate(dataLeft,dataRight,operation);
        break; 
	  case(UNQUOTED_STRING + (UNQUOTED_STRING<<8)):
	     result = StrStrEvaluate(dataLeft,dataRight,operation);
	     break;	
	  case(LIST + (LIST<<8)):
	     result = ListListEvaluate(dataLeft,dataRight,operation);
	     break;
	  case(UNQUOTED_STRING + (LIST<<8)):
	     result = StrListEvaluate(dataLeft,dataRight,operation);
	    break;	
	  case(LIST + (UNQUOTED_STRING<<8)):
	     result = ListStrEvaluate(dataLeft,dataRight,operation);
	     break;	
	  case(FLOAT32 + (FLOAT32<<8)):
	     result = FloatFloatEvaluate(dataLeft,dataRight,operation);
	     break;
	  case(FLOAT64 + (FLOAT32<<8)):
	     result = DoubleFloatEvaluate(dataLeft,dataRight,operation);
	     break;
	  case(FLOAT32 + (FLOAT64<<8)):
	     result = FloatDoubleEvaluate(dataLeft,dataRight,operation);
	     break;
	  case(FLOAT64 + (FLOAT64<<8)):
	     result = DoubleDoubleEvaluate(dataLeft,dataRight,operation);
	     break;
     case(FLOAT32 + (COMPLEX<<8)):
        result = FloatCompEvaluate(dataLeft,dataRight,operation);
        break;	
     case(COMPLEX + (FLOAT32<<8)):
        result = CompFloatEvaluate(dataLeft,dataRight,operation);
        break;	
     case(COMPLEX + (COMPLEX<<8)):
        result = CompCompEvaluate(dataLeft,dataRight,operation);
        break;


// 2D matrix
     case(FLOAT32 + (MATRIX2D<<8)):
        result = FloatMatrixEvaluate(dataLeft,dataRight,operation);
        break;	
     case(MATRIX2D + (FLOAT32<<8)):
        result = MatrixFloatEvaluate(dataLeft,dataRight,operation);
        break;	
     case(COMPLEX + (MATRIX2D<<8)):
        result = CompMatrixEvaluate(dataLeft,dataRight,operation);
        break;	
     case(MATRIX2D + (COMPLEX<<8)):
         result = MatrixCompEvaluate(dataLeft,dataRight,operation);
        break;	        
     case(COMPLEX + (CMATRIX2D<<8)):
        result = CompCMatrixEvaluate(dataLeft,dataRight,operation);
        break;	
     case(CMATRIX2D + (COMPLEX<<8)):
        result = CMatrixCompEvaluate(dataLeft,dataRight,operation);
        break;	         
     case(MATRIX2D + (MATRIX2D<<8)):
        result = MatrixMatrixEvaluate(dataLeft,dataRight,operation);
        break;	         
     case(FLOAT32 + (CMATRIX2D<<8)):
        result = FloatCMatrixEvaluate(dataLeft,dataRight,operation);
        break;	
     case(CMATRIX2D + (FLOAT32<<8)):
        result = CMatrixFloatEvaluate(dataLeft,dataRight,operation);
        break;	
     case(MATRIX2D + (CMATRIX2D<<8)):
        result = MatrixCMatrixEvaluate(dataLeft,dataRight,operation);
        break;
     case(CMATRIX2D + (MATRIX2D<<8)):
        result = CMatrixMatrixEvaluate(dataLeft,dataRight,operation);
        break;	        	        
     case(CMATRIX2D + (CMATRIX2D<<8)):
        result = CMatrixCMatrixEvaluate(dataLeft,dataRight,operation);
        break;

// 2D double matrix
     case(FLOAT32 + (DMATRIX2D<<8)):
        result = FloatDMatrixEvaluate(dataLeft,dataRight,operation);
        break;	
     case(DMATRIX2D + (FLOAT32<<8)):
        result = DMatrixFloatEvaluate(dataLeft,dataRight,operation);
        break;	
     case(FLOAT64 + (DMATRIX2D<<8)):
        result = DoubleDMatrixEvaluate(dataLeft,dataRight,operation);
        break;	
     case(DMATRIX2D + (FLOAT64<<8)):
        result = DMatrixDoubleEvaluate(dataLeft,dataRight,operation);
        break;	
     case(DMATRIX2D + (DMATRIX2D<<8)):
        result = DMatrixDMatrixEvaluate(dataLeft,dataRight,operation);
        break;	

// 3D Matrix
     case(MATRIX3D + (FLOAT32<<8)):
        result = Matrix3DFloatEvaluate(dataLeft,dataRight,operation);
        break;	        	        
     case(FLOAT32 + (MATRIX3D<<8)):
        result = FloatMatrix3DEvaluate(dataLeft,dataRight,operation);
        break; 
     case(CMATRIX3D + (FLOAT32<<8)):
        result = CMatrix3DFloatEvaluate(dataLeft,dataRight,operation);
        break;	        	        
     case(FLOAT32 + (CMATRIX3D<<8)):
        result = FloatCMatrix3DEvaluate(dataLeft,dataRight,operation);
        break;
     case(COMPLEX + (MATRIX3D<<8)):
        result = CompMatrix3DEvaluate(dataLeft,dataRight,operation);
        break; 
     case(MATRIX3D + (COMPLEX<<8)):
        result = Matrix3DCompEvaluate(dataLeft,dataRight,operation);
        break; 
     case(CMATRIX3D + (COMPLEX<<8)):
        result = CMatrix3DComplexEvaluate(dataLeft,dataRight,operation);
        break; 
     case(COMPLEX + (CMATRIX3D<<8)):
        result = ComplexCMatrix3DEvaluate(dataLeft,dataRight,operation);
        break; 
     case(MATRIX3D + (CMATRIX3D<<8)):
        result = Matrix3DCMatrix3DEvaluate(dataLeft,dataRight,operation);
        break;  
     case(CMATRIX3D + (MATRIX3D<<8)):
        result = CMatrix3DMatrix3DEvaluate(dataLeft,dataRight,operation);
        break;
     case(MATRIX3D + (MATRIX3D<<8)):
        result = Matrix3DMatrix3DEvaluate(dataLeft,dataRight,operation);
        break;  
     case(CMATRIX3D + (CMATRIX3D<<8)):
        result = CMatrix3DCMatrix3DEvaluate(dataLeft,dataRight,operation);
        break; 
// 4D matrix
      case(MATRIX4D + (FLOAT32<<8)):
        result = Matrix4DFloatEvaluate(dataLeft,dataRight,operation);
        break;	        	        
     case(FLOAT32 + (MATRIX4D<<8)):
        result = FloatMatrix4DEvaluate(dataLeft,dataRight,operation);
        break; 
     case(CMATRIX4D + (FLOAT32<<8)):
        result = CMatrix4DFloatEvaluate(dataLeft,dataRight,operation);
        break;	        	        
     case(FLOAT32 + (CMATRIX4D<<8)):
        result = FloatCMatrix4DEvaluate(dataLeft,dataRight,operation);
        break;
     case(COMPLEX + (MATRIX4D<<8)):
        result = ComplexMatrix4DEvaluate(dataLeft,dataRight,operation);
        break; 
     case(MATRIX4D + (COMPLEX<<8)):
        result = Matrix4DComplexEvaluate(dataLeft,dataRight,operation);
        break; 
     case(CMATRIX4D + (COMPLEX<<8)):
        result = CMatrix4DComplexEvaluate(dataLeft,dataRight,operation);
        break; 
     case(COMPLEX + (CMATRIX4D<<8)):
        result = ComplexCMatrix4DEvaluate(dataLeft,dataRight,operation);
        break; 
     case(MATRIX4D + (CMATRIX4D<<8)):
        result = Matrix4DCMatrix4DEvaluate(dataLeft,dataRight,operation);
        break;  
     case(CMATRIX4D + (MATRIX4D<<8)):
        result = CMatrix4DMatrix4DEvaluate(dataLeft,dataRight,operation);
        break;
     case(MATRIX4D + (MATRIX4D<<8)):
        result = Matrix4DMatrix4DEvaluate(dataLeft,dataRight,operation);
        break;  
     case(CMATRIX4D + (CMATRIX4D<<8)):
        result = CMatrix4DCMatrix4DEvaluate(dataLeft,dataRight,operation);
        break; 
     case(CLASS + (CLASSITEM<<8)):
        result = ClassClassItemEvaluate(itfc,dataLeft,dataRight,operation);
        break; 
     case(CLASS + (CLASS<<8)):
        result = ClassClassEvaluate(itfc,dataLeft,dataRight,operation);
        break; 
     case(STRUCTURE + (CLASSITEM<<8)):
        result = StructureClassItemEvaluate(itfc,dataLeft,dataRight,operation);
        break; 
      case(STRUCTURE + (STRUCTURE<<8)):
        result = StructureStructureEvaluate(itfc,dataLeft,dataRight,operation);
        break; 

	  default:
	     if(operation == NEQ) // Special case of comparing two dissimilar types
	     {
	        dataLeft->MakeAndSetFloat(1);
	        return(1);
	     }
	     else if(operation == EQ || operation == '=')
	     {
	        dataLeft->MakeAndSetFloat(0);
	        return(0);
	     }
	     ErrorMessage("'%s' is an undefined operation",GetOperationString(type,operation));
	     result = ERR;
	     break;	   
	}
	return(result);
}



/*********************************************************************************
   Evaluate expression of the form "result = r1 operator r2" where 
   r1 & r2  are null variables
*********************************************************************************/

short NullNullEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
   switch(operation)
   {
      case ('=') :
      case (EQ) :
         dataLeft->MakeAndSetFloat(1);
         break;
      case (NEQ) :
         dataLeft->MakeAndSetFloat(0);
         break;         
      default:
         ErrorMessage("'%s' is an undefined operation\n           between nulls",GetOpString(operation));
         return(ERR);
   }
   return(OK);
}


/*********************************************************************************
   Evaluate a LIST operating on a LIST
*********************************************************************************/

short ListListEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   char **lstLeft  = dataLeft->GetList();
   char **lstRight = dataRight->GetList();
   long lenLeft    = dataLeft->GetDimX();
   long lenRight   = dataRight->GetDimX();

   switch(operation)
   {
      case ('+') :
      {
         char **total = JoinLists(lstLeft,lstRight,lenLeft,lenRight);
         dataLeft->AssignList(total,lenRight+lenLeft); 
         break;
      }
      case ('=') :
      case (EQ) :
      {
         int cmp = CompareLists(lstLeft,lstRight,lenLeft,lenRight);
         dataLeft->MakeAndSetFloat(cmp);
         break;
      }
      case (NEQ) :
      {
         int cmp = CompareLists(lstLeft,lstRight,lenLeft,lenRight);
         dataLeft->MakeAndSetFloat(!cmp);
         break;  
      }
      default:
         ErrorMessage("'%s' is an undefined operation\n           between lists",GetOpString(operation));
         return(ERR);
   }
   return(OK);
}


/*********************************************************************************
   Evaluate a LIST operating on a NULL
*********************************************************************************/

short ListNullEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
   switch(operation)
   {
      case ('+') :
         break; 
      case ('=') :
      case (EQ) :
         dataLeft->MakeAndSetFloat(0);
         break; 
      case (NEQ) :
         dataLeft->MakeAndSetFloat(1);
         break; 
      default:
         ErrorMessage("'%s' is an undefined operation\n           between list and null",GetOpString(operation));
         return(ERR);
   }
   return(OK);
}



/*********************************************************************************
   Evaluate a Null operating on a LIST
*********************************************************************************/

short NullListEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
   switch(operation)
   {
      case ('+') :
         dataLeft->Assign(dataRight);
         dataRight->SetNull();
         break; 
      case ('=') :
      case (EQ) :
         dataLeft->MakeAndSetFloat(0);
         break; 
      case (NEQ) :
         dataLeft->MakeAndSetFloat(1);
         break; 
      default:
         ErrorMessage("'%s' is an undefined operation\n           between null and list",GetOpString(operation));
         return(ERR);
   }
   return(OK);
}



/*********************************************************************************
   Evaluate expression of the form "result = r1 operator r2" where 
   r1 & r2  are real scalars
*********************************************************************************/


short FloatFloatEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
	float r1,r2;
	float result;
	
	r1 = dataLeft->GetReal();
	r2 = dataRight->GetReal();
	
	switch(operation)
	{
	   case ('&') :
	      result = nhint(r1) & nhint(r2);
	      break;
	   case ('|') :
	      result = nhint(r1) | nhint(r2);
	      break;
	   case ('=') :
      case (EQ) :
         result = nint(FloatEqual(r1,r2));
         break;
	   case (LEQ) :
	      result = nint(FloatLessEqual(r1,r2));
	      break;
	   case (GEQ) :
	      result = nint(FloatGreaterEqual(r1,r2));
	      break;
	   case (NEQ) :
	      result = nint(FloatNotEqual(r1,r2));
	      break;
	   case ('<') :
	      result = nint(FloatLess(r1,r2));
	      break;
	   case ('>') :
	      result = nint(FloatGreater(r1,r2));
	      break;
	   case ('%') :
         if(nint(r2) == 0)
         {
            ErrorMessage("operand to modulus operator rounds to zero");
            return(ERR);
         }
	      result = nhint(r1) % nhint(r2);
	      break;
	   case ('+') :
	      result = r1 + r2;
	      break;
	   case ('-') :
	      result = r1 - r2;
	      break;
	   case ('*') :
	      result = r1 * r2;
	      break;
	   case ('/') :
         if(r2 == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         }
	      result = r1 / r2;
	      break;
	   case ('^') :
	      result = pow(r1,r2);
	      break;
	   default:
         ErrorMessage("'%s' is an undefined operation between floats",GetOpString(operation));
         return(ERR);
	}

   dataLeft->SetReal(result);

   return(OK);
}


/*********************************************************************************
   Evaluate expression of the form "result = r1 operator r2" where 
   r1 is double and r2 is a float.
*********************************************************************************/


short DoubleFloatEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
	double r1;
	float r2;
	double result;
	
	r1 = dataLeft->GetDouble();
	r2 = dataRight->GetReal();
	
	switch(operation)
	{
	   case ('&') :
	      result = nhint(r1) & nhint(r2);
	      break;
	   case ('|') :
	      result = nhint(r1) | nhint(r2);
	      break;
	   case ('=') :
      case (EQ) : 
         result = nint(DoubleEqual(r1,r2));
         break;
	   case (LEQ) :
	      result = nint(DoubleLessEqual(r1,r2));
	      break;
	   case (GEQ) :
	      result = nint(DoubleGreaterEqual(r1,r2));
	      break;
	   case (NEQ) :
	      result = nint(DoubleNotEqual(r1,r2));
	      break;
	   case ('<') :
	      result = nint(DoubleLess(r1,r2));
	      break;
	   case ('>') :
	      result = nint(DoubleGreater(r1,r2));
	      break;
	   case ('%') :
         if(nint(r2) == 0)
         {
            ErrorMessage("operand to modulus operator rounds to zero");
            return(ERR);
         }
	      result = nhint(r1) % nhint(r2);
	      break;
	   case ('+') :
	      result = r1 + (double)r2;
	      break;
	   case ('-') :
	      result = r1 - (double)r2;
	      break;
	   case ('*') :
	      result = r1 * (double)r2;
	      break;
	   case ('/') :
         if(r2 == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         }
	      result = r1 / (double)r2;
	      break;
	   case ('^') :
	      result = pow(r1,(double)r2);
	      break;
	   default:
         ErrorMessage("'%s' is an undefined operation between double and float",GetOpString(operation));
         return(ERR);
	}

   dataLeft->SetDouble(result);

   return(OK);
}


/*********************************************************************************
   Evaluate expression of the form "result = r1 operator r2" where 
   r1 is float and r2 is a double.
*********************************************************************************/


short FloatDoubleEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
	float r1;
	double r2;
	double result;
	
	r1 = dataLeft->GetReal();
	r2 = dataRight->GetDouble();
	
	switch(operation)
	{
	   case ('&') :
	      result = nhint(r1) & nhint(r2);
	      break;
	   case ('|') :
	      result = nhint(r1) | nhint(r2);
	      break;
	   case ('=') :
      case (EQ) : 
         result = nint(DoubleEqual(r1,r2));
         break;
	   case (LEQ) :
	      result = nint(DoubleLessEqual(r1,r2));
	      break;
	   case (GEQ) :
	      result = nint(DoubleGreaterEqual(r1,r2));
	      break;
	   case (NEQ) :
	      result = nint(DoubleNotEqual(r1,r2));
	      break;
	   case ('<') :
	      result = nint(DoubleLess(r1,r2));
	      break;
	   case ('>') :
	      result = nint(DoubleGreater(r1,r2));
	      break;
	   case ('%') :
         if(nint(r2) == 0)
         {
            ErrorMessage("operand to modulus operator rounds to zero");
            return(ERR);
         }
	      result = nhint(r1) % nhint(r2);
	      break;
	   case ('+') :
	      result = (double)r1 + r2;
	      break;
	   case ('-') :
	      result = (double)r1 - r2;
	      break;
	   case ('*') :
	      result = (double)r1 * r2;
	      break;
	   case ('/') :
         if(r2 == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         }
	      result = (double)r1 / r2;
	      break;
	   case ('^') :
	      result = pow((double)r1,r2);
	      break;
	   default:
         ErrorMessage("'%s' is an undefined operation between float and double",GetOpString(operation));
         return(ERR);
	}

   dataLeft->MakeAndSetDouble(result);

   return(OK);
}


/*********************************************************************************
   Evaluate expression of the form "result = r1 operator r2" where 
   r1 & r2  are double scalars
*********************************************************************************/


short DoubleDoubleEvaluate(Variable *dataLeft,Variable *dataRight, unsigned char operation)
{
	double r1;
	double r2;
	double result;
	
	r1 = dataLeft->GetDouble();
	r2 = dataRight->GetDouble();
	
	switch(operation)
	{
	   case ('&') :
	      result = nhint(r1) & nhint(r2);
	      break;
	   case ('|') :
	      result = nhint(r1) | nhint(r2);
	      break;
	   case ('=') :
      case (EQ) : 
         result = nint(DoubleEqual(r1,r2));
         break;
	   case (LEQ) :
	      result = nint(DoubleLessEqual(r1,r2));
	      break;
	   case (GEQ) :
	      result = nint(DoubleGreaterEqual(r1,r2));
	      break;
	   case (NEQ) :
	      result = nint(DoubleNotEqual(r1,r2));
	      break;
	   case ('<') :
	      result = nint(DoubleLess(r1,r2));
	      break;
	   case ('>') :
	      result = nint(DoubleGreater(r1,r2));
	      break;
	   case ('%') :
         if(nint(r2) == 0)
         {
            ErrorMessage("operand to modulus operator rounds to zero");
            return(ERR);
         }
	      result = nhint(r1) % nhint(r2);
	      break;
	   case ('+') :
	      result = r1 + r2;
	      break;
	   case ('-') :
	      result = r1 - r2;
	      break;
	   case ('*') :
	      result = r1 * r2;
	      break;
	   case ('/') :
         if(r2 == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         }
	      result = r1 / r2;
	      break;
	   case ('^') :
	      result = pow(r1,r2);
	      break;
	   default:
         ErrorMessage("'%s' is an undefined operation between double and double",GetOpString(operation));
         return(ERR);
	}

   dataLeft->SetDouble(result);

   return(OK);
}



/*********************************************************************************
   Evaluate expression of the form "result = c1 operator c2" where 
   c1 & c2  are complex scalars
*********************************************************************************/
                    
short CompCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
	float denom;
	complex c1,c2;
	complex result;
	
	c1 = dataLeft->GetComplex();
	c2 = dataRight->GetComplex();
	
	switch(operation)
	{
	   case (EQ) :
	   case ('=') :
	      dataLeft->MakeAndSetFloat((FloatEqual(c1.r,c2.r) && FloatEqual(c1.i,c2.i)));
	      return(OK);
	      break;	
	   case (NEQ) :
	      dataLeft->MakeAndSetFloat((FloatNotEqual(c1.r,c2.r) || FloatNotEqual(c1.i,c2.i)));
	      return(OK);
	      break;		      
	   case ('+') :
	      result.r = c1.r + c2.r;
	      result.i = c1.i + c2.i;
	      break;
	   case ('-') :
	      result.r = c1.r - c2.r;
	      result.i = c1.i - c2.i;	 
	      break;
	   case ('*') :
	      result.r = c1.r*c2.r - c1.i*c2.i;
	      result.i = c1.r*c2.i + c1.i*c2.r;	
	      break;
	   case ('/') :
	      denom  = c2.r*c2.r + c2.i*c2.i;
         if(denom == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         }
	      result.r = (c1.r*c2.r+c1.i*c2.i)/denom;
	      result.i = (c1.i*c2.r-c1.r*c2.i)/denom;	
	      break;

	   default:
         ErrorMessage("'%s' is an undefined operation\n           between complex numbers",GetOpString(operation));
         return(ERR);
	}
   dataLeft->SetComplex(result);
   return(OK);
}



/*********************************************************************************
   Evaluate expression of form "result = r operator c" where 
   r is real and c is complex
*********************************************************************************/
    
short FloatCompEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
	complex result;
	complex c;
	float r,denom;
	
	r = dataLeft->GetReal();
	c = dataRight->GetComplex();
	
	switch(operation)
	{
	   case ('+') :
	      result.r = r + c.r;
	      result.i =     c.i;
	      break;
	   case ('-') :
	      result.r = r - c.r;
	      result.i =   - c.i;
	      break;
	   case ('*') :
	      result.r = r * c.r;
	      result.i = r * c.i;
	      break;
	   case ('/') :
	      denom = c.r*c.r + c.i*c.i;
         if(denom == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         }
	      result.r = (r*c.r)/denom;
	      result.i = (-r*c.i)/denom;	
	      break;
	   case ('=') :
      case (EQ) :
      {
         dataLeft->MakeAndSetFloat(((c.i == 0) && nint(FloatEqual(c.r,r))));
         return(OK);
         break;
      }
	   case (NEQ) :
      {
	      dataLeft->MakeAndSetFloat(((c.i != 0) || nint(FloatNotEqual(c.r,r))));
         return(OK);
         break;
      }
	   default:
         ErrorMessage("'%s' is an undefined operation between\n           real and complex types",GetOpString(operation));
         return(ERR);
	}
   dataLeft->MakeAndSetComplex(result);
   return(OK);
}		                                


/*********************************************************************************
   Evaluate expression of form "result = c operator r" where 
   r is real and c is complex
*********************************************************************************/
 
short CompFloatEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
	complex result;
	complex c;
	float r;
	
	c = dataLeft->GetComplex();
	r = dataRight->GetReal();
	
	switch(operation)
	{
	   case ('+') :
	      result.r = c.r + r;
	      result.i = c.i;
	      break;
	   case ('-') :
	      result.r = c.r - r;
	      result.i = c.i;
	      break;
	   case ('*') :
	      result.r = c.r * r;
	      result.i = c.i * r;
	      break;
	   case ('/') :
         if(r == 0)
         {
            ErrorMessage("divide by zero");
            return(ERR);
         }
	      result.r = c.r / r;
	      result.i = c.i / r;
	      break;
      case ('=') :
      case (EQ) :
      {
         dataLeft->MakeAndSetFloat(((c.i == 0) && nint(FloatEqual(c.r,r))));
         return(OK);
         break;
      }
	   case (NEQ) :
      {
	      dataLeft->MakeAndSetFloat(((c.i != 0) || nint(FloatNotEqual(c.r,r))));
         return(OK);
         break;
      }
	   default:
         ErrorMessage("'%s' is an undefined operation between\n           complex and real types.",GetOpString(operation));
         return(ERR);
	}
   dataLeft->SetComplex(result);
   return(OK);
}	


/*********************************************************************************
   Evaluate a STRING operating on a STRING
*********************************************************************************/
 
short  StrStrEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   int cmp;
   char *lStr = dataLeft->GetData();
   char *rStr = dataRight->GetData();

   switch(operation)
   {
      case ('+') :
      {
         long len = strlen(lStr) + strlen(rStr);
         char *total = new char[len+1];
         strcpy(total,lStr);
         strcat(total,rStr);
         dataLeft->AssignString(total);
         break;
      }
      case ('=') :
      case (EQ) :
         cmp = (strcmp(lStr,rStr) == 0);
         dataLeft->MakeAndSetFloat(cmp);
         break;
      case (NEQ) :
         cmp = (strcmp(lStr,rStr) != 0);
         dataLeft->MakeAndSetFloat(cmp);
         break;         
      default:
         ErrorMessage("'%s' is an undefined operation\n           between strings '%s' and '%s'",
            GetOpString(operation),dataLeft->GetString(),dataRight->GetString());
         return(ERR);
   }
   return(OK);
}


/*********************************************************************************
   Evaluate a STRING operating on a LIST
*********************************************************************************/
 
short StrListEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   switch(operation)
   {
      case ('+') :
      {
         char **lstRight = dataRight->GetList();
         long lstLen = dataRight->GetDimX();
         InsertStringIntoList(dataLeft->GetString(), &lstRight, lstLen, 0);
         dataLeft->AssignList(lstRight,lstLen+1);
         dataRight->SetNull();
         break;  
      }
      case ('=') :
      case (EQ) :
         dataLeft->MakeAndSetFloat(0);
         break;
      case (NEQ) :
         dataLeft->MakeAndSetFloat(1);
         break;
      default:
         ErrorMessage("'%s' is an undefined operation\n           between string & list",GetOpString(operation));
         return(ERR);
   }
   return(OK);
}


/*********************************************************************************
   Evaluate a LIST operating on a STRING
*********************************************************************************/
 
short ListStrEvaluate(Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   switch(operation)
   {
      case ('+') :
         dataLeft->AddToList(dataRight->GetString());
         break; 
      case ('=') :
      case (EQ) :
         dataLeft->MakeAndSetFloat(0);
         break;
      case (NEQ) :
         dataLeft->MakeAndSetFloat(1);
         break;
      default:
         ErrorMessage("'%s' is an undefined operation\n           between list & string",GetOpString(operation));
         return(ERR);
   }
   return(OK);
}



/*********************************************************************************
   Evaluate a STRUCTURE operating on a STRUCTURE result is a STRUCTURE
*********************************************************************************/

short StructureStructureEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   Variable *structLeft  = dataLeft;
   Variable *structRight = dataRight;
   Variable *total;


   switch(operation)
   {
      case ('+') :
      {
         total = JoinStructures(structLeft,structRight);
         dataLeft->AssignStructure(total);
         break;
      }
      
      default:
         ErrorMessage("'%s' is an undefined operation\n           between structures",GetOpString(operation));
         return(ERR);
   }
   return(OK);
}



   /*
short StructureStringEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   Variable *strucData = dataLeft->GetStruct();
   char *member = dataRight->GetString();

   switch(operation)
   { 
      case(ARROW):
      {
         Variable *var;
         for(var = strucData->next; var != NULL; var = var->next)
         {
            if(!strcmp(var->GetName(),member))
            {
               Variable *temp = new Variable;
               if(CopyVariable(temp,var,FULL_COPY) == ERR)
                  return(ERR);
               dataLeft->Assign(temp);
               return(OK);
            }
         }
         ErrorMessage("'%s' is not a member of '%s'",member,dataLeft->GetName());
         return(ERR);
      }
   }

   return(ERR);
}*/

/*********************************************************************************
   Evaluate a STRUCTURE operating on a CLASS result is variant
*********************************************************************************/

short StructureClassItemEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
	extern short CheckForMacroProcedure(Interface *itfc, char *command, char *arg);

   Variable *strucData = dataLeft->GetStruct();
	char *structName = dataLeft->GetName();
   ClassItem *rData = (ClassItem*)dataRight->GetData();
   char *member = (char*)rData->name;

   switch(operation)
   { 
      case(ARROW):
      {
         Variable *var;
         for(var = strucData->next; var != NULL; var = var->next) // Loop over the structure items
         {
            if(!strcmp(var->GetName(),member)) // Find the member
            {
					if(rData->type == CLASS_ARRAY)  // Its an array a->b[index]
					{
					   if(!strcmp(rData->args,"")) // No arguments
                  {
							ErrorMessage("Array index not provided");
								return(ERR);
						}
						Variable* result = new Variable();
						short spos = 0;

                  EvaluateArrayOperand(itfc, rData->name, var, rData->args, result);
				//		dataLeft->SetName(result->GetName());
						dataLeft->Assign(result);
						result->NullData();
						delete result;
						return(OK);							
					}
					else if(rData->type == CLASS_FUNCTION)  // Is it calling a class function a->func()
					{
					   if(!strcmp(rData->args,"")) // No arguments
                  {
							if(var->GetType() == UNQUOTED_STRING) // Assume a function
							{
								CText func = var->GetString();
								long pos = func.Search(0,':'); // Functions have a colon
								if(pos != -1) // Pass as argument alias(structureName)
								{
									CText args = "alias(";
									args = args + structName;
									args = args + ")";			
									short r = CheckForMacroProcedure(itfc,func.Str(),args.Str());				
									if(r == 0 && itfc->nrRetValues > 0)
										CopyVariable(dataLeft,&itfc->retVar[1],FULL_COPY);
									return(r);
								}
							}
							Variable *temp = new Variable; // A normal variable
							if(CopyVariable(temp,var,FULL_COPY) == ERR)
								return(ERR);
							dataLeft->Assign(temp);
							temp->NullData();
							delete temp;
							return(OK);							
					   }
						else // Yes arguments a->func(arg1, arg2 ...)
						{
							if(var->GetType() == UNQUOTED_STRING)
							{
								CText func = var->GetString();
								long pos = func.Search(0,':');
								if(pos != -1) // Pass as arguments alias(structureName), arg1, arg2 ...
								{
									CText args = "alias(";
									args = args + structName;
									args = args + "),";			
									args = args + rData->args;			
									short r = CheckForMacroProcedure(itfc,func.Str(),args.Str());				
									if(r == 0 && itfc->nrRetValues > 0)
										CopyVariable(dataLeft,&itfc->retVar[1],FULL_COPY);
									return(r);
								}
								Variable *temp = new Variable; // A normal variable
								if(CopyVariable(temp,var,FULL_COPY) == ERR)
									return(ERR);
								dataLeft->Assign(temp);
								temp->NullData();
								delete temp;
								return(OK);	
							}
						}
					}
					else if(rData->type == CLASS_MEMBER) // A normal variable
					{
						Variable *temp = new Variable;
						if(CopyVariable(temp,var,FULL_COPY) == ERR)
							return(ERR);
					//	dataLeft->SetName(var->GetName());
						dataLeft->Assign(temp);
						temp->NullData();
						delete temp;
						return(OK);
					}
            }
         }
         ErrorMessage("'%s' is not a member of '%s'",member,dataLeft->GetName());
         return(ERR);
      }
   }

   return(ERR);
}


/******************************************************************************************
   Evaluate a CLASS operating on a CLASS item - result is variant
******************************************************************************************/

short ClassClassItemEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   short r = ERR;

   switch(operation)
   { 
      case(ARROW): // -> class dereferencing operator
      {
         ClassData *lData = (ClassData*)dataLeft->GetData();
         ClassItem *rData = (ClassItem*)dataRight->GetData();

      // Validate object pointer
         if(CheckClassValidity(lData,true) == ERR)
            return(ERR);

         r = ProcessClassProcedure(itfc, lData, rData->name, rData->args);

         if(itfc->nrRetValues > 0)
            CopyVariable(dataLeft,&itfc->retVar[1],FULL_COPY);

         break;
      }
      default:
      {
         ErrorMessage("'%s' is an undefined operation\n           between a class and a class-item",GetOpString(operation));
         return(ERR); 
      }
   }

   return(r);
 
}


/*********************************************************************************
   Evaluate a CLASS operating on a CLASS result is variant
*********************************************************************************/

short ClassClassEvaluate(Interface *itfc, Variable *dataLeft, Variable *dataRight, unsigned char operation)
{
   ClassData *lData = (ClassData*)dataLeft->GetData();
   ClassData *rData = (ClassData*)dataRight->GetData();

   if(CheckClassValidity(lData,true) == ERR || CheckClassValidity(rData,true) == ERR)
      return(ERR);

   switch(operation)
   { 
      case('='):
      case(EQ):
      {
         if((lData->code == rData->code) && (lData->data == rData->data) && (lData->type == rData->type))
         {
            dataLeft->SetNull();
            dataLeft->MakeAndSetFloat(1);
         }
         else
         {
            dataLeft->SetNull();
            dataLeft->MakeAndSetFloat(0);
         }
         return(OK);
      }
      default:
      {
         ErrorMessage("'%s' is an undefined operation\n           between a class and a class",GetOpString(operation));
         return(ERR); 
      }
   }

}

// Process a general class procedure


short ProcessClassProcedure(Interface *itfc, ClassData *cls, char *name, char *args)
{
   int r;
   CText argTxt;
   CArg carg;

   int nrArgs = carg.Count(args);

   if(name[0] == '\0')
   {
      if(nrArgs%2 != 0)
         argTxt.Format("\"text\",%s",args);
      else
         argTxt.Format("%s",args);
   }
   else
      argTxt.Format("\"%s\",%s",name, args);

   switch(cls->type)
   {
      case(PLOT_CLASS): // e.g. rg->trace(0)
      {
         Plot *pd = (Plot*)cls->data;
	  //  	MSG msg;
		//	while(pd->plotParent->busy)
		//		PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);
      //   pd->plotParent->busy = true;
         r = ProcessPlotClassReferences(itfc, pd, name, args);
      //   pd->plotParent->busy = false;
         return(r);
      }
      case(TRACE_CLASS): // e.g. value = trc->tracecolor, trc->tracecolor(value)
      {
         Trace *di = (Trace*)cls->data;
         CArg carg;
         short nrArgs = carg.Count(argTxt.Str());
         if((r = GetOrSetTraceParameters(itfc,&carg,nrArgs,1,di,di->tracePar,NULL)) == OK)
         {
				// itfc->nrRetValues is nonzero if we were getting a trace parameter.
				// We only want to (re)display the plot if we changed a trace parameter.
				// Therefore, only call DisplayAllPlots() if itfc->nrRetValues is zero.
				// This avoids a useless redisplay each time a macro reads a trace attribute.
				// TODO: Improve the hacky logic in the first term of the following if!
				//       This only happens to work.
            if(!(itfc->nrRetValues))
               di->DisplayAll(); 
         }
         return(r);
      }
      case(XLABEL_CLASS): // e.g. value = lbl->color, lbl->color(value)
      {
         Plot* pd = (Plot*)cls->data;
         r = pd->ProcessLabelParameters(itfc, "xlabel", argTxt.Str());
         return(r);
      }
      case(YLABEL_CLASS): // e.g.  value = lbl->size, lbl->size(value)
      {
         Plot* pd = (Plot*)cls->data;
         r = pd->ProcessLabelParameters(itfc, "ylabel", argTxt.Str());
         return(r);
      }
      case(YLABEL_LEFT_CLASS): // e.g.  value = lbl->size, lbl->size(value)
      {
         Plot* pd = (Plot*)cls->data;
         r = pd->ProcessLabelParameters(itfc, "ylabelleft", argTxt.Str());
         return(r);
      }
      case(YLABEL_RIGHT_CLASS): // e.g.  value = lbl->size, lbl->size(value)
      {
         Plot* pd = (Plot*)cls->data;
         r = pd->ProcessLabelParameters(itfc, "ylabelright", argTxt.Str());
         return(r);
      }
      case(TITLE_CLASS): // e.g. value = ttl->style, ttl->style(value)
      {
         Plot* pd = (Plot*)cls->data;
         r = pd->ProcessLabelParameters(itfc, "title", argTxt.Str());
         return(r);
      }
      case(AXES_CLASS): // e.g. value = ax->font, ax->font(value)
      {
         Plot* pd = (Plot*)cls->data;
         r = pd->ProcessAxesParameters(itfc, argTxt.Str());
         return(r);
      }
      case(GRID_CLASS): // e.g. value = ax->font, ax->font(value)
      {
         Plot* pd = (Plot*)cls->data;
         r = pd->ProcessGridParameters(itfc, argTxt.Str());
         return(r);
      }
      case(OBJECT_CLASS): // e.g. obj->parameter(value), value = obj->parameter()
      {
         ObjectData *obj = (ObjectData*)cls->data;
         r = ProcessObjectClassReferences(itfc, obj, name, args);
         return(r);
      }
      case(WINDOW_CLASS):  // e.g. win->parameter(value), value = win->parameter
      {
         WinData *win = (WinData*)cls->data;
         itfc->nrRetValues = 0;
         r = win->ProcessClassProc(itfc, name, args);
         return(r);
      }
		case(INSET_CLASS):
		{
			Inset* inset = (Inset*)cls->data;
			r = inset->ProcessInsetParameters(itfc, argTxt.Str());
			return r;
		}
   }
   return(ERR);
}

/********************************************************************
  If the current part of the expression is a simple variable this 
  routine calls the variable as a class item.   
********************************************************************/

short GetSimpleOperand(Interface *itfc, char *name, char *expression, short &i, Variable *result)
{
 //  result->MakeAndSetString(name);
   result->MakeClassItem(CLASS_MEMBER,name,NULL);
//return(UNQUOTED_STRING);

   return(CLASSITEM);
}


/********************************************************************
  If the current part of the expression is a function call this 
  routine calls the function as a class item.   
********************************************************************/

short GetFunctionOperand(Interface *itfc, char *function, char *expression, short &i, Variable *result)
{
   char * arguments = new char[strlen(expression)+1];
   
// Extract the expression enclosed by round brackets '()'. Place in 'arguments'
   if(ExtractSubExpression(expression,arguments,i,'(',')') < 0)
   {
      delete [] arguments;
	   return(ERR);
   }

   result->MakeClassItem(CLASS_FUNCTION,function,arguments);

   delete [] arguments;

   return(CLASSITEM);
}

short EvaluateStructureArrayOperand(Interface *itfc, char *function, char *expression, short &i, Variable *result)
{
   char * arguments = new char[strlen(expression)+1];
   
// Extract the expression enclosed by round brackets '[]'. Place in 'arguments'
   if(ExtractSubExpression(expression,arguments,i,'[',']') < 0)
   {
      delete [] arguments;
	   return(ERR);
   }
	short len = strlen(arguments);
	for(int i = len-1; i >= 0 ; i--)
		 arguments[i+1] = arguments[i];
	arguments[0] = '[';
	arguments[len+1] = ']';
	arguments[len+2] = '\0';


   result->MakeClassItem(CLASS_ARRAY,function,arguments);

   delete [] arguments;

   return(CLASSITEM);
}


/********************************************************************
  If the current part of the expression is an array  this 
  routine returns the array as a class item.   
********************************************************************/

short GetArrayOperand(Interface *itfc, char *name, char *expression, short &i, Variable *result)
{
   char * arguments = new char[strlen(expression)+1];
   
// Extract the expression enclosed by round brackets '[]'. Place in 'arguments'
   if(ExtractSubExpression(expression,arguments,i,'[',']') < 0)
   {
      delete [] arguments;
	   return(ERR);
   }

   result->MakeClassItem(CLASS_ARRAY,name,arguments);

   delete [] arguments;

   return(CLASSITEM);
}
