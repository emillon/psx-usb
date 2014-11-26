#define F_CPU 2000000

#include <avr/io.h>
#include <util/delay.h>

#include "usb_keyboard.h"

#define PSX_PIN_DATA PC0
#define PSX_PIN_CMD PC1
#define PSX_PIN_ATT PC2
#define PSX_PIN_CLOCK PC3
#define DDR DDRC
#define PORT PORTC
#define PIN PINC

#define PSX_LEFT 7
#define PSX_DOWN 6
#define PSX_RIGHT 5
#define PSX_UP 4
#define PSX_START 3
#define PSX_RESERVED2 2
#define PSX_RESERVED1 1
#define PSX_SELECT 0

#define PSX_SQUARE 15
#define PSX_CROSS 14
#define PSX_CIRCLE 13
#define PSX_TRIANGLE 12
#define PSX_R1 11
#define PSX_L1 10
#define PSX_R2 9
#define PSX_L2 8

#define KEY_RESERVED(i) ((i) == PSX_RESERVED1 || (i) == PSX_RESERVED2)

static const uint8_t mapping[] = {
	[PSX_LEFT] = KEY_LEFT,
	[PSX_DOWN] = KEY_DOWN,
	[PSX_RIGHT] = KEY_RIGHT,
	[PSX_UP] = KEY_UP,
	[PSX_START] = KEY_ENTER,
	[PSX_SELECT] = KEY_ESC,
	[PSX_SQUARE] = KEY_H,
	[PSX_CROSS] = KEY_J,
	[PSX_CIRCLE] = KEY_K,
	[PSX_TRIANGLE] = KEY_L,
	[PSX_L1] = KEY_Y,
	[PSX_L2] = KEY_U,
	[PSX_R1] = KEY_I,
	[PSX_R2] = KEY_O,
};

#define DELAY_CLOCK_US 50

static inline void configure_pin_input(int pin)
{
	DDR &= ~(1 << pin);
}

static inline void configure_pin_output(int pin)
{
	DDR |= (1 << pin);
}

static inline void signal_up(int pin)
{
	PORT |= (1 << pin);
}

static inline void signal_down(int pin)
{
	PORT &= ~(1 << pin);
}

static inline int signal_read(int pin)
{
	return (PIN & (1 << pin));
}

static void setup(void)
{
	configure_pin_input(PSX_PIN_DATA);
	signal_up(PSX_PIN_DATA); // pullup

	configure_pin_output(PSX_PIN_CMD);

	configure_pin_output(PSX_PIN_ATT);
	signal_up(PSX_PIN_ATT);

	configure_pin_output(PSX_PIN_CLOCK);
	signal_up(PSX_PIN_CLOCK);

	usb_init();
	while (!usb_configured()) /* wait */ ;

	_delay_ms(1000);
}

static uint8_t transmit(uint8_t in)
{
	uint8_t out = 0;
	for (int i = 0; i < 8 ; i++) {
		int bit_in = in & (1 << i);
		if (bit_in) {
			signal_up(PSX_PIN_CMD);
		} else {
			signal_down(PSX_PIN_CMD);
		}
		_delay_us(DELAY_CLOCK_US);
		signal_down(PSX_PIN_CLOCK);
		_delay_us(DELAY_CLOCK_US);
		int bit_out = signal_read(PSX_PIN_DATA);
		if (bit_out) {
			out |= (1 << i);
		} else {
			out &= ~(1 << i);
		}
		signal_up(PSX_PIN_CLOCK);
		_delay_us(DELAY_CLOCK_US);
	}
	return out;
}

static uint16_t read_joystick(void)
{
	signal_down(PSX_PIN_ATT);

	transmit(0x01);
	transmit(0x42);
	transmit(0x00);
	uint8_t data1 = transmit(0x00);
	uint8_t data2 = transmit(0x00);

	signal_up(PSX_PIN_ATT);

	return (data2 << 8) | data1;
}

static inline uint8_t keypress_replace(uint8_t from, uint8_t to)
{
	for (int i = 0; i < 6; i++) {
		if (keyboard_keys[i] == from) {
			keyboard_keys[i] = to;
			return 0;
		}
	}
	return -1;
}

// Store a new key in keyboard_keys.
// Return -1 on error.
static uint8_t keypress_add(uint8_t key)
{
	return keypress_replace(0, key);
}

// Remove a key from keyboard_keys.
// Return -1 on error.
static uint8_t keypress_remove(uint8_t key)
{
	return keypress_replace(key, 0);
}

static uint16_t loop(uint16_t last_js)
{
	_delay_ms(16);
	uint16_t js = read_joystick();

	if (js != last_js) {
		for (int i = 0; i < 16 ; i++) {
			if (KEY_RESERVED(i)) {
				continue;
			}
			int was_pressed = !(last_js & (1 << i));
			int is_pressed = !(js & (1 << i));

			if (is_pressed && !was_pressed) {
				keypress_add(mapping[i]);
			}
			if (was_pressed && !is_pressed) {
				keypress_remove(mapping[i]);
			}
		}
		usb_keyboard_send();
	}

	return js;
}

int main(void)
{
	setup();
	uint16_t js = 0;
	while(1){
		js = loop(js);
	}
	return 0;
}
