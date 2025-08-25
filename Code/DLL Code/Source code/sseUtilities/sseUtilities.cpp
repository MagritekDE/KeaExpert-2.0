
#include "../Global files/includesDLL.h"


// Locally defined procedure and global variables
EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short IntegerToTemperature(DLLParameters*,char *args);
int GetRFPulse(DLLParameters* par, char* args);
int GetRFPulseSlice(DLLParameters* par, char* args);


/*******************************************************************************
	Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
  short r = RETURN_FROM_DLL;

       if(!strcmp(command,"inttotemp"))           r = IntegerToTemperature(dpar,parameters); 
		 else if (!strcmp(command, "getwetpar1"))   r = GetRFPulse(dpar, parameters);
		 else if (!strcmp(command, "getwetpar2"))   r = GetRFPulseSlice(dpar, parameters);

  return(r);
}

/*******************************************************************************
	Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
  TextMessage("\n\n   SSE utilities DLL module (V1.0)\n\n");
  TextMessage("   inttotemp ...... convert an integer returned by the temp controller to a temperature in Celsius\n");
  TextMessage("   getwetpar1 ..... function for WET-Suppression sequences\n");
  TextMessage("   getwetpar2 ..... function for WET-Suppression sequences\n");
}

/*******************************************************************************
	Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
  syntax[0] = '\0';
       if(!strcmp(cmd,"inttotemp"))           strcpy(syntax,"DOUBLE tempC = inttotemp(INT x)");
		 else if (!strcmp(cmd, "getwetpar1"))   strcpy(syntax, "(tphase, tamp, n300, d300) = getwetpar1(, w3, Osup1, aSup1, useSup1, Osup2, aSup2, useSup2, Osup3, aSup3, useSup3, b1Freq, decoupleAmpPre, decouplePre, pulseLengthH90)");
		 else if (!strcmp(cmd, "getwetpar2"))   strcpy(syntax, "(tphase, tamp, n301, d301) = getwetpar2(180Amplitude, pulseLengthInv, freqAmp, freqOffset)");

  if(syntax[0] == '\0')
    return(false);
  return(true);
}


/*********************************************************************************
	This function convert a number returned by the temperature controller into 
	a temperature in Celsius (double precision). The input can be float or double.
	An invalid input will return -273 C.
*********************************************************************************/

short IntegerToTemperature(DLLParameters* par, char *args)
{
  short nrArgs;
  Variable var;
  long type;

  if((nrArgs = ArgScan(par->itfc,args,1,"integer to be converted","e","v",&var)) < 0)
    return(nrArgs);  

  type = var.GetType();
  // See if its a float/double variable   
	switch(type)
	{
		case(FLOAT32): 
		case(FLOAT64): 
		{
			double input;
			par->nrRetVar = 1;
			if(type == FLOAT32)
				input = (int)(var.GetReal() + 0.5);
			else
				input = (int)(var.GetDouble() + 0.5);
			double A = -5.775e-7;
			double B = 3.9083e-3;
			double R0 = 100.0;
			double Rt = 125.0 * input / (1<<24);
			double C = R0 - Rt;
			double D = R0 * B * R0 * B - 4 * R0 * A * C;
			if(D < 0)
			{
				par->retVar[1].MakeAndSetDouble(-273.0);
				return(OK);
			}
			double t = (-R0 * B + sqrt(D)) / (2.0 * R0 * A);
			par->retVar[1].MakeAndSetDouble(t);
			break;
		}
  
     default:
	  {
       ErrorMessage("Argument to 'inttofloat' should be a float or double");
       return(ERR);
	  }
  } 

  return(OK);
}


/*********************************************************************************
	Hidden WET suppression commands
	getRFPulse and getRFPulseSlice renamed
	getwetpar1 and getwetpar2
*********************************************************************************/

