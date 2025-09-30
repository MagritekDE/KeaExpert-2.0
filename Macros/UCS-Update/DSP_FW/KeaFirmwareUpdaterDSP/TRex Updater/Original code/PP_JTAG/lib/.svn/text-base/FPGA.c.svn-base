#include "lib/FPGA.h"

void Write_FPGA(volatile unsigned char * Reg, unsigned char d1)
{
	*Reg = d1;
	__asm("nop");	
	__asm("nop");	
	return;
}

unsigned char Read_FPGA(volatile unsigned char * Reg)
{
	unsigned char d1;
	d1 = *Reg;
	__asm("nop");	
	__asm("nop");	
	return d1;
}