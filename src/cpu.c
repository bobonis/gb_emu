#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "memory.h"
#include "rom.h"
#include "interrupts.h"
#include "timers.h"
#include "hardware.h"
#include "definitions.h"

/* DEBUG FLAGS */
int debug_mooneye = FALSE;
int debug_test_run = FALSE;
unsigned short debug_pc = 0x0000;

/* CPU VARIABLES */

unsigned char operand8 = 0x00;
unsigned short operand16 = 0x0000;

unsigned char previous = 0;
unsigned char current = 0;


struct cpu cpustate = {
    FALSE,      //halt
    FALSE,      //stop
    TRUE,       //ime
    0,          //ime_delay
    FALSE,      //interrupt Does nothing ATM
    FALSE       //repeat
};

const struct opCode opCodes[256] = {
    { NOP,          0,  4,  "NOP"       },  // 0x00
    { LD_BC_nn,     2, 12,  "LD_BC_nn"  },  // 0x01
    { LD_BC_A,      0,  8,  "LD_BC_A"   },  // 0x02
    { INC_BC,       0,  8,  "INC_BC"    },  // 0x03
    { INC_B,        0,  4,  "INC_B"     },  // 0x04
    { DEC_B,        0,  4,  "DEC_B"     },  // 0x05
    { LD_B_n,       1,  8,  "LD_B_n"    },  // 0x06
    { RLCA,         0,  4,  "RLCA"      },  // 0x07
    { LD_nn_SP,     2, 20,  "LD_nn_SP"  },  // 0x08
    { ADD_HL_BC,    0,  8,  "ADD_HL_BC" },  // 0x09
    { LD_A_BC,      0,  8,  "LD_A_BC"   },  // 0x0A
    { DEC_BC,       0,  8,  "DEC_BC"    },  // 0x0B
    { INC_C,        0,  4,  "INC_C"     },  // 0x0C
    { DEC_C,        0,  4,  "DEC_C"     },  // 0x0D
    { LD_C_n,       1,  8,  "LD_C_n"    },  // 0x0E
    { RRCA,         0,  4,  "RRCA"      },  // 0x0F
    { STOP,         1,  4,  "STOP"      },  // 0x10
    { LD_DE_nn,     2, 12,  "LD_DE_nn"  },  // 0x11
    { LD_DE_A,      0,  8,  "LD_DE_A"   },  // 0x12
    { INC_DE,       0,  8,  "INC_DE"    },  // 0x13
    { INC_D,        0,  4,  "INC_D"     },  // 0x14
    { DEC_D,        0,  4,  "DEC_D"     },  // 0x15
    { LD_D_n,       1,  8,  "LD_D_n"    },  // 0x16
    { RLA,          0,  4,  "RLA"       },  // 0x17
    { JR_n,         1, 12,  "JR_n"      },  // 0x18
    { ADD_HL_DE,    0,  8,  "ADD_HL_DE" },  // 0x19
    { LD_A_DE,      0,  8,  "LD_A_DE"   },  // 0x1A
    { DEC_DE,       0,  8,  "DEC_DE"    },  // 0x1B
    { INC_E,        0,  4,  "INC_E"     },  // 0x1C
    { DEC_E,        0,  4,  "DEC_E"     },  // 0x1D
    { LD_E_n,       1,  8,  "LD_E_n"    },  // 0x1E
    { RRA,          0,  4,  "RRA"       },  // 0x1F
    { JR_NZ_n,      1,  8,  "JR_NZ_n"   },  // 0x20
    { LD_HL_nn,     2, 12,  "LD_HL_nn"  },  // 0x21
    { LDI_HL_A,     0,  8,  "LDI_HL_A"  },  // 0x22
    { INC_HL,       0,  8,  "INC_HL"    },  // 0x23
    { INC_H,        0,  4,  "INC_H"     },  // 0x24
    { DEC_H,        0,  4,  "DEC_H"     },  // 0x25
    { LD_H_n,       1,  8,  "LD_H_n"    },  // 0x26
    { DAA,          0,  4,  "DAA"       },  // 0x27
    { JR_Z_n,       1,  8,  "JR_Z_n"    },  // 0x28
    { ADD_HL_HL,    0,  8,  "ADD_HL_HL" },  // 0x29
    { LDI_A_HL,     0,  8,  "LDI_A_HL"  },  // 0x2A
    { DEC_HL,       0,  8,  "DEC_HL"    },  // 0x2B
    { INC_L,        0,  4,  "INC_L"     },  // 0x2C
    { DEC_L,        0,  4,  "DEC_L"     },  // 0x2D
    { LD_L_n,       1,  8,  "LD_L_n"    },  // 0x2E
    { CPL,          0,  4,  "CPL"       },  // 0x2F
    { JR_NC_n,      1,  8,  "JR_NC_n"   },  // 0x30
    { LD_SP_nn,     2, 12,  "LD_SP_nn"  },  // 0x31
    { LDD_HL_A,     0,  8,  "LDD_HL_A"  },  // 0x32
    { INC_SP,       0,  8,  "INC_SP"    },  // 0x33
    { INC_MHL,      0, 12,  "INC_MHL"   },  // 0x34
    { DEC_MHL,      0, 12,  "DEC_MHL"   },  // 0x35
    { LD_HL_n,      1, 12,  "LD_HL_n"   },  // 0x36
    { SCF,          0,  4,  "SCF"       },  // 0x37
    { JR_C_n,       1,  8,  "JR_C_n"    },  // 0x38
    { ADD_HL_SP,    0,  8,  "ADD_HL_SP" },  // 0x39
    { LDD_A_HL,     0,  8,  "LDD_A_HL"  },  // 0x3A
    { DEC_SP,       0,  8,  "DEC_SP"    },  // 0x3B
    { INC_A,        0,  4,  "INC_A"     },  // 0x3C
    { DEC_A,        0,  4,  "DEC_A"     },  // 0x3D
    { LD_A_n,       1,  8,  "LD_A_n"    },  // 0x3E
    { CCF,          0,  4,  "CCF"       },  // 0x3F
    { LD_B_B,       0,  4,  "LD_B_B"    },  // 0x40
    { LD_B_C,       0,  4,  "LD_B_C"    },  // 0x41
    { LD_B_D,       0,  4,  "LD_B_D"    },  // 0x42
    { LD_B_E,       0,  4,  "LD_B_E"    },  // 0x43
    { LD_B_H,       0,  4,  "LD_B_H"    },  // 0x44
    { LD_B_L,       0,  4,  "LD_B_L"    },  // 0x45
    { LD_B_HL,      0,  8,  "LD_B_HL"   },  // 0x46
    { LD_B_A,       0,  4,  "LD_B_A"    },  // 0x47
    { LD_C_B,       0,  4,  "LD_C_B"    },  // 0x48
    { LD_C_C,       0,  4,  "LD_C_C"    },  // 0x49
    { LD_C_D,       0,  4,  "LD_C_D"    },  // 0x4A
    { LD_C_E,       0,  4,  "LD_C_E"    },  // 0x4B
    { LD_C_H,       0,  4,  "LD_C_H"    },  // 0x4C
    { LD_C_L,       0,  4,  "LD_C_L"    },  // 0x4D
    { LD_C_HL,      0,  8,  "LD_C_HL"   },  // 0x4E
    { LD_C_A,       0,  4,  "LD_C_A"    },  // 0x4F
    { LD_D_B,       0,  4,  "LD_D_B"    },  // 0x50
    { LD_D_C,       0,  4,  "LD_D_C"    },  // 0x51
    { LD_D_D,       0,  4,  "LD_D_D"    },  // 0x52
    { LD_D_E,       0,  4,  "LD_D_E"    },  // 0x53
    { LD_D_H,       0,  4,  "LD_D_H"    },  // 0x54
    { LD_D_L,       0,  4,  "LD_D_L"    },  // 0x55
    { LD_D_HL,      0,  8,  "LD_D_HL"   },  // 0x56
    { LD_D_A,       0,  4,  "LD_D_A"    },  // 0x47
    { LD_E_B,       0,  4,  "LD_E_B"    },  // 0x58
    { LD_E_C,       0,  4,  "LD_E_C"    },  // 0x59
    { LD_E_D,       0,  4,  "LD_E_D"    },  // 0x5A
    { LD_E_E,       0,  4,  "LD_E_E"    },  // 0x5B
    { LD_E_H,       0,  4,  "LD_E_H"    },  // 0x5C
    { LD_E_L,       0,  4,  "LD_E_L"    },  // 0x5D
    { LD_E_HL,      0,  8,  "LD_E_HL"   },  // 0x5E
    { LD_E_A,       0,  4,  "LD_E_A"    },  // 0x5F
    { LD_H_B,       0,  4,  "LD_H_B"    },  // 0x60
    { LD_H_C,       0,  4,  "LD_H_C"    },  // 0x61
    { LD_H_D,       0,  4,  "LD_H_D"    },  // 0x62
    { LD_H_E,       0,  4,  "LD_H_E"    },  // 0x63
    { LD_H_H,       0,  4,  "LD_H_H"    },  // 0x64
    { LD_H_L,       0,  4,  "LD_H_L"    },  // 0x65
    { LD_H_HL,      0,  8,  "LD_H_HL"   },  // 0x66
    { LD_H_A,       0,  4,  "LD_H_A"    },  // 0x67
    { LD_L_B,       0,  4,  "LD_L_B"    },  // 0x68
    { LD_L_C,       0,  4,  "LD_L_C"    },  // 0x69
    { LD_L_D,       0,  4,  "LD_L_D"    },  // 0x6A
    { LD_L_E,       0,  4,  "LD_L_E"    },  // 0x6B
    { LD_L_H,       0,  4,  "LD_L_H"    },  // 0x6C
    { LD_L_L,       0,  4,  "LD_L_L"    },  // 0x6D
    { LD_L_HL,      0,  8,  "LD_L_HL"   },  // 0x6E
    { LD_L_A,       0,  4,  "LD_L_A"    },  // 0x6F
    { LD_HL_B,      0,  8,  "LD_HL_B"   },  // 0x70
    { LD_HL_C,      0,  8,  "LD_HL_C"   },  // 0x71
    { LD_HL_D,      0,  8,  "LD_HL_D"   },  // 0x72
    { LD_HL_E,      0,  8,  "LD_HL_E"   },  // 0x73
    { LD_HL_H,      0,  8,  "LD_HL_H"   },  // 0x74
    { LD_HL_L,      0,  8,  "LD_HL_L"   },  // 0x75
    { HALT,         0,  4,  "HALT"      },  // 0x76
    { LD_HL_A,      0,  8,  "LD_HL_A"   },  // 0x77
    { LD_A_B,       0,  4,  "LD_A_B"    },  // 0x78
    { LD_A_C,       0,  4,  "LD_A_C"    },  // 0x79
    { LD_A_D,       0,  4,  "LD_A_D"    },  // 0x7A
    { LD_A_E,       0,  4,  "LD_A_E"    },  // 0x7B
    { LD_A_H,       0,  4,  "LD_A_H"    },  // 0x7C
    { LD_A_L,       0,  4,  "LD_A_L"    },  // 0x7D
    { LD_A_HL,      0,  8,  "LD_A_HL"   },  // 0x7E
    { LD_A_A,       0,  4,  "LD_A_A"    },  // 0x7F
    { ADD_A_B,      0,  4,  "ADD_A_B"   },  // 0x80
    { ADD_A_C,      0,  4,  "ADD_A_C"   },  // 0x81
    { ADD_A_D,      0,  4,  "ADD_A_D"   },  // 0x82
    { ADD_A_E,      0,  4,  "ADD_A_E"   },  // 0x83
    { ADD_A_H,      0,  4,  "ADD_A_H"   },  // 0x84
    { ADD_A_L,      0,  4,  "ADD_A_L"   },  // 0x85
    { ADD_A_HL,     0,  8,  "ADD_A_HL"  },  // 0x86
    { ADD_A_A,      0,  4,  "ADD_A_A"   },  // 0x87
    { ADC_A_B,      0,  4,  "ADC_A_B"   },  // 0x88
    { ADC_A_C,      0,  4,  "ADC_A_C"   },  // 0x89
    { ADC_A_D,      0,  4,  "ADC_A_D"   },  // 0x8A
    { ADC_A_E,      0,  4,  "ADC_A_E"   },  // 0x8B
    { ADC_A_H,      0,  4,  "ADC_A_H"   },  // 0x8C
    { ADC_A_L,      0,  4,  "ADC_A_L"   },  // 0x8D
    { ADC_A_HL,     0,  8,  "ADC_A_HL"  },  // 0x8E
    { ADC_A_A,      0,  4,  "ADC_A_A"   },  // 0x8F
    { SUB_B,        0,  4,  "SUB_B"     },  // 0x90
    { SUB_C,        0,  4,  "SUB_C"     },  // 0x91
    { SUB_D,        0,  4,  "SUB_D"     },  // 0x92
    { SUB_E,        0,  4,  "SUB_E"     },  // 0x93
    { SUB_H,        0,  4,  "SUB_H"     },  // 0x94
    { SUB_L,        0,  4,  "SUB_L"     },  // 0x95
    { SUB_HL,       0,  8,  "SUB_HL"    },  // 0x96
    { SUB_A,        0,  4,  "SUB_A"     },  // 0x97
    { SBC_A_B,      0,  4,  "SBC_A_B"   },  // 0x98
    { SBC_A_C,      0,  4,  "SBC_A_C"   },  // 0x99
    { SBC_A_D,      0,  4,  "SBC_A_D"   },  // 0x9A
    { SBC_A_E,      0,  4,  "SBC_A_E"   },  // 0x9B
    { SBC_A_H,      0,  4,  "SBC_A_H"   },  // 0x9C
    { SBC_A_L,      0,  4,  "SBC_A_L"   },  // 0x9D
    { SBC_A_HL,     0,  4,  "SBC_A_HL"  },  // 0x9E
    { SBC_A_A,      0,  4,  "SBC_A_A"   },  // 0x9F
    { AND_B,        0,  4,  "AND_B"     },  // 0xA0
    { AND_C,        0,  4,  "AND_C"     },  // 0xA1
    { AND_D,        0,  4,  "AND_D"     },  // 0xA2
    { AND_E,        0,  4,  "AND_E"     },  // 0xA3
    { AND_H,        0,  4,  "AND_H"     },  // 0xA4
    { AND_L,        0,  4,  "AND_L"     },  // 0xA5
    { AND_HL,       0,  8,  "AND_HL"    },  // 0xA6
    { AND_A,        0,  4,  "AND_A"     },  // 0xA7
    { XOR_B,        0,  4,  "XOR_B"     },  // 0xA8
    { XOR_C,        0,  4,  "XOR_C"     },  // 0xA9
    { XOR_D,        0,  4,  "XOR_D"     },  // 0xAA
    { XOR_E,        0,  4,  "XOR_E"     },  // 0xAB
    { XOR_H,        0,  4,  "XOR_H"     },  // 0xAC
    { XOR_L,        0,  4,  "XOR_L"     },  // 0xAD
    { XOR_HL,       0,  8,  "XOR_HL"    },  // 0xAE
    { XOR_A,        0,  4,  "XOR_A"     },  // 0xAF
    { OR_B,         0,  4,  "OR_B"      },  // 0xB0
    { OR_C,         0,  4,  "OR_C"      },  // 0xB1
    { OR_D,         0,  4,  "OR_D"      },  // 0xB2
    { OR_E,         0,  4,  "OR_E"      },  // 0xB3
    { OR_H,         0,  4,  "OR_H"      },  // 0xB4
    { OR_L,         0,  4,  "OR_L"      },  // 0xB5
    { OR_HL,        0,  8,  "OR_HL"     },  // 0xB6
    { OR_A,         0,  4,  "OR_A"      },  // 0xB7
    { CP_B,         0,  4,  "CP_B"      },  // 0xB8
    { CP_C,         0,  4,  "CP_C"      },  // 0xB9
    { CP_D,         0,  4,  "CP_D"      },  // 0xBA
    { CP_E,         0,  4,  "CP_E"      },  // 0xBB
    { CP_H,         0,  4,  "CP_H"      },  // 0xBC
    { CP_L,         0,  4,  "CP_L"      },  // 0xBD
    { CP_HL,        0,  8,  "CP_HL"     },  // 0xBE
    { CP_A,         0,  4,  "CP_A"      },  // 0xBF
    { RET_NZ,       0,  8,  "RET_NZ"    },  // 0xC0
    { POP_BC,       0, 12,  "POP_BC"    },  // 0xC1
    { JP_NZ_nn,     2, 12,  "JP_NZ_nn"  },  // 0xC2
    { JP_nn,        2, 16,  "JP_nn"     },  // 0xC3
    { CALL_NZ_nn,   2, 12,  "CALL_NZ_nn"},  // 0xC4
    { PUSH_BC,      0, 16,  "PUSH_BC"   },  // 0xC5
    { ADD_A_n,      1,  8,  "ADD_A_n"   },  // 0xC6
    { RST00,        0, 16,  "RST00"     },  // 0xC7
    { RET_Z,        0,  8,  "RET_Z"     },  // 0xC8
    { RET,          0, 16,  "RET"       },  // 0xC9
    { JP_Z_nn,      2, 12,  "JP_Z_nn"   },  // 0xCA
    { CB,           3,  4,  "CB"        },  // 0xCB
    { CALL_Z_nn,    2, 12,  "CALL_Z_nn" },  // 0xCC
    { CALL_nn,      2, 24,  "CALL_nn"   },  // 0xCD
    { ADC_A_n,      1,  8,  "ADC_A_n"   },  // 0xCE
    { RST08,        0, 16,  "RST08"     },  // 0xCF
    { RET_NC,       0,  8,  "RET_NC"    },  // 0xD0
    { POP_DE,       0, 12,  "POP_DE"    },  // 0xD1
    { JP_NC_nn,     2, 12,  "JP_NC_nn"  },  // 0xD2
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xD3
    { CALL_NC_nn,   2, 12,  "CALL_NC_nn"},  // 0xD4
    { PUSH_DE,      0, 16,  "PUSH_DE"   },  // 0xD5
    { SUB_n,        1,  8,  "SUB_n"     },  // 0xD6
    { RST10,        0, 16,  "RST10"     },  // 0xD7
    { RET_C,        0,  8,  "RET_C"     },  // 0xD8
    { RETI,         0, 16,  "RETI"      },  // 0xD9
    { JP_C_nn,      2, 12,  "JP_C_nn"   },  // 0xDA
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xDB
    { CALL_C_nn,    2, 12,  "CALL_C_nn" },  // 0xDC
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xDD
    { SBC_A_n,      1,  8,  "SBC_A_n"   },  // 0xDE
    { RST18,        0, 16,  "RST18"     },  // 0xDF
    { LDH_n_A,      1, 12,  "LDH_n_A"   },  // 0xE0
    { POP_HL,       0, 12,  "POP_HL"    },  // 0xE1
    { LD_MC_A,      0,  8,  "LD_MC_A"   },  // 0xE2
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xE3
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xE4
    { PUSH_HL,      0, 16,  "PUSH_HL"   },  // 0xE5
    { AND_n,        1,  8,  "AND_n"     },  // 0xE6
    { RST20,        0, 16,  "RST20"     },  // 0xE7
    { ADD_SP_n,     1, 16,  "ADD_SP_n"  },  // 0xE8
    { JP_HL,        0,  4,  "JP_HL"     },  // 0xE9
    { LD_nn_A,      2, 16,  "LD_nn_A"   },  // 0xEA
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xEB
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xEC
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xED
    { XOR_n,        1,  8,  "XOR_n"     },  // 0xEE
    { RST28,        0, 16,  "RST28"     },  // 0xEF
    { LDH_A_n,      1, 12,  "LDH_A_n"   },  // 0xF0
    { POP_AF,       0, 12,  "POP_AF"    },  // 0xF1
    { LD_A_MC,      0,  8,  "LD_A_MC"   },  // 0xF2
    { DI,           0,  4,  "DI"        },  // 0xF3
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xF4
    { PUSH_AF,      0, 16,  "PUSH_AF"   },  // 0xF5
    { OR_n,         1,  8,  "OR_n"      },  // 0xF6
    { RST30,        0, 16,  "RST30"     },  // 0xF7
    { LDHL_SP_n,    1, 12,  "LDHL_SP_n" },  // 0xF8
    { LD_SP_HL,     0,  8,  "LD_SP_HL"  },  // 0xF9
    { LD_A_nn,      2, 16,  "LD_A_nn"   },  // 0xFA
    { EI,           0,  4,  "EI"        },  // 0xFB
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xFC
    { NOTVALID,     0,  0,  "NOTVALID"  },  // 0xFD
    { CP_n,         1,  8,  "CP_n"      },  // 0xFE
    { RST38,        0, 16,  "RST38"     },  // 0xFF
};


