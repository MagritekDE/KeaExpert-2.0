/*************************************************************
     Kea Pulse Programmer for FPGA transceiver board

Provides DLL commands to generate pulse sequences for the 
Kea spectrometer.

*************************************************************/

#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <shellapi.h>


#define VERSION 2.28 // 27.1.22

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Version history
//
//
// 2.28 -----------------------------
// 1. Added dual channel shaped RF pulse shapedrf2
// 
// 2.27 -----------------------------
// 1. Updated the acquireon/off commands to match Spinsolve code
// 
// 2.26 -----------------------------
//
// 1. Added table support to the delays in the acquire command.
//
// 2.25 -----------------------------
//
// 1. Added new commands acquireon and acquireoff which allow other p.s. commands while acquiring
//
// 2.24 -----------------------------
//
// 1. Added new commands ttldupon and ttldupoff to toggle TTL5 for the LF duplexer.
//
// 2.23 -----------------------------
//
// 1. Updated gradramp command to change 1-4 gradients simultaneously
//
// 2.22 -----------------------------
//
// 1. Added new command AcquireLongData which averages a number of points together to allow effectively longer
//    dwell times.
// 2. Added missing options for wobble in txon.
//
// 2.21 -----------------------------
//
// 1. Added settxfreqs commands so both channel frequencies can be set.
// 2. Remove code at end of RF pulses which is resetting the phase and frequency as this took up time.
//    however this does mean that the default frequency is then changed after a pulse which sets the frequency
//    and also that the total pulse command duration is shorter.
// 3. Added a line to SelectDuration which prevents jitter when the duration is a constant.
// 4. Added table option to RF amplitude
//
// 2.20 -----------------------------
// 
// 1. Fixed bug in txon for two channels. The second ch. phase and frequency were always the same as the first.
// 2. Fixed typo in syntax help for chirprf (was chirpedrf).
//
// 2.19 -----------------------------
//
// 1. Addition of nops to the AcquireData command to prevent phase jitter in the RF pulses
//
// 2.18 -----------------------------
//
// 1. Added gradramp16 command
// 2. Simplified shim16 ASM code - increased serial transfer rate to allow 2 us steps.
// 3. Modified AcquireData and ResetMemoryPointer to always store the data address in DATA_ADRS rather than r5
//
// 2.17 -----------------------------
//
// 1. Fixed bug in memreset which prevented it from working when it had no arguments.
// 2. Added chirprf command (ported from Kea 1 code).
//
// 2.16 -----------------------------
//
// 1. Add the shim16 command for use with the ultracompact spectrometer
//
// 2.15 -----------------------------
// 1. Corrected bug in setrxgain and setlfrxgain. The serial interface was not being reset correctly.
//
// 2.14 -----------------------------
//
// 1. Updated all 15 channel commands to include the 32768 offset required to give correct zero.
// 2. Fixed bug in 15 channel gradient commands which was preventing the second bank (8-15) from being accessed reliably
// 3. Add shaped gradient command for the 15 channel controller.
//
// 2.13 -----------------------------
//
// 1. Got setrxfreq command working correctly
//
// 2.12 -----------------------------
//
// 1. Added code for 15 channel gradient controller ramp (gradramp15).
// 2. Added nop command for experimentation purposes.
// 3. In the set rx gain command increased the wait after updating each gain setting to 10 us. (Necessary?)
// 4. SetTxFreq now has a channel option.
// 5. The phase in the pulse command can now be a table.
// 6. Shaped pulse tidied up so there is only one version.
// 7. SwitchOnOneTxChannel and SwitchOnBothTxChannels can now use phase tables.
// 8. MakeADelay can now use a table.
//
// 2.11 -----------------------------
//
// 1. Add grad15 command to control gradients on the new 15 channel controller
// 2. Added jitter parameters to core parameter list.
//
// 2.10 -----------------------------
// 
// 1. Simplified TTL code for switching RF pulses.
// 2. Allowed shaped pulses for channel 2.
// 3. Removed shaped pulse option which uses 5 arguments.
//
///////////////////////////////////////////////////////////////////////////////////////////////////


// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);
//short UpdateTx(DLLParameters*,char *args);
short SelectAmplitude(Interface *itfc ,FILE* fp, CText amp, CText channel);
short SelectDuration(Interface *itfc ,FILE* fp, char *reg, CText duration);
short GetGradientNumber(Interface *itfc , FILE *fp, CText grad);
short GetGradientAmplitude(Interface *itfc , FILE *fp, CText var);
short SelectGradient(Interface *itc, FILE *fp, CText grad);
short SelectGradient15(Interface *itfc ,FILE* fp, CText grad);
short SelectGradient16(Interface *itfc ,FILE* fp, CText grad);
short SelectNumber(Interface *itfc ,FILE* fp, CText num, char *dst, long min, long max);
short SelectPhase(Interface *itfc ,FILE* fp, CText phase, CText channel);
short AcquireData(DLLParameters*,char *args);
short AcquireLongData(DLLParameters*,char *args);
short BothChannelsRFPulse(DLLParameters*,char *args);
short ChannelOneRFPulse(DLLParameters*,char *args);
short ClearData(DLLParameters*,char *args);
short DecrementTableIndex(DLLParameters*,char *args);
short EitherChannelRFPulse(DLLParameters*,char *args);
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
short PulseGradient(DLLParameters*,char*);
short RampedGradient(DLLParameters* par, char *args);
short RampedGradients(DLLParameters* par, char *args);
short ResetMemoryPointer(DLLParameters*,char *args);
short SetLowFreqOffset(DLLParameters* par, char *args);
//short SetLowFreqOffset2(DLLParameters* par, char *args);
short SetLowFreqRxGain(DLLParameters*,char *args);
short SetRxFreq(DLLParameters*,char *args);
short SetRxGain(DLLParameters*,char *args);
short SetTableIndex(DLLParameters*,char *args);
short SetTTL(DLLParameters*,char *args);
short SetTxFreq(DLLParameters*,char *args);
short SetTxFreqs(DLLParameters* par, char *args);
short ShapedGradientPulse(DLLParameters*,char *args);
short ShapedRF(DLLParameters*,char *args);
short SwitchOffGradient(DLLParameters*,char*);
short SwitchOffShim(DLLParameters*,char*);
short SwitchOffTx(DLLParameters*,char *args);
short SwitchOnBothTxChannels(DLLParameters*,char *args);
short SwitchOnGradient(DLLParameters*,char*);
short SwitchOnOneTxChannel(DLLParameters*,char *args, short nrargs);
short SwitchOnTx(DLLParameters*,char *args);
short TTLOff(DLLParameters*,char *args);
short TTLDuplexerOff(DLLParameters*,char *args);
short TTLOn(DLLParameters*,char *args);
short TTLDuplexerOn(DLLParameters*,char *args);
short TTLPulse(DLLParameters*,char*);
short TTLTranslate(DLLParameters*,char *args);
short UpdateFrequencies(DLLParameters*,char *args);
short WaitForTrigger(DLLParameters*,char *args);
void InsertUniqueStringIntoList(char *str, char ***list, long &position);
short SelectFrequency(Interface *itfc, FILE*fp, CText freq, CText channel);
short NoOperation(DLLParameters* par, char *args);
short CloseHandle(DLLParameters* par, char *args);
short ChirpedRF(DLLParameters* par, char *args);
short AcquireDataOn(DLLParameters* par, char *args);
short AcquireDataOff(DLLParameters* par, char *args);
short ShapedRFDualChannel(DLLParameters* par, char* args);
short SkipOnZero(DLLParameters*,char*);
short SkipEnd(DLLParameters*,char*);

char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list
long label;     // label counter (to generate unique label)

// Extension procedure to add commands to Prospa 

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;
      
        if (!strcmp(command, "acqulong"))        r = AcquireLongData(dpar, parameters);
   else if (!strcmp(command, "acquire"))         r = AcquireData(dpar, parameters);
   else if (!strcmp(command, "acquireon"))       r = AcquireDataOn(dpar, parameters);
   else if (!strcmp(command, "acquireoff"))      r = AcquireDataOff(dpar, parameters);
   else if (!strcmp(command, "chirprf"))         r = ChirpedRF(dpar, parameters);
   else if (!strcmp(command, "cleardata"))       r = ClearData(dpar, parameters);
   else if (!strcmp(command, "delay"))           r = MakeADelay(dpar, parameters);
   else if (!strcmp(command, "decindex"))        r = DecrementTableIndex(dpar, parameters);
   else if (!strcmp(command, "endloop"))         r = LoopEnd(dpar, parameters);
   else if (!strcmp(command, "endiftrue"))       r = SkipEnd(dpar, parameters);
   else if (!strcmp(command, "endskip"))         r = SkipEnd(dpar, parameters);
   else if (!strcmp(command, "endpp"))           r = EndPP(dpar, parameters);
   else if (!strcmp(command, "execwait"))        r = ExecuteAndWait(dpar, parameters);
   else if (!strcmp(command, "gradon"))          r = SwitchOnGradient(dpar, parameters);
   else if (!strcmp(command, "gradoff"))         r = SwitchOffGradient(dpar, parameters);
   else if (!strcmp(command, "gradramp"))        r = RampedGradients(dpar, parameters);
   else if (!strcmp(command, "helpfolder"))      r = GetHelpFolder(dpar, parameters);
   else if(!strcmp(command,  "iftrue"))          r = SkipOnZero(dpar, parameters);
   else if (!strcmp(command, "incindex"))        r = IncrementTableIndex(dpar, parameters);
   else if (!strcmp(command, "inctxamp"))        r = IncTxAmplitude(dpar, parameters);
   else if (!strcmp(command, "incrxfreq"))       r = IncRxFrequency(dpar, parameters);
   else if (!strcmp(command, "inctxfreq"))       r = IncTxFrequency(dpar, parameters);
   else if (!strcmp(command, "initpp"))          r = InitialisePP(dpar, parameters);
   else if (!strcmp(command, "loop"))            r = LoopStart(dpar, parameters);
   else if (!strcmp(command, "memreset"))        r = ResetMemoryPointer(dpar, parameters);
   else if (!strcmp(command, "nop"))             r = NoOperation(dpar, parameters);
   else if (!strcmp(command, "ppversion"))       r = GetPPVersion(dpar, parameters);
   else if (!strcmp(command, "pulse"))           r = MakeAnRFPulse(dpar, parameters);
   else if (!strcmp(command, "setindex"))        r = SetTableIndex(dpar, parameters);
   else if (!strcmp(command, "setrxfreq"))       r = SetRxFreq(dpar, parameters);
   else if (!strcmp(command, "setrxgain"))       r = SetRxGain(dpar, parameters);
   else if (!strcmp(command, "setlfrxgain"))     r = SetLowFreqRxGain(dpar, parameters);
   else if (!strcmp(command, "setlfrxoffset"))   r = SetLowFreqOffset(dpar, parameters);
   else if (!strcmp(command, "settxfreq"))       r = SetTxFreq(dpar, parameters);
   else if (!strcmp(command, "settxfreqs"))      r = SetTxFreqs(dpar, parameters);
   else if (!strcmp(command, "shapedgrad"))      r = ShapedGradientPulse(dpar, parameters);
   else if (!strcmp(command, "shapedrf"))        r = ShapedRF(dpar, parameters);
   else if (!strcmp(command, "shapedrf2"))       r = ShapedRFDualChannel(dpar, parameters);
   else if (!strcmp(command, "skiponzero"))      r = SkipOnZero(dpar, parameters);
   else if (!strcmp(command, "skiponfalse"))     r = SkipOnZero(dpar, parameters);
   else if (!strcmp(command, "ttl"))             r = TTLOn(dpar, parameters);
   else if (!strcmp(command, "ttlon"))           r = TTLOn(dpar, parameters);
   else if (!strcmp(command, "ttldupon"))        r = TTLDuplexerOn(dpar, parameters);
   else if (!strcmp(command, "ttloff"))          r = TTLOff(dpar, parameters);
   else if (!strcmp(command, "ttldupoff"))       r = TTLDuplexerOff(dpar, parameters);
   else if (!strcmp(command, "ttlpulse"))        r = TTLPulse(dpar, parameters);
   else if (!strcmp(command, "ttltranslate"))    r = TTLTranslate(dpar, parameters);
   else if (!strcmp(command, "trigger"))         r = WaitForTrigger(dpar, parameters);
   else if (!strcmp(command, "txoff"))           r = SwitchOffTx(dpar, parameters);
   else if (!strcmp(command, "txon"))            r = SwitchOnTx(dpar, parameters);
   else if (!strcmp(command, "wait"))            r = MakeALongDelay(dpar, parameters);
    
                
   return(r);
}

// Extension procedure to list commands in DLL 

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   Kea-2 Pulse Programmer DLL module (V%1.2f)\n\n",VERSION);
   TextMessage("   acqlong ..... acquire some data with extra long dwell-times\n");
   TextMessage("   acquire ..... acquire some data\n");
   TextMessage("   acquireon ... start acquiring some data\n");
   TextMessage("   acquireoff .. complete acquiring the data\n");
   TextMessage("   chirprf ..... make a frequency and amplitude moduated RF pulse\n");
   TextMessage("   cleardata ... clear data memory\n");
   TextMessage("   decindex .... decrement a table index\n");
   TextMessage("   delay ....... generate a short delay\n");
   TextMessage("   endloop ..... end a loop\n");
   TextMessage("   endpp ....... finish the pulse program\n");
   TextMessage("   endiftrue ... end a iftrue block\n");
   TextMessage("   endskip ..... end a skip\n");
   TextMessage("   execwait .... execute a program and wait for it to exit\n");
   TextMessage("   gradon ...... set a gradient\n");
   TextMessage("   gradoff ..... zero a gradient\n");
   TextMessage("   gradramp .... change one or more gradients using a linear ramp (4 channel)\n");
   TextMessage("   incrxfreq ... increment the rx frequency\n");
   TextMessage("   inctxfreq ... increment the tx frequency\n");
   TextMessage("   initpp ...... initialise pulse program\n");
   TextMessage("   incindex .... increment a table index\n");
   TextMessage("   inctxamp .... increment tx amplitude\n");
   TextMessage("   loop ........ start a loop\n");
   TextMessage("   memreset .... reset memory pointer\n");
   TextMessage("   ppversion ... returns the version number of this DLL\n");
   TextMessage("   pulse ....... generate an RF pulse\n");
   TextMessage("   setindex .... set a table index\n");
   TextMessage("   setrxfreq ... set the receive frequency\n");
   TextMessage("   setlfrxgain . set the low frequency receive amplifier gain\n");
   TextMessage("   setlfrxoffset set the low frequency receive amplifier offset\n");
   TextMessage("   setrxgain ... set the receive amplifier gain\n");
   TextMessage("   settxfreq ... set the pulse frequency\n");
   TextMessage("   settxfreqs .. set the pulse frequencies for both channels\n");
   TextMessage("   shapedrf .... make a phase and amplitude moduated RF pulse\n");
   TextMessage("   shapedrf2 ... make a dual channel phase and amplitude moduated RF pulse\n");
   TextMessage("   shapedgrad .. make an amplitude modulated gradient pulse (4 channel controller)\n");
   TextMessage("   ttlon ....... switch on a TTL level\n");
   TextMessage("   ttloff ...... switch off a TTL level\n");
   TextMessage("   ttldupon .... switch the TTL level for the LF duplexer\n");
   TextMessage("   ttldupoff ... switch off the TTL level for the LF duplexer\n");
   TextMessage("   ttlpulse .... generate TTL pulse\n");
   TextMessage("   ttltranslate  translate TTL pin number to byte code\n");
   TextMessage("   trigger ..... wait for trigger input\n");
   TextMessage("   txoff ....... turn off the transmitter output\n");
   TextMessage("   txon ........ turn on the transmitter output\n");
   TextMessage("   wait ........ generate a long delay\n");
}

