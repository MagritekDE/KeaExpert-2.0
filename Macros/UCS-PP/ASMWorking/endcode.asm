
;*************************************************************
;        End pulse program
;
        jmp     SKIP1 
ABORT   move    #$10000,r5             ; Write 100 into the first 1000 memory location
        move    b1,y:(r5)+
        move    #$99,r7
        do      r7,ABORTLP
        move   #100,a1
        move    a1,y:(r5)+
ABORTLP nop

SKIP1    WOBBLE4
        move    a1,x:FPGA_TTL

; Reset gradients to zero
        bsr     greset

SKIP2   move    x:-(r6),b2              ; Restore registers
        move    x:-(r6),b1
        move    x:-(r6),b0
        move    x:-(r6),y1
        move    x:-(r6),y0
        move    x:-(r6),x1
        move    x:-(r6),x0
        move    x:-(r6),r7
        move    x:-(r6),r5
        move    x:-(r6),r4
        move    x:-(r6),r3
        move    x:-(r6),r2
        move    x:-(r6),r1
        move    x:-(r6),r0
        rts



; 
;*************************************************************
;        Wait n x 1us routine (100MHz clock)
;        On entry, r7 contains "n"
; 
wait    rep     #90
        nop
        move    (r7)-
        move    r7,b
        tst     b
        bne     wait
        rts
; 
;*************************************************************
;        short fixed wait 100ns routine (100MHz clock)
; 
swait   rep     #4
        nop
        rts

; 
;*************************************************************
;        Short variable wait of n x 100 ns routine (100MHz clock)
;        On entry, r7 contains "n"
; 
svwait  move    (r7)-
        move    r7,b
        tst     b
        nop
        nop
        nop
        bne     svwait
        rts

; 
;*************************************************************
;       Initialise gradients
; 
greset  move    x:GRADRESET,a1
        brclr   #$00000,a1,SKIPGRD

; Reset code for different gradient controller versions
        clr     a
        clr     b
        move    x:GRADVERSION,a1
        cmp     #04,a
        bne     G15
; 4 channel gradient controller reset code
G4      movep   #$2C,x:A_PCRD           ; Set up SSI 0
        movep   #$18080A,x:A_CRA0       ; /10 clk, 16 bit word transferred 
        movep   #$13C3C,x:A_CRB0        ; enable SSI port with sc1/2 are outputs
        move    #04,r7                  ; 4 gradients to reset
        move    #$00,b0                 ; Initial gradient = 0
        do      r7,G4LP                 ; Loop 4 times
        movep   b0,x:A_PDRE             ; Set the gradient channel
        move    #$00,a1
        movep   a1,x:A_TX10             ; Zero the gradeint
        rep     #180                    ; Wait 2 us
        nop                             ; for data to transfer
        inc     b                       ; Increment gradient channel
G4LP    nop
        bra     SKIPGRD 
; 15 channel gradient controller reset code
G15     cmp     #15,a
        bne     G16
        movep   #$2C,x:A_PCRD           ; Set up SSI 0
        movep   #$18080A,x:A_CRA0       ; /10 clk, 16 bit word transferred 
        movep   #$13C3C,x:A_CRB0        ; enable SSI port with sc1/2 are outputs
        move    #$00,a1
        movep   a1,x:A_PDRC		    ; Select second group of DACs     
        move    #$07,a1
        movep   a1,x:A_PDRE             ; Select reset line		
        move    #$00,a1
        movep   a1,x:A_TX10             ; Send zero to all gradients
        rep     #180                    ; Wait 2 us
        nop 
        bra     SKIPGRD 
; 16 channel gradient controller reset code
G16     cmp     #16,a
        bne     SKIPGRD
        movep   #$2C,x:A_PCRD           ; Turn on SSI 1 on Port D
        movep   #$180802,x:A_CRA1       ; /2 clk, 24 bit word transferred
        movep   #$13C3C,x:A_CRB1        ; Enable SSI port with sc1/2 are outputs
        move    #16,r7
        move    #$00,r5                 ; Gradient couner
        do      r7,G16LP
        clr     a
        clr     b
        move    r5,a1
        move    #$0001,b1               ; Assume first group
        cmp    #8,a                     ; See if channel is < 8
        jlt    G16SKIP
        sub    #8,a                     ; Subtract 8
        move    #$0000,b1               ; Second group
G16SKIP nop
        movep   b1,x:A_PDRC             ; Select group of 8 DACs
        move    a1,b0                   ; Save subgroup (4 channels)
        lsr     #2,a                    ; Shift 2 bits to right to determine subgroup (result 0/1)
        add     #4,a                    ; Add 4 to only access pins Y4 or Y5 on U7
        move    a1,x:A_PDRE             ; Select block of 4 DACs
        move    #$0,a1                  ; Get gradient amplitude
        move    #$00FFFF,x1
        and      x1,a1
        move    b0,b1                   ; Restore subgroup
        lsl     #16,b                   ; Move into correct format for DAC
        move    #$030000,x1
        and      x1,b
        move    #$100000,x1
        or       x1,b
        move     b1,x1
        or       x1,a                   ; Add amplitude word
        move    a1,x:A_TX10             ; Send channel info + grad. amplitude to DAC
        rep     #180                    ; Wait 2 us
        nop 
        clr     b
        move    r5,b1                   ; Next gradient
        add     #1,b
        move    b1,r5 
G16LP   nop


SKIPGRD nop
        movep   #$24,x:A_PCRD           ; Turn off SSI 1 on Port D (prevents serial noise)
        rts

; 
; *************************************************************
; 
;        TTL parameters
; 
ttl   dc      $00
; 
;*************************************************************


