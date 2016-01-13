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

const struct opCode opCodes[256] = {
	{NOP,		0,	4,  "NOP"},		    // 0x00
	{LD_BC_nn,  2,  12, "LD_BC_nn" },	// 0x01
	{tempfunction,0},		// 0x02
	{tempfunction,0},		// 0x03
	{tempfunction,0},		// 0x04
	{DEC_B,     0,  4,  "DEC_B"},		// 0x05
	{LD_B_n,	1,	8,  "LD_B_n"},	    // 0x06
	{tempfunction,0},		// 0x07
	{tempfunction,0},		// 0x08
	{tempfunction,0},		// 0x09
	{tempfunction,0},		// 0x0A
	{tempfunction,0},		// 0x0B
	{tempfunction,0},		// 0x0C
	{DEC_C,     0,  4, "DEC_C"},		// 0x0D
	{LD_C_n,	1,	8, "LD_C_n"},		// 0x0E
	{tempfunction,0},		// 0x0F
	{tempfunction,0},		// 0x10
	{LD_DE_nn,  2,  12, "LD_DE_nn"},	// 0x11
	{tempfunction,0},		// 0x12
	{tempfunction,0},		// 0x13
	{tempfunction,0},		// 0x14
	{DEC_D,     0,  4, "DEC_D"},		// 0x15
	{LD_D_n,	1,	8, "LD_D_n"},		// 0x16
	{tempfunction,0},		// 0x17
	{JR_n,		1,	8, "JR_n"},		// 0x18
	{tempfunction,0},		// 0x19
	{tempfunction,0},		// 0x1A
	{tempfunction,0},		// 0x1B
	{tempfunction,0},		// 0x1C
	{DEC_E,     0,  4, "DEC_E"},		// 0x1D
	{LD_E_n,	1,	8, "LD_E_n"},		// 0x1E
	{tempfunction,0},		// 0x1F
	{JR_NZ_n,	1,	8, "JR_NZ_n"},		// 0x20
	{LD_HL_nn,  2,  12, "LD_HL_nn"},	// 0x21
	{tempfunction,0},		// 0x22
	{tempfunction,0},		// 0x23
	{tempfunction,0},		// 0x24
	{DEC_H,     0,  4, "DEC_H"},		// 0x25
	{LD_H_n,	1,	8, "LD_H_n"},		// 0x26
	{tempfunction,0},		// 0x27
	{JR_Z_n,	1,	8, "JR_Z_n"},		// 0x28
	{tempfunction,0},		// 0x29
	{tempfunction,0},		// 0x2A
	{tempfunction,0},		// 0x2B
	{tempfunction,0},		// 0x2C
	{DEC_L,     0,  4, "DEC_L"},		// 0x2D
	{LD_L_n,	1,	8, "LD_L_n"},		// 0x2E
	{tempfunction,0},		// 0x2F
	{JR_NC_n,	1,	8, "JR_NC_n"},		// 0x30
	{LD_SP_nn,  2,  12, "LD_SP_nn"},	// 0x31
	{LDD_HL_A,  0,  8, "LDD_HL_A"},	    // 0x32
	{tempfunction,0},		// 0x33
	{tempfunction,0},		// 0x34
	{DEC_HL,    0,  12, "DEC_HL"},	// 0x35
	{tempfunction,0},		// 0x36
	{tempfunction,0},		// 0x37
	{JR_C_n,	1,	8, "JR_C_n"},		// 0x38
	{tempfunction,0},		// 0x39
	{tempfunction,0},		// 0x3A
	{tempfunction,0},		// 0x3B
	{tempfunction,0},		// 0x3C
	{DEC_A,     0,  4, "DEC_A"},		// 0x3D
	{tempfunction,0},		// 0x3E
	{tempfunction,0},		// 0x3F
	{tempfunction,0},		// 0x40
	{tempfunction,0},		// 0x41
	{tempfunction,0},		// 0x42
	{tempfunction,0},		// 0x43
	{tempfunction,0},		// 0x44
	{tempfunction,0},		// 0x45
	{tempfunction,0},		// 0x46
	{tempfunction,0},		// 0x47
	{tempfunction,0},		// 0x48
	{tempfunction,0},		// 0x49
	{tempfunction,0},		// 0x4A
	{tempfunction,0},		// 0x4B
	{tempfunction,0},		// 0x4C
	{tempfunction,0},		// 0x4D
	{tempfunction,0},		// 0x4E
	{tempfunction,0},		// 0x4F
	{tempfunction,0},		// 0x50
	{tempfunction,0},		// 0x51
	{tempfunction,0},		// 0x52
	{tempfunction,0},		// 0x53
	{tempfunction,0},		// 0x54
	{tempfunction,0},		// 0x55
	{tempfunction,0},		// 0x56
	{tempfunction,0},		// 0x57
	{tempfunction,0},		// 0x58
	{tempfunction,0},		// 0x59
	{tempfunction,0},		// 0x5A
	{tempfunction,0},		// 0x5B
	{tempfunction,0},		// 0x5C
	{tempfunction,0},		// 0x5D
	{tempfunction,0},		// 0x5E
	{tempfunction,0},		// 0x5F
	{tempfunction,0},		// 0x60
	{tempfunction,0},		// 0x61
	{tempfunction,0},		// 0x62
	{tempfunction,0},		// 0x63
	{tempfunction,0},		// 0x64
	{tempfunction,0},		// 0x65
	{tempfunction,0},		// 0x66
	{tempfunction,0},		// 0x67
	{tempfunction,0},		// 0x68
	{tempfunction,0},		// 0x69
	{tempfunction,0},		// 0x6A
	{tempfunction,0},		// 0x6B
	{tempfunction,0},		// 0x6C
	{tempfunction,0},		// 0x6D
	{tempfunction,0},		// 0x6E
	{tempfunction,0},		// 0x6F
	{tempfunction,0},		// 0x70
	{tempfunction,0},		// 0x71
	{tempfunction,0},		// 0x72
	{tempfunction,0},		// 0x73
	{tempfunction,0},		// 0x74
	{tempfunction,0},		// 0x75
	{tempfunction,0},		// 0x76
	{tempfunction,0},		// 0x77
	{tempfunction,0},		// 0x78
	{tempfunction,0},		// 0x79
	{tempfunction,0},		// 0x7A
	{tempfunction,0},		// 0x7B
	{tempfunction,0},		// 0x7C
	{tempfunction,0},		// 0x7D
	{tempfunction,0},		// 0x7E
	{tempfunction,0},		// 0x7F
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
	{tempfunction,0},		// 0x90
	{tempfunction,0},		// 0x91
	{tempfunction,0},		// 0x92
	{tempfunction,0},		// 0x93
	{tempfunction,0},		// 0x94
	{tempfunction,0},		// 0x95
	{tempfunction,0},		// 0x96
	{tempfunction,0},		// 0x97
	{tempfunction,0},		// 0x98
	{tempfunction,0},		// 0x99
	{tempfunction,0},		// 0x9A
	{tempfunction,0},		// 0x9B
	{tempfunction,0},		// 0x9C
	{tempfunction,0},		// 0x9D
	{tempfunction,0},		// 0x9E
	{tempfunction,0},		// 0x9F
	{tempfunction,0},		// 0xA0
	{tempfunction,0},		// 0xA1
	{tempfunction,0},		// 0xA2
	{tempfunction,0},		// 0xA3
	{tempfunction,0},		// 0xA4
	{tempfunction,0},		// 0xA5
	{tempfunction,0},		// 0xA6
	{tempfunction,0},		// 0xA7
	{XOR_B,     0,  4, "XOR_B"},		// 0xA8
	{XOR_C,     0,  4, "XOR_C"},		// 0xA9
	{XOR_D,     0,  4, "XOR_D"},		// 0xAA
	{XOR_E,     0,  4, "XOR_E"},		// 0xAB
	{XOR_H,     0,  4, "XOR_H"},		// 0xAC
	{XOR_L,     0,  4, "XOR_L"},		// 0xAD
	{XOR_HL,    0,  8, "XOR_HL"},		// 0xAE
	{XOR_A,     0,  4, "XOR_A"},		// 0xAF
	{tempfunction,0},		// 0xB0
	{tempfunction,0},		// 0xB1
	{tempfunction,0},		// 0xB2
	{tempfunction,0},		// 0xB3
	{tempfunction,0},		// 0xB4
	{tempfunction,0},		// 0xB5
	{tempfunction,0},		// 0xB6
	{tempfunction,0},		// 0xB7
	{tempfunction,0},		// 0xB8
	{tempfunction,0},		// 0xB9
	{tempfunction,0},		// 0xBA
	{tempfunction,0},		// 0xBB
	{tempfunction,0},		// 0xBC
	{tempfunction,0},		// 0xBD
	{tempfunction,0},		// 0xBE
	{tempfunction,0},		// 0xBF
	{RET_NZ,     0,  8, "RET_NZ"},	// 0xC0
	{tempfunction,0},		// 0xC1
	{JP_NZ_nn,	2,	12, "JP_NZ_nn"},	// 0xC2
	{JP_nn,		2,	12, "JP_nn"},	// 0xC3
	{tempfunction,0},		// 0xC4
	{tempfunction,0},		// 0xC5
	{ADD_A_n,	1,	 8, "ADD_A_n"},	// 0xC6
	{tempfunction,0},		// 0xC7
	{RET_Z,     0,   8, "RET_Z"},    // 0xC8
	{RET,       0,   8, "RET"},	// 0xC9
	{JP_Z_nn,	2,	12, "JP_Z_nn"},	// 0xCA
	{tempfunction,0},		// 0xCB
	{tempfunction,0},		// 0xCC
	{tempfunction,0},		// 0xCD
	{ADC_A_n,   1,   8, "ADC_A_n"},	// 0xCE
	{tempfunction,0},		// 0xCF
	{RET_NC,     0,  8, "RET_NC"},	// 0xD0
	{tempfunction,0},		// 0xD1
	{JP_NC_nn,	2,	12, "JP_NC_nn"},	// 0xD2
	{tempfunction,0},		// 0xD3
	{tempfunction,0},		// 0xD4
	{tempfunction,0},		// 0xD5
	{tempfunction,0},		// 0xD6
	{tempfunction,0},		// 0xD7
	{RET_C,     0,   8, "RET_C"},    // 0xD8
	{RETI,      0,   8, "RETI"},    // 0xD9
	{JP_C_nn,	2,	12, "JP_C_nn"},	// 0xDA
	{tempfunction,0},		// 0xDB
	{tempfunction,0},		// 0xDC
	{tempfunction,0},		// 0xDD
	{tempfunction,0},		// 0xDE
	{tempfunction,0},		// 0xDF
	{tempfunction,0},		// 0xE0
	{tempfunction,0},		// 0xE1
	{tempfunction,0},		// 0xE2
	{tempfunction,0},		// 0xE3
	{tempfunction,0},		// 0xE4
	{tempfunction,0},		// 0xE5
	{tempfunction,0},		// 0xE6
	{tempfunction,0},		// 0xE7
	{tempfunction,0},		// 0xE8
	{JP_HL,		0,	4, "JP_HL"},		// 0xE9
	{tempfunction,0},		// 0xEA
	{tempfunction,0},		// 0xEB
	{tempfunction,0},		// 0xEC
	{tempfunction,0},		// 0xED
	{XOR_n,     1,  8, "XOR_n"},		// 0xEE
	{tempfunction,0},		// 0xEF
	{tempfunction,0},		// 0xF0
	{tempfunction,0},		// 0xF1
	{tempfunction,0},		// 0xF2
	{tempfunction,0},		// 0xF3
	{tempfunction,0},		// 0xF4
	{tempfunction,0},		// 0xF5
	{tempfunction,0},		// 0xF6
	{tempfunction,0},		// 0xF7
	{tempfunction,0},		// 0xF8
	{tempfunction,0},		// 0xF9
	{tempfunction,0},		// 0xFA
	{tempfunction,0},		// 0xFB
	{tempfunction,0},		// 0xFC
	{tempfunction,0},		// 0xFD
	{tempfunction,0},		// 0xFE
	{tempfunction,0},		// 0xFF
};