/*******************************************************************************
   Extension procedure to return syntax in DLL 
*******************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';

   if(!strcmp(cmd,"acqulong"))           strcpy(syntax,"acqulong(mode, number points:n, points to average:n, scale factor:n)");
   else if(!strcmp(cmd,"acquire"))       strcpy(syntax,"acquire(mode, number points:n, [duration:d])");
   else if(!strcmp(cmd,"acquireon"))     strcpy(syntax,"acquireon(number points:n)");
   else if(!strcmp(cmd,"acquireoff"))    strcpy(syntax,"acquireoff(number points:n)");
   else if(!strcmp(cmd,"chirprf"))       strcpy(syntax,"chirprf(mode, atable:t, ftable:f, phase:p, table_size:n, table_step_duration:d)");
   else if(!strcmp(cmd,"cleardata"))     strcpy(syntax,"cleardata(number:n)");
   else if(!strcmp(cmd,"decindex"))      strcpy(syntax,"decindex(table:t)");
   else if(!strcmp(cmd,"delay"))         strcpy(syntax,"delay(duration:d)");
   else if(!strcmp(cmd,"endloop"))       strcpy(syntax,"endloop(name)");
   else if(!strcmp(cmd,"endpp"))         strcpy(syntax,"endpp()");
   else if(!strcmp(cmd,"endskip"))       strcpy(syntax,"endskip(name)");
   else if(!strcmp(cmd,"endiftrue"))     strcpy(syntax,"endiftrue(name)");
   else if(!strcmp(cmd,"execwait"))      strcpy(syntax,"execwait(program,arguments)");
   else if(!strcmp(cmd,"gradon"))        strcpy(syntax,"gradon(address:n,level:n/t)");
   else if(!strcmp(cmd,"shimon"))        strcpy(syntax,"shimon(address:n,level:n/t)");
   else if(!strcmp(cmd,"gradoff"))       strcpy(syntax,"gradoff(address:n)");
   else if(!strcmp(cmd,"gradramp"))      strcpy(syntax,"gradramp([address(1-4):n, start(1-4):n/t, end(1-4):n/t], steps:n, delay:d)]]");
   else if(!strcmp(cmd,"incindex"))      strcpy(syntax,"incindex(table:t)");
   else if(!strcmp(cmd,"incrxfreq"))     strcpy(syntax,"incrxfreq(increment:f)");
   else if(!strcmp(cmd,"inctxfreq"))     strcpy(syntax,"inctxfreq(increment:f)");
   else if(!strcmp(cmd,"inctxamp"))      strcpy(syntax,"inctxamp(amp:a, increment:a)");
   else if(!strcmp(cmd,"initpp"))        strcpy(syntax,"initpp(filename)");
   else if(!strcmp(cmd,"loop"))          strcpy(syntax,"loop(name,n)");
   else if(!strcmp(cmd,"memreset"))      strcpy(syntax,"memreset([address:n])");
   else if(!strcmp(cmd,"nop"))           strcpy(syntax,"nop()");     
   else if(!strcmp(cmd,"ppversion"))     strcpy(syntax,"(INT v) = ppversion()");
	else if(!strcmp(cmd,"pulse"))         strcpy(syntax,"pulse(mode, amp:a, phase:p, duration:d [,freq:f] OR pulse(ch1 ,a1, p1, f1, ch2 ,a2, p2, f2, d])");
   else if(!strcmp(cmd,"setindex"))      strcpy(syntax,"setindex(table:t,index:n)");
   else if(!strcmp(cmd,"setrxfreq"))     strcpy(syntax,"setrxfreq(freq:f)");
   else if(!strcmp(cmd,"settxfreq"))     strcpy(syntax,"settxfreq(freq:f) OR settxfreq(channel:1/2 freq:f) ");
   else if(!strcmp(cmd,"settxfreqs"))    strcpy(syntax,"settxfreqs(freq1:f, freq2:f)");
   else if(!strcmp(cmd,"setrxgain"))     strcpy(syntax,"setrxgain(gain:g)");
   else if(!strcmp(cmd,"setlfrxgain"))   strcpy(syntax,"setlfrxgain(gain:g)");
   else if(!strcmp(cmd,"setlfrxoffset")) strcpy(syntax,"setlfrxoffset(offset:n)");
   else if (!strcmp(cmd,"shapedrf"))    strcpy(syntax, "shapedrf(mode, atable:t, stable:t, phase:p, table_size:n, table_step_duration:d)");
   else if (!strcmp(cmd,"shapedrf2"))   strcpy(syntax, "shapedrf2(mode, atable:t, stable:t, phase1:p, phase2:p, freq1:f, freq2:f, table_size:n, table_step_duration:d)");
   else if(!strcmp(cmd,"shapedgrad"))    strcpy(syntax,"shapedgrad(address:n, atable:t, table_size:n, table_step_duration:d)");
   else if(!strcmp(cmd,"trigger"))       strcpy(syntax,"trigger(mode, ignore:n)");
   else if(!strcmp(cmd,"ttltranslate"))  strcpy(syntax,"(INT byte) = ttltranslate(pin number)");
   else if(!strcmp(cmd,"ttl"))           strcpy(syntax,"ttl(byte:b)");
   else if(!strcmp(cmd,"ttlon"))         strcpy(syntax,"ttlon(byte:b)");
   else if(!strcmp(cmd,"ttldupon"))      strcpy(syntax,"ttldupon()");
   else if(!strcmp(cmd,"ttloff"))        strcpy(syntax,"ttloff(byte:b)");
   else if(!strcmp(cmd,"ttldupoff"))     strcpy(syntax,"ttldupoff()");
   else if(!strcmp(cmd,"ttlpulse"))      strcpy(syntax,"ttlpulse(byte:b, duration:d)");
   else if(!strcmp(cmd,"txoff"))         strcpy(syntax,"txoff(mode)");
	else if(!strcmp(cmd,"txon"))          strcpy(syntax,"txon(mode, amp:a, phase:p [,freq:f]) OR txon(ch1, a1, p1, f1, ch2, a2, p2, f2)");
   else if(!strcmp(cmd,"wait"))          strcpy(syntax,"wait(duration:w)");


   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short GetHelpFolder(DLLParameters* par, char *args)
{
   par->retVar[1].MakeAndSetString("Macros\\Kea-PP");
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
 //  parList = NULL;
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
	   fprintf(fp,"\n        move    #$10000,a1                 ; Make DATA_ADRS point to the start of fid memory");
	   fprintf(fp,"\n        move    a1,y:DATA_ADRS");
   }
   else
   {
      InsertUniqueStringIntoList(adrs.Str(),&parList,szList);
      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Reset the memory pointer");
      fprintf(fp,"\n        move    x:NR%s,a1",adrs.Str()+1);
	   fprintf(fp,"\n        move    a1,y:DATA_ADRS                 ; Make DATA_ADRS point to specified part of fid memory");
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
            Modify the receiver amplifier gain (4/11/07)
**********************************************************************/