char getRFPulse[] =
"procedure(getwetpar1, w3, Osup1, aSup, useSup1, Osup2, useSup2, Osup3, useSup3, b1Freq, decoupleAmpPre, decouplePre, pulseLengthC90, centerFreq1H); \
	d300 = pulseLengthC90;\
	n300 = round(w3/d300); \
	sp = cmatrix(n300); \
	a1 = 0; \
	a2 = 0; \
	a3 = 0; \
	if (useSup1 == \"yes\"); \
		a1 = ucsRun:convertTxGain(aSup); \
	endif; \
	if (useSup2 == \"yes\"); \
		a2 = ucsRun:convertTxGain(aSup); \
	endif; \
	if (useSup3 == \"yes\"); \
		a3 = ucsRun:convertTxGain(aSup); \
	endif; \
	if (decouplePre == \"yes\"); \
		decoupleAmpPre = decoupleAmpPre; \
	else; \
		decoupleAmpPre = -85; \
	endif; \
	aDec = ucsRun:convertTxGain(decoupleAmpPre); \
	b1Freq = single(b1Freq); \
	Osup1 = -(centerFreq1H - Osup1)*b1Freq; \
	Osup2 = -(centerFreq1H - Osup2)*b1Freq; \
	Osup3 = -(centerFreq1H - Osup3)*b1Freq; \
	xmax = 1.5; \
	k = [0:n300 - 1]; \
	sp = exp(-(d300*(k - n300/2))^2/(w3/(2*xmax))^2).*(a1*exp(i*2*pi*Osup1*d300*1e-6*k) + a2*exp(i*2*pi*Osup2*d300*1e-6*k) + a3*exp(i*2*pi*Osup3*d300*1e-6*k));\
	tampPresat = mag(sp); \
	tphasePresat = 32768/pi*phase(sp); \
	tampDec = matrix(n300); \
	tphaseDec = matrix(n300); \
	tampDec = tampDec + aDec; \
	Q = [2, 2, 2, 0, 0, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2]; \
	q = mag((Q - 2)); \
	sizeQ = size(Q); \
	for (k = 0 to n300/(sizeQ*4) step 4); \
		for (m = 0 to sizeQ-1); \
			tphaseDec[(k + 0)*sizeQ + m] = Q[m]; \
			tphaseDec[(k + 1)*sizeQ + m] = q[m]; \
			tphaseDec[(k + 2)*sizeQ + m] = q[m]; \
			tphaseDec[(k + 3)*sizeQ + m] = Q[m]; \
		next(m); \
	next(k); \
	tphaseDec = tphaseDec*32768/2; \
	tAmpPhase = gFX3->interleaveTables(tampPresat,tphasePresat,tampDec,tphaseDec); \
endproc(tAmpPhase, n300, d300);";



int GetRFPulse(DLLParameters* par, char* args)
{
	UpdateProspaArgumentVariables(par->itfc, args);
	short r = ProcessMacroStr(par->itfc, false, getRFPulse);
	UpdateProspaReturnVariables(par, par->itfc);
	return(r);
}

char getRFPulseSlice[] =
"procedure(getwetpar2, 180Amplitude, pulseLengthInv, freqAmp, freqOffset); \
	d301 = 10; \
	n301 = trunc(pulseLengthInv*1000/d301); \
	N = n301; \
	sp = matrix(N); \
	freq = matrix(N); \
	phasetable = cmatrix(N); \
	xmax = 2.7; \
	total = 0; \
	for(k = 0 to N - 1); \
		x = -xmax + k*2*xmax/(N - 1); \
		sp[k] = exp(-x^2);\
		total = total + sp[k]^2;\
		freq[k] = total;\
	next(k);\
	freq = (freq - freq[N - 1]/2)/freq[N - 1]*2;\
	tfreq = (freq - freqOffset/1000) * freqAmp/1000;\
	ph0 = 0;\
	for(k = 0 to N - 1);\
		phasetable[k] = exp(i*2*pi*tfreq[k]*d301 + i*ph0);\
		ph0 = phase(phasetable[k]);\
	next(k);\
	tamp = ucsRun:convertTxGain(180Amplitude)*mag(sp);\
   tamp0 = 0*tamp;\
	tphase = 32768/pi*phase(phasetable);\
   tAmpPhase1 = gFX3->interleaveTables(tamp0,tphase);\
	tAmpPhase2 = gFX3->interleaveTables(tamp, tphase);\
endproc(tAmpPhase1, tAmpPhase2, n301, d301);";

int GetRFPulseSlice(DLLParameters* par, char* args)
{
	UpdateProspaArgumentVariables(par->itfc, args);
	short r = ProcessMacroStr(par->itfc, false, getRFPulseSlice);
	UpdateProspaReturnVariables(par, par->itfc);
	return(r);
}
