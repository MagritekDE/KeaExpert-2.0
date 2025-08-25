/***************************************************************************************************
 Code to allow access to the FX3 based Kea and generation of pulse programs



 TRex/FX3 pulse program commands

 acquire
 acquireon/off
 cleardata
 decindex
 delay
 dualshapedrf1/2
 gradon/off
 gradramp
 endloop
 endpp
 incindex
 initpp
 loop
 pulse
 report
 setindex
 setrxfreq
 settxfreq
 settxfreqs
 shapedrf1/2
 shim16
 skiponfalse/zero
 ttlon/off
 txon/off
 wait

 Pulse program organisation

 The pulse program running in the TRex is an event table. Each event consists of 96 bits

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)

An event will last for the number of clock cycles specified in the duration field before moving
onto the next event. Looping or skiponzero can cause the event order to change.

The organisation of the arguments depends on the command. Possible command codes are currently

0x00 - Write to TRex address
0x01 - Read from TRex address
0x02 - No operation
0x03 - Reset pulse sequence
0x04 - Loop command
0x05 - End loop command
0x06 - Kea rxGain
0x07 - Kea gradient
0x08 - Spinsolve rxGain and channel selection
0x09 - Delay or nop
0x0A - Skip command
0x0C - Send trigger pulse to backplane
0x0D - Spinsolve shim ramp controller
0x0E - Spinsolve shim controller

Addresses are 16 bits with the top 4 bits specifying the TRex module

0x0 - Legacy
0x1 - DDS
0x2 - ADC
0x3 - BRAM
0x4 - DDC
0x5 - BRAM-Block
0x6 - FIFO
0x7 - uBlaze

Minimum read/write durations are 15 (150 ns). For table based versions of these commands use 

Detailed command syntax (durations are the minimum nr of clock cycles of 10 ns)

Write to a TRex address:

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
       15          0x00        adrs (12)       value (16)

Write to a TRex address the value in the write table buffer

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
       20          0x00        adrs (12)      0x01000000

Write to a TRex address the value in the write table buffer plus an increment

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
		 20          0x00        adrs (12)      0x02000000 + inc

Write to a TRex address the value in the read table buffer minus a decrement

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
		 20          0x00        adrs (12)      0x03000000 - dec

Read from a TRex address and store in write table buffer and shim16 table buffer

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
		 15          0x01        adrs (12)      0x00000000

Read from an address and save in write buffer

 Duration (32)   Command (8)  Argument1 (24)  Argument2 (32)
		 20          0x01        adrs (12)      0x00000000




**************************************************************************************************/


/**************************************************************************************************
	Version history

V0.9
   1. The delay command now supports table entry
	2. The delay command now support times up to 42 seconds
	3. The wait command now supports times >> 21 seconds
	4. Event table now reports command duration in annotation field
	5. The shim16 command now supports amplitude tables
	6. The pulse command now supports multiple table entries
	7. The gradon command now supports amplitude tables

V1.0
   1. Addition of trigger command
	2. Acqure sum mode now has 4 k points available and a shift value for scaling

V1.1
  1. Waitfortrigger can now have mode as a user variable

V2.0
  1. Port from Spinsolve to Kea code, removal of Spinsolve only commands
  2. Replacement of explicit event push commands with AddEvent function and general tidyup

	Last modified 29 July 2025

**************************************************************************************************/


#pragma pack(push, 8)
#include <wtypes.h>
#pragma pack(pop)

#include "../Global files/includesDLL.h"
#include "stdafx.h"
#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>


using namespace std;

#define VERSION 2.0

#define MAX_STACK_SIZE 4096      // Maximum number of points which can be summed or stacked
#define MAX_DATA_POINTS 128*1024 // Maximum number of complex data points which can be collected

typedef unsigned long uint32;
typedef signed long int32;
typedef struct ETInfo
{
	uint32 lineNr;
	CText str;
} ETInfo;

vector<ETInfo> eventTableInfo;

// DLL helper functions
EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);

// FX3 PS commands
short InitialisePP(DLLParameters*, char*);
short Acquire(DLLParameters*, char*);
short AcquireOn(DLLParameters*, char*);
short AcquireOff(DLLParameters*, char*);
short ClearData(DLLParameters*, char*);
short ChirpedRF2(DLLParameters*, char*);
short Delay(DLLParameters*, char*);
short GetPPVersion(DLLParameters*, char*);
short IncTableIndex(DLLParameters* par, char* args);
short DecrementTableIndex(DLLParameters* par, char* args);
short IncRxFrequency(DLLParameters* par, char* args);
short IncTxAmplitude(DLLParameters* par, char* args);
short IncTxFrequency(DLLParameters* par, char* args);
short LoopEnd(DLLParameters*, char*);
short LoopStart(DLLParameters*, char*);
short RFPulse(DLLParameters* par, char* args);
short RFPulseBoost(DLLParameters* par, char* args);
short SinglePulse(DLLParameters* par, char* args);
short DualPulse(DLLParameters* par, char* args);
short DualShapedRFPulse1(DLLParameters* par, char* args);
short DualShapedRFPulse2(DLLParameters* par, char* args);
short GradientRamp(DLLParameters* par, char* args);
short TxOn(DLLParameters* par, char* args);
short TxOff(DLLParameters* par, char* args);
short TTLOn(DLLParameters* par, char* args);
short TTLOff(DLLParameters* par, char* args);
short SetTableIndex(DLLParameters*, char*);
short SetRxFreq(DLLParameters* par, char* args);
short SetTxFreq(DLLParameters* par, char* args);
short SetTxFreqs(DLLParameters* par, char* args);
short SetRxGain(DLLParameters* par, char* args);
short SelectRxAmplifier(DLLParameters* par, char* args);
short ShapedRFPulse(DLLParameters* par, char* args);
short ShapedRFPulse1(DLLParameters* par, char* args);
short ShapedRFPulse2(DLLParameters* par, char* args);
short SkipOnZero(DLLParameters*, char*);
short SkipEnd(DLLParameters*, char*);
short GradientOn(DLLParameters* par, char* args);
short GradientOff(DLLParameters* par, char* args);
short Wait(DLLParameters*, char*);
short UpdateEventTable(DLLParameters* par, char* args);
short EndPP(DLLParameters* par, char* args);
short UnpackData(DLLParameters* par, char* args);
short PackPS(DLLParameters* par, char* args);
short SetUpdateTableMode(DLLParameters* par, char* args);
short GetHelpFolder(DLLParameters* par, char* args);
short WaitForTrigger(DLLParameters* par, char* args);

// FX3 PS utilities
short Report(DLLParameters* par, char* args);
short Report2(DLLParameters* par, char* args);

uint32 ConvertTxGain(float ampdB);
void ConvertFrequency(double frequencyMHz, uint32& digFreq1, uint32& digFreq2);
long GetAcqTime(float dwellTime, long nrPnts, bool useFIRFilter);

// General utilities
double GetMsTime();
long nint(float num);
long nint(double num);
uint32 nuint(float num);
uint32 nuint(double num);

void InsertUniqueStringIntoList(char* str, char*** list, long& position);
bool IsSequenceVariable(char *varName, long& pos);
bool IsSequenceParameter(char* varName, long& pos);
void AddToVariableUpdateTable(long varPos, long adrs, long value);
void AddToFixedUpdateTable(long special, long varPos, long adrs, long value);
void AddPhaseToUpdateTable(char* phaseName, long adrs, long value);
void EvaluateArg(void* itfc, char* varName, short varType, float &result);
void EvaluateArg(void* itfc, char* varName, short varType, double &result);
void EvaluateArg(void* itfc, char* varName, short varType, uint32& result);
void EvaluateArg(void* itfc, char* varName, short varType, int32& result);
void EvaluateArg(void* itfc, char* varName, short varType, uint32& tableAdrs, uint32& tableEntries);
short EvaluateArg(void* itfc, char* varName, Variable* resultVar);
void EvaluateArg(void* itfc, char* varName, short varType, CText& result);

// Global variables
char **parList; // Parameter list - built up by pp commands
int32 szList;    // Number of entries in parameter list
std::vector<uint32> loopNrStk; // Loop number stack
//uint32 loopStackCnt = 0;

typedef struct SkipInfo
{
	CText skipName;
	uint32 skipStartLine;
	uint32 skipEndLine;
	uint32 skipValue;
}
SkipInfo;

std::vector<SkipInfo> skipInfo; // Skip number stack


Variable* psVariables; // List of ps parameters which will be modified
Variable* ppParameterList; // Complete list of ps parameters

// TRex addresses
const uint32 FPGA_Sync      = 0x0080;
const uint32 FPGA_DDS_CR    = 0x1081;

const uint32 FPGA_DDS1_CFR1 = 0x1180;
const uint32 FPGA_DDS1_CFR2 = 0x1181;
const uint32 FPGA_DDS1_FTW  = 0x1187;
const uint32 FPGA_DDS1_POW  = 0x1188;
const uint32 FPGA_DDS1_Pro0 = 0x118E;
const uint32 FPGA_DDS1_Pro1 = 0x118F;
const uint32 FPGA_DDS1_Pro2 = 0x1190;
const uint32 FPGA_DDS1_RR   = 0x1196;

const uint32 FPGA_DDS2_CFR1 = 0x1280;
const uint32 FPGA_DDS2_CFR2 = 0x1281;
const uint32 FPGA_DDS2_FTW  = 0x1287;
const uint32 FPGA_DDS2_POW  = 0x1288;
const uint32 FPGA_DDS2_Pro0 = 0x128E;
const uint32 FPGA_DDS2_Pro1 = 0x128F;
const uint32 FPGA_DDS2_Pro2 = 0x1290;
const uint32 FPGA_DDS2_RR   = 0x1296;

int32 gCurGradLevel = 0; // Used to check for non-zero gradient at end of sequence

bool gGenerateFullUpdateTable = false; // Whether the update table should be generated

/*******************************************************************************
Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;


	if(!strcmp(command, "acquire"))                 r = Acquire(dpar,parameters);
   else if (!strcmp(command, "acquireon"))         r = AcquireOn(dpar,parameters);  
   else if (!strcmp(command, "acquireoff"))        r = AcquireOff(dpar,parameters); 
   else if (!strcmp(command, "chirprf2"))          r = ChirpedRF2(dpar,parameters);      
   else if (!strcmp(command, "cleardata"))         r = ClearData(dpar,parameters);      
	else if (!strcmp(command, "delay"))             r = Delay(dpar, parameters);
	else if (!strcmp(command, "report"))            r = Report(dpar, parameters);
	else if (!strcmp(command, "report2"))           r = Report2(dpar, parameters);
	else if (!strcmp(command, "decindex"))          r = DecrementTableIndex(dpar,parameters);
	else if (!strcmp(command, "dualshapedrf1"))     r = DualShapedRFPulse1(dpar, parameters);
	else if (!strcmp(command, "dualshapedrf2"))     r = DualShapedRFPulse2(dpar, parameters); 
   else if (!strcmp(command, "endloop"))           r = LoopEnd(dpar,parameters);     
   else if (!strcmp(command, "endpp"))             r = EndPP(dpar,parameters);  
	else if (!strcmp(command, "endskip"))           r = SkipEnd(dpar, parameters);
	else if (!strcmp(command, "endiftrue"))         r = SkipEnd(dpar, parameters);
	//  else if(!strcmp(command,"execwait"))          r = ExecuteAndWait(dpar,parameters);
	else if (!strcmp(command, "gradon"))            r = GradientOn(dpar, parameters);
	else if (!strcmp(command, "gradoff"))           r = GradientOff(dpar,parameters);
   else if (!strcmp(command, "gradramp"))          r = GradientRamp(dpar,parameters);
   else if(!strcmp(command,  "helpfolder"))        r = GetHelpFolder(dpar,parameters);  
   else if (!strcmp(command, "incindex"))          r = IncTableIndex(dpar,parameters);
   else if (!strcmp(command, "inctxamp"))          r = IncTxAmplitude(dpar,parameters);   
   else if (!strcmp(command, "incrxfreq"))         r = IncRxFrequency(dpar,parameters);   
   else if (!strcmp(command, "inctxfreq"))         r = IncTxFrequency(dpar,parameters);
	else if (!strcmp(command, "initpp"))            r = InitialisePP(dpar,parameters);  
   //else if(!strcmp(command,"lockoff"))           r = Lockoff(dpar,parameters);     
   //else if(!strcmp(command,"lockon"))            r = Lockon(dpar,parameters);  
   else if (!strcmp(command, "loop"))              r = LoopStart(dpar,parameters);     
   //else if(!strcmp(command,"memreset"))          r = ResetMemoryPointer(dpar,parameters);     
   //else if(!strcmp(command,"nop"))               r = NoOperation(dpar,parameters);     
 	else if (!strcmp(command, "packps"))            r = PackPS(dpar, parameters);
  // else if (!strcmp(command, "ppversion"))         r = GetPPVersion(dpar,parameters);    
   else if (!strcmp(command, "pulse"))             r = RFPulse(dpar,parameters);   
	else if (!strcmp(command, "pulseboost"))        r = RFPulseBoost(dpar, parameters);

   else if (!strcmp(command, "selectrxamp"))       r = SelectRxAmplifier(dpar,parameters);   
   else if (!strcmp(command, "setindex"))          r = SetTableIndex(dpar,parameters);   
   else if (!strcmp(command, "setrxfreq"))         r = SetRxFreq(dpar,parameters);   
   else if (!strcmp(command, "setrxgain"))         r = SetRxGain(dpar,parameters);   
   else if (!strcmp(command, "settxfreq"))         r = SetTxFreq(dpar,parameters); 
   else if (!strcmp(command, "settxfreqs"))        r = SetTxFreqs(dpar,parameters); 
	else if (!strcmp(command, "shapedrf"))          r = ShapedRFPulse(dpar, parameters);
	else if (!strcmp(command, "shapedrf1"))         r = ShapedRFPulse1(dpar, parameters);
	else if (!strcmp(command, "shapedrf2"))         r = ShapedRFPulse2(dpar, parameters);
	else if (!strcmp(command, "iftrue"))            r = SkipOnZero(dpar, parameters);
	else if (!strcmp(command, "skiponzero"))        r = SkipOnZero(dpar, parameters);
	else if (!strcmp(command, "skiponfalse"))       r = SkipOnZero(dpar, parameters);
 //  else if(!strcmp(command,"ttl"))               r = TTLOn(dpar,parameters);      
   else if (!strcmp(command, "ttlon"))             r = TTLOn(dpar,parameters);      
   else if (!strcmp(command, "ttloff"))            r = TTLOff(dpar,parameters);      
 //  else if(!strcmp(command,"ttlpulse"))          r = TTLPulse(dpar,parameters);      
 //  else if(!strcmp(command,"ttltranslate"))      r = TTLTranslate(dpar,parameters);      
   else if (!strcmp(command, "trigger"))           r = WaitForTrigger(dpar,parameters);       
   else if (!strcmp(command, "txoff"))             r = TxOff(dpar,parameters);   
   else if (!strcmp(command, "txon"))              r = TxOn(dpar,parameters);  
	else if (!strcmp(command, "updatemode"))        r = SetUpdateTableMode(dpar, parameters);
	else if (!strcmp(command, "updatepstable"))     r = UpdateEventTable(dpar, parameters);
	else if (!strcmp(command, "unpackdata"))        r = UnpackData(dpar, parameters);
	else if (!strcmp(command, "wait"))              r = Wait(dpar, parameters);  

   return(r);
}

/*******************************************************************************
Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   FX3 Pulse Programmer DLL module (V%1.1f)\n\n",VERSION);

   TextMessage("   acquire ............ acquire some data\n");
   TextMessage("   acquireon .......... start acquiring or append some data\n");
   TextMessage("   acquireoff ......... pause or finish acquiring the data\n");
   TextMessage("   chirprf2 ........... make a frequency and amplitude moduated RF pulse\n");
   TextMessage("   cleardata .......... clear data memory used for summing\n");
   TextMessage("   decindex ........... decrement a table index\n");
	TextMessage("   acquire ............ collect some data\n");
	TextMessage("   delay .............. generate a delay\n");
	TextMessage("   dualshapedrf1 ...... make an amplitude moduated RF pulse on channel 1 and 2\n");
	TextMessage("   dualshapedrf2 ...... make a phase and amplitude moduated RF pulse on channel 1 and 2\n");
	//TextMessage("   diffgradenable ..... enable the diffusion gradient\n");
   //TextMessage("   diffgraddisable .... disable the diffusion gradient\n");
   //TextMessage("   diffgradon ......... set the diffusion gradient amplitude\n");
   //TextMessage("   diffgradoff ........ zero the diffusion gradient amplitude\n");
   //TextMessage("   diffgradramp ......  use a linear ramp to control the diffusion gradient amplitude\n");
   TextMessage("   endloop ............ end a loop\n");
   TextMessage("   endpp .............. finish the pulse program\n");
	TextMessage("   endskip ............ end a skip\n");
	TextMessage("   endiftrue........... end an if block\n");
	//  TextMessage("   execwait ........ execute a program and wait for it to exit\n");
	
	TextMessage("   gradon ............. switch on the gradient\n");
	TextMessage("   gradoff ............ switch off the gradient\n");
   TextMessage("   gradramp ........... use a linear ramp to control the gradient amplitude\n");
	TextMessage("   iftrue ............. run code to endif if statement true\n");
	TextMessage("   incrxfreq .......... (NOT YET)increment the rx frequency\n");
	TextMessage("   inctxfreq .......... (NOT YET)increment the tx frequency\n");
   TextMessage("   initpp ............. initialise pulse program\n");
   TextMessage("   incindex ........... increment a table index\n");
   TextMessage("   inctxamp ........... (NOT YET)increment tx amplitude\n");
   TextMessage("   lockoff ............ (NOT YET)switch off the lock\n");
   TextMessage("   lockon ............. (NOT YET)switch on the lock\n");
   TextMessage("   loop ............... start a loop\n");
   TextMessage("   memreset ........... (NOT YET)reset memory pointer\n");
	TextMessage("   packps ............. pack ps data for sending to fx3\n");
   TextMessage("   ppversion .......... returns the version number of this DLL\n");
	TextMessage("   pulse .............. generate an RF pulse\n");
	TextMessage("   pulseboost ......... add an extra 3dB to pulse amplitude\n");
  // 	TextMessage("   report ............. reports psInfo contents\n");
	TextMessage("   report ............. list the last event table generated\n");
	TextMessage("   report2 ............ list the last event table generated (in pastable C-format)\n");
	TextMessage("   selectrxamp ........ (NOT YET)select rx amplifier to use\n");
	TextMessage("   setindex ........... set a table index\n");
   TextMessage("   setrxfreq .......... set the receive frequency\n");
   TextMessage("   setrxgain .......... (NOT YET)set the receive amplifier gain\n");
   TextMessage("   settxfreq .......... set the pulse frequency\n");
   TextMessage("   settxfreqs ......... set the pulse frequencies for both channels\n");
	TextMessage("   shapedrf1 .......... make an amplitude moduated RF pulse\n");
	TextMessage("   shapedrf2 .......... make a phase and amplitude moduated RF pulse\n");
	TextMessage("   skiponzero ......... skip code on zero argument\n");
	TextMessage("   skiponfalse ........ skip code on zero (false) argument\n");
   TextMessage("   ttlon .............. switch on a TTL level\n");
   TextMessage("   ttloff ............. switch off a TTL level\n");
 //  TextMessage("   ttlpulse ........... generate TTL pulse\n");
 //  TextMessage("   ttltranslate ....... translate TTL pin number to byte code\n");
   TextMessage("   trigger ............ wait for trigger input\n");
   TextMessage("   txoff .............. turn off the transmitter output\n");
	TextMessage("   txon ............... turn on the transmitter output\n");
	TextMessage("   updatemode ......... whether to generate full update tables or not\n");
	TextMessage("   updatepstable ...... update the p.s. event table\n");
	TextMessage("   unpackdata ......... reorganise data received from fx3\n");
   TextMessage("   wait ............... generate a long delay\n");

}

/*******************************************************************************
Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
	
	if (!strcmp(cmd, "acquire"))                   strcpy(syntax, "acquire(mode, number points:n, [duration:d, [shift_value:n]])");
	else if (!strcmp(cmd, "acquireon"))            strcpy(syntax, "acquireon([mode=overwrite/start/append], number points:n)");
	else if (!strcmp(cmd, "acquireoff"))           strcpy(syntax, "acquireoff(mode=overwrite/pause/finish, number points:n)");
	else if (!strcmp(cmd, "chirprf2"))             strcpy(syntax, "chirpr2f(channel (1/2), atable:t, ftable:f, phase:p, table_size:n, table_step_duration:d)");
	else if (!strcmp(cmd, "cleardata"))            strcpy(syntax, "cleardata(nr_points:n, nr_summations:n)");
   else if (!strcmp(cmd ,"decindex"))             strcpy(syntax, "decindex(table:t)");
	else if (!strcmp(cmd, "delay"))                strcpy(syntax, "delay(duration:d/t)");
	else if (!strcmp(cmd, "dualshapedrf1"))        strcpy(syntax, "dualshapedrf1(atable:t, phase1 : p, phase2 : p,  table_size : n, table_step_duration : d)");
	else if (!strcmp(cmd, "dualshapedrf2"))        strcpy(syntax, "dualshapedrf1(aptable:t, phase1 : p, phase2 : p,  table_size : n, table_step_duration : d)");
	else if (!strcmp(cmd, "endloop"))              strcpy(syntax, "endloop(name, [duration:d])");
	else if (!strcmp(cmd, "endpp"))                strcpy(syntax, "endpp([print ps? (0/1)])");
	else if (!strcmp(cmd, "endskip"))              strcpy(syntax, "endskip(name)");
	else if (!strcmp(cmd, "endiftrue"))            strcpy(syntax, "endiftrue(name)");
	//  else if(!strcmp(cmd,"execwait"))          strcpy(syntax,"execwait(program,arguments)");
	else if (!strcmp(cmd, "gradon"))               strcpy(syntax, "gradon(level:n/t)");
	else if (!strcmp(cmd, "gradoff"))              strcpy(syntax, "gradoff()");
   else if (!strcmp(cmd, "gradramp"))             strcpy(syntax, "gradramp(start:n, end:n, steps:n, delay:d)");
	else if (!strcmp(cmd, "iftrue"))               strcpy(syntax, "iftrue(name, value (0/1)))");
	else if (!strcmp(cmd, "incindex"))             strcpy(syntax, "incindex(table:t, [increment:d])");
	else if (!strcmp(cmd, "incrxfreq"))            strcpy(syntax, "incrxfreq(increment:f)");
	else if (!strcmp(cmd, "inctxfreq"))            strcpy(syntax, "inctxfreq(increment:f) OR inctxfreq(channel (1/2), increment:f) ");
	else if (!strcmp(cmd ,"inctxamp"))             strcpy(syntax, "inctxamp(amp:a, increment:a)");
	else if (!strcmp(cmd, "initpp"))               strcpy(syntax, "initpp(filename)");
	else if (!strcmp(cmd, "lockoff"))              strcpy(syntax, "lockoff()");
	else if (!strcmp(cmd, "lockon"))               strcpy(syntax, "lockon()");
	else if (!strcmp(cmd, "loop"))                 strcpy(syntax, "loop(name, nrLoops:n, [duration:d])");
	//  else if(!strcmp(cmd,"memreset"))             strcpy(syntax,"memreset([address:n])");
	//  else if(!strcmp(cmd,"nop"))                  strcpy(syntax,"nop()");     
	else if (!strcmp(cmd, "packps"))               strcpy(syntax, "MATRIX1D result = packps(DMATRIX1D input)");
	else if (!strcmp(cmd, "ppversion"))            strcpy(syntax, "(INT v) = ppversion()");
	else if (!strcmp(cmd, "pulse"))                strcpy(syntax, "pulse(channel (1/2), amp:a/t, phase:p/t, duration:d/t [,freq:f/t] OR pulse(1 ,a1, p1, f1, 2 ,a2, p2, f2, d])");
	else if (!strcmp(cmd, "pulseboost"))           strcpy(syntax, "pulseboost(channel (1/2), boost:n (1/0))");
	else if (!strcmp(cmd, "report"))               strcpy(syntax, "report()");
	else if (!strcmp(cmd, "report2"))              strcpy(syntax, "report2()");
	//  else if(!strcmp(cmd,"selectrxamp"))          strcpy(syntax,"selectrxamp(number n)");
	else if (!strcmp(cmd, "setindex"))             strcpy(syntax, "setindex(table:t,index:n)");
	else if (!strcmp(cmd, "setrxfreq"))            strcpy(syntax, "setrxfreq(freq:f)");
	else if (!strcmp(cmd, "settxfreq"))            strcpy(syntax, "settxfreq(freq:f) OR settxfreq(channel:1/2 freq:f) ");
	else if (!strcmp(cmd, "settxfreqs"))           strcpy(syntax, "settxfreqs(freq1:f, freq2:f)");
	else if (!strcmp(cmd, "skiponzero"))           strcpy(syntax, "skiponzero(name, value (0/1))");
	else if (!strcmp(cmd, "skiponfalse"))          strcpy(syntax, "skiponfalse(name, value (0/1))");
	//  else if(!strcmp(cmd,"setrxgain"))            strcpy(syntax,"setrxgain(channel:n, gain:g)");
	else if (!strcmp(cmd, "shapedrf1"))            strcpy(syntax, "shapedrf1(channel (1/2), atable:t, phase:p, table_size:n, table_step_duration:d)"); 
	else if (!strcmp(cmd, "shapedrf2"))            strcpy(syntax, "shapedrf2(channel (1/2) aptable:t, phase:p, table_size:n, table_step_duration:d"); 
   else if (!strcmp(cmd, "trigger"))              strcpy(syntax, "trigger(\"high\"/\"low\"/\"rising\"/\"falling\")");
 //  else if(!strcmp(cmd,"ttltranslate"))         strcpy(syntax,"(INT byte) = ttltranslate(pin number)");
 //  else if(!strcmp(cmd,"ttl"))                  strcpy(syntax,"ttl(byte:b)");
   else if(!strcmp(cmd, "ttlon"))                 strcpy(syntax, "ttlon(byte:b)");
   else if(!strcmp(cmd, "ttloff"))                strcpy(syntax, "ttloff(byte:b)");
 //  else if(!strcmp(cmd,"ttlpulse"))             strcpy(syntax,"ttlpulse(byte:b, duration:d)");
   else if (!strcmp(cmd,"txoff"))                 strcpy(syntax, "txoff(mode)");
	else if (!strcmp(cmd,"txon"))                  strcpy(syntax, "txon(mode, amp:a, phase:p [,freq:f]) OR txon(ch1, a1, p1, f1, ch2, a2, p2, f2)");
	else if (!strcmp(cmd,"updatemode"))            strcpy(syntax, "updatemode(\"full\"/\"minimal\")");
	else if (!strcmp(cmd,"updatepstable"))         strcpy(syntax, "updatepstable(eventTable, updateTable)");
	else if (!strcmp(cmd,"unpackdata"))            strcpy(syntax, "CMATRIX1D result = unpackdata(STR mode, MATRIX1D input)");
	else if (!strcmp(cmd,"wait"))                  strcpy(syntax, "wait(duration:w)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short GetHelpFolder(DLLParameters* par, char* args)
{
	par->retVar[1].MakeAndSetString("Macros\\Pulse Programming");
	par->nrRetVar = 1;
	return(OK);
}


/*************************************************************************
*       Return the value of the highres clock in milliseconds
*************************************************************************/

