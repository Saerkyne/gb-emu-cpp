#include "cpu.h"
#include "cpu_instructions.h"
#include "cpu_routines.h"
#include "memory_bus.h"
#include "emulator_core.h"
#include <stdio.h>

gb_cpu_registers cpu_registers; // Global instance of CPU registers
uint8_t cpu_current_op_code = 0; // Current operation code being executed
uint32_t cpu_instruction_counter = 0; // Counter for the number of instructions executed
void* cpu_current_instruction_execute = nullptr; // Pointer to the current instruction's execute function

void cpu_reset()
{
	// AFter executing boot rom, registers should be set to the following values:
	cpu_registers.af = 0x01B0;
	cpu_registers.bc = 0x0013;
	cpu_registers.de = 0x00D8;
	cpu_registers.hl = 0x014D;
	cpu_registers.sp = 0xFFFE;
	cpu_registers.pc = 0x0100; // Start executing at the beginning of the cartridge
}

void cpu_fetch()
{
	cpu_current_op_code = memory_bus_read(cpu_registers.pc++);
	const gb_cpu_instruction& instruction = instructions[cpu_current_op_code];
	cpu_current_instruction_execute = instruction.execute;
}

bool cpu_execute()
{
	if (!cpu_current_instruction_execute)
	{
		const gb_cpu_instruction& instruction = instructions[cpu_current_op_code];
		const uint8_t pchi = ((cpu_registers.pc - 1) & 0xFF00) >> 8;
		const uint8_t pclo = ((cpu_registers.pc - 1) & 0xFF);
		printf("Unknown instruction %.2X at: %.2X%.2X (%s), count %i\n", cpu_current_op_code, pchi, pclo, instruction.dissassembly, cpu_instruction_counter);
		return false;
	}
	// This actually executes the instruction
	((cpu_execute_op)cpu_current_instruction_execute)();
	return true;
}

void cpu_noop() // 0x00
{
	core_advance_cpu_clocks(4); // NOP takes 4 clock cycles
}

void cpu_ld_bc_nn() // 0x01
{
	cpu_routine_ld_16(cpu_registers.b, cpu_registers.c); // Load 16-bit immediate value into BC register pair
}

void cpu_ld_bc_a() // 0x02
{
	cpu_routine_ld_ptr8(cpu_registers.bc, cpu_registers.a); // Load 8-bit value into a from 16-bit memory address in BC
}

void cpu_inc_bc() // 0x03
{
	cpu_routine_inc_16(cpu_registers.bc); // Increment BC register pair
}

void cpu_inc_b() // 0x04
{
	cpu_routine_inc_8(cpu_registers.b); // Increment B register
}

void cpu_dec_b() // 0x05
{
	cpu_routine_dec_8(cpu_registers.b); // Decrement B register
}

void cpu_ld_b_n() // 0x06
{
	cpu_routine_ld_8(cpu_registers.b); // Load 8-bit immediate value into B register
}

void cpu_add_hl_bc() // 0x09
{
	cpu_routine_add_hl_16(cpu_registers.bc); // Add BC register pair to HL register pair
}

void cpu_ld_a_bc() // 0x0A
{
	cpu_routine_ld_ptr16(cpu_registers.a, cpu_registers.bc); // Load 8-bit value into A from 16-bit memory address in BC
}

void cpu_dec_bc() // 0x0B
{
	cpu_routine_dec_16(cpu_registers.bc); // Decrement BC register pair
}

void cpu_inc_c() // 0x0C
{
	cpu_routine_inc_8(cpu_registers.c); // Increment C register
}

void cpu_dec_c() // 0x0D
{
	cpu_routine_dec_8(cpu_registers.c); // Decrement C register
}

void cpu_ld_c_n() // 0x0E
{
	cpu_routine_ld_8(cpu_registers.c); // Load 8-bit immediate value into C register
}

void cpu_ld_de_nn() // 0x11
{
	cpu_routine_ld_16(cpu_registers.d, cpu_registers.e); // Load 16-bit immediate value into DE register pair
}

