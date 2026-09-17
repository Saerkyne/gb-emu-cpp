#include "interrupts.h"
#include "memory_bus.h"
#include "emulator_core.h"
#include "cpu.h"

extern uint8_t cpu_halt_count;

bool interrupt_master_enable = false;
uint8_t interrupt_enable_ime_delay = 0;

void interrupt_enable_write(uint8_t value) {
    memory[ADDR_IO_IE] = value;
}

void interrupt_flag_write(uint8_t value) {
    memory[ADDR_IO_IF] = value;
}

void interrupt_raise_flag(uint8_t flag) {
    memory[ADDR_IO_IF] |= flag;
}

void interrupt_jump_to(uint16_t addr) {
    core_advance_cpu_clocks(4);
    const uint8_t pc_high = (cpu_registers.pc & 0xFF00) >> 8;
    const uint8_t pc_low = cpu_registers.pc & 0xFF;
    cpu_registers.sp--;
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.sp, pc_high);
    cpu_registers.sp--;
    core_advance_cpu_clocks(4);
    memory_bus_write(cpu_registers.sp, pc_low);
    core_advance_cpu_clocks(4);
    cpu_registers.pc = addr;
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

        const bool enable_v_blank = CHECK_BIT(interrupt_enable, 0);
        const bool enable_lcd_stat = CHECK_BIT(interrupt_enable, 1);
        const bool enable_timer = CHECK_BIT(interrupt_enable, 2);
        const bool enable_serial = CHECK_BIT(interrupt_enable, 3);
        const bool enable_joypad = CHECK_BIT(interrupt_enable, 4);
        const bool flag_v_blank = CHECK_BIT(interrupt_flag, 0);
        const bool flag_lcd_stat = CHECK_BIT(interrupt_flag, 1);
        const bool flag_timer = CHECK_BIT(interrupt_flag, 2);
        const bool flag_serial = CHECK_BIT(interrupt_flag, 3);
        const bool flag_joypad = CHECK_BIT(interrupt_flag, 4);

        if (enable_v_blank && flag_v_blank) {
            CLEAR_BIT(memory[ADDR_IO_IF], 0);
            interrupt_jump_to(INTERRUPT_HANDLER_V_BLANK);
        }
        if (enable_lcd_stat && flag_lcd_stat) {
            CLEAR_BIT(memory[ADDR_IO_IF], 1);
            interrupt_jump_to(INTERRUPT_HANDLER_LCD_STATUS);
        }
        if (enable_timer && flag_timer) {
            CLEAR_BIT(memory[ADDR_IO_IF], 2);
            interrupt_jump_to(INTERRUPT_HANDLER_TIMER);
        }
        if (enable_serial && flag_serial) {
            CLEAR_BIT(memory[ADDR_IO_IF], 3);
            interrupt_jump_to(INTERRUPT_HANDLER_SERIAL);
        }
        if (enable_joypad && flag_joypad) {
            CLEAR_BIT(memory[ADDR_IO_IF], 4);
            interrupt_jump_to(INTERRUPT_HANDLER_JOYPAD);
        }
    }
}