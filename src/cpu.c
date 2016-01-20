#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "memory.h"
#include "rom.h"
#include "interrupts.h"

#define ZERO_F 			7
#define SUBSTRACT_F		6
#define HALF_CARRY_F	5
#define CARRY_F			4


unsigned char help = 1;
unsigned char instruction = 0x00;
unsigned char operand8 = 0x00;
unsigned short operand16 = 0x0000;
unsigned char cpuCycles = 0;        //count internal cpu cycles

const struct opCode opCodes[256] = {
	{NOP,		0,	4,  "NOP"},		    // 0x00
	{LD_BC_nn,  2,  12, "LD_BC_nn" },	// 0x01
	{LD_BC_A,   0,  8, "LD_BC_A"},		// 0x02
	{INC_BC,    0,  8, "INC_BC"},		// 0x03
	{INC_B,     0,  4,  "INC_B"},		// 0x04
	{DEC_B,     0,  4,  "DEC_B"},		// 0x05
	{LD_B_n,	1,	8,  "LD_B_n"},	    // 0x06
	{tempfunction,0},		// 0x07
	{tempfunction,0},		// 0x08
	{ADD_HL_BC, 0,  8, "ADD_HL_BC"},	// 0x09
	{LD_A_BC,   0,  8, "LD_A_BC"},		// 0x0A
	{DEC_BC,    0,  8, "DEC_BC"},		// 0x0B
	{INC_C,     0,  4, "INC_C"},		// 0x0C
	{DEC_C,     0,  4, "DEC_C"},		// 0x0D
	{LD_C_n,	1,	8, "LD_C_n"},		// 0x0E
	{tempfunction,0},		// 0x0F
	{tempfunction,0},		// 0x10
	{LD_DE_nn,  2,  12, "LD_DE_nn"},	// 0x11
	{LD_DE_A,    0,  8, "LD_DE_A"},		// 0x12
	{INC_DE,    0,  8, "INC_DE"},		// 0x13
	{INC_D,     0,  4, "INC_D"},		// 0x14
	{DEC_D,     0,  4, "DEC_D"},		// 0x15
	{LD_D_n,	1,	8, "LD_D_n"},		// 0x16
	{RLA,       0,  4, "RLA"},  		// 0x17
	{JR_n,		1,	8, "JR_n"},		    // 0x18
	{ADD_HL_DE, 0,  8, "ADD_HL_DE"},	// 0x19
	{LD_A_DE,   0,  8, "LD_A_DE"},		// 0x1A
	{DEC_DE,    0,  8, "DEC_DE"},		// 0x1B
	{INC_E,     0,  4, "INC_E"},		// 0x1C
	{DEC_E,     0,  4, "DEC_E"},		// 0x1D
	{LD_E_n,	1,	8, "LD_E_n"},		// 0x1E
	{tempfunction,0},		// 0x1F
	{JR_NZ_n,	1,	8,  "JR_NZ_n"},		// 0x20
	{LD_HL_nn,  2,  12, "LD_HL_nn"},	// 0x21
	{LDI_HL_A,  0,  8,  "LDI_HL_A"},	// 0x22
	{INC_HL,    0,  8, "INC_HL"},		// 0x23
	{INC_H,     0,  4, "INC_H"},		// 0x24
	{DEC_H,     0,  4, "DEC_H"},		// 0x25
	{LD_H_n,	1,	8, "LD_H_n"},		// 0x26
	{DAA,       0,  4,  "DAA"},   		// 0x27
	{JR_Z_n,	1,	8, "JR_Z_n"},		// 0x28
	{ADD_HL_HL, 0,  8, "ADD_HL_HL"},	// 0x29
	{LDI_A_HL,  0,  8,  "LDI_A_HL"},	// 0x2A
	{DEC_HL,    0,  8, "DEC_HL"},		// 0x2B
	{INC_L,     0,  4, "INC_L"},		// 0x2C
	{DEC_L,     0,  4, "DEC_L"},		// 0x2D
	{LD_L_n,	1,	8, "LD_L_n"},		// 0x2E
	{CPL,       0,  4, "CPL"},	    	// 0x2F
	{JR_NC_n,	1,	8,  "JR_NC_n"},		// 0x30
	{LD_SP_nn,  2,  12, "LD_SP_nn"},	// 0x31
	{LDD_HL_A,  0,  8,  "LDD_HL_A"},	// 0x32
	{INC_SP,    0,  8, "INC_SP"},		// 0x33
	{INC_MHL,    0,  12, "INC_MHL"},	// 0x34
	{DEC_MHL,    0,  12, "DEC_MHL"},	// 0x35
	{LD_HL_n,    1, 12, "LD_HL_n"},	    // 0x36
	{tempfunction,0},		// 0x37
	{JR_C_n,	1,	8,  "JR_C_n"},		// 0x38
	{ADD_HL_SP, 0,  8, "ADD_HL_SP"},	// 0x39
	{tempfunction,0},		// 0x3A
	{DEC_SP,    0,  8, "DEC_SP"},		// 0x3B
	{INC_A,     0,  4, "INC_A"},		// 0x3C
	{DEC_A,     0,  4, "DEC_A"},		// 0x3D
	{LD_A_n,    1,  8, "LD_A_n"},		// 0x3E
	{CCF,       0,  4, "CCF"},  		// 0x3F
	{LD_B_B,    0,  4, "LD_B_B"},		// 0x40
	{LD_B_C,    0,  4, "LD_B_C"},		// 0x41
	{LD_B_D,    0,  4, "LD_B_D"},		// 0x42
	{LD_B_E,    0,  4, "LD_B_E"},		// 0x43
	{LD_B_H,    0,  4, "LD_B_H"},		// 0x44
	{LD_B_L,    0,  4, "LD_B_L"},		// 0x45
	{LD_B_HL,   0,  8, "LD_B_HL"},		// 0x46
	{LD_B_A,    0,  4, "LD_B_A"},		// 0x47
	{LD_C_B,    0,  4, "LD_C_B"},		// 0x48
	{LD_C_C,    0,  4, "LD_C_C"},		// 0x49
	{LD_C_D,    0,  4, "LD_C_D"},		// 0x4A
	{LD_C_E,    0,  4, "LD_C_E"},		// 0x4B
	{LD_C_H,    0,  4, "LD_C_H"},		// 0x4C
	{LD_C_L,    0,  4, "LD_C_L"},		// 0x4D
	{LD_C_HL,   0,  8, "LD_C_HL"},		// 0x4E
	{LD_C_A,    0,  4, "LD_C_A"},		// 0x4F
	{LD_D_B,    0,  4, "LD_D_B"},		// 0x50
	{LD_D_C,    0,  4, "LD_D_C"},		// 0x51
	{LD_D_D,    0,  4, "LD_D_D"},		// 0x52
	{LD_D_E,    0,  4, "LD_D_E"},		// 0x53
	{LD_D_H,    0,  4, "LD_D_H"},		// 0x54
	{LD_D_L,    0,  4, "LD_D_L"},		// 0x55
	{LD_D_HL,   0,  8, "LD_D_HL"},		// 0x56
	{LD_D_A,    0,  4, "LD_D_A"},		// 0x47
	{LD_E_B,    0,  4, "LD_E_B"},		// 0x58
	{LD_E_C,    0,  4, "LD_E_C"},		// 0x59
	{LD_E_D,    0,  4, "LD_E_D"},		// 0x5A
	{LD_E_E,    0,  4, "LD_E_E"},		// 0x5B
	{LD_E_H,    0,  4, "LD_E_H"},		// 0x5C
	{LD_E_L,    0,  4, "LD_E_L"},		// 0x5D
	{LD_E_HL,   0,  8, "LD_E_HL"},		// 0x5E
	{LD_E_A,    0,  4, "LD_E_A"},		// 0x5F
	{LD_H_B,    0,  4, "LD_H_B"},		// 0x60
	{LD_H_C,    0,  4, "LD_H_C"},		// 0x61
	{LD_H_D,    0,  4, "LD_H_D"},		// 0x62
	{LD_H_E,    0,  4, "LD_H_E"},		// 0x63
	{LD_H_H,    0,  4, "LD_H_H"},		// 0x64
	{LD_H_L,    0,  4, "LD_H_L"},		// 0x65
	{LD_H_HL,   0,  8, "LD_H_HL"},		// 0x66
	{LD_H_A,    0,  4, "LD_H_A"},		// 0x67
	{LD_L_B,    0,  4, "LD_L_B"},		// 0x68
	{LD_L_C,    0,  4, "LD_L_C"},		// 0x69
	{LD_L_D,    0,  4, "LD_L_D"},		// 0x6A
	{LD_L_E,    0,  4, "LD_L_E"},		// 0x6B
	{LD_L_H,    0,  4, "LD_L_H"},		// 0x6C
	{LD_L_L,    0,  4, "LD_L_L"},		// 0x6D
	{LD_L_HL,   0,  8, "LD_L_HL"},		// 0x6E
	{LD_L_A,    0,  4, "LD_L_A"},		// 0x6F
    {LD_HL_B,    0,  8, "LD_HL_B"},		// 0x70
	{LD_HL_C,    0,  8, "LD_HL_C"},		// 0x71
	{LD_HL_D,    0,  8, "LD_HL_D"},		// 0x72
	{LD_HL_E,    0,  8, "LD_HL_E"},		// 0x73
	{LD_HL_H,    0,  8, "LD_HL_H"},		// 0x74
	{LD_HL_L,    0,  8, "LD_HL_L"},		// 0x75
	{tempfunction,0},		// 0x76
	{LD_HL_A,    0,  8, "LD_HL_A"},		// 0x77
	{LD_A_B,    0,  4, "LD_A_B"},		// 0x78
	{LD_A_C,    0,  4, "LD_A_C"},		// 0x79
	{LD_A_D,    0,  4, "LD_A_D"},		// 0x7A
	{LD_A_E,    0,  4, "LD_A_E"},		// 0x7B
	{LD_A_H,    0,  4, "LD_A_H"},		// 0x7C
	{LD_A_L,    0,  4, "LD_A_L"},		// 0x7D
	{LD_A_HL,   0,  8, "LD_A_HL"},		// 0x7E
	{LD_A_A,    0,  4, "LD_A_A"},		// 0x7F
	{ADD_A_B,   0,  4, "ADD_A_B"},		// 0x80
	{ADD_A_C,   0,  4, "ADD_A_C"},		// 0x81
	{ADD_A_D,   0,  4, "ADD_A_D"},		// 0x82
	{ADD_A_E,   0,  4, "ADD_A_E"},		// 0x83
	{ADD_A_H,   0,  4, "ADD_A_H"},		// 0x84
	{ADD_A_L,   0,  4, "ADD_A_L"},		// 0x85
	{ADD_A_HL,  0,  8, "ADD_A_HL"},		// 0x86
	{ADD_A_A,   0,  4, "ADD_A_A"},		// 0x87
	{ADC_A_B,   0,  4, "ADC_A_B"},		// 0x88
	{ADC_A_C,   0,  4, "ADC_A_C"},		// 0x89
	{ADC_A_D,   0,  4, "ADC_A_D"},		// 0x8A
	{ADC_A_E,   0,  4, "ADC_A_E"},		// 0x8B
	{ADC_A_H,   0,  4, "ADC_A_H"},		// 0x8C
	{ADC_A_L,   0,  4, "ADC_A_L"},		// 0x8D
	{ADC_A_HL,  0,  8, "ADC_A_HL"},		// 0x8E
	{ADC_A_A,   0,  4, "ADC_A_A"},		// 0x8F
	{SUB_B,     0,  4, "SUB_B"},		// 0x90
	{SUB_C,     0,  4, "SUB_C"},		// 0x91
	{SUB_D,     0,  4, "SUB_D"},		// 0x92
	{SUB_E,     0,  4, "SUB_E"},		// 0x93
	{SUB_H,     0,  4, "SUB_H"},		// 0x94
	{SUB_L,     0,  4, "SUB_L"},		// 0x95
	{SUB_HL,    0,  8, "SUB_HL"},		// 0x96
	{SUB_A,     0,  4, "SUB_A"},		// 0x97
	{tempfunction,0},		// 0x98
	{tempfunction,0},		// 0x99
	{tempfunction,0},		// 0x9A
	{tempfunction,0},		// 0x9B
	{tempfunction,0},		// 0x9C
	{tempfunction,0},		// 0x9D
	{tempfunction,0},		// 0x9E
	{tempfunction,0},		// 0x9F
	{AND_B,     0,  4, "AND_B"},		// 0xA0
	{AND_C,     0,  4, "AND_C"},		// 0xA1
	{AND_D,     0,  4, "AND_D"},		// 0xA2
	{AND_E,     0,  4, "AND_E"},		// 0xA3
	{AND_H,     0,  4, "AND_H"},		// 0xA4
	{AND_L,     0,  4, "AND_L"},		// 0xA5
	{AND_HL,    0,  8, "AND_HL"},		// 0xA6
	{AND_A,     0,  4, "AND_A"},		// 0xA7
	{XOR_B,     0,  4, "XOR_B"},		// 0xA8
	{XOR_C,     0,  4, "XOR_C"},		// 0xA9
	{XOR_D,     0,  4, "XOR_D"},		// 0xAA
	{XOR_E,     0,  4, "XOR_E"},		// 0xAB
	{XOR_H,     0,  4, "XOR_H"},		// 0xAC
	{XOR_L,     0,  4, "XOR_L"},		// 0xAD
	{XOR_HL,    0,  8, "XOR_HL"},		// 0xAE
	{XOR_A,     0,  4, "XOR_A"},		// 0xAF
	{OR_B,      0,  4, "OR_B"}, 		// 0xB0
	{OR_C,      0,  4, "OR_C"}, 		// 0xB1
	{OR_D,      0,  4, "OR_D"}, 		// 0xB2
	{OR_E,      0,  4, "OR_E"}, 		// 0xB3
	{OR_H,      0,  4, "OR_H"}, 		// 0xB4
	{OR_L,      0,  4, "OR_L"}, 		// 0xB5
	{OR_HL,      0,  8, "OR_HL"}, 		// 0xB6
	{OR_A,       0,  4,  "OR_A"},		// 0xB7
	{CP_B,       0,  4, "CP_B"},		// 0xB8
	{CP_C,       0,  4, "CP_C"},		// 0xB9
	{CP_D,       0,  4, "CP_D"},		// 0xBA
	{CP_E,       0,  4, "CP_E"},		// 0xBB
	{CP_H,       0,  4, "CP_H"},		// 0xBC
	{CP_L,       0,  4, "CP_L"},		// 0xBD
	{CP_HL,      0,  4, "CP_HL"},		// 0xBE
	{CP_A,       0,  4, "CP_A"},		// 0xBF
	{RET_NZ,     0,  8, "RET_NZ"},	    // 0xC0
	{POP_BC,     0,  12, "POP_BC"},		// 0xC1
	{JP_NZ_nn,	2,	12, "JP_NZ_nn"},	// 0xC2
	{JP_nn,		2,	12, "JP_nn"},	    // 0xC3
	{tempfunction,0},		// 0xC4
	{PUSH_BC,   0,  16, "PUSH_BC"},		// 0xC5
	{ADD_A_n,	1,	 8, "ADD_A_n"},	    // 0xC6
	{RST00,     0,   32, "RST00"},		// 0xC7
	{RET_Z,     0,   8, "RET_Z"},       // 0xC8
//	{RET,       0,   8, "RET"},	        // 0xC9
	{RET,       0,   16, "RET"},	    // 0xC9
	{JP_Z_nn,	2,	 12, "JP_Z_nn"},	// 0xCA
	{CB,        3,   4,  "CB"}, 		// 0xCB
	{tempfunction,0},		// 0xCC
	{CALL_nn,   2,   24, "CALL_nn"},	// 0xCD
	{ADC_A_n,   1,   8, "ADC_A_n"},	    // 0xCE
	{RST08,     0,   32, "RST08"},		// 0xCF
	{RET_NC,    0,  8,  "RET_NC"},	    // 0xD0
	{POP_DE,    0,  12, "POP_DE"},		// 0xD1
	{JP_NC_nn,	2,	12, "JP_NC_nn"},	// 0xD2
	{tempfunction,0},		// 0xD3
	{tempfunction,0},		// 0xD4
	{PUSH_DE,   0,  16, "PUSH_DE"},		// 0xD5
	{SUB_n,     1,  8,  "SUB_n"},		// 0xD6
	{RST10,     0,   32, "RST10"},		// 0xD7
	{RET_C,     0,   8, "RET_C"},       // 0xD8
	{RETI,      0,   8, "RETI"},        // 0xD9
	{JP_C_nn,	2,	12, "JP_C_nn"},	    // 0xDA
	{tempfunction,0},		// 0xDB
	{tempfunction,0},		// 0xDC
	{tempfunction,0},		// 0xDD
	{tempfunction,0},		// 0xDE
	{RST18,     0,   32, "RST18"},		// 0xDF
	{LDH_n_A,   1,  12, "LDH_n_A"},		// 0xE0
	{POP_HL,    0,  12, "POP_HL"},		// 0xE1
	{LD_MC_A,   0,  8,  "LD_MC_A"},		// 0xE2
	{tempfunction,0},		// 0xE3
	{tempfunction,0},		// 0xE4
	{PUSH_HL,   0,  16, "PUSH_HL"},		// 0xE5
	{AND_n,     1,  8, "AND_n"},		// 0xE6
	{RST20,      0,   32, "RST20"},		// 0xE7
	{tempfunction,0},		// 0xE8
	{JP_HL,		0,	4, "JP_HL"},		// 0xE9
	{LD_nn_A,   2,  16, "LD_nn_A"},		// 0xEA
	{tempfunction,0},		// 0xEB
	{tempfunction,0},		// 0xEC
	{tempfunction,0},		// 0xED
	{XOR_n,     1,  8, "XOR_n"},		// 0xEE
	{RST28,     0,   32, "RST28"},		// 0xEF
	{LDH_A_n,   1,  12, "LDH_A_n"},		// 0xF0
	{POP_AF,    0,  12, "POP_AF"},		// 0xF1
	{tempfunction,0},		// 0xF2
	{DI,        0,  4, "DI"},		    // 0xF3
	{tempfunction,0},		// 0xF4
	{PUSH_AF,   0,  16, "PUSH_AF"},		// 0xF5
	{OR_n,      1,  8, "OR_n"}, 		// 0xF6
	{RST30,     0,   32, "RST30"},		// 0xF7
	{tempfunction,0},		// 0xF8
	{tempfunction,0},		// 0xF9
	{LD_A_nn,   2, 16, "LD_A_nn"},		// 0xFA
	{EI,        0, 4,  "EI"},   		// 0xFB
	{tempfunction,0},		// 0xFC
	{tempfunction,0},		// 0xFD
	{CP_n,       1,  4, "CP_n"},		// 0xFE
	{RST38,      0,   32, "RST38"},		// 0xFF
};


