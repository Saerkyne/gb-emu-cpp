#pragma once

// This file contains the most common routines for the Game Boy emulator.

#define cpu_routine_ld_8(reg8)										\
{																	\
	core_advance_cpu_clocks(4);										\
	reg8 = memory_bus_read(cpu_registers.pc++);						\
	core_advance_cpu_clocks(4);										\
}

#define cpu_routine_ld_16(reg_high, reg_low)					    \
{																	\
	core_advance_cpu_clocks(4);										\
	reg_low = memory_bus_read(cpu_registers.pc++);					\
	core_advance_cpu_clocks(4);										\
	reg_high = memory_bus_read(cpu_registers.pc++);					\
	core_advance_cpu_clocks(4);										\
}		

#define cpu_routine_dec_8(reg8)										\
{																	\
	SET_FLAG_SUBTRACT(1);											\
	SET_FLAG_HALF_CARRY((reg8 & 0xF) == 0x0);						\
	reg8--;															\
	SET_FLAG_ZERO(reg8 == 0);										\
	core_advance_cpu_clocks(4);										\
}