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
#define PORTOUT PORTC
#define PORTIN PINC


#define DELAY_CLOCK_US 50

struct joystick_state {
	int up;
	int down;
	int left;
	int right;
	int square;
	int cross;
	int circle;
	int triangle;
};

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
	PORTOUT |= (1 << pin);
}

static inline void signal_down(int pin)
{
	PORTOUT &= ~(1 << pin);
}

static inline int signal_read(int pin)
{
	return (PORTIN & (1 << pin));
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

static void transmit(uint8_t in, uint8_t *out)
{
	*out = 0;
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
			*out |= (1 << i);
		} else {
			*out &= ~(1 << i);
		}
		signal_up(PSX_PIN_CLOCK);
		_delay_us(DELAY_CLOCK_US);
	}
}

static void read_joystick(struct joystick_state *js)
{
	signal_down(PSX_PIN_ATT);

	print("Read\n");
	uint8_t cmd = 0x01;
	uint8_t byte = 0x00;
	transmit(cmd, &byte);
	phex(cmd); print(" -> "); phex(byte); print("\n");
	cmd = 0x42;
	transmit(cmd, &byte);
	phex(cmd); print(" -> "); phex(byte); print("\n");
	cmd = 0x00;
	transmit(cmd, &byte);
	phex(cmd); print(" -> "); phex(byte); print("\n");
	cmd = 0x00;
	transmit(cmd, &byte);
	phex(cmd); print(" -> "); phex(byte); print("\n");
	cmd = 0x00;
	transmit(cmd, &byte);
	phex(cmd); print(" -> "); phex(byte); print("\n");

	signal_up(PSX_PIN_ATT);
}

static void loop(void)
{
	_delay_ms(1000);
	static struct joystick_state js;
	read_joystick(&js);
}

int main(void)
{
	setup();
	while(1){
		loop();
	}
	return 0;
}
