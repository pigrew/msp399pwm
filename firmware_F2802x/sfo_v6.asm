; From sfo_v6.lib

	.ref FS$$TOI
	.ref FS$$ADD
	.ref FS$$DIV
	.ref FS$$MPY
	.ref FS$$SUB
	.ref U$$TOFS

; EPwm1 registers are DP 0x1c
; HRPWR
;


; 0x0735  (4aa3) (EPwmReg#24 storage?)
; 0x0736 (89e7)
; 0x0738 SFO_State
; 0x073a SFO_CAL
; 0x073b MEP_SF
; 0x075E MEP_ScaleFactor
; 0x0766 ePWM Array       [0x6800, 0x6800, 0x6840]


	.global _SFO
_SFO:
    PUSH		 XAR1				; Save XAR1
    ADDB         SP, #8				; SP += 8 (for locals)
    MOV          *-SP[4], #0		; SP[-4] = 0
    B            SFO_ENTRY, UNC		; Always jmp

SFO_INIT:
    EALLOW       
    MOVW         DP, #0x1a0         ; 0001 10 1000 00bb bbbb => 6800; EPWM1BASE; HRPWR = 0x6821
    MOV          @0x21, #0          ; HRPWR  = 0
    OR           @0x21, #0x0008	    ; HRPWR |= 0x0008
    OR           @0x21, #0x0020		; HRPWR |= 0x0020
    EDIS         

    MOVW         DP, #0x1c          ; DP = 0x0001 1100    0700
    MOVB         @0x38, #0x01, UNC	; *(0x0738) = 1
    MOV          @0x35, #0			; *(0x0735) = 0
    MOV          @0x36, #0			; *(0x0736) = 0; state variable?; 0=START; 1=initialized
    MOV          @0x3a, #0			; *(0x073a) = 0
    B            SFO_END, UNC

SFO_START_MEAS:
    EALLOW       
    MOV          AL, @0x3a
    SBF          L3f7180, NEQ
    MOVW         DP, #0x1a0			; ePWM1 page 0x6800
    AND          @0x21, #0xfc3f		; HRPWR &= 0xfc3f (clears 0x03c0)
    MOVB         @0x22, #0x50, UNC	; Secret register??? BASE+0x22 = 0x50
    SB           L3F71C3, UNC

L3f7180:
    SETC         SXM				; Enable sign-extended mode
    MOVL         XAR4, #0x000766	; XAR4 = 0x000766
    MOV          ACC, @0x3a << 1	; ACC = *(0x073a) << 1
    ADDL         @XAR4, ACC			; XAR4 += ACC
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
    MOVW         DP, #0x1a0				; ePWM1 page
    MOV          @0x24, #0				; Secret Reg 0x24 = 0
    MOV          @0x25, #0				; Secret Reg 0x25 = 0
    OR           @0x21, #0x0004			; HRPWR |= 0x0004
    EDIS         
    MOVW         DP, #0x1c
    MOVB         @0x38, #0x02, UNC		; Update 0x0738=2 (next state)
    B            SFO_END, UNC			; end

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
    MOV          @0x24, #0				; Secret Reg 0x24 = 0
    MOV          @0x25, #0				; Secret Reg 0x25 = 0
    OR           @0x21, #0x0004			; HRPWR |= 0x0004
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
    B            SFO_END, UNC
L3F7209:
    EALLOW       
    MOVW         DP, #0x1a0			; ePWM1
    AND          AL, @0x21, #0x10	;
    LSR          AL, 4				;AL = HRPWR & 0x10 >> 4
    SBF          L3F7218, NEQ		; Skip recording != 0
    AND          @0x21, #0xfffb		; HRPWR &= 0xFFFB (clear bit #2)
    MOV          AL, @0x24			; AL = ePWMReg#24
    MOVW         DP, #0x1c			;
    MOVB         @0x38, #0x05, UNC	; State = 5
    MOV          @0x35, AL			; Record reg#24 into 0x0735
L3F7218:
    EDIS         
    B            SFO_END, UNC		; and return...
SFO_FINISH:
    MOV          AL, @0x35			; AL = *0x0735 = stored value of EPwmReg#24,       		AL=0x416d
    LCR          U$$TOFS			; ACC = (float)AL =                       			  	ACC=19053.0
    MOVL         *-SP[2], ACC		; SP[-2] = float val									sp[2] = 19053.0
    MOV          ACC, #0x7f00 << 15	; ACC = 0x3780 0000 => 1.0								acc = 1.0
    LCR          FS$$DIV			; float single acc = acc / SP[-2]						acc = 5.24e-5
    MOVL         XAR1, @ACC			; So... XAR1 = 1.0/(*0x0735)							xar1= 5.24e-5
    MOVW         DP, #0x1c			;
    MOV          AL, @0x36			; AL = 0x0736											al=0x2386
    LCR          U$$TOFS			; Convert to single float								acc=35303.0
    MOVL         *-SP[2], ACC		;														sp[2]=35303.0
    MOV          ACC, #0x7f00 << 15	;														acc=1.0
    LCR          FS$$DIV			; ACC = 1/(*0x0736)										acc=2.832e-5
    MOVL         *-SP[2], ACC		; SP[-2] = ACC											sp[2]=2.832e-5
    MOVL         @ACC, XAR1			; ACC = XAR1; weird.									xar1=5.24e-5
    LCR          FS$$SUB			; floating point subtraction acc =  sp[-2] - acc		acc=2.41589678e-5
    MOVL         @XAR6, ACC			; *XAR6 = ACC											xar6=2.41589678e-5
    MOV          @AL, #0xff00		;
    MOV          @AH, #0x477f		; ACC = 0x477fff00 = 65535.0f							acc=65535.0
    MOVL         *-SP[2], ACC		;														sp[2]=65535.0
    MOVL         ACC, @XAR6			;														acc=2.41589678e-5
    LCR          FS$$MPY			; (ACC = ACC * SP[2])									acc=1.58325
    MOVL         *-SP[6], ACC		;														sp[6]=1.58325
    MOVB         *-SP[7], #0xf0, UNC ; SP[7] = 0xf0 (240)									sp[7]=0xf0=240
    MOV          AL, *-SP[7]		;														al=0xf0
    LCR          U$$TOFS			; ACC = 240.0f											acc=240.0f
    MOVL         XAR6, *-SP[6]		; 														xar6=1.58325
    MOVL         *-SP[2], XAR6		;														sp[2]=1.58325
    LCR          FS$$DIV			;														acc = 240/1.58325=151.58
    MOVL         @XAR6, ACC			;														xar6=151.58
    MOV          ACC, #0x7e00 << 15	;														acc = 0.5f
    MOVL         *-SP[2], ACC		;														sp[2]=0.5f
    MOVL         ACC, @XAR6			;														acc = 151.58
    LCR          FS$$ADD			;	acc += 0.5											acc=152.08
    LCR          FS$$TOI			;	convert to int										acc=0x98
    MOVZ         AR6, @AL			;														ar6=0x98 (MEP count)
    SETC         SXM				; enable sign extended multiply
    MOVW         DP, #0x1c			; globals page
    MOVL         XAR4, #0x00073b	; xar4 = 0x00073b (address of MEP_SF array)
    MOV          ACC, @0x3a			; ACC = SFO_CAL
    ADDL         @XAR4, ACC			; SFO_SF[0] = ??
    MOV          *+XAR4[0], AR6     ; SFO_SF[0] = new MEP count?

    EALLOW       
    MOV          AL, @0x3b			; AL = SFO_SF[0] (old one???)
    MOVW         DP, #0x1d
    MOV          @0x1e, AL			; MEP_ScaleFactor = new scale factor
    EDIS

    MOVW         DP, #0x1c
    CMPB         AL, #0xff
    MOVB         @0x38, #0x01, UNC	; Compare scale factor to 0xFF
    MOVB         *-SP[4], #0x02, GT	; Set return value to SFO_ERROR (failure) if >=255
    SB           SKIP_REG_UPDATE, GT
    MOV          AL, @0x3b			; AL = MEP_SP[0]
    MOVW         DP, #0x1a0			; ePWM1 Data page
    MOVB         *-SP[4], #0x01, UNC; Set return value to SFO_DONE (success)
    MOV          @0x26, AL			; Update ePWM1.HRMSTEP!!!

SKIP_REG_UPDATE:
    MOVW         DP, #0x1c
    MOV          AL, @0x3a			; AL = 0x0738
    SBF          SFO_END, EQ		; Return if  (0x0738 == 0)
    MOV          ACC, @0x3a << 1	;ACC = *0x38 << 1
    MOVL         XAR4, #0x000766	; Maybe iterate over the list of SFO structures?
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
    SB           SFO_END, UNC


;;; Jump table to figure out what to do
SFO_ENTRY:
    MOVW         DP, #0x1c			;  DP 0x0700
    MOV          AL, @0x38			; AL = *(0x0738)

    CMPB         AL, #0x3			; Compare state variable with 3, if so, go to second table
    SB           SFO_ENTRY_CONTINUED, GT

    CMPB         AL, #0x3           ; 3 =>
    BF           L3F71E0, EQ

    CMPB         AL, #0x0			; 0 => Need to initilize?
    BF           SFO_INIT, EQ

    CMPB         AL, #0x1			; 1 => Start Measurement
    BF           SFO_START_MEAS, EQ

    CMPB         AL, #0x2			; 2 =>
    BF           L3F71D0, EQ

    SB           SFO_END, UNC

SFO_ENTRY_CONTINUED:
    CMPB         AL, #0x4			; 4=>
    BF           L3F7209, EQ

    CMPB         AL, #0x5			; 5=> Finish run
    SBF          SFO_FINISH, EQ

SFO_END:
    SPM          #0					; Restore product shift to zero
    MOV          AL, *-SP[4]		; Return SP[-4] from AL.
    SUBB         SP, #8				; Remove 8 words of locals from stack
    MOVL         XAR1, *--SP		; Restore XAR1 register
    LRETR        
