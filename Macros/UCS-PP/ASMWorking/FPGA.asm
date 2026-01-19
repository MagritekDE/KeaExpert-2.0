;*********************************************************************************
;
;     EQUATES for the FPGA Transceiver Board
;
;     Last update: 4 November 2008
;
;*********************************************************************************

        page    132,55,0,0,0
        opt     mex

fpga   ident   1,0

;------------------------------------------------------------------------
;
;       EQUATES for I/O Port Programming
;
;------------------------------------------------------------------------

;       Register Addresses
; Legacy Registers
FPGA_TTL            EQU     $20000         ; TTL
FPGA_Sync           EQU     $20080         ; Sync
FPGA_SampleA        EQU     $20080         ; Sample A
FPGA_SampleB        EQU     $20081         ; Sample B
FPGA_HardVer        EQU     $20082         ; FPGA Hardware version
FPGA_SoftBuild      EQU     $20083         ; FPGA Software Build

; DDS Registers
FPGA_DDS_CR          EQU     $21081         ; DDS Control Register
FPGA_DDS_IR          EQU     $21082         ; DDS Info Register
FPGA_DDS1_PAR_MODE   EQU     $21083         ; DDS Parallel Port mode
FPGA_DDS1_PAR        EQU     $21084         ; DDS Parallel Port 16bit write
FPGA_DDS1_CFR1       EQU     $21180         ; DDS Serial Write CFR1 register 32bit
FPGA_DDS1_CFR2       EQU     $21181         ; DDS Serial Write CFR2 register 32bit
FPGA_DDS1_CFR3       EQU     $21182         ; DDS Serial Write CFR3 register 32bit
FPGA_DDS1_ADAC       EQU     $21183         ; DDS Serial Write Auxiliary DAC register 32bit
FPGA_DDS1_IOU        EQU     $21184         ; DDS Serial Write I/O Update rate register 32bit
FPGA_DDS1_FTW        EQU     $21187         ; DDS Serial Write Frequency tuning word register 32bit
FPGA_DDS1_POW        EQU     $21188         ; DDS Serial Write Phase offset word word register 16bit
FPGA_DDS1_ASF        EQU     $21189         ; DDS Serial Write Amplitude scale factor register 32bit
FPGA_DDS1_MSR        EQU     $2118a         ; DDS Serial Write Multichip sync register 32bit
FPGA_DDS1_DRL        EQU     $2118b         ; DDS Serial Write Digital Ramp limit register 64bit
FPGA_DDS1_DRS        EQU     $2118c         ; DDS Serial Write Digital Ramp step size register 64bit
FPGA_DDS1_DRR        EQU     $2118d         ; DDS Serial Write Digital Ramp rate register 32bit
FPGA_DDS1_Pro0       EQU     $2118e         ; DDS Serial Write Profile 0 register 64bit
FPGA_DDS1_Pro1       EQU     $2118f         ; DDS Serial Write Profile 1 register 64bit
FPGA_DDS1_Pro2       EQU     $21190         ; DDS Serial Write Profile 2 register 64bit
FPGA_DDS1_Pro3       EQU     $21191         ; DDS Serial Write Profile 3 register 64bit
FPGA_DDS1_Pro4       EQU     $21192         ; DDS Serial Write Profile 4 register 64bit
FPGA_DDS1_Pro5       EQU     $21193         ; DDS Serial Write Profile 5 register 64bit
FPGA_DDS1_Pro6       EQU     $21194         ; DDS Serial Write Profile 6 register 64bit
FPGA_DDS1_Pro7       EQU     $21195         ; DDS Serial Write Profile 7 register 64bit
FPGA_DDS1_RR         EQU     $21196         ; DDS Serial RAM register 32bit

