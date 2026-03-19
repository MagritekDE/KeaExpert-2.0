
;***************************************************************************
      nolist
      include  'ioequ.asm'
      include  'FPGA.asm'
      list
;***************************************************************************



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

        movep   #$0FEFE1,x:A_BCR        ; Set up wait states, 7 for AA3

        move    #$000000,a1             ; Unblank RF amp
        move    a1,x:FPGA_TTL
        move    a1,x:FPGA_DRP1_CR       ; Clear on-state flag
        move    a1,x:FPGA_ADC_CR
        move    #$000100,a1             ; Turn off DDS
        move    a1,x:FPGA_DDS_CR

        move    #2,r7                   ; Wait 2us
        bsr     wait

        move    x:-(r6),b2              ; Restore registers
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
