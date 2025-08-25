#include "stdafx.h"
#include "variablesOther.h"
#include "cArg.h"
#include "evaluate.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "macro_class.h"
#include "mymath.h"
#include "scanstrings.h"
#include "structure.h"
#include "memoryLeak.h"

/**********************************************************************
*          Routines to interact with the variable class               *
*                                                                     *
* GetVariable ............ Find and return a variable                 *
* IsAVariable ............ Check to see if a variable is defined      *
* AddVariable ............ Add a variable if it doesn't alreday exist *
* AddLocalVariable ....... Add a new variable to the local list       *
* AddGlobalVariable ...... Add a new variable to the global list      *
* RemoveVariableByName ... CLI function to remove variables           *
* DefineLocalVariables ... CLI function to add local variables        *
* DefineGlobalVariables .. CLI function to add global variables       *
* CopyVariable ........... Make a copy of a variable                  *
*                                                                     *
*                                           2/5/2002 CDE              *
**********************************************************************/


Variable globalVariable; // Externally accessible

bool allowWindowVars = true;
bool allowGlobalVars = true;

EXPORT Variable* GetAliasedVariable(Interface *itfc, char[], char*);

/*************************************************************
  See if the variable of name 'name' can be found in either  
  the local, window or global variable lists. (In that order) 
  If so return it to the calling program.                     
                                                              
  restrictions = ALL_VAR, NOT_ALIAS, NOT_ALIAS_SPECIFY_TYPE   
*************************************************************/

Variable* GetVariable(Interface *itfc, short scope, short restrictions, char name[], short &type)
{
   Variable *var = 0;

   if(scope == ALL)
   {
      var = GetVariable(itfc, ALL_VAR | DO_NOT_RESOLVE,name,type);
   }
   else if(scope == GLOBAL)
   {
      var = globalVariable.Get(ALL_VAR | DO_NOT_RESOLVE,name,type);

   }
   else if(scope == WINDOW)
   {
 //     if(GetGUIWin() && itfc->win) // Check window variables
      if(itfc->win) // Check window variables
      {
        // if(itfc->win == GetGUIWin())
         {
        //    var = GetGUIWin()->varList.Get(ALL_VAR | DO_NOT_RESOLVE,name,type);
            var = itfc->win->varList.Get(ALL_VAR | DO_NOT_RESOLVE,name,type);
         }
      }
   }
   else if(scope == LOCAL)
   {
      if(itfc && itfc->macro) // Check local variables
      {
         var = itfc->macro->varList.Get(ALL_VAR | DO_NOT_RESOLVE,name,type);        
      }
   }

   return(var);
}


EXPORT Variable* GetVariable(Interface *itfc, short restrictions, char name[], short &type)
{
   
   Variable *var = NULL;

	                
// Is it in the macro variable list? **************
   if(itfc && itfc->macro)
   {
      if((var = itfc->macro->varList.Get(restrictions,name,type)) != NULL)
      {
         var->SetScope(LOCAL);      
         return(var);
      }         
   }

// If no window variable allowed return
   if(!allowWindowVars)
      return(NULL);

// Is it in the window variable list? *************
 //  if(itfc && itfc->win && GetGUIWin())
   if(itfc && itfc->win)
   {
  // Only access windowvars procedure called via the GUI window
  //    if(itfc->win == GetGUIWin())
      if(itfc->win)
      {
     //    if((var = GetGUIWin()->varList.Get(restrictions,name,type)) != NULL)
         if((var = itfc->win->varList.Get(restrictions,name,type)) != NULL)
         {
            var->SetScope(WINDOW);
            return(var);
         }
      }
      else
      {
  //       TextMessage("invalid access to window variable\n");
      }
   }
   
// If no global allowed return
   if(!allowGlobalVars)
      return(NULL);

// Otherwise is it in the global variable list? ***
   if((var = globalVariable.Get(restrictions,name,type)) != NULL)
      var->SetScope(GLOBAL);
   
// Is it an alias? If so resolve	
   // Test this modification
 //  if(!(restrictions | DO_NOT_RESOLVE) && var && var->GetAlias())
   if(!(restrictions & DO_NOT_RESOLVE) && var && var->GetAlias())
   {
       var = GetAliasedVariable(itfc,var->GetAlias()->GetName(),var->GetData()); 
       if(!var)
       {
          ErrorMessage("Variable (for which %s is an alias)\n          is no longer present",name);
          return(NULL);
       }       
   }	
   
   return(var);
}


EXPORT Variable* GetAliasedVariable(Interface *itfc, char name[], char *data)
{   
   Variable *var = NULL;
       
// Is it in the macro variable list? **************
   if(itfc->macro)
   {
      if((var = itfc->macro->varList.Get(name,data)) != NULL)
      {
         var->SetScope(LOCAL);
         return(var);
      }
   }

// Is it in the window variable list? *************
 //  if(GetGUIWin() && itfc->win && (itfc->win == GetGUIWin()))
   if(itfc->win && (itfc->win == GetGUIWin()))
   {
      if((var = itfc->win->varList.Get(name,data)) != NULL)
  //    if((var = GetGUIWin()->varList.Get(name,data)) != NULL)
      {
         var->SetScope(WINDOW);
         return(var);
      }
   }
   
// Otherwise is it in the global variable list? ***   
   var = globalVariable.Get(name,data);
   if(var) var->SetScope(GLOBAL);
   return(var);
}