void cpu_ld_de_a() // 0x12
{
	cpu_routine_ld_ptr8(cpu_registers.de, cpu_registers.a); // Load 8-bit value into register a from 16-bit memory address in DE
}

void cpu_inc_de() // 0x13
{
	cpu_routine_inc_16(cpu_registers.de); // Increment DE register pair
}

void cpu_inc_d() // 0x14
{
	cpu_routine_inc_8(cpu_registers.d); // Increment D register
}

void cpu_dec_d() // 0x15
{
	cpu_routine_dec_8(cpu_registers.d); // Decrement D register
}

void cpu_ld_d_n() // 0x16
{
	cpu_routine_ld_8(cpu_registers.d); // Load 8-bit immediate value into D register
}

void cpu_add_hl_de() // 0x19
{
	cpu_routine_add_hl_16(cpu_registers.de); // Add DE register pair to HL register pair
}

void cpu_ld_a_de() // 0x1A
{
	cpu_routine_ld_ptr16(cpu_registers.a, cpu_registers.de); // Load 8-bit value into A from 16-bit memory address in DE
}

void cpu_dec_de() // 0x1B
{
	cpu_routine_dec_16(cpu_registers.de); // Decrement DE register pair
}

void cpu_inc_e() // 0x1C
{
	cpu_routine_inc_8(cpu_registers.e); // Increment E register
}

void cpu_dec_e() // 0x1D
{
	cpu_routine_dec_8(cpu_registers.e); // Decrement E register
}

void cpu_ld_e_n() // 0x1E
{
	cpu_routine_ld_8(cpu_registers.e); // Load 8-bit immediate value into E register
}

void cpu_ld_hl_nn() // 0x21
{
	cpu_routine_ld_16(cpu_registers.h, cpu_registers.l); // Load 16-bit immediate value into HL register pair
}

void cpu_inc_hl() // 0x23
{
	cpu_routine_inc_16(cpu_registers.hl); // Increment HL register pair
}

void cpu_inc_h() // 0x24
{
	cpu_routine_inc_8(cpu_registers.h); // Increment H register
}

void cpu_dec_h() // 0x25
{
	cpu_routine_dec_8(cpu_registers.h); // Decrement H register
}

void cpu_ld_h_n() // 0x26
{
	cpu_routine_ld_8(cpu_registers.h); // Load 8-bit immediate value into H register
}

void cpu_dec_hl() // 0x2B
{
	cpu_routine_dec_16(cpu_registers.hl); // Decrement HL register pair
}

void cpu_inc_l() // 0x2C
{
	cpu_routine_inc_8(cpu_registers.l); // Increment L register
}

void cpu_dec_l() // 0x2D
{
	cpu_routine_dec_8(cpu_registers.l); // Decrement L register
}

void cpu_ld_l_n() // 0x2E
{
	cpu_routine_ld_8(cpu_registers.l); // Load 8-bit immediate value into L register
}

void cpu_ld_sp_nn() // 0x31
{
	cpu_routine_ld_16(cpu_registers.s, cpu_registers.p); // Load 16-bit immediate value into SP register
}

void cpu_ldd_hl_a() // 0x32
{
	core_advance_cpu_clocks(4); // LD (HL-), A takes 8 clock cycles, but we jump 4 per action instead of all at once
	memory_bus_write(cpu_registers.hl, cpu_registers.a); // Write A to memory at HL
	cpu_registers.hl = (cpu_registers.hl - 1) & 0xFFFF; // Decrement HL, ensure it's 16 bits
	core_advance_cpu_clocks(4);
}

void cpu_inc_sp() // 0x33
{
	cpu_routine_inc_16(cpu_registers.sp); // Increment SP register
}

void cpu_add_hl_sp() // 0x39
{
	cpu_routine_add_hl_16(cpu_registers.sp); // Add SP register to HL register pair
}

void cpu_dec_sp() // 0x3B
{
	cpu_routine_dec_16(cpu_registers.sp); // Decrement SP register
}

void cpu_inc_a() // 0x3C
{
	cpu_routine_inc_8(cpu_registers.a); // Increment A register
}