const struct extendedopCode extendedopCodes[256] = {
    { RLC_B,        0,  8               },  // 0x00
    { RLC_C,        0,  8               },  // 0x01
    { RLC_D,        0,  8               },  // 0x02
    { RLC_E,        0,  8               },  // 0x03
    { RLC_H,        0,  8               },  // 0x04
    { RLC_L,        0,  8               },  // 0x05
    { RLC_HL,       0, 16               },  // 0x06
    { RLC_A,        0,  8               },  // 0x07
    { RRC_B,        0,  8               },  // 0x08
    { RRC_C,        0,  8               },  // 0x09
    { RRC_D,        0,  8               },  // 0x0A
    { RRC_E,        0,  8               },  // 0x0B
    { RRC_H,        0,  8               },  // 0x0C
    { RRC_L,        0,  8               },  // 0x0D
    { RRC_HL,       0, 16               },  // 0x0E
    { RRC_A,        0,  8               },  // 0x0F
    { RL_B,         0,  8               },  // 0x10
    { RL_C,         0,  8               },  // 0x11
    { RL_D,         0,  8               },  // 0x12
    { RL_E,         0,  8               },  // 0x13
    { RL_H,         0,  8               },  // 0x14
    { RL_L,         0,  8               },  // 0x15
    { RL_HL,        0, 16               },  // 0x16
    { RL_A,         0,  8               },  // 0x17
    { RR_B,         0,  8               },  // 0x18
    { RR_C,         0,  8               },  // 0x19
    { RR_D,         0,  8               },  // 0x1A
    { RR_E,         0,  8               },  // 0x1B
    { RR_H,         0,  8               },  // 0x1C
    { RR_L,         0,  8               },  // 0x1D
    { RR_HL,        0, 16               },  // 0x1E
    { RR_A,         0,  8               },  // 0x1F
    { SLA_B,        0,  8               },  // 0x20
    { SLA_C,        0,  8               },  // 0x21
    { SLA_D,        0,  8               },  // 0x22
    { SLA_E,        0,  8               },  // 0x23
    { SLA_H,        0,  8               },  // 0x24
    { SLA_L,        0,  8               },  // 0x25
    { SLA_HL,       0, 16               },  // 0x26
    { SLA_A,        0,  8               },  // 0x27
    { SRA_B,        0,  8               },  // 0x28
    { SRA_C,        0,  8               },  // 0x29
    { SRA_D,        0,  8               },  // 0x2A
    { SRA_E,        0,  8               },  // 0x2B
    { SRA_H,        0,  8               },  // 0x2C
    { SRA_L,        0,  8               },  // 0x2D
    { SRA_HL,       0, 16               },  // 0x2E
    { SRA_A,        0,  8               },  // 0x2F
    { SWAP_B,       0,  8               },  // 0x30
    { SWAP_C,       0,  8               },  // 0x31
    { SWAP_D,       0,  8               },  // 0x32
    { SWAP_E,       0,  8               },  // 0x33
    { SWAP_H,       0,  8               },  // 0x34
    { SWAP_L,       0,  8               },  // 0x35
    { SWAP_HL,      0, 16               },  // 0x36
    { SWAP_A,       0,  8               },  // 0x37
    { SRL_B,        0,  8               },  // 0x38
    { SRL_C,        0,  8               },  // 0x39
    { SRL_D,        0,  8               },  // 0x3A
    { SRL_E,        0,  8               },  // 0x3B
    { SRL_H,        0,  8               },  // 0x3C
    { SRL_L,        0,  8               },  // 0x3D
    { SRL_HL,       0, 16               },  // 0x3E
    { SRL_A,        0,  8               },  // 0x3F
    { BIT_0_B,      0,  8               },  // 0x40
    { BIT_0_C,      0,  8               },  // 0x41
    { BIT_0_D,      0,  8               },  // 0x42
    { BIT_0_E,      0,  8               },  // 0x43
    { BIT_0_H,      0,  8               },  // 0x44
    { BIT_0_L,      0,  8               },  // 0x45
    { BIT_0_HL,     0, 16               },  // 0x46
    { BIT_0_A,      0,  8               },  // 0x47
    { BIT_1_B,      0,  8               },  // 0x48
    { BIT_1_C,      0,  8               },  // 0x49
    { BIT_1_D,      0,  8               },  // 0x4A
    { BIT_1_E,      0,  8               },  // 0x4B
    { BIT_1_H,      0,  8               },  // 0x4C
    { BIT_1_L,      0,  8               },  // 0x4D
    { BIT_1_HL,     0, 16               },  // 0x4E
    { BIT_1_A,      0,  8               },  // 0x4F
    { BIT_2_B,      0,  8               },  // 0x50
    { BIT_2_C,      0,  8               },  // 0x51
    { BIT_2_D,      0,  8               },  // 0x52
    { BIT_2_E,      0,  8               },  // 0x53
    { BIT_2_H,      0,  8               },  // 0x54
    { BIT_2_L,      0,  8               },  // 0x55
    { BIT_2_HL,     0, 16               },  // 0x56
    { BIT_2_A,      0,  8               },  // 0x57
    { BIT_3_B,      0,  8               },  // 0x58
    { BIT_3_C,      0,  8               },  // 0x59
    { BIT_3_D,      0,  8               },  // 0x5A
    { BIT_3_E,      0,  8               },  // 0x5B
    { BIT_3_H,      0,  8               },  // 0x5C
    { BIT_3_L,      0,  8               },  // 0x5D
    { BIT_3_HL,     0, 16               },  // 0x5E
    { BIT_3_A,      0,  8               },  // 0x5F
    { BIT_4_B,      0,  8               },  // 0x60
    { BIT_4_C,      0,  8               },  // 0x61
    { BIT_4_D,      0,  8               },  // 0x62
    { BIT_4_E,      0,  8               },  // 0x63
    { BIT_4_H,      0,  8               },  // 0x64
    { BIT_4_L,      0,  8               },  // 0x65
    { BIT_4_HL,     0, 16               },  // 0x66
    { BIT_4_A,      0,  8               },  // 0x67
    { BIT_5_B,      0,  8               },  // 0x68
    { BIT_5_C,      0,  8               },  // 0x69
    { BIT_5_D,      0,  8               },  // 0x6A
    { BIT_5_E,      0,  8               },  // 0x6B
    { BIT_5_H,      0,  8               },  // 0x6C
    { BIT_5_L,      0,  8               },  // 0x6D
    { BIT_5_HL,     0, 16               },  // 0x6E
    { BIT_5_A,      0,  8               },  // 0x6F
    { BIT_6_B,      0,  8               },  // 0x70
    { BIT_6_C,      0,  8               },  // 0x71
    { BIT_6_D,      0,  8               },  // 0x72
    { BIT_6_E,      0,  8               },  // 0x73
    { BIT_6_H,      0,  8               },  // 0x74
    { BIT_6_L,      0,  8               },  // 0x75
    { BIT_6_HL,     0, 16               },  // 0x76
    { BIT_6_A,      0,  8               },  // 0x77
    { BIT_7_B,      0,  8               },  // 0x78
    { BIT_7_C,      0,  8               },  // 0x79
    { BIT_7_D,      0,  8               },  // 0x7A
    { BIT_7_E,      0,  8               },  // 0x7B
    { BIT_7_H,      0,  8               },  // 0x7C
    { BIT_7_L,      0,  8               },  // 0x7D
    { BIT_7_HL,     0, 16               },  // 0x7E
    { BIT_7_A,      0,  8               },  // 0x7F
    { RES_0_B,      0,  8               },  // 0x80
    { RES_0_C,      0,  8               },  // 0x81
    { RES_0_D,      0,  8               },  // 0x82
    { RES_0_E,      0,  8               },  // 0x83
    { RES_0_H,      0,  8               },  // 0x84
    { RES_0_L,      0,  8               },  // 0x85
    { RES_0_HL,     0, 16               },  // 0x86
    { RES_0_A,      0,  8               },  // 0x87
    { RES_1_B,      0,  8               },  // 0x88
    { RES_1_C,      0,  8               },  // 0x89
    { RES_1_D,      0,  8               },  // 0x8A
    { RES_1_E,      0,  8               },  // 0x8B
    { RES_1_H,      0,  8               },  // 0x8C
    { RES_1_L,      0,  8               },  // 0x8D
    { RES_1_HL,     0, 16               },  // 0x8E
    { RES_1_A,      0,  8               },  // 0x8F
    { RES_2_B,      0,  8               },  // 0x90
    { RES_2_C,      0,  8               },  // 0x91
    { RES_2_D,      0,  8               },  // 0x92
    { RES_2_E,      0,  8               },  // 0x93
    { RES_2_H,      0,  8               },  // 0x94
    { RES_2_L,      0,  8               },  // 0x95
    { RES_2_HL,     0, 16               },  // 0x96
    { RES_2_A,      0,  8               },  // 0x97
    { RES_3_B,      0,  8               },  // 0x98
    { RES_3_C,      0,  8               },  // 0x99
    { RES_3_D,      0,  8               },  // 0x9A
    { RES_3_E,      0,  8               },  // 0x9B
    { RES_3_H,      0,  8               },  // 0x9C
    { RES_3_L,      0,  8               },  // 0x9D
    { RES_3_HL,     0, 16               },  // 0x9E
    { RES_3_A,      0,  8               },  // 0x9F
    { RES_4_B,      0,  8               },  // 0xA0
    { RES_4_C,      0,  8               },  // 0xA1
    { RES_4_D,      0,  8               },  // 0xA2
    { RES_4_E,      0,  8               },  // 0xA3
    { RES_4_H,      0,  8               },  // 0xA4
    { RES_4_L,      0,  8               },  // 0xA5
    { RES_4_HL,     0, 16               },  // 0xA6
    { RES_4_A,      0,  8               },  // 0xA7
    { RES_5_B,      0,  8               },  // 0xA8
    { RES_5_C,      0,  8               },  // 0xA9
    { RES_5_D,      0,  8               },  // 0xAA
    { RES_5_E,      0,  8               },  // 0xAB
    { RES_5_H,      0,  8               },  // 0xAC
    { RES_5_L,      0,  8               },  // 0xAD
    { RES_5_HL,     0, 16               },  // 0xAE
    { RES_5_A,      0,  8               },  // 0xAF
    { RES_6_B,      0,  8               },  // 0xB0
    { RES_6_C,      0,  8               },  // 0xB1
    { RES_6_D,      0,  8               },  // 0xB2
    { RES_6_E,      0,  8               },  // 0xB3
    { RES_6_H,      0,  8               },  // 0xB4
    { RES_6_L,      0,  8               },  // 0xB5
    { RES_6_HL,     0, 16               },  // 0xB6
    { RES_6_A,      0,  8               },  // 0xB7
    { RES_7_B,      0,  8               },  // 0xB8
    { RES_7_C,      0,  8               },  // 0xB9
    { RES_7_D,      0,  8               },  // 0xBA
    { RES_7_E,      0,  8               },  // 0xBB
    { RES_7_H,      0,  8               },  // 0xBC
    { RES_7_L,      0,  8               },  // 0xBD
    { RES_7_HL,     0, 16               },  // 0xBE
    { RES_7_A,      0,  8               },  // 0xBF
    { SET_0_B,      0,  8               },  // 0xC0
    { SET_0_C,      0,  8               },  // 0xC1
    { SET_0_D,      0,  8               },  // 0xC2
    { SET_0_E,      0,  8               },  // 0xC3
    { SET_0_H,      0,  8               },  // 0xC4
    { SET_0_L,      0,  8               },  // 0xC5
    { SET_0_HL,     0, 16               },  // 0xC6
    { SET_0_A,      0,  8               },  // 0xC7
    { SET_1_B,      0,  8               },  // 0xC8
    { SET_1_C,      0,  8               },  // 0xC9
    { SET_1_D,      0,  8               },  // 0xCA
    { SET_1_E,      0,  8               },  // 0xCB
    { SET_1_H,      0,  8               },  // 0xCC
    { SET_1_L,      0,  8               },  // 0xCD
    { SET_1_HL,     0, 16               },  // 0xCE
    { SET_1_A,      0,  8               },  // 0xCF
    { SET_2_B,      0,  8               },  // 0xD0
    { SET_2_C,      0,  8               },  // 0xD1
    { SET_2_D,      0,  8               },  // 0xD2
    { SET_2_E,      0,  8               },  // 0xD3
    { SET_2_H,      0,  8               },  // 0xD4
    { SET_2_L,      0,  8               },  // 0xD5
    { SET_2_HL,     0, 16               },  // 0xD6
    { SET_2_A,      0,  8               },  // 0xD7
    { SET_3_B,      0,  8               },  // 0xD8
    { SET_3_C,      0,  8               },  // 0xD9
    { SET_3_D,      0,  8               },  // 0xDA
    { SET_3_E,      0,  8               },  // 0xDB
    { SET_3_H,      0,  8               },  // 0xDC
    { SET_3_L,      0,  8               },  // 0xDD
    { SET_3_HL,     0, 16               },  // 0xDE
    { SET_3_A,      0,  8               },  // 0xDF
    { SET_4_B,      0,  8               },  // 0xE0
    { SET_4_C,      0,  8               },  // 0xE1
    { SET_4_D,      0,  8               },  // 0xE2
    { SET_4_E,      0,  8               },  // 0xE3
    { SET_4_H,      0,  8               },  // 0xE4
    { SET_4_L,      0,  8               },  // 0xE5
    { SET_4_HL,     0, 16               },  // 0xE6
    { SET_4_A,      0,  8               },  // 0xE7
    { SET_5_B,      0,  8               },  // 0xE8
    { SET_5_C,      0,  8               },  // 0xE9
    { SET_5_D,      0,  8               },  // 0xEA
    { SET_5_E,      0,  8               },  // 0xEB
    { SET_5_H,      0,  8               },  // 0xEC
    { SET_5_L,      0,  8               },  // 0xED
    { SET_5_HL,     0, 16               },  // 0xEE
    { SET_5_A,      0,  8               },  // 0xEF
    { SET_6_B,      0,  8               },  // 0xF0
    { SET_6_C,      0,  8               },  // 0xF1
    { SET_6_D,      0,  8               },  // 0xF2
    { SET_6_E,      0,  8               },  // 0xF3
    { SET_6_H,      0,  8               },  // 0xF4
    { SET_6_L,      0,  8               },  // 0xF5
    { SET_6_HL,     0, 16               },  // 0xF6
    { SET_6_A,      0,  8               },  // 0xF7
    { SET_7_B,      0,  8               },  // 0xF8
    { SET_7_C,      0,  8               },  // 0xF9
    { SET_7_D,      0,  8               },  // 0xFA
    { SET_7_E,      0,  8               },  // 0xFB
    { SET_7_H,      0,  8               },  // 0xFC
    { SET_7_L,      0,  8               },  // 0xFD
    { SET_7_HL,     0, 16               },  // 0xFE
    { SET_7_A,      0,  8               },  // 0xFF
};

