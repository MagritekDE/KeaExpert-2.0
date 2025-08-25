#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include "stdlib.h"




short LawsonHansonNNLS(char arg[]);
short Soften(char arg[]);

short NNLS(float *weighting,double* spectrum, float *Tau,float Time[],float Data[],long x_dim,long y_dim) ;
void DeleteFromSet(long *setp,long *setz,long index,long x_dim);
void FindLamda(double *lamda,double *spectrum,double *Soln,long *setp,long *zeroindex, long x_dim);
void FindLamda(float *lamda,float *spectrum,float *Soln,long *setp,long *zeroindex, long x_dim);
void LinearEqnSolve(double **Systemmat,long x_dim,long *setp,double *ans);
void LinearEqnSolve(float **Systemmat,long x_dim,long *setp,float *ans);
void Transpose(double **mat,long x_dim,long y_dim,double **ans);
void Transpose(float **mat,long x_dim,long y_dim,float **ans);
void Matmult(double **mat,long x_dim,long y_dim,double *vec,double *temp);
void Matmult(float **mat,long x_dim,long y_dim,float *vec,float *temp);
short QRdecmp(double **mat,long *setp,long *oldsetp,long x_dim,long y_dim);
short QRdecmp(float **mat,long *setp,long *oldsetp,long x_dim,long y_dim);
void Orthomult(double **mat,double *u,double *b,long x_dim,long y_dim);
void Orthomult(float **mat,float *u,float *b,long x_dim,long y_dim);
void House(long column,long pivot,double **mat,long y_dim,double *u,double *b);
void House(long column,long pivot,float **mat,long y_dim,float *u,float *b);
void Soften(float *slope, float *softslope, long x_dim);

void printmd(double **mat,long x_dim,long y_dim);
void printm(float **mat,long x_dim,long y_dim);
void printvf(float *vec,long y_dim);
void printvi(long *vec,long y_dim);
void printvd(double *vec, long y_dim);


void PlotTestData(double *x, float *y, long N);
void PlotTestData(double *y, long N);

void CopyVectorToDouble(Variable* var, double *v);
void CopyMatrixToDouble(Variable* var, double **m);
void CopyDoubleVectorToFloat(double* in, float* out, long len);


/**********************************************************
  Interface to NNLS algorithm
**********************************************************/

