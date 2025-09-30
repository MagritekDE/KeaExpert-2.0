;*********************************************************************************
;
;     EQUATES for 56303 I/O registers and ports
;
;     Last update: June 11 1995
;
;*********************************************************************************

        page    132,55,0,0,0
        opt     mex

ioequ   ident   1,0

;------------------------------------------------------------------------
;
;       EQUATES for I/O Port Programming
;
;------------------------------------------------------------------------

;       Register Addresses

A_HDR    EQU     $FFFFC9         ; PS- Host port GPIO data Register
A_HDDR   EQU     $FFFFC8         ; PS- Host port GPIO direction Register
A_PCRC   EQU     $FFFFBF         ; Port C Control Register
A_PRRC   EQU     $FFFFBE         ; Port C Direction Register
A_PDRC   EQU     $FFFFBD         ; Port C GPIO Data Register
A_PCRD   EQU     $FFFFAF         ; Port D Control register
A_PRRD   EQU     $FFFFAE         ; Port D Direction Data Register
A_PDRD   EQU     $FFFFAD         ; Port D GPIO Data Register
A_PCRE   EQU     $FFFF9F         ; Port E Control register
A_PRRE   EQU     $FFFF9E         ; Port E Direction Register
A_PDRE   EQU     $FFFF9D         ; Port E Data Register
A_OGDB   EQU     $FFFFFC         ; OnCE GDB Register


;------------------------------------------------------------------------
;
;       EQUATES for Host Interface
;
;------------------------------------------------------------------------

;       Register Addresses

A_HCR    EQU     $FFFFC2          ; Host Control Register
A_HSR    EQU     $FFFFC3          ; Host Status Rgister
A_HPCR   EQU     $FFFFC4          ; Host Polarity Control Register
A_HBAR   EQU     $FFFFC5          ; Host Base Address Register
A_HRX    EQU     $FFFFC6          ; Host Receive Register
A_HTX    EQU     $FFFFC7          ; Host Transmit Register

;       HCR bits definition
A_HRIE   EQU    $0              ; Host Receive interrupts Enable
A_HTIE   EQU    $1              ; Host Transmit Interrupt Enable
A_HCIE   EQU    $2              ; Host Command Interrupt Enable
A_HF2    EQU    $3              ; Host Flag 2
A_HF3    EQU    $4              ; Host Flag 3

;       HSR bits definition
A_HRDF   EQU    $0              ; Host Receive Data Full
A_HTDE   EQU    $1              ; Host Receive Data Emptiy
A_HCP    EQU    $2              ; Host Command Pending
A_HF0    EQU    $3              ; Host Flag 0
A_HF1    EQU    $4              ; Host Flag 1


;       HPCR bits definition
A_HGEN   EQU    $0              ; Host Port GPIO Enable
A_HA8EN  EQU    $1              ; Host Address 8 Enable
A_HA9EN  EQU    $2              ; Host Address 9 Enable
A_HCSEN  EQU    $3              ; Host Chip Select Enable
A_HREN   EQU    $4              ; Host Request Enable
A_HAEN   EQU    $5              ; Host Acknowledge Enable
A_HEN    EQU    $6              ; Host Enable
A_HOD    EQU    $8              ; Host Request Open Drain mode
A_HDSP   EQU    $9              ; Host Data Strobe Polarity
A_HASP   EQU    $A              ; Host Address Strobe Polarity
A_HMUX   EQU    $B              ; Host Multiplexed bus select
A_HD_HS  EQU    $C              ; Host Double/Single Strobe select
A_HCSP   EQU    $D              ; Host Chip Select Polarity
A_HRP    EQU    $E              ; Host Request PolarityPolarity
A_HAP    EQU    $F              ; Host Acknowledge Polarity


;------------------------------------------------------------------------
;
;       EQUATES for Serial Communications Interface (SCI)
;
;------------------------------------------------------------------------

;       Register Addresses