void execute (void){
    
/*  CPU STATE CHECK */

/* Opcode that enables IME has an internal delay
 * In order to emulate this behavior, we introduce
 * an internal timer.
 * After IE execution, the next opcode is executed
 * as IME = 0.
 */
    if (cpustate.ime_delay > 0){            // IME is about to change
        cpustate.ime_delay -= 1;            // Decrease IME delay timer
        if (cpustate.ime_delay == 0){       // if timer expired
            cpustate.ime = TRUE;            // Enable MASTER INTERRUPT
        }        
    }

/* Opcode HALT halts cpu execution. Internal
 * clock is ticking waiting for an interrupt to
 * occur.
 * In case IME = 1, execution continues with 
 * handling the interrupt.
 * In case IME = 0, execution continues with
 * next opcode.
 */
    if (cpustate.halt == TRUE){             // CPU is HALTED
        hardwareTick();
        return;
    }
/* If IME='0' and CPU is halted, when any interrupt 
 * is triggered by setting any IF flag to '1' with the
 * corresponding bit in IE set to '1', it takes 4 clocks
 * to exit halt mode, even if the CPU doesn't jump to
 * the interrupt vector.
 */
    if (cpustate.interrupt == TRUE){
        cpustate.interrupt = FALSE;
        //hardwareTick();
    }
 
/* If IME = 1, check for interrupts */
    if (cpustate.ime == TRUE){
        handleInterrupts();
    }

/*  CPU OPERATION */

    int operand_length;                         // Decide how many octets to fetch after opcode
    unsigned char instruction;                  // Instruction to be exectued
    int extended_opcode = FALSE;                // Extended opcode FLAG


    if (registers.PC+1 == debug_pc)
        debug_mooneye = TRUE;
    
    instruction = readMemory8( registers.PC );  // Fetch next opcode

    previous = current;
    current = instruction;
    
    if (debug_test_run){
        if (current == 0x00 && previous == 0x18){
            if (registers.A == 0x00){
                printf("[DEBUG]Test Passed\n");
                exit(0);
            }
            else{
                printf("[DEBUG]Test Failed\n");
                //printf("[DEBUG] A=%3d,B=%3d,C=%3d,D=%3d,E=%3d,F=%3d,H=%3d,L=%3d\n",registers.A,registers.B,registers.C,registers.D,registers.E,registers.F,registers.H,registers.L);
                exit(1);
            }
        }
    }


/* TODO
    if (cpustate.repeat == TRUE){
        registers.PC -= 1;
        cpustate.repeat = FALSE;
    }
 */   

    if (instruction == 0xCB){
        
        
        instruction = readMemory8(++registers.PC);

        if (debug_mooneye)
            printf("[DEBUG] EPC-0x%04x, PC-0x%04x, SP-0x%04x, A=0x%02x, B=0x%02x, C=0x%02x, D=0x%02x, E=0x%02x, F=0x%02x, H=0x%02x, L=0x%02x\n",
            instruction,registers.PC+1,registers.SP,registers.A,registers.B,registers.C,registers.D,registers.E,registers.F,registers.H,registers.L);   
        
        operand_length = extendedopCodes[instruction].opLength;
        extended_opcode = TRUE;
    }
    else{
        operand_length = opCodes[instruction].opLength;
    }    

        if (debug_mooneye && extended_opcode == 0)
            printf("[DEBUG] OPC-0x%04x, PC-0x%04x, SP-0x%04x, A=0x%02x, B=0x%02x, C=0x%02x, D=0x%02x, E=0x%02x, F=0x%02x, H=0x%02x, L=0x%02x\n",
            instruction,registers.PC+1,registers.SP,registers.A,registers.B,registers.C,registers.D,registers.E,registers.F,registers.H,registers.L);   
   
    switch ( operand_length ){
        case 0 :
            registers.PC = registers.PC + 1;    // Increment PC to be ready for next cycle

            break;
        case 1 :
            operand8 = readMemory8( registers.PC + 1 ); // Fetch 8-bit operand
            registers.PC = registers.PC + 2;    // Increment PC to be ready for next cycle

            break;
        case 2 :
            operand16 = readMemory16( registers.PC + 1 ); // Fetch 16-bit operand
            registers.PC = registers.PC + 3;    // Increment PC to be ready for next cycle

            break;
        default:
            break;
    };

    if (extended_opcode){
        ((void (*)(void))extendedopCodes[instruction].function)();
    }
    else{
        ((void (*)(void))opCodes[instruction].function)();
    }
    
        
    return;
}

