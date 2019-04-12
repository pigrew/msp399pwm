        SFO():
3f715f:   B2BD        MOVL         *SP++, XAR1
3f7160:   FE08        ADDB         SP, #8
3f7161:   2B44        MOV          *-SP[4], #0
3f7162:   FFEF0123    B            291, UNC
3f7164:   7622        EALLOW       
3f7165:   761F01A0    MOVW         DP, #0x1a0
3f7167:   2B21        MOV          @0x21, #0
3f7168:   1A210008    OR           @0x21, #0x0008
3f716a:   1A210020    OR           @0x21, #0x0020
3f716c:   761A        EDIS         
3f716d:   761F001C    MOVW         DP, #0x1c
3f716f:   56BF0138    MOVB         @0x38, #0x01, UNC
3f7171:   2B35        MOV          @0x35, #0
3f7172:   2B36        MOV          @0x36, #0
3f7173:   2B3A        MOV          @0x3a, #0
3f7174:   FFEF0128    B            296, UNC
3f7176:   7622        EALLOW       
3f7177:   923A        MOV          AL, @0x3a
3f7178:   ED08        SBF          8, NEQ
3f7179:   761F01A0    MOVW         DP, #0x1a0
3f717b:   1821FC3F    AND          @0x21, #0xfc3f
3f717d:   56BF5022    MOVB         @0x22, #0x50, UNC
3f717f:   6F44        SB           68, UNC
3f7180:   3B01        SETC         SXM
3f7181:   8F000766    MOVL         XAR4, #0x000766
3f7183:   5603013A    MOV          ACC, @0x3a << 1
3f7185:   560100A4    ADDL         @XAR4, ACC
3f7187:   8AC4        MOVL         XAR4, *+XAR4[0]
3f7188:   D020        MOVB         XAR0, #0x20
3f7189:   9294        MOV          AL, *+XAR4[AR0]
3f718a:   9637        MOV          @0x37, AL
3f718b:   5603013A    MOV          ACC, @0x3a << 1
3f718d:   8F000766    MOVL         XAR4, #0x000766
3f718f:   560100A4    ADDL         @XAR4, ACC
3f7191:   8AC4        MOVL         XAR4, *+XAR4[0]
3f7192:   D008        MOVB         XAR0, #0x8
3f7193:   9294        MOV          AL, *+XAR4[AR0]
3f7194:   9639        MOV          @0x39, AL
3f7195:   5603013A    MOV          ACC, @0x3a << 1
3f7197:   8F000766    MOVL         XAR4, #0x000766
3f7199:   560100A4    ADDL         @XAR4, ACC
3f719b:   8AC4        MOVL         XAR4, *+XAR4[0]
3f719c:   DC20        ADDB         XAR4, #32
3f719d:   18C4FFFC    AND          *+XAR4[0], #0xfffc
3f719f:   8F000766    MOVL         XAR4, #0x000766
3f71a1:   5603013A    MOV          ACC, @0x3a << 1
3f71a3:   560100A4    ADDL         @XAR4, ACC
3f71a5:   8AC4        MOVL         XAR4, *+XAR4[0]
3f71a6:   761F01A0    MOVW         DP, #0x1a0
3f71a8:   DC20        ADDB         XAR4, #32
3f71a9:   1AC40018    OR           *+XAR4[0], #0x0018
3f71ab:   CD21FC3F    AND          AH, @0x21, #0xfc3f
3f71ad:   761F001C    MOVW         DP, #0x1c
3f71af:   8F000766    MOVL         XAR4, #0x000766
3f71b1:   923A        MOV          AL, @0x3a
3f71b2:   9CFF        ADDB         AL, #-1
3f71b3:   5008        ORB          AL, #0x8
3f71b4:   761F01A0    MOVW         DP, #0x1a0
3f71b6:   900F        ANDB         AL, #0xf
3f71b7:   FF85        LSL          AL, 6
3f71b8:   CAA8        OR           AL, @AH
3f71b9:   9621        MOV          @0x21, AL
3f71ba:   761F001C    MOVW         DP, #0x1c
3f71bc:   5603013A    MOV          ACC, @0x3a << 1
3f71be:   560100A4    ADDL         @XAR4, ACC
3f71c0:   8AC4        MOVL         XAR4, *+XAR4[0]
3f71c1:   28945000    MOV          *+XAR4[AR0], #0x5000
3f71c3:   761F01A0    MOVW         DP, #0x1a0
3f71c5:   2B24        MOV          @0x24, #0
3f71c6:   2B25        MOV          @0x25, #0
3f71c7:   1A210004    OR           @0x21, #0x0004
3f71c9:   761A        EDIS         
3f71ca:   761F001C    MOVW         DP, #0x1c
3f71cc:   56BF0238    MOVB         @0x38, #0x02, UNC
3f71ce:   FFEF00CE    B            206, UNC
3f71d0:   7622        EALLOW       
3f71d1:   761F01A0    MOVW         DP, #0x1a0
3f71d3:   CC210010    AND          AL, @0x21, #0x10
3f71d5:   FFC3        LSR          AL, 4
3f71d6:   ED42        SBF          66, NEQ
3f71d7:   1821FFFB    AND          @0x21, #0xfffb
3f71d9:   9224        MOV          AL, @0x24
3f71da:   761F001C    MOVW         DP, #0x1c
3f71dc:   56BF0338    MOVB         @0x38, #0x03, UNC
3f71de:   9636        MOV          @0x36, AL
3f71df:   6F39        SB           57, UNC
3f71e0:   7622        EALLOW       
3f71e1:   923A        MOV          AL, @0x3a
3f71e2:   761F01A0    MOVW         DP, #0x1a0
3f71e4:   56B1C822    MOVB         @0x22, #0xc8, EQ
3f71e6:   EC0E        SBF          14, EQ
3f71e7:   3B01        SETC         SXM
3f71e8:   761F001C    MOVW         DP, #0x1c
3f71ea:   8F000766    MOVL         XAR4, #0x000766
3f71ec:   5603013A    MOV          ACC, @0x3a << 1
3f71ee:   560100A4    ADDL         @XAR4, ACC
3f71f0:   8AC4        MOVL         XAR4, *+XAR4[0]
3f71f1:   D008        MOVB         XAR0, #0x8
3f71f2:   2894C800    MOV          *+XAR4[AR0], #0xc800
3f71f4:   761F01A0    MOVW         DP, #0x1a0
3f71f6:   2B24        MOV          @0x24, #0
3f71f7:   2B25        MOV          @0x25, #0
3f71f8:   1A210004    OR           @0x21, #0x0004
3f71fa:   761A        EDIS         
3f71fb:   2B43        MOV          *-SP[3], #0
3f71fc:   9243        MOV          AL, *-SP[3]
3f71fd:   5232        CMPB         AL, #0x32
3f71fe:   6305        SB           5, GEQ
3f71ff:   0A43        INC          *-SP[3]
3f7200:   9243        MOV          AL, *-SP[3]
3f7201:   5232        CMPB         AL, #0x32
3f7202:   64FD        SB           -3, LT
3f7203:   761F001C    MOVW         DP, #0x1c
3f7205:   56BF0438    MOVB         @0x38, #0x04, UNC
3f7207:   FFEF0095    B            149, UNC
3f7209:   7622        EALLOW       
3f720a:   761F01A0    MOVW         DP, #0x1a0
3f720c:   CC210010    AND          AL, @0x21, #0x10
3f720e:   FFC3        LSR          AL, 4
3f720f:   ED09        SBF          9, NEQ
3f7210:   1821FFFB    AND          @0x21, #0xfffb
3f7212:   9224        MOV          AL, @0x24
3f7213:   761F001C    MOVW         DP, #0x1c
3f7215:   56BF0538    MOVB         @0x38, #0x05, UNC
3f7217:   9635        MOV          @0x35, AL
3f7218:   761A        EDIS         
3f7219:   FFEF0083    B            131, UNC
3f721b:   9235        MOV          AL, @0x35
3f721c:   767F78B1    LCR          U$$TOFS
3f721e:   1E42        MOVL         *-SP[2], ACC
3f721f:   FF2F7F00    MOV          ACC, #0x7f00 << 15
3f7221:   767F75FA    LCR          FS$$DIV
3f7223:   8BA9        MOVL         XAR1, @ACC
3f7224:   761F001C    MOVW         DP, #0x1c
3f7226:   9236        MOV          AL, @0x36
3f7227:   767F78B1    LCR          U$$TOFS
3f7229:   1E42        MOVL         *-SP[2], ACC
3f722a:   FF2F7F00    MOV          ACC, #0x7f00 << 15
3f722c:   767F75FA    LCR          FS$$DIV
3f722e:   1E42        MOVL         *-SP[2], ACC
3f722f:   B2A9        MOVL         @ACC, XAR1
3f7230:   767F767D    LCR          FS$$SUB
3f7232:   1EA6        MOVL         @XAR6, ACC
3f7233:   28A9FF00    MOV          @AL, #0xff00
3f7235:   28A8477F    MOV          @AH, #0x477f
3f7237:   1E42        MOVL         *-SP[2], ACC
3f7238:   06A6        MOVL         ACC, @XAR6
3f7239:   767F76F5    LCR          FS$$MPY
3f723b:   1E46        MOVL         *-SP[6], ACC
3f723c:   56BFF047    MOVB         *-SP[7], #0xf0, UNC
3f723e:   9247        MOV          AL, *-SP[7]
3f723f:   767F78B1    LCR          U$$TOFS
3f7241:   C446        MOVL         XAR6, *-SP[6]
3f7242:   C242        MOVL         *-SP[2], XAR6
3f7243:   767F75FA    LCR          FS$$DIV
3f7245:   1EA6        MOVL         @XAR6, ACC
3f7246:   FF2F7E00    MOV          ACC, #0x7e00 << 15
3f7248:   1E42        MOVL         *-SP[2], ACC
3f7249:   06A6        MOVL         ACC, @XAR6
3f724a:   767F7682    LCR          FS$$ADD
3f724c:   767F7800    LCR          FS$$TOI
3f724e:   88A9        MOVZ         AR6, @AL
3f724f:   3B01        SETC         SXM
3f7250:   761F001C    MOVW         DP, #0x1c
3f7252:   8F00073B    MOVL         XAR4, #0x00073b
3f7254:   853A        MOV          ACC, @0x3a
3f7255:   560100A4    ADDL         @XAR4, ACC
3f7257:   7EC4        MOV          *+XAR4[0], AR6
3f7258:   7622        EALLOW       
3f7259:   923B        MOV          AL, @0x3b
3f725a:   761F001D    MOVW         DP, #0x1d
3f725c:   961E        MOV          @0x1e, AL
3f725d:   761A        EDIS         
3f725e:   761F001C    MOVW         DP, #0x1c
3f7260:   52FF        CMPB         AL, #0xff
3f7261:   56BF0138    MOVB         @0x38, #0x01, UNC
3f7263:   56B20244    MOVB         *-SP[4], #0x02, GT
3f7265:   6207        SB           7, GT
3f7266:   923B        MOV          AL, @0x3b
3f7267:   761F01A0    MOVW         DP, #0x1a0
3f7269:   56BF0144    MOVB         *-SP[4], #0x01, UNC
3f726b:   9626        MOV          @0x26, AL
3f726c:   761F001C    MOVW         DP, #0x1c
3f726e:   923A        MOV          AL, @0x3a
3f726f:   EC2D        SBF          45, EQ
3f7270:   5603013A    MOV          ACC, @0x3a << 1
3f7272:   8F000766    MOVL         XAR4, #0x000766
3f7274:   560100A4    ADDL         @XAR4, ACC
3f7276:   8AC4        MOVL         XAR4, *+XAR4[0]
3f7277:   D020        MOVB         XAR0, #0x20
3f7278:   9237        MOV          AL, @0x37
3f7279:   9694        MOV          *+XAR4[AR0], AL
3f727a:   5603013A    MOV          ACC, @0x3a << 1
3f727c:   8F000766    MOVL         XAR4, #0x000766
3f727e:   560100A4    ADDL         @XAR4, ACC
3f7280:   8AC4        MOVL         XAR4, *+XAR4[0]
3f7281:   D008        MOVB         XAR0, #0x8
3f7282:   9239        MOV          AL, @0x39
3f7283:   9694        MOV          *+XAR4[AR0], AL
3f7284:   6F18        SB           24, UNC
3f7285:   761F001C    MOVW         DP, #0x1c
3f7287:   9238        MOV          AL, @0x38
3f7288:   5203        CMPB         AL, #0x3
3f7289:   620E        SB           14, GT
3f728a:   5203        CMPB         AL, #0x3
3f728b:   56C1FF55    BF           -171, EQ
3f728d:   5200        CMPB         AL, #0x0
3f728e:   56C1FED6    BF           -298, EQ
3f7290:   5201        CMPB         AL, #0x1
3f7291:   56C1FEE5    BF           -283, EQ
3f7293:   5202        CMPB         AL, #0x2
3f7294:   56C1FF3C    BF           -196, EQ
3f7296:   6F06        SB           6, UNC
3f7297:   5204        CMPB         AL, #0x4
3f7298:   56C1FF71    BF           -143, EQ
3f729a:   5205        CMPB         AL, #0x5
3f729b:   EC80        SBF          -128, EQ
3f729c:   FF69        SPM          #0
3f729d:   9244        MOV          AL, *-SP[4]
3f729e:   FE88        SUBB         SP, #8
3f729f:   8BBE        MOVL         XAR1, *--SP
3f72a0:   0006        LRETR        