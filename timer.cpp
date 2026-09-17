#include "timer.h"
#include "memory_bus.h"

extern const uint8_t cpu_halt_count;
const uint16_t timer_tac_edge_bits[4] = { 9, 3, 5, 7 };

gb_timer_registers* timer_registers = (gb_timer_registers*)(memory + 0xFF04); // Timer registers start at 0xFF04

uint16_t timer_internal_sysclk = 0;
uint8_t timer_interrupt_delay = 0;

void timer_init() {
    timer_internal_sysclk = 0xABCC;
    timer_registers->timer_div = 0xAB;
    timer_registers->timer_tac = 0;
    timer_registers->timer_tima = 0;
    timer_registers->timer_tma = 0;
}

void timer_check_clock_edges(const uint16_t prev_sysclk) {
    const uint8_t div_prev = timer_registers->timer_div;
    const bool div_bit_4_prev = (timer_registers->timer_div >> 4) & 1;

    // Expose upper 8 bits
    timer_registers->timer_div = timer_internal_sysclk >> 8;

    const bool div_bit_4_now = (timer_registers->timer_div >> 4) & 1;
    if (div_bit_4_prev && !div_bit_4_now) {
        // Falling edge detected on DIV bit 4
        // TODO: Tick APU counter here
    }

    const bool tac_timer_enable = CHECK_BIT(timer_registers->timer_tac, 2);
    if (!tac_timer_enable) {
        return;
    }

    const uint8_t tac_clock_select = timer_registers->timer_tac & 0b00000011;
    const uint16_t tac_divider_bit = timer_tac_edge_bits[tac_clock_select];
    const bool prev_edge = CHECK_BIT(prev_sysclk, tac_divider_bit);
    const bool curr_edge = CHECK_BIT(timer_internal_sysclk, tac_divider_bit);
    if (prev_edge && !curr_edge) {
        // Falling edge detected on the selected TAC divider bit
        timer_tick_tima();
    }
}

void timer_increase_div(const uint8_t cycles) {
    for (uint8_t c = 0; c < cycles; c++) {
        const uint16_t prev_sysclk = timer_internal_sysclk;
        timer_internal_sysclk++;

        timer_check_clock_edges(prev_sysclk);
    }
}

void timer_advance_clocks(const uint8_t cycles) {
    if (timer_interrupt_delay > 0) {
        for (uint8_t c = 0; c < cycles; c++) {
            timer_interrupt_delay--;
            if (timer_interrupt_delay == 0) {
                // TODO: Raise timer interrupt flag here
                break;
            }
        }
    }

    if (cpu_halt_count == 2) {
        // CPU is in STOP state, do not increase DIV
        return;
    }
    timer_increase_div(cycles);
}

void timer_on_div_write(const uint8_t _value) {
    const uint16_t prev_sysclk = timer_internal_sysclk;
    timer_internal_sysclk = 0x0000;
    timer_check_clock_edges(prev_sysclk);
}

void timer_tick_tima() {
    if (timer_registers->timer_tima < 255) {
        timer_registers->timer_tima++;
    } else {
        // TIMA overflow
        timer_registers->timer_tima = timer_registers->timer_tma;
        timer_interrupt_delay = 4; // Delay before the timer interrupt is triggered
    }
}