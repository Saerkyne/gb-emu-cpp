#pragma once
#include <stdint.h>

#define SET_FLAG_ZERO(value)		cpu_registers.f = (cpu_registers.f & ~(1UL << 7)) | ((value) << 7);
#define GET_FLAG_ZERO				((cpu_registers.f & ~(1UL << 7)) >> 7)

#define SET_FLAG_SUBTRACT(value)	cpu_registers.f = (cpu_registers.f & ~(1UL << 6)) | ((value) << 6);
#define GET_FLAG_SUBTRACT			((cpu_registers.f & ~(1UL << 6)) >> 6)

#define SET_FLAG_HALF_CARRY(value)	cpu_registers.f = (cpu_registers.f & ~(1UL << 5)) | ((value) << 5);
#define GET_FLAG_HALF_CARRY			((cpu_registers.f & ~(1UL << 5)) >> 5)

#define SET_FLAG_CARRY(value)		cpu_registers.f = (cpu_registers.f & ~(1UL << 4)) | ((value) << 4);
#define GET_FLAG_CARRY				((cpu_registers.f & ~(1UL << 4)) >> 4)



struct gb_cpu_registers {
	struct {
		union {
			struct {
				uint8_t f; // Flags
				uint8_t a; // Accumulator
			};
			uint16_t af; // AF register pair
		};
	};

	struct {
		union {
			struct {
				uint8_t c; // General purpose register C
				uint8_t b; // General purpose register B
			};
			uint16_t bc; // BC register pair
		};
	};

	struct {
		union {
			struct {
				uint8_t e; // General purpose register E
				uint8_t d; // General purpose register D
			};
			uint16_t de; // DE register pair
		};
	};

	struct {
		union {
			struct {
				uint8_t l; // General purpose register L
				uint8_t h; // General purpose register H
			};
			uint16_t hl; // HL register pair
		};
	};

	struct {
		union {
			struct {
				uint8_t p;
				uint8_t s;
			};
			uint16_t sp; // Stack Pointer
		};
	};

	uint16_t pc; // Program Counter
};

typedef void(*cpu_execute_op)(); // Function pointer type for CPU instruction execution

extern gb_cpu_registers cpu_registers; // Global instance of CPU registers

void cpu_reset();
void cpu_tick();
void cpu_fetch();
bool cpu_execute();

