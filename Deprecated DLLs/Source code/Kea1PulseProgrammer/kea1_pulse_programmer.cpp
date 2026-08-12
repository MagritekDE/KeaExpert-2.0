/*************************************************************
              Kea Pulse Programmer

Provides DLL commands to generate pulse sequences for the 
Kea spectrometer.

*************************************************************/

#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <shellapi.h>
// Locally defined procedure and global variables

#define VERSION 2.02  // 30/9/11

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);
short MakeADelay(DLLParameters*,char*);
short InitialisePP(DLLParameters*,char*);
short MakeAnRFPulse(DLLParameters*,char*);
short TTLPulse(DLLParameters*,char*);
short PulseGradient(DLLParameters*,char*);
short SwitchOnGradient(DLLParameters*,char*);
short SwitchOffGradient(DLLParameters*,char*);
short LoopStart(DLLParameters*,char *args);
short LoopEnd(DLLParameters*,char *args);
short ClearData(DLLParameters*,char *args);
short SetTTL(DLLParameters*,char *args);
short ExecuteAndWait(DLLParameters*,char*);
short AcquireData(DLLParameters*,char *args);
short MakeALongDelay(DLLParameters*,char *args);
short UpdateFrequencies(DLLParameters*,char *args);
short IncFrequency(DLLParameters*,char *args);
short IncRxFrequency(DLLParameters*,char *args);
short DecRxFrequency(DLLParameters* par, char *args);
short IncTxFrequency(DLLParameters*,char *args);
short DecTxFrequency(DLLParameters* par, char *args);
short SwitchOnTx(DLLParameters*,char *args);
short SwitchOffTx(DLLParameters*,char *args);
short SetRxGain(DLLParameters*,char *args);
short SetRxFreq(DLLParameters*,char *args);
short SetTxFreq(DLLParameters*,char *args);
short IncTxAmplitude(DLLParameters*,char *args);
short ResetMemoryPointer(DLLParameters*,char *args);
short GetPPVersion(DLLParameters*,char *args);
short TTLOn(DLLParameters*,char *args);
short TTLOff(DLLParameters*,char *args);
short TTLTranslate(DLLParameters*,char *args);
short ShapedRF(DLLParameters*,char *args);
short ShapedRF2(DLLParameters*,char *args);
short ShapedGradientPulse(DLLParameters*,char *args);
short GetHelpFolder(DLLParameters*,char *args);
short SetTableIndex(DLLParameters*,char *args);
short IncrementTableIndex(DLLParameters*,char *args);
short DecrementTableIndex(DLLParameters*,char *args);
short EndPP(DLLParameters*,char *args);
short RampedGradient(DLLParameters* par, char *args);
short ChirpedRF(DLLParameters* par, char *args);

void InsertUniqueStringIntoList(char *str, char ***list, long &position);


char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list
long label;     // label counter (to generate unique label)

// Extension procedure to add commands to Prospa 

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;
      
        if(!strcmp(command,"acquire"))      r = AcquireData(dpar,parameters);  
   else if(!strcmp(command,"cleardata"))    r = ClearData(dpar,parameters);      
   else if(!strcmp(command,"chirprf"))      r = ChirpedRF(dpar,parameters);      
   else if(!strcmp(command,"delay"))        r = MakeADelay(dpar,parameters);   
   else if(!strcmp(command,"decindex"))     r = DecrementTableIndex(dpar,parameters);  
   else if(!strcmp(command,"decrxfreq"))    r = DecRxFrequency(dpar,parameters);
   else if(!strcmp(command,"dectxfreq"))    r = DecTxFrequency(dpar,parameters);
   else if(!strcmp(command,"endloop"))      r = LoopEnd(dpar,parameters);      
   else if(!strcmp(command,"endpp"))        r = EndPP(dpar,parameters);  
   else if(!strcmp(command,"execwait"))     r = ExecuteAndWait(dpar,parameters);
   else if(!strcmp(command,"gradon"))       r = SwitchOnGradient(dpar,parameters);   
   else if(!strcmp(command,"gradoff"))      r = SwitchOffGradient(dpar,parameters);   
   else if(!strcmp(command,"helpfolder"))   r = GetHelpFolder(dpar,parameters);  
   else if(!strcmp(command,"incindex"))     r = IncrementTableIndex(dpar,parameters);   
   else if(!strcmp(command,"inctxamp"))     r = IncTxAmplitude(dpar,parameters);   
   else if(!strcmp(command,"incrxfreq"))    r = IncRxFrequency(dpar,parameters);   
   else if(!strcmp(command,"inctxfreq"))    r = IncTxFrequency(dpar,parameters);
   else if(!strcmp(command,"initpp"))       r = InitialisePP(dpar,parameters);      
   else if(!strcmp(command,"loop"))         r = LoopStart(dpar,parameters);     
   else if(!strcmp(command,"memreset"))     r = ResetMemoryPointer(dpar,parameters);     
   else if(!strcmp(command,"ppversion"))    r = GetPPVersion(dpar,parameters);      
   else if(!strcmp(command,"pulse"))        r = MakeAnRFPulse(dpar,parameters); 
   else if(!strcmp(command,"gradramp"))     r = RampedGradient(dpar,parameters);    
   else if(!strcmp(command,"setindex"))     r = SetTableIndex(dpar,parameters);   
   else if(!strcmp(command,"setrxfreq"))    r = SetRxFreq(dpar,parameters);   
   else if(!strcmp(command,"setrxgain"))    r = SetRxGain(dpar,parameters);   
   else if(!strcmp(command,"settxfreq"))    r = SetTxFreq(dpar,parameters); 
   else if(!strcmp(command,"shapedgrad"))   r = ShapedGradientPulse(dpar,parameters);   
   else if(!strcmp(command,"shapedrf"))     r = ShapedRF(dpar,parameters);      
   else if(!strcmp(command,"ttl"))          r = SetTTL(dpar,parameters);      
   else if(!strcmp(command,"ttlon"))        r = TTLOn(dpar,parameters);      
   else if(!strcmp(command,"ttloff"))       r = TTLOff(dpar,parameters);      
   else if(!strcmp(command,"ttlpulse"))     r = TTLPulse(dpar,parameters);      
   else if(!strcmp(command,"ttltranslate")) r = TTLTranslate(dpar,parameters);      
   else if(!strcmp(command,"txoff"))        r = SwitchOffTx(dpar,parameters);   
   else if(!strcmp(command,"txon"))         r = SwitchOnTx(dpar,parameters);   
   else if(!strcmp(command,"wait"))         r = MakeALongDelay(dpar,parameters);   
    
                
   return(r);
}

// Extension procedure to list commands in DLL 

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   Kea-1 Pulse Programmer DLL module (V%1.2f)\n\n",VERSION);
   TextMessage("   acquire ..... acquire some data\n");
   TextMessage("   cleardata ... clear data memory\n");
   TextMessage("   chirprf ..... make a frequency and amplitude moduated RF pulse\n");
   TextMessage("   decindex .... decrement a table index\n");
   TextMessage("   decrxfreq ... decrement the rx frequency\n");
   TextMessage("   dectxfreq ... decrement the tx frequency\n");
   TextMessage("   delay ....... generate a short delay\n");
   TextMessage("   endloop ..... end a loop\n");
   TextMessage("   endpp ....... finish the pulse program\n");
   TextMessage("   execwait .... execute a program and wait for it to exit\n");
   TextMessage("   gradon ...... set a gradient\n");
   TextMessage("   gradoff ..... zero a gradient\n");
   TextMessage("   gradramp .... change a gradient via linear ramp\n");
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
   TextMessage("   setrxgain ... set the receive amplifier gain\n");
   TextMessage("   settxfreq ... set the pulse frequency\n");
   TextMessage("   shapedrf .... make a phase and amplitude moduated RF pulse\n");
   TextMessage("   shapedgrad .. make an amplitude modulated gradient pulse\n");
   TextMessage("   ttl ......... set TTL levels\n");
   TextMessage("   ttlon ....... switch on a TTL level\n");
   TextMessage("   ttloff ...... switch off a TTL level\n");
   TextMessage("   ttlpulse .... generate TTL pulse\n");
   TextMessage("   ttltranslate  translate TTL pin number to byte code\n");
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

   if(!strcmp(cmd,"acquire"))            strcpy(syntax,"acquire(mode, number points:n, [duration:d])");
   else if(!strcmp(cmd,"cleardata"))     strcpy(syntax,"cleardata(number:n)");
   else if(!strcmp(cmd,"chirprf"))       strcpy(syntax,"chirprf(mode, atable:t, ftable:f, phase:p, table_size:n, table_step_duration:d)");
   else if(!strcmp(cmd,"decindex"))      strcpy(syntax,"decindex(table:t)");
   else if(!strcmp(cmd,"decrxfreq"))     strcpy(syntax,"decrxfreq(decrement:f)");
   else if(!strcmp(cmd,"dectxfreq"))     strcpy(syntax,"dectxfreq(decrement:f)");
   else if(!strcmp(cmd,"delay"))         strcpy(syntax,"delay(duration:d)");
   else if(!strcmp(cmd,"endloop"))       strcpy(syntax,"endloop(name)");
   else if(!strcmp(cmd,"endpp"))         strcpy(syntax,"endpp(mode)");
   else if(!strcmp(cmd,"execwait"))      strcpy(syntax,"execwait(program,arguments)");
   else if(!strcmp(cmd,"gradon"))        strcpy(syntax,"gradon(address:n,level:n/t)");
   else if(!strcmp(cmd,"gradoff"))       strcpy(syntax,"gradoff(address:n)");
   else if(!strcmp(cmd,"gradramp"))      strcpy(syntax,"gradramp(address:n, start:n/t, end:n/t, steps:n, delay:d)");
   else if(!strcmp(cmd,"incindex"))      strcpy(syntax,"incindex(table:t)");
   else if(!strcmp(cmd,"incrxfreq"))     strcpy(syntax,"incrxfreq(increment:f)");
   else if(!strcmp(cmd,"inctxfreq"))     strcpy(syntax,"inctxfreq(increment:f)");
   else if(!strcmp(cmd,"inctxamp"))      strcpy(syntax,"inctxamp(amp:a, increment:a)");
   else if(!strcmp(cmd,"initpp"))        strcpy(syntax,"initpp(filename)");
   else if(!strcmp(cmd,"loop"))          strcpy(syntax,"loop(name,n)");
   else if(!strcmp(cmd,"memreset"))      strcpy(syntax,"memreset([address:n])");
   else if(!strcmp(cmd,"ppversion"))     strcpy(syntax,"(INT v) = ppversion()");
   else if(!strcmp(cmd,"pulse"))         strcpy(syntax,"pulse(mode,amp:a, phase:p, duration:d)");
   else if(!strcmp(cmd,"setindex"))      strcpy(syntax,"setindex(table:t,index:n)");
   else if(!strcmp(cmd,"setrxfreq"))     strcpy(syntax,"setrxfreq(freq:f)");
   else if(!strcmp(cmd,"settxfreq"))     strcpy(syntax,"settxfreq(freq:f)");
   else if(!strcmp(cmd,"setrxgain"))     strcpy(syntax,"setrxgain(gain:g)");
   else if(!strcmp(cmd,"shapedrf"))      strcpy(syntax,"shapedrf(mode, atable:t, stable:t, phase:p, table_size:n, table_step_duration:d)");
   else if(!strcmp(cmd,"shapedgrad"))    strcpy(syntax,"shapedgrad(address:n, atable:t, table_size:n, table_step_duration:d)");
   else if(!strcmp(cmd,"ttltranslate"))  strcpy(syntax,"(INT byte) = ttltranslate(pin number)");
   else if(!strcmp(cmd,"ttl"))           strcpy(syntax,"ttl(byte:b)");
   else if(!strcmp(cmd,"ttlon"))         strcpy(syntax,"ttlon(byte:b)");
   else if(!strcmp(cmd,"ttloff"))        strcpy(syntax,"ttloff(byte:b)");
   else if(!strcmp(cmd,"ttlpulse"))      strcpy(syntax,"ttlpulse(byte:b, duration:d)");
   else if(!strcmp(cmd,"txoff"))         strcpy(syntax,"txoff()");
   else if(!strcmp(cmd,"txon"))          strcpy(syntax,"txon(mode, [amp:a, phase:o])");
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
      ErrorMessage("directory '%s' not found",dir);
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(adrs.Str(),&parList,szList);


   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   if(nrArgs == 0)
   {
      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Reset the memory pointer");
	   fprintf(fp,"\n        move    #$10000,r5              ; Make r5 point to the start of fid memory");
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
      ErrorMessage("Invalid pin number (2-9)");
      return(ERR);
   }

  code = (float)translate[pinNr];

  par->retVar[1].MakeAndSetFloat(code);
  par->nrRetVar = 1;

  return(OK);
}