//EXPORT int IsNullVariable(char arg[])
//{
//   short nrArgs;
//   short type;
//   char name[50];
//   Variable *var = NULL;
//
//// Get variable name *******************************
//   if((nrArgs = ArgScan(itfc,arg,1,"variable name","c","s",name)) < 0)
//      return(nrArgs);
//
//   var = GetVariable(ALL_VAR,name,type);
//   
//   if(var && (var->GetType() == NULL_VARIABLE))
//      ansVar->MakeAndSetFloat(1.0);
//   else
//      ansVar->MakeAndSetFloat(0.0);
//   
//   return(OK);          
//}
   
/************************************************************
* See if the variable of name 'name' can be found in one of *
* the local, window or global variable lists. If so return  *
* 1 otherwise return 0.                                     *
************************************************************/

EXPORT int IsAVariable(Interface* itfc ,char arg[])
{
   short nrArgs;
   CText name;

// Get variable name *******************************
   if((nrArgs = ArgScan(itfc,arg,1,"variable name","e","t",&name)) < 0)
      return(nrArgs);
          
// Report find *************************************
   if(IsAVariableCore(itfc,name.Str()))
      itfc->retVar[1].MakeAndSetFloat(1.0);
   else
      itfc->retVar[1].MakeAndSetFloat(0.0);
   
   itfc->nrRetValues = 1;
   return(OK);
}

/************************************************************
* A variable may be hidden/visible readwrite/readonly       
************************************************************/

int SetVariableStatus(Interface *itfc, char args[])
{
   short nrArgs;
   short type;
   Variable *var;
   CText varName;
   CText visiblity = "visible";
   CText readstatus = "readwrite";
   CText lifetime = "transient";

// Prompt user
   if((nrArgs = ArgScan(itfc,args,1,"name, visibility status, read status, lifetime","eeee","tttt",&varName,&visiblity,&readstatus,&lifetime)) < 0)
	{
		itfc->nrRetValues = 0;
      return(nrArgs);
	}

// Get the variable
   var = globalVariable.Get(ALL_VAR,varName.Str(),type);
   if(!var)
   {
      ErrorMessage("variable '%s' is not defined",varName.Str());
      return(ERR);
   }

// Return to user 
   if(nrArgs == 1)
   {
   // Get visibility
      if(var->GetVisible())
         itfc->retVar[1].MakeAndSetString("visible");
      else
         itfc->retVar[1].MakeAndSetString("hidden");

   // Get read status
      if(var->GetReadOnly())
         itfc->retVar[2].MakeAndSetString("readonly");
      else
          itfc->retVar[2].MakeAndSetString("readwrite");

   // Get lifetime status
      if(var->GetPermanent())
         itfc->retVar[3].MakeAndSetString("permanent");
      else
          itfc->retVar[3].MakeAndSetString("transient");
      itfc->nrRetValues = 3;

      return(OK);
   }

// Set visibility
   if(visiblity == "hidden")
      var->SetVisible(false);
   else if(visiblity == "visible")
      var->SetVisible(true);
   else
   {
      ErrorMessage("invalid visibility status");
      return(ERR);
   }

// Set read status
   if(readstatus == "readonly")
      var->SetReadOnly(true);
   else if(readstatus == "readwrite")
      var->SetReadOnly(false);
   else
   {
      ErrorMessage("invalid read status");
      return(ERR);
   }

// Set lifetime status
   if(lifetime == "permanent")
      var->SetPermanent(true);
   else if(lifetime == "transient")
      var->SetPermanent(false);
   else
   {
      ErrorMessage("invalid lifetime status");
      return(ERR);
   }

	itfc->nrRetValues = 0;
   return(OK);
}


/************************************************************
   Return true if variable exists, false if not
	handle both simple and structure variables
************************************************************/

bool IsAVariableCore(Interface *itfc, char name[])
{
   Variable *var = NULL;
   short type;
   char *structName;
   char *memberName;
   bool isStruct = false;
	bool result = false;
	bool SplitStructStr(char *name, char *leftStr, char *rightStr);

// Check for a simple variable which is not null
	var = GetVariable(itfc,ALL_VAR,name,type);
	if(var && var->GetType() != NULL_VARIABLE)
		return(true);

// See if this is a structure member
	int sz = strlen(name);
   structName = new char[sz+1];
   memberName = new char[sz+1];

   if(SplitStructStr(name, structName, memberName))
   {
		if(!(var = GetVariable(itfc,ALL_VAR,structName,type)))
		{
			delete [] structName;
			delete [] memberName;
			return(result);
		}

	   if(var->GetType() == STRUCTURE)
      {
			char *leftStr = new char[sz+1];
         char *rightStr = new char[sz+1];
         var = var->GetStruct();
         if(var)
         {
				result = true;
				while(SplitStructStr(memberName, leftStr, rightStr))
				{
               var = var->Get(leftStr);
					result = false;

					if(var->GetType() == STRUCTURE)
					{
						var = var->GetStruct();
						if(var)
						{
							result = true;
							strcpy(memberName,rightStr);
						}
					}
					else
						break;
				}
			   if(!var->Get(memberName))
					result = false;
         }
			delete [] structName;
			delete [] memberName;
			delete [] leftStr;
			delete [] rightStr;
			return(result);
      }
	}

	delete [] structName;
	delete [] memberName;
   return(false);
}