void cpuReset()
{
    cpustate.halt = FALSE;
    cpustate.stop = FALSE;
    cpustate.ime = TRUE;
    cpustate.ime_delay = 0;
    cpustate.interrupt = FALSE;
    cpustate.repeat = FALSE;
}

void NOTVALID(void){
    printf("[ERROR] Reached not valid opcode.\n");
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
 void LD_HL_B (void) { writeMemory8(registers.HL, registers.B);} 
 void LD_HL_C (void) { writeMemory8(registers.HL, registers.C);}
 void LD_HL_D (void) { writeMemory8(registers.HL, registers.D);}
 void LD_HL_E (void) { writeMemory8(registers.HL, registers.E);}
 void LD_HL_H (void) { writeMemory8(registers.HL, registers.H);}
 void LD_HL_L (void) { writeMemory8(registers.HL, registers.L);}
 void LD_HL_n (void) { writeMemory8(registers.HL, operand8);}
 
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
 void LD_BC_A (void){writeMemory8(registers.BC, registers.A);}
 void LD_DE_A (void){writeMemory8(registers.DE, registers.A);}
 void LD_HL_A (void){writeMemory8(registers.HL, registers.A);}
 void LD_nn_A (void){writeMemory8(operand16, registers.A);}

/*
 * LD A,(C)
 * Description: Put value at address $FF00 + register C into A.
 */
void LD_A_MC (void) { registers.A = readMemory8(0xFF00 + registers.C);} 
 /*
  * LD (C),A
  * Description: Put A into address $FF00 + register C.
  */
  void LD_MC_A (void) {writeMemory8(0xFF00 + registers.C, registers.A);}
/*
 * LDD A,(HL)
 * Description: Put value at address HL into A. Decrement HL.
 * Same as: LD A,(HL) - DEC HL
 */
void LDD_A_HL (void){
    registers.A = readMemory8(registers.HL);
    registers.HL--;
}
  
/*
 * LDD (HL),A
 * Description:
 * Put A into memory address HL. Decrement HL.
 * Same as: LD (HL),A - DEC HL
 */
void LDD_HL_A (void) {    
    writeMemory8(registers.HL, registers.A);
    registers.HL--;
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
     writeMemory8(registers.HL, registers.A);
     registers.HL++;
 }
 
/*
 * LDH (n),A
 * Description: Put A into memory address $FF00+n.
 * Use with: n = one byte immediate value.
 */
void LDH_n_A (void) { writeMemory8(0xFF00 + operand8, registers.A); }
/*
 * LDH A,(n)
 * Description: Put memory address $FF00+n into A.
 * Use with: n = one byte immediate value.
 */
void LDH_A_n (void) { registers.A = readMemory8(0xFF00 + operand8); }
 
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
 * LD SP,HL
 * Description: Put HL into Stack Pointer (SP).
 */
void LD_SP_HL (void) { registers.SP = registers.HL; hardwareTick(); }

/* 
 * LD HL,SP+n
 * LDHL SP,n
 * Description: Put SP + n effective address into HL.
 * Use with: n = one byte signed immediate value.
 * Flags affected:
 * Z - Reset.
 * N - Reset.
 * H - Set or reset according to operation.
 * C - Set or reset according to operation.
 */
void LDHL_SP_n (void){
    
    unsigned short result = registers.SP + (signed char)operand8;
    
    if ((result & 0xFF) < (registers.SP & 0xFF))
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);

    if ((result & 0x0F) < (registers.SP & 0x0F))
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    registers.HL = registers.SP + (signed char)operand8;
    
    resetFlag(ZERO_F);
    resetFlag(SUBSTRACT_F);
    
    hardwareTick();   

} 
/*
 * LD (nn),SP
 * Description: Put Stack Pointer (SP) at address n.
 * Use with: nn = two byte immediate address.
 */
 void LD_nn_SP (void) { writeMemory16(operand16, registers.SP); }
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
 void POP_AF (void){registers.AF = stackPop16(); registers.F &= 0xF0;}
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
void ADD_A_A (void){ add (registers.A); }
void ADD_A_B (void){ add (registers.B); }
void ADD_A_C (void){ add (registers.C); }
void ADD_A_D (void){ add (registers.D); }
void ADD_A_E (void){ add (registers.E); }
void ADD_A_H (void){ add (registers.H); }
void ADD_A_L (void){ add (registers.L); }
void ADD_A_HL (void){ add (readMemory8(registers.HL)); }
void ADD_A_n (void){ add (operand8); }
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
void ADC_A_A (void){ adc (registers.A); }
void ADC_A_B (void){ adc (registers.B); }
void ADC_A_C (void){ adc (registers.C); }
void ADC_A_D (void){ adc (registers.D); }
void ADC_A_E (void){ adc (registers.E); }
void ADC_A_H (void){ adc (registers.H); }
void ADC_A_L (void){ adc (registers.L); }
void ADC_A_HL (void){ adc (readMemory8(registers.HL)); }
void ADC_A_n (void){ adc (operand8); }
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
 * SBC A,n
 * Description: Subtract n + Carry flag from A.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero.
 * N - Set.
 * H - Set if no borrow from bit 4.
 * C - Set if no borrow.
 */