/**********************************************************************
              Increment the transmitter frequency (4/11/07)

     Adds a constant to the current tx frequency and then send this 
     out to the DDS.

     inctxfreq(increment_value)


     increment_value ......... a frequency variable (e.g. "f1") units MHz

     Command takes 1250 ns.

**********************************************************************/

short IncTxFrequency(DLLParameters* par, char *args)
{
   short nrArgs;
   CText incTx;

   if((nrArgs = ArgScan(par->itfc,args,1,"increment name","e","q",&incTx)) < 0)
      return(nrArgs);

   if(incTx[0] != 'f')
   {
      ErrorMessage("Invalid frequency reference '%s'",incTx.Str());
      return(ERR);
   }

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(incTx.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Increment the transmitter frequency");

   fprintf(fp,"\n\n; Read in the base transmitter frequency");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    x:TXF00,a2");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:TXF01,a1");
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:TXF02,a1");
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:TXF03,a1");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Add Tx frequency step to 6620");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_0,a2",incTx.Str()+1);
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_1,a1",incTx.Str()+1);
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_2,a1",incTx.Str()+1);
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_3,a1",incTx.Str()+1);
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Write over base frequency");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$0000ff,a");
   fprintf(fp,"\n        move    a1,x:TXF03");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$00ff00,a");
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,x:TXF02");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$ff0000,a");
   fprintf(fp,"\n        lsr     #16,a");
   fprintf(fp,"\n        move    a1,x:TXF01");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        move    a2,x:TXF00");

   fprintf(fp,"\n\n; Update the Tx frequency");

   fprintf(fp,"\n        move    x:TXF00,a1");
   fprintf(fp,"\n        move    a1,x:$20084");
   fprintf(fp,"\n        move    x:TXF01,a1");
   fprintf(fp,"\n        move    a1,x:$20085");
   fprintf(fp,"\n        move    x:TXF02,a1");
   fprintf(fp,"\n        move    a1,x:$20086");
   fprintf(fp,"\n        move    x:TXF03,a1");
   fprintf(fp,"\n        move    a1,x:$20087");

   fprintf(fp,"\n\n; Update phase by toggling 9852 synch");
   fprintf(fp,"\n        move    #$00040,a1              ; Set sync bit high");
   fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine");
   fprintf(fp,"\n        move    y:RF,x1                 ; Load the current RF state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine");
   fprintf(fp,"\n        move    a1,x:$20000             ; Update RF");
   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
   fprintf(fp,"\n        move    y:RF,x1                 ; Load the current RF state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine (sync bit now low)");
   fprintf(fp,"\n        move    a1,x:$20000             ; Update RF");


   fclose(fp);

   return(OK);
}


/**********************************************************************
              Decrement the transmitter frequency (30/8/11)

     Subtracts a constant from the current tx frequency and then sends this 
     out to the DDS.

     dectxfreq(decrement_value)


     decrement_value ......... a frequency variable (e.g. "f1") units MHz

     Command takes 1250 ns.

**********************************************************************/

short DecTxFrequency(DLLParameters* par, char *args)
{
   short nrArgs;
   CText decTx;

   if((nrArgs = ArgScan(par->itfc,args,1,"decrement variable name","e","q",&decTx)) < 0)
      return(nrArgs);

   if(decTx[0] != 'f')
   {
      ErrorMessage("Invalid frequency reference '%s'",decTx.Str());
      return(ERR);
   }

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(decTx.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Decrement the transmitter frequency");

   fprintf(fp,"\n\n; Read in the base transmitter frequency");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    x:TXF00,a2");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:TXF01,a1");
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:TXF02,a1");
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:TXF03,a1");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Subtract Tx frequency step from register b");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_0,a2",decTx.Str()+1);
   fprintf(fp,"\n        sub     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_1,a1",decTx.Str()+1);
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        sub     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_2,a1",decTx.Str()+1);
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        sub     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_3,a1",decTx.Str()+1);
   fprintf(fp,"\n        sub     a,b");

   fprintf(fp,"\n\n; Write over base frequency");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$0000ff,a");
   fprintf(fp,"\n        move    a1,x:TXF03");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$00ff00,a");
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,x:TXF02");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$ff0000,a");
   fprintf(fp,"\n        lsr     #16,a");
   fprintf(fp,"\n        move    a1,x:TXF01");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        move    a2,x:TXF00");

   fprintf(fp,"\n\n; Update the Tx frequency");

   fprintf(fp,"\n        move    x:TXF00,a1");
   fprintf(fp,"\n        move    a1,x:$20084");
   fprintf(fp,"\n        move    x:TXF01,a1");
   fprintf(fp,"\n        move    a1,x:$20085");
   fprintf(fp,"\n        move    x:TXF02,a1");
   fprintf(fp,"\n        move    a1,x:$20086");
   fprintf(fp,"\n        move    x:TXF03,a1");
   fprintf(fp,"\n        move    a1,x:$20087");

   fprintf(fp,"\n\n; Update phase by toggling 9852 synch");
   fprintf(fp,"\n        move    #$00040,a1              ; Set sync bit high");
   fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine");
   fprintf(fp,"\n        move    y:RF,x1                 ; Load the current RF state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine");
   fprintf(fp,"\n        move    a1,x:$20000             ; Update RF");
   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
   fprintf(fp,"\n        move    y:RF,x1                 ; Load the current RF state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine (sync bit now low)");
   fprintf(fp,"\n        move    a1,x:$20000             ; Update RF");


   fclose(fp);

   return(OK);
}

/**********************************************************************
              Increment the transmitter amplitude (4/11/07)

     Adds a constant to the current amplitude and then send this 
     out to the DDS.

     inctxamp(mode, amplitude_to_increment, increment_value)

     mode .................... "rf"/"number"  
                               Update the rf amplitude or just the number
     amplitude_to_increment .. variable to increment (e.g. "a1")
     increment_value ......... a 12 bit number variable (e.g. "n1")

     Command takes 720 ns in rf mode
**********************************************************************/

short IncTxAmplitude(DLLParameters* par, char *args)
{
   short nrArgs;
   CText amp,inc;

   if((nrArgs = ArgScan(par->itfc,args,2,"amplitude_name, increment_name","ee","qq",&amp,&inc)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(amp.Str(),&parList,szList);
   InsertUniqueStringIntoList(inc.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Increment the transmitter amplitude");

   fprintf(fp,"\n\n; Read in the base transmitter amplitude");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");

	fprintf(fp,"\n        move    x:TXA%s_1,a1",amp.Str()+1);
   fprintf(fp,"\n        add     a,b");
	fprintf(fp,"\n        move    x:TXA%s_0,a1",amp.Str()+1);
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        add     a,b");

 
   fprintf(fp,"\n\n; Add Tx amplitude step");
   fprintf(fp,"\n        clr     a");
	fprintf(fp,"\n        move    x:NR%s,a1",inc.Str()+1);
   fprintf(fp,"\n        add     a,b");


   fprintf(fp,"\n\n; Write over base amplitude");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$0000ff,a");
   fprintf(fp,"\n        move    a1,x:TXA%s_1",amp.Str()+1);
	fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$00ff00,a");
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,x:TXA%s_0",amp.Str()+1);
	fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");

   fprintf(fp,"\n\n; Update phase by toggling 9852 synch");
   fprintf(fp,"\n        move    #$00040,a1              ; Set sync bit high");
   fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine");
   fprintf(fp,"\n        move    y:RF,x1                 ; Load the current RF state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine");
   fprintf(fp,"\n        move    a1,x:$20000             ; Update RF");
   fprintf(fp,"\n        move    y:TTL,a1                ; Load the current TTL state");
   fprintf(fp,"\n        move    y:RF,x1                 ; Load the current RF state");
   fprintf(fp,"\n        or      x1,a1                   ; Combine (sync bit now low)");
   fprintf(fp,"\n        move    a1,x:$20000             ; Update RF");

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
      ErrorMessage("Invalid frequency reference '%s'",incRx.Str());
      return(ERR);
   }

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(incRx.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Increment the receiver frequency");

   fprintf(fp,"\n\n; Read in the base receiver frequency");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    x:RXF00,a2");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:RXF01,a1");
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:RXF02,a1");
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:RXF03,a1");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Add Rx frequency step to 6620");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_0,a2",incRx.Str()+1);
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_1,a1",incRx.Str()+1);
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_2,a1",incRx.Str()+1);
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_3,a1",incRx.Str()+1);
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Write over base frequency");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$0000ff,a");
   fprintf(fp,"\n        move    a1,x:RXF03");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$00ff00,a");
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,x:RXF02");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$ff0000,a");
   fprintf(fp,"\n        lsr     #16,a");
   fprintf(fp,"\n        move    a1,x:RXF01");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        move    a2,x:RXF00");

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Update Rx frequencies");
   fprintf(fp,"\n        move    #$03,a1");
   fprintf(fp,"\n        move    a1,x:$200C7");
   fprintf(fp,"\n        move    #$03,a1");
   fprintf(fp,"\n        move    a1,x:$200C6");
   fprintf(fp,"\n        move    x:RXF00,a1");
   fprintf(fp,"\n        move    a1,x:$200C3");
   fprintf(fp,"\n        move    x:RXF01,a1");
   fprintf(fp,"\n        move    a1,x:$200C2");
   fprintf(fp,"\n        move    x:RXF02,a1");
   fprintf(fp,"\n        move    a1,x:$200C1");
   fprintf(fp,"\n        move    x:RXF03,a1");
   fprintf(fp,"\n        move    a1,x:$200C0");

   fclose(fp);

   return(OK);
}


/**********************************************************************
              Decrement the receiver frequency (30/8/11)
**********************************************************************/