double GetMsTime()
{
	LARGE_INTEGER tick,freq;
	QueryPerformanceCounter(&tick);
	QueryPerformanceFrequency(&freq);
	double time = 1000.0L*(double)tick.QuadPart/(double)freq.QuadPart;
	return(time);
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

long nint(double num)
{
	if (num > 0)
		return((long)(num + 0.5));
	else
		return((long)(num - 0.5));

}

uint32 nuint(float num)
{
	return((uint32)(num + 0.5));
}

uint32 nuint(double num)
{
	if(num >= 0)
		return((uint32)(num + 0.5));
	else
		return((uint32)(num - 0.5));
}

// This structure holds all the event table information
struct
{
	std::vector<uint32> ps;
	bool inLoop;
	float dwellTime;
	bool getDuration;
	uint32 lineCnt;
	bool flatFilter;
	float pgo;
	uint32 currentTTL;
	uint32 endTTL;          // The state the TTL should have on exiting p.s.
	Variable* var;
	Variable* ppList;
	CText mode;
	double txFreqCh1;
	double txFreqCh2;
	double rxFreq;
	uint32 etStartAdrs;
}
psInfo;


/************************************************************************************
 Add an event to the pulse sequence event table ps
*************************************************************************************/

void AddEvent(uint32 duration, uint32 arg1, uint32 arg2)
{
	std::vector<uint32>* ps = &(psInfo.ps);

	ps->push_back(duration);
	ps->push_back(arg1);
	ps->push_back(arg2);

	psInfo.lineCnt++;
}

/************************************************************************************
FX3 based PS commands
*************************************************************************************/

typedef struct
{
	uint32 specialVar;
	uint32 varPos;
	uint32 adrs;
	uint32 value;
}
psUpdateValue;

vector<psUpdateValue> psVariableUpdateTable;
vector<psUpdateValue> psFixedUpdateTable;

/************************************************************************************
 Function to initialise the psInfo data structure before running a pulse program
************************************************************************************/

short InitialisePP(DLLParameters* par, char* args)
{
	CText dir;
	CText psInfoName;
	short nrArgs, varType;

	if ((nrArgs = ArgScan(par->itfc, args, 1, "ps info structure", "e", "q", &psInfoName)) < 0)
		return(nrArgs);

	// Get the list of variables to be modified (should be defined locally in PS macro)
	psVariables = GetVariable(par->itfc, ALL_VAR, "variables", varType);

  // Initialise the update tables
	psVariableUpdateTable.clear();
	psFixedUpdateTable.clear();

  // Intialise documentation table
	eventTableInfo.clear();

	// Clear the ps table
	psInfo.ps.clear();

	// Get the ps info structure (Prospa format)
	psInfo.var = GetVariable(par->itfc, ALL_VAR, psInfoName.Str(), varType);

	if (psInfo.var->GetType() != STRUCTURE)
	{
		ErrorMessage("Invalid data type - should be a structure");
		return(ERR);
	}

	// Extract these to a local C-structure
	Variable* struc, * svar;
	struc = psInfo.var->GetStruct();
	svar = struc->next;

	psInfo.getDuration = false;

// Some defaults
	psInfo.currentTTL = 0x8000;  // Switches on the driver bias during the sequence
	psInfo.etStartAdrs = 0x3000; // Start address of pulse program


	while (svar != NULL)
	{
		char* name = svar->GetName();
		if (!strcmp(name, "mode") && svar->GetType() == UNQUOTED_STRING)
			psInfo.mode = svar->GetString();
		else if (!strcmp(name, "dwellTime") && svar->GetType() == FLOAT32)
			psInfo.dwellTime = svar->GetReal();
		else if (!strcmp(name, "getDuration") && svar->GetType() == FLOAT32)
			psInfo.getDuration = (bool)((uint32)(svar->GetReal() + 0.5) == 1);
		else if (!strcmp(name, "flatFilter") && svar->GetType() == UNQUOTED_STRING)
			psInfo.flatFilter = (bool)(!strcmp(svar->GetString(),"yes"));
		else if (!strcmp(name, "pgo") && svar->GetType() == FLOAT32)
			psInfo.pgo = svar->GetReal();
		else if (!strcmp(name, "b1FreqTxCh1") && svar->GetType() == FLOAT64)
			psInfo.txFreqCh1 = svar->GetDouble();
		else if (!strcmp(name, "b1FreqTxCh2") && svar->GetType() == FLOAT64)
			psInfo.txFreqCh2 = svar->GetDouble();
		else if (!strcmp(name, "b1FreqRx") && svar->GetType() == FLOAT64)
			psInfo.rxFreq = svar->GetDouble();
		else if (!strcmp(name, "pgo") && svar->GetType() == FLOAT32)
			psInfo.pgo = svar->GetReal();
		else if (!strcmp(name, "ttlInit") && svar->GetType() == FLOAT32)
			psInfo.currentTTL = nint(svar->GetReal());
		else if (!strcmp(name, "ttlEnd") && svar->GetType() == FLOAT32)
			psInfo.endTTL = nint(svar->GetReal());
		// Add the list of variables for this sequence
		else if (!strcmp(name, "ppList") && svar->GetType() == LIST)
			psInfo.ppList = svar;
		// Add the start code to the ps event table
		else if (!strcmp(name, "psStartCode") && svar->GetType() == DMATRIX2D)
		{
			double **startCode = svar->GetDMatrix2D();
			uint32 sz = svar->GetDimX();
			for (int i = 0; i < sz; i++)
				psInfo.ps.push_back(nint(startCode[0][i]));
		}
		svar = svar->next;
	}

	psInfo.lineCnt = psInfo.ps.size()/3;

	CText txt = "TRex initialisation begins";
	ETInfo info1 = { 0, txt.Str() };
	eventTableInfo.push_back(info1);

	txt = "TRex initialisation ends";
	ETInfo info2 = { psInfo.lineCnt-1, txt.Str() };
	eventTableInfo.push_back(info2);

	// Get a pointer to the new ps command list (appended to by each command).
	std::vector<uint32>* ps = &(psInfo.ps);

	// Initialise the loop number stack
	loopNrStk.clear();
	skipInfo.clear();

	// Free any previous parameter list so we can build a new one
	if (szList > 0)
		FreeList(parList, szList + 1);
	parList = MakeList(1);
	szList = 0;

	// Initialise the current gradient level - should be zero at end of sequence
	gCurGradLevel = 0;

	return(OK);
}

/************************************************************************************
  Add an end event to the event table, update any skip addresss and then
  update psInfo->ps with the new event table
************************************************************************************/

short EndPP(DLLParameters* par, char* args)
{
	short nrArgs;
	CText verboseStr = "0";
	Variable result;
	bool verbose;

	if ((nrArgs = ArgScan(par->itfc, args, 0, "print ps?", "e", "q", &verboseStr)) < 0)
		return(nrArgs);

	// Print out the event table if argument is 1
	Evaluate(par->itfc, NORMAL, verboseStr.Str(), &result);
	if (result.GetType() == FLOAT32)
		verbose = (bool)result.GetReal();
	else
		verbose = false;
	if (verbose)
	{
		Report(par, args);
	}

	// Generate the event entry to end the PS
	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		int lineCnt = psInfo.lineCnt;

		// Add the events to end the ps
		AddEvent(20,  0x00000000, psInfo.endTTL); // Reset RF
		AddEvent(200, 0x07000003, 0); // Reset x grad
		AddEvent(200, 0x07000002, 0); // Reset y grad
		AddEvent(200, 0x07000001, 0); // Reset z grad
		AddEvent(80,  0x03000000, 0); // Reset o grad

		// Text for the report command
		CText txt = "endpp";
		ETInfo info = { lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		// Update any skip addresses
		std::vector<uint32>* ps = &(psInfo.ps);
		uint32 psOffset = ps->size();
		int skipLines = skipInfo.size();
		for (int i = 0; i < skipLines; i++)
		{
			uint32 start = skipInfo[i].skipStartLine;
			uint32 end = skipInfo[i].skipEndLine;
		   if(skipInfo[i].skipValue == 0)
				(*ps)[start * 3 + 2] = end * 3 + 0x8000;
			else
				(*ps)[start * 3 + 2] = end * 3;
		}

	  // Add the event table to the psInfo as a double matrix
		int psLines = ps->size();
		double** psM = MakeDMatrix2D(psLines, 1);
		for (int i = 0; i < psLines; i++)
		{
			psM[0][i] = (double)(*ps)[i];
		}

		// Find and add the ps variable to psInfo
		Variable* struc, * svar;
		struc = psInfo.var->GetStruct();
		svar = struc->next;
		bool found = false;

		while (svar != NULL)
		{
			char* name = svar->GetName();
			if (!strcmp(name, "ps"))
			{
				found = true;
				break;
			}
			svar = svar->next;
		}
		if (found)
		{
			svar->FreeData();
			svar->AssignDMatrix2D(psM, psLines, 1);
		}
		else
		{
			Variable* v = struc->Add(DMATRIX2D, "ps");
			v->AssignDMatrix2D(psM, psLines, 1);
		}

		// Check if a gradient level is left high
		if (abs(gCurGradLevel) > 1000) // This is a guess - need something more concrete
		{
			ErrorMessage("Gradient value is not insignificant at end of sequence - please check!");
			return(ERR);
		}

		par->retVar[1].MakeNullVar();
		par->nrRetVar = 1;
		FreeList(parList, szList + 1); // 1 bigger because we started with 1 dummy entry
		szList = 0;
		return(OK);
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		// Generate the variable update table 
		int utRows = psVariableUpdateTable.size();
		double** ut = MakeDMatrix2D(4, utRows);
		for (int i = 0; i < utRows; i++)
		{
			ut[i][0] = psVariableUpdateTable[i].specialVar;
			ut[i][1] = psVariableUpdateTable[i].varPos;
			ut[i][2] = psVariableUpdateTable[i].adrs;
			ut[i][3] = psVariableUpdateTable[i].value;
		}

		// Copy the collected ps data back to the psInfo variable
		Variable* struc, * svar;
		struc = psInfo.var->GetStruct();
		svar = struc->next;
		bool found = false;

		while (svar != NULL)
		{
			char* name = svar->GetName();
			if (!strcmp(name, "updateTable"))
			{
				found = true;
				break;
			}
			svar = svar->next;
		}
		if (found)
		{
			svar->FreeData();
			svar->AssignDMatrix2D(ut, 4, utRows);
		}
		else
		{
			Variable* v = struc->Add(DMATRIX2D, "updateTable");
			v->AssignDMatrix2D(ut, 4, utRows);
		}

		// Generate the fixed update table 
		utRows = psFixedUpdateTable.size();
		double** utFixed = MakeDMatrix2D(4, utRows);
		for (int i = 0; i < utRows; i++)
		{
			utFixed[i][0] = psFixedUpdateTable[i].specialVar;
			utFixed[i][1] = psFixedUpdateTable[i].varPos;
			utFixed[i][2] = psFixedUpdateTable[i].adrs;
			utFixed[i][3] = psFixedUpdateTable[i].value;
		}

		// Copy the collected ps data back to the psInfo variable
		struc = psInfo.var->GetStruct();
		svar = struc->next;
		found = false;

		while (svar != NULL)
		{
			char* name = svar->GetName();
			if (!strcmp(name, "fixedUpdateTable"))
			{
				found = true;
				break;
			}
			svar = svar->next;
		}
		if (found)
		{
			svar->FreeData();
			svar->AssignDMatrix2D(utFixed, 4, utRows);
		}
		else
		{
			Variable* v = struc->Add(DMATRIX2D, "fixedUpdateTable");
			v->AssignDMatrix2D(utFixed, 4, utRows);
		}

		par->retVar[1].MakeAndSetList(parList, szList);
		par->nrRetVar = 1;
		FreeList(parList, szList + 1); // 1 bigger because we started with 1 dummy entry
		szList = 0;
		return(OK);
	}
	par->retVar[1].MakeNullVar();
	par->nrRetVar = 1;
	FreeList(parList, szList + 1); // 1 bigger because we started with 1 dummy entry
	szList = 0;
	return(OK);
}


short Report(DLLParameters* par, char* args)
{
	if (psInfo.getDuration)
		return(OK);

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
	//	TextMessage("\n   In loop = %d\n", (int)psInfo.inLoop);
	//	TextMessage("   Current TTL = %ld\n", psInfo.currentTTL);
		int sz = psInfo.ps.size();
		int infoSize = eventTableInfo.size();
		int i, j;

		TextMessage("\n  Line     Clocks      Arg1      Arg2\n");
		for (i = 0; i < sz; i += 3)
		{
			for (j = 0; j < eventTableInfo.size(); j++)
			{
				if (eventTableInfo[j].lineNr == i/3)
					break;
			}
			if(j < infoSize)
				TextMessage("%4d:  %10u   %08X   %08X # %s\n", i / 3, psInfo.ps[i], psInfo.ps[i + 1], psInfo.ps[i + 2], eventTableInfo[j].str.Str());
			else
				TextMessage("%4d:  %10u   %08X   %08X\n", i / 3, psInfo.ps[i], psInfo.ps[i + 1], psInfo.ps[i + 2]);
		}
	}
	return(OK);
}

short Report2(DLLParameters* par, char* args)
{
	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		TextMessage("\n   In loop = %d\n", (int)psInfo.inLoop);
		TextMessage("   Current TTL = %ld\n", psInfo.currentTTL);
		int sz = psInfo.ps.size();
		//	int off = 110;

		TextMessage("\n     Clocks        Arg1        Arg2\n");
		for (int i = 0; i < sz; i += 3)
		{
			if(i == sz-3)
				TextMessage("    %10u,     0x%08X,     0x%08X\n", psInfo.ps[i], psInfo.ps[i + 1], psInfo.ps[i + 2]);
			else
				TextMessage("    %10u,     0x%08X,     0x%08X,\n", psInfo.ps[i], psInfo.ps[i + 1], psInfo.ps[i + 2]);
		}
	}
	return(OK);
}



/***********************************************************************************************************

	PS command 'acquire'. Blocking acquire using the continuous acquisition mode in the TRex.
	Saves 'nrPnts' on the TRex SRAM. Mode options are: (N = nrPnts-1, mem is SRAM data memory)

	Format is:

	acquire("overwrite/append/inegrate", nrPnts, [duration])
	acquire("integrateandscale", nrPnts, shift, [duration])
	acquire("sum/stack", nrPnts, duration, [shift])

	Modes:

	overwrite ........... collects data overwriting existing data memory: mem[0..N] = newData[0..N]
	append .............. collects data appending to existing mem[0..M] = mem[0..M-N], newData[0..N]
	integrate ........... sums new data and stores a single number in SRAM mem[i] = sum(newData[0..N]), i++
	integrateandscale ... same as integrate but scale data first mem[i] = sum(newData[0..N]>>shiftValue), i++
	stack/sum ........... adds data set to existing data  mem[0..N] = newData[0..N] + existingData[0..N]

*************************************************************************************************************/

short Acquire(DLLParameters* par, char* args)
{
	CText mode, nrPntsStr, durationStr="0", shiftValueStr="0";
	short nrArgs;
	uint32 startLineNr = psInfo.lineCnt;

// In compile mode extract and evaluate the value of each passed argument and embed these in the event table (psInfo.ps)
	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		// Get mode
		if ((nrArgs = ArgScan(par->itfc, args, 1, "mode", "c", "t", &mode)) < 0)
			return(nrArgs);

		// Extract the arguments based on the supplied mode
		if (mode == "integrateandscale")
		{
			if ((nrArgs = ArgScan(par->itfc, args, 3, "mode, nr points, shift factor, [duration]", "cccc", "tttt", &mode, &nrPntsStr, &shiftValueStr, &durationStr)) < 0)
				return(nrArgs);
		}
		else if (mode == "sum" || mode == "stack")
		{
			if ((nrArgs = ArgScan(par->itfc, args, 3, "mode, nr points, duration, [shift factor]", "cccc", "tttt", &mode, &nrPntsStr, &durationStr, &shiftValueStr)) < 0)
				return(nrArgs);
		}
		else if (mode == "overwrite" || mode == "append" || mode == "integrate") 
		{
			if ((nrArgs = ArgScan(par->itfc, args, 2, "mode, nrPnts, [duration]", "ccc", "ttt", &mode, &nrPntsStr, &durationStr)) < 0)
				return(nrArgs);
		}
		else
		{
			ErrorMessage("Invalid acquire mode '%s' (overwrite/append/integrate/sum/stack/integrateandscale)",psInfo.mode.Str());
			return(ERR);
		}

		// Extract the number of points to acquire from the argument list
		uint32 nrPnts;
		EvaluateArg(par->itfc, nrPntsStr.Str(), INTEGER, nrPnts);
		if(nrPnts > MAX_DATA_POINTS)
		{
			ErrorMessage("Maximum number of data points is %ld", MAX_DATA_POINTS);
			return(ERR);
		}

		// Calculate the expected acqusition time
		float dwellTime = psInfo.dwellTime;
		bool flatFilter = psInfo.flatFilter;
		long trueAcqTime = (uint32)(GetAcqTime(dwellTime, nrPnts, flatFilter) * 100.0 + 0.5);

		// Get the optional acquire duration
		uint32 digDuration = 0;
		float duration;
		EvaluateArg(par->itfc, durationStr.Str(), FLOAT32, duration);
		digDuration = (nint)(duration * 100);
		if (digDuration > 0 && (digDuration < (trueAcqTime + 120 + 10))) // Needs to be long enough to include all events
		{
			ErrorMessage("Acquire duration is not long enough - should be > %g us", (trueAcqTime + 120 + 10) / 100.0);
			return(ERR);
		}

		// Now generate the event table entries based on the supplied acquire mode
		// Start with the simplest - the overwrite mode. This just overwrites the current memory entries
		if (mode == "start" || mode == "overwrite")
		{
			AddEvent(20,          0x41A3,  nrPnts); // Number of points to collect
			AddEvent(20,          0x4081,  0x03);   // DDC and output enable
			AddEvent(20,          0x6081,  0x01);   // ???
			AddEvent(20,          0x6081,  0x06);   // FIFO select FIR/CIC as input
			AddEvent(20,          0x4082,  0x00);   // Overwrite mode
		   AddEvent(20,          0x0000,  psInfo.currentTTL | 0x10);
			AddEvent(trueAcqTime, 0x0000,  psInfo.currentTTL | 0x01);

			if (digDuration > 0)
				AddEvent(digDuration - trueAcqTime - 120, 0x0000, psInfo.currentTTL); // ADC Stop
			else
				AddEvent(80, 0x0000, psInfo.currentTTL); // ADC Stop

		}
		// This mode appends the data to any existing data stored in memory
		else if (mode == "append")
		{
			AddEvent(20,          0x41A3,  nrPnts); // Number of points to collect
			AddEvent(20,          0x4081,  0x03);   // DDC and output enable
			AddEvent(20,          0x6081,  0x06);   // FIFO select FIR/CIC as input
			AddEvent(20,          0x4082,  0x02);   // Append mode
			AddEvent(20,          0x4083,  nrPnts); // Number of points per block
			AddEvent(20,          0x0000,  psInfo.currentTTL | 0x10); // Reset CIC
			AddEvent(trueAcqTime, 0x0000,  psInfo.currentTTL | 0x01); // ADC start

			if (digDuration > 0)
				AddEvent(digDuration - trueAcqTime - 120, 0x0000, psInfo.currentTTL); // ADC Stop
			else
				AddEvent(80, 0x0000, psInfo.currentTTL); // ADC Stop

		}
		// This mode sums all the data and returns a single complex point
		else if (mode == "integrate")
		{
			AddEvent(20,          0x41A3,  nrPnts); // Number of points to collect
			AddEvent(20,          0x4081,  0x03);   // DDC and output enable
			AddEvent(20,          0x6081,  0x06);   // FIFO select FIR/CIC as input
			AddEvent(20,          0x4082,  0x01);   // Integrate mode
			AddEvent(20,          0x4083,  nrPnts); // Number of points per block
			AddEvent(20,          0x0000,  psInfo.currentTTL | 0x10); // Reset CIC
			AddEvent(trueAcqTime, 0x0000,  psInfo.currentTTL | 0x01); // ADC start

			if (digDuration > 0)
				AddEvent(digDuration - trueAcqTime - 120, 0x0000, psInfo.currentTTL); // ADC Stop
			else
				AddEvent(80, 0x0000, psInfo.currentTTL); // ADC Stop

		}
		// This mode sums all the data and returns a single complex point but scales it first by a power of 2
		else if (mode == "integrateandscale")
		{
			uint32 shiftValue = 0;

			EvaluateArg(par->itfc, shiftValueStr.Str(), INTEGER, shiftValue);
			if (shiftValue >= 256)
			{
				ErrorMessage("shiftValue for integrate and scale mode should be < 256");
				return(ERR);
			}

			AddEvent(20,          0x41A3,  nrPnts); // Number of points to collect
			AddEvent(20,          0x4081,  0x03);   // DDC and output enable
			AddEvent(20,          0x6081,  0x06);   // FIFO select FIR/CIC as input
			AddEvent(20,          0x4082,  0x01 + (shiftValue << 8));   // Integrate mode with shift
			AddEvent(20,          0x4083,  nrPnts); // Number of points per block
			AddEvent(20,          0x0000,  psInfo.currentTTL | 0x10); // Reset CIC
			AddEvent(trueAcqTime, 0x0000,  psInfo.currentTTL | 0x01); // ADC start

			if (digDuration > 0)
				AddEvent(digDuration - trueAcqTime - 120, 0x0000, psInfo.currentTTL); // ADC Stop
			else
				AddEvent(80, 0x0000, psInfo.currentTTL); // ADC Stop
		}
		// This mode sums the data to existing memory and returns nrPnts (with optional shifting)
		else if (mode == "sum" || mode == "stack")
		{
			uint32 shiftValue = 0;

		   EvaluateArg(par->itfc, shiftValueStr.Str(), INTEGER, shiftValue);

			if (digDuration < (trueAcqTime + 120 + 10)) // Needs to be long enough to include all events
			{
				ErrorMessage("Acquire duration is not long enough - should be > %g us", (trueAcqTime + 120 + 10) / 100.0);
				return(ERR);
			}

			if (nrPnts > MAX_STACK_SIZE)
			{
				ErrorMessage("The maximum size for summing/stacking NMR data is %d points", MAX_STACK_SIZE);
				return(ERR);
			}

			AddEvent(20,          0x41A3,  nrPnts); // Number of points to collect
			AddEvent(20,          0x4081,  0x03);   // DDC and output enable
			AddEvent(20,          0x6081,  0x06);   // FIFO select FIR/CIC as input
			AddEvent(20,          0x4082,  0x03 + (shiftValue << 8));   // Sum mode with shift
			AddEvent(20,          0x4083,  nrPnts); // Number of points per block
			AddEvent(20,          0x0000,  psInfo.currentTTL | 0x10); // Reset CIC
			AddEvent(trueAcqTime, 0x0000,  psInfo.currentTTL | 0x01); // ADC start

			if (digDuration > 0)
				AddEvent(digDuration - trueAcqTime - 120, 0x0000, psInfo.currentTTL); // ADC Stop
			else
				AddEvent(80, 0x0000, psInfo.currentTTL); // ADC Stop
		}
		else
		{
			ErrorMessage("Invalid mode");
			return(ERR);
		}

		// Generate information which will be add to the event table if listed with the report command
		std::vector<uint32>* ps = &(psInfo.ps);
		uint32 endIdx = ps->size();
		uint32 totDuration = 0;
		uint32 startIdx = ps->size();

		for (int i = startIdx; i < endIdx; i += 3)
			totDuration = totDuration + ps->at(i);

		if (mode == "integrateandscale")
		{
			CText txt;
			txt.Format("acquire(%s,%s,%s) [%g us]", mode.Str(), nrPntsStr.Str(), shiftValueStr.Str(), totDuration/100.0);
			ETInfo info = { startLineNr, txt.Str() };
			eventTableInfo.push_back(info);
		}
		else if (mode == "sum" || mode == "stack")
		{
			CText txt;
			txt.Format("acquire(%s,%s,%s,%s) [%g us]", mode.Str(), nrPntsStr.Str(), durationStr.Str(), shiftValueStr.Str(), totDuration / 100.0);
			ETInfo info = { startLineNr, txt.Str() };
			eventTableInfo.push_back(info);
		}
		else
		{
			CText txt;
			if (nrArgs == 3)
				txt.Format("acquire(%s,%s,%s) [%g us]", mode.Str(), nrPntsStr.Str(), durationStr.Str(), totDuration / 100.0);
			else
				txt.Format("acquire(%s,%s) [%g us]", mode.Str(), nrPntsStr.Str(), totDuration / 100.0);
			ETInfo info = { startLineNr, txt.Str() };
			eventTableInfo.push_back(info);
		}
	}

