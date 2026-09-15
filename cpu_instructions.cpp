#include "cpu_instructions.h"
#include "cpu.h"

const struct gb_cpu_instruction instructions[256] = {
	{ "NOP",			0,		cpu_noop		},	// 0x00
	{ "LD BC, nn",		2,		cpu_ld_bc_nn	},	// 0x01
	{ "LD (BC), A",		0,		nullptr			},	// 0x02
	{ "INC BC",			0,		nullptr			},	// 0x03
	{ "INC B",			0,		nullptr			},	// 0x04
	{ "DEC B",			0,		cpu_dec_b		},	// 0x05
	{ "LD B, n",		1,		cpu_ld_b_n		},	// 0x06
	{ "RLC A",			0,		nullptr			},	// 0x07
	{ "LD nn, SP",		2,		nullptr			},	// 0x08
	{ "ADD HL, BC",		0,		nullptr			},	// 0x09
	{ "LD A, (BC)",		0,		nullptr			},	// 0x0A
	{ "DEC BC",			0,		nullptr			},	// 0x0B
	{ "INC C",			0,		nullptr			},	// 0x0C
	{ "DEC C",			0,		nullptr			},	// 0x0D
	{ "LD C, n",		1,		cpu_ld_c_n		},	// 0x0E
	{ "RRC A",			0,		nullptr			},	// 0x0F
	{ "STOP",			1,		nullptr			},	// 0x10
	{ "LD DE, nn",		2,		cpu_ld_de_nn	},	// 0x11
	{ "LD (DE), A",		0,		nullptr			},	// 0x12
	{ "INC DE",			0, 		nullptr 		},	// 0x13
	{ "INC D",			0,		nullptr			},	// 0x14
	{ "DEC D",			0,		nullptr			},	// 0x15
	{ "LD D, n",		1,		cpu_ld_d_n		},	// 0x16
	{ "RLA",			0,		nullptr			},	// 0x17
	{ "JR n",			1,		nullptr			},	// 0x18
	{ "ADD HL, DE",		0,		nullptr			},	// 0x19
	{ "LD A, (DE)",		0,		nullptr			},	// 0x1A
	{ "DEC DE",			0,		nullptr			},	// 0x1B
	{ "INC E",			0,		nullptr			},	// 0x1C
	{ "DEC E",			0,		nullptr			},	// 0x1D
	{ "LD E, n",		1,		cpu_ld_e_n		},	// 0x1E
	{ "RRA",			0,		nullptr			},	// 0x1F
	{ "JR NZ, n",		1,		nullptr			},	// 0x20
	{ "LD HL, nn",		2,		cpu_ld_hl_nn	},	// 0x21
	{ "LD (HL+), A",	0,		nullptr			},	// 0x22
	{ "INC HL",			0,		nullptr			},	// 0x23
	{ "INC H",			0,		nullptr			},	// 0x24
	{ "DEC H",			0,		nullptr			},	// 0x25
	{ "LD H, n",		1,		cpu_ld_h_n		},	// 0x26
	{ "DAA",			0,		nullptr			},	// 0x27
	{ "JR Z, n",		1,		nullptr			},	// 0x28
	{ "ADD HL, HL",		0,		nullptr			},	// 0x29
	{ "LD A, (HL+)",	0,		nullptr			},	// 0x2A
	{ "DEC HL",			0,		nullptr			},	// 0x2B
	{ "INC L",			0,		nullptr			},	// 0x2C
	{ "DEC L",			0,		nullptr			},	// 0x2D
	{ "LD L, n",		1,		cpu_ld_l_n		},	// 0x2E
	{ "CPL",			0,		nullptr			},	// 0x2F
	{ "JR NC, n",		1,		nullptr			},	// 0x30
	{ "LD SP, nn",		2,		cpu_ld_sp_nn	},	// 0x31
	{ "LD (HL-), A",	0,		cpu_ldd_hl_a	},	// 0x32
	{ "INC SP",			0,		nullptr			},	// 0x33
	{ "INC (HL)",		0,		nullptr			},	// 0x34
	{ "DEC (HL)",		0,		nullptr			},	// 0x35
	{ "LD (HL), n",		1,		nullptr			},	// 0x36
	{ "SCF",			0,		nullptr			},	// 0x37
	{ "JR C, n",		1, 		nullptr 		},	// 0x38
	{ "ADD HL, SP",		0,		nullptr			},	// 0x39
	{ "LD A, (HL-)",	0,		nullptr			},	// 0x3A
	{ "DEC SP",			0,		nullptr			},	// 0x3B
	{ "INC A",			0,		nullptr			},	// 0x3C
	{ "DEC A",			0,		nullptr			},	// 0x3D
	{ "LD A, n",		1,		nullptr			},	// 0x3E
	{ "CCF",			0,		nullptr			},	// 0x3F
	{ "LD B, B",		0,		nullptr			},	// 0x40
	{ "LD B, C",		0,		nullptr			},	// 0x41
	{ "LD B, D",		0,		nullptr			},	// 0x42
	{ "LD B, E",		0,		nullptr			},	// 0x43
	{ "LD B, H",		0,		nullptr			},	// 0x44
	{ "LD B, L",		0,		nullptr			},	// 0x45
	{ "LD B, (HL)",		0,		nullptr			},	// 0x46
	{ "LD B, A",		0,		nullptr			},	// 0x47
	{ "LD C, B",		0,		nullptr			},	// 0x48
	{ "LD C, C",		0,		nullptr			},	// 0x49
	{ "LD C, D",		0,		nullptr			},	// 0x4A
	{ "LD C, E",		0,		nullptr			},	// 0x4B
	{ "LD C, H",		0,		nullptr			},	// 0x4C
	{ "LD C, L",		0,		nullptr			},	// 0x4D
	{ "LD C, (HL)",		0,		nullptr			},	// 0x4E
	{ "LD C, A",		0,		nullptr			},	// 0x4F
	{ "LD D, B",		0, 		nullptr 		},	// 0x50
    { "LD D, C", 		0, 		nullptr 		},	// 0x51
	{ "LD D, D",		0,		nullptr			},	// 0x52
	{ "LD D, E",		0,		nullptr			},	// 0x53
	{ "LD D, H",		0,		nullptr			},	// 0x54
	{ "LD D, L",		0,		nullptr			},	// 0x55
	{ "LD D, (HL)",		0,		nullptr			},	// 0x56
	{ "LD D, A",		0,		nullptr			},	// 0x57
	{ "LD E, B",		0,		nullptr			},	// 0x58
	{ "LD E, C",		0,		nullptr			},	// 0x59
	{ "LD E, D",		0,		nullptr			},	// 0x5A
	{ "LD E, E",		0,		nullptr			},	// 0x5B
	{ "LD E, H",		0,		nullptr			},	// 0x5C
	{ "LD E, L",		0,		nullptr			},	// 0x5D
	{ "LD E, (HL)",		0,		nullptr			},	// 0x5E
	{ "LD E, A",		0,		nullptr			},	// 0x5F
	{ "LD H, B",		0,		nullptr			},	// 0x60
	{ "LD H, C",		0,		nullptr			},	// 0x61
	{ "LD H, D",		0,		nullptr			},	// 0x62
	{ "LD H, E",		0,		nullptr			},	// 0x63
	{ "LD H, H",		0,		nullptr			},	// 0x64
	{ "LD H, L",		0,		nullptr			},	// 0x65
	{ "LD H, (HL)",		0,		nullptr			},	// 0x66
	{ "LD H, A",		0,		nullptr			},	// 0x67
	{ "LD L, B",		0,		nullptr			},	// 0x68
	{ "LD L, C",		0,		nullptr			},	// 0x69
	{ "LD L, D",		0,		nullptr			},	// 0x6A
	{ "LD L, E",		0, 		nullptr 		},	// 0x6B
    { "LD L, H", 		0, 		nullptr 		},	// 0x6C
	{ "LD L, L",		0,		nullptr			},	// 0x6D
	{ "LD L, (HL)",		0,		nullptr			},	// 0x6E
	{ "LD L, A",		0,		nullptr			},	// 0x6F
	{ "LD (HL), B",		0,		nullptr			},	// 0x70
	{ "LD (HL), C",		0,		nullptr			},	// 0x71
	{ "LD (HL), D",		0,		nullptr			},	// 0x72
	{ "LD (HL), E",		0,		nullptr			},	// 0x73
	{ "LD (HL), H",		0,		nullptr			},	// 0x74
	{ "LD (HL), L",		0,		nullptr			},	// 0x75
	{ "HALT",			0,		nullptr			},	// 0x76
	{ "LD (HL), A",		0,		nullptr			},	// 0x77
	{ "LD A, B",		0,		nullptr			},	// 0x78
	{ "LD A, C",		0,		nullptr			},	// 0x79
	{ "LD A, D",		0,		nullptr			},	// 0x7A
	{ "LD A, E",		0,		nullptr			},	// 0x7B
	{ "LD A, H",		0,		nullptr			},	// 0x7C
	{ "LD A, L",		0,		nullptr			},	// 0x7D
	{ "LD A, (HL)",		0,		nullptr			},	// 0x7E
	{ "LD A, A",		0,		nullptr			},	// 0x7F
	{ "ADD A, B",		0,		nullptr			},	// 0x80
	{ "ADD A, C",		0,		nullptr			},	// 0x81
	{ "ADD A, D",		0,		nullptr			},	// 0x82
	{ "ADD A, E",		0,		nullptr			},	// 0x83
	{ "ADD A, H",		0,		nullptr			},	// 0x84
	{ "ADD A, L",		0,		nullptr			},	// 0x85
	{ "ADD A, (HL)",	0,		nullptr			},	// 0x86
	{ "ADD A, A",		0,		nullptr			},	// 0x87
	{ "ADC A, B",		0,		nullptr			},	// 0x88
	{ "ADC A, C",		0,		nullptr			},	// 0x89
	{ "ADC A, D",		0,		nullptr			},	// 0x8A
	{ "ADC A, E",		0,		nullptr			},	// 0x8B
	{ "ADC A, H",		0,		nullptr			},	// 0x8C
	{ "ADC A, L",		0,		nullptr			},	// 0x8D
	{ "ADC A, (HL)",	0,		nullptr			},	// 0x8E
	{ "ADC A, A",		0,		nullptr			},	// 0x8F
	{ "SUB B",			0,		nullptr			},	// 0x90
	{ "SUB C",			0,		nullptr			},	// 0x91
	{ "SUB D",			0,		nullptr			},	// 0x92
	{ "SUB E",			0,		nullptr			},	// 0x93
	{ "SUB H",			0,		nullptr			},	// 0x94
	{ "SUB L",			0,		nullptr			},	// 0x95
	{ "SUB (HL)",		0,		nullptr			},	// 0x96
	{ "SUB A",			0,		nullptr			},	// 0x97
	{ "SBC A, B",		0,		nullptr			},	// 0x98
	{ "SBC A, C",		0,		nullptr			},	// 0x99
	{ "SBC A, D",		0,		nullptr			},	// 0x9A
	{ "SBC A, E",		0,		nullptr			},	// 0x9B
	{ "SBC A, H",		0,		nullptr			},	// 0x9C
	{ "SBC A, L",		0,		nullptr			},	// 0x9D
	{ "SBC A, (HL)",	0,		nullptr			},	// 0x9E
	{ "SBC A, A",		0, 		nullptr 		},	// 0x9F
	{ "AND B",			0,		nullptr 		},	// 0xA0
	{ "AND C",			0,		nullptr			},	// 0xA1
	{ "AND D",			0,		nullptr			},	// 0xA2
	{ "AND E",			0,		nullptr			},	// 0xA3
	{ "AND H",			0,		nullptr			},	// 0xA4
	{ "AND L",			0,		nullptr			},	// 0xA5
	{ "AND (HL)",		0,		nullptr			},	// 0xA6
	{ "AND A",			0,		nullptr			},	// 0xA7
	{ "XOR B",			0,		nullptr			},	// 0xA8
	{ "XOR C",			0,		nullptr			},	// 0xA9
	{ "XOR D",			0,		nullptr			},	// 0xAA
	{ "XOR E",			0,		nullptr			},	// 0xAB
	{ "XOR H",			0,		nullptr			},	// 0xAC
	{ "XOR L",			0,		nullptr			},	// 0xAD
	{ "XOR (HL)",		0,		nullptr			},	// 0xAE
	{ "XOR A",			0,		cpu_xor_a		},	// 0xAF
	{ "OR B",			0,		nullptr			},	// 0xB0
	{ "OR C",			0,		nullptr			},	// 0xB1
	{ "OR D",			0,		nullptr			},	// 0xB2
	{ "OR E",			0,		nullptr			},	// 0xB3
	{ "OR H",			0,		nullptr			},	// 0xB4
	{ "OR L",			0,		nullptr			},	// 0xB5
	{ "OR (HL)",		0,		nullptr			},	// 0xB6
	{ "OR A",			0,		nullptr			},	// 0xB7
	{ "CP B",			0,		nullptr			},	// 0xB8
	{ "CP C",			0,		nullptr			},	// 0xB9
	{ "CP D",			0,		nullptr			},	// 0xBA
	{ "CP E",			0,		nullptr			},	// 0xBB
	{ "CP H",			0,		nullptr			},	// 0xBC
	{ "CP L",			0,		nullptr			},	// 0xBD
	{ "CP (HL)",		0,		nullptr			},	// 0xBE
	{ "CP A",			0,		nullptr			},	// 0xBF
	{ "RET NZ",			0,		nullptr			},	// 0xC0
	{ "POP BC",			0,		nullptr			},	// 0xC1
	{ "JP NZ, nn",		2, 		nullptr 		},  // 0xC2
	{ "JP nn", 			2,		cpu_jp_nn		},	// 0xC3
	{ "CALL NZ, nn",	2,		nullptr			},	// 0xC4
	{ "PUSH BC",		0,		nullptr			},	// 0xC5
	{ "ADD A, n",		1,		nullptr			},	// 0xC6
	{ "RST 00h",		0,		nullptr			},	// 0xC7
	{ "RET Z", 			0,		nullptr			},	// 0xC8
	{ "RET", 			0,		nullptr			},	// 0xC9
	{ "JP Z, nn", 		2,		nullptr			},	// 0xCA
	{ "PREFIX CB", 		0,		nullptr			},	// 0xCB
	{ "CALL Z, nn", 	2,		nullptr			},	// 0xCC
	{ "CALL nn", 		2,		nullptr			},	// 0xCD
	{ "ADC A, n",		1,		nullptr			},	// 0xCE
	{ "RST 08h",		0,		nullptr			},	// 0xCF
	{ "RET NC",			0,		nullptr			},	// 0xD0
	{ "POP DE",			0,		nullptr			},	// 0xD1
	{ "JP NC, nn",		2,		nullptr			},	// 0xD2
	{ "???",			0,		nullptr			},	// 0xD3
	{ "CALL NC, nn",	2,		nullptr			},	// 0xD4
	{ "PUSH DE",		0,		nullptr			},	// 0xD5
	{ "SUB n",			1,		nullptr			},	// 0xD6
	{ "RST 10h",		0,		nullptr			},	// 0xD7
	{ "RET C", 			0,		nullptr			},	// 0xD8
	{ "RETI", 			0,		nullptr			},	// 0xD9
	{ "JP C, nn", 		2,		nullptr			},	// 0xDA
	{ "???",			0,		nullptr			},	// 0xDB
	{ "CALL C, nn", 	2,		nullptr			},	// 0xDC
	{ "???", 			0,		nullptr			},	// 0xDD
	{ "SBC A, n", 		1,		nullptr			},	// 0xDE
	{ "RST 18h",		0,		nullptr			},	// 0xDF
	{ "LD (n), A",		1,		nullptr			},	// 0xE0
	{ "POP HL",			0,		nullptr			},	// 0xE1
	{ "LD (C), A",		0,		nullptr			},	// 0xE2
	{ "???",			0,		nullptr			},	// 0xE3
	{ "???",			0,		nullptr			},	// 0xE4
	{ "PUSH HL",		0,		nullptr			},	// 0xE5
	{ "AND n",			1,		nullptr			},	// 0xE6
	{ "RST 20h",		0,		nullptr			},	// 0xE7
	{ "ADD SP, n",		1,		nullptr			},	// 0xE8
	{ "JP (HL)",		0,		nullptr			},	// 0xE9
	{ "LD nn, A",		2,		nullptr			},	// 0xEA
	{ "???",			0,		nullptr			},	// 0xEB
	{ "???",			0,		nullptr			},	// 0xEC
	{ "???",			0,		nullptr			},	// 0xED
	{ "XOR n",			1,		nullptr			},	// 0xEE
	{ "RST 28h",		0,		nullptr			},	// 0xEF
	{ "LD A, (n)",		1,		nullptr			},	// 0xF0
	{ "POP AF", 		0,		nullptr			},	// 0xF1
	{ "LD A, (C)", 		0,		nullptr			},	// 0xF2
	{ "DI", 			0,		nullptr			},	// 0xF3
	{ "???",			0,		nullptr			},	// 0xF4
	{ "PUSH AF",		0,		nullptr			},	// 0xF5
	{ "OR n",			1,		nullptr			},	// 0xF6
	{ "RST 30h",		0,		nullptr			},	// 0xF7
	{ "LD HL, SP+n",	1,		nullptr			},	// 0xF8
	{ "LD SP, HL",		0,		nullptr			},	// 0xF9
	{ "LD A, nn", 		2,		nullptr			},	// 0xFA
	{ "EI", 			0,		nullptr			},	// 0xFB
	{ "???",			0,		nullptr			},	// 0xFC
	{ "???",			0,		nullptr			},	// 0xFD
	{ "CP n",			1,		nullptr			},	// 0xFE
	{ "RST 38h",		0,		nullptr			},	// 0xFF
};