/************************************************************
  Split a string (name) looking for the structure delimiter
************************************************************/

bool SplitStructStr(char *name, char *leftStr, char *rightStr)
{
	long start;

	if((start = FindSubStr(name,"->",0,0)) == -1)
		return(false);

	strncpy(leftStr,name,start);
   leftStr[start] ='\0';
   strcpy(rightStr,&(name[start+2]));
   if(strlen(leftStr) == 0 || strlen(rightStr) == 0)
      return(0);

	return(1);
}
//
///************************************************************
//         Return true if variable exists, false if not
//************************************************************/
//
//bool IsAVariableCore(Interface *itfc, char name[])
//{
//   Variable *var = NULL;
//   short type;
//   long start;
//   char *structName;
//   char *memberName;
//   bool isStruct = false;
//
//// See if this is a structure member
//   start = FindSubStr(name,"->",0,0);
//   if(start > 0)
//   {
//      int sz = strlen(name);
//      structName = new char[sz+1];
//      memberName = new char[sz+1];
//      strncpy(structName,name,start);
//      structName[start] ='\0';
//      strcpy(memberName,&(name[start+2]));
//      if(strlen(structName) == 0 || strlen(memberName) == 0)
//      {
//         delete [] structName;
//         delete [] memberName;
//         return(0);
//      }
//      isStruct = true;
//      strcpy(name,structName);
//   }
//
//// Is it in the macro variable list? ***************
//   if(itfc && itfc->macro)
//   {
//      var = itfc->macro->varList.Get(ALL_VAR,name,type);
//   }
//
//// Is it in the window variable list? **************
//   if(!var && itfc && itfc->win) // && (itfc->win == GetGUIWin()))
//   {
//      var = itfc->win->varList.Get(ALL_VAR,name,type);
//   }
//   
//// Otherwise is it in the global variable list? ****
//   if(!var)
//      var = globalVariable.Get(ALL_VAR,name,type);
//
//// Can't find variable
//   if(!var)
//      return(false);
//
//// Report find for normal variable *************************************
//   if(!isStruct && var->GetType() != NULL_VARIABLE)
//      return(true);
//
//// Check for structures
//   if(isStruct)
//   {
//      if(var->GetType() == STRUCTURE)
//      {
//         var = var->GetStruct();
//         if(var)
//         {
//            var = var->Get(memberName);
//            if(var && var->GetType() != NULL_VARIABLE)
//            {
//               delete [] structName;
//               delete [] memberName;
//               return(true);
//            }
//         }
//      }
//      delete [] structName;
//      delete [] memberName;
//      return(false);
//   }
//
//
//   
//   return(false);
//}

/*******************************************************************
*           Report the argument type as a string                   *
*******************************************************************/

