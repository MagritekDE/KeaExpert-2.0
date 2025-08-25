#include "stdafx.h"
#include "carg.h"
#include "evaluate_simple.h"
#include "structure.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "evaluate.h"
#include "macro_class.h"
#include "mymath.h"
#include "list_functions.h"
#include "scanstrings.h"
#include "variablesOther.h"
#include "control.h"
#include "memoryLeak.h"

short ProcessStructArrayAssignment(char* expression, short start, Variable *dstVar, Variable *srcVar, long x, long *xa);
int SetStructureValues(Interface* itfc ,char args[]);

// Extract an item from a structure by index returning the value and the name

int GetStructureItem(Interface *itfc, char arg[])
{
   int index;
   short nrArgs;
   Variable strucVar;
   Variable *struc, *svar;

   // Get variable list name *******************************
   if ((nrArgs = ArgScan(itfc, arg, 2, "index", "ee", "vl",&strucVar,  &index)) < 0)
      return(nrArgs);

   if (strucVar.GetType() == STRUCTURE)
   {
      struc = strucVar.GetStruct();
      svar = struc->next;
      // Count entries in structure
      int width = 0;
      while (svar != NULL)
      {
         width++;
         svar = svar->next;
      }
      if (index >= 0 && index < width)
      {
         svar = struc->next;
         for (int i = 0; i < width; i++)
         {
            if (i == index)
            {   
					itfc->retVar[1].MakeAndSetString(svar->GetName()); 
					itfc->retVar[2].FullCopy(svar);  
					itfc->nrRetValues = 2;
               return(OK);
            }
            svar = svar->next;
         }
      }
      else
      {
         ErrorMessage("index out of range");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("invalid data type for argument 1 - should be a structure");
      return(ERR);
   }
   return(OK);
}

// Check the integraty of a structure
//
//int CheckStructure(Interface *itfc, char arg[])
//{
// int index;
//   short nrArgs;
//   Variable strucVar;
//   Variable *struc, *svar;
//	CText itemName;
//	char localName[100];
//
//   if ((nrArgs = ArgScan(itfc, arg, 1, "structure", "e", "v",&strucVar)) < 0)
//		return(nrArgs);
//
//   if (strucVar.GetType() == STRUCTURE)
//   {
//      struc = strucVar.GetStruct();
//      svar = struc->next;
//      // Count entries in structure
//      int width = 0;
//      while (svar != NULL)
//      {
//         width++;
//         svar = svar->next;
//      }
//		// Search for the item
//      svar = struc->next;
//      for (int i = 0; i < width; i++)
//      {
//			char* name = svar->GetName();
//			strcpy(localName,name);
//         svar = svar->next;
//      }
//      itfc->retVar[1].MakeAndSetFloat(width); 
//		itfc->nrRetValues = 1;
//		return(OK);
//   
//   }
//   else
//   {
//      ErrorMessage("invalid data type for argument 1 - should be a structure");
//      return(ERR);
//   }
//   return(OK);
//}

// Find the index of a structure item - return -1 is not found

int GetStructureItemIndex(Interface *itfc, char arg[])
{
   int index;
   short nrArgs;
   Variable strucVar;
   Variable *struc, *svar;
	CText itemName;

   // Get variable list name *******************************
   if ((nrArgs = ArgScan(itfc, arg, 1, "structure, name", "ee", "vt",&strucVar, &itemName)) < 0)
      return(nrArgs);

   if (strucVar.GetType() == STRUCTURE)
   {
      struc = strucVar.GetStruct();
      svar = struc->next;
      // Count entries in structure
      int width = 0;
      while (svar != NULL)
      {
         width++;
         svar = svar->next;
      }
		// Search for the item
      svar = struc->next;
      for (int i = 0; i < width; i++)
      {
			if(!strcmp(svar->GetName(), itemName.Str()))
         {      
            itfc->retVar[1].MakeAndSetFloat(i); 
				itfc->nrRetValues = 1;
            return(OK);
         }
         svar = svar->next;
      }
      itfc->retVar[1].MakeAndSetFloat(-1); 
		itfc->nrRetValues = 1;
		return(OK);
   
   }
   else
   {
      ErrorMessage("invalid data type for argument 1 - should be a structure");
      return(ERR);
   }
   return(OK);
}

/******************************************************************************
* Generate a structure from a list of variables or from one of the standard
* lists i.e. local, winvar, global or all or just make a blank structure
******************************************************************************/

int MakeStructure(Interface* itfc, char arg[])
{
   CText list;
   short nrArgs,nrArgs2;
	Variable *varBase,*struc,*memberVar;
   Variable *var;
   Variable argVar;
   CArg carg;
   CArg carg2(';');
	extern bool IsValidVariableName(char *str);
	bool processEscapesBak;

   nrArgs = carg.Count(arg);
   nrArgs2 = carg2.Count(arg);

	processEscapesBak = itfc->processEscapes;
	itfc->processEscapes = false; // Make sure we don't escape characters during the conversion.

   if(nrArgs == 0 && nrArgs2 == 0) // Make an empty structure
   {
      itfc->retVar[1].MakeStruct();
   }
   else if(nrArgs == 1 && nrArgs2 == 1) // Make a structure from window or local variables, or make a structure from an assignment or single variable
   {
      Variable result;
		char name[MAX_STR];
		char value[MAX_STR];

		if(ParseAssignmentString(carg.Extract(1),name,value) == OK) // An single assignment a = 23
		{
		   itfc->retVar[1].MakeStruct();
         Variable* struc = itfc->retVar[1].GetStruct();
			if(Evaluate(itfc,RESPECT_ALIAS,value,&result) < 0) 
			{
				itfc->processEscapes = processEscapesBak;
				return(ERR);
			}

			if(IsValidVariableName(name))
			{
				memberVar = struc->Add(NULL_VARIABLE,name);
				CopyVariable(memberVar,&result,FULL_COPY);
			}
			else
			{
				ErrorMessage("Invalid variable name '%s'",name);
				itfc->processEscapes = processEscapesBak;
				return(ERR);
			}
		}
		else
		{

		// Get variable list name *******************************
			if((nrArgs = ArgScan(itfc,arg,0,"list/local/winvar","e","v",&argVar)) < 0)
				return(nrArgs);

			if(argVar.GetType() == UNQUOTED_STRING)
			{
				list = argVar.GetString();

				if(list == "winvar" || list == "windowvar") // Fill structure with current window variables
				{
					if(GetGUIWin())
					{
						itfc->retVar[1].MakeStruct();
						struc = itfc->retVar[1].GetStruct();
						varBase = &(GetGUIWin()->varList);
						for(var = varBase->next; var != NULL; var = var->next)
						{
							memberVar = struc->Add(STRUCTURE,var->GetName());
							if(CopyVariable(memberVar,var,FULL_COPY) == ERR)
							{
								itfc->processEscapes = processEscapesBak;
								return(ERR);
							}
						}
					}
					else
					{
						itfc->retVar[1].MakeNullVar();
					}
				}
				else if(list == "local") // Fill structure with local variables
				{
					if(itfc && itfc->macro)
					{
						itfc->retVar[1].MakeStruct();
						struc = itfc->retVar[1].GetStruct();
						varBase = &(itfc->macro->varList);
						for(var = varBase->next; var != NULL; var = var->next)
						{
							memberVar = struc->Add(STRUCTURE,var->GetName());
							if(CopyVariable(memberVar,var,FULL_COPY) == ERR)
							{
								itfc->processEscapes = processEscapesBak;
								return(ERR);
							}
						}
					}
					else
					{
						itfc->retVar[1].MakeNullVar();
					}
				}
				else // Some other string so just make it an element
				{
					itfc->retVar[1].MakeStruct();
					Variable* struc = itfc->retVar[1].GetStruct();
					if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(1),&result) < 0) 
					{
						itfc->processEscapes = processEscapesBak;
						return(ERR);
					}

					if(IsValidVariableName(carg.Extract(1)))
					{
						memberVar = struc->Add(NULL_VARIABLE,carg.Extract(1));
						CopyVariable(memberVar,&result,FULL_COPY);
					}
					else
					{
						ErrorMessage("Invalid variable name '%s'",name);
						itfc->processEscapes = processEscapesBak;
						return(ERR);
					}
				}
			}

			else if(argVar.GetType() == LIST) // Converts a parameter list to a structure
			{
				Variable output;
				output.MakeStruct();
				struc = output.GetStruct();
				char** lst = argVar.GetList();
				int dim = argVar.GetDimX();
				for(int i = 0; i < dim; i++)
				{
					char* entry = lst[i];
					int sz = strlen(entry)+1;
					char* srcName = new char[sz];
					char* srcValue = new char[sz];
					if(ParseAssignmentString(entry,srcName,srcValue) == ERR)
					{
						delete [] srcName;
						delete [] srcValue;
						itfc->processEscapes = processEscapesBak;
						ErrorMessage("Entry %d in list is not a parameter string",i);
						return(ERR);
					}
					memberVar = struc->Add(STRUCTURE,srcName);
					Variable result;
					short err;
					if((err = Evaluate(itfc, RESPECT_ALIAS, srcValue, &result)) < 0)
					{
						delete [] srcName;
						delete [] srcValue;
						itfc->processEscapes = processEscapesBak;
						return(err);
					}
					if(CopyVariable(memberVar,&result,FULL_COPY) == ERR)
					{
						delete [] srcName;
						delete [] srcValue;
						itfc->processEscapes = processEscapesBak;
						return(ERR);
					}
					delete [] srcName;
					delete [] srcValue;
				}
				itfc->retVar[1].Assign(&output);
				output.NullData();
			}
			else // A single valid variable which is not a string or list
			{
				itfc->retVar[1].MakeStruct();
				Variable* struc = itfc->retVar[1].GetStruct();
				if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(1),&result) < 0) 
				{
					itfc->processEscapes = processEscapesBak;
				   return(ERR);
				}

				if(IsValidVariableName(carg.Extract(1)))
				{
					memberVar = struc->Add(NULL_VARIABLE,carg.Extract(1));
					CopyVariable(memberVar,&result,FULL_COPY);
				}
				else
				{
					ErrorMessage("Invalid variable name '%s'",name);
					itfc->processEscapes = processEscapesBak;
					return(ERR);
				}
			}
		}
   }
   else if(nrArgs > 1 && nrArgs2 == 1) // A comma delimited list of strings ("a","b","c") variable names  (a,b,c) or assignments (a=23,b="hi",c=[1,2,3])
   {
      Variable result;
		Variable returnVar;

		returnVar.MakeStruct();
      Variable* struc = returnVar.GetStruct();

		char name[MAX_STR];
		char value[MAX_STR];

		for(int i = 1; i <= nrArgs; i++)
      {
			if(ParseAssignmentString(carg.Extract(i),name,value) == OK) // It's a list of assignments a = 12, b = "hi", c = [1,2,3,4]
			{
				if(Evaluate(itfc,RESPECT_ALIAS,value,&result) < 0) 
				{
					itfc->processEscapes = processEscapesBak;
				   return(ERR);
				}
				if(IsValidVariableName(name))
				{
					memberVar = struc->Add(NULL_VARIABLE,name);
					CopyVariable(memberVar,&result,FULL_COPY);
				}
				else
				{
					itfc->processEscapes = processEscapesBak;
					ErrorMessage("Invalid variable name '%s'",name);
					return(ERR);
				}
			}
			else // It's a list of variables so convert to structure elements
			{
				if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(i),&result) < 0) 
				{
					itfc->processEscapes = processEscapesBak;
				   return(ERR);
				}
				if(IsValidVariableName(carg.Extract(i)))
				{
					memberVar = struc->Add(NULL_VARIABLE,carg.Extract(i));
					CopyVariable(memberVar,&result,FULL_COPY);
				}
				else
				{
					itfc->processEscapes = processEscapesBak;
					ErrorMessage("Invalid variable name '%s'",name);
					return(ERR);
				}
		
			}
		}
		itfc->retVar[1].Assign(&returnVar);
		returnVar.NullData();
   }
   else if(nrArgs >= 1 && nrArgs2 > 1) // A comma delimited list of strings ("a","b","c") variable names  (a,b,c) or assignments (a=23,b="hi",c=[1,2,3])
   {
		Variable returnVar;

      // Check to see if how many elements there are - the last one might be empty e.g. s = struct(a = 23;) this defines a single element structure array
		char *str1 = carg2.Extract(nrArgs2);
		CArg carg3;
		int nrArgs3 = carg3.Count(str1);
      if(nrArgs3 == 0)
         nrArgs2--;

     // Make the structure array
		returnVar.MakeStructArray(nrArgs2);

		Variable *varArray = (Variable*)(returnVar.GetData());

		char name[MAX_STR];
		char value[MAX_STR];

		for(int i = 1; i <= nrArgs2; i++)
		{
			char *str1 = carg2.Extract(i);
			CArg carg3;
			int nrArgs3 = carg3.Count(str1);
         if(nrArgs3 == 0)
         {
            itfc->processEscapes = processEscapesBak;
				ErrorMessage("Empty structure array element");
				return(ERR);
         }
		   Variable* result = varArray[i-1].GetStruct();

			for(int j = 1; j <= nrArgs3; j++)
			{
				if(ParseAssignmentString(carg3.Extract(j),name,value) == OK) // It's a list of assignments a = 12, b = "hi", c = [1,2,3,4]
				{
					if(Evaluate(itfc,RESPECT_ALIAS,value,result) < 0) 
					{
						itfc->processEscapes = processEscapesBak;
						return(ERR);
					}
					if(IsValidVariableName(name))
					{
						memberVar = result->Add(NULL_VARIABLE,name);
						CopyVariable(memberVar,result,FULL_COPY);
					}
					else
					{
						itfc->processEscapes = processEscapesBak;
						ErrorMessage("Invalid variable name '%s'",name);
						return(ERR);
					}
				}
			}

		}
		itfc->retVar[1].Assign(&returnVar);
		returnVar.NullData();
	}

   itfc->nrRetValues = 1;

	itfc->processEscapes = processEscapesBak;

   return(OK);

}