void cpu_noop();		// 0x00
void cpu_ld_bc_nn();	// 0x01
void cpu_ld_bc_a();		// 0x02
void cpu_inc_bc();		// 0x03
void cpu_inc_b();		// 0x04
void cpu_dec_b();		// 0x05
void cpu_ld_b_n();		// 0x06
void cpu_rlca();		// 0x07
void cpu_ld_nn_sp();	// 0x08
void cpu_add_hl_bc();	// 0x09
void cpu_ld_a_bc();		// 0x0A
void cpu_dec_bc();		// 0x0B
void cpu_inc_c();		// 0x0C
void cpu_dec_c();		// 0x0D
void cpu_ld_c_n();		// 0x0E
void cpu_rrca();		// 0x0F
void cpu_stop();		// 0x10
void cpu_ld_de_nn();	// 0x11
void cpu_ld_de_a();		// 0x12
void cpu_inc_de();		// 0x13
void cpu_inc_d();		// 0x14
void cpu_dec_d();		// 0x15
void cpu_ld_d_n();		// 0x16
void cpu_rla();			// 0x17
void cpu_jr_n();		// 0x18
void cpu_add_hl_de();	// 0x19
void cpu_ld_a_de();		// 0x1A
void cpu_dec_de();		// 0x1B
void cpu_inc_e();		// 0x1C
void cpu_dec_e();		// 0x1D
void cpu_ld_e_n();		// 0x1E
void cpu_rra();			// 0x1F
void cpu_jr_nz_n();		// 0x20
void cpu_ld_hl_nn();	// 0x21
void cpu_ldi_hl_a();	// 0x22
void cpu_inc_hl();		// 0x23
void cpu_inc_h();		// 0x24
void cpu_dec_h();		// 0x25
void cpu_ld_h_n();		// 0x26
void cpu_daa();			// 0x27
void cpu_jr_z_n();		// 0x28
void cpu_add_hl_hl();	// 0x29
void cpu_ldi_a_hl();	// 0x2A
void cpu_dec_hl();		// 0x2B
void cpu_inc_l();		// 0x2C
void cpu_dec_l();		// 0x2D
void cpu_ld_l_n();		// 0x2E
void cpu_cpl();			// 0x2F
void cpu_jr_nc_n();		// 0x30
void cpu_ld_sp_nn();	// 0x31
void cpu_ldd_hl_a();	// 0x32
void cpu_inc_sp();		// 0x33
void cpu_inc__hl();		// 0x34
void cpu_dec__hl();		// 0x35
void cpu_ld_hl_n();		// 0x36
void cpu_scf();			// 0x37
void cpu_jr_c_n();		// 0x38
void cpu_add_hl_sp();	// 0x39
void cpu_ldd_a_hl();	// 0x3A
void cpu_dec_sp();		// 0x3B
void cpu_inc_a();		// 0x3C
void cpu_dec_a();		// 0x3D
void cpu_ld_a_n();		// 0x3E
void cpu_ccf();			// 0x3F
void cpu_ld_b_b();		// 0x40
void cpu_ld_b_c();		// 0x41
void cpu_ld_b_d();		// 0x42
void cpu_ld_b_e();		// 0x43
void cpu_ld_b_h();		// 0x44
void cpu_ld_b_l();		// 0x45
void cpu_ld_b_hl();		// 0x46
void cpu_ld_b_a();		// 0x47
void cpu_ld_c_b();		// 0x48
void cpu_ld_c_c();		// 0x49
void cpu_ld_c_d();		// 0x4A
void cpu_ld_c_e();		// 0x4B
void cpu_ld_c_h();		// 0x4C
void cpu_ld_c_l();		// 0x4D
void cpu_ld_c_hl();		// 0x4E
void cpu_ld_c_a();		// 0x4F
void cpu_ld_d_b();		// 0x50
void cpu_ld_d_c();		// 0x51
void cpu_ld_d_d();		// 0x52
void cpu_ld_d_e();		// 0x53
void cpu_ld_d_h();		// 0x54
void cpu_ld_d_l();		// 0x55
void cpu_ld_d_hl();		// 0x56
void cpu_ld_d_a();		// 0x57
void cpu_ld_e_b();		// 0x58
void cpu_ld_e_c();		// 0x59
void cpu_ld_e_d();		// 0x5A
void cpu_ld_e_e();		// 0x5B
void cpu_ld_e_h();		// 0x5C
void cpu_ld_e_l();		// 0x5D
void cpu_ld_e_hl();		// 0x5E
void cpu_ld_e_a();		// 0x5F
void cpu_ld_h_b();		// 0x60
void cpu_ld_h_c();		// 0x61
void cpu_ld_h_d();		// 0x62
void cpu_ld_h_e();		// 0x63
void cpu_ld_h_h();		// 0x64
void cpu_ld_h_l();		// 0x65
void cpu_ld_h_hl();		// 0x66
void cpu_ld_h_a();		// 0x67
void cpu_ld_l_b();		// 0x68
void cpu_ld_l_c();		// 0x69
void cpu_ld_l_d();		// 0x6A
void cpu_ld_l_e();		// 0x6B
void cpu_ld_l_h();		// 0x6C
void cpu_ld_l_l();		// 0x6D
void cpu_ld_l_hl();		// 0x6E
void cpu_ld_l_a();		// 0x6F
void cpu_ld_hl_b();		// 0x70
void cpu_ld_hl_c();		// 0x71
void cpu_ld_hl_d();		// 0x72
void cpu_ld_hl_e();		// 0x73
void cpu_ld_hl_h();		// 0x74
void cpu_ld_hl_l();		// 0x75
void cpu_halt();		// 0x76
void cpu_ld_hl_a();		// 0x77
void cpu_ld_a_b();		// 0x78
void cpu_ld_a_c();		// 0x79
void cpu_ld_a_d();		// 0x7A
void cpu_ld_a_e();		// 0x7B
void cpu_ld_a_h();		// 0x7C
void cpu_ld_a_l();		// 0x7D
void cpu_ld_a_hl();		// 0x7E
void cpu_ld_a_a();		// 0x7F
void cpu_add_a_b();		// 0x80
void cpu_add_a_c();		// 0x81
void cpu_add_a_d();		// 0x82
void cpu_add_a_e();		// 0x83
void cpu_add_a_h();		// 0x84
void cpu_add_a_l();		// 0x85
void cpu_add_a_hl();	// 0x86
void cpu_add_a_a();		// 0x87
void cpu_adc_a_b();		// 0x88
void cpu_adc_a_c();		// 0x89
void cpu_adc_a_d();		// 0x8A
void cpu_adc_a_e();		// 0x8B
void cpu_adc_a_h();		// 0x8C
void cpu_adc_a_l();		// 0x8D
void cpu_adc_a_hl();	// 0x8E
void cpu_adc_a_a();		// 0x8F
void cpu_sub_a_b();		// 0x90
void cpu_sub_a_c();		// 0x91
void cpu_sub_a_d();		// 0x92
void cpu_sub_a_e();		// 0x93
void cpu_sub_a_h();		// 0x94
void cpu_sub_a_l();		// 0x95
void cpu_sub_a_hl();	// 0x96
void cpu_sub_a_a();		// 0x97
void cpu_sbc_a_b();		// 0x98
void cpu_sbc_a_c();		// 0x99
void cpu_sbc_a_d();		// 0x9A
void cpu_sbc_a_e();		// 0x9B
void cpu_sbc_a_h();		// 0x9C
void cpu_sbc_a_l();		// 0x9D
void cpu_sbc_a_hl();	// 0x9E
void cpu_sbc_a_a();		// 0x9F
void cpu_and_a_b();		// 0xA0
void cpu_and_a_c();		// 0xA1
void cpu_and_a_d();		// 0xA2
void cpu_and_a_e();		// 0xA3
void cpu_and_a_h();		// 0xA4
void cpu_and_a_l();		// 0xA5
void cpu_and_a_hl();	// 0xA6
void cpu_and_a_a();		// 0xA7
void cpu_xor_a_b();		// 0xA8
void cpu_xor_a_c();		// 0xA9
void cpu_xor_a_d();		// 0xAA
void cpu_xor_a_e();		// 0xAB
void cpu_xor_a_h();		// 0xAC
void cpu_xor_a_l();		// 0xAD
void cpu_xor_a_hl();	// 0xAE
void cpu_xor_a();		// 0xAF
void cpu_or_a_b();		// 0xB0
void cpu_or_a_c();		// 0xB1
void cpu_or_a_d();		// 0xB2
void cpu_or_a_e();		// 0xB3
void cpu_or_a_h();		// 0xB4
void cpu_or_a_l();		// 0xB5
void cpu_or_a_hl();		// 0xB6
void cpu_or_a_a();		// 0xB7
void cpu_cp_a_b();		// 0xB8
void cpu_cp_a_c();		// 0xB9
void cpu_cp_a_d();		// 0xBA
void cpu_cp_a_e();		// 0xBB
void cpu_cp_a_h();		// 0xBC
void cpu_cp_a_l();		// 0xBD
void cpu_cp_a_hl();		// 0xBE
void cpu_cp_a_a();		// 0xBF
void cpu_ret_nz();		// 0xC0
void cpu_pop_bc();		// 0xC1
void cpu_jp_nz_nn();	// 0xC2
void cpu_jp_nn();		// 0xC3
void cpu_call_nz_nn();	// 0xC4
void cpu_push_bc();		// 0xC5
void cpu_add_a_n();		// 0xC6
void cpu_rst_00();		// 0xC7
void cpu_ret_z();		// 0xC8
void cpu_ret();			// 0xC9
void cpu_jp_z_nn();		// 0xCA
void cpu_prefix_cb();	// 0xCB
void cpu_call_z_nn();	// 0xCC
void cpu_call_nn();		// 0xCD
void cpu_adc_a_n();		// 0xCE
void cpu_rst_08();		// 0xCF
void cpu_ret_nc();		// 0xD0
void cpu_pop_de();		// 0xD1
void cpu_jp_nc_nn();	// 0xD2
						// 0xD3
