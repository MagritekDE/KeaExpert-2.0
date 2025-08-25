#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include "stdlib.h"

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
short FilterBackProjection(DLLParameters*,char arg[]);
short RtoXY(DLLParameters*,char arg[]);
long nint(float num);
int HelpFolder(DLLParameters*,char*);


// Extension procedure to add commands to Prospa 

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

   if(!strcmp(command,"fbp"))             r = FilterBackProjection(dpar, parameters);      
   else if(!strcmp(command,"rtoxy"))      r = RtoXY(dpar, parameters);      
   else if(!strcmp(command,"helpfolder")) r = HelpFolder(dpar,parameters);   
       
   return(r);
}

// Extension procedure to list commands in DLL 

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   FBP DLL module (V2.00)\n\n");
   TextMessage("   fbp ... apply filtered back project to a file\n");  
   TextMessage("   rtoxy ... radial to matrix copy\n");  
}

/*******************************************************************************
   Extension procedure to return syntax in DLL 
*******************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';

   if(!strcmp(cmd,"fbp"))  strcpy(syntax,"MAT image = fbp(MAT image, VEC projection, FLOAT angle)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

int HelpFolder(DLLParameters* par,char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\FBP");
   par->nrRetVar = 1;
   return(OK);
}


short RtoXY(DLLParameters *par, char arg[])
{
   short nrArgs;
   long x,y;
   float r;
   Variable vecVar;
   long right,left;
   float dleft,dright,dinterp;

// Get radial vector 
   if((nrArgs = ArgScan(par->itfc, arg,1,"vector","e","v",&vecVar)) < 0)
     return(nrArgs); 

// Check type and size of vector   
   if(VarType(&vecVar) != MATRIX2D || VarHeight(&vecVar) != 1)
   {
      ErrorMessage("argument should be a 2D matrix");
      return(ERR);
   }

// Make the matrix
   long size = VarWidth(&vecVar)*2;
   par->retVar[1].MakeAndLoadMatrix2D((float**)-1,size,size);
   float **mat = par->retVar[1].GetMatrix2D();
   float *vec = vecVar.GetMatrix2D()[0];

// Add the data
   for(y = 0; y < size; y++)
   {
      for(x = 0; x < size; x++)
      {
         r = sqrt(float((x-size/2)*(x-size/2)+(y-size/2)*(y-size/2)));
         if(r < size/2-1)
         {
            left = int(r);
            right = left+1;
            dleft = vec[left];
            dright = vec[right];
            dinterp = (dright-dleft)*(r-left)/float(right-left) + dleft;
            mat[y][x] = dinterp;
         }
         else
         {
            mat[y][x] = 0.0;
         }
      }
   }

   par->nrRetVar = 1;

   return(OK);
}


short FilterBackProjection(DLLParameters* par, char arg[])
{
   short nrArgs;
   float angle;
   long size;
   long x,y,k;
   float cosa,sina;
   complex *data;
   Variable matVar,vecVar;
   float **mat;
   float **vec;
   short type;

// Get projection angle 
   if((nrArgs = ArgScan(par->itfc,arg,3,"matrix, vector, angle","eee","vvf",&matVar,&vecVar,&angle)) < 0)
     return(nrArgs); 

// Check for valid matrix and vector dimensions
   if(VarType(&matVar) != MATRIX2D || !(VarRowSize(&matVar) > 1 && VarColSize(&matVar) > 1))
   {
      ErrorMessage("first argument should be a 2D matrix");
      return(ERR);
   } 
   
   if(VarType(&vecVar) != MATRIX2D || VarRowSize(&vecVar) != 1)
   {
      ErrorMessage("second argument should be a 2D matrix");
      return(ERR);
   }

   if(VarRowSize(&matVar) != VarColSize(&matVar))
   {
      ErrorMessage("matrix should be square");
      return(ERR);
   }

   if(VarColSize(&vecVar) != VarColSize(&matVar))
   {
      ErrorMessage("vector should have same number of rows as matrix");
      return(ERR);
   }

// Get pointers to input vector and output matrix & the data dimension
   mat = VarRealMatrix(&matVar);
   vec = VarRealMatrix(&vecVar);
   if(!mat || !vec)
   {
      ErrorMessage("matrix or vector have no data");
      return(ERR);
   }
   size = VarColSize(&matVar);
   
// Make temps for fast calculation
   cosa = cos(angle);
   sina = sin(angle);
      
// Add to each point in the 2D data set the projection value 
   for(y = 0; y < size; y++)
   {
      for(x = 0; x < size; x++)
      {
         k = nint((x-size/2)*cosa+(y-size/2)*sina) + size/2;
         if(k < size && k > 0)
            mat[y][x] += vec[0][k];
      }
   }
   
	par->retVar[1].MakeAndLoadMatrix2D(mat,size,size);
   par->nrRetVar = 1;

   return(OK);
}


/*****************************************************************************************
*                           Return the nearest integer to the float num                  *
*****************************************************************************************/

long nint(float num)
{
   if(num > 0)
      return((long)(num+0.5));
   else
      return((long)(num-0.5));
   
}