EXPORT int ReturnVariableType(Interface* itfc ,char args[])
{
   short nArg,r;
   short type;
   Variable var,*pvar;
   CArg carg;

   Variable *ans = &itfc->retVar[1];

// Extract arguments - but do it explicitly to save time since 
// don't actually need the matrix info, just the dimensions
   nArg = carg.Count(args);

   if(nArg == 0)
   {
   // Prompt the user  
	   if((r = ArgScan(itfc,args,1,"variable","e","v",&var)) < 0)
	      return(r); 
   }
   if(nArg == 1)
   {
      char *arg1 = carg.Extract(1);
	   if(!(pvar = GetVariable(itfc,ALL_VAR,arg1,type)))
      {
	      if((r = ArgScan(itfc,args,1,"variable","e","v",&var)) < 0)
	         return(r); 
         pvar = &var;
      }
   }
   else
   {
      ErrorMessage("Only 1 argument expected");
      return(ERR);
   }

// Tell user of type  
   switch(VarType(pvar))
   {
      case(UNQUOTED_STRING):
         ans->MakeAndSetString("string");
         break;
         
      case(FLOAT32):
         ans->MakeAndSetString("float");
         break;
  
      case(FLOAT64):
         ans->MakeAndSetString("double");
         break;

      case(COMPLEX):
         ans->MakeAndSetString("complex");
         break;

      case(LIST):
         ans->MakeAndSetString("list");
         break;

      case(LIST2D):
         ans->MakeAndSetString("list2d");
         break;

      case(MATRIX2D):
         if(VarColSize(pvar) == 1 || VarRowSize(pvar) == 1)
            ans->MakeAndSetString("matrix1d");
         else
            ans->MakeAndSetString("matrix2d");
         break;

      case(DMATRIX2D):
         if(VarColSize(pvar) == 1 || VarRowSize(pvar) == 1)
            ans->MakeAndSetString("dmatrix1d");
         else
            ans->MakeAndSetString("dmatrix2d");
         break;

      case(CMATRIX2D):
         if(VarColSize(pvar) == 1 || VarRowSize(pvar) == 1)
            ans->MakeAndSetString("cmatrix1d");
         else
            ans->MakeAndSetString("cmatrix2d");
         break;

      case(MATRIX3D):
         ans->MakeAndSetString("matrix3d");
         break;

      case(CMATRIX3D):
         ans->MakeAndSetString("cmatrix3d");
         break;

      case(MATRIX4D):
         ans->MakeAndSetString("matrix4d");
         break;

      case(CMATRIX4D):
         ans->MakeAndSetString("cmatrix4d");
         break;

      case(STRUCTURE):
         ans->MakeAndSetString("structure");
         break;

      case(STRUCTURE_ARRAY):
         ans->MakeAndSetString("structure_array");
         break;

      case(CLASS):
      {
         ClassData *cData = (ClassData*)pvar->GetData();
         switch(cData->type)
         {
            case(WINDOW_CLASS):
               ans->MakeAndSetString("window");
               break;
            case(OBJECT_CLASS):
               ans->MakeAndSetString("object");
               break;
            case(PLOT_CLASS):
               ans->MakeAndSetString("plot");
               break;
            case(XLABEL_CLASS):
               ans->MakeAndSetString("xlabel");
               break;
            case(YLABEL_CLASS):
               ans->MakeAndSetString("ylabel");
               break;
            case(YLABEL_LEFT_CLASS):
               ans->MakeAndSetString("ylabelleft");
               break;
            case(YLABEL_RIGHT_CLASS):
               ans->MakeAndSetString("ylabelright");
               break;
            case(TITLE_CLASS):
               ans->MakeAndSetString("title");
               break;
            case(AXES_CLASS):
               ans->MakeAndSetString("axes");
               break;
            case(GRID_CLASS):
               ans->MakeAndSetString("grid");
               break;
            case(INSET_CLASS):
               ans->MakeAndSetString("inset");
               break;
         }
         break;
      }
      case(NULL_VARIABLE):
         ans->MakeAndSetString("null");
         break;
   }
   itfc->nrRetValues = 1;
   return(OK);
}
 
 
/*******************************************************************
*      Add a variable to global, local or window list              *
*******************************************************************/

EXPORT Variable* AddVariable(Interface *itfc, short scope, short type, char name[])
{
   Variable *var = NULL;
   
   switch(scope)
   {
// Make it global variable
      case(GLOBAL): 
	   {   
	      var = globalVariable.Get(ALL_VAR,name,type);     // Search for global of this name
	      if(!var)
         {
				if(itfc->win) // If not found see if it is a window variable
				{
					var = itfc->win->varList.Get(ALL_VAR,name,type);   // Search for it
               if (var && itfc->inCLI)
               {
                  TextMessage("Found Global assignment where a window variable exists - in CLI\n");
                  return(var);
               }
				}
	         var = globalVariable.Add(type,name);  // Not found so make a global of this name
            var->SetScope(GLOBAL);
         }
	      return(var);
	   }
// Make it a local variable if it already exists
// If it doesn't and its a window variable then use that
      case(LOCAL): 
      { 
	      if(itfc->macro) // Does a macro exist?
	      {
	         var = itfc->macro->varList.Get(ALL_VAR,name,type);   // Search for local variable of this name
	         if(!var) // Not found?
            {// Is it a window variable?
               if(itfc->win) // If so return window variable
               {
	               var = itfc->win->varList.Get(ALL_VAR,name,type);   // Search for it
	               if(var)
							return(var);                                    // Return if found
						//else // See if its a global - this can cause confusion if a global is inadvertantly added in the CLI
						//{
						//	 var = globalVariable.Get(ALL_VAR,name,type);   // Search for it
						//	 if(var)
						//	   return(var);                                 // Return if found
						//}
	            } 
             // Otherwise make local variable
               var = itfc->macro->varList.Add(type,name); 
               var->SetScope(LOCAL);
            }
	      }
	      else // Not in a macro so it must be global
		   {
		      var = globalVariable.Get(ALL_VAR,name,type);      // Search for it
		      if(!var) // Not found so make a global
            {
		         var = globalVariable.Add(type,name); 
               var->SetScope(GLOBAL);
            }
		   } 
		   break;
		}
		case(WINDOW): // Make it a window variable
      { // Only access windowvars procedure called via the GUI window
         if(itfc->win)
	      {
            var = itfc->win->varList.Get(ALL_VAR,name,type);   // Search for it
            if(!var)
            {
              var = itfc->win->varList.Add(type,name); // Not found so make it
              var->SetScope(WINDOW);
            }
	      }
	      else // Not in a window so its global
		   {
		      var = globalVariable.Get(ALL_VAR,name,type);     // Search for it
		      if(!var)
            {
		         var = globalVariable.Add(type,name);  // Not found so make it
               var->SetScope(GLOBAL);
            }
		   }      
      }
   }
   return(var);
}



 
/*******************************************************************
*      Add a variable to global, local or window list              *
*******************************************************************/

