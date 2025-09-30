#include "lib/ioe56303.h"
#include "lib/DSP.h"

void setupDSP(void)
{
	*M_BCR = 0x0F9FE1; /*  Set up wait states, 4 for AA3 */
	__asm("nop");	
	__asm("nop");
	__asm("nop");	
	__asm("nop");
	return;
} 

unsigned char yRead(unsigned char * addr)
{
	unsigned char data;
	unsigned char * a1;
	a1 = addr;
	data = *a1;
	__asm("
	move r0,x:(r6)+  	\n
	move %1,r0			\n	
	move y:(r0),%0		\n
	nop					\n
	move x:-(r6),r0		\n
	" : "=S" (data) : "A" (a1));
	return data;
}

/*__asm("
	global FDSPread
	FDSPread ; sets up entry point (C function address).
		move ssh,y:(r6)+ ; save the return address.

		; Save all registers to be used
		move r0,x:(r6)+ 
		; Function body.
		move y:(r6-2),r0
		clr	 a
		move y:(r0),a
		; Restore all registers used
		move x:-(r6),r0
		tst a
		move y:(r6),ssh ; get the return address.
		rts
		");
	*/		