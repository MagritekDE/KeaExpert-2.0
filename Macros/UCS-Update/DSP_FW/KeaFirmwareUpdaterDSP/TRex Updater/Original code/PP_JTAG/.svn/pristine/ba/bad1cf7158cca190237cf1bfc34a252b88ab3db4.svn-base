/**********************************************************************************
 *
 *     DEFINES for 56302/3 I/O registers and ports
 *
 *     Derived from .asm file (Last .asm update: June 11 1995).
 *
 *     Modified by Robin Dykstra 2000
 *
 ********************************************************************************** */


/*------------------------------------------------------------------------
 *
 *       EQUATES for I/O Port Programming
 *
 *------------------------------------------------------------------------ */

/*       Register Addresses */

#define M_HDR   (char*)0xFFFFC9 /* Host port GPIO data Register */
#define M_HDDR  (char*)0xFFFFC8 /* Host port GPIO direction Register */
#define M_PCRC  (char*)0xFFFFBF /* Port C Control Register */
#define M_PRRC  (char*)0xFFFFBE /* Port C Direction Register */
#define M_PDRC  (char*)0xFFFFBD /* Port C GPIO Data Register */
#define M_PCRD  (char*)0xFFFFAF /* Port D Control register */
#define M_PRRD  (char*)0xFFFFAE /* Port D Direction Data Register */
#define M_PDRD  (char*)0xFFFFAD /* Port D GPIO Data Register */
#define M_PCRE  (char*)0xFFFF9F /* Port E Control register */
#define M_PRRE  (char*)0xFFFF9E /* Port E Direction Register */
#define M_PDRE  (char*)0xFFFF9D /* Port E Data Register */
#define M_OGDB  (char*)0xFFFFFC /* OnCE GDB Register  */


/*------------------------------------------------------------------------
 *
 *       EQUATES for Host Interface
 *
 *------------------------------------------------------------------------ */

/*       Register Addresses */

#define M_HCR   (char*)0xFFFFC2 /* Host Control Register */
#define M_HSR   (char*)0xFFFFC3 /* Host Status Rgister */
#define M_HPCR  (char*)0xFFFFC4 /* Host Polarity Control Register */
#define M_HBAR  (char*)0xFFFFC5 /* Host Base Address Register */
#define M_HRX   (char*)0xFFFFC6 /* Host Receive Register */
#define M_HTX   (char*)0xFFFFC7 /* Host Transmit Register */

/*       HCR bits definition */
#define M_HRIE  (0x0)   /* Host Receive interrupts Enable */
#define M_HTIE  (0x1)   /* Host Transmit Interrupt Enable */
#define M_HCIE  (0x2)   /* Host Command Interrupt Enable */
#define M_HF2   (0x3)   /* Host Flag 2 */
#define M_HF3   (0x4)   /* Host Flag 3 */

/*       HSR bits definition */
#define M_HRDF  (0x0)   /* Host Receive Data Full */
#define M_HTDE  (0x1)   /* Host Receive Data Emptiy */
#define M_HCP   (0x2)   /* Host Command Pending */
#define M_HF0   (0x3)   /* Host Flag 0 */
#define M_HF1   (0x4)   /* Host Flag 1 */


/*       HPCR bits definition */
#define M_HGEN  (0x0)   /* Host Port GPIO Enable */
#define M_HA8EN (0x1)   /* Host Address 8 Enable */
#define M_HA9EN (0x2)   /* Host Address 9 Enable */
#define M_HCSEN (0x3)   /* Host Chip Select Enable */
#define M_HREN  (0x4)   /* Host Request Enable */
#define M_HAEN  (0x5)   /* Host Acknowledge Enable */
#define M_HEN   (0x6)   /* Host Enable */
#define M_HOD   (0x8)   /* Host Request Open Drain mode */
#define M_HDSP  (0x9)   /* Host Data Strobe Polarity */
#define M_HASP  (0xA)   /* Host Address Strobe Polarity */
#define M_HMUX  (0xB)   /* Host Multiplexed bus select */
#define M_HD_HS (0xC)   /* Host Double/Single Strobe select */
#define M_HCSP  (0xD)   /* Host Chip Select Polarity */
#define M_HRP   (0xE)   /* Host Request PolarityPolarity */
#define M_HAP   (0xF)   /* Host Acknowledge Polarity */