A_STXH   EQU     $FFFF97         ; SCI Transmit Data Register (high)
A_STXM   EQU     $FFFF96         ; SCI Transmit Data Register (middle)
A_STXL   EQU     $FFFF95         ; SCI Transmit Data Register (low)
A_SRXH   EQU     $FFFF9A         ; SCI Receive Data Register (high)
A_SRXM   EQU     $FFFF99         ; SCI Receive Data Register (middle)
A_SRXL   EQU     $FFFF98         ; SCI Receive Data Register (low)
A_STXA   EQU     $FFFF94         ; SCI Transmit Address Register
A_SCR    EQU     $FFFF9C         ; SCI Control Register
A_SSR    EQU     $FFFF93         ; SCI Status Register
A_SCCR   EQU     $FFFF9B         ; SCI Clock Control Register

;       SCI Control Register Bit Flags

A_WDS    EQU     $7              ; Word Select Mask (WDS0-WDS3)
A_WDS0   EQU     0               ; Word Select 0
A_WDS1   EQU     1               ; Word Select 1
A_WDS2   EQU     2               ; Word Select 2
A_SSFTD  EQU     3               ; SCI Shift Direction
A_SBK    EQU     4               ; Send Break
A_WAKE   EQU     5               ; Wakeup Mode Select
A_RWU    EQU     6               ; Receiver Wakeup Enable
A_WOMS   EQU     7               ; Wired-OR Mode Select
A_SCRE   EQU     8               ; SCI Receiver Enable
A_SCTE   EQU     9               ; SCI Transmitter Enable
A_ILIE   EQU     10              ; Idle Line Interrupt Enable
A_SCRIE  EQU     11              ; SCI Receive Interrupt Enable
A_SCTIE  EQU     12              ; SCI Transmit Interrupt Enable
A_TMIE   EQU     13              ; Timer Interrupt Enable
A_TIR    EQU     14              ; Timer Interrupt Rate
A_SCKP   EQU     15              ; SCI Clock Polarity
A_REIE   EQU     16              ; SCI Error Interrupt Enable (REIE)

;       SCI Status Register Bit Flags

A_TRNE   EQU     0               ; Transmitter Empty
A_TDRE   EQU     1               ; Transmit Data Register Empty
A_RDRF   EQU     2               ; Receive Data Register Full
A_IDLE   EQU     3               ; Idle Line Flag
A_OR     EQU     4               ; Overrun Error Flag
A_PE     EQU     5               ; Parity Error
A_FE     EQU     6               ; Framing Error Flag
A_R8     EQU     7               ; Received Bit 8 (R8) Address

;       SCI Clock Control Register

A_CD     EQU     $FFF            ; Clock Divider Mask (CD0-CD11)
A_COD    EQU     12              ; Clock Out Divider
A_SCP    EQU     13              ; Clock Prescaler
A_RCM    EQU     14              ; Receive Clock Mode Source Bit
A_TCM    EQU     15              ; Transmit Clock Source Bit

;------------------------------------------------------------------------
;
;       EQUATES for Synchronous Serial Interface (SSI)
;
;------------------------------------------------------------------------

;
;       Register Addresses Of SSI0
A_TX0    EQU     $FFFFBC         ; SSI0 Transmit Data Register 0
A_TX00   EQU     $FFFFBC         ; SSI0 Transmit Data Register 0
A_TX01   EQU     $FFFFBB         ; SSIO Transmit Data Register 1
A_TX02   EQU     $FFFFBA         ; SSIO Transmit Data Register 2
A_TSR0   EQU     $FFFFB9         ; SSI0 Time Slot Register
A_RX0    EQU     $FFFFB8         ; SSI0 Receive Data Register
A_SSISR0 EQU     $FFFFB7         ; SSI0 Status Register
A_CRB0   EQU     $FFFFB6         ; SSI0 Control Register B
A_CRA0   EQU     $FFFFB5         ; SSI0 Control Register A
A_TSMA0  EQU     $FFFFB4         ; SSI0 Transmit Slot Mask Register A
A_TSMB0  EQU     $FFFFB3         ; SSI0 Transmit Slot Mask Register B
A_RSMA0  EQU     $FFFFB2         ; SSI0 Receive Slot Mask Register A
A_RSMB0  EQU     $FFFFB1         ; SSI0 Receive Slot Mask Register B