/******************************************************************************
* Append a structure to an existing one
******************************************************************************/

int AppendToStructure(Interface* itfc, char arg[])
{
	short nrArgs;
	Variable *origVar, appendVar;
	CText origVarName;

// Get structures (name and value)
	if((nrArgs = ArgScan(itfc,arg,2,"structure_to_modify, structure_to_append","ce","tv",&origVarName, &appendVar)) < 0)
		return(nrArgs);

	short type;
	origVar = GetVariable(itfc,ALL_VAR,origVarName.Str(),type);

	if(origVar->GetType() != STRUCTURE || appendVar.GetType() != STRUCTURE)
	{
		ErrorMessage("Arguments should be structures");
		return(ERR);
	}

   Variable* strucA = origVar->GetStruct();
   Variable* strucB = appendVar.GetStruct();
   Variable *var,*memberVar;

   for(var = strucB->next; var != NULL; var = var->next)
   {
      memberVar = strucA->Add(STRUCTURE,var->GetName());
      if(CopyVariable(memberVar,var,FULL_COPY) == ERR)
         return(NULL);
   }

	return(OK);
}

/******************************************************************************
* Generate a structure array given its dimension
******************************************************************************/

int MakeStructureArray(Interface* itfc, char arg[])
{
   short nrArgs;

	int arraySize = 1;

// Get array size *******************************
	if((nrArgs = ArgScan(itfc,arg,1,"dimension","e","d",&arraySize)) < 0)
		return(nrArgs);

	itfc->retVar[1].MakeStructArray(arraySize);

   itfc->nrRetValues = 1;

   return(OK);

}