short LawsonHansonNNLS(DLLParameters* par, char arg[])
{
	char str[100];
	short nrArgs;
   short r;
   Variable vRegularization,vSpectrum,vKernel,vData;
   long index;
   long x_dim,y_dim;
   long i,j;
   double biggest;
   double lamda;
   long indextozero,newindex;
   short repeat;
   CText callBack = "";
   bool memoryError = true;
   bool algorithmFault = false;
   double *spectrum = NULL;
   double *data = NULL;
   double **systemmat = NULL;
   double **designmmat = NULL;
   double **designmmatT = NULL;
   double *ansW = NULL;
   double* ans = NULL;
   double *W = NULL;
   double* Soln = NULL;
   long*  setp = NULL;
   long*  setz = NULL;
   long*  oldsetp = NULL;
   float *spectrumf = NULL;

// Get user parameters 
   if((nrArgs = ArgScan(par->itfc, arg,3,"dataVec, kernelMat, regularization, callBack","eeee","vvvt",&vData,&vKernel,&vRegularization,&callBack)) < 0)
      return(nrArgs);

   if((VarType(&vData) != MATRIX2D || VarType(&vKernel) != MATRIX2D) || (VarType(&vRegularization) != MATRIX2D && VarType(&vRegularization) != FLOAT32))
   {
      ErrorMessage("Invalid input data");
      return(ERR);
   }

   x_dim = VarWidth(&vKernel);
   y_dim = VarWidth(&vData);

   if(VarHeight(&vKernel) != y_dim)
   {
      ErrorMessage("Invalid kernel matrix height");
      return(ERR);
   }

   if(VarType(&vRegularization) == MATRIX2D)
   {
      if(VarWidth(&vRegularization) != x_dim || (VarHeight(&vRegularization) != x_dim) && (VarHeight(&vRegularization) != 1))
      {
         ErrorMessage("Invalid regularization matrix dimension");
         return(ERR);
      }
   }
   
// Assume a memory error might occur
   memoryError = true;


// Copy input variable to local double precision matrices
   if(!(data = MakeDVectorNR(1,y_dim)))
      goto abort;
   CopyVectorToDouble(&vData,data);

// Make system matrix and design matrix
   if(!(systemmat = MakeDMatrix2DNR(1,y_dim+x_dim,1,x_dim+1)))
      goto abort;

   if(!(designmmat = MakeDMatrix2DNR(1,y_dim+x_dim,1,x_dim)))
      goto abort;

   for(j = 1; j <= y_dim; j++)
      for(i = 1; i <= x_dim; i++)
         designmmat[j][i] = systemmat[j][i] = vKernel.GetMatrix2D()[j-1][i-1];

   if(VarType(&vRegularization) == MATRIX2D)
   {
      if(VarHeight(&vRegularization) == x_dim) // Regularization is a square matrix
      {
         for(j = y_dim+1; j <= x_dim+y_dim; j++)
            for(i = 1; i <= x_dim; i++)
               designmmat[j][i] =  systemmat[j][i] = vRegularization.GetMatrix2D()[j-y_dim-1][i-1];
      }
      else
      {
         for(j = y_dim+1; j <= x_dim+y_dim; j++)
            for(i = 1; i <= x_dim; i++)
               designmmat[j][i] = systemmat[j][i] = 0;
            for(i = 1; i <= x_dim; i++)
               designmmat[y_dim+i][i] =  systemmat[y_dim+i][i] = vRegularization.GetMatrix2D()[0][i-1];
      }
   }
   else
   {
      for(j = y_dim+1; j <= x_dim+y_dim; j++) // Regularization is number
      {
         for(i = 1; i <= x_dim; i++)
         {
            if(i == j-y_dim)
               designmmat[j][i] =  systemmat[j][i] = vRegularization.GetReal();
            else
               designmmat[j][i] =  systemmat[j][i] = 0.0;
         }
      }
   }

   for(j = 1; j <= y_dim; j++)
      systemmat[j][x_dim+1] = data[j];
   for(j = y_dim+1; j <= x_dim+y_dim; j++)
      systemmat[j][x_dim+1] = 0;

// Make a zeroed spectrum
   if(!(spectrum = MakeDVectorNR(1,x_dim)))
      goto abort;
   for(i = 1; i <= x_dim; i++)
      spectrum[i] = 0;

   if(!(ansW = MakeDVectorNR(1,x_dim+y_dim)))
      goto abort;
   for(j = 1; j <= y_dim; j++)
      ansW[j] = data[j];
   for(j = y_dim+1; j <= x_dim+y_dim; j++)
      ansW[j] = 0;

	if(!(designmmatT = MakeDMatrix2DNR(1,x_dim,1,y_dim+x_dim)))
      goto abort;
	Transpose(designmmat,x_dim,y_dim+x_dim,designmmatT);

   if(!(W = MakeDVectorNR(1,x_dim)))
      goto abort;

   Matmult(designmmatT, x_dim+y_dim, x_dim, ansW, W);

	if(!(ans = MakeDVectorNR(1,y_dim+x_dim)))
      goto abort;

	for(i = 1; i <= y_dim+x_dim; i++)
      ans[i] = 0;

	if(!(Soln     = MakeDVectorNR(1,x_dim)))
      goto abort;
	if(!(setp    = MakeIVectorNR(1,x_dim)))	
      goto abort;
	if(!(setz    = MakeIVectorNR(1,x_dim)))
      goto abort;
	if(!(oldsetp = MakeIVectorNR(1,x_dim)))
      goto abort;

// OK no memory error happended!
   memoryError = false;

// Find largest value in W
	index = 1;
	biggest = W[1];				
	for(i = 2; i <= x_dim; i++)
   {
		if(W[i] > biggest)
		{
			biggest = W[i];
			index = i;
		}
   }

// "setp" initially empty (all zero) it will be filled with indices as we go
// in "setz" "1" indicates position filled, "-1" indicates position empty
	for(i = 1; i <= x_dim; i++)
	{ 
		oldsetp[i]=setp[i]=Soln[i]=0;		 
		setz[i]=1;				
	}
// printmd(systemmat,x_dim+1,y_dim+x_dim);


//	MAIN LOOP STARTS ****************************************************

//	step #5 L&H
   int count = 0;
	while(1)
   { 

      if(ProcessBackgroundEvents() != OK)
      {
         goto abort;
      }
   //   TextMessage("count = %d\n",count);
      count++;
   // Append "index" to P
		for(i = 1; i <= x_dim; i++) oldsetp[i]=setp[i];	
		i=1;		  
		while(setp[i]!=0) i++;						 
		setp[i] = index;					
		setz[index]=-1;	
			
   // Step #6 L&H
   // Orthogonal transforms are applied to systemmat. 
   // A diagonal matrix can be now be constructed using systemmat
   // columns that are indexed in "setp"
      if((r = QRdecmp(systemmat,setp,oldsetp,x_dim+1,y_dim+x_dim)) == 1)
      {
         goto abort;
      }
      else if(r == ERR)
      {
         memoryError = true;
         goto abort;
      }

   // Solve the LS problem defined by Systemmat and setp
   // Soln should now contain better estimates of spectrum at problem "frequencies" stored in setp (P in L&H)
		LinearEqnSolve(systemmat,x_dim+1,setp,Soln); 

   // IMPORTANT, we test Soln[index] to see if it is negative (theoretically it can't be negative)
   // However, Soln[index]<0 can occur due to round off error, p165 L&H
   // Setting W[index] to zero and reanalysising W stabilises the algorithm.
		if(Soln[index]<0)
			TextMessage("\n   Correction made for roundoff error\n");

		while(Soln[index]<0)
		{
			W[index] = 0;
			biggest = W[1];			//  choose a new index to add to setp
			
			newindex = 0;
			biggest = 0;
      // Find biggest W element whose index is still in setz
			for(i = 1; i <= x_dim; i++)	
         {
				if((W[i]>biggest)&&(setz[i]==1))
				{
					biggest=W[i];
					newindex=i;
				}
         }
			if(newindex==0)
         {
            goto abort;
			}
				
			setz[index] = 1;     // remake setp and setz
			setz[newindex] = -1;
			i = 1;
			while(setp[i]!=0)i++;    
			setp[i-1]=index=newindex;
			
         if((r = QRdecmp(systemmat,setp,oldsetp,x_dim+1,y_dim+x_dim)) == 1)
         {
            goto abort;
         }
         else if(r == ERR)
         {
            memoryError = true;
            goto abort;
         }
			LinearEqnSolve(systemmat,x_dim+1,setp,Soln); 
		}
		
		repeat=1;	
		
		while(repeat)
		{	
         if(ProcessBackgroundEvents() != OK)
         {
            goto abort;
         }
     // STEP #7 #8 #9  is there is a negative value in Soln? if so what's lamda won't be zero
			FindLamda(&lamda,spectrum,Soln,setp,&indextozero,x_dim); 	
			
    // If there is a negative value in "Soln" we have to keep 
    // removing indices from "setp" until all elements in "Soln" are positive
			if(indextozero != 0) // Some zj < 0
			{   		
				if(lamda == 0)
				{
					ErrorMessage("algorithm fault - infinite loop");    // now superfluous (yeah!)
               algorithmFault = true;
               goto abort;
				}
          // STEP #10 #11
				for(i = 1; i <= x_dim; i++)
					spectrum[i] = spectrum[i] + lamda*(Soln[i]-spectrum[i]);	
				for(i = 1; i <= x_dim; i++) oldsetp[i]=setp[i];			// save old setp
				DeleteFromSet(setp,setz,indextozero,x_dim);		      //  record this removal in setp and setz
				spectrum[indextozero]=0;
			 // STEP #6	
            if((r = QRdecmp(systemmat,setp,oldsetp,x_dim+1,y_dim+x_dim)) == 1)
            {
               goto abort;
            }
            else if(r == ERR)
            {
               memoryError = true;
               goto abort;
            }
				LinearEqnSolve(systemmat,x_dim+1,setp,Soln); 
			}
			else // All zj > 0 so copy solution to spectrum and back to step 2 (non-negative criterion)
			{

				repeat = 0;
				for(i = 1; i <= x_dim; i++)
					spectrum[i] = Soln[i];
			}
		}
		

  //	step #2 L&H : W = Designmat'*(data-Designmat*spectrum)
		Matmult(designmmat,x_dim,y_dim+x_dim,spectrum,ans);	
		for(i = 1; i <= y_dim; i++)	
			ans[i] = data[i]-ans[i];	
		for(i = y_dim+1; i <= y_dim+x_dim; i++)
			ans[i] =- ans[i];					  
		Matmult(designmmatT,y_dim+x_dim,x_dim,ans,W);	
 //     printvd(W,x_dim);

   //	Step #4 L&H
		biggest=0; index=0;
   // Find biggest W element whose index is still in setz,
   // if all these elements are < 0 then we are finished
      short cnt = 0;
		for(i = 1; i <= x_dim; i++)	
      {
			if((W[i] > biggest) && (setz[i] == 1))
			{
				biggest=W[i];
				index=i;
            cnt++;
			}
      }
      CText arg;
      arg.Format("%s(%f)",callBack.Str(),(float)cnt/x_dim);
      ProcessMacroStr(par->itfc,1,arg.Str());

   // Step #3 L&H - finished condition
		if(index == 0 || setp[x_dim]!=0)
         break;
	} 

   if(spectrumf = MakeVector(x_dim))
   {
      CopyDoubleVectorToFloat(spectrum,spectrumf,x_dim);
      par->retVar[1].MakeMatrix2DFromVector(spectrumf,x_dim,1);
      par->nrRetVar = 1;
   }
   else
      memoryError = true;

abort:

// Tidy up
   if(spectrumf) FreeVector(spectrumf);
	if(setp) FreeIVectorNR(setp,1,x_dim);
	if(setz) FreeIVectorNR(setz,1,x_dim);
	if(oldsetp) FreeIVectorNR(oldsetp,1,x_dim);
   if(Soln) FreeDVectorNR(Soln,1,x_dim);
   if(data) FreeDVectorNR(data,1,x_dim);
   if(W) FreeDVectorNR(W,1,x_dim);
   if(spectrum) FreeDVectorNR(spectrum,1,x_dim);
	if(ans) FreeDVectorNR(ans,1,y_dim+x_dim);
   if(ansW) FreeDVectorNR(ansW,1,x_dim+y_dim);
	if(systemmat) FreeDMatrix2DNR(systemmat,1,y_dim+x_dim,1,x_dim+1);
	if(designmmat) FreeDMatrix2DNR(designmmat,1,y_dim+x_dim,1,x_dim);
	if(designmmatT) FreeDMatrix2DNR(designmmatT,1,x_dim,1,y_dim+x_dim);

   if(memoryError)
   {
      ErrorMessage("Out of memory in L&H inversion");
      return(ERR);
   }

   if(algorithmFault)
      return(ERR);

   return(OK);
}



