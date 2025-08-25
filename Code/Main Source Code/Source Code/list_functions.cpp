#include "stdafx.h"
#include "cArg.h"
#include "allocate.h"
#include "control.h"
#include "evaluate.h"
#include "globals.h"
#include "interface.h"
#include "list_functions.h"
#include "macro_class.h"
#include "mymath.h"
#include "process.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "ctext_utilities.h"
#include <algorithm>
#include <string>
#include <vector>
#include "memoryLeak.h"

using std::random_shuffle;
using std::string;
using std::vector;

int Convert2DListTo1D(Interface* itfc ,char arg[]);
int Convert1DListTo2D(Interface* itfc ,char arg[]);
int SortListBySubstring(char** list, long size, long start, long end, int *indicies);
void SortListInReverse(char** list, long size, short mode, int *indicies);
short CompareHexStrings(char *str1, char* str2);
int FixListFaults(Interface *itfc, char args[]);


/******************************************************************************
*                       Functions for working with Lists                       
*      
* CLI commands
*  
* AssignParameterList ........ Assign each element of parameter list to a real variable
* GetParameterListValue ...... Get the value of one parameter in a list
* NewList .................... Generate a list with a specified number of elements. 
* MergeParameterLists ........ Merges two parameter lists not duplicating names
* RemoveStringFromList ....... Remove a entry from a list.
* RemovePrefix ............... Remove a prefix from a list variable
* RenameListEntry ............ Rename a parameter list entry key.
* RemoveListEntry ............ Remove list entry at specified position.
* SetParameterListValue ...... Set the value of one parameter in a list
* SplitParameterString ....... Returns the parameter name and value
* 
*
* Lower level function calls
*
* AppendStringToList ......... Add a string to the end of a list.
* CompareLists ............... Compare two lists.
* FreeList ................... Free the memory used by a list.
* InsertStringIntoList ....... Insert a new string into a list.
* JoinLists .................. Join two lists together.
* MakeListFromText ........... Convert a comma delimited string into a list.
* MakeListFromEvaluatedText .. Convert a string into a list after evaluating
*                              each comma delimited expression in the string.
* ReplaceStringInList ........ Replace one list entry with another.
* 
*
* Last modified 5/8/2020 by Craig Eccles - © Magritek Ltd/GmbH
******************************************************************************/

/******************************************************************************
*                    Check for and remove list faults
*
* Syntax: a = fixlist(input, [mode])
*
* Default mode (quotes) searches for embedded quotes and escapes them if
* they are not already escaped.
*
******************************************************************************/

int FixListFaults(Interface* itfc ,char arg[])
{ 
   short r;  // Function return
   Variable input;
	CText mode = "quotes";

// Get number of entries to add to list      
   if((r = ArgScan(itfc,arg,1,"list to fix [, mode]","ee","vt",&input,&mode)) < 0)
      return(r); 

	if(mode == "quotes")
	{
		if(input.GetType() == LIST)
		{
			int lstSize = input.GetDimX();
			char **lstIn = input.GetList();
			char **lstOut = MakeEmptyList(lstSize);
			CText replacement;

			for(int i = 0; i < lstSize; i++)
			{
				char *entry = lstIn[i];
				int sz = strlen(entry)+1;
				char* srcName = new char[sz];
				char* srcValue = new char[sz];
				replacement = "";
				if(ParseAssignmentString(entry,srcName,srcValue) == ERR)
				{ // Fix value in simple list
					int len = strlen(entry);
					if(srcValue[0] == '"' && srcValue[len-1] == '"')
					{
						for(int j = 0; j < len; j++)
						{
							if(j > 0 && j < len-1 && entry[j] == '"')
								replacement.Append('\\');
							replacement.Append(entry[j]);
						}
						ReplaceStringInList(replacement.Str(), &lstOut, lstSize, i);
					}
					else
					{
						CText outStr;
						outStr = srcName;
						outStr = outStr + " = ";
						outStr = outStr + srcValue;
						ReplaceStringInList(outStr.Str(), &lstOut, lstSize, i);
					}
				}
				else // Fix value in assignment list
				{
					int len = strlen(srcValue);
					if(srcValue[0] == '"' && srcValue[len-1] == '"')
					{
						for(int j = 0; j < len; j++)
						{
							if(j > 0 && j < len-1 && srcValue[j] == '"')
								replacement.Append('\\');
							replacement.Append(srcValue[j]);
						}
						CText outStr;
						outStr = srcName;
						outStr = outStr + " = ";
						outStr = outStr + replacement;
						ReplaceStringInList(outStr.Str(), &lstOut, lstSize, i);
					}
					else
					{
						CText outStr;
						outStr = srcName;
						outStr = outStr + " = ";
						outStr = outStr + srcValue;
						ReplaceStringInList(outStr.Str(), &lstOut, lstSize, i);
					}
				}
				delete [] srcName;
				delete [] srcValue;
			}
			itfc->retVar[1].AssignList(lstOut, lstSize);
			itfc->nrRetValues = 1;
		}
	}

	return(0);

}

               
/******************************************************************************
*                    Make a string list and store in ansVar
*
* Syntax: a = list(N) ... makes a list with N entries i.e. 
*                            ["item 0", "item 1" ... "item N-1"]
*
* Syntax: a = list(structure) ... makes a list from the contents of structure
*
* Syntax: a = list(w,h) ... makes a 2D list with w*h entries i.e. 
*                         ["item 00", "item 01" ... "item 0(w-1)";
*                          "item 10", "item 11" ... "item 1(w-1)";
*                          ...
*                          "item (h-1)0", "item (h-1)1" ... "item (h-1)(w-1)";
*
******************************************************************************/

int NewList(Interface* itfc ,char arg[])
{ 
   short r;               // Function return
   Variable arg1, arg2;
   long width = 1; // List width
   long height = 1; // List height

// Get number of entries to add to list      
   if((r = ArgScan(itfc,arg,1,"entries","ee","vl",&arg1,&height)) < 0)
      return(r); 

   if (r == 1)
   {
      if (arg1.GetType() == FLOAT32)
      {
         width = nint(arg1.GetReal());

         char **list;           // Point to array of strings

      //Check for valid list size
         if (width < 0)
         {
            ErrorMessage("invalid list size");
            return(ERR);
         }
         if (width == 0)
            itfc->retVar[1].MakeAndSetList(NULL, 0);
         else
         {
            if(width >= 32767)
            {
               ErrorMessage("can't allocate memory for list");
               return(ERR);
            }

            // Make new list                              
            list = MakeList(width);
            if (!list)
            {
               ErrorMessage("can't allocate memory for list");
               return(ERR);
            }
            // Return list to the user      
            itfc->retVar[1].MakeAndSetList(list, width);
            FreeList(list, width);
         }
         itfc->nrRetValues = 1;

         return(OK);
      }
      else if (arg1.GetType() == STRUCTURE)
      {
         Variable *struc, *svar;

         struc = arg1.GetStruct();
         svar = struc->next;
         // Count entries in structure
         width = 0;
         while (svar != NULL)
         {
            width++;
            svar = svar->next;
         }
         // Make new list                              
         char **list = NULL;
         svar = struc->next;
         for(int i = 0; i < width; i++)
         {
            CText value;
            bool quote;
            ConvertVariableToText(svar, value, "", false);
            (svar->GetType() == UNQUOTED_STRING) ? (quote = true) : (quote = false);
            CText temp = svar->GetName();
            temp.Concat(" = ");
            if (quote)
            {
               temp.Concat("\"");
               temp = temp + value;
               temp.Concat("\"");
            }
            else
            {
               temp = temp + value;
            }

            AppendStringToList(temp.Str(), &list, i);
            svar = svar->next;
         }
         // Return list to the user      
         itfc->retVar[1].AssignList(list, width);
         return(OK);
      }
      else
      {
         ErrorMessage("invalid type for argument 1");
         return(ERR);
      }
	}
	else
	{
      if (arg1.GetType() == FLOAT32)
      {
         width = nint(arg1.GetReal());

         //Check for valid list size
         if (width < 0 || height < 0)
         {
            ErrorMessage("invalid 2D list size");
            return(ERR);
         }
         itfc->retVar[1].MakeAndSet2DList(width, height);
         itfc->nrRetValues = 1;
      }
      else
      {
         ErrorMessage("invalid type for argument 1");
         return(ERR);
      }

		return(OK);
	}
}

/******************************************************************************
*  Return a list of the names in a parameter list
* 
* Syntax: lstOut = getlistnames(lstIn) ... find all the names in lstIn and
*                                          return in a new list lstOut
******************************************************************************/


int GetListNames(Interface* itfc ,char args[])
{
	short nrArgs;
	Variable listVar;
	CText cmd;
	char *srcName;
	char *srcValue;
	char *varName;

// Get the source list *******************
	if((nrArgs = ArgScan(itfc,args,1,"list","e","v",&listVar)) < 0)
		return(nrArgs); 

// Check that 'listVar' is a list ********************
	if(listVar.GetType() != LIST)
	{
		ErrorMessage("argument 1 should be a list");
		return(ERR);
	}

// Get list handle **********************************
   char **srcList = listVar.GetList();

// Get the size of lists *****************************
   long srcEntries = (long)listVar.GetDimX();

// Make the output list *****************************
   char **newList = 0;
   long totalEntries = 0;
	int cnt = 0;

// Scan through list looking for names ************
   for(int i = 0; i < srcEntries; i++)
   {
      cmd = srcList[i];
		srcName = new char[cmd.Size()+1];
		srcValue = new char[cmd.Size()+1];
		varName = new char[cmd.Size()+1];
	   ParseAssignmentString(cmd.Str(),srcName,srcValue);  
		if(srcName[0] == '(')
		{
			int k = 0;
			for(int j = 1; j < strlen(srcName); j++)
			{
				if(srcName[j] == ',')
				{
					varName[k] = '\0';
			      AppendStringToList(varName, &newList, cnt++);
					k = 0;
					continue;
				}
				else if(srcName[j] == ')')
				{
					varName[k] = '\0';
			      AppendStringToList(varName, &newList, cnt++);
					break;
				}
				varName[k++] = srcName[j];
			}
		}
		else
			AppendStringToList(srcName, &newList, cnt++);

	// Free up memory
		if(srcName) delete [] srcName;
		if(srcValue) delete [] srcValue;
		if(varName) delete [] varName;
   }

// Copy the result to ansVar	************************
   itfc->retVar[1].AssignList(newList,cnt);
   itfc->nrRetValues = 1;

   return(OK);
}

