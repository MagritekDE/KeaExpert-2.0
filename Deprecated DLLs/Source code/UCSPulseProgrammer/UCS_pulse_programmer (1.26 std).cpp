/*************************************************************
                 UCS Pulse Programmer 

Provides DLL commands to generate pulse sequences for the 
Spinsolve (UC) spectrometer - proton/carbon channel only

*************************************************************/

#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <shellapi.h>

#define DSP_MEMORY_ADRS 10000 // Memory location 30000 for new DSP 10000 for old DSP

#define VERSION 1.26 //  fork from 1.18 for Std SW 

/******************************************************************************************************************
   Version history

 1.0 - copied from Kea2PP folder

 1.1 -----------------------------

   1. Added rxchannel and rxgainselection 
   2. Added the shim16 command for use with the ultracompact spectrometer
   3. Removed shim command.

 1.2 -----------------------------

   1. Added gradramp16 command
   2. Simplified shim16 ASM code - increased serial transfer rate to allow 2 us steps.

 1.3 -----------------------------

   1. Fixed bug in txon for two channels. The second ch. phase and frequency were always the same as the first.

 1.4 -----------------------------
   
   1. Fixed address bug for second channel - different on UCS
   2. Removed references to internal and external channels. It is either channel 1 or 2.

 1.5 -----------------------------

   1. Added settxfreqs commands so both channel frequencies can be set.
   2. Remove code at end of RF pulses which is resetting the phase and frequency as this took up time.
      however this does mean that the default frequency is then changed after a pulse which sets the frequency
      and also that the total pulse command duration is shorter.
   3. Added a line to SelectDuration which prevents jitter when the duration is a constant.
   4. When setting Tx frequencies don't write second frequeny to parameter list.

 1.6 -----------------------------

   1. Txon was not correctly switching on TTL lines for channel 2 (carbon) NEED TO TEST

 1.7 -----------------------------

   1. Correct error in txoff (switching off proton)
   2. Correct error in pulse (switching off carbon)
   3. Correct error in dual pulse (switching off both)
   4. Added nops to cleardata and acquire to improve data transfer reliability

 1.8 -----------------------------
   
   1. Change clock phase on shim16 and gradramp16 commands to make them more reliable

 1.9 ------------------------------

   1. Added new commands acquireon and acquireoff which allow other p.s. commands while acquiring
   2. Added new command chirprf (frequency modulated RF)
   3. Added new commands gradon and gradoff (for new gradient controller)
	4. Renamed gradramp16 to shimramp16.
	5. Added new command gradramp (for new gradient controller)

 1.10 ------------------------------

   1. Modified the gradon and gradoff commands to include a gradient amplifier switch.
   2. Modified the gradramp to include a gradient amplifier switch on.

 1.11 ------------------------------

   1. Added the diffgradon and diffgradoff commands which drive the new diffusion board
   2. Added the diffgradramp command. This command need work to allow more steps.

 1.12 ------------------------------

   1. Modified the resetting of the gradient serial line to include all bits except strobe to remove 
      possiblity of random toggling of tristate lines (24->04)

 1.13 ------------------------------

   1. Separated out gradient enable/disable from setting diffusion gradient amplitude to avoid signal glitches 

 1.14 ------------------------------

   1. Modified acquireoff to fix lockup in V50 of transceiver firmware. 

 1.15 ------------------------------

   1. Modified shim16 to allow for second shim board (i.e. 16-31)

 1.16 ------------------------------

   1. Removed execwait as this is available in Prospa now.

 1.17 ------------------------------

   1. Made read/write address 0x30000 to suit new DSP (and old) board.
	2. Fixed setup for 16 bit gradient board so it works with the new DSP board
	3. Add lockon and lockoff commands.
	4. Added missing error return from the pulse command.
	5. Added options n1 and n2 to txon to switch on these outputs without gating RF (for testing)
	6. Added single instruction delays in txoff command which was preventing new DSP board from working correctly.
	7. Modified diffgrad commands to work with low current (0.6A) gradient board.

1.18 ------------------------------

	1. Added single instruction delay in shapedrf command which was preventing the new DSP board from working correctly.
	2. Added nop delay in AcquireDataOff to allow new DSP board to working correctly.


1.23 ------------------------------ (note this skips changes in Prospa version)

	1. Removed check on long pulse lengths in pulse command.

1.24 ------------------------------

	1. Modified pulse and txon/off use correct TTL lines so 3rd channel can be selected in TempCtrl

1.25 ------------------------------

	1. Removal of unnecessary bit in TTL gating for pulse/txon command (0x300)
	2. Addition of n1 and n2 modes for pulse command (test with blanking off)
	3. Fixed bug in SelectNumber which was not checking correct limits
	4. Fixed bug in SelectFrequency - not checking for n1 rather n. Also added n2

1.26 ------------------------------

	1. Acquire on/off extended to allow append
	2. Modified option in acquireon - start == overwrite
	3. Zeroing phase as well as amplitude on RF pulses to fix small glitch on new X-channel system

	
   Last modified 21 July 2021 CDE - special version for Standard software

******************************************************************************************************************/


// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);
//short UpdateTx(DLLParameters*,char *args);
short AcquireData(DLLParameters*,char *args);
short BothChannelsRFPulse(DLLParameters*,char *args);
short ChannelOneRFPulse(DLLParameters*,char *args);
short ClearData(DLLParameters*,char *args);
short CloseHandle(DLLParameters* par, char *args);
short DecrementTableIndex(DLLParameters*,char *args);
short EitherChannelRFPulse(DLLParameters*,char *args);
short SingleChannelRFPulse(DLLParameters* par,char *args);
short EndPP(DLLParameters*,char *args);
short ExecuteAndWait(DLLParameters*,char*);
short GetHelpFolder(DLLParameters*,char *args);
short GetPPVersion(DLLParameters*,char *args);
short IncFrequency(DLLParameters*,char *args);
short IncrementTableIndex(DLLParameters*,char *args);
short IncRxFrequency(DLLParameters*,char *args);
short IncTxAmplitude(DLLParameters*,char *args);
short IncTxFrequency(DLLParameters*,char *args);
short InitialisePP(DLLParameters*,char*);
short LoopEnd(DLLParameters*,char *args);
short LoopStart(DLLParameters*,char *args);
short MakeADelay(DLLParameters*,char*);
short MakeALongDelay(DLLParameters*,char *args);
short MakeAnRFPulse(DLLParameters*,char*);
short NoOperation(DLLParameters* par, char *args);
short RampedGradient(DLLParameters* par, char *args);
short RampedDiffGradient(DLLParameters* par, char *args);
short ResetMemoryPointer(DLLParameters*,char *args);
short SelectAmplitude(Interface *itfc ,FILE* fp, CText amp, CText channel);
short SelectDuration(Interface *itfc ,FILE* fp, char *reg, CText duration);
short SelectFrequency(Interface *itfc, FILE*fp, CText freq, CText channel);
short SelectNumber(Interface *itfc ,FILE* fp, CText num, char *dst, long min, long max);
short SelectPhase(Interface *itfc ,FILE* fp, CText phase, CText channel);
short SelectRxAmplifier(DLLParameters*,char *args);
short SetRxFreq(DLLParameters*,char *args);
short SetRxGain(DLLParameters*,char *args);
short SetTableIndex(DLLParameters*,char *args);
short SetTTL(DLLParameters*,char *args);
short SetTxFreq(DLLParameters*,char *args);
short SetTxFreqs(DLLParameters* par, char *args);
short ShapedRF(DLLParameters*,char *args);
short SwitchOffTx(DLLParameters*,char *args);
short SwitchOnBothTxChannels(DLLParameters*,char *args);
short SwitchOnOneTxChannel(DLLParameters*,char *args, short nrargs);
short SwitchOnTx(DLLParameters*,char *args);
short TTLOff(DLLParameters*,char *args);
short TTLOn(DLLParameters*,char *args);
short TTLPulse(DLLParameters*,char*);
short TTLTranslate(DLLParameters*,char *args);
short UpdateFrequencies(DLLParameters*,char *args);
short WaitForTrigger(DLLParameters*,char *args);
void InsertUniqueStringIntoList(char *str, char ***list, long &position);
short SwitchOnShim16(DLLParameters*,char*);
short RampedShim16(DLLParameters* par, char *args);
short SelectShim16(Interface *itfc ,FILE* fp, CText grad);
short ChirpedRF(DLLParameters* par, char *args);
short AcquireDataOn(DLLParameters* par, char *args);
short AcquireDataOff(DLLParameters* par, char *args);
short SwitchOnGradient(DLLParameters*,char*);
short SwitchOffGradient(DLLParameters*,char*);
short SwitchOnDiffGradient(DLLParameters*,char*);
short SwitchOffDiffGradient(DLLParameters*,char*);
short RampedGradient16(DLLParameters* par, char *args);
short EnableDiffGradient(DLLParameters* par, char *args);
short DisableDiffGradient(DLLParameters* par, char *args);
short ResetShims(DLLParameters* par, char *args);
short DSPTest(DLLParameters* par, char *args);
short Lockoff(DLLParameters* par, char *args);
short Lockon(DLLParameters* par, char *args);



char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list
long label;     // label counter (to generate unique label)

/*******************************************************************************
   Extension procedure to add commands to Prospa 
*******************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;
      
        if(!strcmp(command,"acquire"))           r = AcquireData(dpar,parameters);  
	else if(!strcmp(command,"acquireon"))         r = AcquireDataOn(dpar,parameters);  
   else if(!strcmp(command,"acquireoff"))        r = AcquireDataOff(dpar,parameters); 
   else if(!strcmp(command,"chirprf"))           r = ChirpedRF(dpar,parameters);      
   else if(!strcmp(command,"cleardata"))         r = ClearData(dpar,parameters);      
   else if(!strcmp(command,"delay"))             r = MakeADelay(dpar,parameters);   
   else if(!strcmp(command,"decindex"))          r = DecrementTableIndex(dpar,parameters); 
   else if(!strcmp(command,"diffgradenable"))    r = EnableDiffGradient(dpar,parameters);  
   else if(!strcmp(command,"diffgraddisable"))   r = DisableDiffGradient(dpar,parameters);  
   else if(!strcmp(command,"diffgradon"))        r = SwitchOnDiffGradient(dpar,parameters); 
   else if(!strcmp(command,"diffgradoff"))       r = SwitchOffDiffGradient(dpar,parameters); 
   else if(!strcmp(command,"diffgradramp"))      r = RampedDiffGradient(dpar,parameters);   
   else if(!strcmp(command,"endloop"))           r = LoopEnd(dpar,parameters);      
   else if(!strcmp(command,"endpp"))             r = EndPP(dpar,parameters);  
 //  else if(!strcmp(command,"execwait"))          r = ExecuteAndWait(dpar,parameters);
   else if(!strcmp(command,"gradon"))            r = SwitchOnGradient(dpar,parameters);  
   else if(!strcmp(command,"gradoff"))           r = SwitchOffGradient(dpar,parameters); 
   else if(!strcmp(command,"gradramp"))          r = RampedGradient(dpar,parameters);   
   else if(!strcmp(command,"dsptest"))           r = DSPTest(dpar,parameters);   
   else if(!strcmp(command,"helpfolder"))        r = GetHelpFolder(dpar,parameters);  
   else if(!strcmp(command,"incindex"))          r = IncrementTableIndex(dpar,parameters);   
   else if(!strcmp(command,"inctxamp"))          r = IncTxAmplitude(dpar,parameters);   
   else if(!strcmp(command,"incrxfreq"))         r = IncRxFrequency(dpar,parameters);   
   else if(!strcmp(command,"inctxfreq"))         r = IncTxFrequency(dpar,parameters);
   else if(!strcmp(command,"initpp"))            r = InitialisePP(dpar,parameters);  
	else if(!strcmp(command,"lockoff"))           r = Lockoff(dpar,parameters);     
   else if(!strcmp(command,"lockon"))            r = Lockon(dpar,parameters);     
   else if(!strcmp(command,"loop"))              r = LoopStart(dpar,parameters);     
   else if(!strcmp(command,"memreset"))          r = ResetMemoryPointer(dpar,parameters);     
   else if(!strcmp(command,"nop"))               r = NoOperation(dpar,parameters);     
   else if(!strcmp(command,"ppversion"))         r = GetPPVersion(dpar,parameters);      
   else if(!strcmp(command,"pulse"))             r = MakeAnRFPulse(dpar,parameters);    
   else if(!strcmp(command,"selectrxamp"))       r = SelectRxAmplifier(dpar,parameters);   
   else if(!strcmp(command,"setindex"))          r = SetTableIndex(dpar,parameters);   
   else if(!strcmp(command,"setrxfreq"))         r = SetRxFreq(dpar,parameters);   
   else if(!strcmp(command,"setrxgain"))         r = SetRxGain(dpar,parameters);   
   else if(!strcmp(command,"settxfreq"))         r = SetTxFreq(dpar,parameters); 
   else if(!strcmp(command,"settxfreqs"))        r = SetTxFreqs(dpar,parameters); 
   else if(!strcmp(command,"shapedrf"))          r = ShapedRF(dpar,parameters);  
   else if(!strcmp(command,"shim16"))            r = SwitchOnShim16(dpar,parameters); 
   else if(!strcmp(command,"shimramp16"))        r = RampedShim16(dpar,parameters);   
   else if(!strcmp(command,"ttl"))               r = TTLOn(dpar,parameters);      
   else if(!strcmp(command,"ttlon"))             r = TTLOn(dpar,parameters);      
   else if(!strcmp(command,"ttloff"))            r = TTLOff(dpar,parameters);      
   else if(!strcmp(command,"ttlpulse"))          r = TTLPulse(dpar,parameters);      
   else if(!strcmp(command,"ttltranslate"))      r = TTLTranslate(dpar,parameters);      
   else if(!strcmp(command,"trigger"))           r = WaitForTrigger(dpar,parameters);       
   else if(!strcmp(command,"txoff"))             r = SwitchOffTx(dpar,parameters);   
   else if(!strcmp(command,"txon"))              r = SwitchOnTx(dpar,parameters);  
   else if(!strcmp(command,"wait"))              r = MakeALongDelay(dpar,parameters);      
//   else if(!strcmp(command,"resetshims"))        r = ResetShims(dpar,parameters);      
                
   return(r);
}

/*******************************************************************************
   Extension procedure to list commands in DLL
*******************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   UCS (Proton/Carbon) Pulse Programmer DLL module (V%1.2f)\n\n",VERSION);
   TextMessage("   acquire ......... acquire some data\n");
   TextMessage("   acquireon ....... start acquiring or append some data\n");
   TextMessage("   acquireoff ...... pause or finish acquiring the data\n");
   TextMessage("   chirprf ......... make a frequency and amplitude moduated RF pulse\n");
   TextMessage("   cleardata ....... clear data memory\n");
   TextMessage("   decindex ........ decrement a table index\n");
   TextMessage("   delay ........... generate a short delay\n");
   TextMessage("   diffgradenable .. enable the diffusion gradient\n");
   TextMessage("   diffgraddisable . disable the diffusion gradient\n");
   TextMessage("   diffgradon ...... set the diffusion gradient amplitude\n");
   TextMessage("   diffgradoff ..... zero the diffusion gradient amplitude\n");
   TextMessage("   diffgradramp ...  use a linear ramp to control the diffusion gradient amplitude\n");
   TextMessage("   endloop ......... end a loop\n");
   TextMessage("   endpp ........... finish the pulse program\n");
 //  TextMessage("   execwait ........ execute a program and wait for it to exit\n");
   TextMessage("   gradon .......... switch on the gradient\n");
   TextMessage("   gradoff ......... switch off the gradient\n");
   TextMessage("   gradramp ........ use a linear ramp to control the gradient amplitude\n");
   TextMessage("   incrxfreq ....... increment the rx frequency\n");
   TextMessage("   inctxfreq ....... increment the tx frequency\n");
   TextMessage("   initpp .......... initialise pulse program\n");
   TextMessage("   incindex ........ increment a table index\n");
   TextMessage("   inctxamp ........ increment tx amplitude\n");
   TextMessage("   lockoff ......... switch off the lock\n");
   TextMessage("   lockon .......... switch on the lock\n");
   TextMessage("   loop ............ start a loop\n");
   TextMessage("   memreset ........ reset memory pointer\n");
   TextMessage("   ppversion ....... returns the version number of this DLL\n");
   TextMessage("   pulse ........... generate an RF pulse\n");
   TextMessage("   selectrxamp ..... select rx amplifier to use\n");
   TextMessage("   setindex ........ set a table index\n");
   TextMessage("   setrxfreq ....... set the receive frequency\n");
   TextMessage("   setrxgain ....... set the receive amplifier gain\n");
   TextMessage("   settxfreq ....... set the pulse frequency\n");
   TextMessage("   settxfreqs ...... set the pulse frequencies for both channels\n");
   TextMessage("   shapedrf ........ make a phase and amplitude moduated RF pulse\n");
   TextMessage("   shim16 .......... set a shim current\n");
   TextMessage("   shimramp16 ...... ramp one of the shim currents\n");
   TextMessage("   ttlon ........... switch on a TTL level\n");
   TextMessage("   ttloff .......... switch off a TTL level\n");
   TextMessage("   ttlpulse ........ generate TTL pulse\n");
   TextMessage("   ttltranslate .... translate TTL pin number to byte code\n");
   TextMessage("   trigger ......... wait for trigger input\n");
   TextMessage("   txoff ........... turn off the transmitter output\n");
   TextMessage("   txon ............ turn on the transmitter output\n");
   TextMessage("   wait ............ generate a long delay\n");
}

/*******************************************************************************
   Extension procedure to return syntax in DLL 
*******************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';

   if(!strcmp(cmd,"acquire"))                strcpy(syntax,"acquire(mode, number points:n, [duration:d])");
   else if(!strcmp(cmd,"acquireon"))         strcpy(syntax,"acquireon([mode=overwrite/start/append], number points:n)");
   else if(!strcmp(cmd,"acquireoff"))        strcpy(syntax,"acquireoff(mode=overwrite/pause/finish, number points:n)");
   else if(!strcmp(cmd,"chirprf"))           strcpy(syntax,"chirprf(channel (1/2), atable:t, ftable:f, phase:p, table_size:n, table_step_duration:d)");
	else if(!strcmp(cmd,"cleardata"))         strcpy(syntax,"cleardata(number:n)");
   else if(!strcmp(cmd,"decindex"))          strcpy(syntax,"decindex(table:t)");
   else if(!strcmp(cmd,"delay"))             strcpy(syntax,"delay(duration:d/t)");
   else if(!strcmp(cmd,"diffgradenable"))    strcpy(syntax,"diffgradenable()");
   else if(!strcmp(cmd,"diffgraddisable"))   strcpy(syntax,"diffgraddisable()");
   else if(!strcmp(cmd,"diffgradon"))        strcpy(syntax,"diffgradon(level:n/t)");
   else if(!strcmp(cmd,"diffgradoff"))       strcpy(syntax,"diffgradoff()");
   else if(!strcmp(cmd,"diffgradramp"))      strcpy(syntax,"diffgradramp(start:n/t, end:n/t, steps:n, delay:d)");
   else if(!strcmp(cmd,"endloop"))           strcpy(syntax,"endloop(name)");
   else if(!strcmp(cmd,"endpp"))             strcpy(syntax,"endpp()");
 //  else if(!strcmp(cmd,"execwait"))          strcpy(syntax,"execwait(program,arguments)");
   else if(!strcmp(cmd,"gradon"))            strcpy(syntax,"gradon(level:n/t)");
   else if(!strcmp(cmd,"gradoff"))           strcpy(syntax,"gradoff()");
   else if(!strcmp(cmd,"gradramp"))          strcpy(syntax,"gradramp(start:n/t, end:n/t, steps:n, delay:d)");
   else if(!strcmp(cmd,"incindex"))          strcpy(syntax,"incindex(table:t)");
   else if(!strcmp(cmd,"incrxfreq"))         strcpy(syntax,"incrxfreq(increment:f)");
   else if(!strcmp(cmd,"inctxfreq"))         strcpy(syntax,"inctxfreq(increment:f)");
   else if(!strcmp(cmd,"inctxamp"))          strcpy(syntax,"inctxamp(amp:a, increment:a)");
   else if(!strcmp(cmd,"initpp"))            strcpy(syntax,"initpp(filename)");
   else if(!strcmp(cmd,"lockoff"))           strcpy(syntax,"lockoff()");
   else if(!strcmp(cmd,"lockon"))            strcpy(syntax,"lockon()");
   else if(!strcmp(cmd,"loop"))              strcpy(syntax,"loop(name,n)");
   else if(!strcmp(cmd,"memreset"))          strcpy(syntax,"memreset([address:n])");
   else if(!strcmp(cmd,"nop"))               strcpy(syntax,"nop()");     
   else if(!strcmp(cmd,"ppversion"))         strcpy(syntax,"(INT v) = ppversion()");
	else if(!strcmp(cmd,"pulse"))             strcpy(syntax,"pulse(channel (1/2), amp:a, phase:p, duration:d [,freq:f] OR pulse(1 ,a1, p1, f1, 2 ,a2, p2, f2, d])");
   else if(!strcmp(cmd,"selectrxamp"))       strcpy(syntax,"selectrxamp(number n)");
   else if(!strcmp(cmd,"setindex"))          strcpy(syntax,"setindex(table:t,index:n)");
   else if(!strcmp(cmd,"setrxfreq"))         strcpy(syntax,"setrxfreq(freq:f)");
   else if(!strcmp(cmd,"settxfreq"))         strcpy(syntax,"settxfreq(freq:f) OR settxfreq(channel:1/2 freq:f) ");
   else if(!strcmp(cmd,"settxfreqs"))        strcpy(syntax,"settxfreqs(freq1:f, freq2:f)");
   else if(!strcmp(cmd,"setrxgain"))         strcpy(syntax,"setrxgain(channel:n, gain:g)");
   else if(!strcmp(cmd,"shapedrf"))          strcpy(syntax,"shapedrf(channel (1/2), atable:t, stable:t, phase:p, table_size:n, table_step_duration:d)");
   else if(!strcmp(cmd,"shim16"))            strcpy(syntax,"shim16(channel:n, amplitude:n)");
	else if(!strcmp(cmd,"shimramp16"))        strcpy(syntax,"shimramp16([address:n], start:n/t, end:n/t, steps:n, delay:d)");
	else if(!strcmp(cmd,"trigger"))           strcpy(syntax,"trigger()");
   else if(!strcmp(cmd,"ttltranslate"))      strcpy(syntax,"(INT byte) = ttltranslate(pin number)");
   else if(!strcmp(cmd,"ttl"))               strcpy(syntax,"ttl(byte:b)");
   else if(!strcmp(cmd,"ttlon"))             strcpy(syntax,"ttlon(byte:b)");
   else if(!strcmp(cmd,"ttloff"))            strcpy(syntax,"ttloff(byte:b)");
   else if(!strcmp(cmd,"ttlpulse"))          strcpy(syntax,"ttlpulse(byte:b, duration:d)");
   else if(!strcmp(cmd,"txoff"))             strcpy(syntax,"txoff(mode)");
	else if(!strcmp(cmd,"txon"))              strcpy(syntax,"txon(mode, amp:a, phase:p [,freq:f]) OR txon(ch1, a1, p1, f1, ch2, a2, p2, f2)");
   else if(!strcmp(cmd,"wait"))              strcpy(syntax,"wait(duration:w)");
 //  else if(!strcmp(cmd,"resetshims"))         strcpy(syntax,"resetshims(amplitude:n)");


   if(syntax[0] == '\0')
      return(false);
   return(true);
}


/******************************************************************************************
  Start acquiring data from the Kea transceiver. 
  
  acquireon(nr_points)
  
  Last modified: 26-Sept-14
*******************************************************************************************/

short AcquireDataOn(DLLParameters* par, char *args)
{
   short nrArgs;
   CText nrPnts;
   long NrPnts;
   FILE *fp;
	CArg carg;
	CText mode = "overwrite";

	nrArgs = carg.Count(args);

	if(nrArgs == 1)
	{
		if((nrArgs = ArgScan(par->itfc,args,1,"nr points","e","q",&nrPnts)) < 0)
			return(nrArgs);
	}
	else
	{
		if((nrArgs = ArgScan(par->itfc,args,2,"mode, nr points","ee","tq",&mode, &nrPnts)) < 0)
			return(nrArgs);
	}

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add new variables to the list
   InsertUniqueStringIntoList(nrPnts.Str(),&parList,szList);

// Open the output file
   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

	fprintf(fp,"\n\n;");
	fprintf(fp,"\n;***************************************************************************");
	fprintf(fp,"\n; Start acquiring data (using FIFO)");

	if(mode == "overwrite" || mode == "start")
	{
		if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
		fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; Number of data points to collect");
		fprintf(fp,"\n        move    #$000003,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_DRP1_CR        ; Enable data ready");
		fprintf(fp,"\n        move    #$000001,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_FIFO_CR        ; Reset FIFO");
		fprintf(fp,"\n        move    #$000006,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_FIFO_CR        ; Turn FIFO capture on and select CIC output");
		fprintf(fp,"\n        move    y:TTL,a1                 ; Load the current TTL state");
		fprintf(fp,"\n        or      #$000010,a               ; Reset CIC");
		fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Send to FPGA");
		fprintf(fp,"\n        and     #$ffffef,a               ; Remove CIC flag");
		fprintf(fp,"\n        or      #$000001,a               ; Start ADC capture");
		fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Send to FPGA");
		fprintf(fp,"\n        move    a1,y:TTL                 ; Save total TTL state");
	}
	else if(mode == "append")
	{
		if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
		fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; Number of data points to collect");
		fprintf(fp,"\n        move    #$000003,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_DRP1_CR        ; Enable data ready");
		fprintf(fp,"\n        move    #$000006,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_FIFO_CR        ; Turn FIFO capture on and select CIC output");
		fprintf(fp,"\n        move    y:TTL,a1                 ; Load the current TTL state");
		fprintf(fp,"\n        or      #$000010,a               ; Reset CIC");
		fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Send to FPGA");
		fprintf(fp,"\n        and     #$ffffef,a               ; Remove CIC flag");
		fprintf(fp,"\n        or      #$000001,a               ; Start ADC capture");
		fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Send to FPGA");
		fprintf(fp,"\n        move    a1,y:TTL                 ; Save total TTL state");

	}

	fclose(fp);

   return(OK);
} 

/******************************************************************************************
  Stop acquiring data from the Kea transceiver. 
  
  acquireoff(mode, nr_points)
  
  Last modified: 21-June-16
*******************************************************************************************/