// In table mode extract the parameter names as they appear in the pulse program (not in pulse program blocks) and 
// add these to the update table. Then determine the location and value of these parameters in the event table.
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		// Get mode
		if ((nrArgs = ArgScan(par->itfc, args, 1, "mode", "e", "q", &mode)) < 0)
			return(nrArgs);

		if (mode == "integrateandscale")
		{
			if ((nrArgs = ArgScan(par->itfc, args, 3, "mode, nr points, shift factor, [duration]", "eeee", "qqqq", &mode, &nrPntsStr, &shiftValueStr, &durationStr)) < 0)
				return(nrArgs);
		}
		else if (mode == "sum" || mode == "stack")
		{
			if ((nrArgs = ArgScan(par->itfc, args, 3, "mode, nr points, duration, [shift factor]", "eeee", "qqqq", &mode, &nrPntsStr, &durationStr, &shiftValueStr)) < 0)
				return(nrArgs);
		}
		else
		{
			if ((nrArgs = ArgScan(par->itfc, args, 2, "mode, nrPnts, [duration]", "eee", "qqq", &mode, &nrPntsStr, &durationStr)) < 0)
				return(nrArgs);
		}

		// Add new variables to the list
		if (nrPntsStr[0] == 'n')
			InsertUniqueStringIntoList(nrPntsStr.Str(), &parList, szList);

		if (mode == "integrateandscale")
		{
			if (nrArgs >= 3)
			{
				if (shiftValueStr[0] == 'n')
					InsertUniqueStringIntoList(shiftValueStr.Str(), &parList, szList);
			}
			if (nrArgs == 4)
			{
				if (durationStr[0] == 'd' || durationStr[0] == 't')
					InsertUniqueStringIntoList(durationStr.Str(), &parList, szList);
			}
		}
		else if (mode == "sum" || mode == "stack")
		{
			if (nrArgs >= 3)
			{
				if (durationStr[0] == 'd' || durationStr[0] == 't')
					InsertUniqueStringIntoList(durationStr.Str(), &parList, szList);
			}
			if (nrArgs == 4)
			{
				if (shiftValueStr[0] == 'n')
					InsertUniqueStringIntoList(shiftValueStr.Str(), &parList, szList);
			}
		}
		else
		{
			if (nrArgs >= 3)
			{
				if (durationStr[0] == 'd' || durationStr[0] == 't')
					InsertUniqueStringIntoList(durationStr.Str(), &parList, szList);
			}
		}

		uint32 tableAdrs = psInfo.lineCnt * 6;

		if (mode == "start" || mode == "overwrite")
		{
			long varPos;

			// Update the number of points
			if (IsSequenceVariable(nrPntsStr.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos + 0x1000, tableAdrs + 5, 0);
				AddToVariableUpdateTable(varPos, tableAdrs + 4, 0);
			}
			if (IsSequenceParameter(nrPntsStr.Str(), varPos))
			{
				AddToFixedUpdateTable(0, varPos + 0x1000, tableAdrs + 5, 0);
				AddToFixedUpdateTable(0, varPos, tableAdrs + 4, 0);
			}
			// Update the acquisition duration value - this parameter 
			// must be updated externally
			AddToFixedUpdateTable(1, 0x1000, tableAdrs + 37, 0);
			AddToFixedUpdateTable(1, 0, tableAdrs + 36, 0);
		}
		else
		{
			long varPos;
		
			// Update the number of points
			if (IsSequenceVariable(nrPntsStr.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos + 0x1000, tableAdrs + 5, 0);
				AddToVariableUpdateTable(varPos,          tableAdrs + 4, 0);
				AddToVariableUpdateTable(varPos + 0x1000, tableAdrs + 29, 0);
				AddToVariableUpdateTable(varPos,          tableAdrs + 28, 0);
			}
			if (IsSequenceParameter(nrPntsStr.Str(), varPos))
			{
				AddToFixedUpdateTable(0, varPos + 0x1000, tableAdrs + 5, 0);
				AddToFixedUpdateTable(0, varPos,          tableAdrs + 4, 0);
				AddToFixedUpdateTable(0, varPos + 0x1000, tableAdrs + 29, 0);
				AddToFixedUpdateTable(0, varPos,          tableAdrs + 28, 0);
			}
			// Update the duration parameter (special case)
			if (IsSequenceParameter(durationStr.Str(), varPos))
			{
				AddToFixedUpdateTable(0, varPos + 0x11000, tableAdrs + 43, 0);
				AddToFixedUpdateTable(0, varPos + 0x10000, tableAdrs + 42, 0);
			}
        //Update acquisition duration parameters (not in argument list)
			AddToFixedUpdateTable(1, 0x1000, tableAdrs + 37, 0);
			AddToFixedUpdateTable(1, 0,      tableAdrs + 36, 0);
		}

		if (mode == "integrateandscale")
		{
			long varPos;

			// Update the shift parameter 
			if (IsSequenceParameter(shiftValueStr.Str(), varPos))
			{
				AddToFixedUpdateTable(6, varPos, tableAdrs + 22, 0);
			}
		}
		else if (mode == "sum" || mode == "stack")
		{
			long varPos;

			// Update the shift parameter 
			if (IsSequenceParameter(shiftValueStr.Str(), varPos))
			{
				AddToFixedUpdateTable(7, varPos, tableAdrs + 22, 0);
			}
		}

		if (psInfo.mode == "tables")
			psInfo.lineCnt += 8;
	}

	return(OK);
}


/*******************************************************************************

  Generate code to acquire some data
  Format is:
     acquireon(mode, nrPnts, [duration])
  or
     acquireon(nrPnts)

*******************************************************************************/

short AcquireOn(DLLParameters* par, char* args)
{
	CText mode = "overwrite", nrPntsStr;
	short nrArgs;
	CArg carg;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		nrArgs = carg.Count(args);

		if (nrArgs == 1)
		{
			if (ArgScan(par->itfc, args, 1, "nrPnts", "c", "t", &nrPntsStr) < 0)
				return(nrArgs);

			mode = "overwrite";
		}
		else
		{
			if (ArgScan(par->itfc, args, 2, "mode, nrPnts", "cc", "tt", &mode, &nrPntsStr) < 0)
				return(nrArgs);
		}

		CText txt;
		if (nrArgs == 2)
			txt.Format("acquireon(%s,%s)", mode.Str(), nrPntsStr.Str());
		else
			txt.Format("acquireon(%s)", nrPntsStr.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		std::vector<uint32>* ps = &(psInfo.ps);

		uint32 nrPnts;
		EvaluateArg(par->itfc, nrPntsStr.Str(), INTEGER, nrPnts);

		if (mode == "start" || mode == "overwrite")
		{
			ps->push_back(20);  ps->push_back(0x41A3); ps->push_back(nrPnts);
			ps->push_back(20);  ps->push_back(0x4081); ps->push_back(0x03);
			ps->push_back(20);  ps->push_back(0x6081); ps->push_back(0x01);
			ps->push_back(20);  ps->push_back(0x6081); ps->push_back(0x06);
			ps->push_back(20);  ps->push_back(0x4082); ps->push_back(0x00);
			ps->push_back(20);  ps->push_back(0x0000); ps->push_back(psInfo.currentTTL | 0x10);
			ps->push_back(80);  ps->push_back(0x0000); ps->push_back(psInfo.currentTTL | 0x01);

			psInfo.currentTTL = 0x01;
			psInfo.lineCnt += 7;
		}
		else if (mode == "adc")
		{
			ps->push_back(20);              ps->push_back(0x41A3); ps->push_back(nrPnts);
			ps->push_back(20);              ps->push_back(0x4081); ps->push_back(0x03);
			ps->push_back(20);              ps->push_back(0x6081); ps->push_back(0x01);
			ps->push_back(20);              ps->push_back(0x6081); ps->push_back(0x02);
			ps->push_back(20);              ps->push_back(0x4082); ps->push_back(0x00);
			ps->push_back(20);              ps->push_back(0x0000); ps->push_back(psInfo.currentTTL | 0x10);
			ps->push_back(80);              ps->push_back(0x0000); ps->push_back(psInfo.currentTTL | 0x01);

			psInfo.currentTTL = 0x01;
			psInfo.lineCnt += 7;
		}
		else if (mode == "append")
		{
			long psOffset = ps->size();
			ps->push_back(20);  ps->push_back(0x41A3); ps->push_back(nrPnts);
			ps->push_back(20);  ps->push_back(0x4081); ps->push_back(0x03);
			ps->push_back(20);  ps->push_back(0x6081); ps->push_back(0x06);
			ps->push_back(20);  ps->push_back(0x4082); ps->push_back(0x00);
			ps->push_back(20);  ps->push_back(0x0000); ps->push_back(psInfo.currentTTL | 0x10);
			ps->push_back(100);  ps->push_back(0x0000); ps->push_back(psInfo.currentTTL | 0x01);

			psInfo.currentTTL = 0x01;
			psInfo.lineCnt += 6;

		}
		else
		{
			ErrorMessage("Invalid mode");
			return(ERR);
		}
	}
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		nrArgs = carg.Count(args);

		if (nrArgs == 1)
		{
			if (ArgScan(par->itfc, args, 1, "nrPnts", "e", "q", &nrPntsStr) < 0)
				return(nrArgs);

			mode = "overwrite";
		}
		else
		{
			if (ArgScan(par->itfc, args, 2, "mode, nrPnts", "cc", "tt", &mode, &nrPntsStr) < 0)
				return(nrArgs);
		}

		// Add new variables to the list
		if (nrPntsStr[0] == 'n')
			InsertUniqueStringIntoList(nrPntsStr.Str(), &parList, szList);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;
		if (IsSequenceVariable(nrPntsStr.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos,            adrs + 4,  0);
			AddToVariableUpdateTable(varPos + 0x1000,   adrs + 5,  0);
		}
		if (IsSequenceParameter(nrPntsStr.Str(), varPos))
		{
			AddToFixedUpdateTable(0, varPos, adrs + 4, 0);
			AddToFixedUpdateTable(0, varPos + 0x1000, adrs + 5, 0);
		}

		if (mode == "start" || mode == "overwrite")
			psInfo.lineCnt += 7;
		else
			psInfo.lineCnt += 6;
	}

	return(OK);
}


short AcquireOff(DLLParameters* par, char* args)
{
	CText nrPnts;
	CText mode = "overwrite";
	int nrArgs;

	if ((nrArgs = ArgScan(par->itfc, args, 2, "mode, nr points", "cc", "tt", &mode, &nrPnts)) < 0)
		return(nrArgs);

	CText txt;
	txt.Format("acquireoff(%s,%s)", mode.Str(), nrPnts.Str());
	ETInfo info = { psInfo.lineCnt, txt.Str() };

	eventTableInfo.push_back(info);

	std::vector<uint32>* ps = &(psInfo.ps);

   psInfo.currentTTL = psInfo.currentTTL & ~(0x00000001);
	ps->push_back(15);  ps->push_back(0x0000); ps->push_back(psInfo.currentTTL);   // Stop acquiring
	psInfo.lineCnt += 1;
	
	return(OK);
}


/******************************************************
 Return the approximate acquisition time in us

    GetAcqTime(dwellTIme, nrPnts, useFIRFilter)

    dwellTime : sampling interval in us
    nrPnts : number of points to collect.
    useFIRFilter : 1 / 0 (flat filter)

*******************************************************/

long GetAcqTime( float dwellTime, long nrPnts, bool useFIRFilter)
{
	long CIC_Scale;
	long ignorePnts = 6;
	long MaxSpeed = 65;
	long nrTaps = 20;
	float FIRStartDelay;
	float CICDelay;
	float FIRDelay;
	float FIRScale;
	float acqTm;

	if (useFIRFilter)
		CIC_Scale = 3;
	else
		CIC_Scale = 11;

	FIRScale = 2;

	if (2 * nrPnts > nrTaps + 1)
		FIRStartDelay = 21 * MaxSpeed;
	else
		FIRStartDelay = 2 * nrPnts * MaxSpeed;

	CICDelay = 10.0 * (CIC_Scale + 71 + (3 * MaxSpeed)) / 1000.0;
	FIRDelay = 10.0 * (50 + FIRScale + FIRStartDelay) / 1000.0;

	if (useFIRFilter)
		acqTm = (int)((nrPnts + ignorePnts) * dwellTime + CICDelay + FIRDelay +0.5);
	else
	{
		// Make allowances for CIC shift register delays - how to calculate these values ?
		//scaleDwellTime = [0.5, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000]
		//scaleDelay = [0.11, 0.15, 0.19, 0.27, 0.31, 0.35, 0.39, 0.43, 0.47, 0.51, 0.55, 0.59] + 0.32
		//indx = findindex(scaleDwellTime, dwellTime);
		//CICScaleDelay = scaleDelay[indx];
		long CICScaleDelay = 1; //scaleDelay[indx];
		acqTm = (int)((nrPnts + ignorePnts) * dwellTime + CICDelay + CICScaleDelay +0.5);
	}

	return(acqTm + 1);
}


/*******************************************************************************

  Generate code to make a shaped RF pulse with amplitude and frequency tables
  Format is:
  chirprf2(channel, tables, phaseOffset, tableSize, stepDuration)

  Minimum step length is 2 us.

  At the end of the pulse switch-off is a 2 us delay before the end
  of the command. So total (minimum) length is pgo + 2*n + 2

*******************************************************************************/

short ChirpedRF2(DLLParameters* par, char* args)
{
	CText channelName = " ", tableName = " ", phName = " ", tableSizeName = " ", stepName = " ";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "channel, tables, phaseOffset, tableSize, stepDuration", "ccccc", "ttttt", &channelName, &tableName, &phName, &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		// Extract the values from the arguments checking data type
		int32 tableSize;
		uint32 tableAdrs;
		uint32 tableEntries;
		float tableStep;
		uint32 phaseOffset;

		try
		{
			EvaluateArg(par->itfc, tableName.Str(), MATRIX2D, tableAdrs, tableEntries);
			EvaluateArg(par->itfc, tableSizeName.Str(), INTEGER, tableSize);
			EvaluateArg(par->itfc, phName.Str(), INTEGER, phaseOffset);
			EvaluateArg(par->itfc, stepName.Str(), FLOAT32, tableStep);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		// Select the DDS address and gate TTL values depending on the channel
		uint32 ddsAdrs, ddsGate, hpaGate;
		if (channelName == "1")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x4000;
			ddsGate = 0x0008;
		}
		else if (channelName == "2")
		{
			ddsAdrs = 0x128E;
			hpaGate = 0x0100;
			ddsGate = 0x0002;
		}
		else
		{
			ErrorMessage("Invalid channel reference: %s", channelName.Str());
			return(ERR);
		}

		float pgo = psInfo.pgo * 100;

		if (tableEntries == 3 * tableSize) // Amplitude and frequency shaped pulses have triple the number of entries
		{
			uint32 loopNr = (psInfo.lineCnt + 18) * 3;

			if (tableStep < 4)
			{
				ErrorMessage("Table step duration must be >= 4 us");
				return(ERR);
			}

			if (tableSize - 1 <= 0)
			{
				ErrorMessage("Must be at least 2 ampltiude/frequency steps in table");
				return(ERR);
			}

			// Tweak the timing so we can control the delays
			uint32 delay0 = nuint(pgo - 169);
			uint32 delay1 = nuint(100 * (tableStep - 4) + 195);
			uint32 delay2 = nuint(100 * (tableStep - 4) + 210);
			uint32 delay3 = nuint(100 * (tableStep - 4) + 495 - delay2);

			// Note the delays here are the minimum 15 is required for reading table values. All others can use 10

			// Initialise table amplitude
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate); // Turn on RF gate
			ps->push_back(delay0);  ps->push_back(0x00006081);   ps->push_back(0x00000004);      // Set FIFO Control Register (DDC disable data capture from DDC to FIFO)
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1);   // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(tableAdrs + 0);   // Set index to 0
			// Switch on RF with PGO first using first amplitude value from table
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1);   // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table & inc adrs
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(phaseOffset);     // Write phase to DDS with offset

			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read frequency hiword from table & inc adrs
			ps->push_back(10);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write frequency to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read frequency lowword from table & inc adrs
			ps->push_back(10);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write frequency to DDS
			// Turn on the RF
			ps->push_back(delay1);  ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate | ddsGate); // Turn on DDS output gate as well
			// Start of loop
			ps->push_back(10);      ps->push_back(0x04000000);   ps->push_back(tableSize - 1);   // Loop start (note do one loop less than needed since first pulse is set up before)
			// Increment table
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1);   // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(0x02000000 + 3);  // Increment offset by 3
			// Set amplitude and phase
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1);   // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO & inc adrs
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table & inc adrs
			ps->push_back(10);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(phaseOffset);     // Write phase to DDS with offset

			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read frequency hi-word from table & inc adrs
			ps->push_back(10);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write frequency to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read frequency lo-word from table & inc adrs
			ps->push_back(delay2);  ps->push_back(ddsAdrs);      ps->push_back(0x01000000);       // Write frequency to DDS
			// End of loop
			ps->push_back(10);     ps->push_back(0x05000000);    ps->push_back(loopNr);          // Branch to adrs loopNr
			// Turn off RF
			ps->push_back(delay3);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait for last step to finish
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL); // Turn off RF gates
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(0x00000000);      // Write zero amplitude to DDS
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(0x00000000);      // Write zero phase to DDS
			ps->push_back(100 - 40);  ps->push_back(0x09000000);   ps->push_back(0x00000000);    // Wait for DDS to update and make total a multiple of 1 us
			psInfo.lineCnt += 40;
		}
		else
		{
			ErrorMessage("Real table size should be three times the number of steps (amplitude+freqHi+freqLo)");
			return(ERR);
		}

	}
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "channel, tables, phaseOffset, tableSize, stepDuration", "eeeee", "qqqqq", &channelName, &tableName, &phName, &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		if (channelName[0] == 'p')
			InsertUniqueStringIntoList(phName.Str(), &parList, szList);

		uint32 adrs = psInfo.lineCnt * 6;
		long pos;

		//	AddPhaseToUpdateTable(phName.Str(), adrs + 76, 0);
		//	AddPhaseToUpdateTable(phName.Str(), adrs + 166, 0);

		psInfo.lineCnt += 40;
	}


	return(OK);
}

/*******************************************************************************
  Decrement a table index
 
    decindex(table, dec)
 
    table : name of table array(2x1) generated with saveTable.
    dec : amount by which index should be decremented(positive only)
 
    duration = 1 us
 
*******************************************************************************/

short DecrementTableIndex(DLLParameters* par, char* args)
{
	CText tableName;
	CText decrementName;
	short nrArgs;
	uint32 decrement;
	uint32 tableAdrs;
	uint32 tableEntries;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "table, decrement", "cc", "tt", &tableName, &decrementName)) < 0)
			return(nrArgs);

		try
		{
			EvaluateArg(par->itfc, tableName.Str(), MATRIX2D, tableAdrs, tableEntries);
			EvaluateArg(par->itfc, decrementName.Str(), INTEGER, decrement);
		}
		catch (char* errorStr)
		{
			ErrorMessage(errorStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		ps->push_back(20); ps->push_back(0x00006081); ps->push_back(0x00000004);
		ps->push_back(20); ps->push_back(0x00006084); ps->push_back(tableAdrs - 1);
		ps->push_back(20); ps->push_back(0x01006086); ps->push_back(0);
		ps->push_back(20); ps->push_back(0x01006086); ps->push_back(0);
		ps->push_back(20); ps->push_back(0x00006086); ps->push_back(0x03000000 + decrement);
		psInfo.lineCnt += 5;
	}
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "table, increment", "ee", "qq", &tableName, &decrementName)) < 0)
			return(nrArgs);

		if (tableName[0] == 't')
			InsertUniqueStringIntoList(tableName.Str(), &parList, szList);
		if (tableName[0] == 'n')
			InsertUniqueStringIntoList(decrementName.Str(), &parList, szList);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;

		if (IsSequenceVariable(tableName.Str(), varPos))
			AddToVariableUpdateTable(varPos, adrs + 10, 0);
		if (IsSequenceVariable(decrementName.Str(), varPos))
			AddToVariableUpdateTable(varPos, adrs + 28, 0);
		psInfo.lineCnt += 5;
	}

	return(OK);
}

/*******************************************************************************

  Generate code to add a delay
  Format is:
  delay(duration)

*******************************************************************************/

short Delay(DLLParameters* par, char* args)
{
	CText delayName;
	short nrArgs;
	Variable result;

	uint32 startAdrs = psInfo.lineCnt * 6;

	// In compile mode extract and evaluate the value of each passed argument and embed these in the event table (psInfo.ps) which is designed to generate a delay
	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "delay value (us)", "c", "t", &delayName)) < 0)
			return(nrArgs);

		float delay;
		uint32  tableAdrs, tableEntries;

		// Extract the delay value or table reference
		if (delayName[0] != 't') // Delay is a variable or number (units us)
		{
			try
			{
				EvaluateArg(par->itfc, delayName.Str(), FLOAT32, delay);
			}
			catch (char* errStr)
			{
				ErrorMessage(errStr);
				return(ERR);
			}
			if (delay < 0)
			{
				ErrorMessage("Delay is negative");
				return(ERR);
			}
			if (delay <= 0.1)
			{
				ErrorMessage("Delay is < 100 ns");
				return(ERR);
			}
		}
		else // Delay is from a table (units clock cycles)
		{
			try
			{
				EvaluateArg(par->itfc, delayName.Str(), MATRIX2D, tableAdrs, tableEntries);
			}
			catch (char* errStr)
			{
				ErrorMessage(errStr);
				return(ERR);
			}
		}


		std::vector<uint32>* ps = &(psInfo.ps);
		uint32 psOffset = ps->size();

		// Generate the event table code
		if (delayName[0] != 't') // Using a number in us (min delay is 0.1 us)
		{
			CText txt;
			txt.Format("delay(%s) [%g us]", delayName.Str(), delay);
			ETInfo info = { psInfo.lineCnt, txt.Str() };
			eventTableInfo.push_back(info);

			uint32 digDelay = nint(delay * 100);
			ps->push_back(digDelay);       ps->push_back(0x09000000); ps->push_back(0);
			psInfo.lineCnt += 1;
		}
		else  // Update using a table - min delay 1.7 us max delay 42 s (table must subtract 160 (== 1.6 us) to get correct timing)
		{
			CText txt;
			txt.Format("delay(%s) [table]", delayName.Str());
			ETInfo info = { psInfo.lineCnt, txt.Str() };
			eventTableInfo.push_back(info);

			uint32 valueAdrsLo = psInfo.lineCnt * 6 + 66 + 0x3000; // Points to lower word in digDelay in event table
			uint32 valueAdrsHi = psInfo.lineCnt * 6 + 67 + 0x3000; // Points to higher word in digDelay in event table

			ps->push_back(10);               ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1);   // dspWrite to set start table address
			ps->push_back(15);               ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);               ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(15);               ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(15);               ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);               ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(15);               ps->push_back(valueAdrsHi);  ps->push_back(0x01000000);      // Write high amp word to last delay 
			ps->push_back(15);               ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Not sure why this is needed, but it is!
			ps->push_back(15);               ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(15);               ps->push_back(valueAdrsLo);  ps->push_back(0x01000000);      // Write low amp word to last delay
			ps->push_back(15);               ps->push_back(0x09000000);   ps->push_back(0);               // Not sure why this is needed, but it is!
			ps->push_back(10);               ps->push_back(0x09000000);   ps->push_back(0);               // This is the delay line - gets updated by table
			psInfo.lineCnt += 12;
		}
	}

// In table mode extract the parameter names as they appear in the pulse program (not in pulse program blocks) and 
// add these to the update table. Then determine the location and value of these parameters in the event table.
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "delay value (us)", "e", "q", &delayName)) < 0)
			return(nrArgs);

		if (delayName[0] == 'd') // || duration[0] == 't')
			InsertUniqueStringIntoList(delayName.Str(), &parList, szList);

		long pos;

		if (IsSequenceVariable(delayName.Str(), pos))
		{
			AddToVariableUpdateTable(pos + 0x1000, startAdrs + 1, 0);
			AddToVariableUpdateTable(pos,          startAdrs + 0, 0);
		}
		if (IsSequenceParameter(delayName.Str(), pos))
		{
			AddToFixedUpdateTable(0, pos + 0x1000, startAdrs + 1, 0);
			AddToFixedUpdateTable(0, pos,          startAdrs + 0, 0);
		}
		if(psInfo.mode == "tables")
			psInfo.lineCnt += 1;
	}

	return(OK);
}

/*******************************************************************************
  Return the version number
*******************************************************************************/

short GetPPVersion(DLLParameters* par, char* args)
{
	par->retVar[1].MakeAndSetFloat(VERSION);
	par->nrRetVar = 1;
	return(OK);
}


short IncRxFrequency(DLLParameters* par, char* args)
{
	ErrorMessage("Not implemented yet on FX3");
	return(ERR);
}

short IncTxAmplitude(DLLParameters* par, char* args)
{
	ErrorMessage("Not implemented yet on FX3");
	return(ERR);
}


short IncTxFrequency(DLLParameters* par, char* args)
{
	ErrorMessage("Not implemented yet on FX3");
	return(ERR);
}


/*******************************************************************************

  Use the update table to modify the p.s. event table. Note that the event
  table is modified in place - it is not returned. This minimises time and
  memory use.

*******************************************************************************/