void cpu_call_nc_nn();	// 0xD4
void cpu_push_de();		// 0xD5
void cpu_sub_a_n();		// 0xD6
void cpu_rst_10();		// 0xD7
void cpu_ret_c();		// 0xD8
void cpu_reti();		// 0xD9
void cpu_jp_c_nn();		// 0xDA
						// 0xDB
void cpu_call_c_nn();	// 0xDC
						// 0xDD
void cpu_sbc_a_n();		// 0xDE
void cpu_rst_18();		// 0xDF
void cpu_ldh_n_a();		// 0xE0
void cpu_pop_hl();		// 0xE1
void cpu_ldh_c_a();		// 0xE2
						// 0xE3
						// 0xE4
void cpu_push_hl();		// 0xE5
void cpu_and_a_n();		// 0xE6
void cpu_rst_20();		// 0xE7
void cpu_add_sp_n();	// 0xE8
void cpu_jp_hl();		// 0xE9
void cpu_ld_nn_a();		// 0xEA
						// 0xEB
						// 0xEC
						// 0xED
void cpu_xor_a_n();		// 0xEE
void cpu_rst_28();		// 0xEF
void cpu_ldh_a_n();		// 0xF0
void cpu_pop_af();		// 0xF1
void cpu_ldh_a_c();		// 0xF2
void cpu_di();			// 0xF3
						// 0xF4