/**************************************************************************************
  Take selected variables and add then to a returned structure. If keyList is defined
  then only include those variables in the resultant list otherwise add all local
  variables.

  Syntax: STRUC par = mkparstruct([keyList])

***************************************************************************************/

int MakeParameterStructure(Interface* itfc ,char arg[])
{
   short nrArgs;
   Variable keyVar;
   CText strType = "all";
   int type;

  // Get the key list *******************
   if ((nrArgs = ArgScan(itfc, arg, 0, "keys, [type]", "ee", "vt", &keyVar, &strType)) < 0)
      return(nrArgs);

   if (strType == "all")
      type = 4096;
   else if (strType == "string")
      type = 1;
   else if (strType == "float" || strType == "float32")
      type = 4;
   else if (strType == "complex")
      type = 5;
   else if (strType == "list")
      type = 8;
   else if (strType == "matrix")
      type = 6;
   else if (strType == "cmatrix")
      type = 7;
   else if (strType == "struct")
      type = 16;
   else if (strType == "float64")
      type = 20;
   else
   {
      ErrorMessage("invalid variable type");
      return(ERR);
   }


// Scan through the local variable list counting entries
   if(itfc->macro)
   {
		// Make the returned structure
		itfc->retVar[1].MakeStruct();
      Variable* struc = itfc->retVar[1].GetStruct();
      itfc->nrRetValues = 1;
 
   // Scan through the local variable list adding entries to parameter list
      Variable *varBase = &(itfc->macro->varList);
		Variable *var = 0;
      for (var = varBase->next; var != NULL; var = var->next) // Loop over local variables
      {
         if (nrArgs >= 1) // Use the key list to restrict the search
         {
            char** keyList = keyVar.GetList();
            long keyEntries = keyVar.GetDimX();
            bool found = false;
            for (int j = 0; j < keyEntries; j++) // Loop over keys
            {
               if (!strcmp(var->GetName(), keyList[j]))
                  found = true;
            }
            if (!found)
               continue;
         }

         // Ignore built in variables
         if(var->GetData() && (!strcmp(var->GetName(),"nrArgs") || !strcmp(var->GetName(),"parentCtrl"))) // Ignore these variables
            continue;

         // Restrict the type of data returned
         if (type == 4096 || var->GetType() == type)
         {
            Variable* memberVar = struc->Add(NULL_VARIABLE, var->GetName());
            CopyVariable(memberVar, var, FULL_COPY);
         }
      }
      return(OK);
   }
   else
   {
      ErrorMessage("can only be used in a macro procedure");
      return(ERR);
   }
}