short UpdateEventTable(DLLParameters* par, char* args)
{
	short nrArgs;
	CText eventTableName;
	Variable *eventTable, updateTable;
	short tableType;

	if ((nrArgs = ArgScan(par->itfc, args, 2, "event_table, update_table", "ce", "tv", &eventTableName, &updateTable)) < 0)
		return(nrArgs);

	eventTable = GetVariable(par->itfc, ALL, eventTableName.Str(), tableType);


	if (tableType != DMATRIX2D || (eventTable->GetDimX()%3) != 0 || eventTable->GetDimY() != 1)
	{
		ErrorMessage("The event-table parameter should be a vector with length a multiple of 3");
		return(ERR);
	}

	if (updateTable.GetType() != DMATRIX2D || updateTable.GetDimX() != 4)
	{
		ErrorMessage("The update-table parameter should be a 2D matrix with 4 columns");
		return(ERR);
	}

	double** events= eventTable->GetDMatrix2D();
	double** updates = updateTable.GetDMatrix2D();

	uint32 nrEvents = eventTable->GetDimX() / 3;
	uint32 nrParameters = updateTable.GetDimY();

	// Loop over the update table modifying the relevant pulse sequence event table entries
	for (int parameter = 0; parameter < nrParameters; parameter ++)
	{
		bool hiWord = (((uint32)nuint(updates[parameter][1]) & 0x1000) > 0);
		uint32 adrs = (uint32)nuint(updates[parameter][2]);
		uint32 value = (uint32)nuint(updates[parameter][3]);

		uint32 adrsWord = (adrs >> 1);
		//TextMessage("Adrs = %ld\n", adrs);
		//TextMessage("Hi-word = %d\n", hiWord);
		// Because we are writing the 32 bit word in two steps we need to keep the other 16 bit part unchanged  
		uint32 currentValue = nuint(events[0][adrsWord]);
		if (hiWord)
			events[0][adrsWord] = (value << 16) + (0x0000FFFF & currentValue);
		else
			events[0][adrsWord] = (0xFFFF0000 & currentValue) + value;
		
		//TextMessage("Value = %X\n", nuint(events[0][adrsWord]));
	
	}


	return(OK);
}


/*******************************************************************************

  Generate code to add a (long) delay
  Format is:
  wait(duration)

*******************************************************************************/

short Wait(DLLParameters* par, char* args)
{
	CText delayName;
	short nrArgs;
	Variable result;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "delay value (us)", "c", "t", &delayName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("wait(%s)", delayName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);
		// We need double precision because of potentially large numbers
		double delay;
		try
		{
			EvaluateArg(par->itfc, delayName.Str(), FLOAT64, delay);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}
		// For  delays > 42 repeat the 42 s delay and then add a remainder
		if (delay > 42e6)
		{
			uint32 nrLoops = (uint32)(delay / 42.0e6);
			uint32 remainder = nuint(delay - nrLoops * 42.0e6);
			uint32 digDelay1 = nuint(42e6 * 100);
			uint32 digDelay2 = remainder * 100 - 10 - 10 * nrLoops;

			if (digDelay2 < 10)
			{
				ErrorMessage("Invalid remainder time for delay command");
				return(ERR);
			}

			std::vector<uint32>* ps = &(psInfo.ps);
			uint32 loopIndex = (psInfo.lineCnt + 1) * 3;
			ps->push_back(10);         ps->push_back(0x04000000);   ps->push_back(nrLoops);
			ps->push_back(digDelay1);  ps->push_back(0x09000000);   ps->push_back(0);
			ps->push_back(10);         ps->push_back(0x05000000);   ps->push_back(loopIndex);
			ps->push_back(digDelay2);  ps->push_back(0x09000000);   ps->push_back(0);
			psInfo.lineCnt += 4;
		}
		else if(delay >= 0.4) // For short delays < 42 s we could use a single command but for compatibility with STD SW we keep the event table the same
		{
			uint32 digDelay = nuint(delay * 100 - 30);
			uint32 nrLoops = 1;

			std::vector<uint32>* ps = &(psInfo.ps);
			uint32 loopIndex = (psInfo.lineCnt + 1) * 3;
			ps->push_back(10);         ps->push_back(0x04000000);   ps->push_back(nrLoops);
			ps->push_back(10);         ps->push_back(0x09000000);   ps->push_back(0);
			ps->push_back(10);         ps->push_back(0x05000000);   ps->push_back(loopIndex);
			ps->push_back(digDelay);   ps->push_back(0x09000000);   ps->push_back(0);
			psInfo.lineCnt += 4;
		}
		else
		{
			ErrorMessage("The shortest long delay is 0.4 us");
			return(ERR);
		}
	}
	// When calculating the table addresses we need to consider the delay
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "delay value (us)", "e", "q", &delayName)) < 0)
			return(nrArgs);

		if (delayName[0] == 'w') 
			InsertUniqueStringIntoList(delayName.Str(), &parList, szList);

		long pos;
		uint32 adrs = psInfo.lineCnt * 6;

		if (IsSequenceParameter(delayName.Str(), pos))
		{
			AddToFixedUpdateTable(0, pos + 0x010000, adrs + 4, 0);
			AddToFixedUpdateTable(0, pos + 0x021000, adrs + 7, 0);
			AddToFixedUpdateTable(0, pos + 0x020000, adrs + 6, 0);
			AddToFixedUpdateTable(0, pos + 0x041000, adrs + 19, 0);
			AddToFixedUpdateTable(0, pos + 0x040000, adrs + 18, 0);
		}

		if (IsSequenceVariable(delayName.Str(), pos))
		{
			AddToVariableUpdateTable(pos + 0x010000, adrs + 4, 0);  // nrLoops
			AddToVariableUpdateTable(pos + 0x021000, adrs + 7, 0);  // digDelay1 upper
			AddToVariableUpdateTable(pos + 0x020000, adrs + 6, 0);  // digDelay1 lower
			AddToVariableUpdateTable(pos + 0x041000, adrs + 19, 0); // digDelay2 upper
			AddToVariableUpdateTable(pos + 0x040000, adrs + 18, 0); // digDelay2 lower
		}
		psInfo.lineCnt += 4;
	}

	return(OK);
}


/*******************************************************************************

  Generate code to add a loop
  Format is:
  loop(loopName, nrLoops, [duration])

*******************************************************************************/

short LoopStart(DLLParameters* par, char* args)
{
	CText loopName, nrLoopsName, durationName = "1";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "loopName, nrLoops, [duration]", "ccc", "ttt", &loopName, &nrLoopsName, &durationName)) < 0)
			return(nrArgs);

		// Evaluate the arguments
		uint32 nrLoops;
		float durationF;
		try
		{
			EvaluateArg(par->itfc, nrLoopsName.Str(), INTEGER, nrLoops);
			EvaluateArg(par->itfc, durationName.Str(), FLOAT32, durationF);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		// Check for invalid values
		//if (nrLoops <= 0)
		//{
		//	ErrorMessage("Number of loops must be greater than zero");
		//	return(ERR);
		//}
		uint32 duration = nint(durationF * 100);
		if (duration < 10)
		{
			ErrorMessage("Duration must be >= 100 ns");
			return(ERR);
		}

		// Generate report info
		CText txt;
		if (nrArgs == 2)
			txt.Format("loop(%s, %s) [1 us]", loopName.Str(), nrLoopsName.Str());
		else if (nrArgs == 3)
			txt.Format("loop(%s, %s, %s)  [%g us]", loopName.Str(), nrLoopsName.Str(), durationName.Str(), durationF);
		else
		{
			ErrorMessage("Invalid number of arguments (2/3)");
			return(ERR);
		}
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		// Add the event
		AddEvent(duration, 0x04000000, nrLoops);

		// Make a note of the loop command position in the event table
		loopNrStk.push_back(psInfo.lineCnt);
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "loopName, nrLoops", "ee", "qq", &loopName, &nrLoopsName)) < 0)
			return(nrArgs);

		if (nrLoopsName[0] == 'n')
			InsertUniqueStringIntoList(nrLoopsName.Str(), &parList, szList);

		long pos;
		uint32 adrs = psInfo.lineCnt * 6;
		if (IsSequenceVariable(nrLoopsName.Str(), pos))
		{
			AddToVariableUpdateTable(pos + 0x1000, adrs + 5, 0);
			AddToVariableUpdateTable(pos,          adrs + 4, 0);
		}
		if (IsSequenceParameter(nrLoopsName.Str(), pos))
		{
			AddToFixedUpdateTable(0, pos + 0x1000, adrs + 5, 0);
			AddToFixedUpdateTable(0, pos         , adrs + 4, 0);
		}
		psInfo.lineCnt += 1;

	}

	return(OK);
}


/*******************************************************************************

  Generate code to end a loop
  Format is:
  loopend(loopName)

*******************************************************************************/

short LoopEnd(DLLParameters* par, char* args)
{
	CText loopName,durationName = "1";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "loopName, [duration]", "cc", "tt", &loopName, &durationName)) < 0)
			return(nrArgs);

		// Evaluate the duration
		float durationF;
		try
		{
			EvaluateArg(par->itfc, durationName.Str(), FLOAT32, durationF);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		// Check for valid duration
		uint32 duration = nint(durationF * 100);
		if (duration < 2)
		{
			ErrorMessage("Duration must be >= 20 ns");
			return(ERR);
		}

		// Save report output
		CText txt;
		if (nrArgs == 1)
			txt.Format("loopend(%s) [1 us]", loopName.Str());
		else if (nrArgs == 2)
			txt.Format("loopend(%s,%s) [%g us]", loopName.Str(), durationName.Str(), durationF);
		else
		{
			ErrorMessage("Invalid number of arguments (1/2)");
			return(ERR);
		}
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

	// Check for and remove the corresponding loop index
		if(loopNrStk.size() == 0)
		{
			ErrorMessage("Invalid loop reference");
			return(ERR);
		}
		uint32 loopToIndex = loopNrStk[loopNrStk.size()-1]*3;
		loopNrStk.pop_back();

		// Add the event
		AddEvent(duration, 0x05000000, loopToIndex);
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		psInfo.lineCnt += 1;
	}

	return(OK);
}


/*******************************************************************************

  Generate code to make a shaped RF pulse (amplitude table only)
  Format is:
  shapedrf1(channel, table(s), phaseOffset, tableSize, stepDuration)

  Minimum step length is 2 us.

  At the end of the pulse switch-off is a 2 us delay before the end
  of the command. So total (minimum) length is pgo + 2*n + 2
*******************************************************************************/

short ShapedRFPulse1(DLLParameters* par, char* args)
{
	CText channelName = " ", tableName = " ", phName = " ", tableSizeName = " ", stepName = " ";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "channel, table(s), phaseOffset, tableSize, stepDuration", "ccccc", "ttttt", &channelName, &tableName, &phName, &tableSizeName, &stepName)) < 0)
			return(nrArgs);


		CText txt;
		txt.Format("shapedrf1(%s, %s, %s, %s, %s)", channelName.Str(), tableName.Str(), phName.Str(), tableSizeName.Str(), stepName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		// Extract the values from the arguments checking data type
		int32 tableSize;
		uint32 tableAdrs;
		uint32 tableEntries;
		float tableStep;
		uint32 phaseOffset;

		try
		{
			EvaluateArg(par->itfc, tableName.Str(), MATRIX2D, tableAdrs, tableEntries);
			EvaluateArg(par->itfc, tableSizeName.Str(), INTEGER, tableSize);
			EvaluateArg(par->itfc, phName.Str(), INTEGER, phaseOffset);
			EvaluateArg(par->itfc, stepName.Str(), FLOAT32, tableStep);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		// Select the DDS address and gate TTL values depending on the channel
		uint32 ddsAdrs, ddsGate, hpaGate;
		if (channelName == "1")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x4000;
			ddsGate = 0x0008;
		}
		else if(channelName == "2")
		{
			ddsAdrs = 0x128E;
			hpaGate = 0x0100;
			ddsGate = 0x0002;
		}
		else
		{
			ErrorMessage("Invalid channel reference: %s", channelName.Str());
			return(ERR);
		}

		float pgo = psInfo.pgo * 100;

		// Amplitude tables have the same number of elements as the table size
		if (tableEntries == tableSize) // Amplitude shaped pulse
		{
			uint32 loopNr = (psInfo.lineCnt + 14) * 3;

			if(tableStep < 2)
			{
				ErrorMessage("Table step duration must be >= 2 us");
				return(ERR);
			}

			if(tableSize-1 <= 0)
			{
				ErrorMessage("Must be at least 2 amplitude steps in table");
				return(ERR);
			}

			// Note that the sum of all delays has been calculated and checked to ensure that the total pulse length is pgo + 2*steps + 1 
			// for the case of 2 us step duration

			uint32 delay0 = nuint(pgo-204);
			uint32 delay1 = nuint(100 * (tableStep - 2) + 67);
			uint32 delay2 = nuint(100 * (tableStep - 2) + 55);
			uint32 delay3 = nuint(100 * (tableStep - 2) + 252 - delay2);

			// Note the delays here are the minimum 15 is required for reading table values. All others can use 10

			// Initialise table amplitude
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate); // Turn on RF gate
			ps->push_back(delay0);  ps->push_back(0x00006081);   ps->push_back(0x00000004);      // Set FIFO Control Register (DDC disable data capture from DDC to FIFO)
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(tableAdrs + 0); // Set index to 0
			// Switch on RF with PGO first using first amplitude value from table
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(phaseOffset);     // Write phase to DDS
			ps->push_back(delay1);  ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate | ddsGate); // Turn on DDS output gate as well
			// Start of loop
			ps->push_back(10);      ps->push_back(0x04000000);   ps->push_back(tableSize-1);     // Loop start (note do one loop less than needed since first pulse is set up before)
			// Increment table
			ps->push_back(10);      ps->push_back(0x09000000);   ps->push_back(0x00000000);      // LoopNr points to start of this line
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(0x02000000 + 1);  // Increment offset by 1
			// Set amplitude
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(delay2);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait to get internal step length correct
			// End of loop
			ps->push_back(10);     ps->push_back(0x05000000);    ps->push_back(loopNr);          // Branch to adrs loopNr
			// Turn off RF
			ps->push_back(delay3);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait for last step to finish
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL); // Turn off RF gates
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(0x00000000);      // Write zero amplitude to DDS
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(0x00000000);      // Write zero phase to DDS
			ps->push_back(100-40);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait for DDS to update and make total a multiple of 1 us
			psInfo.lineCnt += 33;
		}
		else
		{
			ErrorMessage("Real table size does not match size argument");
			return(ERR);
		}

	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "channel, table(s), phaseOffset, tableSize, stepDuration", "eeeee", "qqqqq", &channelName, &tableName, &phName, &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		if (channelName[0] == 'p')
			InsertUniqueStringIntoList(phName.Str(), &parList, szList);

		uint32 adrs = psInfo.lineCnt * 6;
		long pos;

	//	AddPhaseToUpdateTable(phName.Str(), adrs + 76, 0);
	//	AddPhaseToUpdateTable(phName.Str(), adrs + 166, 0);
	
		psInfo.lineCnt += 33;
	}


	return(OK);
}


/*******************************************************************************

  Generate code to make a shaped RF pulse with amplitude and phase tables
  Format is:
  shapedrf2(channel, tables, phaseOffset, tableSize, stepDuration)

  Minimum step length is 2 us. 

  At the end of the pulse switch-off is a 2 us delay before the end
  of the command. So total (minimum) length is pgo + 2*n + 2

*******************************************************************************/

short ShapedRFPulse2(DLLParameters* par, char* args)
{
	CText channelName = " ", tableName = " ", phName = " ", tableSizeName = " ", stepName = " ";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "channel, tables, phaseOffset, tableSize, stepDuration", "ccccc", "ttttt", &channelName, &tableName, &phName, &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("shapedrf2(%s, %s, %s, %s, %s)", channelName.Str(), tableName.Str(), phName.Str(), tableSizeName.Str(), stepName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		// Extract the values from the arguments checking data type
		int32 tableSize;
		uint32 tableAdrs;
		uint32 tableEntries;
		float tableStep;
		uint32 phaseOffset;

		try
		{
			EvaluateArg(par->itfc, tableName.Str(), MATRIX2D, tableAdrs, tableEntries);
			EvaluateArg(par->itfc, tableSizeName.Str(), INTEGER, tableSize);
			EvaluateArg(par->itfc, phName.Str(), INTEGER, phaseOffset);
			EvaluateArg(par->itfc, stepName.Str(), FLOAT32, tableStep);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		// Select the DDS address and gate TTL values depending on the channel
		uint32 ddsAdrs, ddsGate, hpaGate;
		if (channelName == "1")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x4000;
			ddsGate = 0x0008;
		}
		else if (channelName == "2")
		{
			ddsAdrs = 0x128E;
			hpaGate = 0x0100;
			ddsGate = 0x0002;
		}
		else
		{
			ErrorMessage("Invalid channel reference: %s", channelName.Str());
			return(ERR);
		}

		float pgo = psInfo.pgo * 100;

		if (tableEntries == 2 * tableSize) // Amplitude and phased shaped pulses have double the number of entries
		{
			uint32 loopNr = (psInfo.lineCnt + 15) * 3;

			if (tableStep < 2)
			{
				ErrorMessage("Table step duration must be >= 2 us");
				return(ERR);
			}

			if (tableSize - 1 <= 0)
			{
				ErrorMessage("Must be at least 2 amplitude steps in table");
				return(ERR);
			}

			// Tweak the timing so we can control the delays
			uint32 delay0 = nuint(pgo - 145);
			uint32 delay1 = nuint(100 * (tableStep - 2) + 40);
			uint32 delay2 = nuint(100 * (tableStep - 2) + 50);
			uint32 delay3 = nuint(100 * (tableStep - 2) + 270 - delay2);

			// Note the delays here are the minimum 15 is required for reading table values. All others can use 10

			// Initialise table amplitude
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate); // Turn on RF gate
			ps->push_back(delay0);  ps->push_back(0x00006081);   ps->push_back(0x00000004);      // Set FIFO Control Register (DDC disable data capture from DDC to FIFO)
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(tableAdrs + 0); // Set index to 0
			// Switch on RF with PGO first using first amplitude value from table
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read phase from table
			ps->push_back(10);      ps->push_back(ddsAdrs);      ps->push_back(0x02000000 + phaseOffset);     // Write phase to DDS with offset
			// Turn on the RF
			ps->push_back(delay1);  ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate | ddsGate); // Turn on DDS output gate as well
			// Start of loop
			ps->push_back(10);      ps->push_back(0x04000000);   ps->push_back(tableSize - 1);     // Loop start (note do one loop less than needed since first pulse is set up before)
			// Increment table
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(0x02000000 + 2);  // Increment offset by 2
			// Set amplitude and phase
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read phase from table
			ps->push_back(delay2);  ps->push_back(ddsAdrs);      ps->push_back(0x02000000 + phaseOffset);      // Write phase to DDS with offset
			// End of loop
			ps->push_back(10);     ps->push_back(0x05000000);    ps->push_back(loopNr);          // Branch to adrs loopNr
			// Turn off RF
			ps->push_back(delay3);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait for last step to finish
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL); // Turn off RF gates
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(0x00000000);      // Write zero amplitude to DDS
			ps->push_back(15);      ps->push_back(ddsAdrs);      ps->push_back(0x00000000);      // Write zero phase to DDS
			ps->push_back(100 - 40);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait for DDS to update and make total a multiple of 1 us
			psInfo.lineCnt += 34;
		}
		else
		{
			ErrorMessage("Real table size should be twice the number of steps (amplitude+phase)");
			return(ERR);
		}

	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "channel, table(s), phaseOffset, tableSize, stepDuration", "eeeee", "qqqqq", &channelName, &tableName, &phName, &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		if (channelName[0] == 'p')
			InsertUniqueStringIntoList(phName.Str(), &parList, szList);

		uint32 adrs = psInfo.lineCnt * 6;
		long varPos;

		// Update table phases
		AddPhaseToUpdateTable(phName.Str(), adrs + 76, 0);

		// Update table loop limit
		if (IsSequenceVariable(tableSizeName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 88, 0);
		}
		if (IsSequenceParameter(tableSizeName.Str(), varPos))
		{
			AddToFixedUpdateTable(2, varPos, adrs + 88, 0);
		}

		// Update step duration
		if (IsSequenceVariable(stepName.Str(), varPos))
		{ 
			AddToVariableUpdateTable(varPos + 0x1000, adrs + 79, 0);
			AddToVariableUpdateTable(varPos, adrs + 78, 0);
			AddToVariableUpdateTable(varPos + 0x1000, adrs + 163, 0);
			AddToVariableUpdateTable(varPos, adrs + 162, 0);
			AddToVariableUpdateTable(varPos + 0x1000, adrs + 175, 0);
			AddToVariableUpdateTable(varPos, adrs + 174, 0);
		}

		if (IsSequenceParameter(stepName.Str(), varPos))
		{
			AddToFixedUpdateTable(2, varPos + 0x1000, adrs + 79, 0);
			AddToFixedUpdateTable(2, varPos, adrs + 78, 0);
			AddToFixedUpdateTable(3, varPos + 0x1000, adrs + 163, 0);
			AddToFixedUpdateTable(3, varPos, adrs + 162, 0);
			AddToFixedUpdateTable(4, varPos + 0x1000, adrs + 175, 0);
			AddToFixedUpdateTable(4, varPos, adrs + 174, 0);
		}
		// Update table addresses
		if (IsSequenceParameter(tableName.Str(), varPos))
		{
			AddToFixedUpdateTable(2, varPos, adrs + 16, 0);
			AddToFixedUpdateTable(3, varPos, adrs + 22, 0);
			AddToFixedUpdateTable(2, varPos, adrs + 28, 0);
			AddToFixedUpdateTable(2, varPos, adrs + 94, 0);
			AddToFixedUpdateTable(2, varPos, adrs + 118, 0);
		}

		psInfo.lineCnt += 34;
	}


	return(OK);
}


/*******************************************************************************

  Generate code to make a dual shaped RF pulse

  Format is:
  shapedrf1(atables, phaseOffset1, phaseOffset2, tableSize, stepDuration)

  aptable are 2 concatenated tables ch1Amp, ch2Amp

  Minimum step length is 4 us. Because channels cannot be simultaneously
  updated Channel 2 is displaced by 1 us w.r.t. Channel 1.
  At the end of the channel 1 pulse switch-off is a 2 us delay before the end
  of the command. So total (minimum) length is pgo + 2*n + 2

*******************************************************************************/

short DualShapedRFPulse1(DLLParameters* par, char* args)
{
	CText channelName = " ", tableName = " ", ph1Name = " ", ph2Name = " ", tableSizeName = " ", stepName = " ";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "aptables, phase1Offset, phase2Offset, tableSize, stepDuration", "ccccc", "ttttt", &tableName, &ph1Name, &ph2Name, &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		// Extract the values from the arguments checking data type
		int32 tableSize;
		uint32 tableAdrs, tableEntries;
		float tableStep;
		uint32 phaseOffset1;
		uint32 phaseOffset2;

		try
		{
			EvaluateArg(par->itfc, tableName.Str(), MATRIX2D, tableAdrs, tableEntries);
			EvaluateArg(par->itfc, tableSizeName.Str(), INTEGER, tableSize);
			EvaluateArg(par->itfc, ph1Name.Str(), INTEGER, phaseOffset1);
			EvaluateArg(par->itfc, ph2Name.Str(), INTEGER, phaseOffset2);
			EvaluateArg(par->itfc, stepName.Str(), FLOAT32, tableStep);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		// Select the DDS address and gate TTL values for dual channel
		uint32 ddsAdrs1, ddsAdrs2, ddsGate, hpaGate;

		ddsAdrs1 = 0x118E;
		ddsAdrs2 = 0x128E;
		hpaGate = 0x4100;
		ddsGate = 0x000A;


		float pgo = psInfo.pgo * 100;

		// Check table size - should be 2 times passed size
		if (tableEntries == 2 * tableSize) // amplitude dual shaped pulse
		{
			uint32 loopNr = (psInfo.lineCnt + 19) * 3;

			if (tableStep < 4)
			{
				ErrorMessage("Table step duration must be >= 4 us");
				return(ERR);
			}

			// Tweak the timing so we can control the delays
			uint32 delay0 = nuint(pgo - 290);
			uint32 delay1 = nuint(100 * (tableStep - 4) + 55);
			uint32 delay2 = nuint(100 * (tableStep - 4) + 140);
			uint32 delay3 = nuint(100 * (tableStep - 4) + 360 - delay2);

			// Initialise table amplitude
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | 0x4100); // Turn on RF gate
			ps->push_back(delay0);  ps->push_back(0x00006081);   ps->push_back(0x00000004);      // Set FIFO Control Register (DDC disable data capture from DDC to FIFO)
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(tableAdrs + 0); // Set index to 0
			// Switch on RF using first amplitude value from table
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(15);      ps->push_back(ddsAdrs1);     ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(75);      ps->push_back(ddsAdrs1);     ps->push_back(phaseOffset1);    // Write phase to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(15);      ps->push_back(ddsAdrs2);     ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(75);      ps->push_back(ddsAdrs2);     ps->push_back(phaseOffset2);    // Write phase to DDS
			// Turn on the RF
			ps->push_back(100);     ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | 0x4108); // Turn on DDS output for channel 1
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | 0x410A); // Turn on DDS output for channel 1&2 
			ps->push_back(delay1);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait to get the  initial step length correct (can't quite the first step delay to be long enough - should be 200 ns longer)
			// Start of loop
			ps->push_back(10);      ps->push_back(0x04000000);   ps->push_back(tableSize - 1);     // Loop start
			// Increment table indicies
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address (LoopNr points to start of this line)
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(0x02000000 + 2);  // Increment offset by 2
			// Set amplitude and phase
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(15);      ps->push_back(ddsAdrs1);     ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(75);      ps->push_back(ddsAdrs1);     ps->push_back(phaseOffset1);    // Write phase to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(15);      ps->push_back(ddsAdrs2);     ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(ddsAdrs2);     ps->push_back(phaseOffset2);    // Write phase to DDS
			ps->push_back(delay2);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait to get the internal step duration correct
			// End of loop
			ps->push_back(10);     ps->push_back(0x05000000);    ps->push_back(loopNr);          // Branch to adrs loopNr
			// Turn off RF
			ps->push_back(delay3);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait to get las step length correct
			ps->push_back(100);     ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | 0x00102); // Turn off RF ch 1
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL); // Turn off RF ch 1 & 2
			ps->push_back(10);      ps->push_back(ddsAdrs1);     ps->push_back(0x00000000);      // Write zero amplitude to DDS 1
			ps->push_back(60);      ps->push_back(ddsAdrs1);     ps->push_back(0x00000000);      // Write zero phase to DDS 1 (delay needs to be at least this long)
			ps->push_back(10);      ps->push_back(ddsAdrs2);     ps->push_back(0x00000000);      // Write zero amplitude to DDS 2
			ps->push_back(10);      ps->push_back(ddsAdrs2);     ps->push_back(0x00000000);      // Write zero phase to DDS 2 (total 2 us at end)
			psInfo.lineCnt += 43;
		}
		else
		{
			ErrorMessage("Real table size does not match size argument (should be 2 times larger)");
			return(ERR);
		}

	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "aptables, phaseOffset, tableSize, stepDuration", "eeeee", "qqqqq", &tableName, &ph1Name, &ph2Name, &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		//if (channelName[0] == 'n')
		//	InsertUniqueStringIntoList(channelName.Str(), &parList, szList);
		//if (amplitudeName[0] == 'n')
		//	InsertUniqueStringIntoList(amplitudeName.Str(), &parList, szList);

		//uint32 adrs = psInfo.lineCnt * 3;
		//long pos;
		//if (IsSequenceVariable(amplitudeName.Str(), pos))
		//{
		//	AddToUpdateTable(pos, adrs + 1, 0);
		//	AddToUpdateTable(pos, adrs + 2, 0);
		//}
		psInfo.lineCnt += 43;
	}


	return(OK);
}