const struct extendedopCode extendedopCodes[256] = {
	{tempfunction,0},		    // 0x00
	{tempfunction,0},		    // 0x01
	{tempfunction,0},		    // 0x02
	{tempfunction,0},		    // 0x03
	{tempfunction,0},		    // 0x04
	{tempfunction,0},		    // 0x05
	{tempfunction,0},		    // 0x06
	{tempfunction,0},		    // 0x07
	{tempfunction,0},		    // 0x08
	{tempfunction,0},		    // 0x09
	{tempfunction,0},		    // 0x0A
	{tempfunction,0},		    // 0x0B
	{tempfunction,0},		    // 0x0C
	{tempfunction,0},		    // 0x0D
	{tempfunction,0},		    // 0x0E
	{tempfunction,0},		    // 0x0F
	{tempfunction,0},		    // 0x10
	{RL_C,        0, 8},		    // 0x11
	{tempfunction,0},		    // 0x12
	{tempfunction,0},		    // 0x13
	{tempfunction,0},		    // 0x14
	{tempfunction,0},		    // 0x15
	{tempfunction,0},		    // 0x16
	{tempfunction,0},		    // 0x17
	{tempfunction,0},		    // 0x18
	{tempfunction,0},		    // 0x19
	{tempfunction,0},		    // 0x1A
	{tempfunction,0},		    // 0x1B
	{tempfunction,0},		    // 0x1C
	{tempfunction,0},		    // 0x1D
	{tempfunction,0},		    // 0x1E
	{tempfunction,0},		    // 0x1F
	{SLA_B,       0,     8},    // 0x20
	{SLA_C,       0,     8},    // 0x21
	{SLA_D,       0,     8},    // 0x22
	{SLA_E,       0,     8},    // 0x23
	{SLA_H,       0,     8},    // 0x24
	{SLA_L,       0,     8},    // 0x25
	{tempfunction,0},           // 0x26
	{SLA_A,       0,     8},    // 0x27
	{tempfunction,0},		    // 0x28
	{tempfunction,0},		    // 0x29
	{tempfunction,0},		    // 0x22
	{tempfunction,0},		    // 0x2B
	{tempfunction,0},		    // 0x2C
	{tempfunction,0},		    // 0x2D
	{tempfunction,0},		    // 0x2E
	{tempfunction,0},		    // 0x2F
	{tempfunction,0},		    // 0x30
	{tempfunction,0},		    // 0x31
	{tempfunction,0},		    // 0x32
	{tempfunction,0},		    // 0x33
	{tempfunction,0},		    // 0x34
	{tempfunction,0},		    // 0x35
	{tempfunction,0},		    // 0x36
	{SWAP_A,          0,  8},		        // 0x37
	{tempfunction,0},		    // 0x38
	{tempfunction,0},		    // 0x39
	{tempfunction,0},		    // 0x3A
	{tempfunction,0},		    // 0x3B
	{tempfunction,0},		    // 0x3C
	{tempfunction,0},		    // 0x3D
	{tempfunction,0},		    // 0x3E
	{tempfunction,0},		    // 0x3F
	{tempfunction,0},		    // 0x40
	{tempfunction,0},		    // 0x41
	{tempfunction,0},		    // 0x42
	{tempfunction,0},		    // 0x43
	{tempfunction,0},		    // 0x44
	{tempfunction,0},		    // 0x45
	{tempfunction,0},		    // 0x46
	{tempfunction,0},		    // 0x47
	{tempfunction,0},		    // 0x48
	{tempfunction,0},		    // 0x49
	{tempfunction,0},		    // 0x4A
	{tempfunction,0},		    // 0x4B
	{tempfunction,0},		    // 0x4C
	{tempfunction,0},		    // 0x4D
	{tempfunction,0},		    // 0x4E
	{tempfunction,0},		    // 0x4F
	{tempfunction,0},		    // 0x50
	{tempfunction,0},		    // 0x51
	{tempfunction,0},		    // 0x52
	{tempfunction,0},		    // 0x53
	{tempfunction,0},		    // 0x54
	{tempfunction,0},		    // 0x55
	{tempfunction,0},		    // 0x56
	{tempfunction,0},		    // 0x47
	{tempfunction,0},		    // 0x58
	{tempfunction,0},		    // 0x59
	{tempfunction,0},		    // 0x5A
	{tempfunction,0},		    // 0x5B
	{tempfunction,0},		    // 0x5C
	{tempfunction,0},		    // 0x5D
	{tempfunction,0},		    // 0x5E
	{tempfunction,0},		    // 0x5F
	{tempfunction,0},		    // 0x60
	{tempfunction,0},		    // 0x61
	{tempfunction,0},		    // 0x62
	{tempfunction,0},		    // 0x63
	{tempfunction,0},		    // 0x64
	{tempfunction,0},		    // 0x65
	{tempfunction,0},		    // 0x66
	{tempfunction,0},		    // 0x67
	{tempfunction,0},		    // 0x68
	{tempfunction,0},		    // 0x69
	{tempfunction,0},		    // 0x6A
	{tempfunction,0},		    // 0x6B
	{tempfunction,0},		    // 0x6C
	{tempfunction,0},		    // 0x6D
	{tempfunction,0},		    // 0x6E
	{tempfunction,0},		    // 0x6F
    {tempfunction,0},		    // 0x70
	{tempfunction,0},		    // 0x71
	{tempfunction,0},		    // 0x72
	{tempfunction,0},		    // 0x73
	{tempfunction,0},		    // 0x74
	{tempfunction,0},		    // 0x75
	{tempfunction,0},		    // 0x76
	{tempfunction,0},		    // 0x77
	{tempfunction,0},		    // 0x78
	{tempfunction,0},		    // 0x79
	{tempfunction,0},		    // 0x7A
	{tempfunction,0},		    // 0x7B
	{BIT_7_H,     0,   8, "BIT_7_H"},		// 0x7C
	{tempfunction,0},		    // 0x7D
	{tempfunction,0},		    // 0x7E
	{tempfunction,0},		    // 0x7F
	{tempfunction,0},		    // 0x80
	{tempfunction,0},		    // 0x81
	{tempfunction,0},		    // 0x82
	{tempfunction,0},		    // 0x83
	{tempfunction,0},		    // 0x84
	{tempfunction,0},		    // 0x85
	{tempfunction,0},		    // 0x86
	{RES_0_A,       0,  8, "RES_0_A"},		// 0x87
	{tempfunction,0},		    // 0x88
	{tempfunction,0},		    // 0x89
	{tempfunction,0},		    // 0x8A
	{tempfunction,0},		    // 0x8B
	{tempfunction,0},		    // 0x8C
	{tempfunction,0},		    // 0x8D
	{tempfunction,0},		    // 0x8E
	{tempfunction,0},		    // 0x8F
	{tempfunction,0},		    // 0x90
	{tempfunction,0},		    // 0x91
	{tempfunction,0},		    // 0x92
	{tempfunction,0},		    // 0x93
	{tempfunction,0},		    // 0x94
	{tempfunction,0},		    // 0x95
	{tempfunction,0},		    // 0x96
	{tempfunction,0},		    // 0x97
	{tempfunction,0},		    // 0x98
	{tempfunction,0},		    // 0x99
	{tempfunction,0},		    // 0x9A
	{tempfunction,0},		    // 0x9B
	{tempfunction,0},		    // 0x9C
	{tempfunction,0},		    // 0x9D
	{tempfunction,0},		    // 0x9E
	{tempfunction,0},		    // 0x9F
	{tempfunction,0},		    // 0xA0
	{tempfunction,0},		    // 0xA1
	{tempfunction,0},		    // 0xA2
	{tempfunction,0},		    // 0xA3
	{tempfunction,0},		    // 0xA4
	{tempfunction,0},		    // 0xA5
	{tempfunction,0},		    // 0xA6
	{tempfunction,0},		    // 0xA7
	{tempfunction,0},		    // 0xA8
	{tempfunction,0},		    // 0xA9
	{tempfunction,0},		    // 0xAA
	{tempfunction,0},		    // 0xAB
	{tempfunction,0},		    // 0xAC
	{tempfunction,0},		    // 0xAD
	{tempfunction,0},		    // 0xAE
	{tempfunction,0},		    // 0xAF
	{tempfunction,0},		    // 0xB0
	{tempfunction,0},		    // 0xB1
	{tempfunction,0},		    // 0xB2
	{tempfunction,0},		    // 0xB3
	{tempfunction,0},		    // 0xB4
	{tempfunction,0},		    // 0xB5
	{tempfunction,0},		    // 0xB6
	{tempfunction,0},		    // 0xB7
	{tempfunction,0},		    // 0xB8
	{tempfunction,0},		    // 0xB9
	{tempfunction,0},		    // 0xBA
	{tempfunction,0},		    // 0xBB
	{tempfunction,0},		    // 0xBC
	{tempfunction,0},		    // 0xBD
	{tempfunction,0},		    // 0xBE
	{tempfunction,0},		    // 0xBF
	{tempfunction,0},		    // 0xC0
	{tempfunction,0},		    // 0xC1
	{tempfunction,0},		    // 0xC2
	{tempfunction,0},		    // 0xC3
	{tempfunction,0},		    // 0xC4
	{tempfunction,0},		    // 0xC5
	{tempfunction,0},		    // 0xC6
	{tempfunction,0},		    // 0xC7
	{tempfunction,0},		    // 0xC8
	{tempfunction,0},		    // 0xC9
	{tempfunction,0},		    // 0xCA
	{tempfunction,0},		    // 0xCB
	{tempfunction,0},		    // 0xCC
	{tempfunction,0},		    // 0xCD
	{tempfunction,0},		    // 0xCE
	{tempfunction,0},		    // 0xCF
	{tempfunction,0},		    // 0xD0
	{tempfunction,0},		    // 0xD1
	{tempfunction,0},		    // 0xD2
	{tempfunction,0},		    // 0xD3
	{tempfunction,0},		    // 0xD4
	{tempfunction,0},		    // 0xD5
	{tempfunction,0},		    // 0xD6
	{tempfunction,0},		    // 0xD7
	{tempfunction,0},		    // 0xD8
	{tempfunction,0},		    // 0xD9
	{tempfunction,0},		    // 0xDA
	{tempfunction,0},		    // 0xDB
	{tempfunction,0},		    // 0xDC
	{tempfunction,0},		    // 0xDD
	{tempfunction,0},		    // 0xDE
	{tempfunction,0},		    // 0xDF
	{tempfunction,0},		    // 0xE0
	{tempfunction,0},		    // 0xE1
	{tempfunction,0},		    // 0xE2
	{tempfunction,0},		    // 0xE3
	{tempfunction,0},		    // 0xE4
	{tempfunction,0},		    // 0xE5
	{tempfunction,0},		    // 0xE6
	{tempfunction,0},		    // 0xE7
	{tempfunction,0},		    // 0xE8
	{tempfunction,0},		    // 0xE9
	{tempfunction,0},		    // 0xEA
	{tempfunction,0},		    // 0xEB
	{tempfunction,0},		    // 0xEC
	{tempfunction,0},		    // 0xED
	{tempfunction,0},		    // 0xEE
	{tempfunction,0},		    // 0xEF
	{tempfunction,0},		    // 0xF0
	{tempfunction,0},		    // 0xF1
	{tempfunction,0},		    // 0xF2
	{tempfunction,0},		    // 0xF3
	{tempfunction,0},		    // 0xF4
	{tempfunction,0},		    // 0xF5
	{tempfunction,0},		    // 0xF6
	{tempfunction,0},		    // 0xF7
	{tempfunction,0},		    // 0xF8
	{tempfunction,0},		    // 0xF9
	{tempfunction,0},		    // 0xFA
	{tempfunction,0},		    // 0xFB
	{tempfunction,0},		    // 0xFC
	{tempfunction,0},		    // 0xFD
	{tempfunction,0},		    // 0xFE
	{tempfunction,0},		    // 0xFF
};

