/*******************************************************/
/* file: ports.h                                       */
/* abstract:  This file contains extern declarations   */
/*            for providing stimulus to the JTAG ports.*/
/*******************************************************/

#ifndef ports_dot_h
#define ports_dot_h

/* these constants are used to send the appropriate ports to setPort */
/* they should be enumerated types, but some of the microcontroller  */
/* compilers don't like enumerated types */


/* make clock go down->up->down*/
void pulseClock();

void setreadByteAddress(unsigned char addr);
/* read the next byte of data from the xsvf file */

void readByte(unsigned char *data);

void waitTime(long microsec);

unsigned char * getreadByteAddress();

#endif
