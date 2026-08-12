/////////////////////////////////////////
// Terranova Pulse Programmer
/////////////////////////////////////////

#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <shellapi.h>
// Locally defined procedure and global variables

#define VERSION 2.10

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool  GetCommandSyntax(char* cmd, char* syntax);

short AcquireData(DLLParameters*,char*);
short ClearData(DLLParameters*,char*);
short DelayPulse(DLLParameters*,char*);
short DoRemez(DLLParameters*,char*parameters);
short EndPP(DLLParameters*,char *args);
short ExecuteAndWait(DLLParameters*,char*);
short GetHelpFolder(DLLParameters*,char*);
short GetPPVersion(DLLParameters*,char*);
short InitialisePP(DLLParameters*,char*);
short LoopEnd(DLLParameters*,char*);
short LoopStart(DLLParameters*,char*);
short MakeADelay(DLLParameters*,char*);
short MakeAnRFPulse(DLLParameters*,char*);
short PGSEOff(DLLParameters*,char*);
short PGSEOn(DLLParameters*,char*);
short PolarizingPulse(DLLParameters*,char*);
short PolarizingPulse2(DLLParameters*,char*);
short ResetMemoryPointer(DLLParameters*,char*);
short ResetTxOscillator(DLLParameters*,char*);
short SetGradientLevel(DLLParameters*,char*);
short SetTTL(DLLParameters*,char*);
short ShapedGradientPulse(DLLParameters*,char*);
short ShapedRFPulse(DLLParameters*,char*);
short StartPolarizingPulse(DLLParameters*,char*);
short StopPolarizingPulse(DLLParameters*,char*);
short TTLOff(DLLParameters*,char*);
short TTLOn(DLLParameters*,char*);


void InsertUniqueStringIntoList(char *str, char ***list, long &position);


char **parList; // Parameter list - built up by pp commands
long szList;    // Number of entries in parameter list
long label;     // label counter (to generate unique label)

// Extension procedure to add commands to Prospa 

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *par)
{
   short r = RETURN_FROM_DLL;
      
        if(!strcmp(command,"acquire"))      r = AcquireData(par,parameters);  
   else if(!strcmp(command,"cleardata"))    r = ClearData(par,parameters);      
   else if(!strcmp(command,"delay"))        r = MakeADelay(par,parameters);   
   else if(!strcmp(command,"endloop"))      r = LoopEnd(par,parameters);      
   else if(!strcmp(command,"endpp"))        r = EndPP(par,parameters);  
   else if(!strcmp(command,"execwait"))     r = ExecuteAndWait(par,parameters);
   else if(!strcmp(command,"grad"))         r = SetGradientLevel(par,parameters);   
   else if(!strcmp(command,"pgseon"))       r = PGSEOn(par,parameters);  
   else if(!strcmp(command,"pgseoff"))      r = PGSEOff(par,parameters);  
   else if(!strcmp(command,"offseton"))     r = PGSEOn(par,parameters);  
   else if(!strcmp(command,"offsetoff"))    r = PGSEOff(par,parameters); 
   else if(!strcmp(command,"helpfolder"))   r = GetHelpFolder(par,parameters);   
   else if(!strcmp(command,"initpp"))       r = InitialisePP(par,parameters);      
   else if(!strcmp(command,"loop"))         r = LoopStart(par,parameters);     
   else if(!strcmp(command,"memreset"))     r = ResetMemoryPointer(par,parameters);     
   else if(!strcmp(command,"ppversion"))    r = GetPPVersion(par,parameters);      
   else if(!strcmp(command,"pulse"))        r = MakeAnRFPulse(par,parameters);      
   else if(!strcmp(command,"polzpulse"))    r = PolarizingPulse(par,parameters);   
   else if(!strcmp(command,"polzpulse2"))   r = PolarizingPulse2(par,parameters);  
//   else if(!strcmp(command,"remez"))        r = DoRemez(par,parameters);   
   else if(!strcmp(command,"resettx"))      r = ResetTxOscillator(par,parameters);   
   else if(!strcmp(command,"shapedrf"))     r = ShapedRFPulse(par,parameters);   
   else if(!strcmp(command,"shapedgrad"))   r = ShapedGradientPulse(par,parameters); 
   else if(!strcmp(command,"polzon"))       r = StartPolarizingPulse(par,parameters); 
   else if(!strcmp(command,"polzoff"))      r = StopPolarizingPulse(par,parameters); 
   else if(!strcmp(command,"ttlon"))        r = TTLOn(par,parameters);      
   else if(!strcmp(command,"ttloff"))       r = TTLOff(par,parameters);      
    
                
   return(r);
}

// Extension procedure to list commands in DLL 

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   Terranova Pulse Programmer DLL module (V%1.2f)\n\n",VERSION);
   TextMessage("   acquire ..... acquire some data\n");
   TextMessage("   cleardata ... clear data memory\n");
   TextMessage("   delay ....... generate a short delay\n");
   TextMessage("   endloop ..... end a loop\n");
   TextMessage("   endpp ....... finish the pulse program\n");
   TextMessage("   execwait .... execute a program and wait for it to exit\n");
   TextMessage("   grad ........ set gradient amplitude\n");
   TextMessage("   initpp ...... initialise pulse program\n");
   TextMessage("   loop ........ start a loop\n");
   TextMessage("   memreset .... reset memory pointer\n");
   TextMessage("   pgseon ...... switch on the pgse gradient\n");
   TextMessage("   pgseoff ..... switch off the pgse gradient\n");
   TextMessage("   offseton .... switch on a B0 offset\n");
   TextMessage("   offsetoff ... switch off the B0 offset\n");
   TextMessage("   polzpulse ... make a polarizing pulse\n");
   TextMessage("   polzpon ..... switch on the polariztion\n");
   TextMessage("   polzoff ..... switch off the polarization\n");
   TextMessage("   polzpulse2 .. make a polarizing pulse with controlled turnoff\n");
   TextMessage("   ppversion ... returns the version number of this DLL\n");
   TextMessage("   pulse ....... generate an RF pulse\n");
   TextMessage("   resettx ..... reset the digital oscillator\n");
   TextMessage("   shapedrf .... make an amplitude modulated RF pulse\n");
   TextMessage("   shapedgrad .. make an amplitude modulated gradient pulse\n");
   TextMessage("   ttlon ....... switch on a TTL level\n");
   TextMessage("   ttloff ...... switch off a TTL level\n");

}

