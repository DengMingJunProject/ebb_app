#ifndef __AT88SC_REG_H__
#define __AT88SC_REG_H__

///<config zone register define len+addr
#define	ATR_R			0x0800
#define FC_R			0x0208
#define MTZ_R			0x020a
#define CMC_R			0x040c
#define LHC_R			0x0810
#define DCR_R			0x0118
#define IDN_R			0x0719
#define AR0_R			0x0120
#define PR0_R			0x0121
#define AR1_R			0x0122
#define PR1_R			0x0123
#define AR2_R			0x0124
#define PR2_R			0x0125
#define AR3_R			0x0126
#define PR3_R			0x0127
#define AR4_R			0x0128
#define PR4_R			0x0129
#define AR5_R			0x012a
#define PR5_R			0x012b
#define AR6_R			0x012c
#define PR6_R			0x012d
#define AR7_R			0x012e
#define PR7_R			0x012f
#define AR8_R			0x0130
#define PR8_R			0x0131
#define AR9_R			0x0132
#define PR9_R			0x0133
#define AR10_R			0x0134
#define PR10_R			0x0135
#define AR11_R			0x0136
#define PR11_R			0x0137
#define AR12_R			0x0138
#define PR12_R			0x0139
#define AR13_R			0x013a
#define PR13_R			0x013b
#define AR14_R			0x013c
#define PR14_R			0x013d
#define AR15_R			0x013e
#define PR15_R			0x013f
#define IC_R			0x1040
#define AAC0_R			0x0150
#define CC0_R			0x0751
#define SEKS0_R			0x0858
#define AAC1_R			0x0160
#define CC1_R			0x0761
#define SEKS1_R			0x0868
#define AAC2_R			0x0170
#define CC2_R			0x0771
#define SEKS2_R			0x0878
#define AAC3_R			0x0180
#define CC3_R			0x0781
#define SEKS3_R			0x0888
#define SSG0_R			0x0890
#define SSG1_R			0x0898
#define SSG2_R			0x08a0
#define SSG3_R			0x08a8
#define PACW0_R			0x01b0
#define W0_R			0x03b1
#define PACR0_R			0x01b4
#define R0_R			0x03b5
#define PACW1_R			0x01b8
#define W1_R			0x03b9
#define PACR1_R			0x01bc
#define R1_R			0x03bd
#define PACW2_R			0x01c0
#define W2_R			0x03c1
#define PACR2_R			0x01c4
#define R2_R			0x03c5
#define PACW3_R			0x01c8
#define W3_R			0x03c9
#define PACR3_R			0x01cc
#define R3_R			0x03cd
#define PACW4_R			0x01d0
#define W4_R			0x03d1
#define PACR4_R			0x01d4
#define R4_R			0x03d5
#define PACW5_R			0x01d8
#define W5_R			0x03d9
#define PACR5_R			0x01dc
#define R5_R			0x03dd
#define PACW6_R			0x01e0
#define W6_R			0x03e1
#define PACR6_R			0x01e4
#define R6_R			0x03e5
#define PACW7_R			0x01e8
#define W7_R			0x03e9
#define PACR7_R			0x01ec
#define R7_R			0x03ed
#define reserved_R		0x10f0

enum{
	AT88SC0104C_PWD = 0xDD4297,
	AT88SC0204C_PWD = 0xE54747,
	AT88SC0404C_PWD = 0x605734,
	AT88SC0808C_PWD = 0x22E83F,
	AT88SC1616C_PWD = 0x200CE0,
	AT88SC3216C_PWD = 0xCB2850,
	AT88SC6416C_PWD = 0xF7620B,
	AT88SC12816C_PWD = 0x22EF67,
	AT88SC25616C_PWD = 0x17C33A,
};

#endif