;       Register Addresses Of SSI1
A_TX1    EQU     $FFFFAC         ; SSI1 Transmit Data Register 0
A_TX10   EQU     $FFFFAC         ; SSI1 Transmit Data Register 0
A_TX11   EQU     $FFFFAB         ; SSI1 Transmit Data Register 1
A_TX12   EQU     $FFFFAA         ; SSI1 Transmit Data Register 2
A_TSR1   EQU     $FFFFA9         ; SSI1 Time Slot Register
A_RX1    EQU     $FFFFA8         ; SSI1 Receive Data Register
A_SSISR1 EQU     $FFFFA7         ; SSI1 Status Register
A_CRB1   EQU     $FFFFA6         ; SSI1 Control Register B
A_CRA1   EQU     $FFFFA5         ; SSI1 Control Register A
A_TSMA1  EQU     $FFFFA4         ; SSI1 Transmit Slot Mask Register A
A_TSMB1  EQU     $FFFFA3         ; SSI1 Transmit Slot Mask Register B
A_RSMA1  EQU     $FFFFA2         ; SSI1 Receive Slot Mask Register A
A_RSMB1  EQU     $FFFFA1         ; SSI1 Receive Slot Mask Register B

;       SSI Control Register A Bit Flags

A_PM     EQU     $FF             ; Prescale Modulus Select Mask (PM0-PM7)
A_PSR    EQU     11              ; Prescaler Range
A_DC     EQU     $1F000          ; Frame Rate Divider Control Mask (DC0-DC7)
A_ALC    EQU     18              ; Alignment Control (ALC)
A_WL     EQU     $380000         ; Word Length Control Mask (WL0-WL7)
A_SSC1   EQU     22              ; Select SC1 as TR #0 drive enable (SSC1)

;       SSI Control Register B Bit Flags

A_OF     EQU     $3              ; Serial Output Flag Mask
A_OF0    EQU     0               ; Serial Output Flag 0
A_OF1    EQU     1               ; Serial Output Flag 1
A_SCD    EQU     $1C             ; Serial Control Direction Mask
A_SCD0   EQU     2               ; Serial Control 0 Direction
A_SCD1   EQU     3               ; Serial Control 1 Direction
A_SCD2   EQU     4               ; Serial Control 2 Direction
A_SCKD   EQU     5               ; Clock Source Direction
A_SHFD   EQU     6               ; Shift Direction
A_FSL    EQU     $180            ; Frame Sync Length Mask (FSL0-FSL1)
A_FSL0   EQU     7               ; Frame Sync Length 0
A_FSL1   EQU     8               ; Frame Sync Length 1
A_FSR    EQU     9               ; Frame Sync Relative Timing
A_FSP    EQU     10              ; Frame Sync Polarity
A_CKP    EQU     11              ; Clock Polarity
A_SYN    EQU     12              ; Sync/Async Control
A_MOD    EQU     13              ; SSI Mode Select
;NOTE: REMOVE THIS NEXT LINE FOR COMPATIBILITY
A_SSTE   EQU     16              ; SSI Transmit #0 Enable
;A_SSTE   EQU     $1C000          ; SSI Transmit enable Mask
A_SSTE2  EQU     14              ; SSI Transmit #2 Enable
A_SSTE1  EQU     15              ; SSI Transmit #1 Enable
A_SSTE0  EQU     16              ; SSI Transmit #0 Enable
A_SSRE   EQU     17              ; SSI Receive Enable
A_SSTIE  EQU     18              ; SSI Transmit Interrupt Enable
A_SSRIE  EQU     19              ; SSI Receive Interrupt Enable
A_STLIE  EQU     20              ; SSI Transmit Last Slot Interrupt Enable
A_SRLIE  EQU     21              ; SSI Receive Last Slot Interrupt Enable
A_STEIE  EQU     22              ; SSI Transmit Error Interrupt Enable
A_SREIE  EQU     23              ; SSI Receive Error Interrupt Enable

;       SSI Status Register Bit Flags

