#include "cpu.h"
#include "cpu_instructions.h"
#include "cpu_routines.h"
#include "memory_bus.h"
#include "emulator_core.h"
#include "timer.h"
#include "interrupts.h"
#include "debug_log.h"

gb_cpu_registers cpu_registers; // Global instance of CPU registers
uint8_t cpu_current_op_code = 0; // Current operation code being executed
uint32_t cpu_instruction_counter = 0; // Counter for the number of instructions executed
void* cpu_current_instruction_execute = nullptr; // Pointer to the current instruction's execute function
uint8_t cpu_halt_count = 0; // 0 == not halted, 1 == halt instruction, 2 == stop instruction
bool cpu_halt_bug = false;
bool cpu_debug_instructions = true;
bool ime_delay_occured = false; // Added to test IME set bug, used in conditional below.

void cpu_reset() {
	// AFter executing boot rom, registers should be set to the following values:
	cpu_registers.af = 0x01B0;
	cpu_registers.bc = 0x0013;
	cpu_registers.de = 0x00D8;
	cpu_registers.hl = 0x014D;
	cpu_registers.sp = 0xFFFE;
	cpu_registers.pc = 0x0100; // Start executing at the beginning of the cartridge
}

void cpu_tick() {
	if (cpu_halt_count == 0) {
		cpu_fetch();
		cpu_execute();
		cpu_instruction_counter++;
	} else {
		core_advance_cpu_clocks(4); // Halted CPU still advances clock cycles until an interrupt occurs or it is resumed
	}

	if (cpu_instruction_counter > 10000 && cpu_debug_instructions) {
		cpu_debug_instructions = false;
		debug_log_close_file();
	}

	interrupt_service_routine();
}

void cpu_fetch() {

	if (cpu_debug_instructions) {
		cpu_dump_registers(cpu_registers);
	}

	cpu_current_op_code = memory_bus_read(cpu_registers.pc++);
	const bool is_extended_cb_instruction = cpu_current_op_code == 0xCB;

	if (cpu_halt_bug) {
		cpu_registers.pc--; // Apply the halt bug by decrementing the program counter
		cpu_halt_bug = false;
	}
	
	if (is_extended_cb_instruction) {
		core_advance_cpu_clocks(4);
		const uint8_t cpu_current_op_code_cb = memory_bus_read(cpu_registers.pc++);
		const gb_cpu_pre_cb_instruction& cb_instruction = cb_instructions[cpu_current_op_code_cb];
		cpu_current_instruction_execute = cb_instruction.execute;
	} else {
		const gb_cpu_instruction& instruction = instructions[cpu_current_op_code];
		cpu_current_instruction_execute = instruction.execute;
	}
}

bool cpu_execute() {
	if (!cpu_current_instruction_execute) {
		const gb_cpu_instruction& instruction = instructions[cpu_current_op_code];
		const uint8_t pchi = ((cpu_registers.pc - 1) & 0xFF00) >> 8;
		const uint8_t pclo = ((cpu_registers.pc - 1) & 0xFF);
		debug_log("Unknown instruction %.2X at: %.2X%.2X (%s), count %i\n", cpu_current_op_code, pchi, pclo, instruction.dissassembly, cpu_instruction_counter);
		return false;
	}
	// This actually executes the instruction
	((cpu_execute_op)cpu_current_instruction_execute)();
	return true;
}

void cpu_dump_registers(const gb_cpu_registers& registers) {
	const uint8_t op_code = memory_bus_read(registers.pc);
	const gb_cpu_instruction& instruction = instructions[op_code];
	const uint8_t pc_high = (registers.pc & 0xFF00) >> 8;
	const uint8_t pc_low = (registers.pc & 0xFF);
	const uint8_t sp_high = (registers.sp & 0xFF00) >> 8;
	const uint8_t sp_low = (registers.sp & 0xFF);

	if (instruction.operand_length == 0) {
		debug_log("AF: %.2X%.2X  BC: %.2X%.2X  DE: %.2X%.2X  HL: %.2X%.2X  SP: %.2X%.2X  PC: %.2X%.2X %s\n",
			registers.a, registers.f, registers.b, registers.c, registers.d, registers.e, registers.h, registers.l, sp_high, sp_low, pc_high, pc_low, instruction.dissassembly);
	}
	else if (instruction.operand_length == 1) {
		const uint8_t operand = memory_bus_read(registers.pc + 1);
		debug_log("AF: %.2X%.2X  BC: %.2X%.2X  DE: %.2X%.2X  HL: %.2X%.2X  SP: %.2X%.2X  PC: %.2X%.2X %s\n",
			registers.a, registers.f, registers.b, registers.c, registers.d, registers.e, registers.h, registers.l, sp_high, sp_low, pc_high, pc_low, instruction.dissassembly);
	}
	else if (instruction.operand_length == 2) {
		const uint8_t op_low = memory_bus_read(registers.pc + 1);
		const uint8_t op_high = memory_bus_read(registers.pc + 2);
		debug_log("AF: %.2X%.2X  BC: %.2X%.2X  DE: %.2X%.2X  HL: %.2X%.2X  SP: %.2X%.2X  PC: %.2X%.2X %s\n",
			registers.a, registers.f, registers.b, registers.c, registers.d, registers.e, registers.h, registers.l, sp_high, sp_low, pc_high, pc_low, instruction.dissassembly);
	}
}

void cpu_noop() { // 0x00
	core_advance_cpu_clocks(4); // NOP takes 4 clock cycles
}

void cpu_ld_bc_nn() { // 0x01
	cpu_routine_ld_16(cpu_registers.b, cpu_registers.c); // Load 16-bit immediate value into BC register pair
}

void cpu_ld_bc_a() { // 0x02
	cpu_routine_ld_ptr8(cpu_registers.bc, cpu_registers.a); // Load 8-bit value into a from 16-bit memory address in BC
}

void cpu_inc_bc() { // 0x03
	cpu_routine_inc_16(cpu_registers.bc); // Increment BC register pair
}

void cpu_inc_b() { // 0x04
	cpu_routine_inc_8(cpu_registers.b); // Increment B register
}

void cpu_dec_b() { // 0x05
	cpu_routine_dec_8(cpu_registers.b); // Decrement B register
}

void cpu_ld_b_n() { // 0x06
	cpu_routine_ld_8(cpu_registers.b); // Load 8-bit immediate value into B register
}

void cpu_rlca() {
	core_advance_cpu_clocks(4); // RLC A takes 4 clock cycles
	SET_FLAG_ZERO(0);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY((cpu_registers.a & 0x80) != 0); 
	cpu_registers.a = (cpu_registers.a << 1) | GET_FLAG_CARRY;
}

void cpu_ld_nn_sp() { // 0x08
	core_advance_cpu_clocks(4);
	uint16_t temp = memory_bus_read(cpu_registers.pc++);
	core_advance_cpu_clocks(4);
	temp |= ((uint16_t)memory_bus_read(cpu_registers.pc++)) << 8;
	core_advance_cpu_clocks(4);
	memory_bus_write(temp++, (cpu_registers.sp & 0xFF));
	core_advance_cpu_clocks(4);
	memory_bus_write(temp++, (cpu_registers.sp & 0xFF00) >> 8);
	core_advance_cpu_clocks(4);
}

void cpu_add_hl_bc() { // 0x09
	cpu_routine_add_hl_16(cpu_registers.bc); // Add BC register pair to HL register pair
}

void cpu_ld_a_bc() { // 0x0A
	cpu_routine_ld_ptr16(cpu_registers.a, cpu_registers.bc); // Load 8-bit value into A from 16-bit memory address in BC
}

void cpu_dec_bc() { // 0x0B
	cpu_routine_dec_16(cpu_registers.bc); // Decrement BC register pair
}

void cpu_inc_c() { // 0x0C
	cpu_routine_inc_8(cpu_registers.c); // Increment C register
}

void cpu_dec_c() { // 0x0D
	cpu_routine_dec_8(cpu_registers.c); // Decrement C register
}

void cpu_ld_c_n() { // 0x0E
	cpu_routine_ld_8(cpu_registers.c); // Load 8-bit immediate value into C register
}

void cpu_rrca() { // 0x0F
	core_advance_cpu_clocks(4); // RRC A takes 4 clock cycles
	SET_FLAG_ZERO(0);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY((cpu_registers.a & 0x01) != 0); 
	cpu_registers.a = (cpu_registers.a >> 1) | (GET_FLAG_CARRY << 7);
}

void cpu_stop() { // 0x10
	core_advance_cpu_clocks(4); 
	if (memory_bus_read(cpu_registers.pc++) != 0) {
		debug_log("CPU - Corrupted STOP at PC: %04X, should have operand 0x00\n",cpu_registers.pc);
	}
	core_advance_cpu_clocks(4);
	timer_on_div_write(0);
	cpu_halt_count = 2;
}
void cpu_ld_de_nn() { // 0x11
	cpu_routine_ld_16(cpu_registers.d, cpu_registers.e); // Load 16-bit immediate value into DE register pair
}

void cpu_ld_de_a() { // 0x12
	cpu_routine_ld_ptr8(cpu_registers.de, cpu_registers.a); // Load 8-bit value into register a from 16-bit memory address in DE
}

void cpu_inc_de() { // 0x13
	cpu_routine_inc_16(cpu_registers.de); // Increment DE register pair
}

void cpu_inc_d() { // 0x14
	cpu_routine_inc_8(cpu_registers.d); // Increment D register
}

void cpu_dec_d() { // 0x15
	cpu_routine_dec_8(cpu_registers.d); // Decrement D register
}

void cpu_ld_d_n() { // 0x16
	cpu_routine_ld_8(cpu_registers.d); // Load 8-bit immediate value into D register
}

void cpu_rla() { // 0x17
	core_advance_cpu_clocks(4); // RLA takes 4 clock cycles
	SET_FLAG_ZERO(0);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	uint8_t temp = GET_FLAG_CARRY;
	SET_FLAG_CARRY((cpu_registers.a & 0x80) != 0);
	cpu_registers.a = (cpu_registers.a << 1) | temp;
}

void cpu_jr_n() { // 0x18
	core_advance_cpu_clocks(4);
	uint8_t temp = memory_bus_read(cpu_registers.pc++);
	core_advance_cpu_clocks(4);
	cpu_registers.pc = (cpu_registers.pc + (int8_t)temp) & 0xFFFF;
	core_advance_cpu_clocks(4);
}

void cpu_add_hl_de() { // 0x19
	cpu_routine_add_hl_16(cpu_registers.de); // Add DE register pair to HL register pair
}

void cpu_ld_a_de() { // 0x1A
	cpu_routine_ld_ptr16(cpu_registers.a, cpu_registers.de); // Load 8-bit value into A from 16-bit memory address in DE
}

void cpu_dec_de() { // 0x1B
	cpu_routine_dec_16(cpu_registers.de); // Decrement DE register pair
}

void cpu_inc_e() { // 0x1C
	cpu_routine_inc_8(cpu_registers.e); // Increment E register
}

void cpu_dec_e() { // 0x1D
	cpu_routine_dec_8(cpu_registers.e); // Decrement E register
}

void cpu_ld_e_n() { // 0x1E
	cpu_routine_ld_8(cpu_registers.e); // Load 8-bit immediate value into E register
}

void cpu_rra() { // 0x1F
	core_advance_cpu_clocks(4); // RRA takes 4 clock cycles
	SET_FLAG_ZERO(0);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	uint8_t temp = GET_FLAG_CARRY;
	SET_FLAG_CARRY((cpu_registers.a & 0x01));
	cpu_registers.a = (cpu_registers.a >> 1) | (temp << 7);
}

