/* Standalone near-call runtime; no banked window is used. */
.section .startup,"ax",@progbits
.globl _start
_start:
    sei
    movb #0x21,0x0010
    lds #0x4000
    /* Copy initialized data from fixed Flash to RAM. */
    ldx #__data_load
    ldy #__data_start
    ldd #__data_size
    beq 2f
1:  movb 1,x+,1,y+
    dbne d,1b
2:  ldx #__bss_start
    ldd #__bss_size
    beq 4f
3:  clr 1,x+
    dbne d,3b
4:  jsr main
5:  bra 5b

/* GCC passes the sole unsigned char argument in B. FlashCmd lives in RAM. */
.section .text.BBFlashCommand,"ax",@progbits
.globl BBFlashCommand
BBFlashCommand:
    jsr FlashCmd
    rts

.section .text.exit,"ax",@progbits
.globl exit
.globl _exit
exit:
_exit:
    sei
6:  bra 6b