void cpu_push_af();		// 0xF5
void cpu_or_a_n();		// 0xF6
void cpu_rst_30();		// 0xF7
void cpu_ld_hl_sp_n();	// 0xF8
void cpu_ld_sp_hl();	// 0xF9
void cpu_ld_a_nn();		// 0xFA
void cpu_ei();			// 0xFB
						// 0xFC
						// 0xFD
void cpu_cp_a_n();		// 0xFE
void cpu_rst_38();		// 0xFF

// CB Prefixed instructions
void cpu_cb_rlc_b();	// 0x00
void cpu_cb_rlc_c();	// 0x01
void cpu_cb_rlc_d();	// 0x02
void cpu_cb_rlc_e();	// 0x03
void cpu_cb_rlc_h();	// 0x04
void cpu_cb_rlc_l();	// 0x05
void cpu_cb_rlc_hl();	// 0x06
void cpu_cb_rlc_a();	// 0x07
void cpu_cb_rrc_b();	// 0x08
void cpu_cb_rrc_c();	// 0x09
void cpu_cb_rrc_d();	// 0x0A
void cpu_cb_rrc_e();	// 0x0B
void cpu_cb_rrc_h();	// 0x0C
void cpu_cb_rrc_l();	// 0x0D
void cpu_cb_rrc_hl();	// 0x0E
void cpu_cb_rrc_a();	// 0x0F
void cpu_cb_rl_b();		// 0x10
void cpu_cb_rl_c();		// 0x11
void cpu_cb_rl_d();		// 0x12
void cpu_cb_rl_e();		// 0x13
void cpu_cb_rl_h();		// 0x14
void cpu_cb_rl_l();		// 0x15
void cpu_cb_rl_hl();	// 0x16
void cpu_cb_rl_a();		// 0x17
void cpu_cb_rr_b();		// 0x18
void cpu_cb_rr_c();		// 0x19
void cpu_cb_rr_d();		// 0x1A
void cpu_cb_rr_e();		// 0x1B
void cpu_cb_rr_h();		// 0x1C
void cpu_cb_rr_l();		// 0x1D
void cpu_cb_rr_hl();	// 0x1E
void cpu_cb_rr_a();		// 0x1F
void cpu_cb_sla_b();	// 0x20
void cpu_cb_sla_c();	// 0x21
void cpu_cb_sla_d();	// 0x22
void cpu_cb_sla_e();	// 0x23
void cpu_cb_sla_h();	// 0x24
void cpu_cb_sla_l();	// 0x25
void cpu_cb_sla_hl();	// 0x26
void cpu_cb_sla_a();	// 0x27
void cpu_cb_sra_b();	// 0x28
void cpu_cb_sra_c();	// 0x29
void cpu_cb_sra_d();	// 0x2A
void cpu_cb_sra_e();	// 0x2B
void cpu_cb_sra_h();	// 0x2C
void cpu_cb_sra_l();	// 0x2D
void cpu_cb_sra_hl();	// 0x2E
void cpu_cb_sra_a();	// 0x2F
void cpu_cb_swap_b();	// 0x30
void cpu_cb_swap_c();	// 0x31
void cpu_cb_swap_d();	// 0x32
void cpu_cb_swap_e();	// 0x33
void cpu_cb_swap_h();	// 0x34
void cpu_cb_swap_l();	// 0x35
void cpu_cb_swap_hl();	// 0x36
void cpu_cb_swap_a();	// 0x37
void cpu_cb_srl_b();	// 0x38
void cpu_cb_srl_c();	// 0x39
void cpu_cb_srl_d();	// 0x3A
void cpu_cb_srl_e();	// 0x3B
void cpu_cb_srl_h();	// 0x3C
void cpu_cb_srl_l();	// 0x3D
void cpu_cb_srl_hl();	// 0x3E
void cpu_cb_srl_a();	// 0x3F
void cpu_cb_bit_0_b();	// 0x40
void cpu_cb_bit_0_c();	// 0x41
void cpu_cb_bit_0_d();	// 0x42
void cpu_cb_bit_0_e();	// 0x43
void cpu_cb_bit_0_h();	// 0x44
void cpu_cb_bit_0_l();	// 0x45
void cpu_cb_bit_0_hl();	// 0x46
void cpu_cb_bit_0_a();	// 0x47
void cpu_cb_bit_1_b();	// 0x48
void cpu_cb_bit_1_c();	// 0x49
void cpu_cb_bit_1_d();	// 0x4A
void cpu_cb_bit_1_e();	// 0x4B
void cpu_cb_bit_1_h();	// 0x4C
void cpu_cb_bit_1_l();	// 0x4D
void cpu_cb_bit_1_hl();	// 0x4E
void cpu_cb_bit_1_a();	// 0x4F
void cpu_cb_bit_2_b();	// 0x50
void cpu_cb_bit_2_c();	// 0x51
void cpu_cb_bit_2_d();	// 0x52
void cpu_cb_bit_2_e();	// 0x53
void cpu_cb_bit_2_h();	// 0x54
void cpu_cb_bit_2_l();	// 0x55
void cpu_cb_bit_2_hl();	// 0x56
void cpu_cb_bit_2_a();	// 0x57
void cpu_cb_bit_3_b();	// 0x58
void cpu_cb_bit_3_c();	// 0x59
void cpu_cb_bit_3_d();	// 0x5A
void cpu_cb_bit_3_e();	// 0x5B
void cpu_cb_bit_3_h();	// 0x5C
void cpu_cb_bit_3_l();	// 0x5D
void cpu_cb_bit_3_hl();	// 0x5E
void cpu_cb_bit_3_a();	// 0x5F
void cpu_cb_bit_4_b();	// 0x60
void cpu_cb_bit_4_c();	// 0x61
void cpu_cb_bit_4_d();	// 0x62
void cpu_cb_bit_4_e();	// 0x63
void cpu_cb_bit_4_h();	// 0x64
void cpu_cb_bit_4_l();	// 0x65
void cpu_cb_bit_4_hl();	// 0x66
void cpu_cb_bit_4_a();	// 0x67
void cpu_cb_bit_5_b();	// 0x68
void cpu_cb_bit_5_c();	// 0x69
void cpu_cb_bit_5_d();	// 0x6A
void cpu_cb_bit_5_e();	// 0x6B
void cpu_cb_bit_5_h();	// 0x6C
void cpu_cb_bit_5_l();	// 0x6D
void cpu_cb_bit_5_hl();	// 0x6E
void cpu_cb_bit_5_a();	// 0x6F
void cpu_cb_bit_6_b();	// 0x70
void cpu_cb_bit_6_c();	// 0x71
void cpu_cb_bit_6_d();	// 0x72
void cpu_cb_bit_6_e();	// 0x73
void cpu_cb_bit_6_h();	// 0x74
void cpu_cb_bit_6_l();	// 0x75
void cpu_cb_bit_6_hl();	// 0x76
void cpu_cb_bit_6_a();	// 0x77
void cpu_cb_bit_7_b();	// 0x78
void cpu_cb_bit_7_c();	// 0x79
void cpu_cb_bit_7_d();	// 0x7A
void cpu_cb_bit_7_e();	// 0x7B
void cpu_cb_bit_7_h();	// 0x7C
void cpu_cb_bit_7_l();	// 0x7D
void cpu_cb_bit_7_hl();	// 0x7E
void cpu_cb_bit_7_a();	// 0x7F
void cpu_cb_res_0_b();	// 0x80
void cpu_cb_res_0_c();	// 0x81
void cpu_cb_res_0_d();	// 0x82
void cpu_cb_res_0_e();	// 0x83
void cpu_cb_res_0_h();	// 0x84
void cpu_cb_res_0_l();	// 0x85
void cpu_cb_res_0_hl();	// 0x86
void cpu_cb_res_0_a();	// 0x87
void cpu_cb_res_1_b();	// 0x88
void cpu_cb_res_1_c();	// 0x89
void cpu_cb_res_1_d();	// 0x8A
void cpu_cb_res_1_e();	// 0x8B
void cpu_cb_res_1_h();	// 0x8C
void cpu_cb_res_1_l();	// 0x8D
void cpu_cb_res_1_hl();	// 0x8E
void cpu_cb_res_1_a();	// 0x8F
void cpu_cb_res_2_b();	// 0x90
void cpu_cb_res_2_c();	// 0x91
void cpu_cb_res_2_d();	// 0x92
void cpu_cb_res_2_e();	// 0x93
void cpu_cb_res_2_h();	// 0x94
void cpu_cb_res_2_l();	// 0x95
void cpu_cb_res_2_hl();	// 0x96
void cpu_cb_res_2_a();	// 0x97
void cpu_cb_res_3_b();	// 0x98
void cpu_cb_res_3_c();	// 0x99
void cpu_cb_res_3_d();	// 0x9A
void cpu_cb_res_3_e();	// 0x9B
void cpu_cb_res_3_h();	// 0x9C
void cpu_cb_res_3_l();	// 0x9D
void cpu_cb_res_3_hl();	// 0x9E
void cpu_cb_res_3_a();	// 0x9F
void cpu_cb_res_4_b();	// 0xA0
void cpu_cb_res_4_c();	// 0xA1
void cpu_cb_res_4_d();	// 0xA2
void cpu_cb_res_4_e();	// 0xA3
void cpu_cb_res_4_h();	// 0xA4
void cpu_cb_res_4_l();	// 0xA5
void cpu_cb_res_4_hl();	// 0xA6
void cpu_cb_res_4_a();	// 0xA7
void cpu_cb_res_5_b();	// 0xA8
void cpu_cb_res_5_c();	// 0xA9
void cpu_cb_res_5_d();	// 0xAA
void cpu_cb_res_5_e();	// 0xAB
void cpu_cb_res_5_h();	// 0xAC
void cpu_cb_res_5_l();	// 0xAD
void cpu_cb_res_5_hl();	// 0xAE
void cpu_cb_res_5_a();	// 0xAF
void cpu_cb_res_6_b();	// 0xB0
void cpu_cb_res_6_c();	// 0xB1
void cpu_cb_res_6_d();	// 0xB2
void cpu_cb_res_6_e();	// 0xB3
void cpu_cb_res_6_h();	// 0xB4
void cpu_cb_res_6_l();	// 0xB5
void cpu_cb_res_6_hl();	// 0xB6
void cpu_cb_res_6_a();	// 0xB7
void cpu_cb_res_7_b();	// 0xB8
void cpu_cb_res_7_c();	// 0xB9
void cpu_cb_res_7_d();	// 0xBA
void cpu_cb_res_7_e();	// 0xBB
void cpu_cb_res_7_h();	// 0xBC
void cpu_cb_res_7_l();	// 0xBD
void cpu_cb_res_7_hl();	// 0xBE
void cpu_cb_res_7_a();	// 0xBF
void cpu_cb_set_0_b();	// 0xC0
void cpu_cb_set_0_c();	// 0xC1
void cpu_cb_set_0_d();	// 0xC2
void cpu_cb_set_0_e();	// 0xC3
void cpu_cb_set_0_h();	// 0xC4
void cpu_cb_set_0_l();	// 0xC5
void cpu_cb_set_0_hl();	// 0xC6
void cpu_cb_set_0_a();	// 0xC7
void cpu_cb_set_1_b();	// 0xC8
void cpu_cb_set_1_c();	// 0xC9
void cpu_cb_set_1_d();	// 0xCA
void cpu_cb_set_1_e();	// 0xCB
void cpu_cb_set_1_h();	// 0xCC
void cpu_cb_set_1_l();	// 0xCD
void cpu_cb_set_1_hl();	// 0xCE
void cpu_cb_set_1_a();	// 0xCF
void cpu_cb_set_2_b();	// 0xD0
void cpu_cb_set_2_c();	// 0xD1
void cpu_cb_set_2_d();	// 0xD2
void cpu_cb_set_2_e();	// 0xD3
void cpu_cb_set_2_h();	// 0xD4
void cpu_cb_set_2_l();	// 0xD5
void cpu_cb_set_2_hl();	// 0xD6
void cpu_cb_set_2_a();	// 0xD7
void cpu_cb_set_3_b();	// 0xD8
void cpu_cb_set_3_c();	// 0xD9
void cpu_cb_set_3_d();	// 0xDA
void cpu_cb_set_3_e();	// 0xDB
void cpu_cb_set_3_h();	// 0xDC
void cpu_cb_set_3_l();	// 0xDD
void cpu_cb_set_3_hl();	// 0xDE
void cpu_cb_set_3_a();	// 0xDF
void cpu_cb_set_4_b();	// 0xE0
void cpu_cb_set_4_c();	// 0xE1
void cpu_cb_set_4_d();	// 0xE2
void cpu_cb_set_4_e();	// 0xE3
void cpu_cb_set_4_h();	// 0xE4
void cpu_cb_set_4_l();	// 0xE5
void cpu_cb_set_4_hl();	// 0xE6
void cpu_cb_set_4_a();	// 0xE7
void cpu_cb_set_5_b();	// 0xE8
void cpu_cb_set_5_c();	// 0xE9
void cpu_cb_set_5_d();	// 0xEA
void cpu_cb_set_5_e();	// 0xEB
void cpu_cb_set_5_h();	// 0xEC
void cpu_cb_set_5_l();	// 0xED
void cpu_cb_set_5_hl();	// 0xEE
void cpu_cb_set_5_a();	// 0xEF
void cpu_cb_set_6_b();	// 0xF0
void cpu_cb_set_6_c();	// 0xF1
void cpu_cb_set_6_d();	// 0xF2
void cpu_cb_set_6_e();	// 0xF3
void cpu_cb_set_6_h();	// 0xF4
void cpu_cb_set_6_l();	// 0xF5
void cpu_cb_set_6_hl();	// 0xF6
void cpu_cb_set_6_a();	// 0xF7
void cpu_cb_set_7_b();	// 0xF8
void cpu_cb_set_7_c();	// 0xF9
void cpu_cb_set_7_d();	// 0xFA
void cpu_cb_set_7_e();	// 0xFB
void cpu_cb_set_7_h();	// 0xFC
void cpu_cb_set_7_l();	// 0xFD
void cpu_cb_set_7_hl();	// 0xFE
void cpu_cb_set_7_a();	// 0xFF