/******************************************************************************
*                                                                             *
*  FFT based routines :                                                       *
*                                                                             *
*  FFT ........... Fast-fourier transform (used by most of the following).    *
*                                                                             *
*                                           Craig Eccles 1992                 *
*                                                                             *
******************************************************************************/
#include "stdafx.h"
#include "fourier.h"
#include <math.h>
#include "allocate.h"
#include "cArg.h"
#include "command_other.h"
#include "evaluate.h"
#include "globals.h"
#include "interface.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

#define FORWARD 1
#define REVERSE -1
#define SWAP(a,b) {float temp=(a);(a)=(b);(b)=temp;}

void FFT(complex cmplx[],long nn,short isign);
short PowerOf2(long size);
void RealFFT(complex cdata[], long n, short isign);

/******************************************************************************
                             Complex FFT data                                 
 
  Applies a standard radix 2 FFT to 1D data in row or column form
  Result is a row or a column complex matrix
******************************************************************************/

int FourierTransform(Interface* itfc ,char args[])
{
   short type;
   long size;
   Variable result;
   CArg carg;

   Variable *ans = &itfc->retVar[1];
   
// Make sure there is only a single argument ******
   if(carg.Count(args) != 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
     
// Process passed argument ************************
   if((type = Evaluate(itfc,RESPECT_ALIAS,args,&result)) == ERR)
      return(ERR);

// Process 1D real data ****************************    
   if(type == MATRIX2D)
   {
      if(result.GetDimY() == 1) // Process row data
      {
	       size = result.GetDimX();
	       if(PowerOf2(size) == ERR) 
	       {
	          ErrorMessage("size of vector for fft is not a power of 2");    
	          return(ERR);
	       }
	        
          float **mat = result.GetMatrix2D();
          complex **cout = MakeCMatrix2D(size,1L); // Row vector
	       
          for(long i = 0; i < size; i++) // Copy col matrix to row cmatrix
          {
             cout[0][i].r = mat[0][i];
             cout[0][i].i = 0;
          }
            
          FFT(cout[0],size,FORWARD);
          DataReorder(cout[0],size);
          
		 	 ans->AssignCMatrix2D(cout,size,1); // Save result in 'ans' variable 
       }
       else if(result.GetDimX() == 1) // Process column data
       {
          size = result.GetDimY();  
	       if(PowerOf2(size) == ERR) 
	       {
	          ErrorMessage("size of vector for fft is not a power of 2");    
	          return(ERR);
	       } 
	              
	       float **mat = result.GetMatrix2D();
          complex **crow = MakeCMatrix2D(size,1L);  // Row vector
          complex **ccol = MakeCMatrix2D(1L,size); // Col vector

          for(long i = 0; i < size; i++) // Copy column matrix to row cmatrix
          {
             crow[0][i].r = mat[i][0];
             crow[0][i].i = 0;
          }
            
          FFT(crow[0],size,FORWARD);
          DataReorder(crow[0],size);

          for(long i = 0; i < size; i++) // Copy row cmatrix to column cmatrix
          {
             ccol[i][0] = crow[0][i];
          }
           
          FreeCMatrix2DNR(crow,0L, 1L, 0L, size-1);         
		 	 ans->AssignCMatrix2D(ccol,1,size); // Save result in 'ans' variable
       }
   } 

// Process 1D double precision data ****************************    
   //else if(type == DMATRIX2D)
   //{
   //   if(result.GetDimY() == 1) // Process row data
   //   {
	  //     size = result.GetDimX();
	  //     if(PowerOf2(size) == ERR) 
	  //     {
	  //        ErrorMessage("size of vector for fft is not a power of 2");    
	  //        return(ERR);
	  //     }
	  //      
   //       double **mat = result.GetDMatrix2D();
   //       complexd **cout = MakeCDMatrix2D(size,1L); // Row vector
	  //     
   //       for(long i = 0; i < size; i++) // Copy col matrix to row cmatrix
   //       {
   //          cout[0][i].r = mat[0][i];
   //          cout[0][i].i = 0;
   //       }
   //         
   //       FFT(cout[0],size,FORWARD);
   //       DataReorder(cout[0],size);
   //       
		 //	 ans->AssignCDMatrix2D(cout,size,1); // Save result in 'ans' variable 
   //    }
   //    else if(result.GetDimX() == 1) // Process column data
   //    {
   //       size = result.GetDimY();  
	  //     if(PowerOf2(size) == ERR) 
	  //     {
	  //        ErrorMessage("size of vector for fft is not a power of 2");    
	  //        return(ERR);
	  //     } 
	  //            
	  //     float **mat = result.GetMatrix2D();
   //       complex **crow = MakeCMatrix2D(size,1L);  // Row vector
   //       complex **ccol = MakeCMatrix2D(1L,size); // Col vector

   //       for(long i = 0; i < size; i++) // Copy column matrix to row cmatrix
   //       {
   //          crow[0][i].r = mat[i][0];
   //          crow[0][i].i = 0;
   //       }
   //         
   //       FFT(crow[0],size,FORWARD);
   //       DataReorder(crow[0],size);

   //       for(long i = 0; i < size; i++) // Copy row cmatrix to column cmatrix
   //       {
   //          ccol[i][0] = crow[0][i];
   //       }
   //        
   //       FreeCMatrix2DNR(crow,0L, 1L, 0L, size-1);         
		 //	 ans->AssignCDMatrix2D(ccol,1,size); // Save result in 'ans' variable
   //    }
   //} 

// Process 1D complex data ****************************
   else if(type == CMATRIX2D)
   {
      if(result.GetDimY() == 1) // Process row data
      {
         size = result.GetDimX();
         if(PowerOf2(size) == ERR) 
         {
            ErrorMessage("size of vector is not a power of 2");    
            return(ERR);
         }      
         complex **cmat = result.GetCMatrix2D();
       
         FFT(cmat[0],size,FORWARD);
         DataReorder(cmat[0],size);

		 	ans->AssignCMatrix2D(cmat,size,1); // Save result in 'ans' variable
         result.SetNull(); // Make sure data belongs to 'ans' only
	   }
      else if(result.GetDimX() == 1) // Process column data
      {
         size = result.GetDimY();  
	      if(PowerOf2(size) == ERR) 
	      {
	         ErrorMessage("size of vector for fft is not a power of 2");    
	         return(ERR);
	      } 
	             
         complex **ccol = result.GetCMatrix2D();    // Column vector
         complex **crow = MakeCMatrix2D(size,1L); // Row vector

         for(long i = 0; i < size; i++) // Copy to row vector
            crow[0][i] = ccol[i][0];
            
         FFT(crow[0],size,FORWARD); // Take FFT
         DataReorder(crow[0],size);

         for(long i = 0; i < size; i++) // Copy back to column vector
            ccol[i][0] = crow[0][i];
            
         FreeCMatrix2DNR(crow,0L, 1L, 0L, size-1);  // Remove temp row vector   

		 	ans->AssignCMatrix2D(ccol,1,size); // Save result in 'ans' variable
         result.SetNull(); // Make sure data belongs to 'ans' only
      }
   }
   
// Check for invalid data types to transform ****************
   else
   {
      ErrorMessage("fft data type should be a row or column (c)matrix");
      return(ERR);
   }
   itfc->nrRetValues = 1;
   return(OK);
}

/******************************************************************************
    Return integer power of 2 which size equals returns zero if size is not a 
    power of 2
******************************************************************************/

short PowerOf2(long size)
{
   long i,comp = 1;
   
   for(i = 0; i < 32; i++)
   {
      if(size == (comp << i))
         break;
   }
   if(i == 32) return(ERR);
   
   return(i);
}
     
/******************************************************************************
    Perform a real Fourier transform on a real or complex data set
    (in the later case just look at the real part)
******************************************************************************/

int RealFourierTransform(Interface* itfc ,char args[])
{
   short type;
   long size;
   long i,j;
   Variable result;
   Variable *ans = &itfc->retVar[1];
   CArg carg;

// Make sure there is only a single argument ******
   if(carg.Count(args) != 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
       
     
// Process passed argument ************************
   if((type = Evaluate(itfc,RESPECT_ALIAS,args,&result)) == ERR)
   {
      return(ERR);
   }

// Process real data
   if(type == MATRIX2D)
   {
      if(result.GetDimY() == 1) // Process row data
      {
	      size = result.GetDimX();
         if(PowerOf2(size) == ERR) 
         {
            ErrorMessage("size of vector is not a power of 2");    
            return(ERR);
         }      
         float **cin = result.GetMatrix2D();         // Input array
         complex **cout = MakeCMatrix2D(size/2,1L);  // Output array
         complex *work = MakeCVector(size/2);        // Work array
  
       // Copy real data to work array
		   j = 0;
		   for(i = 0, j = 0; i < size; i+=2)
		   {
		      work[j].r   = cin[0][i];
		      work[j++].i = cin[0][i+1];
		   }  
		   
		 // FT real data       		 
         RealFFT(work,size/2,FORWARD);
         
       // Copy back complex data
		   for(i = 0; i < size/2; i++)
		   {
		      cout[0][i].r = work[i].r;
		      cout[0][i].i = work[i].i;
		   }
         FreeCVector(work);         
		 	ans->AssignCMatrix2D(cout,size/2,1); // Save result in 'ans' variable 
	   }
     // Process a column matrix
       else if(result.GetDimX() == 1) // Process column data
       {
          size = result.GetDimY();  
         if(PowerOf2(size) == ERR) 
         {
            ErrorMessage("size of vector is not a power of 2");    
            return(ERR);
         }      
         float **cin = result.GetMatrix2D();  // Input array
         complex **cout = MakeCMatrix2D(1L, size/2); // Output array
         complex *work = MakeCVector(size/2);  // Work array

       // Copy real data to work array
		   j = 0;
		   for(i = 0, j = 0; i < size; i+=2)
		   {
		      work[j].r   = cin[i][0];
		      work[j++].i = cin[i+1][0];
		   }  
		   
		 // FT real data       
         RealFFT(work,size/2,FORWARD);
         
       // Copy back complex data
		   for(i = 0; i < size/2; i++)
		   {
		      cout[i][0].r = work[i].r;
		      cout[i][0].i = work[i].i;
		   }
         FreeCVector(work);         
		 	ans->AssignCMatrix2D(cout,1,size/2); // Save result in 'ans' variable 
	   }	   
	}
      
// Process 1D complex data ****************************    
   else if(type == CMATRIX2D)
   {
      // Process a row matrix
      if(result.GetDimY() == 1) // Process row data
      {
         size = result.GetDimX();
         if(PowerOf2(size) == ERR) 
         {
            ErrorMessage("size of vector is not a power of 2");    
            return(ERR);
         }      
         complex **cin = result.GetCMatrix2D(); // Input array
         complex **cout = MakeCMatrix2D(size/2,1L);    // Output array
         complex *work = MakeCVector(size/2);    // Work array

       // Copy real data to work array
		   j = 0;
		   for(i = 0, j = 0; i < size; i+=2)
		   {
		      work[j].r   = cin[0][i].r;
		      work[j++].i = cin[0][i+1].r;
		   }  
		   
		 // FT real data       
         RealFFT(work,size/2,FORWARD);
         
       // Copy back complex data
		   for(i = 0; i < size/2; i++)
		   {
		      cout[0][i].r = work[i].r;
		      cout[0][i].i = work[i].i;
		   }
         FreeCVector(work);         
		 	ans->AssignCMatrix2D(cout,size/2,1); // Save result in 'ans' variable 
	   }
      // Process a column matrix
      else if(result.GetDimX() == 1) // Process column data
      {
         size = result.GetDimY();  
         if(PowerOf2(size) == ERR) 
         {
            ErrorMessage("size of vector is not a power of 2");    
            return(ERR);
         }      
         complex **cin = result.GetCMatrix2D();         // Input array
         complex **cout = MakeCMatrix2D(1L, size/2); // Output array
         complex *work = MakeCVector(size/2); // Work array

       // Copy real data to work array
		   j = 0;
		   for(i = 0, j = 0; i < size; i+=2)
		   {
		      work[j].r   = cin[i][0].r;
		      work[j++].i = cin[i+1][0].r;
		   }  
		   
		 // FT real data       
         RealFFT(work,size/2,FORWARD);
         
       // Copy back complex data
		   for(i = 0; i < size/2; i++)
		   {
		      cout[i][0].r = work[i].r;
		      cout[i][0].i = work[i].i;
		   }
         FreeCVector(work);         
		 	ans->AssignCMatrix2D(cout,1,size/2); // Save result in 'ans' variable 
	   }	   
	}
   else
   {
      ErrorMessage("Argument should be a row or column (c)matrix");
      return(ERR);
   }	
  
   return(0);
}

void RealFFT(complex cdata[], long n, short isign)
{
   int i,i1,i2,i3,i4,n2p3;
   float c1=0.5,c2,h1r,h1i,h2r,h2i;
   double wr,wi,wpr,wpi,wtemp,theta;
   float *data;

   data = (&cdata[0].r) - 1; // Convert complex array to float 

   theta = 3.1415927/(double)n;
   if(isign == FORWARD)
   {
      c2 = -0.5;
      FFT(cdata,n,FORWARD);
   }
   else
   {
      c2 = 0.5;
      theta = -theta;
   }

   wtemp = sin(0.5*theta);
   wpr = -2.0*wtemp*wtemp;
   wpi = sin(theta);
   wr = 1.0 + wpr;
   wi = wpi;
   n2p3 = 2*n+3;

   for(i = 2; i <= n/2; i++)
   {
      i4 = 1 + (i3=n2p3-(i2=1+(i1=i+i-1)));
      h1r = c1*(data[i1]+data[i3]);
      h1i = c1*(data[i2]-data[i4]);
      h2r = -c2*(data[i2]+data[i4]);
      h2i = c2*(data[i1]-data[i3]);
      data[i1] = h1r+wr*h2r-wi*h2i;
      data[i2] = h1i+wr*h2i+wi*h2r;
      data[i3] = h1r-wr*h2r+wi*h2i;
      data[i4] = -h1i+wr*h2i+wi*h2r;
      wr = (wtemp=wr)*wpr-wi*wpi+wr;
      wi=wi*wpr+wtemp*wpi+wi;
   }
   if(isign == FORWARD)
   {
      data[1] = (h1r=data[1])+data[2];
      data[2] = h1r-data[2];
   }
   else
   {
      data[1] = c1*((h1r=data[1])+data[2]);
      data[2] = c1*(h1r-data[2]);
      FFT(cdata,n,REVERSE);
   }
}


/******************************************************************************
                          Complex inverse FFT data                             
******************************************************************************/

int InverseTransform(Interface* itfc ,char args[])
{
   Variable result;
   short type;
   long size;

   Variable *ans = &itfc->retVar[1];

   if(args[0] != '\0')
   {
	// Process passed argument
      if((type = Evaluate(itfc,RESPECT_ALIAS,args,&result)) == ERR)
	      return(ERR);
      
      if(type == MATRIX2D)
      {
         if(result.GetDimY() == 1) // Process row data
         {
	          size = result.GetDimX();
		       if(PowerOf2(size) == ERR) 
		       {
		          ErrorMessage("size of vector for ifft is not a power of 2");    
		          return(ERR);
		       }         
             float **mat = result.GetMatrix2D();
             complex **cmat = MakeCMatrix2D(size,1L); // Row vector
		       
	          for(long i = 0; i < size; i++) // Copy col matrix to row cmatrix
	          {
	             cmat[0][i].r = mat[0][i];
	             cmat[0][i].i = 0;
	          }              
	          DataReorder(cmat[0],size);          
	          FFT(cmat[0],size,REVERSE);
			 	 ans->AssignCMatrix2D(cmat,size,1); // Save result in 'ans' variable 
	      }
          else if(result.GetDimX() == 1) // Process column data
          {
             size = result.GetDimY();   
		       if(PowerOf2(size) == ERR) 
		       {
		          ErrorMessage("size of vector for ifft is not a power of 2");    
		          return(ERR);
		       } 	      
	          float **mat = result.GetMatrix2D();
	          complex **crow = MakeCMatrix2D(size,1L); // Row vector
	          complex **ccol = MakeCMatrix2D(1L, size); // Col vector
		       
	          for(long i = 0; i < size; i++) // Copy col matrix to row cmatrix
	          {
	             crow[0][i].r = mat[i][0];
	             crow[0][i].i = 0;
	          } 
	                       
	          DataReorder(crow[0],size);          
	          FFT(crow[0],size,REVERSE);
	          
	          for(long i = 0; i < size; i++) // Copy row cmatrix to column cmatrix
	          {
	             ccol[i][0] = crow[0][i];
	          }
	           
	          FreeCMatrix2DNR(crow,0L, 1L, 0L, size-1); 	          
			 	 ans->AssignCMatrix2D(ccol,size,1); // Save result in 'ans' variable 
	      }	      
	   }
      else if(type == CMATRIX2D)
      {
         if(result.GetDimY() == 1) // Process row data
         {
            size = result.GetDimX();
	         if(PowerOf2(size) == ERR) 
	         {
	            ErrorMessage("size of vector is not a power of 2");    
	            return(ERR);
	         }          
            complex **cmat = result.GetCMatrix2D();
            DataReorder(cmat[0],size);          
            FFT(cmat[0],size,REVERSE);
		 	   ans->AssignCMatrix2D(cmat,size,1); // Save result in 'ans' variable
            result.SetNull(); // Make sure data belongs to 'ans' only
         }
         else if(result.GetDimX() == 1) // Process column data
         {
            size = result.GetDimY();   
			   if(PowerOf2(size) == ERR) 
			   {
			      ErrorMessage("size of vector for fft is not a power of 2");    
			      return(ERR);
			   } 
            complex **ccol = result.GetCMatrix2D();    // Column vector
	         complex **crow = MakeCMatrix2D(size,1L); // Row vector

	         for(long i = 0; i < size; i++) // Copy to row vector
	            crow[0][i] = ccol[i][0];

            DataReorder(crow[0],size);          
            FFT(crow[0],size,REVERSE);

	         for(long i = 0; i < size; i++) // Copy back to column vector
	            ccol[i][0] = crow[0][i];
	            
	         FreeCMatrix2DNR(crow,0L, 1L, 0L, size-1);  // Remove temp row vector	
		 	   ans->AssignCMatrix2D(ccol,1,size); // Save result in 'ans' variable
            result.SetNull(); // Make sure data belongs to 'ans' only
			}
	   } 
      else
      {
         ErrorMessage("argument should be a row vector");
         return(ERR);
      } 
   }
   else
   {
      ErrorMessage("row vector not supplied");
      return(ERR);
   }
   itfc->nrRetValues = 1;      
   return(OK);
}

/******************************************************************************
                       Generates complex data from real data                 
******************************************************************************/

int HilbertTransform(Interface* itfc ,char args[])
{
   long i,j;
   CArg carg;
   short type;
   long size;
   Variable result;

   Variable *ans = &itfc->retVar[1];
   
// Make sure there is only a single argument ******
   if(carg.Count(args) != 1)
   {
      ErrorMessage("expecting a single argument");
      return(ERR);
   }
       
// Process passed argument ************************
   if((type = Evaluate(itfc,RESPECT_ALIAS,args,&result)) == ERR)
      return(ERR);

// Process 1D real data ****************************
   if(type == MATRIX2D)
   {
      if(result.GetDimY() == 1) // Process row data
      {
	       size = result.GetDimX();
	       if(PowerOf2(size) == ERR) 
	       {
	          ErrorMessage("size of vector for fft is not a power of 2");    
	          return(ERR);
	       }
	        
          float **mat = result.GetMatrix2D();
          complex **cout = MakeCMatrix2D(2*size,1L); // Row vector twice as long
	     
          for(i = 0; i < 2*size; i++) // Zero output array
          {
            cout[0][i].r = 0;
            cout[0][i].i = 0;
          }

          for(i = size*2-2; i >= 2; i-=2) // Copy col matrix to row cmatrix interpolating
          {
             j = i/2;
             cout[0][i].r = mat[0][j];
             cout[0][i].i = 0;
             cout[0][i-1].r = (mat[0][j]+mat[0][j-1])/2;
             cout[0][i-1].i = 0;
          }
            
          FFT(cout[0],size*2,REVERSE);
          
     // Scale by 2 to allow for negative frequencies - except for DC point */
			 for(i = 1; i < size; i++)
			 {
			    cout[0][i].r *= 2;
			    cout[0][i].i *= 2;
			 }
 
          FFT(cout[0],size,FORWARD);
                       
		 	 ans->AssignCMatrix2D(cout,size,1); // Save result in 'ans' variable 
       }
   }
   else if(type == CMATRIX2D)
   {
      if(result.GetDimY() == 1) // Process row data
      {
	       size = result.GetDimX();
	       if(PowerOf2(size) == ERR) 
	       {
	          ErrorMessage("size of vector for fft is not a power of 2");    
	          return(ERR);
	       }
	        
          complex **cmat = result.GetCMatrix2D();
          complex **cout = MakeCMatrix2D(2*size,1L); // Row vector twice as long

          for(i = 0; i < 2*size; i++) // Zero output array
          {
            cout[0][i].r = 0;
            cout[0][i].i = 0;
          }

          for(i = size*2-2; i >= 2; i-=2) // Copy col matrix to row cmatrix interpolating
          {
             j = i/2;
             cout[0][i].r = cmat[0][j].r;
             cout[0][i].i = 0;
             cout[0][i-1].r = (cmat[0][j].r+cmat[0][j-1].r)/2;
             cout[0][i-1].i = 0;
          }
            
          FFT(cout[0],size*2,REVERSE);
          
     // Scale by 2 to allow for negative frequencies - except for DC point */
			 for(i = 1; i < size; i++)
			 {
			    cout[0][i].r *= 2;
			    cout[0][i].i *= 2;
			 }

          FFT(cout[0],size,FORWARD);
                       
		 	 ans->AssignCMatrix2D(cout,size,1); // Save result in 'ans' variable 
       }      
   }

   return(0);
}


/******************************************************************************
       Reorder data so that center of data set is zero frequency or time      
******************************************************************************/

void DataReorder(float *data, long size)
{
   long i,offset;
   float temp;
   
   offset = size/2;

   for(i = 0; i < offset; i++)
   {
      temp = data[i];
      data[i] = data[i + offset];
      data[i + offset] = temp;
   }
}


/******************************************************************************
       Reorder data so that center of data set is zero frequency or time      
******************************************************************************/

void DataReorder(double *data, long size)
{
   long i,offset;
   double temp;
   
   offset = size/2;

   for(i = 0; i < offset; i++)
   {
      temp = data[i];
      data[i] = data[i + offset];
      data[i + offset] = temp;
   }
}


void DataReorder(complex *data, long size)
{
   long i,offset;
   complex temp;
   
   offset = size/2;

   for(i = 0; i < offset; i++)
   {
      temp = data[i];
      data[i] = data[i + offset];
      data[i + offset] = temp;
   }
}






/******************************************************************************
      Fast Fourier transform - using algorithm from Numerical Recipes p 411   
******************************************************************************/

void FFT(complex cmplx[],long nn,short isign)
{
   float tempi,tempr;
   float fc,fs; 
   long j,i,n,istep;
   long k,m,mmax;
   float theta;
   float *data;
   static long num_old = 0;
   static long isign_old = 0;
   long trig_step,t;
   long size;
   static float *cosTable;
   static float *sinTable;
         
   data = (float*)((&cmplx[0].r) - 1); // Convert complex array to float 

// Calculate trig look-up table 
   if(nn != num_old || isign != isign_old)
   {
      size = nn*sizeof(float);
   
      theta = 6.283185307171959/nn;

      FreeVector(cosTable);
      if((cosTable = MakeVector(size)) == (float*)0)
      {
          ErrorMessage("Unable to allocate\rmemory for ft");
          return;
      }
      FreeVector(sinTable);
      if((sinTable = MakeVector(size)) == (float*)0)
      {
          ErrorMessage("Unable to allocate\rmemory for ft");
          return;
      }   
      
      for(k = 0; k < nn/2; k++)
      {
         cosTable[k] = (float)cos((theta*k));
         sinTable[k] = (float)isign*sin((theta*k));
      }
   }
   else /* Already made trig look up table */
   {
   }
   
   num_old = nn;
   isign_old = isign;

   
// Divide by nr of complex data points if reverse ft ********
   if(isign == REVERSE)
   {
      for(i = 1; i <= nn*2; i++)
	     data[i] /= nn;
   }

   n = nn << 1; // Number of real and imaginary data points 
   j = 1;

// Swap bits 
   for(i = 1; i < n; i+=2)
   {
      if(j > i)
      {
         SWAP(data[j],data[i]);
         SWAP(data[j+1],data[i+1]);
      }
      m = n >> 1;
      while(m >= 2 && j > m)
      {
         j -= m;
         m >>= 1;
      }
      j += m;
   }

// Perform ft 
   mmax = 2;
   trig_step = nn;
   while(n > mmax)
   {
      istep = mmax<<1;
      t = 0;

      for(m = 1; m < mmax; m+=2)
      {
         if(t == 0 && isign == FORWARD)
         {
	        for(i = m; i <= n; i+= istep)
	        {
	           j = i + mmax;
  
	           tempr = data[j];
	           tempi = data[j+1]; 
	           data[j]   = data[i] - tempr;
	           data[j+1] = data[i+1] - tempi; 
  	           data[i]   += tempr;
	           data[i+1] += tempi;
	        }
         }

         else if(t == nn>>2 && isign == FORWARD)
         {
	        for(i = m; i <= n; i+= istep)
	        {
	           j = i + mmax;
           
	           tempr = -data[j+1];
	           tempi = data[j]; 
	           data[j]   = data[i] - tempr;
	           data[j+1] = data[i+1] - tempi; 
  	           data[i]   += tempr;
	           data[i+1] += tempi;
	        }
        }      
        else /* All other angles and inverse transform */
        {
           {
              fc = cosTable[t];
              fs = sinTable[t];
              
	          for(i = m; i <= n; i+= istep)
	          { 	
	              j = i + mmax;

	              tempr =   fc*data[j] - fs*data[j+1];
	              tempi =   fc*data[j+1] + fs*data[j]; 
	              data[j]   = data[i] - tempr;
	              data[j+1] = data[i+1] - tempi;  
	              data[i]   += tempr;
	              data[i+1] += tempi;
	           }
	        }
         }
         t += trig_step>>1;
      }
      trig_step = trig_step>>1;
      mmax = istep;
   }
}



/*
short
MakeFBPFID(char arg[])
{
   ParamList *fd;
   short nrArgs;
   float *smxdata;
   float angle;
   long width,height,width1d;
   long x,y,k;
   float cosa,sina,value;
   complex *data;

// Get projection angle 

   if(arg[0] == '\0')
   {
      sprintf(arg,"\r\r   angle ... %f",angle);
      if(GetStringFromUser(arg,15) == -1) return(0);
   }
   if((nrArgs = str_scan(arg,"%vf",&angle)) == -1)
   {
       return(-1);
   }
   angle = angle/180*PI;
   
// Get pointer to 2D data and dimensions
  
   fd = &filePar[(**plot2).fileNr];
   
   HLock((Handle)fd->smxHandle);
   smxdata = *(fd->smxHandle);


   width = fd->xDim;
   height = fd->yDim;

// Get 1D pointer and dimension 

   data = *data1DHandle;   
   width1d = (**plot1).maxRect.right;
   
// Make temps for fast calculation 

   cosa = cos(angle);
   sina = sin(angle);
      
// Add to each point in the 2D data set to the 1D data set 

   for(y = 0; y < height; y++)
   {
      for(x = 0; x < width; x++)
      {
         value = smxdata[y*height+x];
         
         k = ((x-width/2)*cosa+(y-height/2)*sina) + width1d/2;
         if(k < width1d && k > 0)
            data[k].r += value;
      }
   }

// Unlock 2D handle 

   HUnlock((Handle)(**plot2).dataHandle);
   
   return(0);
}
 */