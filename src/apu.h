#pragma once
#include <stdint.h>

#define TO_FLOAT_SAMPLE(value) (((float)value / 15.f) * -2.f) + 1.f

#define APU_VOLUME_LEFT ((audio_registers->nr5[0] & 0b01110000) >> 4)
#define APU_VOLUME_RIGHT (audio_registers->nr5[0] & 0b00000111)

#define APU_STATUS_SQ1 (audio_registers->nr5[2] & 0b00000001)
#define APU_STATUS_SQ2 ((audio_registers->nr5[2] & 0b00000010) >> 1)
#define APU_STATUS_WAVE ((audio_registers->nr5[2] & 0b00000100) >> 2)
#define APU_STATUS_NOISE ((audio_registers->nr5[2] & 0b00001000) >> 3)

#define APU_ENABLE_SQ1_LEFT ((audio_registers->nr5[1] & 0b00010000) >> 4)
#define APU_ENABLE_SQ1_RIGHT (audio_registers->nr5[1] & 0b00000001)
#define APU_ENABLE_SQ2_LEFT ((audio_registers->nr5[1] & 0b00100000) >> 5)
#define APU_ENABLE_SQ2_RIGHT ((audio_registers->nr5[1] & 0b00000010) >> 1)
#define APU_ENABLE_WAVE_LEFT ((audio_registers->nr5[1] & 0b01000000) >> 6)
#define APU_ENABLE_WAVE_RIGHT ((audio_registers->nr5[1] & 0b00000100) >> 2)
#define APU_ENABLE_NOISE_LEFT ((audio_registers->nr5[1] & 0b10000000) >> 7)
#define APU_ENABLE_NOISE_RIGHT ((audio_registers->nr5[1] & 0b00001000) >> 3)

#define APU_POWER_STATUS (audio_registers->nr5[2] >> 7)

#define SET_APU_STATUS_SQ1(value) audio_registers->nr5[2] = ((audio_registers->nr5[2] & 0b11111110) | (value))
#define SET_APU_STATUS_SQ2(value) audio_registers->nr5[2] = ((audio_registers->nr5[2] & 0b11111101) | ((value) << 1))
#define SET_APU_STATUS_WAVE(value) audio_registers->nr5[2] = ((audio_registers->nr5[2] & 0b11111011) | ((value) << 2))
#define SET_APU_STATUS_NOISE(value) audio_registers->nr5[2] = ((audio_registers->nr5[2] & 0b11110111) | ((value) << 3))

#define SQ1_DAC_POWER ((audio_registers->nr1[2] >> 4) > 0)
#define SQ1_SWEEP_PERIOD ((audio_registers->nr1[0] & 0b01110000) >> 4)
#define SQ1_SWEEP_NEGATE ((audio_registers->nr1[0] & 0b00001000) >> 3)
#define SQ1_SWEEP_SHIFT (audio_registers->nr1[0] & 0b00000111)
#define SQ1_DUTY ((audio_registers->nr1[0] & 0b11000000) >> 6)
#define SQ1_LENGTH_LOAD ((audio_registers->nr1[1] & 0b00111111))
#define SQ1_ENV_VOLUME (audio_registers->nr1[2] >> 4)
#define SQ1_ENV_ADD ((audio_registers->nr1[2] & 0b00001000) >> 3)
#define SQ1_ENV_PERIOD (audio_registers->nr1[2] & 0b00000111)
#define SQ1_FREQUENCY_LSB (audio_registers->nr1[3])
#define SQ1_TRIGGER ((audio_registers->nr1[4] & 0b10000000) >> 7)
#define SQ1_LENGTH_ENABLE ((audio_registers->nr1[4] & 0b01000000) >> 6)
#define SQ1_FREQUENCY_MSB (audio_registers->nr1[4] & 0b00000111)
#define SQ1_FREQUENCY ((SQ1_FREQUENCY_MSB << 8) | SQ1_FREQUENCY_LSB)
#define SET_SQ1_TRIGGER(value) audio_registers->nr1[4] = ((audio_registers->nr1[4] & ~(1 << 7)) | (value << 7))
#define SET_SQ1_LENGTH_LOAD(value) audio_registers->nr1[1] = ((audio_registers->nr1[1] & 0b11000000) | value)