/*******************************************************************************

  Generate code to make a dual shaped RF pulse with amplitude and phase control

  Format is:
  shapedrf2(aptables, phaseOffset1, phaseOffset2, tableSize, stepDuration)

  aptables are 4 concatenated tables ch1Amp, ch1Phase, ch2Amp, ch2Phase

  Minimum step length is 4 us. Because channels cannot be simultaneously
  updated Channel 2 is displaced by 1 us w.r.t. Channel 1.
  At the end of the channel 1 pulse switch-off is a 2 us delay before the end
  of the command. So total (minimum) length is pgo + 4*n + 2

*******************************************************************************/

short DualShapedRFPulse2(DLLParameters* par, char* args)
{
	CText channelName = " ", tableName = " ", ph1Name = " ", ph2Name = " ", tableSizeName = " ", stepName = " ";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "aptables, phase1Offset, phase2Offset, tableSize, stepDuration", "ccccc", "ttttt",&tableName, &ph1Name, &ph2Name , &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("dualshapedrf2(%s,%s,%s,%s,%s)", tableName.Str(), ph1Name.Str(), ph2Name.Str(), tableSizeName.Str(), stepName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		// Extract the values from the arguments checking data type
		int32 tableSize;
		uint32 tableAdrs, tableEntries;
		float tableStep;
		uint32 phaseOffset1;
		uint32 phaseOffset2;

		try
		{
			EvaluateArg(par->itfc, tableName.Str(), MATRIX2D, tableAdrs, tableEntries);
			EvaluateArg(par->itfc, tableSizeName.Str(), INTEGER, tableSize);
			EvaluateArg(par->itfc, ph1Name.Str(), INTEGER, phaseOffset1);
			EvaluateArg(par->itfc, ph2Name.Str(), INTEGER, phaseOffset2);
			EvaluateArg(par->itfc, stepName.Str(), FLOAT32, tableStep);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		// Select the DDS address and gate TTL values for dual channel
		uint32 ddsAdrs1, ddsAdrs2, ddsGate, hpaGate;
	
		ddsAdrs1 = 0x118E;
		ddsAdrs2 = 0x128E;
		hpaGate = 0x4100;
		ddsGate = 0x000A;


		float pgo = psInfo.pgo * 100;

		// Check table size - should be 4 times passed size
		if (tableEntries == 4*tableSize) // amplitude phase dual shaped pulse
		{
			uint32 loopNr = (psInfo.lineCnt + 21) * 3;

			if (tableStep < 4)
			{
				ErrorMessage("Table step duration must be >= 4 us");
				return(ERR);
			}

			// Tweak the timing so we can control the delays
			uint32 delay0 = nuint(pgo - 310);
			uint32 delay1 = nuint(100 * (tableStep - 4) + 60);
			uint32 delay2 = nuint(100 * (tableStep - 4) + 140);
			uint32 delay3 = nuint(100 * (tableStep - 4) + 360 - delay2);

			// Initialise table amplitude
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | 0x4100); // Turn on RF gate
			ps->push_back(delay0);  ps->push_back(0x00006081);   ps->push_back(0x00000004);      // Set FIFO Control Register (DDC disable data capture from DDC to FIFO)
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(tableAdrs + 0); // Set index to 0
			// Switch on RF using first amplitude value from table
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs1);     ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(65);      ps->push_back(ddsAdrs1);     ps->push_back(0x02000000+phaseOffset1);     // Write phase to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs2);     ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(80);      ps->push_back(ddsAdrs2);     ps->push_back(0x02000000+phaseOffset2);     // Write phase to DDS
			// Turn on the RF
			ps->push_back(100);     ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | 0x4108); // Turn on DDS output for channel 1
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | 0x410A); // Turn on DDS output for channel 1&2 
			ps->push_back(delay1);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait to get the  initial step length correct (can't quite the first step delay to be long enough - should be 200 ns longer)
			// Start of loop
			ps->push_back(10);      ps->push_back(0x04000000);   ps->push_back(tableSize - 1);     // Loop start
			// Increment table indicies
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address (LoopNr points to start of this line)
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006086);   ps->push_back(0x02000000 + 4);  // Increment offset by 4
			// Set amplitude and phase
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1); // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
			ps->push_back(10);      ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
			ps->push_back(10);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs1);     ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs1);     ps->push_back(0x02000000+phaseOffset1);     // Write phase to DDS
			ps->push_back(65);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs2);     ps->push_back(0x01000000);      // Write amplitude to DDS
			ps->push_back(15);      ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
			ps->push_back(10);      ps->push_back(ddsAdrs2);     ps->push_back(0x02000000+phaseOffset2);     // Write phase to DDS
			ps->push_back(delay2);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait to get the internal step duration correct
			// End of loop
			ps->push_back(10);     ps->push_back(0x05000000);    ps->push_back(loopNr);          // Branch to adrs loopNr
			// Turn off RF
			ps->push_back(delay3);  ps->push_back(0x09000000);   ps->push_back(0x00000000);      // Wait to get last step length correct
			ps->push_back(100);     ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | 0x00102); // Turn off RF ch 1
			ps->push_back(10);      ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL); // Turn off RF ch 1 & 2
			ps->push_back(10);      ps->push_back(ddsAdrs1);     ps->push_back(0x00000000);      // Write zero amplitude to DDS 1
			ps->push_back(60);      ps->push_back(ddsAdrs1);     ps->push_back(0x00000000);      // Write zero phase to DDS 1 (delay needs to be at least this long)
			ps->push_back(10);      ps->push_back(ddsAdrs2);     ps->push_back(0x00000000);      // Write zero amplitude to DDS 2
			ps->push_back(10);      ps->push_back(ddsAdrs2);     ps->push_back(0x00000000);      // Write zero phase to DDS 2 (total 2 us at end)
			psInfo.lineCnt += 47;
		}
		else
		{
			ErrorMessage("Real table size does not match size argument (should be 4 times larger)");
			return(ERR);
		}

	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "aptables, phaseOffset, tableSize, stepDuration", "eeeee", "qqqqq", &tableName, &ph1Name, &ph2Name, &tableSizeName, &stepName)) < 0)
			return(nrArgs);

		uint32 adrs = psInfo.lineCnt * 6;
		long varPos;

		// Update phases for each table
		AddPhaseToUpdateTable(ph1Name.Str(), adrs + 76, 0);
		AddPhaseToUpdateTable(ph2Name.Str(), adrs + 100, 0);

		// Update table loop limit
		if (IsSequenceVariable(tableSizeName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 124, 0);
		}
		if (IsSequenceParameter(tableSizeName.Str(), varPos))
		{
			AddToFixedUpdateTable(2, varPos, adrs + 124, 0);
		}

		// Update step duration
		if (IsSequenceVariable(stepName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos + 0x1000, adrs + 115, 0);
			AddToVariableUpdateTable(varPos, adrs + 114, 0);
			AddToVariableUpdateTable(varPos + 0x1000, adrs + 229, 0);
			AddToVariableUpdateTable(varPos, adrs + 228, 0);
			AddToVariableUpdateTable(varPos + 0x1000, adrs + 241, 0);
			AddToVariableUpdateTable(varPos, adrs + 240, 0);
		}
		if (IsSequenceParameter(stepName.Str(), varPos))
		{
			AddToFixedUpdateTable(5, varPos + 0x1000, adrs + 115, 0);
			AddToFixedUpdateTable(5, varPos, adrs + 114, 0);
			AddToFixedUpdateTable(6, varPos + 0x1000, adrs + 229, 0);
			AddToFixedUpdateTable(6, varPos, adrs + 228, 0);
			AddToFixedUpdateTable(7, varPos + 0x1000, adrs + 241, 0);
			AddToFixedUpdateTable(7, varPos, adrs + 240, 0);
		}
		// Update table addresses
		if (IsSequenceParameter(tableName.Str(), varPos))
		{
			AddToFixedUpdateTable(2, varPos, adrs + 16, 0);
			AddToFixedUpdateTable(3, varPos, adrs + 22, 0);
			AddToFixedUpdateTable(2, varPos, adrs + 28, 0);
			AddToFixedUpdateTable(2, varPos, adrs + 130, 0);
			AddToFixedUpdateTable(2, varPos, adrs + 154, 0);
		}

		psInfo.lineCnt += 47;
	}


	return(OK);
}

/*******************************************************************************

  Generate code to clear the TRex data summing array in BRAM
  Format is:
  cleardata(nrPnts, nrSums)

  Note that the number of points is ignored - the complete data BRAM is reset

  Duration 50 us.

*******************************************************************************/


short ClearData(DLLParameters* par, char* args)
{
	CText nrPointsName,nrSumsName = "1";
	short nrArgs;

	if ((nrArgs = ArgScan(par->itfc, args, 1, "number of points, number of summations", "cc", "tt", &nrPointsName, &nrSumsName)) < 0)
		return(nrArgs);

	CText txt;
	txt.Format("cleardata(%s,%s)", nrPointsName.Str(), nrSumsName.Str());
	ETInfo info = { psInfo.lineCnt, txt.Str() };
	eventTableInfo.push_back(info);

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		uint32 nrSums;
		if (nrArgs == 2)
		{
			try
			{
				EvaluateArg(par->itfc, nrSumsName.Str(), INTEGER, nrSums);

			}
			catch (char* errStr)
			{
				ErrorMessage(errStr);
				return(ERR);
			}
		}
		else
		{
			nrSums = 1;
		}

		std::vector<uint32>* ps = &(psInfo.ps);
		ps->push_back(20);       ps->push_back(0x04085); ps->push_back(nrSums); // Initialize the number of summations
		ps->push_back(4980);     ps->push_back(0x04084); ps->push_back(0);      // This is time enough to reset the BRAM (4096*10ns)
		psInfo.lineCnt += 2;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if (nrSumsName[0] == 'n')
			InsertUniqueStringIntoList(nrSumsName.Str(), &parList, szList);

		long pos;
		uint32 adrs = psInfo.lineCnt * 6;

		if (IsSequenceVariable(nrSumsName.Str(), pos))
		{
			AddToVariableUpdateTable(pos, adrs + 4, 0);
			AddToVariableUpdateTable(pos + 0x1000, adrs + 5, 0);
		}
		if (IsSequenceParameter(nrSumsName.Str(), pos))
		{
			AddToFixedUpdateTable(0, pos, adrs + 4, 0);
			AddToFixedUpdateTable(0, pos + 0x1000, adrs + 5, 0);
		}

		psInfo.lineCnt += 2;
	}

	return(OK);
}


/*******************************************************************************

  Generate code to set the receive frequency 
  Format is:
  setrxfreq(frequency)

*******************************************************************************/

short SetRxFreq(DLLParameters* par, char* args)
{
	CText channelName, freqName;
	short nrArgs;
	Variable freqVar;
	short freqType = -1;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "frequency", "c", "t", &freqName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("setrxfreq(%s)", freqName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		double frequency;
		try
		{
			freqType = EvaluateArg(par->itfc, freqName.Str(), &freqVar);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		if (freqType == FLOAT64 || freqType == FLOAT32)
		{
			if (freqType == FLOAT64)
				frequency = freqVar.GetDouble();
			else
				frequency = (double)freqVar.GetReal();

			uint32 hiWord, loWord;
			ConvertFrequency(frequency, hiWord, loWord);

			std::vector<uint32>* ps = &(psInfo.ps);
			ps->push_back(20);          ps->push_back(0x4190);   ps->push_back(hiWord);
			ps->push_back(180);         ps->push_back(0x4190);   ps->push_back(loWord);
			psInfo.lineCnt += 2;
		}
		else if (freqType == MATRIX2D)
		{
			float** freqTable = freqVar.GetMatrix2D();
			std::vector<uint32>* ps = &(psInfo.ps);
			ps->push_back(15);               ps->push_back(0x00006084); ps->push_back(nint(freqTable[0][0]) - 1);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(1);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(1);
			ps->push_back(20);               ps->push_back(0x00006084); ps->push_back(0x01000000);
			ps->push_back(20);               ps->push_back(0x01006085); ps->push_back(1);
			ps->push_back(20);               ps->push_back(0x01006085); ps->push_back(1);
			ps->push_back(20);               ps->push_back(0x00004190); ps->push_back(0x01000000);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(1);
			ps->push_back(40);               ps->push_back(0x00004190); ps->push_back(0x01000000);
			psInfo.lineCnt += 9;
		}
		else
		{
			ErrorMessage("invalid data type for freqeucny in setrxfreq");
			return(ERR);
		}
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "frequency", "e", "q", &freqName)) < 0)
			return(nrArgs);

		if (freqName[0] == 'f')
		{
			InsertUniqueStringIntoList(freqName.Str(), &parList, szList);

			uint32 adrs = psInfo.lineCnt * 6;
			long pos;
			if (IsSequenceVariable(freqName.Str(), pos))
			{
				AddToVariableUpdateTable(pos + 0x1000, adrs + 4, 0);
				AddToVariableUpdateTable(pos, adrs + 10, 0);
			}
			if (IsSequenceParameter(freqName.Str(), pos))
			{
				AddToFixedUpdateTable(0, pos + 0x1000, adrs + 4, 0);
				AddToFixedUpdateTable(0, pos, adrs + 10, 0);
			}
			psInfo.lineCnt += 2;
		}
		else if (freqName[0] == 't')
		{
			InsertUniqueStringIntoList(freqName.Str(), &parList, szList);

			uint32 adrs = psInfo.lineCnt * 6;
			long pos;

			if (IsSequenceVariable(freqName.Str(), pos))
			{
				AddToVariableUpdateTable(pos, adrs + 4, 0);
			}
			if (IsSequenceParameter(freqName.Str(), pos))
			{
				AddToFixedUpdateTable(2, pos, adrs + 4, 0);
			}
			psInfo.lineCnt += 9;
		}
		else
		{
			ErrorMessage("invalid data type for freqeucny in setrxfreq");
			return(ERR);
		}


	}
	return(OK);
}
/*******************************************************************************

  Generate code to set the transmit frequency for channel 1 or 2
  Format is:
  settxfreq(channel (1/2), frequency ())

*******************************************************************************/

short SetTxFreq(DLLParameters* par, char* args)
{
	CText channelName, freqName;
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "channel, frequency", "cc", "tt", &channelName, &freqName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("settxfreq(%s,%s)", channelName.Str(), freqName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		uint32 channel;
		double frequency;
		try
		{
			EvaluateArg(par->itfc, freqName.Str(), FLOAT64, frequency);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		uint32 hiWord, loWord;
		ConvertFrequency(frequency, hiWord, loWord);

		std::vector<uint32>* ps = &(psInfo.ps);
		uint32 psOffset = ps->size();

		if (channelName == "1")
		{
			ps->push_back(15);         ps->push_back(0x118E);   ps->push_back(0);
			ps->push_back(15);         ps->push_back(0x118E);   ps->push_back(0);
			ps->push_back(15);         ps->push_back(0x118E);   ps->push_back(hiWord);
			ps->push_back(200 - 45);   ps->push_back(0x118E);   ps->push_back(loWord);
		}
		else if (channelName == "2")
		{
			ps->push_back(15);         ps->push_back(0x128E);   ps->push_back(0);
			ps->push_back(15);         ps->push_back(0x128E);   ps->push_back(0);
			ps->push_back(15);         ps->push_back(0x128E);   ps->push_back(hiWord);
			ps->push_back(200 - 45);   ps->push_back(0x128E);   ps->push_back(loWord);
		}
		else
		{
			ErrorMessage("Invalid channel: %ld", channelName.Str());
			return(ERR);
		}
		psInfo.lineCnt += 4;

	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "channel, frequency", "ee", "qq", &channelName, &freqName)) < 0)
			return(nrArgs);

		if (freqName[0] == 'f')
			InsertUniqueStringIntoList(freqName.Str(), &parList, szList);

		uint32 adrs = psInfo.lineCnt * 6;
		long pos;
		if (IsSequenceVariable(freqName.Str(), pos))
		{
			AddToVariableUpdateTable(pos + 0x1000, adrs + 16, 0);
			AddToVariableUpdateTable(pos         , adrs + 22, 0);
		}
		if (IsSequenceParameter(freqName.Str(), pos))
		{
			AddToFixedUpdateTable(0, pos + 0x1000, adrs + 16, 0);
			AddToFixedUpdateTable(0, pos, adrs + 22, 0);
		}
		psInfo.lineCnt += 4;

	}
	return(OK);
}



/*******************************************************************************

  Generate code to set the transmit frequency for channel 1 and 2
  Format is:
  settxfreqs(frequency1, frequency2)

*******************************************************************************/

short SetTxFreqs(DLLParameters* par, char* args)
{
	CText ch1Freq, ch2Freq;
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "Ch1 freq, Ch2 freq", "cc", "tt", &ch1Freq, &ch2Freq)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("settxfreqs(%s,%s)", ch1Freq.Str(), ch2Freq.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		double freq1, freq2;
		try
		{
			EvaluateArg(par->itfc, ch1Freq.Str(), FLOAT64, freq1);
			EvaluateArg(par->itfc, ch2Freq.Str(), FLOAT64, freq2);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		uint32 hiWord1, loWord1;
		uint32 hiWord2, loWord2;
		ConvertFrequency(freq1, hiWord1, loWord1);
		ConvertFrequency(freq2, hiWord2, loWord2);

		std::vector<uint32>* ps = &(psInfo.ps);
		uint32 psOffset = ps->size();
		ps->push_back(15);         ps->push_back(0x118E);   ps->push_back(0);
		ps->push_back(15);         ps->push_back(0x118E);   ps->push_back(0);
		ps->push_back(15);         ps->push_back(0x118E);   ps->push_back(hiWord1);
		ps->push_back(200 - 45);   ps->push_back(0x118E);   ps->push_back(loWord1);
		ps->push_back(15);         ps->push_back(0x128E);   ps->push_back(0);
		ps->push_back(15);         ps->push_back(0x128E);   ps->push_back(0);
		ps->push_back(15);         ps->push_back(0x128E);   ps->push_back(hiWord2);
		ps->push_back(200 - 45);   ps->push_back(0x128E);   ps->push_back(loWord2);
		psInfo.lineCnt += 8;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "Ch1 freq, Ch2 freq", "ee", "qq", &ch1Freq, &ch2Freq)) < 0)
			return(nrArgs);

		if (ch1Freq[0] == 'f')
			InsertUniqueStringIntoList(ch1Freq.Str(), &parList, szList);
		if (ch2Freq[0] == 'f')
			InsertUniqueStringIntoList(ch2Freq.Str(), &parList, szList);

		uint32 adrs = psInfo.lineCnt * 6;
		long pos;
		if (IsSequenceVariable(ch1Freq.Str(), pos))
		{
			AddToVariableUpdateTable(pos + 0x1000, adrs + 16,  0);
			AddToVariableUpdateTable(pos,          adrs + 22, 0);
		}
		if (IsSequenceVariable(ch2Freq.Str(), pos))
		{
			AddToVariableUpdateTable(pos + 0x1000, adrs + 40, 0);
			AddToVariableUpdateTable(pos,          adrs + 46, 0);
		}
		if (IsSequenceParameter(ch1Freq.Str(), pos))
		{
			AddToFixedUpdateTable(0, pos + 0x1000, adrs + 16, 0);
			AddToFixedUpdateTable(0, pos, adrs + 22, 0);
		}
		if (IsSequenceParameter(ch2Freq.Str(), pos))
		{
			AddToFixedUpdateTable(0, pos + 0x1000, adrs + 40, 0);
			AddToFixedUpdateTable(0, pos, adrs + 46, 0);
		}
		psInfo.lineCnt += 8;
	}
	return(OK);
}


// Search through the variable list looking for varName
// returning the index in the list or -1 on failure
bool IsSequenceVariable(char *varName, long &pos)
{
	char** list = psVariables->GetList();

// Scan through list searching for key, if found return index
	for (long i = 0; i < psVariables->GetDimX(); i++)
	{
		if (!strcmp(list[i], varName))
		{
			pos = i;
			return true;
		}
	}
	pos = -1;
	return false;
}


// Search through the pulse program parameter list looking for varName
// returning the index in the list or -1 on failure
bool IsSequenceParameter(char* varName, long& pos)
{
	if (!gGenerateFullUpdateTable)
		return(false);

	char** list = psInfo.ppList->GetList();
	int sz = psInfo.ppList->GetDimX();

	// Scan through list searching for key, if found return index
	for (long i = 0; i < sz; i++)
	{
		if (!strcmp(list[i], varName))
		{
			pos = i;
			return true;
		}
	}
	pos = -1;
	return false;
}

// Add to the variable parameter update table the address in the pulse sequence table
// to update, the value and also the variable index
void AddToVariableUpdateTable(long varPos, long adrs, long value)
{
	psUpdateValue psUpdate;
	psUpdate.varPos = varPos;
	psUpdate.adrs = adrs;
	psUpdate.value = value;
	psUpdate.specialVar = 0;
	psVariableUpdateTable.push_back(psUpdate);
}

// Add to the fixed parameter update table the address in the pulse sequence table
// to update, the value and also the variable index
void AddToFixedUpdateTable(long special, long varPos, long adrs, long value)
{
	psUpdateValue psUpdate;
	psUpdate.varPos = varPos;
	psUpdate.adrs = adrs;
	psUpdate.value = value;
	psUpdate.specialVar = special;
	psFixedUpdateTable.push_back(psUpdate);
}


// Add to the parameter update table the address in the pulse sequence table
// to update, the value and also the phase number (px)
void AddPhaseToUpdateTable(char *phaseName, long adrs, long value)
{
	psUpdateValue psUpdate;
	int phaseIndex;
	int szVar = psVariables->GetDimX();
	sscanf(phaseName, "p%d", &phaseIndex);
	psUpdate.varPos = phaseIndex; // Phase indices are 1 based
	psUpdate.adrs = adrs;
	psUpdate.value = value;
	psUpdate.specialVar = 1;
	psVariableUpdateTable.push_back(psUpdate);
}

short RFPulse(DLLParameters* par, char* args)
{
	short r = ERR;
	CArg carg;
	short nrArgs = carg.Count(args);

	if (nrArgs == 4 || nrArgs == 5)
		r = SinglePulse(par, args);
	else if (nrArgs == 9)
		r = DualPulse(par, args);
	else
		ErrorMessage("Invalid number of arguments - expecting 4, 5 or 9");

	return(r);
}

/*********************************************************************************************************
	Removing the inverse SINC filter from the DDS output results in an amplitude increase of 3 dB.
	syntax:

		pulseboost(channel (1/2), boost:n (1/0)

		boost = 0 normal amplitude (inverse filter included)
		boost = 1 +3dB amplitude (inverse filter deactivated)

		Duration 2us

		Note if applied directly after a pulse a short delay before this command should be included

**********************************************************************************************************/


short RFPulseBoost(DLLParameters* par, char* args)
{
	CText channelName;
	CText boostName = "1";
	short nrArgs;
	int32 value;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "channel (1/2), boost (1/0)", "cc", "tt", &channelName, &boostName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("pulseboost(%s, %s)", channelName.Str(), boostName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		int32 channel, boost;

		try
		{
			EvaluateArg(par->itfc, channelName.Str(), INTEGER, channel);
			EvaluateArg(par->itfc, boostName.Str(), INTEGER, boost);
		}
		catch (char* errorStr)
		{
			ErrorMessage(errorStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		if (boost == 0)
			value = 0x49;
		else
			value = 0x09;
		

		if (channel == 1)
		{
			ps->push_back(200); ps->push_back(0x00001180); ps->push_back(value);
		}
		else if (channel == 2)
		{
			ps->push_back(200); ps->push_back(0x00001280); ps->push_back(value);
		}
		else
		{
			ErrorMessage("Invalid channel number (1/2)");
			return(ERR);
		}

		psInfo.lineCnt += 1;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "channel (1/2), boost (1/0)", "ee", "qq", &channelName, &boostName)) < 0)
			return(nrArgs);

		if (boostName[0] == 'n')
			InsertUniqueStringIntoList(boostName.Str(), &parList, szList);

		//long varPos;
		//uint32 adrs = psInfo.lineCnt * 6;

		//if (IsSequenceVariable(tableName.Str(), varPos))
		//	AddToVariableUpdateTable(varPos, adrs + 10, 0);
		//if (IsSequenceVariable(incrementName.Str(), varPos))
		//	AddToVariableUpdateTable(varPos, adrs + 28, 0);
		//if (IsSequenceParameter(tableName.Str(), varPos))
		//{
		//	AddToFixedUpdateTable(2, varPos, adrs + 10, 0);
		//}
		psInfo.lineCnt += 1;
	}

	return(OK);
}

