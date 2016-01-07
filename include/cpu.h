const struct opCode{
	void *function;
	unsigned char opLength;
	unsigned char cycles;
}extern const opCodes[256];

int execute (void);
void tempfunction (void);
//8-Bit ALU
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
//DEC n
void DEC_A (void);
void DEC_B (void);
void DEC_C (void);
void DEC_D (void);
void DEC_E (void);
void DEC_H (void);
void DEC_L (void);
void DEC_HL (void);

//LD n,nn
void LD_BC_nn (void);
void LD_DE_nn (void);
void LD_HL_nn (void);
void LD_SP_nn (void);

//LDD (HL),A
void LDD_HL_A (void);

void NOP (void);


void LD_B_n (void);
void LD_C_n (void);
void LD_D_n (void);
void LD_E_n (void);
void LD_H_n (void);
void LD_L_n (void);

//SET
void SET_0_E (void);

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


	
