#define F_CPU 2000000

#include <avr/io.h>
#include <util/delay.h>

#include "print.h"
#include "usb_debug_only.h"

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
#define PSX_SELECT 0

#define PSX_SQUARE 15
#define PSX_CROSS 14
#define PSX_CIRCLE 13
#define PSX_TRIANGLE 12
#define PSX_R1 11
#define PSX_L1 10
#define PSX_R2 9
#define PSX_L2 8

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

static inline void handle_change(uint16_t js, uint16_t last_js, int btn)
{
	int was_released = last_js & (1 << btn);
	int is_pressed = !(js & (1 << btn));
	if (was_released && is_pressed) {
		switch(btn) {
		case PSX_UP: print("up"); break;
		case PSX_DOWN: print("down"); break;
		case PSX_LEFT: print("left"); break;
		case PSX_RIGHT: print("right"); break;
		case PSX_START: print("start"); break;
		case PSX_SELECT: print("select"); break;
		case PSX_SQUARE: print("square"); break;
		case PSX_CROSS: print("cross"); break;
		case PSX_CIRCLE: print("circle"); break;
		case PSX_TRIANGLE: print("triangle"); break;
		case PSX_L1: print("l1"); break;
		case PSX_L2: print("l2"); break;
		case PSX_R1: print("r1"); break;
		case PSX_R2: print("r2"); break;
		}
		print("\n");
	}
}

static uint16_t loop(uint16_t last_js)
{
	_delay_ms(16);
	uint16_t js = read_joystick();

	if (js != last_js) {
		handle_change(js, last_js, PSX_UP);
		handle_change(js, last_js, PSX_DOWN);
		handle_change(js, last_js, PSX_LEFT);
		handle_change(js, last_js, PSX_RIGHT);
		handle_change(js, last_js, PSX_START);
		handle_change(js, last_js, PSX_SELECT);
		handle_change(js, last_js, PSX_SQUARE);
		handle_change(js, last_js, PSX_CROSS);
		handle_change(js, last_js, PSX_CIRCLE);
		handle_change(js, last_js, PSX_TRIANGLE);
		handle_change(js, last_js, PSX_L1);
		handle_change(js, last_js, PSX_L2);
		handle_change(js, last_js, PSX_R1);
		handle_change(js, last_js, PSX_R2);
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
