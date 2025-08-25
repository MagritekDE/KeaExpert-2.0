#include "stdafx.h"
#include "variablesClass.h"
#include "allocate.h"
#include "globals.h"
#include "files.h"
#include "list_functions.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/******************************************************************************
*
*                        Variable class member functions 
*
* Variable ................ Class initialiser
*
* Add ..................... Add a variable of the specified name and type
* Get ..................... Find a variable in a linked list by name
* GetNext ................. Get the next variable in the list 
*
* MakeAndSetInteger ....... Make and initialise an integer variable
* MakeAndSetFloat ......... Make and initialise a float variable
* MakeAndSetComplex ....... Make and initialise a complex variable
* MakeAndSetString ........ Make and initialise a string variable
* MakeAndLoadMatrix2D ....... Make a matrix variable and initialise
* MakeAndLoadCMatrix2D ...... Make a complex matrix variable and initialise
* MakeMatrix2DFromVector .... Use a vector to make and assign a matrix variable
* MakeCMatrix2DFromCVector .. Use a complex vector to make and assign a complex matrix variable
* AssignMatrix2D ............ Modify a matrix using an existing array
* AssignCMatrix2D ........... Modify a complex matrix using an existing array
*
* FreeData ................ Free all memory used by a variable
* RemoveAll ............... Remove all variables in a linked variable list
* Remove .................. Remove a single variable from a linked variable list
*
*                                                                    
*                                           16/3/2005 CDE             
*******************************************************************************/



/*************************************************************************************
  Constructor for variable class
**************************************************************************************/

Variable::Variable()
{
   next = (Variable*)0;
   last = (Variable*)0;
   data = (char*)0;
   visible = true;
   system = false;
   permanent = false;
   readOnly = false;
   scope = GLOBAL;
   size = 0;
   dim1 = dim2 = dim3 = dim4 = 0;
   aliasVar = (Variable*)0;
   aliasRef = 0;
   type = NULL_VARIABLE;
   name = NULL;
   data = NULL;
}


/*************************************************************************************
  Variable destructor
**************************************************************************************/

Variable::~Variable()
{
   Remove();
}


/*************************************************************************************
  Add variable to end of a linked list
**************************************************************************************/

Variable* Variable::Add(short type, char name[])
{
   Variable* var = new Variable;
   // Search to the end of the list
   Variable* temp = this;
   while (temp->next != NULL)
      temp = temp->next;
   // Add new variable to end
   temp->next = var;
   var->last = temp;
   var->SetName(name);
   var->type = type;
   return(var);
}


/*************************************************************************************
  Add variable to the start of a linked list
**************************************************************************************/

Variable* Variable::AddToStart(short type, char name[])
{
   Variable *var = new Variable;
   if(next != (Variable*)0)
      next->last = var;
   var->next = next;
   var->last = this;
   next = var;
   var->SetName(name);
   var->type = type;	
   return(var);
}




/*************************************************************************************
   Count the number of variables in a list including the current one
**************************************************************************************/

short Variable::Count()
{
   Variable* var = this;
   short cnt = 1;

   while(var->next != NULL)
   {
      cnt++;
      var = var->next;
   }
   return(cnt);
}

Variable* Variable::GetLast()
{
   Variable *var = this;

   do
   {
      var = var->next;
   }
   while(var->next != NULL);

   return(var);
}

/*************************************************************************************
   Add a new name for the variable
**************************************************************************************/

void Variable::SetName(char *str)
{
   long len = strlen(str);
   if(name) delete [] name;
   name = new char[len+1];
   strcpy(name,str);
}

void Variable::FreeName()
{  
   if(name) 
		delete [] name;
   name = 0;
}

/*************************************************************************************
   Initialise a variable without freeing memory (except for name)
**************************************************************************************/

void Variable::SetNull()
{
   dim1 = dim2 = dim3 = dim4 = 0;
   data = NULL;
   type = NULL_VARIABLE;
	if(name)
		delete [] name;
	name = NULL;
}

/*************************************************************************************
   Make an integer variable
**************************************************************************************/

void Variable::MakeAndSetInteger(long value)
{	
   long *temp;
   FreeData();
   data = (char*)new long;
   temp = (long*)data;
   *temp = value;
   type = INTEGER;
   aliasVar = NULL;
}


/*************************************************************************************
   Make a null variable by deleting the data
**************************************************************************************/

void Variable::MakeNullVar()
{	
   if(aliasVar && !permanent && !system)
   {
      aliasVar->aliasRef--;
  //    TextMessage("\nMakeNullVar: Removing reference to '%s' from '%s' (%hd)",aliasVar->name,name,aliasVar->aliasRef);

      aliasVar = NULL;
   }
   else
      FreeData();
   type = NULL_VARIABLE;
}

/*************************************************************************************
   Make a null variable by setting the data to NULL
**************************************************************************************/

void Variable::NullData()
{	
   if(aliasVar && !permanent && !system)
   {
      aliasVar->aliasRef--;
  //    TextMessage("\nNullData: Removing reference to '%s' from '%s' (%hd)",aliasVar->name,name,aliasVar->aliasRef);
      aliasVar = NULL;
   }
   data = NULL;
   type = NULL_VARIABLE;
}

/*************************************************************************************
   Make a real number variable
**************************************************************************************/

void Variable::MakeAndSetFloat(float value)
{	
   float *temp;
   FreeData();
   data = (char*)new float;
   temp = (float*)data;
   *temp = value;
   type = FLOAT32;
   aliasVar = NULL;
   aliasRef = 0; 
}

/*************************************************************************************
   Make a double precision real number variable
**************************************************************************************/

void Variable::MakeAndSetDouble(double value)
{	
   double *temp;
   FreeData();
   data = (char*)new double;
   temp = (double*)data;
   *temp = value;
   type = FLOAT64;
   aliasVar = NULL;
   aliasRef = 0; 
}

/*************************************************************************************
   Make a structure variable
**************************************************************************************/

void Variable::MakeStruct()
{	
   FreeData();
   data = (char*)new Variable;
   type = STRUCTURE;
   aliasVar = NULL;
   aliasRef = 0; 
}

/*************************************************************************************
   Make a structure array
**************************************************************************************/

void Variable::MakeStructArray(long size)
{	
   FreeData();
   data = (char*)new Variable[size];
   Variable *temp = (Variable*)data;
   for(int i = 0; i < size; i++)
      temp[i].MakeStruct();
   type = STRUCTURE_ARRAY;
   dim1 = size;
   aliasVar = NULL;
   aliasRef = 0; 
}

/********************************************************************************************
   Make a procedure variable - this contains the text, name and location of the procedure
********************************************************************************************/