short SetRxGain(DLLParameters* par, char *args)
{
   short nrArgs;
   CText gain;

   if((nrArgs = ArgScan(par->itfc,args,1,"gain name","e","q",&gain)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(gain.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Rx gain");

   fprintf(fp,"\n        movep   #$2C,x:A_PCRC           ; Set up SSI 0"); 
   fprintf(fp,"\n        movep   #$100803,x:A_CRA0       ; /3 clk, 16 bit word transferred"); 
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB0        ; Enable SSI port with sc1/2 are outputs"); 

   fprintf(fp,"\n        move    x:GAIN%s_0,a1",gain.Str()+1); 
   fprintf(fp,"\n        movep   #$0,x:A_PDRE            ; Select first gain block");
   fprintf(fp,"\n        move    a1,x:A_TX00");
   fprintf(fp,"\n        move    #10,r7                  ; Wait 10 us");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n        move    x:GAIN%s_1,a1",gain.Str()+1); 
   fprintf(fp,"\n        movep   #$4,x:A_PDRE            ; Select second gain block");
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
       Modify the receiver amplifier gain for the low freqency RX amp
**********************************************************************/

short SetLowFreqRxGain(DLLParameters* par, char *args)
{
   short nrArgs;
   CText gain;

   if((nrArgs = ArgScan(par->itfc,args,1,"gain name","e","q",&gain)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(gain.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Low frequency Rx gain");

   fprintf(fp,"\n        movep   #$2C,x:A_PCRC           ; Set up SSI 0"); 
   fprintf(fp,"\n        movep   #$18080A,x:A_CRA0       ; /10 clk, 24 bit word transferred"); 
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB0        ; Enable SSI port with sc1/2 are outputs"); 

   fprintf(fp,"\n        move    x:GAIN%s_0,a1",gain.Str()+1); 
   fprintf(fp,"\n        move    #$00FFFF,x1"); 
   fprintf(fp,"\n        and      x1,a1"); 
   fprintf(fp,"\n        move    #$100000,x1             ; Select gain DAC"); 
   fprintf(fp,"\n        or      x1,a1"); 
   fprintf(fp,"\n        movep   #$0000,x:A_PDRE         ; Select Rx amp interface");
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
       Modify the receiver offset for the low freqency RX amp
**********************************************************************/

short SetLowFreqOffset(DLLParameters* par, char *args)
{
   short nrArgs;

   CText gain;

   if((nrArgs = ArgScan(par->itfc,args,1,"offset name","e","q",&gain)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(gain.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Low frequency Rx Amp offset");

   fprintf(fp,"\n        movep   #$2C,x:A_PCRC           ; Set up SSI 0"); 
   fprintf(fp,"\n        movep   #$18080A,x:A_CRA0       ; /10 clk, 24 bit word transferred"); 
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB0        ; Enable SSI port with sc1/2 are outputs"); 

   fprintf(fp,"\n        move    x:NR%s,a1",gain.Str()+1); 
   fprintf(fp,"\n        move    #$00FFFF,x1"); 
   fprintf(fp,"\n        and      x1,a1"); 
   fprintf(fp,"\n        move    #$110000,x1"); 
   fprintf(fp,"\n        or      x1,a1                   ; Select offset DAC");
   fprintf(fp,"\n        movep   #$0,x:A_PDRE            ; Select gain block");
   fprintf(fp,"\n        move    a1,x:A_TX00");
   fprintf(fp,"\n        move    #10,r7                  ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n        movep   #$0007,x:A_PDRE         ; Select unused serial port"); 
   fprintf(fp,"\n        movep   #$3c3c,x:A_CRB0         ; Disable SSI 0"); 
   fprintf(fp,"\n        movep   #$24,x:A_PCRC"); 

   fclose(fp);

   return(OK);
}


//
///**********************************************************************
//       Modify the receiver offset for the low freqency RX amp
//**********************************************************************/
//
//short SetLowFreqOffset2(DLLParameters* par, char *args)
//{
//   short nrArgs;
//
//   CText gain;
//
//   if((nrArgs = ArgScan(par->itfc,args,1,"offset name","e","q",&gain)) < 0)
//      return(nrArgs);
//
//   if(!parList)
//   {
//      Error(par->itfc,"Pulse sequence not initialised");
//      return(ERR);
//   }
//   InsertUniqueStringIntoList(gain.Str(),&parList,szList);
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
//   fprintf(fp,"\n; Set Low frequency Rx Amp offset");
//
//   fprintf(fp,"\n        movep   #$2C,x:A_PCRC           ; Set up SSI 0"); 
//   fprintf(fp,"\n        movep   #$18080A,x:A_CRA0       ; /10 clk, 24 bit word transferred"); 
//   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB0        ; Enable SSI port with sc1/2 are outputs"); 
//
//   fprintf(fp,"\n        move    x:NR%s,a1",gain.Str()+1); 
//   fprintf(fp,"\n        move    #$00FFFF,x1"); 
//   fprintf(fp,"\n        and      x1,a1"); 
//   fprintf(fp,"\n        move    #$120000,x1"); 
//   fprintf(fp,"\n        or      x1,a1                   ; Select offset DAC");
//   fprintf(fp,"\n        movep   #$0,x:A_PDRE            ; Select gain block");
//   fprintf(fp,"\n        move    a1,x:A_TX00");
//   fprintf(fp,"\n        move    #10,r7                  ; Wait 2 us");
//   fprintf(fp,"\n        bsr     wait");
//
//   fprintf(fp,"\n        movep   #$0007,x:A_PDRE         ; Select unused serial port"); 
//   fprintf(fp,"\n        movep   #$3c3c,x:A_CRB0         ; Disable SSI 0"); 
//   fprintf(fp,"\n        movep   #$24,x:A_PCRC"); 
//
//   fclose(fp);
//
//   return(OK);
//}


/**********************************************************************
            Modify the receiver frequency (8/7/11)

            This sets the current receive frequency and also the 
            frequency stored in the parameter list.
**********************************************************************/

short SetRxFreq(DLLParameters* par, char *args)
{
   short nrArgs;
   CText freq;
   float fFreq;

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


   if (freq[0] == 'f')
   {
      fprintf(fp, "\n        move    x:FX%s_0,a1", freq.Str() + 1);
      fprintf(fp, "\n        move    a1,x:RXF00");
      fprintf(fp, "\n        move    a1,x:FPGA_DRP1_PI");
      fprintf(fp, "\n        move    x:FX%s_1,a1", freq.Str() + 1);
      fprintf(fp, "\n        move    a1,x:RXF01");
      fprintf(fp, "\n        move    a1,x:FPGA_DRP1_PI");
   }
   else if (freq[0] == 't')
   {
      fprintf(fp, "\n        clr a");
      fprintf(fp, "\n        clr b");
      fprintf(fp, "\n        move    x:TABLE%s,a0", freq.Str() + 1);
      fprintf(fp, "\n        dec a");             // a0 points to table index
      fprintf(fp, "\n        move    a0,r5");     // Read current table index
      fprintf(fp, "\n        move    y:(r5),a0");
      fprintf(fp, "\n        move    x:TABLE%s,b0", freq.Str() + 1);
      fprintf(fp, "\n        add     b,a");       // Add the index to table start to find current value
      fprintf(fp, "\n        move    a0,r5");

      fprintf(fp, "\n        move    y:(r5)+,a1"); // Read the high freq word from the table
      fprintf(fp, "\n        move    a1,x:RXF00");
      fprintf(fp, "\n        move    a1,x:FPGA_DRP1_PI");
      fprintf(fp, "\n        move    y:(r5),a1"); // Read the low freq word from the table
      fprintf(fp, "\n        move    a1,x:RXF01");
      fprintf(fp, "\n        move    a1,x:FPGA_DRP1_PI");
   }
   else if (sscanf(freq.Str(), "%f", &fFreq) == 1) // Fixed frequency
   {
      if (fFreq < 0 || fFreq > 500)
      {
         Error(par->itfc, "invalid frequency '%lg' [0 ... 500]", fFreq);
         fclose(fp);
         return(ERR);
      }
      __int64 DDSFword = (__int64)((fFreq * 4294967296.0L) / 1000.0L + 0.5);
      unsigned long w1 = (unsigned long)((DDSFword & 0xFFFF0000) / 65536.0L);
      unsigned long w2 = (unsigned long)(DDSFword & 0x0000FFFF);
      fprintf(fp, "\n        move    #%lu,a1",w1); // Read the high freq word from the table
      fprintf(fp, "\n        move    a1,x:RXF00");
      fprintf(fp, "\n        move    a1,x:FPGA_DRP1_PI");
      fprintf(fp, "\n        move    #%lu,a1",w2); // Read the low freq word from the table
      fprintf(fp, "\n        move    a1,x:RXF01");
      fprintf(fp, "\n        move    a1,x:FPGA_DRP1_PI");
   }
   else
   {
      Error(par->itfc, "Invalid frequency reference '%s'", freq.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);

   return(OK);
}


/**********************************************************************

         Modify the transceiver transmitter frequency (22/10/13)

         Two options:

         settxfreq(frequency: fx)
         settxfreq(channel: 1/2, frequency: fx)

**********************************************************************/

short SetTxFreq(DLLParameters* par, char *args)
{
   short nrArgs;
   CText freq,channel = "1";
   CArg carg;
   bool dualfreq;

// Get the parameters
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

   if(channel == "1" || channel == "i" || channel == "e")
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

   short nrArgs = carg.Count(args);

   if(nrArgs <= 5)
      EitherChannelRFPulse(par,args);

   else if(nrArgs == 9)
      BothChannelsRFPulse(par,args);

   else
   {
      Error(par->itfc,"invalid number of arguments");
      return(ERR);
   }

   return(OK);
}





/*********************************************************************************************************
 Produce a dual RF pulse on channel 1 & 2 of specified amplitude frequency, phase and duration 
 Channel 1 pulse uses the internal TTL gate or external pin 5 while channel 2 used the external TTL pin 4. 

 22-10-13

 Note that there is a delay of pgo (5 us min) before the pulse appears to allow the RF amp to switch on.
 Also note that there is a delay of 1.0 us after the pulse before another command will appear.
 Finally there should be a minimum delay of 0.25 us after the pulse before another RF pulse is generated
 to allow the FPGA to switch the RF amplitude to zero.

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
   fprintf(fp,"\n        cmp    #49999,a                  ; Pulse must be < 1 ms");
   fprintf(fp,"\n        jgt    ABORT");
   fprintf(fp,"\n        move   #2,b1                     ; Abort code");
   fprintf(fp,"\n        cmp    #24,a                     ; Pulse must be >= 500 ns");
   fprintf(fp,"\n        jlt    ABORT");

// Gate the RF amplifiers
   fprintf(fp,"\n; Gate the RF amplifiers");

   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");

   if((ch1 == "1" && ch2 == "2") || (ch1 == "e" && ch2 == "2"))
      fprintf(fp,"\n        or      #$50000,a               ; TTL 0x01 (pin 5) & TTL 0x04 (pin 4)");
   else if(ch1 == "i" && ch2 == "2")
      fprintf(fp,"\n        or      #$44000,a               ; Internal HPA & TTL 0x04 (pin 4)");
   else
   {
      Error(par->itfc,"invalid channel values ch1 = {i,e,1} ch2 = {2}");
      return(ERR);
   }

   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");

   fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
   fprintf(fp,"\n        move    x:PGO,a1");
   fprintf(fp,"\n        add     #4,a                   ; Tweek it"); //
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        nop");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   
// Set channel i/1 amplitude
	fprintf(fp,"\n; Set channel i/1 amplitude");
   channel = "1";
   if(SelectAmplitude(par->itfc ,fp, amp1, channel) == ERR) return(ERR);

// Set channel i/1 phase
   fprintf(fp,"\n; Set channel i/1 phase");
	if(SelectPhase(par->itfc ,fp, phase1,channel) == ERR) return(ERR);

// Set channel 1 frequency
   fprintf(fp,"\n; Set channel i/1 frequency");
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

   if(ch1 == "1" || ch1 == "e")
      fprintf(fp,"\n        and     #$FAFFF0,a               ; Switch off channel 1 & 2");
   else if(ch1 == "i")
      fprintf(fp,"\n        and     #$FBBFF0,a               ; Switch off internal channel & channel 2");

   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update Kea");

   fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
   fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");

   fprintf(fp,"\n        rep     #50                    ; Allow time for channel 1 to reset");
   fprintf(fp,"\n        nop");

   fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
   fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");

	fclose(fp);

   return(OK);
}


/*********************************************************************************************************
 Produce a single RF pulse on channel 1 or 2 of specified amplitude frequency, phase and duration 
 Channel 1 pulse use the internal TTL gate channel 2 the external TTL gate pulse. 

 Note that there is a delay of pgo (1.5 us min) before the pulse appears to allow the RF amp to switch on.
 Also note that there is a delay of 0.35 us after the pulse before another command will appear.
 Finally there should be a minimum delay of 0.25 us after the pulse before another RF pulse is generated
 to allow the FPGA to switch the RF amplitude to zero.

 (4/11/13)
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
   if(amp[0] == 'a' || amp[0] == 't')
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

   if(channel == "i" || channel == "e" || channel == "1" || channel == "2")
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate an RF pulse");
		fprintf(fp,"\n;"); 

	// Get the user defined delay
      fprintf(fp,"\n        clr    a");
		if(SelectDuration(par->itfc ,fp, "a1", duration) == ERR) return(ERR);

	// Test for invalid delay (< 0.5 us or > 1 ms)
      fprintf(fp,"\n; Check for invalid pulse length");
      fprintf(fp,"\n        move   #1,b1                     ; Abort code");
      fprintf(fp,"\n        cmp    #49999,a                  ; Pulse must be < 1 ms");
      fprintf(fp,"\n        jgt    ABORT");
      fprintf(fp,"\n        move   #2,b1                     ; Abort code");
      fprintf(fp,"\n        cmp    #24,a                     ; Pulse must be >= 500 ns");
      fprintf(fp,"\n        jlt    ABORT");

	// Update channel TTL gate
      fprintf(fp,"\n; Unblank the RF amp");
      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL level");
	   if(channel == "i") // Internal channel 1 Kea pulse
		   fprintf(fp,"\n        or      #$004000,a              ; Internal HPA");
	   else if(channel == "1" || channel == "e") // External channel 1 Kea pulse
			fprintf(fp,"\n        or      #$010000,a              ; TTL 0x01 (pin 5)");
      else
         fprintf(fp,"\n        or      #$040000,a               ; TTL 0x04 (pin 4)");

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

      fprintf(fp,"\n       nop");
      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");
		if(channel == "2")
			fprintf(fp,"\n        or      #$000002,a             ; Channel 2 RF on");
		else  
			fprintf(fp,"\n        or      #$000008,a             ; Channel 1/i/e RF on");
		fprintf(fp,"\n        move    a1,y:TTL                ; Load the current TTL word");
		fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");

		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Start timer"); 
      fprintf(fp, "\n        nop");
      fprintf(fp, "\n        nop"); // Remove jitter
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");

	// End RF pulse
		fprintf(fp,"\n; End pulse");
    	fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      if(channel == "1" || channel == "e")
         fprintf(fp,"\n        and     #$FEFFF7,a               ; Switch off channel 1");
      else if(channel == "i")
         fprintf(fp,"\n        and     #$FFBFF7,a               ; Switch off internal channel");
      else if(channel == "2")
         fprintf(fp,"\n        and     #$FBFFFD,a               ; Switch off channel 2");


      fprintf(fp,"\n        move    a1,y:TTL                 ; Update TTL word");

	   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");

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
	}
   else
   {
		fclose(fp);
      Error(par->itfc,"unknown RF pulse mode");
      return(ERR);
   }

   return(OK);
}



/**********************************************************************
     Generate a shaped gradient pulse (11/06/08)

     shapedgrad(grad, amplitudeTable, tableSize, tableStep)

     grad ............. which gradient to control (1=x; 2=y, 3=z)
     amplitudeTable ... table of amplitudes (-2^15 -> 2^15-1)
     tableSize ........ number of value in the table
     tableStep ........ duration of each table value (in us)

     shapedgrad(n?/x/y/z/o, t?, n?/constant, d?/constant)

     Two other options exist for controlling more than one gradient
     simultaneously:

     shapedgrad(grad1, table1, grad2, table2, tableSize, tableStep)

     shapedgrad(grad1, table1, grad2, table2, grad3, table3, tableSize, tableStep)

**********************************************************************/

short ShapedGradientPulse(DLLParameters* par, char *args)
{
   short nrArgs;
   long Grad,Size,Duration;
   float fDuration;
   CArg carg;

   int n = carg.Count(args);

   if(n == 4)
   {
      CText grad,atable,size,duration;

      if((nrArgs = ArgScan(par->itfc,args,4,"grad, atable, table_size, table_step","eeee","qqqq",&grad,&atable,&size,&duration)) < 0)
         return(nrArgs);

      if(!parList)
      {
         Error(par->itfc,"Pulse sequence not initialised");
         return(ERR);
      }

// Set up parameter list
      if(grad[0] == 'n')
         InsertUniqueStringIntoList(grad.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable.Str(),&parList,szList);
      if(size[0] == 'n')
         InsertUniqueStringIntoList(size.Str(),&parList,szList);
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
      fprintf(fp,"\n; Make a shaped gradient pulse");

      fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");

      if(SelectGradient(par->itfc,fp,grad))
         return(ERR);

// Extract the table address
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);

// Extract the table size
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            Error(par->itfc,"invalid table size '%s' [>= 1]",grad);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the table
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

      fprintf(fp,"\n; Delay");

// Delay for each table value
      if(duration[0] == 'd')
      {
         fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
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
         fprintf(fp,"\n        move    #%ld,a1",Duration);
      }

      fprintf(fp,"\n        sub     #11,a"); // Tweek it
      fprintf(fp,"\n        move    a1,r3");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fprintf(fp,"\n        movep   #$24,x:A_PCRD          ; Turn off SSI 1 on Port D");

// Finish off by zeroing the gradient
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      fclose(fp);
   }

// Two gradients controlled ************************************

   else if(n == 6)
   {
      CText grad1,atable1,grad2,atable2,size,duration;

      if((nrArgs = ArgScan(par->itfc,args,6,"adrs1, atable1, adrs2, atable2, table_size, table_step","eeeeee","qqqqqq",&grad1,&atable1,&grad2,&atable2,&size,&duration)) < 0)
         return(nrArgs);

      if(!parList)
      {
         Error(par->itfc,"Pulse sequence not initialised");
         return(ERR);
      }

   // Set up parameter list
      if(grad1[0] == 'n')
         InsertUniqueStringIntoList(grad1.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable1.Str(),&parList,szList);
      if(grad2[0] == 'n')
         InsertUniqueStringIntoList(grad2.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable2.Str(),&parList,szList);
      if(size[0] == 'n')
         InsertUniqueStringIntoList(size.Str(),&parList,szList);
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
      fprintf(fp,"\n; Make 2 shaped gradient pulses");

      fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");

// Extract the table addresses for the two gradients
      fprintf(fp,"\n        move    x:TABLE%s,r4",atable1.Str()+1); 
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable2.Str()+1);

// Extract the table size
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            Error(par->itfc,"invalid table size '%s' [>= 1]",grad2);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      if(SelectGradient(par->itfc,fp,grad1)) return(ERR);
      fprintf(fp,"\n        move    y:(r4)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

      fprintf(fp,"\n        rep     #185                    ; Wait 2us");
      fprintf(fp,"\n        nop");

      if(SelectGradient(par->itfc,fp,grad2)) return(ERR);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

// Delay for each table value
      fprintf(fp,"\n; Delay");

      if(duration[0] == 'd')
      {
         fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
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
         fprintf(fp,"\n        move    #%ld,a1",Duration);
      }

      fprintf(fp,"\n        sub     #111,a"); // Tweek it
      fprintf(fp,"\n        move    a1,r3");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fprintf(fp,"\n        movep   #$24,x:A_PCRD          ; Turn off SSI 1 on Port D");

      //SelectGradient(par->itfc,fp,grad1);
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      //SelectGradient(par->itfc,fp,grad2);
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      fclose(fp);
   }

// Three gradients controlled ************************************

   else if(n == 8)
   {
      CText grad1,atable1,grad2,atable2,grad3,atable3,size,duration;

      if((nrArgs = ArgScan(par->itfc,args,8,"adrs1, atable1, adrs2, atable2, adrs3, atable3, table_size, table_step","eeeeeeee","qqqqqqqq",&grad1,&atable1,&grad2,&atable2,&grad3,&atable3,&size,&duration)) < 0)
         return(nrArgs);

      if(!parList)
      {
         Error(par->itfc,"Pulse sequence not initialised");
         return(ERR);
      }
   // Set up parameter list
      if(grad1[0] == 'n')
         InsertUniqueStringIntoList(grad1.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable1.Str(),&parList,szList);
      if(grad2[0] == 'n')
         InsertUniqueStringIntoList(grad2.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable2.Str(),&parList,szList);
      if(grad3[0] == 'n')
         InsertUniqueStringIntoList(grad3.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable3.Str(),&parList,szList);
      if(size[0] == 'n')
         InsertUniqueStringIntoList(size.Str(),&parList,szList);
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
      fprintf(fp,"\n; Make  3 shaped gradient pulses");

      fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");

// Extract the table addresses for the three gradients
      fprintf(fp,"\n        move    x:TABLE%s,r4",atable1.Str()+1); // Get data table addresses
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable2.Str()+1); // Get data table addresses
      fprintf(fp,"\n        move    x:TABLE%s,r1",atable3.Str()+1); // Get data table addresses

// Extract the table sizes
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            Error(par->itfc,"invalid table size '%s' [>= 1]",grad2);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      if(SelectGradient(par->itfc,fp,grad1)) return(ERR);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(r4)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

      fprintf(fp,"\n        rep     #185                    ; Wait 2us");
      fprintf(fp,"\n        nop");

      if(SelectGradient(par->itfc,fp,grad2)) return(ERR);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

      fprintf(fp,"\n        rep     #185                    ; Wait 2us");
      fprintf(fp,"\n        nop");

      if(SelectGradient(par->itfc,fp,grad3)) return(ERR);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(r1)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

// Delay for each table value
      fprintf(fp,"\n; Delay");
      if(duration[0] == 'd')
      {
         fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
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
         fprintf(fp,"\n        move    #%ld,a1",Duration);
      }

      fprintf(fp,"\n        sub     #211,a"); // Tweek it
      fprintf(fp,"\n        move    a1,r3");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fprintf(fp,"\n        movep   #$24,x:A_PCRD          ; Turn off SSI 1 on Port D");

      //SelectGradient(par->itfc,fp,grad1);
      //fprintf(fp,"\n        movep   a1,x:A_PDRE");
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      //SelectGradient(par->itfc,fp,grad2);
      //fprintf(fp,"\n        movep   a1,x:A_PDRE");
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      //SelectGradient(par->itfc,fp,grad3);
      //fprintf(fp,"\n        movep   a1,x:A_PDRE");
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      fclose(fp);
   }
   else
   {
      Error(par->itfc,"4, 6 or 8 parameters expected");
      return(ERR);
   }

   return(OK);
}



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

   if(channel == "i") // Internal Kea pulse channel 1
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
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Zero phase");
      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set correct frequency");
      fprintf(fp,"\n        move    x:TXF01,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");

	   fclose(fp);
   }

   else if(channel == "1" || channel == "e") // External Kea pulse channel 1
   {
	   fprintf(fp,"\n\n;");
	   fprintf(fp,"\n;***************************************************************************");
	   fprintf(fp,"\n; Generate a modulated external pulse (ch 1)");
	   fprintf(fp,"\n;"); 

      fprintf(fp,"\n        clr a                           ; Clear the accumulator");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");
      fprintf(fp,"\n        move    #$10000,a1              ; Switch on rf bias (external ch1)");
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
      fprintf(fp,"\n        move    #$10008,a1              ; Switch on rf (external ch1)");
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
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Zero phase");
      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set correct frequency");
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
      fprintf(fp,"\n        move    #$40000,a1              ; Switch on rf bias (external ch2)");
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
      fprintf(fp,"\n        move    #$40002,a1              ; Switch on rf (external ch2)");
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
     Generate a dual channel shaped RF pulse (27 Jan 2022)

     shapedrf2(channels, amplitudeTable, phaseTable, phase1, phase2, freq1, freq2, tableSize, tableStep)

     channels .......... either "1,2" or "i,2" or "e,2"
     amplitudeTable .... table of amplitudes (0 -> 2^14-1) (ch1/2 Interleaved)
     phaseTable ........ table of phases (0 -> 2^16-1) (ch1/2 Interleaved)
     phase1 ............ a phase to add to the channel 1 phase table (0 -> 2^16-1)
     phase2 ............ a phase to add to the channel 2 phase table (0 -> 2^16-1)
     freq1 ............. frequency of channel 1
     freq2 ............. frequency of channel 2
     tableSize ......... number of value in channel 1 or 2 table (half the total)
     tableStep ......... duration of each table value (in us) Minimum 4.5 us

     Note that the tables are interleaved ch1, ch2 i.e.

     a(0)ch1, a(0)ch2, a(1)ch1, a(1)ch2, .... a(n-1)ch1, a(n-1)ch2
     p(0)ch1, p(0)ch2, p(1)ch1, p(1)ch2, .... p(n-1)ch1, p(n-1)ch2

     where n is the tableSize

**********************************************************************/

short ShapedRFDualChannel(DLLParameters* par, char* args)
{
   short nrArgs;
   CText channels, dur, size, duration;
   CText atable, ptable, phase1, phase2, freq1, freq2;
   long Duration, Phase, Size;
   float fDuration;
   const int pgoOffset  = 5;
   const int firstDelay = 402;
   const int midDelay   = 167;
   const int endDelay   = 170;

   if ((nrArgs = ArgScan(par->itfc, args, 9, "channels, atable, stable, phase1, phase2, f1, f2, table_size, table_step", "eeeeeeeee", "qqqqqqqqq", &channels, &atable, &ptable, &phase1, &phase2, &freq1, &freq2, &size, &duration)) < 0)
      return(nrArgs);
   if (!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(atable.Str(), &parList, szList);
   InsertUniqueStringIntoList(ptable.Str(), &parList, szList);
   if (phase1[0] == 'p' || phase1[0] == 'n')
      InsertUniqueStringIntoList(phase1.Str(), &parList, szList);
   if (phase2[0] == 'p' || phase2[0] == 'n')
      InsertUniqueStringIntoList(phase2.Str(), &parList, szList);
   if (freq1[0] == 'f')
      InsertUniqueStringIntoList(freq1.Str(), &parList, szList);
   if (freq2[0] == 'f')
      InsertUniqueStringIntoList(freq2.Str(), &parList, szList);
   if (size[0] == 'n')
      InsertUniqueStringIntoList(size.Str(), &parList, szList);
   if (duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(), &parList, szList);

   // Allow for different channels
   CText rfGateCode, rfOnCode;
   if (channels == "12" || channels == "e2")
   {
      rfGateCode = "#$50000";
      rfOnCode  = "#$50008";
   }
   else if (channels == "i2")
   {
      rfGateCode = "#$44000";
      rfOnCode   = "#$4400A";
   }
   else
   {
      ErrorMessage("Invalid channel code - should be one of: '12'/'e2'/i2");
      return(ERR);
   }

   FILE* fp = fopen("midCode.asm", "a");
   if (!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }


   fprintf(fp, "\n\n;");
   fprintf(fp, "\n;***************************************************************************");
   fprintf(fp, "\n; Generate modulated  pulses on channel 1 and 2");
   fprintf(fp, "\n;");

   fprintf(fp, "\n        clr a                            ; Clear the accumulator");
   fprintf(fp, "\n        move    y:TTL,x1                 ; Load the current TTL level");
   fprintf(fp, "\n        move    %s,a1               ; Switch on rf bias (ch1 & ch2)", rfGateCode.Str());
   fprintf(fp, "\n        or      x1,a1                    ; Combine with ttl output");
   fprintf(fp, "\n        move    a1,x:FPGA_TTL");

   fprintf(fp, "\n; Start a timer to give pgo delay before RF comes on");
   fprintf(fp, "\n        move    x:PGO,a1                 ; All delays add to 1us before pulse comes on");
   fprintf(fp, "\n        add     #%d,a", pgoOffset);      // Tweak the delay
   fprintf(fp, "\n        movep   a1,x:A_TCPR2");
   fprintf(fp, "\n        nop");
   fprintf(fp, "\n        movep   #$200A01,x:A_TCSR2");

   // Get the amplitude and phase table addresses and store in r5 and r4
   fprintf(fp, "\n; Get the amplitude and phase table pointers");
   fprintf(fp, "\n        move    x:TABLE%s,r5", atable.Str() + 1);
   fprintf(fp, "\n        move    x:TABLE%s,r4", ptable.Str() + 1);

   // Get the size of the tables store in r2
   if (size[0] == 'n')
   {
      fprintf(fp, "\n        move    x:NR%s,r2", size.Str() + 1);
   }
   else if (sscanf(size.Str(), "%ld", &Size) == 1)
   {
      if (Size < 2)
      {
         ErrorMessage("invalid table size '%s' [>= 2]", size);
         fclose(fp);
         return(ERR);
      }
      fprintf(fp, "\n        move    #%ld,r2", Size);
   }
   fprintf(fp, "\n        move    r2,a0");
   fprintf(fp, "\n        dec     a                        ; Decrement because first value already used before loop");
   fprintf(fp, "\n        move    a0,r2");

   // Channel 1 info
   fprintf(fp, "\n; Set the rf output to its initial value");
   fprintf(fp, "\n        move    y:(r5)+,a1               ; Load ch-1 amplitude");
   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");
   fprintf(fp, "\n        move    y:(r4)+,a1               ; Load ch-1 phase");


   // Add phase shift for phase cycling
   if (phase1[0] == 'p')
      fprintf(fp, "\n        move    x:TXP%s,y1", phase1.Str() + 1);
   else if (phase1[0] == 'n')
      fprintf(fp, "\n        move    x:NR%s,y1", phase1.Str() + 1);
   else if (sscanf(phase1.Str(), "%ld", &Phase) == 1)
      fprintf(fp, "\n        move    #%ld,y1", Phase);
   else
   {
      ErrorMessage("invalid phase value '%s'", phase1.Str());
      fclose(fp);
      return(ERR);
   }

   fprintf(fp, "\n        add     y1,a                     ; Add ch-1 phase to table");

   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");
   fprintf(fp, "\n        move    x:FX%s_0,a1", freq1.Str() + 1);
   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");
   fprintf(fp, "\n        move    x:FX%s_1,a1", freq1.Str() + 1);
   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");

   fprintf(fp, "\n        move    #15,r7");
   fprintf(fp, "\n        bsr     svwait");

   // Channel 2 info
   fprintf(fp, "\n; Set the rf output to its initial value");
   fprintf(fp, "\n        move    y:(r5)+,a1               ; Load  ch-2 amplitude");
   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");
   fprintf(fp, "\n        move    y:(r4)+,a1               ; Load  ch-2 phase");


   // Add phase shift for phase cycling
   if (phase1[0] == 'p')
      fprintf(fp, "\n        move    x:TXP%s,y1", phase2.Str() + 1);
   else if (phase1[0] == 'n')
      fprintf(fp, "\n        move    x:NR%s,y1", phase2.Str() + 1);
   else if (sscanf(phase2.Str(), "%ld", &Phase) == 1)
      fprintf(fp, "\n        move    #%ld,y1", Phase);
   else
   {
      ErrorMessage("invalid phase value '%s'", phase2.Str());
      fclose(fp);
      return(ERR);
   }

   fprintf(fp, "\n        add     y1,a                     ; Add ch-2 phase to table");

   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");
   fprintf(fp, "\n        move    x:FX%s_0,a1", freq2.Str() + 1);
   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");
   fprintf(fp, "\n        move    x:FX%s_1,a1", freq2.Str() + 1);
   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");

   //fprintf(fp,"\n        move    #15,r7");
   //fprintf(fp,"\n        bsr     svwait");

   fprintf(fp, "\n; Wait for pgo delay to end");
   fprintf(fp, "\n        jclr    #21,x:A_TCSR2,*");
   fprintf(fp, "\n        movep   #$200A00,x:A_TCSR2       ; Turn off timer");

   fprintf(fp, "\n; Start modulated pulse");
   fprintf(fp, "\n        move    %s,a1               ; Switch on ch-1", rfOnCode.Str());
   fprintf(fp, "\n        or      x1,a1                    ; and combine with internal ttl bias line");
   fprintf(fp, "\n        move    a1,x:FPGA_TTL");
   fprintf(fp, "\n        move    a,b");

   fprintf(fp, "\n        move    #185,a0                  ; Wait 2 us before switching second pulse (matches delay in loop)");
   fprintf(fp, "\n        rep     a0");
   fprintf(fp, "\n        nop");

   fprintf(fp, "\n        move    #$0430A,b1               ; Switch on ch-2");
   fprintf(fp, "\n        or      x1,b1                    ; and combine with internal ttl bias line");
   fprintf(fp, "\n        move    b1,x:FPGA_TTL");

   // Load step length
   fprintf(fp, "\n; Load step length");
   if (duration[0] == 'd')
   {
      fprintf(fp, "\n        move    x:DELAY%s,a1", duration.Str() + 1);
   }
   else if (sscanf(duration.Str(), "%f", &fDuration) == 1)
   {
      if (fDuration < 4 || fDuration > 327670)
      {
         ErrorMessage("invalid duration '%g' [4...327670]", fDuration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp, "\n        move   #%ld,a1", Duration);
   }
   else
   {
      ErrorMessage("Invalid duration reference '%s'", duration.Str());
      fclose(fp);
      return(ERR);
   }
   // Delay to get length of first pulse step correct

   fprintf(fp, "\n        lsl     #1,a                     ; Multiply step delay by two as we use a 10 ns delay step");
   fprintf(fp, "\n        sub     #%d,a                   ; Adjust the step length allowing for other delays", firstDelay);
   fprintf(fp, "\n        move    a1,a0");
   fprintf(fp, "\n        rep     a0                       ; Wait until the correct step length is reached");
   fprintf(fp, "\n        nop");

   // Calculate delay for subsequent steps
   fprintf(fp, "\n        add     #%d,a                   ; Modify delay value to correct subsequent steps", midDelay);
   fprintf(fp, "\n        move    a1,a0");

   // Loop over the tables
   fprintf(fp, "\n; Loop over table entries updating amplitudes and phases - note that ch1 and ch2 values are interleaved");

   fprintf(fp, "\n        do      r2,LBL%ld                  ; Step the amplitude r2 times", label);

   // Channel 1
   fprintf(fp, "\n        move    y:(r5)+,a1               ; Load amplitude");
   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0      ; Set amplitude");

   fprintf(fp, "\n        move    y:(r4)+,a1               ; Load phase");

   // Add phase shift for phase cycling
   if (phase1[0] == 'p')
      fprintf(fp, "\n        move    x:TXP%s,y1", phase1.Str() + 1);
   else if (phase1[0] == 'n')
      fprintf(fp, "\n        move    x:NR%s,y1", phase1.Str() + 1);
   else if (sscanf(phase1.Str(), "%ld", &Phase) == 1)
      fprintf(fp, "\n        move    #%ld,y1", Phase);
   else
   {
      ErrorMessage("invalid phase value '%s'", phase1.Str());
      fclose(fp);
      return(ERR);
   }
   fprintf(fp, "\n        add     y1,a                     ; Add phase to table");

   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0      ; Set phase");
   fprintf(fp, "\n        move    x:FX%s_0,a1", freq1.Str() + 1);
   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");
   fprintf(fp, "\n        move    x:FX%s_1,a1", freq1.Str() + 1);
   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");

   fprintf(fp, "\n        move    #15,r7                   ; Wait 1 us to allow channel 1 to update");
   fprintf(fp, "\n        bsr     svwait");

   // Channel 2
   fprintf(fp, "\n        move    y:(r5)+,a1               ; Load amplitude");
   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0      ; Set amplitude");

   fprintf(fp, "\n        move    y:(r4)+,a1               ; Load phase");

   // Add phase shift for phase cycling
   if (phase2[0] == 'p')
      fprintf(fp, "\n        move    x:TXP%s,y1", phase2.Str() + 1);
   else if (phase2[0] == 'n')
      fprintf(fp, "\n        move    x:NR%s,y1", phase2.Str() + 1);
   else if (sscanf(phase2.Str(), "%ld", &Phase) == 1)
      fprintf(fp, "\n        move    #%ld,y1", Phase);
   else
   {
      ErrorMessage("invalid phase value '%s'", phase2.Str());
      fclose(fp);
      return(ERR);
   }
   fprintf(fp, "\n        add     y1,a                     ; Add phase to table");

   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0      ; Set phase");
   fprintf(fp, "\n        move    x:FX%s_0,a1", freq2.Str() + 1);
   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");
   fprintf(fp, "\n        move    x:FX%s_1,a1", freq2.Str() + 1);
   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");

   fprintf(fp, "\n        rep     a0                       ; Adjust for correct step length and allow channel 2 to update");
   fprintf(fp, "\n        nop");

   fprintf(fp, "\nLBL%ld    nop", label++);

   fprintf(fp, "\n; End Delay (correct for last pulses)");
   fprintf(fp, "\n        rep     #%d", endDelay);
   fprintf(fp, "\n        nop");

   fprintf(fp, "\n; End channel 1 pulse");
   fprintf(fp, "\n        move    #$000302,a1");
   fprintf(fp, "\n        or      x1,a1                    ; Combine with TTL output");
   fprintf(fp, "\n        move    a1,x:FPGA_TTL");

   fprintf(fp, "\n        move    #$000000,a1              ; Zero amplitude");
   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");
   fprintf(fp, "\n        move    #$000000,a1              ; Zero phase");
   fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");

   fprintf(fp, "\n        rep     #170                     ; Tweak channel 2 pulse length");
   fprintf(fp, "\n        nop");

   fprintf(fp, "\n; End channel 2 pulse");
   fprintf(fp, "\n        move    #$000000,a1");
   fprintf(fp, "\n        or      x1,a1                    ; Combine with TTL output");
   fprintf(fp, "\n        move    a1,x:FPGA_TTL");

   fprintf(fp, "\n        move    #$000000,a1              ; Zero amplitude");
   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");
   fprintf(fp, "\n        move    #$000000,a1              ; Zero phase");
   fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");

   fclose(fp);


   return(OK);
}

/**********************************************************************
 Generate the last statement of a skip 

 skiponzero(name, nrToTest)

**********************************************************************/

short SkipOnZero(DLLParameters* par,char* args)
{
   short nrArgs;
   CText testValueName;
   CText skipName;
   int testValue;
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,2,"skip name, number to test","ee","qq",&skipName,&testValueName)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   if(testValueName[0] == 'n')
      InsertUniqueStringIntoList(testValueName.Str(),&parList,szList);

   InsertUniqueStringIntoList(skipName.Str(),&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Skip on zero");

   if(skipName[0] == 's')
   {
      fprintf(fp, "\n        clr     a"); // Clear a

      if(testValueName[0] == 'n')
         fprintf(fp,"\n        move    x:NR%s,a1                 ; Load zero test into a0",testValueName.Str()+1);
      else if(sscanf(testValueName.Str(),"%ld",&testValue) == 1)
         fprintf(fp,"\n        move    #%ld,a1                   ; Load zero test into a0",testValue);

      fprintf(fp,"\n        cmp     #000000,a"); // Check for zero - skip if equal
      fprintf(fp,"\n        jeq     ZEROSKIP%s                    ; Repeat code until Loop end",skipName.Str()+1);
      fclose(fp);
   }
   else
   {
      Error(par->itfc,"invalid skip name '%s'",skipName.Str());
      fclose(fp);
      return(ERR);
   }

   return(OK);
}


/**********************************************************************
Generate the last statement of a skip 

endskip(name)

**********************************************************************/

short SkipEnd(DLLParameters* par, char *args)
{
   short nrArgs;
   CText skipName;
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,1,"loop name","e","q",&skipName)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

   InsertUniqueStringIntoList(skipName.Str(),&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Skip end");

   if(skipName[0] == 's')
   {
      fprintf(fp,"\nZEROSKIP%s  nop                ; Identifies end of loop",skipName.Str()+1);
   }
   else
   {
      Error(par->itfc,"invalid loop name '%s'",skipName.Str());
      fclose(fp);
      return(ERR);
   }

   fclose(fp);

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

   if((ch1 == "1" && ch2 == "2") || (ch1 == "e" && ch2 == "2"))
      fprintf(fp,"\n        or      #$50000,a               ; TTL 0x01 (pin 5) & TTL 0x04 (pin 4)");
   else if(ch1 == "i" && ch2 == "2")
      fprintf(fp,"\n        or      #$44000,a               ; Internal HPA & TTL 0x04 (pin 4)");

   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");

   fprintf(fp,"\n; Start a timer to give pgo delay before RF comes on");
   fprintf(fp,"\n        move    x:PGO,a1");
   fprintf(fp,"\n        add     #5,a                   ; Tweek it"); //
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        nop");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   
// Set channel i/1 amplitude
	fprintf(fp,"\n; Set channel i/1 amplitude");
   channel = "1";
	if(SelectAmplitude(par->itfc ,fp, amp1, channel) == ERR) return(ERR);

// Set channel i/1 phase
   fprintf(fp,"\n; Set channel i/1 phase");
	if(SelectPhase(par->itfc ,fp, phase1, channel) == ERR) return(ERR);

// Set channel 1 frequency
   fprintf(fp,"\n; Set channel i/1 frequency");
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

     txon("i/e/1/2", ...) or
     txon("wi/we/w1/w2", ...) (for wobble mode)

	  other options

	  txon(mode, amplitude, phase, [frequency])

     "i" internal TTL (TTL-INT 6 HPA) CH 1 RF
     "e" external TTL (TTL-EXT pin 5) CH 1 RF
     "1" external TTL (TTL-EXT pin 5) Ch 1 RF
     "2" external TTL (TTL-EXT pin 4) Ch 2 RF
     "wi" internal TTL (TTL-INT 6 HPA + TTL-INT 4 DUP) CH 1 RF
     "we" external TTL (TTL-EXT pin 5+ TTL-INT 4 DUP) CH 1 RF
     "w1" external TTL (TTL-EXT pin 5+ TTL-INT 4 DUP) Ch 1 RF
     "w2" external TTL (TTL-EXT pin 4+ TTL-INT 4 DUP) Ch 2 RF

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

   if(channel == "i")
      fprintf(fp,"\n        or      #$04000,a              ; Internal HPA");
   else if(channel == "wi")
      fprintf(fp,"\n        or      #$05000,a              ; Internal HPA + Duplexer relay");
   else if(channel == "e" || channel == "1")
      fprintf(fp,"\n        or      #$10000,a              ; TTL 0x01 (pin 5)");
   else if(channel == "we" || channel == "w1")
      fprintf(fp,"\n        or      #$11000,a              ; TTL 0x01 (pin 5) + Duplexer relay");
   else if(channel == "2")
      fprintf(fp,"\n        or      #$40000,a              ; TTL 0x04 (pin 4)");
   else if(channel == "w2")
      fprintf(fp,"\n        or      #$41000,a              ; TTL 0x04 (pin 4) + Duplexer relay");
   else if(channel == "1nb" || channel == "2nb")
      fprintf(fp,"\n        or      #$00000,a              ; No TTL line");

   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL RF remains the same");

   fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to pgo before pulse comes on");
   fprintf(fp,"\n        add     #4,a"); // Tweek it
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
      if(channel == "i" || channel == "e" || channel == "1" || channel == "w1" || channel == "1nb")
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
      fprintf(fp,"\n        or      #$00002,a              ; Channel 2 RF on");
   else
      fprintf(fp,"\n        or      #$00008,a              ; Channel 1 RF on");

   fprintf(fp,"\n        move    a1,y:TTL                ; Load the current TTL word");
	fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");
	fprintf(fp,"\n        nop");

	fclose(fp);


   return(OK);
}


/**********************************************************************

     Switch off the transceiver transmitter output

     txoff(["w"])              # All channels off
     txoff("i/e/1/2")     # Internal/external/channel1/2 off

     Note: keeps the TTL and other RF channels status intact. 

     This command takes xxx ns before the RF turns off

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

	if(nrArgs == 0 || mode[0] == 'w') // Switch off both channels
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch off transmitter");
      fprintf(fp,"\n;"); 
      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
      fprintf(fp,"\n        and     #$FABFF5,a              ; Switch off all RF TTL lines");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL state");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update system");


      fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Zero phase");
      fprintf(fp,"\n        move    x:TXF00,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set correct frequency");
      fprintf(fp,"\n        move    x:TXF01,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");

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
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch off one TX channel");
      fprintf(fp,"\n;"); 

      fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL word");

      if(mode == "1" || mode == "w1" || mode == "1nb")
         fprintf(fp,"\n        and     #$FEFFF7,a               ; Switch off channel 1");
      else if(mode == "i" || mode == "wi")
         fprintf(fp,"\n        and     #$FFBFF7,a               ; Switch off internal channel");
      else if(mode == "2" || mode == "w2" || mode == "2nb")
         fprintf(fp,"\n        and     #$FBFFFD,a               ; Switch off channel 2");
		else
		{
         Error(par->itfc,"invalid channel (1/2/1nb/2nb/w1/w2)");
         fclose(fp);
			return(ERR);
		}

      fprintf(fp,"\n        move    a1,y:TTL                 ; Update TTL word");
	   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL & RF");

      if(mode == "2" || mode == "w2" || mode == "2nb") // Channel 2
      {
         fprintf(fp,"\n        move    #$000000,a1             ; Zero amplitude"); 
         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
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
   fprintf(fp,"\n        sub     #12,a"); // Tweek delay
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
   CText ignore = "0";

// Get the table name and desired index value
   if((nrArgs = ArgScan(par->itfc,args,0,"mode, ignore","ee","tq",&mode,&ignore)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   
   if(ignore[0] == 'n')
      InsertUniqueStringIntoList(ignore.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Wait for trigger input");
   fprintf(fp,"\n        clr     a");
   // Get the ignore flag
   int dIgnore;
   if(ignore[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",ignore.Str()+1);
   }
   else if(sscanf(ignore.Str(),"%d",&dIgnore) == 1) 
   {
      if(dIgnore != 0 && dIgnore != 1)
      {
         Error(par->itfc,"invalid ignore flag [0/1]",dIgnore);
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move   #%d,a1",dIgnore);
   }
   else
   {
      Error(par->itfc,"Invalid ignore flag '%s'",ignore.Str());
      fclose(fp);
      return(ERR);
   }

   fprintf(fp,"\n        tst     a                      ;Should we ignore the trigger?");
   fprintf(fp,"\n        jset    #0,a,LBL%ld             ; Collect n samples",label);

   if(mode == "on high")
      fprintf(fp,"\n        jset    #12,x:A_HDR,*          ;Wait for trigger level to go high");
   else
      fprintf(fp,"\n        jclr    #12,x:A_HDR,*          ;Wait for trigger level to go low");
   fprintf(fp,"\nLBL%ld    nop",label++);

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
   CText table, incrementStr = "1";

// Get the table name and desired index value
   if((nrArgs = ArgScan(par->itfc,args,1,"table, increment","ee","qq",&table, &incrementStr)) < 0)
      return(nrArgs);

// Add names to parameter list
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(table.Str(),&parList,szList);

   short increment;
   if(sscanf(incrementStr.Str(),"%d",&increment) != 1)
   {
      Error(par->itfc,"Invalid increment");
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
   fprintf(fp,"\n; Increment a table index");

   if(table[0] == 't')
   {
      fprintf(fp,"\n        move    x:TABLE%s,a0",table.Str()+1);
      fprintf(fp,"\n        clr     b");
      fprintf(fp,"\n        move    #%d,b1",increment);
      fprintf(fp,"\n        dec     a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0"); 
      fprintf(fp,"\n        rep     b"); // specify number of increments
      fprintf(fp,"\n        inc     a"); // increment the table index
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
   CText table, decrementStr = "1";

// Get the table name and desired index value
   if((nrArgs = ArgScan(par->itfc,args,1,"table, increment","ee","qq",&table, &decrementStr)) < 0)
      return(nrArgs);

// Add names to parameter list
   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(table.Str(),&parList,szList);

   short decrement;
   if(sscanf(decrementStr.Str(),"%d",&decrement) != 1)
   {
      Error(par->itfc,"Invalid decrement");
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
   fprintf(fp,"\n; Decrement a table index");

   if(table[0] == 't')
   {
      fprintf(fp,"\n        move    x:TABLE%s,a0",table.Str()+1);
      fprintf(fp,"\n        clr     b");
      fprintf(fp,"\n        move    #%d,b1",decrement);
      fprintf(fp,"\n        dec     a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0"); 
      fprintf(fp,"\n        rep     b"); // specify number of decrements
      fprintf(fp,"\n        dec     a"); // decrement the table index
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
     Set a gradient level - takes 2.2 us

     gradon([3,2,1,0],n?/t?)
     gradon([x,y,z,o],n?/t?)
     gradon([x,y,z,o],n?/t?)
     gradon(n?,n?/t?)

**********************************************************************/

short SwitchOnGradient(DLLParameters* par, char *args)
{
   short nrArgs;
   CText address,amplitude;
   long Address, Amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,2,"address,amplitude","ee","qq",&address,&amplitude)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add to parameter list if not a constant
   if(address[0] == 'n')
      InsertUniqueStringIntoList(address.Str(),&parList,szList);
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
   fprintf(fp,"\n; Set Gradient level");


// Choose the gradient level

   fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");

   if(SelectGradient(par->itfc,fp,address))
		return(ERR);

   if(amplitude[0] == 'n') // Amplitude is via a number reference
   {
   // Set the level
      fprintf(fp,"\n        move    x:NR%s,a1",amplitude.Str()+1);
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10            ; Set up gradient level");
      fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      fprintf(fp,"\n        bsr     wait");
   }
   else if(sscanf(amplitude.Str(),"%ld",&Amplitude) == 1) // Amplitude is a number
   {
      fprintf(fp,"\n        move    #%ld,a1",Amplitude);
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10            ; Set up gradient level");
      fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      fprintf(fp,"\n        bsr     wait");
   }
   else if(amplitude[0] == 't') // Amplitude is t[index]
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",amplitude.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",amplitude.Str()+1);

      fprintf(fp,"\n        add     b,a");  // Add the index to table start to find current value
      fprintf(fp,"\n        move    a0,r5"); // 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read the table value

   // Set the level
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10            ; Set up gradient level");
      fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      fprintf(fp,"\n        bsr     wait");
   }
   else
   {
      fclose(fp);
      Error(par->itfc,"Invalid amplitude '%s'",amplitude.Str());
      return(ERR);
   }

   fprintf(fp,"\n        movep   #$24,x:A_PCRD          ; Turn off SSI 1 on Port D");

   fclose(fp);

   return(OK);
}



/**********************************************************************
     Switch off the gradient - takes 2.2 us 

     gradoff([3,2,1,o])
     gradoff([x,y,z,o])
     gradoff(n?)

**********************************************************************/

short SwitchOffGradient(DLLParameters* par, char *args)
{
   short nrArgs;
   CText address;
   long Address;
   
   if((nrArgs = ArgScan(par->itfc,args,1,"address","e","q",&address)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add to parameter table is not a constant
   if(address[0] == 'n')
      InsertUniqueStringIntoList(address.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Zero gradient");

   fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");

// Choose the gradient to zero
   if(address[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",address.Str()+1);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(address == "x")
   {
      fprintf(fp,"\n        move    #3,a1");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(address == "y")
   {
      fprintf(fp,"\n        move    #2,a1");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(address == "z")
   {
      fprintf(fp,"\n        move    #1,a1");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(address == "o")
   {
      fprintf(fp,"\n        move    #0,a1");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else if(sscanf(address.Str(),"%ld",&Address) == 1)
   {
      fprintf(fp,"\n        move    #%ld,a1",Address);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else
   {
      fclose(fp);
      Error(par->itfc,"Invalid address '%s'",address.Str());
      return(ERR);
   }

// Zero the gradient level
   fprintf(fp,"\n        move    #$00,a1"); 
   fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
   fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n        movep   #$24,x:A_PCRD          ; Turn off SSI 1 on Port D");
   
   fclose(fp);

   return(OK);
}


/**********************************************************************
     Pulse a gradient 

     gradpulse([0,1,2,3],level, duration)
     gradpulse([o,z,y,x],level, duration)

**********************************************************************/

short PulseGradient(DLLParameters* par, char *args)
{
   short nrArgs;
   CText address,amplitude,duration;
   long Address;
   
   if((nrArgs = ArgScan(par->itfc,args,2,"address,amplitude,duration","eee","qqq",&address,&amplitude,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }
   if(address[0] == 'g')
      InsertUniqueStringIntoList(address.Str(),&parList,szList);
   if(amplitude[0] == 'a')
      InsertUniqueStringIntoList(amplitude.Str(),&parList,szList);
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
   fprintf(fp,"\n; Pulse a gradient");

// Choose the gradient to pulse
   if(address[0] == 'g')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",address.Str()+1);
   }
   else if(address == "x")
      fprintf(fp,"\n        move    #3,a1");
   else if(address == "y")
      fprintf(fp,"\n        move    #2,a1");
   else if(address == "z")
      fprintf(fp,"\n        move    #1,a1");
   else if(address == "o")
      fprintf(fp,"\n        move    #0,a1");
   else if(sscanf(address.Str(),"%ld",&Address) == 1)
   {
      if(Address < 0 || Address > 3)
      {
         Error(par->itfc,"invalid address '%ld' [0,1,2,3]",Address);
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #%ld,a1",Address);
   }
   fprintf(fp,"\n        movep   a1,x:A_PDRE");

// Set the level
   fprintf(fp,"\n        move    x:NR%s,a1",amplitude.Str()+1);
   fprintf(fp,"\n        lsl     #8,a");    
   fprintf(fp,"\n        movep   a1,x:A_TX10            ; Set up gradient level");
   fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
   fprintf(fp,"\n        bsr     wait");
  
// Wait some
   fprintf(fp,"\n; Delay");
   fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
   fprintf(fp,"\n        sub     #20,a"); // Tweek delay
   fprintf(fp,"\n        move    a1,r3");
   fprintf(fp,"\n        movep   r3,x:A_TCPR2");
   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

// Zero the gradient
   fprintf(fp,"\n        move    #$00,a1"); 
   fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
   fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
   fprintf(fp,"\n        bsr     wait");


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
      fprintf(fp,"\n        sub     #9,a"); // Tweek it
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
      fprintf(fp,"\n        sub     #9,a"); // Tweek it
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

      fprintf(fp,"\n        sub     #19,a"); // Tweek it
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
      fprintf(fp,"\n        move    x:TTL%s,a1              ; Read in the new TTL byte",byte.Str()+1);
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
      Byte = Byte << 16;

      fprintf(fp,"\n        move    #%ld,a1                 ; Read in the new TTL byte",Byte);
      fprintf(fp,"\n        move    y:TTL,x1                ; Read in the current TTL state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with current TTL state");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save total TTL state");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL             ; Update TTL output");
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
     Switch on a the TTl Duplexer level 

     ttlduplexeron()

**********************************************************************/

short TTLDuplexerOn(DLLParameters* par, char *args)
{
   short nrArgs;

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
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set LF duplexer TTL level to on");

   fprintf(fp,"\n        move    y:TTL,a1                ; Read in the current TTL state");
   fprintf(fp,"\n        or      #$002000,a              ; Combine with current TTL state");
   fprintf(fp,"\n        move    a1,y:TTL                ; Save total TTL state");
   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL output");

   fclose(fp);
   return(OK);

}  


short TTLDuplexerOff(DLLParameters* par, char *args)
{
   short nrArgs;
   CText byte;
   long Byte;

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
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set LF duplexer TTL level to off");

   fprintf(fp,"\n        move    y:TTL,a1                ; Read in the current TTL state");
   fprintf(fp,"\n        and     #$FFDFFF,a              ; Switch off LF duplexer line (0x002000)");
   fprintf(fp,"\n        move    a1,y:TTL                ; Save total TTL state");
   fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Update TTL output");

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
      fprintf(fp,"\n        move    a1,x:FPGA_TTL             ; Update TTL output");
   }
   else if(sscanf(byte.Str(),"%ld",&Byte) == 1)
   {
      if(Byte < 1 || Byte > 0x80)
      {
         Error(par->itfc,"Invalid TTL value '%ld'",Byte);
         fclose(fp);
         return(ERR);
      }
      Byte = Byte << 16;
      fprintf(fp,"\n        move    #%ld,a1                 ; Read in the TTL byte",Byte);
      fprintf(fp,"\n        not     a                       ; Invert the byte");
      fprintf(fp,"\n        move    y:TTL,x1                ; Read the current TTL state");
      fprintf(fp,"\n        and     x1,a1                   ; And with current TTL state to switch off TTL line");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL state");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL             ; Update TTL output");
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
   fprintf(fp,"\n        move    y:DATA_ADRS,r5              ; Make r5 point to the start of fid memory");

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
   fprintf(fp,"\n        move    a1,y:(r5)+");
   fprintf(fp,"\nclearm  nop");
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

  Last modified: 6-Nov-2012
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

   if((nrArgs = ArgScan(par->itfc,args,2,"mode, nr points, [[duration], dataAdrs]","eeee","qqqq",&mode,&nrPnts,&duration,&adrs)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add new variables to the list
   InsertUniqueStringIntoList(nrPnts.Str(),&parList,szList);
   if(nrArgs >= 3)
   {
      if(duration[0] == 'd' || duration[0] == 't')
         InsertUniqueStringIntoList(duration.Str(),&parList,szList);
   }
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
   if(mode == "overwrite") // Each data point is stored in a new location, subsequent calls overwrite this
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
             fprintf(fp,"\n        move    y:DATA_ADRS,r5               ; Specify the save address");
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);
         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1        ; Load data from channel A");

         fprintf(fp,"\n         move    a1,y:(r5)+              ; Save to memory");
         fprintf(fp,"\n         move    x:FPGA_SampleB,a1       ; Load data from channel B");

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

         if(duration[0] == 'd') // Delay variable
         {
			   fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
            fprintf(fp,"\n        sub     #5,a"); // Tweek it
         }
         else if(duration[0] == 't') // Table delay
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
            fprintf(fp,"\n        sub     #15,a"); // Tweek it
         }

		//	fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
		//	fprintf(fp,"\n        sub     #5,a"); // Tweek it
			fprintf(fp,"\n        move    a1,r3");
			fprintf(fp,"\n        movep   r3,x:A_TCPR2");
			fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the current data address");
		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);
         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1        ; Load data from channel A");

         fprintf(fp,"\n         move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\n         move    x:FPGA_SampleB,a1       ; Load data from channel B");

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
	else if(mode == "append") // Each data point is stored in a new location, subsequent calls append to this
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

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the current data address");

		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);

         fprintf(fp,"\n        do      r7,LBL%ld               ; Collect n samples",label);
			fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1       ; Load data from channel A");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\nLBL%ld  nop",label);

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    r5,y:DATA_ADRS           ; Save data address",adrs.Str()+1);

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

         fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the data address",adrs.Str()+1);

         if(duration[0] == 'd') // Delay variable
         {
			   fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
            fprintf(fp,"\n        sub     #5,a"); // Tweek it
         }
         else if(duration[0] == 't') // Table delay
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
            fprintf(fp,"\n        sub     #15,a"); // Tweek it
         }
			fprintf(fp,"\n        move    a1,r3");
			fprintf(fp,"\n        movep   r3,x:A_TCPR2");
			fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the current data address");

		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);

         fprintf(fp,"\n        do      r7,LBL%ld               ; Collect n samples",label);
			fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1       ; Load data from channel A");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");


         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Save to memory");

         fprintf(fp,"\nLBL%ld  nop",label);

         fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
         fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

         fprintf(fp,"\n        move    r5,y:DATA_ADRS           ; Save data address",adrs.Str()+1);

			fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
			fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2");
 

			fclose(fp);

         label++;

		}		
	}


	else if(mode == "sum") // Each data point is stored in a new location, subsequent calls add over the top of this
	{
		if(nrArgs == 2) // No delay
		{
         fprintf(fp,"\n\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Acquire (summing without delay)");

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
         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
         fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Load the number of samples into r7",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the current data address");
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

         if(duration[0] == 'd') // Delay variable
         {
			   fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
            fprintf(fp,"\n        sub     #5,a"); // Tweek it
         }
         else if(duration[0] == 't') // Table delay
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
            fprintf(fp,"\n        sub     #15,a"); // Tweek it
         }

       //  fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
       //  fprintf(fp,"\n        sub     #5,a"); // Tweek it
         fprintf(fp,"\n        move    a1,r3");
         fprintf(fp,"\n        movep   r3,x:A_TCPR2");
         fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");

         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the current data address");
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
   else if(mode == "integrate") // All points are added together and stored in the next memory location
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

         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the current data address");

		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1        ; Load data from channel A");

         fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n         move    x:FPGA_SampleB,a1       ; Load data from channel B");

         fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        sub     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Restore r5");


         fprintf(fp,"\nLBL%ld  nop",label);

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        add     #2,a");
         fprintf(fp,"\n        move    a1,y:DATA_ADRS           ; Save data address");

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

         if(duration[0] == 'd') // Delay variable
         {
			   fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
            fprintf(fp,"\n        sub     #5,a"); // Tweek it
         }
         else if(duration[0] == 't') // Table delay
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
            fprintf(fp,"\n        sub     #15,a"); // Tweek it
         }

       //  fprintf(fp,"\n        move    x:DELAY%s,a1           ; Total acquisition time",duration.Str()+1);
      //   fprintf(fp,"\n        sub     #5,a"); // Tweek it
         fprintf(fp,"\n        move    a1,r3");
         fprintf(fp,"\n        movep   r3,x:A_TCPR2");
         fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
         fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
         fprintf(fp,"\n        nop                             ; To prevent phase jumping");

         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the current data address");

		   if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
 
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label);
         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:FPGA_SampleA,a1       ; Load data from channel A");

         fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:FPGA_SampleB,a1       ; Load data from channel B");

         fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        sub     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Restore r5");

         fprintf(fp,"\n        nop                             ; To prevent phase jumping");

         fprintf(fp,"\nLBL%ld  nop",label);

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        add     #2,a");
         fprintf(fp,"\n        move    a1,y:DATA_ADRS           ; Save data address");

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


/**********************************************************************
     Acquire data while averaging each point collected

     execwait(file to run, argument list)
**********************************************************************/

short AcquireLongData(DLLParameters* par, char *args)
{
   short nrArgs;
   CText nrPntsTxt;
   CText sumPntsTxt;
   CText mode; 
   long nrPnts;
   long sumPnts;
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,3,"mode, nr points, sum points","eee","qqq",&mode,&nrPntsTxt,&sumPntsTxt)) < 0)
      return(nrArgs);

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add new variables to the list
   InsertUniqueStringIntoList(nrPntsTxt.Str(),&parList,szList);
   InsertUniqueStringIntoList(sumPntsTxt.Str(),&parList,szList);

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
		fprintf(fp,"\n; Acquire data with average (overwrite without delay)");

		if(SelectNumber(par->itfc,fp,nrPntsTxt,"a1",2,65536)) return(ERR);
      fprintf(fp,"\n        move    a1,y0");
		if(SelectNumber(par->itfc,fp,sumPntsTxt,"a1",2,65536)) return(ERR);
      fprintf(fp,"\n        move    a1,x0");
      fprintf(fp,"\n        mpy     y0,x0,a");  
      fprintf(fp,"\n        asl     #23,a,a");  
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
      fprintf(fp,"\n        move    y:DATA_ADRS,r5          ; Specify the save address");
		if(SelectNumber(par->itfc,fp,nrPntsTxt,"r7",2,65536)) return(ERR);
		if(SelectNumber(par->itfc,fp,sumPntsTxt,"r3",2,65536)) return(ERR);


      fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label+1);

      fprintf(fp,"\n        do      r3,LBL%ld                ; Average m samples",label);

      fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
		fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

      fprintf(fp,"\n        move    x:FPGA_SampleA,a1        ; Load data from channel A");

      fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");

      fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
      fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

      fprintf(fp,"\n         move    x:FPGA_SampleB,a1       ; Load data from channel B");

      fprintf(fp,"\n        move    y:(r5),b1               ; Get last value at this location");
      fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
      fprintf(fp,"\n        move    a1,y:(r5)              ; Write to memory");

      fprintf(fp,"\n         move    r5,a1");
      fprintf(fp,"\n         sub     #1,a");
      fprintf(fp,"\n         move    a1,r5                    ; Restore r5");
      fprintf(fp,"\nLBL%ld    nop",label);
      fprintf(fp,"\n       nop");

      fprintf(fp,"\n         move    r5,a1");
      fprintf(fp,"\n         add     #2,a");
      fprintf(fp,"\n         move    a1,r5                    ; Next data point r5");
      fprintf(fp,"\nLBL%ld    nop",label+1);

      fprintf(fp,"\n        move    y:TTL,a1                ; Stop ADC capture");
      fprintf(fp,"\n        move    a1,x:FPGA_TTL           ; Send to FPGA");

      label += 2;
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


/******************************************************************************************
  Start acquiring data from the transceiver. 
  
  acquireon(nr_points)
  
  Last modified: September 2021
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
   if(mode == "adc")
   {
      if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
      fprintf(fp,"\n        move    a1,x:FPGA_DRP1_SampleNo  ; Number of data points to collect");
      fprintf(fp,"\n        move    #$000003,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_DRP1_CR        ; Enable data ready");
      fprintf(fp,"\n        move    #$000001,a1");
      fprintf(fp,"\n        move    a1,x:FPGA_FIFO_CR        ; Reset FIFO");
      fprintf(fp,"\n        move    #$000002,a1");
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
  Stop acquiring data from the transceiver. 
  
  acquireoff(mode, nr_points)
  
  Last modified: May 2021
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
      fprintf(fp,"\n        move    #$10000,r5               ; Specify the save address");
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
   else if(mode == "adc") // Data directly from the ADC - store in 
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Stop acquiring data and copy to DSP");
      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        clr     b");
      if(SelectNumber(par->itfc,fp,nrPnts,"a1",2,65536)) return(ERR);
      if(SelectNumber(par->itfc,fp,nrPnts,"b1",2,65536)) return(ERR);
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
      fprintf(fp,"\n        move    #$10000,r5               ; Specify the save address");
      if(SelectNumber(par->itfc,fp,nrPnts,"r7",2,65536)) return(ERR);
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Flush out first word needed for slow spinsolve bus");
      fprintf(fp,"\n        do      r7,LBL%ld                  ; Copy n samples",label+1);
      fprintf(fp,"\n        clr     a");
      fprintf(fp,"\n        move    x:FPGA_FIFO_D,a1         ; Load data from FIFO get first 16 bit word");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    a1,y:(r5)+                ; Save to memory for now");
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
      fprintf(fp,"\n        and     #$fffffe,a               ;");
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
	
	


/**********************************************************************
     Run an external script file and wait for it to complete.

     execwait(file to run, argument list)
**********************************************************************/

STARTUPINFO startupinfo;
PROCESS_INFORMATION processinfo;

short ExecuteAndWait(DLLParameters* par, char *args)
{
   CText cmdline;
   CText file;
   CText arguments;
   short nrArgs;

   if((nrArgs = ArgScan(par->itfc,args,1,"file,arguments","ee","qq",&file,&arguments)) < 0)
      return(nrArgs);

   cmdline.Format("%s %s",file.Str(),arguments.Str());
   startupinfo.cb = sizeof(STARTUPINFO);
   bool st = CreateProcess(file.Str(),cmdline.Str(),NULL,NULL,false,NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,NULL,NULL,&startupinfo,&processinfo);
   WaitForSingleObject(processinfo.hProcess, INFINITE);
   CloseHandle(processinfo.hProcess);

   return(OK);

}

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
      Error(par->itfc,"Can't open output file (1)");
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
   fprintf(fp,"\n                ds      1       ; 11 - not used");
   fprintf(fp,"\n                ds      1       ; 12 - not used");
   fprintf(fp,"\nRXF00           ds      1       ; 13 - Rx Frequency word 0");
   fprintf(fp,"\nRXF01           ds      1       ; 14 - Rx Frequency word 1");
   fprintf(fp,"\nGRADVERSION     ds      1       ; 15 - Gradient version number");
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
   fprintf(fp,"\nUseTrigger      ds      1       ; 26 - If = 1 then wait for trigger");
   fprintf(fp,"\n                ds      1       ; 27 - Not used");
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
      Error(par->itfc,"Can't open output file (2)");
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
      Error(par->itfc,"Can't open output file (3)");
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
      Error(par->itfc,"Can't open endCode file (4)");
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
      Error(par->itfc,"Can't open output file (5)");
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


/*****************************************************************************************************************

Ramp 1-4 gradients
 
     gradramp(address1, start1, end1,
              [address2, start2, end2],
              [address3, start3, end3],
              [address4, start4, end4],
              nrSteps, stepDuration)
 
     where addressi is x, y, z or o (3,2,1,0); 
     starti is the initial gradient value;
     endi is the final value; 
     nrSteps the number of gradient steps between start and end 
     stepDuration the length of each step in microseconds. This must be >= 2 us for 1 gradient >=2.5 for more.

     Temporary registers:
     scatch pad        0,
     number of steps   1,
     duration of steps 2,
     gradient location 3,4,5,6
     start amplitude   7,8,9,10
     end amplitude     11,12,13,14,
     loop counter      15
*****************************************************************************************************************/


short RampedGradients(DLLParameters* par, char *args)
{
   short nrArgs;
   CText grad1,start1,end1;
   CText grad2,start2,end2;
   CText grad3,start3,end3;
   CText grad4,start4,end4;
   CText steps,duration;
   long Steps;
   long Start,End;
   long Address;
   long Duration;
   float fDuration;
   long number,amplitude;
   CArg carg;
   
   int n = carg.Count(args);

   if(n == 5)
   {
      if((nrArgs = ArgScan(par->itfc,args,5,"g1, start1, end1, steps, duration","eeeee","qqqqq",&grad1,&start1,&end1,&steps,&duration)) < 0)
         return(nrArgs);
   }
   else if(n == 8)
   {
      if((nrArgs = ArgScan(par->itfc,args,8,"g1, start1, end1, g2, start2, end2, steps, duration","eeeeeeee","qqqqqqqq",&grad1,&start1,&end1,&grad2,&start2,&end2,&steps,&duration)) < 0)
         return(nrArgs);
   }
   else if(n == 11)
   {
      if((nrArgs = ArgScan(par->itfc,args,11,"g1, start1, end1, g2, start2, end2,  g3, start3, end3, steps, duration","eeeeeeeeeee","qqqqqqqqqqq",&grad1,&start1,&end1,&grad2,&start2,&end2,&grad3,&start3,&end3,&steps,&duration)) < 0)
         return(nrArgs);
   }
   else if(n == 14)
   {
      if((nrArgs = ArgScan(par->itfc,args,14,"g1, start1, end1, g2, start2, end2,  g3, start3, end3, g4, start4, end4, steps, duration","eeeeeeeeeeeeee","qqqqqqqqqqqqqq",&grad1,&start1,&end1,&grad2,&start2,&end2,&grad3,&start3,&end3,&grad4,&start4,&end4,&steps,&duration)) < 0)
         return(nrArgs);
   }
   else
   {
      ErrorMessage("Invalid number of argument (should be 5, 8, 11 or 14)");
      return(ERR);
   }

   if(!parList)
   {
      Error(par->itfc,"Pulse sequence not initialised");
      return(ERR);
   }

// Add variables to parameter list
   if(n >= 5) // Ramp 1 gradient
   {
      if(grad1[0] == 'n')
         InsertUniqueStringIntoList(grad1.Str(),&parList,szList);
      if(start1[0] == 'n' || start1[0] == 't')
         InsertUniqueStringIntoList(start1.Str(),&parList,szList);
      if(end1[0] == 'n' || end1[0] == 't')
         InsertUniqueStringIntoList(end1.Str(),&parList,szList);
   }
   if(n >= 8) // Ramp 2 gradients
   {
      if(grad2[0] == 'n')
         InsertUniqueStringIntoList(grad2.Str(),&parList,szList);
      if(start2[0] == 'n' || start2[0] == 't')
         InsertUniqueStringIntoList(start2.Str(),&parList,szList);
      if(end2[0] == 'n' || end2[0] == 't')
         InsertUniqueStringIntoList(end2.Str(),&parList,szList);
   }
   if(n >= 11) // Ramp 3 gradients
   {
      if(grad3[0] == 'n')
         InsertUniqueStringIntoList(grad3.Str(),&parList,szList);
      if(start3[0] == 'n' || start3[0] == 't')
         InsertUniqueStringIntoList(start3.Str(),&parList,szList);
      if(end3[0] == 'n' || end3[0] == 't')
         InsertUniqueStringIntoList(end3.Str(),&parList,szList);
   }
   if(n >= 14) // Ramp 4 gradients
   {
      if(grad4[0] == 'n')
         InsertUniqueStringIntoList(grad4.Str(),&parList,szList);
      if(start4[0] == 'n' || start4[0] == 't')
         InsertUniqueStringIntoList(start4.Str(),&parList,szList);
      if(end4[0] == 'n' || end4[0] == 't')
         InsertUniqueStringIntoList(end4.Str(),&parList,szList);
   }

// Number and duration
   if(steps[0] == 'n')
      InsertUniqueStringIntoList(steps.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

// Open asm file for append
   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      Error(par->itfc,"Can't open output file");
      return(ERR);
   }

	fprintf(fp,"\n\n;");
	fprintf(fp,"\n;***************************************************************************");
	fprintf(fp,"\n; Generate ramped gradient(s)");
	fprintf(fp,"\n;"); 

   fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");

// Get and store the gradient numbers
   GetGradientNumber(par->itfc,fp,grad1);
   fprintf(fp,"\n        move    a1,y:(TMP+3)"); 
   if(n >= 8)
   {
      GetGradientNumber(par->itfc,fp,grad2);
      fprintf(fp,"\n        move    a1,y:(TMP+4)"); 
   }
   if(n >= 11)
   {
      GetGradientNumber(par->itfc,fp,grad3);
      fprintf(fp,"\n        move    a1,y:(TMP+5)"); 
   }
   if(n == 14)
   {
      GetGradientNumber(par->itfc,fp,grad4);
      fprintf(fp,"\n        move    a1,y:(TMP+6)"); 
   }

// Get the start amplitudes
   GetGradientAmplitude(par->itfc,fp,start1);
   fprintf(fp,"\n        move    a1,y:(TMP+7)"); 
   if(n >= 8)
   {
      GetGradientAmplitude(par->itfc,fp,start2);
      fprintf(fp,"\n        move    a1,y:(TMP+8)"); 
   }
   if(n >= 11)
   {
      GetGradientAmplitude(par->itfc,fp,start3);
      fprintf(fp,"\n        move    a1,y:(TMP+9)"); 
   }
   if(n == 14)
   {
      GetGradientAmplitude(par->itfc,fp,start4);
      fprintf(fp,"\n        move    a1,y:(TMP+10)"); 
   }

// Get the end amplitudes
   GetGradientAmplitude(par->itfc,fp,end1);
   fprintf(fp,"\n        move    a1,y:(TMP+11)"); 
   if(n >= 8)
   {
      GetGradientAmplitude(par->itfc,fp,end2);
      fprintf(fp,"\n        move    a1,y:(TMP+12)"); 
   }
   if(n >= 11)
   {
      GetGradientAmplitude(par->itfc,fp,end3);
      fprintf(fp,"\n        move    a1,y:(TMP+13)"); 
   }
   if(n == 14)
   {
      GetGradientAmplitude(par->itfc,fp,end4);
      fprintf(fp,"\n        move    a1,y:(TMP+14)"); 
   }

// Get the number of gradient steps
   if(steps[0] == 'n') // Steps via a number reference
   {
   // Set the level
      fprintf(fp,"\n        move    x:NR%s,a0",steps.Str()+1);
      fprintf(fp,"\n        dec     a");
      fprintf(fp,"\n        move    a0,y:(TMP+1)");    
   }
   else if(sscanf(steps.Str(),"%ld",&number) == 1) // Steps is a number
   {
      if(number > 0)
      {
         fprintf(fp,"\n        move    #%ld,a0",number);
         fprintf(fp,"\n        dec     a");
         fprintf(fp,"\n        move    a0,y:(TMP+1)");  
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
   fprintf(fp,"\n        move    a1,y:(TMP+2)");    // Step duration

// Set the initial step count value
   fprintf(fp,"\n        move    #0000,a1");
   fprintf(fp,"\n        move    a1,y:(TMP+15)");   // Loop counter  

   fprintf(fp,"\n        move    y:(TMP+1),a0             ; Load the number of steps into r7");
   fprintf(fp,"\n        inc     a");
   fprintf(fp,"\n        move    a0,r2");

// This is the loop over the gradient steps
   fprintf(fp,"\n        do      r2,LBL%ld                ; Calculate each step value",label);

// Set to first gradient
   fprintf(fp,"\n; **************************************************");
   fprintf(fp,"\n; v = n*(end1-start1)/steps + start1");
   fprintf(fp,"\n; **************************************************");
   fprintf(fp,"\n        move    y:(TMP+3),a");    // Set gradient to 1
   fprintf(fp,"\n        movep   a1,x:A_PDRE");
   fprintf(fp,"\n        move    y:(TMP+11),b");   // end1 -> b
   fprintf(fp,"\n        move    b1,y:(TMP+0)");   // end1 -> mem0
   fprintf(fp,"\n        move    y:(TMP+7),x1");   // start1 -> x1 
   fprintf(fp,"\n        move    y:(TMP+0),b");    // tmp0 -> b
   fprintf(fp,"\n        sub     x1,b");           // b-x1->b (end1-start1)->b
   fprintf(fp,"\n        move    b1,x0");          // b->x0 x0=(end1-start1)
   fprintf(fp,"\n        move    y:(TMP+15),y0");  // n->y0

   fprintf(fp,"\n        mpy     y0,x0,b");        // y0*x0->b ie. n1*(end1-start1)->b
   fprintf(fp,"\n        asl     #23,b,b");        // b = b * 2^23 part of multiply
   fprintf(fp,"\n        move    y:(TMP+1),y0");   // nrSteps->y0
   fprintf(fp,"\n        tfr     b,a");            // b->a (efficient move of all 56 bits)
   fprintf(fp,"\n        abs     b");              // |b|->b
   fprintf(fp,"\n        clr     b	b1,x0");       // 0->b 0->x0 (?)
   fprintf(fp,"\n        move    x0,b0");          // 0->b0 (?)
   fprintf(fp,"\n        asl     b");              // ??
   fprintf(fp,"\n        rep     #$18");           // Generates 18 bits of divide precision
   fprintf(fp,"\n        div     y0,b");           // n1*(end1-start1)/nrSteps
   fprintf(fp,"\n        eor     y0,a");           // fix remainer
   fprintf(fp,"\n        bpl     LBL%ld",label+1); // Test if divisor is positive
   fprintf(fp,"\n        neg     b");              // negate otherwise
   fprintf(fp,"\nLBL%ld    nop",label+1);
   fprintf(fp,"\n        move    b0,b");           //
   fprintf(fp,"\n        move    y:(TMP+7),x0");   // start1->x0
   fprintf(fp,"\n        add     x0,b");           // n*(end1-start1)/nrSteps + start1 -> b
   fprintf(fp,"\n        lsl     #8,b");           // b = b*2^8
   fprintf(fp,"\n        movep   b1,x:A_TX10");    // result->gradient output

// Include a delay to make the step the right length
   fprintf(fp,"\n        move    y:(TMP+2),a1"); 
   fprintf(fp,"\n        sub     #47,a");
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");


// Set to second gradient
   if(n >= 8)
   {
      fprintf(fp,"\n; **************************************************");
      fprintf(fp,"\n; v = n*(end2-start2)/steps + start2");
      fprintf(fp,"\n; **************************************************");
      fprintf(fp,"\n        move    y:(TMP+4),a");    // Set gradient to 2
      fprintf(fp,"\n        movep   a1,x:A_PDRE");

      fprintf(fp,"\n        move    y:(TMP+12),b");   // end2 -> b
      fprintf(fp,"\n        move    b1,y:(TMP+0)");   // end2 -> mem0
      fprintf(fp,"\n        move    y:(TMP+8),x1");   // start2 -> x1 
      fprintf(fp,"\n        move    y:(TMP+0),b");    // tmp0 -> b
      fprintf(fp,"\n        sub     x1,b");           // b-x1->b (end2-start2)->b
      fprintf(fp,"\n        move    b1,x0");          // b->x0 x0=(end2-start2)
      fprintf(fp,"\n        move    y:(TMP+15),y0");  // n->y0

      fprintf(fp,"\n        mpy     y0,x0,b");        // y0*x0->b ie. n1*(end2-start2)->b
      fprintf(fp,"\n        asl     #23,b,b");        // b = b * 2^23 part of multiply

      fprintf(fp,"\n        move    y:(TMP+1),y0");   // nrSteps->y0
      fprintf(fp,"\n        tfr     b,a");            // b->a (efficient move of all 56 bits)
      fprintf(fp,"\n        abs     b");              // |b|->b
      fprintf(fp,"\n        clr     b	b1,x0");       // 0->b 0->x0 (?)
      fprintf(fp,"\n        move    x0,b0");          // 0->b0 (?)
      fprintf(fp,"\n        asl     b");              // ??
      fprintf(fp,"\n        rep     #$18");           // Generates 18 bits of divide precision
      fprintf(fp,"\n        div     y0,b");           // n*(end2-start2)/nrSteps
      fprintf(fp,"\n        eor     y0,a");           // fix remainer
      fprintf(fp,"\n        bpl     LBL%ld",label+2); // Test if divisor is positive
      fprintf(fp,"\n        neg     b");              // negate otherwise
      fprintf(fp,"\nLBL%ld    nop",label+2);
      fprintf(fp,"\n        move    b0,b");           // result->tmp3
      fprintf(fp,"\n        move    y:(TMP+8),x0");   // start2->x0
      fprintf(fp,"\n        add     x0,b");           // n*(end2-start2)/nrSteps + start2 -> b
      fprintf(fp,"\n        lsl     #8,b");           // b = b*2^8
      fprintf(fp,"\n        movep   b1,x:A_TX10");    // result->gradient output

   // Include a delay to make the step the right length
      fprintf(fp,"\n        move    y:(TMP+2),a1"); 
      fprintf(fp,"\n        sub     #47,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
   }

// Set to third gradient
   if(n >= 11)
   {
      fprintf(fp,"\n; **************************************************");
      fprintf(fp,"\n; v = n*(end3-start3)/steps + start3");
      fprintf(fp,"\n; **************************************************");
      fprintf(fp,"\n        move    y:(TMP+5),a");    // Set gradient to 3
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(TMP+13),b");   // end3 -> b
      fprintf(fp,"\n        move    b1,y:(TMP+0)");   // end3 -> mem0
      fprintf(fp,"\n        move    y:(TMP+9),x1");   // start3-> x1 
      fprintf(fp,"\n        move    y:(TMP+0),b");    // tmp0 -> b
      fprintf(fp,"\n        sub     x1,b");           // b-x1->b (end3-start3)->b
      fprintf(fp,"\n        move    b1,x0");          // b->x0 x0=(end3-start3)
      fprintf(fp,"\n        move    y:(TMP+15),y0");  // n->y0

      fprintf(fp,"\n        mpy     y0,x0,b");        // y0*x0->b ie. n*(end3-start3)->b
      fprintf(fp,"\n        asl     #23,b,b");        // b = b * 2^23 part of multiply

      fprintf(fp,"\n        move    y:(TMP+1),y0");   // nrSteps->y0
      fprintf(fp,"\n        tfr     b,a");            // b->a (efficient move of all 56 bits)
      fprintf(fp,"\n        abs     b");              // |b|->b
      fprintf(fp,"\n        clr     b	b1,x0");       // 0->b 0->x0 (?)
      fprintf(fp,"\n        move    x0,b0");          // 0->b0 (?)
      fprintf(fp,"\n        asl     b");              // ??
      fprintf(fp,"\n        rep     #$18");           // Generates 18 bits of divide precision
      fprintf(fp,"\n        div     y0,b");           // n*(end3-start3)/nrSteps
      fprintf(fp,"\n        eor     y0,a");           // fix remainer
      fprintf(fp,"\n        bpl     LBL%ld",label+3); // Test if divisor is positive
      fprintf(fp,"\n        neg     b");              // negate otherwise
      fprintf(fp,"\nLBL%ld    nop",label+3);
      fprintf(fp,"\n        move    b0,b");           // 
      fprintf(fp,"\n        move    y:(TMP+9),x0");   // start3->x0
      fprintf(fp,"\n        add     x0,b");           // n*(end3-start3)/nrSteps + start3 -> b
      fprintf(fp,"\n        lsl     #8,b");           // b = b*2^8
      fprintf(fp,"\n        movep   b1,x:A_TX10");    // result->gradient output

   // Include a delay to make the step the right length
      fprintf(fp,"\n        move    y:(TMP+2),a1"); 
      fprintf(fp,"\n        sub     #47,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
   }

// Set to fourth gradient
   if(n >= 14)
   {

      fprintf(fp,"\n; **************************************************");
      fprintf(fp,"\n; v = n*(end4-start4)/steps + start4");
      fprintf(fp,"\n; **************************************************");
      fprintf(fp,"\n        move    y:(TMP+6),a");    // Set gradient to 4
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(TMP+14),b");   // end4 -> b
      fprintf(fp,"\n        move    b1,y:(TMP+0)");   // end4 -> mem0
      fprintf(fp,"\n        move    y:(TMP+10),x1");  // start4-> x1 
      fprintf(fp,"\n        move    y:(TMP+0),b");    // tmp0 -> b
      fprintf(fp,"\n        sub     x1,b");           // b-x1->b (end4-start4)->b
      fprintf(fp,"\n        move    b1,x0");          // b->x0 x0=(end4-start4)
      fprintf(fp,"\n        move    y:(TMP+15),y0");  // n->y0

      fprintf(fp,"\n        mpy     y0,x0,b");        // y0*x0->b ie. n*(end4-start4)->b
      fprintf(fp,"\n        asl     #23,b,b");        // b = b * 2^23 part of multiply

      fprintf(fp,"\n        move    y:(TMP+1),y0");   // nrSteps->y0
      fprintf(fp,"\n        tfr     b,a");            // b->a (efficient move of all 56 bits)
      fprintf(fp,"\n        abs     b");              // |b|->b
      fprintf(fp,"\n        clr     b	b1,x0");       // 0->b 0->x0 (?)
      fprintf(fp,"\n        move    x0,b0");          // 0->b0 (?)
      fprintf(fp,"\n        asl     b");              // ??
      fprintf(fp,"\n        rep     #$18");           // Generates 18 bits of divide precision
      fprintf(fp,"\n        div     y0,b");           // n*(end4-start4)/nrSteps
      fprintf(fp,"\n        eor     y0,a");           // fix remainer
      fprintf(fp,"\n        bpl     LBL%ld",label+4);  // Test if divisor is positive
      fprintf(fp,"\n        neg     b");               // negate otherwise
      fprintf(fp,"\nLBL%ld    nop",label+4);
      fprintf(fp,"\n        move    b0,b");            // result->tmp3
      fprintf(fp,"\n        move    y:(TMP+10),x0");   // start4->x0
      fprintf(fp,"\n        add     x0,b");            // n*(end4-start4)/nrSteps + start4 -> b
      fprintf(fp,"\n        lsl     #8,b");            // b = b*2^8
      fprintf(fp,"\n        movep   b1,x:A_TX10");     // result->gradient output

   // Include a delay to make the step the right length
      fprintf(fp,"\n        move    y:(TMP+2),a1"); 
      fprintf(fp,"\n        sub     #47,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
   }
// Increment gradient step loop counter
   fprintf(fp,"\n        move    y:(TMP+15),a0");  // Increment count n
   fprintf(fp,"\n        inc    a"); 
   fprintf(fp,"\n        move    a0,y:(TMP+15)");  
 
// End of loop label
   fprintf(fp,"\nLBL%ld    nop",label);

// Include a delay to make the last step the right length
   fprintf(fp,"\n        rep     #80                  ; Wait 1us");
   fprintf(fp,"\n        nop");

   fprintf(fp,"\n        movep   #$24,x:A_PCRD          ; Turn off SSI 1 on Port D");

// Update label counter
   if(n == 5)
      label += 2;
   else if(n == 8)
      label += 3;
   else if(n == 11)
      label += 4;
   else if(n == 14)
      label += 5;

   fclose(fp);

   return(OK);
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

short GetGradientNumber(Interface *itfc , FILE *fp, CText grad)
{
   long Grad;

// Choose first gradient to change
   if(grad[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",grad.Str()+1);
   }
   else if(grad == "x")
   {
      fprintf(fp,"\n        move    #3,a1");
   }
   else if(grad == "y")
   {
      fprintf(fp,"\n        move    #2,a1");
   }
   else if(grad == "z")
   {
      fprintf(fp,"\n        move    #1,a1");
   }
   else if(grad == "o")
   {
      fprintf(fp,"\n        move    #0,a1");
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
   }
	else
   {
      Error(itfc,"invalid gradient index '%s' [0,1,2,3] or [x,y,z,o]",grad);
      fclose(fp);
      return(ERR);
   }

   return(OK);
}

short GetGradientAmplitude(Interface *itfc , FILE *fp, CText var)
{
   long amplitude;

   if(var[0] == 'n') // Start amplitude is via a number reference
   {
      fprintf(fp,"\n        move    x:NR%s,a1",var.Str()+1);
   }
   else if(var[0] == 't') // start amplitude is t[index]
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",var.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",var.Str()+1);

      fprintf(fp,"\n        add     b,a"); // Add current index to table start address
      fprintf(fp,"\n        move    a0,r5"); 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read table value
   }
   else if(sscanf(var.Str(),"%ld",&amplitude) == 1) // Start amplitude is a number
   {
      fprintf(fp,"\n        move    #%ld,a1",amplitude);
   }
   else
   {
      Error(itfc,"Invalid start amplitude '%s'",var.Str());
      fclose(fp);
      return(ERR);
   }
   return(OK);
}

short SelectGradient15(Interface *itfc ,FILE* fp, CText grad)
{
   long Grad;
   long Address;

// Choose the gradient channel
   fprintf(fp,"\n        clr     a");
   if(grad[0] == 'n')
   {
      fprintf(fp,"\n        move    x:NR%s,a1",grad.Str()+1);
   }
   else if(sscanf(grad.Str(),"%ld",&Address) == 1) // channel is a number
   {
      if(Address < 0 || Address > 15)
      {
         Error(itfc,"invalid gradient index '%s' [0, ... 15]",grad.Str());
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #%ld,a1",Address);
   }
	else
   {
      Error(itfc,"invalid gradient index '%s' [0, ... 15]",grad.Str());
      fclose(fp);
      return(ERR);
   }

   fprintf(fp,"\n        move    #$0001,b1               ; Select first group");
   fprintf(fp,"\n        cmp    #8,a                     ; See if channel is < 8");
   fprintf(fp,"\n        jlt    LBL%ld",label);
   fprintf(fp,"\n        sub    #8,a                     ; Subtract 8");
   fprintf(fp,"\n        move    #$0000,b1               ; Select second group");
   fprintf(fp,"\nLBL%ld  nop",label++);
   fprintf(fp,"\n        movep   b1,x:A_PDRC");
   fprintf(fp,"\n        movep   a1,x:A_PDRE");

   return(OK);
}

short SelectGradient16(Interface *itfc ,FILE* fp, CText grad)
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
      if(Channel < 0 || Channel > 16)
      {
         Error(itfc,"invalid gradient index '%s' [0, ... 16]",grad.Str());
         fclose(fp);
         return(ERR);
      }
      fprintf(fp,"\n        move    #%ld,a1",Channel);
   }
	else
   {
      Error(itfc,"invalid gradient index '%s' [0, ... 16]",grad.Str());
      fclose(fp);
      return(ERR);
   }

   fprintf(fp,"\n        move    #$0001,b1               ; Assume first group");
   fprintf(fp,"\n        cmp    #8,a                     ; See if channel is < 8");
   fprintf(fp,"\n        jlt    LBL%ld",label);
   fprintf(fp,"\n        sub    #8,a                     ; Subtract 8");
   fprintf(fp,"\n        move    #$0000,b1               ; Second group");
   fprintf(fp,"\nLBL%ld  nop",label++);
   fprintf(fp,"\n        movep   b1,x:A_PDRC             ; Select group of 8 DACs");

   fprintf(fp,"\n        move    a1,b0                   ; Save subgroup (4 channels)");
   fprintf(fp,"\n        lsr     #2,a                    ; Shift 2 bits to right to determine subgroup (result 0/1)");                     // 
   fprintf(fp,"\n        add     #4,a                    ; Add 4 to only access pins Y4 or Y5 on U7"); 
   fprintf(fp,"\n        move    a1,x:A_PDRE             ; Select block of 4 DACs");


   return(OK);
}

//
//
//// Set the amplitude
//   if(amplitude[0] != 'n')
//   {
//      if(sscanf(amplitude.Str(),"%ld",&Amplitude) == 1)
//      {
//         if(Amplitude > 32768 || Amplitude < -32767)
//         {
//            Error(par->itfc,"invalid shim amplitude '%s' [-32767 ... 32768]",channel);
//            fclose(fp);
//            return(ERR);
//         }
//      }
//	   else
//      {
//         Error(par->itfc,"invalid shim amplitude '%s' [-32767 ... 32768]",channel);
//         fclose(fp);
//         return(ERR);
//      }
//      fprintf(fp,"\n        move    #$%X,a1",Amplitude); 
//   }
//   else
//   {
//      fprintf(fp,"\n        move    x:NR%s,a1",amplitude.Str()+1); 
//   }
//   fprintf(fp,"\n        move    #$00FFFF,x1"); 
//   fprintf(fp,"\n        and      x1,a1"); 
//
//
//   if(channel[0] == 'n')
//      fprintf(fp,"\n        move    x:NR%s,b1                ; Load channel number",channel.Str()+1);
//   else
//      fprintf(fp,"\n        move    #%ld,b1                  ; Load channel number",Channel);
//
//   fprintf(fp,"\n        cmp     #8,b                    ; See if channel is < 8");
//   fprintf(fp,"\n        jlt     LBL%ld",label);
//   fprintf(fp,"\n        sub     #8,b                    ; Subtract 8");
//   fprintf(fp,"\nLBL%ld  nop",label++);
//   fprintf(fp,"\n        lsl     #16,b");                    // Shift 16 bits to left
//   fprintf(fp,"\n        move    #$030000,x1");               // Extract lower 2 bits of channel number
//   fprintf(fp,"\n        and      x1,b"); 
//   fprintf(fp,"\n        move    #$100000,x1");               // Set data register mode bit
//   fprintf(fp,"\n        or       x1,b"); 
//   fprintf(fp,"\n        move     b1,x1");
//   fprintf(fp,"\n        or       x1,a"); 
//
//   fprintf(fp,"\n        move    a1,x:A_TX10            ; Send data to DAC"); 
//   fprintf(fp,"\n        move    #3,r7                  ; Wait 3 us");
//   fprintf(fp,"\n        bsr     wait");
//   fprintf(fp,"\n        movep   #$24,x:A_PCRD           ; Turn off SSI 1 on Port D");

/**************************************************************************************************
Utility routine which initializes a register with a number either as a variable or a constant.
also includes bounds check in the case of a constant
***************************************************************************************************/

short SelectNumber(Interface *itfc ,FILE* fp, CText num, char *reg, long min, long max)
{
   long Number;

   if(num[0] == 'n')
	{
	   fprintf(fp,"\n        move    x:NR%s,%s",num.Str()+1,reg);
	}
	else if(sscanf(num.Str(),"%ld",&Number) == 1)
	{
		if(Number < 2 || Number > 65536)
		{
			Error(itfc,"invalid number of points '%ld' [%ld ... %ld]",Number,min,max);
			fclose(fp);
         return(ERR);
		}
      fprintf(fp,"\n        move    #%ld,%s",Number,reg);
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
   if(channel == "1" || channel == "1nb" || channel == "i" || channel == "e" ||
      channel == "w1" || channel == "wi" || channel == "we")
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
   else  if(amp[0] == 't') // Table based amplitude
   {
      fprintf(fp,"\n        clr a");
      fprintf(fp,"\n        clr b");
      fprintf(fp,"\n        move    x:TABLE%s,a0",amp.Str()+1);
      fprintf(fp,"\n        dec a"); // a0 points to table index
      fprintf(fp,"\n        move    a0,r5"); // Read current table index
      fprintf(fp,"\n        move    y:(r5),a0");

      fprintf(fp,"\n        move    x:TABLE%s,b0",amp.Str()+1);

      fprintf(fp,"\n        add     b,a");  // Add the index to table start to find current value
      fprintf(fp,"\n        move    a0,r5"); // 
      fprintf(fp,"\n        move    y:(r5),a1"); // Read the table value
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
   if(channel == "1" || channel == "1nb" || channel == "i" || channel == "e" ||
      channel == "w1" || channel == "wi" || channel == "we")
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
	   fprintf(fp,"\n        move   x:DELAY%s,%s",duration.Str()+1,reg);
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
      fprintf(fp,"\n        move   #%ld,%s",Duration,reg);
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
   else if (freq[0] == 't') // Frequency is a table - note that registers a, b and r5 as well as reg are modified
   {
      fprintf(fp, "\n        clr a");
      fprintf(fp, "\n        clr b");
      fprintf(fp, "\n        move    x:TABLE%s,a0", freq.Str() + 1);
      fprintf(fp, "\n        dec a");             // a0 points to table index
      fprintf(fp, "\n        move    a0,r5");     // Read current table index
      fprintf(fp, "\n        move    y:(r5),a0");
      fprintf(fp, "\n        move    x:TABLE%s,b0", freq.Str() + 1);
      fprintf(fp, "\n        add     b,a");       // Add the index to table start to find current value
      fprintf(fp, "\n        move    a0,r5");

      if (channel == "1" || channel == "1nb" || channel == "w1" || channel == "wi")
      {
         fprintf(fp, "\n        move    y:(r5)+,a1"); // Read the high freq word from the table
         fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");
         fprintf(fp, "\n        move    y:(r5),a1"); // Read the low freq word from the table
         fprintf(fp, "\n        move    a1,x:FPGA_DDS1_Pro0");
      }
      else
      {
         fprintf(fp, "\n        move    y:(r5)+,a1"); // Read the high freq word from the table
         fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");
         fprintf(fp, "\n        move    y:(r5),a1"); // Read the low freq word from the table
         fprintf(fp, "\n        move    a1,x:FPGA_DDS2_Pro0");
      }
   }
   // This code is not reliable - for some reason the pulse duration becomes unstable for some
   // frequencies.
   else if(sscanf(freq.Str(),"%lf",&fFreq) == 1) // Fixed frequency
   {
      if(fFreq < 0 || fFreq > 500)
      {
         Error(itfc,"invalid frequency '%lg' [0 ... 500]",fFreq);
         fclose(fp);
         return(ERR);
      }
      __int64 DDSFword = (__int64)((fFreq * 4294967296.0L)/1000.0L + 0.5); 
      unsigned long w1  = (unsigned long)((DDSFword & 0xFFFF0000)/65536.0L);
      unsigned long w2 = (unsigned long)(DDSFword & 0x0000FFFF);

      if(channel == "1")
      {
         fprintf(fp,"\n        move    #%lu,a1",w1);
         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
         fprintf(fp,"\n        move    #%lu,a1",w2);
         fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
      }
      else if(channel == "2")
      {
         fprintf(fp,"\n        move    #%lu,a1",w1);
         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
         fprintf(fp,"\n        move    #%lu,a1",w2);
         fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
      }
      fprintf(fp,"\n        nop                             ; Eliminate pulse length jitter");
   }
   else
   {
      Error(itfc,"Invalid frequency reference '%s'",freq.Str());
      fclose(fp);
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

   if(channel != "i" && channel != "e" && channel != "1" && channel != "2")
   {
      ErrorMessage("unknown RF chirp pulse channel");
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

   if(channel == "i") // Internal channel 1 Kea pulse
	   fprintf(fp,"\n        or      #$004000,a              ; Internal HPA");
   else if(channel == "1" || channel == "e") // External channel 1 Kea pulse
		fprintf(fp,"\n        or      #$010000,a              ; TTL 0x01 (pin 5)");
   else
      fprintf(fp,"\n        or      #$040000,a               ; TTL 0x04 (pin 4)");
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
	if(channel == "2")
		fprintf(fp,"\n        or      #$040002,a             ; Channel 2 RF on");
	else if(channel == "1" || channel == "e")
		fprintf(fp,"\n        or      #$010008,a             ; Channel 1/e RF on");
   else
		fprintf(fp,"\n        or      #$004008,a             ; Channel i RF on");

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

 //  if(channel == "1" || channel == "e")
 //     fprintf(fp,"\n        and     #$FEFFF7,a               ; Switch off channel 1");
 //  else if(channel == "i")
 //     fprintf(fp,"\n        and     #$FFBFF7,a               ; Switch off internal channel");
 //  else if(channel == "2")
 //     fprintf(fp,"\n        and     #$FBFFFD,a               ; Switch off channel 2");


	//fprintf(fp,"\n        move    a1,y:TTL                 ; Load the current TTL word");
   fprintf(fp,"\n        move    a1,x:FPGA_TTL            ; Update TTL & RF");

   fprintf(fp,"\n        move    #$000000,a1"); 

	if(channel == "2")
	{
		fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ;Zero amplitude");
		fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Zero phase");
		fprintf(fp,"\n        move    x:TXF00,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0      ; Set last saved frequency");
		fprintf(fp,"\n        move    x:TXF01,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_DDS2_Pro0");
	}
	else
	{
		fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ;Zero amplitude");
		fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Zero phase");
		fprintf(fp,"\n        move    x:TXF00,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0      ; Set last saved frequency");
		fprintf(fp,"\n        move    x:TXF01,a1");
		fprintf(fp,"\n        move    a1,x:FPGA_DDS1_Pro0");
	}

	fclose(fp);
  

   return(OK);
}