Variable* AddGlobalVariable(short type, char name[])
{
   Variable *var = NULL;
 
   var = globalVariable.Get(ALL_VAR,name,type);     // Search for it
   if(!var)
   {
      var = globalVariable.Add(type,name);  // Not found so make it
      var->SetScope(GLOBAL);
   }
   return(var);

}

/*******************************************************************
*      Delete all global variables                                 *
*******************************************************************/

void DeleteGlobalVariables()
{
   Variable *var = &globalVariable;
	Variable *next = var->next;
 
	while(next)
	{
		var = next;
		next = var->next;
		delete var;
	}
}

/************************************************************
* Add a new local variable if it doesn't already exist.     *
* Return the variable to the calling program.               *                                               *
************************************************************/

EXPORT Variable* AddLocalVariable(Interface *itfc, short type, char name[])
{
   Variable *var = NULL;

   if(itfc && itfc->macro)
   {
   // If is already exists don't make a new one
      if((var = itfc->macro->varList.Get(ALL_VAR,name,type)) != NULL)
         return(var);
   // Otherwise add new variable to local list
      var = itfc->macro->varList.Add(type,name);
      var->SetScope(LOCAL);
   }   
   return(var);
}

// Find a variable and return an alias to it
EXPORT int MakeVariableAlias(Interface* itfc ,char arg[])
{
   short nrArgs;
   CText name;
   CText method;
   short type;
   Variable *var,result;

   method = "noeval";

// Extract variable name and mode
   if((nrArgs = ArgScan(itfc,arg,1,"Variable, mode","ce","tt",&name,&method)) < 0)
      return(nrArgs);

// Make an alias of the variable but resolve it first if necessary
   if(method == "eval")
   {
      type = Evaluate(itfc, RESPECT_ALIAS, name.Str(), &result);
      
      if(type == UNQUOTED_STRING)
      {
      	var = GetVariable(itfc,ALL_VAR,result.GetString(),type);
      	if(!var)
	      {
	         ErrorMessage("variable '%s' not found",result.GetString());
	         return(ERR);
	      }            
      }
      else
      {
         ErrorMessage("variable '%s' should be a string variable",name);
         return(ERR);
      }
   }
   else
   {
		var = GetVariable(itfc,ALL_VAR,name.Str(),type);
		if(!var)
		{
		   ErrorMessage("variable %s not found",name);
		   return(ERR);
		}
   }
 
   itfc->retVar[1].MakeAndSetAlias(var);
   itfc->nrRetValues = 1;

   return(OK);
}


/*********************************************************
*  De-alias a variable by returning a copy of the original     
**********************************************************/

int DealiasVariable(Interface *itfc, char arg[])
{
	CText name;
	short nrArgs, type;

// Extract variable name to dealias
   if((nrArgs = ArgScan(itfc,arg,1,"Variable","c","t",&name)) < 0)
      return(nrArgs);

	Variable *var = GetVariable(itfc,ALL_VAR,name.Str(),type);
	if(!var)
	{
		ErrorMessage("variable %s not found",name.Str());
		return(ERR);
	}
	// Check if its an alias
	if(!var->GetAlias())
	{
		ErrorMessage("variable %s is not an alias",name.Str());
		return(ERR);
	}
   // Make a full copy of the variable
	Variable v;
	v.FullCopy(var->GetAlias());
	itfc->retVar[1].Assign(&v);
//	v.FreeName();
	v.SetNull();
	// This doesn't work because retVar may already have the same aliased data
	//itfc->retVar[1].FullCopy(var->GetAlias());
	//itfc->retVar[1].FreeName();

	itfc->nrRetValues = 1;
	return(OK);
}

/*********************************************************
*           Remove variables from a variable list      
**********************************************************/