void Variable::MakeAndSetProcedure(char *text, char *procedureName, char *macroName, char *macroPath, long lineNr)
{
   ProcedureInfo *procInfo;

   AddExtension(macroName,".mac");

// Make a new procedure info structure
   procInfo = new ProcedureInfo;
// Save procedure name
   procInfo->procName = new char[strlen(procedureName)+1];
   strcpy(procInfo->procName,procedureName);
// Save procedure macro
   procInfo->macroName = new char[strlen(macroName)+1];
   strcpy(procInfo->macroName,macroName);
// Save macro path
   procInfo->macroPath = new char[strlen(macroPath)+1];
   strcpy(procInfo->macroPath,macroPath);
// Save procedure text
   procInfo->procedure = new char[strlen(text)+1];
   strcpy(procInfo->procedure,text);
// Save line number
   procInfo->startLine = lineNr;
  // TextMessage("Cache allocated : %s:%s %Xl %ld\n",procInfo->macroName, procInfo->procName, (long)procInfo->procedure, strlen(text));

// Attach to variable
   data = (char*)procInfo;
   type = PROCEDURE;
   aliasVar = NULL;
   aliasRef = 0; 
}

Variable* Variable::GetStructVariable(char *name)
{
   Variable *var;
   for(var = this->next; var != NULL; var = var->next)
   {
      if(!strcmp(var->GetName(),name))
         return(var);
   }

   return(NULL);
}


char* Variable::GetProcedureInfo(char *procName, char *macroName, char *macroPath)
{
   ProcedureInfo *proc = (ProcedureInfo*)data;

   strcpy(procName,proc->procName);
   strcpy(macroName,proc->macroName);
   strcpy(macroPath,proc->macroPath);
   return(proc->procedure);
}

char* Variable::GetProcedureText(long &startLine)
{
   ProcedureInfo *proc = (ProcedureInfo*)data;
   startLine = proc->startLine;
   return(proc->procedure);
}



/*************************************************************************************
   Copy the contents of varIn to this variable but make an alias to the data
**************************************************************************************/

short Variable::CopyWithAlias(Variable *varIn)
{ 	
	bool inCriticalSection = false;

// If variable to copy is an alias then make this
// new variable an alias too   
   if(varIn->aliasVar)
   {   
     if(varIn->scope == WINDOW || varIn->scope == GLOBAL ||
        this->scope == WINDOW || this->scope == GLOBAL)
	  {
		  inCriticalSection = true;
        EnterCriticalSection(&csVariable);
	  }

	// If the variable already exists and is an alias
	// then reduce reference count   
	   if(!permanent && aliasVar && !system)
	   {
	      aliasVar->aliasRef--;
    //     aliasVar = NULL;
   //      TextMessage("\nCopyWithAlias: Removing reference to '%s' from '%s' (%hd)",aliasVar->name,name,aliasVar->aliasRef);
	   }
	
	// Remove any existing data (if not an alias)
	   if(!aliasVar)      
	      FreeData();
	
	// Copy type and dimensions
	   type = varIn->type;
	   dim1 = varIn->GetDimX();
	   dim2 = varIn->GetDimY();
	   dim3 = varIn->GetDimZ();
	   dim4 = varIn->GetDimQ();
	//   scope = varIn->scope;
	   
			// Copy the name
		if(varIn->name && !name)
		{
			name = new char[strlen(varIn->name)+1];
			strcpy(name,varIn->name);
		}

      aliasVar = varIn->aliasVar;
      data = aliasVar->data;
      if(!permanent && !system)
      {
         aliasVar->aliasRef++;
  //       TextMessage("\nCopyWithAlias: Adding reference %s = %s (alias to %s) (%hd)",name,varIn->name,varIn->aliasVar->name,aliasVar->aliasRef);
      }

		if(inCriticalSection)
         LeaveCriticalSection(&csVariable);

   }
// Otherwise just copy varIns contents
   else
   {
      if(FullCopy(varIn) == ERR)
      {
         return(ERR);
      }
   }

   return(OK);
}

/*************************************************************************************
   Copy varIn to this variable including data
**************************************************************************************/