/*******************************************************************************
  
  Generate code for a single RF pulse on channel 1 or 2
  Format is:
  pulse(channel (1/2/1nb/2nb), amplitude (dB/t), phase (p/t), duration (d/t), [frequency (f/t)])

  Valid combinations (s:scalar, t:table)

  amp  phase  duration

  s       s      s
  t       s      s
  s       t      s
  t       t      s
  s       s      t

  amp  phase  duration freq

  s       s      s      s
  t       s      s      s
  s       t      s      s
  t       t      s      s
  s       s      s      t
  t       s      s      t
  s       t      s      t
  
*******************************************************************************/

short SinglePulse(DLLParameters* par, char* args)
{
	CText chName, ampName, phName, durName, freqName = " ";
	short nrArgs;
	float amplitude, duration;
	double frequency = 0;
	uint32 digAmp, digPhase, digDur;
	uint32 ddsAdrs, ddsGate, hpaGate;
	uint32  tableAdrs, tableEntries;
	CText channel;

	uint32 startAdrs = psInfo.lineCnt * 6; // Start 16 bit word address for this part of the event table

	// In compile mode extract and evaluate the value of each passed argument and embed these in the event table (psInfo.ps) which is designed to generate an RF pulse
	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 4, "channel, amplitude, phase, duration, [frequency]", "ccccc", "ttttt", &chName, &ampName, &phName, &durName, &freqName)) < 0)
			return(nrArgs);

		// Update the event-table info vector
		if (nrArgs == 5)
		{
			CText txt;
			txt.Format("pulse(%s,%s,%s,%s,%s)", chName.Str(), ampName.Str(), phName.Str(), durName.Str(), freqName.Str());
			ETInfo info = { psInfo.lineCnt, txt.Str() };
			eventTableInfo.push_back(info);
		}
		else
		{
			CText txt;
			txt.Format("pulse(%s,%s,%s,%s)", chName.Str(), ampName.Str(), phName.Str(), durName.Str());
			ETInfo info = { psInfo.lineCnt, txt.Str() };
			eventTableInfo.push_back(info);
		}

		if (ampName[0] == 't')
		{
			try
			{
				EvaluateArg(par->itfc, chName.Str(), UNQUOTED_STRING, channel);
				EvaluateArg(par->itfc, ampName.Str(), MATRIX2D, tableAdrs, tableEntries);
				EvaluateArg(par->itfc, phName.Str(), INTEGER, digPhase);
				EvaluateArg(par->itfc, durName.Str(), FLOAT32, duration);
			}
			catch (char* errorStr)
			{
				ErrorMessage(errorStr);
				return(ERR);
			}
		}
		else if (freqName[0] == 't')
		{
			try
			{
				EvaluateArg(par->itfc, chName.Str(), UNQUOTED_STRING, channel);
				EvaluateArg(par->itfc, ampName.Str(), FLOAT32, amplitude);
				EvaluateArg(par->itfc, phName.Str(), INTEGER, digPhase);
				EvaluateArg(par->itfc, durName.Str(), FLOAT32, duration);
				EvaluateArg(par->itfc, freqName.Str(), MATRIX2D, tableAdrs, tableEntries);
				digAmp = ConvertTxGain(amplitude);
			}
			catch (char* errorStr)
			{
				ErrorMessage(errorStr);
				return(ERR);
			}
		}
		else
		{
			try
			{
				EvaluateArg(par->itfc, chName.Str(), UNQUOTED_STRING, channel);
				EvaluateArg(par->itfc, ampName.Str(), FLOAT32, amplitude);
				EvaluateArg(par->itfc, phName.Str(), INTEGER, digPhase);
				EvaluateArg(par->itfc, durName.Str(), FLOAT32, duration);
				digAmp = ConvertTxGain(amplitude);
			}
			catch (char* errorStr)
			{
				ErrorMessage(errorStr);
				return(ERR);
			}
		}

		digDur = nuint(100 * duration);
		float pgo = psInfo.pgo * 100;
		if (pgo < 250)
		{
			ErrorMessage("pgo must be at least 1.5 us");
			return(ERR);
		}

		// The pulse sequence event table
		std::vector<uint32>* ps = &(psInfo.ps);

		if (channel == "i" || channel == "internal")
		{
			ddsAdrs = 0x0118E;
			hpaGate = 0x04000;
			ddsGate = 0x00008;
		}
		else if (channel == "e" || channel == "external" || channel == "1")
		{
			ddsAdrs = 0x0118E;
			hpaGate = 0x00100;
			ddsGate = 0x00008;
		}
		else if (channel == "1nb")
		{
			ddsAdrs = 0x0118E;
			hpaGate = 0x00000;
			ddsGate = 0x00008;
		}
		else if (channel == "2")
		{
			ddsAdrs = 0x0128E;
			hpaGate = 0x02000;
			ddsGate = 0x00002;
		}
		else if (channel == "2nb")
		{
			ddsAdrs = 0x0128E;
			hpaGate = 0x00000;
			ddsGate = 0x00002;
		}
		else if (channel == "wi")
		{
			ddsAdrs = 0x0128E;
			hpaGate = 0x05000;
			ddsGate = 0x00008;
		}
		else if (channel == "we")
		{
			ddsAdrs = 0x0128E;
			hpaGate = 0x01100;
			ddsGate = 0x00008;
		}
		else
		{
			ErrorMessage("Invalid channel '%s'", channel.Str());
			return(ERR);
		}


		if (nrArgs == 5) // Frequency included in argument list
		{
			uint32 txFreq1, txFreq2;

			if (freqName[0] != 't')
			{
				EvaluateArg(par->itfc, freqName.Str(), FLOAT64, frequency);
				ConvertFrequency(frequency, txFreq1, txFreq2);
			}

			if (ampName[0] == 't') // Amplitude is defined in a table
			{
				ps->push_back(15);               ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate);
				ps->push_back(10);               ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1);   // dspWrite to set start table address
				ps->push_back(10);               ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
				ps->push_back(15);               ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
				ps->push_back(10);               ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
				ps->push_back(10);               ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
				ps->push_back(15);               ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write amplitude to DDS
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(digPhase);        // Write phase to DDS
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(txFreq1);
				ps->push_back(nint(pgo - 60));   ps->push_back(ddsAdrs);      ps->push_back(txFreq2);
				ps->push_back(digDur);           ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
				ps->push_back(15);               ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL);
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(0);
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(0);
				ps->push_back(100 - 45);         ps->push_back(0x09000000);   ps->push_back(0);
				psInfo.lineCnt += 16;
			}
			else if (freqName[0] == 't') // Frequency is defined in a table
			{
				ps->push_back(15);               ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate);
				ps->push_back(10);               ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1);   // dspWrite to set start table address
				ps->push_back(10);               ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Dummy read for FIFO
				ps->push_back(15);               ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
				ps->push_back(10);               ps->push_back(0x00006084);   ps->push_back(0x01000000);      // dspWrite to set start table address
				ps->push_back(10);               ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read for FIFO
				ps->push_back(15);               ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Read amplitude from table
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(digAmp);          // Write amplitude to DDS
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(digPhase);        // Write phase to DDS
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write freq word 1 to DDS
				ps->push_back(15);               ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read table offset
				ps->push_back(nint(pgo - 145));  ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write freq word 2 to DDS
				ps->push_back(digDur);           ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
				ps->push_back(15);               ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL);
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(0);
				ps->push_back(15);               ps->push_back(ddsAdrs);      ps->push_back(0);
				ps->push_back(100 - 45);         ps->push_back(0x09000000);   ps->push_back(0);
				psInfo.lineCnt += 17;
			}
			else // Amplitude and frequency are scalars
			{
				ps->push_back(15);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate);
				ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(digAmp);
				ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(digPhase);
				ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(txFreq1);
				ps->push_back(nint(pgo - 60));   ps->push_back(ddsAdrs);    ps->push_back(txFreq2);
				ps->push_back(digDur);           ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
				ps->push_back(15);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL);
				ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(0);
				ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(0);
				ps->push_back(100 - 45);         ps->push_back(0x09000000); ps->push_back(0);
				psInfo.lineCnt += 10;
			}
		}
		else // Frequency is not supplied in the argument list so use default
		{
			CText txt;
			txt.Format("pulse(%s,%s,%s,%s)", chName.Str(), ampName.Str(), phName.Str(), durName.Str());
			ETInfo info = { psInfo.lineCnt, txt.Str() };
			eventTableInfo.push_back(info);

			if (ampName[0] == 't') // Amplitude is defined by a table
			{
				ps->push_back(15);             ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL | hpaGate);
				ps->push_back(10);             ps->push_back(0x00006084);   ps->push_back(tableAdrs - 1);   // SRAM streaming address <= current table index location
				ps->push_back(10);             ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read contents of this address (FIFO dummy)
				ps->push_back(15);             ps->push_back(0x01006086);   ps->push_back(0x00000000);      // Read contents of this address (real) and send to DSPWrite tableValue input
				ps->push_back(10);             ps->push_back(0x00006084);   ps->push_back(0x01000000);      // SRAM streaming address <=  Current table index
				ps->push_back(10);             ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Dummy read of table amplitude (FIFO)
				ps->push_back(15);             ps->push_back(0x01006085);   ps->push_back(0x00000000);      // Real read of table amplitude. Send to DSPWrite tableValue input
				ps->push_back(15);             ps->push_back(ddsAdrs);      ps->push_back(0x01000000);      // Write amplitude to DDS using tableValue
				ps->push_back(15);             ps->push_back(ddsAdrs);      ps->push_back(digPhase);        // Write phase to DDS
				ps->push_back(digDur);         ps->push_back(0x00000);      ps->push_back(psInfo.currentTTL | hpaGate | ddsGate); // Start pulse
				ps->push_back(15);             ps->push_back(0x00000);      ps->push_back(psInfo.currentTTL); // Stop pulse
				ps->push_back(15);             ps->push_back(ddsAdrs);      ps->push_back(0);               // Zero amplitude and phase
				ps->push_back(15);             ps->push_back(ddsAdrs);      ps->push_back(0);
				ps->push_back(100 - 45);       ps->push_back(0x09000000);   ps->push_back(0);               // Pad timing
				psInfo.lineCnt += 14;
			}
			else // Amplitude is supplied as a scalar 
			{
				ps->push_back(15);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate);
				ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(digAmp);
				ps->push_back(nint(pgo - 30));   ps->push_back(ddsAdrs);    ps->push_back(digPhase);
				ps->push_back(digDur);           ps->push_back(0x00000);    ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
				ps->push_back(15);               ps->push_back(0x00000);    ps->push_back(psInfo.currentTTL);
				ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(0);
				ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(0);
				ps->push_back(100 - 45);         ps->push_back(0x09000000); ps->push_back(0);
				psInfo.lineCnt += 8;
			}
		}
	}

	// In table mode extract the parameter names as they appear in the pulse program (not in pulse program blocks) and 
	// add these to the update table. Then determine the location and value of these parameters in the event table.

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 4, "channel, amplitude, phase, duration, [frequency]", "eeeee", "qqqqq", &chName, &ampName, &phName, &durName, &freqName)) < 0)
			return(nrArgs);

		if (ampName[0] == 'a' || ampName[0] == 't')
			InsertUniqueStringIntoList(ampName.Str(), &parList, szList);
		if (freqName[0] == 'f' || freqName[0] == 't')
			InsertUniqueStringIntoList(freqName.Str(), &parList, szList);
		if (phName[0] == 'p' || phName[0] == 't')
			InsertUniqueStringIntoList(phName.Str(), &parList, szList);
		if (durName[0] == 'd')
			InsertUniqueStringIntoList(durName.Str(), &parList, szList);

		long varPos;
		uint32 psOffset = 0;

		if (nrArgs == 5)
		{

			if (ampName[0] == 't')
			{
				psOffset = 16;
			}
			else if (freqName[0] == 't')
			{
				psOffset = 17;

			}
			else
			{
				if (IsSequenceVariable(ampName.Str(), varPos))
					AddToVariableUpdateTable(varPos, startAdrs + 10, 0);
				if (IsSequenceVariable(durName.Str(), varPos))
				{
					AddToVariableUpdateTable(varPos, startAdrs + 30, 0);
					AddToVariableUpdateTable(varPos + 0x1000, startAdrs + 31, 0);
				}
				if (IsSequenceVariable(freqName.Str(), varPos))
				{
					AddToVariableUpdateTable(varPos, startAdrs + 28, 0);
					AddToVariableUpdateTable(varPos + 0x1000, startAdrs + 22, 0);
				}
				AddPhaseToUpdateTable(phName.Str(), startAdrs + 16, 0);

				if (IsSequenceParameter(ampName.Str(), varPos))
				{
					AddToFixedUpdateTable(0, varPos, startAdrs + 10, 0);
				}
				if (IsSequenceParameter(durName.Str(), varPos))
				{
					AddToFixedUpdateTable(0, varPos, startAdrs + 30, 0);
					AddToFixedUpdateTable(0, varPos + 0x1000, startAdrs + 31, 0);
				}
				if (IsSequenceParameter(freqName.Str(), varPos))
				{
					AddToFixedUpdateTable(0, varPos, startAdrs + 28, 0);
					AddToFixedUpdateTable(0, varPos + 0x1000, startAdrs + 22, 0);
				}
				psOffset = 10;
			}
		}
		else
		{

			if (ampName[0] == 't')
			{
				//if (IsSequenceVariable(ampName.Str(), varPos))
				//	AddToUpdateTable(varPos, startAdrs + 10, 0);
				//if (IsSequenceVariable(durName.Str(), varPos))
				//{
				//	AddToUpdateTable(varPos,          startAdrs + 18, 0);
				//	AddToUpdateTable(varPos + 0x1000, startAdrs + 19, 0);
				//}
				//AddPhaseToUpdateTable(phName.Str(), astartAdrsdrs + 16, 0);
				psOffset = 14;
			}
			else
			{
				if (IsSequenceVariable(ampName.Str(), varPos))
				{
					AddToVariableUpdateTable(varPos, startAdrs + 10, 0);
				}
				if (IsSequenceVariable(durName.Str(), varPos))
				{
					AddToVariableUpdateTable(varPos, startAdrs + 18, 0);
					AddToVariableUpdateTable(varPos + 0x1000, startAdrs + 19, 0);
				}
				AddPhaseToUpdateTable(phName.Str(), startAdrs + 16, 0);

				if (IsSequenceParameter(ampName.Str(), varPos))
				{
					AddToFixedUpdateTable(0, varPos, startAdrs + 10, 0);
				}
				if (IsSequenceParameter(durName.Str(), varPos))
				{
					AddToFixedUpdateTable(0, varPos, startAdrs + 18, 0);
					AddToFixedUpdateTable(0, varPos + 0x1000, startAdrs + 19, 0);
				}
				psOffset = 8;
			}
		}
		if (psInfo.mode == "tables")
			psInfo.lineCnt += psOffset;
	}

	return(OK);
}


/*******************************************************************************

  Generate code for a dual RF pulse on channel 1 and 2
  Format is:
  pulse(1, amplitude1 (dB), phase1 (), frequency1 (), 
        2, amplitude2 (dB), phase2 (), frequency2 (), duration ())

* *****************************************************************************/


short DualPulse(DLLParameters* par, char* args)
{
	// Get the raw arguments (no evaluation)
	short nrArgs;
	CText chName1, ampName1, phName1, freqName1;
	CText chName2, ampName2, phName2, freqName2;
	CText durName;


	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 9, "channel1, amplitude1, phase1, frequency1, channel2, amplitude2, phase2, frequency2, duration", "ccccccccc", "ttttttttt",
			&chName1, &ampName1, &phName1, &freqName1, &chName2, &ampName2, &phName2, &freqName2, &durName)) < 0)
			return(nrArgs);


		// Update the event-table info vector
		CText txt;
		txt.Format("pulse(%s,%s,%s,%s,%s,%s,%s,%s,%s)",chName1.Str(), ampName1.Str(), phName1.Str(), freqName1.Str(), chName2.Str(), ampName2.Str(), phName2.Str(), freqName2.Str(), durName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);


		// Evaluate the arguments checking for errors
		float amplitude1, amplitude2;
		float phase1, phase2;
		double frequency1, frequency2;
		float duration;
		try
		{
			EvaluateArg(par->itfc, ampName1.Str(), FLOAT32, amplitude1);
			EvaluateArg(par->itfc, ampName2.Str(), FLOAT32, amplitude2);
			EvaluateArg(par->itfc, phName1.Str(), FLOAT32, phase1);
			EvaluateArg(par->itfc, phName2.Str(), FLOAT32, phase2);
			EvaluateArg(par->itfc, freqName1.Str(), FLOAT64, frequency1);
			EvaluateArg(par->itfc, freqName2.Str(), FLOAT64, frequency2);
			EvaluateArg(par->itfc, durName.Str(), FLOAT32, duration);
		}
		catch (char* errorStr)
		{
			ErrorMessage(errorStr);
			return(ERR);
		}

		// Get a pointer to the pulse sequence code
		std::vector<uint32>* ps = &(psInfo.ps);

		// Convert amplitudes from dB to 14 bit
		uint32 digAmp1, digAmp2;
		digAmp1 = ConvertTxGain(amplitude1);
		digAmp2 = ConvertTxGain(amplitude2);

		// Convert frequencies from MHz to 48 bit
		uint32 txFreq11, txFreq12, txFreq21, txFreq22;
		ConvertFrequency(frequency1, txFreq11, txFreq12);
		ConvertFrequency(frequency2, txFreq21, txFreq22);

		// Convert phase (conversion already done in Prospa)
		uint32 digPhase1 = phase1;
		uint32 digPhase2 = phase2;

		// Convert the duration from us to multiples of 10 ns
		uint32 digDur = 100 * duration;

		// Some constants
		uint32 pgo = nint(psInfo.pgo * 100);
		uint32 ddsAdrs1 = 0x118E;
		uint32 hpaGate1 = 0x4000;
		uint32 ddsGate1 = 0x0008;
		uint32 ddsAdrs2 = 0x128E;
		uint32 hpaGate2 = 0x0100;
		uint32 ddsGate2 = 0x0002;

		long psOffset = ps->size();
		ps->push_back(15);             ps->push_back(0x00000000);  ps->push_back(psInfo.currentTTL | hpaGate1 | hpaGate2);
		ps->push_back(15);             ps->push_back(ddsAdrs1);    ps->push_back(digAmp1);
		ps->push_back(15);             ps->push_back(ddsAdrs1);    ps->push_back(digPhase1);
		ps->push_back(15);             ps->push_back(ddsAdrs1);    ps->push_back(txFreq11);
		ps->push_back(200 - 60);       ps->push_back(ddsAdrs1);    ps->push_back(txFreq12);
		ps->push_back(15);             ps->push_back(ddsAdrs2);    ps->push_back(digAmp2);
		ps->push_back(15);             ps->push_back(ddsAdrs2);    ps->push_back(digPhase2);
		ps->push_back(15);             ps->push_back(ddsAdrs2);    ps->push_back(txFreq21);
		ps->push_back(pgo - 245);      ps->push_back(ddsAdrs2);    ps->push_back(txFreq22);
		ps->push_back(digDur);         ps->push_back(0x00000000);  ps->push_back(psInfo.currentTTL | hpaGate1 | hpaGate2 | ddsGate1 | ddsGate2);
		ps->push_back(15);             ps->push_back(0x00000000);  ps->push_back(psInfo.currentTTL);
		ps->push_back(15);             ps->push_back(ddsAdrs1);    ps->push_back(0);
		ps->push_back(80);             ps->push_back(ddsAdrs1);    ps->push_back(0);
		ps->push_back(15);             ps->push_back(ddsAdrs2);    ps->push_back(0);
		ps->push_back(200 - 125);      ps->push_back(ddsAdrs2);    ps->push_back(0);
		psInfo.lineCnt += 15;

	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 9, "channel1, amplitude1, phase1, frequency1, channel2, amplitude2, phase2, frequency2, duration", "eeeeeeeee", "qqqqqqqqq",
			&chName1, &ampName1, &phName1, &freqName1, &chName2, &ampName2, &phName2, &freqName2, &durName)) < 0)
			return(nrArgs);

		// Record variable names if they have the correct prefix
		if (ampName1[0] == 'a' || ampName1[0] == 't')
			InsertUniqueStringIntoList(ampName1.Str(), &parList, szList);
		if (phName1[0] == 'p' || phName1[0] == 't')
			InsertUniqueStringIntoList(phName1.Str(), &parList, szList);
		if (freqName1[0] == 'f')
			InsertUniqueStringIntoList(freqName1.Str(), &parList, szList);
		if (ampName2[0] == 'a' || ampName2[0] == 't')
			InsertUniqueStringIntoList(ampName2.Str(), &parList, szList);
		if (phName2[0] == 'p' || phName2[0] == 't')
			InsertUniqueStringIntoList(phName2.Str(), &parList, szList);
		if (freqName2[0] == 'f')
			InsertUniqueStringIntoList(freqName2.Str(), &parList, szList);
		if (durName[0] == 'd')
			InsertUniqueStringIntoList(durName.Str(), &parList, szList);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;
		if (IsSequenceVariable(ampName1.Str(), varPos))
			AddToVariableUpdateTable(varPos, adrs + 10, 0);
		if (IsSequenceParameter(ampName1.Str(), varPos))
			AddToFixedUpdateTable(0,varPos, adrs + 10, 0);

		AddPhaseToUpdateTable(phName1.Str(), adrs + 16, 0);

		if (IsSequenceVariable(freqName1.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos + 0x1000, adrs + 22, 0);
			AddToVariableUpdateTable(varPos, adrs + 28, 0);
		}
		if (IsSequenceParameter(freqName1.Str(), varPos))
		{
			AddToFixedUpdateTable(0, varPos + 0x1000, adrs + 22, 0);
			AddToFixedUpdateTable(0, varPos, adrs + 28, 0);
		}

		if (IsSequenceVariable(ampName2.Str(), varPos))
			AddToVariableUpdateTable(varPos, adrs + 34, 0);
		if (IsSequenceParameter(ampName2.Str(), varPos))
			AddToFixedUpdateTable(0, varPos, adrs + 34, 0);

		AddPhaseToUpdateTable(phName2.Str(), adrs + 40, 0);

		if (IsSequenceVariable(freqName2.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos + 0x1000, adrs + 46, 0);
			AddToVariableUpdateTable(varPos, adrs + 52, 0);
		}
		if (IsSequenceParameter(freqName2.Str(), varPos))
		{
			AddToFixedUpdateTable(0, varPos + 0x1000, adrs + 46, 0);
			AddToFixedUpdateTable(0, varPos, adrs + 52, 0);
		}

		if (IsSequenceVariable(durName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 54, 0);
			AddToVariableUpdateTable(varPos+0x1000, adrs + 55, 0);
		}
		if (IsSequenceParameter(durName.Str(), varPos))
		{
			AddToFixedUpdateTable(0, varPos, adrs + 54, 0);
			AddToFixedUpdateTable(0, varPos + 0x1000, adrs + 55, 0);
		}

		psInfo.lineCnt += 15;
	}
	return(OK);

}

/*******************************************************************************
  Generates a ramp on the gradient channel (same as shimramp16(16, ...)

	 gradramp(start, end, steps, duration)

	 start    : initial amplitude (signed 16 bit number)
	 end      : final amplitude (signed 16 bit number)
	 steps    : number of steps is (2-511)
	 duration : length of each step (us) (2-2047)

	 Total ramp duration = duration * steps
*******************************************************************************/