A_IF     EQU     $3              ; Serial Input Flag Mask
A_IF0    EQU     0               ; Serial Input Flag 0
A_IF1    EQU     1               ; Serial Input Flag 1
A_TFS    EQU     2               ; Transmit Frame Sync Flag
A_RFS    EQU     3               ; Receive Frame Sync Flag
A_TUE    EQU     4               ; Transmitter Underrun Error FLag
A_ROE    EQU     5               ; Receiver Overrun Error Flag
A_TDE    EQU     6               ; Transmit Data Register Empty
A_RDF    EQU     7               ; Receive Data Register Full

;       SSI Transmit Slot Mask Register A

A_SSTSA  EQU     $FFFF           ; SSI Transmit Slot Bits Mask A (TS0-TS15)

;       SSI Transmit Slot Mask Register B

A_SSTSB  EQU     $FFFF           ; SSI Transmit Slot Bits Mask B (TS16-TS31)

;       SSI Receive Slot Mask Register A

A_SSRSA  EQU     $FFFF           ; SSI Receive Slot Bits Mask A (RS0-RS15)

;       SSI Receive Slot Mask Register B

A_SSRSB  EQU     $FFFF           ; SSI Receive Slot Bits Mask B (RS16-RS31)



;------------------------------------------------------------------------
;
;       EQUATES for Exception Processing
;
;------------------------------------------------------------------------


;       Register Addresses

A_IPRC   EQU     $FFFFFF         ; Interrupt Priority Register Core
A_IPRP   EQU     $FFFFFE         ; Interrupt Priority Register Peripheral

;       Interrupt Priority Register Core (IPRC)

A_IAL    EQU     $7              ; IRQA Mode Mask
A_IAL0   EQU     0               ; IRQA Mode Interrupt Priority Level (low)
A_IAL1   EQU     1               ; IRQA Mode Interrupt Priority Level (high)
A_IAL2   EQU     2               ; IRQA Mode Trigger Mode
A_IBL    EQU     $38             ; IRQB Mode Mask
A_IBL0   EQU     3               ; IRQB Mode Interrupt Priority Level (low)
A_IBL1   EQU     4               ; IRQB Mode Interrupt Priority Level (high)
A_IBL2   EQU     5               ; IRQB Mode Trigger Mode
A_ICL    EQU     $1C0            ; IRQC Mode Mask
A_ICL0   EQU     6               ; IRQC Mode Interrupt Priority Level (low)
A_ICL1   EQU     7               ; IRQC Mode Interrupt Priority Level (high)
A_ICL2   EQU     8               ; IRQC Mode Trigger Mode
A_IDL    EQU     $E00            ; IRQD Mode Mask
A_IDL0   EQU     9               ; IRQD Mode Interrupt Priority Level (low)
A_IDL1   EQU     10              ; IRQD Mode Interrupt Priority Level (high)
A_IDL2   EQU     11              ; IRQD Mode Trigger Mode
A_D0L    EQU     $3000           ; DMA0 Interrupt priority Level Mask
A_D0L0   EQU     12              ; DMA0 Interrupt Priority Level (low)
A_D0L1   EQU     13              ; DMA0 Interrupt Priority Level (high)
A_D1L    EQU     $C000           ; DMA1 Interrupt Priority Level Mask
A_D1L0   EQU     14              ; DMA1 Interrupt Priority Level (low)
A_D1L1   EQU     15              ; DMA1 Interrupt Priority Level (high)
A_D2L    EQU     $30000          ; DMA2 Interrupt priority Level Mask
A_D2L0   EQU     16              ; DMA2 Interrupt Priority Level (low)
A_D2L1   EQU     17              ; DMA2 Interrupt Priority Level (high)
A_D3L    EQU     $C0000          ; DMA3 Interrupt Priority Level Mask
A_D3L0   EQU     18              ; DMA3 Interrupt Priority Level (low)
A_D3L1   EQU     19              ; DMA3 Interrupt Priority Level (high)
A_D4L    EQU     $300000         ; DMA4 Interrupt priority Level Mask
A_D4L0   EQU     20              ; DMA4 Interrupt Priority Level (low)
A_D4L1   EQU     21              ; DMA4 Interrupt Priority Level (high)
A_D5L    EQU     $C00000         ; DMA5 Interrupt priority Level Mask
A_D5L0   EQU     22              ; DMA5 Interrupt Priority Level (low)
A_D5L1   EQU     23              ; DMA5 Interrupt Priority Level (high)


