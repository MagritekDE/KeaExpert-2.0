;***************************************************************************
;
;       Space for filter coeffs, samples etc
;
        org    y:$0

CONSTANTS     equ     *     
TTL           ds      1                ; TTL state
;RF            ds      1                ; RF TTL state
DATA_ADRS     ds      1                ; Acquisition memory address for append mode
TX_AMP        ds      1                ; Current Tx amplitude
TX_PHASE      ds      1                ; Current Tx phase
TMP           ds      1                ; temporary variables

; **************************************************************************
; 
        org     p:$002000               ; start

        move    r0,x:(r6)+              ; Save registers
        move    r1,x:(r6)+
        move    r2,x:(r6)+
        move    r3,x:(r6)+
        move    r4,x:(r6)+
        move    r5,x:(r6)+
        move    r7,x:(r6)+
        move    x0,x:(r6)+
        move    x1,x:(r6)+
        move    y0,x:(r6)+
        move    y1,x:(r6)+
        move    b0,x:(r6)+
        move    b1,x:(r6)+
        move    b2,x:(r6)+


      ;  movep   #$0F9FE1,x:A_BCR        ; Set up wait states, 4 for AA3
        movep   #$0FFFE1,x:A_BCR        ; Set up wait states, 7 for AA3
        TRIGGERCODE

;Startup sequence
        WOBBLE1
        move    a1,x:FPGA_TTL
        move    #$000000,a1                     ; turn off reset pulse
        move    a1,x:FPGA_Sync
; AD9910
        move    #$000000,a1
        move    a1,x:FPGA_DDS_CR
; ADC LTC2207
        move    #$00d6d8,a1
        move    a1,x:FPGA_ADC_SW            ; startup time from shutdown
        move    #$000003,a1                 ; 3 for dithering 1 without
        move    a1,x:FPGA_ADC_CR            ; Start-up ADC
; Reset Sequence
; AD9910 and DDC
        move    #$000023,a1                     ; Reset AD9910 and CIC and DDC-DDS
        move    a1,x:FPGA_Sync
        move    #$000000,a1                     ; turn off reset pulse
        move    a1,x:FPGA_Sync
; Setup DDC
        move    #$000001,a1                 ; clear on set 0
        move    a1,x:FPGA_DRP1_CR
; Set DDC DDS Frequency
        move    x:RXF00,a1
        move    a1,x:FPGA_DRP1_PI          ; Update DRP DDS frequency
        move    x:RXF01,a1
        move    a1,x:FPGA_DRP1_PI
; Set DDC DDS Phase
        move    x:RXP0,a1
        move    a1,x:FPGA_DRP1_PO          ; Update DRP DDS phase
        move    #0,a1
        move    a1,x:FPGA_DRP1_PO
; Set CIC Fast Mode Specs
        move    #>1,a1
        move    a1,x:FPGA_DRP1_FASTMODE
        move    #>65,a1                     ; Minimum value for largest dwell times
        move    a1,x:FPGA_DRP1_MAXSpeedR
        move    x:SKIPPNTS,a1               ; Number of points to ignore from 
        move    a1,x:FPGA_DRP1_SD           ; Start if data collected
; Set CIC Decimation and scale value
        move    x:DEC1,a                    ; Update CIC Decimation
        move    a1,x:FPGA_DRP1_Dec
        move    x:ATT1,a                    ; Update Scale
        move    a1,x:FPGA_DRP1_Sca
        move    #$000001,a1                 ; turn off reset pulse
        move    a1,x:FPGA_Sync
; Set up filter coeffs
        WOBBLE2
        move    a1,x:FPGA_TTL
        move    #$10,r1                     ; Coefficients are stored from y:#$10
        move    #$000003,a1
        move    a1,x:FPGA_DRP1_FIRC
        move    #$000001,a1
        move    a1,x:FPGA_DRP1_FIRC
        do      x:Ntaps,fcl                  ; Do Ntaps coeffs
        move    y:(r1)+,a1
        move    a1,x:FPGA_DRP1_FIRData       ; Write bottom byte to data register 0 and inc pointer
