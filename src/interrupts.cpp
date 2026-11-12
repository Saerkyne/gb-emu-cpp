#include "interrupts.h"
#include "memory_bus.h"
#include "emulator_core.h"
#include "cpu.h"

extern uint8_t cpu_halt_count;

bool interrupt_master_enable = false;
uint8_t interrupt_enable_ime_delay = 0;
bool interrupt_v_blank = false;
bool interrupt_lcd_stat = false;
bool interrupt_timer = false;
bool interrupt_serial = false;
bool interrupt_joypad = false;

void interrupt_enable_write(uint8_t value) {
    memory[ADDR_IO_IE] = value;
}

void interrupt_flag_write(uint8_t value) {
    memory[ADDR_IO_IF] = value;
}

void interrupt_raise_flag(uint8_t flag) {
    memory[ADDR_IO_IF] |= flag;
}

void interrupt_jump() {
    uint16_t destination;
    core_advance_cpu_clocks(4);
    const uint8_t pc_high = (cpu_registers.pc & 0xFF00) >> 8;
    const uint8_t pc_low = cpu_registers.pc & 0xFF;
    cpu_registers.sp--;
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.sp, pc_high);
    if (((memory[ADDR_IO_IE] & memory[ADDR_IO_IF]) & 0x1F) == 0x00)
    {
		destination = 0x0000; // If no interrupts are enabled or requested, jump to address 0x0000
        // Don't clear the IF byte
    }
    else
    {
		if (CHECK_BIT(memory[ADDR_IO_IE], 0) && CHECK_BIT(memory[ADDR_IO_IF], 0))
		{
			CLEAR_BIT(memory[ADDR_IO_IF], 0);
            destination = INTERRUPT_HANDLER_V_BLANK;
		}
		else if (CHECK_BIT(memory[ADDR_IO_IE], 1) && CHECK_BIT(memory[ADDR_IO_IF], 1))
		{
			CLEAR_BIT(memory[ADDR_IO_IF], 1);
            destination = INTERRUPT_HANDLER_LCD_STATUS;
		}
		else if (CHECK_BIT(memory[ADDR_IO_IE], 2) && CHECK_BIT(memory[ADDR_IO_IF], 2))
		{
			CLEAR_BIT(memory[ADDR_IO_IF], 2);
            destination = INTERRUPT_HANDLER_TIMER;
		}
		else if (CHECK_BIT(memory[ADDR_IO_IE], 3) && CHECK_BIT(memory[ADDR_IO_IF], 3))
		{
			CLEAR_BIT(memory[ADDR_IO_IF], 3);
            destination = INTERRUPT_HANDLER_SERIAL;
		}
		else if (CHECK_BIT(memory[ADDR_IO_IE], 4) && CHECK_BIT(memory[ADDR_IO_IF], 4))
		{
			CLEAR_BIT(memory[ADDR_IO_IF], 4);
            destination = INTERRUPT_HANDLER_JOYPAD;
		}
    }
    cpu_registers.sp--;
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.sp, pc_low);
    core_advance_cpu_clocks(4);
    cpu_registers.pc = destination;
    core_advance_cpu_clocks(4);
    interrupt_master_enable = false;
}

void interrupt_service_routine() {
    const uint8_t interrupt_enable = memory[ADDR_IO_IE];
    const uint8_t interrupt_flag = memory[ADDR_IO_IF];
    const bool interrupt_pending = ((interrupt_enable & interrupt_flag) & 0x1F) != 0;

    if (interrupt_pending) {
        if (cpu_halt_count == 1) {
            core_advance_cpu_clocks(4);
            cpu_halt_count = 0;
        }
        if (!interrupt_master_enable) {
            return;
        }

        interrupt_jump();
    }
}