/*******************************************************************************
   Extension procedure to return syntax in DLL 
*******************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';

   if(!strcmp(cmd,"acquire"))            strcpy(syntax,"acquire(mode, number points:n)");
   else if(!strcmp(cmd,"cleardata"))     strcpy(syntax,"cleardata(number:n)");
   else if(!strcmp(cmd,"delay"))         strcpy(syntax,"delay(duration:d)");
   else if(!strcmp(cmd,"endloop"))       strcpy(syntax,"endloop(name)");
   else if(!strcmp(cmd,"endpp"))         strcpy(syntax,"endpp()");
   else if(!strcmp(cmd,"execwait"))      strcpy(syntax,"execwait(program,arguments)");
   else if(!strcmp(cmd,"grad"))          strcpy(syntax,"grad(address:n,level:g)");
   else if(!strcmp(cmd,"pgseon"))        strcpy(syntax,"pgseon(amplitude:g)");
   else if(!strcmp(cmd,"pgseoff"))       strcpy(syntax,"pgseoff()");
   else if(!strcmp(cmd,"offseton"))      strcpy(syntax,"offseton(amplitude:g)");
   else if(!strcmp(cmd,"offsetoff"))     strcpy(syntax,"offsetoff()");
   else if(!strcmp(cmd,"initpp"))        strcpy(syntax,"initpp(filename)");
   else if(!strcmp(cmd,"loop"))          strcpy(syntax,"loop(name:l,number:n)");
   else if(!strcmp(cmd,"memreset"))      strcpy(syntax,"memreset()");
   else if(!strcmp(cmd,"ppversion"))     strcpy(syntax,"(INT v) = ppversion()");
   else if(!strcmp(cmd,"pulse"))         strcpy(syntax,"pulse(phase:p,duration:d)");
   else if(!strcmp(cmd,"pgseon"))        strcpy(syntax,"pgseon(amplitude:g)");
   else if(!strcmp(cmd,"pgseoff"))       strcpy(syntax,"pgseoff()");
   else if(!strcmp(cmd,"polzpulse"))     strcpy(syntax,"polzpulse(amplitude:a,duration:d)");
   else if(!strcmp(cmd,"polzon"))        strcpy(syntax,"polzon(amplitude:a)");
   else if(!strcmp(cmd,"polzoff"))       strcpy(syntax,"polzoff()");
   else if(!strcmp(cmd,"polzpulse2"))    strcpy(syntax,"polzpulse2(amplitude:a,duration:d,table:t,table_size:n,table_step_duration:d)");
   else if(!strcmp(cmd,"resettx"))       strcpy(syntax,"resettx()");
   else if(!strcmp(cmd,"shapedrf"))      strcpy(syntax,"shapedrf(atable:t, phase:p, pulse_duration:d)");
   else if(!strcmp(cmd,"shapedgrad"))    strcpy(syntax,"shapedgrad(address:n, atable:t, table_size:n, table_step_duration:d)");
   else if(!strcmp(cmd,"ttlon"))         strcpy(syntax,"ttlon(byte:b)");
   else if(!strcmp(cmd,"ttloff"))        strcpy(syntax,"ttloff(byte:b)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short GetHelpFolder(DLLParameters *par, char *args)
{
   par->retVar[1].MakeAndSetString("Macros\\TN-PP");
   par->nrRetVar = 1;
   return(OK);
}

/*******************************************************************************
  This function will generate a file with the pulse parameters 
  and initialization code 
*******************************************************************************/

short InitialisePP(DLLParameters *par, char *args)
{
   char dir[MAX_PATH];
   short nrArgs;

   if((nrArgs = ArgScan(par->itfc,args,1,"working directory","e","s",dir)) < 0)
      return(nrArgs);

   if(!SetCurrentDirectory(dir))
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
   remove("midCode.asm");

   return(OK);
}

// Return the version number
short GetPPVersion(DLLParameters *par, char *args)
{
   par->retVar[1].MakeAndSetFloat(VERSION);
   par->nrRetVar = 1;
   return(OK);
}
         
short ResetMemoryPointer(DLLParameters *par, char *args)
{
   short nrArgs;
   char incTx[10],incRx[10];


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

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Reset the memory pointer");
	fprintf(fp,"\n        move    #$10000,r4              ; Make r4 point to the start of fid memory");

   fclose(fp);

   return(OK);
}



/**********************************************************************
                   Produce a polarizing pulse
**********************************************************************/

