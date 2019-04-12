; From sfo_v6.lib

	.ref FS$$TOI
	.ref FS$$ADD
	.ref FS$$DIV
	.ref FS$$MPY
	.ref FS$$SUB
	.ref U$$TOFS


	.global _SFO
_SFO:
    PUSH		 XAR1				; Save XAR1
    ADDB         SP, #8				; SP += 8 (for locals)
    MOV          *-SP[4], #0		; SP[-4] = 0
    B            L3F7285, UNC		; Always jmp
L3F7164:
    EALLOW       
    MOVW         DP, #0x1a0         ; 0001 10 1000 00bb bbbb => 6800; EPWM1BASE; HRPWR = 0x6821
    MOV          @0x21, #0          ; HRPWR  = 0
    OR           @0x21, #0x0008	    ; HRPWR |= 0x0008
    OR           @0x21, #0x0020		; HRPWR |= 0x0020
    EDIS         
    MOVW         DP, #0x1c          ; DP = 0x0001 1100    0700
    MOVB         @0x38, #0x01, UNC	; *(0x0738) = 1
    MOV          @0x35, #0			; *(0x0735) = 0
    MOV          @0x36, #0			; *(0x0736) = 0
    MOV          @0x3a, #0			; *(0x073a) = 0
    B            L3F729C, UNC
L3F7176:
    EALLOW       
    MOV          AL, @0x3a
    SBF          L3f7180, NEQ
    MOVW         DP, #0x1a0
    AND          @0x21, #0xfc3f
    MOVB         @0x22, #0x50, UNC
    SB           L3F71C3, UNC

L3f7180:
    SETC         SXM
    MOVL         XAR4, #0x000766
    MOV          ACC, @0x3a << 1
    ADDL         @XAR4, ACC
    MOVL         XAR4, *+XAR4[0]
    MOVB         XAR0, #0x20
    MOV          AL, *+XAR4[AR0]
    MOV          @0x37, AL
    MOV          ACC, @0x3a << 1
    MOVL         XAR4, #0x000766
    ADDL         @XAR4, ACC
    MOVL         XAR4, *+XAR4[0]
    MOVB         XAR0, #0x8
    MOV          AL, *+XAR4[AR0]
    MOV          @0x39, AL
    MOV          ACC, @0x3a << 1
    MOVL         XAR4, #0x000766
    ADDL         @XAR4, ACC
    MOVL         XAR4, *+XAR4[0]
    ADDB         XAR4, #32
    AND          *+XAR4[0], #0xfffc
    MOVL         XAR4, #0x000766
    MOV          ACC, @0x3a << 1
    ADDL         @XAR4, ACC
    MOVL         XAR4, *+XAR4[0]
    MOVW         DP, #0x1a0
    ADDB         XAR4, #32
    OR           *+XAR4[0], #0x0018
    AND          AH, @0x21, #0xfc3f
    MOVW         DP, #0x1c
    MOVL         XAR4, #0x000766
    MOV          AL, @0x3a
    ADDB         AL, #-1
    ORB          AL, #0x8
    MOVW         DP, #0x1a0
    ANDB         AL, #0xf
    LSL          AL, 6
    OR           AL, @AH
    MOV          @0x21, AL
    MOVW         DP, #0x1c
    MOV          ACC, @0x3a << 1
    ADDL         @XAR4, ACC
    MOVL         XAR4, *+XAR4[0]
    MOV          *+XAR4[AR0], #0x5000
L3F71C3:
    MOVW         DP, #0x1a0
    MOV          @0x24, #0
    MOV          @0x25, #0
    OR           @0x21, #0x0004
    EDIS         
    MOVW         DP, #0x1c
    MOVB         @0x38, #0x02, UNC
    B            L3F729C, UNC
L3F71D0:
    EALLOW       
    MOVW         DP, #0x1a0
    AND          AL, @0x21, #0x10
    LSR          AL, 4
    SBF          L3F7218, NEQ
    AND          @0x21, #0xfffb
    MOV          AL, @0x24
    MOVW         DP, #0x1c
    MOVB         @0x38, #0x03, UNC
    MOV          @0x36, AL
    SB           L3F7218, UNC
L3F71E0:
    EALLOW       
    MOV          AL, @0x3a
    MOVW         DP, #0x1a0
    MOVB         @0x22, #0xc8, EQ
    SBF          L3f71fa, EQ
    SETC         SXM
    MOVW         DP, #0x1c
    MOVL         XAR4, #0x000766
    MOV          ACC, @0x3a << 1
    ADDL         @XAR4, ACC
    MOVL         XAR4, *+XAR4[0]
    MOVB         XAR0, #0x8
    MOV          *+XAR4[AR0], #0xc800
    MOVW         DP, #0x1a0
    MOV          @0x24, #0
    MOV          @0x25, #0
    OR           @0x21, #0x0004
