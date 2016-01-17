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

unsigned char instruction = 0x00;
unsigned char operand8 = 0x00;
unsigned short operand16 = 0x0000;
unsigned char cpuCycles = 0;        //count internal cpu cycles

const struct opCode opCodes[256] = {
	{NOP,		0,	4,  "NOP"},		    // 0x00
	{LD_BC_nn,  2,  12, "LD_BC_nn" },	// 0x01
	{LD_BC_A,    0,  8, "LD_BC_A"},		// 0x02
	{tempfunction,0},		// 0x03
	{INC_B,     0,  4,  "INC_B"},		// 0x04
	{DEC_B,     0,  4,  "DEC_B"},		// 0x05
	{LD_B_n,	1,	8,  "LD_B_n"},	    // 0x06
	{tempfunction,0},		// 0x07
	{tempfunction,0},		// 0x08
	{tempfunction,0},		// 0x09
	{LD_A_BC,   0,  8, "LD_A_BC"},		// 0x0A
	{DEC_BC,    0,  8, "DEC_BC"},		// 0x0B
	{INC_C,     0,  4, "INC_C"},		// 0x0C
	{DEC_C,     0,  4, "DEC_C"},		// 0x0D
	{LD_C_n,	1,	8, "LD_C_n"},		// 0x0E
	{tempfunction,0},		// 0x0F
	{tempfunction,0},		// 0x10
	{LD_DE_nn,  2,  12, "LD_DE_nn"},	// 0x11
	{LD_DE_A,    0,  8, "LD_DE_A"},		// 0x12
	{tempfunction,0},		// 0x13
	{INC_D,     0,  4, "INC_D"},		// 0x14
	{DEC_D,     0,  4, "DEC_D"},		// 0x15
	{LD_D_n,	1,	8, "LD_D_n"},		// 0x16
	{tempfunction,0},		// 0x17
	{JR_n,		1,	8, "JR_n"},		    // 0x18
	{tempfunction,0},		// 0x19
	{LD_A_DE,   0,  8, "LD_A_DE"},		// 0x1A
	{DEC_DE,    0,  8, "DEC_DE"},		// 0x1B
	{INC_E,     0,  4, "INC_E"},		// 0x1C
	{DEC_E,     0,  4, "DEC_E"},		// 0x1D
	{LD_E_n,	1,	8, "LD_E_n"},		// 0x1E
	{tempfunction,0},		// 0x1F
	{JR_NZ_n,	1,	8,  "JR_NZ_n"},		// 0x20
	{LD_HL_nn,  2,  12, "LD_HL_nn"},	// 0x21
	{LDI_HL_A,  0,  8,  "LDI_HL_A"},	// 0x22
	{tempfunction,0},		// 0x23
	{INC_H,     0,  4, "INC_H"},		// 0x24
	{DEC_H,     0,  4, "DEC_H"},		// 0x25
	{LD_H_n,	1,	8, "LD_H_n"},		// 0x26
	{tempfunction,0},		// 0x27
	{JR_Z_n,	1,	8, "JR_Z_n"},		// 0x28
	{tempfunction,0},		// 0x29
	{LDI_A_HL,  0,  8,  "LDI_A_HL"},	// 0x22
	{DEC_HL,    0,  8, "DEC_HL"},		// 0x2B
	{INC_L,     0,  4, "INC_L"},		// 0x2C
	{DEC_L,     0,  4, "DEC_L"},		// 0x2D
	{LD_L_n,	1,	8, "LD_L_n"},		// 0x2E
	{CPL,       0,  4, "CPL"},	    	// 0x2F
	{JR_NC_n,	1,	8,  "JR_NC_n"},		// 0x30
	{LD_SP_nn,  2,  12, "LD_SP_nn"},	// 0x31
	{LDD_HL_A,  0,  8,  "LDD_HL_A"},	// 0x32
	{tempfunction,0},		// 0x33
	{INC_HL,    0,  12, "INC_C"},		// 0x34
	{DEC_MHL,    0,  12, "DEC_MHL"},	// 0x35
	{LD_HL_n,    1, 12, "LD_HL_n"},	    // 0x36
	{tempfunction,0},		// 0x37
	{JR_C_n,	1,	8,  "JR_C_n"},		// 0x38
	{tempfunction,0},		// 0x39
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
	{tempfunction,0},		// 0xC7
	{RET_Z,     0,   8, "RET_Z"},       // 0xC8
//	{RET,       0,   8, "RET"},	        // 0xC9
	{RET,       0,   16, "RET"},	    // 0xC9
	{JP_Z_nn,	2,	 12, "JP_Z_nn"},	// 0xCA
	{tempfunction,0},		// 0xCB
	{tempfunction,0},		// 0xCC
	{CALL_nn,   2,   24, "CALL_nn"},	// 0xCD
	{ADC_A_n,   1,   8, "ADC_A_n"},	    // 0xCE
	{tempfunction,0},		// 0xCF
	{RET_NC,    0,  8,  "RET_NC"},	    // 0xD0
	{POP_DE,    0,  12, "POP_DE"},		// 0xD1
	{JP_NC_nn,	2,	12, "JP_NC_nn"},	// 0xD2
	{tempfunction,0},		// 0xD3
	{tempfunction,0},		// 0xD4
	{PUSH_DE,   0,  16, "PUSH_DE"},		// 0xD5
	{SUB_n,     1,  8,  "SUB_n"},		// 0xD6
	{tempfunction,0},		// 0xD7
	{RET_C,     0,   8, "RET_C"},       // 0xD8
	{RETI,      0,   8, "RETI"},        // 0xD9
	{JP_C_nn,	2,	12, "JP_C_nn"},	    // 0xDA
	{tempfunction,0},		// 0xDB
	{tempfunction,0},		// 0xDC
	{tempfunction,0},		// 0xDD
	{tempfunction,0},		// 0xDE
	{tempfunction,0},		// 0xDF
	{LDH_n_A,   1,  12, "LDH_n_A"},		// 0xE0
	{POP_HL,    0,  12, "POP_HL"},		// 0xE1
	{LD_MC_A,   0,  8,  "LD_MC_A"},		// 0xE2
	{tempfunction,0},		// 0xE3
	{tempfunction,0},		// 0xE4
	{PUSH_HL,   0,  16, "PUSH_HL"},		// 0xE5
	{AND_n,     1,  8, "AND_n"},		// 0xE6
	{tempfunction,0},		// 0xE7
	{tempfunction,0},		// 0xE8
	{JP_HL,		0,	4, "JP_HL"},		// 0xE9
	{LD_nn_A,   2,  16, "LD_nn_A"},		// 0xEA
	{tempfunction,0},		// 0xEB
	{tempfunction,0},		// 0xEC
	{tempfunction,0},		// 0xED
	{XOR_n,     1,  8, "XOR_n"},		// 0xEE
	{tempfunction,0},		// 0xEF
	{LDH_A_n,   1,  12, "LDH_A_n"},		// 0xF0
	{POP_AF,    0,  12, "POP_AF"},		// 0xF1
	{tempfunction,0},		// 0xF2
	{DI,        0,  4, "DI"},		    // 0xF3
	{tempfunction,0},		// 0xF4
	{PUSH_AF,   0,  16, "PUSH_AF"},		// 0xF5
	{OR_n,      1,  8, "OR_n"}, 		// 0xF6
	{tempfunction,0},		// 0xF7
	{tempfunction,0},		// 0xF8
	{tempfunction,0},		// 0xF9
	{LD_A_nn,   2, 16, "LD_A_nn"},		// 0xFA
	{EI,        0, 4,  "EI"},   		// 0xFB
	{tempfunction,0},		// 0xFC
	{tempfunction,0},		// 0xFD
	{CP_n,       1,  4, "CP_n"},		// 0xFE
	{tempfunction,0},		// 0xFF
};