;       Interrupt Priority Register Peripheral (IPRP)

A_HPL    EQU     $3              ; Host Interrupt Priority Level Mask
A_HPL0   EQU     0               ; Host Interrupt Priority Level (low)
A_HPL1   EQU     1               ; Host Interrupt Priority Level (high)
A_S0L    EQU     $C              ; SSI0 Interrupt Priority Level Mask
A_S0L0   EQU     2               ; SSI0 Interrupt Priority Level (low)
A_S0L1   EQU     3               ; SSI0 Interrupt Priority Level (high)
A_S1L    EQU     $30             ; SSI1 Interrupt Priority Level Mask
A_S1L0   EQU     4               ; SSI1 Interrupt Priority Level (low)
A_S1L1   EQU     5               ; SSI1 Interrupt Priority Level (high)
A_SCL    EQU     $C0             ; SCI  Interrupt Priority Level  Mask
A_SCL0   EQU     6               ; SCI  Interrupt Priority Level  (low)
A_SCL1   EQU     7               ; SCI  Interrupt Priority Level  (high)
A_T0L    EQU     $300            ; TIMER Interrupt Priority Level Mask
A_T0L0   EQU     8               ; TIMER Interrupt Priority Level (low)
A_T0L1   EQU     9               ; TIMER Interrupt Priority Level (high)


;------------------------------------------------------------------------
;
;       EQUATES for TIMER
;
;------------------------------------------------------------------------

;       Register Addresses Of TIMER0

A_TCSR0  EQU     $FFFF8F         ; TIMER0 Control/Status Register
A_TLR0   EQU     $FFFF8E         ; TIMER0 Load Reg
A_TCPR0  EQU     $FFFF8D         ; TIMER0 Compare Register
A_TCR0   EQU     $FFFF8C         ; TIMER0 Count Register

;       Register Addresses Of TIMER1

A_TCSR1  EQU     $FFFF8B         ; TIMER1 Control/Status Register
A_TLR1   EQU     $FFFF8A         ; TIMER1 Load Reg
A_TCPR1  EQU     $FFFF89         ; TIMER1 Compare Register
A_TCR1   EQU     $FFFF88         ; TIMER1 Count Register


;       Register Addresses Of TIMER2

A_TCSR2  EQU     $FFFF87         ; TIMER2 Control/Status Register
A_TLR2   EQU     $FFFF86         ; TIMER2 Load Reg
A_TCPR2  EQU     $FFFF85         ; TIMER2 Compare Register
A_TCR2   EQU     $FFFF84         ; TIMER2 Count Register
A_TPLR   EQU     $FFFF83         ; TIMER Prescaler Load Register
A_TPCR   EQU     $FFFF82         ; TIMER Prescalar Count Register


;       Timer Control/Status Register Bit Flags

A_TE     EQU     0               ; Timer Enable
A_TOIE   EQU     1               ; Timer Overflow Interrupt Enable
A_TCIE   EQU     2               ; Timer Compare Interrupt Enable
A_TC     EQU     $F0             ; Timer Control Mask (TC0-TC3)
A_INV    EQU     8               ; Inverter Bit
A_TRM    EQU     9               ; Timer Restart Mode
A_DIR    EQU     11              ; Direction Bit
A_DI     EQU     12              ; Data Input
A_DO     EQU     13              ; Data Output
A_PCE    EQU     15              ; Prescaled Clock Enable
A_TOF    EQU     20              ; Timer Overflow Flag
A_TCF    EQU     21              ; Timer Compare Flag

;       Timer Prescaler Register Bit Flags

A_PS    EQU      $600000         ; Prescaler Source Mask
A_PS0   EQU      21
A_PS1   EQU      22