short DecRxFrequency(DLLParameters* par, char *args)
{
   short nrArgs;
   CText decRx;

   
   if((nrArgs = ArgScan(par->itfc,args,1,"decrement name","e","q",&decRx)) < 0)
      return(nrArgs);

   if(decRx[0] != 'f')
   {
      ErrorMessage("Invalid frequency reference '%s'",decRx.Str());
      return(ERR);
   }

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(decRx.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Decrement the receiver frequency");

   fprintf(fp,"\n\n; Read in the base receiver frequency");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    x:RXF00,a2");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:RXF01,a1");
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:RXF02,a1");
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:RXF03,a1");
   fprintf(fp,"\n        add     a,b");

   fprintf(fp,"\n\n; Add Rx frequency step to 6620");
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_0,a2",decRx.Str()+1);
   fprintf(fp,"\n        sub     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_1,a1",decRx.Str()+1);
   fprintf(fp,"\n        lsl     #16,a");
   fprintf(fp,"\n        sub     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_2,a1",decRx.Str()+1);
   fprintf(fp,"\n        lsl     #8,a");
   fprintf(fp,"\n        sub     a,b");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        move    x:FX%s_3,a1",decRx.Str()+1);
   fprintf(fp,"\n        sub     a,b");

   fprintf(fp,"\n\n; Write over base frequency");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$0000ff,a");
   fprintf(fp,"\n        move    a1,x:RXF03");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$00ff00,a");
   fprintf(fp,"\n        lsr     #8,a");
   fprintf(fp,"\n        move    a1,x:RXF02");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        and     #$ff0000,a");
   fprintf(fp,"\n        lsr     #16,a");
   fprintf(fp,"\n        move    a1,x:RXF01");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        add     b,a");
   fprintf(fp,"\n        move    a2,x:RXF00");

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Update Rx frequencies");
   fprintf(fp,"\n        move    #$03,a1");
   fprintf(fp,"\n        move    a1,x:$200C7");
   fprintf(fp,"\n        move    #$03,a1");
   fprintf(fp,"\n        move    a1,x:$200C6");
   fprintf(fp,"\n        move    x:RXF00,a1");
   fprintf(fp,"\n        move    a1,x:$200C3");
   fprintf(fp,"\n        move    x:RXF01,a1");
   fprintf(fp,"\n        move    a1,x:$200C2");
   fprintf(fp,"\n        move    x:RXF02,a1");
   fprintf(fp,"\n        move    a1,x:$200C1");
   fprintf(fp,"\n        move    x:RXF03,a1");
   fprintf(fp,"\n        move    a1,x:$200C0");

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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(gain.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Rx gain");

   fprintf(fp,"\n        movep   #$2C,x:A_PCRC           ; Set up SSI 0"); 
   fprintf(fp,"\n        movep   #$100803,x:A_CRA0       ; /2 clk, 16 bit word transferred"); 
   fprintf(fp,"\n        movep   #$13C3C,x:A_CRB0        ; Enable SSI port with sc1/2 are outputs"); 

   fprintf(fp,"\n        move    x:GAIN%s_0,a1",gain.Str()+1); 
   fprintf(fp,"\n        movep   #$0,x:A_PDRE            ; Select first gain block");
   fprintf(fp,"\n        move    a1,x:A_TX00");
   fprintf(fp,"\n        move    #2,r7                  ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n        move    x:GAIN%s_1,a1",gain.Str()+1); 
   fprintf(fp,"\n        movep   #$4,x:A_PDRE            ; Select second gain block");
   fprintf(fp,"\n        move    a1,x:A_TX00");
   fprintf(fp,"\n        move    #2,r7                  ; Wait 2 us");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n        movep   #$0007,x:A_PDRE         ; Select unused serial port"); 
   fprintf(fp,"\n        movep   #$3c3c,x:A_CRB0         ; Disable SSI 0"); 
   fprintf(fp,"\n        movep   #$24,x:A_PCRC"); 

   fclose(fp);

   return(OK);
}


/**********************************************************************
            Modify the receiver frequency (4/11/07)
**********************************************************************/

short SetRxFreq(DLLParameters* par, char *args)
{
   short nrArgs;
   CText freq;

   
   if((nrArgs = ArgScan(par->itfc,args,1,"freq name","e","q",&freq)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(freq.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Rx frequency");


   fprintf(fp,"\n        move    #$03,a1");                  // Will modify Rx frequency: 6620 address 0x303
   fprintf(fp,"\n        move    a1,x:$200C7");              // C => 6620 Chip select 7=>upper address
   fprintf(fp,"\n        move    #$03,a1");
   fprintf(fp,"\n        move    a1,x:$200C6");              // C => 6620 Chip select 6=>lower address

   fprintf(fp,"\n        move    x:FX%s_0,a1",freq.Str()+1); // Copy the new frequency to
   fprintf(fp,"\n        move    a1,x:RXF00");               // the main DSP Rx frequency location
   fprintf(fp,"\n        move    a1,x:$200C3");              // and then to the 6620

   fprintf(fp,"\n        move    x:FX%s_1,a1",freq.Str()+1); // Repeat for byte 1
   fprintf(fp,"\n        move    a1,x:RXF01");    
   fprintf(fp,"\n        move    a1,x:$200C2");

   fprintf(fp,"\n        move    x:FX%s_2,a1",freq.Str()+1); // Repeat for byte 2
   fprintf(fp,"\n        move    a1,x:RXF02");  
   fprintf(fp,"\n        move    a1,x:$200C1");

   fprintf(fp,"\n        move    x:FX%s_3,a1",freq.Str()+1);  // Repeat for byte 3
   fprintf(fp,"\n        move    a1,x:RXF03");  
   fprintf(fp,"\n        move    a1,x:$200C0");               // Update 6620 frequency now

   fclose(fp);

   return(OK);
}


/**********************************************************************
            Modify the transceiver transmitter frequency (4/11/07)
**********************************************************************/

short SetTxFreq(DLLParameters* par, char *args)
{
   short nrArgs;
   CText freq;

   if((nrArgs = ArgScan(par->itfc,args,1,"freq name","e","q",&freq)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(freq.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Tx frequency");
   fprintf(fp,"\n;"); 

   fprintf(fp,"\n        move    x:FX%s_0,a1",freq.Str()+1); // Copy the new frequency to
   fprintf(fp,"\n        move    a1,x:TXF00");               // the main Tx frequency location
   fprintf(fp,"\n        move    a1,x:$20084");              // and then copy to the transceiver
   fprintf(fp,"\n        move    x:FX%s_1,a1",freq.Str()+1); // Tx buffer
   fprintf(fp,"\n        move    a1,x:TXF01");
   fprintf(fp,"\n        move    a1,x:$20085");
   fprintf(fp,"\n        move    x:FX%s_2,a1",freq.Str()+1);
   fprintf(fp,"\n        move    a1,x:TXF02");
   fprintf(fp,"\n        move    a1,x:$20086");
   fprintf(fp,"\n        move    x:FX%s_3,a1",freq.Str()+1);
   fprintf(fp,"\n        move    a1,x:TXF03");
   fprintf(fp,"\n        move    a1,x:$20087");              // Now update the Tx output 


   fprintf(fp,"\n        move    #$52,a1              ; Turn on 9852 fq_ud");
   fprintf(fp,"\n        move    y:TTL,x1             ; Load the current TTL level");
   fprintf(fp,"\n        or      x1,a1                ; Combine with TTL output");
   fprintf(fp,"\n        move    y:RF,x1              ; Load the current RF level");
   fprintf(fp,"\n        or      x1,a1                ; Combine with TTL output");
   fprintf(fp,"\n        move    a1,x:$20000"); 

   fprintf(fp,"\n        move    #$00,a1              ; Turn off 9852 fq_ud");
   fprintf(fp,"\n        move    y:TTL,x1             ; Load the current RF level");
   fprintf(fp,"\n        or      x1,a1                ; Combine with RF output");
   fprintf(fp,"\n        move    y:RF,x1              ; Load the current RF level");
   fprintf(fp,"\n        or      x1,a1                ; Combine with RF output");
   fprintf(fp,"\n        move    a1,x:$20000");

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
   short nrArgs;
   CText mode,freq,amp,phase,duration;
   long Duration;
   long Amplitude;
   long Phase;
   float fDuration;
   
   if((nrArgs = ArgScan(par->itfc,args,4,"mode,amp,phase,duration,","eeee","qqqq",&mode,&amp,&phase,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   if(amp[0] == 'a')
      InsertUniqueStringIntoList(amp.Str(),&parList,szList);
   if(phase[0] == 'p')
      InsertUniqueStringIntoList(phase.Str(),&parList,szList);
   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

	if(mode == "i") // Internal Kea pulse
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate a pulse");
		fprintf(fp,"\n;"); 

      fprintf(fp,"\n; Check for invalid pulse length");
      fprintf(fp,"\n        clr    a");

      if(duration[0] == 'd')
      {
		   fprintf(fp,"\n        move   x:DELAY%s,a1",duration.Str()+1);
      }
      else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
      {
         if(fDuration < 0.25 || fDuration > 327670)
         {
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
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

      fprintf(fp,"\n        move   #1,b1                     ; Abort code");
      fprintf(fp,"\n        cmp    #49999,a                  ; Pulse must be < 1 ms");
      fprintf(fp,"\n        jgt    ABORT");
      fprintf(fp,"\n        move   #2,b1                     ; Abort code");
      fprintf(fp,"\n        cmp    #24,a                     ; Pulse must be >= 500 ns");
      fprintf(fp,"\n        jlt    ABORT");

      fprintf(fp,"\n; Unblank the RF amp");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");
		fprintf(fp,"\n        move    #$04012,a1              ; Unblank RF amp");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    x:PGO,r3                ; All delays add to pgo before pulse comes on");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
		fprintf(fp,"\n        nop");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      
		fprintf(fp,"\n; Set amplitude");
      if(amp[0] == 'a')
      {
		   fprintf(fp,"\n        move    x:TXA%s_0,a1",amp.Str()+1);
         fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		   fprintf(fp,"\n        move    x:TXA%s_1,a1",amp.Str()+1);
		   fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");
      }
      else if(sscanf(amp.Str(),"%ld",&Amplitude) == 1)
      {
         if(Amplitude < 0 || Amplitude > 65536)
         {
            ErrorMessage("invalid amplitude '%ld' [1...65536]",Duration);
            fclose(fp);
            return(ERR);
         }
         long amp1 = (long)(Amplitude/256.0);
         long amp2 = Amplitude - 256*amp1;
         fprintf(fp,"\n        move    #%ld,a1",amp1);
         fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		   fprintf(fp,"\n        move    #%ld,a1",amp2);
		   fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");
      }
      else
      {
         ErrorMessage("Invalid amplitude reference '%s'",amp.Str());
         fclose(fp);
         return(ERR);
      }

      fprintf(fp,"\n; Set phase");
      if(phase[0] == 'p')
      {
		   fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         if(Phase < 0 || Phase > 3)
         {
            ErrorMessage("invalid phase '%ld' [0,1,2,3]",Phase);
            fclose(fp);
            return(ERR);
         }
         Phase = Phase*0x10;
         fprintf(fp,"\n        move    #%ld,a1",Phase);
      }
      else
      {
         ErrorMessage("Invalid phase reference '%s'",phase.Str());
         fclose(fp);
         return(ERR);
      }

		fprintf(fp,"\n        move    a1,x:$20080");
		fprintf(fp,"\n        move    #$00,a1");
		fprintf(fp,"\n        move    a1,x:$20081");
		fprintf(fp,"\n        move    #$04052,a1              ; Update phase by toggling 9852 synch");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");

		fprintf(fp,"\n        nop");
		fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");

		fprintf(fp,"\n; Start pulse");
      if(duration[0] == 'd')
	    	fprintf(fp,"\n        move    x:DELAY%s,r3",duration.Str()+1);
      else
         fprintf(fp,"\n        move   #%ld,r3",Duration);

		fprintf(fp,"\n        movep   r3,x:A_TCPR2");
		fprintf(fp,"\n        move    #$0401A,a1              ; Start pulse");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Start timer"); 
		fprintf(fp,"\n        nop");
    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");

		fprintf(fp,"\n; End pulse");
		fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
	   fprintf(fp,"\n        move    #$12,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");

		fclose(fp);
	}
	else if(mode == "e") // External Kea pulse
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate a pulse");
		fprintf(fp,"\n;"); 

      fprintf(fp,"\n; Check for invalid pulse length");
      fprintf(fp,"\n        clr    a");
      if(duration[0] == 'd')
      {
		   fprintf(fp,"\n        move   x:DELAY%s,a1",duration.Str()+1);
      }
      else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
      {
         if(fDuration < 0.25 || fDuration > 327670)
         {
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
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
      fprintf(fp,"\n        move   #1,b1                     ; Abort code");
      fprintf(fp,"\n        cmp    #49999,a                  ; Pulse must be < 1 ms");
      fprintf(fp,"\n        jgt    ABORT");
      fprintf(fp,"\n        move   #2,b1                     ; Abort code");
      fprintf(fp,"\n        cmp    #24,a                     ; Pulse must be >= 500 ns");
      fprintf(fp,"\n        jlt    ABORT");

      fprintf(fp,"\n; Unblank the RF amp");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");
		fprintf(fp,"\n        move    #$10012,a1              ; Unblank RF amp");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        move    x:PGO,r3                ; All delays add to pgo before pulse comes on");
		fprintf(fp,"\n        movep   r3,x:A_TCPR2");
		fprintf(fp,"\n        nop");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

		fprintf(fp,"\n; Set amplitude");
      if(amp[0] == 'a')
      {
		   fprintf(fp,"\n        move    x:TXA%s_0,a1",amp.Str()+1);
         fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		   fprintf(fp,"\n        move    x:TXA%s_1,a1",amp.Str()+1);
		   fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");
      }
      else if(sscanf(amp.Str(),"%ld",&Amplitude) == 1)
      {
         if(Amplitude < 0 || Amplitude > 65536)
         {
            ErrorMessage("invalid amplitude '%ld' [1...65536]",Duration);
            fclose(fp);
            return(ERR);
         }
         long amp1 = (long)(Amplitude/256.0);
         long amp2 = Amplitude - 256*amp1;
         fprintf(fp,"\n        move    #%ld,a1",amp1);
         fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		   fprintf(fp,"\n        move    #%ld,a1",amp2);
		   fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");
      }
      else
      {
         ErrorMessage("Invalid amplitude reference '%s'",amp.Str());
         fclose(fp);
         return(ERR);
      }

		fprintf(fp,"\n; Set phase"); // Phase needs to be set before amplitude to allow time for it to settle
      if(phase[0] == 'p')
      {
		   fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         if(Phase < 0 || Phase > 3)
         {
            ErrorMessage("invalid phase '%ld' [0,1,2,3]",Phase);
            fclose(fp);
            return(ERR);
         }
         Phase = Phase*0x10;
         fprintf(fp,"\n        move    #%ld,a1",Phase);
      }
      else
      {
         ErrorMessage("Invalid phase reference '%s'",phase.Str());
         fclose(fp);
         return(ERR);
      }
		fprintf(fp,"\n        move    a1,x:$20080");
		fprintf(fp,"\n        move    #$00,a1");
		fprintf(fp,"\n        move    a1,x:$20081");
		fprintf(fp,"\n        move    #$10052,a1              ; Update phase by toggling 9852 synch");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");

		fprintf(fp,"\n        nop");
		fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");

		fprintf(fp,"\n; Start pulse");
      if(duration[0] == 'd')
	    	fprintf(fp,"\n        move    x:DELAY%s,r3",duration.Str()+1);
      else
         fprintf(fp,"\n        move   #%ld,r3",Duration);

		fprintf(fp,"\n        movep   r3,x:A_TCPR2");
		fprintf(fp,"\n        move    #$1001A,a1              ; Start pulse");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Start timer");
		fprintf(fp,"\n        nop");
		fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");

		fprintf(fp,"\n; End pulse");
		fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
		fprintf(fp,"\n        move    #$12,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");

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
   extern bool SelectGradient(FILE* fp, CText grad);
   CArg carg;

   int n = carg.Count(args);

   if(n == 4)
   {
      CText grad,atable,size,duration;

      if((nrArgs = ArgScan(par->itfc,args,4,"grad, atable, table_size, table_step","eeee","qqqq",&grad,&atable,&size,&duration)) < 0)
         return(nrArgs);

      if(!parList)
      {
         ErrorMessage("Pulse sequence not initialised");
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
         ErrorMessage("Can't open output file");
         return(ERR);
      }

      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Make a shaped gradient pulse");

      if(SelectGradient(fp,grad))
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
            ErrorMessage("invalid table size '%s' [>= 1]",grad);
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
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
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
         ErrorMessage("Pulse sequence not initialised");
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
         ErrorMessage("Can't open output file");
         return(ERR);
      }

      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Make 2 shaped gradient pulses");

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
            ErrorMessage("invalid table size '%s' [>= 1]",grad2);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      if(SelectGradient(fp,grad1)) return(ERR);
      fprintf(fp,"\n        move    y:(r4)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

      fprintf(fp,"\n        rep     #185                    ; Wait 2us");
      fprintf(fp,"\n        nop");

      if(SelectGradient(fp,grad2)) return(ERR);
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
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
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

      //SelectGradient(fp,grad1);
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      //SelectGradient(fp,grad2);
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
         ErrorMessage("Pulse sequence not initialised");
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
         ErrorMessage("Can't open output file");
         return(ERR);
      }

      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Make  3 shaped gradient pulses");

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
            ErrorMessage("invalid table size '%s' [>= 1]",grad2);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      if(SelectGradient(fp,grad1)) return(ERR);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(r4)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

      fprintf(fp,"\n        rep     #185                    ; Wait 2us");
      fprintf(fp,"\n        nop");

      if(SelectGradient(fp,grad2)) return(ERR);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10             ; Set up gradient level");

      fprintf(fp,"\n        rep     #185                    ; Wait 2us");
      fprintf(fp,"\n        nop");

      if(SelectGradient(fp,grad3)) return(ERR);
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
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
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

      //SelectGradient(fp,grad1);
      //fprintf(fp,"\n        movep   a1,x:A_PDRE");
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      //SelectGradient(fp,grad2);
      //fprintf(fp,"\n        movep   a1,x:A_PDRE");
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      //SelectGradient(fp,grad3);
      //fprintf(fp,"\n        movep   a1,x:A_PDRE");
      //fprintf(fp,"\n        move    #$00,a1"); 
      //fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
      //fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      //fprintf(fp,"\n        bsr     wait");

      fclose(fp);
   }
   else
   {
      ErrorMessage("4, 6 or 8 parameters expected");
      return(ERR);
   }

   return(OK);
}

bool SelectGradient(FILE* fp, CText grad)
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
         ErrorMessage("invalid gradient index '%s' [0,1,2,3]",grad);
         fclose(fp);
         return(true);
      }
      fprintf(fp,"\n        move    #%ld,a1",Grad);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }

   return(false);
}

/**********************************************************************
     Generate a shaped rf pulse (4/11/07)

     shapedrf(mode, amplitude, amplitudeTable, phaseTable, tableSize, tableStep)

     mode ............. internal or external RF pulse
     amplitudeTable ... table of amplitudes (0 -> 2^12-1)
     phaseTable ....... table of phases (0 -> 2^14-1)
     tableSize ........ number of value in the table
     tableStep ........ duration of each table value (in us)

     There are two operating modes:

     internal ... gate pulse is sent to the internal Kea RF amplifier
     external ... gate pulse is sent to the external TTL port (pin 5)

**********************************************************************/

short ShapedRF2(DLLParameters* par, char *args)
{
   short nrArgs;
   CText mode,dur,atable,ptable,size,duration;
   long Duration,Size;
   float fDuration;
   
   if((nrArgs = ArgScan(par->itfc,args,5,"mode, atable, ptable, table_size, table_step","eeeee","qqqqq",&mode,&atable,&ptable,&size,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(atable.Str(),&parList,szList);
   InsertUniqueStringIntoList(ptable.Str(),&parList,szList);
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

	if(mode == "i") // Internal Kea pulse
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate a modulated pulse");
		fprintf(fp,"\n;"); 

      fprintf(fp,"\n        movep   #$0F3FE1,x:A_BCR        ; Set up wait states, 1 for AA3");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");

      fprintf(fp,"\n; Adjust step length");
      if(duration[0] == 'd')
      {
		   fprintf(fp,"\n        move   x:DELAY%s,a1",duration.Str()+1);
      }
      else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
      {
         if(fDuration < 0.25 || fDuration > 327670)
         {
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
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

      fprintf(fp,"\n        lsl     #1,a");
      fprintf(fp,"\n        sub     #46,a");
      fprintf(fp,"\n        move    a1,a0");

      fprintf(fp,"\n; Zero the rf output and unblank the RF amp");
      fprintf(fp,"\n        move    #0,a1                   ; Zero amplitude"); 
		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        move    #$04052,a1              ; Transfer new data to DDS");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        move    #$04012,a1");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); // Was 41
      fprintf(fp,"\n        sub     #25,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n        move    #$0401a,a1              ; switch on rf");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000"); 
		fprintf(fp,"\n        move    a,b"); 

// Get the table addresses
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
      fprintf(fp,"\n        move    x:TABLE%s,r4",ptable.Str()+1);

 // Get the size of the tables
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            ErrorMessage("invalid table size '%s' [>= 1]",size);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      fprintf(fp,"\n        move    y:(r4),a1               ; Load phase"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");  
		fprintf(fp,"\n        move    a1,x:$20080             ; Byte 1");
      fprintf(fp,"\n        move    y:(r4)+,a1              ; Load phase"); 
      fprintf(fp,"\n        and     #$0000FF,a              ; Get at lower byte");
		fprintf(fp,"\n        move    a1,x:$20081             ; Byte 2");
 
      fprintf(fp,"\n        nop");

      fprintf(fp,"\n        move    y:(r5),a1               ; Load amplitude"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");   

      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load amplitude"); 
      fprintf(fp,"\n        and     #$0000FF,a              ; Get at lower byte");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

      fprintf(fp,"\n; Adjust for correct step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fprintf(fp,"\n; End Delay (correct for last pulse)");
      fprintf(fp,"\n        rep     #40                    ; Wait 400 ns");
      fprintf(fp,"\n        nop");

		fprintf(fp,"\n; End pulse");
		fprintf(fp,"\n        move    #$12,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        movep   #$0FFFE1,x:A_BCR        ; Set up wait states, 7 for AA3");

		fclose(fp);
	}
	else if(mode == "e") // External Kea pulse
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate a modulated pulse");
		fprintf(fp,"\n;"); 

      fprintf(fp,"\n        movep   #$0F3FE1,x:A_BCR        ; Set up wait states, 1 for AA3");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");

      fprintf(fp,"\n; Adjust step length");
      if(duration[0] == 'd')
      {
		   fprintf(fp,"\n        move   x:DELAY%s,a1",duration.Str()+1);
      }
      else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
      {
         if(fDuration < 0.25 || fDuration > 327670)
         {
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
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
      fprintf(fp,"\n        lsl     #1,a");
      fprintf(fp,"\n        sub     #46,a");
      fprintf(fp,"\n        move    a1,a0");

      fprintf(fp,"\n; Zero the rf output and unblank the RF amp");
      fprintf(fp,"\n        move    #0,a1                   ; Zero amplitude"); 
		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        move    #$10052,a1              ; Transfer new data to DDS");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        move    #$10012,a1");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); // Was 41
      fprintf(fp,"\n        sub     #25,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n        move    #$1001a,a1              ; switch on rf");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000"); 
		fprintf(fp,"\n        move    a,b"); 

// Get the table addresses
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
      fprintf(fp,"\n        move    x:TABLE%s,r4",ptable.Str()+1);

 // Get the size of the tables
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            ErrorMessage("invalid table size '%s' [>= 1]",size);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      fprintf(fp,"\n        move    y:(r4),a1               ; Load phase"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");  
		fprintf(fp,"\n        move    a1,x:$20080             ; Byte 1");
      fprintf(fp,"\n        move    y:(r4)+,a1              ; Load phase"); 
      fprintf(fp,"\n        and     #$0000FF,a              ; Get at lower byte");
		fprintf(fp,"\n        move    a1,x:$20081             ; Byte 2");
 
      fprintf(fp,"\n        nop");

      fprintf(fp,"\n        move    y:(r5),a1               ; Load amplitude"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");   

      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load amplitude"); 
      fprintf(fp,"\n        and     #$0000FF,a              ; Get at lower byte");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

      fprintf(fp,"\n; Adjust for correct step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fprintf(fp,"\n; End Delay (correct for last pulse)");
      fprintf(fp,"\n        rep     #40                    ; Wait 400 ns");
      fprintf(fp,"\n        nop");

		fprintf(fp,"\n; End pulse");
		fprintf(fp,"\n        move    #$12,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        movep   #$0FFFE1,x:A_BCR        ; Set up wait states, 7 for AA3");

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
     Generate a shaped rf pulse (4/11/07)

     shapedrf(mode, amplitudeTable, tableSize, tableStep)

     shapedrf(mode, amplitudeTable, signTable, phase, tableSize, tableStep)

     shapedrf(i/e, a?/constant, t?, t?, n?/constant, d?/constant

     mode ............. internal or external RF pulse
     amplitudeTable ... table of amplitudes (0 -> 2^12-1)
     signTable ........ table of signs (0 -> 1)
     phase ............ phase step value 0,16,32,48 (x,-x,y,-y)
     tableSize ........ number of value in the table
     tableStep ........ duration of each table value (in us)

     There are two operating modes:

     internal ... gate pulse is sent to the internal Kea RF amplifier
     external ... gate pulse is sent to the external TTL port (pin 5)

**********************************************************************/

short ShapedRF(DLLParameters* par, char *args)
{
   short nrArgs;
   CText mode,dur,atable,stable,phase,size,duration;
   long Duration,Phase,Size;
   float fDuration;
   CArg carg;

   int n = carg.Count(args);

   if(n == 5)
   {
      return(ShapedRF2(par,args));
   }


   if((nrArgs = ArgScan(par->itfc,args,6,"mode, atable, stable, phase, table_size, table_step","eeeeee","qqqqqq",&mode,&atable,&stable,&phase,&size,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

// Set up parameter list
   InsertUniqueStringIntoList(atable.Str(),&parList,szList);
   InsertUniqueStringIntoList(stable.Str(),&parList,szList);
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

	if(mode == "i") // Internal Kea pulse
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate a modulated RF pulse");
		fprintf(fp,"\n;"); 

      fprintf(fp,"\n        clr a                           ; Clear the accumulator");

      fprintf(fp,"\n        movep   #$0F3FE1,x:A_BCR        ; Set up wait states, 1 for AA3");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");

// Delay for each table value
      fprintf(fp,"\n; Step length");
      if(duration[0] == 'd')
      {
         fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
      }
      else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
      {
         if(fDuration < 0.25 || fDuration > 327670)
         {
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
            fclose(fp);
            return(ERR);
         }
         Duration = (long)(fDuration * 50 - 1 + 0.5);
         fprintf(fp,"\n        move    #%ld,a1",Duration);
      }

      fprintf(fp,"\n        lsl     #1,a");
      fprintf(fp,"\n        sub     #46,a");
      fprintf(fp,"\n        move    a1,a0");

      fprintf(fp,"\n; Zero the rf output and unblank the RF amp");
      fprintf(fp,"\n        move    #0,a1                   ; Zero amplitude"); 
		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        move    #$04052,a1              ; Transfer new data to DDS");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        move    #$04012,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");


      fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); // Was 41
      fprintf(fp,"\n        sub     #25,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n        move    #$0401a,a1              ; switch on rf");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000"); 
		fprintf(fp,"\n        move    a,b"); 

// Get the table addresses
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
      fprintf(fp,"\n        move    x:TABLE%s,r4",stable.Str()+1);

// Get the size of the tables
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            ErrorMessage("invalid table size '%s' [>= 1]",size);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld                 ; Step the amplitude r2 times",label);
      fprintf(fp,"\n        move    y:(r4),a1               ; Load sign"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");  

// Get phase
      if(phase[0] == 'p')
      {
          fprintf(fp,"\n        move    x:TXP%s,y1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         fprintf(fp,"\n        move    #%ld,y1",Phase);
      }
		fprintf(fp,"\n        add     y1,a                    ; Add phase to table");
		fprintf(fp,"\n        move    a1,x:$20080             ; Byte 1");
      fprintf(fp,"\n        move    y:(r4)+,a1              ; Load sign"); 
		fprintf(fp,"\n        move    a1,x:$20081             ; Byte 2");
 
      fprintf(fp,"\n        nop");

      fprintf(fp,"\n        move    y:(r5),a1               ; Load amplitude"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");   

      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load amplitude"); 
      fprintf(fp,"\n        and     #$0000FF,a              ; Get at lower byte");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

      fprintf(fp,"\n; Adjust for correct step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\nLBL%ld    nop",label++);

      fprintf(fp,"\n; End Delay (correct for last pulse)");
      fprintf(fp,"\n        rep     #40                     ; Wait 400 ns");
      fprintf(fp,"\n        nop");

		fprintf(fp,"\n; End pulse");
		fprintf(fp,"\n        move    #$12,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        movep   #$0FFFE1,x:A_BCR        ; Set up wait states, 7 for AA3");

		fclose(fp);
	}
	else if(mode == "e") // External Kea pulse
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate a modulated RF pulse");
		fprintf(fp,"\n;"); 

      fprintf(fp,"\n        clr a                           ; Clear the accumulator");

      fprintf(fp,"\n        movep   #$0F3FE1,x:A_BCR        ; Set up wait states, 1 for AA3");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");

// Delay for each table value
      fprintf(fp,"\n; Step length");
      if(duration[0] == 'd')
      {
         fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
      }
      else if(sscanf(duration.Str(),"%f",&fDuration) == 1)
      {
         if(fDuration < 0.5 || fDuration > 327670)
         {
            ErrorMessage("invalid delay '%ld' [0.5...327670]",Duration);
            fclose(fp);
            return(ERR);
         }
         Duration = (long)(fDuration * 50 - 1 + 0.5);
         fprintf(fp,"\n        move    #%ld,a1",Duration);
      }

      fprintf(fp,"\n        lsl     #1,a");
      fprintf(fp,"\n        sub     #46,a");
      fprintf(fp,"\n        move    a1,a0");

      fprintf(fp,"\n; Zero the rf output and unblank the RF amp");
      fprintf(fp,"\n        move    #0,a1                   ; Zero amplitude"); 
		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        move    #$10052,a1              ; Transfer new data to DDS");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        move    #$10012,a1");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); // Was 41
      fprintf(fp,"\n        sub     #25,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n        move    #$1001a,a1              ; switch on rf");
		fprintf(fp,"\n        or      x1,a1                   ; combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000"); 
		fprintf(fp,"\n        move    a,b"); 

// Get the table addresses
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
      fprintf(fp,"\n        move    x:TABLE%s,r4",stable.Str()+1);

      
// Get the size of the tables
// Extract the table size
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            ErrorMessage("invalid table size '%s' [>= 1]",size);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }

// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);
      fprintf(fp,"\n        move    y:(r4),a1               ; Load sign"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");  

// Get phase
      if(phase[0] == 'p')
      {
          fprintf(fp,"\n        move    x:TXP%s,y1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         fprintf(fp,"\n        move    #%ld,y1",Phase);
      }

		fprintf(fp,"\n        add     y1,a                    ; Add phase to table");
		fprintf(fp,"\n        move    a1,x:$20080             ; Byte 1");
      fprintf(fp,"\n        move    y:(r4)+,a1              ; Load sign"); 
		fprintf(fp,"\n        move    a1,x:$20081             ; Byte 2");
 
 
      fprintf(fp,"\n        nop");

      fprintf(fp,"\n        move    y:(r5),a1               ; Load amplitude"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");   

      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load amplitude"); 
      fprintf(fp,"\n        and     #$0000FF,a              ; Get at lower byte");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

      fprintf(fp,"\n; Adjust for correct step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fprintf(fp,"\n; End Delay (correct for last pulse)");
      fprintf(fp,"\n        rep     #40                    ; Wait 400 ns");
      fprintf(fp,"\n        nop");

		fprintf(fp,"\n; End pulse");
		fprintf(fp,"\n        move    #$12,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        movep   #$0FFFE1,x:A_BCR        ; Set up wait states, 7 for AA3");

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


short ChirpedRF(DLLParameters* par, char *args)
{
   short nrArgs;
   CText mode,dur,atable,ftable,phase,size,duration;
   long Duration,Phase,Size;
   float fDuration;
   CArg carg;

   int n = carg.Count(args);


   if((nrArgs = ArgScan(par->itfc,args,6,"mode, atable, ftable, phase, table_size, table_step","eeeeee","qqqqqq",&mode,&atable,&ftable,&phase,&size,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
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

	if(mode == "i") // Internal Kea pulse
	{
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate an amplitude and frequency modulated internal RF pulse");
		fprintf(fp,"\n;"); 

      fprintf(fp,"\n        clr a                           ; Clear the accumulator");

      fprintf(fp,"\n        movep   #$0F3FE1,x:A_BCR        ; Set up wait states, 1 for AA3");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");

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
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
            fclose(fp);
            return(ERR);
         }
         Duration = (long)(fDuration * 50 - 1 + 0.5);
         fprintf(fp,"\n        move    #%ld,a1",Duration);
      }

      fprintf(fp,"\n        lsl     #1,a");
      fprintf(fp,"\n        sub     #46,a");
      fprintf(fp,"\n        move    a1,a0");
// Zero the rf output and unblank the RF amp
      fprintf(fp,"\n; Zero the rf output and unblank the RF amp");
      fprintf(fp,"\n        move    #0,a1                   ; Zero amplitude"); 
		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        move    #$04052,a1              ; Transfer new data to DDS");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        move    #$04012,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");

// Wait for pgo delay
      fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); // Was 41
      fprintf(fp,"\n        sub     #25,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n        move    #$0401a,a1              ; switch on rf");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000"); 
		fprintf(fp,"\n        move    a,b"); 

// Get the table addresses
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
      fprintf(fp,"\n        move    x:TABLE%s,r4",ftable.Str()+1);

// Get the size of the tables
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            ErrorMessage("invalid table size '%s' [>= 1]",size);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }
 
// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);
 
    // Get phase
      if(phase[0] == 'p')
      {
          fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         fprintf(fp,"\n        move    #%ld,a1",Phase);
      }

    // Update the frequency from table
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:$20084");
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:$20085");
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:$20086");
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:$20087");
      fprintf(fp,"\n        nop");

    // Update the phase from parameter
		fprintf(fp,"\n        move    a1,x:$20080");
		fprintf(fp,"\n        move    #$00,a1");
		fprintf(fp,"\n        move    a1,x:$20081");

   // Update the DDS
      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

    // Update the amplitude from table
      fprintf(fp,"\n        move    y:(r5),a1               ; Load amplitude"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");   
		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load amplitude"); 
      fprintf(fp,"\n        and     #$0000FF,a              ; Get at lower byte");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

   // Update the DDS
      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

      fprintf(fp,"\n; Adjust for correct step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\nLBL%ld    nop",label++);

      fprintf(fp,"\n; End Delay (correct for last pulse)");
      fprintf(fp,"\n        rep     #40                     ; Wait 400 ns");
      fprintf(fp,"\n        nop");

		fprintf(fp,"\n; End pulse");
		fprintf(fp,"\n        move    #$12,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        movep   #$0FFFE1,x:A_BCR        ; Set up wait states, 7 for AA3");

		fclose(fp);
	}
   else if(mode == "e") 
   {
		fprintf(fp,"\n\n;");
		fprintf(fp,"\n;***************************************************************************");
		fprintf(fp,"\n; Generate an amplitude and frequency modulated external RF pulse");
		fprintf(fp,"\n;"); 

      fprintf(fp,"\n        clr a                           ; Clear the accumulator");

      fprintf(fp,"\n        movep   #$0F3FE1,x:A_BCR        ; Set up wait states, 1 for AA3");
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");

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
            ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
            fclose(fp);
            return(ERR);
         }
         Duration = (long)(fDuration * 50 - 1 + 0.5);
         fprintf(fp,"\n        move    #%ld,a1",Duration);
      }

      fprintf(fp,"\n        lsl     #1,a");
      fprintf(fp,"\n        sub     #46,a");
      fprintf(fp,"\n        move    a1,a0");
// Zero the rf output and unblank the RF amp
      fprintf(fp,"\n; Zero the rf output and unblank the RF amp");
      fprintf(fp,"\n        move    #0,a1                   ; Zero amplitude"); 
		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n        move    #$10052,a1              ; Transfer new data to DDS");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        move    #$10012,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000");

// Wait for pgo delay
      fprintf(fp,"\n        move    x:PGO,a1                ; All delays add to 1us before pulse comes on"); // Was 41
      fprintf(fp,"\n        sub     #25,a");
      fprintf(fp,"\n        movep   a1,x:A_TCPR2");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
    	fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n        move    #$1001a,a1              ; switch on rf");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with ttl output");
		fprintf(fp,"\n        move    a1,x:$20000"); 
		fprintf(fp,"\n        move    a,b"); 

// Get the table addresses
      fprintf(fp,"\n        move    x:TABLE%s,r5",atable.Str()+1);
      fprintf(fp,"\n        move    x:TABLE%s,r4",ftable.Str()+1);

// Get the size of the tables
      if(size[0] == 'n')
      {
          fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
      }
      else if(sscanf(size.Str(),"%ld",&Size) == 1)
      {
         if(Size < 1)
         {
            ErrorMessage("invalid table size '%s' [>= 1]",size);
            fclose(fp);
            return(ERR);
         }
         fprintf(fp,"\n        move    #%ld,r2",Size);
      }
 
// Loop over the tables
      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);
 
    // Get phase
      if(phase[0] == 'p')
      {
          fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      }
      else if(sscanf(phase.Str(),"%ld",&Phase) == 1)
      {
         fprintf(fp,"\n        move    #%ld,a1",Phase);
      }

    // Update the frequency from table
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:$20084");
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:$20085");
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:$20086");
      fprintf(fp,"\n        move    y:(r4)+,a1");
      fprintf(fp,"\n        move    a1,x:$20087");
      fprintf(fp,"\n        nop");

    // Update the phase from parameter
		fprintf(fp,"\n        move    a1,x:$20080");
		fprintf(fp,"\n        move    #$00,a1");
		fprintf(fp,"\n        move    a1,x:$20081");

   // Update the DDS
      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");


    // Update the amplitude from table
      fprintf(fp,"\n        move    y:(r5),a1               ; Load amplitude"); 
      fprintf(fp,"\n        lsr     #8,a                    ; get upper byte");   
		fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    y:(r5)+,a1              ; Load amplitude"); 
      fprintf(fp,"\n        and     #$0000FF,a              ; Get at lower byte");
		fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");


   // Update the DDS
      fprintf(fp,"\n        or      #$00040,b               ; Transfer new data to DDS");
		fprintf(fp,"\n        move    b1,x:$20000");
      fprintf(fp,"\n        and     #$ffffbf,b");
		fprintf(fp,"\n        move    b1,x:$20000");

      fprintf(fp,"\n; Adjust for correct step length");
      fprintf(fp,"\n        rep     a0");
      fprintf(fp,"\n        nop");

      fprintf(fp,"\nLBL%ld    nop",label++);

      fprintf(fp,"\n; End Delay (correct for last pulse)");
      fprintf(fp,"\n        rep     #40                     ; Wait 400 ns");
      fprintf(fp,"\n        nop");

		fprintf(fp,"\n; End pulse");
		fprintf(fp,"\n        move    #$12,a1");
		fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
		fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        movep   #$0FFFE1,x:A_BCR        ; Set up wait states, 7 for AA3");

		fclose(fp);
	}
	
   else
   {
		fclose(fp);
      ErrorMessage("unknown RF chirp pulse mode");
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

     Note that there is a 1.10us delay before the rf appears. This allows
     the pulse phase and amplitude to be set and gives time for the 
     HPA biasing to be switched on.

**********************************************************************/

short SwitchOnTx(DLLParameters* par, char *args)
{
   short nrArgs;
   CText mode,freq,amp,phase,duration;

   
   if((nrArgs = ArgScan(par->itfc,args,1,"mode,[amp,phase]","eee","qqq",&mode,&amp,&phase)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   if(nrArgs == 3)
   {
      InsertUniqueStringIntoList(amp.Str(),&parList,szList);
      InsertUniqueStringIntoList(phase.Str(),&parList,szList);
   }
   else if(nrArgs == 2)
   {
      ErrorMessage("1 or 3 arguments expected");
      return(ERR);
   }

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   if(mode == "e")
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch on transmitter and external HPA");
      fprintf(fp,"\n;"); 

      fprintf(fp,"\n        move    x:PGO,r3                ; All delays add to pgo before pulse comes on");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
		fprintf(fp,"\n        nop");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
      fprintf(fp,"\n        move    #$10012,a1              ; Unblank RF amp");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n; Set amplitude");
      fprintf(fp,"\n        move    x:TXA%s_0,a1",amp.Str()+1);
      fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    x:TXA%s_1,a1",amp.Str()+1);
      fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n; Set phase");
      fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      fprintf(fp,"\n        move    a1,x:$20080");
      fprintf(fp,"\n        move    #$00,a1");
      fprintf(fp,"\n        move    a1,x:$20081");
      fprintf(fp,"\n        move    #$10052,a1              ; Update phase by toggling 9852 synch");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n; Start pulse");
      fprintf(fp,"\n        move    #$1001A,a1              ; Start pulse");
      fprintf(fp,"\n        move    a1,y:RF                 ; Save the RF state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        nop");
   }
   else if(mode == "i")
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch on transmitter and internal HPA");
      fprintf(fp,"\n;"); 

      fprintf(fp,"\n        move    x:PGO,r3                ; All delays add to pgo before pulse comes on");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
		fprintf(fp,"\n        nop");
		fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

  //    fprintf(fp,"\n        move    #38,r3                  ; Load delay into timer");
  //    fprintf(fp,"\n        movep   r3,x:A_TCPR2");
		//fprintf(fp,"\n        nop");
  //    fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL level");
      fprintf(fp,"\n        move    #$0C012,a1              ; Unblank RF amp");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
      fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n; Set amplitude");
      fprintf(fp,"\n        move    x:TXA%s_0,a1",amp.Str()+1);
      fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    x:TXA%s_1,a1",amp.Str()+1);
      fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n; Set phase");
      fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      fprintf(fp,"\n        move    a1,x:$20080");
      fprintf(fp,"\n        move    #$00,a1");
      fprintf(fp,"\n        move    a1,x:$20081");
      fprintf(fp,"\n        move    #$0C052,a1              ; Update phase by toggling 9852 synch");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
      fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n; Start pulse");
      fprintf(fp,"\n        move    #$0C01A,a1              ; Start pulse");
      fprintf(fp,"\n        move    a1,y:RF                 ; Save the RF state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL output");
      fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        nop");
   }
   else if(mode == "n") 
   {
      if(nrArgs == 3)
      {
         fprintf(fp,"\n\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Switch on transmitter but not HPA");
         fprintf(fp,"\n;"); 
         fprintf(fp,"\n        move    #38,r3                  ; Load 0.8us into timer");
         fprintf(fp,"\n        movep   r3,x:A_TCPR2");
         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

         fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");

         fprintf(fp,"\n; Set amplitude");
         fprintf(fp,"\n        move    x:TXA%s_0,a1",amp.Str()+1);
         fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
         fprintf(fp,"\n        move    x:TXA%s_1,a1",amp.Str()+1);
         fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

         fprintf(fp,"\n; Set phase");
         fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
         fprintf(fp,"\n        move    a1,x:$20080");
         fprintf(fp,"\n        move    #$00,a1");
         fprintf(fp,"\n        move    a1,x:$20081");
         fprintf(fp,"\n        move    #$00052,a1              ; Update phase by toggling 9852 synch"); // RF off
         fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
         fprintf(fp,"\n        move    a1,x:$20000");

         fprintf(fp,"\n        nop");
         fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
         fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

         fprintf(fp,"\n; Start pulse");
         fprintf(fp,"\n        move    #$0001A,a1              ; Start pulse");
         fprintf(fp,"\n        move    a1,y:RF                 ; Update the RF state");
         fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
         fprintf(fp,"\n        move    a1,x:$20000");
         fprintf(fp,"\n        nop");
      }
      else // Simple pulse on
      {
         fprintf(fp,"\n\n;");
         fprintf(fp,"\n; Switch on transmitter but not HPA");
         fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
         fprintf(fp,"\n        move    #$0001A,a1              ; Start pulse");
         fprintf(fp,"\n        move    a1,y:RF                 ; Update the RF state");
         fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
         fprintf(fp,"\n        move    a1,x:$20000");
         fprintf(fp,"\n        nop");
      }
   }
   else if(mode == "we") // Wobble external
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch on transmitter and external HPA");
      fprintf(fp,"\n;"); 
      fprintf(fp,"\n        move    #38,r3                  ; Load delay into timer");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
      fprintf(fp,"\n        move    #$11012,a1              ; Unblank RF amp");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n; Set amplitude");
      fprintf(fp,"\n        move    x:TXA%s_0,a1",amp.Str()+1);
      fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    x:TXA%s_1,a1",amp.Str()+1);
      fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n; Set phase");
      fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      fprintf(fp,"\n        move    a1,x:$20080");
      fprintf(fp,"\n        move    #$00,a1");
      fprintf(fp,"\n        move    a1,x:$20081");
      fprintf(fp,"\n        move    #$11052,a1              ; Update phase by toggling 9852 synch");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n; Start pulse");
      fprintf(fp,"\n        move    #$1101A,a1              ; Start pulse in wobble mode");
      fprintf(fp,"\n        move    a1,y:RF                 ; Save the RF state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        nop");
   }
   else if(mode == "wi") // Wobble internal
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch on transmitter and internal HPA");
      fprintf(fp,"\n;"); 
      fprintf(fp,"\n        move    #38,r3                  ; Load delay into timer");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");

      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
      fprintf(fp,"\n        move    #$0D012,a1              ; Unblank RF amp");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n; Set amplitude");
      fprintf(fp,"\n        move    x:TXA%s_0,a1",amp.Str()+1);
      fprintf(fp,"\n        move    a1,x:$200A1             ; Byte 1");
      fprintf(fp,"\n        move    x:TXA%s_1,a1",amp.Str()+1);
      fprintf(fp,"\n        move    a1,x:$200A2             ; Byte 2");

      fprintf(fp,"\n; Set phase");
      fprintf(fp,"\n        move    x:TXP%s,a1",phase.Str()+1);
      fprintf(fp,"\n        move    a1,x:$20080");
      fprintf(fp,"\n        move    #$00,a1");
      fprintf(fp,"\n        move    a1,x:$20081");
      fprintf(fp,"\n        move    #$0D052,a1              ; Update phase by toggling 9852 synch");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");

      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for parameters to update");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

      fprintf(fp,"\n; Start pulse");
      fprintf(fp,"\n        move    #$0D01A,a1              ; Start pulse in wobble mode");
      fprintf(fp,"\n        move    a1,y:RF                 ; Save the RF state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000");
      fprintf(fp,"\n        nop");
   }
   else
   {
      ErrorMessage("invalid mode %s",mode);
      return(ERR);
   }

   fclose(fp);

   return(OK);

}

/**********************************************************************
     Switch off the transceiver transmitter output

     txoff()

     Note keeps the TTL state intact. 

     This command takes 150 ns before the RF turns off

**********************************************************************/


short SwitchOffTx(DLLParameters* par, char *args)
{
   short nrArgs;
   CText mode;

   
   if((nrArgs = ArgScan(par->itfc,args,0,"[mode]","e","q",&mode)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   if(nrArgs == 0)
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch off transmitter");
      fprintf(fp,"\n;"); 
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
      fprintf(fp,"\n        move    #$12,a1                 ; Set the RF state");
      fprintf(fp,"\n        move    a1,y:RF                 ; Save the current RF state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000             ; Update transceiver state");

      fclose(fp);
   }
   else if(mode[0] == 'w') // Wobble mode
   {
      fprintf(fp,"\n\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Switch off transmitter");
      fprintf(fp,"\n;"); 
      fprintf(fp,"\n        move    y:TTL,x1                ; Load the current TTL state");
      fprintf(fp,"\n        move    #$01012,a1              ; Set the RF state");
      fprintf(fp,"\n        move    a1,y:RF                 ; Save the current RF state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000             ; Update transceiver state");

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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(byte.Str(),&parList,szList);
   InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Pulse TTL level");
   fprintf(fp,"\n        move    x:TTL%s,a1                 ; Read in the TTL byte",byte.Str()+1);

   fprintf(fp,"\n        move    y:RF,x1                    ; Read the lower 16 bits");
   fprintf(fp,"\n        or      x1,a1                      ; Combine with TTL state");
   fprintf(fp,"\n        move    a1,x:$20000                ; Update TTL output");

   fprintf(fp,"\n; Delay");
   fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
   fprintf(fp,"\n        sub     #21,a"); // Tweek delay
   fprintf(fp,"\n        move    a1,r3");
   fprintf(fp,"\n        movep   r3,x:A_TCPR2");
   fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*            ; Wait for pulse to end");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2         ; Turn off timer");

   fprintf(fp,"\n; Reset TTL level");
   fprintf(fp,"\n        move    y:RF,a1                    ; Read the lower 16 bits");
   fprintf(fp,"\n        move    a1,x:$20000");

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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(table.Str(),&parList,szList);
   if(index[0] == 'n')
       InsertUniqueStringIntoList(index.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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
         ErrorMessage("Invalid index '%s'",index.Str());
         fclose(fp);
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Invalid table reference '%s'",table.Str());
      fclose(fp);
      return(ERR);
   }

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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(table.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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
      ErrorMessage("invalid table reference '%s'",table.Str());
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(table.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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
      ErrorMessage("invalid table reference '%s'",table.Str());
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
      ErrorMessage("Pulse sequence not initialised");
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
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set Gradient level");

// Choose the gradient level
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
      ErrorMessage("Invalid address '%s'",address.Str());
      return(ERR);
   }

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

      fprintf(fp,"\n        add     b,a"); 
      fprintf(fp,"\n        move    a0,r5"); // Update table index
      fprintf(fp,"\n        move    y:(r5),a1"); // Update table index

   // Set the level
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX10            ; Set up gradient level");
      fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
      fprintf(fp,"\n        bsr     wait");
   }
   else
   {
      fclose(fp);
      ErrorMessage("Invalid amplitude '%s'",amplitude.Str());
      return(ERR);
   }

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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

// Add to parameter table is not a constant
   if(address[0] == 'n')
      InsertUniqueStringIntoList(address.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Zero gradient");

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
      ErrorMessage("Invalid address '%s'",address.Str());
      return(ERR);
   }

// Zero the gradient level
   fprintf(fp,"\n        move    #$00,a1"); 
   fprintf(fp,"\n        movep   a1,x:A_TX10            ; Zero the gradient");
   fprintf(fp,"\n        move    #2,r7                  ; Wait 2us");
   fprintf(fp,"\n        bsr     wait");


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
      ErrorMessage("Pulse sequence not initialised");
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
      ErrorMessage("Can't open output file");
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
      fprintf(fp,"\n        move    #3,a1",Address);
   else if(address == "y")
      fprintf(fp,"\n        move    #2,a1",Address);
   else if(address == "z")
      fprintf(fp,"\n        move    #1,a1",Address);
   else if(address == "o")
      fprintf(fp,"\n        move    #0,a1",Address);
   else if(sscanf(address.Str(),"%ld",&Address) == 1)
   {
      if(Address < 0 || Address > 3)
      {
         ErrorMessage("invalid address '%ld' [0,1,2,3]",Address);
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   if(duration[0] == 'd')
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   };


   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Delay");

   if(duration[0] == 'd')
   {
      fprintf(fp,"\n        move    x:DELAY%s,a1",duration.Str()+1);
      fprintf(fp,"\n        sub     #10,a"); // Tweek it
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
         ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp,"\n        move    #%ld,a1",Duration);
      fprintf(fp,"\n        sub     #10,a"); // Tweek it
      fprintf(fp,"\n        move    a1,r3");
      fprintf(fp,"\n        movep   r3,x:A_TCPR2");
      fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
      fprintf(fp,"\n        nop");
      fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
      fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");
   }
   else
   {
      ErrorMessage("invalid delay or delay reference '%s'",duration.Str());
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   if(wait[0] == 'w')
      InsertUniqueStringIntoList(wait.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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
         ErrorMessage("invalid delay '%ld' [2 us...167 s]",wait);
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
         ErrorMessage("invalid delay '%ld' [2 us...167 s]",wait);
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
     Set the TLL levels 

     ttl(byte)

     Takes 150 ns to set up level. The TTL level is combined with the
     lower 16 bits to prevent any disruption to the RF.
**********************************************************************/

short SetTTL(DLLParameters* par, char *args)
{
   short nrArgs;
   CText byte;


   if((nrArgs = ArgScan(par->itfc,args,1,"byte name","e","q",&byte)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(byte.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Set TTL level");

   fprintf(fp,"\n        move    x:TTL%s,a1              ; Read in the TTL byte",byte.Str()+1);
   fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL state");

   fprintf(fp,"\n        move    y:RF,x1                 ; Read the lower 16 bits");
   fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
   fprintf(fp,"\n        move    a1,x:$20000             ; Update TTL output");

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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   if(byte[0] == 'b')
      InsertUniqueStringIntoList(byte.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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

      fprintf(fp,"\n        move    y:RF,x1                 ; Read the lower 16 bits (RF stuff)");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000             ; Update TTL output");
   }
   else if(sscanf(byte.Str(),"%ld",&Byte) == 1)
   {
      if(Byte < 1 || Byte > 0x80)
      {
         ErrorMessage("Invalid TTL value '%ld'",Byte);
         fclose(fp);
         return(ERR);
      }
      Byte = Byte << 16;
      fprintf(fp,"\n        move    #%ld,a1                 ; Read in the new TTL byte",Byte);
      fprintf(fp,"\n        move    y:TTL,x1                ; Read in the current TTL state");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with current TTL state");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save total TTL state");

      fprintf(fp,"\n        move    y:RF,x1                 ; Read the lower 16 bits (RF stuff)");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000             ; Update TTL output");
   }
   else
   {
      ErrorMessage("Invalid TTL reference '%s'",byte.Str());
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   if(byte[0] == 'b')
      InsertUniqueStringIntoList(byte.Str(),&parList,szList);


   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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

      fprintf(fp,"\n        move    y:RF,x1                 ; Read the lower 16 bits (RF stuff)");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000             ; Update TTL output");
   }
   else if(sscanf(byte.Str(),"%ld",&Byte) == 1)
   {
      if(Byte < 1 || Byte > 0x80)
      {
         ErrorMessage("Invalid TTL value '%ld'",Byte);
         fclose(fp);
         return(ERR);
      }
      Byte = Byte << 16;
      fprintf(fp,"\n        move    #%ld,a1                 ; Read in the TTL byte",Byte);
      fprintf(fp,"\n        not     a                       ; Invert the byte");
      fprintf(fp,"\n        move    y:TTL,x1                ; Read the current TTL state");
      fprintf(fp,"\n        and     x1,a1                   ; And with current TTL state to switch off TTL line");
      fprintf(fp,"\n        move    a1,y:TTL                ; Save new TTL state");

      fprintf(fp,"\n        move    y:RF,x1                 ; Read the lower 16 bits (RF stuff)");
      fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
      fprintf(fp,"\n        move    a1,x:$20000             ; Update TTL output");
   }
   else
   {
      ErrorMessage("Invalid TTL reference '%s'",byte.Str());
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   if(nrTimes[0] == 'n')
      InsertUniqueStringIntoList(nrTimes.Str(),&parList,szList);

   InsertUniqueStringIntoList(loopName.Str(),&parList,szList);


   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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
         ErrorMessage("invalid number of loops '%s'",nrTimes.Str());
         fclose(fp);
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("invalid loop name '%s'",loopName.Str());
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   InsertUniqueStringIntoList(loopName.Str(),&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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
      ErrorMessage("invalid loop name '%s'",loopName.Str());
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   if(nrPnts[0] == 'n')
      InsertUniqueStringIntoList(nrPnts.Str(),&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Clear the data memory");
   fprintf(fp,"\n;");
   fprintf(fp,"\n        move    #$10000,r5              ; Make r5 point to the start of fid memory");

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
   fprintf(fp,"\n        move    #$10000,r5              ; Make r5 point to the start of fid memory");
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

  Last modified: 12/6/08
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
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   InsertUniqueStringIntoList(nrPnts.Str(),&parList,szList);
   if(nrArgs >= 3)
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);
   if(nrArgs == 4)
      InsertUniqueStringIntoList(adrs.Str(),&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   };

   if(mode == "overwrite")
   {
		if(nrArgs == 2) // No delay
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire data (overwrite)");
		  
			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    #$10000,r5              ; Specify the save address");
			fprintf(fp,"\n        move    x:NR%s,r7                ; Load the number of samples into r7",nrPnts.Str()+1);
         fprintf(fp,"\n        do      r7,LBL%ld                 ; Collect n samples",label+2);
         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel A");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

 //        fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");

 //        fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label);
 //        fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    a1,y:(r5)+              ; Save to memory",label);

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel B");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

  //       fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
  //       fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label+1);
  //       fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    a1,y:(r5)+              ; Save to memory",label+1);

         fprintf(fp,"\nLBL%ld    nop",label+2);

         label += 3;
			fclose(fp);
		}
		else
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire data (overwrite)");

			fprintf(fp,"\n        move    x:DELAY%s,a1           ; Total acquisition time",duration.Str()+1);
			fprintf(fp,"\n        sub     #5,a"); // tweek it
			fprintf(fp,"\n        move    a1,r3");
			fprintf(fp,"\n        movep   r3,x:A_TCPR2");
			fprintf(fp,"\n        movep   #$200a01,x:A_TCSR2      ; Set timer2");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up adc timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (t1) for input capture");
         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Specify the save address",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    #$10000,r5              ; Specify the save address");
			fprintf(fp,"\n        move    x:NR%s,r7               ; Load the number of samples into r7",nrPnts.Str()+1);

         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label+2);
			fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel A");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

     //    fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
     //    fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label);
    //     fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    a1,y:(r5)+              ; Save to memory",label);

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel B");
         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

    //     fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
    //     fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label+1);
    //     fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    a1,y:(r5)+              ; Save to memory",label+1);

         fprintf(fp,"\nLBL%ld  nop",label+2);

			fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
			fprintf(fp,"\n        movep   #$200a00,x:A_TCSR2");

         label += 3;

			fclose(fp);
		}
	}
	else if(mode == "append")
	{
		if(nrArgs == 2) // No delay
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire (append)");

         fprintf(fp,"\n        move    y:DATA_ADRS,r5          ; Specify the data address",adrs.Str()+1);

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag"); // Get a dummy sample
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

			fprintf(fp,"\n        move    x:NR%s,r7                ; Load the number of samples into r7",nrPnts.Str()+1);

         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label+2);
			fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; clear timer1 flag");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel A");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

   //      fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
   //      fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label);
   //      fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    a1,y:(r5)+              ; Save to memory",label);

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel B");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");


   //      fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
   //      fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label+1);
   //      fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    a1,y:(r5)+              ; Save to memory",label+1);


         fprintf(fp,"\nLBL%ld  nop",label+2);

         label += 3;

         fprintf(fp,"\n        move    r5,y:DATA_ADRS           ; Save data address",adrs.Str()+1);

			fclose(fp);
		}
		else
		{
			fprintf(fp,"\n\n;");
			fprintf(fp,"\n;***************************************************************************");
			fprintf(fp,"\n; Acquire (append)");

         fprintf(fp,"\n        move    y:DATA_ADRS,r5           ; Specify the data address",adrs.Str()+1);

			fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
			fprintf(fp,"\n        sub     #5,a"); // Tweek it
			fprintf(fp,"\n        move    a1,r3");
			fprintf(fp,"\n        movep   r3,x:A_TCPR2");
			fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

			fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
			fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
			fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");
			fprintf(fp,"\n        move    x:NR%s,r7                ; Load the number of samples into r7",nrPnts.Str()+1);

         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label+2);
			fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; wait for timer1 flag");
			fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; clear timer1 flag");

    //     fprintf(fp,"\n        clr a");
         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel A");
         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

   //      fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
  //       fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label);
  //       fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    a1,y:(r5)+              ; Save to memory",label);

   //      fprintf(fp,"\n        clr a");
         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel B");
         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

  //       fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
  //       fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label+1);
 //        fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    a1,y:(r5)+              ; Save to memory",label+1);

         fprintf(fp,"\nLBL%ld  nop",label+2);

			fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
			fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2");
 
         fprintf(fp,"\n        move    r5,y:DATA_ADRS           ; Save data address",adrs.Str()+1);

         label += 3;

			fclose(fp);
		}		
	}

	else if(mode == "sum")
	{
		if(nrArgs == 2) // No delay
		{
         fprintf(fp,"\n\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Acquire (summing)");

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
             fprintf(fp,"\n        move    #$10000,r5              ; Load the number of samples into r7");
         fprintf(fp,"\n        move    x:NR%s,r7               ; Load the number of samples into r7",nrPnts.Str()+1);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label+2);

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel A");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");
    //     fprintf(fp,"\n        and      #$00FFFF,a             ; zero upper 4 bits");

   //      fprintf(fp,"\n        jclr    #15,a1,LBL%ld            ; Convert from 16 to 24 bit word",label);
   //      fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld   move    y:(r5),b1               ; Get last value at this location",label);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel B");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

    //     fprintf(fp,"\n        and      #$00FFFF,a             ; zero upper 4 bits");
    //     fprintf(fp,"\n        jclr    #15,a1,LBL%ld            ; Convert from 16 to 24 bit word",label+1);
   //      fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
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
         fprintf(fp,"\n; Acquire (summing)");

         fprintf(fp,"\n        clr     a                       ; Clear register a");
         fprintf(fp,"\n        clr     b                       ; Clear register b");
         fprintf(fp,"\n        move    x:DELAY%s,a1             ; Total acquisition time",duration.Str()+1);
         fprintf(fp,"\n        sub     #5,a"); // Tweek it
         fprintf(fp,"\n        move    a1,r3");
         fprintf(fp,"\n        movep   r3,x:A_TCPR2");
         fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
         fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         if(nrArgs == 4)
             fprintf(fp,"\n        move    x:MEM%s,r5               ; Load the number of samples into r7",adrs.Str()+1);
         else
             fprintf(fp,"\n        move    #$10000,r5              ; Load the number of samples into r7");
         fprintf(fp,"\n        move    x:NR%s,r7                ; Load the number of samples into r7",nrPnts.Str()+1);
         fprintf(fp,"\n        do      r7,LBL%ld                 ; Collect n samples",label+2);

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel A");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

     //    fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");

     //    fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label);
     //    fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    y:(r5),b1               ; Get last value at this location",label);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel B");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

      //   fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");

     //    fprintf(fp,"\n        jclr    #15,a1,LBL%ld             ; Convert from 16 to 24 bit word",label+1);
     //    fprintf(fp,"\n        or      #$FF0000,a              ; Extend sign if negative");
         fprintf(fp,"\nLBL%ld    move    y:(r5),b1               ; Get last value at this location",label+1);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");


         fprintf(fp,"\nLBL%ld    nop",label+2);

         fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
         fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2");


         fclose(fp);

         label += 3;
      }
   }
   else if(mode == "integrate")
   {
		if(nrArgs == 2) // No delay
		{
         fprintf(fp,"\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Acquire (integrating)");

         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
         fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:NR%s,r7               ; Load the number of samples into r7",nrPnts.Str()+1);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label+2);

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel A");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

  //       fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
  //       fprintf(fp,"\n        jclr    #15,a1,LBL%ld            ; Convert from 16 to 24 bit word",label);
  //       fprintf(fp,"\n        or      #$FF0000,a");
         fprintf(fp,"\nLBL%ld  move    y:(r5),b1               ; Get last value at this location",label);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel B");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

   //      fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
   //      fprintf(fp,"\n        jclr    #15,a1,LBL%ld            ; Convert from 16 to 24 bit word",label+1);
   //      fprintf(fp,"\n        or      #$FF0000,a");
         fprintf(fp,"\nLBL%ld   move    y:(r5),b1               ; Get last value at this location",label+1);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        sub     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Restore r5");


         fprintf(fp,"\nLBL%ld  nop",label+2);

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        add     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Increment r5 by 2");

         fclose(fp);

         label += 3;
      }
      else
      {
         fprintf(fp,"\n;");
         fprintf(fp,"\n;***************************************************************************");
         fprintf(fp,"\n; Acquire (integrating)");

         fprintf(fp,"\n        move    x:DELAY%s,a1           ; Total acquisition time",duration.Str()+1);
         fprintf(fp,"\n        sub     #5,a"); // Tweek it
         fprintf(fp,"\n        move    a1,r3");
         fprintf(fp,"\n        movep   r3,x:A_TCPR2");
         fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2      ; Set timer2");

         fprintf(fp,"\n        movep   #0,x:A_TLR1             ; Set up ADC timer");
         fprintf(fp,"\n        movep   #0,x:A_TCSR1            ; Disable timer");
         fprintf(fp,"\n        movep   #$361,x:A_TCSR1         ; Set up timer1 (T1) for input capture");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:NR%s,r7                ; Load the number of samples into r7",nrPnts.Str()+1);
         fprintf(fp,"\n        do      r7,LBL%ld                ; Collect n samples",label+2);

         fprintf(fp,"\n        jclr    #21,x:A_TCSR1,*         ; Wait for timer1 flag");
         fprintf(fp,"\n        movep   #$200361,x:A_TCSR1      ; Clear timer1 flag");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel A");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

     //    fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
     //    fprintf(fp,"\n        jclr    #15,a1,LBL%ld            ; Convert from 16 to 24 bit word",label);
    //     fprintf(fp,"\n        or      #$FF0000,a");
         fprintf(fp,"\nLBL%ld   move    y:(r5),b1               ; Get last value at this location",label);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    x:$20000,a1             ; Load data from channel B");

         fprintf(fp,"\n        lsl     #8,a");
         fprintf(fp,"\n        move    a1,y1");
         fprintf(fp,"\n        mpy     #$8000,y1,a");

    //     fprintf(fp,"\n        and      #$00FFFF,a             ; Zero upper 4 bits");
    //     fprintf(fp,"\n        jclr    #15,a1,LBL%ld            ; Convert from 16 to 24 bit word",label+1);
    //     fprintf(fp,"\n        or      #$FF0000,a");
         fprintf(fp,"\nLBL%ld   move    y:(r5),b1               ; Get last value at this location",label+1);
         fprintf(fp,"\n        add     b,a                     ; Accumulate in a");
         fprintf(fp,"\n        move    a1,y:(r5)+              ; Write to memory");

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        sub     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Restore r5");


         fprintf(fp,"\nLBL%ld  nop",label+2);

         fprintf(fp,"\n        move    r5,a1");
         fprintf(fp,"\n        add     #2,a");
         fprintf(fp,"\n        move    a1,r5                    ; Increment r5 by 2");

         fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for acqdelay to end");
         fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2");

         fclose(fp);

         label += 3;
      }
   }
   else
   {
      fclose(fp);
      ErrorMessage("invalid acquire mode");
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


short EndPP(DLLParameters* par, char *args)
{
   char *text;
   FILE *fp;
   char varName[50];
   long sz,len;
   short nrArgs;
   CText mode = "i",startFile;

   if((nrArgs = ArgScan(par->itfc,args,0,"mode","e","q",&mode)) < 0)
      return(nrArgs);


// Write the header file
   fp = fopen("temp.asm","w");
   if(!fp)
   {
      ErrorMessage("Can't open output file 'temp.asm'");
      return(ERR);
   }

   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n      nolist");
   fprintf(fp,"\n      include  'ioequ.asm'");
   fprintf(fp,"\n      list");
   fprintf(fp,"\n;***************************************************************************\n");

   fprintf(fp,"\n                org     x:$0\n\n");
   fprintf(fp,"\nPARAM_BASE      equ     *       ; Memory for pulse program parameters");
   fprintf(fp,"\nRXG1            ds      1       ; 00 - Receiver gain block 1");
   fprintf(fp,"\nRXG2            ds      1       ; 01 - Receiver gain block 2");
   fprintf(fp,"\nDEC2            ds      1       ; 02 - Decimation for CIC2");
   fprintf(fp,"\nDEC5            ds      1       ; 03 - Decimation for CIC5");
   fprintf(fp,"\nDECFIR          ds      1       ; 04 - Decimation for FIR");
   fprintf(fp,"\nATT2            ds      1       ; 05 - Attenuation for CIC2");
   fprintf(fp,"\nATT5            ds      1       ; 06 - Attenuation for CIC5");
   fprintf(fp,"\nATTFIR          ds      1       ; 07 - Attenuation for FIR");
   fprintf(fp,"\nNtaps           ds      1       ; 08 - Taps for FIR");
   fprintf(fp,"\nTXF00           ds      1       ; 09 - Tx Frequency word 0");
   fprintf(fp,"\nTXF01           ds      1       ; 10 - Tx Frequency word 1");
   fprintf(fp,"\nTXF02           ds      1       ; 11 - Tx Frequency word 2");
   fprintf(fp,"\nTXF03           ds      1       ; 12 - Tx Frequency word 3");
   fprintf(fp,"\nRXF00           ds      1       ; 13 - Rx Frequency word 0");
   fprintf(fp,"\nRXF01           ds      1       ; 14 - Rx Frequency word 1");
   fprintf(fp,"\nRXF02           ds      1       ; 15 - Rx Frequency word 2");
   fprintf(fp,"\nRXF03           ds      1       ; 16 - Rx Frequency word 3");
   fprintf(fp,"\nRXP0            ds      1       ; 17 - Rx Phase word 0");
   fprintf(fp,"\nRXP1            ds      1       ; 18 - Rx Phase word 1");
   fprintf(fp,"\nPGO             ds      1       ; 19 - Pulse gate overhead delay");

// Loop over variable list extracting variable names
   short c = 20;
   for(short i = 0; i < szList; i++)
   {
	   strncpy(varName,parList[i],50);
      if(varName[0] == 'f')
      {
         fprintf(fp,"\nFX%s_0           ds      1       ; %hd - Frequency %s word 0",varName+1,c++,varName+1);
         fprintf(fp,"\nFX%s_1           ds      1       ; %hd - Frequency %s word 1",varName+1,c++,varName+1);
         fprintf(fp,"\nFX%s_2           ds      1       ; %hd - Frequency %s word 2",varName+1,c++,varName+1);
         fprintf(fp,"\nFX%s_3           ds      1       ; %hd - Frequency %s word 3",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'a')
      {
         fprintf(fp,"\nTXA%s_0          ds      1       ; %hd - Tx amplitude %s word 0",varName+1,c++,varName+1);
         fprintf(fp,"\nTXA%s_1          ds      1       ; %hd - Tx amplitude %s word 1",varName+1,c++,varName+1);
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
   fp = fopen("startCode.asm","r");
   if(!fp)
   {
      ErrorMessage("Can't open startCode.asm file");
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
      ErrorMessage("Can't open output file");
      return(ERR);
   }
   fprintf(fp,"\n\n");
   fwrite(text,1,sz,fp);
   fclose(fp);
   delete [] text;

// Readin middle code
   fp = fopen("midCode.asm","r");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
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
      ErrorMessage("Can't open output file");
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
      ErrorMessage("Can't open output file");
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
      ErrorMessage("Can't open output file");
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


/*******************************************************************************

   Make a gradient ramp which has a start and end amplitude and a duration
  
   Syntax   gradramp(x/y/z, start, end, nrSteps, stepDuration)
  
   start and end can be constants or tables

*********************************************************************************/

short RampedGradient(DLLParameters* par, char *args)
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
      ErrorMessage("Pulse sequence not initialised");
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
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n        movep   #$2C,x:A_PCRD          ; Turn on SSI 1 on Port D");

   // Choose the gradient level
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
   else if(sscanf(grad.Str(),"%ld",&Address) == 1)
   {
      fprintf(fp,"\n        move    #%ld,a1",Address);
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
   }
   else
   {
      fclose(fp);
      ErrorMessage("Invalid address '%s'",grad.Str());
      return(ERR);
   }


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
      fprintf(fp,"\n        move    #%ld,a1",amplitude);
      fprintf(fp,"\n        move    a1,y:(TMP+7)");  
   }
   else
   {
      ErrorMessage("Invalid start amplitude '%s'",steps.Str());
      fclose(fp);
      return(ERR);
   }

// Get the end amplitude
   if(end[0] == 'n') // End amplitude is via a number reference
   {
      fprintf(fp,"\n        move    x:NR%s,a1",end.Str()+1);
      fprintf(fp,"\n        move    a1,y:(TMP+8)");    
   }
   else if(end[0] == 't') // end amplitude is t[index]
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
   else if(sscanf(end.Str(),"%ld",&amplitude) == 1) // Start amplitude is a number
   {
      fprintf(fp,"\n        move    #%ld,a1",amplitude);
      fprintf(fp,"\n        move    a1,y:(TMP+8)");  
   }
   else
   {
      ErrorMessage("Invalid end amplitude '%s'",steps.Str());
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
         ErrorMessage("Invalid number of steps '%s'",steps.Str());
         fclose(fp);
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("Invalid number of steps '%s'",steps.Str());
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
         ErrorMessage("invalid delay '%ld' [1...327670]",Duration);
         fclose(fp);
         return(ERR);
      }
      Duration = (long)(fDuration * 50 - 1 + 0.5);
      fprintf(fp,"\n        move   #%ld,a1",Duration);
   }
   else
   {
      ErrorMessage("Invalid step duration '%s'",duration.Str());
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
   fprintf(fp,"\n        lsl     #8,b");    
   fprintf(fp,"\n        movep   b1,x:A_TX10");


// Include a delay to make the step the right length
   fprintf(fp,"\n        move    y:(TMP+9),a1"); 
   fprintf(fp,"\n        sub     #47,a");
   fprintf(fp,"\n        movep   a1,x:A_TCPR2");
	fprintf(fp,"\n        movep   #$200A01,x:A_TCSR2");
   fprintf(fp,"\n        nop");
   fprintf(fp,"\n        jclr    #21,x:A_TCSR2,*         ; Wait for pulse to end");
   fprintf(fp,"\n        movep   #$200A00,x:A_TCSR2      ; Turn off timer");

   //fprintf(fp,"\n        rep     #124                  ; Wait 1us");
   //fprintf(fp,"\n        nop");
   fprintf(fp,"\n        move    y:(TMP+5),a0");  
   fprintf(fp,"\n        inc    a"); 
   fprintf(fp,"\n        move    a0,y:(TMP+5)");  

  fprintf(fp,"\nLBL%ld    nop",label);

// Include a delay to make the last step the right length
   fprintf(fp,"\n        rep     #80                  ; Wait 1us");
   fprintf(fp,"\n        nop");

   fprintf(fp,"\n        movep   #$24,x:A_PCRD          ; Turn off SSI 1 on Port D");

   label += 2;

   fclose(fp);

   return(OK);
}