/*********************************************************************
*    Make a class (structure with functions)                         *
*********************************************************************/

int MakeClass(Interface *itfc, char arg[])
{
	short nrArgs,err;
	CText class_initialiser;
	Variable *struc,*alias;
	extern short CheckForMacroProcedure(Interface *itfc, char *command, char *arg);

	// Count the number of arguments
   CArg carg;
	nrArgs = carg.Count(arg);
	if(nrArgs == 0)
	{
		ErrorMessage("Correct argument syntax: class_name, arg1, agr2 ...");
		return(ERR);
	}

	// Get the class name *******************************
   if((err = ArgScan(itfc,arg,1,"class_initialise","e","t",&class_initialiser)) < 0)
      return(err);

	// Make a structure.
	Variable *sParent = new Variable;
	sParent->MakeStruct();
	struc = sParent->GetStruct();

	// Generate the name of the initialistion function for the class
	CText fullInitName;
	CText initName;

	fullInitName = class_initialiser.Str();
	int pos = fullInitName.Search(0,':');
	if(pos == -1)
	{
		ErrorMessage("The first argument should include the procedure name for the class initialiser with ':' i.e. myMacro:classInit or :classInit");
		return(ERR);
	}
	initName = fullInitName.End(pos+1);

	// Add a local variable which is an alias to the structure
	// The variable name is "localStructName" which should be unique
	alias = AddLocalVariable(itfc,STRUCTURE,"localStructName");
	alias->MakeAndSetAlias(sParent);

	// Add the name 'init' to the structure
	Variable *member = struc->Add(UNQUOTED_STRING,initName.Str());
	member->MakeAndSetString(fullInitName.Str());

	// Generate a string which is the structure's local name followed by the argument list to init procedure
	CText args;
	args = "localStructName";
	for(int i = 1; i < nrArgs; i++)
	{
		args = args + "," + carg.Extract(i+1);
	}

	// Run the initialisation procedure (adding new entries to the structure)
	short r = CheckForMacroProcedure(itfc,fullInitName.Str(),args.Str());				

	// Return the structure
	itfc->retVar[1].AssignStructure(sParent);
	itfc->nrRetValues = 1;

   // Clean up by removing the alias variable but not the contents
	alias->Remove();
	alias->SetNull();
	delete alias;

   // Remove the internal variable which is now assigned to the return var
	sParent->SetNull();
	delete sParent;

	return(OK);
}