;       Timer Control Bits
A_TC0    EQU     4               ; Timer Control 0
A_TC1    EQU     5               ; Timer Control 1
A_TC2    EQU     6               ; Timer Control 2
A_TC3    EQU     7               ; Timer Control 3


;------------------------------------------------------------------------
;
;       EQUATES for Direct Memory Access (DMA)
;
;------------------------------------------------------------------------

;       Register Addresses Of DMA
A_DSTR   EQU     $FFFFF4         ; DMA Status Register
A_DOR0   EQU     $FFFFF3         ; DMA Offset Register 0
A_DOR1   EQU     $FFFFF2         ; DMA Offset Register 1
A_DOR2   EQU     $FFFFF1         ; DMA Offset Register 2
A_DOR3   EQU     $FFFFF0         ; DMA Offset Register 3


;       Register Addresses Of DMA0

A_DSR0   EQU     $FFFFEF         ; DMA0 Source Address Register
A_DDR0   EQU     $FFFFEE         ; DMA0 Destination Address Register
A_DCO0   EQU     $FFFFED         ; DMA0 Counter
A_DCR0   EQU     $FFFFEC         ; DMA0 Control Register

;       Register Addresses Of DMA1

A_DSR1   EQU     $FFFFEB         ; DMA1 Source Address Register
A_DDR1   EQU     $FFFFEA         ; DMA1 Destination Address Register
A_DCO1   EQU     $FFFFE9         ; DMA1 Counter
A_DCR1   EQU     $FFFFE8         ; DMA1 Control Register

;       Register Addresses Of DMA2

A_DSR2   EQU     $FFFFE7         ; DMA2 Source Address Register
A_DDR2   EQU     $FFFFE6         ; DMA2 Destination Address Register
A_DCO2   EQU     $FFFFE5         ; DMA2 Counter
A_DCR2   EQU     $FFFFE4         ; DMA2 Control Register

;       Register Addresses Of DMA4

A_DSR3   EQU     $FFFFE3         ; DMA3 Source Address Register
A_DDR3   EQU     $FFFFE2         ; DMA3 Destination Address Register
A_DCO3   EQU     $FFFFE1         ; DMA3 Counter
A_DCR3   EQU     $FFFFE0         ; DMA3 Control Register

;       Register Addresses Of DMA4


A_DSR4   EQU     $FFFFDF         ; DMA4 Source Address Register
A_DDR4   EQU     $FFFFDE         ; DMA4 Destination Address Register
A_DCO4   EQU     $FFFFDD         ; DMA4 Counter
A_DCR4   EQU     $FFFFDC         ; DMA4 Control Register

;       Register Addresses Of DMA5

A_DSR5   EQU     $FFFFDB         ; DMA5 Source Address Register
A_DDR5   EQU     $FFFFDA         ; DMA5 Destination Address Register
A_DCO5   EQU     $FFFFD9         ; DMA5 Counter
A_DCR5   EQU     $FFFFD8         ; DMA5 Control Register

;       DMA Control Register

A_DSS    EQU    $3              ; DMA Source Space Mask (DSS0-Dss1)
A_DSS0   EQU    0               ; DMA Source Memory space 0
A_DSS1   EQU    1               ; DMA Source Memory space 1
A_DDS    EQU    $C              ; DMA Destination Space Mask (DDS-DDS1)
A_DDS0   EQU    2               ; DMA Destination Memory Space 0
A_DDS1   EQU    3               ; DMA Destination Memory Space 1
A_DAM    EQU    $3f0            ; DMA Address Mode Mask (DAM5-DAM0)
A_DAM0   EQU    4               ; DMA Address Mode 0
A_DAM1   EQU    5               ; DMA Address Mode 1
A_DAM2   EQU    6               ; DMA Address Mode 2
A_DAM3   EQU    7               ; DMA Address Mode 3
A_DAM4   EQU    8               ; DMA Address Mode 4
A_DAM5   EQU    9               ; DMA Address Mode 5
A_D3D    EQU    10              ; DMA Three Dimensional Mode
A_DRS    EQU    $F800           ; DMA Request Source Mask (DRS0-DRS4)
A_DCON   EQU    16              ; DMA Continuous Mode
A_DPR    EQU    $60000          ; DMA Channel Priority
A_DPR0   EQU    17              ; DMA Channel Priority Level (low)
A_DPR1   EQU    18              ; DMA Channel Priority Level (high)
A_DTM    EQU    $380000         ; DMA Transfer Mode Mask (DTM2-DTM0)
A_DTM0   EQU    19              ; DMA Transfer Mode 0
A_DTM1   EQU    20              ; DMA Transfer Mode 1
A_DTM2   EQU    21              ; DMA Transfer Mode 2
A_DIE    EQU    22              ; DMA Interrupt Enable bit
A_DE     EQU    23              ; DMA Channel Enable bit