int execute (void){
	
    unsigned char operand_length;
    int extended_opcode = FALSE;

    instruction = readMemory8(registers.PC);

    if (instruction == 0xCB){
        //printf("[DEBUG] CB \n");
        instruction = readMemory8(++registers.PC);
        cpuCycles = extendedopCodes[instruction].cycles; //init cpuCycles, it may be increased after opcode execution
        operand_length = extendedopCodes[instruction].opLength;
        extended_opcode = TRUE;
    }
    else{
        //instruction = memory[registers.PC];
        cpuCycles = opCodes[instruction].cycles; //init cpuCycles, it may be increased after opcode execution
        operand_length = opCodes[instruction].opLength;
    }    


//    printf("[DEBUG] OPC-0x%04x, PC-0x%04x, SP-0x%04x, ",instruction,registers.PC,registers.SP);

	switch (operand_length){
		case 0 :
			registers.PC = registers.PC + 1;
//			printf("ARG-0x0000, ");
			break;
		case 1 :
			operand8 = readMemory8(registers.PC + 1);
			registers.PC = registers.PC + 2;
//			printf("ARG-0x%04x, ",operand8);
			break;
		case 2 :
			//operand16 = memory[registers.PC + 1] | (memory[registers.PC + 2] << 8);
            operand16 = readMemory16(registers.PC + 1);
			registers.PC = registers.PC + 3;
//			printf("ARG-0x%04x, ",operand16);
			break;
       /* case 3 :
            instruction = memory[registers.PC+1];
            registers.PC += 2; 
            ((void (*)(void))extendedopCodes[instruction].function)();
            printf("A=0x%02x, B=0x%02x, C=0x%02x, D=0x%02x, E=0x%02x, F=0x%02x, H=0x%02x, L=0x%02x, memory[HL]=0x%04x\n"
            ,registers.A,registers.B,registers.C,registers.D,registers.E,registers.F,registers.H,registers.L,memory[0xFF00]);
            return cpuCycles;
            break;
       */
	};

    if (extended_opcode){
        ((void (*)(void))extendedopCodes[instruction].function)();
    }
    else{
        ((void (*)(void))opCodes[instruction].function)();
    }
    

//            printf("A=0x%02x, B=0x%02x, C=0x%02x, D=0x%02x, E=0x%02x, F=0x%02x, H=0x%02x, L=0x%02x, memory[SP]=0x%04x\n"
//           ,registers.A,registers.B,registers.C,registers.D,registers.E,registers.F,registers.H,registers.L,memory[registers.SP + 1]);

	return cpuCycles;
	
}


	
void tempfunction(void) {
	
	printf("[ERROR] Opcode 0x%02x not implemented\nOpcode_Progress = 88\%\n[*****************===]\n",instruction);
	exit(1);

}



 /********************
 * 8-Bit Loads       *
 *********************/