#define SQ2_DUTY ((audio_registers->nr2[0] & 0b11000000) >> 6)
#define SQ2_LENGTH_LOAD ((audio_registers->nr2[1] & 0b00111111))
#define SQ2_ENV_VOLUME (audio_registers->nr2[2] >> 4)
#define SQ2_ENV_ADD ((audio_registers->nr2[2] & 0b00001000) >> 3)
#define SQ2_ENV_PERIOD (audio_registers->nr2[2] & 0b00000111)
#define SQ2_FREQUENCY_LSB (audio_registers->nr2[3])
#define SQ2_TRIGGER ((audio_registers->nr2[4] & 0b10000000) >> 7)
#define SQ2_LENGTH_ENABLE ((audio_registers->nr2[4] & 0b01000000) >> 6)
#define SQ2_FREQUENCY_MSB (audio_registers->nr2[4] & 0b00000111)
#define SQ2_FREQUENCY ((SQ2_FREQUENCY_MSB << 8) | SQ2_FREQUENCY_LSB)
#define SET_SQ2_TRIGGER(value) audio_registers->nr2[4] = ((audio_registers->nr2[4] & ~(1 << 7)) | (value << 7))
#define SET_SQ2_LENGTH_LOAD(value) audio_registers->nr2[1] = ((audio_registers->nr2[1] & 0b11000000) | value)

#define WAVE_DAC_POWER ((audio_registers->nr3[0] & 0b10000000) >> 7)
#define WAVE_LENGTH_LOAD (audio_registers->nr3[1])
#define WAVE_VOLUME_CODE ((audio_registers->nr3[2] & 0b01100000) >> 5)
#define WAVE_FREQUENCY_LSB (audio_registers->nr3[3])
#define WAVE_TRIGGER ((audio_registers->nr3[4] & 0b10000000) >> 7)
#define WAVE_LENGTH_ENABLE ((audio_registers->nr3[4] & 0b01000000) >> 6)
#define WAVE_FREQUENCY_MSB (audio_registers->nr3[4] & 0b00000111)
#define WAVE_FREQUENCY ((WAVE_FREQUENCY_MSB << 8) | WAVE_FREQUENCY_LSB)
#define SET_WAVE_TRIGGER(value) audio_registers->nr3[4] = ((audio_registers->nr3[4] & ~(1 << 7)) | (value << 7))
#define SET_WAVE_LENGTH_LOAD(value) audio_registers->nr3[1] = value

#define NOISE_LENGTH_LOAD (audio_registers->nr4[1] & 0b00111111)
#define NOISE_ENV_VOLUME (audio_registers->nr4[2] >> 4)
#define NOISE_ENV_ADD ((audio_registers->nr4[2] & 0b00001000) >> 3)
#define NOISE_ENV_PERIOD (audio_registers->nr4[2] & 0b00000111)
#define NOISE_CLOCK_SHIFT (audio_registers->nr4[3] >> 4)
#define NOISE_WIDTH_MODE ((audio_registers->nr4[3] & 0b00001000) >> 3)
#define NOISE_DIVISOR_CODE (audio_registers->nr4[3] & 0b00000111)
#define NOISE_TRIGGER (audio_registers->nr4[4] >> 7)
#define NOISE_LENGTH_ENABLE ((audio_registers->nr4[4] & 0b01000000) >> 6)
#define SET_NOISE_TRIGGER(value) audio_registers->nr4[4] = ((audio_registers->nr4[4] & ~(1 << 7)) | (value << 7))
#define SET_NOISE_LENGTH_LOAD(value) (audio_registers->nr4[1] = value & 0b00111111)


struct audio_register_set {
	uint8_t nr1[5]; // NR10, NR11, NR12, NR13, NR14
	uint8_t nr2[5]; // NR21, NR22, NR23, NR24
	uint8_t nr3[5]; // NR30, NR31, NR32, NR33, NR34
	uint8_t nr4[5]; // NR41, NR42, NR43, NR44
	uint8_t nr5[3]; // NR50, NR51, NR52
};

void apu_init();
void apu_reset();
void apu_tick(uint8_t clocks);
void apu_shutdown();

void apu_on_frame_end();
void apu_counter_tick();

uint8_t apu_read(uint16_t addr);
void apu_write(uint16_t addr, uint8_t value);