;       DMA Status Register

A_DTD    EQU    $3F             ; Channel Transfer Done Status MASK (DTD0-DTD5)
A_DTD0   EQU     0               ; DMA Channel Transfer Done Status 0
A_DTD1   EQU     1               ; DMA Channel Transfer Done Status 1
A_DTD2   EQU     2               ; DMA Channel Transfer Done Status 2
A_DTD3   EQU     3               ; DMA Channel Transfer Done Status 3
A_DTD4   EQU     4               ; DMA Channel Transfer Done Status 4
A_DTD5   EQU     5               ; DMA Channel Transfer Done Status 5
A_DACT   EQU     8               ; DMA Active State
A_DCH    EQU     $E00            ; DMA Active Channel Mask (DCH0-DCH2)
A_DCH0   EQU     9               ; DMA Active Channel 0
A_DCH1   EQU     10              ; DMA Active Channel 1
A_DCH2   EQU     11              ; DMA Active Channel 2


;------------------------------------------------------------------------
;
;       EQUATES for Phase Locked Loop (PLL)
;
;------------------------------------------------------------------------

;       Register Addresses Of PLL

A_PCTL   EQU     $FFFFFD         ; PLL Control Register

;       PLL Control Register

A_MF     EQU     $FFF            ; Multiplication Factor Bits Mask (MF0-MF11)
A_DF     EQU     $7000           ; Division Factor Bits Mask (DF0-DF2)
A_XTLR   EQU     15              ; XTAL Range select bit
A_XTLD   EQU     16              ; XTAL Disable Bit
A_PSTP   EQU     17              ; STOP Processing State Bit
A_PEN    EQU     18              ; PLL Enable Bit
A_PCOD   EQU     19              ; PLL Clock Output Disable Bit
A_PD     EQU    $F00000         ; PreDivider Factor Bits Mask (PD0-PD3)


;------------------------------------------------------------------------
;
;       EQUATES for BIU
;
;------------------------------------------------------------------------

;       Register Addresses Of BIU


A_BCR    EQU     $FFFFFB         ; Bus Control Register
A_DCR    EQU     $FFFFFA         ; DRAM Control Register
A_AAR0   EQU     $FFFFF9         ; Address Attribute Register 0
A_AAR1   EQU     $FFFFF8         ; Address Attribute Register 1
A_AAR2   EQU     $FFFFF7         ; Address Attribute Register 2
A_AAR3   EQU     $FFFFF6         ; Address Attribute Register 3
A_IDR    EQU     $FFFFF5         ; ID Register

;       Bus Control Register

A_BA0W   EQU     $1F             ; Area 0 Wait Control Mask (BA0W0-BA0W4)
A_BA1W   EQU     $3E0            ; Area 1 Wait Control Mask (BA1W0-BA14)
A_BA2W   EQU     $1C00           ; Area 2 Wait Control Mask (BA2W0-BA2W2)
A_BA3W   EQU     $E000           ; Area 3 Wait Control Mask (BA3W0-BA3W3)
A_BDFW   EQU     $1F0000         ; Default Area Wait Control Mask (BDFW0-BDFW4)
A_BBS    EQU     21              ; Bus State
A_BLH    EQU     22              ; Bus Lock Hold
A_BRH    EQU     23              ; Bus Request Hold

