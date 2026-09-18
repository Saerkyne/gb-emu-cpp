#include "cpu_instructions.h"
#include "cpu.h"

const struct gb_cpu_instruction instructions[256] = {
	{ "NOP",			0,		cpu_noop		},	// 0x00
	{ "LD BC, nn",		2,		cpu_ld_bc_nn	},	// 0x01
	{ "LD (BC), A",		0,		cpu_ld_bc_a		},	// 0x02
	{ "INC BC",			0,		cpu_inc_bc		},	// 0x03
	{ "INC B",			0,		cpu_inc_b		},	// 0x04
	{ "DEC B",			0,		cpu_dec_b		},	// 0x05
	{ "LD B, n",		1,		cpu_ld_b_n		},	// 0x06
	{ "RLC A",			0,		cpu_rlca		},	// 0x07
	{ "LD nn, SP",		2,		cpu_ld_nn_sp	},	// 0x08
	{ "ADD HL, BC",		0,		cpu_add_hl_bc	},	// 0x09
	{ "LD A, (BC)",		0,		cpu_ld_a_bc		},	// 0x0A
	{ "DEC BC",			0,		cpu_dec_bc		},	// 0x0B
	{ "INC C",			0,		cpu_inc_c		},	// 0x0C
	{ "DEC C",			0,		cpu_dec_c		},	// 0x0D
	{ "LD C, n",		1,		cpu_ld_c_n		},	// 0x0E
	{ "RRC A",			0,		cpu_rrca		},	// 0x0F
	{ "STOP",			1,		cpu_stop		},	// 0x10
	{ "LD DE, nn",		2,		cpu_ld_de_nn	},	// 0x11
	{ "LD (DE), A",		0,		cpu_ld_de_a		},	// 0x12
	{ "INC DE",			0, 		cpu_inc_de 		},	// 0x13
	{ "INC D",			0,		cpu_inc_d		},	// 0x14
	{ "DEC D",			0,		cpu_dec_d		},	// 0x15
	{ "LD D, n",		1,		cpu_ld_d_n		},	// 0x16
	{ "RLA",			0,		cpu_rla			},	// 0x17
	{ "JR n",			1,		cpu_jr_n		},	// 0x18
	{ "ADD HL, DE",		0,		cpu_add_hl_de	},	// 0x19
	{ "LD A, (DE)",		0,		cpu_ld_a_de		},	// 0x1A
	{ "DEC DE",			0,		cpu_dec_de		},	// 0x1B
	{ "INC E",			0,		cpu_inc_e		},	// 0x1C
	{ "DEC E",			0,		cpu_dec_e		},	// 0x1D
	{ "LD E, n",		1,		cpu_ld_e_n		},	// 0x1E
	{ "RRA",			0,		cpu_rra			},	// 0x1F
	{ "JR NZ, n",		1,		cpu_jr_nz_n		},	// 0x20
	{ "LD HL, nn",		2,		cpu_ld_hl_nn	},	// 0x21
	{ "LD (HL+), A",	0,		cpu_ldi_hl_a	},	// 0x22
	{ "INC HL",			0,		cpu_inc_hl		},	// 0x23
	{ "INC H",			0,		cpu_inc_h		},	// 0x24
	{ "DEC H",			0,		cpu_dec_h		},	// 0x25
	{ "LD H, n",		1,		cpu_ld_h_n		},	// 0x26
	{ "DAA",			0,		cpu_daa			},	// 0x27
	{ "JR Z, n",		1,		cpu_jr_z_n		},	// 0x28
	{ "ADD HL, HL",		0,		cpu_add_hl_hl	},	// 0x29
	{ "LD A, (HL+)",	0,		cpu_ldi_a_hl	},	// 0x2A
	{ "DEC HL",			0,		cpu_dec_hl		},	// 0x2B
	{ "INC L",			0,		cpu_inc_l		},	// 0x2C
	{ "DEC L",			0,		cpu_dec_l		},	// 0x2D
	{ "LD L, n",		1,		cpu_ld_l_n		},	// 0x2E
	{ "CPL",			0,		cpu_cpl			},	// 0x2F
	{ "JR NC, n",		1,		cpu_jr_nc_n		},	// 0x30
	{ "LD SP, nn",		2,		cpu_ld_sp_nn	},	// 0x31
	{ "LD (HL-), A",	0,		cpu_ldd_hl_a	},	// 0x32
	{ "INC SP",			0,		cpu_inc_sp		},	// 0x33
	{ "INC (HL)",		0,		cpu_inc__hl		},	// 0x34
	{ "DEC (HL)",		0,		cpu_dec__hl		},	// 0x35
	{ "LD (HL), n",		1,		cpu_ld_hl_n		},	// 0x36
	{ "SCF",			0,		cpu_scf			},	// 0x37
	{ "JR C, n",		1, 		cpu_jr_c_n 		},	// 0x38
	{ "ADD HL, SP",		0,		cpu_add_hl_sp	},	// 0x39
	{ "LD A, (HL-)",	0,		cpu_ldd_a_hl	},	// 0x3A
	{ "DEC SP",			0,		cpu_dec_sp		},	// 0x3B
	{ "INC A",			0,		cpu_inc_a		},	// 0x3C
	{ "DEC A",			0,		cpu_dec_a		},	// 0x3D
	{ "LD A, n",		1,		cpu_ld_a_n		},	// 0x3E
	{ "CCF",			0,		cpu_ccf			},	// 0x3F
	{ "LD B, B",		0,		cpu_ld_b_b		},	// 0x40
	{ "LD B, C",		0,		cpu_ld_b_c		},	// 0x41
	{ "LD B, D",		0,		cpu_ld_b_d		},	// 0x42
	{ "LD B, E",		0,		cpu_ld_b_e		},	// 0x43
	{ "LD B, H",		0,		cpu_ld_b_h		},	// 0x44
	{ "LD B, L",		0,		cpu_ld_b_l		},	// 0x45
	{ "LD B, (HL)",		0,		cpu_ld_b_hl		},	// 0x46
	{ "LD B, A",		0,		cpu_ld_b_a		},	// 0x47
	{ "LD C, B",		0,		cpu_ld_c_b		},	// 0x48
	{ "LD C, C",		0,		cpu_ld_c_c		},	// 0x49
	{ "LD C, D",		0,		cpu_ld_c_d		},	// 0x4A
	{ "LD C, E",		0,		cpu_ld_c_e		},	// 0x4B
	{ "LD C, H",		0,		cpu_ld_c_h		},	// 0x4C
	{ "LD C, L",		0,		cpu_ld_c_l		},	// 0x4D
	{ "LD C, (HL)",		0,		cpu_ld_c_hl		},	// 0x4E
	{ "LD C, A",		0,		cpu_ld_c_a		},	// 0x4F
	{ "LD D, B",		0, 		cpu_ld_d_b 		},	// 0x50
    { "LD D, C", 		0, 		cpu_ld_d_c 		},	// 0x51
	{ "LD D, D",		0,		cpu_ld_d_d		},	// 0x52
	{ "LD D, E",		0,		cpu_ld_d_e		},	// 0x53
	{ "LD D, H",		0,		cpu_ld_d_h		},	// 0x54
	{ "LD D, L",		0,		cpu_ld_d_l		},	// 0x55
	{ "LD D, (HL)",		0,		cpu_ld_d_hl		},	// 0x56
	{ "LD D, A",		0,		cpu_ld_d_a		},	// 0x57
	{ "LD E, B",		0,		cpu_ld_e_b		},	// 0x58
	{ "LD E, C",		0,		cpu_ld_e_c		},	// 0x59
	{ "LD E, D",		0,		cpu_ld_e_d		},	// 0x5A
	{ "LD E, E",		0,		cpu_ld_e_e		},	// 0x5B
	{ "LD E, H",		0,		cpu_ld_e_h		},	// 0x5C
	{ "LD E, L",		0,		cpu_ld_e_l		},	// 0x5D
	{ "LD E, (HL)",		0,		cpu_ld_e_hl		},	// 0x5E
	{ "LD E, A",		0,		cpu_ld_e_a		},	// 0x5F
	{ "LD H, B",		0,		cpu_ld_h_b		},	// 0x60
	{ "LD H, C",		0,		cpu_ld_h_c		},	// 0x61
	{ "LD H, D",		0,		cpu_ld_h_d		},	// 0x62
	{ "LD H, E",		0,		cpu_ld_h_e		},	// 0x63
	{ "LD H, H",		0,		cpu_ld_h_h		},	// 0x64
	{ "LD H, L",		0,		cpu_ld_h_l		},	// 0x65
	{ "LD H, (HL)",		0,		cpu_ld_h_hl		},	// 0x66
	{ "LD H, A",		0,		cpu_ld_h_a		},	// 0x67
	{ "LD L, B",		0,		cpu_ld_l_b		},	// 0x68
	{ "LD L, C",		0,		cpu_ld_l_c		},	// 0x69
	{ "LD L, D",		0,		cpu_ld_l_d		},	// 0x6A
	{ "LD L, E",		0, 		cpu_ld_l_e 		},	// 0x6B
    { "LD L, H", 		0, 		cpu_ld_l_h 		},	// 0x6C
	{ "LD L, L",		0,		cpu_ld_l_l		},	// 0x6D
	{ "LD L, (HL)",		0,		cpu_ld_l_hl		},	// 0x6E
	{ "LD L, A",		0,		cpu_ld_l_a		},	// 0x6F
	{ "LD (HL), B",		0,		cpu_ld_hl_b		},	// 0x70
	{ "LD (HL), C",		0,		cpu_ld_hl_c		},	// 0x71
	{ "LD (HL), D",		0,		cpu_ld_hl_d		},	// 0x72
	{ "LD (HL), E",		0,		cpu_ld_hl_e		},	// 0x73
	{ "LD (HL), H",		0,		cpu_ld_hl_h		},	// 0x74
	{ "LD (HL), L",		0,		cpu_ld_hl_l		},	// 0x75
	{ "HALT",			0,		cpu_halt		},	// 0x76
	{ "LD (HL), A",		0,		cpu_ld_hl_a		},	// 0x77
	{ "LD A, B",		0,		cpu_ld_a_b		},	// 0x78
	{ "LD A, C",		0,		cpu_ld_a_c		},	// 0x79
	{ "LD A, D",		0,		cpu_ld_a_d		},	// 0x7A
	{ "LD A, E",		0,		cpu_ld_a_e		},	// 0x7B
	{ "LD A, H",		0,		cpu_ld_a_h		},	// 0x7C
	{ "LD A, L",		0,		cpu_ld_a_l		},	// 0x7D
	{ "LD A, (HL)",		0,		cpu_ld_a_hl		},	// 0x7E
	{ "LD A, A",		0,		cpu_ld_a_a		},	// 0x7F
	{ "ADD A, B",		0,		cpu_add_a_b		},	// 0x80
	{ "ADD A, C",		0,		cpu_add_a_c		},	// 0x81
	{ "ADD A, D",		0,		cpu_add_a_d		},	// 0x82
	{ "ADD A, E",		0,		cpu_add_a_e		},	// 0x83
	{ "ADD A, H",		0,		cpu_add_a_h		},	// 0x84
	{ "ADD A, L",		0,		cpu_add_a_l		},	// 0x85
	{ "ADD A, (HL)",	0,		cpu_add_a_hl	},	// 0x86
	{ "ADD A, A",		0,		cpu_add_a_a		},	// 0x87
	{ "ADC A, B",		0,		cpu_adc_a_b		},	// 0x88
	{ "ADC A, C",		0,		cpu_adc_a_c		},	// 0x89
	{ "ADC A, D",		0,		cpu_adc_a_d		},	// 0x8A
	{ "ADC A, E",		0,		cpu_adc_a_e		},	// 0x8B
	{ "ADC A, H",		0,		cpu_adc_a_h		},	// 0x8C
	{ "ADC A, L",		0,		cpu_adc_a_l		},	// 0x8D
	{ "ADC A, (HL)",	0,		cpu_adc_a_hl	},	// 0x8E
	{ "ADC A, A",		0,		cpu_adc_a_a		},	// 0x8F
	{ "SUB B",			0,		cpu_sub_a_b		},	// 0x90
	{ "SUB C",			0,		cpu_sub_a_c		},	// 0x91
	{ "SUB D",			0,		cpu_sub_a_d		},	// 0x92
	{ "SUB E",			0,		cpu_sub_a_e		},	// 0x93
	{ "SUB H",			0,		cpu_sub_a_h		},	// 0x94
	{ "SUB L",			0,		cpu_sub_a_l		},	// 0x95
	{ "SUB (HL)",		0,		cpu_sub_a_hl	},	// 0x96
	{ "SUB A",			0,		cpu_sub_a_a		},	// 0x97
	{ "SBC A, B",		0,		cpu_sbc_a_b		},	// 0x98
	{ "SBC A, C",		0,		cpu_sbc_a_c		},	// 0x99
	{ "SBC A, D",		0,		cpu_sbc_a_d		},	// 0x9A
	{ "SBC A, E",		0,		cpu_sbc_a_e		},	// 0x9B
	{ "SBC A, H",		0,		cpu_sbc_a_h		},	// 0x9C
	{ "SBC A, L",		0,		cpu_sbc_a_l		},	// 0x9D
	{ "SBC A, (HL)",	0,		cpu_sbc_a_hl	},	// 0x9E
	{ "SBC A, A",		0, 		cpu_sbc_a_a		},	// 0x9F
	{ "AND B",			0,		cpu_and_a_b		},	// 0xA0
	{ "AND C",			0,		cpu_and_a_c		},	// 0xA1
	{ "AND D",			0,		cpu_and_a_d		},	// 0xA2
	{ "AND E",			0,		cpu_and_a_e		},	// 0xA3
	{ "AND H",			0,		cpu_and_a_h		},	// 0xA4
	{ "AND L",			0,		cpu_and_a_l		},	// 0xA5
	{ "AND (HL)",		0,		cpu_and_a_hl	},	// 0xA6
	{ "AND A",			0,		cpu_and_a_a		},	// 0xA7
	{ "XOR B",			0,		cpu_xor_a_b		},	// 0xA8
	{ "XOR C",			0,		cpu_xor_a_c		},	// 0xA9
	{ "XOR D",			0,		cpu_xor_a_d		},	// 0xAA
	{ "XOR E",			0,		cpu_xor_a_e		},	// 0xAB
	{ "XOR H",			0,		cpu_xor_a_h		},	// 0xAC
	{ "XOR L",			0,		cpu_xor_a_l		},	// 0xAD
	{ "XOR (HL)",		0,		cpu_xor_a_hl	},	// 0xAE
	{ "XOR A",			0,		cpu_xor_a		},	// 0xAF
	{ "OR B",			0,		cpu_or_a_b		},	// 0xB0
	{ "OR C",			0,		cpu_or_a_c		},	// 0xB1
	{ "OR D",			0,		cpu_or_a_d		},	// 0xB2
	{ "OR E",			0,		cpu_or_a_e		},	// 0xB3
	{ "OR H",			0,		cpu_or_a_h		},	// 0xB4
	{ "OR L",			0,		cpu_or_a_l		},	// 0xB5
	{ "OR (HL)",		0,		cpu_or_a_hl		},	// 0xB6
	{ "OR A",			0,		cpu_or_a_a		},	// 0xB7
	{ "CP B",			0,		cpu_cp_a_b		},	// 0xB8
	{ "CP C",			0,		cpu_cp_a_c		},	// 0xB9
	{ "CP D",			0,		cpu_cp_a_d		},	// 0xBA
	{ "CP E",			0,		cpu_cp_a_e		},	// 0xBB
	{ "CP H",			0,		cpu_cp_a_h		},	// 0xBC
	{ "CP L",			0,		cpu_cp_a_l		},	// 0xBD
	{ "CP (HL)",		0,		cpu_cp_a_hl		},	// 0xBE
	{ "CP A",			0,		cpu_cp_a_a		},	// 0xBF
	{ "RET NZ",			0,		cpu_ret_nz		},	// 0xC0
	{ "POP BC",			0,		cpu_pop_bc		},	// 0xC1
	{ "JP NZ, nn",		2, 		cpu_jp_nz_nn	},  // 0xC2
	{ "JP nn", 			2,		cpu_jp_nn		},	// 0xC3
	{ "CALL NZ, nn",	2,		cpu_call_nz_nn	},	// 0xC4
	{ "PUSH BC",		0,		cpu_push_bc		},	// 0xC5
	{ "ADD A, n",		1,		cpu_add_a_n		},	// 0xC6
	{ "RST 00h",		0,		cpu_rst_00		},	// 0xC7
	{ "RET Z", 			0,		cpu_ret_z		},	// 0xC8
	{ "RET", 			0,		cpu_ret			},	// 0xC9
	{ "JP Z, nn", 		2,		cpu_jp_z_nn		},	// 0xCA
	{ "PREFIX CB", 		0,		cpu_prefix_cb	},	// 0xCB
	{ "CALL Z, nn", 	2,		cpu_call_z_nn	},	// 0xCC
	{ "CALL nn", 		2,		cpu_call_nn		},	// 0xCD
	{ "ADC A, n",		1,		cpu_adc_a_n		},	// 0xCE
	{ "RST 08h",		0,		cpu_rst_08		},	// 0xCF
	{ "RET NC",			0,		cpu_ret_nc		},	// 0xD0
	{ "POP DE",			0,		cpu_pop_de		},	// 0xD1
	{ "JP NC, nn",		2,		cpu_jp_nc_nn	},	// 0xD2
	{ "???",			0,		nullptr			},	// 0xD3
	{ "CALL NC, nn",	2,		cpu_call_nc_nn	},	// 0xD4
	{ "PUSH DE",		0,		cpu_push_de		},	// 0xD5
	{ "SUB n",			1,		cpu_sub_a_n		},	// 0xD6
	{ "RST 10h",		0,		cpu_rst_10		},	// 0xD7
	{ "RET C", 			0,		cpu_ret_c		},	// 0xD8
	{ "RETI", 			0,		cpu_reti		},	// 0xD9
	{ "JP C, nn", 		2,		cpu_jp_c_nn		},	// 0xDA
	{ "???",			0,		nullptr			},	// 0xDB
	{ "CALL C, nn", 	2,		cpu_call_c_nn	},	// 0xDC
	{ "???", 			0,		nullptr			},	// 0xDD
	{ "SBC A, n", 		1,		cpu_sbc_a_n		},	// 0xDE
	{ "RST 18h",		0,		cpu_rst_18		},	// 0xDF
	{ "LD (n), A",		1,		cpu_ldh_n_a		},	// 0xE0
	{ "POP HL",			0,		cpu_pop_hl		},	// 0xE1
	{ "LD (C), A",		0,		cpu_ld_c_a		},	// 0xE2
	{ "???",			0,		nullptr			},	// 0xE3
	{ "???",			0,		nullptr			},	// 0xE4
	{ "PUSH HL",		0,		cpu_push_hl		},	// 0xE5
	{ "AND n",			1,		cpu_and_a_n		},	// 0xE6
	{ "RST 20h",		0,		cpu_rst_20		},	// 0xE7
	{ "ADD SP, n",		1,		cpu_add_sp_n	},	// 0xE8
	{ "JP (HL)",		0,		cpu_jp_hl		},	// 0xE9
	{ "LD nn, A",		2,		cpu_ld_nn_a		},	// 0xEA
	{ "???",			0,		nullptr			},	// 0xEB
	{ "???",			0,		nullptr			},	// 0xEC
	{ "???",			0,		nullptr			},	// 0xED
	{ "XOR n",			1,		cpu_xor_a_n		},	// 0xEE
	{ "RST 28h",		0,		cpu_rst_28		},	// 0xEF
	{ "LD A, (n)",		1,		cpu_ld_a_n		},	// 0xF0
	{ "POP AF", 		0,		cpu_pop_af		},	// 0xF1
	{ "LD A, (C)", 		0,		cpu_ld_a_c		},	// 0xF2
	{ "DI", 			0,		cpu_di			},	// 0xF3
	{ "???",			0,		nullptr			},	// 0xF4
	{ "PUSH AF",		0,		cpu_push_af		},	// 0xF5
	{ "OR n",			1,		cpu_or_a_n		},	// 0xF6
	{ "RST 30h",		0,		cpu_rst_30		},	// 0xF7
	{ "LD HL, SP+n",	1,		cpu_ld_hl_sp_n	},	// 0xF8
	{ "LD SP, HL",		0,		cpu_ld_sp_hl	},	// 0xF9
	{ "LD A, nn", 		2,		cpu_ld_a_nn		},	// 0xFA
	{ "EI", 			0,		cpu_ei			},	// 0xFB
	{ "???",			0,		nullptr			},	// 0xFC
	{ "???",			0,		nullptr			},	// 0xFD
	{ "CP n",			1,		cpu_cp_a_n		},	// 0xFE
	{ "RST 38h",		0,		cpu_rst_38		},	// 0xFF
};

