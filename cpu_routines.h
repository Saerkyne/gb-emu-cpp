#pragma once

// This file contains the most common routines for the Game Boy emulator.

#define cpu_routine_ld_8(reg8)                      \
	{                                               \
		core_advance_cpu_clocks(4);                 \
		reg8 = memory_bus_read(cpu_registers.pc++); \
		core_advance_cpu_clocks(4);                 \
	}

#define cpu_routine_ld_16(reg_high, reg_low)            \
	{                                                   \
		core_advance_cpu_clocks(4);                     \
		reg_low = memory_bus_read(cpu_registers.pc++);  \
		core_advance_cpu_clocks(4);                     \
		reg_high = memory_bus_read(cpu_registers.pc++); \
		core_advance_cpu_clocks(4);                     \
	}

#define cpu_routine_ld_ptr8(reg16, reg8) \
	{                                    \
		core_advance_cpu_clocks(4);      \
		memory_bus_write(reg16, reg8);   \
		core_advance_cpu_clocks(4);      \
	}

#define cpu_routine_ld_ptr16(reg8, reg16) \
	{                                     \
		core_advance_cpu_clocks(4);       \
		reg8 = memory_bus_read(reg16);    \
		core_advance_cpu_clocks(4);       \
	}

#define cpu_routine_inc_16(reg16)   \
	{                               \
		core_advance_cpu_clocks(4); \
		reg16 = (reg16 + 1);        \
		core_advance_cpu_clocks(4); \
	}

#define cpu_routine_dec_16(reg16)   \
	{                               \
		core_advance_cpu_clocks(4); \
		reg16 = (reg16 - 1);        \
		core_advance_cpu_clocks(4); \
	}

#define cpu_routine_inc_8(reg8)                   \
	{                                             \
		SET_FLAG_SUBTRACT(0);                     \
		SET_FLAG_HALF_CARRY((reg8 & 0xF) == 0x0); \
		reg8++;                                   \
		SET_FLAG_ZERO(reg8 == 0);                 \
		core_advance_cpu_clocks(4);               \
	}

#define cpu_routine_dec_8(reg8)                   \
	{                                             \
		SET_FLAG_SUBTRACT(1);                     \
		SET_FLAG_HALF_CARRY((reg8 & 0xF) == 0x0); \
		reg8--;                                   \
		SET_FLAG_ZERO(reg8 == 0);                 \
		core_advance_cpu_clocks(4);               \
	}

#define cpu_routine_add_hl_16(reg16)                                         \
	{                                                                        \
		SET_FLAG_SUBTRACT(0);                                                \
		uint32_t temp = cpu_registers.hl + reg16;                            \
		SET_FLAG_CARRY(temp > 0xFFFF);                                       \
		bool hc = ((cpu_registers.hl & 0x0FFF) + (reg16 & 0x0FFF)) > 0x0FFF; \
		SET_FLAG_HALF_CARRY(hc);                                             \
		core_advance_cpu_clocks(4);                                          \
		cpu_registers.hl = temp & 0xFFFF;                                    \
		core_advance_cpu_clocks(4);                                          \
	}

#define cpu_routine_add_a_8(reg8)                                           \
	{                                                                       \
		SET_FLAG_SUBTRACT(0);                                               \
		uint32_t temp = cpu_registers.a;                                    \
		SET_FLAG_HALF_CARRY(((temp & 0xF) + ((uint32_t)reg8 & 0xF)) > 0xF); \
		cpu_registers.a += reg8;                                            \
		SET_FLAG_ZERO(cpu_registers.a == 0);                                \
		SET_FLAG_CARRY(temp > cpu_registers.a);                             \
		core_advance_cpu_clocks(4);                                         \
	}

#define cpu_routine_adc_a_8(reg8)                                           \
	{                                                                       \
		SET_FLAG_SUBTRACT(0);                                               \
		uint8_t carry = GET_FLAG_CARRY;                                     \
		uint32_t temp = cpu_registers.a + reg8 + carry;                     \
		bool hc = (((cpu_registers.a & 0xF) + (reg8 & 0xF)) + carry) > 0xF; \
		SET_FLAG_HALF_CARRY(hc);                                            \
		SET_FLAG_CARRY(temp > 0xFF);                                        \
		temp &= 0xFF;                                                       \
		cpu_registers.a = temp;                                             \
		SET_FLAG_ZERO(temp == 0);                                           \
		core_advance_cpu_clocks(4);                                         \
	}