/*------------------------------------------------------------------------
 *
 *       EQUATES for Serial Communications Interface (SCI)
 *
 *------------------------------------------------------------------------ */

/*       Register Addresses */

#define M_STXH  (char*)0xFFFF97 /* SCI Transmit Data Register (high) */
#define M_STXM  (char*)0xFFFF96 /* SCI Transmit Data Register (middle) */
#define M_STXL  (char*)0xFFFF95 /* SCI Transmit Data Register (low) */
#define M_SRXH  (char*)0xFFFF9A /* SCI Receive Data Register (high) */
#define M_SRXM  (char*)0xFFFF99 /* SCI Receive Data Register (middle) */
#define M_SRXL  (char*)0xFFFF98 /* SCI Receive Data Register (low) */
#define M_STXA  (char*)0xFFFF94 /* SCI Transmit Address Register */
#define M_SCR   (char*)0xFFFF9C /* SCI Control Register */
#define M_SSR   (char*)0xFFFF93 /* SCI Status Register */
#define M_SCCR  (char*)0xFFFF9B /* SCI Clock Control Register */

/*       SCI Control Register Bit Flags */

#define M_WDS   (0x7)   /* Word Select Mask (WDS0-WDS3) */
#define M_WDS0  (0)     /* Word Select 0 */
#define M_WDS1  (1)     /* Word Select 1 */
#define M_WDS2  (2)     /* Word Select 2 */
#define M_SSFTD (3)     /* SCI Shift Direction  */
#define M_SBK   (4)     /* Send Break */
#define M_WAKE  (5)     /* Wakeup Mode Select */
#define M_RWU   (6)     /* Receiver Wakeup Enable */
#define M_WOMS  (7)     /* Wired-OR Mode Select */
#define M_SCRE  (8)     /* SCI Receiver Enable */
#define M_SCTE  (9)     /* SCI Transmitter Enable */
#define M_ILIE  (10)    /* Idle Line Interrupt Enable */
#define M_SCRIE (11)    /* SCI Receive Interrupt Enable */
#define M_SCTIE (12)    /* SCI Transmit Interrupt Enable */
#define M_TMIE  (13)    /* Timer Interrupt Enable */
#define M_TIR   (14)    /* Timer Interrupt Rate */
#define M_SCKP  (15)    /* SCI Clock Polarity */
#define M_REIE  (16)    /* SCI Error Interrupt Enable (REIE) */

/*       SCI Status Register Bit Flags */

#define M_TRNE  (0)     /* Transmitter Empty */
#define M_TDRE  (1)     /* Transmit Data Register Empty */
#define M_RDRF  (2)     /* Receive Data Register Full */
#define M_IDLE  (3)     /* Idle Line Flag */
#define M_OR    (4)     /* Overrun Error Flag  */
#define M_PE    (5)     /* Parity Error */
#define M_FE    (6)     /* Framing Error Flag */
#define M_R8    (7)     /* Received Bit 8 (R8) Address */

/*       SCI Clock Control Register  */

#define M_CD    (0xFFF) /* Clock Divider Mask (CD0-CD11) */
#define M_COD   (12)    /* Clock Out Divider */
#define M_SCP   (13)    /* Clock Prescaler */
#define M_RCM   (14)    /* Receive Clock Mode Source Bit */
#define M_TCM   (15)    /* Transmit Clock Source Bit */

/*------------------------------------------------------------------------
 *
 *       EQUATES for Synchronous Serial Interface (SSI)
 *
 *------------------------------------------------------------------------ */

/*
 *       Register Addresses Of SSI0  */
#define M_TX00  (char*)0xFFFFBC /* SSI0 Transmit Data Register 0 */
#define M_TX01  (char*)0xFFFFBB /* SSIO Transmit Data Register 1 */
#define M_TX02  (char*)0xFFFFBA /* SSIO Transmit Data Register 2 */
#define M_TSR0  (char*)0xFFFFB9 /* SSI0 Time Slot Register */
#define M_RX0   (char*)0xFFFFB8 /* SSI0 Receive Data Register */
#define M_SSISR0        (char*)0xFFFFB7 /* SSI0 Status Register */
#define M_CRB0  (char*)0xFFFFB6 /* SSI0 Control Register B */
#define M_CRA0  (char*)0xFFFFB5 /* SSI0 Control Register A */
#define M_TSMA0 (char*)0xFFFFB4 /* SSI0 Transmit Slot Mask Register A */
#define M_TSMB0 (char*)0xFFFFB3 /* SSI0 Transmit Slot Mask Register B */
#define M_RSMA0 (char*)0xFFFFB2 /* SSI0 Receive Slot Mask Register A */
#define M_RSMB0 (char*)0xFFFFB1 /* SSI0 Receive Slot Mask Register B */