/*********************************************************************
*    Save the association information about class names and files    
*    used in the current project.
*    Format of the passed list should be:
*    class_name, class_macro, ...
*********************************************************************/

int SetClassInfo(Interface *itfc, char args[])
{
	int err;
	Variable classInfo;
	extern CText *gClassMacro, *gClassName;
	extern int gNrClassDeclarations;

	// Get the class name *******************************
   if((err = ArgScan(itfc,args,1,"class_info","e","v",&classInfo)) < 0)
      return(err);

	if(classInfo.GetType() == LIST)
	{
		int sz = classInfo.GetDimX();
		char **lst = classInfo.GetList();
		if(gClassMacro)
			delete [] gClassMacro;
		if(gClassName)
			delete [] gClassName;

		gClassMacro = new CText[sz];
		gClassName = new CText[sz];
		gNrClassDeclarations = sz;

		for(int i = 0; i < sz; i++)
		{
			char* entry = lst[i];
			int sz = strlen(entry)+1;
			char* className = new char[sz];
			char* macroName = new char[sz];
			if(ParseAssignmentString(entry,className,macroName) == ERR)
			{
				delete [] className;
				delete [] macroName;
				ErrorMessage("Entry %d in list is not a parameter string",i);
				return(ERR);
			}
			gClassMacro[i] = macroName;
			gClassName[i] = className;
			delete [] className;
		   delete [] macroName;
		}
	}
	else
	{
		ErrorMessage("Invalid data type - should be a parameter list");
		return(ERR);
	}

	itfc->nrRetValues = 0;
	return(OK);
}



/*********************************************************************
*    Given a structure with defined names add values                 *
*********************************************************************/

