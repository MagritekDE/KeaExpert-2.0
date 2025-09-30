/*******************************************************/
/* file: ports.c                                       */
/* abstract:  This file contains the routines to       */
/*            output values on the JTAG ports, to read */
/*            the TDO bit, and to read a byte of data  */
/*            from the prom                            */
/* Revisions:                                          */
/* 12/01/2008:  Same code as before (original v5.01).  */
/*              Updated comments to clarify instructions.*/
/*              Add print in setPort for xapp058_example.exe.*/
/*******************************************************/
#include "XSVF/ports.h"
#include "lib/FPGA.h"
#include "lib/DSPio.h"
#include "lib/pulse_parameters.h"
#include <math.h>
/*#include "prgispx.h"*/

int  g_JTAG;
unsigned int  ByteDA;
unsigned int  addresscount;
unsigned char * g_ydataAddress;


/* toggle tck LH.  No need to modify this code.  It is output via setPort. */
void pulseClock()
{
    setPort(TCK,0);  /* set the TCK port to low  */
    setPort(TCK,1);  /* set the TCK port to high */
}

/* setreadByteAddress:  Implement to source the next byte from your XSVF file location */
void setreadByteAddress(unsigned char addr)
{
    g_JTAG = 0;
    ByteDA = 0;
    addresscount = 0;
    g_ydataAddress = (unsigned char *) addr;
}

/* getreadByteAddress:  Implement to source the next byte from your XSVF file location */
unsigned char * getreadByteAddress()
{
    return (unsigned char *)addresscount;
}
/* readByte:  Implement to source the next byte from your XSVF file location */
/* read in a byte of data from the prom */
void readByte(unsigned char *data)
{
    *data   = rymem( g_ydataAddress);
    switch(ByteDA)
    {
	    case 0:
	    	*data = *data >> 16;
	    	ByteDA += 1;
	    	break;
	    case 1:
	    	*data = *data >> 8;
	    	ByteDA += 1;
	    	break;
	    default:
	    	g_ydataAddress += 1;
	    	ByteDA = 0;
	    	break;
    }
    *data = *data & 0x0000ff;
    addresscount += 1;
}

/* readTDOBit:  Implement to return the current value of the JTAG TDO signal.*/
/* read the TDO bit from port */
/*unsigned char readTDOBit()
{
    unsigned char d1;
    d1 = *FPGA_JTAG_RD;
    __asm("nop");
    __asm("nop");
    return d1;
}*/ 

/* waitTime:  Implement as follows: */
/* REQUIRED:  This function must consume/wait at least the specified number  */
/*            of microsec, interpreting microsec as a number of microseconds.*/
/* REQUIRED FOR SPARTAN/VIRTEX FPGAs and indirect flash programming:         */
/*            This function must pulse TCK for at least microsec times,      */
/*            interpreting microsec as an integer value.                     */
/* RECOMMENDED IMPLEMENTATION:  Pulse TCK at least microsec times AND        */
/*                              continue pulsing TCK until the microsec wait */
/*                              requirement is also satisfied.               */
void waitTime(long microsec)
{
    long        tckCycles;
    long        i;
	tckCycles = microsec;  
    /* This implementation is highly recommended!!! */
    /* This implementation requires you to tune the tckCyclesPerMicrosec 
       variable (above) to match the performance of your embedded system
       in order to satisfy the microsec wait time requirement. */
    for ( i = 0; i < tckCycles; ++i )
    {
        setPort(TCK,0);  /* set the TCK port to low  */
    	setPort(TCK,1);  /* set the TCK port to high */
    }
}