/*
 * LD nn,n
 * Description: Put value nn into n.
 * Use with: nn = B,C,D,E,H,L,BC,DE,HL,SP
 *           n = 8 bit immediate value
 */
void LD_B_n (void){ registers.B = operand8; }
void LD_C_n (void){ registers.C = operand8; }
void LD_D_n (void){ registers.D = operand8; }
void LD_E_n (void){ registers.E = operand8; }
void LD_H_n (void){ registers.H = operand8; }
void LD_L_n (void){ registers.L = operand8; }

/*
 * LD r1,r2
 * Description: Put value r2 into r1.
 * Use with: r1,r2 = A,B,C,D,E,H,L,(HL)
 */
 
 void LD_B_B (void) { registers.B = registers.B;}
 void LD_B_C (void) { registers.B = registers.C;}
 void LD_B_D (void) { registers.B = registers.D;}
 void LD_B_E (void) { registers.B = registers.E;}
 void LD_B_H (void) { registers.B = registers.H;}
 void LD_B_L (void) { registers.B = registers.L;}
 void LD_B_HL (void) { registers.B = readMemory8(registers.HL);}
 void LD_C_B (void) { registers.C = registers.B;}
 void LD_C_C (void) { registers.C = registers.C;}
 void LD_C_D (void) { registers.C = registers.D;}
 void LD_C_E (void) { registers.C = registers.E;}
 void LD_C_H (void) { registers.C = registers.H;}
 void LD_C_L (void) { registers.C = registers.L;}
 void LD_C_HL (void) { registers.C = readMemory8(registers.HL);}
 void LD_D_B (void) { registers.D = registers.B;} 
 void LD_D_C (void) { registers.D = registers.C;}
 void LD_D_D (void) { registers.D = registers.D;}
 void LD_D_E (void) { registers.D = registers.E;}
 void LD_D_H (void) { registers.D = registers.H;}
 void LD_D_L (void) { registers.D = registers.L;}
 void LD_D_HL (void) { registers.D = readMemory8(registers.HL);}
 void LD_E_B (void) { registers.E = registers.B;} 
 void LD_E_C (void) { registers.E = registers.C;}
 void LD_E_D (void) { registers.E = registers.D;}
 void LD_E_E (void) { registers.E = registers.E;}
 void LD_E_H (void) { registers.E = registers.H;}
 void LD_E_L (void) { registers.E = registers.L;}
 void LD_E_HL (void) { registers.E = readMemory8(registers.HL);}
 void LD_H_B (void) { registers.H = registers.B;} 
 void LD_H_C (void) { registers.H = registers.C;}
 void LD_H_D (void) { registers.H = registers.D;}
 void LD_H_E (void) { registers.H = registers.E;}
 void LD_H_H (void) { registers.H = registers.H;}
 void LD_H_L (void) { registers.H = registers.L;}
 void LD_H_HL (void) { registers.H = readMemory8(registers.HL);}
 void LD_L_B (void) { registers.L = registers.B;} 
 void LD_L_C (void) { registers.L = registers.C;}
 void LD_L_D (void) { registers.L = registers.D;}
 void LD_L_E (void) { registers.L = registers.E;}
 void LD_L_H (void) { registers.L = registers.H;}
 void LD_L_L (void) { registers.L = registers.L;}
 void LD_L_HL (void) { registers.L = readMemory8(registers.HL);}
 void LD_HL_B (void) { writeMemory(registers.HL, registers.B);} 
 void LD_HL_C (void) { writeMemory(registers.HL, registers.C);}
 void LD_HL_D (void) { writeMemory(registers.HL, registers.D);}
 void LD_HL_E (void) { writeMemory(registers.HL, registers.E);}
 void LD_HL_H (void) { writeMemory(registers.HL, registers.H);}
 void LD_HL_L (void) { writeMemory(registers.HL, registers.L);}
 void LD_HL_n (void) { writeMemory(registers.HL, operand8);}
 
