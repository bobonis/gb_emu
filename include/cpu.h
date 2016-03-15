const struct opCode{
	void *function;
	unsigned char opLength;
	unsigned char cycles;
    char *function_name;
}extern const opCodes[256];

const struct extendedopCode{
	void *function;
	unsigned char opLength;
	unsigned char cycles;
    char *function_name;
}extern const extendedopCodes[256];

struct cpu{
    int halt;
    int stop;
    int ime;
    int ime_delay;
    int interrupt;
    int repeat;
}extern cpustate;

extern int cpuHALT;
extern int cpuSTOP;

void execute (void);
void tempfunction (void);
void NOTVALID (void);

/********************
* CPU Microcode     *
*********************/
void bit (unsigned char pos, unsigned char value);
void sbc (unsigned char value);
void add (unsigned char value);
void adc (unsigned char value);
void xor (unsigned char value);
void and (unsigned char value);
void add16 (unsigned short value);
void or (unsigned char value);
void cp (unsigned char value);
void sub (unsigned char value);
unsigned char inc (unsigned char value);
unsigned char dec (unsigned char value);
unsigned char srl (unsigned char value);
unsigned char rr (unsigned char value);
unsigned char res (unsigned char pos, unsigned char value);
unsigned char set (unsigned char pos, unsigned char value);
unsigned char rrc (unsigned char value);
unsigned char rlc (unsigned char value);
unsigned char rl (unsigned char value);
unsigned char swap (unsigned char value);
unsigned char sra (unsigned char value);

unsigned char sla (unsigned char value);    //Thug Life
/********************
* 8-Bit Loads       *
*********************/
//LD nn,n
void LD_B_n (void);
void LD_C_n (void);
void LD_D_n (void);
void LD_E_n (void);
void LD_H_n (void);
void LD_L_n (void);
//LD r1,r2
void LD_B_B (void);
void LD_B_C (void);
void LD_B_D (void);
void LD_B_E (void);
void LD_B_H (void);
void LD_B_L (void);
void LD_B_HL (void);
void LD_C_B (void);
void LD_C_C (void);
void LD_C_D (void);
void LD_C_E (void);
void LD_C_H (void);
void LD_C_L (void);
void LD_C_HL (void);
void LD_D_B (void); 
void LD_D_C (void);
void LD_D_D (void);
void LD_D_E (void);
void LD_D_H (void);
void LD_D_L (void);
void LD_D_HL (void);
void LD_E_B (void); 
void LD_E_C (void);
void LD_E_D (void);
void LD_E_E (void);
void LD_E_H (void);
void LD_E_L (void);
void LD_E_HL (void);
void LD_H_B (void); 
void LD_H_C (void);
void LD_H_D (void);
void LD_H_E (void);
void LD_H_H (void);
void LD_H_L (void);
void LD_H_HL (void);
void LD_L_B (void); 
void LD_L_C (void);
void LD_L_D (void);
void LD_L_E (void);
void LD_L_H (void);
void LD_L_L (void);
void LD_L_HL (void);
void LD_HL_B (void); 
void LD_HL_C (void);
void LD_HL_D (void);
void LD_HL_E (void);
void LD_HL_H (void);
void LD_HL_L (void);
void LD_HL_n (void);
//LD A,n
void LD_A_n (void);
void LD_A_A (void);
void LD_A_B (void);
void LD_A_C (void);
void LD_A_D (void);
void LD_A_E (void);
void LD_A_H (void);
void LD_A_L (void);
void LD_A_BC (void);
void LD_A_DE (void);
void LD_A_HL (void);
void LD_A_nn (void);
//LD n,A
void LD_B_A (void);
void LD_C_A (void);
void LD_D_A (void);
void LD_E_A (void);
void LD_H_A (void);
void LD_L_A (void);
void LD_BC_A (void);
void LD_DE_A (void);
void LD_HL_A (void);
void LD_nn_A (void);
//LD A,(C)
void LD_A_MC (void);
//LD (C),A
void LD_MC_A (void);
//LDD A,(HL)
void LDD_A_HL (void);
//LDI A,(HL)
void LDI_A_HL (void);
//LDI (HL),A
void LDI_HL_A (void);
//LDH (n),A
void LDH_n_A (void);
//LDH A,(n)
void LDH_A_n (void);
/********************
 * 16-Bit Loads     *
 ********************/