void cpu_jr_nz_n() { // 0x20
	// OLD (kept for reference): cpu_routine_jr_conditional_n(GET_FLAG_ZERO != 0); // Jump relative if NZ flag is set
	cpu_routine_jr_conditional_n(GET_FLAG_ZERO == 0); // Jump relative if NZ flag is set
}

void cpu_ld_hl_nn() { // 0x21
	cpu_routine_ld_16(cpu_registers.h, cpu_registers.l); // Load 16-bit immediate value into HL register pair
}

void cpu_ldi_hl_a() { // 0x22
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, cpu_registers.a);
	core_advance_cpu_clocks(4);
	cpu_registers.hl = (cpu_registers.hl + 1) & 0xFFFF; // Increment HL, ensure it's 16 bits
}

void cpu_inc_hl() { // 0x23
	cpu_routine_inc_16(cpu_registers.hl); // Increment HL register pair
}

void cpu_inc_h() { // 0x24
	cpu_routine_inc_8(cpu_registers.h); // Increment H register
}

void cpu_dec_h() { // 0x25
	cpu_routine_dec_8(cpu_registers.h); // Decrement H register
}

void cpu_ld_h_n() { // 0x26
	cpu_routine_ld_8(cpu_registers.h); // Load 8-bit immediate value into H register
}

void cpu_daa() { // 0x27
	core_advance_cpu_clocks(4);
	if (!GET_FLAG_SUBTRACT) {
		// after an addition, adjust if (half-)carry occured or if result is out of bounds
		if (GET_FLAG_CARRY || cpu_registers.a > 0x99) {
			cpu_registers.a += 0x60;
			SET_FLAG_CARRY(1);
		}
		if (GET_FLAG_HALF_CARRY || (cpu_registers.a & 0x0F) > 0x09) {
			cpu_registers.a += 0x6;
		}
	} else {
		// after a subtraction, adjust if (half-)carry occured
		if (GET_FLAG_CARRY) {
			cpu_registers.a -= 0x60;
		}
		if (GET_FLAG_HALF_CARRY) {
			cpu_registers.a -= 0x6;
		}
	}
	SET_FLAG_ZERO(cpu_registers.a == 0);
	SET_FLAG_HALF_CARRY(0);
}

void cpu_jr_z_n() { // 0x28
	// Might be bugged, leaving it as a learning experience if it is
	// OLD (kept for reference): cpu_routine_jr_conditional_n(GET_FLAG_ZERO == 0); // Jump relative if Z flag is set
	cpu_routine_jr_conditional_n(GET_FLAG_ZERO != 0); // Jump relative if Z flag is set
}

void cpu_add_hl_hl() { // 0x29
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_CARRY((cpu_registers.hl & 0x8000) != 0); // Set carry flag if the addition results in a carry from bit 15
	SET_FLAG_HALF_CARRY((cpu_registers.hl & 0x0800) != 0); // Set half-carry flag if the addition results in a carry from bit 11
	core_advance_cpu_clocks(4);
	cpu_registers.hl = (cpu_registers.hl << 1) & 0xFFFF; // Perform the addition (HL + HL) and ensure it's 16 bits
}

void cpu_ldi_a_hl() { // 0x2A
	core_advance_cpu_clocks(4);
	cpu_registers.a = memory_bus_read(cpu_registers.hl); // Load value from memory at HL into A register
	core_advance_cpu_clocks(4);
	cpu_registers.hl = (cpu_registers.hl + 1) & 0xFFFF; // Increment HL, ensure it's 16 bits
}

void cpu_dec_hl() { // 0x2B
	cpu_routine_dec_16(cpu_registers.hl); // Decrement HL register pair
}

void cpu_inc_l() { // 0x2C
	cpu_routine_inc_8(cpu_registers.l); // Increment L register
}

void cpu_dec_l() { // 0x2D
	cpu_routine_dec_8(cpu_registers.l); // Decrement L register
}

void cpu_ld_l_n() { // 0x2E
	cpu_routine_ld_8(cpu_registers.l); // Load 8-bit immediate value into L register
}

void cpu_cpl() { // 0x2F
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(1);
	SET_FLAG_HALF_CARRY(1);
	cpu_registers.a = ~cpu_registers.a; // Complement the A register (bitwise NOT)
}

void cpu_jr_nc_n() { // 0x30
	cpu_routine_jr_conditional_n(GET_FLAG_CARRY == 0); // Jump relative if NC flag is set
}

void cpu_ld_sp_nn() { // 0x31
	cpu_routine_ld_16(cpu_registers.s, cpu_registers.p); // Load 16-bit immediate value into SP register
}

void cpu_ldd_hl_a() { // 0x32
	core_advance_cpu_clocks(4); // LD (HL-), A takes 8 clock cycles, but we jump 4 per action instead of all at once
	memory_bus_write(cpu_registers.hl, cpu_registers.a); // Write A to memory at HL
	cpu_registers.hl = (cpu_registers.hl - 1) & 0xFFFF; // Decrement HL, ensure it's 16 bits
	core_advance_cpu_clocks(4);
}

void cpu_inc_sp() { // 0x33
	cpu_routine_inc_16(cpu_registers.sp); // Increment SP register
}

void cpu_inc__hl() { // 0x34
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl); // Read the value from memory at HL into a temporary variable
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY((temp & 0xF) == 0xF);
	temp = (temp + 1) & 0xFF;
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp); // Write the incremented value back to memory at HL
}

void cpu_dec__hl() { // 0x35
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl); // Read the value from memory at HL into a temporary variable
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(1);
	SET_FLAG_HALF_CARRY((temp & 0xF) == 0);
	temp = (temp - 1) & 0xFF;
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp); // Write the incremented value back to memory at HL
}

void cpu_ld_hl_n() { // 0x36
	core_advance_cpu_clocks(4);
	uint8_t temp = memory_bus_read(cpu_registers.pc++);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
	core_advance_cpu_clocks(4);
}

void cpu_scf() { // 0x37
	SET_FLAG_CARRY(1);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_SUBTRACT(0);
	core_advance_cpu_clocks(4);
}

void cpu_jr_c_n() { // 0x38
	cpu_routine_jr_conditional_n(GET_FLAG_CARRY != 0); // Jump relative if C flag is set
}

void cpu_add_hl_sp() { // 0x39
	cpu_routine_add_hl_16(cpu_registers.sp); // Add SP register to HL register pair
}

void cpu_ldd_a_hl() { // 0x3A
	core_advance_cpu_clocks(4);
	cpu_registers.a = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	cpu_registers.hl = (cpu_registers.hl - 1) & 0xFFFF; // Decrement HL, ensure it's 16 bits
}

void cpu_dec_sp() { // 0x3B
	cpu_routine_dec_16(cpu_registers.sp); // Decrement SP register
}

void cpu_inc_a() { // 0x3C
	cpu_routine_inc_8(cpu_registers.a); // Increment A register
}

void cpu_dec_a() { // 0x3D
	cpu_routine_dec_8(cpu_registers.a); // Decrement A register
}

void cpu_ld_a_n() { // 0x3E
	cpu_routine_ld_8(cpu_registers.a); // Load 8-bit immediate value into A register
}

void cpu_ccf() { // 0x3F
	core_advance_cpu_clocks(4);
	SET_FLAG_CARRY(!GET_FLAG_CARRY);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_SUBTRACT(0);
}

void cpu_ld_b_b() { // 0x40
	core_advance_cpu_clocks(4); // No action, but wait 4 clock cycles
}

void cpu_ld_b_c() { // 0x41
	core_advance_cpu_clocks(4);
	cpu_registers.b = cpu_registers.c; // Load value from C into B
}

void cpu_ld_b_d() { // 0x42
	core_advance_cpu_clocks(4);
	cpu_registers.b = cpu_registers.d; // Load value from D into B
}

void cpu_ld_b_e() { // 0x43
	core_advance_cpu_clocks(4);
	cpu_registers.b = cpu_registers.e; // Load value from E into B
}

void cpu_ld_b_h() { // 0x44
	core_advance_cpu_clocks(4);
	cpu_registers.b = cpu_registers.h; // Load value from H into B
}

void cpu_ld_b_l() { // 0x45
	core_advance_cpu_clocks(4);
	cpu_registers.b = cpu_registers.l; // Load value from L into B
}

void cpu_ld_b_hl() { // 0x46
	cpu_routine_ld_ptr16(cpu_registers.b, cpu_registers.hl); // Load 8-bit value into B from 16-bit memory address in HL
}

void cpu_ld_b_a() { // 0x47
	core_advance_cpu_clocks(4);
	cpu_registers.b = cpu_registers.a; // Load value from A into B
}

void cpu_ld_c_b() { // 0x48
	core_advance_cpu_clocks(4);
	cpu_registers.c = cpu_registers.b; // Load value from B into C
}

void cpu_ld_c_c() { // 0x49
	core_advance_cpu_clocks(4); // No action, but wait 4 clock cycles
}

void cpu_ld_c_d() { // 0x4A
	core_advance_cpu_clocks(4);
	cpu_registers.c = cpu_registers.d; // Load value from D into C
}

void cpu_ld_c_e() { // 0x4B
	core_advance_cpu_clocks(4);
	cpu_registers.c = cpu_registers.e; // Load value from E into C
}

void cpu_ld_c_h() { // 0x4C
	core_advance_cpu_clocks(4);
	cpu_registers.c = cpu_registers.h; // Load value from H into C
}

void cpu_ld_c_l() { // 0x4D
	core_advance_cpu_clocks(4);
	cpu_registers.c = cpu_registers.l; // Load value from L into C
}

void cpu_ld_c_hl() { // 0x4E
	cpu_routine_ld_ptr16(cpu_registers.c, cpu_registers.hl); // Load 8-bit value into C from 16-bit memory address in HL
}

void cpu_ld_c_a() { // 0x4F
	core_advance_cpu_clocks(4);
	cpu_registers.c = cpu_registers.a; // Load value from A into C
}

void cpu_ld_d_b() { // 0x50
	core_advance_cpu_clocks(4);
	cpu_registers.d = cpu_registers.b; // Load value from B into D
}

void cpu_ld_d_c() { // 0x51
	core_advance_cpu_clocks(4);
	cpu_registers.d = cpu_registers.c; // Load value from C into D
}

void cpu_ld_d_d() { // 0x52
	core_advance_cpu_clocks(4); // No action, but wait 4 clock cycles
}

void cpu_ld_d_e() { // 0x53
	core_advance_cpu_clocks(4);
	cpu_registers.d = cpu_registers.e; // Load value from E into D
}

void cpu_ld_d_h() { // 0x54
	core_advance_cpu_clocks(4);
	cpu_registers.d = cpu_registers.h; // Load value from H into D
}

void cpu_ld_d_l() { // 0x55
	core_advance_cpu_clocks(4);
	cpu_registers.d = cpu_registers.l; // Load value from L into D
}

void cpu_ld_d_hl() { // 0x56
	cpu_routine_ld_ptr16(cpu_registers.d, cpu_registers.hl); // Load 8-bit value into D from 16-bit memory address in HL
}

void cpu_ld_d_a() { // 0x57
	core_advance_cpu_clocks(4);
	cpu_registers.d = cpu_registers.a; // Load value from A into D
}

void cpu_ld_e_b() { // 0x58
	core_advance_cpu_clocks(4);
	cpu_registers.e = cpu_registers.b; // Load value from B into E
}

void cpu_ld_e_c() { // 0x59
	core_advance_cpu_clocks(4);
	cpu_registers.e = cpu_registers.c; // Load value from C into E
}

void cpu_ld_e_d() { // 0x5A
	core_advance_cpu_clocks(4);
	cpu_registers.e = cpu_registers.d; // Load value from D into E
}