/*
 * LD A,n
 * Description: Put value n into A.
 * Use with: n = A,B,C,D,E,H,L,(BC),(DE),(HL),(nn),#
             nn = two byte immediate value. (LS byte first.)
*/
void LD_A_A  (void){registers.A = registers.A;}
void LD_A_B  (void){registers.A = registers.B;}
void LD_A_C  (void){registers.A = registers.C;}
void LD_A_D  (void){registers.A = registers.D;}
void LD_A_E  (void){registers.A = registers.E;}
void LD_A_H  (void){registers.A = registers.H;}
void LD_A_L  (void){registers.A = registers.L;}
void LD_A_BC (void){registers.A = readMemory8(registers.BC);}
void LD_A_DE (void){registers.A = readMemory8(registers.DE);}
void LD_A_HL (void){registers.A = readMemory8(registers.HL);}
void LD_A_nn (void){registers.A = readMemory8(operand16);}
void LD_A_n  (void){registers.A = operand8;}

/*
 * LD n,A
 * Description: Put value A into n.
 * Use with: n = A,B,C,D,E,H,L,(BC),(DE),(HL),(nn)
 *           nn = two byte immediate value. (LS byte first.)
 */
 void LD_B_A (void){registers.B = registers.A;}
 void LD_C_A (void){registers.C = registers.A;}
 void LD_D_A (void){registers.D = registers.A;}
 void LD_E_A (void){registers.E = registers.A;}
 void LD_H_A (void){registers.H = registers.A;}
 void LD_L_A (void){registers.L = registers.A;}
 void LD_BC_A (void){writeMemory(registers.BC, registers.A);}
 void LD_DE_A (void){writeMemory(registers.DE, registers.A);}
 void LD_HL_A (void){writeMemory(registers.HL, registers.A);}
 void LD_nn_A (void){writeMemory(operand16, registers.A);}
 
 /*
  * LD (C),A
  * Description: Put A into address $FF00 + register C.
  */
  void LD_MC_A (void) {writeMemory(0xFF00 + registers.C, registers.A);}
  
