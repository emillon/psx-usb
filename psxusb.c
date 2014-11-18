#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>

#define PSX_PIN_DATA PC0
#define PSX_PIN_CMD PC1
#define PSX_PIN_ATT PC2
#define PSX_PIN_CLOCK PC3
#define DDR DDRC
#define PORT PORTC

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
	PORT |= (1 << pin);
}

static inline void signal_down(int pin)
{
	PORT &= ~(1 << pin);
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
}

static void read_joystick(struct joystick_state *js)
{
	signal_down(PSX_PIN_ATT);

	// ...

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