//TODO check ansVar reference at end
int RemoveVariableByName(Interface* itfc ,char arg[])
{
   Variable *var;
   short nrArgs,n,type;
   CText sc;
   short scope = -1;
   int j = 2;
	CArg carg;

   nrArgs = carg.Count(arg);

// Get the scope (if any)
   if((n = ArgScan(itfc,arg,1,"[mode], variables","e","t",&sc)) < 0)
     return(n);

   if(sc == "global")
      scope = GLOBAL;
   else if(sc == "window")
      scope = WINDOW;
   else if(sc == "local")
      scope = LOCAL;
   else
      j = 1;
    
// Loop over variable list   
   for(short i = j; i <= nrArgs; i++)
   { 	
      if(ArgScan(itfc,carg.Extract(i),1,"","e","t",&sc) < 0)
      {
         ErrorMessage("argument '%s' is not a string",sc.Str());
         return(ERR);
      }

   // Find the variable (but don't resolve if alias) ***
      var = GetVariable(itfc,scope,ALL_VAR | DO_NOT_RESOLVE,sc.Str(),type);

   // Check to see if its a structure
      if(!var)
			var = GetStructureVariable(itfc,scope, sc.Str(),Structure::NO_REPORT);

      if(!var)
	   {
         ErrorMessage("Can't find variable '%s'",sc.Str());
         return(ERR);
	   }

	// Check to see if variable is permanent ************	
	   if(var->GetPermanent())
	   {
         ErrorMessage("Can't delete permanent variable '%s'",sc.Str());
         return(ERR);
	   }
	
	// Check to see if the variable is aliases **********	  
	   if(var->GetAliasCnt() != 0)
	   {	
         ErrorMessage("Can't delete aliased variable '%s'",sc.Str());
         return(ERR);
	   }

   // Make sure that ansVar doesn't point to deleted data
	   if(itfc->retVar[1].GetAlias() == var) 
         itfc->retVar[1].MakeAndSetFloat(0);
	   	
	// If not then remove it ****************************	
	   delete var;

   }
   itfc->nrRetValues = 0;
   return(OK);
}

// Remove all global variables
/*
int RemoveAllVariables(char args[])
{
   for(short i = 1; i <= nrArgs; i++)
   {
	   strcpy(varName,ExtractArg(arg,i));
	
	// Find the variable (but don't resolve if alias) ***
	   if(!(var = GetVariable(ALL_VAR | DO_NOT_RESOLVE,varName,type)))
	   {
	      if(itfc->inCLI)
	      {
	         ErrorMessage("can't find variable %s",varName);
	         return(ERR);
	      }
	      else
	         continue;
	   }
	
	// Check to see if variable is permanent ************	
	   if(var->permanent)
	   {
	      ErrorMessage("Can't delete permanent variable '%s'",varName);
	      return(ERR);
	   }
	
	// Check to see if the variable is aliases **********	  
	   if(var->aliasRef != 0)
	   {	   
	      ErrorMessage("Can't delete aliased variable '%s'",varName);
	      return(ERR);
	   }

// Make sure that ansVar doesn't point to deleted data
     if(ansVar->aliasVar == var) 
         ansVar->MakeAndSetFloat(0);
	   	
	// If not then remove it ****************************	
	   delete var;
   }
}
*/



/**********************************************************
* Specify a list of variable to have scope within the     *
* of a window.                                            *
**********************************************************/
//TODO convert to CText
EXPORT int DefineWindowVariables(Interface* itfc ,char arg[])
{
   short nrArgs;
   char varName[50];
	CArg carg;
   Variable result;
   
   if(!GetGUIWin())
   {
      ErrorMessage("no current window");
      return(ERR);
   }
     
   nrArgs = carg.Count(arg);
   
   for(int i = 1; i <= nrArgs; i++)
   {
      strcpy(varName,carg.Extract(i));
      if(varName[0] == '"' && varName[strlen(varName)-1] == '"')
      {
          Evaluate(itfc,RESPECT_ALIAS,varName,&result);
          if(result.GetType() != UNQUOTED_STRING)
          {
             ErrorMessage("argument should be a valid variable name");
             return(ERR);
          }
          AddVariable(itfc,WINDOW, NULL_VARIABLE, result.GetString());
      }
      else
          AddVariable(itfc,WINDOW, NULL_VARIABLE, varName);          
   }

   return(OK);
}

