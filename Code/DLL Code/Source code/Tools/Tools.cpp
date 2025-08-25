
#include "../Global files/includesDLL.h"

//#include <atlcomcli.h>



// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short SquareFunction(DLLParameters*,char *args);
short RootFunction(DLLParameters*,char *args);
short CartesianToPolar(DLLParameters*,char *args);
short PolarToCartesian(DLLParameters*,char *args);
short OpenFile(DLLParameters* par, char *args);
short CloseFile(DLLParameters* par, char *args);
short ReadALine(DLLParameters* par, char *args);
short testspeed(DLLParameters *par, char *args);
short StringToNumbers(DLLParameters* par, char *args);


FILE *gFile = NULL;

char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list

/*******************************************************************************
Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
  short r = RETURN_FROM_DLL;


  if(!strcmp(command,"square"))           r = SquareFunction(dpar,parameters);      
  else if(!strcmp(command,"root"))        r = RootFunction(dpar,parameters);      
  else if(!strcmp(command,"ctop"))        r = CartesianToPolar(dpar,parameters);      
  else if(!strcmp(command,"ptoc"))        r = PolarToCartesian(dpar,parameters);  
  else if(!strcmp(command,"openfile"))    r = OpenFile(dpar,parameters);
  else if(!strcmp(command,"closefile"))   r = CloseFile(dpar,parameters);
  else if(!strcmp(command,"readline"))    r = ReadALine(dpar,parameters);
  else if(!strcmp(command,"testspeed"))    r = testspeed(dpar,parameters);
  else if(!strcmp(command,"strtoarray"))    r = StringToNumbers(dpar,parameters);

 
  return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
  TextMessage("\n\n   Tools DLL module\n\n");
  TextMessage("   asciitostr ..... converts ascii number in an array to a string\n");
  TextMessage("   ctop ........... converts Cartesian coordinates to polar\n");
  TextMessage("   openfile ....... open a file for reading or writing\n");
  TextMessage("   closefile ...... close an open file\n");
  TextMessage("   readline ....... read a line from an ascii file\n");
  TextMessage("   ptoc ........... converts polar coordinates to Cartesian\n");
  TextMessage("   square ......... squares argument\n");
  TextMessage("   strtoascii ..... convert a string to an array of ascii equivalents\n");
  TextMessage("   strtoarray ..... converts an numeric string to a numeric array with byte swapping\n");
  TextMessage("   root ........... takes square root of argument\n");
}

/*******************************************************************************
Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
  syntax[0] = '\0';
  if(!strcmp(cmd,"square"))           strcpy(syntax,"NUM y = square(NUM x)");
  else if(!strcmp(cmd,"openfile"))    strcpy(syntax,"openfile(fileName, mode");
  else if(!strcmp(cmd,"closefile"))   strcpy(syntax,"closefile()");
  else if(!strcmp(cmd,"readline"))    strcpy(syntax,"STR line = readline()");
  else if(!strcmp(cmd,"root"))        strcpy(syntax,"NUM y = root(NUM x)");
  else if(!strcmp(cmd,"ctop"))        strcpy(syntax,"(FLOAT r,FLOAT t) = ctop(FLOAT x, FLOAT y)");
  else if(!strcmp(cmd,"ptoc"))        strcpy(syntax,"(FLOAT x,FLOAT y) = ptoc(FLOAT r, FLOAT t)");
  else if(!strcmp(cmd,"asciitostr"))  strcpy(syntax,"STR r = asciitostr(VEC a/ FLOAT a)");
  else if(!strcmp(cmd,"strtoascii"))  strcpy(syntax,"VEC r = stringtoascii(STR a)");
  else if(!strcmp(cmd,"strtoarray"))  strcpy(syntax,"VEC r = strtoarray(VEC a, STR mode, INT nibbles_per_block, STR nibble_order, INT words_per_block ");

  if(syntax[0] == '\0')
    return(false);
  return(true);
}

/****************************************************************************
   Some example procedures to open and close a file and read a line of text
   Here is a test macro

   cd("E:\\")
   openfile("test.txt")
   txt = ""
   while(1)
      line = readline()
      if(line == null)
        exitwhile
      endif
      txt = txt + line[:-2] + " "
   endwhile
   closefile()

   pr txt
*****************************************************************************/