/*       Register Addresses Of SSI1                                         */
#define M_TX10  (char*)0xFFFFAC /* SSI1 Transmit Data Register 0 */
#define M_TX11  (char*)0xFFFFAB /* SSI1 Transmit Data Register 1 */
#define M_TX12  (char*)0xFFFFAA /* SSI1 Transmit Data Register 2 */
#define M_TSR1  (char*)0xFFFFA9 /* SSI1 Time Slot Register */
#define M_RX1   (char*)0xFFFFA8 /* SSI1 Receive Data Register */
#define M_SSISR1        (char*)0xFFFFA7 /* SSI1 Status Register */
#define M_CRB1  (char*)0xFFFFA6 /* SSI1 Control Register B */
#define M_CRA1  (char*)0xFFFFA5 /* SSI1 Control Register A */
#define M_TSMA1 (char*)0xFFFFA4 /* SSI1 Transmit Slot Mask Register A */
#define M_TSMB1 (char*)0xFFFFA3 /* SSI1 Transmit Slot Mask Register B */
#define M_RSMA1 (char*)0xFFFFA2 /* SSI1 Receive Slot Mask Register A */
#define M_RSMB1 (char*)0xFFFFA1 /* SSI1 Receive Slot Mask Register B */

/*       SSI Control Register A Bit Flags */

#define M_PM    (0xFF)  /* Prescale Modulus Select Mask (PM0-PM7)               */
#define M_PSR   (11)    /* Prescaler Range        */
#define M_DC    (0x1F000)       /* Frame Rate Divider Control Mask (DC0-DC7) */
#define M_ALC   (18)    /* Alignment Control (ALC) */
#define M_WL    (0x380000)      /* Word Length Control Mask (WL0-WL7) */
#define M_SSC1  (22)    /* Select SC1 as TR #0 drive enable (SSC1) */

/*       SSI Control Register B Bit Flags                                    */

#define M_OF    (0x3)   /* Serial Output Flag Mask */
#define M_OF0   (0)     /* Serial Output Flag 0                      */
#define M_OF1   (1)     /* Serial Output Flag 1                      */
#define M_SCD   (0x1C)  /* Serial Control Direction Mask             */
#define M_SCD0  (2)     /* Serial Control 0 Direction                 */
#define M_SCD1  (3)     /* Serial Control 1 Direction                */
#define M_SCD2  (4)     /* Serial Control 2 Direction                */
#define M_SCKD  (5)     /* Clock Source Direction */
#define M_SHFD  (6)     /* Shift Direction                           */
#define M_FSL   (0x180) /* Frame Sync Length Mask (FSL0-FSL1) */
#define M_FSL0  (7)     /* Frame Sync Length 0 */
#define M_FSL1  (8)     /* Frame Sync Length 1 */
#define M_FSR   (9)     /* Frame Sync Relative Timing */
#define M_FSP   (10)    /* Frame Sync Polarity */
#define M_CKP   (11)    /* Clock Polarity                            */
#define M_SYN   (12)    /* Sync/Async Control                        */
#define M_MOD   (13)    /* SSI Mode Select */
#define M_SSTE  (0x1C000)       /* SSI Transmit enable Mask                   */
#define M_SSTE2 (14)    /* SSI Transmit #2 Enable                    */
#define M_SSTE1 (15)    /* SSI Transmit #1 Enable                     */
#define M_SSTE0 (16)    /* SSI Transmit #0 Enable                     */
#define M_SSRE  (17)    /* SSI Receive Enable                        */
#define M_SSTIE (18)    /* SSI Transmit Interrupt Enable             */
#define M_SSRIE (19)    /* SSI Receive Interrupt Enable               */
#define M_STLIE (20)    /* SSI Transmit Last Slot Interrupt Enable  */
#define M_SRLIE (21)    /* SSI Receive Last Slot Interrupt Enable  */
#define M_STEIE (22)    /* SSI Transmit Error Interrupt Enable  */
#define M_SREIE (23)    /* SSI Receive Error Interrupt Enable               */

/*       SSI Status Register Bit Flags                                        */

