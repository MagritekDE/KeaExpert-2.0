#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <shellapi.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#define PI  (3.141592653589793)
#define PI2 (6.283185307179586)
#define NFMAX (16384)  /* maximum filter length */
#define EPS (1.e-30)  /* small number */
#define ITRMAX (100)  /* maximum number of iterations */
#define NBMAX  (100)  /* maximum number of bands */
#define MAG_LENGTH 16384    /* size of magnitude vector to write */

int nfcns,ngrid;
int iext[(NFMAX/2+2)+1];
double alpha[(NFMAX/2+2)+1],des[16*(NFMAX/2+2)+1];
double grid[16*(NFMAX/2+2)+1];
double wt[16*(NFMAX/2+2)+1];
double dev;
double ad[(NFMAX/2+2)+1],x[(NFMAX/2+2)+1],y[(NFMAX/2+2)+1];

short DoRemez(char *);
void remez(double[], int);
void testfn(void);
double gee(int, int);
double d(int, int, int);
void rerror(char[]);
int get_int(char *title_string,int low_limit,int up_limit);
double get_float(char *title_string,double low_limit,double up_limit);
float htotal[2*MAG_LENGTH];
float hin[2*MAG_LENGTH];

short DoRemez(DLLParameters* par, char *parameters) 
{
	short nrArgs;
	Variable *ansVar;
	
	short type;			// variable type i.e. FLOAT, CHARSTRING ...
	
//	ansVar = GetVariable(ALL_VAR,"ans",type);  

	int i,j,k,l;
    int jtype,kup,lband,lgrid,nfilt,nbands;
    int jb,neg,nodd,nm1,nz;
    double delf,fup,temp,change;
    double *mag;

    double deviat[NBMAX+1];
	
	float edge1, edge2, edge3, edge4, edge5, edge6;
	double edge[2*NBMAX+1];

	float fx1, fx2, fx3;
	double fx[NBMAX+1];

	float wtx1, wtx2, wtx3;
	double wtx[NBMAX+1];
    
    static double h[(NFMAX/2+2)+1];

	// Arguments are NumberOfCoefficients, FilterType, NumberOfBands, EdgeFrequenciesForBands, GainOfBands, WeightOfBands
	if((nrArgs = ArgScan(par->itfc, parameters,12,"Parameters required are...","eeeeeeeeeeee","llllffffffff",&nfilt, &jtype, &nbands, &lgrid, &edge1, &edge2, &edge3, &edge4, &fx1, &fx2, &wtx1, &wtx2)) < 0)
		return(nrArgs);

	// Prospa passes these values as 'float' but Remez requires them to be 'double'
	edge[1] = (double)edge1;
	edge[2] = (double)edge2;
	edge[3] = (double)edge3;
	edge[4] = (double)edge4;

	fx[1]   = (double)fx1;
	fx[2]   = (double)fx2;

	wtx[1]  = (double)wtx1;
	wtx[2]  = (double)wtx2;


	/* END INPUT DATA SECTION */

	//if(nfilt > NFMAX){
	//	printf("Error: # coeff > %d\n...now exiting to system...\n",NFMAX);
	//	exit(1);
	//}

	//if(nfilt < 3) rerror("Error: # coeff < 3");
	//if(nbands <= 0) nbands = 1;
	//if(lgrid <= 0) lgrid = 16;

	//printf("#coeff = %d\nType = %d\n#bands =  %d\nGrid = %d\n",
	//	nfilt,jtype,nbands,lgrid);
	//for(i=1;i<=2*nbands;i++) printf("E[%d] = %.2lf\n",i,edge[i]);
	//for(i=1;i<=nbands;i++) printf("Gain, wt[%d] = %.2lf  %.2lf\n",i,fx[i],wtx[i]);
	//TextMessage('\n');

	neg = 1;
	if(jtype == 1) neg = 0;
	nodd  = nfilt % 2;
	nfcns = nfilt / 2;
	if(nodd == 1 && neg == 0) nfcns++;

	/* SET UP THE DENSE GRID.  THE NUMBER OF POINTS IN THE GRID
	/* IS (FILTER LENGTH + 1)*GRID DENSITY/2 */
	grid[1] = edge[1];
	delf = lgrid * nfcns;
	delf = 0.5/delf;
	if(neg != 0 && edge[1] < delf) grid[1] = delf;

	/* CALCULATE THE DESIRED MAGNITUDE RESPONSE AND THE WEIGHT
	/* FUNCTION ON THE GRID */
	j = 1;
	l = 1;
	lband = 1;
	while(1){   /* loop until break */
		fup = edge[l+1];
		do {
			temp = grid[j];
			des[j] = fx[lband];
			if(jtype == 2) des[j] *= temp;
			wt[j] = wtx[lband];
			if(jtype == 2 && fx[lband] >= 0.0001) 
				wt[j] /= temp;
			j++;
			grid[j] = temp + delf;
		} while(grid[j] <= fup);

		grid[j-1] = fup;
		des[j-1] = fx[lband];
		if(jtype == 2) des[j-1] *= fup;
		wt[j-1] = wtx[lband];
		if(jtype == 2 && fx[lband] >= 0.0001) wt[j-1] /= fup;
		lband++;
		l += 2;
		if(lband > nbands) break;
		grid[j] = edge[l];
	}  /* END while(1) */

	ngrid = j-1;
	if(neg == nodd && grid[ngrid] > (0.5 - delf)) ngrid--;

	/* SET UP A NEW APPROXIMATION PROBLEM WHICH IS EQUIVALENT
	/* TO THE ORIGINAL PROBLEM */
	if(neg <= 0){
		if(nodd == 1);    /* DO NOTHING */
		else {
			for(j=1;j <= ngrid;j++){
				change = cos(PI * grid[j]);
				wt[j] *= change;
				if(change == 0.0) change = EPS;
				des[j] /= change;
			}
		}
	}
	else {
		if(nodd == 1) {
			for(j=1; j <= ngrid; j++){
				change = sin(PI2 * grid[j]);
				wt[j] *= change;
				if(change == 0.0) change = EPS;
				des[j] /= change;
			}
		}
		else {
			for(j=1;j <= ngrid;j++){
				change = sin(PI * grid[j]);
				wt[j] *= change;
				if(change == 0.0) change = EPS;
				des[j] /= change;
			}
		}
	}

	/* INITIAL GUESS FOR THE EXTREMAL FREQUENCIES -- EQUALLY SPACED ALONG THE GRID */
	temp = (ngrid - 1)/nfcns;
	for(j=1;j<=nfcns;j++)
		iext[j] = (j-1) * temp + 1;
	iext[nfcns+1] = ngrid;
	nm1 = nfcns - 1;
	nz = nfcns + 1;

	/* CALL THE REMEZ EXCHANGE ALGORITHM TO DO THE APPROXIMATION PROBLEM */
	remez(edge,nbands);

	if(neg <= 0){
		if(nodd == 0) {
			h[1] = 0.25 * alpha[nfcns];
			for(j=2;j<=nm1;j++)
				h[j] = 0.25 * (alpha[nz - j] + alpha[nfcns + 2 - j]);
			h[nfcns] = 0.5 * alpha[1] + 0.25 * alpha[2];
		}
		else {
			for(j=1;j<=nm1;j++) h[j] = 0.5 * alpha[nz - j];
			h[nfcns] = alpha[1];
		}
	}
	else {
		if(nodd == 0) {
			h[1] = 0.25 * alpha[nfcns];
			for(j=2;j<=nm1;j++)
				h[j] = 0.25 * (alpha[nz - j] - alpha[nfcns + 2 - j]);
			h[nfcns] = 0.5 * alpha[1] - 0.25 * alpha[2];
		}
		else {
			h[1] = 0.25 * alpha[nfcns];
			h[2] = 0.25 * alpha[nm1];
			for(j=3;j<=nm1;j++)
				h[j] = 0.25 * (alpha[nz-j] - alpha[nfcns + 3 - j]);
			h[nfcns] = 0.5 * alpha[1] - 0.25 * alpha[3];
			h[nz] = 0.0;
		}
	}

	/* PROGRAM OUTPUT SECTION */
	//putchar('\n');
	//for(j=1;j<=70;j++) putchar('*');
	//putchar('\n');
	//printf("                        FINITE IMPULSE RESPONSE (FIR)\n");
	//printf("                        LINEAR PHASE DIGITAL FILTER DESIGN\n");
	//printf("                        REMEZ EXCHANGE ALGORITHM\n");
	//switch(jtype){
	//case 1: printf("                        BANDPASS FILTER\n");
	//		break;
	//case 2: printf("                        DIFFERENTIATOR\n");
	//		break;
	//case 3: printf("                        HILBERT TRANSFORMER\n");
	//		break;
	//}
	//printf("              FILTER LENGTH = %d\n",nfilt);
	//printf("              ***** IMPULSE RESPONSE *****\n");
	//for(j=1;j<=nfcns;j++){
	//	k = nfilt + 1 - j;
	//	if(neg == 0)
	//		printf("                   H(%3d) = %17.9e = H(%4d)\n",j,h[j],k);
	//	if(neg == 1)
	//		printf("                   H(%3d) = %17.9e = -H(%4d)\n",j,h[j],k);
	//}
	//if(neg == 1 && nodd == 1)
	//	printf("                   H(%3d) =  0.0\n",nz);   /* nz = nfcns + 1 */
	//putchar('\n');
	for(k=1;k<=nbands;k+=4) {
		kup = k + 3;
		if(kup > nbands) kup = nbands;
		for(j=1;j<=23;j++) TextMessage(" ");
		for(j=k;j<=kup;j++) TextMessage("BAND%2d         ",j);
		TextMessage("\n");
		TextMessage(" LOWER BAND EDGE");
		for(j=k;j<=kup;j++) TextMessage("%15.8f",edge[2*j-1]);
		TextMessage("\n");
		TextMessage(" UPPER BAND EDGE");
		for(j=k;j<=kup;j++) TextMessage("%15.8f",edge[2*j]);
		TextMessage("\n");
		if(jtype != 2){
			TextMessage(" DESIRED VALUE  ");
			for(j=k;j<=kup;j++) TextMessage("%15.8f",fx[j]);
			TextMessage("\n");
		}
		if(jtype == 2){
			TextMessage(" DESIRED SLOPE  ");
			for(j=k;j<=kup;j++) TextMessage("%15.8f",fx[j]);
			TextMessage("\n");
		}
		TextMessage(" WEIGHTING      ");
		for(j=k;j<=kup;j++) TextMessage("%15.8f",wtx[j]);
		TextMessage("\n");
		for(j=k;j<=kup;j++) deviat[j] = dev/wtx[j];
		TextMessage(" DEVIATION      ");
		for(j=k;j<=kup;j++) TextMessage("%15.8f",deviat[j]);
		TextMessage("\n");
		if(jtype != 1) continue;
		for(j=k;j<=kup;j++) {
			if(fx[j] > 1e-5) deviat[j] = 20.0 * log10(deviat[j]);
			else  deviat[j] = -40.0 * log10(1.0-deviat[j]);
		}
		TextMessage(" DEVIATION IN DB");
		for(j=k;j<=kup;j++) TextMessage("%15.8f",deviat[j]);
		TextMessage("\n");
		}
		TextMessage("\n");
		TextMessage(" EXTREMAL FREQUENCIES\n");
		TextMessage(" ");
		for(j=1;j<=nz;j++){
		TextMessage("%12.7f",grid[iext[j]]);
		if(!(j%5)){ TextMessage("\n");TextMessage(" ");}  /* CR every 5 columns */
	}
	TextMessage("\n");



	k=0;
	for(j=1;j<=nfcns;j++) {
		htotal[k++] = h[j];
	}
	if(neg == 1 && nodd == 1) 
		htotal[k++] = 0.0;
	if(nodd == 1) 
		j=nfcns-1; 
	else 
		j=nfcns;

	for(;j>=1;j--) {
		if(neg == 0) 
			htotal[k++] = h[j];
		if(neg == 1) 
			htotal[k++] = -h[j];
	}
	for(i=0; i < k ; i++)
		hin[i]=htotal[i];

		//TextMessage("\nFIR coefficients written to text file COEF.DAT\n");
		//chan=fopen("coef.dat","wt");
		//for(i = 0 ; i < k ; i++) {
			//fTextMessage(chan,"\n%15.10f",htotal[i]);
		//}



	// Declare coefficients storage
	//float Coefficients[MAX_SIZE_OF_COEFF_VECTOR];
	float *Coefficients = new float[nfilt];

	// clear array of coefficients
	for (short Count = 0; Count < nfilt; Count++)
		Coefficients[Count] = 0;
//
//	// Create filter object
//	FilterDefinition FilterDef;
//
//	// Assign values to object
//	FilterDef.NumCoeffs   = 24;
//	FilterDef.FilterType  = 1;
//	FilterDef.Bands	      = 2;
//	FilterDef.Edge1	      = 0;
//	FilterDef.Edge2	      = 0.08;
//	FilterDef.Edge3	      = 0.16;
//	FilterDef.Edge4	      = 0.5;
//	FilterDef.Edge5	      = 0;
//	FilterDef.Edge6	      = 0;
//	FilterDef.GainBand1	  = 1;
//	FilterDef.GainBand2	  = 0;
//	FilterDef.GainBand3	  = 0;
//	FilterDef.WeightBand1 = 1;
//	FilterDef.WeightBand2 = 1;
//	FilterDef.WeightBand3 = 0;
//
//	// for test only
//	FilterDef.Edge1 = 1.23456;
//	
////	float p = test(&FilterDef, Coefficients);
//	
////	Coefficients[2] = p;
//
////	Coefficients[3] = (float)Remez::DoRemez(10);
//
//	gee(1,2);
//	Coefficients[4] = testfn2(&FilterDef, Coefficients);
//

	
	
	for (short Count = 0; Count < nfilt ; Count++)
		Coefficients[Count]=htotal[Count];
	
	// array data[] is populated with remez algorithm
	ansVar->MakeMatrix2DFromVector(Coefficients, nfilt, 1);
	
	//// Free allocated memory
	delete [] Coefficients;

	
	return(OK);
}   /* END main() */