// Used by nnls2d
short Soften(DLLParameters* par, char arg[])
{
   Variable var;
   short r;
   float *v;
   float *soft;
   long w,h;
   float max;

// Extract parameter ************************   
   if((r = ArgScan(par->itfc, arg,1,"vector","e","v",&var)) < 0)
      return(r); 

// Check for invalid variable type ************
   if(VarType(&var) != MATRIX2D || VarHeight(&var) != 1)
   {
      ErrorMessage("data must be a real 1D matrix");
      return(ERR);
   }

// Get the matrix dimensions ******************
   v = VarRealMatrix(&var)[0];
   w = VarWidth(&var);
   h = VarHeight(&var);

// Allocate space for the difference-vector ***
   par->retVar[1].MakeAndLoadMatrix2D(NULL,w,1);
   soft = VarRealMatrix(&par->retVar[1])[0];

// Soften the vector
   for(long i = 1; i <= w-2; i++)
   {
      max = v[i]*v[i-1];
      if(v[i]*v[i] > max)
         max = v[i]*v[i];
      if(v[i+1]*v[i+1] > max)
         max = v[i+1]*v[i+1];
      soft[i] = sqrt(max);
   }
   soft[0] = v[0];
   if(v[1]*v[1] > v[0]*v[0])
      soft[0] = v[1];

   soft[w-1] = v[w-1];
   if(v[w-2]*v[w-2] > v[w-1]*v[w-1])
      soft[w-1] = v[w-2];

   par->nrRetVar = 1;

   return(OK);
}