#define M_IF    (0x3)   /* Serial Input Flag Mask            */
#define M_IF0   (0)     /* Serial Input Flag 0                       */
#define M_IF1   (1)     /* Serial Input Flag 1                       */
#define M_TFS   (2)     /* Transmit Frame Sync Flag                  */
#define M_RFS   (3)     /* Receive Frame Sync Flag                   */
#define M_TUE   (4)     /* Transmitter Underrun Error FLag           */
#define M_ROE   (5)     /* Receiver Overrun Error Flag               */
#define M_TDE   (6)     /* Transmit Data Register Empty              */
#define M_RDF   (7)     /* Receive Data Register Full */

/*       SSI Transmit Slot Mask Register A */

#define M_SSTSA (0xFFFF)        /* SSI Transmit Slot Bits Mask A (TS0-TS15) */

/*       SSI Transmit Slot Mask Register B */

#define M_SSTSB (0xFFFF)        /* SSI Transmit Slot Bits Mask B (TS16-TS31) */

/*       SSI Receive Slot Mask Register A */

#define M_SSRSA (0xFFFF)        /* SSI Receive Slot Bits Mask A (RS0-RS15) */

/*       SSI Receive Slot Mask Register B */

#define M_SSRSB (0xFFFF)        /* SSI Receive Slot Bits Mask B (RS16-RS31) */



/*------------------------------------------------------------------------
 *
 *       EQUATES for Exception Processing
 *
 *------------------------------------------------------------------------ */


/*       Register Addresses */

#define M_IPRC  (char*)0xFFFFFF /* Interrupt Priority Register Core */
#define M_IPRP  (char*)0xFFFFFE /* Interrupt Priority Register Peripheral */

/*       Interrupt Priority Register Core (IPRC)  */

#define M_IAL   (0x7)   /* IRQA Mode Mask */
#define M_IAL0  (0)     /* IRQA Mode Interrupt Priority Level (low) */
#define M_IAL1  (1)     /* IRQA Mode Interrupt Priority Level (high) */
#define M_IAL2  (2)     /* IRQA Mode Trigger Mode */
#define M_IBL   (0x38)  /* IRQB Mode Mask */
#define M_IBL0  (3)     /* IRQB Mode Interrupt Priority Level (low) */
#define M_IBL1  (4)     /* IRQB Mode Interrupt Priority Level (high) */
#define M_IBL2  (5)     /* IRQB Mode Trigger Mode */
#define M_ICL   (0x1C0) /* IRQC Mode Mask */
#define M_ICL0  (6)     /* IRQC Mode Interrupt Priority Level (low) */
#define M_ICL1  (7)     /* IRQC Mode Interrupt Priority Level (high) */
#define M_ICL2  (8)     /* IRQC Mode Trigger Mode */
#define M_IDL   (0xE00) /* IRQD Mode Mask */
#define M_IDL0  (9)     /* IRQD Mode Interrupt Priority Level (low) */
#define M_IDL1  (10)    /* IRQD Mode Interrupt Priority Level (high) */
#define M_IDL2  (11)    /* IRQD Mode Trigger Mode */
#define M_D0L   (0x3000)        /* DMA0 Interrupt priority Level Mask */
#define M_D0L0  (12)    /* DMA0 Interrupt Priority Level (low) */
#define M_D0L1  (13)    /* DMA0 Interrupt Priority Level (high) */
#define M_D1L   (0xC000)        /* DMA1 Interrupt Priority Level Mask */
#define M_D1L0  (14)    /* DMA1 Interrupt Priority Level (low) */
#define M_D1L1  (15)    /* DMA1 Interrupt Priority Level (high) */
#define M_D2L   (0x30000)       /* DMA2 Interrupt priority Level Mask */
#define M_D2L0  (16)    /* DMA2 Interrupt Priority Level (low) */
#define M_D2L1  (17)    /* DMA2 Interrupt Priority Level (high) */
#define M_D3L   (0xC0000)       /* DMA3 Interrupt Priority Level Mask */
#define M_D3L0  (18)    /* DMA3 Interrupt Priority Level (low) */
#define M_D3L1  (19)    /* DMA3 Interrupt Priority Level (high) */
#define M_D4L   (0x300000)      /* DMA4 Interrupt priority Level Mask */
#define M_D4L0  (20)    /* DMA4 Interrupt Priority Level (low) */
#define M_D4L1  (21)    /* DMA4 Interrupt Priority Level (high) */
#define M_D5L   (0xC00000)      /* DMA5 Interrupt priority Level Mask */
#define M_D5L0  (22)    /* DMA5 Interrupt Priority Level (low) */
#define M_D5L1  (23)    /* DMA5 Interrupt Priority Level (high) */


