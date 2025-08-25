

#include "../Global files/includesDLL.h"

//#include <atlcomcli.h>



// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);


int GetRFPulseSlice(DLLParameters* par, char* args);

FILE *gFile = NULL;

char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list

/*******************************************************************************
Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
  short r = RETURN_FROM_DLL;

  if(!strcmp(command,"getRFPulseSlice"))           r = GetRFPulseSlice(dpar,parameters);
 
  return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
  TextMessage("\n\n   PSCommands DLL module\n\n");
  TextMessage("   getRFPulseSlice ..... function for WET-Suppression sequences\n");

}

/*******************************************************************************
Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
  syntax[0] = '\0';
  if(!strcmp(cmd,"getRFPulseSlice"))  strcpy(syntax,"(tphase, tamp, n301, d301) = getRFPulseSlice(180Amplitude, pulseLengthInv, freqAmp, freqOffset)");
 
  if(syntax[0] == '\0')
    return(false);
  return(true);
}

//
char TestMacro[] =
"procedure(testProc, N);\
   print(N);\
   print(vartype(N));\
   a = [1:N];\
   b = a*2;\
   print(b);\
endproc(b);";

char getRFPulseSlice[] =
"procedure(getRFPulseSlice, 180Amplitude, pulseLengthInv, freqAmp, freqOffset); \
d301 = 2   # us; \
n301 = trunc(pulseLengthInv * 1000 / d301); \
N = n301; \
sp = matrix(N); \
freq = matrix(N); \
phasetable = cmatrix(N); \
xmax = 2.7; \
total = 0; \
for (k = 0 to N - 1); \
x = -xmax + k * 2 * xmax / (N - 1); \
sp[k] = exp(-x ^ 2);\
total = total + sp[k] ^ 2;\
freq[k] = total;\
next(k);\
freq = (freq - freq[N - 1] / 2) / freq[N - 1] * 2;\
tfreq = (freq - freqOffset / 1000) * freqAmp / 1000  # MHz;\
ph0 = 0;\
for (k = 0 to N - 1);\
phasetable[k] = exp(i * 2 * pi * tfreq[k] * d301 + i * ph0);\
ph0 = phase(phasetable[k]);\
next(k);\
tamp = ucsRun:convertTxGain(180Amplitude) * mag(sp);\
tphase = 32768 / pi * phase(phasetable);\
endproc(tphase, tamp, n301, d301);";


int GetRFPulseSlice(DLLParameters* par, char* args)
{
   UpdateProspaArgumentVariables(par->itfc, args);

   // Run the procedure and return any results
   short r = ProcessMacroStr(par->itfc, false, TestMacro);

   return(OK);
}