fcl     nop
; Set FIR values
        move    x:DECFIR,a1                  ; Update CIC decimation set to 1
        move    a1,x:FPGA_DRP1_FIRC
        move    x:ATTFIR,a1
        move    a1,x:FPGA_DRP1_FIRScale
        move    #6000,a1
        move    a1,x:FPGA_DRP1_FIRRC
        move    x:DELAYFIR,a1
        move    #20,a1
        move    a1,x:FPGA_DRP1_FIRSD    ; Update filter delay count
        WOBBLE3
        move    a1,x:FPGA_TTL
; SETUP ADC
        move    #$000009,a1
        move    a1,x:FPGA_ADC_RW            ; delay from ADC capture to ND DDC
        move    #$000004,a1
        move    a1,x:FPGA_ADC_IR            ; Reset Overflow enable flag
        move    #$000000,a1
        move    a1,x:FPGA_ADC_IR            
        move    #$000001,a1                 ; Reset Overflow bit
        move    a1,x:FPGA_ADC_OV

; Reset FIFO
        move    #$000001,a1                 ; RESET FIFO
        move    a1,x:FPGA_FIFO_CR
; Set up the AD9910 DDS

; Reset Serial control (new dual DDS board)
        move    #$001000,a1                 ; Reset serial IO to AD9910
        move    a1,x:FPGA_DDS_CR
        move    #$000400,a1                 ;
        move    a1,x:FPGA_DDS_CR
; Setup DDS PLL clk frequency
        move    #$000538,a1                 ; CFR3 32-16     #$0438
        move    a1,x:FPGA_DDS1_CFR3
        move    #$004128,a1                 ; CFR3 15-0      #$4120      1 GHz clock
        move    a1,x:FPGA_DDS1_CFR3
        move    #2,r7
        bsr     wait
        move    #$000538,a1                 ; CFR3 32-16     #$0438
        move    a1,x:FPGA_DDS2_CFR3
        move    #$004128,a1                 ; CFR3 15-0      #$4140    1 GHz clock
        move    a1,x:FPGA_DDS2_CFR3
        move    #2,r7
        bsr     wait
        move    #$000010,a1                 ; 000010 I/O update
        move    a1,x:FPGA_Sync
        move    #2,r7
        bsr     wait
        move    #$000000,a1                 ; 000010 I/O update
        move    a1,x:FPGA_Sync
        move    #100,r7
        bsr     wait

; Set Auxilary DAC value
        move    #$000000,a1                 ; Aux DAC 32-16
        move    a1,x:FPGA_DDS1_ADAC
        move    #$000085,a1                 ; Aux DAC 15-0
        move    a1,x:FPGA_DDS1_ADAC
        move    #2,r7
        bsr     wait
        move    #$000000,a1                 ; Aux DAC 32-16
        move    a1,x:FPGA_DDS2_ADAC
        move    #$000085,a1                 ; Aux DAC 15-0
        move    a1,x:FPGA_DDS2_ADAC
        move    #2,r7
        bsr     wait
; setup CFR1
        move    #$000049,a1                 ; CFR1 32-16
        move    a1,x:FPGA_DDS2_CFR1
        move    #$000000,a1                 ; CFR1 15-0
        move    a1,x:FPGA_DDS2_CFR1
        move    #2,r7
        bsr     wait
        move    #$000049,a1                 ; CFR1 32-16
        move    a1,x:FPGA_DDS1_CFR1
        move    #$000000,a1                 ; CFR1 15-0
        move    a1,x:FPGA_DDS1_CFR1
        move    #2,r7
        bsr     wait
; Switch off parallel update mode
        move    #$000000,a1        
        move    a1,x:FPGA_DDS1_PAR_MODE

;        clr     a                           ; ADDED
;        move    x:TXA1,a1
;        asl     #2,a,a
;        move    a1,x:FPGA_DDS2_PAR