/*       Interrupt Priority Register Peripheral (IPRP) */

#define M_HPL   (0x3)   /* Host Interrupt Priority Level Mask */
#define M_HPL0  (0)     /* Host Interrupt Priority Level (low) */
#define M_HPL1  (1)     /* Host Interrupt Priority Level (high) */
#define M_S0L   (0xC)   /* SSI0 Interrupt Priority Level Mask */
#define M_S0L0  (2)     /* SSI0 Interrupt Priority Level (low) */
#define M_S0L1  (3)     /* SSI0 Interrupt Priority Level (high) */
#define M_S1L   (0x30)  /* SSI1 Interrupt Priority Level Mask */
#define M_S1L0  (4)     /* SSI1 Interrupt Priority Level (low) */
#define M_S1L1  (5)     /* SSI1 Interrupt Priority Level (high) */
#define M_SCL   (0xC0)  /* SCI  Interrupt Priority Level  Mask        */
#define M_SCL0  (6)     /* SCI  Interrupt Priority Level  (low) */
#define M_SCL1  (7)     /* SCI  Interrupt Priority Level  (high) */
#define M_T0L   (0x300) /* TIMER Interrupt Priority Level Mask */
#define M_T0L0  (8)     /* TIMER Interrupt Priority Level (low) */
#define M_T0L1  (9)     /* TIMER Interrupt Priority Level (high) */


/*------------------------------------------------------------------------
 *
 *       EQUATES for TIMER
 *
 *------------------------------------------------------------------------ */

/*       Register Addresses Of TIMER0 */

#define M_TCSR0 (char*)0xFFFF8F /* TIMER0 Control/Status Register             */
#define M_TLR0  (char*)0xFFFF8E /* TIMER0 Load Reg    */
#define M_TCPR0 (char*)0xFFFF8D /* TIMER0 Compare Register */
#define M_TCR0  (char*)0xFFFF8C /* TIMER0 Count Register */

/*       Register Addresses Of TIMER1 */

#define M_TCSR1 (char*)0xFFFF8B /* TIMER1 Control/Status Register             */
#define M_TLR1  (char*)0xFFFF8A /* TIMER1 Load Reg    */
#define M_TCPR1 (char*)0xFFFF89 /* TIMER1 Compare Register */
#define M_TCR1  (char*)0xFFFF88 /* TIMER1 Count Register */


/*       Register Addresses Of TIMER2 */

#define M_TCSR2 (char*)0xFFFF87 /* TIMER2 Control/Status Register             */
#define M_TLR2  (char*)0xFFFF86 /* TIMER2 Load Reg    */
#define M_TCPR2 (char*)0xFFFF85 /* TIMER2 Compare Register */
#define M_TCR2  (char*)0xFFFF84 /* TIMER2 Count Register */
#define M_TPLR  (char*)0xFFFF83 /* TIMER Prescaler Load Register */
#define M_TPCR  (char*)0xFFFF82 /* TIMER Prescalar Count Register */


/*       Timer Control/Status Register Bit Flags  */

#define M_TE    (0)     /* Timer Enable  */
#define M_TOIE  (1)     /* Timer Overflow Interrupt Enable */
#define M_TCIE  (2)     /* Timer Compare Interrupt Enable */
#define M_TC    (0xF0)  /* Timer Control Mask (TC0-TC3) */
#define M_INV   (8)     /* Inverter Bit */
#define M_TRM   (9)     /* Timer Restart Mode  */
#define M_DIR   (11)    /* Direction Bit */
#define M_DI    (12)    /* Data Input */
#define M_DO    (13)    /* Data Output */
#define M_PCE   (15)    /* Prescaled Clock Enable */
#define M_TOF   (20)    /* Timer Overflow Flag */
#define M_TCF   (21)    /* Timer Compare Flag  */

/*       Timer Prescaler Register Bit Flags                                         */

#define M_PS    (0x600000)      /* Prescaler Source Mask */
#define M_PS0   (21)
#define M_PS1   (22)