// Place hold function
short NoFunc(DLLParameters* par, char arg[])
{
   Variable var;
   short r;

// Extract parameter ************************   
   if((r = ArgScan(par->itfc,arg,1,"parameter","e","v",&var)) < 0)
      return(r); 

   if(par->retVar[1].FullCopy(&var) == ERR)
      return(ERR);
   par->nrRetVar = 1;

   return(OK);
}


void CopyVectorToDouble(Variable* var, double *v)
{
   long width = VarWidth(var);

   for(long i = 0; i < width; i++)
   {
      v[i+1] = VarRealMatrix(var)[0][i];
   }
}

void CopyMatrixToDouble(Variable* var, double **m)
{
   long width = VarWidth(var);
   long height = VarHeight(var);

   for(long j = 0; j < height; j++)
   {
      for(long i = 0; i < width; i++)
      {
         m[j+1][i+1] = VarRealMatrix(var)[j][i];
      }
   }
}

void CopyDoubleVectorToFloat(double* in, float* out, long len)
{
   for(long i = 0; i < len; i++)
   {
      out[i] = in[i+1];
   }
}




/**********************************************************************************************
   Removes element in setp that has spectrum[setp]=0
***********************************************************************************************/

void DeleteFromSet(long *setp, long *setz, long index, long x_dim)  
{
	long i,j;
	
	setz[index] = 1;
	i = 1;
	while(setp[i] != index) i++;
	for(j = i; j < x_dim; j++)
		setp[j] = setp[j+1];
	setp[x_dim]=0;
}