int SetStructureValues(Interface* itfc ,char args[])
{

   short nrArgs;
   Variable varIn;
   CArg carg;
   Variable strucIn;

   nrArgs = carg.Count(args);

   if(nrArgs <= 1)
   {
      ErrorMessage("At least 2 arguments required");
      return(ERR);
   }

   if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(1),&strucIn) < 0) 
     return(ERR);

   if(strucIn.GetType() == STRUCTURE)
   {
      itfc->retVar[1].MakeStruct();
      Variable *sOut = itfc->retVar[1].GetStruct();
      Variable *sIn = strucIn.GetStruct();
      if(sIn->Count()-1 != nrArgs-1)
      {
         ErrorMessage("Incorrect number of structure members");
         return(ERR);
      }

      Variable *srcIn = sIn->next;
      Variable *srcOut = sOut;
      Variable *memberVar;

      for(int i = 2; i <= nrArgs; i++)
      {
         if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(i),&varIn) < 0) 
            return(ERR);

         if(!srcIn || !(srcIn->GetName()))
         {
            ErrorMessage("Too few input structure members");
            return(ERR);
         }
         memberVar = srcOut->Add(varIn.GetType(),srcIn->GetName());
         if(CopyVariable(memberVar,&varIn,FULL_COPY) == ERR)
            return(ERR);
         srcIn = srcIn->next;
      }
   }
   else if(strucIn.GetType() == STRUCTURE_ARRAY)
   {
      int size = strucIn.GetDimX();
      itfc->retVar[1].MakeStructArray(size);
      Variable *sArrayOut = itfc->retVar[1].GetStruct();
      Variable *sArrayIn = strucIn.GetStruct();

      Variable *sIn = sArrayIn[0].GetStruct()->next;
      int nr = sIn->Count();

      if(nr*size != nrArgs-1)
      {
         ErrorMessage("There should be %d input structure members - there are %hd",nr*size,nrArgs-1);
         return(ERR);
      }

      Variable *memberVar;

      for(int j = 0; j < size; j++)
      {
         Variable *sIn = sArrayIn[j].GetStruct()->next;
         Variable *sOut = sArrayOut[j].GetStruct();
         int nr = sIn->Count();

         for(int i = 0; i < nr; i++)
         {
            if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(j*nr+i+2),&varIn) < 0) 
               return(ERR);

            memberVar = sOut->Add(varIn.GetType(),sIn->GetName());
            if(CopyVariable(memberVar,&varIn,FULL_COPY) == ERR)
               return(ERR);
            sIn = sIn->next;
         }
      }
   }
   else
   {
      ErrorMessage("First argument should be a structure or structure array");
      return(ERR);
   }

   itfc->nrRetValues = 1;      

   return(OK);
}

/*********************************************************************
*       Assign data in srcVar to the structure array in dstVar[x]    *
*********************************************************************/

short ProcessStructArrayAssignment(char* expression, short start, Variable *dstVar, Variable *srcVar, long x, long *xa)
{
  int i,j;
   long size = strlen(expression);
   char* member = new char[size+1];

// Extract structure name
   for(i = start; i < size; i++)
   {
      if(expression[i] == '-' && expression[i+1] == '>') // ARROW
         break;
   }

   if(i == 0)
   {
      ErrorMessage("Invalid structure name '%s'",dstVar->GetName());
      goto err;
   }

// Extract member name
   for(j = i+2; j < size; j++)// ARROW
   {
      member[j-i-2] = expression[j];// ARROW
   }
   member[j-i-2] = '\0';// ARROW

// Extract structure array value
   Variable *var = (Variable*)dstVar->GetData();
   Variable *strucData = (&(var[x]))->GetStruct();
   Variable *memberVar;

// If a member is not given then src variable must be a structure or its an error   
   if(j-i-2 == 0)// ARROW
   {
      if(srcVar->GetType() == STRUCTURE) // Add a preexisting structure
      {
         Variable *srcTemp,*dstTemp;
         Variable *srcStruc = srcVar->GetStruct();
         Variable *var = (Variable*)dstVar->GetData();
         Variable *dstStruc = (&(var[x]))->GetStruct();

         dstStruc->RemoveAll(); // Erase any existing values

        // Add new elements to dstStruc
         for(srcTemp = srcStruc->next; srcTemp != NULL; srcTemp = srcTemp->next)
         {
            memberVar = dstStruc->Add(STRUCTURE,srcTemp->GetName());
            if(CopyVariable(memberVar,srcTemp,FULL_COPY) == ERR)
            {
               goto err;
            }
         }
         goto ok;
      }
      else
      {
         ErrorMessage("Member name not defined for structure '%s'",expression);
         goto err;
      }
      goto ok;
   }

// A member has been given either replace existing value or add new one
   Variable *newVar;
   for(var = strucData->next; var != NULL; var = var->next)
   {
      if(!strcmp(var->GetName(),member)) // Old member found
      {
         if(CopyVariable(var,srcVar,FULL_COPY) == ERR) // Replace with new member
            goto err;
         goto ok;
      }
   }

// New member not found so add 
// new variable to the structure
   newVar = strucData->Add(STRUCTURE,member);
   if(CopyVariable(newVar,srcVar,FULL_COPY) == ERR)
      goto err;
   goto ok;	
 
// Exit options

err:
   delete [] member;
   return(ERR);

ok:
   delete [] member;
   return(OK);

}