short OpenFile(DLLParameters* par, char *args)
{
  short nrArgs;
  CText mode="r";
  CText fileName;

  // Argument can be a constant float or matrix or a variable containing these
  if((nrArgs = ArgScan(par->itfc,args,1,"data to be opened, mode","ee","tt",&fileName,&mode)) < 0)
    return(nrArgs); 

  if(gFile = fopen(fileName.Str(),mode.Str()))
     return(OK);

  ErrorMessage("unable to option file %s",fileName.Str());

  return(ERR);
}
    

short CloseFile(DLLParameters* par, char *args)
{
  if(gFile)
  {
     fclose(gFile);
     gFile = NULL;
     return(OK);
  }

  ErrorMessage("no open file");

  return(ERR);
}


short ReadALine(DLLParameters* par, char *args)
{
  char str[100];
  char *err;
  CText line;
  int sz;

  
  for(;;)
  {
     err = fgets(str,100,gFile);
     if(err == NULL)
     {
         par->retVar[1].MakeNullVar();
         par->nrRetVar = 1;
         return(OK);
     }
    
     line.Concat(str);
     sz = line.Size();
     if(line[sz-1] == '\n')
        break;

  }

   par->retVar[1].MakeAndSetString(line.Str());
   par->nrRetVar = 1;

  return(OK);
}

short StringToNumbers(DLLParameters* par, char *args)
{
  short nrArgs;
  CText txt,format = "%f";
  long step;
  long nr_per_step = 1;
  CText order;

  // Argument can be a constant float or matrix or a variable containing these
  if((nrArgs = ArgScan(par->itfc,args,5,"string ,format, step, byte, order, numbers_per_step","eeeee","ttltl",&txt,&format,&step,&order,&nr_per_step)) < 0)
    return(nrArgs); 

  char *cstr = txt.Str();
  int del;
  int substep = step/nr_per_step;
  long sz = txt.Size()/step;
  long sz2 = txt.Size()/substep;


  if(format == "x" || format == "d")
  {
     char cformat[5] = "%";
     strncat(cformat,format.Str(),1);
     float* out = new float[sz2];
     char *temp = new char[step+1];
     char *ordered = new char[step+1];
     long data;
     long cnt = 0;
     for(int i = 0; i < sz; i++)
     {
        strncpy(temp,&cstr[i*step],step);
        temp[step] = '\0';
        int j;
        for(j = 0; j < step; j++)
        {
           ordered[j] = temp[(int)order[j] - (int)'0' ];
        }
        ordered[j] = '\0';
        for(j = 0; j < nr_per_step; j++)
        {
           strncpy(temp,&ordered[substep*j],substep);
           temp[substep] = '\0';
           sscanf(temp,cformat,&data);
           out[cnt++] = (float)data;
        }
     }

     par->retVar[1].MakeMatrix2DFromVector(out,1,sz2);
     par->nrRetVar = 1;

     delete [] out;
     delete [] temp;
     return(OK);
  }

  ErrorMessage("Invalid number format (x,d)");
  return(ERR);
}
  

/*********************************************************************************
This function will square the passed argument if it is a scalar or a matrix
The result is returned in the global variable ansVar. 
*********************************************************************************/

short SquareFunction(DLLParameters* par, char *args)
{
  short nrArgs;
  Variable var;
  short type;
  complex cIn,cOut;

  // Argument can be a constant float or matrix or a variable containing these
  if((nrArgs = ArgScan(par->itfc,args,1,"data to be squared","e","v",&var)) < 0)
    return(nrArgs);  

  // See if its a float variable   
  switch(VarType(&var))
  {
     case(FLOAT32): 

       par->retVar[1].MakeAndSetFloat(VarReal(&var)*VarReal(&var));
       break;

     case(COMPLEX): 

       cIn = VarComplex(&var);
       cOut.r = cIn.r*cIn.r - cIn.i*cIn.i;
       cOut.i = 2*cIn.r*cIn.i;
       par->retVar[1].MakeAndSetComplex(cOut);
       break;

     case(MATRIX2D):
       {
         // Get some information from the variable         
         long rows = VarRowSize(&var);
         long cols = VarColSize(&var);
         float** arrayIn = VarRealMatrix(&var);

         // Allocate space for the output matrix   
         float** arrayOut = MakeMatrix2D(cols,rows);

         // Square input matrix and store in output matrix      
         for(short i = 0; i < rows; i++)
           for(short j = 0; j < cols; j++)
             arrayOut[i][j] = arrayIn[i][j]*arrayIn[i][j];

         // Return result to the user
         par->retVar[1].MakeAndLoadMatrix2D(arrayOut,cols,rows);
         FreeMatrix2D(arrayOut);
         break;
       }   
     default:

       ErrorMessage("Argument to 'square' should be a float, complex number or a real matrix");
       return(ERR);
  } 

  par->nrRetVar = 1;

  return(OK);
}