void SBC_A_A (void){ sbc (registers.A); }
void SBC_A_B (void){ sbc (registers.B); }
void SBC_A_C (void){ sbc (registers.C); }
void SBC_A_D (void){ sbc (registers.D); }
void SBC_A_E (void){ sbc (registers.E); }
void SBC_A_H (void){ sbc (registers.H); }
void SBC_A_L (void){ sbc (registers.L); }
void SBC_A_HL (void){ sbc (readMemory8(registers.HL)); }
void SBC_A_n (void){ sbc (operand8); }
  
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
 void AND_A (void) {and (registers.A);}
 void AND_B (void) {and (registers.B);}
 void AND_C (void) {and (registers.C);}
 void AND_D (void) {and (registers.D);}
 void AND_E (void) {and (registers.E);}
 void AND_H (void) {and (registers.H);}
 void AND_L (void) {and (registers.L);}
 void AND_HL (void) {and (readMemory8(registers.HL));}
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
 void CP_A (void) {cp (registers.A);}
 void CP_B (void) {cp (registers.B);}
 void CP_C (void) {cp (registers.C);}
 void CP_D (void) {cp (registers.D);}
 void CP_E (void) {cp (registers.E);}
 void CP_H (void) {cp (registers.H);}
 void CP_L (void) {cp (registers.L);}
 void CP_HL (void) {cp (readMemory8(registers.HL));}
 void CP_n (void) {cp (operand8);}
 
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
 
 void INC_A (void) {registers.A = inc (registers.A);}
 void INC_B (void) {registers.B = inc (registers.B);}
 void INC_C (void) {registers.C = inc (registers.C);}
 void INC_D (void) {registers.D = inc (registers.D);}
 void INC_E (void) {registers.E = inc (registers.E);}
 void INC_H (void) {registers.H = inc (registers.H);}
 void INC_L (void) {registers.L = inc (registers.L);}
 void INC_MHL (void) {writeMemory8(registers.HL, inc (readMemory8(registers.HL)));}
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
void DEC_A (void) {registers.A = dec (registers.A);}
void DEC_B (void) {registers.B = dec (registers.B);}
void DEC_C (void) {registers.C = dec (registers.C);}
void DEC_D (void) {registers.D = dec (registers.D);}
void DEC_E (void) {registers.E = dec (registers.E);}
void DEC_H (void) {registers.H = dec (registers.H);}
void DEC_L (void) {registers.L = dec (registers.L);}
void DEC_MHL (void) {writeMemory8(registers.HL, dec (readMemory8(registers.HL)));}
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
 void ADD_HL_BC (void) {add16 (registers.BC); hardwareTick(); }
 void ADD_HL_DE (void) {add16 (registers.DE); hardwareTick(); }
 void ADD_HL_HL (void) {add16 (registers.HL); hardwareTick(); }
 void ADD_HL_SP (void) {add16 (registers.SP); hardwareTick(); }
/*
 * ADD SP,n
 * Description: Add n to Stack Pointer (SP).
 * Use with: n = one byte signed immediate value (#).
 * Flags affected:
 * Z - Reset.
 * N - Reset.
 * H - Set or reset according to operation.
 * C - Set or reset according to operation.
 */
void ADD_SP_n (void){
    
    unsigned short result = registers.SP + (signed char)operand8;
    
    if ((result & 0xFF) < (registers.SP & 0xFF))
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);

    if ((result & 0x0F) < (registers.SP & 0x0F))
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    registers.SP += (signed char)operand8;
    
    resetFlag(ZERO_F);
    resetFlag(SUBSTRACT_F);
    //updateTimers(8);
    hardwareTick();
    hardwareTick();
} 
/*
 * INC nn
 * Description: Increment register nn.
 * Use with: nn = BC,DE,HL,SP
 * Flags affected:
 * None.
 */
 void INC_BC (void) {registers.BC++; hardwareTick(); }
 void INC_DE (void) {registers.DE++; hardwareTick(); }
 void INC_HL (void) {registers.HL++; hardwareTick(); }
 void INC_SP (void) {registers.SP++; hardwareTick(); }
/*
 * DEC nn
 * Description: Decrement register nn.
 * Use with: nn = BC,DE,HL,SP
 * Flags affected: None.
 */
 
 void DEC_BC (void) {registers.BC--; hardwareTick(); }
 void DEC_DE (void) {registers.DE--; hardwareTick(); }
 void DEC_HL (void) {registers.HL--; hardwareTick(); }
 void DEC_SP (void) {registers.SP--; hardwareTick(); }
 
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

void DAA (void){

    unsigned short correction = registers.A;

    if (testFlag(SUBSTRACT_F)){
        if (testFlag(HALF_CARRY_F))
            correction = ( correction - 0x06 ) & 0xFF;
        if (testFlag(CARRY_F))
            correction -= 0x60;
    }
    else{
        // if the lower 4 bits form a number greater than 9 or H is set, add $06 to the accumulator
        if ((( correction & 0x0F ) > 0x09) || ( testFlag( HALF_CARRY_F ) == TRUE ))
            correction += 0x06;
        // if the upper 4 bits form a number greater than 9 or C is set, add $60 to the accumulator
        if (( correction > 0x9F ) || ( testFlag( CARRY_F ) == TRUE ))
            correction += 0x60;
    }
  
    resetFlag(HALF_CARRY_F);

    if ((correction & 0x100) == 0x100)
        setFlag(CARRY_F);
    
    correction &= 0xFF;
    
    if (correction == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);

    registers.A = correction;
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
     //registers.F = (registers.F)|((~(registers.F & 0x10)) & 0x10);
     if (testFlag(CARRY_F))
        resetFlag(CARRY_F);
     else
        setFlag(CARRY_F);
 }
/*
 * SCF
 * Description: Set Carry flag.
 * Flags affected:
 * Z - Not affected.
 * N - Reset.
 * H - Reset.
 * C - Set.
 */
void SCF (void){
    setFlag(CARRY_F);
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
}
/*
 * NOP
 * Description: No operation.
 */
void NOP (void){ };

/*
 * DI
 * Description:
 * This instruction disables interrupts but not immediately. 
 * Interrupts are disabled after instruction DI is executed.
 * Flags affected: None.
*/
void DI (void){ cpustate.ime = FALSE; cpustate.ime_delay = 0; }

/*
 * HALT
 * Description: Power down CPU until an interrupt occurs. Use this
 * when ever possible to reduce energy consumption.
 */
void HALT (void){ cpustate.halt = TRUE; }

/*
 * STOP
 * Description: Halt CPU & LCD display until button pressed.
 */
void STOP (void){
    cpustate.stop = TRUE;
}

/*
 * EI
 * Description: Enable interrupts. This intruction enables interrupts
 * but not immediately. Interrupts are enabled after instruction EI is executed.
 * Flags affected: None.
 */
void EI (void){ cpustate.ime_delay = 2; }
/********************
 * Rotates & Shifts *
 ********************/