/*      Timer Control Bits */
#define M_TC0   (4)     /* Timer Control 0 */
#define M_TC1   (5)     /* Timer Control 1 */
#define M_TC2   (6)     /* Timer Control 2 */
#define M_TC3   (7)     /* Timer Control 3 */


/*------------------------------------------------------------------------
 *
 *       EQUATES for Direct Memory Access (DMA)
 *
 *------------------------------------------------------------------------ */

/*       Register Addresses Of DMA */
#define M_DSTR  (char*)0xFFFFF4 /* DMA Status Register */
#define M_DOR0  (char*)0xFFFFF3 /* DMA Offset Register 0 */
#define M_DOR1  (char*)0xFFFFF2 /* DMA Offset Register 1 */
#define M_DOR2  (char*)0xFFFFF1 /* DMA Offset Register 2 */
#define M_DOR3  (char*)0xFFFFF0 /* DMA Offset Register 3 */


/*       Register Addresses Of DMA0 */

#define M_DSR0  (char*)0xFFFFEF /* DMA0 Source Address Register */
#define M_DDR0  (char*)0xFFFFEE /* DMA0 Destination Address Register  */
#define M_DCO0  (char*)0xFFFFED /* DMA0 Counter */
#define M_DCR0  (char*)0xFFFFEC /* DMA0 Control Register  */

/*       Register Addresses Of DMA1 */

#define M_DSR1  (char*)0xFFFFEB /* DMA1 Source Address Register */
#define M_DDR1  (char*)0xFFFFEA /* DMA1 Destination Address Register  */
#define M_DCO1  (char*)0xFFFFE9 /* DMA1 Counter */
#define M_DCR1  (char*)0xFFFFE8 /* DMA1 Control Register */

/*       Register Addresses Of DMA2 */

#define M_DSR2  (char*)0xFFFFE7 /* DMA2 Source Address Register */
#define M_DDR2  (char*)0xFFFFE6 /* DMA2 Destination Address Register  */
#define M_DCO2  (char*)0xFFFFE5 /* DMA2 Counter */
#define M_DCR2  (char*)0xFFFFE4 /* DMA2 Control Register */

/*       Register Addresses Of DMA4 */

#define M_DSR3  (char*)0xFFFFE3 /* DMA3 Source Address Register */
#define M_DDR3  (char*)0xFFFFE2 /* DMA3 Destination Address Register  */
#define M_DCO3  (char*)0xFFFFE1 /* DMA3 Counter */
#define M_DCR3  (char*)0xFFFFE0 /* DMA3 Control Register */

/*       Register Addresses Of DMA4 */


#define M_DSR4  (char*)0xFFFFDF /* DMA4 Source Address Register */
#define M_DDR4  (char*)0xFFFFDE /* DMA4 Destination Address Register  */
#define M_DCO4  (char*)0xFFFFDD /* DMA4 Counter */
#define M_DCR4  (char*)0xFFFFDC /* DMA4 Control Register  */

/*       Register Addresses Of DMA5 */

#define M_DSR5  (char*)0xFFFFDB /* DMA5 Source Address Register */
#define M_DDR5  (char*)0xFFFFDA /* DMA5 Destination Address Register  */
#define M_DCO5  (char*)0xFFFFD9 /* DMA5 Counter */
#define M_DCR5  (char*)0xFFFFD8 /* DMA5 Control Register */

/*      DMA Control Register */

#define M_DSS   (0x3)   /* DMA Source Space Mask (DSS0-Dss1) */
#define M_DSS0  (0)     /* DMA Source Memory space 0 */
#define M_DSS1  (1)     /* DMA Source Memory space 1     */
#define M_DDS   (0xC)   /* DMA Destination Space Mask (DDS-DDS1) */
#define M_DDS0  (2)     /* DMA Destination Memory Space 0 */
#define M_DDS1  (3)     /* DMA Destination Memory Space 1 */
#define M_DAM   (0x3f0) /* DMA Address Mode Mask (DAM5-DAM0) */
#define M_DAM0  (4)     /* DMA Address Mode 0 */
#define M_DAM1  (5)     /* DMA Address Mode 1 */
#define M_DAM2  (6)     /* DMA Address Mode 2 */
#define M_DAM3  (7)     /* DMA Address Mode 3 */
#define M_DAM4  (8)     /* DMA Address Mode 4 */
#define M_DAM5  (9)     /* DMA Address Mode 5 */
#define M_D3D   (10)    /* DMA Three Dimensional Mode */
#define M_DRS   (0xF800)        /* DMA Request Source Mask (DRS0-DRS4) */
#define M_DCON  (16)    /* DMA Continuous Mode */
#define M_DPR   (0x60000)       /* DMA Channel Priority */
#define M_DPR0  (17)    /* DMA Channel Priority Level (low) */
#define M_DPR1  (18)    /* DMA Channel Priority Level (high) */
#define M_DTM   (0x380000)      /* DMA Transfer Mode Mask (DTM2-DTM0) */
#define M_DTM0  (19)    /* DMA Transfer Mode 0 */
#define M_DTM1  (20)    /* DMA Transfer Mode 1 */
#define M_DTM2  (21)    /* DMA Transfer Mode 2 */
#define M_DIE   (22)    /* DMA Interrupt Enable bit */
#define M_DE    (23)    /* DMA Channel Enable bit  */

