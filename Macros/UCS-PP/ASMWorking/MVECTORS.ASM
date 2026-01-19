;
        page    132,60
;****************************************************************************
;   myVECTORS.ASM
;   Vector table for the 56303
;
;   Copyright (c) MOTOROLA 1996
;            Semiconductor Products Sector
;            Digital Signal Processing Division
;
;****************************************************************************
;
         ORG       P:0
;
vectors  JMP       START                        ; Hardware RESET
;
         jmp       *
         NOP                        ;  Stack Error

         jmp       *
         NOP                        ;- Debug Request Interrupt
;
         jmp       *
         NOP                        ;- Debug Request Interrupt
;
         jmp       *
         NOP                        ;- Trap
;
         jmp       *
         NOP                        ;- NMI
;
         NOP                                    ;- Reserved
         NOP
;
         NOP                                    ;- Reserved
         NOP
;
         jsr       cmdint                       ;- IRQA
;
         jmp       *
         NOP                        ;- IRQB
;
         jmp       *
         NOP                        ;- IRQC
;
         jmp       *
         NOP                        ;- IRQD
;
         jmp       *
         NOP                        ;- DMA Channel 0
;
         jmp       *
         NOP                        ;- DMA Channel 1
;
         jmp       *
         NOP                        ;- DMA Channel 2
;
         jmp       *
         NOP                        ;- DMA Channel 3
;
         jmp       *
         NOP                        ;- DMA Channel 4
;
         jmp       *
         NOP                        ;- DMA Channel 5
;
         jmp       *
         NOP                        ;- Timer 0 Compare
;
         jmp       *
         NOP                        ;- Timer 0 Overflow
;
         jmp       *
         NOP                        ;- Timer 1 Compare
;
         jmp       *
         NOP                        ;- Timer 1 Overflow
;
         jmp       *
         NOP                        ;- Timer 2 Compare
;
         jmp       *
         NOP                        ;- Timer 2 Overflow
;
         jsr       ssi_rx_isr                    ;- ESSI0 Receive Data
;
         jsr       ssi_rxe_isr                   ;- ESSI0 Receive Data w/ Exception Status
;
         jsr       ssi_rxls_isr                  ;- ESSI0 Receive last slot
;
         jsr       ssi_tx_isr                    ;- ESSI0 Transmit Data
;
         jsr       ssi_txe_isr                   ;- ESSI0 Transmit Data w/ Exception Status
;
         jsr       ssi_txls_isr                  ;- ESSI0 Transmit last slot
;
         NOP                                     ;- Reserved
         NOP
;
         NOP                                     ;- Reserved
         NOP
;
         jmp       *
         NOP                        ;- ESSI1 Receive Data
;
         jmp       *
         NOP                        ;- ESSI1 Receive Data w/ Exception Status
;
         jmp       *
         NOP                        ;- ESSI1 Receive last slot
;
         jmp       *
         NOP                        ;- ESSI1 Transmit Data
;
         jmp       *
         NOP                        ;- ESSI1 Transmit Data w/ Exception Status
;
         jmp       *
         NOP                        ;- ESSI1 Transmit last slot
;

         NOP                                     ;- Reserved
         NOP
;
         NOP                                     ;- Reserved
         NOP
;
         jmp       *
         NOP                        ;- SCI Receive Data
;
         jmp       *
         NOP                        ;- SCI Receive Data w/ Exception Status
;
         jmp       *
         NOP                        ;- SCI Transmit Data
;
         jmp       *
         NOP                        ;- SCI Idle Line
;
         jmp       *
         NOP                        ;- SCI Timer
;
         NOP                                     ;- Reserved
         NOP
;
         NOP                                     ;- Reserved
         NOP
;
         NOP                                     ;- Reserved
         NOP
;
;
         jmp       *
         NOP                        ; Host receive data full
;
;
         jmp       *
         NOP                        ;- Host transmit data empty
;
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
         jmp       *
         NOP                        ; Available for Host Command
;
;
