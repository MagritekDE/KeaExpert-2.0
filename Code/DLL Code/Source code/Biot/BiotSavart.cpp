#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <process.h>

#define VERSION 2.0

#define ABORT -5

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short BiotSavart(DLLParameters*,char *arg);
short BiotSavartGradient(DLLParameters*,char *arg);
long nint(float num);
short HelpFolder(DLLParameters*,char *args);


/*******************************************************************************
    Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

   if(!strcmp(command,"fieldcalc"))         r = BiotSavart(dpar,parameters);                         
   else if(!strcmp(command,"gradcalc"))     r = BiotSavartGradient(dpar,parameters);                         
   else if(!strcmp(command,"helpfolder"))   r = HelpFolder(dpar,parameters);   

   return(r);
}

/*******************************************************************************
    Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   BiotSavart Module (V2.00)\n\n");
   TextMessage("   fieldcalc ... calculate B field from wire file using biot-savart\n");
   TextMessage("   gradcalc ... calculate B gradient from wire file using biot-savart\n");
}

/*******************************************************************************
    Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"fieldcalc"))  strcpy(syntax,"(MAT bx, by, bz) = fieldcalc(MAT wire_info)");
   if(!strcmp(cmd,"gradcalc"))  strcpy(syntax,"MAT gx, gy, gz) = fieldcalc(MAT wire_info)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short HelpFolder(DLLParameters* par,char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\Biot");
   par->nrRetVar = 1;
   return(OK);
}

short BiotSavart(DLLParameters* par, char *arg)
{
// Some variables
   float dlx,dly,dlz;
   float drx,dry,drz;
   float dBx,dBy,dBz;
   float R;
   long xi,yi,zi,j;
   float x,y,z;
   float I = 1;
   float ***Bx, ***By, ***Bz;
   CText display = "yes";

// Load the wire data. Each row contains the end coords of a wire segment and the current in this segment
   Variable wireVar,xVar,yVar,zVar;
   short r;

   if((r = ArgScan(par->itfc,arg,3,"wire-info, x-range, y-range, z-range","eeeee","vvvvt",&wireVar,&xVar,&yVar,&zVar,&display)) < 0)
      return(r); 

// Check for errors
   if(VarType(&wireVar) != MATRIX2D || VarWidth(&wireVar) != 6 || VarHeight(&wireVar) < 1)
   {
      ErrorMessage("wire matrix is invalid");
      return(ERR);
   }

   if(VarType(&xVar) != MATRIX2D || VarWidth(&xVar) < 1 || VarHeight(&xVar) != 1)
   {
      ErrorMessage("x-range matrix is invalid");
      return(ERR);
   }
   if(VarType(&yVar) != MATRIX2D || VarWidth(&yVar) < 1 || VarHeight(&yVar) != 1)
   {
      ErrorMessage("y-range matrix is invalid");
      return(ERR);
   }
   if(VarType(&zVar) != MATRIX2D || VarWidth(&zVar) < 1 || VarHeight(&zVar) != 1)
   {
      ErrorMessage("z-range matrix is invalid");
      return(ERR);
   }

// Get dimensions
   long sx = VarWidth(&xVar);
   float **xM = VarRealMatrix(&xVar);
   long sy = VarWidth(&yVar);
   float **yM = VarRealMatrix(&yVar);
   long sz = VarWidth(&zVar);
   float **zM = VarRealMatrix(&zVar);

   float **wire = VarRealMatrix(&wireVar);
   long nrseg = VarHeight(&wireVar);

// Allocate space for field matrix
   if(!(Bx = MakeMatrix3D(sx,sy,sz)))
   {
      ErrorMessage("insufficient memory for Bx matrix");
      return(ERR);
   }
   if(!(By = MakeMatrix3D(sx,sy,sz)))
   {
      FreeMatrix3D(Bx);
      ErrorMessage("insufficient memory for By matrix");
      return(ERR);
   }
   if(!(Bz = MakeMatrix3D(sx,sy,sz)))
   {
      FreeMatrix3D(Bx);
      FreeMatrix3D(By);
      ErrorMessage("insufficient memory for Bz matrix");
      return(ERR);
   }

   for(zi = 0; zi < sz; zi++)
   {  
      z = zM[0][zi];

      for(yi = 0; yi < sy; yi++)
      {
         if(ProcessBackgroundEvents() != OK)
            return(ABORT);

         y = yM[0][yi];

         for(xi = 0; xi < sx; xi++)  // Step along z axis
         {
            x = xM[0][xi];

            Bx[zi][yi][xi] = 0;
            By[zi][yi][xi] = 0;
            Bz[zi][yi][xi] = 0;
            for(j = 0; j < nrseg; j++) // Loop over coil segments
            {
               dlx = wire[j][0]-wire[j][1];
               dly = wire[j][2]-wire[j][3];
               dlz = wire[j][4]-wire[j][5];
               drx = x - 0.5*(wire[j][0]+wire[j][1]);
               dry = y - 0.5*(wire[j][2]+wire[j][3]);
               drz = z - 0.5*(wire[j][4]+wire[j][5]);
               R = sqrt(drx*drx+dry*dry+drz*drz);
               dBx = (dly*drz-dlz*dry)/(R*R*R);
               dBy = (dlz*drx-dlx*drz)/(R*R*R);
               dBz = (dlx*dry-dly*drx)/(R*R*R);
               Bx[zi][yi][xi] += I*1e-7*dBx;
               By[zi][yi][xi] += I*1e-7*dBy;
               Bz[zi][yi][xi] += I*1e-7*dBz;
            }
         }
      }
      if(display == "display")
         TextMessage("\n   zi = %ld of %ld",zi,sz);
   }

// Return field
   par->retVar[1].AssignMatrix3D(Bx, sx, sy, sz);
   par->retVar[2].AssignMatrix3D(By, sx, sy, sz);
   par->retVar[3].AssignMatrix3D(Bz, sx, sy, sz);
   par->nrRetVar = 3;

   return(OK);
}

short BiotSavartGradient(DLLParameters* par, char *arg)
{
// Some variables
   float dlx,dly,dlz;
   float drx,dry,drz;
   float dBzdx,dBzdy,dBzdz;
   float R;
   long xi,yi,zi,j;
   float x,y,z;
   float I = 1;
   float ***Bx, ***By, ***Bz;
   char direction[50];

// Load the wire data. Each row contains the end coords of a wire segment and the current in this segment
   Variable wireVar,xVar,yVar,zVar;
   short r;

   if((r = ArgScan(par->itfc,arg,3,"wire-info, x-range, y-range, z-range","eeee","vvvv",&wireVar,&xVar,&yVar,&zVar)) < 0)
      return(r); 

// Check for errors
   if(VarType(&wireVar) != MATRIX2D || VarWidth(&wireVar) != 6 || VarHeight(&wireVar) < 1)
   {
      ErrorMessage("wire matrix is invalid");
      return(ERR);
   }

   if(VarType(&xVar) != MATRIX2D || VarWidth(&xVar) < 1 || VarHeight(&xVar) != 1)
   {
      ErrorMessage("x-range matrix is invalid");
      return(ERR);
   }
   if(VarType(&yVar) != MATRIX2D || VarWidth(&yVar) < 1 || VarHeight(&yVar) != 1)
   {
      ErrorMessage("y-range matrix is invalid");
      return(ERR);
   }
   if(VarType(&zVar) != MATRIX2D || VarWidth(&zVar) < 1 || VarHeight(&zVar) != 1)
   {
      ErrorMessage("z-range matrix is invalid");
      return(ERR);
   }

// Get dimensions
   long sx = VarWidth(&xVar);
   float **xM = VarRealMatrix(&xVar);
   long sy = VarWidth(&yVar);
   float **yM = VarRealMatrix(&yVar);
   long sz = VarWidth(&zVar);
   float **zM = VarRealMatrix(&zVar);

   float **wire = VarRealMatrix(&wireVar);
   long nrseg = VarHeight(&wireVar);


// Allocate space for field matrix
   if(!(Bx = MakeMatrix3D(sx,sy,sz)))
   {
      ErrorMessage("insufficient memory for Bx matrix");
      return(ERR);
   }
   if(!(By = MakeMatrix3D(sx,sy,sz)))
   {
      FreeMatrix3D(Bx);
      ErrorMessage("insufficient memory for By matrix");
      return(ERR);
   }
   if(!(Bz = MakeMatrix3D(sx,sy,sz)))
   {
      FreeMatrix3D(Bx);
      FreeMatrix3D(By);
      ErrorMessage("insufficient memory for Bz matrix");
      return(ERR);
   }

   for(zi = 0; zi < sz; zi++)
   {  

      z = zM[0][zi];

      for(yi = 0; yi < sy; yi++)
      {
         if(ProcessBackgroundEvents() != OK)
            return(ABORT);

         y = yM[0][yi];

         for(xi = 0; xi < sx; xi++)  // Step along z axis
         {
            x = xM[0][xi];

            Bx[zi][yi][xi] = 0;
            By[zi][yi][xi] = 0;
            Bz[zi][yi][xi] = 0;
            for(j = 0; j < nrseg; j++) // Loop over coil segments
            {
               dlx = wire[j][0]-wire[j][1];
               dly = wire[j][2]-wire[j][3];
               dlz = wire[j][4]-wire[j][5];
               drx = x - 0.5*(wire[j][0]+wire[j][1]);
               dry = y - 0.5*(wire[j][2]+wire[j][3]);
               drz = z - 0.5*(wire[j][4]+wire[j][5]);
               R = sqrt(drx*drx+dry*dry+drz*drz);
            //   dBx = (dly*drz-dlz*dry)/(R*R*R);
           //    dBy = (dlz*drx-dlx*drz)/(R*R*R);
               dBzdz = -3*drz*(dlx*dry-dly*drx)/(R*R*R*R*R);
               dBzdx = -dly/(R*R*R)+dBzdz;
               dBzdy = dlx/(R*R*R)+dBzdz;

               Bx[zi][yi][xi] += I*1e-7*dBzdx;
               By[zi][yi][xi] += I*1e-7*dBzdy;
               Bz[zi][yi][xi] += I*1e-7*dBzdz;
            }
         }
      }
      TextMessage("\n   zi = %ld of %ld",zi,sz);
   }

// Return field
   par->retVar[1].AssignMatrix3D(Bx, sx, sy, sz);
   par->retVar[2].AssignMatrix3D(By, sx, sy, sz);
   par->retVar[3].AssignMatrix3D(Bz, sx, sy, sz);
   par->nrRetVar = 3;

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
