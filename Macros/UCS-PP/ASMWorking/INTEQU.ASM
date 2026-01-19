;*********************************************************************************
;
;     EQUATES for ONYXE 56302 interrupts
;
;     Last update: June 11 1995 
;
;*********************************************************************************

	page	132,55,0,0,0
	opt	mex
intequ  ident   1,0

	if	@DEF(I_VEC)
	;leave user definition as is.
	else
I_VEC	equ	$0
	endif

;------------------------------------------------------------------------
; Non-Maskable interrupts
;------------------------------------------------------------------------
I_RESET  EQU  I_VEC+$00   ; Hardware RESET
I_STACK  EQU  I_VEC+$02   ; Stack Error
I_ILL    EQU  I_VEC+$04   ; Illegal Instruction
I_DBG    EQU  I_VEC+$06   ; Debug Request      
I_TRAP   EQU  I_VEC+$08   ; Trap
I_NMI    EQU  I_VEC+$0A   ; Non Maskable Interrupt

;------------------------------------------------------------------------
; Interrupt Request Pins
;------------------------------------------------------------------------
I_IRQA   EQU  I_VEC+$10   ; IRQA
I_IRQB   EQU  I_VEC+$12   ; IRQB
I_IRQC   EQU  I_VEC+$14   ; IRQC
I_IRQD   EQU  I_VEC+$16   ; IRQD

;------------------------------------------------------------------------
; DMA Interrupts
;------------------------------------------------------------------------
I_DMA0   EQU  I_VEC+$18   ; DMA Channel 0
I_DMA1   EQU  I_VEC+$1A   ; DMA Channel 1
I_DMA2   EQU  I_VEC+$1C   ; DMA Channel 2
I_DMA3   EQU  I_VEC+$1E   ; DMA Channel 3
I_DMA4   EQU  I_VEC+$20   ; DMA Channel 4
I_DMA5   EQU  I_VEC+$22   ; DMA Channel 5

;------------------------------------------------------------------------
; Timer Interrupts
;------------------------------------------------------------------------
I_TIM0C  EQU  I_VEC+$24   ; TIMER 0 compare
I_TIM0OF EQU  I_VEC+$26   ; TIMER 0 overflow
I_TIM1C  EQU  I_VEC+$28   ; TIMER 1 compare
I_TIM1OF EQU  I_VEC+$2A   ; TIMER 1 overflow
I_TIM2C  EQU  I_VEC+$2C   ; TIMER 2 compare
I_TIM2OF EQU  I_VEC+$2E   ; TIMER 2 overflow

;------------------------------------------------------------------------
; ESSI Interrupts
;------------------------------------------------------------------------
I_SI0RD  EQU  I_VEC+$30   ; ESSI0 Receive Data
I_SI0RDE EQU  I_VEC+$32   ; ESSI0 Receive Data With Exception Status
I_SI0RLS EQU  I_VEC+$34   ; ESSI0 Receive last slot
I_SI0TD  EQU  I_VEC+$36   ; ESSI0 Transmit data
I_SI0TDE EQU  I_VEC+$38   ; ESSI0 Transmit Data With Exception Status
I_SI0TLS EQU  I_VEC+$3A   ; ESSI0 Transmit last slot
I_SI1RD  EQU  I_VEC+$40   ; ESSI1 Receive Data
I_SI1RDE EQU  I_VEC+$42   ; ESSI1 Receive Data With Exception Status
I_SI1RLS EQU  I_VEC+$44   ; ESSI1 Receive last slot
I_SI1TD  EQU  I_VEC+$46   ; ESSI1 Transmit data
I_SI1TDE EQU  I_VEC+$48   ; ESSI1 Transmit Data With Exception Status
I_SI1TLS EQU  I_VEC+$4A   ; ESSI1 Transmit last slot

;------------------------------------------------------------------------
; SCI Interrupts
;------------------------------------------------------------------------
I_SCIRD  EQU  I_VEC+$50   ; SCI Receive Data 
I_SCIRDE EQU  I_VEC+$52   ; SCI Receive Data With Exception Status
I_SCITD  EQU  I_VEC+$54   ; SCI Transmit Data
I_SCIIL  EQU  I_VEC+$56   ; SCI Idle Line
I_SCITM  EQU  I_VEC+$58   ; SCI Timer

;------------------------------------------------------------------------
; HOST Interrupts
;------------------------------------------------------------------------
I_HRDF   EQU     I_VEC+$60   ; Host Receive Data Full
I_HTDE   EQU     I_VEC+$62   ; Host Transmit Data Empty
I_HC     EQU     I_VEC+$64   ; Default Host Command

;------------------------------------------------------------------------
; INTERRUPT ENDING ADDRESS
;------------------------------------------------------------------------
I_INTEND EQU  I_VEC+$FF   ; last address of interrupt vector space
         LIST