short Variable::FullCopy(Variable *varIn)
{ 	
	bool inCriticalSection = false;

  if(varIn->scope == WINDOW || varIn->scope == GLOBAL ||
     this->scope == WINDOW || this->scope == GLOBAL)
  {
	  inCriticalSection = true;
     EnterCriticalSection(&csVariable);
  }

// If the variable already exists and is an alias
// then reduce reference count   
   if(!permanent && aliasVar && !system)
   {
      aliasVar->aliasRef--;
 //     TextMessage("\nFullCopy: Removing reference to '%s' from '%s' (%hd)",aliasVar->name,name,aliasVar->aliasRef);
   }

// Remove any existing data (if not an alias or has identical data)
   if(!aliasVar) 
   {
 //  if(varIn->GetData() == data)
 //     int a = 23;
   if(!aliasVar && varIn->GetData() != data)      
      FreeData();
   }

// Copy type and dimensions
   type = varIn->GetType();
   dim1 = varIn->GetDimX();
   dim2 = varIn->GetDimY();
   dim3 = varIn->GetDimZ();
   dim4 = varIn->GetDimQ();

// Copy the name
	if(varIn->name && !name)
	{
		name = new char[strlen(varIn->name)+1];
		strcpy(name,varIn->name);
	}

 //  scope = varIn->scope;
   
// Initialize alias settings
   if(!varIn->data)
   {
      if(aliasVar)
      {
         aliasVar = NULL;
         aliasRef = 0;
      }
      data = NULL;
      goto ok;
   }

// Copy varIns contents if not already present
   if(varIn->GetData() != data || aliasVar)
   {
      // Initialize alias settings
      aliasVar = NULL;
      aliasRef = 0;

      switch(varIn->GetType())
      {
         case(INTEGER):
         {
            long *temp;
            data = (char*)new long;
            if(!data) 
            {
               ErrorMessage("unable to allocate integer memory");
               goto err;
            }
            temp = (long*)data;
            *temp = VarInteger(varIn);
            break;
         }   
         case(FLOAT32):
         {
            float *temp;
            data = (char*)new float;
            if(!data) 
            {
               ErrorMessage("unable to allocate float memory");
               goto err;
            }
            temp = (float*)data;
            *temp = VarReal(varIn);
            break;
         }
         case(FLOAT64):
         {
            double *temp;
            data = (char*)new double;
            if(!data) 
            {
               ErrorMessage("unable to allocate double memory");
               goto err;
            }
            temp = (double*)data;
            *temp = varIn->GetDouble();
            break;
         }
         case(COMPLEX):
         {
            complex *temp;
            data = (char*)new complex;
            if(!data) 
            {
               ErrorMessage("unable to allocate complex memory");
               goto err;
            }
            temp = (complex*)data;
            *temp = VarComplex(varIn); 
            break;
         }  
         case(UNQUOTED_STRING):
         {
            data = new char[strlen(varIn->data)+1];
            if(!data) 
            {
               ErrorMessage("unable to allocate string memory");
               goto err;
            }
            strcpy(data,VarString(varIn));
            break;
         } 
         case(STRUCTURE):
         {
            Variable *var,*memberVar;
            data = (char*)new Variable;
            Variable* struc = (Variable*)data;
            if(!data) 
            {
               ErrorMessage("unable to allocate structure memory");
               goto err;
            }

            Variable *varBase = varIn->GetStruct();
            for(var = varBase->next; var != NULL; var = var->next)
            {
					memberVar = struc->Add(STRUCTURE,var->GetName());

					if(CopyVariable(memberVar,var,FULL_COPY) == ERR)
					{
						goto err;
					}
            }
            break;
         }
         case(STRUCTURE_ARRAY):
         {
            Variable *arrayIn, *arrayOut ;
            Variable *strucIn,*strucOut;
            int size = varIn->GetDimX();
            arrayIn = (Variable*)varIn->GetData();
				data = (char*)0;
            this->MakeStructArray(size);
            arrayOut = (Variable*)this->GetData();
            for(int i = 0; i < size; i++)
            {
               strucIn = &(arrayIn[i]);
               strucOut = &(arrayOut[i]);
               if(CopyVariable(strucOut,strucIn,FULL_COPY) == ERR)
               {
                  goto err;
               }
            }
            break;
         }
         case(CLASS):
         {
            ClassData *temp;
            data = (char*)new ClassData;
            if(!data) 
            {
               ErrorMessage("unable to allocate class memory");
               goto err;
            }
            temp = (ClassData*)data;
            *temp = *(ClassData*)varIn->GetData(); 
            break;
         }
         case(CLASSITEM):
         {
            ClassItem *temp;
            data = (char*)new ClassItem;
            if(!data) 
            {
               ErrorMessage("unable to allocate classitem memory");
               goto err;
            }
            temp = (ClassItem*)data;
            *temp = *(ClassItem*)varIn->GetData(); 
            break;
         }
         case(LIST):
         {
            char **loclist,**srcList;         
		      loclist = new char*[dim1];
            if(!loclist) 
            {
               ErrorMessage("unable to allocate list memory");
               goto err;
            }
		      srcList = (char**)varIn->data;
		   // Store each element of list in data array
		      for(int x = 0; x < dim1; x++)
		      {
		         loclist[x] = new char[strlen(srcList[x])+1];
               if(!loclist[x]) 
               {
                  ErrorMessage("unable to allocate list memory");
                  goto err;
               }
		         strcpy(loclist[x],srcList[x]);
		      }
	      // Attach the list data   
	         data = (char*)loclist; 
            break;
         }
         case(LIST2D):
         {
				List2DData *srcList = (List2DData*)(varIn->data);
				List2DData *dstList = new List2DData(srcList->nrRows);

            if(!dstList) 
            {
               ErrorMessage("unable to allocate 2D list memory");
               goto err;
            }

		   // Store each element of list in data array
		      for(int y = 0; y < srcList->nrRows; y++)
				{
					if(!dstList->MakeRow(y,srcList->rowSz[y]))
					{
						delete dstList;
						ErrorMessage("unable to allocate list memory");
						goto err;
					}
					for(int x = 0; x < dstList->rowSz[y]; x++)
					{
						if(!dstList->AddEntry(srcList->strings[y][x],x,y))
						{
						   delete dstList;
							ErrorMessage("Invalid 2D list index or unable to allocate list memory");
							goto err;
						}
					}
				}
	      // Attach the list data   
	         data = (char*)dstList; 
            break;
         }
         case(MATRIX2D): 
         {  
            float **arrayIn, **arrayOut;
		      arrayIn = (float**)varIn->data;
	         data = (char*)MakeMatrix2D(dim1,dim2);
            if(!data) 
            {
               ErrorMessage("unable to allocate 2D matrix memory");
               goto err;
            }
		      arrayOut = (float**)data;	   
   	
			   for(int y = 0; y < dim2; y++)
			      for(int x = 0; x < dim1; x++)
			         arrayOut[y][x] = arrayIn[y][x];
			   break;
	      }
         case(DMATRIX2D): 
         {  
            double **arrayIn, **arrayOut;
		      arrayIn = (double**)varIn->data;
	         data = (char*)MakeDMatrix2D(dim1,dim2);
            if(!data) 
            {
               ErrorMessage("unable to allocate 2D matrix memory");
               goto err;
            }
		      arrayOut = (double**)data;	   
   	
			   for(int y = 0; y < dim2; y++)
			      for(int x = 0; x < dim1; x++)
			         arrayOut[y][x] = arrayIn[y][x];
			   break;
	      }
         case(CMATRIX2D): 
         {  
            complex **arrayIn, **arrayOut;
		      arrayIn = (complex**)varIn->data;
	         data = (char*)MakeCMatrix2D(dim1,dim2);
            if(!data) 
            {
               ErrorMessage("unable to allocate 2D cmatrix memory");
               goto err;
            }
		      arrayOut = (complex**)data;	   
   	
			   for(int y = 0; y < dim2; y++)
			      for(int x = 0; x < dim1; x++)
			         arrayOut[y][x] = arrayIn[y][x];
			   break;
	      }          
         case(MATRIX3D): 
         {  
            float ***arrayIn, ***arrayOut;
		      arrayIn = (float***)varIn->data;
	         data = (char*)MakeMatrix3D(dim1,dim2,dim3);
            if(!data) 
            {
               ErrorMessage("unable to allocate 3D matrix memory");
               goto err;
            }
		      arrayOut = (float***)data;	   
   	
		      for(int z = 0; z < dim3; z++)
			      for(int y = 0; y < dim2; y++)
			         for(int x = 0; x < dim1; x++)
			            arrayOut[z][y][x] = arrayIn[z][y][x];
			   break;
	      }
         case(CMATRIX3D): 
         {  
            complex ***arrayIn, ***arrayOut;
		      arrayIn = (complex***)varIn->data;
	         data = (char*)MakeCMatrix3D(dim1,dim2,dim3);
            if(!data) 
            {
               ErrorMessage("unable to allocate 3D cmatrix memory");
               goto err;
            }
		      arrayOut = (complex***)data;	   
   	
		      for(int z = 0; z < dim3; z++)
			      for(int y = 0; y < dim2; y++)
			         for(int x = 0; x < dim1; x++)
			            arrayOut[z][y][x] = arrayIn[z][y][x];
			   break;
	      } 
         case(MATRIX4D): 
         {  
            float ****arrayIn, ****arrayOut;
		      arrayIn = (float****)varIn->data;
	         data = (char*)MakeMatrix4D(dim1,dim2,dim3,dim4);
            if(!data) 
            {
               ErrorMessage("unable to allocate 4D matrix memory");
               goto err;
            }
		      arrayOut = (float****)data;	   
   	
		      for(int q = 0; q < dim4; q++)
		         for(int z = 0; z < dim3; z++)
			         for(int y = 0; y < dim2; y++)
			            for(int x = 0; x < dim1; x++)
			               arrayOut[q][z][y][x] = arrayIn[q][z][y][x];
			   break;
	      }
         case(CMATRIX4D): 
         {  
            complex ****arrayIn, ****arrayOut;
		      arrayIn = (complex****)varIn->data;
	         data = (char*)MakeCMatrix4D(dim1,dim2,dim3,dim4);
            if(!data) 
            {
               ErrorMessage("unable to allocate 4D cmatrix memory");
               goto err;
            }
		      arrayOut = (complex****)data;	   
   	
		      for(int q = 0; q < dim4; q++)
		         for(int z = 0; z < dim3; z++)
			         for(int y = 0; y < dim2; y++)
			            for(int x = 0; x < dim1; x++)
			               arrayOut[q][z][y][x] = arrayIn[q][z][y][x];
			   break;
	      }
	   }  
   }
   else
   {
      ErrorMessage("Full copy data matching error");
      goto err;
   }

ok:
	if(inCriticalSection)
      LeaveCriticalSection(&csVariable);

   return(OK);

err:
	if(inCriticalSection)
      LeaveCriticalSection(&csVariable);
   return(ERR);
}