int execute (void){
	
    
	instruction = memory[registers.PC];
    cpuCycles = opCodes[instruction].cycles; //init cpuCycles, it may be increased after opcode execution
	
	//printf("[DEBUG] Opcode    - 0x%04x, P counter - 0x%04x, S pointer - 0x%04x\n",memory[registers.PC],registers.PC,registers.SP);
    // printf("[DEBUG] Registers - A=0x%02x, B=0x%02x, C=0x%02x, D=0x%02x, E=0x%02x, F=0x%02x, H=0x%02x, L=0x%02x\n"
    //             ,registers.A,registers.B,registers.C,registers.D,registers.E,registers.F,registers.H,registers.L);
    //printf("[DEBUG] OPC-0x%04x-[%s],\tPC-0x%04x, SP-0x%04x, ",instruction,opCodes[instruction].function_name,registers.PC,registers.SP);

    printf("[DEBUG] OPC-0x%04x, PC-0x%04x, SP-0x%04x, ",instruction,registers.PC,registers.SP);
	switch (opCodes[instruction].opLength){
		case 0 :
			registers.PC = registers.PC + 1;
			printf("ARG-0x0000, ");
			break;
		case 1 :
			operand8 = memory[registers.PC + 1];
			registers.PC = registers.PC + 2;
			printf("ARG-0x%04x, ",operand8);
			break;
		case 2 :
			operand16 = memory[registers.PC + 1] | (memory[registers.PC + 2] << 8);
			registers.PC = registers.PC + 3;
			printf("ARG-0x%04x, ",operand16);
			break;
	};
	
	//opCodes[memory[registers.PC]].function();
	((void (*)(void))opCodes[instruction].function)();

    printf("A=0x%02x, B=0x%02x, C=0x%02x, D=0x%02x, E=0x%02x, F=0x%02x, H=0x%02x, L=0x%02x\n"
        ,registers.A,registers.B,registers.C,registers.D,registers.E,registers.F,registers.H,registers.L);


	return cpuCycles;
	
}


	
void tempfunction(void) {
	
	printf("[ERROR] Opcode 0x%02x not implemented\nOpcode_Progress = 82\%\n[****************====]\n",instruction);
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
 void LD_B_HL (void) { registers.B = memory[registers.HL];}
 void LD_C_B (void) { registers.C = registers.B;}
 void LD_C_C (void) { registers.C = registers.C;}
 void LD_C_D (void) { registers.C = registers.D;}
 void LD_C_E (void) { registers.C = registers.E;}
 void LD_C_H (void) { registers.C = registers.H;}
 void LD_C_L (void) { registers.C = registers.L;}
 void LD_C_HL (void) { registers.C = memory[registers.HL];}
 void LD_D_B (void) { registers.D = registers.B;} 
 void LD_D_C (void) { registers.D = registers.C;}
 void LD_D_D (void) { registers.D = registers.D;}
 void LD_D_E (void) { registers.D = registers.E;}
 void LD_D_H (void) { registers.D = registers.H;}
 void LD_D_L (void) { registers.D = registers.L;}
 void LD_D_HL (void) { registers.D = memory[registers.HL];}
 void LD_E_B (void) { registers.E = registers.B;} 
 void LD_E_C (void) { registers.E = registers.C;}
 void LD_E_D (void) { registers.E = registers.D;}
 void LD_E_E (void) { registers.E = registers.E;}
 void LD_E_H (void) { registers.E = registers.H;}
 void LD_E_L (void) { registers.E = registers.L;}
 void LD_E_HL (void) { registers.E = memory[registers.HL];}
 void LD_H_B (void) { registers.H = registers.B;} 
 void LD_H_C (void) { registers.H = registers.C;}
 void LD_H_D (void) { registers.H = registers.D;}
 void LD_H_E (void) { registers.H = registers.E;}
 void LD_H_H (void) { registers.H = registers.H;}
 void LD_H_L (void) { registers.H = registers.L;}
 void LD_H_HL (void) { registers.H = memory[registers.HL];}
 void LD_L_B (void) { registers.L = registers.B;} 
 void LD_L_C (void) { registers.L = registers.C;}
 void LD_L_D (void) { registers.L = registers.D;}
 void LD_L_E (void) { registers.L = registers.E;}
 void LD_L_H (void) { registers.L = registers.H;}
 void LD_L_L (void) { registers.L = registers.L;}
 void LD_L_HL (void) { registers.L = memory[registers.HL];}
 void LD_HL_B (void) { memory[registers.HL] = registers.B;} 
 void LD_HL_C (void) { memory[registers.HL] = registers.C;}
 void LD_HL_D (void) { memory[registers.HL] = registers.D;}
 void LD_HL_E (void) { memory[registers.HL] = registers.E;}
 void LD_HL_H (void) { memory[registers.HL] = registers.H;}
 void LD_HL_L (void) { memory[registers.HL] = registers.L;}
 void LD_HL_n (void) { memory[registers.HL] = operand8;}
 
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
void LD_A_BC (void){registers.A = memory[registers.BC];}
void LD_A_DE (void){registers.A = memory[registers.DE];}
void LD_A_HL (void){registers.A = memory[registers.HL];}
void LD_A_nn (void){registers.A = memory[operand16];}
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
 void LD_BC_A (void){memory[registers.HL] = registers.A;}
 void LD_DE_A (void){memory[registers.HL] = registers.A;}
 void LD_HL_A (void){memory[registers.HL] = registers.A;}
 void LD_nn_A (void){memory[operand16] = registers.A;}
 
 /*
  * LD (C),A
  * Description: Put A into address $FF00 + register C.
  */
  void LD_MC_A (void) {memory[0xFF00+registers.C] = registers.A;}
  
/*
 * LDD (HL),A
 * Description:
 * Put A into memory address HL. Decrement HL.
 * Same as: LD (HL),A - DEC HL
 */
void LDD_HL_A (void) {    
    memory[registers.HL] = registers.A;
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
     registers.A = memory[registers.HL];
     registers.HL++;
}

/*
 * LDI (HL),A
 * Description: Put A into memory address HL. Increment HL.
 * Same as: LD (HL),A - INC HL
 */
 void LDI_HL_A (void){
     memory[registers.HL] = registers.A;
     registers.HL++;
 }
 
/*
 * LDH (n),A
 * Description: Put A into memory address $FF00+n.
 * Use with: n = one byte immediate value.
 */
 void LDH_n_A (void) { memory[0xFF00+operand8] = registers.A; }
/*
 * LDH A,(n)
 * Description: Put memory address $FF00+n into A.
 * Use with: n = one byte immediate value.
 */
  void LDH_A_n (void) { registers.A = memory[0xFF00+operand8]; }
 
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
 void POP_AF (void){registers.HL = stackPop16();}
 void POP_BC (void){registers.HL = stackPop16();}
 void POP_DE (void){registers.HL = stackPop16();}
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
void ADD_A_HL (void){ add (registers.A, memory[registers.HL]); }
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
void ADC_A_HL (void){ adc (registers.A, memory[registers.HL]); }
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
 void SUB_HL (void) { sub (memory[registers.HL]);}
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
void XOR_HL (void) { xor (memory[registers.HL]); }
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
 void AND_A (void) {and (registers.A);}
 void AND_B (void) {and (registers.B);}
 void AND_C (void) {and (registers.C);}
 void AND_D (void) {and (registers.D);}
 void AND_E (void) {and (registers.E);}
 void AND_H (void) {and (registers.H);}
 void AND_L (void) {and (registers.L);}
 void AND_HL (void) {and (memory[registers.HL]);}
 void AND_n (void) {and (operand8);}

 
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
 void OR_HL (void) {or (memory[registers.HL]);}
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
 void CP_HL (void) {comp (memory[registers.HL]);}
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
 void INC_HL (void) {inc (&memory[registers.HL]);}
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
 //void RST00 (void) {stackPush16}
 
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


















//void SET_0_E (void){ registers.E = registers.E & 0x1; }



