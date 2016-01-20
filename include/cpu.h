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

int execute (void);
void tempfunction (void);
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
//LD (C),A
void LD_MC_A (void);
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
 /*******************
 * ADD, INC, DEC    *
 ********************/
/********************
 * Miscellaneous    *
 ********************/
void DAA (void); 
void CPL (void);
void CCF (void);
void NOP (void);
void DI (void);
void EI (void); 
/********************
 * Rotates & Shifts *
 ********************/
void SLA_A (void);
void SLA_B (void);
void SLA_C (void);
void SLA_D (void);
void SLA_E (void);
void SLA_H (void);
void SLA_L (void); 
/********************
 * Bit Opcodes      *
 ********************/
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

void SWAP_A (void);
void RES_0_A (void);
void BIT_7_H (void);
void  RL_C (void);


	