/*********************************************************************************
This function will take the square root of the passed argument if it is 
a scalar or a matrix. The result is returned in the global variable ansVar. 
*********************************************************************************/

short RootFunction(DLLParameters* par, char *args)
{
  short nrArgs;
  Variable var;
  short type;

  // Argument can be a constant float or matrix or a variable containing these      
  if((nrArgs = ArgScan(par->itfc,args,1,"data to be rooted","e","v",&var)) < 0)
    return(nrArgs);  

  // See if its a float variable   
  switch(VarType(&var))
  {
     case(FLOAT32): 
     {
        par->retVar[1].MakeAndSetFloat(sqrt(VarReal(&var)));
        break;
     }

     case(MATRIX2D):
     {
         // Get some information from the variable
         long rows = VarRowSize(&var);
         long cols = VarColSize(&var);
         float** arrayIn = VarRealMatrix(&var);

         // Allocate space for the output matrix   
         float** arrayOut = MakeMatrix2D(cols,rows);

         // Square input matrix and store in output matrix		
         for(short i = 0; i < rows; i++)
           for(short j = 0; j < cols; j++)
             arrayOut[i][j] = sqrt(arrayIn[i][j]);

         // Return result to the user
         par->retVar[1].MakeAndLoadMatrix2D(arrayOut,cols,rows);
         FreeMatrix2D(arrayOut);
         break;
     }   
     default:

    ErrorMessage("Argument to 'root' should be a float or a real matrix");
    return(ERR);
  }

  par->nrRetVar = 1;

  return(OK);  	 
}


/*********************************************************************************
This procedure converted a number in cartesian coordinates (x,y) 
into polar coordinates (r,theta)
*********************************************************************************/

short CartesianToPolar(DLLParameters* par, char *args)
{
  short nrArgs;
  float x,y;
  float mag,theta;

  // Extract arguments
  if((nrArgs = ArgScan(par->itfc,args,2,"x,y","ee","ff",&x,&y)) < 0)
    return(nrArgs);  

  // Calculate polar
  mag = sqrt(x*x + y*y);
  theta = atan(y/x);

  // Return result to Prospa
  par->retVar[1].MakeAndSetFloat(mag);
  par->retVar[2].MakeAndSetFloat(theta);
  par->nrRetVar = 2;

  return(OK);
}


/*********************************************************************************
This procedure converted a number in polar coordinates (r,theta) 
into Cartesian coordinates (x,y)
*********************************************************************************/

short PolarToCartesian(DLLParameters* par, char *args)
{
  short nrArgs;
  float x,y;
  float mag,theta;

  // Extract arguments
  if((nrArgs = ArgScan(par->itfc,args,2,"mag,theta","ee","ff",&mag,&theta)) < 0)
    return(nrArgs);  

  // Calculate Cartesian
  x = mag*cos(theta);
  y = mag*sin(theta);

  // Return result to Prospa
  // Fill the answer matrix with this array and return 
  par->retVar[1].MakeAndSetFloat(x);
  par->retVar[2].MakeAndSetFloat(y);
  par->nrRetVar = 2;

  return(OK);
}

short testspeed(DLLParameters *par, char *args)
{
   double w;
   int N = 1e7;
   double **y = MakeDMatrix2D(N,1);
   for(int q = 0; q < N; q++)
   {
      w = q*0.0001;
      y[0][q] = sin(w);
   }
   par->retVar[1].AssignDMatrix2D(y,N,1);
   par->nrRetVar = 1;
   return(OK);
}
