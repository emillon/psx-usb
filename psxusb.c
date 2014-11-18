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

#define PSX_SQUARE 7
#define PSX_CROSS 6
#define PSX_CIRCLE 5
#define PSX_TRIANGLE 4
#define PSX_R1 3
#define PSX_L1 2
#define PSX_R2 1
#define PSX_L2 0

#define DELAY_CLOCK_US 50

struct joystick_state {
	int up;
	int down;
	int left;
	int right;
	int start;
	int select;
	int square;
	int cross;
	int circle;
	int triangle;
	int l1;
	int l2;
	int r1;
	int r2;
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

static void read_joystick(struct joystick_state *js)
{
	signal_down(PSX_PIN_ATT);

	transmit(0x01);
	transmit(0x42);
	transmit(0x00);
	uint8_t data1 = transmit(0x00);
	uint8_t data2 = transmit(0x00);

	signal_up(PSX_PIN_ATT);

#define F(f, data, mask) do { js->f = !(data & (1 << mask)); } while(0)
	F(up, data1, PSX_UP);
	F(down, data1, PSX_DOWN);
	F(left, data1, PSX_LEFT);
	F(right, data1, PSX_RIGHT);
	F(start, data1, PSX_START);
	F(select, data1, PSX_SELECT);
	F(square, data2, PSX_SQUARE);
	F(cross, data2, PSX_CROSS);
	F(circle, data2, PSX_CIRCLE);
	F(triangle, data2, PSX_TRIANGLE);
	F(l1, data2, PSX_L1);
	F(l2, data2, PSX_L2);
	F(r1, data2, PSX_R1);
	F(r2, data2, PSX_R2);
#undef F
}

static void loop(void)
{
	_delay_ms(16);
	static struct joystick_state js;
	read_joystick(&js);

#define P(btn) do { \
	print(#btn); \
	if(js.btn) { \
		print("[X]\n"); \
	} else { \
		print("[ ]\n"); \
	} \
} while(0)

	print("\n\nRead\n");
	P(up);
	P(down);
	P(left);
	P(right);
	P(start);
	P(select);
	P(square);
	P(cross);
	P(circle);
	P(triangle);
	P(l1);
	P(l2);
	P(r1);
	P(r2);
}

int main(void)
{
	setup();
	while(1){
		loop();
	}
	return 0;
}