void cpu_ld_e_e() { // 0x5B
	core_advance_cpu_clocks(4); // No action, but wait 4 clock cycles
}

void cpu_ld_e_h() { // 0x5C
	core_advance_cpu_clocks(4);
	cpu_registers.e = cpu_registers.h; // Load value from H into E
}

void cpu_ld_e_l() { // 0x5D
	core_advance_cpu_clocks(4);
	cpu_registers.e = cpu_registers.l; // Load value from L into E
}

void cpu_ld_e_hl() { // 0x5E
	cpu_routine_ld_ptr16(cpu_registers.e, cpu_registers.hl); // Load 8-bit value into E from 16-bit memory address in HL
}

void cpu_ld_e_a() { // 0x5F
	core_advance_cpu_clocks(4);
	cpu_registers.e = cpu_registers.a; // Load value from A into E
}

void cpu_ld_h_b() { // 0x60
	core_advance_cpu_clocks(4);
	cpu_registers.h = cpu_registers.b; // Load value from B into H
}

void cpu_ld_h_c() { // 0x61
	core_advance_cpu_clocks(4);
	cpu_registers.h = cpu_registers.c; // Load value from C into H
}

void cpu_ld_h_d() { // 0x62
	core_advance_cpu_clocks(4);
	cpu_registers.h = cpu_registers.d; // Load value from D into H
}

void cpu_ld_h_e() { // 0x63
	core_advance_cpu_clocks(4);
	cpu_registers.h = cpu_registers.e; // Load value from E into H
}

void cpu_ld_h_h() { // 0x64
	core_advance_cpu_clocks(4); // No action, but wait 4 clock cycles
}

void cpu_ld_h_l() { // 0x65
	core_advance_cpu_clocks(4);
	cpu_registers.h = cpu_registers.l; // Load value from L into H
}

void cpu_ld_h_hl() { // 0x66
	cpu_routine_ld_ptr16(cpu_registers.h, cpu_registers.hl); // Load 8-bit value into H from 16-bit memory address in HL
}

void cpu_ld_h_a() { // 0x67
	core_advance_cpu_clocks(4);
	cpu_registers.h = cpu_registers.a; // Load value from A into H
}

void cpu_ld_l_b() { // 0x68
	core_advance_cpu_clocks(4);
	cpu_registers.l = cpu_registers.b; // Load value from B into L
}

void cpu_ld_l_c() { // 0x69
	core_advance_cpu_clocks(4);
	cpu_registers.l = cpu_registers.c; // Load value from C into L
}

void cpu_ld_l_d() { // 0x6A
	core_advance_cpu_clocks(4);
	cpu_registers.l = cpu_registers.d; // Load value from D into L
}

void cpu_ld_l_e() { // 0x6B
	core_advance_cpu_clocks(4);
	cpu_registers.l = cpu_registers.e; // Load value from E into L
}

void cpu_ld_l_h() { // 0x6C
	core_advance_cpu_clocks(4);
	cpu_registers.l = cpu_registers.h; // Load value from H into L
}

void cpu_ld_l_l() { // 0x6D
	core_advance_cpu_clocks(4); // No action, but wait 4 clock cycles
}

void cpu_ld_l_hl() { // 0x6E
	cpu_routine_ld_ptr16(cpu_registers.l, cpu_registers.hl); // Load 8-bit value into L from 16-bit memory address in HL
}

void cpu_ld_l_a() { // 0x6F
	core_advance_cpu_clocks(4);
	cpu_registers.l = cpu_registers.a; // Load value from A into L
}

void cpu_ld_hl_b() { // 0x70
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.b); // Load 8-bit value into register B from 16-bit memory address in HL
}

void cpu_ld_hl_c() { // 0x71
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.c); // Load 8-bit value into register C from 16-bit memory address in HL
}

void cpu_ld_hl_d() { // 0x72
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.d); // Load 8-bit value into register D from 16-bit memory address in HL
}

void cpu_ld_hl_e() { // 0x73
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.e); // Load 8-bit value into register E from 16-bit memory address in HL
}

void cpu_ld_hl_h() { // 0x74
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.h); // Load 8-bit value into register H from 16-bit memory address in HL
}

void cpu_ld_hl_l() { // 0x75
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.l); // Load 8-bit value into register L from 16-bit memory address in HL
}

void cpu_halt() { // 0x76
	core_advance_cpu_clocks(4);
	const uint8_t interrupt_enable = memory_bus_read(ADDR_IO_IE);
	const uint8_t interrupt_flag = memory_bus_read(ADDR_IO_IF);
	const bool interrupt_pending = ((interrupt_enable & interrupt_flag) & 0x1F) != 0;
	if (!interrupt_master_enable && interrupt_pending != 0) {
		cpu_halt_bug = true; // Set the halt bug flag if CPU is halted with pending interrupts and IME is disabled
	} else {
		cpu_halt_count = 1; // Set halt count to indicate CPU is halted
	}
}

void cpu_ld_hl_a() { // 0x77
	cpu_routine_ld_ptr8(cpu_registers.hl, cpu_registers.a); // Load 8-bit value into register A from 16-bit memory address in HL
}

void cpu_ld_a_b() { // 0x78
	core_advance_cpu_clocks(4);
	cpu_registers.a = cpu_registers.b; // Load value from B into A
}

void cpu_ld_a_c() { // 0x79
	core_advance_cpu_clocks(4);
	cpu_registers.a = cpu_registers.c; // Load value from C into A
}

void cpu_ld_a_d() { // 0x7A
	core_advance_cpu_clocks(4);
	cpu_registers.a = cpu_registers.d; // Load value from D into A
}

void cpu_ld_a_e() { // 0x7B
	core_advance_cpu_clocks(4);
	cpu_registers.a = cpu_registers.e; // Load value from E into A
}

void cpu_ld_a_h() { // 0x7C
	core_advance_cpu_clocks(4);
	cpu_registers.a = cpu_registers.h; // Load value from H into A
}

void cpu_ld_a_l() { // 0x7D
	core_advance_cpu_clocks(4);
	cpu_registers.a = cpu_registers.l; // Load value from L into A
}

void cpu_ld_a_hl() { // 0x7E
	cpu_routine_ld_ptr16(cpu_registers.a, cpu_registers.hl); // Load 8-bit value into A from 16-bit memory address in HL
}

void cpu_ld_a_a() { // 0x7F
	core_advance_cpu_clocks(4); // No action, but wait 4 clock cycles
}

void cpu_add_a_b() { // 0x80
	cpu_routine_add_a_8(cpu_registers.b); // Add B register to A register
}

void cpu_add_a_c() { // 0x81
	cpu_routine_add_a_8(cpu_registers.c); // Add C register to A register
}

void cpu_add_a_d() { // 0x82
	cpu_routine_add_a_8(cpu_registers.d); // Add D register to A register
}

void cpu_add_a_e() { // 0x83
	cpu_routine_add_a_8(cpu_registers.e); // Add E register to A register
}

void cpu_add_a_h() { // 0x84
	cpu_routine_add_a_8(cpu_registers.h); // Add H register to A register
}

void cpu_add_a_l() { // 0x85
	cpu_routine_add_a_8(cpu_registers.l); // Add L register to A register
}

void cpu_add_a_hl() { // 0x86
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	uint32_t temp_a = cpu_registers.a;
	uint32_t temp_hl = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_HALF_CARRY(((temp_a & 0xF) + (temp_hl & 0xF)) > 0xF);
	cpu_registers.a += temp_hl;
	SET_FLAG_ZERO(cpu_registers.a == 0);
	SET_FLAG_CARRY(temp_a > cpu_registers.a);
}

