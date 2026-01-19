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
;       Address     Function
;       $28XXX      uBlaze control
;       $29XXX      Legacy
;       $2aXXX      DDS
;       $2bXXX      ADC
;       $2cXXX      DRP (DDC)
;       $2dXXX      DAC
;       $2eXXX      NA
;       $2fXXX      See FPGAtemp.asm (temperature control board)
;
;------------------------------------------------------------------------

;       Register Addresses
; microBlaze control Registers
FPGALOCK_UB_CR        EQU      $28000         ; uB Control Register
; bit0 : 0 - select DSP control peripherals : 0 - select uB control peripherals
; bit1 : 0 - reset uB : 1 - run uB
; bit4 : 0 - high Z T1 output : 1 - Enable T1 output ; make sure DSP T-REX V2_1 is either not connected (or code changed) as the T-REX V2_1 will always drive the T1 bus
FPGALOCK_MEM_ADDRH     EQU      $28001         ; Extended Shared Memory addressing control in 1024x16 chunks

FPGALOCK_MEM           EQU      $28400         ; addr start 
FPGALOCK_MEM_CHUNKSIZE EQU      $200           ; 1024x16 memory size
FPGALOCK_MEM_START_I   EQU      $00           ; extended MEM start for uB instruction mem
FPGALOCK_MEM_SIZE_I    EQU      $10            ; extended MEM size for uB instruction mem
FPGALOCK_MEM_START_D   EQU      $10            ; extended MEM start for data mem
FPGALOCK_MEM_SIZE_D    EQU      $10            ; extended MEM size for data mem
FPGALOCK_MEM_START_P   EQU      $20            ; extended MEM start for pulse program mem
FPGALOCK_MEM_SIZE_P    EQU      $10            ; extended MEM size for pulse program mem

; put in ability to write to uB memory uB can only be written to when in reset mode


; Legacy Registers
FPGALOCK_TTL            EQU     $29000         ; TTL
FPGALOCK_Sync           EQU     $29080         ; Sync
FPGALOCK_SampleA        EQU     $29080         ; Sample A
FPGALOCK_SampleB        EQU     $29081         ; Sample B
FPGALOCK_HardVer        EQU     $29082         ; FPGA Hardware version
FPGALOCK_SoftBuild      EQU     $29083         ; FPGA Software Build

; DDS Registers
FPGALOCK_DDS_CR         EQU     $2a081         ; DDS Control Register
FPGALOCK_DDS_IR         EQU     $2a082         ; DDS Info Register
FPGALOCK_DDS1_PAR_MODE   EQU     $2a083         ; DDS Parallel Port mode
FPGALOCK_DDS1_PAR        EQU     $2a084         ; DDS Parallel Port 16bit write
FPGALOCK_DDS1_CFR1       EQU     $2a180         ; DDS Serial Write CFR1 register 32bit
FPGALOCK_DDS1_CFR2       EQU     $2a181         ; DDS Serial Write CFR2 register 32bit
FPGALOCK_DDS1_CFR3       EQU     $2a182         ; DDS Serial Write CFR3 register 32bit
FPGALOCK_DDS1_ADAC       EQU     $2a183         ; DDS Serial Write Auxiliary DAC register 32bit
FPGALOCK_DDS1_IOU        EQU     $2a184         ; DDS Serial Write I/O Update rate register 32bit
FPGALOCK_DDS1_FTW        EQU     $2a187         ; DDS Serial Write Frequency tuning word register 32bit
FPGALOCK_DDS1_POW        EQU     $2a188         ; DDS Serial Write Phase offset word word register 16bit
FPGALOCK_DDS1_ASF        EQU     $2a189         ; DDS Serial Write Amplitude scale factor register 32bit
FPGALOCK_DDS1_MSR        EQU     $2a18a         ; DDS Serial Write Multichip sync register 32bit
FPGALOCK_DDS1_DRL        EQU     $2a18b         ; DDS Serial Write Digital Ramp limit register 64bit
FPGALOCK_DDS1_DRS        EQU     $2a18c         ; DDS Serial Write Digital Ramp step size register 64bit
FPGALOCK_DDS1_DRR        EQU     $2a18d         ; DDS Serial Write Digital Ramp rate register 32bit
FPGALOCK_DDS1_Pro0       EQU     $2a18e         ; DDS Serial Write Profile 0 register 64bit
FPGALOCK_DDS1_Pro1       EQU     $2a18f         ; DDS Serial Write Profile 1 register 64bit
FPGALOCK_DDS1_Pro2       EQU     $2a190         ; DDS Serial Write Profile 2 register 64bit
FPGALOCK_DDS1_Pro3       EQU     $2a191         ; DDS Serial Write Profile 3 register 64bit
FPGALOCK_DDS1_Pro4       EQU     $2a192         ; DDS Serial Write Profile 4 register 64bit
FPGALOCK_DDS1_Pro5       EQU     $2a193         ; DDS Serial Write Profile 5 register 64bit
FPGALOCK_DDS1_Pro6       EQU     $2a194         ; DDS Serial Write Profile 6 register 64bit
FPGALOCK_DDS1_Pro7       EQU     $2a195         ; DDS Serial Write Profile 7 register 64bit
FPGALOCK_DDS1_RR         EQU     $2a196         ; DDS Serial RAM register 32bit

; ADC Registers
FPGALOCK_ADC_CR         EQU     $2b081         ; ADC Control Register
FPGALOCK_ADC_IR         EQU     $2b082         ; ADC Info Register
FPGALOCK_ADC_SW         EQU     $2b083         ; ADC Startup Wait Register
FPGALOCK_ADC_RW         EQU     $2b084         ; ADC Ready Wait Register
FPGA_ADC_OV             EQU     $22085         ; ADC overflow 1=>reset        

; DRP1 Registers
FPGALOCK_DRP1_CR        EQU     $2c081         ; DRP1 Control Register
FPGALOCK_DRP1_PI        EQU     $2c190         ; DRP1 Phase Increment Register
FPGALOCK_DRP1_PO        EQU     $2c191         ; DRP1 Phase Offset Register
FPGALOCK_DRP1_Dec       EQU     $2c1a0         ; DRP1 Decimation factor Registor
FPGALOCK_DRP1_Reset     EQU     $2c1a1         ; DRP1 Decimation factor Registor
FPGALOCK_DRP1_FASTMODE  EQU     $2c1a2         ;
FPGALOCK_DRP1_SampleNo  EQU     $2c1a3         ;
FPGALOCK_DRP1_MAXSpeedR EQU     $2c1a4         ;
FPGALOCK_DRP1_SD        EQU     $2c1a5         ; DRP1 Sample Delay - Delay before output appears
FPGALOCK_DRP1_Sca       EQU     $2c1b0         ; DRP1 Scale factor Register
FPGALOCK_DRP1_FIRC      EQU     $2c1c0         ; DRP1 FIR Control
FPGALOCK_DRP1_FIRScale  EQU     $2c1c1         ; DRP1 FIR Scale
FPGALOCK_DRP1_FIRData   EQU     $2c1c2         ; DRP1 FIR tap data
FPGALOCK_DRP1_FIRRC     EQU     $2c1c3         ; DRP1 FIR Reset Count
FPGALOCK_DRP1_FIRSD     EQU     $2c1c4         ; DRP1 startup delay

; DAC Registers
FPGALOCK_DAC_CR         EQU     $2d000         ; DAC Control Register
FPGALOCK_DAC_DATA       EQU     $2d001         ; DAC DATA Register
