/**********************************************************************************
 *
 *     DEFINES for FPGA Transceiver I/O registers and ports
 *
 *     Created by Matthew van der Werff 2009
 *     Beta Solutions
 *
 ********************************************************************************** */
void Write_FPGA(volatile unsigned char * Reg, unsigned char d1);
unsigned char Read_FPGA(volatile unsigned char * Reg);

/*------------------------------------------------------------------------
 *
 *       EQUATES for FPGA Transceiver Control
 *
 *------------------------------------------------------------------------ */
 
 /*------------------------------------------------------------------------
 *
 *       EQUATES Legacy Registers
 *
 *------------------------------------------------------------------------ */
 
 #define FPGA_TTL   		(volatile unsigned char*)0x020000 /* FPGA TTL Control Register */
 #define FPGA_Sync   	 	(volatile unsigned char*)0x020080 /* FPGA Sync Register */
 #define FPGA_SampleA   	(volatile unsigned char*)0x020080 /* FPGA SampleA Register */
 #define FPGA_SampleB   	(volatile unsigned char*)0x020081 /* FPGA SampleB Register */
 #define FPGA_HardVer  	    (volatile unsigned char*)0x020082 /* FPGA Hardware Version */
 #define FPGA_SoftBuild 	(volatile unsigned char*)0x020083 /* FPGA Software Build Number */
 
 /*------------------------------------------------------------------------
 * JTAG Control Registers
 *
 * 
 *------------------------------------------------------------------------ */
 
 #define FPGA_JTAG_Con    	(volatile unsigned char*)0x027080 /* FPGA JTAG Control Register */
 #define FPGA_JTAG_WR    	(volatile unsigned char*)0x027081 /* FPGA JTAG Write Register */
 #define FPGA_JTAG_RD_Buff	(volatile unsigned char*)0x027082 /* FPGA JTAG Read Register */
 #define FPGA_JTAG_RD  		(volatile unsigned char*)0x027083 /* FPGA JTAG Read Register */
 
 /*       JTAG_Con   bits definition */
 #define FPGA_JTAGEN  (0x0)   /* Enable the JTAG port */

 /*       JTAG_Wr   bits definition */
 #define FPGA_JTAG_TCK  (0x0)   /* JTAG TCK */
 #define FPGA_JTAG_TDO  (0x1)   /* JTAG TCK */
 #define FPGA_JTAG_TMS  (0x2)   /* JTAG TCK */

 /*       JTAG_RD   bits definition */
 #define FPGA_JTAG_TDI  (23)   /* JTAG TCK */

 