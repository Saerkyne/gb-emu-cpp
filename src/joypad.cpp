#include "joypad.h"
#include "memory_bus.h"
#include "interrupts.h"

bool joy_button_a = false;
bool joy_button_b = false;
bool joy_button_select = false;
bool joy_button_start = false;
bool joy_dpad_right = false;
bool joy_dpad_left = false;
bool joy_dpad_up = false;
bool joy_dpad_down = false;

void joypad_reset()
{
	joy_button_a = false;
	joy_button_b = false;
	joy_button_select = false;
	joy_button_start = false;
	joy_dpad_right = false;
	joy_dpad_left = false;
	joy_dpad_up = false;
	joy_dpad_down = false;
}

uint8_t joypad_read_input()
{
	uint8_t joypad_register = memory[JOYPAD_REGISTER_ADDRESS];
	joypad_register |= 0b11000000; // Set upper 2 bits to 1
	joypad_register &= 0xF0; // Clear lower 4 bits
	bool action_buttons = CHECK_BIT(joypad_register, 5);
	bool direction_buttons = CHECK_BIT(joypad_register, 4);
	if (action_buttons && direction_buttons)
	{
		joypad_register |= 0x0F; // No buttons pressed
	}
	else
	{
		if (!action_buttons)
		{
			CHANGE_BIT(joypad_register, !joy_button_start, 3);
			CHANGE_BIT(joypad_register, !joy_button_select, 2);
			CHANGE_BIT(joypad_register, !joy_button_b, 1);
			CHANGE_BIT(joypad_register, !joy_button_a, 0);
		}
		else if (!direction_buttons)
		{
			CHANGE_BIT(joypad_register, !joy_dpad_down, 3);
			CHANGE_BIT(joypad_register, !joy_dpad_up, 2);
			CHANGE_BIT(joypad_register, !joy_dpad_left, 1);
			CHANGE_BIT(joypad_register, !joy_dpad_right, 0);
		}
	}

	memory[JOYPAD_REGISTER_ADDRESS] = joypad_register;
	return joypad_register;
}

void joypad_write_input(uint8_t flags)
{
	bool action_buttons = CHECK_BIT(flags, 5);
	bool direction_buttons = CHECK_BIT(flags, 4);
	uint8_t joypad_register = memory[JOYPAD_REGISTER_ADDRESS];
	CHANGE_BIT(joypad_register, action_buttons, 5);
	CHANGE_BIT(joypad_register, direction_buttons, 4);
	memory[JOYPAD_REGISTER_ADDRESS] = joypad_register;
}

#define button_check(button, pressed, flag)											\
{																					\
	if (button != pressed)													\
	{																				\
		if (!flag && button && !pressed)											\
		{																			\
			interrupt_raise_flag(INTERRUPT_FLAG_JOYPAD);							\
		}																			\
		button = pressed;															\
	}																				\
}

#define on_action_button(button, pressed)											\
{																					\
	bool action_buttons_flag = CHECK_BIT(memory[JOYPAD_REGISTER_ADDRESS], 5);		\
	button_check(button, pressed, action_buttons_flag);								\
}

#define on_direction_button(button, pressed)										\
{																					\
	bool direction_buttons_flag = CHECK_BIT(memory[JOYPAD_REGISTER_ADDRESS], 4);	\
	button_check(button, pressed, direction_buttons_flag);							\
}


void on_joypad_button_a(bool pressed)
{
	on_action_button(joy_button_a, pressed);
}

void on_joypad_button_b(bool pressed)
{
	on_action_button(joy_button_b, pressed);
}

void on_joypad_button_select(bool pressed)
{
	on_action_button(joy_button_select, pressed);
}

void on_joypad_button_start(bool pressed)
{
	on_action_button(joy_button_start, pressed);
}

void on_joypad_dpad_right(bool pressed)
{
	on_direction_button(joy_dpad_right, pressed);
}

void on_joypad_dpad_left(bool pressed)
{
	on_direction_button(joy_dpad_left, pressed);
}

void on_joypad_dpad_up(bool pressed)
{
	on_direction_button(joy_dpad_up, pressed);
}

void on_joypad_dpad_down(bool pressed)
{
	on_direction_button(joy_dpad_down, pressed);
}