/**********************************************************************************************
   Run thru all indices in setp, calculate the lamda value for indices that 
   correspond to negative coefficients in Soln, see which lamda is smallest
   L&H step 7 if zj > 0 for all j members of P return then zeroindex = 0
   L&H step 8&9 lamda ( or alpha) = xq/(xq-zq) = min{xj/(xj-zj) : zj <= 0 j member of P
***********************************************************************************************/

void FindLamda(double *lamda, double *spectrum, double *Soln, long *setp, long *zeroindex, long x_dim)
{
	long i,j;
	(*zeroindex) = 0;
	(*lamda) = 0;

	for(i = 1; (setp[i] != 0) && (i <= x_dim); i++)  
   {
      j = setp[i];
      if(Soln[j] < 0)
		{
			if((*lamda) == 0)
			{
				(*lamda) = spectrum[j]/(spectrum[j]-Soln[j]);   // for the first time thru the loop
				(*zeroindex) = j;
			}
			if(spectrum[j]/(spectrum[j]-Soln[j]) < (*lamda))  // successive loops compare to current lamda
			{   
				(*lamda) = spectrum[j]/(spectrum[j]-Soln[j]);
				(*zeroindex) = j;
			}
		}
   }
}


void FindLamda(float *lamda, float *spectrum, float *Soln, long *setp, long *zeroindex, long x_dim)
{
	long i,j;
	(*zeroindex) = 0;
	(*lamda) = 0;

	for(i = 1; (setp[i] != 0) && (i <= x_dim); i++)  
   {
      j = setp[i];
      if(Soln[j] < 0)
		{
			if((*lamda) == 0)
			{
				(*lamda) = spectrum[j]/(spectrum[j]-Soln[j]);   // for the first time thru the loop
				(*zeroindex) = j;
			}
			if(spectrum[j]/(spectrum[j]-Soln[j]) < (*lamda))  // successive loops compare to current lamda
			{   
				(*lamda) = spectrum[j]/(spectrum[j]-Soln[j]);
				(*zeroindex) = j;
			}
		}
   }
}