/*********************************************************
*         Swap the contents of two variables             *
*********************************************************/
//TODO Check input mode
int SwapVariables(Interface* itfc ,char args[])
{
   short nrArgs;
   Variable *var1,*var2;
   CText var1Name,var2Name;
   char typeStr[20];
   short type1,type2;
   long i,j,k;
   
// Extract variables from command line
   if((nrArgs = ArgScan(itfc,args,2,"var1, var2","cc","tt",&var1Name,&var2Name)) < 0)
      return(nrArgs);

// Get the variables
   var1 = GetVariable(itfc,ALL_VAR,var1Name.Str(),type1);
   var2 = GetVariable(itfc,ALL_VAR,var2Name.Str(),type2);

// Check they are the same type
   if(type1 != type2)
   {
      ErrorMessage("variables must be of the same type");
      return(ERR);
   }

// Swap them!  
   switch(type1)
   {
      case(FLOAT32):
      {
         float temp;
         temp = var1->GetReal();
         var1->MakeAndSetFloat(var2->GetReal());
         var2->MakeAndSetFloat(temp);
         break;
      }
      case(UNQUOTED_STRING):       
      case(QUOTED_STRING):
      {
         char temp[MAX_STR];
         strcpy(temp,var1->GetString());
         var1->MakeAndSetString(var2->GetString());
         var2->MakeAndSetString(temp);
         break;
      }      
      case(COMPLEX):
      {
         complex temp;
         temp = var1->GetComplex();
         var1->MakeAndSetComplex(var2->GetComplex());
         var2->MakeAndSetComplex(temp);
         break;
      }
      case(LIST):
      {
         char list1Str[MAX_STR];
         char list2Str[MAX_STR];
         if(!MatricesSameSize(var1,var2))
         {
            ErrorMessage("lists must be same size");
            return(ERR);
         }  
         long xdim = var1->GetDimX();
                
         char **list1 = var1->GetList();
         char **list2 = var2->GetList();
         
         for(i = 0; i < xdim; i++)
         {
            strncpy_s(list1Str,MAX_STR,list1[i],_TRUNCATE);
            strncpy_s(list2Str,MAX_STR,list2[i],_TRUNCATE);
            var1->ReplaceListItem(list2Str,i);                         
            var2->ReplaceListItem(list1Str,i);                         
         }         
         break;
      }      
      case(MATRIX2D):
      {
         float temp;
         if(!MatricesSameSize(var1,var2))
         {
            ErrorMessage("matrices must be same size");
            return(ERR);
         }
         long xdim = var1->GetDimX();
         long ydim = var1->GetDimY();
         float **m1 = var1->GetMatrix2D();
         float **m2 = var2->GetMatrix2D();
         

         for(j = 0; j < ydim; j++)
         {
            for(i = 0; i < xdim; i++)
            {
               temp = m1[j][i];
               m1[j][i] = m2[j][i];
               m2[j][i] = temp;               
            }
         }
         
         break;
      } 
      case(CMATRIX2D):
      {
         complex temp;
         if(!MatricesSameSize(var1,var2))
         {
            ErrorMessage("matrices must be same size");
            return(ERR);
         }
         long xdim = var1->GetDimX();
         long ydim = var1->GetDimY();
         complex **m1 = var1->GetCMatrix2D();
         complex **m2 = var2->GetCMatrix2D();
         

         for(j = 0; j < ydim; j++)
         {
            for(i = 0; i < xdim; i++)
            {
               temp = m1[j][i];
               m1[j][i] = m2[j][i];
               m2[j][i] = temp;               
            }
         }
         
         break;
      }  
      case(MATRIX3D):
      {
         float temp;
         if(!MatricesSameSize(var1,var2))
         {
            ErrorMessage("matrices must be same size");
            return(ERR);
         }
         long xdim = var1->GetDimX();
         long ydim = var1->GetDimY();
         long zdim = var1->GetDimZ();
         float ***m1 = var1->GetMatrix3D();
         float ***m2 = var2->GetMatrix3D();
         
         for(k = 0; k < zdim; k++)
         {
	         for(j = 0; j < ydim; j++)
	         {
	            for(i = 0; i < xdim; i++)
	            {
	               temp = m1[k][j][i];
	               m1[k][j][i] = m2[k][j][i];
	               m2[k][j][i] = temp;               
	            }
	         }
         }
         
         break;
      } 
      case(CMATRIX3D):
      {
         complex temp;
         if(!MatricesSameSize(var1,var2))
         {
            ErrorMessage("matrices must be same size");
            return(ERR);
         }
         long xdim = var1->GetDimX();
         long ydim = var1->GetDimY();
         long zdim = var1->GetDimZ();
         complex ***m1 = var1->GetCMatrix3D();
         complex ***m2 = var2->GetCMatrix3D();
         
         for(k = 0; k < zdim; k++)
         {
	         for(j = 0; j < ydim; j++)
	         {
	            for(i = 0; i < xdim; i++)
	            {
	               temp = m1[k][j][i];
	               m1[k][j][i] = m2[k][j][i];
	               m2[k][j][i] = temp;               
	            }
	         }
         }
         break;
      }                       
                           
      default:
      {
         GetVariableTypeAsString(type1,typeStr);
         ErrorMessage("swap not supported for %s",typeStr);
      }
   }
   return(OK);
}

/*********************************************************
*  Return the name of a variable type as a string        *
*********************************************************/

void GetVariableTypeAsString(short type, char *typeStr)
{
   switch(type)
   {
      case(FLOAT32):           strcpy(typeStr,"float");             break;
      case(COMPLEX):         strcpy(typeStr,"complex");           break;
      case(MATRIX2D):        strcpy(typeStr,"matrix");            break;
      case(MATRIX3D):        strcpy(typeStr,"3D matrix");         break;
      case(CMATRIX3D):       strcpy(typeStr,"3D complex matrix"); break;
      case(MATRIX4D):        strcpy(typeStr,"4D matrix");         break;
      case(CMATRIX4D):       strcpy(typeStr,"4D complex matrix"); break;
      case(CMATRIX2D):       strcpy(typeStr,"cmatrix");           break;
      case(UNQUOTED_STRING): strcpy(typeStr,"string");            break;
      case(LIST):            strcpy(typeStr,"list");              break;
      case(LIST2D):          strcpy(typeStr,"list2d");            break;
      case(CLASS):           strcpy(typeStr,"class");             break;
      case(CLASSITEM):       strcpy(typeStr,"class function");    break;
      case(NULL_VARIABLE):   strcpy(typeStr,"null variable");     break;
      default:               strcpy(typeStr,"unknown type");      break;
   }
}