short GradientRamp(DLLParameters* par, char* args)
{
	CText channelName = " ", startName = " ", endName = " ", stepsName = " ", durationName = " ";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 5, "channel, start, end, steps, duration", "ccccc", "ttttt", &channelName, &startName, &endName, &stepsName, &durationName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("gradramp(%s, %s, %s, %s, %s)", channelName.Str(), startName.Str(), endName.Str(), stepsName.Str(), durationName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		int32 startValue;
		int32 endValue;
		uint32 nrSteps;
		uint32 duration;
		uint32 channel;
		uint32 tableAdrs;
		uint32 tableEntries;

		// Convert named channel to number
		if (channelName == "x")
			channelName = "3";
		else if(channelName == "y")
			channelName = "2";
		else if(channelName == "z")
			channelName = "1";
		else if(channelName == "o")
			channelName = "0";

		// Extract the values by evaluating the arguments
		if (startName[0] != 't' && endName[0] != 't') // Scalar values
		{
			try
			{
				EvaluateArg(par->itfc, channelName.Str(), INTEGER, channel);
				EvaluateArg(par->itfc, startName.Str(), INTEGER, startValue);
				EvaluateArg(par->itfc, endName.Str(), INTEGER, endValue);
				EvaluateArg(par->itfc, stepsName.Str(), INTEGER, nrSteps);
				EvaluateArg(par->itfc, durationName.Str(), INTEGER, duration);
			}
			catch (char* errorStr)
			{
				ErrorMessage(errorStr);
				return(ERR);
			}
		}

		if (startName[0] == 't' && endName[0] != 't') // Table/Scalar values
		{
			try
			{
				EvaluateArg(par->itfc, channelName.Str(), INTEGER, channel);
				EvaluateArg(par->itfc, startName.Str(), MATRIX2D, tableAdrs, tableEntries);
				EvaluateArg(par->itfc, endName.Str(), INTEGER, endValue);
				EvaluateArg(par->itfc, stepsName.Str(), INTEGER, nrSteps);
				EvaluateArg(par->itfc, durationName.Str(), INTEGER, duration);
			}
			catch (char* errorStr)
			{
				ErrorMessage(errorStr);
				return(ERR);
			}
		}

		if (startName[0] != 't' && endName[0] == 't') // Scalar/Table values
		{
			try
			{
				EvaluateArg(par->itfc, channelName.Str(), INTEGER, channel);
				EvaluateArg(par->itfc, startName.Str(), INTEGER, startValue);
				EvaluateArg(par->itfc, endName.Str(), MATRIX2D, tableAdrs, tableEntries);
				EvaluateArg(par->itfc, stepsName.Str(), INTEGER, nrSteps);
				EvaluateArg(par->itfc, durationName.Str(), INTEGER, duration);
			}
			catch (char* errorStr)
			{
				ErrorMessage(errorStr);
				return(ERR);
			}
		}

		if(channel < 0 || channel > 3)
		{
			ErrorMessage("Invalid channel name/number");
			return(ERR);
		}

		if (startName[0] != 't') // Check limits for scalar
		{
			if (startValue > 32767 || startValue < -32768)
			{
				ErrorMessage("start value for a shim ramp must be a valid 16 bit signed number (-32768->+32767)");
				return(ERR);
			}
		}

		if (endName[0] != 't')// Check limits for scalar
		{
			if (endValue > 32767 || endValue < -32768)
			{
				ErrorMessage("end value for a shim ramp must be a valid 16 bit signed number (-32768->+32767)");
				return(ERR);
			}
		}

		if (nrSteps < 2 || nrSteps >= 512)
		{
			ErrorMessage("Number of steps must be between 2 and 511");
			return(ERR);
		}

		if (duration < 2 || duration >= 2048)
		{
			ErrorMessage("Step duration must be between 2 and 2046 us");
			return(ERR);
		}

		if (duration % 2 != 0)
		{
			ErrorMessage("Step duration must be a multiple of 2 us");
			return(ERR);
		}

		uint32  rampTime = nrSteps * 100 * duration;

		duration = duration / 2; // Multiple of 2 us

		if (startName[0] != 't' && endName[0] != 't') // Scalar values
		{
			uint32 cmd = 0x0D000000 | ((nrSteps & 0xFF) << 16) | (endValue & 0xFFFF);
			uint32 value = ((duration & 0x7FF) << 21) | ((nrSteps & 0x300) << 11) | ((channel << 16) & 0x030000) | (startValue & 0xFFFF);
			AddEvent(rampTime, cmd, value);
		}
		else if (startName[0] == 't' && endName[0] != 't') // Table/Scalar values
		{
			uint32 cmd = 0x0D000000 | ((nrSteps & 0xFF) << 16) | (endValue & 0xFFFF);
			uint32 value = ((duration & 0x7FF) << 21) | ((nrSteps & 0x300) << 11) | ((channel << 16) & 0x030000) | (0x0000 & 0xFFFF);
			uint32 valueAdrs = psInfo.lineCnt * 6 + 52 + 0x3000;  // Points to lower word in cmd location in event table in BRAM(0x3000)

			AddEvent(10, 0x00006084, tableAdrs - 1);  // dspWrite to set start table address
			AddEvent(15, 0x01006086, 0x00000000);      // Dummy read for FIFO
			AddEvent(15, 0x01006086, 0x00000000);      // Read table offset
			AddEvent(10, 0x00006084, 0x01000000);      // dspWrite to set start table address
			AddEvent(15, 0x01006085, 0x00000000);      // Dummy read for FIFO
			AddEvent(15, 0x01006085, 0x00000000);      // Read end gradient amplitude from table
			AddEvent(10, valueAdrs, 0x04000000);       // Write amplitude to following line in event table
			AddEvent(10, 0x09000000, 0x00000000);      // Not sure why this is needed, but it is!
			AddEvent(rampTime, cmd, value);            // Generate the ramp with end table value
		}
		else if (startName[0] != 't' && endName[0] == 't') // Scalar/Table values
		{
			uint32 cmd = 0x0D000000 | ((nrSteps & 0xFF) << 16) | (0x0000 & 0xFFFF);
			uint32 value = ((duration & 0x7FF) << 21) | ((nrSteps & 0x300) << 11) | ((channel << 16) & 0x030000) | (startValue & 0xFFFF);
			uint32 valueAdrs = psInfo.lineCnt * 6 + 50 + 0x3000;  // Points to lower word in cmd location in event table
			AddEvent(10, 0x00006084, tableAdrs - 1); // dspWrite to set start table address
			AddEvent(15, 0x01006086, 0x00000000);    // Dummy read for FIFO
			AddEvent(15, 0x01006086, 0x00000000);    // Read table offset
			AddEvent(10, 0x00006084, 0x01000000);    // dspWrite to set start table address
			AddEvent(15, 0x01006085, 0x00000000);    // Dummy read for FIFO
			AddEvent(15, 0x01006085, 0x00000000);    // Read end gradient amplitude from table
			AddEvent(10, valueAdrs,  0x04000000);    // Write amplitude to following line in event table
			AddEvent(10, 0x09000000, 0x00000000);    // Not sure why this is needed, but it is!
			AddEvent(rampTime, cmd, value);          // Generate the ramp with end table value
		}
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 4, "start, end, steps, duration", "eeee", "qqqq", &startName, &endName, &stepsName, &durationName)) < 0)
			return(nrArgs);

		if (startName[0] == 'n')
			InsertUniqueStringIntoList(startName.Str(), &parList, szList);
		if (endName[0] == 'n')
			InsertUniqueStringIntoList(endName.Str(), &parList, szList);
		if (stepsName[0] == 'n')
			InsertUniqueStringIntoList(stepsName.Str(), &parList, szList);
		if (durationName[0] == 'd')
			InsertUniqueStringIntoList(durationName.Str(), &parList, szList);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;

		// Save fixed update table information
		if (IsSequenceParameter(startName.Str(), varPos))
		{
			AddToFixedUpdateTable(0, varPos, adrs + 4, 0);
		}
		if (IsSequenceParameter(endName.Str(), varPos))
		{
			AddToFixedUpdateTable(0, varPos, adrs + 2, 0);
		}

		// Save variable update table information
		if (IsSequenceVariable(startName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 4, 0);
		}

		if (IsSequenceVariable(endName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 2, 0);
		}

		//  if (IsSequenceParameter(stepsName.Str(), varPos))
		  //{
		  //	AddToFixedUpdateTable(4, varPos + 0x1000, adrs + 3, 0);
		  //	AddToFixedUpdateTable(5, varPos + 0x1000, adrs + 5, 0);
		  //	AddToFixedUpdateTable(6, varPos, adrs + 0, 0);
		  //	AddToFixedUpdateTable(6, varPos + 0x1000, adrs + 1, 0);
		  //}
		  //if (IsSequenceParameter(durationName.Str(), varPos))
		  //{
		  //	AddToFixedUpdateTable(8, varPos, adrs + 0, 0);
		  //	AddToFixedUpdateTable(8, varPos + 0x1000, adrs + 1, 0);
		  //}

		psInfo.lineCnt += 1;

	}
	return(OK);
}



/**********************************************************************
	  Skip to endskip with name skipto if variable = 0

	  skiponzero(skipto, variable)

	  This allows some code to be skipped based on a UI variable
**********************************************************************/

short SkipOnZero(DLLParameters* par, char* args)
{
	CText skipName = " ", testValueName = " ";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "skip name, number to test", "cc", "tt", &skipName, &testValueName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("skiponzero(%s,%s)", skipName.Str(), testValueName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		uint32 skip;
		try
		{
			EvaluateArg(par->itfc, testValueName.Str(), INTEGER, skip);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}
		std::vector<uint32>* ps = &(psInfo.ps);
		uint32 psOffset = ps->size();

		ps->push_back(15);  ps->push_back(0x0A000000); ps->push_back(0x00000000);

		if (skip > 1)
			skip = 1;

		// Record the start line number and skipname - see endpp for implementation
		SkipInfo skipdata = { skipName, psInfo.lineCnt, 0, skip};
		skipInfo.push_back(skipdata);

		psInfo.lineCnt += 1;
	}
	
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "skip name, number to test", "ee", "qq", &skipName, &testValueName)) < 0)
			return(nrArgs);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;

		if (IsSequenceVariable(testValueName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 4, 0);
		}

		if (IsSequenceParameter(testValueName.Str(), varPos))
		{
			AddToFixedUpdateTable(3, varPos, adrs + 4, 0);
		}
		psInfo.lineCnt += 1;

	}

	return(OK);
}


/**********************************************************************
	  Destination of the skiponzero command

	  endskip(skipto)

**********************************************************************/

short SkipEnd(DLLParameters* par, char* args)
{
	short nrArgs;
	CText skipName;
	FILE* fp;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "loop name", "c", "t", &skipName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("skipend(%s)", skipName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		std::vector<uint32>* ps = &(psInfo.ps);
		uint32 psOffset = ps->size();


		ps->push_back(15);  ps->push_back(0x09000000); ps->push_back(0x00000000);

		// Record the end line number and skipname - see endpp for implementation
		int cnt;
		bool found = false;
		for (cnt = 0; cnt < skipInfo.size(); cnt++)
		{
			if (skipInfo[cnt].skipName == skipName)
			{
				skipInfo[cnt].skipEndLine = psInfo.lineCnt;
				found = true;
			}
		}
		if (!found)
		{
			ErrorMessage("missing skiponzero command for endskip '%s'", skipName.Str());
			return(ERR);
		}
		psInfo.lineCnt += 1;

	}
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "skip name", "e", "q", &skipName)) < 0)
			return(nrArgs);

		psInfo.lineCnt += 1;
	}
	return(OK);
}

short SetRxGain(DLLParameters* par, char* args)
{
	ErrorMessage("Not implemented yet on FX3");
	return(ERR);
}


short SelectRxAmplifier(DLLParameters* par, char* args)
{
	ErrorMessage("Not implemented yet on FX3");
	return(ERR);
}


/*******************************************************************************
  Set the gradient output on 1 channel to a specific level

	 gradon(channel, amplitude)

	 channel: channel name (x/y/z/o or 3/2/1/0)
	 amplitude  : amplitude (signed 16 bit number or table)

	 Total duration = 2us (3us for table values)
*******************************************************************************/

short GradientOn(DLLParameters* par, char* args)
{
	CText channelName;
	CText amplitudeName;
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "channel, amplitude", "cc", "tt", &channelName , &amplitudeName)) < 0)
			return(nrArgs);

		// Generate event table entry when report command used
		CText txt;
		txt.Format("gradon(%s,%s)",channelName.Str(),amplitudeName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		// Extract arguments
		uint32 channel; 
		int32 amplitude;
		uint32  tableAdrs, tableEntries;

		// Convert named channel to number
		if (channelName == "x")
			channelName = "3";
		else if(channelName == "y")
			channelName = "2";
		else if(channelName == "z")
			channelName = "1";
		else if(channelName == "o")
			channelName = "0";

		if (amplitudeName[0] != 't') // Single values
		{
			try
			{
				EvaluateArg(par->itfc, channelName.Str(), INTEGER, channel);
				EvaluateArg(par->itfc, amplitudeName.Str(), INTEGER, amplitude);
			}
			catch (char* errStr)
			{
				ErrorMessage(errStr);
				return(ERR);
			}
		}
		else // Gradient table
		{
			try
			{
				EvaluateArg(par->itfc, channelName.Str(), INTEGER, channel);
				EvaluateArg(par->itfc, amplitudeName.Str(), MATRIX2D, tableAdrs, tableEntries);
			}
			catch (char* errorStr)
			{
				ErrorMessage(errorStr);
				return(ERR);
			}
		}

		if(channel < 0 || channel > 3)
		{
			ErrorMessage("Invalid channel name/number");
			return(ERR);
		}

		// Generate the event information for a gradient table
		if (amplitudeName[0] == 't')
		{
			uint32 cmd = 0x07000000 | channel;                   // Combine command number with channel
			uint32 value = 0x0000;                               // Value will be updated from table
			uint32 valueAdrs = psInfo.lineCnt * 6 + 52 + 0x3000; // Points to lower word in value location in event table
			AddEvent(10,  0x00006084, tableAdrs - 1);   // dspWrite to set start table address
			AddEvent(15,  0x01006086, 0x00000000);      // Dummy read for FIFO
			AddEvent(15,  0x01006086, 0x00000000);      // Read table offset
			AddEvent(10,  0x00006084, 0x01000000);      // dspWrite to set start table address
			AddEvent(15,  0x01006085, 0x00000000);      // Dummy read for FIFO
			AddEvent(15,  0x01006085, 0x00000000);      // Read end gradient amplitude from table
			AddEvent(10,  valueAdrs,  0x01000000);      // Write amplitude to following line in event table
			AddEvent(10,  0x09000000, 0x00000000);      // Not sure why this delay is needed, but it is!
			AddEvent(200, cmd,        value);
		}
		// Generate the event information for a fixed value
		else
		{
			uint32 cmd = 0x07000000 | channel;
			uint32 value = amplitude & 0xFFFF;
			AddEvent(200, cmd, value);
			gCurGradLevel = amplitude;
		}
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "channel, amplitude", "ee", "qq", &channelName, &amplitudeName)) < 0)
			return(nrArgs);

		if (amplitudeName[0] == 'n')
			InsertUniqueStringIntoList(amplitudeName.Str(), &parList, szList);

		uint32 adrs = psInfo.lineCnt * 6;
		long varPos;
		if (IsSequenceVariable(amplitudeName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 4, 0); // Just modify the 16 bits which contains the amplitude
		}
		if (IsSequenceParameter(amplitudeName.Str(), varPos)) // To generate complete update table
		{
			AddToFixedUpdateTable(0, varPos, adrs + 4, 0);
		}
		if (amplitudeName[0] == 't')
			psInfo.lineCnt += 9;
		else
			psInfo.lineCnt += 1;
	}

	return(OK);
}


/*******************************************************************************
  Set the gradient output to zero

	 gradoff(channel)

	 Total  duration = 2us
*******************************************************************************/

short GradientOff(DLLParameters* par, char* args)
{
	CText channelName;
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "channel", "c", "t", &channelName)) < 0)
			return(nrArgs);

		// Generate event table entry when report command used
		CText txt;
		txt.Format("gradoff(%s)", channelName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		// Convert named channel to number
		if (channelName == "x")
			channelName = "3";
		else if(channelName == "y")
			channelName = "2";
		else if(channelName == "z")
			channelName = "1";
		else if(channelName == "o")
			channelName = "0";

		// Extract channel name
		uint32 channel; 
		try
		{
			EvaluateArg(par->itfc, channelName.Str(), INTEGER, channel);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		uint32 cmd = 0x07000000 | channel;
		uint32 value =0;
		AddEvent(200, cmd, value);
		gCurGradLevel = 0;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		uint32 adrs = psInfo.lineCnt * 6;
		psInfo.lineCnt += 1;
	}

	return(OK);
}



short ShapedRFPulse(DLLParameters* par, char* args)
{
	ErrorMessage("Not implemented on FX3 - DSP sequences which use this command must be updated");
	return(ERR);
}



/**********************************************************************
	  Switch on a TTl level

	  ttlon(byte,[shift?])

**********************************************************************/

short TTLOn(DLLParameters* par, char* args)
{
	CText byteName, shift = "true";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "byte name, [shift?]", "cc", "tt", &byteName, &shift)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("ttlon(%s)", byteName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		int32 byteValue;
		try
		{
			EvaluateArg(par->itfc, byteName.Str(), INTEGER, byteValue);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		// The default action is to left shift the data by 8.  Set shiftName = "false" to prevent this.
		if(shift == "true" || shift == "yes")
			byteValue = byteValue << 8;

		std::vector<uint32>* ps = &(psInfo.ps);	
		psInfo.currentTTL = psInfo.currentTTL | byteValue;
		ps->push_back(100);   ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL);

		psInfo.lineCnt += 1;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "byte name [shift?]", "ee", "qq", &byteName, &shift)) < 0)
			return(nrArgs);

		if (byteName[0] == 'b')
			InsertUniqueStringIntoList(byteName.Str(), &parList, szList);

		uint32 adrs = psInfo.lineCnt * 6;
		long varPos;
		if (IsSequenceVariable(byteName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 4, 0); 
		}
		if (IsSequenceParameter(byteName.Str(), varPos))
		{
			AddToFixedUpdateTable(0, varPos, adrs + 4, 0);
		}
		psInfo.lineCnt += 1;
	}

	return(OK);
}

/**********************************************************************
	  Switch off a TTl level

	  ttloff(byte)

**********************************************************************/

short TTLOff(DLLParameters* par, char* args)
{
	CText byteName, shift = "true";
	short nrArgs;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "byte name, [shift?]", "cc", "tt", &byteName, &shift)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("ttloff(%s)", byteName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		int32 byteValue;
		try
		{
			EvaluateArg(par->itfc, byteName.Str(), INTEGER, byteValue);
		}
		catch (char* errStr)
		{
			ErrorMessage(errStr);
			return(ERR);
		}

		// The default action is to left shift the data by 8.  Set shiftName = "false" to prevent this.
		if (shift == "true" || shift == "yes")
			byteValue = byteValue << 8;

		std::vector<uint32>* ps = &(psInfo.ps);
		psInfo.currentTTL = psInfo.currentTTL & ~byteValue;
		ps->push_back(15);   ps->push_back(0x00000000);   ps->push_back(psInfo.currentTTL);

		psInfo.lineCnt += 1;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "byte name [shift?]", "ee", "qq", &byteName, &shift)) < 0)
			return(nrArgs);

		if (byteName[0] == 'b')
			InsertUniqueStringIntoList(byteName.Str(), &parList, szList);

		uint32 adrs = psInfo.lineCnt * 6;
		long varPos;
		if (IsSequenceVariable(byteName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 4, 0);
		}
		if (IsSequenceParameter(byteName.Str(), varPos))
		{
			AddToFixedUpdateTable(0, varPos, adrs + 4, 0);
		}
		psInfo.lineCnt += 1;
	}

	return(OK);
}

/*******************************************************************************
  Switches on the specified rf channel
 
    txon(channel, amplitude, phaseValue, [frequency])
 
       channel    : 1/2/"1nb"/"2nb"/"w1"/"w2" (nb = no blanking pulse, w = wobble mode)
       amplitude  : amplitude in dB or a table ref (14 bit words)
       phaseValue : phase offset (16 bits == 0-360 degrees)
       frequency  : an optional frequency in MHz or as a table ref (2 x 16 bit words)
 
       Duration = pgo (note that txon/txoff pulses will be 200 ns
                       longer than expected based on intervening delays)
 
       Note: 1. When stepping through a frequency table you must increment by 2
                as the frequency array has two, 16 bit values.
             2. The amplitude or the frequency can be a table but not both.

* *****************************************************************************/

short TxOn(DLLParameters* par, char* args)
{
	CText chName = " ", ampName = " ", phName = " ", freqName = " ";
	short nrArgs;
	float amplitude;
	double frequency = 0;
	uint32 digAmp, digPhase;
	uint32 channel, ddsAdrs, ddsGate, hpaGate;
	Variable ampVar;
	Variable freqVar;
	short ampType = -1, freqType = -1;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 3, "channel, amplitude, phase, [frequency]", "cccc", "tttt", &chName, &ampName, &phName, &freqName)) < 0)
			return(nrArgs);

		CText txt;
		if(nrArgs == 3)
			txt.Format("txon(%s, %s, %s)", chName.Str(), ampName.Str(), phName.Str());
		else
			txt.Format("txon(%s, %s, %s, %s)", chName.Str(), ampName.Str(), phName.Str(), freqName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		// Extract the values by evaluating the arguments
		try
		{
			short type;
			CText channel;
			EvaluateArg(par->itfc, chName.Str(), UNQUOTED_STRING, channel);
			chName = channel;
			ampType = EvaluateArg(par->itfc, ampName.Str(), &ampVar);
			EvaluateArg(par->itfc, phName.Str(), INTEGER, digPhase);
			if (nrArgs == 4)
				freqType = EvaluateArg(par->itfc, freqName.Str(), &freqVar);
		}
		catch (char* errorStr)
		{
			ErrorMessage(errorStr);
			return(ERR);
		}


		// Select the gate addresses based on the channel
		if (chName == "1")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x4000;
			ddsGate = 0x0008;
		}
		else if (chName == "1nb")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x0000;
			ddsGate = 0x0008;
		}
		else if (chName == "wi")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x5000;
			ddsGate = 0x0008;
		}
		else if (chName == "we")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x1100;
			ddsGate = 0x0008;
		}
		else if (chName == "2")
		{
			ddsAdrs = 0x128E;
			hpaGate = 0x2000;
			ddsGate = 0x0002;
		}
		else if (chName == "2nb")
		{
			ddsAdrs = 0x128E;
			hpaGate = 0x0000;
			ddsGate = 0x0002;
		}
		//else if (chName == "w2")
		//{
		//	ddsAdrs = 0x128E;
		//	hpaGate = 0x0500;
		//	ddsGate = 0x0002;
		//}
		else
		{
			ErrorMessage("Invalid channel %s for txon", chName.Str());
			return(ERR);
		}

		// Pulse gate overhead and cumulative pulse sequence array
		uint32 pgo = nint(psInfo.pgo * 100);
		std::vector<uint32>* ps = &(psInfo.ps);

		// Amplitude and frequency provided as scalars
		if (ampType == FLOAT32 && (freqType == FLOAT64 || freqType == FLOAT32))
		{
			digAmp = ConvertTxGain(ampVar.GetReal());

			if(freqType == FLOAT64)
				frequency = freqVar.GetDouble();
			else
				frequency = (double)freqVar.GetReal();

			uint32 txFreq1, txFreq2;
			ConvertFrequency(frequency, txFreq1, txFreq2);

			ps->push_back(15);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate);
			ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(digAmp);
			ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(digPhase);
			ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(txFreq1);
			ps->push_back(pgo - 60);         ps->push_back(ddsAdrs);    ps->push_back(txFreq2);
			ps->push_back(20);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
			psInfo.lineCnt += 6;
			psInfo.currentTTL = psInfo.currentTTL | hpaGate | ddsGate;
		}
		else if (ampType == FLOAT32 && freqType == -1) // Amplitude provided as scalar
		{
			digAmp = ConvertTxGain(ampVar.GetReal());

			ps->push_back(15);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate);
			ps->push_back(15);               ps->push_back(ddsAdrs);    ps->push_back(digAmp);
			ps->push_back(pgo - 30);         ps->push_back(ddsAdrs);    ps->push_back(digPhase);
			ps->push_back(20);               ps->push_back(0x00000);    ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
			psInfo.lineCnt += 4;
			psInfo.currentTTL = psInfo.currentTTL | hpaGate | ddsGate;
		}
		else if (ampType == MATRIX2D && freqType == -1) // Amplitude provided as a table but no frequency
		{
			float** ampTable = ampVar.GetMatrix2D();
			ps->push_back(15);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate);
			ps->push_back(15);               ps->push_back(0x00006084); ps->push_back(nint(ampTable[0][0]) - 1);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x00006084); ps->push_back(0x01000000);
			ps->push_back(20);               ps->push_back(0x01006085); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x01006085); ps->push_back(0);
			ps->push_back(20);               ps->push_back(ddsAdrs);    ps->push_back(0x01000000);
			ps->push_back(pgo - 150);        ps->push_back(ddsAdrs);    ps->push_back(digPhase);
			ps->push_back(20);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
			psInfo.lineCnt += 10;
			psInfo.currentTTL = psInfo.currentTTL | hpaGate | ddsGate;
		}
		else if (ampType == MATRIX2D && (freqType == FLOAT64 || freqType == FLOAT32)) // Amplitude provided as a table and frequency as a scalar
		{
			float** ampTable = ampVar.GetMatrix2D();
			if (freqType == FLOAT64)
				frequency = freqVar.GetDouble();
			else
				frequency = (double)freqVar.GetReal();

			uint32 txFreq1, txFreq2;
			ConvertFrequency(frequency, txFreq1, txFreq2);

			ps->push_back(15);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate);
			ps->push_back(15);               ps->push_back(0x00006084); ps->push_back(nint(ampTable[0][0]) - 1);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x00006084); ps->push_back(0x01000000);
			ps->push_back(20);               ps->push_back(0x01006085); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x01006085); ps->push_back(0);
			ps->push_back(20);               ps->push_back(ddsAdrs);    ps->push_back(0x01000000);
			ps->push_back(20);               ps->push_back(ddsAdrs);    ps->push_back(digPhase);
			ps->push_back(20);               ps->push_back(ddsAdrs);    ps->push_back(txFreq1);
			ps->push_back(pgo - 190);        ps->push_back(ddsAdrs);    ps->push_back(txFreq2);
			ps->push_back(20);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
			psInfo.lineCnt += 12;
			psInfo.currentTTL = psInfo.currentTTL | hpaGate | ddsGate;
		}
		else if (ampType == FLOAT32 && freqType == MATRIX2D) // Amplitude provided as scalar, frequency as a table
		{
			digAmp = ConvertTxGain(ampVar.GetReal());
			float** freqTable = freqVar.GetMatrix2D();
			ps->push_back(15);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate);
			ps->push_back(15);               ps->push_back(0x00006084); ps->push_back(nint(freqTable[0][0]) - 1);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x00006084); ps->push_back(0x01000000);
			ps->push_back(20);               ps->push_back(0x01006085); ps->push_back(0);
			ps->push_back(20);               ps->push_back(0x01006085); ps->push_back(0);
			ps->push_back(20);               ps->push_back(ddsAdrs);    ps->push_back(digAmp);
			ps->push_back(20);               ps->push_back(ddsAdrs);    ps->push_back(digPhase);
			ps->push_back(20);               ps->push_back(ddsAdrs);    ps->push_back(0x01000000);
			ps->push_back(20);               ps->push_back(0x01006086); ps->push_back(0);
			ps->push_back(pgo - 210);        ps->push_back(ddsAdrs);    ps->push_back(0x01000000);
			ps->push_back(20);               ps->push_back(0x00000000); ps->push_back(psInfo.currentTTL | hpaGate | ddsGate);
			psInfo.lineCnt += 13;
		}
		psInfo.currentTTL = psInfo.currentTTL | hpaGate | ddsGate;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 3, "channel, amplitude, phase, [frequency]", "eeee", "qqqq", &chName, &ampName, &phName, &freqName)) < 0)
			return(nrArgs);

		if (ampName[0] == 'a' || ampName[0] == 't')
			InsertUniqueStringIntoList(ampName.Str(), &parList, szList);
		if (freqName[0] == 'f')
			InsertUniqueStringIntoList(freqName.Str(), &parList, szList);
		if (phName[0] == 'p' || phName[0] == 't')
			InsertUniqueStringIntoList(phName.Str(), &parList, szList);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;

		if (ampName[0] == 'a' && freqName[0] == 'f') // Amplitude provided as scalar and frequency as a scalar
		{
			if (IsSequenceVariable(ampName.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos, adrs + 10, 0);
			}
			if (IsSequenceVariable(freqName.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos + 0x1000, adrs + 22, 0);
				AddToVariableUpdateTable(varPos, adrs + 28, 0);
			}
			if (IsSequenceParameter(ampName.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos, adrs + 10, 0);
			}
			if (IsSequenceParameter(freqName.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos + 0x1000, adrs + 22, 0);
				AddToVariableUpdateTable(varPos, adrs + 28, 0);
			}
			AddPhaseToUpdateTable(phName.Str(), adrs + 16, 0);
			psInfo.lineCnt += 6;
		}
		else if (ampName[0] == 'a' && freqName[0] != 'f' && freqName[0] != 't') // Amplitude provided as scalar but no frequency
		{
			if (IsSequenceVariable(ampName.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos, adrs + 10, 0);
			}
			if (IsSequenceParameter(ampName.Str(), varPos))
			{
				AddToFixedUpdateTable(0, varPos, adrs + 10, 0);
			}
			AddPhaseToUpdateTable(phName.Str(), adrs + 16, 0);
			psInfo.lineCnt += 4;
		}
		else if (ampName[0] == 't' && freqName[0] != 'f' && freqName[0] != 't') // Amplitude provided as table but no frequency
		{
			AddPhaseToUpdateTable(phName.Str(), adrs + 52, 0);
			psInfo.lineCnt += 10;
		}
		else if (ampName[0] == 't' && freqName[0] == 'f') // Amplitude provided as table and frequency as a scalar
		{
			AddPhaseToUpdateTable(phName.Str(), adrs + 52, 0);
			if (IsSequenceVariable(freqName.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos + 0x1000, adrs + 58, 0);
				AddToVariableUpdateTable(varPos,          adrs + 64, 0);
			}
			if (IsSequenceParameter(freqName.Str(), varPos))
			{
				AddToFixedUpdateTable(0, varPos + 0x1000, adrs + 58, 0);
				AddToFixedUpdateTable(0, varPos         , adrs + 64, 0);
			}
			psInfo.lineCnt += 12;
		}
		else if (ampName[0] == 'a' && freqName[0] == 't') // Amplitude provided as scalar and frequency as a table
		{
			if (IsSequenceVariable(ampName.Str(), varPos))
			{
				AddToVariableUpdateTable(varPos, adrs + 46, 0);
			}
			if (IsSequenceParameter(ampName.Str(), varPos))
			{
				AddToFixedUpdateTable(0, varPos, adrs + 46, 0);
			}
			AddPhaseToUpdateTable(phName.Str(), adrs + 52, 0);
			psInfo.lineCnt += 13;
		}
	}

	return(OK);
}