/*       DMA Status Register */

#define M_DTD   (0x3F)  /* Channel Transfer Done Status MASK (DTD0-DTD5) */
#define M_DTD0  (0)     /* DMA Channel Transfer Done Status 0 */
#define M_DTD1  (1)     /* DMA Channel Transfer Done Status 1 */
#define M_DTD2  (2)     /* DMA Channel Transfer Done Status 2 */
#define M_DTD3  (3)     /* DMA Channel Transfer Done Status 3 */
#define M_DTD4  (4)     /* DMA Channel Transfer Done Status 4 */
#define M_DTD5  (5)     /* DMA Channel Transfer Done Status 5 */
#define M_DACT  (8)     /* DMA Active State */
#define M_DCH   (0xE00) /* DMA Active Channel Mask (DCH0-DCH2) */
#define M_DCH0  (9)     /* DMA Active Channel 0 */
#define M_DCH1  (10)    /* DMA Active Channel 1 */
#define M_DCH2  (11)    /* DMA Active Channel 2 */


/*------------------------------------------------------------------------
 *
 *       EQUATES for Phase Locked Loop (PLL)
 *
 *------------------------------------------------------------------------ */

/*       Register Addresses Of PLL */

#define M_PCTL  (char*)0xFFFFFD /* PLL Control Register */

/*       PLL Control Register */

#define M_MF    (0xFFF) /* Multiplication Factor Bits Mask (MF0-MF11) */
#define M_DF    (0x7000)        /* Division Factor Bits Mask (DF0-DF2) */
#define M_XTLR  (15)    /* XTAL Range select bit */
#define M_XTLD  (16)    /* XTAL Disable Bit */
#define M_PSTP  (17)    /* STOP Processing State Bit  */
#define M_PEN   (18)    /* PLL Enable Bit */
#define M_PCOD  (19)    /* PLL Clock Output Disable Bit */
#define M_PD    (0xF00000)      /* PreDivider Factor Bits Mask (PD0-PD3) */


/*------------------------------------------------------------------------
 *
 *       EQUATES for BIU
 *
 *------------------------------------------------------------------------ */

/*       Register Addresses Of BIU */


#define M_BCR   (char*)0xFFFFFB /* Bus Control Register */
#define M_DCR   (char*)0xFFFFFA /* DRAM Control Register */
#define M_AAR0  (char*)0xFFFFF9 /* Address Attribute Register 0  */
#define M_AAR1  (char*)0xFFFFF8 /* Address Attribute Register 1  */
#define M_AAR2  (char*)0xFFFFF7 /* Address Attribute Register 2  */
#define M_AAR3  (char*)0xFFFFF6 /* Address Attribute Register 3  */
#define M_IDR   (char*)0xFFFFF5 /* ID Register */

/*       Bus Control Register */

#define M_BA0W  (0x1F)  /* Area 0 Wait Control Mask (BA0W0-BA0W4) */
#define M_BA1W  (0x3E0) /* Area 1 Wait Control Mask (BA1W0-BA14) */
#define M_BA2W  (0x1C00)        /* Area 2 Wait Control Mask (BA2W0-BA2W2) */
#define M_BA3W  (0xE000)        /* Area 3 Wait Control Mask (BA3W0-BA3W3) */
#define M_BDFW  (0x1F0000)      /* Default Area Wait Control Mask (BDFW0-BDFW4) */
#define M_BBS   (21)    /* Bus State */
#define M_BLH   (22)    /* Bus Lock Hold */
#define M_BRH   (23)    /* Bus Request Hold */