/*************************************************************************************
   Make this variable a duplicate of varIn (apart from name, scope and list position)
**************************************************************************************/

void Variable::Assign(Variable *varIn)
{ 	
// If the variable already exists and is an alias
// then reduce reference count 
	if(aliasVar)
	{
		if(!permanent && !system)
		{
			aliasVar->aliasRef--;
			aliasVar = NULL;
	  //    TextMessage("\nAssign: Removing reference to '%s' from '%s' (%hd)",aliasVar->name,name,aliasVar->aliasRef);
		}
	}
	else // Remove any existing data (if not an alias)
      FreeData();

// Copy type and dimensions
   type = varIn->GetType();
   dim1 = varIn->GetDimX();
   dim2 = varIn->GetDimY();
   dim3 = varIn->GetDimZ();
   dim4 = varIn->GetDimQ();
  // scope = varIn->scope;
   
// Copy alias
   aliasVar = varIn->aliasVar;

   if(aliasVar && !permanent && !system)
   {
      aliasVar->aliasRef++;
 //     TextMessage("\nAssign: %s = %s (alias to %s) (%hd)",name,varIn->name,varIn->aliasVar->name,varIn->aliasVar->aliasRef);
   }

// Copy data
   data = varIn->data;      
}


/*************************************************************************************
   Make a complex number variable
**************************************************************************************/

void Variable::MakeAndSetComplex(complex value)
{	
   complex *temp;
   FreeData();
   data = (char*)new complex;
   temp = (complex*)data;
   *temp = value;
   type = COMPLEX;
   aliasVar = NULL;  
}

/*************************************************************************************
   Make a complex number variable
**************************************************************************************/

void Variable::MakeAndSetComplex(float real, float imag)
{	
   complex *temp;
   FreeData();
   data = (char*)new complex;
   temp = (complex*)data;
   temp->r = real;
   temp->i = imag;
   type = COMPLEX;
   aliasVar = NULL;   
}

/*************************************************************************************
   Make a string variable
**************************************************************************************/

void Variable::MakeAndSetString(const char* const string)
{
   FreeData();
   data = new char[strlen(string)+1];
   strcpy(data,string);
   type = UNQUOTED_STRING;
   aliasVar = NULL;   
}

/****************************************************************
*    Make a list variable with n entries
****************************************************************/

void Variable::MakeList(short n)
{
// Remove any old data attached to this variable  
   FreeData();
// Make new data  
   data = (char*)::MakeList(n);
// Set x dimensions and variable type
	dim1 = n;
	dim2 = 1;
	dim3 = 1;	
   dim4 = 1;	
   type = LIST; 
   aliasVar = NULL;
   aliasRef = 0;    
}


/****************************************************************
*    Make a list variable with n entries
****************************************************************/

void Variable::MakeAndSet2DList(short x, short y)
{
// Remove any old data attached to this variable  
   FreeData();
// Make new data  
   data = (char*)::Make2DList(x,y);
// Set x dimensions and variable type
	dim1 = x;
	dim2 = y;
	dim3 = 1;	
   dim4 = 1;	
   type = LIST2D; 
   aliasVar = NULL;
   aliasRef = 0;    
}

/****************************************************************
*    Make and set a list variable - this is an array of strings
****************************************************************/

void Variable::MakeAndSetList(char **list, short cols)
{
   char **loclist;

// Remove any old data attached to this variable  
   FreeData();

	if(cols == 0 || list == NULL)
	{
		loclist = NULL;
	}
	else
	{
	// Make an array of character pointers   
		loclist = new char*[cols];
	// Store each element of list in data array
		for(int i = 0; i < cols; i++)
		{
			loclist[i] = new char[strlen(list[i])+1];
			strcpy(loclist[i],list[i]);
		}
	}
// Set x dimensions and variable type
	dim1 = cols;
	dim2 = 1;
	dim3 = 1;
   dim4 = 1;
   type = LIST; 
   aliasVar = NULL;
   aliasRef = 0;    
// Attach the list data   
   data = (char*)loclist;  
}


/****************************************************************
*    Make and set a 2D list variable - this is an array of lists
****************************************************************/

void Variable::Assign2DList(List2DData *list)
{

   FreeData();
   dim1 = list->maxCols;
   dim2 = list->nrRows;
   dim3 = 1;   
   dim4 = 1;
   data = (char*)list;
   type = LIST2D; 
   aliasVar = NULL;
   aliasRef = 0; 
//
//   List2DData* loclist;
//
//// Remove any old data attached to this variable  
//   FreeData();
//
//	if(list == NULL)
//	{
//		loclist = NULL;
//	}
//	else
//	{
//	// Make an array of character pointers  
//		loclist = new List2DData(list->nrRows);
//		for(int j = 0; j < list->nrRows; j++)
//		{
//		  loclist->strings[j] = new char*[list->rowSz[j]];
//		// Store each element of list in data array
//			for(int i = 0; i < list->rowSz[j]; i++)
//			{
//				loclist->strings[j][i] = new char[strlen(list->strings[j][i])+1];
//				strcpy(loclist->strings[j][i],list->strings[j][i]);
//			}
//		}
//	}
// Set x dimensions and variable type

}


/*************************************************************************************
   Add a new entry to a string list
**************************************************************************************/
 
void Variable::AddToList(char *txt)
{
   char **oldlist;
   char **newlist;

// Make an array of character pointers   
   oldlist = (char**)data;
   
// Make new list
   newlist = new char*[dim1+1];
   
// Store each element of old to new list
   for(int i = 0; i < dim1; i++)
   {
      newlist[i] = new char[strlen(oldlist[i])+1];
      strcpy(newlist[i],oldlist[i]);
   }
// Add the new entry
   newlist[dim1] = new char[strlen(txt)+1];
   strcpy(newlist[dim1],txt);   

// Delete old data
   long d1 = dim1;
   FreeData();  

// Update dimensions
	dim1 = d1 + 1;
	dim2 = 1;
	dim3 = 1;		
   type = LIST; 
   aliasVar = NULL;
   aliasRef = 0;    

// Attach the list data   
   data = (char*)newlist;  
}

// Add a float variable to this structure
void Variable::AddToStructure(char *name, float data)
{
   Variable *v = this->Add(FLOAT32,name);
   v->MakeAndSetFloat(data);
}

// Add a double variable to this structure
void Variable::AddToStructure(char *name, double data)
{
   Variable *v = this->Add(FLOAT_64, name);
   v->MakeAndSetDouble(data);
}

// Add a string variable to this structure
void Variable::AddToStructure(char *name, char *txt)
{
   Variable *v = this->Add(UNQUOTED_STRING,name);
   v->MakeAndSetString(txt);
}

// Add a 2D matrix variable to this structure
void Variable::AddToStructure(char *name, float** mat, int width, int height)
{
   Variable *v = this->Add(MATRIX2D,name);
   v->MakeAndLoadMatrix2D(mat,width,height);
}