// Make local or window variables from a strucutre

int AssignStructure(Interface* itfc, char args[])
{
   short nrArgs;
   Variable struc,*strucData,*var,*newVar;
   CText list = "local";

// Get structure name *******************************
   if((nrArgs = ArgScan(itfc,args,1,"structure, variable_list","ee","vt",&struc,&list)) < 0)
      return(nrArgs);

   itfc->nrRetValues = 0;

// Extract structure and assign to list

   if(list == "local")
   {
      if(!itfc || !itfc->macro)
         return(OK);

      Variable *varLocal = &(itfc->macro->varList);

      if(struc.GetType() == STRUCTURE)
      {
         strucData = struc.GetStruct();

         for(var = strucData->next; var != NULL; var = var->next)
         {
            newVar = varLocal->Get(var->GetName());
            if(!newVar)
               newVar = varLocal->Add(var->GetType(),var->GetName());
            if(CopyVariable(newVar,var,FULL_COPY) == ERR)
               return(ERR);
         }
      }
   }
   else if(list == "winvar")
   {
      if(!GetGUIWin())
         return(OK);

      Variable *varLocal = &(GetGUIWin()->varList);

      if(struc.GetType() == STRUCTURE)
      {
         strucData = struc.GetStruct();

         for(var = strucData->next; var != NULL; var = var->next)
         {
            newVar = varLocal->Get(var->GetName());
            if(!newVar)
               newVar = varLocal->Add(var->GetType(),var->GetName());
            if(CopyVariable(newVar,var,FULL_COPY) == ERR)
               return(ERR);
         }
      }
   }
   else
   {
      ErrorMessage("Invalid list name or list not present");
      return(ERR);
   }

   return(OK);
}

/*******************************************************************
* Extract the structure member from string operand
* using the syntax structure.member.
* Return the member in the variable result
*
* If there are a string of such references then evaluate them
********************************************************************/

short EvaluateStructureMember(Interface *itfc, char *operand, Variable *result)
{
   int i,j;
   long s = strlen(operand);
   char* struc = new char[s+1];
   char* member = new char[s+1];

// Extract structure name
   for(i = 0; i < s; i++)
   {
      if(operand[i] == '.')
         break;
      struc[i] = operand[i];
   }
   struc[i] = '\0';

   if(i == 0)
   {
      ErrorMessage("Invalid structure name '%s'",struc);
      goto err;
   }

// Extract member name
   for(j = i+1; j < s; j++)
   {
      member[j-i-1] = operand[j];
   }
   member[j-i-1] = '\0';

   if(j-i-1 == 0)
   {
      ErrorMessage("Invalid structure member name '%s'",member);
      goto err;
   }

// Get structure
   short type;
   Variable *var;

   var = GetVariable(itfc,ALL_VAR,struc,type);

//// A test to see if I can access class member functions using the structure syntax
//   if(var && type == CLASS)
//   {
//      extern short RunClassProc(Interface *itfc, Variable *var, char *func, char *args);
//      if(RunClassProc(itfc, var, member, "") == ERR)
//         return(ERR);
//      if(itfc->nrRetValues >= 1)
//      {
//         if(CopyVariable(result,&itfc->retVar[1],FULL_COPY) == ERR)
//            return(ERR);
//      }
//      delete [] struc;
//      delete [] member;
//      return(OK);
//   }

   if(!var || type != STRUCTURE)
   {
      ErrorMessage("'%s' is not a structure",struc);
      goto err;
   }


// Get the member
   Variable *strucData = var->GetStruct();
   if(!strucData)
   {
      ErrorMessage("'%s' is an invalid structure",struc);
      goto err;
   }

   for(var = strucData->next; var != NULL; var = var->next)
   {
      if(!strcmp(var->GetName(),member))
      {
         delete [] struc;
         delete [] member;
         if(CopyVariable(result,var,FULL_COPY) == ERR)
            return(ERR);
         return(OK);
      }
   }

   ErrorMessage("'%s' is not a member of '%s'",member,struc);

err:

   delete [] struc;
   delete [] member;
   return(ERR);
}