//LD n,nn
void LD_BC_nn (void);
void LD_DE_nn (void);
void LD_HL_nn (void);
void LD_SP_nn (void);
//LD SP,HL
void LD_SP_HL (void);
//LDHL SP,n
void LDHL_SP_n (void);
//LD (nn),SP
void LD_nn_SP (void);
//PUSH nn
void PUSH_AF (void);
void PUSH_BC (void);
void PUSH_DE (void);
void PUSH_HL (void);
//POP nn
void POP_AF (void);
void POP_BC (void);
void POP_DE (void);
void POP_HL (void);
//LDD (HL),A
void LDD_HL_A (void);
/********************
 * 8-Bit ALU        *
 ********************/
//ADD A,n
void ADD_A_A (void);
void ADD_A_B (void);
void ADD_A_C (void);
void ADD_A_D (void);
void ADD_A_E (void);
void ADD_A_H (void);
void ADD_A_L (void);
void ADD_A_HL (void);
void ADD_A_n (void);
//ADC A,n
void ADC_A_A (void);
void ADC_A_B (void);
void ADC_A_C (void);
void ADC_A_D (void);
void ADC_A_E (void);
void ADC_A_H (void);
void ADC_A_L (void);
void ADC_A_HL (void);
void ADC_A_n (void);
//SUB n
void SUB_A (void);
void SUB_B (void);
void SUB_C (void);
void SUB_D (void);
void SUB_E (void);
void SUB_H (void);
void SUB_L (void);
void SUB_HL (void);
void SUB_n (void);
//SBC A,n
void SBC_A_A (void);
void SBC_A_B (void);
void SBC_A_C (void);
void SBC_A_D (void);
void SBC_A_E (void);
void SBC_A_H (void);
void SBC_A_L (void);
void SBC_A_HL (void);
void SBC_A_n (void);
//AND n
void AND_A (void);
void AND_B (void);
void AND_C (void);
void AND_D (void);
void AND_E (void);
void AND_H (void);
void AND_L (void);
void AND_HL (void);
void AND_n (void);
//OR n
void OR_A (void);
void OR_B (void);
void OR_C (void);
void OR_D (void);
void OR_E (void);
void OR_H (void);
void OR_L (void);
void OR_HL (void);
void OR_n (void);
//XOR n
void XOR_A (void);
void XOR_B (void);
void XOR_C (void);
void XOR_D (void);
void XOR_E (void);
void XOR_H (void);
void XOR_L (void);
void XOR_HL (void);
void XOR_n (void);
//CP n
void CP_A (void);
void CP_B (void);
void CP_C (void);
void CP_D (void);
void CP_E (void);
void CP_H (void);
void CP_L (void);
void CP_HL (void);
void CP_n (void);
//INC n
void INC_A (void);
void INC_B (void);
void INC_C (void);
void INC_D (void);
void INC_E (void);
void INC_H (void);
void INC_L (void);
void INC_MHL (void);
//DEC n
void DEC_A (void);
void DEC_B (void);
void DEC_C (void);
void DEC_D (void);
void DEC_E (void);
void DEC_H (void);
void DEC_L (void);
void DEC_MHL (void);
/********************
 * 16-Bit ALU       *
 ********************/
//DEC nn
void DEC_BC (void);
void DEC_DE (void);
void DEC_HL (void);
void DEC_SP (void);
//INC nn
void INC_BC (void);
void INC_DE (void);
void INC_HL (void);
void INC_SP (void);
//ADD HL,n
void ADD_HL_BC (void);
void ADD_HL_DE (void);
void ADD_HL_HL (void);
void ADD_HL_SP (void);
//ADD SP,n
void ADD_SP_n (void);
/*******************
 * ADD, INC, DEC    *
 ********************/
/********************
 * Miscellaneous    *
 ********************/
//SWAP n
void SWAP_A (void);
void SWAP_B (void);
void SWAP_C (void);
void SWAP_D (void);
void SWAP_E (void);
void SWAP_H (void);
void SWAP_L (void);
void SWAP_HL (void); 