/* REMEZ EXCHANGE ALGORITHM */
//void remez(edge,nbands) double edge[];int nbands;{
void remez(double edge[], int nbands) 
{
	int j,k,l;
	int jchnge,jet,jm1,jp1;
	int k1,kkk,klow,kn,knz,kup;
	int loop1,luck;
	int niter,nm1,nu,nut,nut1,nz,nzz;
	int out1,out2,out3;
	double cn,fsh,gtemp;
	double delf,tmp;
	double devl,dtemp,dnum,dden,err;
	double comp,y1,ynz,aa,bb,ft,xt,xe;
	//double d(),gee();
	static double a[67],p[67],q[67];

	devl = -1.0;
	nz = nfcns + 1;
	nzz = nfcns + 2;
	niter = 0;
	comp = 0;
	luck = 0;
	y1 = 0;
	nut1 = 0;

	loop1 = 1;
	TextMessage("Iteration");
	while(niter++ < ITRMAX){   /* LOOP 1 */
	if(loop1 == 0) break;   /* OUT OF LOOP1 */
	TextMessage(" %d",niter);
	if(!(niter%20)) TextMessage("\n");   /* newline every 20 iterations */
	iext[nzz] = ngrid + 1;
	for(j=1;j<=nz;j++) x[j] = cos(PI2 * grid[iext[j]]);
	jet = (int)((nfcns - 1)/15 +1);
	for(j=1;j<=nz;j++) ad[j] = d(j,nz,jet);
	dnum = 0.0;
	dden = 0.0;
	k=1;
	for(j=1;j<=nz;j++){
		l=iext[j];
		dnum += ad[j] * des[l];
		if(wt[l] == 0.0) wt[l] = EPS;
		dden += k*ad[j]/wt[l];
		k = -k;
	}
	dev = dnum/dden;
	nu = 1;
	if(dev > 0.0) nu = -1;
	dev *= -nu;
	k = nu;
	for(j=1;j<=nz;j++){
		l = iext[j];
		if(wt[l] == 0.0) wt[l] = EPS;
		y[j] = des[l] + k*dev/wt[l];
		k = -k;
	}
	if(dev < devl){   /* "ouch" */
		TextMessage("   ********* FAILURE TO CONVERGE **********\n");
		TextMessage("Probable cause is machine rounding error\n");
		TextMessage("The impulse response may be correct\n");
		TextMessage("Check with frequency response\n");
		break;   /* OUT OF LOOP 1 */
	}
	devl = dev;
	jchnge = 0;
	k1 = iext[1];
	knz = iext[nz];
	klow = 0;
	nut = -nu;
	j = 1;

	/* SEARCH FOR THE EXTREMAL FREQUENCIES OF THE BEST APPROXIMATION */
	while(1){   /* LOOP 2 */
		if(j == nzz) ynz = comp;
		if(j >= nzz){
			if(j > nzz){
				if(luck > 9){
				kn = iext[nzz];
				for(j=1;j<=nfcns;j++) iext[j] = iext[j+1];
				iext[nz] = kn;
				break;   /* OUT OF LOOP 2 */
				}
				else{
				if(comp > y1) y1 = comp;
				k1 = iext[nzz];
	lable325:   l = ngrid + 1;    /* label325 ? */
				klow = knz;
				nut = -nut1;
				comp = y1 * (1.00001);
				out1 = 0;
				out2 = 0;
				while(1){   /* LOOP 3 */
					l--;
					if(l <= klow){
						out1 = 1;
						break;   /* out of LOOP 3 */
					}
					else{
						err = (gee(l,nz) - des[l]) * wt[l];
						dtemp = nut * err - comp;
						if(dtemp > 0.0){           /* BREAK OUT OF LOOP 3 */
							out2 = 1;
							break;   /* out of LOOP 3 */
						}
					}
				}     /* END OF while LOOP 3 */
				if(out1) {
					if(luck == 6){
						if(jchnge > 0) break;   /* OUT OF LOOP 2 */
						loop1 = 0;
						break;            /* OUT OF LOOP 2 and LOOP 1 */
					}
					for(j=1;j<=nfcns;j++) iext[nzz - j] = iext[nz - j];
					iext[1] = k1;
					break;   /* OUT OF LOOP 2 */
				}  /* END OF if(out1) */
				if(out2){
					j = nzz;
					comp = nut * err;
					luck += 10;
					goto lable235;
				}
				}     /* END OF if(luck > 9) else */
			}        /* END OF if(j > nzz) */
			else {
				if(k1 > iext[1]) k1 = iext[1];
				if(knz < iext[nz]) knz = iext[nz];
				nut1 = nut;
				nut = -nu;
				l = 0;
				kup = k1;
				comp = ynz * (1.00001);
				luck = 1;
				out1 = 0;
				out2 = 0;
				while(1){   /* LOOP 4 */
					l++;
					if(l >= kup){
						out1 = 1;
						break;   /* out of LOOP 4 */
					}
					err = (gee(l,nz) - des[l]) * wt[l];
					dtemp = nut * err - comp;
					if(dtemp > 0.0){           /* BREAK OUT OF LOOP 4 */
						out2 = 1;
						break;   /* out of LOOP 4 */
					}
				}     /* END OF while LOOP 4 */
				if(out1){
					luck = 6;
					goto lable325;
				}
				if(out2){
				j = nzz;
				comp = nut * err;
				goto lable210;
				}
			}        /* END OF if(j > nzz) else */
		}           /* END OF if(j >= nzz) */
		else {
			kup = iext[j+1];
			l = iext[j] + 1;
			nut = - nut;
			if(j == 2) y1 = comp;
			comp = dev;
			if(l >= kup){
	lable220:   l--;
				out1 = 0;
				out2 = 0;
				out3 = 0;
				while(1){         /* LOOP 5 */
				l--;
				if(l <= klow){
					out1 = 1;
					break;   /* OUT OF LOOP 5 */
				}
				err = (gee(l,nz) - des[l]) * wt[l];
				dtemp = nut * err - comp;
				if(dtemp > 0.0){
					out2 = 1;
					break;   /* OUT OF LOOP 5 */
				}
				if(jchnge > 0){
					out3 = 1;
					break;   /* OUT OF LOOP5 */
				}
				}  /* END OF while LOOP 5 */
				if(out1){   /* if we exited LOOP 5 this way */
				l = iext[j] + 1;
				if(jchnge > 0){
					iext[j] = l - 1;
					j++;
					klow = l - 1;
					jchnge++;
					continue;   /* to top of LOOP 2 */
				}
				out1 = 0;
				out2 = 0;
				while(1){   /* LOOP 6 */
					l++;
					if(l >= kup){
						out1 = 1;
						break;   /* OUT OF LOOP 6 */
					}
					err = (gee(l,nz) - des[l]) * wt[l];
					dtemp = nut * err - comp;
					if(dtemp > 0.0){           /* BREAK OUT OF LOOP 6 */
						comp = nut * err;
						out2 = 1;
						break;   /* OUT OF LOOP 6 */
					}
				}           /* END OF while LOOP 6 */
				if(out1){
					klow = iext[j];
					j++;
					continue;   /* to top of LOOP 2 */
				}
				if(out2) goto lable210;
				}        /* END OF if(out1) */
				else if(out2){
				comp = nut * err;
	lable235:      while(1){         /* LOOP 7 */
					l--;
					if(l <= klow) break;          /* OUT OF LOOP 7 */
					err = (gee(l,nz) - des[l]) * wt[l];
					dtemp = nut * err - comp;
					if(dtemp <= 0.0) break;       /* OUT OF LOOP 7 */
					comp = nut * err;
				}     /* END OF while LOOP 7 */
				klow = iext[j];
				iext[j] = l+1;
				j++;
				jchnge++;
				continue;   /* to top of LOOP 2 */
				}  /* END OF else if(out2) */
				else if(out3){
				klow = iext[j];
				j++;
				continue;   /* to top of LOOP 2 */
				}
			}     /* END OF if(l >= kup) */
			else{
				err = (gee(l,nz) - des[l]) * wt[l];
				dtemp = nut * err - comp;
				if(dtemp <= 0.0) goto lable220;
				comp = nut * err;
	lable210:   while(1){             /* LOOP 8 */
					l++;
					if(l >= kup) break;  /* OUT OF LOOP 8 */
					err = (gee(l,nz) - des[l]) * wt[l];
					dtemp = nut * err - comp;
					if(dtemp <= 0.0) break; /* OUT OF LOOP 8 */
					comp = nut * err;
				}
				iext[j] = l - 1;
				j++;
				klow = l - 1;
				jchnge++;
				continue;   /* to top of LOOP 2 */
			}
		}        /* END OF if(j >= nzz) else */
	}  /* END OF while LOOP 2 */
	}     /* END OF while LOOP 1 */


	/* CALCULATION OF THE COEFFICIENTS OF THE BEST APPROXIMATION
	/* USING THE INVERSE DISCRETE FOURIER TRANSFORM */

	lable400:
	nm1 = nfcns - 1;
	fsh = 1.0e-6;
	gtemp = grid[1];
	x[nzz] = -2.0;
	cn = 2 * nfcns - 1;
	delf = 1.0 / cn;
	l = 1;
	kkk = 0;
	if(edge[1] < 0.01 && edge[2*nbands] > 0.49) 
		kkk = 1;
	if(nfcns <= 3) 
		kkk = 1;
	if(kkk != 1) {
		dtemp = cos(PI2 * grid[1]);
		dnum = cos(PI2 * grid[ngrid]);
		tmp = dtemp - dnum;
		if(tmp == 0.0) tmp = EPS;
		aa = 2.0/tmp;
		bb = -(dtemp + dnum)/tmp;
	}

	for(j=1;j<=nfcns;j++){
		ft = (j - 1) * delf;
		xt = cos(PI2 * ft);
		if(kkk != 1){
			xt = (xt - bb)/aa;
			ft = acos(xt)/PI2;
		}
		out1 = 0;
		out2 = 0;
		while(1){         /* LOOP 9 */
			xe = x[l];
			if(xt > xe){
				out1 = 1;
				break;   /* OUT OF LOOP 9 */
			}
			if((xe - xt) < fsh){
				out2 = 1;
				break;   /* OUT OF LOOP 9 */
			}
			l++;
		}     /* END OF while LOOP 9 */
		if(out1){
			if((xt - xe) < fsh) a[j] = y[l];
			else{
				grid[1] = ft;
				a[j] = gee(1,nz);
			}
		}
		if(out2) a[j] = y[l];
		if(l > 1) l--;
	}  /* END for() LOOP */

	grid[1] = gtemp;
	dden = PI2/cn;

	for(j=1;j<=nfcns;j++){
		dtemp = 0.0;
		dnum = (j-1) * dden;
		if(nm1 >= 1) for(k=1;k<=nm1;k++)
			dtemp += a[k+1] * cos(dnum * k);
		alpha[j] = 2 * dtemp + a[1];
	}  /* END for() LOOP */

	for(j=2;j<=nfcns;j++) alpha[j] *= 2/cn;
	alpha[1] /= cn;
	if(kkk != 1){
		p[1] = 2.0 * alpha[nfcns] * bb + alpha[nm1];
		p[2] = 2.0 * alpha[nfcns] * aa;
		q[1] = alpha[nfcns - 2] - alpha[nfcns];
		for(j=2;j<=nm1;j++){
			if(j >= nm1){
				aa *= 0.5;
				bb *= 0.5;
			}
			p[j+1] = 0.0;
			for(k=1;k<=j;k++){
				a[k] = p[k];
				p[k] = 2.0 * bb * a[k];
			}
			p[2] += 2.0 * aa * a[1];
			jm1 = j-1;
			for(k=1;k<=jm1;k++) p[k] += q[k] + aa * a[k+1];
			jp1 = j + 1;
			for(k=3;k<=jp1;k++) p[k] += aa * a[k-1];
			if(j != nm1){
				for(k=1;k<=j;k++) q[k] = - a[k];
				q[1] += alpha[nfcns - 1 - j];
			}
		}     /* END OF for() LOOP */

		for(j=1;j<=nfcns;j++) 
			alpha[j] = p[j];
	}     /* END OF if(kkk != 1) */

	if(nfcns <= 3){
		alpha[nfcns + 1] = 0.0;
		alpha[nfcns + 2] = 0.0;
	}
}     /* END REMEZ EXCHANGE ALGORITHM */