short PolarizingPulse(DLLParameters *par, char *args)
{
   short nrArgs;
   CText amp,dur;

   
   if((nrArgs = ArgScan(par->itfc,args,2,"amplitude, duration","ee","tt",&amp,&dur)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(amp.Str(),&parList,szList);
   InsertUniqueStringIntoList(dur.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Make a polarization pulse");

   fprintf(fp,"\n; Load the current bus state"); 
   fprintf(fp,"\n        move    y:TTLState,x1");
   fprintf(fp,"\n        move    y:GradState,a1");
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x1");

   fprintf(fp,"\n; Set up the polarizing amplitude"); 
   fprintf(fp,"\n        move    x:POLZI%s,a1",amp.Str()+1); 
   fprintf(fp,"\n        movep   #$05,x:A_PDRE        ; Select Polz dac LTC1655"); 
   fprintf(fp,"\n        lsl     #8,a"); 
   fprintf(fp,"\n        movep   a1,x:A_TX00"); 

   fprintf(fp,"\n; Turn on polarizing relay and wait for relay to settle"); 
   fprintf(fp,"\n        move    #$8000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:PZDLY1,r7");
   fprintf(fp,"\n        bsr     wait");
   
   fprintf(fp,"\n; Turn on polarizing input gate and wait while sample is polarized"); 
   fprintf(fp,"\n        move    #$A000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:DELAY%s,r7",dur.Str()+1);
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n; Turn off polarizing input gate and wait for adiabatic switch off"); 
   fprintf(fp,"\n        move    #$8000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:PZDLY2,r7");
   fprintf(fp,"\n        bsr     wait");
        
   fprintf(fp,"\n; Turn off polarizing relay and wait for contact bounce"); 
   fprintf(fp,"\n        move    #$0000,a1"); 
   fprintf(fp,"\n        or      x1,a1"); 
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:PZDLY3,r7");
   fprintf(fp,"\n        bsr     wait");

   //fprintf(fp,"\n; Turn on the polarizing short and wait for contact bounce"); 
   //fprintf(fp,"\n        move    #$4000,a1"); 
   //fprintf(fp,"\n        or      x1,a1"); 
   //fprintf(fp,"\n        move    a1,x:$20000");
   //fprintf(fp,"\n        move    a1,y:TTLState");
   //fprintf(fp,"\n        move    x:PZDLY4,r7");
   //fprintf(fp,"\n        bsr     wait");

   fclose(fp);

   return(OK);
}




/**********************************************************************
                   Start a polarizing pulse
**********************************************************************/

short StartPolarizingPulse(DLLParameters *par, char *args)
{
   short nrArgs;
   CText amp,dur;

   
   if((nrArgs = ArgScan(par->itfc,args,1,"amplitude","e","t",&amp)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(amp.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Start a polarization pulse");

   fprintf(fp,"\n; Load the current bus state"); 
   fprintf(fp,"\n        move    y:TTLState,x1");
   fprintf(fp,"\n        move    y:GradState,a1");
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x1");

   fprintf(fp,"\n; Set up the polarizing amplitude"); 
   fprintf(fp,"\n        move    x:POLZI%s,a1",amp.Str()+1); 
   fprintf(fp,"\n        movep   #$05,x:A_PDRE        ; Select Polz dac LTC1655"); 
   fprintf(fp,"\n        lsl     #8,a"); 
   fprintf(fp,"\n        movep   a1,x:A_TX00"); 

   fprintf(fp,"\n; Turn on polarizing relay and wait for relay to settle"); 
   fprintf(fp,"\n        move    #$8000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:PZDLY1,r7");
   fprintf(fp,"\n        bsr     wait");
   
   fprintf(fp,"\n; Turn on polarizing input gate"); 
   fprintf(fp,"\n        move    #$A000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");

   fprintf(fp,"\n; Save the polarization state"); 
   fprintf(fp,"\n        move    a1,y:PolzState");



   fclose(fp);

   return(OK);
}


/**********************************************************************
                   Produce a polarizing pulse
**********************************************************************/

short StopPolarizingPulse(DLLParameters *par, char *args)
{
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

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Make a polarization pulse");

   fprintf(fp,"\n; Load the current bus state"); 
   fprintf(fp,"\n        move    y:TTLState,x1");
   fprintf(fp,"\n        move    y:GradState,a1");
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x1");

   fprintf(fp,"\n; Turn off polarizing input gate and wait for adiabatic switch off"); 
   fprintf(fp,"\n        move    #$8000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:PZDLY2,r7");
   fprintf(fp,"\n        bsr     wait");
        
   fprintf(fp,"\n; Turn off polarizing relay and wait for contact bounce"); 
   fprintf(fp,"\n        move    #$0000,a1"); 
   fprintf(fp,"\n        or      x1,a1"); 
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:PZDLY3,r7");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n; Save the polarization state"); 
   fprintf(fp,"\n        move    #$0000,a1"); 
   fprintf(fp,"\n        move    a1,y:PolzState");

   fclose(fp);

   return(OK);
}




/**********************************************************************
            Produce a polarizing pulse with a shaped decay
**********************************************************************/

short PolarizingPulse2(DLLParameters *par, char *args)
{
   short nrArgs;
   CText amp,dur,table,size,step;

   
   if((nrArgs = ArgScan(par->itfc,args,5,"amplitude, duration, table, table_size, table_step","eeeee","ttttt",&amp,&dur,&table,&size,&step)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(amp.Str(),&parList,szList);
   InsertUniqueStringIntoList(dur.Str(),&parList,szList);
   InsertUniqueStringIntoList(table.Str(),&parList,szList);
   InsertUniqueStringIntoList(size.Str(),&parList,szList);
   InsertUniqueStringIntoList(step.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Make a polarization pulse");

   fprintf(fp,"\n; Load the current TTL state"); 
   fprintf(fp,"\n        move    y:TTLState,x1");

   fprintf(fp,"\n; Set up the polarizing amplitude"); 
   fprintf(fp,"\n        move    X:RFAMP%s,a1",amp.Str()+1); 
   fprintf(fp,"\n        movep   #$05,x:A_PDRE        ; Select Polz dac LTC1655"); 
   fprintf(fp,"\n        lsl     #8,a"); 
   fprintf(fp,"\n        movep   a1,x:A_TX00"); 

   fprintf(fp,"\n; Turn on polarizing relay and wait for relay to settle"); 
   fprintf(fp,"\n        move    #$8000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:PZDLY1,r7");
   fprintf(fp,"\n        bsr     wait");
   
   fprintf(fp,"\n; Turn on polarizing input gate and wait while sample is polarized"); 
   fprintf(fp,"\n        move    #$A000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:DELAY%s,r7",dur.Str()+1);
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n; Start stepping through the current vector"); 
   fprintf(fp,"\n        move    x:TABLE%s,r5",table.Str()+1);
   fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);
   fprintf(fp,"\n        move    y:$12000,r3");

   fprintf(fp,"\n        do      r2,LBL%ld               ; Step the current r7 times",label);
   fprintf(fp,"\n        move    y:(r5)+,a1              ; Load current");
   fprintf(fp,"\n        lsl     #8,a"); 
   fprintf(fp,"\n        movep   a1,x:A_TX00             ; Output current");
   fprintf(fp,"\n        move    x:DELAY%s,r7            ; Wait",step.Str()+1);
   fprintf(fp,"\n        bsr     twait"); 
   fprintf(fp,"\nLBL%ld  nop",label++);
   
   fprintf(fp,"\n; Wait for current to drop to zero"); 
   fprintf(fp,"\n        move    x:PZDLY4,r7");
   fprintf(fp,"\n        bsr     wait");

   fprintf(fp,"\n; Turn off polarizing input gate and wait for adiabatic switch off"); 
   fprintf(fp,"\n        move    #$8000,a1"); 
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    x:PZDLY2,r7");
   fprintf(fp,"\n        bsr     wait");
        
   fprintf(fp,"\n; Turn off polarizing relay and wait for contact bounce"); 
   fprintf(fp,"\n        move    #$0000,a1"); 
   fprintf(fp,"\n        or      x1,a1"); 
   fprintf(fp,"\n        move    a1,x:$20000");
   fprintf(fp,"\n        move    a1,y:TTLState");
   fprintf(fp,"\n        move    x:PZDLY3,r7");
   fprintf(fp,"\n        bsr     wait");

   //fprintf(fp,"\n; Turn on the polarizing short and wait for contact bounce"); 
   //fprintf(fp,"\n        move    #$4000,a1"); 
   //fprintf(fp,"\n        or      x1,a1"); 
   //fprintf(fp,"\n        move    a1,x:$20000");
   //fprintf(fp,"\n        move    a1,y:TTLState");
   //fprintf(fp,"\n        move    x:PZDLY4,r7");
   //fprintf(fp,"\n        bsr     wait");

   fclose(fp);

   return(OK);
}


/**********************************************************************
            Reset the digital oscillator
**********************************************************************/

short ResetTxOscillator(DLLParameters *par, char *args)
{
   short nrArgs;
   char freq[10];

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

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Reset Tx digital oscillator");
   fprintf(fp,"\n;"); 

   fprintf(fp,"\n        move    x:DOSCA1,x0             ; Set up dig osc A");
   fprintf(fp,"\n        move    x0,y:DOSC_BUFF_BASE     ; Load default coeff for osc");
   fprintf(fp,"\n        move    x:DOSCA2,x0"); 
   fprintf(fp,"\n        move    x0,y:DOSC_BUFF_BASE+1   ; Load default s1 for osc");
   fprintf(fp,"\n        move    x:DOSCA3,x0");
   fprintf(fp,"\n        move    x0,y:DOSC_BUFF_BASE+2   ; Load default s2 for osc");
   fprintf(fp,"\n        move    x:DOSCB1,x0             ; Set up dig osc B");
   fprintf(fp,"\n        move    x0,y:DOSC_BUFF_BASE+3   ; Load default coeff for osc");
   fprintf(fp,"\n        move    x:DOSCB2,x0");
   fprintf(fp,"\n        move    x0,y:DOSC_BUFF_BASE+4   ; Load default s1 for osc");
   fprintf(fp,"\n        move    x:DOSCB3,x0");
   fprintf(fp,"\n        move    x0,y:DOSC_BUFF_BASE+5   ; Load default s2 for osc");       // Now update the Tx output 

   fclose(fp);


   return(OK);
}


/**********************************************************************
     Generate an RF pulse setting duration and phase

     r1 = 010 +x pulse
          100 +y pulse
          011 -x pulse
          101 -y pulse

     r7 = number of 10us cycles

     r2 = amplitude (optional)

**********************************************************************/

short MakeAnRFPulse(DLLParameters *par, char *args)
{
   short nrArgs;
   CText cycles,phase,amplitude;

   
   if((nrArgs = ArgScan(par->itfc,args,3,"amplitude, phase, duration","eee","ttt",&amplitude,&phase,&cycles)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(amplitude.Str(),&parList,szList);
   InsertUniqueStringIntoList(cycles.Str(),&parList,szList);
   InsertUniqueStringIntoList(phase.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

	fprintf(fp,"\n\n;");
	fprintf(fp,"\n;***************************************************************************");
	fprintf(fp,"\n; Generate a pulse");
	fprintf(fp,"\n;"); 

   fprintf(fp,"\n        move    x:DELAY%s,r7     ; Set number of PP cycles",cycles.Str()+1);
   fprintf(fp,"\n        move    x:RFAMP%s,r2       ; Set Tx amplitude",amplitude.Str()+1);
   fprintf(fp,"\n        move    x:PHASE%s,a0     ; Set Tx phase",phase.Str()+1);
   fprintf(fp,"\n        move    #$40,b0          ; Amplitude"); 
   fprintf(fp,"\n        add     b,a              ; affect pulse"); 
   fprintf(fp,"\n        move    a0,r1            ; Set mode");
   fprintf(fp,"\n        bsr     wait"); 
   fprintf(fp,"\n        clr     a"); 
   fprintf(fp,"\n        movep   a,x:A_TX10      ; Set Tx to zero"); 
	fclose(fp);




   return(OK);

}



/**********************************************************************
     Generate a shaped RF pulse
**********************************************************************/

short ShapedRFPulse(DLLParameters *par, char *args)
{
   short nrArgs;
   

   CText atable,phase,duration;

   if((nrArgs = ArgScan(par->itfc,args,3,"atable, phase, pulse_duration","eee","ttt",&atable,&phase,&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(atable.Str(),&parList,szList);
   InsertUniqueStringIntoList(phase.Str(),&parList,szList);
   InsertUniqueStringIntoList(duration.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Generate a shaped RF pulse");


   fprintf(fp,"\n        move    x:TABLE%s,r3",atable.Str()+1);
   fprintf(fp,"\n        move    r3,x:shapedmem"); // Save initial address

   fprintf(fp,"\n; Delay");
   fprintf(fp,"\n        move    x:DELAY%s,r7     ; Number of cycles",duration.Str()+1); 
   fprintf(fp,"\n        move    x:PHASE%s,a0     ; Set Tx phase",phase.Str()+1);
   fprintf(fp,"\n        move    #$20,b0          ; Shaped-pulse"); 
   fprintf(fp,"\n        add     b,a              ; Shaped-pulse"); 
   fprintf(fp,"\n        move    a0,r1            ; Set mode");

   fprintf(fp,"\n        bsr     wait"); // Do the pulse

   fprintf(fp,"\n        clr     a"); 
   fprintf(fp,"\n        movep   a,x:A_TX10      ; Set Tx to zero"); 

   fclose(fp);

   return(OK);

}

short PGSEOn(DLLParameters *par, char *args)
{
   short nrArgs;
   CText address,amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,1,"amplitude","e","t",&amplitude)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(amplitude.Str(),&parList,szList);

   FILE *fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Switch on the PGSE Gradient");

   //; Set the PGSE gradient amplitude
   fprintf(fp,"\n        move    #$0800,a1            ; Select PGSE gradient"); 
   fprintf(fp,"\n        move    y:TTLState,x1        ; Make sure current TTL state remains");
   fprintf(fp,"\n        or      x1,a1                ; Combine with TTL state");
   fprintf(fp,"\n        move    y:PolzState,x1       ; Combine with polarization state");
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000"); 
   fprintf(fp,"\n        move    #1,r7"); 
   fprintf(fp,"\n        bsr     wait"); 
   fprintf(fp,"\n        move    x:GRAD%s,a1           ; Set gradient amplitude (pgse)",amplitude.Str()+1); 
   fprintf(fp,"\n        movep   #$01,x:A_PDRE        ; Select Z gradient (needed for logic"); 
   fprintf(fp,"\n        lsl     #8,a                 ; to work)"); 
   fprintf(fp,"\n        movep   a1,x:A_TX00"); 
   fprintf(fp,"\n        move    #1,r7"); 
   fprintf(fp,"\n        bsr     wait"); 
   fprintf(fp,"\n        move    #$0100,a1            ; Switch on PGSE gradient");
   fprintf(fp,"\n        move    a1,y:GradState       ; Save the gradient state");
   fprintf(fp,"\n        move    y:TTLState,x1        ; Make sure current TTL state remains");
   fprintf(fp,"\n        or      x1,a1                ; Combine with TTL state");
   fprintf(fp,"\n        move    y:PolzState,x1       ; Combine with polarization state");
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000"); 

   fclose(fp);

   return(OK);
}

short PGSEOff(DLLParameters *par, char *args)
{
   short nrArgs;
   
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

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Switch off the PGSE Gradient");

   // Switch off the PGSE output
   fprintf(fp,"\n        move    #$0000,a1            ; Switch on PGSE gradient");
   fprintf(fp,"\n        move    a1,y:GradState       ; Make sure current TTL state remains");
   fprintf(fp,"\n        move    y:TTLState,x1        ; Make sure current TTL state remains");
   fprintf(fp,"\n        or      x1,a1                ; Combine with TTL state");
   fprintf(fp,"\n        move    y:PolzState,x1       ; Combine with polarization state");
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    a1,x:$20000"); 

   fclose(fp);

   return(OK);
}

/**********************************************************************
     Set a gradient level - takes 2.2 us

     gradon([1,2,3],level)

**********************************************************************/

short SetGradientLevel(DLLParameters *par, char *args)
{
   short nrArgs;
   CText address,amplitude;
   
   if((nrArgs = ArgScan(par->itfc,args,2,"address,amplitude","ee","tt",&address,&amplitude)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
   InsertUniqueStringIntoList(address.Str(),&parList,szList);
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

   // Choose the gradient
   fprintf(fp,"\n        clr b");
   fprintf(fp,"\n        move    x:NR%s,b1",address.Str()+1);
   fprintf(fp,"\n        cmp     #01,b");
   fprintf(fp,"\n        bne     LBL%ld",label);
   fprintf(fp,"\n        move    #02,a1");
   fprintf(fp,"\n        bra     LBL%ld",label+2);

   fprintf(fp,"\nLBL%ld    cmp     #02,b",label);
   fprintf(fp,"\n        bne     LBL%ld",label+1);
   fprintf(fp,"\n        move    #06,a1");
   fprintf(fp,"\n        bra     LBL%ld",label+2);

   fprintf(fp,"\nLBL%ld    cmp     #03,b",label+1);
   fprintf(fp,"\n        bne     ABORT");
   fprintf(fp,"\n        move    #01,a1");


// Set the level
   fprintf(fp,"\nLBL%ld    movep   a1,x:A_PDRE",label+2);
   fprintf(fp,"\n        move    x:GRAD%s,a1",amplitude.Str()+1);
   fprintf(fp,"\n        lsl     #8,a");    
   fprintf(fp,"\n        movep   a1,x:A_TX00            ; Set up gradient level");
   fprintf(fp,"\n        move    #2,r7                  ; Wait 20 us");
   fprintf(fp,"\n        bsr     wait");


   label += 3;

   fclose(fp);

   return(OK);
}



/**********************************************************************
     Generate a short delay 

     delay(duration)

**********************************************************************/

short MakeADelay(DLLParameters *par, char *args)
{
   short nrArgs;
   CText duration;

   if((nrArgs = ArgScan(par->itfc,args,1,"duration name","e","t",&duration)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }
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
   fprintf(fp,"\n        move    x:DELAY%s,r7",duration.Str()+1);
   fprintf(fp,"\n        bsr     wait");

   fclose(fp);

   return(OK);

}  


/**********************************************************************
     Switch on a TTl level 

     ttlon(byte)

     Takes 150 ns to set up level. The TTL level is combined with the
     lower 16 bits to prevent any disruption to the RF.
**********************************************************************/

short TTLOn(DLLParameters *par, char *args)
{
   short nrArgs;
   CText byte;

   if((nrArgs = ArgScan(par->itfc,args,1,"byte name","e","t",&byte)) < 0)
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

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    x:TTL%s,a1              ; Read in the TTL byte",byte.Str()+1);
   fprintf(fp,"\n        move    #$FFFF03,x0             ; Mask for outputs");
   fprintf(fp,"\n        and     x0,a                    ; Check for invalid TTL lines");
   fprintf(fp,"\n        tst     a");
   fprintf(fp,"\n        bne     ABORT                   ; Invalid TTL line",label);

   fprintf(fp,"\n        move    x:TTL%s,a1              ; Read in the TTL byte",byte.Str()+1);
   fprintf(fp,"\n        move    y:TTLState,x1           ; Make sure current TTL state remains");
   fprintf(fp,"\n        or      x1,a1                   ; Combine with TTL state");
   fprintf(fp,"\n        move    a1,y:TTLState           ; Update TTL state");
   fprintf(fp,"\n        move    y:PolzState,x1          ; Combine with polarization state");
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    y:GradState,x1          ; Combine with gradient state");
   fprintf(fp,"\n        or      x1,a1");

   fprintf(fp,"\n        move    a1,x:$20000             ; Update TTL output");


   fclose(fp);

   return(OK);

}  

/**********************************************************************
     Switch off a TTl level 

     ttloff(byte)

     Takes 150 ns to set up level. The TTL level is combined with the
     lower 16 bits to prevent any disruption to the RF.
**********************************************************************/

short TTLOff(DLLParameters *par, char *args)
{
   short nrArgs;
   CText byte;

   if((nrArgs = ArgScan(par->itfc,args,1,"byte name","e","t",&byte)) < 0)
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
   fprintf(fp,"\n; Reset TTL level");

   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        clr     b");
   fprintf(fp,"\n        move    x:TTL%s,a1              ; Read in the TTL byte",byte.Str()+1);
   fprintf(fp,"\n        move    #$FFFF03,x0             ; Mask for outputs");
   fprintf(fp,"\n        and     x0,a                    ; Check for invalid TTL lines");
   fprintf(fp,"\n        tst     a");
   fprintf(fp,"\n        bne     LBL%ld                  ; Invalid TTL line",label);

   fprintf(fp,"\n        move    x:TTL%s,a1              ; Read in the TTL byte",byte.Str()+1);
   fprintf(fp,"\n        not     a                       ; Invert the byte");
   fprintf(fp,"\n        move    y:TTLState,x1           ; Make sure current TTL state remains");
   fprintf(fp,"\n        and     x1,a1                   ; Combine with TTL state");
   fprintf(fp,"\n        move    a1,y:TTLState           ; Update TTL state");

   fprintf(fp,"\n        move    y:PolzState,x1          ; Combine with polarization state");
   fprintf(fp,"\n        or      x1,a1");
   fprintf(fp,"\n        move    y:GradState,x1          ; Combine with gradient state");
   fprintf(fp,"\n        or      x1,a1");

   fprintf(fp,"\n        move    a1,x:$20000             ; Update TTL output");

   fprintf(fp,"\nLBL%ld  ",label++);

   fclose(fp);

   return(OK);

}  

/**********************************************************************
     Generate the first statement of a loop 

     loop(name, repeats)

**********************************************************************/

short LoopStart(DLLParameters *par, char *args)
{
   short nrArgs;
   char nrTimes[10];
   char loopName[10];
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,2,"loop name, number repeats","ee","ss",loopName,nrTimes)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   InsertUniqueStringIntoList(nrTimes,&parList,szList);
   InsertUniqueStringIntoList(loopName,&parList,szList);


   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Loop");
   fprintf(fp,"\n        move    x:NR%s,r2                 ; Load number repeats into r2",nrTimes+1);
   fprintf(fp,"\n        do      r2,LOOP%s                 ; Repeat code until Loop end",loopName+1);

   fclose(fp);

   return(OK);
}

/**********************************************************************
     Generate the last statement of a loop 

     endloop(name)

**********************************************************************/

short LoopEnd(DLLParameters *par, char *args)
{
   short nrArgs;
   char loopName[10];
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,1,"loop name","e","s",loopName)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   InsertUniqueStringIntoList(loopName,&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   };

   fprintf(fp,"\n;");
   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n; Loop end");
   fprintf(fp,"\nLOOP%s  nop                ; Identifies end of loop",loopName+1);

   fclose(fp);

   return(OK);
}

/**********************************************************************
     Clear the data memory 

     cleardata(number of complex points to clear)

**********************************************************************/

short ClearData(DLLParameters *par, char *args)
{
   short nrArgs;
   char nrPnts[10];
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,1,"number","e","s",nrPnts)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   InsertUniqueStringIntoList(nrPnts,&parList,szList);

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
   fprintf(fp,"\n        move    #$10000,r4              ; Make r4 point to the start of fid memory");
   fprintf(fp,"\n        move    x:NR%s,r2                ; Zero NR%s points",nrPnts+1,nrPnts+1);
   fprintf(fp,"\n        clr     a");
   fprintf(fp,"\n        do      r2,clearm");
   fprintf(fp,"\n        move    a1,y:(r4)+");
   fprintf(fp,"\nclearm  nop");
   fprintf(fp,"\n        move    #$10000,r4              ; Make r4 point to the start of fid memory");
   fclose(fp);

   return(OK);
}

/******************************************************************************************
  Acquire data from the Kea transceiver. 
  
  acquire(mode, nr_points, [duration])
  
  Possible modes are:

  overwrite ... data is always written to location y:0x10000
  append ...... data is appended to the last acquired data
  sum ......... data is summed to the previously acquired data
  integrate ... data is summed and stored as one complex point

  The optional duration parameter specifies a minimum period for the acquisition command.

  Last modified: 1/2/07
*******************************************************************************************/

short AcquireData(DLLParameters *par, char *args)
{
   short nrArgs;
   CText nrPnts;
   CText duration;
   CText mode; 
   FILE *fp;

   if((nrArgs = ArgScan(par->itfc,args,2,"mode, nr points","ee","tt",&mode,&nrPnts)) < 0)
      return(nrArgs);

   if(!parList)
   {
      ErrorMessage("Pulse sequence not initialised");
      return(ERR);
   }

   InsertUniqueStringIntoList(nrPnts.Str(),&parList,szList);

   fp = fopen("midCode.asm","a");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   };

   if(mode == "append")
   {
	   fprintf(fp,"\n\n;");
	   fprintf(fp,"\n;***************************************************************************");
	   fprintf(fp,"\n; Sample FID appending data memory");
	   fprintf(fp,"\n;");     
      fprintf(fp,"\n        move    x:$20000,a");
      fprintf(fp,"\n        move    #$08,r1                 ; Acquire mode");
      fprintf(fp,"\n        move    x:NR%s,x0               ; Get number of samples",nrPnts.Str()+1);
      fprintf(fp,"\n        move    x:DECRATE,y0            ; Decimation rate");
      fprintf(fp,"\n        mpy     x0,y0,a                 ; Total number of samples to take");
      fprintf(fp,"\n        move    a0,a1");
      fprintf(fp,"\n        lsr     #1,a                    ; Convert back to integer format");
      fprintf(fp,"\n        move    a1,r7");
      fprintf(fp,"\n        bsr     wait");
      fclose(fp);
   }
   //else if(mode == "downconvert")
   //{
	  // fprintf(fp,"\n\n;");
	  // fprintf(fp,"\n;***************************************************************************");
	  // fprintf(fp,"\n; Sample FID with down conversion, appending to data memory");
	  // fprintf(fp,"\n;");     
   //   fprintf(fp,"\n        move    x:$20000,a");
   //   fprintf(fp,"\n        move    #$10,r1                  ; Acquire with down conversion mode");
   //   fprintf(fp,"\n        move    x:NR%s,x0                ; Get number of sample",nrPnts.Str()+1);
   //   fprintf(fp,"\n        move    x:DECRATE,y0             ; Decimation rate");
   //   fprintf(fp,"\n        mpy     x0,y0,a                  ; Total number of samples to take");
   //   fprintf(fp,"\n        move    a0,a1");
   //   fprintf(fp,"\n        lsr     #1,a                     ; Divide by 2");
   //   fprintf(fp,"\n        move    a1,r7");
   //   fprintf(fp,"\n        bsr     wait");
   //   fclose(fp);
   //}
   else if(mode == "sum")
   {
	   fprintf(fp,"\n\n;");
	   fprintf(fp,"\n;***************************************************************************");
	   fprintf(fp,"\n; Sample FID suming to existing data memory");
	   fprintf(fp,"\n;");     
      fprintf(fp,"\n        move    x:$20000,a");
      fprintf(fp,"\n        move    #$10000,r4              ; Reset data memory pointer");
      fprintf(fp,"\n        move    #$08,r1                 ; Acquire mode");
      fprintf(fp,"\n        move    x:NR%s,x0                ; Get number of samples",nrPnts.Str()+1);
      fprintf(fp,"\n        move    x:DECRATE,y0             ; Decimation rate");
      fprintf(fp,"\n        mpy     x0,y0,a                  ; Total number of samples to take");
      fprintf(fp,"\n        move    a0,a1");
      fprintf(fp,"\n        lsr     #1,a                    ; Divide by 2");
      fprintf(fp,"\n        move    a1,r7");
      fprintf(fp,"\n        bsr     wait");
      fclose(fp);
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

short ExecuteAndWait(DLLParameters *par, char *args)
{
   char cmdline[1000];
   char file[1000];
   char arguments[1000] = "";
   short nrArgs;

   if((nrArgs = ArgScan(par->itfc,args,1,"file,arguments","ee","ss",file,arguments)) < 0)
      return(nrArgs);

   sprintf(cmdline,"%s %s",file,arguments);
   startupinfo.cb = sizeof(STARTUPINFO);
   bool st = CreateProcess(file,cmdline,NULL,NULL,false,NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,	NULL,NULL,&startupinfo,&processinfo);
   WaitForSingleObject(processinfo.hProcess, INFINITE);
   CloseHandle(processinfo.hProcess);

   return(OK);

}

/**********************************************************************
     End the pulse program generation process by adding the parameter 
     block and joining all code segments together.
**********************************************************************/


short EndPP(DLLParameters *par, char *args)
{
   char *text;
   FILE *fp;
   char varName[50];
   long sz,len;


// Write the header file
   fp = fopen("temp.asm","w");
   if(!fp)
   {
      ErrorMessage("Can't open output file");
      return(ERR);
   }

   fprintf(fp,"\n;***************************************************************************");
   fprintf(fp,"\n      nolist");
   fprintf(fp,"\n      include  'ioequ.asm'");
   fprintf(fp,"\n      list");
   fprintf(fp,"\n;***************************************************************************\n");


   fprintf(fp,"\n                org     x:$0\n\n");
   fprintf(fp,"\nPARAM_BASE      equ     *       ; Memory for pulse program parameters");
   fprintf(fp,"\nDOSCA1          ds      1       ; 00 - Osc A default coeff a");
   fprintf(fp,"\nDOSCA2          ds      1       ; 01 - Osc A default s11");
   fprintf(fp,"\nDOSCA3          ds      1       ; 02 - Osc A default s2");
   fprintf(fp,"\nDOSCB1          ds      1       ; 03 - Osc B default coeff a");
   fprintf(fp,"\nDOSCB2          ds      1       ; 04 - Osc B default s1");
   fprintf(fp,"\nDOSCB3          ds      1       ; 05 - Osc B default s21");
   fprintf(fp,"\nCLKRATE         ds      1       ; 06 - Sample clock rate");
   fprintf(fp,"\nDECRATE         ds      1       ; 07 - Decimation");
   fprintf(fp,"\nRGAIN           ds      1       ; 08 - Receiver gain");
   fprintf(fp,"\nTUNE            ds      1       ; 09 - Tuning cap value");
   fprintf(fp,"\nPZDLY1          ds      1       ; 10 - Delay following PZ relay switch on");
   fprintf(fp,"\nPZDLY2          ds      1       ; 11 - Delay after PZ input gate switch off");
   fprintf(fp,"\nPZDLY3          ds      1       ; 12 - Delay following PZ relay switch off");
   fprintf(fp,"\nPZDLY4          ds      1       ; 13 - Delay following PZ short switch on");

// Loop over variable list extracting variable names
   short c = 14;
   for(short i = 0; i < szList; i++)
   {
	   strcpy(varName,parList[i]);
 
      if(varName[0] == 'd')
      {
         fprintf(fp,"\nDELAY%s          ds      1       ; %hd - Delay %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'n')
      {
         fprintf(fp,"\nNR%s             ds      1       ; %hd - Number %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'b')
      {
         fprintf(fp,"\nTTL%s            ds      1       ; %hd - Byte %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'a')
      {
         fprintf(fp,"\nRFAMP%s          ds      1       ; %hd - Rf amplitude %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'c')
      {
         fprintf(fp,"\nPOLZI%s          ds      1       ; %hd - Polarizing current %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'p')
      {
         fprintf(fp,"\nPHASE%s          ds      1       ; %hd - Phase %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 'g')
      {
         fprintf(fp,"\nGRAD%s           ds      1       ; %hd - Gradient amplitude %s",varName+1,c++,varName+1);
      }
      else if(varName[0] == 't')
      {
         fprintf(fp,"\nTABLE%s          ds      1       ; %hd - Table %s",varName+1,c++,varName+1);
      }
   }
   fprintf(fp,"\n\n");
   fclose(fp);

// Readin start code

   fp = fopen("startCode.asm","r");
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

/**********************************************************************
     Generate a shaped gradient pulse
**********************************************************************/

short ShapedGradientPulse(DLLParameters *par, char *args)
{
   short nrArgs;
   CArg carg;
   
   nrArgs = carg.Count(args);

   if(nrArgs == 4) // Control 1 gradient
   {
      CText grad,atable,size,duration;

      if((nrArgs = ArgScan(par->itfc,args,4,"adrs, atable, table_size, table_step","eeee","tttt",&grad,&atable,&size,&duration)) < 0)
         return(nrArgs);

      if(!parList)
      {
         ErrorMessage("Pulse sequence not initialised");
         return(ERR);
      }
      InsertUniqueStringIntoList(grad.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable.Str(),&parList,szList);
      InsertUniqueStringIntoList(size.Str(),&parList,szList);
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

      FILE *fp = fopen("midCode.asm","a");
      if(!fp)
      {
         ErrorMessage("Can't open output file");
         return(ERR);
      }

      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Generate a shaped gradient");

      //fprintf(fp,"\n        clr b");
      //fprintf(fp,"\n        move    x:NR%s,b1",grad.Str()+1);
      //fprintf(fp,"\n        cmp     #01,b");
      //fprintf(fp,"\n        bne     LBL%ld",label);
      //fprintf(fp,"\n        move    #02,a1");
      //fprintf(fp,"\n        bra     LBL%ld",label+2);

      //fprintf(fp,"\nLBL%ld    cmp     #02,b",label);
      //fprintf(fp,"\n        bne     LBL%ld",label+1);
      //fprintf(fp,"\n        move    #06,a1");
      //fprintf(fp,"\n        bra     LBL%ld",label+2);

      //fprintf(fp,"\nLBL%ld    cmp     #03,b",label+1);
      //fprintf(fp,"\n        bne     ABORT");
      //fprintf(fp,"\n        move    #01,a1");

      //fprintf(fp,"\nLBL%ld    movep   a1,x:A_PDRE",label+2);

      //fprintf(fp,"\n        move    x:TABLE%s,r3",atable.Str()+1);
      //fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);

      //fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label+3);

      //fprintf(fp,"\n        move    y:(r3)+,a1              ; Load next amplitude"); 
      //fprintf(fp,"\n        lsl     #8,a");    
      //fprintf(fp,"\n        movep   a1,x:A_TX00             ; Set up gradient level");

      //fprintf(fp,"\n; Delay");
      //fprintf(fp,"\n        move    x:DELAY%s,r7",duration.Str()+1);
      //fprintf(fp,"\n        bsr     wait");

      //fprintf(fp,"\nLBL%ld  nop",label+3);

      //fclose(fp);

      //label += 4;

      fprintf(fp,"\n        move    x:TABLE%s,r3",atable.Str()+1);
      fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);

      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      fprintf(fp,"\n        move    x:NR%s,a1",grad.Str()+1);
      fprintf(fp,"\n        bsr     gradAdrs");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:(r3)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX00             ; Set up gradient level");

      fprintf(fp,"\n; Delay");
      fprintf(fp,"\n        move    x:DELAY%s,r7",duration.Str()+1);
      fprintf(fp,"\n        bsr     wait");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fclose(fp);

      return(OK);
   }
   else if(nrArgs == 6) // Control 2 gradients
   {
      CText grad1,atable1,grad2,atable2,size,duration;

      if((nrArgs = ArgScan(par->itfc,args,6,"adrs1, atable1, adrs2, atable2, table_size, table_step","eeeeee","tttttt",
                           &grad1,&atable1,&grad2,&atable2,&size,&duration)) < 0)
         return(nrArgs);

      if(!parList)
      {
         ErrorMessage("Pulse sequence not initialised");
         return(ERR);
      }
      InsertUniqueStringIntoList(grad1.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable1.Str(),&parList,szList);
      InsertUniqueStringIntoList(grad2.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable2.Str(),&parList,szList);
      InsertUniqueStringIntoList(size.Str(),&parList,szList);
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

      FILE *fp = fopen("midCode.asm","a");
      if(!fp)
      {
         ErrorMessage("Can't open output file");
         return(ERR);
      }

      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Generate two shaped gradients");

      fprintf(fp,"\n        move    x:TABLE%s,a1",atable1.Str()+1);
      fprintf(fp,"\n        move    a1,y:mem1");

      fprintf(fp,"\n        move    x:TABLE%s,a1",atable2.Str()+1);
      fprintf(fp,"\n        move    a1,y:mem2");

      fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);

      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      fprintf(fp,"\n        move    x:NR%s,a1",grad1.Str()+1);
      fprintf(fp,"\n        bsr     gradAdrs");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:mem1,r3"); 
      fprintf(fp,"\n        move    y:(r3)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        move    r3,y:mem1"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX00             ; Set up gradient level");

      fprintf(fp,"\n        move    #1,r7");
      fprintf(fp,"\n        bsr     wait");

      fprintf(fp,"\n        move    x:NR%s,a1",grad2.Str()+1);
      fprintf(fp,"\n        bsr     gradAdrs");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:mem2,r3"); 
      fprintf(fp,"\n        move    y:(r3)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        move    r3,y:mem2");
      fprintf(fp,"\n        lsl     #8,a");  
      fprintf(fp,"\n        movep   a1,x:A_TX00             ; Set up gradient level");

      fprintf(fp,"\n; Delay");
      fprintf(fp,"\n        move    x:DELAY%s,r7",duration.Str()+1);
      fprintf(fp,"\n        bsr     wait");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fclose(fp);


      return(OK);
   }
   else if(nrArgs == 8) // Control 3 gradients
   {
      CText grad1,atable1,grad2,atable2,grad3,atable3,size,duration;

      if((nrArgs = ArgScan(par->itfc,args,8,"adrs1, atable1, adrs2, atable2, adrs3, atable3, table_size, table_step","eeeeeeee","tttttttt",
                           &grad1,&atable1,&grad2,&atable2,&grad3,&atable3,&size,&duration)) < 0)
         return(nrArgs);

      if(!parList)
      {
         ErrorMessage("Pulse sequence not initialised");
         return(ERR);
      }
      InsertUniqueStringIntoList(grad1.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable1.Str(),&parList,szList);
      InsertUniqueStringIntoList(grad2.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable2.Str(),&parList,szList);
      InsertUniqueStringIntoList(grad3.Str(),&parList,szList);
      InsertUniqueStringIntoList(atable3.Str(),&parList,szList);
      InsertUniqueStringIntoList(size.Str(),&parList,szList);
      InsertUniqueStringIntoList(duration.Str(),&parList,szList);

      FILE *fp = fopen("midCode.asm","a");
      if(!fp)
      {
         ErrorMessage("Can't open output file");
         return(ERR);
      }

      fprintf(fp,"\n;");
      fprintf(fp,"\n;***************************************************************************");
      fprintf(fp,"\n; Generate three shaped gradients");

      fprintf(fp,"\n        move    x:TABLE%s,a1",atable1.Str()+1);
      fprintf(fp,"\n        move    a1,y:mem1");

      fprintf(fp,"\n        move    x:TABLE%s,a1",atable2.Str()+1);
      fprintf(fp,"\n        move    a1,y:mem2");

      fprintf(fp,"\n        move    x:TABLE%s,a1",atable3.Str()+1);
      fprintf(fp,"\n        move    a1,y:mem3");

      fprintf(fp,"\n        move    x:NR%s,r2",size.Str()+1);

      fprintf(fp,"\n        do      r2,LBL%ld               ; Step the amplitude r2 times",label);

      fprintf(fp,"\n        move    x:NR%s,a1",grad1.Str()+1);
      fprintf(fp,"\n        bsr     gradAdrs");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:mem1,r3"); 
      fprintf(fp,"\n        move    y:(r3)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        move    r3,y:mem1"); 
      fprintf(fp,"\n        lsl     #8,a");    
      fprintf(fp,"\n        movep   a1,x:A_TX00             ; Set up gradient level");

      fprintf(fp,"\n        move    #1,r7");
      fprintf(fp,"\n        bsr     wait");

      fprintf(fp,"\n        move    x:NR%s,a1",grad2.Str()+1);
      fprintf(fp,"\n        bsr     gradAdrs");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:mem2,r3"); 
      fprintf(fp,"\n        move    y:(r3)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        move    r3,y:mem2");
      fprintf(fp,"\n        lsl     #8,a");  
      fprintf(fp,"\n        movep   a1,x:A_TX00             ; Set up gradient level");

      fprintf(fp,"\n        move    #1,r7");
      fprintf(fp,"\n        bsr     wait");

      fprintf(fp,"\n        move    x:NR%s,a1",grad3.Str()+1);
      fprintf(fp,"\n        bsr     gradAdrs");
      fprintf(fp,"\n        movep   a1,x:A_PDRE");
      fprintf(fp,"\n        move    y:mem3,r3"); 
      fprintf(fp,"\n        move    y:(r3)+,a1              ; Load next amplitude"); 
      fprintf(fp,"\n        move    r3,y:mem3");
      fprintf(fp,"\n        lsl     #8,a");  
      fprintf(fp,"\n        movep   a1,x:A_TX00             ; Set up gradient level");

      fprintf(fp,"\n; Delay");
      fprintf(fp,"\n        move    x:DELAY%s,r7",duration.Str()+1);
      fprintf(fp,"\n        bsr     wait");

      fprintf(fp,"\nLBL%ld  nop",label++);

      fclose(fp);


      return(OK);
   }
}