void DAA (void); 
void CPL (void);
void CCF (void);
void SCF (void);
void NOP (void);
void HALT (void);
void STOP (void);
void DI (void);
void EI (void); 
/********************
 * Rotates & Shifts *
 ********************/
//RLCA
void RLCA (void);
//RRCA
void RRCA (void);
//RLC n
void RLC_A (void);
void RLC_B (void);
void RLC_C (void);
void RLC_D (void);
void RLC_E (void);
void RLC_H (void);
void RLC_L (void);
void RLC_HL (void);
//RRC n
void RRC_A (void);
void RRC_B (void);
void RRC_C (void);
void RRC_D (void);
void RRC_E (void);
void RRC_H (void);
void RRC_L (void);
void RRC_HL (void);
//RR n
void RR_A (void);
void RR_B (void); 
void RR_C (void);  
void RR_D (void); 
void RR_E (void); 
void RR_H (void); 
void RR_L (void);
void RR_HL (void); 
//RL n
void RL_A (void);
void RL_B (void);
void RL_C (void);
void RL_D (void);
void RL_E (void);
void RL_H (void);
void RL_L (void);
void RL_HL (void); 
//SLA,n
void SLA_A (void);
void SLA_B (void);
void SLA_C (void);
void SLA_D (void);
void SLA_E (void);
void SLA_H (void);
void SLA_L (void);
void SLA_HL (void);
//SRA,n
void SRA_A (void);
void SRA_B (void);
void SRA_C (void);
void SRA_D (void);
void SRA_E (void);
void SRA_H (void);
void SRA_L (void);
void SRA_HL (void);
// SRL n
void SRL_A (void); 
void SRL_B (void);
void SRL_C (void);
void SRL_D (void);
void SRL_E (void);
void SRL_H (void);
void SRL_L (void);
void SRL_HL (void);
//RLA 
void RLA (void);
//RRA 
void RRA (void);
/********************
 * Bit Opcodes      *
 ********************/
//SET
void SET_0_A (void);
void SET_1_A (void);
void SET_2_A (void);
void SET_3_A (void);
void SET_4_A (void);
void SET_5_A (void);
void SET_6_A (void);
void SET_7_A (void);

void SET_0_B (void);
void SET_1_B (void);
void SET_2_B (void);
void SET_3_B (void);
void SET_4_B (void);
void SET_5_B (void);
void SET_6_B (void);
void SET_7_B (void);

void SET_0_C (void);
void SET_1_C (void);
void SET_2_C (void);
void SET_3_C (void);
void SET_4_C (void);
void SET_5_C (void);
void SET_6_C (void);
void SET_7_C (void);

void SET_0_D (void);
void SET_1_D (void);
void SET_2_D (void);
void SET_3_D (void);
void SET_4_D (void);
void SET_5_D (void);
void SET_6_D (void);
void SET_7_D (void);

void SET_0_E (void);
void SET_1_E (void);
void SET_2_E (void);
void SET_3_E (void);
void SET_4_E (void);
void SET_5_E (void);
void SET_6_E (void);
void SET_7_E (void);

void SET_0_H (void);
void SET_1_H (void);
void SET_2_H (void);
void SET_3_H (void);
void SET_4_H (void);
void SET_5_H (void);
void SET_6_H (void);
void SET_7_H (void);

void SET_0_L (void);
void SET_1_L (void);
void SET_2_L (void);
void SET_3_L (void);
void SET_4_L (void);
void SET_5_L (void);
void SET_6_L (void);
void SET_7_L (void);

void SET_0_HL (void);
void SET_1_HL (void);
void SET_2_HL (void);
void SET_3_HL (void);
void SET_4_HL (void);
void SET_5_HL (void);
void SET_6_HL (void);
void SET_7_HL (void);  
//RES
void RES_0_A (void);
void RES_1_A (void);
void RES_2_A (void);
void RES_3_A (void);
void RES_4_A (void);
void RES_5_A (void);
void RES_6_A (void);
void RES_7_A (void);

void RES_0_B (void);
void RES_1_B (void);
void RES_2_B (void);
void RES_3_B (void);
void RES_4_B (void);
void RES_5_B (void);
void RES_6_B (void);
void RES_7_B (void);

void RES_0_C (void);
void RES_1_C (void);
void RES_2_C (void);
void RES_3_C (void);
void RES_4_C (void);
void RES_5_C (void);
void RES_6_C (void);
void RES_7_C (void);

