#pragma once
#include <stdint.h>

struct gb_cpu_instruction {
	const char* dissassembly; // Human-readable assembly instruction
	uint8_t operand_length;	  // Length of the operand in bytes
	void* execute;			  // Pointer to the function that executes this instruction
};

struct gb_cpu_pre_cb_instruction {
	const char* dissassembly; // Human-readable assembly instruction
	void* execute;			  // Pointer to the function that executes this instruction
};

extern const gb_cpu_instruction instructions[256];			 // Array of CPU instructions, indexed by opcode
extern const gb_cpu_pre_cb_instruction cb_instructions[256]; // Array of CPU instructions with pre-callback, indexed by opcode