/*******************************************************************************

  Generate code to switch off the transmitter for channel 1 or 2
  Format is:
  txoff(channel (1/2))

* *****************************************************************************/

short TxOff(DLLParameters* par, char* args)
{
	CText chName;
	short nrArgs;
	uint32 channel, ddsAdrs, ddsGate, hpaGate;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "channel", "c", "t", &chName)) < 0)
			return(nrArgs);

		if (chName == "mode")
		{
			try
			{
				CText channel;
				EvaluateArg(par->itfc, chName.Str(), UNQUOTED_STRING, channel);
				chName = channel;
			}
			catch (char* errorStr)
			{
				ErrorMessage(errorStr);
				return(ERR);
			}
		}

		CText txt;
	   txt.Format("txoff(%s)", chName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		std::vector<uint32>* ps = &(psInfo.ps);

		// Select the gate addresses based on the channel
		if (chName == "1")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x4000;
			ddsGate = 0x0008;
		}
		else if (chName == "1nb")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x0000;
			ddsGate = 0x0008;
		}
		else if (chName == "wi")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x4000;
			ddsGate = 0x0008;
		}
		else if (chName == "we")
		{
			ddsAdrs = 0x118E;
			hpaGate = 0x0100;
			ddsGate = 0x0008;
		}
		else if (chName == "2")
		{
			ddsAdrs = 0x128E;
			hpaGate = 0x0100;
			ddsGate = 0x0002;
		}
		else if (chName == "2nb")
		{
			ddsAdrs = 0x128E;
			hpaGate = 0x0000;
			ddsGate = 0x0002;
		}
		else if (chName == "w2")
		{
			ddsAdrs = 0x128E;
			hpaGate = 0x0500;
			ddsGate = 0x0002;
		}
		else
		{
			ErrorMessage("Invalid channel %s for txoff", chName.Str());
			return(ERR);
		}

		// Remove the Tx gate bits from the currentTTL word
		psInfo.currentTTL = psInfo.currentTTL & ~(hpaGate | ddsGate);

		ps->push_back(15);         ps->push_back(0x00000000);  ps->push_back(psInfo.currentTTL);
		ps->push_back(15);         ps->push_back(ddsAdrs);     ps->push_back(0);
		ps->push_back(15);         ps->push_back(ddsAdrs);     ps->push_back(0);
		ps->push_back(100-45);     ps->push_back(0x09000000);  ps->push_back(0);
		psInfo.lineCnt += 4;		
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "channel", "e", "q", &chName)) < 0)
			return(nrArgs);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;
		psInfo.lineCnt += 4;

	}

	return(OK);
}

/*******************************************************************************
  Wait for a trigger pulse on the TD0 line

	 trigger()

	 duration = until trigger appears - minimum 100 ns

*******************************************************************************/


short WaitForTrigger(DLLParameters* par, char* args)
{
	short nrArgs;
	FILE* fp;
	CText conditionName;
	CText condition;
	CText txt;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "trigger condition", "c", "t", &conditionName)) < 0)
			return(nrArgs);

		txt.Format("trigger(%s)", conditionName.Str());
		ETInfo info = { psInfo.lineCnt,txt.Str() };
		eventTableInfo.push_back(info);

		// Extract the values by evaluating the arguments
		try
		{
			short type;
			EvaluateArg(par->itfc, conditionName.Str(), UNQUOTED_STRING , condition);
		}
		catch (char* errorStr)
		{
			ErrorMessage(errorStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);
		uint32 psOffset = ps->size();

		if (condition == "high" || condition == "on high")
		{
			ps->push_back(20);  ps->push_back(0x0F000000); ps->push_back(0x01);
			ps->push_back(20);  ps->push_back(0x09000000); ps->push_back(0x00);
			psInfo.lineCnt += 2;
		}
		else if (condition == "low" || condition == "on low")
		{
			ps->push_back(20);  ps->push_back(0x0F000000); ps->push_back(0x00);
			ps->push_back(20);  ps->push_back(0x09000000); ps->push_back(0x00);
			psInfo.lineCnt += 2;
		}
		else if (condition == "rising" || condition == "on rising")
		{
			ps->push_back(20);  ps->push_back(0x0F000000); ps->push_back(0x00);
			ps->push_back(20);  ps->push_back(0x0F000000); ps->push_back(0x01);
			psInfo.lineCnt += 2;
		}
		else if (condition == "falling" || condition == "on falling")
		{
			ps->push_back(20);  ps->push_back(0x0F000000); ps->push_back(0x01);
			ps->push_back(20);  ps->push_back(0x0F000000); ps->push_back(0x00);
			psInfo.lineCnt += 2;
		}
		else
		{
			ErrorMessage("Invalid condition (should be one of : high/low/rising/falling)");
			return(ERR);
		}
	}
	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "trigger condition", "e", "q", &conditionName)) < 0)
			return(nrArgs);

		if (conditionName[0] == 'a')
			InsertUniqueStringIntoList(conditionName.Str(), &parList, szList);

		psInfo.lineCnt += 2;
	}

	return(OK);
}


/*******************************************************************************
  Increment a table index

	 incindex(table, dec)

	 table : name of table array(2x1) generated with saveTable.
	 inc : amount by which index should be incremented (positive only)

	 duration = 1 us

*******************************************************************************/

short IncTableIndex(DLLParameters* par, char* args)
{
	CText tableName;
	CText incrementName = "1";
	short nrArgs;
	uint32 increment;
	uint32 tableAdrs, tableEntries = 0;
	float durationF;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "table, [increment]", "cc", "tt", &tableName, &incrementName)) < 0)
			return(nrArgs);

		CText txt;
		if (nrArgs == 1)
			txt.Format("incindex(%s) [1 us]", tableName.Str());
		else if (nrArgs == 2)
			txt.Format("incindex(%s, %s) [1 us]", tableName.Str(), incrementName.Str());
		else
		{
			ErrorMessage("Invalid number of arguments (1-3)");
			return(ERR);
		}

		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		try
		{
			EvaluateArg(par->itfc, tableName.Str(), MATRIX2D, tableAdrs, tableEntries);
			EvaluateArg(par->itfc, incrementName.Str(), INTEGER, increment);
		}
		catch (char* errorStr)
		{
			ErrorMessage(errorStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		ps->push_back(20); ps->push_back(0x00006081); ps->push_back(0x00000004); // Set DDC input (necessary?)
		ps->push_back(20); ps->push_back(0x00006084); ps->push_back(tableAdrs - 1); // SRAM streaming address <= current table index location
		ps->push_back(20); ps->push_back(0x01006086); ps->push_back(0); // Read contents of this address (FIFO dummy)
		ps->push_back(20); ps->push_back(0x01006086); ps->push_back(0); // Read contents of this address (current table index) and send to DSPWrite tableValue input
		ps->push_back(20); ps->push_back(0x00006086); ps->push_back(0x02000000 + increment); // Current table index location <=  Current table index + increment 

		psInfo.lineCnt += 5;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 1, "table, increment", "ee", "qq", &tableName, &incrementName)) < 0)
			return(nrArgs);

		if (tableName[0] == 't')
			InsertUniqueStringIntoList(tableName.Str(), &parList, szList);
		if (tableName[0] == 'n')
			InsertUniqueStringIntoList(incrementName.Str(), &parList, szList);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;

		if (IsSequenceVariable(tableName.Str(), varPos))
			AddToVariableUpdateTable(varPos, adrs + 10, 0);
		if (IsSequenceVariable(incrementName.Str(), varPos))
			AddToVariableUpdateTable(varPos, adrs + 28, 0);
		if (IsSequenceParameter(tableName.Str(), varPos))
		{
			AddToFixedUpdateTable(2, varPos, adrs + 10, 0);
		}
		psInfo.lineCnt += 5;
	}

	return(OK);
}

// Set the start index for a table in the TRex SRAM
short SetTableIndex(DLLParameters* par, char* args)
{
	CText tableName;
	CText indexName;
	short nrArgs;
	uint32 index;
	uint32 tableAdrs, tableEntries;

	if (psInfo.mode == "compile" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "table, index", "cc", "tt", &tableName, &indexName)) < 0)
			return(nrArgs);

		CText txt;
		txt.Format("setindex(%s, %s) [1 us]", tableName.Str(), indexName.Str());
		ETInfo info = { psInfo.lineCnt, txt.Str() };
		eventTableInfo.push_back(info);

		try
		{
			EvaluateArg(par->itfc, tableName.Str(), MATRIX2D, tableAdrs, tableEntries);
			EvaluateArg(par->itfc, indexName.Str(), INTEGER, index);
		}
		catch (char* errorStr)
		{
			ErrorMessage(errorStr);
			return(ERR);
		}

		std::vector<uint32>* ps = &(psInfo.ps);

		ps->push_back(20); ps->push_back(0x00006081); ps->push_back(0x00000004);
		ps->push_back(20); ps->push_back(0x00006084); ps->push_back(tableAdrs - 1);
		ps->push_back(60); ps->push_back(0x00006086); ps->push_back(tableAdrs + index);
		psInfo.lineCnt += 3;
	}

	if (psInfo.mode == "tables" || psInfo.mode == "both")
	{
		if ((nrArgs = ArgScan(par->itfc, args, 2, "table, index", "ee", "qq", &tableName, &indexName)) < 0)
			return(nrArgs);

		if (tableName[0] == 't')
			InsertUniqueStringIntoList(tableName.Str(), &parList, szList);
		if (tableName[0] == 'n')
			InsertUniqueStringIntoList(indexName.Str(), &parList, szList);

		long varPos;
		uint32 adrs = psInfo.lineCnt * 6;

		if (IsSequenceVariable(tableName.Str(), varPos))
		{
			AddToVariableUpdateTable(varPos, adrs + 10, 0);
			AddToVariableUpdateTable(varPos, adrs + 16, 0);
		}
		if (IsSequenceParameter(tableName.Str(), varPos))
		{
			AddToFixedUpdateTable(2, varPos, adrs + 10, 0);
			AddToFixedUpdateTable(3, varPos, adrs + 16, 0);
		}
		if (IsSequenceVariable(indexName.Str(), varPos))
			AddToVariableUpdateTable(varPos, adrs + 16, 0);

		// This is a combined parameter
		long varPos1, varPos2;
		if (IsSequenceParameter(tableName.Str(), varPos1) && IsSequenceParameter(indexName.Str(), varPos2))
		{
			AddToFixedUpdateTable(100, varPos1 + varPos2*2^6, adrs + 16, 0);
		}
		psInfo.lineCnt += 3;
	}

	return(OK);
}



/****************************************************************************
  Reorganise data returned from the FX3/TRex
*****************************************************************************/

short UnpackData(DLLParameters* par, char* args)
{
	short nrArgs;
	Variable dataVar;
	CText mode = "FIFO";
	signed int rds, ids;

	if ((nrArgs = ArgScan(par->itfc, args, 2, "mode, data", "ee", "tv", &mode, &dataVar)) < 0)
		return(nrArgs);

	mode.LowerCase();
	if (mode == "fifo")
	{
		if (dataVar.GetType() == MATRIX2D && dataVar.GetDimY() == 1)
		{
			int nrWords = dataVar.GetDimX();
			float** dataIn = (float**)dataVar.GetData();
			complex** dataOut = MakeCMatrix2D(nrWords / 3, 1);
			for (int i = 0; i < nrWords; i += 3)
			{
				unsigned int d1 = dataIn[0][i];
				unsigned int d2 = dataIn[0][i + 1];
				unsigned int d3 = dataIn[0][i + 2];
				unsigned int rd = ((d1 & 0x00FFFF) << 8) + ((d2 & 0x00FF00) >> 8);
				unsigned int id = ((d2 & 0x0000FF) << 16) + (d3 & 0x00FFFF);
				rds = (rd & 0x00800000) ? (rd | 0xFF000000) : rd;
				ids = (id & 0x00800000) ? (id | 0xFF000000) : id;
				dataOut[0][i / 3].r = (float)rds;
				dataOut[0][i / 3].i = (float)ids;
			}
			par->retVar[1].AssignCMatrix2D(dataOut, nrWords / 3, 1);
			par->nrRetVar = 1;
		}
		else
		{
			ErrorMessage("Invalid input data");
			return(ERR);
		}
	}
	else
	{
		ErrorMessage("Unknown unpack mode '%s'", mode.Str());
		return(ERR);
	}

	return(OK);
}

// If set to true this flag will cause all sequence parameters to be added to 
// the update table, otherwise only variable parameters

short SetUpdateTableMode(DLLParameters* par, char* args)
{
	short nrArgs;
	CText updateMode;

	if (gGenerateFullUpdateTable)
		updateMode = "full";
	else
		updateMode = "minimal";

	if ((nrArgs = ArgScan(par->itfc, args, 0, "generate full update", "e", "t", &updateMode)) < 0)
		return(nrArgs);

	if (nrArgs == 0)
	{
		par->nrRetVar = 1;
		par->retVar[1].MakeAndSetString(updateMode.Str());
		return(OK);
	}

	if (updateMode == "full")
	{
		gGenerateFullUpdateTable = true;
	}
	else if (updateMode == "minimal")
	{
		gGenerateFullUpdateTable = false;
	}
	else
	{
		ErrorMessage("Expecting \"full\"/\"minimal\"");
		return(ERR);
	}


	return(OK);
}

/****************************************************************************
  Pack the pulse sequence data for sending to the FX3 and calculate
  the expected duration.
*****************************************************************************/

#define MAX_LOOPS 10

short PackPS(DLLParameters* par, char* args)
{
	short nrArgs;
	Variable dataVar;
	bool inLoop = false;
	int loopStack[MAX_LOOPS];
	_int64 duration[MAX_LOOPS];
	int loopCnt = 0;
	bool inSkip = false;
	int endSkipWord = -1;
	long loopOffset = 0; // Needed if we are using different start code from the original 

	if ((nrArgs = ArgScan(par->itfc, args, 1, "ps data, loop-offset", "ee", "vl", &dataVar, &loopOffset)) < 0)
		return(nrArgs);

	for (int i = 0; i < MAX_LOOPS; i++)
	{
		loopStack[i] = 0;
		duration[i] = 0.0;
	}

	if (dataVar.GetType() == DMATRIX2D && dataVar.GetDimY() == 1)
	{
		int nrWords = dataVar.GetDimX();
		unsigned int lastTime;
		double** dataIn = (double**)(dataVar.GetData());
		float** dataOut = MakeMatrix2D(nrWords * 2, 1);
		for (int i = 0, c = 0; i < nrWords; i += 3, c += 6)
		{
			unsigned int tm = (unsigned int)(dataIn[0][i] + 0.5);
			unsigned int adrs = (unsigned int)(dataIn[0][i + 1] + 0.5);
			unsigned int info = (unsigned int)(dataIn[0][i + 2] + 0.5);
			unsigned int cmd = adrs & 0x0F000000;

			if(inSkip == true) // Check for end of skip section
			{
				if (i == endSkipWord)
					inSkip = false;
			}

			if (inSkip == false) // Sum up the durations if not skipping
			{
				duration[loopCnt] += tm;
				//	TextMessage("%d %d\n", i, tm);
			}

			if (cmd == 0x0A000000) // Check for skip command
			{
				if ((info & 0x8000) > 0) // Skip command is active
				{
					inSkip = true;
					endSkipWord = info & 0x0FFF; // Skip section ends here
				}
			}

			if (inSkip == false) // Note looping if not skipping
			{
				if (cmd == 0x04000000) // Loop command
				{
					loopStack[++loopCnt] = (info & 0xFFFFFF); // Limit loops to 24 bits (!)
				}
				else if (cmd == 0x05000000) // Endloop command
				{
					info = info + loopOffset;
					duration[loopCnt - 1] += duration[loopCnt] * loopStack[loopCnt];
					duration[loopCnt] = 0;
					loopStack[loopCnt] = 0;
					loopCnt--;
				}
			}

			dataOut[0][c + 1] = ((tm & 0xFFFF0000) >> 16);
			dataOut[0][c + 0] = (tm & 0x0000FFFF);
			dataOut[0][c + 3] = ((adrs & 0xFFFF0000) >> 16);
			dataOut[0][c + 2] = (adrs & 0x0000FFFF);
			dataOut[0][c + 5] = ((info & 0xFFFF0000) >> 16);
			dataOut[0][c + 4] = (info & 0x0000FFFF);

			//	TextMessage("%d %X %X %X\n", i, tm, (int)(dataOut[0][c + 1]), (int)(dataOut[0][c + 0]));

		}

		//for (int i = 0; i < MAX_LOOPS; i++)
		//{
		//	TextMessage("%d %d %f\n",i, loopStack[i], (float)duration[i]);
		//}

		par->retVar[1].AssignMatrix2D(dataOut, nrWords * 2, 1);
		par->retVar[2].MakeAndSetDouble((double)(duration[0] + 0.5));
		par->nrRetVar = 2;
	}
	else
	{
		ErrorMessage("Invalid input data - should be a 1D array");
		return(ERR);
	}

	return(OK);
}


// Convert a frequency in MHz to two 32 bit digital words suitable for the DDS
void ConvertFrequency(double frequencyMHz, uint32& hiWord, uint32& loWord)
{
	uint32 DDSFword = nuint(frequencyMHz * 4294967.296); // *2^32/1000
	hiWord = (DDSFword & 0xFFFF0000) >> 16;  // AD9910 Tx frequency
   loWord = (DDSFword & 0x0000FFFF);
}

// Given a variable name evaluate it and return a float
void EvaluateArg(void* itfc, char* varName, short varType, float& result)
{
	Variable resultVar;
	if (Evaluate(itfc, NORMAL, varName, &resultVar) == ERR)
	{
		throw("");
	}


	if (resultVar.GetType() == varType)
	{
		result = resultVar.GetReal();
	}
	else
	{
		CText errStr;
		errStr.Format("Invalid type for %s", varName);
		throw(errStr.Str());
	}
}


// Given a variable name evaluate it and return a double
void EvaluateArg(void* itfc, char* varName, short varType, double &result)
{
	Variable resultVar;
	if (Evaluate(itfc, NORMAL, varName, &resultVar) == ERR)
	{
		throw("");
	}

	if (resultVar.GetType() == FLOAT32)
	{
		result = (double)resultVar.GetReal();
	}
	else if (resultVar.GetType() == FLOAT64)
	{
		result = resultVar.GetDouble();
	}
	else
	{
		CText errStr;
		errStr.Format("Invalid type for %s", varName);
		throw(errStr.Str());
	}
}


// Given a variable name evaluate it and return an unsigned integer
void EvaluateArg(void* itfc, char* varName, short varType, uint32& result)
{
	Variable resultVar;
	if (Evaluate(itfc, NORMAL, varName, &resultVar) == ERR)
	{
		throw("");
	}

	if (resultVar.GetType() == FLOAT32)
	{
		result = nuint(resultVar.GetReal());
	}
	else if (resultVar.GetType() == FLOAT64)
	{
		result = nuint(resultVar.GetDouble());
	}
	else
	{
		CText errStr;
		errStr.Format("Invalid type for %s (type=%d)", varName, resultVar.GetType());
		throw(errStr.Str());
	}
}


// Given a variable name evaluate it and return a string
// Only used by pulse channel number which should be an 1/2
// or 1nb/2nb
void EvaluateArg(void* itfc, char* varName, short varType, CText& result)
{
	Variable resultVar;
	 
	if (!strcmp(varName, "1nb") || !strcmp(varName, "2nb"))
	{
		result = varName;
		return;
	}

	if (Evaluate(itfc, NORMAL, varName, &resultVar) == ERR)
	{
		throw("");
	}

	if (resultVar.GetType() == FLOAT32)
	{
		result.Format("%d", nuint(resultVar.GetReal()));
	}
	else if (resultVar.GetType() == FLOAT64)
	{
		result.Format("%d", nuint(resultVar.GetDouble()));
	}
	else if (resultVar.GetType() == UNQUOTED_STRING)
	{
		result = resultVar.GetString();
	}
	else
	{
		CText errStr;
		errStr.Format("Invalid type for %s", varName, resultVar.GetType());
		throw(errStr.Str());
	}
}

// Given a variable name evaluate it and return a signed intefger
void EvaluateArg(void* itfc, char* varName, short varType, int32& result)
{
	Variable resultVar;
	if (Evaluate(itfc, NORMAL, varName, &resultVar) == ERR)
	{
		throw("");
	}

	if (resultVar.GetType() == FLOAT32)
	{
		result = nint(resultVar.GetReal());
	}
	else if (resultVar.GetType() == FLOAT64)
	{
		result = nint(resultVar.GetDouble());
	}
	else
	{
		CText errStr;
		errStr.Format("Invalid type for %s", varName);
		throw(errStr.Str());
	}
}

// Given a variable name evaluate it and return the table address and length
void EvaluateArg(void* itfc, char* varName, short varType, uint32& tableAdrs, uint32& tableEntries)
{
	Variable resultVar;
	if (Evaluate(itfc, NORMAL, varName, &resultVar) == ERR)
	{
		throw("");
	}

   if (resultVar.GetType() == MATRIX2D)
	{
		if (resultVar.GetDimX() == 2 && resultVar.GetDimY() == 1)
		{
			float** m = resultVar.GetMatrix2D();
			tableAdrs = nuint(m[0][0]);
			tableEntries = nuint(m[0][1])-1;
		}
		else
		{
			CText errStr;
			errStr.Format("Invalid size for %s (should be 2x1)", varName);
			throw(errStr.Str());
		}
	}
	else
	{
		CText errStr;
		errStr.Format("Invalid type for %s", varName);
		throw(errStr.Str());
	}
}

// Given a variable name evaluate it and return as a void pointer
short EvaluateArg(void* itfc, char* varName, Variable* resultVar)
{
	if (Evaluate(itfc, NORMAL, varName, resultVar) == ERR)
	{
		throw("");
	}

	return(resultVar->GetType());
}


uint32 ConvertTxGain(float ampdB)
{
	uint32 amp = nuint(16384 * pow(10.0, ampdB / 20.0) - 1);
	if (amp < 0) amp = 0;
	if (amp > 16384) amp = 16383;
	return amp;
}


/**********************************************************************
	  Add string 'str' into 'list' at 'position'
**********************************************************************/

void InsertUniqueStringIntoList(char* str, char*** list, int32& position)
{
	for (int i = 0; i < position; i++)
	{
		if (!strcmp(str, (*list)[i]))
			return;
	}
	InsertStringIntoList(str, list, position, position++);
}