/*
 * LDD (HL),A
 * Description:
 * Put A into memory address HL. Decrement HL.
 * Same as: LD (HL),A - DEC HL
 */
void LDD_HL_A (void) {    
    writeMemory(registers.HL, registers.A);
// flags are not affected according to cinoop    
//    if (memory[registers.HL] & 0x0F)
//        resetFlag(HALF_CARRY_F);
//    else 
//        setFlag(HALF_CARRY_F);
        
    registers.HL--;
/*
question alam:
    why commented? flags will be ignored?
    or you are unsure? or typo?
*/

//    if (registers.HL == 0)
//        setFlag(ZERO_F);
//    else
//        resetFlag(ZERO_F);
//    setFlag(SUBSTRACT_F);
}

/*
 * LDI A,(HL)
 * Description: Put value at address HL into A. Increment HL.
 * Same as: LD A,(HL) - INC HL
 */
 void LDI_A_HL (void){
     registers.A = readMemory8(registers.HL);
     registers.HL++;
}

/*
 * LDI (HL),A
 * Description: Put A into memory address HL. Increment HL.
 * Same as: LD (HL),A - INC HL
 */
 void LDI_HL_A (void){
     writeMemory(registers.HL, registers.A);
     registers.HL++;
 }
 
/*
 * LDH (n),A
 * Description: Put A into memory address $FF00+n.
 * Use with: n = one byte immediate value.
 */
 //void LDH_n_A (void) { memory[0xFF00+operand8] = registers.A; }
void LDH_n_A (void) { 
    writeMemory(0xFF00 + operand8, registers.A);
} //hardcoded
/*
 * LDH A,(n)
 * Description: Put memory address $FF00+n into A.
 * Use with: n = one byte immediate value.
 */
  void LDH_A_n (void) { registers.A = readMemory8(0xFF00 + operand8); 
  }
 
/********************
 * 16-Bit Loads     *
 ********************/
/*
 * LD n,nn
 * Description: Put value nn into n.
 * Use with: n = BC,DE,HL,SP
 *           nn = 16 bit immediate value
 */
void LD_BC_nn (void) { registers.BC = operand16; }
void LD_DE_nn (void) { registers.DE = operand16; }
void LD_HL_nn (void) { registers.HL = operand16; }
void LD_SP_nn (void) { registers.SP = operand16; }

/*
 * PUSH nn
 * Description: Push register pair nn onto stack.
 * Decrement Stack Pointer (SP) twice.
 * Use with: nn = AF,BC,DE,HL
 */
 void PUSH_AF (void){stackPush16(registers.AF);}
 void PUSH_BC (void){stackPush16(registers.BC);}
 void PUSH_DE (void){stackPush16(registers.DE);}
 void PUSH_HL (void){stackPush16(registers.HL);}
 
/*
 * POP nn
 * Description: Pop two bytes off stack into register pair nn.
 * Increment Stack Pointer (SP) twice.
 * Use with: nn = AF,BC,DE,HL
 */
 void POP_AF (void){registers.AF = stackPop16();}
 void POP_BC (void){registers.BC = stackPop16();}
 void POP_DE (void){registers.DE = stackPop16();}
 void POP_HL (void){registers.HL = stackPop16();}

/********************
 * 8-Bit ALU        *
 ********************/
/*
 * ADD A,n
 * Description: Add n to A.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Set if carry from bit 3.
 * C - Set if carry from bit 7.
 */
void ADD_A_A (void){ add (registers.A, registers.A); }
void ADD_A_B (void){ add (registers.A, registers.B); }
void ADD_A_C (void){ add (registers.A, registers.C); }
void ADD_A_D (void){ add (registers.A, registers.D); }
void ADD_A_E (void){ add (registers.A, registers.E); }
void ADD_A_H (void){ add (registers.A, registers.H); }
void ADD_A_L (void){ add (registers.A, registers.L); }
void ADD_A_HL (void){ add (registers.A, readMemory8(registers.HL)); }
void ADD_A_n (void){ add (registers.A, operand8); }
/*
 * ADC A,n
 * Description: Add n + Carry flag to A.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Set if carry from bit 3.
 * C - Set if carry from bit 7.
 */
void ADC_A_A (void){ adc (registers.A, registers.A); }
void ADC_A_B (void){ adc (registers.A, registers.B); }
void ADC_A_C (void){ adc (registers.A, registers.C); }
void ADC_A_D (void){ adc (registers.A, registers.D); }
void ADC_A_E (void){ adc (registers.A, registers.E); }
void ADC_A_H (void){ adc (registers.A, registers.H); }
void ADC_A_L (void){ adc (registers.A, registers.L); }
void ADC_A_HL (void){ adc (registers.A, readMemory8(registers.HL)); }
void ADC_A_n (void){ adc (registers.A, operand8); }
/*
 * SUB n
 * Description: Subtract n from A.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero.
 * N - Set.
 * H - Set if no borrow from bit 4.
 * C - Set if no borrow.
 */
 
 void SUB_A (void) { sub (registers.A);}
 void SUB_B (void) { sub (registers.B);}
 void SUB_C (void) { sub (registers.C);}
 void SUB_D (void) { sub (registers.D);}
 void SUB_E (void) { sub (registers.E);}
 void SUB_H (void) { sub (registers.H);}
 void SUB_L (void) { sub (registers.L);}
 void SUB_HL (void) { sub (readMemory8(registers.HL));}
 void SUB_n (void) { sub (operand8);}
 
/*
 * XOR n
 * Description: Logical exclusive OR n with register A, result in A.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Reset.
 */
