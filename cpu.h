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
void cpu_dec_b();		// 0x05
void cpu_ld_b_n();		// 0x06
void cpu_dec_c();		// 0x0D
void cpu_ld_c_n();		// 0x0E
void cpu_ld_de_nn();	// 0x11
void cpu_ld_de_a();		// 0x12
void cpu_dec_d();		// 0x15
void cpu_ld_d_n();		// 0x16
void cpu_dec_e();		// 0x1D
void cpu_ld_e_n();		// 0x1E
void cpu_ld_hl_nn();	// 0x21
void cpu_dec_h();		// 0x25
void cpu_ld_h_n();		// 0x26
void cpu_dec_l();		// 0x2D
void cpu_ld_l_n();		// 0x2E
void cpu_ld_sp_nn();	// 0x31
void cpu_ldd_hl_a();	// 0x32
void cpu_dec_a();		// 0x3D
void cpu_ld_a_n();		// 0x3E
void cpu_ld_hl_b();		// 0x70
void cpu_ld_hl_c();		// 0x71
void cpu_ld_hl_d();		// 0x72
void cpu_ld_hl_e();		// 0x73
void cpu_ld_hl_h();		// 0x74
void cpu_ld_hl_l();		// 0x75
void cpu_ld_hl_a();		// 0x77
void cpu_jp_nn();		// 0xC3
void cpu_xor_a();		// 0xAF