short AcquireDataOff(DLLParameters* par, char *args)
{
   short nrArgs;
   CText nrPnts;
   CText mode = "overwrite";
   long NrPnts;
   FILE *fp;

	if((nrArgs = ArgScan(par->itfc,args,2,"mode, nr points","ee","tq",&mode, &nrPnts)) < 0)
		return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add new variables to the list
   InsertUniqueStringIntoList(nrPnts.Str(),&parList,szList);

   // Open the output file
   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

// Overwrite mode
   if(mode == "overwrite") // Each data point is stored in a new location, subsequent calls overwrite this
   {
	   fprintf(fp,"\n\n;");
	   fprintf(fp,"\n;***************************************************************************");
	   fprintf(fp,"\n; Stop acquiring data and copy to DSP");
      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        clr     b");
      if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
      if(SelectNumber(par->itfc,fp,nrPnts,"b1",2,65536)) return(ERR);
      fprintf(fp,"\n        lsl     #1,a                     ; Multiply by 2");
      fprintf(fp,"\n        add     a,b                      ; Add to get x3");
      fprintf(fp,"\n        move    x:FPGA_FIFO_Lv,a1        ; Read FIFO level (first one is invalid)");
      fprintf(fp,"\n        nop ");
      fprintf(fp,"\nLBL%ld    move    x:FPGA_FIFO_Lv,a1        ; Read FIFO level",label);
      fprintf(fp,"\n        cmp     b,a                      ; Compare FIFO count");
      fprintf(fp,"\n        blt     LBL%ld",label);
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    y:TTL,a1                 ; Stop ADC capture");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Send to FPGA");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Copy data from FIFO to main memory");        
      fprintf(fp,"\n        move    #$%d,r5               ; Specify the save address",DSP_MEMORY_ADRS);
      if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Flush out first word needed for slow spinsolve bus");
      fprintf(fp,"\n        do      r7,LBL%ld                  ; Copy n samples",label+1);
      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        clr     b");
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Load data from FIFO get first 16 bit word");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        lsl     #8,a");
      fprintf(fp,"\n        move    a1,y:(r5)                ; Save to memory for now");
      fprintf(fp,"\n        nop"); 
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Load data from FIFO get second 16 bit word");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        tfr     a,b                      ; Copy into b");
      fprintf(fp,"\n        lsr     #8,a");
      fprintf(fp,"\n        and     #$0000FF,a               ; Mask out any unwanted bits");
      fprintf(fp,"\n        move    y:(r5),x1                ; Copy first word out of memory");
      fprintf(fp,"\n        or      x1,a                     ; Build full word into a");
      fprintf(fp,"\n        move    a1,y:(r5)+               ; Save to memory");
      fprintf(fp,"\n        lsl     #16,b");
      fprintf(fp,"\n        and     #$FF0000,b               ; Mask out any unwanted bits");
      fprintf(fp,"\n        move    b1,y:(r5)                ; Save to memory for now");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Load data from FIFO get third 16 bit word");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        and     #$00FFFF,a               ; Mask out any unwanted bits");
      fprintf(fp,"\n        move    y:(r5),x1                ; Copy word out of memory");
      fprintf(fp,"\n        or      x1,a                     ; Build full word into a");
      fprintf(fp,"\n        move    a1,y:(r5)+               ; Save to memory");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\nLBL%ld    nop",label+1);
      label+=2;

	   fclose(fp);
   }
	else if(mode == "pause")
	{
	   fprintf(fp,"\n\n;");
	   fprintf(fp,"\n;***************************************************************************");
	   fprintf(fp,"\n; Stop acquiring data but don't reset");

      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        clr     b");
      if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
      if(SelectNumber(par->itfc,fp,nrPnts,"b1",2,65536)) return(ERR);
      fprintf(fp,"\n        lsl     #1,a                     ; Multiply by 2");
      fprintf(fp,"\n        add     a,b                      ; Add to get x3");
      fprintf(fp,"\n        move    x:FPGA_FIFO_Lv,a1        ; Read FIFO level (first one is invalid)");
      fprintf(fp,"\n        nop ");
      fprintf(fp,"\nLBL%ld    move    x:FPGA_FIFO_Lv,a1        ; Read FIFO level",label);
      fprintf(fp,"\n        cmp     b,a                      ; Compare FIFO count");
      fprintf(fp,"\n        blt     LBL%ld",label);
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    y:TTL,a1                 ; Stop ADC capture");
      fprintf(fp,"\n        and		#$fffffe,a               ;");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Send to FPGA");
      fprintf(fp,"\n        move    a1,y:TTL                 ");

      label+=1;
	   fclose(fp);
	}
	else if(mode == "finish")
	{
	   fprintf(fp,"\n\n;");
	   fprintf(fp,"\n;***************************************************************************");
	   fprintf(fp,"\n; Stop appending data and copy to DSP but append to previous data");

      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        clr     b");
      if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
      if(SelectNumber(par->itfc,fp,nrPnts,"b1",2,65536)) return(ERR);
      fprintf(fp,"\n        lsl     #1,a                     ; Multiply by 2");
      fprintf(fp,"\n        add     a,b                      ; Add to get x3");
      fprintf(fp,"\n        move    x:FPGA_FIFO_Lv,a1        ; Read FIFO level (first one is invalid)");
      fprintf(fp,"\n        nop ");
      fprintf(fp,"\nLBL%ld    move    x:FPGA_FIFO_Lv,a1        ; Read FIFO level",label);
      fprintf(fp,"\n        cmp     b,a                      ; Compare FIFO count");
      fprintf(fp,"\n        blt     LBL%ld",label);
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    y:TTL,a1                 ; Stop ADC capture");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Send to FPGA");

      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Copy data from FIFO to main memory");        
		fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the data address");
      if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Flush out first word needed for slow spinsolve bus");
      fprintf(fp,"\n        do      r7,LBL%ld                  ; Copy n samples",label+1);
      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        clr     b");
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Load data from FIFO get first 16 bit word");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        lsl     #8,a");
      fprintf(fp,"\n        move    a1,y:(r5)                ; Save to memory for now");
      fprintf(fp,"\n        nop"); 
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Load data from FIFO get second 16 bit word");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        tfr     a,b                      ; Copy into b");
      fprintf(fp,"\n        lsr     #8,a");
      fprintf(fp,"\n        and     #$0000FF,a               ; Mask out any unwanted bits");
      fprintf(fp,"\n        move    y:(r5),x1                ; Copy first word out of memory");
      fprintf(fp,"\n        or      x1,a                     ; Build full word into a");
      fprintf(fp,"\n        move    a1,y:(r5)+               ; Save to memory");
      fprintf(fp,"\n        lsl     #16,b");
      fprintf(fp,"\n        and     #$FF0000,b               ; Mask out any unwanted bits");
      fprintf(fp,"\n        move    b1,y:(r5)                ; Save to memory for now");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Load data from FIFO get third 16 bit word");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        and     #$00FFFF,a               ; Mask out any unwanted bits");
      fprintf(fp,"\n        move    y:(r5),x1                ; Copy word out of memory");
      fprintf(fp,"\n        or      x1,a                     ; Build full word into a");
      fprintf(fp,"\n        move    a1,y:(r5)+               ; Save to memory");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\nLBL%ld    nop",label+1);
      label+=2;

	   fclose(fp);
	}
   else
   {
      fclose(fp);
      Error(par->itfc,"invalid acquire mode");
      return(ERR);
   }

   return(OK);
} 
	
	