void XOR_A (void) { xor (registers.A); }
void XOR_B (void) { xor (registers.B); }
void XOR_C (void) { xor (registers.C); }
void XOR_D (void) { xor (registers.D); }
void XOR_E (void) { xor (registers.E); }
void XOR_H (void) { xor (registers.H); }
void XOR_L (void) { xor (registers.L); }
void XOR_HL (void) { xor (readMemory8(registers.HL)); }
void XOR_n (void) { xor (operand8); }

/*
 * AND n
 * Description: Logically AND n with A, result in A.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Set.
 * C - Reset.
 */
 void AND_A (void) {cpu_and (registers.A);}
 void AND_B (void) {cpu_and (registers.B);}
 void AND_C (void) {cpu_and (registers.C);}
 void AND_D (void) {cpu_and (registers.D);}
 void AND_E (void) {cpu_and (registers.E);}
 void AND_H (void) {cpu_and (registers.H);}
 void AND_L (void) {cpu_and (registers.L);}
 void AND_HL (void) {cpu_and (readMemory8(registers.HL));}
 void AND_n (void) {cpu_and (operand8);}

 
/*
 * OR n
 * Description: Logical OR n with register A, result in A.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Reset.
 */
 void OR_A (void) {or (registers.A);}
 void OR_B (void) {or (registers.B);}
 void OR_C (void) {or (registers.C);}
 void OR_D (void) {or (registers.D);}
 void OR_E (void) {or (registers.E);}
 void OR_H (void) {or (registers.H);}
 void OR_L (void) {or (registers.L);}
 void OR_HL (void) {or (readMemory8(registers.HL));}
 void OR_n (void) {or (operand8);}
 
/*
 * CP n
 * Description: Compare A with n. This is basically an A - n
 *              subtraction instruction but the results are thrown away.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero. (Set if A = n.)
 * N - Set.
 * H - Set if no borrow from bit 4.
 * C - Set for no borrow. (Set if A < n.)
*/
 void CP_A (void) {comp (registers.A);}
 void CP_B (void) {comp (registers.B);}
 void CP_C (void) {comp (registers.C);}
 void CP_D (void) {comp (registers.D);}
 void CP_E (void) {comp (registers.E);}
 void CP_H (void) {comp (registers.H);}
 void CP_L (void) {comp (registers.L);}
 void CP_HL (void) {comp (readMemory8(registers.HL));}
 void CP_n (void) {comp (operand8);}
 
/*
 * INC n
 * Description: Increment register n.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Set if carry from bit 3.
 * C - Not affected.
 */
 
 void INC_A (void) {inc (&registers.A);}
 void INC_B (void) {inc (&registers.B);}
 void INC_C (void) {inc (&registers.C);}
 void INC_D (void) {inc (&registers.D);}
 void INC_E (void) {inc (&registers.E);}
 void INC_H (void) {inc (&registers.H);}
 void INC_L (void) {inc (&registers.L);}
 void INC_MHL (void) {inc (&memory[registers.HL]);}
/*
 * DEC n
 * Description: Decrement register n.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Set.
 * H - Set if no borrow from bit 4.
 * C - Not affected.
 */
void DEC_A (void) {dec (&registers.A);}
void DEC_B (void) {dec (&registers.B);}
void DEC_C (void) {dec (&registers.C);}
void DEC_D (void) {dec (&registers.D);}
void DEC_E (void) {dec (&registers.E);}
void DEC_H (void) {dec (&registers.H);}
void DEC_L (void) {dec (&registers.L);}
void DEC_MHL (void) {dec (&memory[registers.HL]);

//    if (memory[registers.HL] == 0)
//        setFlag(ZERO_F);

//memory[registers.HL] -= 1;   

//    if (memory[registers.HL] == 0)
//        setFlag(ZERO_F);
//    else
//        resetFlag(ZERO_F);
//    setFlag(SUBSTRACT_F);

// flags are not affected according to cinoop
// flags indeed are affected
// if (registers.HL == 0xF)
//     setFlag(HALF_CARRY_F);
}
/********************
 * 16-Bit Arithmetic*
 ********************/
/*
 * ADD HL,n
 * Description: Add n to HL.
 * Use with: n = BC,DE,HL,SP
 * Flags affected:
 * Z - Not affected.
 * N - Reset.
 * H - Set if carry from bit 11.
 * C - Set if carry from bit 15.
 */
 void ADD_HL_BC (void) {add16 (registers.BC);}
 void ADD_HL_DE (void) {add16 (registers.DE);}
 void ADD_HL_HL (void) {add16 (registers.HL);}
 void ADD_HL_SP (void) {add16 (registers.SP);}
 
/*
 * INC nn
 * Description: Increment register nn.
 * Use with: nn = BC,DE,HL,SP
 * Flags affected:
 * None.
 */
 void INC_BC (void) {registers.BC++;}
 void INC_DE (void) {registers.DE++;}
 void INC_HL (void) {registers.HL++;}
 void INC_SP (void) {registers.SP++;}
/*
 * DEC nn
 * Description: Decrement register nn.
 * Use with: nn = BC,DE,HL,SP
 * Flags affected: None.
 */
 
 void DEC_BC (void) {registers.BC--;}
 void DEC_DE (void) {registers.DE--;}
 void DEC_HL (void) {registers.HL--;}
 void DEC_SP (void) {registers.SP--;}
 
 /*******************
 * ADD, INC, DEC    *
 ********************/
/********************
 * Miscellaneous    *
 ********************/
/*
 * Description: Decimal adjust register A.
 * This instruction adjusts register A so that the
 * correct representation of Binary Coded Decimal (BCD)
 * is obtained.
 * Flags affected:
 * Z - Set if register A is zero.
 * N - Not affected.
 * H - Reset.
 * C - Set or reset according to operation.
 */
/*  When this instruction is executed, the A register 
    is BCD corrected using the contents of the flags. 
    The exact process is the following: if the least 
    significant four bits of A contain a non-BCD digit 
    (i. e. it is greater than 9) or the H flag is set, 
    then $06 is added to the register. Then the four 
    most significant bits are checked. If this more 
    significant digit also happens to be greater than 9 
    or the C flag is set, then $60 is added.
*/

void DAA (void){
    unsigned char H0,H1;
    unsigned short value = registers.A;
   
    H0 = registers.A & 0x0F;
    H1 = (registers.A & 0xF0) >> 4;
    // if the lower 4 bits form a number greater than 9 or H is set, add $06 to the accumulator
    if ( (H0 > 0x09) || ( testFlag( HALF_CARRY_F ) == TRUE )){
        value += 0x06;
    }
    // if the upper 4 bits form a number greater than 9 or C is set, add $60 to the accumulator
    if ( (H1 > 0x09) || ( testFlag( CARRY_F ) == TRUE )){
        value += 0x60;
    }
    
    if (value >= 0x100){
        setFlag(CARRY_F);
    }
    else{
        resetFlag(CARRY_F);
    }
    
    resetFlag(HALF_CARRY_F);
    
    if (registers.A){
        resetFlag(ZERO_F);
    }
    else{
        setFlag(ZERO_F);
    }
    
    registers.A = value & 0xFF;
}

/*
 * CPL
 * Description: Complement A register. (Flip all bits.)
 * Flags affected:
 * Z - Not affected.
 * N - Set.
 * H - Set.
 * C - Not affected. 
 */
 void CPL (void) {
     registers.A = ~registers.A;
     setFlag(SUBSTRACT_F);
     setFlag(HALF_CARRY_F);
}
  