/*       DRAM Control Register */

#define M_BCW   (0x3)   /* In Page Wait States Bits Mask (BCW0-BCW1) */
#define M_BRW   (0xC)   /* Out Of Page Wait States Bits Mask (BRW0-BRW1) */
#define M_BPS   (0x300) /* DRAM Page Size Bits Mask (BPS0-BPS1) */
#define M_BPLE  (11)    /* Page Logic Enable */
#define M_BME   (12)    /* Mastership Enable */
#define M_BRE   (13)    /* Refresh Enable */
#define M_BSTR  (14)    /* Software Triggered Refresh */
#define M_BRF   (0x7F8000)      /* Refresh Rate Bits Mask (BRF0-BRF7) */
#define M_BRP   (23)    /* Refresh prescaler */

/*       Address Attribute Registers */

#define M_BAT   (0x3)   /* External Access Type and Pin Definition Bits Mask (BAT0-BAT1) */
#define M_BAAP  (2)     /* Address Attribute Pin Polarity */
#define M_BPEN  (3)     /* Program Space Enable */
#define M_BXEN  (4)     /* X Data Space Enable */
#define M_BYEN  (5)     /* Y Data Space Enable */
#define M_BAM   (6)     /* Address Muxing */
#define M_BPAC  (7)     /* Packing Enable */
#define M_BNC   (0xF00) /* Number of Address Bits to Compare Mask (BNC0-BNC3) */
#define M_BAC   (0xFFF000)      /* Address to Compare Bits Mask (BAC0-BAC11) */

/*       control and status bits in SR */

#define M_CP    (0xc00000)      /* mask for CORE-DMA priority bits in SR */
#define M_CA    (0)     /* Carry */
#define M_V     (1)     /* Overflow       */
#define M_Z     (2)     /* Zero */
#define M_N     (3)     /* Negative       */
#define M_U     (4)     /* Unnormalized */
#define M_E     (5)     /* Extension      */
#define M_L     (6)     /* Limit */
#define M_S     (7)     /* Scaling Bit    */
#define M_I0    (8)     /* Interupt Mask Bit 0 */
#define M_I1    (9)     /* Interupt Mask Bit 1 */
#define M_S0    (10)    /* Scaling Mode Bit 0 */
#define M_S1    (11)    /* Scaling Mode Bit 1 */
#define M_SC    (13)    /* Sixteen_Bit Compatibility */
#define M_DM    (14)    /* Double Precision Multiply */
#define M_LF    (15)    /* DO-Loop Flag */
#define M_FV    (16)    /* DO-Forever Flag */
#define M_SA    (17)    /* Sixteen-Bit Arithmetic */
#define M_CE    (19)    /* Instruction Cache Enable */
#define M_SM    (20)    /* Arithmetic Saturation */
#define M_RM    (21)    /* Rounding Mode */
#define M_CP0   (22)    /* bit 0 of priority bits in SR */
#define M_CP1   (23)    /* bit 1 of priority bits in SR */

/*       control and status bits in OMR */
#define M_CDP   (0x300) /* mask for CORE-DMA priority bits in OMR */
#define M_MA    (0)     /* Operating Mode A */
#define M_MB    (1)     /* Operating Mode B */
#define M_MC    (2)     /* Operating Mode C */
#define M_MD    (3)     /* Operating Mode D */
#define M_EBD   (4)     /* External Bus Disable bit in OMR */
#define M_SD    (6)     /* Stop Delay                      */
#define M_MS    (7)     /* Memory Switch bit in OMR */
#define M_CDP0  (8)     /* bit 0 of priority bits in OMR */
#define M_CDP1  (9)     /* bit 1 of priority bits in OMR */
#define M_BEN   (10)    /* Burst Enable  */
#define M_TAS   (11)    /* TA Synchronize Select */
#define M_BRT   (12)    /* Bus Release Timing    */
#define M_ATE   (15)    /* Address Tracing Enable bit in OMR. */
#define M_XYS   (16)    /* Stack Extension space select bit in OMR. */
#define M_EUN   (17)    /* Extensed stack UNderflow flag in OMR. */
#define M_EOV   (18)    /* Extended stack OVerflow flag in OMR. */
#define M_WRP   (19)    /* Extended WRaP flag in OMR. */
#define M_SEN   (20)    /* Stack Extension Enable bit in OMR. */