L3f71fa:
    EDIS         
    MOV          *-SP[3], #0
    MOV          AL, *-SP[3]
    CMPB         AL, #0x32
    SB           L3F7203, GEQ
L3f71ff:
    INC          *-SP[3]
    MOV          AL, *-SP[3]
    CMPB         AL, #0x32
    SB           L3f71ff, LT
L3F7203:
    MOVW         DP, #0x1c
    MOVB         @0x38, #0x04, UNC
    B            L3F729C, UNC
L3F7209:
    EALLOW       
    MOVW         DP, #0x1a0
    AND          AL, @0x21, #0x10
    LSR          AL, 4
    SBF          L3F7218, NEQ
    AND          @0x21, #0xfffb
    MOV          AL, @0x24
    MOVW         DP, #0x1c
    MOVB         @0x38, #0x05, UNC
    MOV          @0x35, AL
L3F7218:
    EDIS         
    B            L3F729C, UNC
L3F721B:
    MOV          AL, @0x35
    LCR          U$$TOFS
    MOVL         *-SP[2], ACC
    MOV          ACC, #0x7f00 << 15
    LCR          FS$$DIV
    MOVL         XAR1, @ACC
    MOVW         DP, #0x1c
    MOV          AL, @0x36
    LCR          U$$TOFS
    MOVL         *-SP[2], ACC
    MOV          ACC, #0x7f00 << 15
    LCR          FS$$DIV
    MOVL         *-SP[2], ACC
    MOVL         @ACC, XAR1
    LCR          FS$$SUB
    MOVL         @XAR6, ACC
    MOV          @AL, #0xff00
    MOV          @AH, #0x477f
    MOVL         *-SP[2], ACC
    MOVL         ACC, @XAR6
    LCR          FS$$MPY
    MOVL         *-SP[6], ACC
    MOVB         *-SP[7], #0xf0, UNC
    MOV          AL, *-SP[7]
    LCR          U$$TOFS
    MOVL         XAR6, *-SP[6]
    MOVL         *-SP[2], XAR6
    LCR          FS$$DIV
    MOVL         @XAR6, ACC
    MOV          ACC, #0x7e00 << 15
    MOVL         *-SP[2], ACC
    MOVL         ACC, @XAR6
    LCR          FS$$ADD
    LCR          FS$$TOI
    MOVZ         AR6, @AL
    SETC         SXM
    MOVW         DP, #0x1c
    MOVL         XAR4, #0x00073b
    MOV          ACC, @0x3a
    ADDL         @XAR4, ACC
    MOV          *+XAR4[0], AR6
    EALLOW       
    MOV          AL, @0x3b
    MOVW         DP, #0x1d
    MOV          @0x1e, AL
    EDIS         
    MOVW         DP, #0x1c
    CMPB         AL, #0xff
    MOVB         @0x38, #0x01, UNC
    MOVB         *-SP[4], #0x02, GT
    SB           L3F726C, GT
    MOV          AL, @0x3b
    MOVW         DP, #0x1a0
    MOVB         *-SP[4], #0x01, UNC
    MOV          @0x26, AL
L3F726C:
    MOVW         DP, #0x1c
    MOV          AL, @0x3a
    SBF          L3F729C, EQ
    MOV          ACC, @0x3a << 1
    MOVL         XAR4, #0x000766
    ADDL         @XAR4, ACC
    MOVL         XAR4, *+XAR4[0]
    MOVB         XAR0, #0x20
    MOV          AL, @0x37
    MOV          *+XAR4[AR0], AL
    MOV          ACC, @0x3a << 1
    MOVL         XAR4, #0x000766
    ADDL         @XAR4, ACC
    MOVL         XAR4, *+XAR4[0]
    MOVB         XAR0, #0x8
    MOV          AL, @0x39
    MOV          *+XAR4[AR0], AL
    SB           L3F729C, UNC
L3F7285:
    MOVW         DP, #0x1c			;  DP 0x0700
    MOV          AL, @0x38			; AL = *(0x0738)
    CMPB         AL, #0x3			; Compare mode variable with 3
    SB           L3F7297, GT
    CMPB         AL, #0x3
    BF           L3F71E0, EQ
    CMPB         AL, #0x0
    BF           L3F7164, EQ
    CMPB         AL, #0x1
    BF           L3F7176, EQ
    CMPB         AL, #0x2
    BF           L3F71D0, EQ
    SB           L3F729C, UNC
L3F7297:
    CMPB         AL, #0x4
    BF           L3F7209, EQ
    CMPB         AL, #0x5
    SBF          L3F721B, EQ
L3F729C:
    SPM          #0					; Restore product shift to zero
    MOV          AL, *-SP[4]		; Return SP[-4] from AL.
    SUBB         SP, #8				; Remove 8 words of locals from stack
    MOVL         XAR1, *--SP		; Restore XAR1 register
    LRETR        