/*
 * CCF
 * Description: Complement carry flag.
 * If C flag is set, then reset it.
 * If C flag is reset, then set it.
 * Flags affected:
 * Z - Not affected.
 * N - Reset.
 * H - Reset.
 * C - Complemented.
 */
 void CCF (void){
     resetFlag(SUBSTRACT_F);
     resetFlag(HALF_CARRY_F);
     registers.F = (registers.F)|((~(registers.F & 0x10)) & 0x10);
 }

/*
 * NOP
 * Description: No operation.
 */
 void NOP (void){ };
/*
 * DI
 * Description:
 * This instruction disables interrupts but not immediately. Interrupts are disabled after
 * instruction after DI is executed.
 * Flags affected: None.
 *
*/
 void DI (void)     { interruptMaster = FALSE; }
/*
 * EI
 * Description: Enable interrupts. This intruction enables interrupts
 * but not immediately. Interrupts are enabled after instruction after EI is executed.
 * Flags affected: None.
 */
 void EI (void) {interruptMaster = TRUE;}
/********************
 * Rotates & Shifts *
 ********************/
/*
 * SLA n
 * Description: Shift n left into Carry. LSB of n set to 0.
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
void SLA_A (void) {registers.A = sla(registers.A);}
void SLA_B (void) {registers.B = sla(registers.B);}
void SLA_C (void) {registers.C = sla(registers.C);}
void SLA_D (void) {registers.D = sla(registers.D);}
void SLA_E (void) {registers.E = sla(registers.E);}
void SLA_H (void) {registers.H = sla(registers.H);}
void SLA_L (void) {registers.L = sla(registers.L);}
/*
 * RLA
 * Description: Rotate A left through Carry flag.
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
 
 void RLA (void) {
//     cinoop sucks
//     int carry = testFlag(CARRY_F) ? 1 : 0;
	int carry = testFlag(CARRY_F);
	if (registers.A & 0x80) 
       setFlag(CARRY_F);
	else 
       resetFlag(CARRY_F);
	
	registers.A <<= 1;
	registers.A += carry;
	
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
 }
/********************
 * Bit Opcodes      *
 ********************/
/********************
 * Jumps            *
 ********************/
void JP_nn (void)	{ registers.PC = operand16; }
void JP_NZ_nn (void){ if (testFlag(ZERO_F)  == 0){ registers.PC = operand16; cpuCycles += 4;}}
void JP_Z_nn (void)	{ if (testFlag(ZERO_F)  == 1){ registers.PC = operand16; cpuCycles += 4;}}
void JP_NC_nn (void){ if (testFlag(CARRY_F) == 0){ registers.PC = operand16; cpuCycles += 4;}}
void JP_C_nn (void)	{ if (testFlag(CARRY_F) == 1){ registers.PC = operand16; cpuCycles += 4;}}
void JP_HL (void)	{ registers.PC = registers.HL; }
void JR_n (void)	{ registers.PC = registers.PC + (signed char)operand8; }
void JR_NZ_n (void)	{ if (testFlag(ZERO_F)  == 0){ registers.PC = registers.PC + (signed char)operand8; cpuCycles += 4;}}
void JR_Z_n (void)	{ if (testFlag(ZERO_F)  == 1){ registers.PC = registers.PC + (signed char)operand8; cpuCycles += 4;}}
void JR_NC_n (void)	{ if (testFlag(CARRY_F) == 0){ registers.PC = registers.PC + (signed char)operand8; cpuCycles += 4;}}
void JR_C_n (void)	{ if (testFlag(CARRY_F) == 1){ registers.PC = registers.PC + (signed char)operand8; cpuCycles += 4;}}
/********************
 * Calls            *
 ********************/
/*
 * CALL nn
 * Description: Push address of next instruction onto stack and then
 *              jump to address nn.
 * Use with: nn = two byte immediate value. (LS byte first.)
 */
 
 void CALL_nn (void) { 
     stackPush16(registers.PC);
     registers.PC = operand16;
 }
/********************
 * Restarts         *
 ********************/
 
/*
 * RST n
 * Description: Push present address onto stack.
 * Jump to address $0000 + n.
 * Use with: n = $00,$08,$10,$18,$20,$28,$30,$38
 */
 void RST00 (void) {
     stackPush16(registers.PC);
     registers.PC = 0x00;
}
 void RST08 (void) {
     stackPush16(registers.PC);
     registers.PC = 0x08;
}
 void RST10 (void) {
     stackPush16(registers.PC);
     registers.PC = 0x10;
}
 void RST18 (void) {
     stackPush16(registers.PC);
     registers.PC = 0x18;
}
 void RST20 (void) {
     stackPush16(registers.PC);
     registers.PC = 0x20;
}
 void RST28 (void) {
     stackPush16(registers.PC);
     registers.PC = 0x28;
}
 void RST30 (void) {
     stackPush16(registers.PC);
     registers.PC = 0x30;
}
 void RST38 (void) {
     stackPush16(registers.PC);
     registers.PC = 0x38;
}
 
/********************
 * Returns          *
 ********************/
/*
 * RET
 * Description: Pop two bytes from stack & jump to that address.
 */
void RET (void){registers.PC = stackPop16();}
/*
 * RET cc
 * Description: Return if following condition is true:
 * Use with:
 * cc = NZ, Return if Z flag is reset.
 * cc = Z, Return if Z flag is set.
 * cc = NC, Return if C flag is reset.
 */
void RET_NZ (void){ if (testFlag(ZERO_F)  == 0){ registers.PC = stackPop16(); cpuCycles += 12;}}
void RET_Z  (void){ if (testFlag(ZERO_F)  == 1){ registers.PC = stackPop16(); cpuCycles += 12;}}
void RET_NC (void){ if (testFlag(CARRY_F) == 0){ registers.PC = stackPop16(); cpuCycles += 12;}}
void RET_C  (void){ if (testFlag(CARRY_F) == 1){ registers.PC = stackPop16(); cpuCycles += 12;}}
/*
 * RETI
 * Description: Pop two bytes from stack & jump to that address then
 * enable interrupts.
 */
void RETI   (void){ registers.PC = stackPop16(); interruptMaster = TRUE;}

/************************
 * Extended instructions*
 ************************/
void CB (void) {printf("OK");}

void SWAP_A (void) {
    
    registers.A = ((registers.A & 0xf) << 4) | ((registers.A & 0xf0) >> 4);
    if(registers.A == 0)
       setFlag(ZERO_F);
    else
       resetFlag(ZERO_F);
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    resetFlag(CARRY_F);
    cpuCycles += 8;
}

void RES_0_A (void) {
    registers.A &= 0xFE;
//    cinoop sucks
//    registers.A &= ~(1 << 0);
    cpuCycles += 8;
}

void BIT_7_H (void) {
    
    if((1 << 7)&registers.H)
        resetFlag(ZERO_F);
    else
        setFlag(ZERO_F);
   
    
    resetFlag(SUBSTRACT_F);
    setFlag(HALF_CARRY_F);
    cpuCycles += 8;
}

void  RL_C (void){
//cinoop sucks    
//    	int carry = testFlag(CARRY_F) ? 1 : 0;
    	int carry = testFlag(CARRY_F);
        if (registers.C&0x80)
           setFlag(CARRY_F);
        else
           resetFlag(CARRY_F);
         
        registers.C <<=1;
        registers.C +=carry;
        
        if (registers.C)
            resetFlag(ZERO_F);
        else
            setFlag(ZERO_F);
            
        resetFlag(SUBSTRACT_F);
        resetFlag(HALF_CARRY_F);
        cpuCycles += 8; 

}