/*********************************************************
*  Assign the contents of variable "srcStr" to the       *
*  variable "dstStr" Do not copy the data just move it   *
*  using names to specify variables.                     * 
*********************************************************/

//EXPORT short AssignVariable(short scope, char *dstStr, char *srcStr)
//{
//   Variable *srcVar,*dstVar;
//   short srcType;
//
//// Find variable ************
//   
//   srcVar = GetVariable(ALL_VAR,srcStr,srcType);
//   dstVar = AddVariable(scope, srcType, dstStr);
//   
//// Do they exist? ***********
//
//   if(!dstVar || !srcVar)
//   {
//      ErrorMessage("invalid variable copy");
//      return(ERR);
//   }
//
//// Do the assignment **************
//
//   dstVar->Assign(srcVar);
//   
// // Remove data reference in srcVar
// 
//   srcVar->FreeData();
//   srcVar->SetType(NULL_VARIABLE);
//	   
//   return(OK);  
//}
//


/*********************************************************
*  Copy source variable to destination variable          *
*  using names to specify variables.                     * 
*
*  mode = FULL_COPY : copy variables completely.         *
*  mode = RESPECT_ALIAS: copy alias info too.               *
*  mode = MAKE_ALIAS : make destination and alias        *
*********************************************************/

//EXPORT short CopyVariable(short scope, char *dstStr, char *srcStr, short mode)
//{
//   Variable *srcVar,*dstVar;
//   short srcType;
//
//// Find variable ************
//   
//   srcVar = GetVariable(ALL_VAR,srcStr,srcType);
//   dstVar = AddVariable(scope, srcType, dstStr);
//   
//// Do they exist? ***********
//
//   if(!dstVar || !srcVar)
//   {
//      ErrorMessage("invalid variable copy");
//      return(ERR);
//   }
//
//// Do the copy **************
//
//   if(mode == FULL_COPY) 
//   {
//      if(dstVar->FullCopy(srcVar) == ERR)
//         return(ERR);
//   }
//	else  
//   {
//	   if(dstVar->CopyWithAlias(srcVar) == ERR)
//         return(ERR);
//   }
//	   
//   return(OK);  
//}					   

/*********************************************************
*  Copy source variable to destination variable          *
*  using variable pointers to specify variables.         *
* 
*  mode = FULL_COPY : copy variables completely.         *
*  mode = RESPECT_ALIAS: copy alias info too.            *
*  mode = MAKE_ALIAS : make destination an alias         *
*********************************************************/

//EXPORT
short CopyVariable(Variable *dstVar, Variable *srcVar, short mode)
{  

// Do they exist? ***********

   if(!dstVar || !srcVar)
   {
      ErrorMessage("invalid variable copy");
      return(ERR);
   }

// Do a full copy - even if source is an alias **************

   if(mode == FULL_COPY)
   {   
      if(dstVar->FullCopy(srcVar) == ERR)
         return(ERR);
	}
	
// Copy, and if source is an alias so to will destination ***********

	else
	{  
	   if(dstVar->CopyWithAlias(srcVar) == ERR)
         return(ERR);
	}
	return(OK);
}

// Specify the number of returned values from a routine so
// print command knows how many to display

int AllowNonLocalVariables(Interface* itfc ,char args[])
{
   short nrArgs;
   CText type1,type2;
   
// Extract variables from command line
   
   if((nrArgs = ArgScan(itfc,args,0,"type1, type2","ee","tt",&type1,&type2)) < 0)
      return(nrArgs);

   allowWindowVars = false;
   allowGlobalVars = false;
 
   if(nrArgs == 1)
   {
      if(type1 == "window")
          allowWindowVars = true;
      else if(type1 == "global")
          allowGlobalVars = true;
   }

   if(nrArgs == 2)
   {
      if(type1 == "window" && type2 == "global")
      {
          allowWindowVars = true;
          allowGlobalVars = true;
      }

      else if(type2 == "window" && type1 == "global")
      {
          allowWindowVars = true;
          allowGlobalVars = true;
      }

   }

   return(OK);
}

/****************************************************************
 Generates the next object validation code
****************************************************************/

long GetNextObjectValidationCode()
{
   static long code = 11111; // start at some large number

   code++;

   if(code == 0) // zero is the invalid code
      code = 11111; // Start again

   return(code);
}

/****************************************************************
 Checks to see if the object code matches the class data code.
 If not then the object has been deleted.
****************************************************************/

short CheckClassValidity(ClassData* cd, bool displayError)
{
   long* code = (long*)cd->data;

   if(cd->code == *code)
      return(OK);
   
	if(displayError)
      ErrorMessage("Referenced object has been deleted!\n");
   return(ERR);
}