/*
 * RLCA
 * Description: Rotate A left. Old bit 7 to Carry flag.
 * Flags affected:
 * Z - Set if result is zero. (Undocummented CPU Bug)
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
void RLCA (void) {registers.A = rlc(registers.A); resetFlag(ZERO_F); }

/*
 * RRCA
 * Description: Rotate A right. Old bit 0 to Carry flag.
 * Flags affected:
 * Z - Set if result is zero. (Undocummented CPU Bug)
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
void RRCA (void){ registers.A = rrc(registers.A); resetFlag(ZERO_F); }

/*
 * RL n
 * Description: Rotate n left through Carry flag.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
 void RL_A (void) {registers.A = rl(registers.A);}
 void RL_B (void) {registers.B = rl(registers.B);}
 void RL_C (void) {registers.C = rl(registers.C);}
 void RL_D (void) {registers.D = rl(registers.D);}
 void RL_E (void) {registers.E = rl(registers.E);}
 void RL_H (void) {registers.H = rl(registers.H);}
 void RL_L (void) {registers.L = rl(registers.L);}
 void RL_HL (void) {writeMemory8(registers.HL,rl(readMemory8(registers.HL)));}

/*
 * RR n
 * Description: Rotate n right through Carry flag.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
 void RR_A (void) {registers.A = rr(registers.A);}
 void RR_B (void) {registers.B = rr(registers.B);}
 void RR_C (void) {registers.C = rr(registers.C);}
 void RR_D (void) {registers.D = rr(registers.D);}
 void RR_E (void) {registers.E = rr(registers.E);}
 void RR_H (void) {registers.H = rr(registers.H);}
 void RR_L (void) {registers.L = rr(registers.L);}
 void RR_HL (void) {writeMemory8(registers.HL,rr(readMemory8(registers.HL)));}
 
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
void SLA_HL (void) {writeMemory8(registers.HL,sla(readMemory8(registers.HL)));}

/*
 * SRA n
 * Description: Shift n right into Carry. MSB doesn't change.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
void SRA_A (void) {registers.A = sra(registers.A);}
void SRA_B (void) {registers.B = sra(registers.B);}
void SRA_C (void) {registers.C = sra(registers.C);}
void SRA_D (void) {registers.D = sra(registers.D);}
void SRA_E (void) {registers.E = sra(registers.E);}
void SRA_H (void) {registers.H = sra(registers.H);}
void SRA_L (void) {registers.L = sra(registers.L);}
void SRA_HL (void) {writeMemory8(registers.HL,sra(readMemory8(registers.HL)));}

/*
 * RLA
 * Description: Rotate A left through Carry flag.
 * Flags affected:
 * Z - Set if result is zero.  (This seems wrong according to op table.)
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
 void RLA (void) {registers.A = rl(registers.A); resetFlag(ZERO_F); }

/*
 * RRA
 * Description: Rotate A right through Carry flag.
 * Flags affected:
 * Z - Set if result is zero. (This seems wrong according to op table.)
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
void RRA (void){registers.A = rr(registers.A); resetFlag(ZERO_F);}

/* RLC n
 * Description: Rotate n left. Old bit 7 to Carry flag.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
 void RLC_A (void) {registers.A = rlc(registers.A);}
 void RLC_B (void) {registers.B = rlc(registers.B);}
 void RLC_C (void) {registers.C = rlc(registers.C);}
 void RLC_D (void) {registers.D = rlc(registers.D);}
 void RLC_E (void) {registers.E = rlc(registers.E);}
 void RLC_H (void) {registers.H = rlc(registers.H);}
 void RLC_L (void) {registers.L = rlc(registers.L);}
 void RLC_HL (void) {writeMemory8(registers.HL,rlc(readMemory8(registers.HL)));}

/* RRC n
 * Description: Rotate n right. Old bit 0 to Carry flag.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
 void RRC_A (void) {registers.A = rrc(registers.A);}
 void RRC_B (void) {registers.B = rrc(registers.B);}
 void RRC_C (void) {registers.C = rrc(registers.C);}
 void RRC_D (void) {registers.D = rrc(registers.D);}
 void RRC_E (void) {registers.E = rrc(registers.E);}
 void RRC_H (void) {registers.H = rrc(registers.H);}
 void RRC_L (void) {registers.L = rrc(registers.L);}
 void RRC_HL (void) {writeMemory8(registers.HL,rrc(readMemory8(registers.HL)));}
 
/*
 * SRL n
 * Description: Shift n right into Carry. MSB set to 0.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
 void SRL_A (void) {registers.A = srl(registers.A);}
 void SRL_B (void) {registers.B = srl(registers.B);}
 void SRL_C (void) {registers.C = srl(registers.C);}
 void SRL_D (void) {registers.D = srl(registers.D);}
 void SRL_E (void) {registers.E = srl(registers.E);}
 void SRL_H (void) {registers.H = srl(registers.H);}
 void SRL_L (void) {registers.L = srl(registers.L);}
 void SRL_HL (void) {writeMemory8(registers.HL,srl(readMemory8(registers.HL)));}
/********************
 * Bit Opcodes      *
 ********************/
/********************
 * Jumps            *
 ********************/
void JP_nn (void)   { registers.PC = operand16; hardwareTick(); }
void JP_NZ_nn (void){ if (testFlag(ZERO_F)  == 0){ registers.PC = operand16; hardwareTick(); }}
void JP_Z_nn (void) { if (testFlag(ZERO_F)  == 1){ registers.PC = operand16; hardwareTick(); }}
void JP_NC_nn (void){ if (testFlag(CARRY_F) == 0){ registers.PC = operand16; hardwareTick(); }}
void JP_C_nn (void) { if (testFlag(CARRY_F) == 1){ registers.PC = operand16; hardwareTick(); }}
void JP_HL (void)   { registers.PC = registers.HL; }
void JR_n (void)    { registers.PC = registers.PC + (signed char)operand8; hardwareTick(); }
void JR_NZ_n (void) { if (testFlag(ZERO_F)  == 0){ registers.PC = registers.PC + (signed char)operand8; hardwareTick(); }}
void JR_Z_n (void)  { if (testFlag(ZERO_F)  == 1){ registers.PC = registers.PC + (signed char)operand8; hardwareTick(); }}
void JR_NC_n (void) { if (testFlag(CARRY_F) == 0){ registers.PC = registers.PC + (signed char)operand8; hardwareTick(); }}
void JR_C_n (void)  { if (testFlag(CARRY_F) == 1){ registers.PC = registers.PC + (signed char)operand8; hardwareTick(); }}
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
 