void cpu_add_a_a() { // 0x87
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY((cpu_registers.a & BIT(3)) != 0);
	SET_FLAG_CARRY((cpu_registers.a & BIT(7)) != 0);
	cpu_registers.a += cpu_registers.a;
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_adc_a_b() { // 0x88
	cpu_routine_adc_a_8(cpu_registers.b); // Add B register and carry flag to A register
}

void cpu_adc_a_c() { // 0x89
	cpu_routine_adc_a_8(cpu_registers.c); // Add C register and carry flag to A register
}

void cpu_adc_a_d() { // 0x8A
	cpu_routine_adc_a_8(cpu_registers.d); // Add D register and carry flag to A register
}

void cpu_adc_a_e() { // 0x8B
	cpu_routine_adc_a_8(cpu_registers.e); // Add E register and carry flag to A register
}

void cpu_adc_a_h() { // 0x8C
	cpu_routine_adc_a_8(cpu_registers.h); // Add H register and carry flag to A register
}

void cpu_adc_a_l() { // 0x8D
	cpu_routine_adc_a_8(cpu_registers.l); // Add L register and carry flag to A register
}

void cpu_adc_a_hl() { // 0x8E
	core_advance_cpu_clocks(4);
	uint32_t temp_hl = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	uint32_t temp_a = cpu_registers.a + temp_hl + GET_FLAG_CARRY;
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY((((cpu_registers.a & 0xF) + (temp_hl & 0xF)) + GET_FLAG_CARRY) > 0xF);
	SET_FLAG_CARRY(temp_a > 0xFF);
	temp_a &= 0xFF;
	cpu_registers.a = temp_a;
	SET_FLAG_ZERO(temp_a == 0);
}

void cpu_adc_a_a() { // 0x8F
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	uint32_t temp = (((uint32_t)cpu_registers.a) << 1) + GET_FLAG_CARRY;
	SET_FLAG_HALF_CARRY((cpu_registers.a & 0x08) != 0);
	SET_FLAG_CARRY(temp > 0xFF);
	temp &= 0xFF;
	cpu_registers.a = temp;
	SET_FLAG_ZERO(temp == 0);
}

void cpu_sub_a_b() { // 0x90
	cpu_routine_sub_a_8(cpu_registers.b); // Subtract B register from A register
}

void cpu_sub_a_c() { // 0x91
	cpu_routine_sub_a_8(cpu_registers.c); // Subtract C register from A register
}

void cpu_sub_a_d() { // 0x92
	cpu_routine_sub_a_8(cpu_registers.d); // Subtract D register from A register
}

void cpu_sub_a_e() { // 0x93
	cpu_routine_sub_a_8(cpu_registers.e); // Subtract E register from A register
}

void cpu_sub_a_h() { // 0x94
	cpu_routine_sub_a_8(cpu_registers.h); // Subtract H register from A register
}

void cpu_sub_a_l() { // 0x95
	cpu_routine_sub_a_8(cpu_registers.l); // Subtract L register from A register
}

void cpu_sub_a_hl() { // 0x96
	core_advance_cpu_clocks(4);
	uint8_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(1);
	SET_FLAG_HALF_CARRY((cpu_registers.a & 0xF) < (temp & 0xF));
	SET_FLAG_CARRY(cpu_registers.a < temp);
	cpu_registers.a -= temp;
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_sub_a_a() { // 0x97
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(1);
	SET_FLAG_ZERO(1);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY(0);
	cpu_registers.a = 0;
}

void cpu_sbc_a_b() { // 0x98
	cpu_routine_sbc_a_8(cpu_registers.b); // Subtract B register and carry flag from A register
}

void cpu_sbc_a_c() { // 0x99
	cpu_routine_sbc_a_8(cpu_registers.c); // Subtract C register and carry flag from A register
}

void cpu_sbc_a_d() { // 0x9A
	cpu_routine_sbc_a_8(cpu_registers.d); // Subtract D register and carry flag from A register
}

void cpu_sbc_a_e() { // 0x9B
	cpu_routine_sbc_a_8(cpu_registers.e); // Subtract E register and carry flag from A register
}

void cpu_sbc_a_h() { // 0x9C
	cpu_routine_sbc_a_8(cpu_registers.h); // Subtract H register and carry flag from A register
}

void cpu_sbc_a_l() { // 0x9D
	cpu_routine_sbc_a_8(cpu_registers.l); // Subtract L register and carry flag from A register
}

void cpu_sbc_a_hl() { // 0x9E
	core_advance_cpu_clocks(4);
	uint32_t temp_hl = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	uint32_t temp_a = cpu_registers.a - (temp_hl + GET_FLAG_CARRY);
	SET_FLAG_CARRY((temp_a & ~0xFF) != 0);
	SET_FLAG_ZERO((temp_a & 0xFF) == 0);
	SET_FLAG_SUBTRACT(1);
	SET_FLAG_HALF_CARRY(((cpu_registers.a ^ temp_hl ^ temp_a) & 0x10) != 0);
	cpu_registers.a = temp_a;
}

void cpu_sbc_a_a() { // 0x9F
	core_advance_cpu_clocks(4);
	if (GET_FLAG_CARRY) {
		cpu_registers.a = 0xFF;
		SET_FLAG_CARRY(1);
		SET_FLAG_HALF_CARRY(1);
		SET_FLAG_ZERO(0);
	} else {
		cpu_registers.a = 0;
		SET_FLAG_CARRY(0);
		SET_FLAG_HALF_CARRY(0);
		SET_FLAG_ZERO(1);
	}
	SET_FLAG_SUBTRACT(1);
}

void cpu_and_a_b() { // 0xA0
	cpu_routine_and_a_8(cpu_registers.b); // AND B register with A register
}

void cpu_and_a_c() { // 0xA1
	cpu_routine_and_a_8(cpu_registers.c); // AND C register with A register
}

void cpu_and_a_d() { // 0xA2
	cpu_routine_and_a_8(cpu_registers.d); // AND D register with A register
}

void cpu_and_a_e() { // 0xA3
	cpu_routine_and_a_8(cpu_registers.e); // AND E register with A register
}

void cpu_and_a_h() { // 0xA4
	cpu_routine_and_a_8(cpu_registers.h); // AND H register with A register
}

void cpu_and_a_l() { // 0xA5
	cpu_routine_and_a_8(cpu_registers.l); // AND L register with A register
}

void cpu_and_a_hl() { // 0xA6
	core_advance_cpu_clocks(4);
	SET_FLAG_HALF_CARRY(1);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_CARRY(0);
	core_advance_cpu_clocks(4);
	cpu_registers.a &= memory_bus_read(cpu_registers.hl);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_and_a_a() { // 0xA7
	core_advance_cpu_clocks(4);
	SET_FLAG_HALF_CARRY(1);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_CARRY(0);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_xor_a_b() { // 0xA8
	cpu_routine_xor_a_8(cpu_registers.b); // XOR B register with A register
}

void cpu_xor_a_c() { // 0xA9
	cpu_routine_xor_a_8(cpu_registers.c); // XOR C register with A register
}

void cpu_xor_a_d() { // 0xAA
	cpu_routine_xor_a_8(cpu_registers.d); // XOR D register with A register
}

void cpu_xor_a_e() { // 0xAB
	cpu_routine_xor_a_8(cpu_registers.e); // XOR E register with A register
}

void cpu_xor_a_h() { // 0xAC
	cpu_routine_xor_a_8(cpu_registers.h); // XOR H register with A register
}

void cpu_xor_a_l() { // 0xAD
	cpu_routine_xor_a_8(cpu_registers.l); // XOR L register with A register
}

void cpu_xor_a_hl() { // 0xAE
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_CARRY(0);
	SET_FLAG_HALF_CARRY(0);
	cpu_registers.a ^= memory_bus_read(cpu_registers.hl);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_xor_a() { // 0xAF
	cpu_registers.a = 0; // XOR A with itself results in 0
	SET_FLAG_SUBTRACT(0); // Clear the subtract flag
	SET_FLAG_HALF_CARRY(0); // Clear the half-carry flag
	SET_FLAG_CARRY(0); // Clear the carry flag
	SET_FLAG_ZERO(1); // Set the zero flag since the result is 0
	core_advance_cpu_clocks(4); // XOR A takes 4 clock cycles
}

void cpu_or_a_b() { // 0xB0
	cpu_routine_or_a_8(cpu_registers.b); // OR B register with A register
}

void cpu_or_a_c() { // 0xB1
	cpu_routine_or_a_8(cpu_registers.c); // OR C register with A register
}

void cpu_or_a_d() { // 0xB2
	cpu_routine_or_a_8(cpu_registers.d); // OR D register with A register
}

void cpu_or_a_e() { // 0xB3
	cpu_routine_or_a_8(cpu_registers.e); // OR E register with A register
}

void cpu_or_a_h() { // 0xB4
	cpu_routine_or_a_8(cpu_registers.h); // OR H register with A register
}

void cpu_or_a_l() { // 0xB5
	cpu_routine_or_a_8(cpu_registers.l); // OR L register with A register
}

void cpu_or_a_hl() { // 0xB6
	core_advance_cpu_clocks(4);
	SET_FLAG_CARRY(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_SUBTRACT(0);
	cpu_registers.a |= memory_bus_read(cpu_registers.hl);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_or_a_a() { // 0xB7
	core_advance_cpu_clocks(4);
	SET_FLAG_CARRY(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_cp_a_b() { // 0xB8
	cpu_routine_cp_a_8(cpu_registers.b); // Compare A register with B register
}

void cpu_cp_a_c() { // 0xB9
	cpu_routine_cp_a_8(cpu_registers.c); // Compare A register with C register
}

void cpu_cp_a_d() { // 0xBA
	cpu_routine_cp_a_8(cpu_registers.d); // Compare A register with D register
}

void cpu_cp_a_e() { // 0xBB
	cpu_routine_cp_a_8(cpu_registers.e); // Compare A register with E register
}

void cpu_cp_a_h() { // 0xBC
	cpu_routine_cp_a_8(cpu_registers.h); // Compare A register with H register
}

void cpu_cp_a_l() { // 0xBD
	cpu_routine_cp_a_8(cpu_registers.l); // Compare A register with L register
}

void cpu_cp_a_hl() { // 0xBE
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(1);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_HALF_CARRY((cpu_registers.a & 0xF) < (temp & 0xF));
	SET_FLAG_CARRY((uint32_t)cpu_registers.a < temp);
	SET_FLAG_ZERO(cpu_registers.a == temp);
}

void cpu_cp_a_a() { // 0xBF
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(1);
	SET_FLAG_ZERO(1);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY(0);
}

void cpu_ret_nz() { // 0xC0
	cpu_routine_ret_conditional(GET_FLAG_ZERO == 0); // Return from subroutine if zero flag not set
}

void cpu_pop_bc() { // 0xC1
	cpu_routine_pop_16(cpu_registers.b, cpu_registers.c); // Pop value from stack into BC register pair
}

void cpu_jp_nz_nn() { // 0xC2
	cpu_routine_jp_conditional_nnnn(GET_FLAG_ZERO == 0); // Jump to address nn if zero flag not set
}

void cpu_jp_nn() { // 0xC3
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

void cpu_call_nz_nn() { // 0xC4
	cpu_routine_call_conditional_nnnn(GET_FLAG_ZERO == 0); // Call subroutine at address nn if zero flag not set
}

void cpu_push_bc() { // 0xC5
	cpu_routine_push_16(cpu_registers.b, cpu_registers.c); // Push BC register pair onto the stack
}

void cpu_add_a_n() { // 0xC6
	core_advance_cpu_clocks(4);
	uint32_t temp_a = cpu_registers.a;
	uint32_t temp_op = memory_bus_read(cpu_registers.pc++); // Read immediate 8-bit operand
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(((temp_a & 0xF) + (temp_op & 0xF)) > 0xF);
	cpu_registers.a += temp_op;
	SET_FLAG_ZERO(cpu_registers.a == 0);
	SET_FLAG_CARRY(temp_a > cpu_registers.a);
}

void cpu_rst_00() { // 0xC7
	cpu_routine_rst_nnnn(0x0000); // Call subroutine at address 0x0000
}

void cpu_ret_z() { // 0xC8
	cpu_routine_ret_conditional(GET_FLAG_ZERO != 0); // Return from subroutine if zero flag is set
}

void cpu_ret() { // 0xC9
	core_advance_cpu_clocks(4);
	uint16_t temp = memory_bus_read(cpu_registers.sp++);
	temp |= ((uint16_t)memory_bus_read(cpu_registers.sp++)) << 8;
	core_advance_cpu_clocks(4);
	cpu_registers.pc = temp;
	core_advance_cpu_clocks(4);
}

void cpu_jp_z_nn() { // 0xCA
	cpu_routine_jp_conditional_nnnn(GET_FLAG_ZERO != 0); // Jump to address nn if zero flag is set
}

void cpu_prefix_cb() { // 0xCB
	
}

void cpu_call_z_nn() { // 0xCC
	cpu_routine_call_conditional_nnnn(GET_FLAG_ZERO != 0); // Call subroutine at address nn if zero flag is set
}

void cpu_call_nn() { // 0xCD
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.pc++);
	core_advance_cpu_clocks(4);
	temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8;
	core_advance_cpu_clocks(4);
	cpu_registers.sp--;
	core_advance_cpu_clocks(4);
	const uint8_t pc_high = (cpu_registers.pc & 0xFF00) >> 8;
	memory_bus_write(cpu_registers.sp, pc_high);
	core_advance_cpu_clocks(4);
	cpu_registers.sp--;
	core_advance_cpu_clocks(4);
	const uint8_t pc_low = (cpu_registers.pc & 0xFF);
	memory_bus_write(cpu_registers.sp, pc_low);
	cpu_registers.pc = temp;
}

void cpu_adc_a_n() { // 0xCE
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	uint32_t temp_op = memory_bus_read(cpu_registers.pc++);
	uint32_t temp_a = cpu_registers.a + temp_op + GET_FLAG_CARRY;
	SET_FLAG_HALF_CARRY((((cpu_registers.a & 0xF) + (temp_op & 0xF)) + GET_FLAG_CARRY) > 0xF);
	SET_FLAG_CARRY(temp_a > 0xFF);
	core_advance_cpu_clocks(4);
	cpu_registers.a = (temp_a & 0xFF);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_rst_08() { // 0xCF
	cpu_routine_rst_nnnn(0x0008); // Call subroutine at address 0x0008
}

void cpu_ret_nc() { // 0xD0
	cpu_routine_ret_conditional(GET_FLAG_CARRY == 0); // Return from subroutine if carry flag not set
}

void cpu_pop_de() { // 0xD1
	cpu_routine_pop_16(cpu_registers.d, cpu_registers.e); // Pop value from stack into DE register pair
}

void cpu_jp_nc_nn() { // 0xD2
	cpu_routine_jp_conditional_nnnn(GET_FLAG_CARRY == 0); // Jump to address nn if carry flag not set
}

void cpu_call_nc_nn() { // 0xD4
	cpu_routine_call_conditional_nnnn(GET_FLAG_CARRY == 0); // Call subroutine at address nn if carry flag not set
}

void cpu_push_de() { // 0xD5
	cpu_routine_push_16(cpu_registers.d, cpu_registers.e); // Push DE register pair onto the stack
}

void cpu_sub_a_n() { // 0xD6
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.pc++);
	SET_FLAG_SUBTRACT(1);
	SET_FLAG_HALF_CARRY((cpu_registers.a & 0xF) < (temp & 0xF));
	SET_FLAG_CARRY(cpu_registers.a < temp);
	core_advance_cpu_clocks(4);
	cpu_registers.a -= temp;
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_rst_10() { // 0xD7
	cpu_routine_rst_nnnn(0x0010); // Call subroutine at address 0x0010
}

void cpu_ret_c() { // 0xD8
	cpu_routine_ret_conditional(GET_FLAG_CARRY != 0); // Return from subroutine if carry flag is set
}

void cpu_reti() { // 0xD9
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.sp++);
	cpu_registers.sp &= 0xFFFF;
	core_advance_cpu_clocks(4);
	temp |= ((uint32_t)memory_bus_read(cpu_registers.sp++)) << 8;
	cpu_registers.sp &= 0xFFFF;
	core_advance_cpu_clocks(4);
	cpu_registers.pc = temp;
	core_advance_cpu_clocks(4);
	interrupt_master_enable = true;
}

void cpu_jp_c_nn() { // 0xDA
	cpu_routine_jp_conditional_nnnn(GET_FLAG_CARRY != 0); // Jump to address nn if carry flag is set
}

void cpu_call_c_nn() { // 0xDC
	cpu_routine_call_conditional_nnnn(GET_FLAG_CARRY != 0); // Call subroutine at address nn if carry flag is set
}

void cpu_sbc_a_n() { // 0xDE
	core_advance_cpu_clocks(4);
	uint16_t temp_op = memory_bus_read(cpu_registers.pc++);
	uint16_t temp_a = cpu_registers.a - (temp_op + GET_FLAG_CARRY);
	SET_FLAG_CARRY((temp_a & ~0xFF) != 0);
	SET_FLAG_ZERO((temp_a & 0xFF) == 0);
	SET_FLAG_SUBTRACT(1);
	SET_FLAG_HALF_CARRY(((cpu_registers.a ^ temp_op ^ temp_a) & 0x10) != 0);
	core_advance_cpu_clocks(4);
	cpu_registers.a = temp_a;
}

void cpu_rst_18() { // 0xDF
	cpu_routine_rst_nnnn(0x0018); // Call subroutine at address 0x0018
}

void cpu_ldh_n_a() { // 0xE0
	core_advance_cpu_clocks(4);
	uint32_t temp = 0xFF00 + (uint32_t)memory_bus_read(cpu_registers.pc++);
	core_advance_cpu_clocks(4);
	memory_bus_write(temp, cpu_registers.a);
	core_advance_cpu_clocks(4);
}

void cpu_pop_hl() { // 0xE1
	cpu_routine_pop_16(cpu_registers.h, cpu_registers.l); // Pop value from stack into HL register pair
}

void cpu_ldh_c_a() { // 0xE2
	core_advance_cpu_clocks(4);
	// TEMP DEBUG FIX: Use correct FF00+C IO address for LD (C),A. Revert this line if you need to restore the prior educational variant.
	// temp was also adding the a register.
	uint32_t temp = 0xFF00 + (uint32_t)cpu_registers.c;
	core_advance_cpu_clocks(4);
	memory_bus_write(temp, cpu_registers.a);
	core_advance_cpu_clocks(4);
}

void cpu_push_hl() { // 0xE5
	cpu_routine_push_16(cpu_registers.h, cpu_registers.l); // Push HL register pair onto the stack
}

void cpu_and_a_n() { // 0xE6
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_CARRY(0);
	SET_FLAG_HALF_CARRY(1);
	core_advance_cpu_clocks(4);
	cpu_registers.a &= memory_bus_read(cpu_registers.pc++);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_rst_20() { // 0xE7
	cpu_routine_rst_nnnn(0x0020); // Call subroutine at address 0x0020
}

void cpu_add_sp_n() { // 0xE8
	core_advance_cpu_clocks(4);
	int8_t temp = (int8_t)memory_bus_read(cpu_registers.pc++);
	core_advance_cpu_clocks(4);
	SET_FLAG_ZERO(0);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_CARRY(((cpu_registers.sp & 0x00FF) + (temp & 0x00FF)) > 0x00FF);
	SET_FLAG_HALF_CARRY(((cpu_registers.sp & 0x000f) + (temp & 0x000F)) > 0x000F);
	core_advance_cpu_clocks(4);
	cpu_registers.sp = (cpu_registers.sp + temp);
	core_advance_cpu_clocks(4);
}

void cpu_jp_hl() { // 0xE9
	core_advance_cpu_clocks(4);
	cpu_registers.pc = cpu_registers.hl;
}

void cpu_ld_nn_a() { // 0xEA
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.pc++);
	core_advance_cpu_clocks(4);
	cpu_registers.pc & 0xFFFF;
	temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8;
	core_advance_cpu_clocks(4);
	cpu_registers.pc &= 0xFFFF;
	core_advance_cpu_clocks(4);
	memory_bus_write(temp, cpu_registers.a);
}

void cpu_xor_a_n() { // 0xEE
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY(0);
	core_advance_cpu_clocks(4);
	cpu_registers.a ^= memory_bus_read(cpu_registers.pc++);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_rst_28() { // 0xEF
	cpu_routine_rst_nnnn(0x0028); // Call subroutine at address 0x0028
}

void cpu_ldh_a_n() { // 0xF0
	core_advance_cpu_clocks(4);
	uint32_t temp = 0xFF00 + (uint32_t)memory_bus_read(cpu_registers.pc++);
	core_advance_cpu_clocks(4);
	cpu_registers.a = memory_bus_read(temp);
	core_advance_cpu_clocks(4);
}

void cpu_pop_af() { // 0xF1
	cpu_routine_pop_16(cpu_registers.a, cpu_registers.f); // Pop value from stack into AF register pair
	cpu_registers.f &= 0xF0; // Clear the lower 4 bits of the F register (flags)
}

void cpu_ldh_a_c() { // 0xF2
	core_advance_cpu_clocks(4);
	cpu_registers.a = memory_bus_read(0xFF00 + cpu_registers.c);
	core_advance_cpu_clocks(4);
}

void cpu_di() { // 0xF3
	core_advance_cpu_clocks(4);
	interrupt_master_enable = false;
	interrupt_enable_ime_delay = 0;
}

void cpu_push_af() { // 0xF5
	cpu_routine_push_16(cpu_registers.a, cpu_registers.f); // Push AF register pair onto the stack
}

void cpu_or_a_n() { // 0xF6
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY(0);
	core_advance_cpu_clocks(4);
	cpu_registers.a |= memory_bus_read(cpu_registers.pc++);
	SET_FLAG_ZERO(cpu_registers.a == 0);
}

void cpu_rst_30() { // 0xF7
	cpu_routine_rst_nnnn(0x0030); // Call subroutine at address 0x0030
}

void cpu_ld_hl_sp_n() { // 0xF8
	core_advance_cpu_clocks(4);
	int8_t temp = (int8_t)memory_bus_read(cpu_registers.pc++);
	cpu_registers.pc &= 0xFFFF;
	core_advance_cpu_clocks(4);
	int16_t res = (int16_t)cpu_registers.sp + temp;
	core_advance_cpu_clocks(4);
	cpu_registers.hl = res & 0xFFFF;
	SET_FLAG_ZERO(0);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(((cpu_registers.sp & 0x000F) + (temp & 0x000F)) > 0x000F);
	SET_FLAG_CARRY(((cpu_registers.sp & 0x00FF) + (temp & 0x00FF)) > 0x00FF);
}

void cpu_ld_sp_hl() { // 0xF9
	core_advance_cpu_clocks(4);
	cpu_registers.sp = cpu_registers.hl;
	core_advance_cpu_clocks(4);
}

void cpu_ld_a_nn() { // 0xFA 
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.pc++);
	cpu_registers.pc &= 0xFFFF;
	core_advance_cpu_clocks(4);
	temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8;
	cpu_registers.pc &= 0xFFFF;
	core_advance_cpu_clocks(4);
	cpu_registers.a = memory_bus_read(temp);
	core_advance_cpu_clocks(4);
}

void cpu_ei() { // 0xFB
	core_advance_cpu_clocks(4);
	interrupt_enable_ime_delay = 1;
}

void cpu_cp_a_n() { // 0xFE
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(1);
	uint32_t temp_op = memory_bus_read(cpu_registers.pc++);
	uint32_t temp_a = cpu_registers.a;
	SET_FLAG_HALF_CARRY((temp_a & 0xF) < (temp_op & 0xF));
	SET_FLAG_CARRY(temp_a < temp_op);
	SET_FLAG_ZERO(temp_a == temp_op);
	core_advance_cpu_clocks(4);
}

void cpu_rst_38() { // 0xFF
	cpu_routine_rst_nnnn(0x0038); // Call subroutine at address 0x0038
}

void cpu_cb_rlc_b() { // 0x00
	cpu_routine_rlc_8(cpu_registers.b); // Rotate B left with carry
}

void cpu_cb_rlc_c() { // 0x01
	cpu_routine_rlc_8(cpu_registers.c); // Rotate C left with carry
}

void cpu_cb_rlc_d() { // 0x02
	cpu_routine_rlc_8(cpu_registers.d); // Rotate D left with carry
}

void cpu_cb_rlc_e() { // 0x03
	cpu_routine_rlc_8(cpu_registers.e); // Rotate E left with carry
}

void cpu_cb_rlc_h() { // 0x04
	cpu_routine_rlc_8(cpu_registers.h); // Rotate H left with carry
}

void cpu_cb_rlc_l() { // 0x05
	cpu_routine_rlc_8(cpu_registers.l); // Rotate L left with carry
}

void cpu_cb_rlc_hl() { // 0x06
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY((temp & 0x80) != 0);
	temp = (temp << 1) | (GET_FLAG_CARRY);
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
}

void cpu_cb_rlc_a() { // 0x07
	cpu_routine_rlc_8(cpu_registers.a); // Rotate A left with carry
}

void cpu_cb_rrc_b() { // 0x08
	cpu_routine_rrc_8(cpu_registers.b); // Rotate B right with carry
}

void cpu_cb_rrc_c() { // 0x09
	cpu_routine_rrc_8(cpu_registers.c); // Rotate C right with carry
}

void cpu_cb_rrc_d() { // 0x0A
	cpu_routine_rrc_8(cpu_registers.d); // Rotate D right with carry
}

void cpu_cb_rrc_e() { // 0x0B
	cpu_routine_rrc_8(cpu_registers.e); // Rotate E right with carry
}

void cpu_cb_rrc_h() { // 0x0C
	cpu_routine_rrc_8(cpu_registers.h); // Rotate H right with carry
}

void cpu_cb_rrc_l() { // 0x0D
	cpu_routine_rrc_8(cpu_registers.l); // Rotate L right with carry
}

void cpu_cb_rrc_hl() { // 0x0E
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY((temp & 0x01) != 0);
	temp = (temp >> 1) | (GET_FLAG_CARRY << 7);
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
}

void cpu_cb_rrc_a() { // 0x0F
	cpu_routine_rrc_8(cpu_registers.a); // Rotate A right with carry
}

void cpu_cb_rl_b() { // 0x10
	cpu_routine_rl_8(cpu_registers.b); // Rotate B left through carry
}

void cpu_cb_rl_c() { // 0x11
	cpu_routine_rl_8(cpu_registers.c); // Rotate C left through carry
}

void cpu_cb_rl_d() { // 0x12
	cpu_routine_rl_8(cpu_registers.d); // Rotate D left through carry
}

void cpu_cb_rl_e() { // 0x13
	cpu_routine_rl_8(cpu_registers.e); // Rotate E left through carry
}

void cpu_cb_rl_h() { // 0x14
	cpu_routine_rl_8(cpu_registers.h); // Rotate H left through carry
}

void cpu_cb_rl_l() { // 0x15
	cpu_routine_rl_8(cpu_registers.l); // Rotate L left through carry
}

void cpu_cb_rl_hl() { // 0x16
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	uint32_t temp_c = GET_FLAG_CARRY;
	SET_FLAG_CARRY((temp & 0x80) != 0);
	temp = ((temp << 1) | temp_c) & 0xFF;
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
}

void cpu_cb_rl_a() { // 0x17
	cpu_routine_rl_8(cpu_registers.a); // Rotate A left through carry
}

void cpu_cb_rr_b() { // 0x18
	cpu_routine_rr_8(cpu_registers.b); // Rotate B right through carry
}

void cpu_cb_rr_c() { // 0x19
	cpu_routine_rr_8(cpu_registers.c); // Rotate C right through carry
}

void cpu_cb_rr_d() { // 0x1A
	cpu_routine_rr_8(cpu_registers.d); // Rotate D right through carry
}

void cpu_cb_rr_e() { // 0x1B
	cpu_routine_rr_8(cpu_registers.e); // Rotate E right through carry
}

void cpu_cb_rr_h() { // 0x1C
	cpu_routine_rr_8(cpu_registers.h); // Rotate H right through carry
}

void cpu_cb_rr_l() { // 0x1D
	cpu_routine_rr_8(cpu_registers.l); // Rotate L right through carry
}

void cpu_cb_rr_hl() { // 0x1E
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	uint32_t temp_c = GET_FLAG_CARRY;
	SET_FLAG_CARRY((temp & 0x01) != 0);
	temp = (temp >> 1) | (temp_c << 7);
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
}

void cpu_cb_rr_a() { // 0x1F
	cpu_routine_rr_8(cpu_registers.a); // Rotate A right through carry
}

void cpu_cb_sla_b() { // 0x20
	cpu_routine_sla_8(cpu_registers.b); // Shift B left arithmetic
}

void cpu_cb_sla_c() { // 0x21
	cpu_routine_sla_8(cpu_registers.c); // Shift C left arithmetic
}

void cpu_cb_sla_d() { // 0x22
	cpu_routine_sla_8(cpu_registers.d); // Shift D left arithmetic
}

void cpu_cb_sla_e() { // 0x23
	cpu_routine_sla_8(cpu_registers.e); // Shift E left arithmetic
}

void cpu_cb_sla_h() { // 0x24
	cpu_routine_sla_8(cpu_registers.h); // Shift H left arithmetic
}

void cpu_cb_sla_l() { // 0x25
	cpu_routine_sla_8(cpu_registers.l); // Shift L left arithmetic
}

void cpu_cb_sla_hl() { // 0x26
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY((temp & 0x80) != 0);
	temp = (temp << 1) & 0xFF;
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
}

void cpu_cb_sla_a() { // 0x27
	cpu_routine_sla_8(cpu_registers.a); // Shift A left arithmetic
}

void cpu_cb_sra_b() { // 0x28
	cpu_routine_sra_8(cpu_registers.b); // Shift B right arithmetic
}

void cpu_cb_sra_c() { // 0x29
	cpu_routine_sra_8(cpu_registers.c); // Shift C right arithmetic
}

void cpu_cb_sra_d() { // 0x2A
	cpu_routine_sra_8(cpu_registers.d); // Shift D right arithmetic
}

void cpu_cb_sra_e() { // 0x2B
	cpu_routine_sra_8(cpu_registers.e); // Shift E right arithmetic
}

void cpu_cb_sra_h() { // 0x2C
	cpu_routine_sra_8(cpu_registers.h); // Shift H right arithmetic
}

void cpu_cb_sra_l() { // 0x2D
	cpu_routine_sra_8(cpu_registers.l); // Shift L right arithmetic
}

void cpu_cb_sra_hl() { // 0x2E
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY((temp & 0x01) != 0);
	temp = (temp & 0x80) | (temp >> 1);
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
}

void cpu_cb_sra_a() { // 0x2F
	cpu_routine_sra_8(cpu_registers.a); // Shift A right arithmetic
}

void cpu_cb_swap_b() { // 0x30
	cpu_routine_swap_8(cpu_registers.b); // Swap upper and lower nibbles of B
}

void cpu_cb_swap_c() { // 0x31
	cpu_routine_swap_8(cpu_registers.c); // Swap upper and lower nibbles of C
}

void cpu_cb_swap_d() { // 0x32
	cpu_routine_swap_8(cpu_registers.d); // Swap upper and lower nibbles of D
}

void cpu_cb_swap_e() { // 0x33
	cpu_routine_swap_8(cpu_registers.e); // Swap upper and lower nibbles of E
}

void cpu_cb_swap_h() { // 0x34
	cpu_routine_swap_8(cpu_registers.h); // Swap upper and lower nibbles of H
}

void cpu_cb_swap_l() { // 0x35
	cpu_routine_swap_8(cpu_registers.l); // Swap upper and lower nibbles of L
}

void cpu_cb_swap_hl() { // 0x36
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY(0);
	temp = ((temp >> 4) | (temp << 4)) & 0xFF;
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
}

void cpu_cb_swap_a() { // 0x37
	cpu_routine_swap_8(cpu_registers.a); // Swap upper and lower nibbles of A
}

void cpu_cb_srl_b() { // 0x38
	cpu_routine_srl_8(cpu_registers.b); // Shift B right logical
}

void cpu_cb_srl_c() { // 0x39
	cpu_routine_srl_8(cpu_registers.c); // Shift C right logical
}

void cpu_cb_srl_d() { // 0x3A
	cpu_routine_srl_8(cpu_registers.d); // Shift D right logical
}

void cpu_cb_srl_e() { // 0x3B
	cpu_routine_srl_8(cpu_registers.e); // Shift E right logical
}

void cpu_cb_srl_h() { // 0x3C
	cpu_routine_srl_8(cpu_registers.h); // Shift H right logical
}

void cpu_cb_srl_l() { // 0x3D
	cpu_routine_srl_8(cpu_registers.l); // Shift L right logical
}

void cpu_cb_srl_hl() { // 0x3E
	core_advance_cpu_clocks(4);
	uint32_t temp = memory_bus_read(cpu_registers.hl);
	core_advance_cpu_clocks(4);
	SET_FLAG_SUBTRACT(0);
	SET_FLAG_HALF_CARRY(0);
	SET_FLAG_CARRY((temp & 0x01) != 0);
	temp = temp >> 1;
	SET_FLAG_ZERO(temp == 0);
	core_advance_cpu_clocks(4);
	memory_bus_write(cpu_registers.hl, temp);
}

void cpu_cb_srl_a() { // 0x3F
	cpu_routine_srl_8(cpu_registers.a); // Shift A right logical
}

void cpu_cb_bit_0_b() { // 0x40
	cpu_routine_bit_n_8(0, cpu_registers.b); // Test bit 0 of B
}

void cpu_cb_bit_0_c() { // 0x41
	cpu_routine_bit_n_8(0, cpu_registers.c); // Test bit 0 of C
}

void cpu_cb_bit_0_d() { // 0x42
	cpu_routine_bit_n_8(0, cpu_registers.d); // Test bit 0 of D
}

void cpu_cb_bit_0_e() { // 0x43
	cpu_routine_bit_n_8(0, cpu_registers.e); // Test bit 0 of E
}

void cpu_cb_bit_0_h() { // 0x44
	cpu_routine_bit_n_8(0, cpu_registers.h); // Test bit 0 of H
}

void cpu_cb_bit_0_l() { // 0x45
	cpu_routine_bit_n_8(0, cpu_registers.l); // Test bit 0 of L
}

void cpu_cb_bit_0_hl() { // 0x46
	cpu_routine_bit_n_ptr_hl(0); // Test bit 0 of (HL)
}

void cpu_cb_bit_0_a() { // 0x47
	cpu_routine_bit_n_8(0, cpu_registers.a); // Test bit 0 of A
}

void cpu_cb_bit_1_b() { // 0x48
	cpu_routine_bit_n_8(1, cpu_registers.b); // Test bit 1 of B
}

void cpu_cb_bit_1_c() { // 0x49
	cpu_routine_bit_n_8(1, cpu_registers.c); // Test bit 1 of C
}

void cpu_cb_bit_1_d() { // 0x4A
	cpu_routine_bit_n_8(1, cpu_registers.d); // Test bit 1 of D
}

void cpu_cb_bit_1_e() { // 0x4B
	cpu_routine_bit_n_8(1, cpu_registers.e); // Test bit 1 of E
}

void cpu_cb_bit_1_h() { // 0x4C
	cpu_routine_bit_n_8(1, cpu_registers.h); // Test bit 1 of H
}

void cpu_cb_bit_1_l() { // 0x4D
	cpu_routine_bit_n_8(1, cpu_registers.l); // Test bit 1 of L
}

void cpu_cb_bit_1_hl() { // 0x4E
	cpu_routine_bit_n_ptr_hl(1); // Test bit 1 of (HL)
}

void cpu_cb_bit_1_a() { // 0x4F
	cpu_routine_bit_n_8(1, cpu_registers.a); // Test bit 1 of A
}

void cpu_cb_bit_2_b() { // 0x50
	cpu_routine_bit_n_8(2, cpu_registers.b); // Test bit 2 of B
}

void cpu_cb_bit_2_c() { // 0x51
	cpu_routine_bit_n_8(2, cpu_registers.c); // Test bit 2 of C
}

void cpu_cb_bit_2_d() { // 0x52
	cpu_routine_bit_n_8(2, cpu_registers.d); // Test bit 2 of D
}

void cpu_cb_bit_2_e() { // 0x53
	cpu_routine_bit_n_8(2, cpu_registers.e); // Test bit 2 of E
}

void cpu_cb_bit_2_h() { // 0x54
	cpu_routine_bit_n_8(2, cpu_registers.h); // Test bit 2 of H
}

void cpu_cb_bit_2_l() { // 0x55
	cpu_routine_bit_n_8(2, cpu_registers.l); // Test bit 2 of L
}

void cpu_cb_bit_2_hl() { // 0x56
	cpu_routine_bit_n_ptr_hl(2); // Test bit 2 of (HL)
}

void cpu_cb_bit_2_a() { // 0x57
	cpu_routine_bit_n_8(2, cpu_registers.a); // Test bit 2 of A
}

void cpu_cb_bit_3_b() { // 0x58
	cpu_routine_bit_n_8(3, cpu_registers.b); // Test bit 3 of B
}

void cpu_cb_bit_3_c() { // 0x59
	cpu_routine_bit_n_8(3, cpu_registers.c); // Test bit 3 of C
}

void cpu_cb_bit_3_d() { // 0x5A
	cpu_routine_bit_n_8(3, cpu_registers.d); // Test bit 3 of D
}

void cpu_cb_bit_3_e() { // 0x5B
	cpu_routine_bit_n_8(3, cpu_registers.e); // Test bit 3 of E
}

void cpu_cb_bit_3_h() { // 0x5C
	cpu_routine_bit_n_8(3, cpu_registers.h); // Test bit 3 of H
}

void cpu_cb_bit_3_l() { // 0x5D
	cpu_routine_bit_n_8(3, cpu_registers.l); // Test bit 3 of L
}

void cpu_cb_bit_3_hl() { // 0x5E
	cpu_routine_bit_n_ptr_hl(3); // Test bit 3 of (HL)
}

void cpu_cb_bit_3_a() { // 0x5F
	cpu_routine_bit_n_8(3, cpu_registers.a); // Test bit 3 of A
}

void cpu_cb_bit_4_b() { // 0x60
	cpu_routine_bit_n_8(4, cpu_registers.b); // Test bit 4 of B
}

void cpu_cb_bit_4_c() { // 0x61
	cpu_routine_bit_n_8(4, cpu_registers.c); // Test bit 4 of C
}

void cpu_cb_bit_4_d() { // 0x62
	cpu_routine_bit_n_8(4, cpu_registers.d); // Test bit 4 of D
}

void cpu_cb_bit_4_e() { // 0x63
	cpu_routine_bit_n_8(4, cpu_registers.e); // Test bit 4 of E
}

void cpu_cb_bit_4_h() { // 0x64
	cpu_routine_bit_n_8(4, cpu_registers.h); // Test bit 4 of H
}

void cpu_cb_bit_4_l() { // 0x65
	cpu_routine_bit_n_8(4, cpu_registers.l); // Test bit 4 of L
}

void cpu_cb_bit_4_hl() { // 0x66
	cpu_routine_bit_n_ptr_hl(4); // Test bit 4 of (HL)
}

void cpu_cb_bit_4_a() { // 0x67
	cpu_routine_bit_n_8(4, cpu_registers.a); // Test bit 4 of A
}

void cpu_cb_bit_5_b() { // 0x68
	cpu_routine_bit_n_8(5, cpu_registers.b); // Test bit 5 of B
}

void cpu_cb_bit_5_c() { // 0x69
	cpu_routine_bit_n_8(5, cpu_registers.c); // Test bit 5 of C
}

void cpu_cb_bit_5_d() { // 0x6A
	cpu_routine_bit_n_8(5, cpu_registers.d); // Test bit 5 of D
}

void cpu_cb_bit_5_e() { // 0x6B
	cpu_routine_bit_n_8(5, cpu_registers.e); // Test bit 5 of E
}

void cpu_cb_bit_5_h() { // 0x6C
	cpu_routine_bit_n_8(5, cpu_registers.h); // Test bit 5 of H
}

void cpu_cb_bit_5_l() { // 0x6D
	cpu_routine_bit_n_8(5, cpu_registers.l); // Test bit 5 of L
}

void cpu_cb_bit_5_hl() { // 0x6E
	cpu_routine_bit_n_ptr_hl(5); // Test bit 5 of (HL)
}

void cpu_cb_bit_5_a() { // 0x6F
	cpu_routine_bit_n_8(5, cpu_registers.a); // Test bit 5 of A
}

void cpu_cb_bit_6_b() { // 0x70
	cpu_routine_bit_n_8(6, cpu_registers.b); // Test bit 6 of B
}

void cpu_cb_bit_6_c() { // 0x71
	cpu_routine_bit_n_8(6, cpu_registers.c); // Test bit 6 of C
}

void cpu_cb_bit_6_d() { // 0x72
	cpu_routine_bit_n_8(6, cpu_registers.d); // Test bit 6 of D
}

void cpu_cb_bit_6_e() { // 0x73
	cpu_routine_bit_n_8(6, cpu_registers.e); // Test bit 6 of E
}

void cpu_cb_bit_6_h() { // 0x74
	cpu_routine_bit_n_8(6, cpu_registers.h); // Test bit 6 of H
}

void cpu_cb_bit_6_l() { // 0x75
	cpu_routine_bit_n_8(6, cpu_registers.l); // Test bit 6 of L
}

void cpu_cb_bit_6_hl() { // 0x76
	cpu_routine_bit_n_ptr_hl(6); // Test bit 6 of (HL)
}

void cpu_cb_bit_6_a() { // 0x77
	cpu_routine_bit_n_8(6, cpu_registers.a); // Test bit 6 of A
}

void cpu_cb_bit_7_b() { // 0x78
	cpu_routine_bit_n_8(7, cpu_registers.b); // Test bit 7 of B
}

void cpu_cb_bit_7_c() { // 0x79
	cpu_routine_bit_n_8(7, cpu_registers.c); // Test bit 7 of C
}

void cpu_cb_bit_7_d() { // 0x7A
	cpu_routine_bit_n_8(7, cpu_registers.d); // Test bit 7 of D
}

void cpu_cb_bit_7_e() { // 0x7B
	cpu_routine_bit_n_8(7, cpu_registers.e); // Test bit 7 of E
}

void cpu_cb_bit_7_h() { // 0x7C
	cpu_routine_bit_n_8(7, cpu_registers.h); // Test bit 7 of H
}

void cpu_cb_bit_7_l() { // 0x7D
	cpu_routine_bit_n_8(7, cpu_registers.l); // Test bit 7 of L
}

void cpu_cb_bit_7_hl() { // 0x7E
	cpu_routine_bit_n_ptr_hl(7); // Test bit 7 of (HL)
}

void cpu_cb_bit_7_a() { // 0x7F
	cpu_routine_bit_n_8(7, cpu_registers.a); // Test bit 7 of A
}

void cpu_cb_res_0_b() { // 0x80
	cpu_routine_res_n_8(0, cpu_registers.b); // Reset bit 0 of B
}

void cpu_cb_res_0_c() { // 0x81
	cpu_routine_res_n_8(0, cpu_registers.c); // Reset bit 0 of C
}

void cpu_cb_res_0_d() { // 0x82
	cpu_routine_res_n_8(0, cpu_registers.d); // Reset bit 0 of D
}

void cpu_cb_res_0_e() { // 0x83
	cpu_routine_res_n_8(0, cpu_registers.e); // Reset bit 0 of E
}

void cpu_cb_res_0_h() { // 0x84
	cpu_routine_res_n_8(0, cpu_registers.h); // Reset bit 0 of H
}

void cpu_cb_res_0_l() { // 0x85
	cpu_routine_res_n_8(0, cpu_registers.l); // Reset bit 0 of L
}

void cpu_cb_res_0_hl() { // 0x86
	cpu_routine_res_n_ptr_hl(0); // Reset bit 0 of (HL)
}

void cpu_cb_res_0_a() { // 0x87
	cpu_routine_res_n_8(0, cpu_registers.a); // Reset bit 0 of A
}

void cpu_cb_res_1_b() { // 0x88
	cpu_routine_res_n_8(1, cpu_registers.b); // Reset bit 1 of B
}

void cpu_cb_res_1_c() { // 0x89
	cpu_routine_res_n_8(1, cpu_registers.c); // Reset bit 1 of C
}

void cpu_cb_res_1_d() { // 0x8A
	cpu_routine_res_n_8(1, cpu_registers.d); // Reset bit 1 of D
}

void cpu_cb_res_1_e() { // 0x8B
	cpu_routine_res_n_8(1, cpu_registers.e); // Reset bit 1 of E
}

void cpu_cb_res_1_h() { // 0x8C
	cpu_routine_res_n_8(1, cpu_registers.h); // Reset bit 1 of H
}

void cpu_cb_res_1_l() { // 0x8D
	cpu_routine_res_n_8(1, cpu_registers.l); // Reset bit 1 of L
}

void cpu_cb_res_1_hl() { // 0x8E
	cpu_routine_res_n_ptr_hl(1); // Reset bit 1 of (HL)
}

void cpu_cb_res_1_a() { // 0x8F
	cpu_routine_res_n_8(1, cpu_registers.a); // Reset bit 1 of A
}

void cpu_cb_res_2_b() { // 0x90
	cpu_routine_res_n_8(2, cpu_registers.b); // Reset bit 2 of B
}

void cpu_cb_res_2_c() { // 0x91
	cpu_routine_res_n_8(2, cpu_registers.c); // Reset bit 2 of C
}

void cpu_cb_res_2_d() { // 0x92
	cpu_routine_res_n_8(2, cpu_registers.d); // Reset bit 2 of D
}

void cpu_cb_res_2_e() { // 0x93
	cpu_routine_res_n_8(2, cpu_registers.e); // Reset bit 2 of E
}

void cpu_cb_res_2_h() { // 0x94
	cpu_routine_res_n_8(2, cpu_registers.h); // Reset bit 2 of H
}

void cpu_cb_res_2_l() { // 0x95
	cpu_routine_res_n_8(2, cpu_registers.l); // Reset bit 2 of L
}

void cpu_cb_res_2_hl() { // 0x96
	cpu_routine_res_n_ptr_hl(2); // Reset bit 2 of (HL)
}

void cpu_cb_res_2_a() { // 0x97
	cpu_routine_res_n_8(2, cpu_registers.a); // Reset bit 2 of A
}

void cpu_cb_res_3_b() { // 0x98
	cpu_routine_res_n_8(3, cpu_registers.b); // Reset bit 3 of B
}

void cpu_cb_res_3_c() { // 0x99
	cpu_routine_res_n_8(3, cpu_registers.c); // Reset bit 3 of C
}

void cpu_cb_res_3_d() { // 0x9A
	cpu_routine_res_n_8(3, cpu_registers.d); // Reset bit 3 of D
}

void cpu_cb_res_3_e() { // 0x9B
	cpu_routine_res_n_8(3, cpu_registers.e); // Reset bit 3 of E
}

void cpu_cb_res_3_h() { // 0x9C
	cpu_routine_res_n_8(3, cpu_registers.h); // Reset bit 3 of H
}

void cpu_cb_res_3_l() { // 0x9D
	cpu_routine_res_n_8(3, cpu_registers.l); // Reset bit 3 of L
}

void cpu_cb_res_3_hl() { // 0x9E
	cpu_routine_res_n_ptr_hl(3); // Reset bit 3 of (HL)
}

void cpu_cb_res_3_a() { // 0x9F
	cpu_routine_res_n_8(3, cpu_registers.a); // Reset bit 3 of A
}

void cpu_cb_res_4_b() { // 0xA0
	cpu_routine_res_n_8(4, cpu_registers.b); // Reset bit 4 of B
}

void cpu_cb_res_4_c() { // 0xA1
	cpu_routine_res_n_8(4, cpu_registers.c); // Reset bit 4 of C
}

void cpu_cb_res_4_d() { // 0xA2
	cpu_routine_res_n_8(4, cpu_registers.d); // Reset bit 4 of D
}

void cpu_cb_res_4_e() { // 0xA3
	cpu_routine_res_n_8(4, cpu_registers.e); // Reset bit 4 of E
}

void cpu_cb_res_4_h() { // 0xA4
	cpu_routine_res_n_8(4, cpu_registers.h); // Reset bit 4 of H
}

void cpu_cb_res_4_l() { // 0xA5
	cpu_routine_res_n_8(4, cpu_registers.l); // Reset bit 4 of L
}

void cpu_cb_res_4_hl() { // 0xA6
	cpu_routine_res_n_ptr_hl(4); // Reset bit 4 of (HL)
}

void cpu_cb_res_4_a() { // 0xA7
	cpu_routine_res_n_8(4, cpu_registers.a); // Reset bit 4 of A
}

void cpu_cb_res_5_b() { // 0xA8
	cpu_routine_res_n_8(5, cpu_registers.b); // Reset bit 5 of B
}

void cpu_cb_res_5_c() { // 0xA9
	cpu_routine_res_n_8(5, cpu_registers.c); // Reset bit 5 of C
}

void cpu_cb_res_5_d() { // 0xAA
	cpu_routine_res_n_8(5, cpu_registers.d); // Reset bit 5 of D
}

void cpu_cb_res_5_e() { // 0xAB
	cpu_routine_res_n_8(5, cpu_registers.e); // Reset bit 5 of E
}

void cpu_cb_res_5_h() { // 0xAC
	cpu_routine_res_n_8(5, cpu_registers.h); // Reset bit 5 of H
}

void cpu_cb_res_5_l() { // 0xAD
	cpu_routine_res_n_8(5, cpu_registers.l); // Reset bit 5 of L
}

void cpu_cb_res_5_hl() { // 0xAE
	cpu_routine_res_n_ptr_hl(5); // Reset bit 5 of (HL)
}

void cpu_cb_res_5_a() { // 0xAF
	cpu_routine_res_n_8(5, cpu_registers.a); // Reset bit 5 of A
}

void cpu_cb_res_6_b() { // 0xB0
	cpu_routine_res_n_8(6, cpu_registers.b); // Reset bit 6 of B
}

void cpu_cb_res_6_c() { // 0xB1
	cpu_routine_res_n_8(6, cpu_registers.c); // Reset bit 6 of C
}

void cpu_cb_res_6_d() { // 0xB2
	cpu_routine_res_n_8(6, cpu_registers.d); // Reset bit 6 of D
}

void cpu_cb_res_6_e() { // 0xB3
	cpu_routine_res_n_8(6, cpu_registers.e); // Reset bit 6 of E
}

void cpu_cb_res_6_h() { // 0xB4
	cpu_routine_res_n_8(6, cpu_registers.h); // Reset bit 6 of H
}

void cpu_cb_res_6_l() { // 0xB5
	cpu_routine_res_n_8(6, cpu_registers.l); // Reset bit 6 of L
}

void cpu_cb_res_6_hl() { // 0xB6
	cpu_routine_res_n_ptr_hl(6); // Reset bit 6 of (HL)
}

void cpu_cb_res_6_a() { // 0xB7
	cpu_routine_res_n_8(6, cpu_registers.a); // Reset bit 6 of A
}

void cpu_cb_res_7_b() { // 0xB8
	cpu_routine_res_n_8(7, cpu_registers.b); // Reset bit 7 of B
}

void cpu_cb_res_7_c() { // 0xB9
	cpu_routine_res_n_8(7, cpu_registers.c); // Reset bit 7 of C
}

void cpu_cb_res_7_d() { // 0xBA
	cpu_routine_res_n_8(7, cpu_registers.d); // Reset bit 7 of D
}

void cpu_cb_res_7_e() { // 0xBB
	cpu_routine_res_n_8(7, cpu_registers.e); // Reset bit 7 of E
}

void cpu_cb_res_7_h() { // 0xBC
	cpu_routine_res_n_8(7, cpu_registers.h); // Reset bit 7 of H
}

void cpu_cb_res_7_l() { // 0xBD
	cpu_routine_res_n_8(7, cpu_registers.l); // Reset bit 7 of L
}

void cpu_cb_res_7_hl() { // 0xBE
	cpu_routine_res_n_ptr_hl(7); // Reset bit 7 of (HL)
}

void cpu_cb_res_7_a() { // 0xBF
	cpu_routine_res_n_8(7, cpu_registers.a); // Reset bit 7 of A
}

void cpu_cb_set_0_b() { // 0xC0
	cpu_routine_set_n_8(0, cpu_registers.b); // Set bit 0 of B
}

void cpu_cb_set_0_c() { // 0xC1
	cpu_routine_set_n_8(0, cpu_registers.c); // Set bit 0 of C
}

void cpu_cb_set_0_d() { // 0xC2
	cpu_routine_set_n_8(0, cpu_registers.d); // Set bit 0 of D
}

void cpu_cb_set_0_e() { // 0xC3
	cpu_routine_set_n_8(0, cpu_registers.e); // Set bit 0 of E
}

void cpu_cb_set_0_h() { // 0xC4
	cpu_routine_set_n_8(0, cpu_registers.h); // Set bit 0 of H
}

void cpu_cb_set_0_l() { // 0xC5
	cpu_routine_set_n_8(0, cpu_registers.l); // Set bit 0 of L
}

void cpu_cb_set_0_hl() { // 0xC6
	cpu_routine_set_n_ptr_hl(0); // Set bit 0 of (HL)
}

void cpu_cb_set_0_a() { // 0xC7
	cpu_routine_set_n_8(0, cpu_registers.a); // Set bit 0 of A
}

void cpu_cb_set_1_b() { // 0xC8
	cpu_routine_set_n_8(1, cpu_registers.b); // Set bit 1 of B
}

void cpu_cb_set_1_c() { // 0xC9
	cpu_routine_set_n_8(1, cpu_registers.c); // Set bit 1 of C
}

void cpu_cb_set_1_d() { // 0xCA
	cpu_routine_set_n_8(1, cpu_registers.d); // Set bit 1 of D
}

void cpu_cb_set_1_e() { // 0xCB
	cpu_routine_set_n_8(1, cpu_registers.e); // Set bit 1 of E
}

void cpu_cb_set_1_h() { // 0xCC
	cpu_routine_set_n_8(1, cpu_registers.h); // Set bit 1 of H
}

void cpu_cb_set_1_l() { // 0xCD
	cpu_routine_set_n_8(1, cpu_registers.l); // Set bit 1 of L
}

void cpu_cb_set_1_hl() { // 0xCE
	cpu_routine_set_n_ptr_hl(1); // Set bit 1 of (HL)
}

void cpu_cb_set_1_a() { // 0xCF
	cpu_routine_set_n_8(1, cpu_registers.a); // Set bit 1 of A
}

void cpu_cb_set_2_b() { // 0xD0
	cpu_routine_set_n_8(2, cpu_registers.b); // Set bit 2 of B
}

void cpu_cb_set_2_c() { // 0xD1
	cpu_routine_set_n_8(2, cpu_registers.c); // Set bit 2 of C
}

void cpu_cb_set_2_d() { // 0xD2
	cpu_routine_set_n_8(2, cpu_registers.d); // Set bit 2 of D
}

void cpu_cb_set_2_e() { // 0xD3
	cpu_routine_set_n_8(2, cpu_registers.e); // Set bit 2 of E
}

void cpu_cb_set_2_h() { // 0xD4
	cpu_routine_set_n_8(2, cpu_registers.h); // Set bit 2 of H
}

void cpu_cb_set_2_l() { // 0xD5
	cpu_routine_set_n_8(2, cpu_registers.l); // Set bit 2 of L
}

void cpu_cb_set_2_hl() { // 0xD6
	cpu_routine_set_n_ptr_hl(2); // Set bit 2 of (HL)
}

void cpu_cb_set_2_a() { // 0xD7
	cpu_routine_set_n_8(2, cpu_registers.a); // Set bit 2 of A
}

void cpu_cb_set_3_b() { // 0xD8
	cpu_routine_set_n_8(3, cpu_registers.b); // Set bit 3 of B
}

void cpu_cb_set_3_c() { // 0xD9
	cpu_routine_set_n_8(3, cpu_registers.c); // Set bit 3 of C
}

void cpu_cb_set_3_d() { // 0xDA
	cpu_routine_set_n_8(3, cpu_registers.d); // Set bit 3 of D
}

void cpu_cb_set_3_e() { // 0xDB
	cpu_routine_set_n_8(3, cpu_registers.e); // Set bit 3 of E
}

void cpu_cb_set_3_h() { // 0xDC
	cpu_routine_set_n_8(3, cpu_registers.h); // Set bit 3 of H
}

void cpu_cb_set_3_l() { // 0xDD
	cpu_routine_set_n_8(3, cpu_registers.l); // Set bit 3 of L
}

void cpu_cb_set_3_hl() { // 0xDE
	cpu_routine_set_n_ptr_hl(3); // Set bit 3 of (HL)
}

void cpu_cb_set_3_a() { // 0xDF
	cpu_routine_set_n_8(3, cpu_registers.a); // Set bit 3 of A
}

void cpu_cb_set_4_b() { // 0xE0
	cpu_routine_set_n_8(4, cpu_registers.b); // Set bit 4 of B
}

void cpu_cb_set_4_c() { // 0xE1
	cpu_routine_set_n_8(4, cpu_registers.c); // Set bit 4 of C
}

void cpu_cb_set_4_d() { // 0xE2
	cpu_routine_set_n_8(4, cpu_registers.d); // Set bit 4 of D
}

void cpu_cb_set_4_e() { // 0xE3
	cpu_routine_set_n_8(4, cpu_registers.e); // Set bit 4 of E
}

void cpu_cb_set_4_h() { // 0xE4
	cpu_routine_set_n_8(4, cpu_registers.h); // Set bit 4 of H
}

void cpu_cb_set_4_l() { // 0xE5
	cpu_routine_set_n_8(4, cpu_registers.l); // Set bit 4 of L
}

void cpu_cb_set_4_hl() { // 0xE6
	cpu_routine_set_n_ptr_hl(4); // Set bit 4 of (HL)
}

void cpu_cb_set_4_a() { // 0xE7
	cpu_routine_set_n_8(4, cpu_registers.a); // Set bit 4 of A
}

void cpu_cb_set_5_b() { // 0xE8
	cpu_routine_set_n_8(5, cpu_registers.b); // Set bit 5 of B
}

void cpu_cb_set_5_c() { // 0xE9
	cpu_routine_set_n_8(5, cpu_registers.c); // Set bit 5 of C
}

void cpu_cb_set_5_d() { // 0xEA
	cpu_routine_set_n_8(5, cpu_registers.d); // Set bit 5 of D
}

void cpu_cb_set_5_e() { // 0xEB
	cpu_routine_set_n_8(5, cpu_registers.e); // Set bit 5 of E
}

void cpu_cb_set_5_h() { // 0xEC
	cpu_routine_set_n_8(5, cpu_registers.h); // Set bit 5 of H
}

void cpu_cb_set_5_l() { // 0xED
	cpu_routine_set_n_8(5, cpu_registers.l); // Set bit 5 of L
}

void cpu_cb_set_5_hl() { // 0xEE
	cpu_routine_set_n_ptr_hl(5); // Set bit 5 of (HL)
}

void cpu_cb_set_5_a() { // 0xEF
	cpu_routine_set_n_8(5, cpu_registers.a); // Set bit 5 of A
}

void cpu_cb_set_6_b() { // 0xF0
	cpu_routine_set_n_8(6, cpu_registers.b); // Set bit 6 of B
}

void cpu_cb_set_6_c() { // 0xF1
	cpu_routine_set_n_8(6, cpu_registers.c); // Set bit 6 of C
}

void cpu_cb_set_6_d() { // 0xF2
	cpu_routine_set_n_8(6, cpu_registers.d); // Set bit 6 of D
}

void cpu_cb_set_6_e() { // 0xF3
	cpu_routine_set_n_8(6, cpu_registers.e); // Set bit 6 of E
}

void cpu_cb_set_6_h() { // 0xF4
	cpu_routine_set_n_8(6, cpu_registers.h); // Set bit 6 of H
}

void cpu_cb_set_6_l() { // 0xF5
	cpu_routine_set_n_8(6, cpu_registers.l); // Set bit 6 of L
}

void cpu_cb_set_6_hl() { // 0xF6
	cpu_routine_set_n_ptr_hl(6); // Set bit 6 of (HL)
}

void cpu_cb_set_6_a() { // 0xF7
	cpu_routine_set_n_8(6, cpu_registers.a); // Set bit 6 of A
}

void cpu_cb_set_7_b() { // 0xF8
	cpu_routine_set_n_8(7, cpu_registers.b); // Set bit 7 of B
}

void cpu_cb_set_7_c() { // 0xF9
	cpu_routine_set_n_8(7, cpu_registers.c); // Set bit 7 of C
}

void cpu_cb_set_7_d() { // 0xFA
	cpu_routine_set_n_8(7, cpu_registers.d); // Set bit 7 of D
}

void cpu_cb_set_7_e() { // 0xFB
	cpu_routine_set_n_8(7, cpu_registers.e); // Set bit 7 of E
}

void cpu_cb_set_7_h() { // 0xFC
	cpu_routine_set_n_8(7, cpu_registers.h); // Set bit 7 of H
}

void cpu_cb_set_7_l() { // 0xFD
	cpu_routine_set_n_8(7, cpu_registers.l); // Set bit 7 of L
}

void cpu_cb_set_7_hl() { // 0xFE
	cpu_routine_set_n_ptr_hl(7); // Set bit 7 of (HL)
}

void cpu_cb_set_7_a() { // 0xFF
	cpu_routine_set_n_8(7, cpu_registers.a); // Set bit 7 of A
}