void LinearEqnSolve(double **Systemmat,long x_dim,long *setp,double *ans)   //Systemmat has already been diagonalised, only need to know x_dim
{
	long i,j,setlength=1;
	double sum,*tempans;
	
	while((setp[setlength]!=0)&&(setlength<=x_dim-1))setlength++; //find length of setp
	setlength--;
	
	tempans=MakeDVectorNR(1,setlength);
	for(i=1;i<=setlength;i++) tempans[i]=0; //init tempans
		
	tempans[setlength]=Systemmat[setlength][x_dim]/Systemmat[setlength][setp[setlength]];
	
	for(i=setlength-1;i>=1;i--){
		sum=0;
		for(j=setlength;j>i;j--)
			sum+=Systemmat[i][setp[j]]*tempans[j];
	   	tempans[i]=(Systemmat[i][x_dim] - sum)/Systemmat[i][setp[i]];
	 } 
	for(i=1;i<=x_dim-1;i++) ans[i]=0;
	for(i=1;i<=setlength;i++) ans[setp[i]]=tempans[i];
	
	FreeDVectorNR(tempans,1,setlength);
}

void LinearEqnSolve(float **Systemmat,long x_dim,long *setp, float *ans)   //Systemmat has already been diagonalised, only need to know x_dim
{
	long i,j,setlength=1;
	float sum,*tempans;
	
	while((setp[setlength]!=0)&&(setlength<=x_dim-1))setlength++; //find length of setp
	setlength--;
	
	tempans=MakeVectorNR(1,setlength);
	for(i=1;i<=setlength;i++) tempans[i]=0; //init tempans
		
	tempans[setlength]=Systemmat[setlength][x_dim]/Systemmat[setlength][setp[setlength]];
	
	for(i=setlength-1;i>=1;i--){
		sum=0;
		for(j=setlength;j>i;j--)
			sum+=Systemmat[i][setp[j]]*tempans[j];
	   	tempans[i]=(Systemmat[i][x_dim] - sum)/Systemmat[i][setp[i]];
	 } 
	for(i=1;i<=x_dim-1;i++) ans[i]=0;
	for(i=1;i<=setlength;i++) ans[setp[i]]=tempans[i];
	
	FreeVectorNR(tempans,1,setlength);
}

/**********************************************************************************************
   Takes the tranpose of (x_dim by y_dim) matrix "mat" and returns resultin "ans".
***********************************************************************************************/

void Transpose(double **mat, long x_dim, long y_dim, double **ans)
{
	long i,j;
	
	for(i = 1; i <= y_dim; i++)
		for(j = 1; j <= x_dim; j++)
			ans[j][i] = mat[i][j];
}

void Transpose(float **mat, long x_dim, long y_dim, float **ans)
{
	long i,j;
	
	for(i = 1; i <= y_dim; i++)
		for(j = 1; j <= x_dim; j++)
			ans[j][i] = mat[i][j];
}

/**********************************************************************************************
   Multiplies the matrix "mat" by the vector "vec", result is returned in "temp".
***********************************************************************************************/

void Matmult(double **mat, long x_dim,long y_dim, double *vec, double *temp) 
{
	long i,j;
	
	for(i = 1; i <= y_dim; i++) temp[i]=0;
	
	for(i = 1; i <= y_dim; i++)
   {
		for(j = 1; j <= x_dim; j++)
      {
			temp[i] += mat[i][j]*vec[j];
      }
	}	
}

void Matmult(float **mat, long x_dim,long y_dim, float *vec, float *temp) 
{
	long i,j;
	
	for(i = 1; i <= y_dim; i++) temp[i]=0;
	
	for(i = 1; i <= y_dim; i++)
   {
		for(j = 1; j <= x_dim; j++)
      {
			temp[i] += mat[i][j]*vec[j];
      }
	}	
}