#define cpu_routine_sub_a_8(reg8)                                    \
	{                                                                \
		SET_FLAG_SUBTRACT(1);                                        \
		SET_FLAG_HALF_CARRY((cpu_registers.a & 0xF) < (reg8 & 0xF)); \
		SET_FLAG_CARRY((uint32_t)cpu_registers.a < (uint32_t)reg8);  \
		cpu_registers.a -= reg8;                                     \
		SET_FLAG_ZERO(cpu_registers.a == 0);                         \
		core_advance_cpu_clocks(4);                                  \
	}

#define cpu_routine_sbc_a_8(reg8)                                           \
	{                                                                       \
		uint16_t temp = cpu_registers.a - (reg8 + GET_FLAG_CARRY);          \
		SET_FLAG_SUBTRACT(1);                                               \
		SET_FLAG_CARRY((temp & ~0xFF) ? 1 : 0);                             \
		SET_FLAG_ZERO((temp & 0xFF) ? 0 : 1);                               \
		SET_FLAG_HALF_CARRY(((cpu_registers.a ^ reg8 ^ temp) & 0x10) != 0); \
		cpu_registers.a = temp;                                             \
		core_advance_cpu_clocks(4);                                         \
	}

#define cpu_routine_and_a_8(reg8)            \
	{                                        \
		SET_FLAG_HALF_CARRY(1);              \
		SET_FLAG_SUBTRACT(0);                \
		SET_FLAG_CARRY(0);                   \
		cpu_registers.a &= reg8;             \
		SET_FLAG_ZERO(cpu_registers.a == 0); \
		core_advance_cpu_clocks(4);          \
	}

#define cpu_routine_xor_a_8(reg8)            \
	{                                        \
		SET_FLAG_HALF_CARRY(0);              \
		SET_FLAG_SUBTRACT(0);                \
		SET_FLAG_CARRY(0);                   \
		cpu_registers.a ^= reg8;             \
		SET_FLAG_ZERO(cpu_registers.a == 0); \
		core_advance_cpu_clocks(4);          \
	}

#define cpu_routine_or_a_8(reg8)             \
	{                                        \
		SET_FLAG_HALF_CARRY(0);              \
		SET_FLAG_SUBTRACT(0);                \
		SET_FLAG_CARRY(0);                   \
		cpu_registers.a |= reg8;             \
		SET_FLAG_ZERO(cpu_registers.a == 0); \
		core_advance_cpu_clocks(4);          \
	}

#define cpu_routine_cp_a_8(reg8)                                     \
	{                                                                \
		SET_FLAG_SUBTRACT(0);                                        \
		SET_FLAG_HALF_CARRY((cpu_registers.a & 0xF) < (reg8 & 0xF)); \
		SET_FLAG_CARRY((uint32_t)cpu_registers.a < (uint32_t)reg8);  \
		SET_FLAG_ZERO(cpu_registers.a == reg8);                      \
		core_advance_cpu_clocks(4);                                  \
	}

#define cpu_routine_rst_nnnn(addr)                                \
	{                                                             \
		core_advance_cpu_clocks(4);                               \
		cpu_registers.sp--;                                       \
		cpu_registers.sp &= 0xFFFF;                               \
		const uint8_t pc_high = (cpu_registers.pc & 0xFF00) >> 8; \
		core_advance_cpu_clocks(4);                               \
		memory_bus_write(cpu_registers.sp, pc_high);              \
		core_advance_cpu_clocks(4);                               \
		cpu_registers.sp--;                                       \
		cpu_registers.sp &= 0xFFFF;                               \
		const uint8_t pc_low = (cpu_registers.pc & 0xFF);         \
		memory_bus_write(cpu_registers.sp, pc_low);               \
		cpu_registers.pc = addr;                                  \
		core_advance_cpu_clocks(4);                               \
	}