int execute (void){
	
	instruction = memory[registers.PC];

	
//	printf("[DEBUG] Opcode    - 0x%04x, P counter - 0x%04x, S pointer - 0x%04x\n",memory[registers.PC],registers.PC,registers.SP);
//  printf("[DEBUG] Registers - A=0x%02x, B=0x%02x, C=0x%02x, D=0x%02x, E=0x%02x, F=0x%02x, H=0x%02x, L=0x%02x\n"
//                  ,registers.A,registers.B,registers.C,registers.D,registers.E,registers.F,registers.H,registers.L);

    printf("[DEBUG] OPC-0x%04x-[%s],\tPC-0x%04x, SP-0x%04x, ",instruction,opCodes[instruction].function_name,registers.PC,registers.SP);

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
	
	return opCodes[instruction].cycles;
	
}


	
void tempfunction(void) {
	
	printf("[ERROR] Opcode 0x%02x not implemented\n",instruction);
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
void ADD_A_HL (void){ add (registers.A, registers.HL); }
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
void ADC_A_HL (void){ adc (registers.A, registers.HL); }
void ADC_A_n (void){ adc (registers.A, operand8); }
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
void XOR_HL (void) { xor (registers.HL); }
void XOR_n (void) { xor (operand8); }
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
void DEC_HL (void) {
    registers.HL--;
    if (registers.HL == 0)
        setFlag(ZERO_F);
    setFlag(SUBSTRACT_F);
    
// please check cpu.c line 353
//    if (registers.HL == 0)
//        setFlag(ZERO_F);
//    else
//        resetFlag(ZERO_F);
//    setFlag(SUBSTRACT_F);

// flags are not affected according to cinoop    
// if (registers.HL == 0xF)
//     setFlag(HALF_CARRY_F);
}
/********************
 * 16-Bit ALU       *
 ********************/
 /*******************
 * ADD, INC, DEC    *
 ********************/
/********************
 * Miscellaneous    *
 ********************/
 void NOP (void){ };
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
void JP_NZ_nn (void){ if (testFlag(ZERO_F)  == 0) registers.PC = operand16; }
void JP_Z_nn (void)	{ if (testFlag(ZERO_F)  == 1) registers.PC = operand16; }
void JP_NC_nn (void){ if (testFlag(CARRY_F) == 0) registers.PC = operand16; }
void JP_C_nn (void)	{ if (testFlag(CARRY_F) == 1) registers.PC = operand16; }
void JP_HL (void)	{ registers.PC = registers.HL; }
void JR_n (void)	{ registers.PC = registers.PC + (signed char)operand8; }
void JR_NZ_n (void)	{ if (testFlag(ZERO_F)  == 0) registers.PC = registers.PC + (signed char)operand8; }
void JR_Z_n (void)	{ if (testFlag(ZERO_F)  == 1) registers.PC = registers.PC + (signed char)operand8; }
void JR_NC_n (void)	{ if (testFlag(CARRY_F) == 0) registers.PC = registers.PC + (signed char)operand8; }
void JR_C_n (void)	{ if (testFlag(CARRY_F) == 1) registers.PC = registers.PC + (signed char)operand8; }
/********************
 * Calls            *
 ********************/
/********************
 * Restarts         *
 ********************/
/********************
 * Returns          *
 ********************/
/*
 * RET
 * Description: Pop two bytes from stack & jump to that address.
 */
void RET    (void){ registers.PC = stackPop16(); }
/*
 * RET cc
 * Description: Return if following condition is true:
 * Use with:
 * cc = NZ, Return if Z flag is reset.
 * cc = Z, Return if Z flag is set.
 * cc = NC, Return if C flag is reset.
 */
void RET_NZ (void){ if (testFlag(ZERO_F)  == 0) registers.PC = stackPop16(); }
void RET_Z  (void){ if (testFlag(ZERO_F)  == 1) registers.PC = stackPop16(); }
void RET_NC (void){ if (testFlag(CARRY_F) == 0) registers.PC = stackPop16(); }
void RET_C  (void){ if (testFlag(CARRY_F) == 1) registers.PC = stackPop16(); }
/*
 * RETI
 * Description: Pop two bytes from stack & jump to that address then
 * enable interrupts.
 */
void RETI   (void){ registers.PC = stackPop16(); interruptMaster = TRUE;}


















//void SET_0_E (void){ registers.E = registers.E & 0x1; }