/*******************************************************************
* Assign a new variable to a structure. If the member doesn't
* exist make a new one.
********************************************************************/

short AssignStructureMember(Interface *itfc,char *dstName, Variable* srcVar)
{
   int i,j;
   long s = strlen(dstName);
   char* struc = new char[s+1];
   char* member = new char[s+1];

// Extract structure name
   for(i = 0; i < s; i++)
   {
      if(dstName[i] == '-' && dstName[i+1] == '>') // ARROW
         break;
      struc[i] = dstName[i];
   }
   struc[i] = '\0';

   if(i == 0)
   {
      ErrorMessage("Invalid structure name '%s'",struc);
      goto err;
   }

// Extract member name
   for(j = i+2; j < s; j++)// ARROW
   {
      member[j-i-2] = dstName[j];// ARROW
   }
   member[j-i-2] = '\0';// ARROW

   if(j-i-2 == 0)// ARROW
   {
      ErrorMessage("Invalid structure member name '%s'",member);
      goto err;
   }

// Get structure
   short type;
   Variable *var;

   var = GetVariable(itfc,ALL_VAR,struc,type);

   if(!var || type != STRUCTURE)
   {
      ErrorMessage("'%s' is not a structure",struc);
      goto err;
   }

// Find the member
   Variable *strucData = var->GetStruct();

   Variable *newVar;
   for(var = strucData->next; var != NULL; var = var->next)
   {
      if(!strcmp(var->GetName(),member)) // Old member found
      {
         delete [] struc;
         delete [] member;
         if(CopyVariable(var,srcVar,FULL_COPY) == ERR) // Replace with new member
            return(ERR);
         return(OK);
      }
   }

// Member not found so add 
// new variable to the structure
   newVar = strucData->Add(STRUCTURE,member);
   delete [] struc;
   delete [] member;
   if(CopyVariable(newVar,srcVar,FULL_COPY) == ERR)
      return(ERR);
   return(OK);

err:

   delete [] struc;
   delete [] member;
   return(ERR);
}

// Extract the structure variable pointed to by varName. This will also search any substructures for the 
// variable.
Variable* GetStructureVariable(Interface *itfc, short scope, char *varName, Structure::errorMode errorMode)
{
   long s = strlen(varName);
 
	char* const varCopy = new char[s+1];
	char* member = varCopy;
	char* struc;
	const char* const selector = "->";
   CText err;
	
	struc = strcpy(varCopy, varName);
	member = strstr(varCopy, selector);
	
	// If "->" not found, or is a prefix
	if ((NULL == member) || (struc == member))
	{
      err.Format("Invalid structure name '%s'",struc);
      goto err;
   }

	// Null terminate the structure name and jump to the member name
	member[0] = '\0';
	member += strlen(selector);

	// If no member name is found
	if ('\0' == member[0]){
		err.Format("Invalid structure member name '%s'",member);
      goto err;
   }

// Get structure
   short type;
   Variable *var = NULL;

   var = GetVariable(itfc,scope,ALL_VAR,struc,type);

   if(!var || type != STRUCTURE)
   {
      err.Format("'%s' is not a structure",struc);
      goto err;
   }

// Find the member
   Variable *strucData = var->GetStruct();

   for(var = strucData->next; var != NULL; var = var->next)
   {
      if(!strcmp(var->GetName(),member)) // Old member found
      {
         delete [] varCopy;
         return(var);
      }
   }
   delete [] varCopy;
   return(NULL);

err:

	if(errorMode == Structure::REPORT)
      ErrorMessage(err.Str());
   delete [] varCopy;
   return(NULL);

}

Variable* JoinStructures(Variable* varA, Variable* varB)
{
   Variable *total;
   Variable *var,*memberVar;
   Variable *strucT,*strucA,*strucB;

   total = new Variable;
   total->MakeStruct();
   strucT = total->GetStruct();
   strucA = varA->GetStruct();
   strucB = varB->GetStruct();
   
   for(var = strucA->next; var != NULL; var = var->next)
   {
      memberVar = strucT->Add(STRUCTURE,var->GetName());
      if(CopyVariable(memberVar,var,FULL_COPY) == ERR)
         return(NULL);
   }

   for(var = strucB->next; var != NULL; var = var->next)
   {
      memberVar = strucT->Add(STRUCTURE,var->GetName());
      if(CopyVariable(memberVar,var,FULL_COPY) == ERR)
         return(NULL);
   }

   return(total);
}