// TODO: check if the second memory bus write should be the low byte.
// Original implementation used high byte.
#define cpu_routine_push_16(reg_high, reg_low)        \
	{                                                 \
		core_advance_cpu_clocks(4);                   \
		cpu_registers.sp--;                           \
		cpu_registers.sp &= 0xFFFF;                   \
		core_advance_cpu_clocks(4);                   \
		memory_bus_write(cpu_registers.sp, reg_high); \
		core_advance_cpu_clocks(4);                   \
		cpu_registers.sp--;                           \
		cpu_registers.sp &= 0xFFFF;                   \
		memory_bus_write(cpu_registers.sp, reg_high); \
		core_advance_cpu_clocks(4);                   \
	}

#define cpu_routine_pop_16(reg_high, reg_low)           \
	{                                                   \
		core_advance_cpu_clocks(4);                     \
		reg_low = memory_bus_read(cpu_registers.sp++);  \
		cpu_registers.sp &= 0xFFFF;                     \
		core_advance_cpu_clocks(4);                     \
		reg_high = memory_bus_read(cpu_registers.sp++); \
		cpu_registers.sp &= 0xFFFF;                     \
		core_advance_cpu_clocks(4);                     \
	}

#define cpu_routine_call_conditional_nnnn(cond)                           \
	{                                                                     \
		core_advance_cpu_clocks(4);                                       \
		if (cond)                                                         \
		{                                                                 \
			uint32_t temp = memory_bus_read(cpu_registers.pc++);          \
			core_advance_cpu_clocks(4);                                   \
			temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8; \
			core_advance_cpu_clocks(4);                                   \
			cpu_registers.sp--;                                           \
			cpu_registers.sp &= 0xFFFF;                                   \
			const uint8_t pc_high = (cpu_registers.pc & 0xFF00) >> 8;     \
			core_advance_cpu_clocks(4);                                   \
			memory_bus_write(cpu_registers.sp, pc_high);                  \
			core_advance_cpu_clocks(4);                                   \
			cpu_registers.sp--;                                           \
			cpu_registers.sp &= 0xFFFF;                                   \
			const uint8_t pc_low = (cpu_registers.pc & 0xFF);             \
			memory_bus_write(cpu_registers.sp, pc_low);                   \
			cpu_registers.pc = temp;                                      \
			core_advance_cpu_clocks(4);                                   \
		}                                                                 \
		else                                                              \
		{                                                                 \
			cpu_registers.pc++;                                           \
			core_advance_cpu_clocks(4);                                   \
			cpu_registers.pc++;                                           \
			core_advance_cpu_clocks(4);                                   \
			cpu_registers.pc &= 0xFFFF;                                   \
		}                                                                 \
	}

#define cpu_routine_ret_conditional(cond)                                 \
	{                                                                     \
		core_advance_cpu_clocks(4);                                       \
		if (cond)                                                         \
		{                                                                 \
			uint32_t temp = memory_bus_read(cpu_registers.sp++);          \
			cpu_registers.sp &= 0xFFFF;                                   \
			core_advance_cpu_clocks(4);                                   \
			temp |= ((uint32_t)memory_bus_read(cpu_registers.sp++)) << 8; \
			cpu_registers.sp &= 0xFFFF;                                   \
			core_advance_cpu_clocks(4);                                   \
			cpu_registers.pc = temp;                                      \
			core_advance_cpu_clocks(4);                                   \
			core_advance_cpu_clocks(4);                                   \
		}                                                                 \
		else                                                              \
		{                                                                 \
			core_advance_cpu_clocks(4);                                   \
		}                                                                 \
	}

#define cpu_routine_jp_conditional_nnnn(cond)                             \
	{                                                                     \
		core_advance_cpu_clocks(4);                                       \
		if (cond)                                                         \
		{                                                                 \
			uint32_t temp = memory_bus_read(cpu_registers.pc++);          \
			cpu_registers.pc &= 0xFFFF;                                   \
			core_advance_cpu_clocks(4);                                   \
			temp |= ((uint32_t)memory_bus_read(cpu_registers.pc++)) << 8; \
			cpu_registers.pc &= 0xFFFF;                                   \
			core_advance_cpu_clocks(4);                                   \
			cpu_registers.pc = temp;                                      \
			core_advance_cpu_clocks(4);                                   \
		}                                                                 \
		else                                                              \
		{                                                                 \
			cpu_registers.pc++;                                           \
			core_advance_cpu_clocks(4);                                   \
			cpu_registers.pc++;                                           \
			core_advance_cpu_clocks(4);                                   \
		}                                                                 \
	}