/*
 * CALL cc,nn
 * Description: Call address n if following condition is true:
 *               cc = NZ, Call if Z flag is reset
 *               cc = Z,  Call if Z flag is set
 *               cc = NC, Call if C flag is reset
 *               cc = C,  Call if C flag is set
 *
 * Use with: nn = two byte immediate value. (LS byte first.)
 */
 
 void CALL_NZ_nn (void){
     if (testFlag(ZERO_F)  == 0){
        stackPush16(registers.PC);
        registers.PC = operand16; 
     }
 }
 void CALL_Z_nn (void){
     if (testFlag(ZERO_F)){
        stackPush16(registers.PC);
        registers.PC = operand16; 
     }
 }
 void CALL_NC_nn (void){
     if (testFlag(CARRY_F)  == 0){
        stackPush16(registers.PC);
        registers.PC = operand16; 
     }
 }
 void CALL_C_nn (void){
     if (testFlag(CARRY_F)){
        stackPush16(registers.PC);
        registers.PC = operand16; 
     }
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
     //hardwareTick();
     stackPush16(registers.PC);
     registers.PC = 0x00;
}
 void RST08 (void) {
     //hardwareTick();
     stackPush16(registers.PC);
     registers.PC = 0x08;
}
 void RST10 (void) {
     //hardwareTick();
     stackPush16(registers.PC);
     registers.PC = 0x10;
}
 void RST18 (void) {
     //hardwareTick();
     stackPush16(registers.PC);
     registers.PC = 0x18;
}
 void RST20 (void) {
     //hardwareTick();
     stackPush16(registers.PC);
     registers.PC = 0x20;
}
 void RST28 (void) {
     //hardwareTick();
     stackPush16(registers.PC);
     registers.PC = 0x28;
}
 void RST30 (void) {
     //hardwareTick();
     stackPush16(registers.PC);
     registers.PC = 0x30;
}
 void RST38 (void) {
     //hardwareTick();
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
void RET (void){registers.PC = stackPop16(); hardwareTick(); }
/*
 * RET cc
 * Description: Return if following condition is true:
 * Use with:
 * cc = NZ, Return if Z flag is reset.
 * cc = Z, Return if Z flag is set.
 * cc = NC, Return if C flag is reset.
 */
void RET_NZ (void){ hardwareTick(); if (testFlag(ZERO_F)  == 0){ RET(); }}
void RET_Z  (void){ hardwareTick(); if (testFlag(ZERO_F)  == 1){ RET(); }}
void RET_NC (void){ hardwareTick(); if (testFlag(CARRY_F) == 0){ RET(); }}
void RET_C  (void){ hardwareTick(); if (testFlag(CARRY_F) == 1){ RET(); }}
/*
 * RETI
 * Description: Pop two bytes from stack & jump to that address then
 * enable interrupts.
 */
void RETI (void){ cpustate.ime = TRUE; RET(); }

/************************
 * Extended instructions*
 ************************/
void CB (void) {}

/*
 * SWAP n
 * Description: Swap upper & lower nibles of n.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Reset.
 */
void SWAP_A (void) { registers.A = swap(registers.A); }
void SWAP_B (void) { registers.B = swap(registers.B); }
void SWAP_C (void) { registers.C = swap(registers.C); }
void SWAP_D (void) { registers.D = swap(registers.D); }
void SWAP_E (void) { registers.E = swap(registers.E); }
void SWAP_H (void) { registers.H = swap(registers.H); }
void SWAP_L (void) { registers.L = swap(registers.L); }
void SWAP_HL (void) { writeMemory8(registers.HL, swap(readMemory8(registers.HL))); }

/*
 * SET b,r
 * Description: Set bit b in register r.
 * Use with: b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * None.
 */
void SET_0_A (void) { registers.A = set(0,registers.A); }
void SET_1_A (void) { registers.A = set(1,registers.A); }
void SET_2_A (void) { registers.A = set(2,registers.A); }
void SET_3_A (void) { registers.A = set(3,registers.A); }
void SET_4_A (void) { registers.A = set(4,registers.A); }
void SET_5_A (void) { registers.A = set(5,registers.A); }
void SET_6_A (void) { registers.A = set(6,registers.A); }
void SET_7_A (void) { registers.A = set(7,registers.A); }

void SET_0_B (void) { registers.B = set(0,registers.B); }
void SET_1_B (void) { registers.B = set(1,registers.B); }
void SET_2_B (void) { registers.B = set(2,registers.B); }
void SET_3_B (void) { registers.B = set(3,registers.B); }
void SET_4_B (void) { registers.B = set(4,registers.B); }
void SET_5_B (void) { registers.B = set(5,registers.B); }
void SET_6_B (void) { registers.B = set(6,registers.B); }
void SET_7_B (void) { registers.B = set(7,registers.B); }

void SET_0_C (void) { registers.C = set(0,registers.C); }
void SET_1_C (void) { registers.C = set(1,registers.C); }
void SET_2_C (void) { registers.C = set(2,registers.C); }
void SET_3_C (void) { registers.C = set(3,registers.C); }
void SET_4_C (void) { registers.C = set(4,registers.C); }
void SET_5_C (void) { registers.C = set(5,registers.C); }
void SET_6_C (void) { registers.C = set(6,registers.C); }
void SET_7_C (void) { registers.C = set(7,registers.C); }

void SET_0_D (void) { registers.D = set(0,registers.D); }
void SET_1_D (void) { registers.D = set(1,registers.D); }
void SET_2_D (void) { registers.D = set(2,registers.D); }
void SET_3_D (void) { registers.D = set(3,registers.D); }
void SET_4_D (void) { registers.D = set(4,registers.D); }
void SET_5_D (void) { registers.D = set(5,registers.D); }
void SET_6_D (void) { registers.D = set(6,registers.D); }
void SET_7_D (void) { registers.D = set(7,registers.D); }

void SET_0_E (void) { registers.E = set(0,registers.E); }
void SET_1_E (void) { registers.E = set(1,registers.E); }
void SET_2_E (void) { registers.E = set(2,registers.E); }
void SET_3_E (void) { registers.E = set(3,registers.E); }
void SET_4_E (void) { registers.E = set(4,registers.E); }
void SET_5_E (void) { registers.E = set(5,registers.E); }
void SET_6_E (void) { registers.E = set(6,registers.E); }
void SET_7_E (void) { registers.E = set(7,registers.E); }

void SET_0_H (void) { registers.H = set(0,registers.H); }
void SET_1_H (void) { registers.H = set(1,registers.H); }
void SET_2_H (void) { registers.H = set(2,registers.H); }
void SET_3_H (void) { registers.H = set(3,registers.H); }
void SET_4_H (void) { registers.H = set(4,registers.H); }
void SET_5_H (void) { registers.H = set(5,registers.H); }
void SET_6_H (void) { registers.H = set(6,registers.H); }
void SET_7_H (void) { registers.H = set(7,registers.H); }

void SET_0_L (void) { registers.L = set(0,registers.L); }
void SET_1_L (void) { registers.L = set(1,registers.L); }
void SET_2_L (void) { registers.L = set(2,registers.L); }
void SET_3_L (void) { registers.L = set(3,registers.L); }
void SET_4_L (void) { registers.L = set(4,registers.L); }
void SET_5_L (void) { registers.L = set(5,registers.L); }
void SET_6_L (void) { registers.L = set(6,registers.L); }
void SET_7_L (void) { registers.L = set(7,registers.L); }

void SET_0_HL (void) { writeMemory8(registers.HL, set(0,readMemory8(registers.HL))); }
void SET_1_HL (void) { writeMemory8(registers.HL, set(1,readMemory8(registers.HL))); }
void SET_2_HL (void) { writeMemory8(registers.HL, set(2,readMemory8(registers.HL))); }
void SET_3_HL (void) { writeMemory8(registers.HL, set(3,readMemory8(registers.HL))); }
void SET_4_HL (void) { writeMemory8(registers.HL, set(4,readMemory8(registers.HL))); }
void SET_5_HL (void) { writeMemory8(registers.HL, set(5,readMemory8(registers.HL))); }
void SET_6_HL (void) { writeMemory8(registers.HL, set(6,readMemory8(registers.HL))); }
void SET_7_HL (void) { writeMemory8(registers.HL, set(7,readMemory8(registers.HL))); }

/*
 * RES b,r
 * Description: Reset bit b in register r.
 * Use with: b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * None.
 */
void RES_0_A (void) { registers.A = res(0,registers.A); }
void RES_1_A (void) { registers.A = res(1,registers.A); }
void RES_2_A (void) { registers.A = res(2,registers.A); }
void RES_3_A (void) { registers.A = res(3,registers.A); }
void RES_4_A (void) { registers.A = res(4,registers.A); }
void RES_5_A (void) { registers.A = res(5,registers.A); }
void RES_6_A (void) { registers.A = res(6,registers.A); }
void RES_7_A (void) { registers.A = res(7,registers.A); }

void RES_0_B (void) { registers.B = res(0,registers.B); }
void RES_1_B (void) { registers.B = res(1,registers.B); }
void RES_2_B (void) { registers.B = res(2,registers.B); }
void RES_3_B (void) { registers.B = res(3,registers.B); }
void RES_4_B (void) { registers.B = res(4,registers.B); }
void RES_5_B (void) { registers.B = res(5,registers.B); }
void RES_6_B (void) { registers.B = res(6,registers.B); }
void RES_7_B (void) { registers.B = res(7,registers.B); }

void RES_0_C (void) { registers.C = res(0,registers.C); }
void RES_1_C (void) { registers.C = res(1,registers.C); }
void RES_2_C (void) { registers.C = res(2,registers.C); }
void RES_3_C (void) { registers.C = res(3,registers.C); }
void RES_4_C (void) { registers.C = res(4,registers.C); }
void RES_5_C (void) { registers.C = res(5,registers.C); }
void RES_6_C (void) { registers.C = res(6,registers.C); }
void RES_7_C (void) { registers.C = res(7,registers.C); }

void RES_0_D (void) { registers.D = res(0,registers.D); }
void RES_1_D (void) { registers.D = res(1,registers.D); }
void RES_2_D (void) { registers.D = res(2,registers.D); }
void RES_3_D (void) { registers.D = res(3,registers.D); }
void RES_4_D (void) { registers.D = res(4,registers.D); }
void RES_5_D (void) { registers.D = res(5,registers.D); }
void RES_6_D (void) { registers.D = res(6,registers.D); }
void RES_7_D (void) { registers.D = res(7,registers.D); }

void RES_0_E (void) { registers.E = res(0,registers.E); }
void RES_1_E (void) { registers.E = res(1,registers.E); }
void RES_2_E (void) { registers.E = res(2,registers.E); }
void RES_3_E (void) { registers.E = res(3,registers.E); }
void RES_4_E (void) { registers.E = res(4,registers.E); }
void RES_5_E (void) { registers.E = res(5,registers.E); }
void RES_6_E (void) { registers.E = res(6,registers.E); }
void RES_7_E (void) { registers.E = res(7,registers.E); }

void RES_0_H (void) { registers.H = res(0,registers.H); }
void RES_1_H (void) { registers.H = res(1,registers.H); }
void RES_2_H (void) { registers.H = res(2,registers.H); }
void RES_3_H (void) { registers.H = res(3,registers.H); }
void RES_4_H (void) { registers.H = res(4,registers.H); }
void RES_5_H (void) { registers.H = res(5,registers.H); }
void RES_6_H (void) { registers.H = res(6,registers.H); }
void RES_7_H (void) { registers.H = res(7,registers.H); }

void RES_0_L (void) { registers.L = res(0,registers.L); }
void RES_1_L (void) { registers.L = res(1,registers.L); }
void RES_2_L (void) { registers.L = res(2,registers.L); }
void RES_3_L (void) { registers.L = res(3,registers.L); }
void RES_4_L (void) { registers.L = res(4,registers.L); }
void RES_5_L (void) { registers.L = res(5,registers.L); }
void RES_6_L (void) { registers.L = res(6,registers.L); }
void RES_7_L (void) { registers.L = res(7,registers.L); }

void RES_0_HL (void) { writeMemory8(registers.HL, res(0,readMemory8(registers.HL))); }
void RES_1_HL (void) { writeMemory8(registers.HL, res(1,readMemory8(registers.HL))); }
void RES_2_HL (void) { writeMemory8(registers.HL, res(2,readMemory8(registers.HL))); }
void RES_3_HL (void) { writeMemory8(registers.HL, res(3,readMemory8(registers.HL))); }
void RES_4_HL (void) { writeMemory8(registers.HL, res(4,readMemory8(registers.HL))); }
void RES_5_HL (void) { writeMemory8(registers.HL, res(5,readMemory8(registers.HL))); }
void RES_6_HL (void) { writeMemory8(registers.HL, res(6,readMemory8(registers.HL))); }
void RES_7_HL (void) { writeMemory8(registers.HL, res(7,readMemory8(registers.HL))); }
/*
 * BIT b,r
 * Description: Test bit b in register r.
 * Use with: b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if bit b of register r is 0.
 * N - Reset.
 * H - Set.
 * C - Not affected.
 */
void BIT_0_A (void) { bit(0,registers.A); }
void BIT_1_A (void) { bit(1,registers.A); }
void BIT_2_A (void) { bit(2,registers.A); }
void BIT_3_A (void) { bit(3,registers.A); }
void BIT_4_A (void) { bit(4,registers.A); }
void BIT_5_A (void) { bit(5,registers.A); }
void BIT_6_A (void) { bit(6,registers.A); }
void BIT_7_A (void) { bit(7,registers.A); }

void BIT_0_B (void) { bit(0,registers.B); }
void BIT_1_B (void) { bit(1,registers.B); }
void BIT_2_B (void) { bit(2,registers.B); }
void BIT_3_B (void) { bit(3,registers.B); }
void BIT_4_B (void) { bit(4,registers.B); }
void BIT_5_B (void) { bit(5,registers.B); }
void BIT_6_B (void) { bit(6,registers.B); }
void BIT_7_B (void) { bit(7,registers.B); }

void BIT_0_C (void) { bit(0,registers.C); }
void BIT_1_C (void) { bit(1,registers.C); }
void BIT_2_C (void) { bit(2,registers.C); }
void BIT_3_C (void) { bit(3,registers.C); }
void BIT_4_C (void) { bit(4,registers.C); }
void BIT_5_C (void) { bit(5,registers.C); }
void BIT_6_C (void) { bit(6,registers.C); }
void BIT_7_C (void) { bit(7,registers.C); }

void BIT_0_D (void) { bit(0,registers.D); }
void BIT_1_D (void) { bit(1,registers.D); }
void BIT_2_D (void) { bit(2,registers.D); }
void BIT_3_D (void) { bit(3,registers.D); }
void BIT_4_D (void) { bit(4,registers.D); }
void BIT_5_D (void) { bit(5,registers.D); }
void BIT_6_D (void) { bit(6,registers.D); }
void BIT_7_D (void) { bit(7,registers.D); }

void BIT_0_E (void) { bit(0,registers.E); }
void BIT_1_E (void) { bit(1,registers.E); }
void BIT_2_E (void) { bit(2,registers.E); }
void BIT_3_E (void) { bit(3,registers.E); }
void BIT_4_E (void) { bit(4,registers.E); }
void BIT_5_E (void) { bit(5,registers.E); }
void BIT_6_E (void) { bit(6,registers.E); }
void BIT_7_E (void) { bit(7,registers.E); }

void BIT_0_H (void) { bit(0,registers.H); }
void BIT_1_H (void) { bit(1,registers.H); }
void BIT_2_H (void) { bit(2,registers.H); }
void BIT_3_H (void) { bit(3,registers.H); }
void BIT_4_H (void) { bit(4,registers.H); }
void BIT_5_H (void) { bit(5,registers.H); }
void BIT_6_H (void) { bit(6,registers.H); }
void BIT_7_H (void) { bit(7,registers.H); }

void BIT_0_L (void) { bit(0,registers.L); }
void BIT_1_L (void) { bit(1,registers.L); }
void BIT_2_L (void) { bit(2,registers.L); }
void BIT_3_L (void) { bit(3,registers.L); }
void BIT_4_L (void) { bit(4,registers.L); }
void BIT_5_L (void) { bit(5,registers.L); }
void BIT_6_L (void) { bit(6,registers.L); }
void BIT_7_L (void) { bit(7,registers.L); }

void BIT_0_HL (void) { bit(0,readMemory8(registers.HL)); }
void BIT_1_HL (void) { bit(1,readMemory8(registers.HL)); }
void BIT_2_HL (void) { bit(2,readMemory8(registers.HL)); }
void BIT_3_HL (void) { bit(3,readMemory8(registers.HL)); }
void BIT_4_HL (void) { bit(4,readMemory8(registers.HL)); }
void BIT_5_HL (void) { bit(5,readMemory8(registers.HL)); }
void BIT_6_HL (void) { bit(6,readMemory8(registers.HL)); }
void BIT_7_HL (void) { bit(7,readMemory8(registers.HL)); }

/* 
 * HELPER FUNCTIONS
 */
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
void adc (unsigned char value){

    unsigned short tmp = registers.A + value + testFlag(CARRY_F);
    

    if (((registers.A & 0x0F) + (value & 0x0F) + testFlag(CARRY_F)) > 0x0F)  
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);

    if (tmp > 0xFF)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
    registers.A = tmp & 0xFF;
    
    if (registers.A == 0)  
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);

    resetFlag(SUBSTRACT_F);
}

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
void xor (unsigned char value){
    registers.A = registers.A ^ value;
    
    if (registers.A == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
    
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    resetFlag(CARRY_F);
}

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
void and (unsigned char value){
    registers.A = registers.A & value;
    
    if (registers.A == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
    
    resetFlag(SUBSTRACT_F);
    setFlag(HALF_CARRY_F);
    resetFlag(CARRY_F);
}

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
void or (unsigned char value){
    registers.A |= value;
    
    if (registers.A == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
    
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    resetFlag(CARRY_F);
}

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
void cp (unsigned char value){
    if (registers.A == value)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
        
    setFlag(SUBSTRACT_F);
    
    if ((value & 0x0f) > (registers.A & 0x0f))
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    if (registers.A < value)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
}

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
void sub (unsigned char value){

    if (value > registers.A)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);

    setFlag(SUBSTRACT_F);
    
    if ((value & 0x0f) > (registers.A & 0x0f))
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    if (registers.A == value)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
        
    registers.A -= value;
}

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
unsigned char inc (unsigned char value){
    
    if ((value & 0x0F) == 0x0F)
        setFlag(HALF_CARRY_F);
    else 
        resetFlag(HALF_CARRY_F);
        
    value = value + 1;
    
    if (value == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);

    resetFlag(SUBSTRACT_F);
    
    return value;
}

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
unsigned char dec (unsigned char value){
    
    if (value & 0x0F)
        resetFlag(HALF_CARRY_F);
    else 
        setFlag(HALF_CARRY_F);
        
    value = value - 1;
    
    if (value == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);

    setFlag(SUBSTRACT_F);
    
    return value;
}

/*
 * SLA n
 * Description: Shift n left into Carry. LSB of n set to 0.
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
unsigned char sla (unsigned char value){
    
    if (value & 0x80){
        setFlag(CARRY_F);
    }
    else{
        resetFlag(CARRY_F);
    }
    
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    
    value = value << 1;
    
    if (value){
        resetFlag(ZERO_F);
    }
    else{
        setFlag(ZERO_F);
    }
    return value;
} 
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
void add (unsigned char value){
    
    unsigned short tmp = registers.A + value;
    
    if (tmp > 255) 
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
    if (((registers.A & 0x0F) + (value & 0x0F)) > 0x0F)  //bobonis needs deskcheck
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    resetFlag(SUBSTRACT_F);
    
    registers.A = tmp & 0xFF;

    if (registers.A == 0x00)  
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);    
}

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
void add16 (unsigned short value){
    
    unsigned long HL_long = (unsigned long)(registers.HL + value);
    
    if ((HL_long&0xffff0000) == 0)
        resetFlag(CARRY_F);
    else
        setFlag(CARRY_F);
    
    if (((registers.HL & 0x0FFF) + (value & 0x0FFF)) > 0x0FFF)
         setFlag(HALF_CARRY_F);
    else
         resetFlag(HALF_CARRY_F);

    registers.HL = (unsigned short)(HL_long&0xffff);
    
    resetFlag(SUBSTRACT_F);
}

/*
 * BIT b,r
 * Description: Test bit b in register r.
 * Use with: b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if bit b of register r is 0.
 * N - Reset.
 * H - Set.
 * C - Not affected.
 */
 void bit (unsigned char pos, unsigned char value){
    if ( (0x01 << pos) & value )
        resetFlag(ZERO_F);
    else
        setFlag(ZERO_F);
           
    resetFlag(SUBSTRACT_F);
    setFlag(HALF_CARRY_F);
}

/*
 * RES b,r
 * Description: Reset bit b in register r.
 * Use with: b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * None.
 */
unsigned char res (unsigned char pos, unsigned char value){
    
    switch (pos){
        case 0:
            value &= 0xFE;
            break;
        case 1:
            value &= 0xFD;
            break;
        case 2:
            value &= 0xFB;
            break;
        case 3:
            value &= 0xF7;
            break;
        case 4:
            value &= 0xEF;
            break;
        case 5:
            value &= 0xDF;
            break;
        case 6:
            value &= 0xBF;
            break;
        case 7:
            value &= 0x7F;
            break;
        default:
            break;
    }
    return value;
}

/*
 * SRL n
 * Description: Shift n right into Carry. MSB set to 0.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
unsigned char srl (unsigned char value){
    
    if (value & 0x01)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);

    value >>= 1;
    
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    
    if (value)
        resetFlag(ZERO_F);
    else
        setFlag(ZERO_F);

    return value;
}

/*
 * RR n
 * Description: Rotate n right through Carry flag.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
unsigned char rr (unsigned char value){
    
    int carry = value & 0x01;
    
    value >>= 1;
    
    if (testFlag(CARRY_F))
        value |= 0x80;
        
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    
    if (value)
        resetFlag(ZERO_F);
    else
        setFlag(ZERO_F);
        
    if (carry)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);

    return value;
}

/*
 * SWAP n
 * Description: Swap upper & lower nibles of n.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Reset.
 */
unsigned char swap (unsigned char value){
    
    value = ((value & 0xf) << 4) | ((value & 0xf0) >> 4);
    
    if (value == 0)
       setFlag(ZERO_F);
    else
       resetFlag(ZERO_F);
    
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    resetFlag(CARRY_F);
    
    return value;
}

/*
 * SET b,r
 * Description: Set bit b in register r.
 * Use with: b = 0 - 7, r = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * None.
 */
unsigned char set (unsigned char pos, unsigned char value){
    
    switch (pos){
        case 0:
            value |= 0x01;
            break;
        case 1:
            value |= 0x02;
            break;
        case 2:
            value |= 0x04;
            break;
        case 3:
            value |= 0x08;
            break;
        case 4:
            value |= 0x10;
            break;
        case 5:
            value |= 0x20;
            break;
        case 6:
            value |= 0x40;
            break;
        case 7:
            value |= 0x80;
            break;
        default:
            break;
    }
    return value;
}

/* RRC n
 * Description: Rotate n right. Old bit 0 to Carry flag.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
unsigned char rrc (unsigned char value){

    if (value & 0x01)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
    value >>= 1;
    
    if ( testFlag(CARRY_F) )
        value |= 0x80;
    
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    
    if (value == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
    
    return value;   
}

/* RLC n
 * Description: Rotate n left. Old bit 7 to Carry flag.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
unsigned char rlc (unsigned char value){
    
    if (value & 0x80)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
    value <<= 1;
    
    if ( testFlag(CARRY_F) )
        value |= 0x1;
    
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    
    if (value == 0)
        setFlag(ZERO_F);
    else
        resetFlag(ZERO_F);
    
    return value;
}

/*
 * RL n
 * Description: Rotate n left through Carry flag.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 7 data.
 */
unsigned char rl (unsigned char value){
    
    int carry = value & 0x80;
    
    value <<= 1;
    
    if (testFlag(CARRY_F))
        value |= 0x01;
    
    if (carry)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
    if (value)
        resetFlag(ZERO_F);
    else
        setFlag(ZERO_F);
    
    resetFlag(HALF_CARRY_F);
    resetFlag(SUBSTRACT_F);
    
    return value;
}

/*
 * SRA n
 * Description: Shift n right into Carry. MSB doesn't change.
 * Use with: n = A,B,C,D,E,H,L,(HL)
 * Flags affected:
 * Z - Set if result is zero.
 * N - Reset.
 * H - Reset.
 * C - Contains old bit 0 data.
 */
unsigned char sra (unsigned char value){
    
    int msb = value & 0x80;
    
    if (value & 0x01)
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
    resetFlag(SUBSTRACT_F);
    resetFlag(HALF_CARRY_F);
    
    value = value >> 1;
    
    if (msb)
        value |= 0x80; 
    
    if (value)
        resetFlag(ZERO_F);
    else
        setFlag(ZERO_F);

    return value;
}

/*
 * SBC A,n
 * Description: Subtract n + Carry flag from A.
 * Use with: n = A,B,C,D,E,H,L,(HL),#
 * Flags affected:
 * Z - Set if result is zero.
 * N - Set.
 * H - Set if no borrow from bit 4.
 * C - Set if no borrow.
 */
void sbc (unsigned char value){
    
    unsigned char result = registers.A - value - testFlag(CARRY_F);
    
    if ((( registers.A & 0x0F ) - ( value & 0x0F ) - testFlag(CARRY_F)) < 0 )
        setFlag(HALF_CARRY_F);
    else
        resetFlag(HALF_CARRY_F);
    
    if (( registers.A - value - testFlag(CARRY_F)) < 0 )
        setFlag(CARRY_F);
    else
        resetFlag(CARRY_F);
    
    setFlag(SUBSTRACT_F);
    
    if (result)
        resetFlag(ZERO_F);
    else
        setFlag(ZERO_F);

    registers.A = result;
}