/* FUNCTION TO EVALUATE THE FREQUENCY RESPONSE USING THE
/* LAGRANGE INTERPOLATION FORMULA IN THE BARYCENTRIC FORM */
double gee(int k, int n) 
{
   int j;
   double c,de,p,xf;

   de = 0.0;
   p = 0.0;
   xf = cos(PI2*grid[k]);

   for(j=1;j<=n;j++){
      c = xf - x[j];
      if(c == 0.0) c = EPS;
      c = ad[j]/c;
      de += c;
      p += c * y[j];
   }
   if(de == 0.0) de = EPS;
   return(p/de);
}



/* FUNCTION TO CALCULATE THE LAGRANGE INTERPOLATION
/* COEFFICIENTS FOR USE IN THE FUNCTION GEE */
double d(int k, int n, int m) 
{
   int j,el;
   double de,q;

   de = 1.0;
   q = x[k];

   for(el=1;el<=m;el++)
      for(j=el;j<=n;j+=m)
         if(j != k) de *= 2.0 * (q - x[j]);

   if(de == 0.0) de = EPS;
   return(1.0/de);
}


void rerror(char error_text[]) 
{
   TextMessage("%s\n",error_text);
   TextMessage("...now exiting to system...\n");
   exit(1);
}

/* FLOAT INPUT */
double get_float(char *title_string,double low_limit,double up_limit)
{
    double i;
    int error_flag;
    char *endcp;
    char ctemp[128];
    if(low_limit > up_limit)
	 {
        TextMessage("\nLimit error lower > upper\n");
		return(ERR);
	 }
    do 
	 {
        TextMessage("\n%s [%G to %G] ? ", title_string, low_limit, up_limit);
        gets_s(ctemp);
        i = strtod(ctemp, &endcp);
        error_flag = (ctemp == endcp) || (*endcp != '\0');
    } 
	 while (i < low_limit || i > up_limit || error_flag);
    return(i);
}

/* int INPUT */
int get_int(char *title_string,int low_limit,int up_limit)
{
    int i;
    int error_flag;
    char *endcp;
    char ctemp[128];
    if(low_limit > up_limit)
	 {
        TextMessage("\nLimit error lower > upper\n");
		return(ERR);
	 }
    do 
	 {
        TextMessage("\n%s [%d to %d] ? ", title_string, low_limit, up_limit);
        gets_s(ctemp);
        i = strtol(ctemp,&endcp,10);
        error_flag = (ctemp == endcp) || (*endcp != '\0');
    }
	 while (i < low_limit || i > up_limit || error_flag);
    return(i);
}