/******************************************************************************
*  Insert a new string into an existing list and return the result in ansVar
* 
* Syntax: a = insertinlist(b,"string",p) ... insert string to list 'b' at position 'p'
*                                            returning the result in new list 'a'
******************************************************************************/

int InsertStringIntoListCLI(Interface* itfc ,char arg[])
{ 
   short r;               // Function return flag
   Variable var;          // Variable containing copy of list
   CText string;  // New string
   static long position;  // Position of new string

// Get list name, string to insert and position to insert it in   
   if((r = ArgScan(itfc,arg,1,"list, string, position","eee","vtl",&var,&string,&position)) < 0)
      return(r); 

// Check for errors 
   if(VarType(&var) != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }

// Insert string into list, returning modified list
   r = InsertStringIntoList(string.Str(), (char***)var.GetDataAdrs(), VarWidth(&var), position);

// Check for errors
   if(r == -1)
   {
      ErrorMessage("invalid insertion position");
      return(ERR);
   }
   if(r == -2)
   {
      ErrorMessage("couldn't allocate memory for modified list");
      return(ERR);
   }
// Return modified list
   var.SetDim1(var.GetDimX()+1);
//   VarWidth(&var)++;
   itfc->retVar[1].MakeAndSetList(var.GetList(),var.GetDimX());
   return(OK);
}

/******************************************************************************
*  Sort a list alphanumerically by list names or part of list names
* 
* Syntax: result = sort(list,[mode]) or
*         result = sort(list, start, end)
*
* mode = forward (alphabetic sort)
*        reverse (alphabetic sort)
*        forward_numeric (numerical sort or alphabetical sort DEFAULT)
*        reverse_numeric (numerical sort or alphabetical sort)
*
* The numeric options check to see if there is a leading number and if
* there is then the sort numerically on that.
*
******************************************************************************/


int SortList(Interface* itfc ,char arg[])
{ 
   short r;               // Function return flag
   Variable var;          // Variable containing copy of list
   long first=-1,last=-1;      // Range of indicies to compare
	Variable varFirst, varLast;
	CText direction = "forward_numeric";
	float *reorderArray = 0;
	char **outList = 0;
	int reorderArraySize = 0;

// Get list name, and sublist range to sort by   
   if((r = ArgScan(itfc,arg,1,"list, (first,last)/mode","eee","vvv",&var,&varFirst,&varLast)) < 0)
      return(r); 

// Check for errors 
   if(VarType(&var) != LIST)
   {
      ErrorMessage("argument is not a list");
      return(ERR);
   }

   char **list = var.GetList();
   long size = var.GetDimX();

	if(r == 3)
	{
		if(varFirst.GetType() == FLOAT32 && varLast.GetType() == FLOAT32)
		{
			first = nint(varFirst.GetReal());
			last = nint(varLast.GetReal());
		}
	}

	if(r == 2)
	{
		if(varFirst.GetType() == UNQUOTED_STRING)
		{
			direction = varFirst.GetString();
			if(direction != "forward_numeric" && direction != "reverse_numeric" && direction != "forward" && direction != "reverse" &&
				direction != "forward_embedded_numeric" && direction != "reverse_embedded_numeric")
			{
				ErrorMessage("second argument should be forward/reverse/forward_numeric/reverse_numeric/forward_embedded_numeric/reverse_embedded_numeric");
				return(ERR);
			}
		}
		else if(varFirst.GetType() == MATRIX2D && varFirst.GetDimY() == 1)
		{
			reorderArray = varFirst.GetMatrix2D()[0];
			reorderArraySize = varFirst.GetDimX();
			if(reorderArraySize != var.GetDimX())
			{
				ErrorMessage("Reorder array size must match size of input list");
				return(ERR);
			}
		}
		else
		{
			ErrorMessage("invalid second argument");
			return(ERR);
		}
	}


	if(first >= size || last < first)
   {
      ErrorMessage("first must be <= last and smaller than list size");
      return(ERR);
   }
	int* indicies;

	if(first < 0)
	{
		if(reorderArray) // User has supplied a reorder array
		{
			int index;
			for(int i = 0; i < reorderArraySize; i++)
			{
				// Check for invalid sort array indicies
				index = reorderArray[i];
				if(index <  0 || index >= reorderArraySize)
				{
					ErrorMessage("reorder array indices my be in size range of input list");
					return(ERR);
				}
			}

		// Make new list which will contain sorted version                              
			char **outList = new char*[reorderArraySize];
			char **inList = var.GetList();
			for(int i = 0; i < reorderArraySize; i++)
			{
				index = reorderArray[i];
				char *str = inList[index];
			   outList[i] = new char[strlen(str)+1];
				strcpy(outList[i],str);
			}
			itfc->retVar[1].AssignList(outList,reorderArraySize);
			itfc->nrRetValues = 1;
			return(OK);
		}
		else
		{
			indicies = new int[size];
			for(int i = 0; i < size; i++)
				indicies[i] = i;
			if(direction == "forward")
				SortList(list,size,1,indicies);
			else if(direction== "forward_numeric")
				SortList(list,size,0,indicies);
			else if(direction== "forward_embedded_numeric")
				SortList(list,size,2,indicies);
			else if(direction== "reverse")
				SortListInReverse(list,size,1,indicies);
			else if(direction== "reverse_embedded_numeric")
				SortListInReverse(list,size,2,indicies);
			else
				SortListInReverse(list,size,0,indicies);
		}
	}
	else
	{
		indicies = new int[size];
		for(int i = 0; i < size; i++)
			indicies[i] = i;
		if(SortListBySubstring(list,size,first,last,indicies))
		{
			ErrorMessage("invalid first and.or last parameters");
			return(ERR);
		}
	}

   itfc->retVar[1].MakeAndSetList(list, size);
   itfc->nrRetValues = 1;
   delete [] indicies;

   return(OK);
}


/******************************************************************************
*  Sort a list alphanumerically by list names or part of list names
*
* Syntax: result = sort(list,[mode]) or
*         result = sort(list, start, end)
*
* mode = forward (alphabetic sort)
*        reverse (alphabetic sort)
*        forward_numeric (numerical sort or alphabetical sort DEFAULT)
*        reverse_numeric (numerical sort or alphabetical sort)
*
* The numeric options check to see if there is a leading number and if
* there is then the sort numerically on that.
*
******************************************************************************/