; Setup CFR2
        move    #$000100,a1                 ; CFR2 32-16
        move    a1,x:FPGA_DDS1_CFR2
        move    #$000080,a1                 ; CFR2 15-0      parallel port mode off match latency on
        move    a1,x:FPGA_DDS1_CFR2
        move    #1,r7
        bsr     wait
        move    #$000100,a1                 ; CFR2 32-16
        move    a1,x:FPGA_DDS2_CFR2
        move    #$000080,a1                 ; CFR2 15-0      parallel port mode off match latency on
        move    a1,x:FPGA_DDS2_CFR2
        move    #1,r7
        bsr     wait
; Setup MultiChip sync
        move    #$002800,a1                 ; MSR 32-16 2c
        move    a1,x:FPGA_DDS1_MSR
        move    x:JITTER_CH1,a1
  ;      move    #$000058,a1                 ; MSR 15-0
        move    a1,x:FPGA_DDS1_MSR
        move    #1,r7
        bsr     wait
        move    #$002800,a1                 ; MSR 32-16
        move    a1,x:FPGA_DDS2_MSR
        move    x:JITTER_CH2,a1
  ;      move    #$000088,a1                 ; MSR 15-0
        move    a1,x:FPGA_DDS2_MSR
        move    #1,r7
        bsr     wait
; Auto generate iou
        move    #$000400,a1                 ;#$000400
        move    a1,x:FPGA_DDS_CR
        move    #2500,r7                   ; wait long time for loop
        bsr     wait

;### End startup if - this is run only during the first loop


; Set values for AD9910 channel 1 profile 0
        move    #$000000,a1                 ;Profile 0   #003fff
        move    a1,x:FPGA_DDS1_Pro0
        move    #$000000,a1
        move    a1,x:FPGA_DDS1_Pro0
        move    x:TXF00,a1                  ; Output W1 to 9910 (freq byte 1)
        move    a1,x:FPGA_DDS1_Pro0
        move    x:TXF01,a1                  ; Output W2 to 9910 (freq byte 0)
        move    a1,x:FPGA_DDS1_Pro0
        move    #2,r7
        bsr     wait
; Set values for AD9910 channel 2 profile 0
        move    #$000000,a1                 ;Profile 0   #003fff
        move    a1,x:FPGA_DDS2_Pro0
        move    #$000000,a1
        move    a1,x:FPGA_DDS2_Pro0
        move    x:TXF00,a1                  ; Output W1 to 9910 (freq byte 1)
        move    a1,x:FPGA_DDS2_Pro0
        move    x:TXF01,a1                  ; Output W2 to 9910 (freq byte 0)
        move    a1,x:FPGA_DDS2_Pro0
        move    #2,r7
        bsr     wait
; Set DDC DDS Frequency
        move    x:RXF00,a1
        move    a1,x:FPGA_DRP1_PI          ; Update DRP DDS frequency
        move    x:RXF01,a1
        move    a1,x:FPGA_DRP1_PI
; Clear Accumulator and setup CFR1
        move    #$000049,a1                 ; CFR1 32-16
        move    a1,x:FPGA_DDS1_CFR1
        move    #$002000,a1                 ; CFR1 15-0
        move    a1,x:FPGA_DDS1_CFR1
        move    #2,r7
        bsr     wait
        move    #$000049,a1                 ; CFR1 32-16
        move    a1,x:FPGA_DDS2_CFR1
        move    #$002000,a1                 ; CFR1 15-0
        move    a1,x:FPGA_DDS2_CFR1
        move    #2,r7
        bsr     wait
; Sync AD9910 and internal DDC
        move    #$000008,a1                 ; SYNC pulse
        move    a1,x:FPGA_Sync
        move    #$000002,a1                 ; Clear Sync port
        move    a1,x:FPGA_Sync
        move    #$000004,a1                 ; Start CE DDC-DDS
        move    a1,x:FPGA_Sync
        move    #$000010,a1                 ; I/O Update to clear registers
        move    a1,x:FPGA_Sync
        move    #$000000,a1                 ; Clear Sync port
        move    a1,x:FPGA_Sync
        move    #2,r7
        bsr     wait

        move    #$000049,a1  ; CFR1 32-16
        move    a1,x:FPGA_DDS1_CFR1
        move    #$000000,a1  ; CFR1 15-0
        move    a1,x:FPGA_DDS1_CFR1
        move    #100,r7
        bsr     wait
        move    #$000049,a1  ; CFR1 32-16
        move    a1,x:FPGA_DDS2_CFR1
        move    #$000000,a1  ; CFR1 15-0
        move    a1,x:FPGA_DDS2_CFR1
        move    #100,r7
        bsr     wait

