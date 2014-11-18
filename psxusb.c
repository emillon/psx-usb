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

static void setup(void)
{
	DDR &= ~(1 << PSX_PIN_DATA);
	PORT |= (1 << PSX_PIN_DATA); // pullup

	DDR |= (1 << PSX_PIN_CMD);

	DDR |= (1 << PSX_PIN_ATT);
	PORT |= (1 << PSX_PIN_ATT);

	DDR |= (1 << PSX_PIN_CLOCK);
	PORT |= (1 << PSX_PIN_CLOCK);
}

static void loop(void)
{
	_delay_ms(1000);
}

int main(void)
{
	setup();
	while(1){
		loop();
	}
	return 0;
}