FPGA_DDS2_CFR1      EQU     $21280         ; DDS Serial Write CFR1 register 32bit
FPGA_DDS2_CFR2      EQU     $21281         ; DDS Serial Write CFR2 register 32bit
FPGA_DDS2_CFR3      EQU     $21282         ; DDS Serial Write CFR3 register 32bit
FPGA_DDS2_ADAC      EQU     $21283         ; DDS Serial Write Auxiliary DAC register 32bit
FPGA_DDS2_IOU       EQU     $21284         ; DDS Serial Write I/O Update rate register 32bit
FPGA_DDS2_FTW       EQU     $21287         ; DDS Serial Write Frequency tuning word register 32bit
FPGA_DDS2_POW       EQU     $21288         ; DDS Serial Write Phase offset word word register 16bit
FPGA_DDS2_ASF       EQU     $21289         ; DDS Serial Write Amplitude scale factor register 32bit
FPGA_DDS2_MSR       EQU     $2128a         ; DDS Serial Write Multichip sync register 32bit
FPGA_DDS2_DRL       EQU     $2128b         ; DDS Serial Write Digital Ramp limit register 64bit
FPGA_DDS2_DRS       EQU     $2128c         ; DDS Serial Write Digital Ramp step size register 64bit
FPGA_DDS2_DRR       EQU     $2128d         ; DDS Serial Write Digital Ramp rate register 32bit
FPGA_DDS2_Pro0      EQU     $2128e         ; DDS Serial Write Profile 0 register 64bit
FPGA_DDS2_Pro1      EQU     $2128f         ; DDS Serial Write Profile 1 register 64bit
FPGA_DDS2_Pro2      EQU     $21290         ; DDS Serial Write Profile 2 register 64bit
FPGA_DDS2_Pro3      EQU     $21291         ; DDS Serial Write Profile 3 register 64bit
FPGA_DDS2_Pro4      EQU     $21292         ; DDS Serial Write Profile 4 register 64bit
FPGA_DDS2_Pro5      EQU     $21293         ; DDS Serial Write Profile 5 register 64bit
FPGA_DDS2_Pro6      EQU     $21294         ; DDS Serial Write Profile 6 register 64bit
FPGA_DDS2_Pro7      EQU     $21295         ; DDS Serial Write Profile 7 register 64bit
FPGA_DDS2_RR        EQU     $21296         ; DDS Serial RAM register 32bit

; ADC Registers
FPGA_ADC_CR         EQU     $22081         ; ADC Control Register
FPGA_ADC_IR         EQU     $22082         ; ADC Info Register
FPGA_ADC_SW         EQU     $22083         ; ADC Startup Wait Register
FPGA_ADC_RW         EQU     $22084         ; ADC Ready Wait Register
FPGA_ADC_OV         EQU     $22085         ; ADC overflow 1=>reset        

; DRP1 Registers
FPGA_DRP1_CR        EQU     $24081         ; DRP1 Control Register
FPGA_DRP1_PI        EQU     $24190         ; DRP1 Phase Increment Register
FPGA_DRP1_PO        EQU     $24191         ; DRP1 Phase Offset Register
FPGA_DRP1_Dec       EQU     $241a0         ; DRP1 Decimation factor Registor
FPGA_DRP1_Reset     EQU     $241a1         ; DRP1 Decimation factor Registor
FPGA_DRP1_FASTMODE  EQU     $241a2         ;
FPGA_DRP1_SampleNo  EQU     $241a3         ;
FPGA_DRP1_MAXSpeedR EQU     $241a4         ;
FPGA_DRP1_SD        EQU     $241a5         ; DRP1 Sample Delay - Delay before output appears
FPGA_DRP1_Sca       EQU     $241b0         ; DRP1 Scale factor Register
FPGA_DRP1_FIRC      EQU     $241c0         ; DRP1 FIR Control
FPGA_DRP1_FIRScale  EQU     $241c1         ; DRP1 FIR Scale
FPGA_DRP1_FIRData   EQU     $241c2         ; DRP1 FIR tap data
FPGA_DRP1_FIRRC     EQU     $241c3         ; DRP1 FIR Reset Count
FPGA_DRP1_FIRSD     EQU     $241c4         ; DRP1 startup delay

; FIFO Registers
FPGA_FIFO_D         EQU     $26080         ; FIFO Data Read
FPGA_FIFO_CR        EQU     $26081         ; FIFO Control Register
FPGA_FIFO_IR        EQU     $26082         ; FIFO Info Register
FPGA_FIFO_Lv        EQU     $26083         ; FIFO Level

; JTAG Registers
FPGA_JTAG_Con       EQU     $27080         ; JTAG Control setup
FPGA_JTAG_WR        EQU     $27081         ; JTAG Write
FPGA_JTAG_RD        EQU     $27082         ; JTAG Read

FPGALOCK_MEM_ADDRH    EQU      $28001         ; Extended Shared Memory addressing control in 1024x16 chunks
FPGALOCK_MEM_LOCK_0   EQU      $28480
FPGALOCK_MEM_LOCK_1   EQU      $28481