;;-***---NEW-CODE-END--***--;;
        nop
        nop
        nop
        nop
;  Configure serial ports

        movep   #$2C,x:A_PCRD           ; Set up SSI 1 on Port D
        movep   #$100803,x:A_CRA1       ; SSI 1 ctrl reg A /3 clk, 16 bit word transferred
        movep   #$13C3C,x:A_CRB1        ; SSI 1 ctrl reg B enable SSI port with sc1/2 are outputs

; Check for HF or LF Rx amplifier
        move    x:LFRXAMP,a1
        brclr   #$00000,a1,SKIPHF

;  Set Receiver gain via serial port (1-100 MHz Rx amp)
        movep   #$2C,x:A_PCRC           ; Set up SSI 0
        movep   #$10080A,x:A_CRA0       ; /10 clk, 16 bit word transferred 
        movep   #$13C3C,x:A_CRB0        ; enable SSI port with sc1/2 are outputs

        move    x:RXG1,a1
        movep   #$0000,x:A_PDRE         ; select first gain block
        movep   a,x:A_TX00              ; Set up first gain block
        move    #10,r7
        bsr     wait
        move    x:RXG2,a1
        movep   #$0004,x:A_PDRE         ; select next gain block
        movep   a,x:A_TX00              ; Set up second gain block
        move    #10,r7
        bsr     wait
        bra     SKIPLF

SKIPHF  nop

;  Set Receiver gain via serial port (0-20 MHz Rx amp)

        movep   #$2C,x:A_PCRC           ; Set up SSI 0
        movep   #$18080A,x:A_CRA0       ; /10 clk, 16 bit word transferred 
        movep   #$13C3C,x:A_CRB0        ; enable SSI port with sc1/2 are outputs

        move    x:RXG1,a1
        move    #$00FFFF,x1
        and      x1,a1
        move    #$100000,x1             ; Select gain DAC
        or      x1,a1
        movep   #$0000,x:A_PDRE         ; Select Rx amp interface
        move    a1,x:A_TX00
        move    #10,r7                  ; Wait 10 us
        bsr     wait

;  Set Receiver offset via serial port (0-20 MHz Rx amp)
        move    x:RXG2,a1
        move    #$00FFFF,x1
        and      x1,a1
        move    #$110000,x1             ; Select offset DAC
        or      x1,a1
        movep   #$0000,x:A_PDRE         ; Select Rx amp interface
        move    a1,x:A_TX00
        move    #10,r7                  ; Wait 10 us
        bsr     wait

SKIPLF  nop

        movep   #$0007,x:A_PDRE         ; Select unused serial port
        movep   #$3c3c,x:A_CRB0         ; Disable SSI 0
        movep   #$24,x:A_PCRC

; Reset gradients to zero
        bsr     greset

; Intialise TTL value
        RF_LOCATION1   
        move    a1,y:TTL 
        move    a1,x:FPGA_TTL

; Initialise RF amplitude and phase
        move    #0,a1
        move    a1,y:TX_AMP  
        move    a1,y:TX_PHASE  

; Intialise Data address
        move    #$10000,a1
        move    a1,y:DATA_ADRS         
        move    a1,r5 

; Wait a bit   
        movep   #0,x:A_TLR2             ; Set up event timer
        move    #100,r3
        movep   r3,x:A_TCPR2            ; Set for first event
        movep   #$A01,x:A_TCSR2
  ;      jclr    #21,x:A_TCSR2,*
        movep   #$200A00,x:A_TCSR2      ; Turn off timer

; Set up repetition time
;        movep   #0,x:A_TLR0             ; Set up event timer
;        move    #$7FFFFF,r3
;        movep   r3,x:A_TCPR0            ; Set for first event
;        movep   #$000A01,x:A_TCSR0      ; Start timer
;        nop