void cpu_dec_a() // 0x3D
{
	cpu_routine_dec_8(cpu_registers.a); // Decrement A register
}

void cpu_ld_a_n() // 0x3E
{
	cpu_routine_ld_8(cpu_registers.a); // Load 8-bit immediate value into A register
}

void cpu_ld_b_hl() // 0x46
{
	cpu_routine_ld_ptr16(cpu_registers.b, cpu_registers.hl); // Load 8-bit value into B from 16-bit memory address in HL
}

void cpu_ld_c_hl() // 0x4E
{
	cpu_routine_ld_ptr16(cpu_registers.c, cpu_registers.hl); // Load 8-bit value into C from 16-bit memory address in HL
}

void cpu_ld_d_hl() // 0x56
{
	cpu_routine_ld_ptr16(cpu_registers.d, cpu_registers.hl); // Load 8-bit value into D from 16-bit memory address in HL
}

void cpu_ld_e_hl() // 0x5E
{
	cpu_routine_ld_ptr16(cpu_registers.e, cpu_registers.hl); // Load 8-bit value into E from 16-bit memory address in HL
}

void cpu_ld_h_hl() // 0x66
{
	cpu_routine_ld_ptr16(cpu_registers.h, cpu_registers.hl); // Load 8-bit value into H from 16-bit memory address in HL
}

void cpu_ld_l_hl() // 0x6E
{
	cpu_routine_ld_ptr16(cpu_registers.l, cpu_registers.hl); // Load 8-bit value into L from 16-bit memory address in HL
}

void cpu_ld_a_hl() // 0x7E
{
	cpu_routine_ld_ptr16(cpu_registers.a, cpu_registers.hl); // Load 8-bit value into A from 16-bit memory address in HL
}

void cpu_ld_hl_b() // 0x70
{
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.b); // Load 8-bit value into register B from 16-bit memory address in HL
}

void cpu_ld_hl_c() // 0x71
{
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.c); // Load 8-bit value into register C from 16-bit memory address in HL
}

void cpu_ld_hl_d() // 0x72
{
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.d); // Load 8-bit value into register D from 16-bit memory address in HL
}

void cpu_ld_hl_e() // 0x73
{
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.e); // Load 8-bit value into register E from 16-bit memory address in HL
}

void cpu_ld_hl_h() // 0x74
{
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.h); // Load 8-bit value into register H from 16-bit memory address in HL
}

void cpu_ld_hl_l() // 0x75
{
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.l); // Load 8-bit value into register L from 16-bit memory address in HL
}

void cpu_ld_hl_a() // 0x77
{
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.a); // Load 8-bit value into register A from 16-bit memory address in HL
}

void cpu_add_a_b() // 0x80
{
	cpu_routine_add_a_8(cpu_registers.b); // Add B register to A register
}

void cpu_add_a_c() // 0x81
{
	cpu_routine_add_a_8(cpu_registers.c); // Add C register to A register
}

void cpu_add_a_d() // 0x82
{
	cpu_routine_add_a_8(cpu_registers.d); // Add D register to A register
}

void cpu_add_a_e() // 0x83
{
	cpu_routine_add_a_8(cpu_registers.e); // Add E register to A register
}

void cpu_add_a_h() // 0x84
{
	cpu_routine_add_a_8(cpu_registers.h); // Add H register to A register
}

void cpu_add_a_l() // 0x85
{
	cpu_routine_add_a_8(cpu_registers.l); // Add L register to A register
}

void cpu_adc_a_b() // 0x88
{
	cpu_routine_adc_a_8(cpu_registers.b); // Add B register and carry flag to A register
}

void cpu_adc_a_c() // 0x89
{
	cpu_routine_adc_a_8(cpu_registers.c); // Add C register and carry flag to A register
}

void cpu_adc_a_d() // 0x8A
{
	cpu_routine_adc_a_8(cpu_registers.d); // Add D register and carry flag to A register
}

void cpu_adc_a_e() // 0x8B
{
	cpu_routine_adc_a_8(cpu_registers.e); // Add E register and carry flag to A register
}

