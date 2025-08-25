#ifndef VARIABLESCLASS_H
#define VARIABLESCLASS_H

#include "defines.h"
#include "list2d.h"

// Variable class ****************

extern "C" class  __declspec(dllexport)  Variable
{
	public:
		Variable();                                                   // Make a new variable
		~Variable();                                                  // Destroy variable

      Variable* Add(short, char[]);                                  // Link a new variable to the current one
      Variable* AddToStart(short, char[]);                           // Link a new variable to the current one at the start

   // Set variable functions ****************
      void SetData(char* d) {data = d;}                             // Set data pointer (don't clear first)		
      void NullData(void);                                          // Set data pointer to NULL 		
      void MakeNullVar(void);                                       // Clear variable information		
      void MakeClass(short,void*);                                  // Make a class variable		
      void MakeClassItem(short, char*, char*);                      // Make a class item variable
      void MakeStruct(void);                                        // Make a structure variable	
      void MakeStructArray(long size);                              // Make an array of structures
      void MakeAndSetInteger(long);                                 // Allocate and set long variable		
		void MakeAndSetFloat(float);                                  // Allocate and set float variable from number
		void MakeAndSetDouble(double);                                // Allocate and set double float variable from number
		void MakeAndSetComplex(complex);                              // Allocate and set complex variable
		void MakeAndSetComplex(float,float);                          // Allocate and set complex variable
		void MakeAndSetString(const char* const);                     // Allocate and set string variable
		void MakeAndSetList(char**,short);                            // Allocate and set string list
		void MakeAndSet2DList(char***,short,short);                   // Allocate and set 2D string list
		void MakeList(short);                                         // Allocate string list
      void MakeAndSet2DList(short x, short y);                      // Allocate space for 2D list

		void MakeAndSetProcedure(char*,char*,char*,char*,long);		  // Allocate a procedure
      
		void MakeAndSetAlias(Variable*);                              // Allocate and set alias
      
      void MakeAndLoadMatrix2D(float**,long,long);                  // Allocate and set 2D float array
      void MakeAndLoadDMatrix2D(double**,long,long);                // Allocate and set 2D double array
      void MakeAndLoadMatrix3D(float***,long,long,long);            // Allocate and set 3D float array
      void MakeAndLoadMatrix4D(float ****,long,long,long,long);     // Allocate and set 4D float array
      void MakeAndLoadCMatrix2D(complex**,long,long);               // Allocate and set 2D complex float array
      void MakeAndLoadCMatrix3D(complex***,long,long,long);         // Allocate and set 3D complex float array
      void MakeAndLoadCMatrix4D(complex ****,long,long,long,long);  // Allocate and set 4D complex float array

      void MakeMatrix2DFromVector(float*, long,long);               // Convert a vector to a 2D real data set
      void MakeMatrix3DFromVector(float*, long,long,long);          // Convert a vector to a 3D real data set
      void MakeMatrix4DFromVector(float*, long,long,long,long);     // Convert a vector to a 4D real data set
      void MakeCMatrix2DFromCVector(complex*, long,long);           // Convert a vector to a 2D complex data set
      void MakeCMatrix3DFromCVector(complex*, long,long,long);      // Convert a vector to a 3D complex data set
      void MakeCMatrix4DFromCVector(complex*, long,long,long,long); // Convert a vector to a 4D complex data set

      void MakeDMatrix2DFromVector(double*, long, long);             // Convert a vector to a 2D double data set


      void SetReal(float in) {*((float*)data) = in;}                // Set float variable from number
      void SetFloat(float in) {*((float*)data) = in;}               // Set float variable from number
      void SetDouble(double in) {*((double*)data) = in;}            // Set double variable from number
      void SetComplex(complex in) {*((complex*)data) = in;}         // Set float variable from number

      void AssignString(char*);                                     // Assign String
      void AssignList(char**,long);                                 // Assign List character arrays
      void AssignStructure(Variable* struc);                        // Assign a structure
      void AssignMatrix2D(float**,long,long);                       // Assign 2D real float array
      void AssignDMatrix2D(double**,long,long);                     // Assign 2D real double array
      void AssignMatrix3D(float***,long,long,long);                 // Assign 3D real float array
      void AssignMatrix4D(float****,long,long,long,long);           // Assign 4D real float array
      void AssignCMatrix2D(complex**,long,long);                    // Assign 2D complex float array
      void AssignCMatrix3D(complex***,long,long,long);              // Assign 3D complex float array
      void AssignCMatrix4D(complex****,long,long,long,long);        // Assign 3D complex float array
      void AssignBreakPointList(char *bpInfo, long nrLines);        // Set a break point list
		void Assign2DList(List2DData *list);                          // Assign 2D list

      void AddToStructure(char* name, float data);                           // Add a float to a structure   
      void AddToStructure(char *name, double data);                          // Add a double to a structure
      void AddToStructure(char* name, char* txt);                            // Add a string to a structure   
      void AddToStructure(char* name, float** mat, int width, int height);   // Add a matrix to a structure   
      void AddToStructure(char* name, COLORREF color);                       // Add a color to a structure   

      Variable*   GetStructVariable(char *name);                    // Get a structure variable by name

    // Get variable functions ****************
      char*       GetData(void)      {return(data);}                // Get raw data pointer
      char**      GetDataAdrs(void)  {return(&data);}               // Get address of data pointer
      char*       GetString(void)    {return(data);}                // Get address of string
      double      GetDouble(void)    {return((double)*((double*)data));}    // Get real (double) number
      float       GetReal(void)      {return((float)*((float*)data));}     // Get real (float) number
      float       GetLong(void)      {return((float)*((long*)data));}      // Get long integer number
      char**      GetList(void)      {return((char**)data);}        // Get list variable
      List2DData* GetList2D(void)    {return((List2DData*)data);}       // Get 2D list variable
      Variable*   GetStruct(void)    {return((Variable*)data);}     // Get structure variable
      Variable*   GetClass(void)     {return((Variable*)data);}     // Get class variable
      complex     GetComplex(void)   {return(*((complex*)data));}   // Get complex number
      float**     GetMatrix2D(void)  {return((float**)data);}       // Get 2D real matrix
      double**    GetDMatrix2D(void)  {return((double**)data);}     // Get 2D double matrix
      complex**   GetCMatrix2D(void) {return((complex**)data);}     // Get 2D complex matrix
      float***    GetMatrix3D(void)  {return((float***)data);}      // Get 3D real matrix
      complex***  GetCMatrix3D(void) {return((complex***)data);}    // Get 3D complex matrix
      float****   GetMatrix4D(void)  {return((float****)data);}     // Get 4D real matrix
      complex**** GetCMatrix4D(void) {return((complex****)data);}   // Get 4D complex matrix

   // Extract cached procedure info and text stored in variable
      char* GetProcedureInfo(char*, char*, char*);                  // Get name and original location of procedure
      char* GetProcedureText(long &startLine);                      // Get procedure text

   // Copy variables
      short FullCopy(Variable*);                                     // Copy variable ignoring aliases
      short CopyWithAlias(Variable*);                                // Copy variable but keep aliases
      void Assign(Variable*);                                       // Assign a variable to this (no new storage allocated)

   // List functions
      char** CopyList(void);                                        // Copy the contents of a list
      short ReplaceListItem(char*, long);                           // Replace an item in a list
      short Replace2DListItem(char*, long, long);                   // Replace an item in a 2D list
      void AddToList(char *txt);                                    // Add a new item to the end of list
      Variable* GetNext(short&);                                    // Get next variable in list

   // Variable search commands
		Variable* Get(short,char[],short&);                           // Get a variable
		Variable* Get(char[],char*);                                  // Get an aliased variable
		Variable* Get(char[]);                                        // Get a variable given the name only

   // Linked list connections
		Variable *next;                                               // Next variable in list
		Variable *last;                                               // Last variable in list
		
   // Get and set variable dimensions
      long GetDimX(void) {return(dim1);}                            // Get the size of the 1st dimension (x)
      long GetDimY(void) {return(dim2);}                            // Get the size of the 2nd dimension (y)
      long GetDimZ(void) {return(dim3);}                            // Get the size of the 3rd dimension (z)
      long GetDimQ(void) {return(dim4);}                            // Get the size of the 4th dimension (?)

      void SetDim1(long d) {dim1 = d;}                              // Set the size of the 1st dimension (x)
      void SetDim2(long d) {dim2 = d;}                              // Set the size of the 2nd dimension (y)
      void SetDim3(long d) {dim3 = d;}                              // Set the size of the 3rd dimension (z)
      void SetDim4(long d) {dim4 = d;}                              // Set the size of the 4th dimension (?)

   // Get and set variable parameters
      char* GetName(void)         {return(name);}                   // Get the variable identifier
      short GetType(void)         {return(type);}                   // Get the type of variable (STRING, FLOAT32 ...)
      bool  GetSystem(void)       {return(system);}                 // Is it a system variable?
      bool  GetPermanent(void)    {return(permanent);}              // Is the variable permanent?
      bool  GetReadOnly(void)     {return(readOnly);}               // Is the variable read only?
      bool  GetVisible(void)      {return(visible);}                // Is the variable visible or hidden?
      short GetScope(void)        {return(scope);}                  // Does the variable have global, local or window scope?
      Variable* GetAlias(void)    {return(aliasVar);}               // Is this variable an alias to another?
      short GetAliasCnt(void)     {return(aliasRef);}               // How many variables are aliased to this one?
      Variable* GetLast(void);                                      // Get last pointer in linked list
      void SetName(char* n);                                        // Set the variable name
      void FreeName();                                              // Free and zero name memory
      void SetType(short t)       {type = t;}                       // Set the variable type (STRING, FLOAT32 ...)
      void SetPermanent(bool p)   {permanent = p;}                  // Set the permanency
      void SetSystem(bool p)      {system = p;}                     // Set whether its a system variable
      void SetReadOnly(bool r)    {readOnly = r;}                   // Set the read/write status
      void SetVisible(bool v)     {visible = v;}                    // Set the visibility status
      void SetScope(short s)      {scope = s;}                      // Set the variables scope
      void SetAlias(Variable* v)  {aliasVar = v;}                   // Define an alias for this variable

  // Memory management
      void  RemoveAll(void);                                         // Remove all variables linked to this one
      void  Remove(void);                                            // Remove one variable
		short FreeData(void);                                          // Free up memory used by variable data
      void  SetNull(void);                                           // Null data without freeing memory		

  // Other
      short Count(void);                                             // Count number of variable in a linked list

      // Internal member variables - not accessable outside class
private:
		char *data;                                                   // Pointer to variable data
      long dim1;                                                    // Number of points in 1st (x) dimension
		long dim2;                                                    // Number of bytes in 2nd  (y) dimension
		long dim3;                                                    // Number of bytes in 3rd  (z) dimension	
		long dim4;                                                    // Number of bytes in 4th  (?) dimension	
      char *name;                                                   // Name of variable
		short type;		                                               // variable type i.e. FLOAT32, QUOTED_STRING ...
		bool system;                                                  // Is variable only used internally?	
		bool permanent;                                               // Is variable permanent?	
		bool readOnly;                                                // Is variable readOnly?	
		bool visible;                                                 // Is variable hidden?	
		short scope;                                                  // Variable scope i.e. LOCAL, GLOBAL, WINDOW
		long size;                                                    // Number of bytes per data point (4/8 usually)
		Variable *aliasVar;                                           // Address of alias variable
		short aliasRef;                                               // Number of aliases which reference this variable
};

#endif //  define VARIABLESCLASS_H