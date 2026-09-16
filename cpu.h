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
void cpu_fetch();
bool cpu_execute();

void cpu_noop();		// 0x00
void cpu_ld_bc_nn();	// 0x01
void cpu_ld_bc_a();		// 0x02
void cpu_inc_bc();		// 0x03
void cpu_inc_b();		// 0x04
void cpu_dec_b();		// 0x05
void cpu_ld_b_n();		// 0x06
void cpu_add_hl_bc();	// 0x09
void cpu_ld_a_bc();		// 0x0A
void cpu_dec_bc();		// 0x0B
void cpu_inc_c();		// 0x0C
void cpu_dec_c();		// 0x0D
void cpu_ld_c_n();		// 0x0E
void cpu_ld_de_nn();	// 0x11
void cpu_ld_de_a();		// 0x12
void cpu_inc_de();		// 0x13
void cpu_inc_d();		// 0x14
void cpu_dec_d();		// 0x15
void cpu_ld_d_n();		// 0x16
void cpu_add_hl_de();	// 0x19
void cpu_ld_a_de();		// 0x1A
void cpu_dec_de();		// 0x1B
void cpu_inc_e();		// 0x1C
void cpu_dec_e();		// 0x1D
void cpu_ld_e_n();		// 0x1E
void cpu_ld_hl_nn();	// 0x21
void cpu_inc_hl();		// 0x23
void cpu_inc_h();		// 0x24
void cpu_dec_h();		// 0x25
void cpu_ld_h_n();		// 0x26
void cpu_dec_hl();		// 0x2B
void cpu_inc_l();		// 0x2C
void cpu_dec_l();		// 0x2D
void cpu_ld_l_n();		// 0x2E
void cpu_ld_sp_nn();	// 0x31
void cpu_ldd_hl_a();	// 0x32
void cpu_inc_sp();		// 0x33
void cpu_add_hl_sp();	// 0x39
void cpu_dec_sp();		// 0x3B
void cpu_inc_a();		// 0x3C
void cpu_dec_a();		// 0x3D
void cpu_ld_a_n();		// 0x3E
void cpu_ld_b_hl();		// 0x46
void cpu_ld_c_hl();		// 0x4E
void cpu_ld_d_hl();		// 0x56
void cpu_ld_e_hl();		// 0x5E
void cpu_ld_h_hl();		// 0x66
void cpu_ld_l_hl();		// 0x6E
void cpu_ld_a_hl();		// 0x7E
void cpu_ld_hl_b();		// 0x70
void cpu_ld_hl_c();		// 0x71
void cpu_ld_hl_d();		// 0x72
void cpu_ld_hl_e();		// 0x73
void cpu_ld_hl_h();		// 0x74
void cpu_ld_hl_l();		// 0x75
void cpu_ld_hl_a();		// 0x77
void cpu_add_a_b();		// 0x80
void cpu_add_a_c();		// 0x81
void cpu_add_a_d();		// 0x82
void cpu_add_a_e();		// 0x83
void cpu_add_a_h();		// 0x84
void cpu_add_a_l();		// 0x85
void cpu_adc_a_b();		// 0x88
void cpu_adc_a_c();		// 0x89
void cpu_adc_a_d();		// 0x8A
void cpu_adc_a_e();		// 0x8B
void cpu_adc_a_h();		// 0x8C
void cpu_adc_a_l();		// 0x8D
void cpu_sub_a_b();		// 0x90
void cpu_sub_a_c();		// 0x91
void cpu_sub_a_d();		// 0x92
void cpu_sub_a_e();		// 0x93
void cpu_sub_a_h();		// 0x94
void cpu_sub_a_l();		// 0x95
void cpu_sbc_a_b();		// 0x98
void cpu_sbc_a_c();		// 0x99
void cpu_sbc_a_d();		// 0x9A
void cpu_sbc_a_e();		// 0x9B
void cpu_sbc_a_h();		// 0x9C
void cpu_sbc_a_l();		// 0x9D
void cpu_and_a_b();		// 0xA0
void cpu_and_a_c();		// 0xA1
void cpu_and_a_d();		// 0xA2
void cpu_and_a_e();		// 0xA3
void cpu_and_a_h();		// 0xA4
void cpu_and_a_l();		// 0xA5
void cpu_xor_a_b();		// 0xA8
void cpu_xor_a_c();		// 0xA9
void cpu_xor_a_d();		// 0xAA
void cpu_xor_a_e();		// 0xAB
void cpu_xor_a_h();		// 0xAC
void cpu_xor_a_l();		// 0xAD
void cpu_xor_a();		// 0xAF
void cpu_or_a_b();		// 0xB0
void cpu_or_a_c();		// 0xB1
void cpu_or_a_d();		// 0xB2
void cpu_or_a_e();		// 0xB3
void cpu_or_a_h();		// 0xB4
void cpu_or_a_l();		// 0xB5
void cpu_cp_a_b();		// 0xB8
void cpu_cp_a_c();		// 0xB9
void cpu_cp_a_d();		// 0xBA
void cpu_cp_a_e();		// 0xBB
void cpu_cp_a_h();		// 0xBC
void cpu_cp_a_l();		// 0xBD
void cpu_ret_nz();		// 0xC0
void cpu_pop_bc();		// 0xC1
void cpu_jp_nn();		// 0xC3
void cpu_call_nz_nn();	// 0xC4
void cpu_push_bc();		// 0xC5
void cpu_rst_00();		// 0xC7
void cpu_ret_z();		// 0xC8
void cpu_call_z_nn();	// 0xCC
void cpu_rst_08();		// 0xCF
void cpu_ret_nc();		// 0xD0
void cpu_pop_de();		// 0xD1
void cpu_call_nc_nn();	// 0xD4
void cpu_push_de();		// 0xD5
void cpu_rst_10();		// 0xD7
void cpu_ret_c();		// 0xD8
void cpu_call_c_nn();	// 0xDC
void cpu_rst_18();		// 0xDF
void cpu_pop_hl();		// 0xE1
void cpu_push_hl();		// 0xE5
void cpu_rst_20();		// 0xE7
void cpu_rst_28();		// 0xEF
void cpu_pop_af();		// 0xF1
void cpu_push_af();		// 0xF5
void cpu_rst_30();		// 0xF7
void cpu_rst_38();		// 0xFF