int SortList2(Interface* itfc, char arg[])
{
   short r;               // Function return flag
   Variable var;          // Variable containing copy of list
   long first = -1, last = -1;      // Range of indicies to compare
   Variable varFirst, varLast;
   CText direction = "forward_numeric";
   float* reorderArray = 0;
   char** outList = 0;
   int reorderArraySize = 0;

   // Get list name, and sublist range to sort by   
   if ((r = ArgScan(itfc, arg, 1, "list, (first,last)/mode", "eee", "vvv", &var, &varFirst, &varLast)) < 0)
      return(r);

   // Check for errors 
   if (VarType(&var) != LIST)
   {
      ErrorMessage("argument is not a list");
      return(ERR);
   }

   char** list = var.GetList();
   long size = var.GetDimX();

   if (r == 3)
   {
      if (varFirst.GetType() == FLOAT32 && varLast.GetType() == FLOAT32)
      {
         first = nint(varFirst.GetReal());
         last = nint(varLast.GetReal());
      }
   }

   if (r == 2)
   {
      if (varFirst.GetType() == UNQUOTED_STRING)
      {
         direction = varFirst.GetString();
         if (direction != "forward_numeric" && direction != "reverse_numeric" && direction != "forward" && direction != "reverse" &&
            direction != "forward_embedded_numeric" && direction != "reverse_embedded_numeric")
         {
            ErrorMessage("second argument should be forward/reverse/forward_numeric/reverse_numeric/forward_embedded_numeric/reverse_embedded_numeric");
            return(ERR);
         }
      }
      else if (varFirst.GetType() == MATRIX2D && varFirst.GetDimY() == 1)
      {
         reorderArray = varFirst.GetMatrix2D()[0];
         reorderArraySize = varFirst.GetDimX();
         if (reorderArraySize != var.GetDimX())
         {
            ErrorMessage("Reorder array size must match size of input list");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid second argument");
         return(ERR);
      }
   }


   if (first >= size || last < first)
   {
      ErrorMessage("first must be <= last and smaller than list size");
      return(ERR);
   }
   int* indicies;

   if (first < 0)
   {
      if (reorderArray) // User has supplied a reorder array
      {
         int index;
         for (int i = 0; i < reorderArraySize; i++)
         {
            // Check for invalid sort array indicies
            index = reorderArray[i];
            if (index < 0 || index >= reorderArraySize)
            {
               ErrorMessage("reorder array indices my be in size range of input list");
               return(ERR);
            }
         }

         // Make new list which will contain sorted version                              
         char** outList = new char* [reorderArraySize];
         char** inList = var.GetList();
         for (int i = 0; i < reorderArraySize; i++)
         {
            index = reorderArray[i];
            char* str = inList[index];
            outList[i] = new char[strlen(str) + 1];
            strcpy(outList[i], str);
         }
         itfc->retVar[1].AssignList(outList, reorderArraySize);
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         indicies = new int[size];
         for (int i = 0; i < size; i++)
            indicies[i] = i;
         if (direction == "forward")
            SortList(list, size, 1, indicies);
         else if (direction == "forward_numeric")
            SortList(list, size, 0, indicies);
         else if (direction == "forward_embedded_numeric")
            SortList(list, size, 2, indicies);
         else if (direction == "reverse")
            SortListInReverse(list, size, 1, indicies);
         else if (direction == "reverse_embedded_numeric")
            SortListInReverse(list, size, 2, indicies);
         else
            SortListInReverse(list, size, 0, indicies);
      }
   }
   else
   {
      indicies = new int[size];
      for (int i = 0; i < size; i++)
         indicies[i] = i;
      if (SortListBySubstring(list, size, first, last, indicies))
      {
         ErrorMessage("invalid first and.or last parameters");
         return(ERR);
      }
   }
   float* findicies = new float[size];
   for (int i = 0; i < size; i++)
      findicies[i] = indicies[i];
   itfc->retVar[1].MakeAndSetList(list, size);
   itfc->retVar[2].MakeMatrix2DFromVector(findicies, size, 1);
   itfc->nrRetValues = 2;
   delete[] indicies;
   delete[] findicies;

   return(OK);
}

void SortList(char** list, long size)
{
   long i,j;
   CText temp;
   // Do a simple insertion sort
   for(j = 1; j < size; j++)
   {
      temp = list[j];
      i = j-1;
      while(i >= 0 && CompareStrings(temp.Str(),list[i]) == -1)
      {
         ReplaceStringInList(list[i], &list, size, i+1);
         i--;
      }
      ReplaceStringInList(temp.Str(), &list, size, i+1);
   }
}

void SortList(char** list, long size, short mode, int *indicies)
{
   long i,j,tempIndex;
   CText temp;
   // Do a simple insertion sort
   for(j = 1; j < size; j++)
   {
      temp = list[j];
		tempIndex = indicies[j];
      i = j-1;
      while(i >= 0 && CompareStrings(temp.Str(),list[i],mode) == -1)
      {
         ReplaceStringInList(list[i], &list, size, i+1);
			indicies[i+1] = indicies[i];
         i--;
      }
      ReplaceStringInList(temp.Str(), &list, size, i+1);
		indicies[i+1] = tempIndex;
   }
}

void SortListInReverse(char** list, long size, short mode, int *indicies)
{
   long i,j,tempIndex;
   CText temp;
   // Do a simple insertion sort
   for(j = 1; j < size; j++)
   {
      temp = list[j];
		tempIndex = indicies[j];
      i = j-1;
      while(i >= 0 && CompareStrings(temp.Str(),list[i],mode) == 1)
      {
			ReplaceStringInList(list[i], &list, size, i+1);
			indicies[i+1] = indicies[i];
         i--;
      }
      ReplaceStringInList(temp.Str(), &list, size, i+1);
		indicies[i+1] = tempIndex;
   }
}

int SortListBySubstring(char** list, long size, long start, long end, int *indicies)
{
   long i,j,tempIndex;
   char temp[MAX_STR];
   char sub1[MAX_STR];
   char sub2[MAX_STR];

   // Do a simple insertion sort
   for(j = 1; j < size; j++)
   {
		int sz = strlen(list[j]);
		if(start >= sz || end >= sz || start > end)
			return(0);
      strncpy_s(temp,MAX_STR,list[j],_TRUNCATE);
		tempIndex = indicies[j];
      strncpy_s(sub1,end-start+2,list[j]+start,_TRUNCATE);
		sub1[end-start+2] = '\0';
      i = j-1;
	   strncpy_s(sub2,end-start+2,list[i]+start,_TRUNCATE);
		sub2[end-start+2] = '\0';
      while(i >= 0 && CompareHexStrings(sub1,sub2) == -1)
      {
         ReplaceStringInList(list[i], &list, size, i+1);
			indicies[i+1] = indicies[i];
         i--;
		  strncpy_s(sub2,end-start+2,list[i]+start,_TRUNCATE);
		  sub2[end-start+2] = '\0';
      }
      ReplaceStringInList(temp, &list, size, i+1);
		indicies[i+1] = tempIndex;
   }
	return(1);
}

/**************************************************************
   Compare strings str1 and str2 alphanumerically returning 
   1 is str1 > str2
   0 if str1 == str2
   -1 if str1 < str2
***************************************************************/

short CompareStrings(char *str1, char* str2, short mode)
{
   long len1 = strlen(str1);
   long len2 = strlen(str2);

   if(!strcmp(str1,".")) return(-1);
   if(!strcmp(str1,"..") && strcmp(str2,".")) return(-1);
   if(!strcmp(str2,".")) return(1);
   if(!strcmp(str2,"..") && strcmp(str1,".")) return(1);
  
// Sort strings starting with numbers
	if(mode == 0)
	{
      float num1, num2;
      int isNum1 = 0;
      int isNum2 = 0;

      if (isdigit(str1[0])) // Prevent inf* and nan* from being interpreted as integers
         isNum1 = sscanf(str1, "%f", &num1);

      if (isdigit(str2[0]))
         isNum2 = sscanf(str2, "%f", &num2);
   
		if(isNum1 == 1 && isNum2 == 1)
		{
			if(num1 > num2) return(1);
			if(num1 == num2) return(0);
			if(num1 < num2) return(-1);
		}
		else if(isNum1 == 1 && isNum2 == 0)
			return(-1);
		else if(isNum1 == 0 && isNum2 == 1)
			return(1);
	}
	else if(mode == 2)
	{
		float num1,num2;
		int isNum1 = 0;
		int isNum2 = 0;
		for(int k = 0; k < len1; k++)
		{
			if(isdigit(str1[k]))
			{
		     isNum1 = sscanf(str1+k,"%f",&num1);
			  break;
			}
		}
		for(int k = 0; k < len2; k++)
		{
			if(isdigit(str2[k]))
			{
		     isNum2 = sscanf(str2+k,"%f",&num2);
			  break;
			}
		}   
		if(isNum1 == 1 && isNum2 == 1)
		{
			if(num1 > num2) return(1);
			if(num1 == num2) return(0);
			if(num1 < num2) return(-1);
		}
		else if(isNum1 == 1 && isNum2 == 0)
			return(-1);
		else if(isNum1 == 0 && isNum2 == 1)
			return(1);
	}
// Sort strings starting with letters
   long shorter;
   (len1 < len2) ?  (shorter = len1) : (shorter = len2);
   for(long i = 0; i < shorter; i++)
   {
      if(tolower(str1[i]) > tolower(str2[i])) return(1);
      if(tolower(str1[i]) < tolower(str2[i])) return(-1);
   }
   if(len1 == len2) 
      return(0); // Must be the same
   if(len1 < len2) 
      return(-1); 
   return(1);
}

/**************************************************************
   Compare strings str1 and str2 alphanumerically returning 
   1 is str1 > str2
   0 if str1 == str2
   -1 if str1 < str2
***************************************************************/

short CompareHexStrings(char *str1, char* str2)
{
   long len1 = strlen(str1);
   long len2 = strlen(str2);

   if(!strcmp(str1,".")) return(-1);
   if(!strcmp(str1,"..") && strcmp(str2,".")) return(-1);
   if(!strcmp(str2,".")) return(1);
   if(!strcmp(str2,"..") && strcmp(str1,".")) return(1);
  
// Sort strings starting with numbers
   int num1,num2;
   int isNum1 = sscanf(str1,"%x",&num1);
   int isNum2 = sscanf(str2,"%x",&num2);

   if(isNum1 == 1 && isNum2 == 1)
   {
      if(num1 > num2) return(1);
      if(num1 == num2) return(0);
      if(num1 < num2) return(-1);
   }
   else if(isNum1 == 1 && isNum2 == 0)
      return(-1);
   else if(isNum1 == 0 && isNum2 == 1)
      return(1);

// Sort strings starting with letters
   long shorter;
   (len1 < len2) ?  (shorter = len1) : (shorter = len2);
   for(long i = 0; i < shorter; i++)
   {
      if(tolower(str1[i]) > tolower(str2[i])) return(1);
      if(tolower(str1[i]) < tolower(str2[i])) return(-1);
   }
   if(len1 == len2) 
      return(0); // Must be the same
   if(len1 < len2) 
      return(-1); 
   return(1);
}
/******************************************************************************
*  Add a prefix to a list variable
* 
* Syntax: a = addprefix(list,prefix) .. add 'prefix' from 'list' entries
*                                       returning the result in new list 'a'
******************************************************************************/

int AddPrefix(Interface* itfc ,char args[])
{
   short r;                 // Function return flag
   Variable var;            // Variable containing copy of list
   static CText prefix;  // Prefix to remove
   CText temp;
   char **list;

   // Get list name and prefix string  
   if((r = ArgScan(itfc,args,2,"list, prefix","ee","vt",&var,&prefix)) < 0)
      return(r); 

   // Check for errors 
   if(VarType(&var) != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }

   // Get dimensions and list pointer
   list = VarList(&var);
   long n = VarWidth(&var);
   long lp = prefix.Size();

   // Add prefix to each list entry  
   for(long i = 0; i < n; i++)
   {
      temp = prefix + list[i];
      ReplaceStringInList(temp.Str(),&list,n,i);
   } 

   // Return modified list to user   
   itfc->retVar[1].MakeAndSetList(list,n);
   itfc->nrRetValues = 1;
   return(OK);    
}

/******************************************************************************
*  Remove a prefix from a list variable
* 
* Syntax: a = rmprefix(list,prefix) ... remove 'prefix' from 'list' entries
*                                       returning the result in new list 'a'
******************************************************************************/

int RemovePrefix(Interface* itfc ,char args[])
{
   short r;                 // Function return flag
   Variable var;            // Variable containing copy of list
   static CText prefix;  // Prefix to remove
   char **list;

// Get list name, and position at which to delete item   
   if((r = ArgScan(itfc,args,2,"list, prefix","ee","vt",&var,&prefix)) < 0)
      return(r); 

// Check for errors 
   if(VarType(&var) != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }
   
// Get dimensions and list pointer
   list = VarList(&var);
   long n = VarWidth(&var);
   long lp = prefix.Size();

// If prefix in list matches passed value then remove it   
   for(long i = 0; i < n; i++)
   {
      if(!strncmp(list[i],prefix.Str(),lp))
      {
         strcpy(list[i],list[i]+lp);
      }
   } 

// Return modified list to user   
   itfc->retVar[1].MakeAndSetList(list,n);
   itfc->nrRetValues = 1;
   return(OK);    
}

/******************************************************************************
   Given a parameter key return the corresponding value in a parameter list

   Syntax: STR value = getlistvalue(LIST list,STR key) 
******************************************************************************/

int GetParameterListValue(Interface* itfc ,char arg[])
{
   short r;               // Function return flag
   Variable var;          // Variable containing copy of list
   CText key;          // Key for value
	CText cmd;
	char *name;
	char *value;

// Get list name, and key (name) to search for   
   if((r = ArgScan(itfc,arg,2,"list, key","ee","vt",&var,&key)) < 0)
      return(r); 
      
// Check for errors 
   if(var.GetType() != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }

// Scan through list searching for key, if found return value
	for(long i = 0; i < var.GetDimX(); i++)
	{
		cmd = var.GetList()[i];
		name = new char[cmd.Size()+1];
		value = new char[cmd.Size()+1];
		if(ParseAssignmentString(cmd.Str(),name,value) == ERR)
      {
         ErrorMessage("invalid parameter list");
			delete [] name;
			delete [] value;
         return(ERR);
      }
		if(!strcmp(name,key.Str()))
		{
         RemoveQuotes(value);
         itfc->retVar[1].MakeAndSetString(value);
         itfc->nrRetValues = 1;
			delete [] name;
			delete [] value;
			return(OK);
		}
		delete [] name;
		delete [] value;
	}

// Couldn't find parameter so return null variable
   itfc->retVar[1].MakeNullVar();
   itfc->nrRetValues = 1;
	return(OK);
}

/******************************************************************************
   Search a parameter list for a key. Returns 1 on success, 0 on failure

   Syntax: INT value = ispar(LIST list,STR key) 
******************************************************************************/

int DoesParameterExist(Interface *itfc, char arg[])
{
   short r;               // Function return flag
   Variable var;          // Variable containing copy of list
   CText key;          // Key for value
	CText cmd;
	char *name;
	char *value;

// Get list name, and key (name) to search for   
   if((r = ArgScan(itfc,arg,2,"list, key","ee","vt",&var,&key)) < 0)
      return(r); 
      
// Check for errors 
   if(var.GetType() != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }

   if(!var.GetList())
   {
      ErrorMessage("list is empty");
      return(ERR);
   }

// Scan through list searching for key, if found return 1
	for(long i = 0; i < var.GetDimX(); i++)
	{
		cmd = var.GetList()[i];
		name = new char[cmd.Size()+1];
		value = new char[cmd.Size()+1];
		if(ParseAssignmentString(cmd.Str(),name,value) == ERR)
      {
			delete [] name;
			delete [] value;
         ErrorMessage("invalid parameter list");
         return(ERR);
      }
		if(!strcmp(name,key.Str()))
		{
			delete [] name;
			delete [] value;
         itfc->retVar[1].MakeAndSetFloat(1.0);
         itfc->nrRetValues = 1;
			return(OK);
		}
		delete [] name;
		delete [] value;
	}

// Not found so return 0
	itfc->retVar[1].MakeAndSetFloat(0.0);
   itfc->nrRetValues = 1;
   return(OK);
}

/******************************************************************************
   Search a list for an entry. Returns index on success, -1 on failure.
   Works on normal lists or parameter lists.

   Syntax: INT index = getlistindex(LIST list,STR entry) 
******************************************************************************/

int FindListIndex(Interface* itfc ,char args[])
{
   short r;               // Function return flag
   Variable var;          // Variable containing copy of list
   CText key;          // Key for value
	CText cmd;
	char *name;
	char *value;

// Get list name, and key (name) to search for   
   if((r = ArgScan(itfc,args,2,"list, key","ee","vt",&var,&key)) < 0)
      return(r); 
      
// Check for errors 
   if(var.GetType() != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }

// Value in case of failure
   itfc->retVar[1].MakeAndSetFloat(-1);
   itfc->nrRetValues = 1;

// Scan through list searching for key, if found return index
	for(long i = 0; i < var.GetDimX(); i++)
	{
		if(!strcmp(var.GetList()[i],key.Str()))
		{
         itfc->retVar[1].MakeAndSetFloat((float)i);
         return(OK);
		}
	}

// Try again assuming its a parameter list
	for(long i = 0; i < var.GetDimX(); i++)
	{
		cmd = var.GetList()[i];
		name = new char[cmd.Size()+1];
		value = new char[cmd.Size()+1];
		if(ParseAssignmentString(cmd.Str(),name,value) == ERR)
      {
			delete [] name;
			delete [] value;
	      itfc->retVar[1].MakeAndSetFloat(-1); // Not found
         return(OK);
      }
		if(!strcmp(name,key.Str()))
		{
			delete [] name;
			delete [] value;
			itfc->retVar[1].MakeAndSetFloat((float)i);
         return(OK);
		}
		delete [] name;
		delete [] value;
	}

   return(OK);
}


int SetParameterListValue(Interface* itfc ,char arg[])
{
   short r;               // Function return flag
   Variable var;          // Variable containing copy of list
	CText cmd;
	char *name = 0;
	char *value = 0;
	CText key;
	CText newValue;
	CText temp;
	char **list;
	long width;

// Get list name, and key (name) to search for   
   if((r = ArgScan(itfc,arg,3,"list, name, value","eee","vtt",&var, &key, &newValue)) < 0)
      return(r); 
      
// Check for errors 
   if(VarType(&var) != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }
	list = VarList(&var);
	width = VarWidth(&var);

// Scan through list searching for key, if found modify value
	for(long i = 0; i < width; i++)
	{
		cmd = list[i];
		name = new char[cmd.Size()+1];
		value = new char[cmd.Size()+1];
		ParseAssignmentString(cmd.Str(),name,value);
		if(!strcmp(name,key.Str()))
		{
			temp.Format("%s = %s",name,newValue);
         ReplaceStringInList(temp.Str(), &list, width, i);
		   itfc->retVar[1].MakeAndSetList(list,width);	
		   itfc->nrRetValues = 1; 
			delete [] name;
			delete [] value;
		   return(OK);
		}
		delete [] name;
		delete [] value;
	}

// Not found so add to list
	temp.Format("%s = %s",key.Str(),newValue);
   AppendStringToList(temp.Str(), &list, width);
   var.SetData((char*)list);
   var.SetDim1(width+1);
   itfc->retVar[1].MakeAndSetList(list,width+1);	
   itfc->nrRetValues = 1; 
	return(OK);
}


/******************************************************************************
   Update all entries in the list1 with those in list2 and return list1
  
   Syntax: setlistvalues(LIST list1, LIST list2) 
******************************************************************************/

int SetParameterListValues(Interface* itfc ,char arg[])
{
   short r;               // Function return flag
   Variable var1,var2;       // Variables containing lists
	char *cmd1;
	char *cmd2;
	char *temp;
	char *name1;
	char *name2;
	char *value1;
	char *value2;
	char **list1;
   char **list2;
	long sz,width1,width2;

// Get main list and update list   
   if((r = ArgScan(itfc,arg,2,"list1, list2","ee","vv",&var1, &var2)) < 0)
      return(r); 
      
// Check for errors 
   if(VarType(&var1) != LIST || VarType(&var2) != LIST)
   {
      ErrorMessage("arguments should be lists");
      return(ERR);
   }
	list1 = VarList(&var1);
	list2 = VarList(&var2);
	width1 = VarWidth(&var1);
	width2 = VarWidth(&var2);

// Scan through list searching for key, if found modify value
	for(long i = 0; i < width1; i++)
	{
      sz = strlen(list1[i]);
      cmd1 = new char[sz+1];
      name1 = new char[sz+1];
      value1 = new char[sz+1];
		strcpy(cmd1,list1[i]);
		ParseAssignmentString(cmd1,name1,value1);
 	   for(long j = 0; j < width2; j++)
	   { 
         sz = strlen(list2[j]);
         cmd2 = new char[sz+1];
         temp = new char[sz+1];
         name2 = new char[sz+1];
         value2 = new char[sz+1];
		   strcpy(cmd2,list2[j]);
		   strcpy(temp,list2[j]);
		   ParseAssignmentString(cmd2,name2,value2);
         if(!strcmp(name1,name2))
         {
            ReplaceStringInList(temp, &list1, width1, i);
         }
         delete [] temp;
         delete [] cmd2;
         delete [] name2;
         delete [] value2;
      }
      delete [] cmd1;
      delete [] name1;
      delete [] value1;
	}

   itfc->retVar[1].MakeAndSetList(list1,width1);	
   itfc->nrRetValues = 1; 
	return(OK);
}



/******************************************************************************
*  Remove a string from an existing list by position and return the result in ansVar
* 
* Syntax: a = rmfromlist(b,i) ... remove string from list 'b' at index 'i'
*                                 returning the result in new list 'a'
*
* Syntax: a = rmfromlist(b,v) ... remove string from list 'b' with value 'v'
*                                 returning the result in new list 'a'
*
* Syntax: a = rmfromlist(b,p) ... remove string from list 'b' with parameter 'p'
*                                 returning the result in new list 'a'
*
******************************************************************************/

int RemoveStringFromList(Interface* itfc ,char arg[])
{ 
   short r;                // Function return flag
   Variable var1;          // Variable containing copy of list
   Variable var2;          // Variable containing index, text or parameter
   long index;             // Index in list of string to remove.
	char cmd[MAX_STR];
	char name[MAX_STR];
	char value[MAX_STR];
   CText mode = "case";
   
 // Get parameters   
   if((r = ArgScan(itfc,arg,2,"list, index/text/parameter,[mode]","eee","vvt",&var1,&var2,&mode)) < 0)
      return(r); 
      
// Check for errors 
   if(var1.GetType() != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }

// Process request depending on type of second parameter
   if(var2.GetType() == FLOAT32)
   {
      long index = nint(var2.GetReal());
      r = RemoveListEntry((char***)var1.GetDataAdrs(), var1.GetDimX(), index);
      var1.SetDim1(var1.GetDimX()-1);
   }
   else if(var2.GetType() == UNQUOTED_STRING)
   {
      char *text = var2.GetString();

      // Scan through list searching for key, if found return index
	   for(index = 0; index < var1.GetDimX(); index++)
	   {
         strncpy_s(cmd,MAX_STR,var1.GetList()[index],_TRUNCATE);
         if(mode == "case")
         {
		      if(!strcmp(cmd,text))
		      {
               r = RemoveListEntry((char***)var1.GetDataAdrs(), var1.GetDimX(), index);
               var1.SetDim1(var1.GetDimX()-1);
			      goto ex;
		      }
         }
         else if(mode == "nocase")
         {
		      if(!stricmp(cmd,text))
		      {
               r = RemoveListEntry((char***)var1.GetDataAdrs(), var1.GetDimX(), index);
               var1.SetDim1(var1.GetDimX()-1);
			      goto ex;
		      }
         }
	   }
   
   // Try again assuming its a parameter list
	   for(index = 0; index < var1.GetDimX(); index++)
	   {
         strncpy_s(cmd,MAX_STR,var1.GetList()[index],_TRUNCATE);
		   if(ParseAssignmentString(cmd,name,value) == ERR)
         {
            itfc->retVar[1].MakeAndSetList(var1.GetList(),var1.GetDimX());
            itfc->nrRetValues = 1;
            return(OK);
         }
		   if(!strcmp(name,text))
		   {
            r = RemoveListEntry((char***)var1.GetDataAdrs(), var1.GetDimX(), index);
            var1.SetDim1(var1.GetDimX()-1);
			   goto ex;
		   }
	   }
   }
ex:

// Check for errors
   if(r == -1)
   {
      ErrorMessage("invalid deletion position");
      return(ERR);
   }
   if(r == -2)
   {
      ErrorMessage("couldn't allocate memory for modified list");
      return(ERR);
   }

// Return modified list
   itfc->retVar[1].MakeAndSetList(var1.GetList(),var1.GetDimX());
   itfc->nrRetValues = 1;
   return(OK);
}


/**************************************************************************************
  Execute the contents of a parameter list as a series of assignments

  Syntax: assignlist(LIST parList)
***************************************************************************************/

int AssignParameterList(Interface* itfc ,char args[])
{
   short nrArgs;
   Variable var;
	char *cmd;
	char *name;
	char *val;
   bool processEscapesOld;

// Get the arguments      
   if((nrArgs = ArgScan(itfc,args,1,"list","e","v",&var)) < 0)
      return(nrArgs);

// If the variable is a list then try and execute it

	if(VarType(&var) == LIST)
	{
      processEscapesOld = itfc->processEscapes;
      itfc->processEscapes = false; // Prevents some files names from getting mangled
		long size = VarWidth(&var);
		for(long i = 0; i < size; i++)
		{
         char *str = VarList(&var)[i];
         int len = strlen(str)+1;
         cmd  = new char[len];
         name = new char[len];
         val  = new char[len];
         strcpy(cmd,str);
			ParseAssignmentString(cmd,name,val);
         if(ProcessAssignment(itfc,name,val) < 0)
         {
            ErrorMessage("List string '%s' is not a valid assignment",name);
            itfc->processEscapes = processEscapesOld;
            delete [] cmd;
            delete [] name;
            delete [] val;
            return(ERR);
         }
         delete [] cmd;
         delete [] name;
         delete [] val;
		}
      itfc->processEscapes = processEscapesOld;
	}
   else if (VarType(&var) == STRUCTURE)
   {
      extern int AssignStructure(Interface * itfc, char args[]);
      return(AssignStructure(itfc,args));
   }
// If the variable is a string then try and execute it
	else if(VarType(&var) == UNQUOTED_STRING)
	{
      processEscapesOld = itfc->processEscapes;
      itfc->processEscapes = false; // Prevents some files names from getting mangled
      Variable result;
      char *str = VarString(&var);
      int len = strlen(str)+1;
      cmd  = new char[len];
      name = new char[len];
      val  = new char[len];
      strcpy(cmd,str);
		ParseAssignmentString(cmd,name,val);
      if((Evaluate(itfc,FULL_COPY,val,&result)) < 0)  // Is this needed?
      {
         itfc->processEscapes = processEscapesOld;
         delete [] cmd;
         delete [] name;
         delete [] val;
         return(ERR);
      }
		if(AssignToExpression(itfc, itfc->varScope,name,&result,true))
      {
         itfc->processEscapes = processEscapesOld;
         return(ERR);
      }
      delete [] cmd;
      delete [] name;
      delete [] val;
      itfc->processEscapes = processEscapesOld;
	}
// null variable so do nothing
   else if(VarType(&var) == NULL_VARIABLE) 
   {
	   return(OK);
   }
// Something else - abort
	else
	{
		ErrorMessage("Can't execute this type of variable");
      return(ERR);
	}

	return(OK);
}

/**************************************************************************************
  Take a parameter string of the form name = value and return name and value as
  separate strings.

  Syntax: (STR name, STR value) = splitpar(parameter_string)
***************************************************************************************/

int SplitParameterString(Interface* itfc ,char args[])
{
   short r;               // Function return flag
   CText parameter;       // Parameter name
   char name[MAX_STR];
   char value[MAX_STR];

// Get the parameter name 
   if((r = ArgScan(itfc,args,1,"parameter string","e","t",&parameter)) < 0)
      return(r); 

	if(ParseAssignmentString(parameter.Str(),name,value) == ERR)
   {
      ErrorMessage("invalid parameter string");
      return(ERR);
   }

   itfc->retVar[1].MakeAndSetString(name);
   RemoveQuotes(value);
   itfc->retVar[2].MakeAndSetString(value);
   itfc->nrRetValues = 2;
   return(OK);
}


/**************************************************************************************
  Find a parameter key in a list and rename it

  Syntax: LIST lst = renamelistkey(list, oldkey, newkey)
***************************************************************************************/

int RenameListEntry(Interface* itfc ,char args[])
{
   short r;               // Function return flag
   Variable var;          // Variable containing copy of list
   CText oldKey,newKey;   // Key for value
   CText entry;
   char name[MAX_STR];
   char value[MAX_STR];
   char **list;
   long width;

// Get list name, and old key (name) to search for and new name for key  
   if((r = ArgScan(itfc,args,3,"list, oldkey, newkey","eee","vtt",&var,&oldKey,&newKey)) < 0)
      return(r); 
      
// Check for errors 
   if(var.GetType() != LIST)
   {
      ErrorMessage("argument 1 is not a list");
      return(ERR);
   }

   list = var.GetList();
   width = var.GetDimX();

// Scan through list searching for key, if found return value
	for(long i = 0; i < width; i++)
	{
		entry = list[i];
		if(ParseAssignmentString(entry.Str(),name,value) == ERR)
      {
         ErrorMessage("invalid parameter list");
         return(ERR);
      }
		if(!strcmp(name,oldKey.Str()))
		{
          CText newEntry = newKey + " = " + value;
          ReplaceStringInList(newEntry.Str(), &list, width, i);  
		}
	}

   itfc->retVar[1].MakeAndSetList(list,width);
   itfc->nrRetValues = 1;

   return(OK);
}


/**************************************************************************************
  Take selected variables and turn them into a parameter list. If keyList is defined
  then only include those variables in the resultant list.

  Syntax: LIST parList = mkparlist([keyList])

***************************************************************************************/

int MakeParameterList(Interface* itfc ,char arg[])
{
    Variable *varBase;
   Variable *var,*varLast = NULL;
   char str[MAX_STR];
   char mat[MAX_STR];
   char **list; // The new list
   short nrArgs;
   Variable keyVar;

  // Get the key list *******************
   if ((nrArgs = ArgScan(itfc, arg, 0, "keys", "e", "v", &keyVar)) < 0)
      return(nrArgs);

// Scan through the local variable list counting entries
   if(itfc->macro)
   {
        long width,index;
      varBase = &(itfc->macro->varList);
      for(var = varBase->next, width = 0; var != NULL; var = var->next) // Loop over local variables
      {
         if (nrArgs == 1)
         {
            char** keyList = keyVar.GetList();
            long keyEntries = keyVar.GetDimX();
            bool found = false;
            for (int j = 0; j < keyEntries; j++) // Loop over keys
            {
               if (!strcmp(var->GetName(), keyList[j]))
               {
                  found = true;
               }
            }
            if (!found)
               continue;
         }

         if(var->GetData() && !strcmp(var->GetName(),"nrArgs")) // Ignore this variable
            continue;

         switch(var->GetType())
         {
            case(FLOAT32):
            case(FLOAT64):
            case(UNQUOTED_STRING):
               width++;
               break;
            case(MATRIX2D):
               if(VarWidth(var) <= 3 && VarHeight(var) == 1)
                  width++;
               break;
         }

         varLast = var;
      }
      if(!varLast)
      {
         ErrorMessage("no local variables defined");
         return(ERR);
      }
   // Make the list
      list = new char*[width];

   // Scan through the local variable list adding entries to parameter list
      varBase = &(itfc->macro->varList);
      for (var = varBase->next, index = 0; var != NULL; var = var->next) // Loop over local variables
      {
         if (nrArgs == 1)
         {
            char** keyList = keyVar.GetList();
            long keyEntries = keyVar.GetDimX();
            bool found = false;
            for (int j = 0; j < keyEntries; j++) // Loop over keys
            {
               if (!strcmp(var->GetName(), keyList[j]))
               {
                  found = true;
               }
            }
            if (!found)
               continue;
         }

         if(var->GetData() && !strcmp(var->GetName(),"nrArgs")) // Ignore this variable
            continue;
         switch(var->GetType())
         {
            case(FLOAT32):
				{
                 if(IsInteger(VarReal(var)))
                  sprintf(str,"%s = %d",var->GetName(),nint(VarReal(var)));
                    else
                  sprintf(str,"%s = %.9g",var->GetName(),VarReal(var));
                    list[index] = new char[strlen(str)+1];
               strcpy(list[index++],str);
               break;
				}
            case(FLOAT64):
				{
                 if(IsInteger(var->GetDouble()))
                     sprintf(str, "%s = %lldd", var->GetName(), nhint(var->GetDouble()));
                  else
                     sprintf(str, "%s = %.17gd", var->GetName(), var->GetDouble());
               list[index] = new char[strlen(str) + 1];
               strcpy(list[index++], str);
               break;
				}
            case(UNQUOTED_STRING):
            {
               char out[MAX_STR];
               AddEscapedQuotes(out,VarString(var));
               sprintf(str,"%s = \"%s\"",var->GetName(),out);
               list[index] = new char[strlen(str)+1];
               strcpy(list[index++],str);
               break;
            }
            case(MATRIX2D):
            {
               if(VarWidth(var) <= 3 && VarHeight(var) == 1) // Limit to 3 by 1 until we have better string handling
               {
                  sprintf(str,"%s = [",var->GetName());
                  for(long x = 0; x < VarWidth(var)-1; x++)
                  {
                     sprintf(mat,"%g, ",VarRealMatrix(var)[0][x]);
                     strcat(str,mat);
                  }
						sprintf(mat,"%g]",VarRealMatrix(var)[0][VarWidth(var)-1]);
                  strcat(str,mat);
                  list[index] = new char[strlen(str)+1];
                  strcpy(list[index++],str);
               }
               break;
            }
         }
      }
      itfc->retVar[1].MakeAndSetList(list,width);
      itfc->nrRetValues = 1;
       FreeList(list,width);
   }
   else
   {
      ErrorMessage("can only be used in a macro procedure");
      return(ERR);
   }
   return(OK);
} 


/********************************************************************************
   Merges parameter 'list1' with parameter 'list2' producing a new list. 
   'result' will include all of 'list1' and anything in 'list2' not in 'list1'

   Syntax: LIST result = mergelists(LIST list1, LIST list2)
*********************************************************************************/

int MergeParameterLists(Interface* itfc ,char args[])
{
	short nrArgs;
	Variable listVar1;
	Variable listVar2;
	long i, j;
	CText cmd;

// Get the two lists to be merged ******************
	if((nrArgs = ArgScan(itfc,args,2,"list 1, list 2","ee","vv",&listVar1,&listVar2)) < 0)
		return(nrArgs); 

   short type1 = listVar1.GetType();
   short type2 = listVar2.GetType();

   if(type1 == LIST && type2 == NULL_VARIABLE)
   {
      CopyVariable(&itfc->retVar[1],&listVar1,FULL_COPY);
      itfc->nrRetValues = 1;
      return(OK);
   }

   if(type2 == LIST && type1 == NULL_VARIABLE)
   {
      CopyVariable(&itfc->retVar[1],&listVar2,FULL_COPY);
      itfc->nrRetValues = 1;
      return(OK);
   }

// Check that they are lists ***********************
	if(type1 != LIST)
	{
		ErrorMessage("argument 1 should be a list");
		return(ERR);
	}

	if(VarType(&listVar2) != LIST)
	{
		ErrorMessage("argument 2 should be a list");
		return(ERR);
	}

// Get list handles *********************************
   char **list1 = VarList(&listVar1);
   char **list2 = VarList(&listVar2);

// Get the size of each list ************************
	long entries1 = (long)VarWidth(&listVar1);
	long entries2 = (long)VarWidth(&listVar2);

// Merge the two lists ******************************
   char **newList = NULL;
   long totalEntries = 0;

// Add entries from list1 ***************************
	for(i = 0; i < entries1; i++)
	{
      AppendStringToList(list1[i], &newList, totalEntries++);
   }

// Add entries from list2 which are not in list1 ****
   for(i = 0; i < entries2; i++)
	{
	   cmd = list2[i];
		char* name2 = new char[cmd.Size()+1];
		char* value2 = new char[cmd.Size()+1];
		ParseAssignmentString(cmd.Str(),name2,value2);
	   for(j = 0; j < entries1; j++)
	   {
	      cmd = list1[j];
			char* name1 = new char[cmd.Size()+1];
		   char* value1 = new char[cmd.Size()+1];
		   ParseAssignmentString(cmd.Str(),name1,value1);
         if(!strcmp(name1,name2))
			{
				delete [] name1;
		      delete [] value1;
            break;
			}
			delete [] name1;
		   delete [] value1;
      }
      if(j == entries1)
      {
         AppendStringToList(list2[i], &newList, totalEntries++);
      }
	// Free allocated space
		delete [] name2;
		delete [] value2;
   }

// Copy the result to ansVar	************************
   itfc->retVar[1].AssignList(newList,totalEntries);
   itfc->nrRetValues = 1;


	return(OK);  
}


/********************************************************************************
   Extract from parameter list 'list' all entries which have a name which matches   
   the passed list of strings (parameter names) or in the second case which has
   a name which matches the passed string. 

   Syntax:  LIST result = getsublist(LIST list, LIST/STR name(s))
*********************************************************************************/

int GetSubParameterList(Interface* itfc ,char args[])
{
	short nrArgs;
	Variable listVar;
	Variable keyVar;
	char cmd[MAX_STR];
	char srcName[MAX_STR];
	char srcValue[MAX_STR];

// Get the source list and the key list *******************
	if((nrArgs = ArgScan(itfc,args,2,"list, names","ee","vv",&listVar,&keyVar)) < 0)
		return(nrArgs); 

// Check that 'listVar' is a list ********************
	if(listVar.GetType() != LIST)
	{
		ErrorMessage("argument 1 should be a list");
		return(ERR);
	}

// Search for names which match the names in the passed simple list *******************
	if(keyVar.GetType() == LIST)
	{
   // Get list handles **********************************
      char **srcList = listVar.GetList();
      char **keyList = keyVar.GetList();

   // Get the size of lists *****************************
	   long srcEntries = (long)listVar.GetDimX();
	   long keyEntries = (long)keyVar.GetDimX();

   // Make the output list *****************************
      char **newList = NULL;
      long totalEntries = 0;

   // Scan through list looking for matches ************
      for(int i = 0; i < srcEntries; i++)
      {
	      strncpy_s(cmd,MAX_STR,srcList[i],_TRUNCATE);
		   ParseAssignmentString(cmd,srcName,srcValue);

         for(int j = 0; j < keyEntries; j++)
         {
            if(!strcmp(srcName,keyList[j]))
            {
               AppendStringToList(srcList[i], &newList, totalEntries++);
            }
         }
      }

   // Copy the result to ansVar	************************
      itfc->retVar[1].AssignList(newList,totalEntries);
      itfc->nrRetValues = 1;
	}

// Search for names which start with the passed string **********************************

   else if(keyVar.GetType() == UNQUOTED_STRING)
   {
   // Get list handles *********************************
      char **list = listVar.GetList();
      char *templt = keyVar.GetString();

   // Get the size of list *****************************
	   long entries = (long)listVar.GetDimX();

   // Make the output list *****************************
       char **newList = NULL;

      long totalEntries = 0;
      long len = strlen(templt);

   // Scan through list looking for matches ************
      for(int i = 0; i < entries; i++)
      {
	      strncpy_s(cmd,MAX_STR,list[i],_TRUNCATE);
		   ParseAssignmentString(cmd,srcName,srcValue);

         if(!strncmp(srcName,templt,len))
         {
            AppendStringToList(list[i], &newList, totalEntries++);
         }
      }

   // Copy the result to ansVar	************************
      itfc->retVar[1].AssignList(newList,totalEntries);
      itfc->nrRetValues = 1;
   }
   else
	{
		ErrorMessage("argument 1 should be a list or a string");
		return(ERR);
	}

	return(OK);  
}

int ShuffleItem(Interface* itfc ,char args[])
{
	short nrArgs;
	Variable listVar;

// Get the source list and the key list *******************
	if((nrArgs = ArgScan(itfc,args,1,"list/matrix","e","v",&listVar)) < 0)
		return(nrArgs); 

// Check that 'listVar' is a list ********************
	if(listVar.GetType() == LIST)
   {

// Search for names which match the names in the passed simple list *******************

   // Get list handles **********************************
      char **srcList = listVar.GetList();

   // Get the size of lists *****************************
	   long srcEntries = (long)listVar.GetDimX();

   // Make the output list *****************************
		vector<string> shuffleMe;

		for (int i = 0; i < srcEntries; i++)
		{
			shuffleMe.push_back(string(srcList[i]));
		}

	// Shuffle it
		random_shuffle(shuffleMe.begin(),shuffleMe.end());
		
      char **newList = NULL;
      long totalEntries = 0;

   // Scan through list looking for matches ************
      for(int i = 0; i < srcEntries; i++)
      {
			AppendStringToList(shuffleMe[i].c_str(), &newList, totalEntries++);
      }

   // Copy the result to ansVar	************************
      itfc->retVar[1].AssignList(newList,totalEntries);
      itfc->nrRetValues = 1;
	}
   else if(listVar.GetType() == MATRIX2D)
   {
// Search for names which match the names in the passed simple list *******************

   // Get list handles **********************************
      float **srcMat = listVar.GetMatrix2D();
      int w = listVar.GetDimX();
      int h = listVar.GetDimY();
      
   // Get the size of lists *****************************
	   long srcEntries = (long)listVar.GetDimX();

   // Make the output list *****************************
		vector<float> shuffleMe;

		for (int y = 0; y < h; y++)
		   for (int x = 0; x < w; x++)
			   shuffleMe.push_back(srcMat[y][x]);

	// Shuffle it
		random_shuffle(shuffleMe.begin(),shuffleMe.end());

   // Copy to output matrix
      float **dstMat = MakeMatrix2D(w,h);
      int cnt = 0;
		for (int y = 0; y < h; y++)
		   for (int x = 0; x < w; x++)
            dstMat[y][x] = shuffleMe[cnt++];

   // Copy the result to ansVar	************************
      itfc->retVar[1].AssignMatrix2D(dstMat,w,h);
      itfc->nrRetValues = 1;
   }
   else
   {
		ErrorMessage("argument 1 should be a list or a matrix");
		return(ERR);
	}

	return(OK);  
}



/*****************************************************************************
*              Make a list with n entries (fill with dummy strings)          *
*****************************************************************************/

EXPORT char** MakeList(long width)
{
   char temp[20]; // String for dummy text entries
   
// Make new list                              
   char **list = new char*[width];
   if(!list) return(NULL);  
// And fill it with dummy strings
	for(long i = 0; i < width; i++)
	{
	  sprintf(temp,"item %ld",i);
	  list[i] = new char[strlen(temp)+1];
	  if(!list[i]) return(NULL);
	  strcpy(list[i],temp);
   }
   return(list);
}


/*****************************************************************************
*              Free space allocated for an array of strings (a list)         *
*****************************************************************************/

EXPORT void FreeList(char **list, long n)
{
	if(!list)
		return;
	for(long i = 0; i < n; i++)
	   delete [] ((char**)list)[i]; 
	delete [] list;
   list = NULL;
}



/*****************************************************************************
*              Make a 2D list with w, h entries (fill with dummy strings)    *
*****************************************************************************/

EXPORT List2DData*  Make2DList(long width, long height)
{
   char temp[20]; // String for dummy text entries
   
// Make new list                              
   List2DData *list = new List2DData(height);
   if(!list)
   {
		 ErrorMessage("Can't allocate memory for 2D list");
		 return(NULL); 
	}

// And fill it with dummy strings
	for(long j = 0; j < height; j++)
	{
		if(!list->MakeRow(j,width))
	   {
			 delete list;
		    ErrorMessage("Can't allocate memory for 2D list");
		    return(NULL); 
		}
		for(long i = 0; i < width; i++)
		{
		  sprintf(temp,"item %ld,%ld",i,j);
		  if(!list->AddEntry(temp,i,j))
		  {
			 delete list;
		    ErrorMessage("Invalid 2D index or can't allocate memory for 2D list entry");
		    return(NULL); 
		  }
		}
	}
	list->nrRows = height;
	list->maxCols = width;
   return(list);
}

/*****************************************************************************
* Join dst list to src list and overwrite dst list 
* Returns -2 if memory allocation error
* Returns 0 if join successful
* Note: you must pass the address of the destination list
*****************************************************************************/

EXPORT short JoinAndOverWriteLists(char*** dst, char **src, long dstLen, long srcLen)
{
   char **joinedList; // The new list
   long i;            // Array index

// Make a new list, equal in length to the sum of the two being joined   
   joinedList = new char*[dstLen+srcLen];
   if(!joinedList) return(-2);
// Copy the destination list into the new list
   for(i = 0; i < dstLen; i++)
   {
      joinedList[i] = new char[strlen((*dst)[i])+1];
      if(!joinedList[i]) return(-2);
      strcpy(joinedList[i],(*dst)[i]);
   }
// Append the source list to the new list
   for(i = dstLen; i < dstLen+srcLen; i++)
   {
      joinedList[i] = new char[strlen(src[i-dstLen])+1];
      if(!joinedList[i]) return(-2);      
      strcpy(joinedList[i],src[i-dstLen]);
   }
// Free the destination list   
   FreeList((*dst),dstLen);  
// Make the destination list the joined list
   (*dst) = joinedList;
   return(OK);
}

/*****************************************************************************
* Joins list1 and list2 and return as a new list. 
* Returns NULL if memory allocation error
*****************************************************************************/

EXPORT char**  JoinLists(char** list1, char **list2, long len1, long len2)
{
   char **joinedList; // The new list
   long i;            // Array index

// Make a new list, equal in length of the sum of the two being joined   
   joinedList = new char*[len1+len2];
   if(!joinedList) return(NULL);
// Copy list1 into the new list
   for(i = 0; i < len1; i++)
   {
      joinedList[i] = new char[strlen(list1[i])+1];
      if(!joinedList[i]) return(NULL);
      strcpy(joinedList[i],list1[i]);
   }
// Append list2 to the new list
   for(i = len1; i < len1+len2; i++)
   {
      joinedList[i] = new char[strlen(list2[i-len1])+1];
      if(!joinedList[i]) return(NULL);      
      strcpy(joinedList[i],list2[i-len1]);
   }
   return(joinedList);
}

/****************************************************************
* Replace string at list index 'position' with 'newStr'
* Returns -1 if invalid position
* Returns -2 if memory allocation error
* Returns 0 if insertion successful
* Note: this routine assumes 'list' is a valid data structure with
* 'listLen' elements - if not it may crash!
* Note: you must pass the address of the  list
****************************************************************/

EXPORT short ReplaceStringInList(char *newStr, char ***list, long listLen, long position)
{
// Check for valid position
   if(position < 0 || position >= listLen)
      return(-1);
// Delete old entry     
   if((*list)[position] != NULL) delete [] (*list)[position];
// Add new one   
   (*list)[position] = new char[strlen(newStr)+1];
   if(!(*list)[position]) return(-2);
   strcpy((*list)[position],newStr);
   return(0);
}


/****************************************************************
* Replace string at list index 'position' with 'newStr'
* Returns -1 if invalid position
* Returns -2 if memory allocation error
* Returns 0 if insertion successful
* Note: this routine assumes 'list' is a valid data structure with
* 'listLen' elements - if not it may crash!
* Note: you must pass the address of the  list
****************************************************************/

EXPORT short ReplaceStringIn2DList(char *newStr, List2DData *list, long listWidth, long listHeight, long xPos, long yPos)
{
// Check for valid position
   if(xPos < 0 || xPos >= listWidth || yPos < 0 || yPos >= listHeight)
      return(-1);
	if(xPos >= list->rowSz[yPos])
		return(-1);

// Delete old entry     
   if((list->strings)[yPos][xPos] != NULL)
		delete [] (list->strings)[yPos][xPos];
// Add new one   
   (list->strings)[yPos][xPos] = new char[strlen(newStr)+1];
   if(!(list->strings)[yPos][xPos]) 
		return(-2);
   strcpy((list->strings)[yPos][xPos],newStr);
   return(0);
}


/****************************************************************
* Remove list entry at index 'position'
* Returns -1 if invalid position
* Returns -2 if memory allocation error
* Returns 0 if removal successful
* Note this routine assumes 'list' is a valid data structure with
* 'listLen' elements - if not it may crash!
* Note: you must pass the address of the  list
****************************************************************/

EXPORT short RemoveListEntry(char ***list, long listLen, long position)
{
   char **newList;
   long i;
// Check for valid position
   if(position < 0 || position >= listLen)
      return(-1);
// Make a new list, one smaller than the old one   
   newList = new char*[listLen-1];
   if(!newList) return(-2);
// Copy first part of original list into new list      
   for(i = 0; i < position; i++)
   {
      newList[i] = new char[strlen((*list)[i])+1];
      if(!newList[i]) return(-2);
      strcpy(newList[i],(*list)[i]);
   }
// Copy reset of original list into new list skipping entry at position    
   for(i = position+1; i < listLen; i++)
   {
      newList[i-1] = new char[strlen((*list)[i])+1];
      if(!newList[i-1]) return(-2);
      strcpy(newList[i-1],(*list)[i]);
   } 
// Update original list with new one  
   FreeList((*list),listLen);  
   (*list) = newList;
   return(OK);
}

/*****************************************************************************
*             Compare lists returning true if equal false if not             *
*****************************************************************************/

EXPORT bool CompareLists(char** dst, char **src, long dstLen, long srcLen)
{
// Make sure their lengths are the same
   if(dstLen != srcLen) return(false);
// Compare the strings for each index   
   for(int i = 0; i < dstLen; i++)
   {
      if(strcmp(dst[i],src[i]))
         return(false);
   }
// All the same so return true
   return(true);
}

/*****************************************************************************
*    Convert a comma delimited list of substrings into a string array (a list)  
*    Returns new list is all ok, NULL if there is a memory allocation error.
*****************************************************************************/

EXPORT char** MakeListFromText(char* text, long *cols)
{
   char **list;
   CArg carg;

   carg.Init(',');

// Make a new string big enough to take longest substring in text
   char *str = new char[strlen(text)+1];
// See how many substrings there are
   (*cols) = carg.Count(text);
//   (*cols) = CountArgsB(text,',');
// Make an array of strings   
   list = new char*[(*cols)];
   if(!list)
	{
		delete[] str;
		return(NULL);
	}
// Store each comma delimited substring in 'text' into the new list
   for(long i = 0; i < (*cols); i++)
   {
      strcpy(str,carg.Extract(i+1));
      list[i] = new char[strlen(str)+1];
      if(!list[i])
		{
			delete[] str;		
			return(NULL);
		}
      strcpy(list[i],str);
   }
// Delete the temporary string and return the completed list  
   delete [] str;
   return(list);
}  

/*****************************************************************************
*  Convert a comma delimited string into a string array after evaluating     *
*  each item to see if it has an embedded expression.                        *
*  Returns NULL list if error occurs.
*****************************************************************************/

EXPORT char** MakeListFromEvaluatedText(Interface *itfc, char* text, long *cols)
{
   char **list;   // Array to hold all substrings in text
   CText s;     // Make a string long enough to take any evaluated argument
   CArg carg;     // Argument list
   Variable result;

// See how many substring there are in text 
   carg.Init(',');
   (*cols) = carg.Count(text);

// Make an array of strings   
   list = new char*[(*cols)];
   if(!list) return(NULL);

// Store each comma delimited evaluated substring of text in list
   for(long i = 0; i < (*cols); i++)
   {
      s.Assign(carg.Extract(i+1));
      if((Evaluate(itfc,RESPECT_ALIAS,s.Str(),&result)) == ERR)
      {
         for(int j = 0; j < i ; j++)
         {
            delete list[j];
         }
         delete [] list;
         return(NULL);  
      }    
      s.Assign(result.GetString());         
      list[i] = new char[s.Size()+1];
      if(!list) 
		{
			ErrorMessage("Can't allocate memory for list");
	    	return(NULL);
		}
      strcpy(list[i],s.Str());
   }
   return(list);
}


/*****************************************************************************
*  Convert a comma delimited unquoted string into a string array after evaluating     *
*  each item to see if it has an embedded expression.                        *
*  Returns NULL list if error occurs.
*****************************************************************************/

EXPORT char** MakeListFromUnquotedText(Interface *itfc, char* text, long *cols)
{
   char **list;   // Array to hold all substrings in text
   CText s;     // Make a string long enough to take any evaluated argument
   CArg carg;     // Argument list
   Variable result;

// See how many substring there are in text 
   carg.Init(',');
   (*cols) = carg.Count(text);

// Make an array of strings   
   list = new char*[(*cols)];
   if(!list) return(NULL);

// Store each comma delimited evaluated substring of text in list
   for(long i = 0; i < (*cols); i++)
   {
      s.Assign(carg.Extract(i+1));
      ReplaceSubStr(s,"\"","\\\"");
      s = "\"" + s;
      s = s + "\"";
      if((Evaluate(itfc,RESPECT_ALIAS,s.Str(),&result)) == ERR)
      {
         for(int j = 0; j < i ; j++)
         {
            delete list[j];
         }
         delete [] list;
         return(NULL);  
      }    
      s.Assign(result.GetString());         
      list[i] = new char[s.Size()+1];
      if(!list) 
		{
			ErrorMessage("Can't allocate memory for list");
	    	return(NULL);
		}
      strcpy(list[i],s.Str());
   }
   return(list);
}

int Convert1DListTo2D(Interface* itfc ,char args[])
{
	short nrArgs;
	Variable listVar;
   List2DData *list2D;  
   char **list1D;  

// Get the source list and the key list *******************
	if((nrArgs = ArgScan(itfc,args,1,"list","e","v",&listVar)) < 0)
		return(nrArgs); 
	
// Check that 'listVar' is a list ********************
	if(listVar.GetType() != LIST)
	{
		ErrorMessage("argument should be a list");
		return(ERR);
	}

// Input 1D list - get length
	list1D = listVar.GetList();
	int width = listVar.GetDimX();

// Make output 2D list - 1 row
	list2D = new List2DData(1);
	if(!list2D->MakeRow(0,width))
	{
		ErrorMessage("Can't allocate memory for 2D list");
		return(ERR);
	}

// Copy from 1D list to 2D list
	for(int i = 0; i < width; i++)
	{
		if(!list2D->AddEntry(list1D[i],i,0))
		{
			delete list2D;
			ErrorMessage("Can't allocate memory for 2D list entry");
			return(ERR);  
		}
	}
	list2D->maxCols = width;


	itfc->retVar[1].Assign2DList(list2D);
	itfc->nrRetValues = 1;

	return(OK);
}

// Convert a 2D list into a 1D list (makes it easier to iterate through it)
int Convert2DListTo1D(Interface* itfc ,char args[])
{
	short nrArgs;
	Variable listVar;
   List2DData *list2D;  

// Get the source list and the key list *******************
	if((nrArgs = ArgScan(itfc,args,1,"2D list","e","v",&listVar)) < 0)
		return(nrArgs); 
	
// Check that 'listVar' is a list ********************
	if(listVar.GetType() != LIST2D)
	{
		ErrorMessage("argument should be a 2D list");
		return(ERR);
	}

// Input 2D list - get length
	list2D = listVar.GetList2D();
	int height = listVar.GetDimY();
	int len = 0;
	for(int y = 0; y < height; y++)
	{
		len += list2D->rowSz[y];
	}
	
// Allocate space for the 1D list
   char **list1D = NULL;

// Copy from 1D list to 2D list
	int pos = 0;
	for(int y = 0; y < height; y++)
	{
		int width = list2D->rowSz[y];

		for(int x = 0; x < width; x++)
		{
			char *entry = list2D->strings[y][x];
			AppendStringToList(entry, &list1D, pos++);
		}
	}

// Copy the list to the output
	itfc->retVar[1].AssignList(list1D, len);
	itfc->nrRetValues = 1;

	return(OK);
}

/*****************************************************************************
*  Convert a comma delimited string into a string array after evaluating     *
*  each item to see if it has an embedded expression.                        *
*  Returns NULL list if error occurs.
*****************************************************************************/

EXPORT List2DData* Make2DListFromEvaluatedText(Interface *itfc, char* text)
{
   List2DData *list;   // Structure to hold all substrings in text
   CText rowStr;      // Make a string to hold the row
   CText colStr;      // Make a string long enough to take any evaluated argument
   CArg cargR;        // Row list
   CArg cargC;        // Column list
   Variable result;
	long nrCols = -1;
	int nrRows;
	int width;

// See how many rows there are in text 
   RemoveEndBlanks(text);
   cargR.Init(';');
   nrRows = cargR.Count(text);
// Ignore a seimcolon at the end
   int len = strlen(text);
   if(text[len-1] == ';')
      nrRows--;

	list = new List2DData(nrRows);


// Extract each row from text
	for(long j = 0; j < nrRows; j++)
	{
      rowStr.Assign(cargR.Extract(j+1));
      if(rowStr.Size() == 0)
      {
         ErrorMessage("Empty 2D list element on row %d",j);
         return(NULL);
      }

		cargC.Init(',');
		width = cargC.Count(rowStr.Str());
		if(width > list->maxCols)
			list->maxCols = width;

	// Set up the row  
		if(!list->MakeRow(j,width))
			return(NULL);

	// Store each comma delimited evaluated substring of text in list
		for(long i = 0; i < width; i++)
		{
			colStr.Assign(cargC.Extract(i+1));
			if((Evaluate(itfc,RESPECT_ALIAS,colStr.Str(),&result)) == ERR)
			{
				delete list;
				ErrorMessage("Invalid entry '%s' in 2D list",colStr.Str());
				return(NULL);  
			}    
			colStr.Assign(result.GetString());  
			if(!list->AddEntry(colStr.Str(),i,j))
			{
				delete list;
				ErrorMessage("Invalid 2D index or can't allocate memory for 2D list entry");
				return(NULL);  
			}
		}
   }
   return(list);
}


/*****************************************************************************
*  Convert a comma delimited string into a string array after evaluating     *
*  each item to see if it has an embedded expression.                        *
*  Returns NULL list if error occurs.
*****************************************************************************/

EXPORT List2DData* Make2DListFromUnquotedText(Interface *itfc, char* text)
{
   List2DData *list;   // Structure to hold all substrings in text
   CText rowStr;      // Make a string to hold the row
   CText colStr;      // Make a string long enough to take any evaluated argument
   CArg cargR;        // Row list
   CArg cargC;        // Column list
   Variable result;
	long nrCols = -1;
	int nrRows;
	int width;

// See how many rows there are in text 
   cargR.Init(';');
   nrRows = cargR.Count(text);
	list = new List2DData(nrRows);

// Extract each row from text
	for(long j = 0; j < nrRows; j++)
	{
      rowStr.Assign(cargR.Extract(j+1));

		cargC.Init(',');
		width = cargC.Count(rowStr.Str());
		if(width > list->maxCols)
			list->maxCols = width;

	// Set up the row  
		if(!list->MakeRow(j,width))
			return(NULL);

	// Store each comma delimited evaluated substring of text in list
		for(long i = 0; i < width; i++)
		{
			colStr.Assign(cargC.Extract(i+1));
         ReplaceSubStr(colStr,"\"","\\\"");
         colStr = "\"" + colStr;
         colStr = colStr + "\"";
			if((Evaluate(itfc,RESPECT_ALIAS,colStr.Str(),&result)) == ERR)
			{
				delete list;
				ErrorMessage("Invalid entry '%s' in 2D list",colStr.Str());
				return(NULL);  
			}    
			colStr.Assign(result.GetString());  
			if(!list->AddEntry(colStr.Str(),i,j))
			{
				delete list;
				ErrorMessage("Invalid 2D index or can't allocate memory for 2D list entry");
				return(NULL);  
			}
		}
   }
   return(list);
}


/*****************************************************************************
*  Copy the list 'src' which has length 'len' and return it to the caller.
*****************************************************************************/
 
EXPORT char** CopyList(char **src, long len)
{
// Make copy array                              
   char **copy = new char*[len];
   if(!copy) return(NULL);
// Fill it from source list
	for(int i = 0; i < len; i++)
	{
	  copy[i] = new char[strlen(src[i])+1];
	  if(!copy[i]) return(NULL);
	  strcpy(copy[i],src[i]);
   }
// Return copied list 
   return(copy);
}

/*****************************************************************************
 Insert a string at position 'pos' in list. 

 Returns -1 if invalid position (shouldn't happen)
 Returns -2 if memory allocation error
 Returns 0 if insertion successful
 Note: this routine assumes 'list' is a valid data structure with
 at least 'listSize' elements - if not it may crash!
 Note: you must pass the address of the  list
 Note: existing entry at position 'pos' will be deleted.

******************************************************************************/

//short InsertStringIntoList(const char *str, char ***list, long listSize, long pos)
//{
//	if(pos < 0 || pos >= listSize)
//		return(-1);
//
//	if((*list)[pos])
//		delete [] (*list)[pos];
//
//	 (*list)[pos] = new char[strlen(str)+1];
//	if(!((*list)[pos]))
//		return(-2);
//	strcpy((*list)[pos],str);
//
//	return(0);
//}


/****************************************************************
* Insert string 'str' into 'list' at entry index 'position'
* Returns -1 if invalid position
* Returns -2 if memory allocation error
* Returns 0 if insertion successful
* Note: this routine assumes 'list' is a valid data structure with
* 'listLen' elements - if not it may crash!
* Note: you must pass the address of the  list
****************************************************************/

EXPORT short InsertStringIntoList(const char* str, char ***list, long listLen, long position)
{
   char **newList;
   long i;
// Check for valid position
   if(position < 0 || position > listLen)
      return(-1);
// Make a new list, one longer than the old one   
   newList = new char*[listLen+1];
   if(!newList) return(-2);
// Copy first part of original list into new list      
   for(i = 0; i < position; i++)
   {
      newList[i] = new char[strlen((*list)[i])+1];
      if(!newList[i]) return(-2);
      strcpy(newList[i],(*list)[i]);
   }
// Add string to list
   newList[position] = new char[strlen(str)+1];
   if(!newList[position]) return(-2);
   strcpy(newList[position],str);
// Copy reset of original list into new list      
   for(i = position+1; i <= listLen; i++)
   {
      newList[i] = new char[strlen((*list)[i-1])+1];
      if(!newList[i]) return(-2);
      strcpy(newList[i],(*list)[i-1]);
   } 
// Update original list with new one  
   if(listLen > 0 && (*list) != NULL)
      FreeList((*list),listLen);  
   (*list) = newList;
   return(OK);
}

/*****************************************************************************
 Make a string list with listSize entries which are all nulled
******************************************************************************/

char** MakeEmptyList(long listSize)
{
	char **list = new char*[listSize];
	if(!list)
		return(0);
	for(int i = 0; i < listSize; i++)
	{
		list[i] = 0;
	}
	return(list);
}


/*****************************************************************************
 Deallocate the memory used by a string list but keep the structure intact
******************************************************************************/

void ClearList(char ***list, long listSize)
{
	for(int i = 0; i < listSize; i++)
	{
		if(list[i])
		{
			delete [] list[i];
			list[i] = 0;
		}
	}
}


 
/*****************************************************************************
 Add a string to an existing list at 'position'. Any elements after this
 are discarded. If position is the size of the original list the you are
 effecitively appended to the end of the list.

 Returns -1 if invalid position (shouldn't happen)
 Returns -2 if memory allocation error
 Returns 0 if insertion successful
 Note: this routine assumes 'list' is a valid data structure with
 at least 'position' elements - if not it may crash!
 Note: you must pass the address of the  list
******************************************************************************/

EXPORT short AppendStringToList(const char* str, char ***list, long position)
{
   char **newList;
   long i;
// Check for valid position
   if(position < 0)
      return(-1);
// Make a new list, one longer than the old one   
   newList = new char*[position+1];
   if(!newList) 
		return(-2);
// Copy first part of original list into new list      
   for(i = 0; i < position; i++)
   {
      newList[i] = new char[strlen((*list)[i])+1];
      if(!newList[i]) 
			return(-2);
      strcpy(newList[i],(*list)[i]);
   }
// Add new string to end of list
   newList[position] = new char[strlen(str)+1];
   if(!newList[position]) 
		return(-2);
   strcpy(newList[position],str);
// Update original list with new one  
   FreeList((*list),position);  
   (*list) = newList;
   return(OK);
}

// 2D list class member functions

// Constructor
List2DData::List2DData(int rows)
{
	nrRows = rows;
   strings = new char**[nrRows];
   if(!strings) return;
	rowSz = new int[nrRows];
	if(!rowSz) return;
	for(long i = 0; i < nrRows; i++)
	{
		strings[i] = 0;
		rowSz[i] = 0;
	}
	maxCols = 0;
}

// Destructor
List2DData::~List2DData()
{
	for(long y = 0; y < nrRows; y++)
	{
		if(rowSz[y] > 0)
		{
	      for(long x = 0; x < rowSz[y] ; x++)
			{
				if(strings[y][x])
				{
					delete [] strings[y][x];
					strings[y][x] = 0;
				}
			}
			rowSz[y] = 0;
		}
		delete [] strings[y];
	}
	delete [] rowSz;
	delete strings;
	strings = 0;
	rowSz = 0;
	nrRows = 0;
	maxCols = 0;
}

// Setup a row in a 2D list
 bool List2DData::MakeRow(int row, int width)
 {
	if(row >= this->nrRows)
		return(false);
	if(strings[row])
		delete [] strings[row];
	strings[row] = new char*[width];
	if(!strings[row]) 
		return(false);
 	for(long i = 0; i < width; i++)
		strings[row][i] = 0;  
	rowSz[row] = width;
	return(true);
 }

 // Add an entry to a 2D list at location x,y
 bool List2DData::AddEntry(char *txt, int x, int y)
 {
	 if(strings[y][x]) 
		 delete [] strings[y][x];
	 if(y < 0 || y >= this->nrRows || x < 0 || x >= this->rowSz[y])
		 return(false);
	strings[y][x] = new char[strlen(txt)+1];
	if(!strings[y][x])
		return(false);   
	strcpy(strings[y][x],txt);

	return(true);
 }