;       DRAM Control Register

A_BCW    EQU     $3              ; In Page Wait States Bits Mask (BCW0-BCW1)
A_BRW    EQU     $C              ; Out Of Page Wait States Bits Mask (BRW0-BRW1)
A_BPS    EQU     $300            ; DRAM Page Size Bits Mask (BPS0-BPS1)
A_BPLE   EQU     11              ; Page Logic Enable
A_BME    EQU     12              ; Mastership Enable
A_BRE    EQU     13              ; Refresh Enable
A_BSTR   EQU     14              ; Software Triggered Refresh
A_BRF    EQU     $7F8000         ; Refresh Rate Bits Mask (BRF0-BRF7)
A_BRP    EQU     23              ; Refresh prescaler

;       Address Attribute Registers

A_BAT    EQU     $3              ; External Access Type and Pin Definition Bits Mask (BAT0-BAT1)
A_BAAP   EQU     2               ; Address Attribute Pin Polarity
A_BPEN   EQU     3               ; Program Space Enable
A_BXEN   EQU     4               ; X Data Space Enable
A_BYEN   EQU     5               ; Y Data Space Enable
A_BAM    EQU     6               ; Address Muxing
A_BPAC   EQU     7               ; Packing Enable
A_BNC    EQU     $F00            ; Number of Address Bits to Compare Mask (BNC0-BNC3)
A_BAC    EQU     $FFF000         ; Address to Compare Bits Mask (BAC0-BAC11)

;       control and status bits in SR

A_CP    EQU     $c00000          ; mask for CORE-DMA priority bits in SR
A_CA    equ     0                ; Carry
A_V     equ     1                ; Overflow
A_Z     equ     2                ; Zero
A_N     equ     3                ; Negative
A_U     equ     4                ; Unnormalized
A_E     equ     5                ; Extension
A_L     equ     6                ; Limit
A_S     equ     7                ; Scaling Bit
A_I0    equ     8                ; Interupt Mask Bit 0
A_I1    equ     9                ; Interupt Mask Bit 1
A_S0    equ     10               ; Scaling Mode Bit 0
A_S1    equ     11               ; Scaling Mode Bit 1
A_SC    equ     13               ; Sixteen_Bit Compatibility
A_DM    equ     14               ; Double Precision Multiply
A_LF    equ     15               ; DO-Loop Flag
A_FV    equ     16               ; DO-Forever Flag
A_SA    equ     17               ; Sixteen-Bit Arithmetic
A_CE    equ     19               ; Instruction Cache Enable
A_SM    equ     20               ; Arithmetic Saturation
A_RM    equ     21               ; Rounding Mode
A_CP0   EQU     22               ; bit 0 of priority bits in SR
A_CP1   EQU     23               ; bit 1 of priority bits in SR

;       control and status bits in OMR
A_CDP   EQU     $300             ; mask for CORE-DMA priority bits in OMR
A_MA    equ     0                ; Operating Mode A
A_MB   equ      1                ; Operating Mode B
A_MC    equ     2                ; Operating Mode C
A_MD    equ     3                ; Operating Mode D
A_EBD   equ     4                ; External Bus Disable bit in OMR
A_SD    equ     6                ; Stop Delay
A_MS    EQU     7                ; Memory Switch bit in OMR
A_CDP0  EQU     8                ; bit 0 of priority bits in OMR
A_CDP1  EQU     9                ; bit 1 of priority bits in OMR
A_BEN   EQU     10               ; Burst Enable
A_TAS   EQU     11               ; TA Synchronize Select
A_BRT   EQU     12               ; Bus Release Timing
A_ATE   equ     15               ; Address Tracing Enable bit in OMR.
A_XYS   equ     16               ; Stack Extension space select bit in OMR.
A_EUN   equ     17               ; Extensed stack UNderflow flag in OMR.
A_EOV   equ     18               ; Extended stack OVerflow flag in OMR.
A_WRP   equ     19               ; Extended WRaP flag in OMR.
A_SEN   equ     20               ; Stack Extension Enable bit in OMR.