void RES_0_D (void);
void RES_1_D (void);
void RES_2_D (void);
void RES_3_D (void);
void RES_4_D (void);
void RES_5_D (void);
void RES_6_D (void);
void RES_7_D (void);

void RES_0_E (void);
void RES_1_E (void);
void RES_2_E (void);
void RES_3_E (void);
void RES_4_E (void);
void RES_5_E (void);
void RES_6_E (void);
void RES_7_E (void);

void RES_0_H (void);
void RES_1_H (void);
void RES_2_H (void);
void RES_3_H (void);
void RES_4_H (void);
void RES_5_H (void);
void RES_6_H (void);
void RES_7_H (void);

void RES_0_L (void);
void RES_1_L (void);
void RES_2_L (void);
void RES_3_L (void);
void RES_4_L (void);
void RES_5_L (void);
void RES_6_L (void);
void RES_7_L (void);

void RES_0_HL (void);
void RES_1_HL (void);
void RES_2_HL (void);
void RES_3_HL (void);
void RES_4_HL (void);
void RES_5_HL (void);
void RES_6_HL (void);
void RES_7_HL (void); 
//BIT
void BIT_0_A (void);
void BIT_1_A (void);
void BIT_2_A (void);
void BIT_3_A (void);
void BIT_4_A (void);
void BIT_5_A (void);
void BIT_6_A (void);
void BIT_7_A (void);

void BIT_0_B (void);
void BIT_1_B (void);
void BIT_2_B (void);
void BIT_3_B (void);
void BIT_4_B (void);
void BIT_5_B (void);
void BIT_6_B (void);
void BIT_7_B (void);

void BIT_0_C (void);
void BIT_1_C (void);
void BIT_2_C (void);
void BIT_3_C (void);
void BIT_4_C (void);
void BIT_5_C (void);
void BIT_6_C (void);
void BIT_7_C (void);

void BIT_0_D (void);
void BIT_1_D (void);
void BIT_2_D (void);
void BIT_3_D (void);
void BIT_4_D (void);
void BIT_5_D (void);
void BIT_6_D (void);
void BIT_7_D (void);

void BIT_0_E (void);
void BIT_1_E (void);
void BIT_2_E (void);
void BIT_3_E (void);
void BIT_4_E (void);
void BIT_5_E (void);
void BIT_6_E (void);
void BIT_7_E (void);

void BIT_0_H (void);
void BIT_1_H (void);
void BIT_2_H (void);
void BIT_3_H (void);
void BIT_4_H (void);
void BIT_5_H (void);
void BIT_6_H (void);
void BIT_7_H (void);

void BIT_0_L (void);
void BIT_1_L (void);
void BIT_2_L (void);
void BIT_3_L (void);
void BIT_4_L (void);
void BIT_5_L (void);
void BIT_6_L (void);
void BIT_7_L (void);

void BIT_0_HL (void);
void BIT_1_HL (void);
void BIT_2_HL (void);
void BIT_3_HL (void);
void BIT_4_HL (void);
void BIT_5_HL (void);
void BIT_6_HL (void);
void BIT_7_HL (void);
/********************
 * Jumps            *
 ********************/
 //JUMPS
void JP_nn (void) ;
void JP_NZ_nn (void);
void JP_Z_nn (void);
void JP_NC_nn (void);
void JP_C_nn (void);
void JP_HL (void);
void JR_n (void);
void JR_NZ_n (void);
void JR_Z_n (void);
void JR_NC_n (void);
void JR_C_n (void);
/********************
 * Calls            *
 ********************/
void CALL_nn (void);
void CALL_NZ_nn (void);
void CALL_Z_nn (void);
void CALL_NC_nn (void);
void CALL_C_nn (void);
/********************
 * Restarts         *
 ********************/
void RST00 (void);
void RST08 (void);
void RST10 (void);
void RST18 (void);
void RST20 (void);
void RST28 (void);
void RST30 (void);
void RST38 (void);
/********************
 * Returns          *
 ********************/
void RET (void);
void RET_NZ (void);
void RET_Z (void);
void RET_NC (void);
void RET_C (void);
void RETI (void);

//Extended OPcodes
void CB (void);



	