// Add a color variable to this structure
void Variable::AddToStructure(char *name, COLORREF color)
{
   Variable *v = this->Add(MATRIX2D,name);
   float col[3];
   col[0] = GetRValue(color);
   col[1] = GetGValue(color);
   col[2] = GetBValue(color);
   v->MakeMatrix2DFromVector(col,3,1);
}

/****************************************************************
*     Replace the nth item in string list with newText
*
* Returns 0 on success
* Returns -1 if position n is invalid
* Returns -2 if memory allocation fails
* Returns -3 if list is not present in variable structure
****************************************************************/

short  Variable::ReplaceListItem(char *newText, long n)
{
   if(type != LIST) 
      return(-1);
      
   if(data == NULL)
      return(-3);
  //xxxChanged 2nd arg def    
   short r = ReplaceStringInList(newText, (char***)&data, dim1, n);

   return(r);
}

/****************************************************************
*     Replace the (x,y)th item in 2D string list with newText
*
* Returns 0 on success
* Returns -1 if position n is invalid
* Returns -2 if memory allocation fails
* Returns -3 if list is not present in variable structure
****************************************************************/

short  Variable::Replace2DListItem(char *newText, long x, long y)
{
   if(type != LIST2D) 
      return(-1);
      
   if(data == NULL)
      return(-3);
  
   short r = ReplaceStringIn2DList(newText, (List2DData*)data, dim1, dim2, x, y);

   return(r);
}


/****************************************************************   
*         Copy the list in variable and return
****************************************************************/

char**  Variable::CopyList()
{
   char **copy;
   char **src;

// Source is variable list data   
   src = (char**)data;
   if(type != LIST) return(NULL);
// Copy the list
   copy = ::CopyList(src,dim1);
// And return it
   return(copy);
}


/*************************************************************************************
   Make an alias of variable "var
**************************************************************************************/
  
void  Variable::MakeAndSetAlias(Variable *var)
{
   if(!aliasVar)
   {
      if(var == this)
        data = NULL;
      else
        FreeData();
   }

   data = var->data;
   dim1 = var->GetDimX();	 
   dim2 = var->GetDimY();	 
   dim3 = var->GetDimZ();
   dim4 = var->GetDimQ();
   	 
   aliasVar = var;
   if(!permanent && !system) // Ignore references from permanent or system variables
      var->aliasRef++;
 //  winNr = var->winNr;
   type = var->GetType();
//   TextMessage("\nMakeAlias '%s' = '%s' (%hd)",name,var->name,var->aliasRef);

}

 /*************************************************************************************
   Allocate space for a real matrix variable. If an array is supplied
   then update the variable to have these values, otherwise zero
   it.
**************************************************************************************/
   