/**********************************************************************************************
 A series of Householder transformation (see p57 L&H), designed specifically for this LS problem
***********************************************************************************************/

short QRdecmp(double **mat, long *setp, long *oldsetp, long x_dim, long y_dim)
{
	long i,k,setlength=1,added=1,removed=1;
	double *u,b;
	
	if(!(u = MakeDVectorNR(1,y_dim)))
      return(ERR);

   for(i = 1; i <= y_dim; i++) u[i]=0;
	
	while((setp[setlength]!=0)&&(oldsetp[setlength]!=0)) setlength++; //find a zero in either setp or oldsetp, lengthsetp tells us how many indices in setp
	if(setp[setlength]==0)	added=0; //i.e. somethings been removed.

	if(added)
   {
		House(setp[setlength],setlength,mat,y_dim,u,&b);  //householder transformation to most recently chosen column, "setp[setlength]", in the matrix
		Orthomult(mat,u,&b,x_dim,y_dim);
	}
	else
   {
		while(setp[removed]==oldsetp[removed]) removed++;  //householder transformation to all columns listed in setp
		k = removed;
		while(setp[k]!=0)
      {
         if(ProcessBackgroundEvents() != OK)
            return(1);
			House(setp[k],k,mat,y_dim,u,&b);
			Orthomult(mat,u,&b,x_dim,y_dim);
			k++;
		}
	}
	FreeDVectorNR(u,1,y_dim);
   return(OK);
}

short QRdecmp(float **mat,long *setp,long *oldsetp,long x_dim,long y_dim)
{
	long i,k,setlength=1,added=1,removed=1;
	float *u,b;
	
	if(!(u = MakeVectorNR(1,y_dim)))
      return(ERR);
   for(i = 1; i <= y_dim; i++) u[i]=0;
	
	while((setp[setlength]!=0)&&(oldsetp[setlength]!=0)) setlength++; //find a zero in either setp or oldsetp, lengthsetp tells us how many indices in setp
	if(setp[setlength]==0)	added=0; //i.e. somethings been removed.

	if(added)
   {
		House(setp[setlength],setlength,mat,y_dim,u,&b);  //householder transformation to most recently chosen column, "setp[setlength]", in the matrix
		Orthomult(mat,u,&b,x_dim,y_dim);
	}
	else
   {
		while(setp[removed]==oldsetp[removed]) removed++;  //householder transformation to all columns listed in setp
		k = removed;
		while(setp[k]!=0)
      {
			House(setp[k],k,mat,y_dim,u,&b);
			Orthomult(mat,u,&b,x_dim,y_dim);
			k++;
		}
	}
	FreeVectorNR(u,1,y_dim);
   return(OK);
}

/**********************************************************************************************
     Function used to apply the householder transformation to each column in some "mat"
***********************************************************************************************/

void Orthomult(double **mat, double *u, double *b, long x_dim, long y_dim)
{
	long i,j;
	double c;

	for(i = 1; i <= x_dim; i++) // For each column i
   { 
		c = 0;
		for(j = 1; j <= y_dim; j++)
			c += u[j]*mat[j][i]/b[0];
		for(j = 1; j <= y_dim; j++)
			mat[j][i] += c*u[j];
	}
}


void Orthomult(float **mat, float *u, float *b, long x_dim, long y_dim)
{
	long i,j;
	float c;

	for(i = 1; i <= x_dim; i++) // For each column i
   { 
		c = 0;
		for(j = 1; j <= y_dim; j++)
			c += u[j]*mat[j][i]/b[0];
		for(j = 1; j <= y_dim; j++)
			mat[j][i] += c*u[j];
	}
}

/**********************************************************************************************
  Reduces an n by n symmetric matrix A to tridiagonal form
  Rotates a vector from "mat" (indexed by "column").
  the outputs "u" (vector) and "b" (float) allow us to apply orthogonal transform elsewhere
***********************************************************************************************/