short ChirpedRF(DLLParameters* par, char *args)
{
   short nrArgs;
   CText channel,dur,atable,ftable,phase,size,duration;
   long Duration,Phase,Size;
   float fDuration;
   CArg carg;
   const int pgoOffset =  5;
   const int firstDelay = 198;
   const int midDelay =   159;
   const int endDelay =   162;

   if((nrArgs = ArgScan(par->itfc,args,6,"channel, atable, ftable, phase, table_size, table_step","eeeeee","qqqqqq",&channel,&atable,&ftable,&phase,&size,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   if(channel != "1" && channel != "2")
   {
      ErrorMessage("unknown RF chirp pulse channel %s",channel.Str());
      return(ERR);
   }

// Set up parameter list
   InsertUniqueStringIntoList(atable.Str(),&parList,szList);
   InsertUniqueStringIntoList(ftable.Str(),&parList,szList);
   if(phase[0] == 'p')
      InsertUniqueStringIntoList(phase.Str(),&parList,szList);
   if(size[0] == 'n')
      InsertUniqueStringIntoList(size.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

	fprintf(fp,"\n\n;");
	fprintf(fp,"\n;***************************************************************************");
	fprintf(fp,"\n; Generate an amplitude and frequency modulated internal RF pulse");
	fprintf(fp,"\n;"); 

   fprintf(fp,"\n        clr a                           ; Clear the accumulator");

   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL level");


   if(channel == "1") // Internal channel 1 Kea pulse
		fprintf(fp,"\n        or      #$004000,a");
   else // Internal channel 2 Kea pulse
      fprintf(fp,"\n        or      #$000300,a");

	fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");

   fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
   fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); 
   fprintf(fp,"\n        add     #%d,a",pgoOffset);
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

  // Get the table addresses
   fprintf(fp,"\n; Get the amplitude and frequency table pointers");
   fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
   fprintf(fp,"\n        move    x:TABLE%s,r4",ftable.Str()+1);

  // Get the size of the tables
   if(size[0] == 'n')
   {
       fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
   }
   else if(sscanf(size.Str(),"%ld",&Size) == 1)
   {
      if(Size < 2)
      {
         ErrorMessage("invalid table size '%s' [>= 2]",size);
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #%ld,r2",Size);
   }

   fprintf(fp,"\n        move    r2,a0"); 
   fprintf(fp,"\n        dec     a                        ; Decrement because first value already used"); 
   fprintf(fp,"\n        move    a0,r2"); 


   if(channel == "2")
   {
      fprintf(fp,"\n; Set the rf output to its initial value");
      fprintf(fp,"\n        move    y:(r5)+,a1               ; Load amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");

   // Get phase from parameter
      if(phase[0] == 'p')
      {
          fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         fprintf(fp,"\n        move    #%ld,a1",Phase);
      }
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set phase");  

      fprintf(fp,"\n        move    y:(r4)+,a1"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
      fprintf(fp,"\n        move    y:(r4)+,a1               ; Load frequency"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
   }
   else
   {
      fprintf(fp,"\n; Set the rf output to its initial value");
      fprintf(fp,"\n        move    y:(r5)+,a1               ; Load amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");

   // Get phase from parameter
      if(phase[0] == 'p')
      {
          fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         fprintf(fp,"\n        move    #%ld,a1",Phase);
      }
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set phase");  

      fprintf(fp,"\n        move    y:(r4)+,a1"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
      fprintf(fp,"\n        move    y:(r4)+,a1               ; Load frequency"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
   }

   fprintf(fp,"\n; Wait for pgo delay to end");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

   fprintf(fp,"\n; Start modulated pulse");

   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
	if(channel == "1")
		fprintf(fp,"\n        or      #$000008,a             ; Channel 1 RF on");
	else
		fprintf(fp,"\n        or      #$000002,a             ; Channel 2 RF on");

	fprintf(fp,"\n        move    a1,y:TTL                ; Load the current TTL word");
   fprintf(fp,"\n        move    a1,x:FPGA_TTL"); 

// Get delay for each table value
   fprintf(fp,"\n; Step length");
   if(duration[0] == 'd')
   {
      fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
   }
   else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
   {
      if(fDuration < 0.25 || fDuration > 327670)
      {
         ErrorMessage("invalid delay '%ld' [1...327670]",fDuration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp,"\n        move    #%ld,a1",Duration);
   }
   else
   {
      ErrorMessage("Invalid duration reference '%s'",duration.Str());
      fclose(fp);
      return(ERR);
   }

//   Delay to get length of first pulse step correct
   fprintf(fp,"\n        lsl     #1,a");
   fprintf(fp,"\n        sub     #%d,a",firstDelay);
   fprintf(fp,"\n        move    a1,a0");

   fprintf(fp,"\n; Delay for correct first step length");
   fprintf(fp,"\n        rep     a0");
   fprintf(fp,"\n        nop");

// Calculate delay for subsequent steps
   fprintf(fp,"\n; Calculate subsequent step length delay");
   fprintf(fp,"\n        add     #%d,a",midDelay);
   fprintf(fp,"\n        move    a1,a0");

// Loop over the tables
   fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

   if(channel == "2")
   {
    // Update the amplitude from table
      fprintf(fp,"\n        move    y:(r5)+,a1               ; Load amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set amplitude");   

   // Get phase from parameter
      if(phase[0] == 'p')
      {
          fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         fprintf(fp,"\n        move    #%ld,a1",Phase);
      }
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set phase");  

    // Update the frequency from table
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set correct frequency");
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
   }
   else
   {
    // Update the amplitude from table
      fprintf(fp,"\n        move    y:(r5)+,a1               ; Load amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set amplitude");   

   // Get phase from parameter
      if(phase[0] == 'p')
      {
          fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         fprintf(fp,"\n        move    #%ld,a1",Phase);
      }
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set phase");  

    // Update the frequency from table
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set correct frequency");
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
   }

   fprintf(fp,"\n; Adjust for correct step length");
   fprintf(fp,"\n        rep     a0");
   fprintf(fp,"\n        nop");

   fprintf(fp,"\nLBL%ld    nop",label++);

   fprintf(fp,"\n; End Delay (correct for last pulse)");
   fprintf(fp,"\n        rep     #%d",endDelay);

   fprintf(fp,"\n        nop");

	fprintf(fp,"\n; End pulse");

   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");

   if(channel == "1")
      fprintf(fp,"\n        and     #$FFBFF7,a               ; Switch off channel 1");
   else
      fprintf(fp,"\n        and     #$FFFCFD,a               ; Switch off channel 2");


	fprintf(fp,"\n        move    a1,y:TTL                 ; Load the current TTL word");
   fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Update TTL & RF");

   fprintf(fp,"\n        move    #$000000,a1"); 

	if(channel == "2")
	{
		fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ;Zero amplitude");
	}
	else
	{
		fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ;Zero amplitude");
	}

	fclose(fp);
  

   return(OK);
}

/**********************************************************************
     Set a gradient level - takes 2.2 us

     gradon(amplitude)
     gradon("nx"/"tx"/integer)

	  The amplitude is signed 16 bit

 The addressing is shown below for the 74HC138 chip

 A0 - PE0 = 0
 A1 - PE2 = 0
 A2 - PC0 = 1

 This means that device #3 is selected. The data sent to this device
 is 10XXXX where XXXX is the data word. This means we are writing to 
 channel 9 (the first channel in device 3).


**********************************************************************/

short SwitchOnGradient(DLLParameters* par, char *args)
{
   short nrArgs;
   CText amplitude;
	long Amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,1,"amplitude","e","q",&amplitude)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add to parameter list if not a constant
   if(amplitude[0] == 'n' || amplitude[0] == 't')
      InsertUniqueStringIntoList(amplitude.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");

// Switch on the gradient amp
   fprintf(fp,"\n; Switch on gradient amp");
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred");
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
  // fprintf(fp,"\n        movep   #$1343C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$01,a1");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad SW PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad SW PEX=001");
	fprintf(fp,"\n        move    #$FFFFFF,a1");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
   fprintf(fp,"\n        move    #15,r7                   ; Wait 15 us");
   fprintf(fp,"\n        bsr     wait");

// Set the gradient level
   fprintf(fp,"\n; Set gradient level");
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred");
 //  fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
   fprintf(fp,"\n        movep   #$1343C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$00,a1");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PEX=000");
   if(amplitude[0] == 'n') // Amplitude is via a number reference
	{
      fprintf(fp,"\n        move    x:NR%s,a1                ; Get gradient amplitude",amplitude.Str()+1);
	}
   else if(sscanf(amplitude.Str(),"%ld",&Amplitude) == 1) // Amplitude is a number
	{
		fprintf(fp,"\n        move    #%ld,a1",Amplitude);
	}
   else if(amplitude[0] == 't') // Amplitude is a table (via current index)
	{
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",amplitude.Str()+1);
      fprintf(fp,"\n        dec a");             // a0 points to table index
      fprintf(fp,"\n        move    a0,r5");     // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");
      fprintf(fp,"\n        move    x:TABLE%s,b0",amplitude.Str()+1);
      fprintf(fp,"\n        add     b,a");       // Add the index to table start to find current value
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read the table value
	}
	fprintf(fp,"\n        move    #$00FFFF,x1");
   fprintf(fp,"\n        and     x1,a1");
   fprintf(fp,"\n        move    #$100000,x1");
   fprintf(fp,"\n        or      x1,a                   ; Add amplitude word");
   fprintf(fp,"\n        move    a1,x:A_TX10            ; Send channel info + grad. amplitude to DAC");
   fprintf(fp,"\n        move    #2,r7                  ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");
   fprintf(fp,"\n        movep   #$04,x:A_PCRD          ; Turn off SSI 1 on Port D");

	fclose(fp);
  
   return(OK);
}

short SwitchOffGradient(DLLParameters* par, char *args)
{
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");

// Reset the gradient level
   fprintf(fp,"\n; Reset the gradient level");
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred");
   fprintf(fp,"\n        movep   #$1343C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$00,a1");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
   fprintf(fp,"\n        move    #$0000,a1");
   fprintf(fp,"\n        move    #$00FFFF,x1");
   fprintf(fp,"\n        and     x1,a1");
   fprintf(fp,"\n        move    #$100000,x1");
   fprintf(fp,"\n        or      x1,a");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send channel info + grad. amplitude to DAC");
   fprintf(fp,"\n        or      x1,a");
   fprintf(fp,"\n        move    #15,r7                   ; Wait 15 us");
   fprintf(fp,"\n        bsr     wait");

// Switch off the gradient amp
   fprintf(fp,"\n; Switch off gradient amp");
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred");
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
 //  fprintf(fp,"\n        movep   #$1343C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$01,a1");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
	fprintf(fp,"\n        move    #$000000,a1");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
   fprintf(fp,"\n        move    #2,r7                   ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");
   fprintf(fp,"\n        movep   #$04,x:A_PCRD           ; Turn off SSI 1 on Port D");


	fclose(fp);
  
   return(OK);
}

/**********************************************************************
     Set the diffusion gradient amplitude - takes xx us

     diffgradon(amplitude)
     diffgradon("nx"/"tx"/integer)

	  The amplitude is signed 16 bit or 20 bit (see below for option)

**********************************************************************/

short SwitchOnDiffGradient(DLLParameters* par, char *args)
{
   short nrArgs;
   CText amplitude;
	long Amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,1,"amplitude","e","q",&amplitude)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add to parameter list if not a constant
   if(amplitude[0] == 'n' || amplitude[0] == 't')
      InsertUniqueStringIntoList(amplitude.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set diffusion gradient amplitude");

// Set the serial port configuration
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred");
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");

// Select the gradient
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");

// Set the gradient level
   if(amplitude[0] == 'n') // Amplitude is via a number reference
	{
      fprintf(fp,"\n        move    x:NR%s,a1                ; Get gradient amplitude",amplitude.Str()+1);
	}
   else if(sscanf(amplitude.Str(),"%ld",&Amplitude) == 1) // Amplitude is a number
	{
		fprintf(fp,"\n        move    #%ld,a1",Amplitude);
	}
   else if(amplitude[0] == 't') // Amplitude is a table (via current index)
	{
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",amplitude.Str()+1);
      fprintf(fp,"\n        dec a");             // a0 points to table index
      fprintf(fp,"\n        move    a0,r5");     // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");
      fprintf(fp,"\n        move    x:TABLE%s,b0",amplitude.Str()+1);
      fprintf(fp,"\n        add     b,a");       // Add the index to table start to find current value
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read the table value
	}
	fprintf(fp,"\n        move    #$00FFFF,x1"); // 16 bit version
//	fprintf(fp,"\n        move    #$0FFFFF,x1"); // 20 bit version
   fprintf(fp,"\n        and     x1,a1");
   fprintf(fp,"\n        move    #$100000,x1");
   fprintf(fp,"\n        or      x1,a                    ; Add amplitude word");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send channel info + grad. amplitude to DAC");
   fprintf(fp,"\n        move    #2,r7                   ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");
   fprintf(fp,"\n        movep   #$04,x:A_PCRD           ; Turn off SSI 1 on Port D");

	fclose(fp);
  
   return(OK);
}

/**********************************************************************
     Zero the diffusion gradient amplitude - takes xx us

     diffgradoff()

**********************************************************************/

short SwitchOffDiffGradient(DLLParameters* par, char *args)
{
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Zero the diffusion gradient amplitude");

// Set the serial port configuration
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred");
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");

// Switch off the diffusion gradient amp
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$00,a1");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
   fprintf(fp,"\n        move    #$0000,a1");
   fprintf(fp,"\n        move    #$00FFFF,x1");
   fprintf(fp,"\n        and     x1,a1");
   fprintf(fp,"\n        move    #$100000,x1");
   fprintf(fp,"\n        or      x1,a");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send channel info + grad. amplitude to DAC");
   fprintf(fp,"\n        or      x1,a");
   fprintf(fp,"\n        move    #2,r7                   ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");
   fprintf(fp,"\n        movep   #$04,x:A_PCRD           ; Turn off SSI 1 on Port D");

	fclose(fp);
  
   return(OK);
}


/**********************************************************************
     Enable the diffusion gradient amplitude - takes xx us

     diffgraddisable()

**********************************************************************/

short EnableDiffGradient(DLLParameters* par, char *args)
{   
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Enable diffusion gradient amp");

// Set the serial port configuration
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred");
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");

// Switch on the gradient amp
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$0001,b1               ; Enable DAC");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
   fprintf(fp,"\n        move    #$200002,a1             ; DAC control word");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
   fprintf(fp,"\n        move    #2,r7                   ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n        move    #$01,a1			    ; Enable amplifier");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0 PE0=1");
   fprintf(fp,"\n        move    #$FFFFFF,a1		    ; Grad control word");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
   fprintf(fp,"\n        move    #2,r7                   ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");
   fprintf(fp,"\n        movep   #$04,x:A_PCRD           ; Turn off SSI 1 on Port D");

	fclose(fp);
  
   return(OK);
}

/**********************************************************************
     Disable the diffusion gradient amplitude - takes xx us

     diffgraddisable()

**********************************************************************/


short DisableDiffGradient(DLLParameters* par, char *args)
{
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Disable diffusion gradient amp");

// Set the serial port configuration
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred");
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");

// Switch off the gradient amp
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$01,a1");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
	fprintf(fp,"\n        move    #$000000,a1");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
   fprintf(fp,"\n        move    #2,r7                   ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");
   fprintf(fp,"\n        movep   #$04,x:A_PCRD           ; Turn off SSI 1 on Port D");

	fclose(fp);
  
   return(OK);
}


/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short GetHelpFolder(DLLParameters* par, char *args)
{
   par->retVar[1].MakeAndSetString("Macros\\Pulse Programming");
   par->nrRetVar = 1;
   return(OK);
}

/*******************************************************************************
  Adds a nop
*******************************************************************************/

short NoOperation(DLLParameters* par, char *args)
{
   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; No operation");
   fprintf(fp,"\n        nop");

   fclose(fp);

   return(OK);
}



short Lockoff(DLLParameters* par, char *args)
{
   FILE* fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Switch off lock");

	fprintf(fp,"\n        move    #32,a1");
   fprintf(fp,"\n        move    a1,x:FPGALOCK_MEM_ADDRH");
   fprintf(fp,"\n        move    #$00,a1");
   fprintf(fp,"\n        move    a1,x:FPGALOCK_MEM_LOCK_0");
   fprintf(fp,"\n        move    #$00,a1");
   fprintf(fp,"\n        move    a1,x:FPGALOCK_MEM_LOCK_1");
	fclose(fp);
	return(OK);
}

short Lockon(DLLParameters* par, char *args)
{
   FILE* fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Switch on lock");


	fprintf(fp,"\n        move    #32,a1");
   fprintf(fp,"\n        move    a1,x:FPGALOCK_MEM_ADDRH");
   fprintf(fp,"\n        move    #$00,a1");
   fprintf(fp,"\n        move    a1,x:FPGALOCK_MEM_LOCK_0");
   fprintf(fp,"\n        move    #$01,a1");
   fprintf(fp,"\n        move    a1,x:FPGALOCK_MEM_LOCK_1");

	fclose(fp);
	return(OK);
}

/*******************************************************************************
  This function will generate a file with the pulse parameters 
  and initialization code 
*******************************************************************************/

short InitialisePP(DLLParameters* par, char *args)
{
   CText dir;
   short nrArgs;

   if((nrArgs = ArgScan(par->itfc,args,1,"working directory","e","t",&dir)) < 0)
      return(nrArgs);

   if(!SetCurrentDirectory(dir.Str()))
   {
      Error(par->itfc,"directory '%s' not found",dir);
      return(ERR);
   }

// Free any previous storage
   if(szList > 0)
      FreeList(parList,szList+1);
   parList = MakeList(1);

   szList = 0;
   label = 1;

// Delete the temp file if it is still present
   int result = remove("midCode.asm");

   return(OK);
}

/*******************************************************************************
  Return the version number
*******************************************************************************/

short GetPPVersion(DLLParameters* par, char *args)
{
   par->retVar[1].MakeAndSetFloat(VERSION);
   par->nrRetVar = 1;
   return(OK);
}
      
/*******************************************************************************
  Return the DSP memory pointer
*******************************************************************************/

short ResetMemoryPointer(DLLParameters* par, char *args)
{
   short nrArgs;
   CText adrs;

   if((nrArgs = ArgScan(par->itfc,args,0,"memory address","e","t",&adrs)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(adrs.Str(),&parList,szList);


   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   if(nrArgs == 0)
   {
      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Reset the memory pointer");
	   fprintf(fp,"\n        move    #$%d,r5              ; Make r5 point to the start of fid memory", DSP_MEMORY_ADRS);
   }
   else
   {
      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Reset the memory pointer");
      fprintf(fp,"\n        move    x:NR%s,a1",adrs.Str()+1);
	   fprintf(fp,"\n        move    a1,r5                 ; Make r5 point to the start of fid memory");
   }
      
   fclose(fp);

   return(OK);
}

/*****************************************************************************

   Translate TTL pin-number to TTL code (for use with ttlon/ttloff commands)

******************************************************************************/

short TTLTranslate(DLLParameters* par, char *args)
{
   short nrArgs;
   static short pinNr;
   unsigned short translate[] = {0x00,0x00,0x40,0x10,0x04,0x01,0x80,0x20,0x08,0x02};
   float code;
   
   if((nrArgs = ArgScan(par->itfc,args,1,"pin number","e","d",&pinNr)) < 0)
      return(nrArgs);

   if(pinNr < 2 || pinNr > 9)
   {
      Error(par->itfc,"Invalid pin number (2-9)");
      return(ERR);
   }

  code = (float)translate[pinNr];

  par->retVar[1].MakeAndSetFloat(code);
  par->nrRetVar = 1;

  return(OK);
}



/**********************************************************************
              Increment the transmitter frequency (6/3/09)

     Adds a constant to the current tx frequency and then send this 
     out to the DDS.

     inctxfreq(increment_value)


     increment_value ......... a frequency variable (e.g. "f1") units MHz

     Command takes xxxx ns.

**********************************************************************/

short IncTxFrequency(DLLParameters* par, char *args)
{
   short nrArgs;
   CText incTx;

   if((nrArgs = ArgScan(par->itfc,args,1,"increment name","e","q",&incTx)) < 0)
      return(nrArgs);

   if(incTx[0] != 'f')
   {
      Error(par->itfc,"Invalid frequency reference '%s'",incTx.Str());
      return(ERR);
   }

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(incTx.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Increment the transmitter frequency");

   fprintf(fp,"\n\n; Read in the base transmitter frequency");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    x:TXF00,a1");
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,a2");
	fprintf(fp,"\n        move    x:TXF00,a1");
	fprintf(fp,"\n        lsl	   #16,a");
   fprintf(fp,"\n        move    a1,b1");
   fprintf(fp,"\n        move    a2,b2");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:TXF01,a1");
   fprintf(fp,"\n        add     a,b");


   fprintf(fp,"\n\n; Add Tx frequency step to base frequency");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_0,a1",incTx.Str()+1);
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,a2");
   fprintf(fp,"\n        move    x:FX%s_0,a1",incTx.Str()+1);
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        add     a,b");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_1,a1",incTx.Str()+1);
   fprintf(fp,"\n        add     a,b");


   fprintf(fp,"\n\n; Write over base frequency");


   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    b1,a1");
   fprintf(fp,"\n        move    b2,a2");
   fprintf(fp,"\n        and     #$00FFFF,a");
   fprintf(fp,"\n        move    a1,x:TXF01");
   fprintf(fp,"\n        move    b1,a1");
   fprintf(fp,"\n        lsr     #16,a");
   fprintf(fp,"\n        move    b2,b1");
   fprintf(fp,"\n        lsl     #8,b");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        move    a1,x:TXF00");

   fclose(fp);

   return(OK);
}

/**********************************************************************
              Increment the transmitter amplitude (6/4/09)

     Adds a constant to the current amplitude and then send this 
     out to the DDS.

     inctxamp(mode, amplitude_to_increment, increment_value)

     mode .................... "rf"/"number"  
                               Update the rf amplitude or just the number
     amplitude_to_increment .. variable to increment (e.g. "a1")
     increment_value ......... a 14 bit number variable (e.g. "n1")

     Command takes xx ns in rf mode
**********************************************************************/

short IncTxAmplitude(DLLParameters* par, char *args)
{
   short nrArgs;
   CText amp,inc;

   if((nrArgs = ArgScan(par->itfc,args,2,"amplitude_name, increment_name","ee","qq",&amp,&inc)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(amp.Str(),&parList,szList);
   InsertUniqueStringIntoList(inc.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Increment the transmitter amplitude");

   fprintf(fp,"\n\n; Read in the base transmitter amplitude");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");

	fprintf(fp,"\n        move    x:TXA%s,b1",amp.Str()+1);
 
   fprintf(fp,"\n\n; Add Tx amplitude step");
	fprintf(fp,"\n        move    x:NR%s,a1",inc.Str()+1);
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Write over base amplitude");
	fprintf(fp,"\n        move    b1,x:TXA%s",amp.Str()+1);


   fclose(fp);

   return(OK);
}


/**********************************************************************
              Increment the receiver frequency (4/11/07)
**********************************************************************/

short IncRxFrequency(DLLParameters* par, char *args)
{
   short nrArgs;
   CText incRx;

   
   if((nrArgs = ArgScan(par->itfc,args,1,"increment name","e","q",&incRx)) < 0)
      return(nrArgs);

   if(incRx[0] != 'f')
   {
      Error(par->itfc,"Invalid frequency reference '%s'",incRx.Str());
      return(ERR);
   }

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(incRx.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Increment the receiver frequency");

   fprintf(fp,"\n\n; Read in the base receiver frequency");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    x:RXF00,a1");
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,a2");
	fprintf(fp,"\n        move    x:RXF00,a1");
	fprintf(fp,"\n        lsl	   #16,a");
   fprintf(fp,"\n        move    a1,b1");
   fprintf(fp,"\n        move    a2,b2");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:RXF01,a1");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Add Rx frequency step to base frequency");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_0,a1",incRx.Str()+1);
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,a2");
   fprintf(fp,"\n        move    x:FX%s_0,a1",incRx.Str()+1);
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        add     a,b");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_1,a1",incRx.Str()+1);
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Write over base frequency");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    b1,a1");
   fprintf(fp,"\n        move    b2,a2");
   fprintf(fp,"\n        and     #$00FFFF,a");
   fprintf(fp,"\n        move    a1,x:RXF01");
   fprintf(fp,"\n        move    b1,a1");
   fprintf(fp,"\n        lsr     #16,a");
   fprintf(fp,"\n        move    b2,b1");
   fprintf(fp,"\n        lsl     #8,b");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        move    a1,x:RXF00");

   fprintf(fp,"\n\n; Update Rx frequencies");
   fprintf(fp,"\n        move    x:RXF00,a1");
   fprintf(fp,"\n        move    a1,x:FPGA_DRP1_PI");
   fprintf(fp,"\n        move    x:RXF01,a1");
   fprintf(fp,"\n        move    a1,x:FPGA_DRP1_PI");

   fclose(fp);

   return(OK);
}


/**********************************************************************
            Modify the receiver amplifier gain (8/5/12)

            3 Rx gain stages can be controlled

            1. Lock (0/4)
            2. Proton (2/6)
            3. Carbon (1/5)
**********************************************************************/

short SetRxGain(DLLParameters* par, char *args)
{
   short nrArgs;
   CText gain;
   CText channel = "lock";

   if((nrArgs = ArgScan(par->itfc,args,2,"channel,gain","ee","qq",&channel,&gain)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   if(gain[0] == 'g')
      InsertUniqueStringIntoList(gain.Str(),&parList,szList);
   if(channel[0] == 'n')
      InsertUniqueStringIntoList(channel.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Rx gain - lock option");


   fprintf(fp,"\n        movep   #$2C,x:A_PCRC           ; Set up SSI 0"); 
   fprintf(fp,"\n        movep   #$10080A,x:A_CRA0       ; /10 clk, 16 bit word transferred"); 
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB0        ; Enable SSI port with sc1/2 are outputs"); 

   if(channel == "lock")
      fprintf(fp,"\n        movep   #$0000,x:A_PDRE         ; Select first gain block");
   else if(channel == "proton")
      fprintf(fp,"\n        movep   #$0002,x:A_PDRE         ; Select first gain block");
   else if(channel == "carbon")
      fprintf(fp,"\n        movep   #$0001,x:A_PDRE         ; Select first gain block");
   else if(channel[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",channel.Str()+1);
      fprintf(fp,"\n        lsr     #8,a");
      fprintf(fp,"\n        move    a1,x:A_PDRE         ; Select first gain block");
   }
   else
   {
      ErrorMessage("invalid channel (lock/carbon/proton or nx)");
      fclose(fp);
      return(ERR);
   }
   fprintf(fp,"\n        move    x:GAIN%s_0,a1",gain.Str()+1); 
   fprintf(fp,"\n        move    a1,x:A_TX00");
   fprintf(fp,"\n        move    #10,r7                  ; Wait 10 us");
   fprintf(fp,"\n        bsr     wait");

   if(channel == "lock")
      fprintf(fp,"\n        movep   #$0004,x:A_PDRE         ; Select second gain block");
   else if(channel == "proton")
      fprintf(fp,"\n        movep   #$0006,x:A_PDRE         ; Select second gain block");
   else if(channel == "carbon")
      fprintf(fp,"\n        movep   #$0005,x:A_PDRE         ; Select second gain block");
   else if(channel[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",channel.Str()+1);
      fprintf(fp,"\n        and     #$0000FF,a");
      fprintf(fp,"\n        move    a1,x:A_PDRE         ; Select second gain block");
   }
   else
   {
      ErrorMessage("invalid channel (lock/carbon/proton or nx)");
      fclose(fp);
      return(ERR);
   }
   fprintf(fp,"\n        move    x:GAIN%s_1,a1",gain.Str()+1);
   fprintf(fp,"\n        move    a1,x:A_TX00");
   fprintf(fp,"\n        move    #10,r7                  ; Wait 10 us");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n        movep   #$0007,x:A_PDRE         ; Select unused serial port"); 
   fprintf(fp,"\n        movep   #$3c3c,x:A_CRB0         ; Disable SSI 0"); 
   fprintf(fp,"\n        movep   #$24,x:A_PCRC"); 

   fclose(fp);

 

   return(OK);
}


/**********************************************************************
        Select which Receiver amplifer to connect to port A (8/5/12)

        One of two Rx gain stages can be selected:

        1. Proton (0x000)
        2. Carbon (0x200)
**********************************************************************/

short SelectRxAmplifier(DLLParameters* par, char *args)
{
   short nrArgs;
   CText channel;

   if((nrArgs = ArgScan(par->itfc,args,1,"channel","e","t",&channel)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(channel.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Select Receiver Amplifier Channel");

   fprintf(fp,"\n        movep   #$2C,x:A_PCRC           ; Set up SSI 0"); 
   fprintf(fp,"\n        movep   #$10080A,x:A_CRA0       ; /2 clk, 16 bit word transferred"); 
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB0        ; Enable SSI port with sc1/2 are outputs"); 

   if(channel[0] == 'n')
      fprintf(fp,"\n        move    x:NR%s,a1",channel.Str()+1);
   else if( channel == "carbon")
       fprintf(fp,"\n        move    #$0000,a1"); 
   else if(channel == "proton")
       fprintf(fp,"\n        move    #$0200,a1"); 
   else
   {
      ErrorMessage("invalid channel (carbon/proton or nx)");
      fclose(fp);
      return(ERR);
   }

   fprintf(fp,"\n        movep   #$0003,x:A_PDRE         ; Select first gain block");
   fprintf(fp,"\n        move    a1,x:A_TX00");
   fprintf(fp,"\n        move    #10,r7                  ; Wait 10 us");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n        movep   #$0007,x:A_PDRE         ; Select unused serial port"); 
   fprintf(fp,"\n        movep   #$3c3c,x:A_CRB0         ; Disable SSI 0"); 
   fprintf(fp,"\n        movep   #$24,x:A_PCRC"); 

   fclose(fp);

   return(OK);
}


/**********************************************************************
            Modify the receiver frequency (8/7/11)

            This sets the current receive frequency and also the 
            frequency stored in the parameter list.
**********************************************************************/

short SetRxFreq(DLLParameters* par, char *args)
{
   short nrArgs;
   CText freq;

   if((nrArgs = ArgScan(par->itfc,args,1,"freq name","e","q",&freq)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(freq.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Rx frequency");

   fprintf(fp,"\n        move    x:FX%s_0,a1",freq.Str()+1);
   fprintf(fp,"\n        move    a1,x:RXF00");
   fprintf(fp,"\n        move    a1,x:FPGA_DRP1_PI");
   fprintf(fp,"\n        move    x:FX%s_1,a1",freq.Str()+1);
   fprintf(fp,"\n        move    a1,x:RXF01");
   fprintf(fp,"\n        move    a1,x:FPGA_DRP1_PI");

   fclose(fp);

   return(OK);
}


/**********************************************************************
         Modify the transceiver transmitter frequency (22/10/13)

         Three options:

         settxfreq()
         settxfreq(frequency: fx)
         settxfreq(channel: 1/2, frequency: fx)

         Command duration 2us

**********************************************************************/

short SetTxFreq(DLLParameters* par, char *args)
{
   short nrArgs;
   CText freq,channel = "1";
   CArg carg;

   nrArgs = carg.Count(args);
  
   if(nrArgs == 1)
   {
      if((nrArgs = ArgScan(par->itfc,args,1,"freq","e","q",&freq)) < 0)
         return(nrArgs);
   }
   else if(nrArgs == 2)
   {
      if((nrArgs = ArgScan(par->itfc,args,2,"channel, freq","ee","qq",&channel,&freq)) < 0)
         return(nrArgs);
   }
   else
   {
      Error(par->itfc,"Invalid number of parameters (1/2)");
      return(ERR);
   }

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add the parameters to the parameter list
   if(nrArgs >= 1)
      InsertUniqueStringIntoList(freq.Str(),&parList,szList);

// Open the asm file for append

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Tx frequency");
   fprintf(fp,"\n;"); 

   fprintf(fp,"\n        move    x:FX%s_0,a1",freq.Str()+1); // Copy the new frequency to
   fprintf(fp,"\n        move    a1,x:TXF00");               // the main Tx frequency location
   fprintf(fp,"\n        move    x:FX%s_1,a1",freq.Str()+1); // Tx buffer
   fprintf(fp,"\n        move    a1,x:TXF01");
   fprintf(fp,"\n        rep     #27");
   fprintf(fp,"\n        nop");

   if(channel == "1")
   {
      fprintf(fp,"\n        move    y:TX_AMP,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0"); // Amplitude
      fprintf(fp,"\n        move    #0,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0"); // Phase
      fprintf(fp,"\n        move    x:FX%s_0,a1",freq.Str()+1);
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0"); // Freq word 1
      fprintf(fp,"\n        move    x:FX%s_1,a1",freq.Str()+1);
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0"); // Freq word 2
      fprintf(fp,"\n        rep     #124                    ; Make duration 2us");
      fprintf(fp,"\n        nop");
   }
   else if(channel == "2")
   {
      fprintf(fp,"\n        move    y:TX_AMP,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0"); // Amplitude
      fprintf(fp,"\n        move    #0,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0"); // Phase
      fprintf(fp,"\n        move    x:FX%s_0,a1",freq.Str()+1);
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0"); // Freq word 1
      fprintf(fp,"\n        move    x:FX%s_1,a1",freq.Str()+1);
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0"); // Freq word 2
      fprintf(fp,"\n        rep     #124                    ; Make duration 2us");
      fprintf(fp,"\n        nop");
   }
   else
   {
      ErrorMessage("Invalid channel reference");
      fclose(fp);
      return(ERR);
   }
  
   fclose(fp);

   return(OK);
}


/**********************************************************************

    Modify the transceiver transmitter frequency on both 
    channels (22/10/13)

    settxfreqs(frequency_ch1: fx, frequency_ch2: fx)

    Command duration 4 us.

**********************************************************************/

short SetTxFreqs(DLLParameters* par, char *args)
{
   short nrArgs;
   CText freq1 = "1";
   CText freq2 = "2";

// Get user supplied arguments
   if((nrArgs = ArgScan(par->itfc,args,2,"freq ch 1, freq ch 2","ee","qq",&freq1,&freq2)) < 0)
         return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Copy the parameters and place in the master list
   if(freq1[0] == 'f')
      InsertUniqueStringIntoList(freq1.Str(),&parList,szList);
   if(freq2[0] == 'f')
      InsertUniqueStringIntoList(freq2.Str(),&parList,szList);
 
      
// Open the asm file for append
   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Tx frequencies");
   fprintf(fp,"\n;"); 

// Get first frequency
   fprintf(fp,"\n        move    x:FX%s_0,a1",freq1.Str()+1); // Copy the new frequency to
   fprintf(fp,"\n        move    a1,x:TXF00");               // the main Tx frequency location for channel 1
   fprintf(fp,"\n        move    x:FX%s_1,a1",freq1.Str()+1); // Tx buffer
   fprintf(fp,"\n        move    a1,x:TXF01");
   fprintf(fp,"\n        rep     #27");
   fprintf(fp,"\n        nop");

// Set for first channel
   fprintf(fp,"\n        move    y:TX_AMP,a1");
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0"); // Amplitude
   fprintf(fp,"\n        move    #0,a1");
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0"); // Phase
   fprintf(fp,"\n        move    x:FX%s_0,a1",freq1.Str()+1);
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0"); // Freq word 1
   fprintf(fp,"\n        move    x:FX%s_1,a1",freq1.Str()+1);
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0"); // Freq word 2
   fprintf(fp,"\n        rep     #124                    ; Make duration 2us");
   fprintf(fp,"\n        nop");

// Get second frequency
   fprintf(fp,"\n        move    x:FX%s_0,a1",freq2.Str()+1); // Copy the new frequency to
 //  fprintf(fp,"\n        move    a1,y:TX2F00");               // the main Tx frequency location for channel 2
   fprintf(fp,"\n        move    x:FX%s_1,a1",freq2.Str()+1); // Tx buffer
 //  fprintf(fp,"\n        move    a1,y:TX2F01");
   fprintf(fp,"\n        rep     #27");
   fprintf(fp,"\n        nop");

// Set for second channel
   fprintf(fp,"\n        move    y:TX_AMP,a1");
   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0"); // Amplitude
   fprintf(fp,"\n        move    #0,a1");
   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0"); // Phase
   fprintf(fp,"\n        move    x:FX%s_0,a1",freq2.Str()+1);
   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0"); // Freq word 1
   fprintf(fp,"\n        move    x:FX%s_1,a1",freq2.Str()+1);
   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0"); // Freq word 2
   fprintf(fp,"\n        rep     #124                    ; Make duration 2us");
   fprintf(fp,"\n        nop");

   fclose(fp);

   return(OK);
}


/**********************************************************************
     Generate an RF pulse setting the amplitude, (4/11/07)
     phase and duration.

     Note that there is a user define delay 'pgo' before the rf appears. 
     This allows the pulse phase and amplitude to be set and gives time for the 
     HPA biasing to be switched on.

     There are two operating modes:

     internal ... gate pulse is sent to the internal Kea RF amplifier
     external ... gate pulse is sent to the external TTL port (pin 5)

**********************************************************************/

short MakeAnRFPulse(DLLParameters* par, char *args)
{
   CArg carg;
	short r;

   short nrArgs = carg.Count(args);

   if(nrArgs <= 5)
      r = EitherChannelRFPulse(par,args);
	
   else if(nrArgs == 9)
      r = BothChannelsRFPulse(par,args);

   else
   {
      Error(par->itfc,"invalid number of arguments");
      return(ERR);
   }

   return(r);
}



/*********************************************************************************************************
 Produce a dual RF pulse on channel 1 & 2 of specified amplitude frequency, phase and duration 
 Channel 1 pulse uses the internal TTL gate or external pin 5 while channel 2 used the external TTL pin 4. 
**********************************************************************************************************/

short BothChannelsRFPulse(DLLParameters* par, char *args)
{
   short nrArgs;
   CText ch1,amp1,freq1,phase1;
   CText ch2,amp2,freq2,phase2;
   CText duration,channel;
   long Phase1,Phase2;
   long Amplitude1,Amplitude2;
   long Frequency1,Frequency2;
   long Duration;
   float fDuration;
   short ph;

  if((nrArgs = ArgScan(par->itfc,args,9,"ch1, amp1, phase1, freq1, ch2, amp2, phase2, freq2, duration","eeeeeeeee","qqqqqqqqq",&ch1,&amp1,&phase1,&freq1,&ch2,&amp2,&phase2,&freq2,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(amp1[0] == 'a')
      InsertUniqueStringIntoList(amp1.Str(),&parList,szList);
   if(freq1[0] == 'f')
      InsertUniqueStringIntoList(freq1.Str(),&parList,szList);
   if(phase1[0] == 'p' || phase1[0] == 't')
      InsertUniqueStringIntoList(phase1.Str(),&parList,szList);
   if(amp2[0] == 'a')
      InsertUniqueStringIntoList(amp2.Str(),&parList,szList);
   if(freq2[0] == 'f')
      InsertUniqueStringIntoList(freq2.Str(),&parList,szList);
   if(phase2[0] == 'p' || phase2[0] == 't')
      InsertUniqueStringIntoList(phase2.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

	fprintf(fp,"\n\n;");
	fprintf(fp,"\n;***************************************************************************");
	fprintf(fp,"\n; Generate an dual channel pulse");
	fprintf(fp,"\n;"); 

   fprintf(fp,"\n; Check for invalid pulse length");
   fprintf(fp,"\n        clr    a");

   if(SelectDuration(par->itfc ,fp, "a1", duration) == ERR) return(ERR);

// Test for valid pulse length
   fprintf(fp,"\n        move   #1,b1                     ; Abort code");
   fprintf(fp,"\n        cmp    #49999,a                  ; Check long pulses >= 1 ms");
   fprintf(fp,"\n        jgt    LBL%ld                    ; Ignore long pulses",label);
   fprintf(fp,"\n        move   #2,b1                     ; Abort code");
   fprintf(fp,"\n        cmp    #24,a                     ; Pulse must be >= 500 ns");
   fprintf(fp,"\n        jlt    ABORT");

// Gate the RF amplifiers
   fprintf(fp,"\n; Gate the RF amplifiers");

   fprintf(fp,"\nLBL%ld  move    y:TTL,a1                ; Load the current TTL level",label++);
	if(ch1 == "1" && ch2 == "2")
      fprintf(fp,"\n        or      #$04100,a               ; Ch1 and Ch2 blanking");
	else if(ch1 == "1nb" && ch2 == "2")
      fprintf(fp,"\n        or      #$04000,a               ; Ch1 blanking only");
	else if(ch1 == "1" && ch2 == "2nb")
      fprintf(fp,"\n        or      #$00100,a               ; Ch2 blanking only");

   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");

   fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
   fprintf(fp,"\n        move    x:PGO,a1");
   fprintf(fp,"\n        add     #4,a                   ; Tweak it"); //
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        nop");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   
// Set channel 1 amplitude
	fprintf(fp,"\n; Set channel 1 amplitude");
   channel = "1";
   if(SelectAmplitude(par->itfc ,fp, amp1, channel) == ERR) return(ERR);

// Set channel 1 phase
   fprintf(fp,"\n; Set channel 1 phase");
	if(SelectPhase(par->itfc ,fp, phase1,channel) == ERR) return(ERR);

// Set channel 1 frequency
   fprintf(fp,"\n; Set channel 1 frequency");
   if(SelectFrequency(par->itfc ,fp, freq1, channel) == ERR) return(ERR);


// Set channel 1 update
   fprintf(fp,"\n; Wait 2.0 us for channel 1 to update");
   fprintf(fp,"\n        move    #20,r7");
	fprintf(fp,"\n        bsr     svwait");

// Set channel 2 amplitude
	fprintf(fp,"\n; Set channel 2 amplitude");
   channel = "2";
   if(SelectAmplitude(par->itfc ,fp, amp2, channel) == ERR) return(ERR);

// Set channel 2 phase
   fprintf(fp,"\n; Set channel 2 phase");
	if(SelectPhase(par->itfc ,fp, phase2,channel) == ERR) return(ERR);

// Set channel 2 frequency
   fprintf(fp,"\n; Set channel 2 frequency");
   if(SelectFrequency(par->itfc ,fp, freq2, channel) == ERR) return(ERR);

// Wait for parameters to update
	fprintf(fp,"\n; Wait for parameters to update");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for timer");

	fprintf(fp,"\n; Start pulse");

   if(SelectDuration(par->itfc ,fp, "r3", duration) == ERR) return(ERR);
	fprintf(fp,"\n        movep   r3,x:A_TCPR2");

   fprintf(fp,"\n        nop                             ; Eliminated pulse length jitter");
   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL level");
   fprintf(fp,"\n        or      #$00000A,a              ; Switch channel 1 & 2 RF");
   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update Kea");

	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Start timer"); 
	fprintf(fp,"\n        nop");
 	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");

	fprintf(fp,"\n; End pulses");
	fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL level");

   if(ch1 == "1" && ch2 == "2")
      fprintf(fp,"\n        and     #$FFBEF5,a               ; Switch off Ch1 and Ch2 (RF and blanking");
   else if(ch1 == "1nb" && ch2 == "2")
      fprintf(fp,"\n        and     #$FFFEF5,a               ; Switch off Ch1 and Ch2 (RF for both and blanking for Ch2");
   else if(ch1 == "1" && ch2 == "2nb")
      fprintf(fp,"\n        and     #$FFBFF5,a               ; Switch off Ch1 and Ch2 (RF for both and blanking for Ch1");
   else if(ch1 == "1nb" && ch2 == "2nb")
      fprintf(fp,"\n        and     #$FFFFF5,a               ; Switch off Ch1 and Ch2 (RF for both");

   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update Kea");

   fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
	fprintf(fp,"\n        move    #$000000,a1"); 
	fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ;Zero Phase");

   fprintf(fp,"\n        rep     #50                    ; Allow time for channel 1 to reset");
   fprintf(fp,"\n        nop");

   fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
	fprintf(fp,"\n        move    #$000000,a1"); 
	fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ;Zero Phase");
	fclose(fp);

   return(OK);
}

/*********************************************************************************************************
 Produce a single RF pulse on channel 1 or 2 of specified amplitude frequency, phase and duration 
**********************************************************************************************************/

short EitherChannelRFPulse(DLLParameters* par,char *args)
{
   short nrArgs;
   CText channel,amp,freq,phase,duration;
   long Frequency;
   short ph;
   char ch;

   if((nrArgs = ArgScan(par->itfc,args,4,"channel,amp,phase,duration,[freq]","eeeee","qqqqq",&channel,&amp,&phase,&duration,&freq)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   if(amp[0] == 'a')
      InsertUniqueStringIntoList(amp.Str(),&parList,szList);
   if(freq[0] == 'f')
      InsertUniqueStringIntoList(freq.Str(),&parList,szList);
   if(phase[0] == 'p' || phase[0] == 't')
      InsertUniqueStringIntoList(phase.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   if(channel == "1" || channel == "2" || channel == "1nb" || channel == "2nb")
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate an RF pulse");
		fprintf(fp,"\n;"); 

	// Get the user defined delay
      fprintf(fp,"\n        clr    a");
		if(SelectDuration(par->itfc ,fp, "a1", duration) == ERR) return(ERR);

	// Test for invalid delay (< 0.5 us)
	   fprintf(fp,"\n; Check for invalid pulse length");
      fprintf(fp,"\n        move   #1,b1                     ; Abort code");
      fprintf(fp,"\n        cmp    #49999,a                  ; Check long pulses >= 1 ms");
      fprintf(fp,"\n        jgt    LBL%ld                    ; Ignore long pulses",label);
      fprintf(fp,"\n        move   #2,b1                     ; Abort code");
      fprintf(fp,"\n        cmp    #24,a                     ; Pulse must be >= 500 ns");
      fprintf(fp,"\n        jlt    ABORT");

	// Update channel TTL gate
      fprintf(fp,"\n; Unblank the RF amp");
      fprintf(fp,"\nLBL%ld  move    y:TTL,a1                ; Load the current TTL level",label++);
	   if(channel == "1") // Internal channel 1 Kea pulse
		   fprintf(fp,"\n        or      #$004000,a              ; Internal HPA (Channel 1)");
	   else if(channel == "2") // Internal channel 2 Kea pulse
			fprintf(fp,"\n        or      #$000100,a              ; Internal HPA (Channe1 2)");

		fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
		fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");

      fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
      fprintf(fp,"\n        move    x:PGO,a1");
      fprintf(fp,"\n        add     #3,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
		fprintf(fp,"\n        nop");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");   
      
	// Update channel amplitude
		fprintf(fp,"\n; Set channel amplitude");
		if(SelectAmplitude(par->itfc ,fp, amp, channel) == ERR) return(ERR);

	// Update channel phase (allow more general phase value?)
      fprintf(fp,"\n; Set phase");
	   if(SelectPhase(par->itfc ,fp, phase,channel) == ERR) return(ERR);

	// Update channel frequency
		if(nrArgs == 5)
		{
		   fprintf(fp,"\n; Set channel frequency");
		   if(SelectFrequency(par->itfc ,fp, freq, channel) == ERR) return(ERR);
		}

	// Wait for parameters to update (delay PGO)
	//	fprintf(fp,"\n        nop"); 
		fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");

	// Start RF pulse
		fprintf(fp,"\n; Start pulse");
		if(SelectDuration(par->itfc ,fp, "r3", duration) == ERR) return(ERR);

		fprintf(fp,"\n        movep   r3,x:A_TCPR2");

      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
		if(channel == "1" || channel == "1nb")
			fprintf(fp,"\n        or      #$000008,a              ; Channel 1 RF on");
      else if(channel == "2" || channel == "2nb")
			fprintf(fp,"\n        or      #$000002,a              ; Channel 2 RF on");
		 
		fprintf(fp,"\n        move    a1,y:TTL                ; Load the current TTL word");
		fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");

		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Start timer"); 
		fprintf(fp,"\n        nop");
    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");

	// End RF pulse
		fprintf(fp,"\n; End pulse");
    	fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

 
      if(channel == "1")
         fprintf(fp,"\n        and     #$FFBFF7,a              ; Switch off channel 1");
      else if(channel == "2")
         fprintf(fp,"\n        and     #$FFFEFD,a              ; Switch off channel 2");
      else if(channel == "1nb")
         fprintf(fp,"\n        and     #$FFFFF7,a              ; Switch off channel 1 (no blanking)");
		      else if(channel == "2nb")
         fprintf(fp,"\n        and     #$FFFFFD,a              ; Switch off channel 2 (no blanking)");

      fprintf(fp,"\n        move    a1,y:TTL                ; Update TTL word");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");

      fprintf(fp,"\n        move    #$000000,a1"); 

		if(channel == "2" || channel == "2nb")
		{
			fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ;Zero amplitude");
		   fprintf(fp,"\n        move    #$000000,a1"); 
	      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ;Zero Phase");
		}
		else
		{
			fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ;Zero amplitude");
			fprintf(fp,"\n        move    #$000000,a1"); 
	      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ;Zero Phase");
		}
		fclose(fp);
	}
   else
   {
		fclose(fp);
      Error(par->itfc,"unknown RF pulse mode");
      return(ERR);
   }

   return(OK);
}
//
//short EitherChannelRFPulse(DLLParameters* par,char *args)
//{
//   short nrArgs;
//   CText channel,amp,freq,phase,duration;
//   long Frequency;
//   short ph;
//   char ch;
//
//   if((nrArgs = ArgScan(par->itfc,args,4,"channel,amp,phase,duration,[freq]","eeeee","qqqqq",&channel,&amp,&phase,&duration,&freq)) < 0)
//      return(nrArgs);
//
//   if(!parList)
//   {
//      Error(par->itfc,"Pulse sequence not initialised");
//      return(ERR);
//   }
//   if(amp[0] == 'a')
//      InsertUniqueStringIntoList(amp.Str(),&parList,szList);
//   if(freq[0] == 'f')
//      InsertUniqueStringIntoList(freq.Str(),&parList,szList);
//   if(phase[0] == 'p' || phase[0] == 't')
//      InsertUniqueStringIntoList(phase.Str(),&parList,szList);
//   if(duration[0] == 'd')
//      InsertUniqueStringIntoList(duration.Str(),&parList,szList);
//
//   FILE *fp = fopen("midCode.asm","a");
//   if(!fp)
//   {
//      Error(par->itfc,"Can't open output file");
//      return(ERR);
//   }
//
//   if(channel == "1" || channel == "2" || channel == "3")
//	{
//		fprintf(fp,"\n\n;");
//		fprintf(fp,"\n;***************************************************************************");
//		fprintf(fp,"\n; Generate an RF pulse");
//		fprintf(fp,"\n;"); 
//
//	// Get the user defined delay
//      fprintf(fp,"\n        clr    a");
//		if(SelectDuration(par->itfc ,fp, "a1", duration) == ERR) return(ERR);
//
//	// Test for invalid delay (< 0.5 us)
//      fprintf(fp,"\n; Check for invalid pulse length");
//      fprintf(fp,"\n        move   #1,b1                     ; Abort code");
//      fprintf(fp,"\n        cmp    #49999,a                  ; Check long pulses >= 1 ms");
//      fprintf(fp,"\n        jgt    LBL%ld                    ; Ignore long pulses",label);
//      fprintf(fp,"\n        move   #2,b1                     ; Abort code");
//      fprintf(fp,"\n        cmp    #24,a                     ; Pulse must be >= 500 ns");
//      fprintf(fp,"\n        jlt    ABORT");
//
//	// Update channel TTL gate
//      fprintf(fp,"\n; Unblank the RF amp");
//      fprintf(fp,"\nLBL%ld  move    y:TTL,a1                ; Load the current TTL level",label++);
//	   if(channel == "1") // Internal channel 1 pulse
//		   fprintf(fp,"\n        or      #$004000,a              ; Internal HPA (proton)");
//	   else if(channel == "2") // Internal channel 2 pulse
//			fprintf(fp,"\n        or      #$000300,a              ; Internal HPA (X1)");
//	   else if(channel == "3") // Internal channel 3 pulse
//			fprintf(fp,"\n        or      #$002000,a              ; Internal HPA (X2)");
//
//		fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
//		fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");
//
//      fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
//      fprintf(fp,"\n        move    x:PGO,a1");
//      fprintf(fp,"\n        add     #3,a");
//      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
//		fprintf(fp,"\n        nop");
//		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");   
//      
//	// Update channel amplitude
//		fprintf(fp,"\n; Set channel amplitude");
//		if(SelectAmplitude(par->itfc ,fp, amp, channel) == ERR) return(ERR);
//
//	// Update channel phase (allow more general phase value?)
//      fprintf(fp,"\n; Set phase");
//	   if(SelectPhase(par->itfc ,fp, phase,channel) == ERR) return(ERR);
//
//	// Update channel frequency
//		if(nrArgs == 5)
//		{
//		   fprintf(fp,"\n; Set channel frequency");
//		   if(SelectFrequency(par->itfc ,fp, freq, channel) == ERR) return(ERR);
//		}
//
//	// Wait for parameters to update (delay PGO)
//	//	fprintf(fp,"\n        nop"); 
//		fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
//
//	// Start RF pulse
//		fprintf(fp,"\n; Start pulse");
//		if(SelectDuration(par->itfc ,fp, "r3", duration) == ERR) return(ERR);
//
//		fprintf(fp,"\n        movep   r3,x:A_TCPR2");
//
//      fprintf(fp,"\n       nop");
//      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
//		if(channel == "1")
//			fprintf(fp,"\n        or      #$000008,a              ; Channel 1 RF on");
//      else if(channel == "2")
//			fprintf(fp,"\n        or      #$000002,a              ; Channel 2 RF on");
//      else if(channel == "3")
//			fprintf(fp,"\n        or      #$000002,a              ; Channel 3 RF on");
//
//		fprintf(fp,"\n        move    a1,y:TTL                ; Load the current TTL word");
//		fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
//
//		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Start timer"); 
//		fprintf(fp,"\n        nop");
//    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
//
//	// End RF pulse
//		fprintf(fp,"\n; End pulse");
//    	fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
//
// 
//      if(channel == "1")
//         fprintf(fp,"\n        and     #$FFBFF7,a              ; Switch off channel 1");
//      else if(channel == "2")
//         fprintf(fp,"\n        and     #$FFFCFD,a              ; Switch off channel 2");
//      else if(channel == "3")
//         fprintf(fp,"\n        and     #$FFDFFD,a              ; Switch off channel 3");
//
//      fprintf(fp,"\n        move    a1,y:TTL                ; Update TTL word");
//	   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
//
//      fprintf(fp,"\n        move    #$000000,a1"); 
//
//		if(channel == "2" || channel == "3")
//		{
//			fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Zero amplitude");
//		}
//		else
//		{
//			fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ; Zero amplitude");
//		}
//		fclose(fp);
//	}
//   else
//   {
//		fclose(fp);
//      Error(par->itfc,"unknown RF pulse mode");
//      return(ERR);
//   }
//
//   return(OK);
//}


//
//short SingleChannelRFPulse(DLLParameters* par,char *args)
//{
//   short nrArgs;
//   CText channel,amp,freq,phase,duration;
//   long Frequency;
//   short ph;
//   char ch;
//
//   if((nrArgs = ArgScan(par->itfc,args,4,"channel,amp,phase,duration,[freq]","eeeee","qqqqq",&channel,&amp,&phase,&duration,&freq)) < 0)
//      return(nrArgs);
//
//   if(!parList)
//   {
//      Error(par->itfc,"Pulse sequence not initialised");
//      return(ERR);
//   }
//   if(channel[0] == 'n')
//      InsertUniqueStringIntoList(channel.Str(),&parList,szList);
//   if(amp[0] == 'a')
//      InsertUniqueStringIntoList(amp.Str(),&parList,szList);
//   if(freq[0] == 'f')
//      InsertUniqueStringIntoList(freq.Str(),&parList,szList);
//   if(phase[0] == 'p' || phase[0] == 't')
//      InsertUniqueStringIntoList(phase.Str(),&parList,szList);
//   if(duration[0] == 'd')
//      InsertUniqueStringIntoList(duration.Str(),&parList,szList);
//
//   FILE *fp = fopen("midCode.asm","a");
//   if(!fp)
//   {
//      Error(par->itfc,"Can't open output file");
//      return(ERR);
//   }
//
//	fprintf(fp,"\n\n;");
//	fprintf(fp,"\n;***************************************************************************");
//	fprintf(fp,"\n; Generate an RF pulse");
//	fprintf(fp,"\n;"); 
//
//	// Update channel 
//	fprintf(fp,"\n; Set channel");
//	if(SelectNumber(par->itfc,fp,channel,"a1",1,3)) return(ERR);
//   fprintf(fp,"\n        move    a1,y:CHANNEL            ; Save the channel");
//
//// Get the user defined delay
//   fprintf(fp,"\n        clr     a");
//	if(SelectDuration(par->itfc ,fp, "a1", duration) == ERR) return(ERR);
//
//// Test for invalid delay (< 0.5 us)
//   fprintf(fp,"\n; Check for invalid pulse length");
//   fprintf(fp,"\n        move    #1,b1                   ; Abort code");
//   fprintf(fp,"\n        cmp     #49999,a                ; Check long pulses >= 1 ms");
//   fprintf(fp,"\n        jgt     LBL%ld                    ; Ignore long pulses",label);
//   fprintf(fp,"\n        move    #2,b1                   ; Abort code");
//   fprintf(fp,"\n        cmp     #24,a                   ; Pulse must be >= 500 ns");
//   fprintf(fp,"\n        jlt     ABORT");
//
//// Update channel TTL gate
//   fprintf(fp,"\n; Unblank the RF amp");
//   fprintf(fp,"\nLBL%ld    move    y:TTL,a1                ; Load the current TTL level",label++);
//   fprintf(fp,"\n        move    y:CHANNEL,b1            ; Select channel");
//   fprintf(fp,"\n        cmp     #1,b                    ; Channel == 1?");
//   fprintf(fp,"\n        jgt     LBL%ld                    ; > 1",label);
//   fprintf(fp,"\n        or      #$004000,a              ; Channel 1");
//	fprintf(fp,"\n        jmp     LBL%ld                   ",label+2);
//
//   fprintf(fp,"\nLBL%ld    cmp     #2,b                    ; Channel == 2?",label++);
//   fprintf(fp,"\n        jgt     LBL%ld                    ; > 2",label);
//	fprintf(fp,"\n        or      #$000100,a              ; Channel 2");
//	fprintf(fp,"\n        jmp     LBL%ld                   ",label+1);
//
//	fprintf(fp,"\nLBL%ld    or      #$002000,a              ; Channel 3",label++);
//
//	fprintf(fp,"\nLBL%ld    move    a1,y:TTL                ; Save new TTL word",label++);
//	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");
//
//   fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
//   fprintf(fp,"\n        move    x:PGO,a1");
//   fprintf(fp,"\n        add     #3,a");
//   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
//	fprintf(fp,"\n        nop");
//	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");   
//      
//// Update channel amplitude
//	fprintf(fp,"\n; Set channel amplitude");
//	if(SelectAmplitude(par->itfc ,fp, amp, channel) == ERR) return(ERR);
//
//// Update channel phase (allow more general phase value?)
//   fprintf(fp,"\n; Set phase");
//	if(SelectPhase(par->itfc ,fp, phase,channel) == ERR) return(ERR);
//
//// Update channel frequency
//	if(nrArgs == 5)
//	{
//		fprintf(fp,"\n; Set channel frequency");
//		if(SelectFrequency(par->itfc ,fp, freq, channel) == ERR) return(ERR);
//	}
//
//// Wait for parameters to update (delay PGO)
////	fprintf(fp,"\n        nop"); 
//	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
//
//// Start RF pulse
//	fprintf(fp,"\n; Start pulse");
//	if(SelectDuration(par->itfc ,fp, "r3", duration) == ERR) return(ERR);
//
//	fprintf(fp,"\n        movep   r3,x:A_TCPR2");
//
//   fprintf(fp,"\n       nop");
//   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
//
//   fprintf(fp,"\n        move    y:CHANNEL,b1            ; Select channel");
//   fprintf(fp,"\n        cmp     #1,b                    ; Channel == 1?");
//   fprintf(fp,"\n        jgt     LBL%ld                    ; > 1",label);
//   fprintf(fp,"\n        or      #$000008,a              ; Channel 1 on");
//   fprintf(fp,"\n        jmp     LBL%ld                   ",label+2);
//
//   fprintf(fp,"\nLBL%ld    cmp     #2,b                    ; Channel == 2?",label++);
//   fprintf(fp,"\n        jgt     LBL%ld                    ; > 2",label);
//	fprintf(fp,"\n        or      #$000002,a              ; Channel 2 on");
//	fprintf(fp,"\n        jmp     LBL%ld                  ",label+1);
//	fprintf(fp,"\nLBL%ld    or      #$000002,a              ; Channel 3 on",label++);
//
//	fprintf(fp,"\nLBL%ld    move    a1,y:TTL                ; Load the current TTL word",label++);
//	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
//
//	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Start timer"); 
//	fprintf(fp,"\n        nop");
//   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
//
//// End RF pulse
//	fprintf(fp,"\n; End pulse");
//   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
// 
//   fprintf(fp,"\n        move    y:CHANNEL,b1            ; Select channel");
//   fprintf(fp,"\n        cmp     #1,b                    ; Channel == 1?");
//   fprintf(fp,"\n        jgt     LBL%ld                    ; > 1",label);
//   fprintf(fp,"\n        and     #$FFBFF7,a              ; Switch off channel 1");
//	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
//   fprintf(fp,"\n        move    #$000000,a1"); 
//   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ; Zero amplitude channel 1");
//   fprintf(fp,"\n        jmp     LBL%ld                   ",label+2);
//
//   fprintf(fp,"\nLBL%ld     cmp    #2,b                    ; Channel == 2?",label++);
//   fprintf(fp,"\n        jgt     LBL%ld                    ; > 2",label);
//	fprintf(fp,"\n        and     #$FFFCFD,a              ; Switch off channel 2");
//	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
//   fprintf(fp,"\n        move    #$000000,a1"); 
//   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Zero amplitude channel 1");
//   fprintf(fp,"\n        jmp     LBL%ld                  ",label+1);
//	fprintf(fp,"\nLBL%ld    and     #$FFDFFD,a              ; Switch off channel 3",label++);
//	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
//   fprintf(fp,"\n        move    #$000000,a1"); 
//   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Zero amplitude channel 1");
//	fprintf(fp,"\nLBL%ld   nop                             ",label++);
//
//	fclose(fp);
//
//
//   return(OK);
//}
//






/**********************************************************************
     Generate a shaped rf pulse (24/5/11)

     shapedrf(mode, amplitude, amplitudeTable, phaseTable, phase, tableSize, tableStep)

     mode ............. internal or external RF pulse (i,e/1,2)
     amplitudeTable ... table of amplitudes (0 -> 2^14-1)
     phaseTable ....... table of phases (0 -> 2^16-1)
     phase ............ a phase to add to the phase table (0 -> 2^16-1)
     tableSize ........ number of value in the table
     tableStep ........ duration of each table value (in us)

     There are three operating modes:

     internal(i)  ... gate pulse is sent to the internal Kea RF amplifier
                      RF to channel 1.
     external (e/1) . gate pulse is sent to the external TTL port (pin 5)
                      RF to channel 1.
     second ch (2) .. gate pulse is sent to the external TTL port (pin 4)
                      RF to channel 2.

**********************************************************************/



short ShapedRF(DLLParameters* par, char *args)
{
   short nrArgs;
   CText channel,dur,atable,ptable,size,phase,duration;
   long Duration,Phase,Size;
   float fDuration;
   CArg carg;
   const int pgoOffset =  5;
   const int firstDelay = 198;
   const int midDelay =   159;
   const int endDelay =   162;

   if((nrArgs = ArgScan(par->itfc,args,6,"mode, atable, stable, phase, table_size, table_step","eeeeee","qqqqqq",&channel,&atable,&ptable,&phase,&size,&duration)) < 0)
      return(nrArgs);
   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(atable.Str(),&parList,szList);
   InsertUniqueStringIntoList(ptable.Str(),&parList,szList);
   if(phase[0] == 'p' || phase[0] == 'n')
      InsertUniqueStringIntoList(phase.Str(),&parList,szList);
   if(size[0] == 'n')
      InsertUniqueStringIntoList(size.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   if(channel == "1") // Internal Kea pulse channel 1
   {
	   fprintf(fp,"\n\n;");
	   fprintf(fp,"\n;***************************************************************************");
	   fprintf(fp,"\n; Generate a modulated internal pulse (ch 1)");
	   fprintf(fp,"\n;"); 

      fprintf(fp,"\n        clr a                           ; Clear the accumulator");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");
      fprintf(fp,"\n        move    #$04000,a1              ; Switch on rf bias (internal ch1)");
	   fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL"); 

      fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
      fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); 
      fprintf(fp,"\n        add     #%d,a",pgoOffset);
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
      fprintf(fp,"\n        nop");
	   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

     // Get the table addresses
      fprintf(fp,"\n; Get the amplitude and phase table pointers");
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
      fprintf(fp,"\n        move    x:TABLE%s,r4",ptable.Str()+1);

     // Get the size of the tables
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 2)
         {
            ErrorMessage("invalid table size '%s' [>= 2]",size);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }
	   fprintf(fp,"\n        move    r2,a0"); 
	   fprintf(fp,"\n        dec     a                        ; Decrement because first value already used"); 
	   fprintf(fp,"\n        move    a0,r2"); 

      fprintf(fp,"\n; Set the rf output to its initial value");
      fprintf(fp,"\n        move    y:(r5)+,a1               ; Load amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
      fprintf(fp,"\n        move    y:(r4)+,a1               ; Load phase"); 
   // Add phase shift for phase cycling
      if(phase[0] == 'p')
         fprintf(fp,"\n        move    x:TXP%s,y1",phase.Str()+1);
      else if(phase[0] == 'n')
         fprintf(fp,"\n        move    x:NR%s,y1",phase.Str()+1);
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
         fprintf(fp,"\n        move    #%ld,y1",Phase);
      else
      {
         ErrorMessage("invalid phase value '%s'",phase.Str());
         fclose(fp);
         return(ERR);
      }

      fprintf(fp,"\n        add     y1,a                    ; Add phase to table"); 
     
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");

      fprintf(fp,"\n; Wait for pgo delay to end");
 	   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n; Start modulated pulse");
      fprintf(fp,"\n        move    #$04008,a1              ; Switch on rf (internal ch1)");
	   fprintf(fp,"\n        or      x1,a1                   ; and combine with internal ttl bias line");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL"); 
	   fprintf(fp,"\n        move    a,b"); 

   // Load step length
      fprintf(fp,"\n; Load step length");
      if(duration[0] == 'd')
      {
	      fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
      }
      else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
      {
         if(fDuration < 2 || fDuration > 327670)
         {
            ErrorMessage("invalid duration '%g' [2...327670]",fDuration);
            fclose(fp);
            return(ERR);
         }
         Duration = (long)(fDuration * 50 - 1 + 0.5);
         fprintf(fp,"\n        move   #%ld,a1",Duration);
      }
      else
      {
         ErrorMessage("Invalid duration reference '%s'",duration.Str());
         fclose(fp);
         return(ERR);
      }
    // Delay to get length of first pulse step correct
      fprintf(fp,"\n        lsl     #1,a");
      fprintf(fp,"\n        sub     #%d,a",firstDelay);
      fprintf(fp,"\n        move    a1,a0");

      fprintf(fp,"\n; Delay for correct first step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

   // Calculate delay for subsequent steps
      fprintf(fp,"\n; Calculate subsequent step length delay");
      fprintf(fp,"\n        add     #%d,a",midDelay);
      fprintf(fp,"\n        move    a1,a0");

   // Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      fprintf(fp,"\n        move    y:(r5)+,a1               ; Load amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set amplitude");   

      fprintf(fp,"\n        move    y:(r4)+,a1               ; Load phase"); 

   // Add phase shift for phase cycling
      if(phase[0] == 'p')
         fprintf(fp,"\n        move    x:TXP%s,y1",phase.Str()+1);
      else if(phase[0] == 'n')
         fprintf(fp,"\n        move    x:NR%s,y1",phase.Str()+1);
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
         fprintf(fp,"\n        move    #%ld,y1",Phase);
      else
      {
         ErrorMessage("invalid phase value '%s'",phase.Str());
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        add     y1,a                    ; Add phase to table"); 

      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set phase");  

      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set correct frequency");
      fprintf(fp,"\n        move    x:TXF01,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");

      fprintf(fp,"\n; Adjust for correct step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fprintf(fp,"\n; End Delay (correct for last pulse)");
      fprintf(fp,"\n        rep     #%d",endDelay);
      fprintf(fp,"\n        nop");

	   fprintf(fp,"\n; End pulse");
	   fprintf(fp,"\n        move    #$000000,a1");
	   fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL");

      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
		fprintf(fp,"\n        move    #$000000,a1             ; Zero phase"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ; Set correct frequency");
      fprintf(fp,"\n        move    x:TXF01,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");

	   fclose(fp);
   }

 
   else if(channel == "2") // External Kea channel 2 pulse
   {
	   fprintf(fp,"\n\n;");
	   fprintf(fp,"\n;***************************************************************************");

	   fprintf(fp,"\n; Generate a modulated external pulse (ch 2)");
	   fprintf(fp,"\n;"); 

      fprintf(fp,"\n        clr a                           ; Clear the accumulator");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");
      fprintf(fp,"\n        move    #$00300,a1              ; Switch on rf bias (ch2)");
	   fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL"); 

      fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
      fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); 
      fprintf(fp,"\n        add     #%d,a",pgoOffset);
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
      fprintf(fp,"\n        nop");
	   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

     // Get the table addresses
      fprintf(fp,"\n; Get the amplitude and phase table pointers");
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
      fprintf(fp,"\n        move    x:TABLE%s,r4",ptable.Str()+1);

     // Get the size of the tables
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 2)
         {
            ErrorMessage("invalid table size '%s' [>= 2]",size);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }
	   fprintf(fp,"\n        move    r2,a0"); 
	   fprintf(fp,"\n        dec     a                        ; Decrement because first value already used"); 
	   fprintf(fp,"\n        move    a0,r2"); 

      fprintf(fp,"\n; Set the rf output to its initial value");
      fprintf(fp,"\n        move    y:(r5)+,a1               ; Load amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
      fprintf(fp,"\n        move    y:(r4)+,a1               ; Load phase"); 
   // Add phase shift for phase cycling
      if(phase[0] == 'p')
         fprintf(fp,"\n        move    x:TXP%s,y1",phase.Str()+1);
      else if(phase[0] == 'n')
         fprintf(fp,"\n        move    x:NR%s,y1",phase.Str()+1);
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
         fprintf(fp,"\n        move    #%ld,y1",Phase);
      else
      {
         ErrorMessage("invalid phase value '%s'",phase.Str());
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        add     y1,a                    ; Add phase to table"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");

      fprintf(fp,"\n; Wait for pgo delay to end");
 	   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n; Start modulated pulse");
      fprintf(fp,"\n        move    #$00302,a1              ; Switch on rf (external ch2)");
	   fprintf(fp,"\n        or      x1,a1                   ; and combine with internal ttl bias line");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL"); 
	   fprintf(fp,"\n        move    a,b"); 


   // Load step length
      fprintf(fp,"\n; Load step length");
      if(duration[0] == 'd')
      {
	      fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
      }
      else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
      {
         if(fDuration < 2 || fDuration > 327670)
         {
            ErrorMessage("invalid duration '%g' [2...327670]",fDuration);
            fclose(fp);
            return(ERR);
         }
         Duration = (long)(fDuration * 50 - 1 + 0.5);
         fprintf(fp,"\n        move   #%ld,a1",Duration);
      }
      else
      {
         ErrorMessage("Invalid duration reference '%s'",duration.Str());
         fclose(fp);
         return(ERR);
      }
    // Delay to get length of first pulse step correct
      fprintf(fp,"\n        lsl     #1,a");
      fprintf(fp,"\n        sub     #%d,a",firstDelay);
      fprintf(fp,"\n        move    a1,a0");

      fprintf(fp,"\n; Delay for correct first step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

   // Calculate delay for subsequent steps
      fprintf(fp,"\n; Calculate subsequent step length delay");
      fprintf(fp,"\n        add     #%d,a",midDelay);
      fprintf(fp,"\n        move    a1,a0");

   // Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      fprintf(fp,"\n        move    y:(r5)+,a1               ; Load amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set amplitude");   

      fprintf(fp,"\n        move    y:(r4)+,a1               ; Load phase"); 

   // Add phase shift for phase cycling
      if(phase[0] == 'p')
         fprintf(fp,"\n        move    x:TXP%s,y1",phase.Str()+1);
      else if(phase[0] == 'n')
         fprintf(fp,"\n        move    x:NR%s,y1",phase.Str()+1);
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
         fprintf(fp,"\n        move    #%ld,y1",Phase);
      else
      {
         ErrorMessage("invalid phase value '%s'",phase.Str());
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        add     y1,a                    ; Add phase to table"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set phase");  

      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set correct frequency");
      fprintf(fp,"\n        move    x:TXF01,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");

      fprintf(fp,"\n; Adjust for correct step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fprintf(fp,"\n; End Delay (correct for last pulse)");
      fprintf(fp,"\n        rep     #%d",endDelay);
      fprintf(fp,"\n        nop");

	   fprintf(fp,"\n; End pulse");
	   fprintf(fp,"\n        move    #$000000,a1");
	   fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL");

      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Zero phase");
      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set correct frequency");
      fprintf(fp,"\n        move    x:TXF01,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");

	   fclose(fp);
   }
   else
   {
		fclose(fp);
      ErrorMessage("unknown RF pulse mode");
      return(ERR);
   }

   return(OK);
}


/**********************************************************************
     Switch on the transceiver transmitter output (4/11/07)

     txon(mode, amplitude, phase)

     There are several operating modes:

     internal ... gate pulse is sent to the internal Kea RF amplifier
     external ... gate pulse is sent to the external TTL port (pin 5)
     none ....... no gate pulse is generated (phase & amp optional)

     Note that there is a delay (pgo) before the rf appears. This allows
     the pulse phase and amplitude to be set and gives time for the 
     HPA biasing to be switched on.

**********************************************************************/

short SwitchOnTx(DLLParameters* par, char *args)
{
   short nrArgs;
   CArg carg;

   nrArgs = carg.Count(args);

   if(nrArgs <= 4)
   {
      return(SwitchOnOneTxChannel(par,args,nrArgs));
   }

   if(nrArgs == 8)
   {
      return(SwitchOnBothTxChannels(par,args));
   }
	else
	{
		Error(par->itfc,"invalid number of arguments");
		return(ERR);
	}

   return(OK);

}

/**********************************************************************
     Switch on the transceiver transmitter outputs

     txon(ch1, amp1, phase1, freq1, ch2, amp2, phase2, freq2)

     There are several operating modes:

     internal ... gate pulse is sent to the internal Kea RF amplifier
     external ... gate pulse is sent to the external TTL port (pin 5)
     none ....... no gate pulse is generated (phase & amp optional)

     Note that there is a delay (pgo) before the rf appears. This allows
     the pulse phase and amplitude to be set and gives time for the 
     HPA biasing to be switched on.

**********************************************************************/

short SwitchOnBothTxChannels(DLLParameters* par, char *args)
{
   short nrArgs;
   CText ch1,amp1,freq1,phase1;
   CText ch2,amp2,freq2,phase2;
   long Frequency1,Frequency2;
   short ph;
   CText channel;

  if((nrArgs = ArgScan(par->itfc,args,8,"ch1, amp1, phase1, freq1, ch2, amp2, phase2, freq2","eeeeeeee","qqqqqqqq",&ch1,&amp1,&phase1,&freq1,&ch2,&amp2,&phase2,&freq2)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(amp1[0] == 'a')
      InsertUniqueStringIntoList(amp1.Str(),&parList,szList);
   if(freq1[0] == 'f')
      InsertUniqueStringIntoList(freq1.Str(),&parList,szList);
   if(phase1[0] == 'p' || phase1[0] == 't' )
      InsertUniqueStringIntoList(phase1.Str(),&parList,szList);
   if(amp2[0] == 'a')
      InsertUniqueStringIntoList(amp2.Str(),&parList,szList);
   if(freq2[0] == 'f')
      InsertUniqueStringIntoList(freq2.Str(),&parList,szList);
   if(phase2[0] == 'p' || phase1[0] == 't' )
      InsertUniqueStringIntoList(phase2.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Switch on both transmitter channels");
   fprintf(fp,"\n;"); 

   fprintf(fp,"\n; Unblank the RF amplifiers");

   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");

   fprintf(fp,"\n        or      #$04100,a               ; Proton and Carbon");
//  fprintf(fp,"\n        or      #$04300,a               ; Proton and Carbon");

   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");

   fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
   fprintf(fp,"\n        move    x:PGO,a1");
   fprintf(fp,"\n        add     #5,a                   ; Tweak it"); //
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        nop");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   
// Set channel i/1 amplitude
	fprintf(fp,"\n; Set channel 1 amplitude");
   channel = "1";
	if(SelectAmplitude(par->itfc ,fp, amp1, channel) == ERR) return(ERR);

// Set channel i/1 phase
   fprintf(fp,"\n; Set channel 1 phase");
	if(SelectPhase(par->itfc ,fp, phase1, channel) == ERR) return(ERR);

// Set channel 1 frequency
   fprintf(fp,"\n; Set channel 1 frequency");
	if(SelectFrequency(par->itfc ,fp, freq1, channel) == ERR) return(ERR);

// Set channel 1 update
   fprintf(fp,"\n; Wait 2.0 us for channel 1 to update");
   fprintf(fp,"\n        move    #20,r7");
	fprintf(fp,"\n        bsr     svwait");

// Set channel 2 amplitude
	fprintf(fp,"\n; Set channel 2 amplitude");
   channel = "2";
	if(SelectAmplitude(par->itfc ,fp, amp2, channel) == ERR) return(ERR);

// Set channel 2 phase
   fprintf(fp,"\n; Set channel 2 phase");
	if(SelectPhase(par->itfc ,fp, phase2, channel) == ERR) return(ERR);

// Set channel 2 frequency
   fprintf(fp,"\n; Set channel 2 frequency");
	if(SelectFrequency(par->itfc ,fp, freq2, channel) == ERR) return(ERR);

// Wait for parameters to update
	fprintf(fp,"\n; Wait for parameters to update");
//   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for timer");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

// Start pulse
	fprintf(fp,"\n; Start pulse");
   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
   fprintf(fp,"\n        or      #$0000A,a              ; Channel 1 & 2 RF on");
   fprintf(fp,"\n        move    a1,y:TTL                ; Load the current TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
	fprintf(fp,"\n        nop");

	fclose(fp);

   return(OK);
}


/**********************************************************************

     Switch on one Tx channel + corresponding TTL line(s)

     txon("1/2", ...) or
     txon("w1/w2", ...) (for wobble mode)

	  other options

	  txon(mode, amplitude, phase, [frequency])

     "1" external TTL (TTL 0x4008) Ch 1 RF (HPA Gate + TRex Gate)
     "2" external TTL (TTL 0x0102) Ch 2 RF (HPA Gate+ TRex Gate)
     "w1" external TTL (TTL 0x5008) Ch 1 RF (Gates + Wobble)
     "w2" external TTL (TTL 0x0502) Ch 2 RF (Gates + Wobble)

**********************************************************************/


short SwitchOnOneTxChannel(DLLParameters* par, char *args, short nrArgs)
{
   CText channel,amp,freq,phase;
   short ph;
   char ch;

	if((nrArgs = ArgScan(par->itfc,args,3,"channel,amp,phase[,freq]","eeee","qqqq",&channel,&amp,&phase,&freq)) < 0)
		return(nrArgs);
	
	if(amp[0] == 'a')
		InsertUniqueStringIntoList(amp.Str(),&parList,szList);
	if(nrArgs == 4 && freq[0] == 'f')
		InsertUniqueStringIntoList(freq.Str(),&parList,szList);
	if(phase[0] == 'p' || phase[0] == 't')
		InsertUniqueStringIntoList(phase.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Switch on one transmitter channel");
   fprintf(fp,"\n;"); 


   fprintf(fp,"\n; Unblank the RF amp");

   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");

   if(channel == "1")
      fprintf(fp,"\n        or      #$04000,a              ; Channel-1 HPA Gate");
   else if(channel == "w1")
      fprintf(fp,"\n        or      #$05000,a              ; Channel-1 HPA Gate + Wobble mode");
   else if(channel == "2")
      fprintf(fp,"\n        or      #$00100,a              ; Channel-2 HPA Gate");
   else if(channel == "w2")
      fprintf(fp,"\n        or      #$00500,a              ; Channel-2 HPA Gate + Wobble mode");
   else if(channel == "1nb" | channel == "2nb")
      fprintf(fp,"\n        or      #$00000,a              ; No TTL line");

   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");

   fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to pgo before pulse comes on");
   fprintf(fp,"\n        add     #4,a"); // Tweak it
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        nop");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   
	fprintf(fp,"\n; Set channel amplitude");
   if(SelectAmplitude(par->itfc ,fp, amp, channel) == ERR) return(ERR);
	fprintf(fp,"\n        move    a1,y:TX_AMP");

   fprintf(fp,"\n; Set channel phase");
   if(SelectPhase(par->itfc ,fp, phase, channel) == ERR) return(ERR);
 
   if(nrArgs == 4)
   {
      fprintf(fp,"\n; Set channel frequency");
	   if(SelectFrequency(par->itfc ,fp, freq, channel) == ERR) return(ERR);
   }
   else
   {
      if(channel == "1" || channel == "w1" || channel == "1nb")
      {
		   fprintf(fp,"\n        move    x:TXF00,a1");
		   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set last saved frequency");
		   fprintf(fp,"\n        move    x:TXF01,a1");
		   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
      }
      else if(channel == "2" || channel == "w2" || channel == "2nb")
      {
		   fprintf(fp,"\n        move    x:TXF00,a1");
		   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set last saved frequency");
		   fprintf(fp,"\n        move    x:TXF01,a1");
		   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
      }
   }
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

	fprintf(fp,"\n; Start pulse");

   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");

   if(channel == "2" || channel == "w2" || channel == "2nb")
      fprintf(fp,"\n        or      #$00002,a              ; Channel 2 TRex RF on");
   else
      fprintf(fp,"\n        or      #$00008,a              ; Channel 1 TRex RF on");

   fprintf(fp,"\n        move    a1,y:TTL                ; Load the current TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
	fprintf(fp,"\n        nop");

	fclose(fp);


   return(OK);
}

/**********************************************************************

     Switch on one Tx channel + corresponding TTL line(s)

     txon("1/2", ...) or
     txon("w1/w2", ...) (for wobble mode)

	  other options

	  txon(mode, amplitude, phase, [frequency])

     "1" external TTL (TTL 0x4008) Ch 1 RF (HPA Gate + TRex Gate)
     "2" external TTL (TTL 0x0102) Ch 2 RF (HPA Gate+ TRex Gate)
     "w1" external TTL (TTL 0x5008) Ch 1 RF (Gates + Wobble)
     "w2" external TTL (TTL 0x0502) Ch 2 RF (Gates + Wobble)

**********************************************************************/
//
//short SwitchOnOneTxChannel(DLLParameters* par, char *args, short nrArgs)
//{
//   CText channel,amp,freq,phase;
//   short ph;
//   char ch;
//
//	if((nrArgs = ArgScan(par->itfc,args,3,"channel,amp,phase[,freq]","eeee","qqqq",&channel,&amp,&phase,&freq)) < 0)
//		return(nrArgs);
//	
//	if(amp[0] == 'a')
//		InsertUniqueStringIntoList(amp.Str(),&parList,szList);
//	if(nrArgs == 4 && freq[0] == 'f')
//		InsertUniqueStringIntoList(freq.Str(),&parList,szList);
//	if(phase[0] == 'p' || phase[0] == 't')
//		InsertUniqueStringIntoList(phase.Str(),&parList,szList);
//
//   FILE *fp = fopen("midCode.asm","a");
//   if(!fp)
//   {
//      Error(par->itfc,"Can't open output file");
//      return(ERR);
//   }
//
//   fprintf(fp,"\n\n;");
//   fprintf(fp,"\n;***************************************************************************");
//   fprintf(fp,"\n; Switch on one transmitter channel");
//   fprintf(fp,"\n;"); 
//
//
//   fprintf(fp,"\n; Unblank the RF amp");
//
//   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
//
//   if(channel == "1")
//      fprintf(fp,"\n        or      #$04000,a               ; Proton");
//   else if(channel == "w1")
//      fprintf(fp,"\n        or      #$05000,a               ; Proton + Duplexer relay");
//   else if(channel == "2")
//      fprintf(fp,"\n        or      #$00100,a               ; X1");
//   //   fprintf(fp,"\n        or      #$00300,a               ; X1");
//   else if(channel == "w2")
//      fprintf(fp,"\n        or      #$00500,a               ; X1 + Duplexer relay");
//    //  fprintf(fp,"\n        or      #$00700,a               ; X1 + Duplexer relay");
//   else if(channel == "3")
//      fprintf(fp,"\n        or      #$02000,a               ; X2");
//   else if(channel == "w3")
//      fprintf(fp,"\n        or      #$02800,a               ; X2 + Duplexer relay");
//   else if(channel == "1nb" || channel == "2nb" || channel == "n3")
//      fprintf(fp,"\n        or      #$00000,a               ; No TTL line");
//
//   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
//	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");
//
//   fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to pgo before pulse comes on");
//   fprintf(fp,"\n        add     #4,a"); // Tweak it
//   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
//	fprintf(fp,"\n        nop");
//	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
//   
//	fprintf(fp,"\n; Set channel amplitude");
//   if(SelectAmplitude(par->itfc ,fp, amp, channel) == ERR) return(ERR);
//	fprintf(fp,"\n        move    a1,y:TX_AMP");
//
//   fprintf(fp,"\n; Set channel phase");
//   if(SelectPhase(par->itfc ,fp, phase, channel) == ERR) return(ERR);
// 
//   if(nrArgs == 4)
//   {
//      fprintf(fp,"\n; Set channel frequency");
//	   if(SelectFrequency(par->itfc ,fp, freq, channel) == ERR) return(ERR);
//   }
//   else
//   {
//      if(channel == "1" || channel == "w1" || channel == "1nb")
//      {
//		   fprintf(fp,"\n        move    x:TXF00,a1");
//		   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ; Set last saved frequency");
//		   fprintf(fp,"\n        move    x:TXF01,a1");
//		   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
//      }
//      else if(channel == "2" || channel == "w2" || channel == "2nb")
//      {
//		   fprintf(fp,"\n        move    x:TXF00,a1");
//		   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Set last saved frequency");
//		   fprintf(fp,"\n        move    x:TXF01,a1");
//		   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
//      }
//      else if(channel == "3" || channel == "w3" || channel == "n3")
//      {
//		   fprintf(fp,"\n        move    x:TXF00,a1");
//		   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Set last saved frequency");
//		   fprintf(fp,"\n        move    x:TXF01,a1");
//		   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
//      }
//   }
//   fprintf(fp,"\n        nop");
//   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
//   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
//
//	fprintf(fp,"\n; Start pulse");
//
//   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
//
//
//   if(channel == "1" || channel == "w1" || channel == "1nb")
//      fprintf(fp,"\n        or      #$00008,a               ; Channel 1 RF on");
//   else if(channel == "2" || channel == "w2" || channel == "2nb")
//      fprintf(fp,"\n        or      #$00002,a               ; Channel 2 RF on");
//   else if(channel == "3" || channel == "w3" || channel == "n3")
//      fprintf(fp,"\n        or      #$00002,a               ; Channel 3 RF on");
//
//   fprintf(fp,"\n        move    a1,y:TTL                ; Load the current TTL word");
//	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
//	fprintf(fp,"\n        nop");
//
//	fclose(fp);
//
//
//   return(OK);
//}


/**********************************************************************

     Switch off the transceiver transmitter output

     txoff(["w"])     # All channels off
     txoff("1/2")     # Channel 1/2 off

     Note: keeps the TTL and other RF channels status intact. 

     This command takes xxx ns before the RF turns off

          Switch on one Tx channel + corresponding TTL line(s)

     "1" external TTL (TTL 0x4008) Ch 1 RF (HPA Gate + TRex Gate)
     "2" external TTL (TTL 0x0102) Ch 2 RF (HPA Gate+ TRex Gate)
     "w1" external TTL (TTL 0x5008) Ch 1 RF (Gates + Wobble)
     "w2" external TTL (TTL 0x0502) Ch 2 RF (Gates + Wobble)

**********************************************************************/


short SwitchOffTx(DLLParameters* par, char *args)
{
   short nrArgs;
   CText mode;

   
   if((nrArgs = ArgScan(par->itfc,args,0,"[mode]","e","q",&mode)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

	if(nrArgs == 0 || mode == "w") // Switch off both channels
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch off transmitter");
      fprintf(fp,"\n;"); 
      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
      fprintf(fp,"\n        and     #$FFBEF5,a              ; Switch off all RF TTL lines");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL state");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update system");


      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude (add delay for bus write"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Zero phase");

      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set correct frequency");

      fprintf(fp,"\n        move    x:TXF01,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");

      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
      fprintf(fp,"\n        move    #$000000,a1             ; (add delay for bus write"); 

      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Zero phase");
      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set correct frequency");
      fprintf(fp,"\n        move    x:TXF01,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");

      fclose(fp);
   }
   else
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch off one TX channel");
      fprintf(fp,"\n;"); 

      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");

      if(mode == "1")
         fprintf(fp,"\n        and     #$FFBFF7,a               ; Switch off channel 1 (Channel 1)");
      else if(mode == "2")
         fprintf(fp,"\n        and     #$FFFEFD,a               ; Switch off channel 2 (Channel 2)");
        // fprintf(fp,"\n        and     #$FFFCF7,a               ; Switch off channel 2 (Carbon)");
		else
		{
			Error(par->itfc,"invalid channel (1/2)");
			fclose(fp);
			return(ERR);
		}

      fprintf(fp,"\n        move    a1,y:TTL                 ; Update TTL word");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");

      if(mode == "2")
      {
         fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
			fprintf(fp,"\n        move    #$000000,a1             ; (add delay for bus write"); 
         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Zero phase");
         fprintf(fp,"\n        move    x:TXF00,a1");
         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Set correct frequency");
         fprintf(fp,"\n        move    x:TXF01,a1");
         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
      }
      else
      {
         fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
         fprintf(fp,"\n        move    #$000000,a1             ; (add delay for bus write"); 
         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ; Zero phase");
         fprintf(fp,"\n        move    x:TXF00,a1");
         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ; Set correct frequency");
         fprintf(fp,"\n        move    x:TXF01,a1");
         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
      }

      fclose(fp);
   }
   return(OK);

}
//
//
//short SwitchOffTx(DLLParameters* par, char *args)
//{
//   short nrArgs;
//   CText mode;
//
//   
//   if((nrArgs = ArgScan(par->itfc,args,0,"[mode]","e","q",&mode)) < 0)
//      return(nrArgs);
//
//   if(!parList)
//   {
//      Error(par->itfc,"Pulse sequence not initialised");
//      return(ERR);
//   }
//
//   FILE *fp = fopen("midCode.asm","a");
//   if(!fp)
//   {
//      Error(par->itfc,"Can't open output file");
//      return(ERR);
//   }
//
//	if(nrArgs == 0 || mode == "w") // Switch off both channels
//   {
//      fprintf(fp,"\n\n;");
//      fprintf(fp,"\n;***************************************************************************");
//      fprintf(fp,"\n; Switch off transmitter");
//      fprintf(fp,"\n;"); 
//      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
//      fprintf(fp,"\n        and     #$FF80F5,a              ; Switch off all RF TTL lines");
//      fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL state");
//      fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update system");
//
//
//      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
//      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
//      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude (add delay for bus write"); 
//      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Zero phase");
//
//      fprintf(fp,"\n        move    x:TXF00,a1");
//      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set correct frequency");
//
//      fprintf(fp,"\n        move    x:TXF01,a1");
//      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
//
//      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
//      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
//      fprintf(fp,"\n        move    #$000000,a1             ; (add delay for bus write"); 
//
//      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Zero phase");
//      fprintf(fp,"\n        move    x:TXF00,a1");
//      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set correct frequency");
//      fprintf(fp,"\n        move    x:TXF01,a1");
//      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
//
//      fclose(fp);
//   }
//   else
//   {
//      fprintf(fp,"\n\n;");
//      fprintf(fp,"\n;***************************************************************************");
//      fprintf(fp,"\n; Switch off one TX channel");
//      fprintf(fp,"\n;"); 
//
//      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
//
//      if(mode == "1")
//         fprintf(fp,"\n        and     #$FFBFF7,a              ; Switch off channel 1 (Proton)");
//      else if(mode == "2")
//         fprintf(fp,"\n        and     #$FFFCFD,a              ; Switch off channel 2 (X1)");
//      else if(mode == "3")
//         fprintf(fp,"\n        and     #$FFDFFD,a              ; Switch off channel 3 (X2)");
//		else
//		{
//			Error(par->itfc,"invalid channel (1/2/3)");
//			fclose(fp);
//			return(ERR);
//		}
//
//      fprintf(fp,"\n        move    a1,y:TTL                ; Update TTL word");
//	   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
//
//      if(mode == "2" | mode == "3")
//      {
//         fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
//         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
//			fprintf(fp,"\n        move    #$000000,a1             ; (Add delay for bus write"); 
//         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Zero phase");
//         fprintf(fp,"\n        move    x:TXF00,a1");
//         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0     ; Set correct frequency");
//         fprintf(fp,"\n        move    x:TXF01,a1");
//         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
//      }
//      else
//      {
//         fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
//         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
//         fprintf(fp,"\n        move    #$000000,a1             ; (add delay for bus write"); 
//         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ; Zero phase");
//         fprintf(fp,"\n        move    x:TXF00,a1");
//         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0     ; Set correct frequency");
//         fprintf(fp,"\n        move    x:TXF01,a1");
//         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
//      }
//
//      fclose(fp);
//   }
//   return(OK);
//
//}


/**********************************************************************
     Generate a TLL pulse 

     Note that the duration of the pulse including setup and shutdown
     will be correct however the pulse itself will be 150 ns shorter
     than expected and will be delayed by this amount.

     ttlpulse(byte, duration)
**********************************************************************/


short TTLPulse(DLLParameters* par, char *args)
{
   short nrArgs;
   CText byte,duration;

   if((nrArgs = ArgScan(par->itfc,args,2,"byte, duration","ee","qq",&byte,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(byte.Str(),&parList,szList);
   InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Pulse TTL level");

   fprintf(fp,"\n        move    x:TTL%s,a1              ; Read in the new TTL byte",byte.Str()+1);
   fprintf(fp,"\n        move    y:TTL,x1                ; Read in the current TTL state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine with current TTL state");
   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL output");

 // fprintf(fp,"\n        move    #10,r7");
	//fprintf(fp,"\n        bsr     svwait");

   fprintf(fp,"\n; Delay");
   fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
   fprintf(fp,"\n        sub     #12,a"); // Tweak delay
   fprintf(fp,"\n        move    a1,r3");
   fprintf(fp,"\n        movep   r3,x:A_TCPR2");
   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

   fprintf(fp,"\n; Reset TTL level");
   fprintf(fp,"\n        move    y:TTL,a1                ; Read in the original TTL state");
   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL output");

   fclose(fp);

   return(OK);

}

/**********************************************************************
     Set the index for table access
**********************************************************************/

short SetTableIndex(DLLParameters* par, char *args)
{
   short nrArgs;
   CText table,index;
   long Index;

// Get the table name and desired index value
   if((nrArgs = ArgScan(par->itfc,args,2,"table,index","ee","qq",&table,&index)) < 0)
      return(nrArgs);

// Add names to parameter list
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(table.Str(),&parList,szList);
   if(index[0] == 'n')
       InsertUniqueStringIntoList(index.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set table index");

  
   if(table[0] == 't')
   {
      if(index[0] == 'n')
      {
         fprintf(fp,"\n        move    x:TABLE%s,a0",table.Str()+1);
         fprintf(fp,"\n        dec a"); // a0 points to table index
         fprintf(fp,"\n        move    a0,r5");
         fprintf(fp,"\n        move    x:NR%s,a0",index.Str()+1);
         fprintf(fp,"\n        move    a0,y:(r5)"); // Update table index
      }
      else if(sscanf(index.Str(),"%ld",&Index) == 1)
      {
         fprintf(fp,"\n        move    x:TABLE%s,a0",table.Str()+1);
         fprintf(fp,"\n        dec a"); // a0 points to table index
         fprintf(fp,"\n        move    a0,r5");
         fprintf(fp,"\n        move    #%ld,a0",Index);
         fprintf(fp,"\n        move    a0,y:(r5)"); // Update table index
      }
      else
      {
         Error(par->itfc,"Invalid index '%s'",index.Str());
         fclose(fp);
         return(ERR);
      }
   }
   else
   {
      Error(par->itfc,"Invalid table reference '%s'",table.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);

   return(OK);
}


/**********************************************************************
     Trigger input control
**********************************************************************/

short WaitForTrigger(DLLParameters* par, char *args)
{
   short nrArgs;
   CText mode = "on high";

// Get the table name and desired index value
   if((nrArgs = ArgScan(par->itfc,args,0,"mode","e","t",&mode)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Wait for trigger input");
   if(mode == "on high")
      fprintf(fp,"\n        jset    #12,x:A_HDR,*         ;Wait for trigger level to go high");
   else
      fprintf(fp,"\n        jclr    #12,x:A_HDR,*         ;Wait for trigger level to go low");
   fprintf(fp,"\n;");

   fclose(fp);

   return(OK);
}

/**********************************************************************
     Increment the index for table access
**********************************************************************/

short IncrementTableIndex(DLLParameters* par, char *args)
{
   short nrArgs;
   CText table;

// Get the table name and desired index value
   if((nrArgs = ArgScan(par->itfc,args,1,"table","e","q",&table)) < 0)
      return(nrArgs);

// Add names to parameter list
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(table.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Increment a table index");

   if(table[0] == 't')
   {
      fprintf(fp,"\n        move    x:TABLE%s,a0",table.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0"); 
      fprintf(fp,"\n        inc a"); // increment the table index
      fprintf(fp,"\n        move    a0,y:(r5)"); // Update table index
   }
   else
   {
      Error(par->itfc,"invalid table reference '%s'",table.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);

   return(OK);
}


/**********************************************************************
     Decrement the index for table access
**********************************************************************/

short DecrementTableIndex(DLLParameters* par, char *args)
{
   short nrArgs;
   CText table;

// Get the table name and desired index value
   if((nrArgs = ArgScan(par->itfc,args,1,"table","e","q",&table)) < 0)
      return(nrArgs);

// Add names to parameter list
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(table.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Increment a table index");

   if(table[0] == 't')
   {
      fprintf(fp,"\n        move    x:TABLE%s,a0",table.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0"); 
      fprintf(fp,"\n        dec a"); // decrement the table index
      fprintf(fp,"\n        move    a0,y:(r5)"); // Update table index
   }
   else
   {
      Error(par->itfc,"invalid table reference '%s'",table.Str());
      fclose(fp);
      return(ERR);
   }
   fclose(fp);

   return(OK);
}




/**********************************************************************
     Generate a short delay 

     delay(duration)

     duration can be a reference e.g. "d1" or d1 
     or a constant in us e.g. 100
     or a table entry
     Delay range is 0.5 ... 327670 us

**********************************************************************/

short MakeADelay(DLLParameters* par, char *args)
{
   short nrArgs;
   CText duration;
   float fDuration;
   long Duration;

   if((nrArgs = ArgScan(par->itfc,args,1,"duration name","e","q",&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(duration[0] == 'd' || duration[0] == 't')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }


   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Delay");

   if(duration[0] == 'd')
   {
      fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
      fprintf(fp,"\n        sub     #9,a"); // Tweak it
      fprintf(fp,"\n        move    a1,r3");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
   }
   else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
   {
      if(fDuration < 0.25 || fDuration > 327670)
      {
         Error(par->itfc,"invalid delay '%f' [0.25 -> 327670]",fDuration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp,"\n        move    #%ld,a1",Duration);
      fprintf(fp,"\n        sub     #9,a"); // Tweak it
      fprintf(fp,"\n        move    a1,r3");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
   }
   else if(duration[0] == 't')
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",duration.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",duration.Str()+1);

      fprintf(fp,"\n        add     b,a");  // Add the index to table start to find current value
      fprintf(fp,"\n        move    a0,r5"); // 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read the table value

      fprintf(fp,"\n        sub     #19,a"); // Tweak it
      fprintf(fp,"\n        move    a1,r3");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        nop");
   }
   else
   {
      Error(par->itfc,"invalid delay or delay reference '%s'",duration.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);

   return(OK);

}  

/**********************************************************************
     Generate a long delay 

     wait(duration)

**********************************************************************/

short MakeALongDelay(DLLParameters* par, char *args)
{
   short nrArgs;
   CText wait;
   long nrSteps;
   long delay1,delay2;
   float fWait;

   if((nrArgs = ArgScan(par->itfc,args,1,"duration name","e","q",&wait)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(wait[0] == 'w')
      InsertUniqueStringIntoList(wait.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   };


   fprintf(fp,"\n\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; General delay");

   if(wait[0] == 'w')
   {
      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        move    x:WAIT%s_0,a1            ; Number of delay steps",wait.Str()+1); 
      fprintf(fp,"\n        move    a1,r1");
      fprintf(fp,"\n        move    x:WAIT%s_1,a1            ; Delay size 1",wait.Str()+1);

      fprintf(fp,"\n        rep     a");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        move    x:WAIT%s_2,a1            ; Delay size 2",wait.Str()+1);
      fprintf(fp,"\n        do      r1,LBL%ld                 ; Repeat delay ",label);
      fprintf(fp,"\n        rep     a");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\nLBL%ld    nop",label++);
   }
   else if(sscanf(wait.Str(),"%f",&fWait) == 1)
   {
      if(fWait < 2 || fWait > 167e6)
      {
         Error(par->itfc,"invalid delay '%ld' [2 us...167 s]",wait);
         fclose(fp);
         return(ERR);
      }

      if(fWait/16777216 < 1) // Can be represented in < 2^24 1us steps?
      {
         float in = (long)fWait; // number of 1us steps
         float fr = fWait - in; // remaining time
         if(in > 1)
         {
            nrSteps = (long)fWait-1; // Number of steps
            delay1 = 103 + (long)(fr*100+0.5) - 19; // Remaining time as 10 ns steps
            delay2 = 95; // Fiddle factor
         }
         else if(fWait > 0.2)
         {
            nrSteps = 0;
            delay1 = (long)(fWait*100+0.5) - 20;
            delay2 = 0;
         }
      }
      else if(fWait/16777216 < 10) // Can be represented in < 2^24 1us steps?
      {
         long in = (long)(fWait/10); // number of 10us steps
         float fr = fWait - in*10; // remaining time
         if(in > 10)
         {
            nrSteps = in-1; // Number of 10 us steps
            delay1 = 1000 + (long)(fr*100+0.5) - 19; // Remaining time as 10 ns steps
            delay2 = 995; // Fiddle factor
         }
         else if(fWait > 0.2)
         {
            nrSteps = 0;
            delay1 = (long)(fWait*1000+0.5) - 19;
            delay2 = 0;
         }
      }
      else
      {
         Error(par->itfc,"invalid delay '%ld' [2 us...167 s]",wait);
         fclose(fp);
         return(ERR);
      }

      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        move    #%ld,a1            ; Number of delay steps",nrSteps); 
      fprintf(fp,"\n        move    a1,r1");
      fprintf(fp,"\n        move    #%ld,a1            ; Delay size 1",delay1);

      fprintf(fp,"\n        rep     a");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        move     #%ld,a1            ; Delay size 2",delay2);
      fprintf(fp,"\n        do      r1,LBL%ld           ; Repeat delay ",label);
      fprintf(fp,"\n        rep     a");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\nLBL%ld    nop",label++);
   }


   fclose(fp);

   return(OK);

}  


/**********************************************************************
     Switch on a TTl level 

     ttlon(byte)

     Takes 150 ns to set up level. The TTL level is combined with the
     lower 16 bits to prevent any disruption to the RF.
**********************************************************************/

short TTLOn(DLLParameters* par, char *args)
{
   short nrArgs;
   CText byte;
   long Byte;

   if((nrArgs = ArgScan(par->itfc,args,1,"byte name","e","q",&byte)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(byte[0] == 'b')
      InsertUniqueStringIntoList(byte.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set TTL level");

   if(byte[0] == 'b')
   {
      fprintf(fp,"\n        move    x:TTL%s,a1            ; Read in the new TTL byte",byte.Str()+1);
      fprintf(fp,"\n        move    y:TTL,x1                ; Read in the current TTL state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with current TTL state");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save total TTL state");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL output");
   }
   else if(sscanf(byte.Str(),"%ld",&Byte) == 1)
   {
      if(Byte < 1 || Byte > 0x80)
      {
         Error(par->itfc,"Invalid TTL value '%ld'",Byte);
         fclose(fp);
         return(ERR);
      }
      Byte = Byte << 8; // Was 16

      fprintf(fp,"\n        move    #$%X,a1               ; Read in the new TTL byte",Byte);
      fprintf(fp,"\n        move    y:TTL,x1                ; Read in the current TTL state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with current TTL state");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save total TTL state");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL output");
   }
   else
   {
      Error(par->itfc,"Invalid TTL reference '%s'",byte.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);
   return(OK);

}  

/**********************************************************************
     Switch off a TTl level 

     ttloff(byte)

     Takes 150 ns to set up level. The TTL level is combined with the
     lower 16 bits to prevent any disruption to the RF.
**********************************************************************/

short TTLOff(DLLParameters* par, char *args)
{
   short nrArgs;
   CText byte;
   long Byte;

   if((nrArgs = ArgScan(par->itfc,args,1,"byte name","e","q",&byte)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(byte[0] == 'b')
      InsertUniqueStringIntoList(byte.Str(),&parList,szList);


   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   };


   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set TTL level");

   if(byte[0] == 'b')
   {
      fprintf(fp,"\n        move    x:TTL%s,a1              ; Read in the TTL byte",byte.Str()+1);
      fprintf(fp,"\n        not     a                       ; Invert the byte");
      fprintf(fp,"\n        move    y:TTL,x1                ; Read the current TTL state");
      fprintf(fp,"\n        and     x1,a1                   ; And with current TTL state to switch off TTL line");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL state");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL output");
   }
   else if(sscanf(byte.Str(),"%ld",&Byte) == 1)
   {
      if(Byte < 1 || Byte > 0x80)
      {
         Error(par->itfc,"Invalid TTL value '%ld'",Byte);
         fclose(fp);
         return(ERR);
      }
      Byte = Byte << 8; // Was 16
      fprintf(fp,"\n        move    #$%X,a1               ; Read in the TTL byte",Byte);
      fprintf(fp,"\n        not     a                       ; Invert the byte");
      fprintf(fp,"\n        move    y:TTL,x1                ; Read the current TTL state");
      fprintf(fp,"\n        and     x1,a1                   ; And with current TTL state to switch off TTL line");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL state");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL output");
   }
   else
   {
      Error(par->itfc,"Invalid TTL reference '%s'",byte.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);
   return(OK);

}  


/**********************************************************************
     Generate the first statement of a loop 

     loop(name, repeats)

**********************************************************************/

short LoopStart(DLLParameters* par, char *args)
{
   short nrArgs;
   CText nrTimes;
   long NrTimes;
   CText loopName;
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,2,"loop name, number repeats","ee","qq",&loopName,&nrTimes)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(nrTimes[0] == 'n')
      InsertUniqueStringIntoList(nrTimes.Str(),&parList,szList);

   InsertUniqueStringIntoList(loopName.Str(),&parList,szList);


   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Loop");

   if(loopName[0] == 'l')
   {
      if(nrTimes[0] == 'n')
      {
         fprintf(fp,"\n        move    x:NR%s,r1                 ; Load number repeats into r1",nrTimes.Str()+1);
         fprintf(fp,"\n        do      r1,LOOP%s                 ; Repeat code until Loop end",loopName.Str()+1);
      }
      else if(sscanf(nrTimes.Str(),"%ld",&NrTimes) == 1)
      {
         fprintf(fp,"\n        move    #%ld,r1                   ; Load number repeats into r1",NrTimes);
         fprintf(fp,"\n        do      r1,LOOP%s                 ; Repeat code until Loop end",loopName.Str()+1);
      }
      else
      {
         Error(par->itfc,"invalid number of loops '%s'",nrTimes.Str());
         fclose(fp);
         return(ERR);
      }
   }
   else
   {
      Error(par->itfc,"invalid loop name '%s'",loopName.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);

   return(OK);
}

/**********************************************************************
     Generate the last statement of a loop 

     endloop(name)

**********************************************************************/

short LoopEnd(DLLParameters* par, char *args)
{
   short nrArgs;
   CText loopName;
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,1,"loop name","e","q",&loopName)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   InsertUniqueStringIntoList(loopName.Str(),&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Loop end");

   if(loopName[0] == 'l')
   {
      fprintf(fp,"\nLOOP%s  nop                ; Identifies end of loop",loopName.Str()+1);
   }
   else
   {
      Error(par->itfc,"invalid loop name '%s'",loopName.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);

   return(OK);
}

/**********************************************************************
     Clear the data memory 

     cleardata(number of complex points to clear)

**********************************************************************/

short ClearData(DLLParameters* par, char *args)
{
   short nrArgs;
   CText nrPnts;
   long NrPnts;
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,1,"number","e","q",&nrPnts)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(nrPnts[0] == 'n')
      InsertUniqueStringIntoList(nrPnts.Str(),&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Clear the data memory");
   fprintf(fp,"\n;");
   fprintf(fp,"\n        move    #$%d,r5              ; Make r5 point to the start of fid memory", DSP_MEMORY_ADRS);

   if(nrPnts[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,r7                ; Zero NR%s*2 points",nrPnts.Str()+1,nrPnts.Str()+1);
   }
   else if(sscanf(nrPnts.Str(),"%ld",&NrPnts) == 1)
   {
      fprintf(fp,"\n        move    #%ld,r7                ; Zero %ld*2 points",NrPnts,NrPnts);
   }

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        do      r7,clearm");
   fprintf(fp,"\n        move    a1,y:(r5)+");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        move    a1,y:(r5)+");
   fprintf(fp,"\nclearm  nop");
   fprintf(fp,"\n        move    #$%d,r5              ; Make r5 point to the start of fid memory", DSP_MEMORY_ADRS);
   fclose(fp);

   return(OK);
}

/******************************************************************************************
  Acquire data from the Kea transceiver. 
  
  acquire(mode, nr_points, [[duration], address])
  
  Possible modes are:

  overwrite ... data is always written to location y:0x10000 or y:address
  append ...... data is appended to the last acquired data
  sum ......... data is summed to the previously acquired data starting at location y:0x10000 or y:address
  integrate ... data is summed and stored as one complex point 

  The optional duration parameter specifies a minimum period for the acquisition command.

  Last modified: 28-2-2014
*******************************************************************************************/

short AcquireData(DLLParameters* par, char *args)
{
   short nrArgs;
   CText nrPnts;
   CText duration;
   CText mode; 
   CText adrs; 
   long NrPnts;
   long Durations;
   long Adrs;
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,2,"mode, nr points, [[duration], saveAdrs]","eeee","qqqq",&mode,&nrPnts,&duration,&adrs)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add new variables to the list
   InsertUniqueStringIntoList(nrPnts.Str(),&parList,szList);
   if(nrArgs >= 3)
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);
   if(nrArgs == 4)
      InsertUniqueStringIntoList(adrs.Str(),&parList,szList);

// Open the output file
   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

// Overwrite mode
   if(mode == "overwrite")
   {
		if(nrArgs == 2) // No delay
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire data (overwrite without delay)");

			if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
			fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; FPGA for the wrap-up algorithm to use ### make sure this is the same number as the loop count or major errors could occur");

         fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
         fprintf(fp,"\n        or      #$000010,a              ; Reset CIC");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");
         fprintf(fp,"\n        and     #$ffffef,a              ; Remove CIC flag");
         fprintf(fp,"\n        or      #$000001,a              ; Start ADC capture");
			fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    #$%d,r5               ; Specify the save address", DSP_MEMORY_ADRS);
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);
         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1        ; Load data from channel A");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n         move    a1,y:(r5)+              ; Save to memory");
         fprintf(fp,"\nLBL%ld    nop",label);

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         label += 1;
			fclose(fp);
		}
		else
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire data (overwrite with delay)");

			if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
			fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; FPGA for the wrap-up algorithm to use ### make sure this is the same number as the loop count or major errors could occur");

         fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
         fprintf(fp,"\n        or      #$000010,a              ; Reset CIC");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");
         fprintf(fp,"\n        and     #$ffffef,a              ; Remove CIC flag");
         fprintf(fp,"\n        or      #$000001,a              ; Start ADC capture");
			fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

			fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
			fprintf(fp,"\n        sub     #5,a"); // Tweak it
			fprintf(fp,"\n        move    a1,r3");
			fprintf(fp,"\n        movep   r3,x:A_TCPR2");
			fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    #$%d,r5               ; Specify the save address", DSP_MEMORY_ADRS);
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);
         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1        ; Load data from channel A");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n         move    a1,y:(r5)+              ; Save to memory");
         fprintf(fp,"\nLBL%ld    nop",label);

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

			fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
			fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2");

         label += 1;

			fclose(fp);
		}
	}
	else if(mode == "append")
	{
		if(nrArgs == 2) // No delay
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire (append without delay)");

			if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
			fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; FPGA for the wrap-up algorithm to use ### make sure this is the same number as the loop count or major errors could occur");
         fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
         fprintf(fp,"\n        or      #$000010,a              ; Reset CIC");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");
         fprintf(fp,"\n        and     #$ffffef,a              ; Remove CIC flag");
         fprintf(fp,"\n        or      #$000001,a              ; Start ADC capture");
			fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the data address");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);

         fprintf(fp,"\n        do      r7,LBL%ld               ; Collect n samples",label);
			fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1       ; Load data from channel A");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\nLBL%ld  nop",label);

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    r5,y:DATA_ADRS           ; Save data address");

			fclose(fp);

         label++;
		}
		else
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire (append with delay)");

			if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
			fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; FPGA for the wrap-up algorithm to use ### make sure this is the same number as the loop count or major errors could occur");

         fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
         fprintf(fp,"\n        or      #$000010,a              ; Reset CIC");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");
         fprintf(fp,"\n        and     #$ffffef,a              ; Remove CIC flag");
         fprintf(fp,"\n        or      #$000001,a              ; Start ADC capture");
			fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the data address");

			fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
			fprintf(fp,"\n        sub     #5,a"); // Tweak it
			fprintf(fp,"\n        move    a1,r3");
			fprintf(fp,"\n        movep   r3,x:A_TCPR2");
			fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);

         fprintf(fp,"\n        do      r7,LBL%ld               ; Collect n samples",label);
			fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1       ; Load data from channel A");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\nLBL%ld  nop",label);

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    r5,y:DATA_ADRS           ; Save data address");

			fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
			fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2");
 

			fclose(fp);

         label++;

		}		
	}

	else if(mode == "test")
	{
	if(nrArgs == 2) // No delay
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire (append without delay)");

			if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
			fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; FPGA for the wrap-up algorithm to use ### make sure this is the same number as the loop count or major errors could occur");
         fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
         fprintf(fp,"\n        or      #$000010,a              ; Reset CIC");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");
         fprintf(fp,"\n        and     #$ffffef,a              ; Remove CIC flag");
         fprintf(fp,"\n        or      #$000001,a              ; Start ADC capture");
			fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the data address");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);

         fprintf(fp,"\n        do      r7,LBL%ld               ; Collect n samples",label);
			fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1       ; Load data from channel A");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        move    #2,b1                     ; Abort code");
         fprintf(fp,"\n        cmp     #$10020,a");
         fprintf(fp,"\n        jlt     CONT");
         fprintf(fp,"\n        enddo");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\nCONT    nop");

         fprintf(fp,"\nLBL%ld  nop",label);

          fprintf(fp,"\n        jge    ABORT");
 
         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    r5,y:DATA_ADRS           ; Save data address");

			fclose(fp);

         label++;
		}
	}

	else if(mode == "sum")
	{
		if(nrArgs == 2) // No delay
		{
         fprintf(fp,"\n\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Acquire (summing without delay)");

         fprintf(fp,"\n        clr     a                       ; Clear register a");
         fprintf(fp,"\n        clr     b                       ; Clear register b");
         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
         fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Load the number of samples into r7",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    #$%d,r5              ; Load the number of samples into r7", DSP_MEMORY_ADRS);
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label+2);

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_TTL,a1             ; Load data from channel A");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

         fprintf(fp,"\nLBL%ld   move    y:(r5),b1               ; Get last value at this location",label);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:FPGA_TTL,a1             ; Load data from channel B");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

         fprintf(fp,"\nLBL%ld   move    y:(r5),b1               ; Get last value at this location",label+1);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\nLBL%ld  nop",label+2);
         fclose(fp);

         label += 3;
      }
	   else
	   {
         fprintf(fp,"\n\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Acquire (summing with delay)");

			if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
			fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; FPGA for the wrap-up algorithm to use ### make sure this is the same number as the loop count or major errors could occur");

         fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
         fprintf(fp,"\n        or      #$000010,a              ; Reset CIC");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");
         fprintf(fp,"\n        and     #$ffffef,a              ; Remove CIC flag");
         fprintf(fp,"\n        or      #$000001,a              ; Start ADC capture");
			fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        clr     a                       ; Clear register a");
         fprintf(fp,"\n        clr     b                       ; Clear register b");
         fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
         fprintf(fp,"\n        sub     #5,a"); // Tweak it
         fprintf(fp,"\n        move    a1,r3");
         fprintf(fp,"\n        movep   r3,x:A_TCPR2");
         fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");

         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    #$%d,r5               ; Specify the save address", DSP_MEMORY_ADRS);
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
 
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);
         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*          ; Wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1       ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1        ; Load data from channel A");

         fprintf(fp,"\n        move    y:(r5),b1               ; Save to memory");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");

         fprintf(fp,"\n        move    y:(r5),b1               ; Save to memory");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\nLBL%ld    nop",label);

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
         fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2");
         fclose(fp);


         label++;
      }
   }
   else if(mode == "integrate")
   {
		if(nrArgs == 2) // No delay
		{
         fprintf(fp,"\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Acquire (integrating without delay)");


			if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
			fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; FPGA for the wrap-up algorithm to use ### make sure this is the same number as the loop count or major errors could occur");

         fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
         fprintf(fp,"\n        or      #$000010,a              ; Reset CIC");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");
         fprintf(fp,"\n        and     #$ffffef,a              ; Remove CIC flag");
         fprintf(fp,"\n        or      #$000001,a              ; Start ADC capture");
			fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
         fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");

		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1        ; Load data from channel A");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        sub     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Restore r5");


         fprintf(fp,"\nLBL%ld  nop",label);

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        add     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Increment r5 by 2");

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fclose(fp);

         label ++;
      }
      else
      {
         fprintf(fp,"\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Acquire (integrating with delay)");


			if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
			fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; FPGA for the wrap-up algorithm to use ### make sure this is the same number as the loop count or major errors could occur");

         fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
         fprintf(fp,"\n        or      #$000010,a              ; Reset CIC");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");
         fprintf(fp,"\n        and     #$ffffef,a              ; Remove CIC flag");
         fprintf(fp,"\n        or      #$000001,a              ; Start ADC capture");
			fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    x:DELAY%s,a1           ; Total acquisition time",duration.Str()+1);
         fprintf(fp,"\n        sub     #5,a"); // Tweak it
         fprintf(fp,"\n        move    a1,r3");
         fprintf(fp,"\n        movep   r3,x:A_TCPR2");
         fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
         fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");

		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
 
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);
         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1       ; Load data from channel A");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        nop");

         fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        sub     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Restore r5");


         fprintf(fp,"\nLBL%ld  nop",label);

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        add     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Increment r5 by 2");

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
         fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2");

         fclose(fp);

         label++;
      }
   }
   else
   {
      fclose(fp);
      Error(par->itfc,"invalid acquire mode");
      return(ERR);
   }
   return(OK);
} 
//
///**********************************************************************
//     Run an external script file and wait for it to complete.
//
//     execwait(file to run, argument list)
//**********************************************************************/
//
//STARTUPINFO startupinfo;
//PROCESS_INFORMATION processinfo;
//
//short ExecuteAndWait(DLLParameters* par, char *args)
//{
//   CText cmdline;
//   CText file;
//   CText arguments;
//   short nrArgs;
//   DWORD exitCode;
//
//   if((nrArgs = ArgScan(par->itfc,args,1,"file,arguments","ee","qq",&file,&arguments)) < 0)
//      return(nrArgs);
//
//   cmdline.Format("%s %s",file.Str(),arguments.Str());
//   startupinfo.cb = sizeof(STARTUPINFO);
//   bool st = CreateProcess(file.Str(),cmdline.Str(),NULL,NULL,false,NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,NULL,NULL,&startupinfo,&processinfo);
//
//   WaitForSingleObject(processinfo.hProcess, INFINITE);
//   bool result = GetExitCodeProcess(processinfo.hProcess, &exitCode);
//
//   CloseHandle(processinfo.hProcess);
//
//   par->retVar[1].MakeAndSetFloat(exitCode);
//   par->nrRetVar = 1;
//
//   return(OK);
//}

/**********************************************************************
     End the pulse program generation process by adding the parameter 
     block and joining all code segments together.
**********************************************************************/

void ReplaceStr(CText &ctext, char* text, char* oldStr, char* newStr);


short EndPP(DLLParameters* par, char *args)
{
   char *text;
   FILE *fp;
   char varName[50];
   long sz,len;
   short nrArgs;
   CText startFile = "startCode.asm";


// Write the header file
   fp = fopen("temp.asm","w");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n      nolist");
   fprintf(fp,"\n      include  'ioequ.asm'");
   fprintf(fp,"\n      include  'FPGA.asm'");
   fprintf(fp,"\n      list");
   fprintf(fp,"\n;***************************************************************************\n");

   fprintf(fp,"\n                org     x:$0\n\n");
   fprintf(fp,"\nPARAM_BASE      equ     *       ; Memory for pulse program parameters");
   fprintf(fp,"\nRXG1            ds      1       ; 00 - Receiver gain block 1");
   fprintf(fp,"\nRXG2            ds      1       ; 01 - Receiver gain block 2");
   fprintf(fp,"\nDEC1            ds      1       ; 02 - Decimation for CIC1");
   fprintf(fp,"\nDEC5            ds      1       ; 03 - Decimation for CIC5");
   fprintf(fp,"\nDECFIR          ds      1       ; 04 - Decimation for FIR");
   fprintf(fp,"\nATT1            ds      1       ; 05 - Attenuation for CIC1");
   fprintf(fp,"\nDELAYFIR        ds      1       ; 06 - Delay for CIC5");
   fprintf(fp,"\nATTFIR          ds      1       ; 07 - Attenuation for FIR");
   fprintf(fp,"\nNtaps           ds      1       ; 08 - Taps for FIR");
   fprintf(fp,"\nTXF00           ds      1       ; 09 - Tx Frequency word 0");
   fprintf(fp,"\nTXF01           ds      1       ; 10 - Tx Frequency word 1");
   fprintf(fp,"\nRXSETCHANNEL    ds      1       ; 11 - RxAmp set channel code");
   fprintf(fp,"\nRXSETGAIN       ds      1       ; 12 - RxAmp set gain code");
   fprintf(fp,"\nRXF00           ds      1       ; 13 - Rx Frequency word 0");
   fprintf(fp,"\nRXF01           ds      1       ; 14 - Rx Frequency word 1");
   fprintf(fp,"\n                ds      1       ; 15 - not used");
   fprintf(fp,"\n                ds      1       ; 16 - not used");
   fprintf(fp,"\nRXP0            ds      1       ; 17 - Rx Phase word 0");
   fprintf(fp,"\nNRSCANS         ds      1       ; 18 - Number of scans to perform");
   fprintf(fp,"\nEXPDELAY        ds      1       ; 19 - Delay between experiments");
   fprintf(fp,"\nPGO             ds      1       ; 20 - Pulse gate overhead delay");
   fprintf(fp,"\nGRADRESET       ds      1       ; 21 - 1 if gradients are to be reset");
   fprintf(fp,"\nLFRXAMP         ds      1       ; 22 - 1 if low frequency Kea");
   fprintf(fp,"\nSKIPPNTS        ds      1       ; 23 - Points to skip at start of acquisition");
   fprintf(fp,"\nJITTER_CH1      ds      1       ; 24 - DDS channel 1 antiphase jitter parameter");
   fprintf(fp,"\nJITTER_CH2      ds      1       ; 25 - DDS channel 2 antiphase jitter parameter");
   fprintf(fp,"\nSoftVersion     ds      1       ; 26 - FPGA software version return");
   fprintf(fp,"\nUseTrigger      ds      1       ; 27 - Use the trigger input (0/1)");
   fprintf(fp,"\n\n; Pulse program info\n");

// Loop over variable list extracting variable names
   short c = 28;
   for(short i = 0; i < szList; i++)
   {
	   strncpy(varName,parList[i],50);
      if(varName[0] == 'f')
      {
         fprintf(fp,"\nFX%s_0           ds      1       ; %hd - Frequency %s word 0",varName+1,c++,varName+1);
         fprintf(fp,"\nFX%s_1           ds      1       ; %hd - Frequency %s word 1",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'a')
      {
         fprintf(fp,"\nTXA%s            ds      1       ; %hd - Tx amplitude %s word 0",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'g')
      {
         fprintf(fp,"\nGAIN%s_0         ds      1       ; %hd - Rx gain %s word 0",varName+1,c++,varName+1);
         fprintf(fp,"\nGAIN%s_1         ds      1       ; %hd - Rx gain %s word 1",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'p')
      {
         fprintf(fp,"\nTXP%s            ds      1       ; %hd - Tx phase %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'd')
      {
         fprintf(fp,"\nDELAY%s          ds      1       ; %hd - Delay %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'w')
      {
         fprintf(fp,"\nWAIT%s_0         ds      1       ; %hd - Wait steps %s",varName+1,c++,varName+1);
         fprintf(fp,"\nWAIT%s_1         ds      1       ; %hd - Wait unit1 %s", varName+1,c++,varName+1);
         fprintf(fp,"\nWAIT%s_2         ds      1       ; %hd - Wait unit2 %s", varName+1,c++,varName+1);
      }
      else if(varName[0] == 'n')
      {
         fprintf(fp,"\nNR%s             ds      1       ; %hd - Number %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'b')
      {
         fprintf(fp,"\nTTL%s            ds      1       ; %hd - Byte %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 't')
      {
         fprintf(fp,"\nTABLE%s          ds      1       ; %hd - Table %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'm')
      {
         fprintf(fp,"\nMEM%s          ds      1       ; %hd - Memory address %s",varName+1,c++,varName+1);
      }
   }
   fprintf(fp,"\n\n");
   fclose(fp);

// Readin start code
   fp = fopen(startFile.Str(),"r");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }
   fseek(fp,0,SEEK_END);
   len = ftell(fp);
   text = new char[len+1];
   fseek(fp,0,SEEK_SET);
   sz = fread(text,1,len,fp);
   text[sz] = '\0';
   fclose(fp);

// Append a trigger string is desired
   CText ctext = text;
   delete [] text;

// Append to total
   fp = fopen("temp.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }
   fprintf(fp,"\n\n");
   fwrite(ctext.Str(),1,ctext.Size(),fp);
   fclose(fp);

// Readin middle code
   fp = fopen("midCode.asm","r");
   if(!fp)
   {
      Error(par->itfc,"Can't open midCode file");
      return(ERR);
   }
   fseek(fp,0,SEEK_END);
   len = ftell(fp);
   text = new char[len+1];
   fseek(fp,0,SEEK_SET);
   sz = fread(text,1,len,fp);
   text[sz] = '\0';
   fclose(fp);

// Append to total
   fp = fopen("temp.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open temporary file");
      return(ERR);
   }
   fprintf(fp,"\n\n");
   fwrite(text,1,sz,fp);
   fclose(fp);
   delete [] text;

// Readin end code
   fp = fopen("endCode.asm","r");
   if(!fp)
   {
      Error(par->itfc,"Can't open endCode file");
      return(ERR);
   }
   fseek(fp,0,SEEK_END);
   len = ftell(fp);
   text = new char[len+1];
   fseek(fp,0,SEEK_SET);
   sz = fread(text,1,len,fp);
   text[sz] = '\0';
   fclose(fp);

// Append to total
   fp = fopen("temp.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }
   fprintf(fp,"\n\n");
   fwrite(text,1,sz,fp);
   fclose(fp);
   delete [] text;

// Delete the temp file
   remove("midCode.asm");

// Copy the variable list to ansVar	************************
   par->retVar[1].MakeAndSetList(parList,szList);
   par->nrRetVar = 1;
   FreeList(parList,szList+1); // 1 bigger because we started with 1 dummy entry
   szList = 0;
   parList = NULL;

   return(OK);
}

/**********************************************************************
     Add string 'str' into 'list' at 'position'
**********************************************************************/

void InsertUniqueStringIntoList(char *str, char ***list, long &position)
{
   for(int i = 0; i < position; i++)
   {
      if(!strcmp(str,(*list)[i]))
         return;
   }
   InsertStringIntoList(str,list,position,position++);
}

void ReplaceStr(CText &result, char* text, char* oldStr, char* newStr)
{
   long i,j,k;
   long lenText,lenOld,lenNew;

   lenText = strlen(text);
   lenOld = strlen(oldStr);
   lenNew = strlen(newStr);

   for(i = 0; i < lenText; i++)
   {
      for(j = 0; j < lenOld; j++)
      {
         if(text[i+j] != oldStr[j])
            break;
      }
      if(j == lenOld) // Match found
      {
         for(k = 0; k < lenNew; k++)
            result.Append(newStr[k]);
         i += lenOld-1;
      }
      else
         result.Append(text[i]);
   }
}


// Utility routine which generates the gradient set up code based on the gradient number or letter


short SelectGradient(Interface *itfc ,FILE* fp, CText grad)
{
   long Grad;

// Choose first gradient to change
   if(grad[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",grad.Str()+1);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(grad == "x")
   {
      fprintf(fp,"\n        move    #3,a1");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(grad == "y")
   {
      fprintf(fp,"\n        move    #2,a1");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(grad == "z")
   {
      fprintf(fp,"\n        move    #1,a1");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(grad == "o")
   {
      fprintf(fp,"\n        move    #0,a1");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(sscanf(grad.Str(),"%ld",&Grad) == 1)
   {
      if(Grad < 0 || Grad > 3)
      {
         Error(itfc,"invalid gradient index '%s' [0,1,2,3]",grad);
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #%ld,a1",Grad);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
	else
   {
      Error(itfc,"invalid gradient index '%s' [0,1,2,3] or [x,y,z,o]",grad);
      fclose(fp);
      return(ERR);
   }

   return(OK);
}




/**************************************************************************************************
  Utility routine which initializes a register with a number either as a variable or a constant.
  also includes bounds check in the case of a constant. reg is the register to get the parameter.
***************************************************************************************************/

short SelectNumber(Interface *itfc, FILE* fp, CText num, char *reg, long min, long max)
{
   long Number;

   if(num[0] == 'n')
	{
	   fprintf(fp,"\n        move   x:NR%s,%s",num.Str()+1,reg);
	}
	else if(sscanf(num.Str(),"%ld",&Number) == 1)
	{
		if(Number < min || Number > max)
		{
			Error(itfc,"invalid number of points '%ld' [%ld ... %ld]",Number,min,max);
			fclose(fp);
         return(ERR);
		}
      fprintf(fp,"\n        move   #%ld,%s",Number,reg);
	}
	else
	{
		Error(itfc,"Invalid number reference '%s'",num.Str());
		fclose(fp);
		return(ERR);
	}

   return(OK);
}

/**************************************************************************************************
Utility routine which initializes the a1 register with a phase value
***************************************************************************************************/


short SelectPhase(Interface *itfc ,FILE* fp, CText phase, CText channel)
{
   float Phase;
   bool correct;

   if(phase[0] == 'p') // Entry for phase cycling
   {
	   fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      correct = true;
   }
   else  if(phase[0] == 't') // Table based phase
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",phase.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",phase.Str()+1);

      fprintf(fp,"\n        add     b,a");  // Add the index to table start to find current value
      fprintf(fp,"\n        move    a0,r5"); // 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read the table value
      correct = false;
   }
   else if(sscanf(phase.Str(),"%f",&Phase) == 1) // Constant from 0 ... 4 
   {
      if(Phase < 0 || Phase >= 4)
      {
         Error(itfc,"invalid phase '%f' [0 ... 4)",Phase);
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #%ld,a1",(long)(Phase*16384.0+0.5));
      correct = true;
   }
   else
   {
      Error(itfc,"Invalid phase reference '%s'",phase.Str());
      fclose(fp);
      return(ERR);
   }

// Save this to current phase
   fprintf(fp,"\n        move    a1,y:TX_PHASE"); 

// Send to appropriate FPGA channel
   if (channel == "1" || channel == "w1" || channel == "1nb")
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
   else
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
  
   if(correct) 
      fprintf(fp,"\n        nop                             ; Eliminate pulse length jitter");


   return(OK);
}

/**************************************************************************************************
Utility routine which initializes the a1 register with an RF amplitude value
***************************************************************************************************/

short SelectAmplitude(Interface *itfc ,FILE* fp, CText amp, CText channel)
{
   long Amplitude;

   if(amp[0] == 'a')
   {
	   fprintf(fp,"\n        move    x:TXA%s,a1",amp.Str()+1);
   }
   else if(sscanf(amp.Str(),"%ld",&Amplitude) == 1)
   {
      if(Amplitude < 0 || Amplitude > 16384)
      {
         Error(itfc,"invalid amplitude '%ld' [0 ... 16383]",Amplitude);
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #%ld,a1",Amplitude);
   }
   else
   {
      Error(itfc,"Invalid amplitude reference '%s'",amp.Str());
      fclose(fp);
      return(ERR);
   }

// Save this to current amplitude
   fprintf(fp,"\n        move    a1,y:TX_AMP");

// Send to appropriate FPGA channel
   if(channel == "1" || channel == "w1" || channel == "1nb")
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
   else if(channel == "2" || channel == "w2" || channel == "2nb")
      fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
   else
   {
      Error(itfc,"Invalid channel reference '%s'",channel.Str());
      fclose(fp);
      return(ERR);
   }

   return(OK);
}

short SelectDuration(Interface *itfc ,FILE* fp, char *reg, CText duration)
{
	float fDuration;
	long Duration;

   if(duration[0] == 'd') // Variable based delay
   {
	   fprintf(fp,"\n        move    x:DELAY%s,%s",duration.Str()+1,reg);
   }
   else if(sscanf(duration.Str(),"%f",&fDuration) == 1) // Fixed delay
   {
      if(fDuration < 0.25 || fDuration > 327670)
      {
         Error(itfc,"invalid delay '%f' [0.25...327670]",fDuration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp,"\n        move    #%ld,%s",Duration,reg);
      fprintf(fp,"\n        nop"); // Remove jitter
   }
   else
   {
      Error(itfc,"Invalid duration reference '%s'",duration.Str());
      fclose(fp);
      return(ERR);
   }

   return(OK);
}

short SelectFrequency(Interface *itfc, FILE*fp, CText freq, CText channel)
{
   double fFreq;

   if(freq[0] == 'f') // Frequency parameter
   {
	   if(channel == "1" || channel == "1nb" || channel == "w1")
	   {
		   fprintf(fp,"\n        move    x:FX%s_0,a1",freq.Str()+1);
		   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
		   fprintf(fp,"\n        move    x:FX%s_1,a1",freq.Str()+1);
		   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
	   }
	   else if(channel == "2" || channel == "2nb" || channel == "w2")
	   {
		   fprintf(fp,"\n        move    x:FX%s_0,a1",freq.Str()+1);
		   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
		   fprintf(fp,"\n        move    x:FX%s_1,a1",freq.Str()+1);
		   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
	   }
      else
      {
         Error(itfc,"Invalid channel reference '%s'",channel.Str());
         fclose(fp);
         return(ERR);
      }
   }
// This code is not reliable - for some reason the pulse duration becomes unstable for some
// frequencies.
  // else if(sscanf(freq.Str(),"%lf",&fFreq) == 1) // Fixed frequency
  // {
  //    if(fFreq < 0 || fFreq > 500)
  //    {
  //       Error(itfc,"invalid frequency '%lg' [0 ... 500]",fFreq);
  //       fclose(fp);
  //       return(ERR);
  //    }
  //    __int64 DDSFword = (__int64)((fFreq * 4294967296.0L)/1000.0L + 0.5); 
  //    long w1  = (unsigned long)((DDSFword & 0xFFFF0000)/65536.0L);
  //    long w2 = (unsigned long)(DDSFword & 0x0000FFFF);

	 //  if(channel == "1")
	 //  {
  //       fprintf(fp,"\n        move    #%ld,a1",w1);
		//   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
  //       fprintf(fp,"\n        move    #%ld,a1",w2);
		//   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
	 //  }
	 //  else if(channel == "2")
	 //  {
  //       fprintf(fp,"\n        move    #%ld,a1",w1);
		//   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
  //       fprintf(fp,"\n        move    #%ld,a1",w2);
		//   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
	 //  }
  ////  fprintf(fp,"\n        nop                             ; Eliminate pulse length jitter");

  // }
   else
   {
      Error(itfc,"Invalid frequency reference '%s'",freq.Str());
      fclose(fp);
      return(ERR);
   }
   return(OK);
}




/**********************************************************************

     Set a shim (16/31 channel version) current level 

     shim16([0 ... 31]/nx, [-2^15 ... 2^15]/nx)

     Note this is designed to work with one or two 16-channel shim controllers

     Control from DSP to Gradient board is via the SSI-1 serial interface.
     The 32 lines are divided between two PCBs. The boards use the same address
     lines but different addresses access different board.
     On each board are four 4 channel DACs. These are selected using the lines
     PE0/PC0/PE2 which are set via the x:A_PDRC/E GPIO lines. 

     Board 1 addresses  Board 2 address

     4 ... DAC 0        0 ... DAC 0
     5 ... DAC 1        1 ... DAC 1
     6 ... DAC 2        2 ... DAC 2
     7 ... DAC 3        3 ... DAC 3

     When writing to the DACs the 16 bit data word is combined with channel
     info according to the following 24 bit word:

     0001|00xx|xxxx|xxxx|xxxx|xxxx
       20   16   12    8    4    0

     bits 0-15 : data
     bits 16 & 17 : which DAC channel (0,1,2,3)
     bit 20 : write to DAC

     Timing: - command 2.5 us.
             - output appears at this time.

**********************************************************************/

short SwitchOnShim16(DLLParameters* par, char *args)
{
   short nrArgs;
   CText channel,amplitude;
   long Channel, Amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,2,"channel,amplitude","ee","qq",&channel,&amplitude)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add to parameter list if not a constant
   if(channel[0] == 'n')
      InsertUniqueStringIntoList(channel.Str(),&parList,szList);
   if(amplitude[0] == 'n')
      InsertUniqueStringIntoList(amplitude.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set 16 channel shim level");

// Set the serial port
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred"); 
  // fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs"); 
   fprintf(fp,"\n        movep   #$1343C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs"); 

// Select the gradient channel
   if(SelectShim16(par->itfc,fp,channel)) return(ERR);

// Set the gradient amplitude
   if(amplitude[0] != 'n')
   {
      if(sscanf(amplitude.Str(),"%ld",&Amplitude) == 1)
      {
         if(Amplitude > 32768 || Amplitude < -32767)
         {
            Error(par->itfc,"invalid shim amplitude '%s' [-32767 ... 32768]",channel);
            fclose(fp);
            return(ERR);
         }
      }
	   else
      {
         Error(par->itfc,"invalid shim amplitude '%s' [-32767 ... 32768]",channel);
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #$%X,a1           ; Get gradient amplitude",Amplitude); 
   }
   else
   {
      fprintf(fp,"\n        move    x:NR%s,a1                ; Get gradient amplitude",amplitude.Str()+1); 
   }
   fprintf(fp,"\n        move    #$00FFFF,x1"); 
   fprintf(fp,"\n        and      x1,a1");  // Mask out data

   fprintf(fp,"\n        move    b0,b1                  ; Restore subgroup"); 
   fprintf(fp,"\n        lsl     #16,b                  ; Move into correct format for DAC");    // Shift 16 bits to left
   fprintf(fp,"\n        move    #$030000,x1");               // Extract lower 2 bits of channel number
   fprintf(fp,"\n        and      x1,b"); 
   fprintf(fp,"\n        move    #$100000,x1");               // Set data register mode bit
   fprintf(fp,"\n        or       x1,b"); 
   fprintf(fp,"\n        move     b1,x1");
   fprintf(fp,"\n        or       x1,a                  ; Add amplitude word"); 

   fprintf(fp,"\n        move    a1,x:A_TX10            ; Send channel info + grad. amplitude to DAC"); 
   fprintf(fp,"\n        move    #2,r7                  ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");
   fprintf(fp,"\n        movep   #$04,x:A_PCRD           ; Turn off SSI 1 on Port D");

   fclose(fp);

   return(OK);
}




/*******************************************************************************

   Make a gradient ramp which has a start and end amplitude and a duration
  
   Syntax   gradramp(start, end, nrSteps, stepDuration)
  
   - channel can be a number or a number variable.
   - start and end can be constants or tables.
   - nrSteps and stepDuration can be constants or number variables. 

   This version is for the gradient controller. See gradon/off for more
   details.

*********************************************************************************/

short RampedGradient(DLLParameters* par, char *args)
{
   short nrArgs;
   CText start,end,steps,duration;
   long Steps;
   long Start,End;
   long Address;
   long Duration;
   float fDuration;
   long number,amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,4,"start,end,steps,duration","eeee","qqqq",&start,&end,&steps,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(start[0] == 'n' || start[0] == 't')
      InsertUniqueStringIntoList(start.Str(),&parList,szList);
   if(end[0] == 'n' || end[0] == 't')
      InsertUniqueStringIntoList(end.Str(),&parList,szList);
   if(steps[0] == 'n')
      InsertUniqueStringIntoList(steps.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Generate a ramped gradient");

// Set the serial port configuration
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred"); 
//  fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
   fprintf(fp,"\n        movep   #$1343C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
	
// Switch on the gradient amp
   fprintf(fp,"\n; Switch on gradient amp");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$01,a1");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
	fprintf(fp,"\n        move    #$FFFFFF,a1");
   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
   fprintf(fp,"\n        move    #15,r7                  ; Wait 15 us");
   fprintf(fp,"\n        bsr     wait");

// Select the gradient
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$00,a1");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");

// Get the start amplitude
   if(start[0] == 'n') // Start amplitude is via a number reference
   {
      fprintf(fp,"\n        move    x:NR%s,a1",start.Str()+1);
      fprintf(fp,"\n        move    a1,y:(TMP+7)");    
   }
   else if(start[0] == 't') // start amplitude is t[index]
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",start.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",start.Str()+1);

      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
      fprintf(fp,"\n        move    a1,y:(TMP+7)");
   }
   else if(sscanf(start.Str(),"%ld",&amplitude) == 1) // Start amplitude is a number
   {
      fprintf(fp,"\n        move    #$%X,a1",amplitude);
      fprintf(fp,"\n        move    a1,y:(TMP+7)");  
   }
   else
   {
      Error(par->itfc,"Invalid start amplitude '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Get the end amplitude
   if(end[0] == 'n') // End amplitude is via a number reference
   {
      fprintf(fp,"\n        move    x:NR%s,a1",end.Str()+1);
      fprintf(fp,"\n        move    a1,y:(TMP+8)");    
   }
   else if(end[0] == 't') // End amplitude is t[index]
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",end.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",end.Str()+1);

      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
      fprintf(fp,"\n        move    a1,y:(TMP+8)");
   }
   else if(sscanf(end.Str(),"%ld",&amplitude) == 1) // End amplitude is a number
   {
      fprintf(fp,"\n        move    #$%X,a1",amplitude);
      fprintf(fp,"\n        move    a1,y:(TMP+8)");  
   }
   else
   {
      Error(par->itfc,"Invalid end amplitude '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Get the number of steps
   if(steps[0] == 'n') // Steps via a number reference
   {
   // Set the level
      fprintf(fp,"\n        move    x:NR%s,a0",steps.Str()+1);
      fprintf(fp,"\n        dec     a");
      fprintf(fp,"\n        move    a0,y:(TMP+6)");    
   }
   else if(sscanf(steps.Str(),"%ld",&number) == 1) // Steps is a number
   {
      if(number > 0)
      {
         fprintf(fp,"\n        move    #%ld,a0",number);
         fprintf(fp,"\n        dec     a");
         fprintf(fp,"\n        move    a0,y:(TMP+6)");  
      }
      else
      {
         Error(par->itfc,"Invalid number of steps '%s'",steps.Str());
         fclose(fp);
         return(ERR);
      }
   }
   else
   {
      Error(par->itfc,"Invalid number of steps '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Step duration
   if(duration[0] == 'd')
   {
		fprintf(fp,"\n        move   x:DELAY%s,a1",duration.Str()+1);
   }
   else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
   {
      if(fDuration < 0.25 || fDuration > 327670)
      {
         Error(par->itfc,"invalid delay '%g' [1...327670]",fDuration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp,"\n        move   #%ld,a1",Duration);
   }
   else
   {
      Error(par->itfc,"Invalid step duration '%s'",duration.Str());
      fclose(fp);
      return(ERR);
   }
   fprintf(fp,"\n        move    a1,y:(TMP+9)");    

// Set the initial n value
   fprintf(fp,"\n        move    #0000,a1");
   fprintf(fp,"\n        move    a1,y:(TMP+5)");    

   fprintf(fp,"\n        move    y:(TMP+6),a0             ; Load the steps into r7");
   fprintf(fp,"\n        inc     a                        ; Load the steps into r7");
   fprintf(fp,"\n        move    a0,r2                    ; Load the steps into r7");

   fprintf(fp,"\n        do      r2,LBL%ld                ; Calculate each step value",label);

   fprintf(fp,"\n; **************************************************");
   fprintf(fp,"\n; v = n*(end-start)/steps + start");
   fprintf(fp,"\n; **************************************************");
   fprintf(fp,"\n        move    y:(TMP+8),b");  
   fprintf(fp,"\n        move    b1,y:(TMP+2)");  
   fprintf(fp,"\n        move    y:(TMP+7),x1");  
   fprintf(fp,"\n        move    y:(TMP+2),b");  
   fprintf(fp,"\n        sub     x1,b");  
   fprintf(fp,"\n        move    b1,x0");  
   fprintf(fp,"\n        move    y:(TMP+5),y0");

   fprintf(fp,"\n        mpy     y0,x0,b");  
   fprintf(fp,"\n        asl     #23,b,b");  
   fprintf(fp,"\n        move    b1,y:(TMP+2)");

   fprintf(fp,"\n        move    y:(TMP+6),y0");  
   fprintf(fp,"\n        tfr     b,a");  
   fprintf(fp,"\n        abs     b");  
   fprintf(fp,"\n        clr     b	b1,x0");  
   fprintf(fp,"\n        move    x0,b0");  
   fprintf(fp,"\n        asl     b");  
   fprintf(fp,"\n        rep     #$18");  
   fprintf(fp,"\n        div     y0,b");  
   fprintf(fp,"\n        eor     y0,a");   
   fprintf(fp,"\n        bpl     LBL%ld",label+1);
   fprintf(fp,"\n        neg     b"); 
   fprintf(fp,"\nLBL%ld    nop",label+1);
   fprintf(fp,"\n        move    b0,b"); 
   fprintf(fp,"\n        move    b1,y:(TMP+3)"); 
   fprintf(fp,"\n        move    y:(TMP+7),x0"); 
   fprintf(fp,"\n        add     x0,b"); 
   fprintf(fp,"\n        move    b1,y:(TMP+4)");   

   fprintf(fp,"\n        move    #$00FFFF,x1");
   fprintf(fp,"\n        and      x1,b1");
   fprintf(fp,"\n        move    #$100000,x1");
   fprintf(fp,"\n        or       x1,b                  ; Add amplitude word");
   fprintf(fp,"\n        move    b1,x:A_TX10            ; Send channel info + grad. amplitude to DAC");

// Include a delay to make the step the right length
   fprintf(fp,"\n        move    y:(TMP+9),a1"); 
   fprintf(fp,"\n        sub     #46,a");
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

   fprintf(fp,"\n        move    y:(TMP+5),a0");  
   fprintf(fp,"\n        inc    a"); 
   fprintf(fp,"\n        move    a0,y:(TMP+5)");  
   
  fprintf(fp,"\nLBL%ld    nop",label);

// Include a delay to make the last step the right length
   fprintf(fp,"\n        rep     #80                  ; Wait 1us");
   fprintf(fp,"\n        nop");

   fprintf(fp,"\n        movep   #$04,x:A_PCRD          ; Turn off SSI 1 on Port D");

   label += 2;

   fclose(fp);

   return(OK);
}



/*******************************************************************************

   Make a gradient ramp which has a start and end amplitude and a duration
  
   Syntax   diffgradramp(start, end, nrSteps, stepDuration)
  
   - channel can be a number or a number variable.
   - start and end can be constants or tables.
   - nrSteps and stepDuration can be constants or number variables. 

   This version is for the diffusion gradient controller. See diffgradon/off for more
   details. See the option below to switch to the 20 bit version

*********************************************************************************/

short RampedDiffGradient(DLLParameters* par, char *args)
{
   short nrArgs;
   CText start,end,steps,duration;
   long Steps;
   long Start,End;
   long Address;
   long Duration;
   float fDuration;
   long number,amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,4,"start,end,steps,duration","eeee","qqqq",&start,&end,&steps,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(start[0] == 'n' || start[0] == 't')
      InsertUniqueStringIntoList(start.Str(),&parList,szList);
   if(end[0] == 'n' || end[0] == 't')
      InsertUniqueStringIntoList(end.Str(),&parList,szList);
   if(steps[0] == 'n')
      InsertUniqueStringIntoList(steps.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Generate a ramped diffusion gradient");

// Set the serial port configuration
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred"); 
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");

// Switch on the gradient amp
 //  fprintf(fp,"\n; Switch on gradient amp");
 //  fprintf(fp,"\n        clr     a");
 //  fprintf(fp,"\n        clr     b");
 //  fprintf(fp,"\n        move    #$0001,b1");
 //  fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
 //  fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
	//fprintf(fp,"\n        move    #$200002,a1		         ; DAC control word");
 //  fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
 //  fprintf(fp,"\n        move    #2,r7                   ; Wait 2 us");
 //  fprintf(fp,"\n        bsr     wait");

 //  fprintf(fp,"\n        move    #$01,a1			         ; Enable amplifier");
 //  fprintf(fp,"\n        move    #$0001,b1");
 //  fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
 //  fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0 PE0=1");
 //  fprintf(fp,"\n        move    #$FFFFFF,a1		         ; Grad control word");
 //  fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
 //  fprintf(fp,"\n        move    #2,r7                  ; Wait 20 us");
 //  fprintf(fp,"\n        bsr     wait");

// Select the gradient
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    #$0001,b1");
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");

// Get the start amplitude
   if(start[0] == 'n') // Start amplitude is via a number reference
   {
      fprintf(fp,"\n        move    x:NR%s,a1",start.Str()+1);
      fprintf(fp,"\n        move    a1,y:(TMP+7)");    
   }
   else if(start[0] == 't') // start amplitude is t[index]
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",start.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",start.Str()+1);

      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
      fprintf(fp,"\n        move    a1,y:(TMP+7)");
   }
   else if(sscanf(start.Str(),"%ld",&amplitude) == 1) // Start amplitude is a number
   {
      fprintf(fp,"\n        move    #$%X,a1",amplitude);
      fprintf(fp,"\n        move    a1,y:(TMP+7)");  
   }
   else
   {
      Error(par->itfc,"Invalid start amplitude '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Get the end amplitude
   if(end[0] == 'n') // End amplitude is via a number reference
   {
      fprintf(fp,"\n        move    x:NR%s,a1",end.Str()+1);
      fprintf(fp,"\n        move    a1,y:(TMP+8)");    
   }
   else if(end[0] == 't') // End amplitude is t[index]
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",end.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",end.Str()+1);

      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
      fprintf(fp,"\n        move    a1,y:(TMP+8)");
   }
   else if(sscanf(end.Str(),"%ld",&amplitude) == 1) // End amplitude is a number
   {
      fprintf(fp,"\n        move    #$%X,a1",amplitude);
      fprintf(fp,"\n        move    a1,y:(TMP+8)");  
   }
   else
   {
      Error(par->itfc,"Invalid end amplitude '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Get the number of steps
   if(steps[0] == 'n') // Steps via a number reference
   {
   // Set the level
      fprintf(fp,"\n        move    x:NR%s,a0",steps.Str()+1);
      fprintf(fp,"\n        dec     a");
      fprintf(fp,"\n        move    a0,y:(TMP+6)");    
   }
   else if(sscanf(steps.Str(),"%ld",&number) == 1) // Steps is a number
   {
      if(number > 0)
      {
         fprintf(fp,"\n        move    #%ld,a0",number);
         fprintf(fp,"\n        dec     a");
         fprintf(fp,"\n        move    a0,y:(TMP+6)");  
      }
      else
      {
         Error(par->itfc,"Invalid number of steps '%s'",steps.Str());
         fclose(fp);
         return(ERR);
      }
   }
   else
   {
      Error(par->itfc,"Invalid number of steps '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Step duration
   if(duration[0] == 'd')
   {
		fprintf(fp,"\n        move   x:DELAY%s,a1",duration.Str()+1);
   }
   else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
   {
      if(fDuration < 0.25 || fDuration > 327670)
      {
         Error(par->itfc,"invalid delay '%g' [1...327670]",fDuration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp,"\n        move   #%ld,a1",Duration);
   }
   else
   {
      Error(par->itfc,"Invalid step duration '%s'",duration.Str());
      fclose(fp);
      return(ERR);
   }
   fprintf(fp,"\n        move    a1,y:(TMP+9)");    

// Set the initial n value
   fprintf(fp,"\n        move    #0000,a1");
   fprintf(fp,"\n        move    a1,y:(TMP+5)");    

   fprintf(fp,"\n        move    y:(TMP+6),a0            ; Load the steps into r7");
   fprintf(fp,"\n        inc     a                       ; Load the steps into r7");
   fprintf(fp,"\n        move    a0,r2                   ; Load the steps into r7");

   fprintf(fp,"\n        do      r2,LBL%ld                 ; Calculate each step value",label);

   fprintf(fp,"\n; **************************************************");
   fprintf(fp,"\n; v = n*(end-start)/steps + start");
   fprintf(fp,"\n; **************************************************");
   fprintf(fp,"\n        move    y:(TMP+8),b");  
   fprintf(fp,"\n        move    b1,y:(TMP+2)");  
   fprintf(fp,"\n        move    y:(TMP+7),x1");  
   fprintf(fp,"\n        move    y:(TMP+2),b");  
   fprintf(fp,"\n        sub     x1,b");  
   fprintf(fp,"\n        move    b1,x0");  
   fprintf(fp,"\n        move    y:(TMP+5),y0");

   fprintf(fp,"\n        mpy     y0,x0,b");  
   fprintf(fp,"\n        asl     #23,b,b");  
   fprintf(fp,"\n        move    b1,y:(TMP+2)");

   fprintf(fp,"\n        move    y:(TMP+6),y0");  
   fprintf(fp,"\n        tfr     b,a");  
   fprintf(fp,"\n        abs     b");  
   fprintf(fp,"\n        clr     b	b1,x0");  
   fprintf(fp,"\n        move    x0,b0");  
   fprintf(fp,"\n        asl     b");  
   fprintf(fp,"\n        rep     #$18");  
   fprintf(fp,"\n        div     y0,b");  
   fprintf(fp,"\n        eor     y0,a");   
   fprintf(fp,"\n        bpl     LBL%ld",label+1);
   fprintf(fp,"\n        neg     b"); 
   fprintf(fp,"\nLBL%ld    nop",label+1);
   fprintf(fp,"\n        move    b0,b"); 
   fprintf(fp,"\n        move    b1,y:(TMP+3)"); 
   fprintf(fp,"\n        move    y:(TMP+7),x0"); 
   fprintf(fp,"\n        add     x0,b"); 
   fprintf(fp,"\n        move    b1,y:(TMP+4)");   

 //  fprintf(fp,"\n        move    #$0FFFFF,x1"); // 20 bit
   fprintf(fp,"\n        move    #$00FFFF,x1"); // 16 bit
   fprintf(fp,"\n        and      x1,b1");
   fprintf(fp,"\n        move    #$100000,x1");
   fprintf(fp,"\n        or       x1,b                   ; Add amplitude word");
   fprintf(fp,"\n        move    b1,x:A_TX10             ; Send channel info + grad. amplitude to DAC");

// Include a delay to make the step the right length
   fprintf(fp,"\n        move    y:(TMP+9),a1"); 
   fprintf(fp,"\n        sub     #46,a");
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

   fprintf(fp,"\n        move    y:(TMP+5),a0");  
   fprintf(fp,"\n        inc    a"); 
   fprintf(fp,"\n        move    a0,y:(TMP+5)");  
   
  fprintf(fp,"\nLBL%ld    nop",label);

// Include a delay to make the last step the right length
   fprintf(fp,"\n        rep     #80                     ; Wait 1us");
   fprintf(fp,"\n        nop");

   fprintf(fp,"\n        movep   #$04,x:A_PCRD           ; Turn off SSI 1 on Port D");

   label += 2;

   fclose(fp);

   return(OK);
}

//
//
//short RampedDiffGradient(DLLParameters* par, char *args)
//{
//   short nrArgs;
//   CText start,end,steps,duration;
//   long Steps;
//   long Start,End;
//   long Address;
//   long Duration;
//   float fDuration;
//   long number,amplitude;
//   
//   if((nrArgs = ArgScan(par->itfc,args,4,"start,end,steps,duration","eeee","qqqq",&start,&end,&steps,&duration)) < 0)
//      return(nrArgs);
//
//   if(!parList)
//   {
//      Error(par->itfc,"Pulse sequence not initialised");
//      return(ERR);
//   }
//
//   if(start[0] == 'n' || start[0] == 't')
//      InsertUniqueStringIntoList(start.Str(),&parList,szList);
//   if(end[0] == 'n' || end[0] == 't')
//      InsertUniqueStringIntoList(end.Str(),&parList,szList);
//   if(steps[0] == 'n')
//      InsertUniqueStringIntoList(steps.Str(),&parList,szList);
//   if(duration[0] == 'd')
//      InsertUniqueStringIntoList(duration.Str(),&parList,szList);
//
//   FILE *fp = fopen("midCode.asm","a");
//   if(!fp)
//   {
//      Error(par->itfc,"Can't open output file");
//      return(ERR);
//   }
//
//   fprintf(fp,"\n;");
//   fprintf(fp,"\n;***************************************************************************");
//   fprintf(fp,"\n; Generate a ramped diffusion gradient");
//
//// Set the serial port configuration
//   fprintf(fp,"\n        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D");
//   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred"); 
//   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs");
//
//// Switch on the gradient amp
//   fprintf(fp,"\n; Switch on gradient amp");
//   fprintf(fp,"\n        clr     a");
//   fprintf(fp,"\n        clr     b");
//   fprintf(fp,"\n        move    #$0001,b1");
//   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
//   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
//	fprintf(fp,"\n        move    #$200002,a1		         ; DAC control word");
//   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
//   fprintf(fp,"\n        move    #2,r7                   ; Wait 2 us");
//   fprintf(fp,"\n        bsr     wait");
//
//   fprintf(fp,"\n        move    #$01,a1			         ; Enable amplifier");
//   fprintf(fp,"\n        move    #$0001,b1");
//   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
//   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0 PE0=1");
//   fprintf(fp,"\n        move    #$FFFFFF,a1		         ; Grad control word");
//   fprintf(fp,"\n        move    a1,x:A_TX10             ; Send gradient control word to DAC board");
//   fprintf(fp,"\n        move    #2,r7                  ; Wait 20 us");
//   fprintf(fp,"\n        bsr     wait");
//
//// Select the gradient
//   fprintf(fp,"\n        clr     a");
//   fprintf(fp,"\n        clr     b");
//   fprintf(fp,"\n        move    #$0001,b1");
//   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select grad dac PC0=1");
//   fprintf(fp,"\n        movep   a1,x:A_PDRE             ; Select grad dac PE2=0");
//
//// Get the start amplitude
//   if(start[0] == 'n') // Start amplitude is via a number reference
//   {
//      fprintf(fp,"\n        move    x:NR%s,a1",start.Str()+1);
//      fprintf(fp,"\n        move    a1,y:(TMP+7)");    
//   }
//   else if(start[0] == 't') // start amplitude is t[index]
//   {
//      fprintf(fp,"\n        clr a");
//      fprintf(fp,"\n        clr b");
//      fprintf(fp,"\n        move    x:TABLE%s,a0",start.Str()+1);
//      fprintf(fp,"\n        dec a"); // a0 points to table index
//      fprintf(fp,"\n        move    a0,r5"); // Read current table index
//      fprintf(fp,"\n        move    y:(r5),a0");
//
//      fprintf(fp,"\n        move    x:TABLE%s,b0",start.Str()+1);
//
//      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
//      fprintf(fp,"\n        move    a0,r5"); 
//      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
//      fprintf(fp,"\n        move    a1,y:(TMP+7)");
//   }
//   else if(sscanf(start.Str(),"%ld",&amplitude) == 1) // Start amplitude is a number
//   {
//      fprintf(fp,"\n        move    #$%X,a1",amplitude);
//      fprintf(fp,"\n        move    a1,y:(TMP+7)");  
//   }
//   else
//   {
//      Error(par->itfc,"Invalid start amplitude '%s'",steps.Str());
//      fclose(fp);
//      return(ERR);
//   }
//
//// Get the end amplitude
//   if(end[0] == 'n') // End amplitude is via a number reference
//   {
//      fprintf(fp,"\n        move    x:NR%s,a1",end.Str()+1);
//      fprintf(fp,"\n        move    a1,y:(TMP+8)");    
//   }
//   else if(end[0] == 't') // End amplitude is t[index]
//   {
//      fprintf(fp,"\n        clr a");
//      fprintf(fp,"\n        clr b");
//      fprintf(fp,"\n        move    x:TABLE%s,a0",end.Str()+1);
//      fprintf(fp,"\n        dec a"); // a0 points to table index
//      fprintf(fp,"\n        move    a0,r5"); // Read current table index
//      fprintf(fp,"\n        move    y:(r5),a0");
//
//      fprintf(fp,"\n        move    x:TABLE%s,b0",end.Str()+1);
//
//      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
//      fprintf(fp,"\n        move    a0,r5"); 
//      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
//      fprintf(fp,"\n        move    a1,y:(TMP+8)");
//   }
//   else if(sscanf(end.Str(),"%ld",&amplitude) == 1) // End amplitude is a number
//   {
//      fprintf(fp,"\n        move    #$%X,a1",amplitude);
//      fprintf(fp,"\n        move    a1,y:(TMP+8)");  
//   }
//   else
//   {
//      Error(par->itfc,"Invalid end amplitude '%s'",steps.Str());
//      fclose(fp);
//      return(ERR);
//   }
//
//// Get the number of steps
//   if(steps[0] == 'n') // Steps via a number reference
//   {
//   // Set the level
//      fprintf(fp,"\n        move    x:NR%s,a0",steps.Str()+1);
//      fprintf(fp,"\n        dec     a");
//      fprintf(fp,"\n        move    a0,y:(TMP+6)");    
//   }
//   else if(sscanf(steps.Str(),"%ld",&number) == 1) // Steps is a number
//   {
//      if(number > 0)
//      {
//         fprintf(fp,"\n        move    #%ld,a0",number);
//         fprintf(fp,"\n        dec     a");
//         fprintf(fp,"\n        move    a0,y:(TMP+6)");  
//      }
//      else
//      {
//         Error(par->itfc,"Invalid number of steps '%s'",steps.Str());
//         fclose(fp);
//         return(ERR);
//      }
//   }
//   else
//   {
//      Error(par->itfc,"Invalid number of steps '%s'",steps.Str());
//      fclose(fp);
//      return(ERR);
//   }
//
//// Step duration
//   if(duration[0] == 'd')
//   {
//		fprintf(fp,"\n        move   x:DELAY%s,a1",duration.Str()+1);
//   }
//   else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
//   {
//      if(fDuration < 0.25 || fDuration > 327670)
//      {
//         Error(par->itfc,"invalid delay '%g' [1...327670]",fDuration);
//         fclose(fp);
//         return(ERR);
//      }
//      Duration = (long)(fDuration * 50 - 1 + 0.5);
//      fprintf(fp,"\n        move   #%ld,a1",Duration);
//   }
//   else
//   {
//      Error(par->itfc,"Invalid step duration '%s'",duration.Str());
//      fclose(fp);
//      return(ERR);
//   }
//   fprintf(fp,"\n        move    a1,y:(TMP+9)");    
//
//// Set the initial n value
//   fprintf(fp,"\n        move    #0000,a1");
//   fprintf(fp,"\n        move    a1,y:(TMP+5)");    
//
//   fprintf(fp,"\n        move    y:(TMP+6),a0             ; Load the steps into r7");
//   fprintf(fp,"\n        inc     a                        ; Load the steps into r7");
//   fprintf(fp,"\n        move    a0,r2                    ; Load the steps into r7");
//
//   fprintf(fp,"\n        do      r2,LBL%ld                ; Calculate each step value",label);
//
//   fprintf(fp,"\n; **************************************************");
//   fprintf(fp,"\n; v = n*(end-start)/steps + start");
//   fprintf(fp,"\n; **************************************************");
//   fprintf(fp,"\n        move    y:(TMP+8),b");  
//   fprintf(fp,"\n        move    b1,y:(TMP+2)");  
//   fprintf(fp,"\n        move    y:(TMP+7),x1");  
//   fprintf(fp,"\n        move    y:(TMP+2),b");  
//   fprintf(fp,"\n        sub     x1,b");  
//   fprintf(fp,"\n        move    b1,x0");  
//   fprintf(fp,"\n        move    y:(TMP+5),y0");
//
//   fprintf(fp,"\n        mpy     y0,x0,b");  
//   fprintf(fp,"\n        asl     #23,b,b");  
//   fprintf(fp,"\n        move    b1,y:(TMP+2)");
//
//   fprintf(fp,"\n        move    y:(TMP+6),y0");  
//   fprintf(fp,"\n        tfr     b,a");  
//   fprintf(fp,"\n        abs     b");  
//   fprintf(fp,"\n        clr     b	b1,x0");  
//   fprintf(fp,"\n        move    x0,b0");  
//   fprintf(fp,"\n        asl     b");  
//   fprintf(fp,"\n        rep     #$18");  
//   fprintf(fp,"\n        div     y0,b");  
//   fprintf(fp,"\n        eor     y0,a");   
//   fprintf(fp,"\n        bpl     LBL%ld",label+1);
//   fprintf(fp,"\n        neg     b"); 
//   fprintf(fp,"\nLBL%ld    nop",label+1);
//   fprintf(fp,"\n        move    b0,b"); 
//   fprintf(fp,"\n        move    b1,y:(TMP+3)"); 
//   fprintf(fp,"\n        move    y:(TMP+7),x0"); 
//   fprintf(fp,"\n        add     x0,b"); 
//   fprintf(fp,"\n        move    b1,y:(TMP+4)");   
//
//   fprintf(fp,"\n        move    #$0FFFFF,x1");
//   fprintf(fp,"\n        and      x1,b1");
//   fprintf(fp,"\n        move    #$100000,x1");
//   fprintf(fp,"\n        or       x1,b                  ; Add amplitude word");
//   fprintf(fp,"\n        move    b1,x:A_TX10            ; Send channel info + grad. amplitude to DAC");
//
//// Include a delay to make the step the right length
//   fprintf(fp,"\n        move    y:(TMP+9),a1"); 
//   fprintf(fp,"\n        sub     #46,a");
//   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
//	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
//   fprintf(fp,"\n        nop");
//   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
//   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
//
//   fprintf(fp,"\n        move    y:(TMP+5),a0");  
//   fprintf(fp,"\n        inc    a"); 
//   fprintf(fp,"\n        move    a0,y:(TMP+5)");  
//   
//  fprintf(fp,"\nLBL%ld    nop",label);
//
//// Include a delay to make the last step the right length
//   fprintf(fp,"\n        rep     #80                  ; Wait 1us");
//   fprintf(fp,"\n        nop");
//
//   fprintf(fp,"\n        movep   #$04,x:A_PCRD          ; Turn off SSI 1 on Port D");
//
//   label += 2;
//
//   fclose(fp);
//
//   return(OK);
//}

/*******************************************************************************

   Make a gradient ramp which has a start and end amplitude and a duration
  
   Syntax   gradramp16(channel, start, end, nrSteps, stepDuration)
  
   - channel can be a number or a number variable.
   - start and end can be constants or tables.
   - nrSteps and stepDuration can be constants or number variables. 

   This version is for the 16 channel gradient controller. See shim16 for more
   details.

*********************************************************************************/

short RampedShim16(DLLParameters* par, char *args)
{
   short nrArgs;
   CText grad,start,end,steps,duration;
   long Steps;
   long Start,End;
   long Address;
   long Duration;
   float fDuration;
   long number,amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,5,"gradient,start,end,steps,duration","eeeee","qqqqq",&grad,&start,&end,&steps,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   if(grad[0] == 'n')
      InsertUniqueStringIntoList(grad.Str(),&parList,szList);
   if(start[0] == 'n' || start[0] == 't')
      InsertUniqueStringIntoList(start.Str(),&parList,szList);
   if(end[0] == 'n' || end[0] == 't')
      InsertUniqueStringIntoList(end.Str(),&parList,szList);
   if(steps[0] == 'n')
      InsertUniqueStringIntoList(steps.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Generate a ramped gradient (16 channel)");

// Set the serial port configuration
   fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");
   fprintf(fp,"\n        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred"); 
 //  fprintf(fp,"\n        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs"); 
   fprintf(fp,"\n        movep   #$01343C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs"); 

// Select the gradient channel
   if(SelectShim16(par->itfc,fp,grad)) return(ERR);

   fprintf(fp,"\n        move    b0,b1                  ; Restore subgroup"); 
   fprintf(fp,"\n        lsl     #16,b                  ; Move into correct format for DAC");    // Shift 16 bits to left
   fprintf(fp,"\n        move    #$030000,x1");               // Extract lower 2 bits of channel number
   fprintf(fp,"\n        and      x1,b"); 
   fprintf(fp,"\n        move    #$100000,x1");               // Set data register mode bit
   fprintf(fp,"\n        or       x1,b"); 
   fprintf(fp,"\n        move    b1,y:(TMP+10)          ; Save");

// Get the start amplitude
   if(start[0] == 'n') // Start amplitude is via a number reference
   {
      fprintf(fp,"\n        move    x:NR%s,a1",start.Str()+1);
      fprintf(fp,"\n        move    a1,y:(TMP+7)");    
   }
   else if(start[0] == 't') // start amplitude is t[index]
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",start.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",start.Str()+1);

      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
      fprintf(fp,"\n        move    a1,y:(TMP+7)");
   }
   else if(sscanf(start.Str(),"%ld",&amplitude) == 1) // Start amplitude is a number
   {
      fprintf(fp,"\n        move    #$%X,a1",amplitude);
      fprintf(fp,"\n        move    a1,y:(TMP+7)");  
   }
   else
   {
      Error(par->itfc,"Invalid start amplitude '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Get the end amplitude
   if(end[0] == 'n') // End amplitude is via a number reference
   {
      fprintf(fp,"\n        move    x:NR%s,a1",end.Str()+1);
      fprintf(fp,"\n        move    a1,y:(TMP+8)");    
   }
   else if(end[0] == 't') // End amplitude is t[index]
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",end.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",end.Str()+1);

      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
      fprintf(fp,"\n        move    a1,y:(TMP+8)");
   }
   else if(sscanf(end.Str(),"%ld",&amplitude) == 1) // End amplitude is a number
   {
      fprintf(fp,"\n        move    #$%X,a1",amplitude);
      fprintf(fp,"\n        move    a1,y:(TMP+8)");  
   }
   else
   {
      Error(par->itfc,"Invalid end amplitude '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Get the number of steps
   if(steps[0] == 'n') // Steps via a number reference
   {
   // Set the level
      fprintf(fp,"\n        move    x:NR%s,a0",steps.Str()+1);
      fprintf(fp,"\n        dec     a");
      fprintf(fp,"\n        move    a0,y:(TMP+6)");    
   }
   else if(sscanf(steps.Str(),"%ld",&number) == 1) // Steps is a number
   {
      if(number > 0)
      {
         fprintf(fp,"\n        move    #%ld,a0",number);
         fprintf(fp,"\n        dec     a");
         fprintf(fp,"\n        move    a0,y:(TMP+6)");  
      }
      else
      {
         Error(par->itfc,"Invalid number of steps '%s'",steps.Str());
         fclose(fp);
         return(ERR);
      }
   }
   else
   {
      Error(par->itfc,"Invalid number of steps '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Step duration
   if(duration[0] == 'd')
   {
		fprintf(fp,"\n        move   x:DELAY%s,a1",duration.Str()+1);
   }
   else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
   {
      if(fDuration < 0.25 || fDuration > 327670)
      {
         Error(par->itfc,"invalid delay '%g' [1...327670]",fDuration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp,"\n        move   #%ld,a1",Duration);
   }
   else
   {
      Error(par->itfc,"Invalid step duration '%s'",duration.Str());
      fclose(fp);
      return(ERR);
   }
   fprintf(fp,"\n        move    a1,y:(TMP+9)");    

// Set the initial n value
   fprintf(fp,"\n        move    #0000,a1");
   fprintf(fp,"\n        move    a1,y:(TMP+5)");    

   fprintf(fp,"\n        move    y:(TMP+6),a0             ; Load the steps into r7");
   fprintf(fp,"\n        inc     a                        ; Load the steps into r7");
   fprintf(fp,"\n        move    a0,r2                    ; Load the steps into r7");

   fprintf(fp,"\n        do      r2,LBL%ld                ; Calculate each step value",label);

   fprintf(fp,"\n; **************************************************");
   fprintf(fp,"\n; v = n*(end-start)/steps + start");
   fprintf(fp,"\n; **************************************************");
   fprintf(fp,"\n        move    y:(TMP+8),b");  
   fprintf(fp,"\n        move    b1,y:(TMP+2)");  
   fprintf(fp,"\n        move    y:(TMP+7),x1");  
   fprintf(fp,"\n        move    y:(TMP+2),b");  
   fprintf(fp,"\n        sub     x1,b");  
   fprintf(fp,"\n        move    b1,x0");  
   fprintf(fp,"\n        move    y:(TMP+5),y0");

   fprintf(fp,"\n        mpy     y0,x0,b");  
   fprintf(fp,"\n        asl     #23,b,b");  
   fprintf(fp,"\n        move    b1,y:(TMP+2)");

   fprintf(fp,"\n        move    y:(TMP+6),y0");  
   fprintf(fp,"\n        tfr     b,a");  
   fprintf(fp,"\n        abs     b");  
   fprintf(fp,"\n        clr     b	b1,x0");  
   fprintf(fp,"\n        move    x0,b0");  
   fprintf(fp,"\n        asl     b");  
   fprintf(fp,"\n        rep     #$18");  
   fprintf(fp,"\n        div     y0,b");  
   fprintf(fp,"\n        eor     y0,a");   
   fprintf(fp,"\n        bpl     LBL%ld",label+1);
   fprintf(fp,"\n        neg     b"); 
   fprintf(fp,"\nLBL%ld    nop",label+1);
   fprintf(fp,"\n        move    b0,b"); 
   fprintf(fp,"\n        move    b1,y:(TMP+3)"); 
   fprintf(fp,"\n        move    y:(TMP+7),x0"); 
   fprintf(fp,"\n        add     x0,b"); 
   fprintf(fp,"\n        move    b1,y:(TMP+4)");   
   fprintf(fp,"\n        move    #$00FFFF,x1"); 
   fprintf(fp,"\n        and      x1,b1"); 
   fprintf(fp,"\n        move     y:(TMP+10),a1");
   fprintf(fp,"\n        move     a1,x1"); 
   fprintf(fp,"\n        or       x1,b                  ; Add amplitude word"); 

   fprintf(fp,"\n        movep   b1,x:A_TX10"); // Send to gradient board


// Include a delay to make the step the right length
   fprintf(fp,"\n        move    y:(TMP+9),a1"); 
   fprintf(fp,"\n        sub     #46,a");
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

   fprintf(fp,"\n        move    y:(TMP+5),a0");  
   fprintf(fp,"\n        inc    a"); 
   fprintf(fp,"\n        move    a0,y:(TMP+5)");  
   
  fprintf(fp,"\nLBL%ld    nop",label);

// Include a delay to make the last step the right length
   fprintf(fp,"\n        rep     #80                  ; Wait 1us");
   fprintf(fp,"\n        nop");

   fprintf(fp,"\n        movep   #$04,x:A_PCRD          ; Turn off SSI 1 on Port D");

   label += 2;

   fclose(fp);

   return(OK);
}

// Select the appropriate shim. The mapping is a bit complex
// Here channel is the shim channel (0-31) and A0,A1 and A2 are the 
// address pins on the 74HC138 selector chip
//
// If Channel number is < 16 A0 = 1 if >=16 A0 = 0

// Channel A0  A1  A2
//    0     x   0   1
//    1     x   0   1
//    2     x   0   1
//    3     x   0   1
//    4     x   1   1
//    5     x   1   1
//    6     x   1   1
//    7     x   1   1
//    8     x   0   0
//    9     x   0   0
//    A     x   0   0
//    B     x   0   0
//    C     x   1   0
//    D     x   1   0
//    E     x   1   0
//    F     x   1   0

short SelectShim16(Interface *itfc ,FILE* fp, CText grad)
{
   long Grad;
   long Channel;

// Choose the gradient channel
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");

   if(grad[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",grad.Str()+1);
   }
   else if(sscanf(grad.Str(),"%ld",&Channel) == 1) // channel is a number
   {
      if(Channel < 0 || Channel > 31)
      {
         Error(itfc,"invalid gradient index '%s' [0, ... 31]",grad.Str());
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #%ld,a1",Channel);
   }
	else
   {
      Error(itfc,"invalid gradient index '%s' [0, ... 31]",grad.Str());
      fclose(fp);
      return(ERR);
   }

   fprintf(fp,"\n        cmp    #16,a                    ; See if first board or second board");
   fprintf(fp,"\n        jlt    LBL%ld",label+1);

 // Top 16 channels
   fprintf(fp,"\n        sub    #16,a                    ; Subtract 16 (result 0-15)");
   fprintf(fp,"\n        move    #$0001,b1               ; Assume first group");
   fprintf(fp,"\n        cmp    #8,a                     ; See if channel is < 8");
   fprintf(fp,"\n        jlt    LBL%ld",label);
   fprintf(fp,"\n        sub    #8,a                     ; Subtract 8");
   fprintf(fp,"\n        move    #$0000,b1               ; Second group");
   fprintf(fp,"\nLBL%ld    nop",label);
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select group of 8 DACs");

   fprintf(fp,"\n        move    a1,b0                   ; Save subgroup (4 channels)");
   fprintf(fp,"\n        lsr     #2,a                    ; Shift 2 bits to right to determine subgroup (result 0/1)");                  
   fprintf(fp,"\n        move    a1,x:A_PDRE             ; Select block of 4 DACs: pins Y0 - Y3 on U7");
   fprintf(fp,"\n        jmp    LBL%ld",label+3);      

 // Bottom 16 channels
   fprintf(fp,"\nLBL%ld    nop",label+1);
   fprintf(fp,"\n        move    #$0001,b1               ; Assume first group");
   fprintf(fp,"\n        cmp    #8,a                     ; See if channel is < 8");
   fprintf(fp,"\n        jlt    LBL%ld",label+2);
   fprintf(fp,"\n        sub    #8,a                     ; Subtract 8");
   fprintf(fp,"\n        move    #$0000,b1               ; Second group");
   fprintf(fp,"\nLBL%ld    nop",label+2);
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select group of 8 DACs");

   fprintf(fp,"\n        move    a1,b0                   ; Save subgroup (4 channels)");
   fprintf(fp,"\n        lsr     #2,a                    ; Shift 2 bits to right to determine subgroup (result 0/1)");                
   fprintf(fp,"\n        add     #4,a                    ; Add 4 to access 4 DACs: pins Y4 - Y7 on U7"); 
   fprintf(fp,"\n        move    a1,x:A_PDRE             ; Select block of 4 DACs");
   fprintf(fp,"\nLBL%ld    nop",label+3);

   label += 4;

   return(OK);
}

//
//
//short ResetShims(DLLParameters* par, char *args)
//{
//   short nrArgs;
//   CText amplitude;
//	long Amplitude;
//   
//   if((nrArgs = ArgScan(par->itfc,args,1,"reset amplitude","e","q",&amplitude)) < 0)
//      return(nrArgs);
//
//   if(!parList)
//   {
//      Error(par->itfc,"Pulse sequence not initialised");
//      return(ERR);
//   }
//
//// Add to parameter list if not a constant
//   if(amplitude[0] == 'n' || amplitude[0] == 't')
//      InsertUniqueStringIntoList(amplitude.Str(),&parList,szList);
//
//   FILE *fp = fopen("midCode.asm","a");
//   if(!fp)
//   {
//      Error(par->itfc,"Can't open output file");
//      return(ERR);
//   }
//
//   fprintf(fp,"\n;");
//   fprintf(fp,"\n;***************************************************************************");
//
//    
//   fprintf(fp,"\n; Initialise 32 channel gradient board(s)"); 
//   fprintf(fp,"\n        movep   #$2C,x:A_PCRD             ; Turn on SSI 1 on Port D");
//   fprintf(fp,"\n        movep   #$180802,x:A_CRA1         ; /2 clk, 24 bit word transferred");
//   fprintf(fp,"\n        movep   #$01343C,x:A_CRB1         ; Enable SSI port with sc1/2 are outputs");
//   fprintf(fp,"\n        move    #32,r7                  ; Loop over 32 shims");
//   fprintf(fp,"\n        move    #$00,r5                 ; Shim counter");
//   fprintf(fp,"\n;    Loop over the shims");
//   fprintf(fp,"\n        do      r7,G32LB5               ; Start the shim loop (r5: 0-31)");
//   fprintf(fp,"\n        clr     a");
//   fprintf(fp,"\n        clr     b");
//   fprintf(fp,"\n        move    r5,a1");
//   fprintf(fp,"\n        cmp    #16,a                    ; See if first board or second board");
//   fprintf(fp,"\n        jlt    G32LB2");   
//   fprintf(fp,"\n;    Select shims 16-31");   
//   fprintf(fp,"\n        sub    #16,a                    ; Subtract 16 (result 0-15)");
//   fprintf(fp,"\n        move    #$0001,b1               ; Assume first group");
//   fprintf(fp,"\n        cmp    #8,a                     ; See if channel is < 8");
//   fprintf(fp,"\n        jlt    G32LB1");
//   fprintf(fp,"\n        sub    #8,a                     ; Subtract 8");
//   fprintf(fp,"\n        move    #$0000,b1               ; Second group");
//   fprintf(fp,"\nG32LB1  nop");
//   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select group of 8 DACs");
//   fprintf(fp,"\n        move    a1,b0                   ; Save subgroup (4 channels)");
//   fprintf(fp,"\n        lsr     #2,a                    ; Shift 2 bits to right to determine subgroup (result 0/1)");
//   fprintf(fp,"\n        move    a1,x:A_PDRE             ; Select block of 4 DACs");
//   fprintf(fp,"\n        jmp     G32LB4");
//
//   fprintf(fp,"\n;    Select shims 0-15");        
//   fprintf(fp,"\nG32LB2  nop"); 
//   fprintf(fp,"\n        move    #$0001,b1               ; Assume first group");
//   fprintf(fp,"\n        cmp    #8,a                     ; See if channel is < 8");
//   fprintf(fp,"\n        jlt    G32LB3");
//   fprintf(fp,"\n        sub    #8,a                     ; Subtract 8");
//   fprintf(fp,"\n        move    #$0000,b1               ; Second group");
//   fprintf(fp,"\nG32LB3  nop");
//   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select group of 8 DACs");
//   fprintf(fp,"\n        move    a1,b0                   ; Save subgroup (4 channels)");
//   fprintf(fp,"\n        lsr     #2,a                    ; Shift 2 bits to right to determine subgroup (result 0/1)");
//   fprintf(fp,"\n        add     #4,a                    ; Add 4 to only access pins Y4 or Y5 on U7");
//   fprintf(fp,"\n        move    a1,x:A_PDRE             ; Select block of 4 DACs");
//        
//   fprintf(fp,"\n;    Zero the shim");
//   fprintf(fp,"\nG32LB4  nop");      
//
//   if(amplitude[0] != 'n')
//   {
//      if(sscanf(amplitude.Str(),"%ld",&Amplitude) == 1)
//      {
//         if(Amplitude > 32768 || Amplitude < -32767)
//         {
//            Error(par->itfc,"invalid shim amplitude '%s' [-32767 ... 32768]",amplitude.Str());
//            fclose(fp);
//            return(ERR);
//         }
//      }
//	   else
//      {
//         Error(par->itfc,"invalid shim amplitude '%s' [-32767 ... 32768]",amplitude.Str());
//         fclose(fp);
//         return(ERR);
//      }
//      fprintf(fp,"\n        move    #$%X,a1           ; Get gradient amplitude",Amplitude); 
//   }
//   else
//   {
//      fprintf(fp,"\n        move    x:NR%s,a1                ; Get gradient amplitude",amplitude.Str()+1); 
//   }
//
// //  fprintf(fp,"\n        move    #$0,a1                  ; Get gradient amplitude");
//   fprintf(fp,"\n        move    #$00FFFF,x1");
//   fprintf(fp,"\n        and      x1,a1");
//   fprintf(fp,"\n        move    b0,b1                   ; Restore subgroup");
//   fprintf(fp,"\n        lsl     #16,b                   ; Move into correct format for DAC");
//   fprintf(fp,"\n        move    #$030000,x1");
//   fprintf(fp,"\n        and      x1,b");
//   fprintf(fp,"\n        move    #$100000,x1");
//   fprintf(fp,"\n        or       x1,b");
//   fprintf(fp,"\n        move     b1,x1");
//   fprintf(fp,"\n        or       x1,a                   ; Add amplitude word");
//   fprintf(fp,"\n        move    a1,x:A_TX10               ; Send channel info + grad. amplitude to DAC");
//   fprintf(fp,"\n        rep     #500                    ; Wait 5 us");
//   fprintf(fp,"\n        nop"); 
//   fprintf(fp,"\n        clr     b");
//   fprintf(fp,"\n        move    r5,b1                   ; Next gradient");
//   fprintf(fp,"\n        add     #1,b");
//   fprintf(fp,"\n        move    b1,r5"); 
//   fprintf(fp,"\nG32LB5  nop");
//   fprintf(fp,"\n        movep   #$04,x:A_PCRD             ; Turn off SSI 1 on Port D (prevents serial noise)");
//
//
//	fclose(fp);
//  
//   return(OK);
//}



short DSPTest(DLLParameters* par, char *args)
{
   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }
	fprintf(fp,"\n\n;***************************************************************************");
   fprintf(fp,"\n; DSP test");
   fprintf(fp,"\n        move    #$000001,a1             ; Test amplitude"); 
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Zero phase");
	  // fprintf(fp,"\n        nop");
   //fprintf(fp,"\n        nop");
   //fprintf(fp,"\n        nop");
   //fprintf(fp,"\n        nop");
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Zero phase");

	fclose(fp);

	return(0);

}