void cpu_adc_a_h() // 0x8C
{
	cpu_routine_adc_a_8(cpu_registers.h); // Add H register and carry flag to A register
}

void cpu_adc_a_l() // 0x8D
{
	cpu_routine_adc_a_8(cpu_registers.l); // Add L register and carry flag to A register
}

void cpu_sub_a_b() // 0x90
{
	cpu_routine_sub_a_8(cpu_registers.b); // Subtract B register from A register
}

void cpu_sub_a_c() // 0x91
{
	cpu_routine_sub_a_8(cpu_registers.c); // Subtract C register from A register
}

void cpu_sub_a_d() // 0x92
{
	cpu_routine_sub_a_8(cpu_registers.d); // Subtract D register from A register
}

void cpu_sub_a_e() // 0x93
{
	cpu_routine_sub_a_8(cpu_registers.e); // Subtract E register from A register
}

void cpu_sub_a_h() // 0x94
{
	cpu_routine_sub_a_8(cpu_registers.h); // Subtract H register from A register
}

void cpu_sub_a_l() // 0x95
{
	cpu_routine_sub_a_8(cpu_registers.l); // Subtract L register from A register
}

void cpu_sbc_a_b() // 0x98
{
	cpu_routine_sbc_a_8(cpu_registers.b); // Subtract B register and carry flag from A register
}

void cpu_sbc_a_c() // 0x99
{
	cpu_routine_sbc_a_8(cpu_registers.c); // Subtract C register and carry flag from A register
}

void cpu_sbc_a_d() // 0x9A
{
	cpu_routine_sbc_a_8(cpu_registers.d); // Subtract D register and carry flag from A register
}

void cpu_sbc_a_e() // 0x9B
{
	cpu_routine_sbc_a_8(cpu_registers.e); // Subtract E register and carry flag from A register
}

void cpu_sbc_a_h() // 0x9C
{
	cpu_routine_sbc_a_8(cpu_registers.h); // Subtract H register and carry flag from A register
}

void cpu_sbc_a_l() // 0x9D
{
	cpu_routine_sbc_a_8(cpu_registers.l); // Subtract L register and carry flag from A register
}

void cpu_and_a_b() // 0xA0
{
	cpu_routine_and_a_8(cpu_registers.b); // AND B register with A register
}

void cpu_and_a_c() // 0xA1
{
	cpu_routine_and_a_8(cpu_registers.c); // AND C register with A register
}

void cpu_and_a_d() // 0xA2
{
	cpu_routine_and_a_8(cpu_registers.d); // AND D register with A register
}

void cpu_and_a_e() // 0xA3
{
	cpu_routine_and_a_8(cpu_registers.e); // AND E register with A register
}

void cpu_and_a_h() // 0xA4
{
	cpu_routine_and_a_8(cpu_registers.h); // AND H register with A register
}

void cpu_and_a_l() // 0xA5
{
	cpu_routine_and_a_8(cpu_registers.l); // AND L register with A register
}

void cpu_xor_a() // 0xAF
{
	cpu_registers.a = 0; // XOR A with itself results in 0
	SET_FLAG_SUBTRACT(0); // Clear the subtract flag
	SET_FLAG_HALF_CARRY(0); // Clear the half-carry flag
	SET_FLAG_CARRY(0); // Clear the carry flag
	SET_FLAG_ZERO(1); // Set the zero flag since the result is 0
	core_advance_cpu_clocks(4); // XOR A takes 4 clock cycles
}

void cpu_jp_nn() // 0xC3
{
	core_advance_cpu_clocks(4); // JP nn takes 16 clock cycles, but we jump 4 per action instead of all at once
	uint32_t temp = memory_bus_read(cpu_registers.pc++); // Read low byte of address
	cpu_registers.pc &= 0xFFFF; // Ensure PC is 16 bits

	core_advance_cpu_clocks(4);
	temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8; // Read high byte of address
	cpu_registers.pc &= 0xFFFF; // Ensure PC is 16 bits

	core_advance_cpu_clocks(4);
	cpu_registers.pc = temp; // Jump to the address
	core_advance_cpu_clocks(4);
}