void House(long column, long pivot, double **mat, long y_dim, double *u, double *b)	
{
	long i;
	double sum=0,s;   
	
	for(i = 1; i <= pivot-1; i++) u[i]=0;	//set "u" to zero from 1 to pivot-1

	for(i = pivot+1; i <= y_dim; i++)
   {	 
		u[i] = mat[i][column];		//copy matrix column from pivot+1 and above
		sum += mat[i][column]*mat[i][column];
	}
	if(mat[pivot][column]<0)
		s = sqrt(sum + mat[pivot][column]*mat[pivot][column]);
	else
		s = -sqrt(sum + mat[pivot][column]*mat[pivot][column]);
	
	u[pivot] = mat[pivot][column]-s;
	b[0] = s*u[pivot];
}

void House(long column, long pivot, float **mat, long y_dim, float *u, float *b)	
{
	long i;
	float sum=0,s;   
	
	for(i = 1; i <= pivot-1; i++) u[i]=0;	//set "u" to zero from 1 to pivot-1

	for(i = pivot+1; i <= y_dim; i++)
   {	 
		u[i] = mat[i][column];		//copy matrix column from pivot+1 and above
		sum += mat[i][column]*mat[i][column];
	}
	if(mat[pivot][column]<0)
		s = sqrt(sum + mat[pivot][column]*mat[pivot][column]);
	else
		s = -sqrt(sum + mat[pivot][column]*mat[pivot][column]);
	
	u[pivot] = mat[pivot][column]-s;
	b[0] = s*u[pivot];
}

void Soften(float *slope, float *softslope, long x_dim)
{
	long i,j;
	float max;
	
	for(i=2;i<x_dim;i++)
   {			//soften slope and curvature
		max=slope[i-1]*slope[i-1];
		for(j=i;j<=i+1;j++)
      {
			if(slope[j]*slope[j]>max) 
				max=slope[j]*slope[j];
		}
		softslope[i]=sqrt(max);
	}
	softslope[1]=slope[1];
	if(slope[2]*slope[2]>slope[1]*slope[1])  softslope[1]=slope[2];
	softslope[x_dim]=slope[x_dim];
	if(slope[x_dim-1]*slope[x_dim-1]>slope[x_dim]*slope[x_dim])  softslope[x_dim]=slope[x_dim-1];
}

// Print a matrix to the screen
void printm(float **mat, long x_dim, long y_dim)
{
	long i,j;
	
   TextMessage("\n\nMatrix = [\n");
	for(i = 1; i <= y_dim; i++)
   {
		for(j=1; j <= x_dim; j++)
      {
			TextMessage("%7.2f ",mat[i][j]);
      }
		TextMessage("\n");
	}
   TextMessage("]\n");
}

// Print a matrix to the screen
void printmd(double **mat, long x_dim, long y_dim)
{
	long i,j;
	
   TextMessage("\n\nMatrix = [\n");
	for(i = 1; i <= y_dim; i++)
   {
		for(j=1; j <= x_dim; j++)
      {
			TextMessage("%7.2f ",mat[i][j]);
      }
		TextMessage("\n");
	}
   TextMessage("]\n");
}

// Print a float vector to the screen
void printvf(float *vec, long y_dim)
{
	long i;
	char str[20];
	
   TextMessage("\n\nVector = [\n");
	for(i=1; i <= y_dim; i++)
		TextMessage("%7.7f\n",vec[i]);
   TextMessage("]\n");
}

// Print a double vector to the screen
void printvd(double *vec, long y_dim)
{
	long i;
	char str[20];
	
   TextMessage("\n\nVector = [\n");
	for(i=1; i <= y_dim; i++)
		TextMessage("%7.7f\n",(float)vec[i]);
   TextMessage("]\n");
}

// Print a long integer vector to the screen
void printvi(long *vec,long y_dim)
{
	long i;
	char str[20];
	
   TextMessage("\n\nVector = [\n");
	for(i = 1; i <= y_dim; i++)
		TextMessage("%ld\n",vec[i]);
   TextMessage("]\n");
}