void Variable::MakeAndLoadMatrix2D(float **array, long xsize, long ysize)
{
   float **temp;
   
   FreeData();
   type = MATRIX2D;

   data = (char*)MakeMatrix2D(xsize,ysize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 2D matrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (float**)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;
   dim4 = 1;
	aliasVar = NULL;  

// Do nothing
   if(array == (float**)-1)
      return;

// Zero matrix
   if(array == NULL) 
   {
	   for(long y = 0; y < ysize; y++)
	   {
	      for(long x = 0; x < xsize; x++)
	      {
	         temp[y][x] = 0.0;
	      }
	   }   
   }

// Update with passed values
   else 
   {
	   for(long y = 0; y < ysize; y++)
	   {
         memcpy(temp[y],array[y],xsize*sizeof(float));
	   }
	}
}


 /*************************************************************************************
   Allocate space for a real matrix variable. If an array is supplied
   then update the variable to have these values, otherwise zero
   it.
**************************************************************************************/
   
void Variable::MakeAndLoadDMatrix2D(double **array, long xsize, long ysize)
{
   double **temp;
   
   FreeData();
   type = DMATRIX2D;

   data = (char*)MakeDMatrix2D(xsize,ysize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 2D matrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (double**)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;
   dim4 = 1;
	aliasVar = NULL;  

// Do nothing
   if(array == (double**)-1)
      return;

// Zero matrix
   if(array == NULL) 
   {
	   for(long y = 0; y < ysize; y++)
	   {
	      for(long x = 0; x < xsize; x++)
	      {
	         temp[y][x] = 0.0;
	      }
	   }   
   }

// Update with passed values
   else 
   {
	   for(long y = 0; y < ysize; y++)
	   {
         memcpy(temp[y],array[y],xsize*sizeof(double));
	   }
	}
}


 /*************************************************************************************
   Allocate space for a complex matrix variable. If an array is supplied
   then update the variable to have these values, otherwise zero
   it.
**************************************************************************************/
      
void Variable::MakeAndLoadCMatrix2D(complex **array, long xsize, long ysize)
{
   complex **temp;
   
   FreeData();
   type = CMATRIX2D;

   data = (char*)MakeCMatrix2D(xsize,ysize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 2D cmatrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (complex**)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;
   dim4 = 1;
	aliasVar = NULL; 

// Do nothing
   if(array == (complex**)-1)
      return;

// Zero matrix
   if(array == NULL) 
   {
	   for(long y = 0; y < ysize; y++)
	   {
	      for(long x = 0; x < xsize; x++)
	      {
	         temp[y][x].r = 0.0;
	         temp[y][x].i = 0.0;
	      }
	   }   
   }

// Update with passed values
   else
   {
	   for(long y = 0; y < ysize; y++)
	   {
         memcpy(temp[y],array[y],xsize<<3);
	   }
	}
}



/*************************************************************************************
   Allocate space for a matrix variable. If an array is supplied
   then update the variable to have these values, otherwise zero
   it.
**************************************************************************************/

void Variable::MakeAndLoadMatrix3D(float ***array, long xsize, long ysize, long zsize)
{
   long x,y,z;
   float ***temp;
   
   FreeData();
   type = MATRIX3D;
   
   data = (char*)MakeMatrix3D(xsize,ysize,zsize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 3D matrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (float***)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;
   dim4 = 1;
   aliasVar = NULL;
   	   
   if(array == NULL) // Zero matrix
   {
	   for(z = 0; z < zsize; z++)
	   {   
		   for(y = 0; y < ysize; y++)
		   {
		      for(x = 0; x < xsize; x++)
		      {
		         temp[z][y][x] = 0.0;
		      }
		   } 
		}  
   }
   else // Update values
   {
	   for(z = 0; z < zsize; z++)
	   {   
		   for(y = 0; y < ysize; y++)
		   {
		      for(x = 0; x < xsize; x++)
		      {
		         temp[z][y][x] = array[z][y][x];
		      }
		   }
		}
	}
}

/*************************************************************************************
   Allocate space for a matrix variable. If an array is supplied
   then update the variable to have these values, otherwise zero
   it.
**************************************************************************************/

void Variable::MakeAndLoadMatrix4D(float ****array, long xsize, long ysize, long zsize, long qsize)
{
   long x,y,z,q;
   float ****temp;
   
   FreeData();
   type = MATRIX4D;
   
   data = (char*)MakeMatrix4D(xsize,ysize,zsize,qsize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 4D matrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (float****)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;
   dim4 = qsize;
   aliasVar = NULL;

   if(array == NULL) // Zero matrix
   {
	   for(q = 0; q < qsize; q++)
      {
	      for(z = 0; z < zsize; z++)
	      {   
		      for(y = 0; y < ysize; y++)
		      {
		         for(x = 0; x < xsize; x++)
		         {
		            temp[q][z][y][x] = 0.0;
		         }
		      } 
		   } 
      }
   }
   else // Update values
   {
	   for(q = 0; q < qsize; q++)
      {
	      for(z = 0; z < zsize; z++)
	      {   
		      for(y = 0; y < ysize; y++)
		      {
		         for(x = 0; x < xsize; x++)
		         {
		            temp[q][z][y][x] = array[q][z][y][x];
		         }
		      }
		   }
      }
	}
}



/*************************************************************************************
   Allocate space for a complex matrix variable. If an array is supplied
   then update the variable to have these values, otherwise zero
   it.
**************************************************************************************/

void Variable::MakeAndLoadCMatrix4D(complex ****array, long xsize, long ysize, long zsize, long qsize)
{
   long x,y,z,q;
   complex ****temp;
   
   FreeData();
   type = CMATRIX4D;
   
   data = (char*)MakeCMatrix4D(xsize,ysize,zsize,qsize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 4D cmatrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (complex****)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;
   dim4 = qsize;
   aliasVar = NULL;
   	   
   if(array == NULL) // Zero matrix
   {
	   for(q = 0; q < qsize; q++)
      {
	      for(z = 0; z < zsize; z++)
	      {   
		      for(y = 0; y < ysize; y++)
		      {
		         for(x = 0; x < xsize; x++)
		         {
		            temp[q][z][y][x].r = 0.0;
		            temp[q][z][y][x].i = 0.0;
		         }
		      } 
		   } 
      }
   }
   else // Update values
   {
	   for(q = 0; q < qsize; q++)
      {
	      for(z = 0; z < zsize; z++)
	      {   
		      for(y = 0; y < ysize; y++)
		      {
		         for(x = 0; x < xsize; x++)
		         {
		            temp[q][z][y][x] = array[q][z][y][x];
		         }
		      }
		   }
      }
	}
}



/*************************************************************************************
   Allocate space for a complex matrix variable. If an array is supplied
   then update the variable to have these values, otherwise zero
  it.
**************************************************************************************/

       
void Variable::MakeAndLoadCMatrix3D(complex ***array, long xsize, long ysize, long zsize)
{
   complex ***temp;
   long x,y,z;

   FreeData();
   type = CMATRIX3D;

   data = (char*)MakeCMatrix3D(xsize,ysize,zsize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 3D cmatrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (complex***)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;
   dim4 = 1;
   aliasVar = 0;
   
   if(array == NULL) // Zero matrix
   { 
	   for(z = 0; z < zsize; z++)
	   {   
		   for(y = 0; y < ysize; y++)
		   {
		      for(x = 0; x < xsize; x++)
		      {
		         temp[z][y][x].r = 0;
		         temp[z][y][x].i = 0;
		      }
		   }  
		} 
   }
   else // Update values
   {  
	   for(z = 0; z < zsize; z++)
	   {   
		   for(y = 0; y < ysize; y++)
		   {
		      for(x = 0; x < xsize; x++)
		      {
		         temp[z][y][x].r = array[z][y][x].r;
		         temp[z][y][x].i = array[z][y][x].i;
		      }
		   }
		}
	}
   aliasVar = 0;	
}

/*************************************************************************************
  Make a list variable give a list array and length
**************************************************************************************/

void Variable::AssignString(char *string)
{
   FreeData();
   data = (char*)string;
   type = UNQUOTED_STRING; 
   aliasVar = NULL;
   aliasRef = 0; 
}

void Variable::AssignBreakPointList(char *bpInfo, long nrLines)
{
   FreeData();
   dim1 = nrLines;
   dim2 = 1;
   dim3 = 1;   
   dim4 = 1;
   data = (char*)bpInfo;
   type = BREAKPOINTINFO;
   aliasVar = NULL;
   aliasRef = 0; 
}

/*************************************************************************************
  Make a list variable give a list array and length
**************************************************************************************/

void Variable::AssignList(char **list, long entries)
{
   FreeData();
   dim1 = entries;
   dim2 = 1;
   dim3 = 1;   
   dim4 = 1;
   data = (char*)list;
   type = LIST; 
   aliasVar = NULL;
   aliasRef = 0; 
}

/*************************************************************************************
  Make a structure variable give an existing structure
**************************************************************************************/

void Variable::AssignStructure(Variable* struc)
{
   FreeData();
   dim1 = struc->dim1;
   dim2 = struc->dim2;
   dim3 = struc->dim3;   
   dim4 = struc->dim4;
   data = struc->data;
   type = struc->type; 
   aliasVar = NULL;
   aliasRef = 0; 
}

/*************************************************************************************
  Make a real 2D matrix variable give a 2D array and dimensions
**************************************************************************************/

void Variable::AssignMatrix2D(float **array,long xsize, long ysize)
{
   FreeData();
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;
   dim4 = 1;   
   data = (char*)array;
   type = MATRIX2D;
   aliasVar = NULL;
   aliasRef = 0; 
}


/*************************************************************************************
  Make a real 2D dmatrix variable give a 2D array and dimensions
**************************************************************************************/

void Variable::AssignDMatrix2D(double **array, long xsize, long ysize)
{
   FreeData();
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;
   dim4 = 1;   
   data = (char*)array;
   type = DMATRIX2D;
   aliasVar = NULL;
   aliasRef = 0; 
}

/*************************************************************************************
  Make a real 3D matrix variable give a 3D array and dimensions
**************************************************************************************/

void Variable::AssignMatrix3D(float ***array,long xsize, long ysize, long zsize)
{
   FreeData();
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize; 
   dim4 = 1;  
   data = (char*)array;
   type = MATRIX3D; 
   aliasVar = NULL;
   aliasRef = 0; 
}


/*************************************************************************************
  Make a real 4D matrix variable give a 4D array and dimensions
**************************************************************************************/

void Variable::AssignMatrix4D(float ****array, long xsize, long ysize, long zsize, long qsize)
{
   FreeData();
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;   
   dim4 = qsize;   
   data = (char*)array;
   type = MATRIX4D; 
   aliasVar = NULL;
   aliasRef = 0; 
}

/*************************************************************************************
  Make a complex 2D matrix variable give a 2D array and dimensions
**************************************************************************************/

void Variable::AssignCMatrix2D(complex **array, long xsize, long ysize)
{
   FreeData();
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;   
   dim4 = 1;
   data = (char*)array;
   type = CMATRIX2D;
   aliasVar = NULL;
   aliasRef = 0;       
}

/*************************************************************************************
  Make a complex 3D matrix variable give a 3D array and dimensions
**************************************************************************************/

void Variable::AssignCMatrix3D(complex ***array, long xsize, long ysize, long zsize)
{
   FreeData();
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;  
   dim4 = 1;
   data = (char*)array;
   type = CMATRIX3D;
   aliasVar = NULL;
   aliasRef = 0;       
}

/*************************************************************************************
  Make a complex 4D matrix variable give a 4D array and dimensions
**************************************************************************************/

void Variable::AssignCMatrix4D(complex ****array, long xsize, long ysize, long zsize, long qsize)
{
   FreeData();
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;   
   dim4 = qsize;   
   data = (char*)array;
   type = CMATRIX4D;
   aliasVar = NULL;
   aliasRef = 0;       
}


/*************************************************************************************
  Pack a real 1D vector into a real 2D matrix dim xsize*ysize
**************************************************************************************/

void Variable::MakeMatrix2DFromVector(float *array, long xsize, long ysize)
{
   float **temp;
   FreeData();
   
   data = (char*)MakeMatrix2D(xsize,ysize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 2D matrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (float**)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;
   dim4 = 1;
   aliasVar = NULL;

   if(array != NULL)
   {
      long j = 0;
	   for(long y = 0; y < ysize; y++)
	   {   
		   for(long x = 0; x < xsize; x++,j++)
		   {
		      temp[y][x] = array[j];
		   }
		}
   }
   type = MATRIX2D;
}


/*************************************************************************************
  Pack a real 1D vector into a real 3D matrix dim xsize*ysize*zsize
**************************************************************************************/

void Variable::MakeMatrix3DFromVector(float *array, long xsize, long ysize, long zsize)
{
   float ***temp;
   FreeData();
   
   data = (char*)MakeMatrix3D(xsize,ysize,zsize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 3D matrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (float***)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;
   dim4 = 1;
   aliasVar = NULL;

   if(array != NULL)
   {
      long j = 0;
	   for(long z = 0; z < zsize; z++)
	   {       
		   for(long y = 0; y < ysize; y++)
		   {   
			   for(long x = 0; x < xsize; x++,j++)
			   {
			      temp[z][y][x] = array[j];
			   }
			}
	   }
   }
   type = MATRIX3D;
}


/*************************************************************************************
  Pack a real 1D vector into a real 4D matrix dim xsize*ysize*zsize*qsize
**************************************************************************************/

void Variable::MakeMatrix4DFromVector(float *array, long xsize, long ysize, long zsize, long qsize)
{
   float ****temp;
   FreeData();
   
   data = (char*)MakeMatrix4D(xsize,ysize,zsize,qsize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 4D matrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (float****)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;
   dim4 = qsize;
   aliasVar = NULL;

   if(array != NULL)
   {
      long j = 0;
	   for(long q = 0; q < qsize; q++)
      {
	      for(long z = 0; z < zsize; z++)
	      {       
		      for(long y = 0; y < ysize; y++)
		      {   
			      for(long x = 0; x < xsize; x++,j++)
			      {
			         temp[q][z][y][x] = array[j];
			      }
			   }
	      }
      }
   }
   type = MATRIX4D;
}


/*************************************************************************************
  Pack a double 1D vector into a double 2D matrix dim xsize*ysize
**************************************************************************************/

void Variable::MakeDMatrix2DFromVector(double* array, long xsize, long ysize)
{
   double** temp;
   FreeData();

   data = (char*)MakeDMatrix2D(xsize, ysize);
   if (!data)
   {
      ErrorMessage("unable to allocate 2D matrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (double**)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;
   dim4 = 1;
   aliasVar = NULL;

   if (array != NULL)
   {
      long j = 0;
      for (long y = 0; y < ysize; y++)
      {
         for (long x = 0; x < xsize; x++, j++)
         {
            temp[y][x] = array[j];
         }
      }
   }
   type = DMATRIX2D;
}

/*************************************************************************************
  Pack a real 1D vector into a real 4D matrix dim xsize*ysize*zsize*qsize
**************************************************************************************/

void Variable::MakeCMatrix4DFromCVector(complex *array, long xsize, long ysize, long zsize, long qsize)
{
   complex ****temp;
   FreeData();
   
   data = (char*)MakeCMatrix4D(xsize,ysize,zsize,qsize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 4D cmatrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (complex****)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;
   dim4 = qsize;
   aliasVar = NULL;

   if(array != NULL)
   {
      long j = 0;
	   for(long q = 0; q < qsize; q++)
      {
	      for(long z = 0; z < zsize; z++)
	      {       
		      for(long y = 0; y < ysize; y++)
		      {   
			      for(long x = 0; x < xsize; x++,j++)
			      {
			         temp[q][z][y][x] = array[j];
			      }
			   }
	      }
      }
   }
   type = CMATRIX4D;
}



/*************************************************************************************
  Pack a complex 1D vector into a complex 2D matrix dim xsize*ysize
**************************************************************************************/

void Variable::MakeCMatrix2DFromCVector(complex *array, long xsize, long ysize)
{
   complex **temp;
   FreeData();
   
   data = (char*)MakeCMatrix2D(xsize,ysize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 2D cmatrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (complex**)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = 1;
   dim4 = 1;
   aliasVar = NULL;

   if(array != NULL)
   {
      long j= 0;
	   for(long y = 0; y < ysize; y++)
	   {
		   for(long x = 0; x < xsize; x++,j++)
		   {
		      temp[y][x] = array[j];
		   }
	   }
   }
   type = CMATRIX2D;
}

/*************************************************************************************
  Pack a complex 1D vector into a complex 3D matrix dim xsize*ysize*zsize
**************************************************************************************/

void Variable::MakeCMatrix3DFromCVector(complex *array, long xsize, long ysize, long zsize)
{
   complex ***temp;
   FreeData();
   
   data = (char*)MakeCMatrix3D(xsize,ysize,zsize);
   if(!data) 
   {
      ErrorMessage("unable to allocate 3D cmatrix memory");
      type = NULL_VARIABLE;
      dim1 = dim2 = dim3 = dim4 = 0;
      return;
   }
   temp = (complex***)data;
   dim1 = xsize;
   dim2 = ysize;
   dim3 = zsize;
   dim4 = 1;
   aliasVar = NULL;

   if(array != NULL)
   {
      long j= 0;
	   for(long z = 0; z < zsize; z++)
	   {       
		   for(long y = 0; y < ysize; y++)
		   {   
			   for(long x = 0; x < xsize; x++,j++)
			   {
			      temp[z][y][x] = array[j];
			   }
			}
	   }
   }
   type = CMATRIX3D;
}

/*************************************************************************************
  Free all data associated with a variable
**************************************************************************************/

short Variable::FreeData(void)
{
   if(data == NULL)
      return(OK);

   if(aliasVar)
   {
      if(!permanent && !system)
      {
         aliasVar->aliasRef--;
   //      TextMessage("\nFreeData: Removing reference to '%s' from '%s' (%hd)",aliasVar->name,name,aliasVar->aliasRef);
      }
      aliasVar = NULL;
      data = NULL;
      type = NULL_VARIABLE;
      scope = LOCAL;

      return(OK);
   }
      
   if(aliasRef != 0)
   {
      ErrorMessage("variable '%s' can't be deleted - it is referenced by %d other variables",name,aliasRef);
      return(ERR);
   }
   
   switch(type)
   {
      case(CMATRIX2D):
         FreeCMatrix2D((complex**)data);
         break;   
      case(MATRIX2D):
         FreeMatrix2D((float**)data);
         break;
      case(DMATRIX2D):
         FreeDMatrix2D((double**)data);
         break;
      case(CMATRIX3D):
         FreeCMatrix3D((complex***)data);
         break;   
      case(MATRIX3D):
         FreeMatrix3D((float***)data);
         break;   
      case(CMATRIX4D):
         FreeCMatrix4D((complex****)data);
         break;   
      case(MATRIX4D):
         FreeMatrix4D((float****)data);
         break;   
      case(LIST):
         for(int i = 0; i < dim1; i++)
            delete [] ((char**)data)[i]; 
         delete [] data;  
         break;
      case(LIST2D):
			delete (List2DData*)data;
			break;
      case(FLOAT64):
         delete (double*)data;
         break;  
      case(FLOAT32):
         delete (float*)data;
         break;  
      case(INTEGER):
         delete (long*)data;
         break;  
      case(COMPLEX):
         delete (complex*)data;
         break;  
      case(QUOTED_STRING):
      case(UNQUOTED_STRING):      
         delete[] (char*)data;
         break;     
      case(PROCEDURE):
      {
         ProcedureInfo *proc = (ProcedureInfo*)data;
 //  TextMessage("Cache deleted : %s:%s %Xl\n",proc->macroName, proc->procName,(long)proc->procedure);

         if(proc->macroName) delete [] proc->macroName;
         if(proc->macroPath) delete [] proc->macroPath;
         if(proc->procedure) delete [] proc->procedure;
         if(proc->procName)  delete [] proc->procName;
         delete proc;
         break;
      }
      case(STRUCTURE):
      {
         Variable* struc = GetStruct();
         if(struc)
         {
            struc->RemoveAll();
            delete struc;
         }
         break;
      }
      case(STRUCTURE_ARRAY):
      {
         Variable *struc;
         Variable* strucArray = (Variable*)data;
         int size = GetDimX();
         for(int i = 0; i < size; i++)
         {
            struc = strucArray[i].GetStruct();
				if(struc)
				{
					struc->RemoveAll();
					delete struc;
				}
            strucArray[i].SetType(NULL_VARIABLE);
         }
         delete [] strucArray;
         break;
      }
      case(CLASS):
      {  
         ClassData* cd = (ClassData*)data;
         if(cd)
            delete cd;
         break;
      }
      case(CLASSITEM):
      {
         ClassItem* ci = (ClassItem*)data;

         if(ci->args)
            delete [] ci->args;
         if(ci->name)
            delete [] ci->name;
            
         delete ci;
         break;
      }
		case(NULL_VARIABLE):
			break;
		default:
		{
			TextMessage("Free Data: Variable '%s' of type '%d' not found\n",this->name, this->type);
		}
         
   }
   data = NULL;
   type = NULL_VARIABLE;
   dim1 = dim2 = dim3 = dim4 = 0;
   scope = LOCAL;

   return(OK);
}
      

/*************************************************************************************
  Remove all variables from a variable list
**************************************************************************************/

void Variable::RemoveAll()
{
   Variable *var = next;
   while(var != (Variable*)0)
   {
      Variable *v = var;
      var = var->next;
      delete v; 
      v = NULL;
   }
   next = (Variable*)0;
} 

 
/*************************************************************************************
  Remove a variable from variable list
**************************************************************************************/

void Variable::Remove()
{
// Free data associated with variable ***************
   FreeData();

// Delete variable name memory (Added V2.2.4 to fix memory leak)
   if(name) 
		delete [] name;
	name = 0;

// If an alias then decrement alias reference counter
   if(aliasVar && !permanent && !system)
   {
      aliasVar->aliasRef--;
 //     TextMessage("\nRemove: %s (alias to %s) (%hd)",name,aliasVar->name,aliasVar->aliasRef);
      aliasVar = NULL;
   }
      
// Check for isolated or first variable *************
   if(last == NULL)
      return;
      
// Bridge gap in linked list ************************
   if(!next) // Last in list
   {
      if(last)
   	 last->next = (Variable*)0;
   }
   else
   {
      if(last)
      {
  		   last->next = next;
   	   next->last = last;
   	}
   }
}


/*************************************************************************************
  Find a variable in a list
**************************************************************************************/

Variable* Variable::Get(short restrictions, char name[], short &type)
{
   if(restrictions & ALL_VAR)
   {
	   for(Variable *var = next; var != NULL; var = var->next)
	   {
	      if(var->name && !strcmp(var->name,name))
	      {
	         type = var->GetType();
	         return(var);
	      }
	   }
	}
	else if(restrictions & NOT_ALIAS)
	{
	   for(Variable *var = next; var != NULL; var = var->next)
	   {
	      if(var->name && !strcmp(var->name,name) && var->aliasVar == NULL)
	      {
	         type = var->GetType();
	         return(var);
	      }
	   }
	}
	else if(restrictions & NOT_ALIAS_SPECIFY_TYPE)
	{
	   for(Variable *var = next; var != NULL; var = var->next)
	   {
	      if(var->name && !strcmp(var->name,name) && var->aliasVar == NULL && var->GetType() == type)
	      {
	         return(var);
	      }
	   }
	}
					
   return((Variable*)0);
}


/*************************************************************************************
  Find a variable in a list
**************************************************************************************/

Variable* Variable::Get(char name[], char *data)
{

	for(Variable *var = next; var != NULL; var = var->next)
	{
	   if(var->name && !strcmp(var->name,name) && var->data == data)
	   {
	      return(var);
	   }
	}
					
   return((Variable*)0);
}

/*************************************************************************************
  Find a variable in a list
**************************************************************************************/

Variable* Variable::Get(char name[])
{

	for(Variable *var = next; var != NULL; var = var->next)
	{
	   if(var->name && !strcmp(var->name,name))
	   {
	      return(var);
	   }
	}
					
   return((Variable*)0);
}

/*************************************************************************************
  Find a variable in a list
**************************************************************************************/

Variable* Variable::GetNext(short &type)
{
   Variable *var = next;
   if(var != NULL)
   {
      type = var->GetType();
      return(var);
   }
   return(NULL);
}

/*************************************************************************************
   Make a class variable
**************************************************************************************/

void Variable::MakeClass(short classType, void* classData)
{	
   ClassData *temp;

   FreeData();

   temp = new ClassData;
   temp->type = classType;
   temp->data = classData;
   long *code = (long*)temp->data;
   temp->code = *code;

   data = (char*)temp;

   type = CLASS;
   aliasVar = NULL;
   aliasRef = 0; 
}


/*************************************************************************************
   Make a class item variable
**************************************************************************************/

void Variable::MakeClassItem(short itemType, char *name, char *args)
{	
   ClassItem *temp;
   FreeData();

   temp = new ClassItem;
   
   temp->type = itemType;
   temp->name = new char[strlen(name)+1];
   strcpy(temp->name,name);
   if(args)
   {
      temp->args = new char[strlen(args)+1];
      strcpy(temp->args,args);
   }
   else
   {
      temp->args = new char[1];
      temp->args[0] = '\0';
   }

   data = (char*)temp;
   type = CLASSITEM;
   aliasVar = NULL;
   aliasRef = 0; 
}
