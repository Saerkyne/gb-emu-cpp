#pragma once
#include <stdint.h>

const uint16_t JOYPAD_REGISTER_ADDRESS = 0xFF00;

uint8_t joypad_read_input();
void joypad_write_input(uint8_t flags);

void on_joypad_button_a(bool pressed);
void on_joypad_button_b(bool pressed);
void on_joypad_button_select(bool pressed);
void on_joypad_button_start(bool pressed);
void on_joypad_dpad_right(bool pressed);
void on_joypad_dpad_left(bool pressed);
void on_joypad_dpad_up(bool pressed);
void on_joypad_dpad_down(bool pressed);