const gb_cpu_pre_cb_instruction cb_instructions[256] = {
	{ "RLC B",			cpu_cb_rlc_b		},	// 0x00
	{ "RLC C",			cpu_cb_rlc_c		},	// 0x01
	{ "RLC D",			cpu_cb_rlc_d		},	// 0x02
	{ "RLC E",			cpu_cb_rlc_e		},	// 0x03
	{ "RLC H",			cpu_cb_rlc_h		},	// 0x04
	{ "RLC L",			cpu_cb_rlc_l		},	// 0x05
	{ "RLC (HL)",		cpu_cb_rlc_hl		},	// 0x06
	{ "RLC A",			cpu_cb_rlc_a		},	// 0x07
	{ "RRC B",			cpu_cb_rrc_b		},	// 0x08
	{ "RRC C",			cpu_cb_rrc_c		},	// 0x09
	{ "RRC D",			cpu_cb_rrc_d		},	// 0x0A
	{ "RRC E",			cpu_cb_rrc_e		},	// 0x0B
	{ "RRC H",			cpu_cb_rrc_h		},	// 0x0C
	{ "RRC L",			cpu_cb_rrc_l		},	// 0x0D
	{ "RRC (HL)",		cpu_cb_rrc_hl		},	// 0x0E
	{ "RRC A",			cpu_cb_rrc_a		},	// 0x0F
	{ "RL B",			cpu_cb_rl_b			},	// 0x10
	{ "RL C",			cpu_cb_rl_c			},	// 0x11
	{ "RL D",			cpu_cb_rl_d			},	// 0x12
	{ "RL E",			cpu_cb_rl_e			},	// 0x13
	{ "RL H",			cpu_cb_rl_h			},	// 0x14
	{ "RL L",			cpu_cb_rl_l			},	// 0x15
	{ "RL (HL)",		cpu_cb_rl_hl		},	// 0x16
	{ "RL A",			cpu_cb_rl_a			},	// 0x17
	{ "RR B",			cpu_cb_rr_b			},	// 0x18
	{ "RR C",			cpu_cb_rr_c			},	// 0x19
	{ "RR D",			cpu_cb_rr_d			},	// 0x1A
	{ "RR E",			cpu_cb_rr_e			},	// 0x1B
	{ "RR H",			cpu_cb_rr_h			},	// 0x1C
	{ "RR L",			cpu_cb_rr_l			},	// 0x1D
	{ "RR (HL)",		cpu_cb_rr_hl		},	// 0x1E
	{ "RR A",			cpu_cb_rr_a			},	// 0x1F
	{ "SLA B",			cpu_cb_sla_b		},	// 0x20
	{ "SLA C",			cpu_cb_sla_c		},	// 0x21
	{ "SLA D",			cpu_cb_sla_d		},	// 0x22
	{ "SLA E",			cpu_cb_sla_e		},	// 0x23
	{ "SLA H",			cpu_cb_sla_h		},	// 0x24
	{ "SLA L",			cpu_cb_sla_l		},	// 0x25
	{ "SLA (HL)",		cpu_cb_sla_hl		},	// 0x26
	{ "SLA A",			cpu_cb_sla_a		},	// 0x27
	{ "SRA B",			cpu_cb_sra_b		},	// 0x28
	{ "SRA C",			cpu_cb_sra_c		},	// 0x29
	{ "SRA D",			cpu_cb_sra_d		},	// 0x2A
	{ "SRA E",			cpu_cb_sra_e		},	// 0x2B
	{ "SRA H",			cpu_cb_sra_h		},	// 0x2C
	{ "SRA L",			cpu_cb_sra_l		},	// 0x2D
	{ "SRA (HL)",		cpu_cb_sra_hl		},	// 0x2E
	{ "SRA A",			cpu_cb_sra_a		},	// 0x2F
	{ "SWAP B",			cpu_cb_swap_b		},	// 0x30
	{ "SWAP C",			cpu_cb_swap_c		},	// 0x31
	{ "SWAP D",			cpu_cb_swap_d		},	// 0x32
	{ "SWAP E",			cpu_cb_swap_e		},	// 0x33
	{ "SWAP H",			cpu_cb_swap_h		},	// 0x34
	{ "SWAP L",			cpu_cb_swap_l		},	// 0x35
	{ "SWAP (HL)",		cpu_cb_swap_hl		},	// 0x36
	{ "SWAP A",			cpu_cb_swap_a		},	// 0x37
	{ "SRL B",			cpu_cb_srl_b		},	// 0x38
	{ "SRL C",			cpu_cb_srl_c		},	// 0x39
	{ "SRL D",			cpu_cb_srl_d		},	// 0x3A
	{ "SRL E",			cpu_cb_srl_e		},	// 0x3B
	{ "SRL H",			cpu_cb_srl_h		},	// 0x3C
	{ "SRL L",			cpu_cb_srl_l		},	// 0x3D
	{ "SRL (HL)",		cpu_cb_srl_hl		},	// 0x3E
	{ "SRL A",			cpu_cb_srl_a		},	// 0x3F
	{ "BIT 0, B",		cpu_cb_bit_0_b		},	// 0x40
	{ "BIT 0, C",		cpu_cb_bit_0_c		},	// 0x41
	{ "BIT 0, D",		cpu_cb_bit_0_d		},	// 0x42
	{ "BIT 0, E",		cpu_cb_bit_0_e		},	// 0x43
	{ "BIT 0, H",		cpu_cb_bit_0_h		},	// 0x44
	{ "BIT 0, L",		cpu_cb_bit_0_l		},	// 0x45
	{ "BIT 0, (HL)",	cpu_cb_bit_0_hl		},	// 0x46
	{ "BIT 0, A",		cpu_cb_bit_0_a		},	// 0x47
	{ "BIT 1, B",		cpu_cb_bit_1_b		},	// 0x48
	{ "BIT 1, C",		cpu_cb_bit_1_c		},	// 0x49
	{ "BIT 1, D",		cpu_cb_bit_1_d		},	// 0x4A
	{ "BIT 1, E",		cpu_cb_bit_1_e		},	// 0x4B
	{ "BIT 1, H",		cpu_cb_bit_1_h		},	// 0x4C
	{ "BIT 1, L",		cpu_cb_bit_1_l		},	// 0x4D
	{ "BIT 1, (HL)",	cpu_cb_bit_1_hl		},	// 0x4E
	{ "BIT 1, A",		cpu_cb_bit_1_a		},	// 0x4F
	{ "BIT 2, B",		cpu_cb_bit_2_b		},	// 0x50
	{ "BIT 2, C",		cpu_cb_bit_2_c		},	// 0x51
	{ "BIT 2, D",		cpu_cb_bit_2_d		},	// 0x52
	{ "BIT 2, E",		cpu_cb_bit_2_e		},	// 0x53
	{ "BIT 2, H",		cpu_cb_bit_2_h		},	// 0x54
	{ "BIT 2, L",		cpu_cb_bit_2_l		},	// 0x55
	{ "BIT 2, (HL)",	cpu_cb_bit_2_hl		},	// 0x56
	{ "BIT 2, A",		cpu_cb_bit_2_a		},	// 0x57
	{ "BIT 3, B",		cpu_cb_bit_3_b		},	// 0x58
	{ "BIT 3, C",		cpu_cb_bit_3_c		},	// 0x59
	{ "BIT 3, D",		cpu_cb_bit_3_d		},	// 0x5A
	{ "BIT 3, E",		cpu_cb_bit_3_e		},	// 0x5B
	{ "BIT 3, H",		cpu_cb_bit_3_h		},	// 0x5C
	{ "BIT 3, L",		cpu_cb_bit_3_l		},	// 0x5D
	{ "BIT 3, (HL)",	cpu_cb_bit_3_hl		},	// 0x5E
	{ "BIT 3, A",		cpu_cb_bit_3_a		},	// 0x5F
	{ "BIT 4, B",		cpu_cb_bit_4_b		},	// 0x60
	{ "BIT 4, C",		cpu_cb_bit_4_c		},	// 0x61
	{ "BIT 4, D",		cpu_cb_bit_4_d		},	// 0x62
	{ "BIT 4, E",		cpu_cb_bit_4_e		},	// 0x63
	{ "BIT 4, H",		cpu_cb_bit_4_h		},	// 0x64
	{ "BIT 4, L",		cpu_cb_bit_4_l		},	// 0x65
	{ "BIT 4, (HL)",	cpu_cb_bit_4_hl		},	// 0x66
	{ "BIT 4, A",		cpu_cb_bit_4_a		},	// 0x67
	{ "BIT 5, B",		cpu_cb_bit_5_b		},	// 0x68
	{ "BIT 5, C",		cpu_cb_bit_5_c		},	// 0x69
	{ "BIT 5, D",		cpu_cb_bit_5_d		},	// 0x6A
	{ "BIT 5, E",		cpu_cb_bit_5_e		},	// 0x6B
	{ "BIT 5, H",		cpu_cb_bit_5_h		},	// 0x6C
	{ "BIT 5, L",		cpu_cb_bit_5_l		},	// 0x6D
	{ "BIT 5, (HL)",	cpu_cb_bit_5_hl		},	// 0x6E
	{ "BIT 5, A",		cpu_cb_bit_5_a		},	// 0x6F
	{ "BIT 6, B",		cpu_cb_bit_6_b		},	// 0x70
	{ "BIT 6, C",		cpu_cb_bit_6_c		},	// 0x71
	{ "BIT 6, D",		cpu_cb_bit_6_d		},	// 0x72
	{ "BIT 6, E",		cpu_cb_bit_6_e		},	// 0x73
	{ "BIT 6, H",		cpu_cb_bit_6_h		},	// 0x74
	{ "BIT 6, L",		cpu_cb_bit_6_l		},	// 0x75
	{ "BIT 6, (HL)",	cpu_cb_bit_6_hl		},	// 0x76
	{ "BIT 6, A",		cpu_cb_bit_6_a		},	// 0x77
	{ "BIT 7, B",		cpu_cb_bit_7_b		},	// 0x78
	{ "BIT 7, C",		cpu_cb_bit_7_c		},	// 0x79
	{ "BIT 7, D",		cpu_cb_bit_7_d		},	// 0x7A
	{ "BIT 7, E",		cpu_cb_bit_7_e		},	// 0x7B
	{ "BIT 7, H",		cpu_cb_bit_7_h		},	// 0x7C
	{ "BIT 7, L",		cpu_cb_bit_7_l		},	// 0x7D
	{ "BIT 7, (HL)",	cpu_cb_bit_7_hl		},	// 0x7E
	{ "BIT 7, A",		cpu_cb_bit_7_a		},	// 0x7F
	{ "RES 0, B",		cpu_cb_res_0_b		},	// 0x80
	{ "RES 0, C",		cpu_cb_res_0_c		},	// 0x81
	{ "RES 0, D",		cpu_cb_res_0_d		},	// 0x82
	{ "RES 0, E",		cpu_res_0_e		},	// 0x83
	{ "RES 0, H",		cpu_res_0_h		},	// 0x84
	{ "RES 0, L",		cpu_res_0_l		},	// 0x85
	{ "RES 0, (HL)",	cpu_res_0_hl	},	// 0x86
	{ "RES 0, A",		cpu_res_0_a		},	// 0x87
	{ "RES 1, B",		cpu_res_1_b		},	// 0x88
	{ "RES 1, C",		cpu_res_1_c		},	// 0x89
	{ "RES 1, D",		cpu_res_1_d		},	// 0x8A
	{ "RES 1, E",		cpu_res_1_e		},	// 0x8B
	{ "RES 1, H",		cpu_res_1_h		},	// 0x8C
	{ "RES 1, L",		cpu_res_1_l		},	// 0x8D
	{ "RES 1, (HL)",	cpu_res_1_hl	},	// 0x8E
	{ "RES 1, A",		cpu_res_1_a		},	// 0x8F
	{ "RES 2, B",		cpu_res_2_b		},	// 0x90
	{ "RES 2, C",		cpu_res_2_c		},	// 0x91
	{ "RES 2, D",		cpu_res_2_d		},	// 0x92
	{ "RES 2, E",		cpu_res_2_e		},	// 0x93
	{ "RES 2, H",		cpu_res_2_h		},	// 0x94
	{ "RES 2, L",		cpu_res_2_l		},	// 0x95
	{ "RES 2, (HL)",	cpu_res_2_hl	},	// 0x96
	{ "RES 2, A",		cpu_res_2_a		},	// 0x97
	{ "RES 3, B",		cpu_res_3_b		},	// 0x98
	{ "RES 3, C",		cpu_res_3_c		},	// 0x99
	{ "RES 3, D",		cpu_res_3_d		},	// 0x9A
	{ "RES 3, E",		cpu_res_3_e		},	// 0x9B
	{ "RES 3, H",		cpu_res_3_h		},	// 0x9C
	{ "RES 3, L",		cpu_res_3_l		},	// 0x9D
	{ "RES 3, (HL)",	cpu_res_3_hl	},	// 0x9E
	{ "RES 3, A",		cpu_res_3_a		},	// 0x9F
	{ "RES 4, B",		cpu_res_4_b		},	// 0xA0
	{ "RES 4, C",		cpu_res_4_c		},	// 0xA1
	{ "RES 4, D",		cpu_res_4_d		},	// 0xA2
	{ "RES 4, E",		cpu_res_4_e		},	// 0xA3
	{ "RES 4, H",		cpu_res_4_h		},	// 0xA4
	{ "RES 4, L",		cpu_res_4_l		},	// 0xA5
	{ "RES 4, (HL)",	cpu_res_4_hl	},	// 0xA6
	{ "RES 4, A",		cpu_res_4_a		},	// 0xA7
	{ "RES 5, B",		cpu_res_5_b		},	// 0xA8
	{ "RES 5, C",		cpu_res_5_c		},	// 0xA9
	{ "RES 5, D",		cpu_res_5_d		},	// 0xAA
	{ "RES 5, E",		cpu_res_5_e		},	// 0xAB
	{ "RES 5, H",		cpu_res_5_h		},	// 0xAC
	{ "RES 5, L",		cpu_res_5_l		},	// 0xAD
	{ "RES 5, (HL)",	cpu_res_5_hl	},	// 0xAE
	{ "RES 5, A",		cpu_res_5_a		},	// 0xAF
	{ "RES 6, B",		cpu_res_6_b		},	// 0xB0
	{ "RES 6, C",		cpu_res_6_c		},	// 0xB1
	{ "RES 6, D",		cpu_res_6_d		},	// 0xB2
	{ "RES 6, E",		cpu_res_6_e		},	// 0xB3
	{ "RES 6, H",		cpu_res_6_h		},	// 0xB4
	{ "RES 6, L",		cpu_res_6_l		},	// 0xB5
	{ "RES 6, (HL)",	cpu_res_6_hl	},	// 0xB6
	{ "RES 6, A",		cpu_res_6_a		},	// 0xB7
	{ "RES 7, B",		cpu_res_7_b		},	// 0xB8
	{ "RES 7, C",		cpu_res_7_c		},	// 0xB9
	{ "RES 7, D",		cpu_res_7_d		},	// 0xBA
	{ "RES 7, E",		cpu_res_7_e		},	// 0xBB
	{ "RES 7, H",		cpu_res_7_h		},	// 0xBC
	{ "RES 7, L",		cpu_res_7_l		},	// 0xBD
	{ "RES 7, (HL)",	cpu_res_7_hl	},	// 0xBE
	{ "RES 7, A",		cpu_res_7_a		},	// 0xBF
	{ "SET 0, B",		cpu_set_0_b		},	// 0xC0
	{ "SET 0, C",		cpu_set_0_c		},	// 0xC1
	{ "SET 0, D",		cpu_set_0_d		},	// 0xC2
	{ "SET 0, E",		cpu_set_0_e		},	// 0xC3
	{ "SET 0, H",		cpu_set_0_h		},	// 0xC4
	{ "SET 0, L",		cpu_set_0_l		},	// 0xC5
	{ "SET 0, (HL)",	cpu_set_0_hl	},	// 0xC6
	{ "SET 0, A",		cpu_set_0_a		},	// 0xC7
	{ "SET 1, B",		cpu_set_1_b		},	// 0xC8
	{ "SET 1, C",		cpu_set_1_c		},	// 0xC9
	{ "SET 1, D",		cpu_set_1_d		},	// 0xCA
	{ "SET 1, E",		cpu_set_1_e		},	// 0xCB
	{ "SET 1, H",		cpu_set_1_h		},	// 0xCC
	{ "SET 1, L",		cpu_set_1_l		},	// 0xCD
	{ "SET 1, (HL)",	cpu_set_1_hl	},	// 0xCE
	{ "SET 1, A",		cpu_set_1_a		},	// 0xCF
	{ "SET 2, B",		cpu_set_2_b		},	// 0xD0
	{ "SET 2, C",		cpu_set_2_c		},	// 0xD1
	{ "SET 2, D",		cpu_set_2_d		},	// 0xD2
	{ "SET 2, E",		cpu_set_2_e		},	// 0xD3
	{ "SET 2, H",		cpu_set_2_h		},	// 0xD4
	{ "SET 2, L",		cpu_set_2_l		},	// 0xD5
	{ "SET 2, (HL)",	cpu_set_2_hl	},	// 0xD6
	{ "SET 2, A",		cpu_set_2_a		},	// 0xD7
	{ "SET 3, B",		cpu_set_3_b		},	// 0xD8
	{ "SET 3, C",		cpu_set_3_c		},	// 0xD9
	{ "SET 3, D",		cpu_set_3_d		},	// 0xDA
	{ "SET 3, E",		cpu_set_3_e		},	// 0xDB
	{ "SET 3, H",		cpu_set_3_h		},	// 0xDC
	{ "SET 3, L",		cpu_set_3_l		},	// 0xDD
	{ "SET 3, (HL)",	cpu_set_3_hl	},	// 0xDE
	{ "SET 3, A",		cpu_set_3_a		},	// 0xDF
	{ "SET 4, B",		cpu_set_4_b		},	// 0xE0
	{ "SET 4, C",		cpu_set_4_c		},	// 0xE1
	{ "SET 4, D",		cpu_set_4_d		},	// 0xE2
	{ "SET 4, E",		cpu_set_4_e		},	// 0xE3
	{ "SET 4, H",		cpu_set_4_h		},	// 0xE4
	{ "SET 4, L",		cpu_set_4_l		},	// 0xE5
	{ "SET 4, (HL)",	cpu_set_4_hl	},	// 0xE6
	{ "SET 4, A",		cpu_set_4_a		},	// 0xE7
	{ "SET 5, B",		cpu_set_5_b		},	// 0xE8
	{ "SET 5, C",		cpu_set_5_c		},	// 0xE9
	{ "SET 5, D",		cpu_set_5_d		},	// 0xEA
	{ "SET 5, E",		cpu_set_5_e		},	// 0xEB
	{ "SET 5, H",		cpu_set_5_h		},	// 0xEC
	{ "SET 5, L",		cpu_set_5_l		},	// 0xED
	{ "SET 5, (HL)",	cpu_set_5_hl	},	// 0xEE
	{ "SET 5, A",		cpu_set_5_a		},	// 0xEF
	{ "SET 6, B",		cpu_set_6_b		},	// 0xF0
	{ "SET 6, C",		cpu_set_6_c		},	// 0xF1
	{ "SET 6, D",		cpu_set_6_d		},	// 0xF2
	{ "SET 6, E",		cpu_set_6_e		},	// 0xF3
	{ "SET 6, H",		cpu_set_6_h		},	// 0xF4
	{ "SET 6, L",		cpu_set_6_l		},	// 0xF5
	{ "SET 6, (HL)",	cpu_set_6_hl	},	// 0xF6
	{ "SET 6, A",		cpu_set_6_a		},	// 0xF7
	{ "SET 7, B",		cpu_set_7_b		},	// 0xF8
	{ "SET 7, C",		cpu_set_7_c		},	// 0xF9
	{ "SET 7, D",		cpu_set_7_d		},	// 0xFA
	{ "SET 7, E",		cpu_set_7_e		},	// 0xFB
	{ "SET 7, H",		cpu_set_7_h		},	// 0xFC
	{ "SET 7, L",		cpu_set_7_l		},	// 0xFD
	{ "SET 7, (HL)",	cpu_set_7_hl	},	// 0xFE
	{ "SET 7, A",		cpu_set_7_a		},	// 0xFF
};