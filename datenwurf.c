#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "decoder.h"

struct rcpin_t {
	uint8_t max;
	uint8_t min;
	uint8_t current;
};

static volatile struct rcpin_t dch = {0};

static void set_duration(uint8_t duration) {
	/* auto-calibrate */
	if (duration < dch.min || dch.min == 0) dch.min = duration;
	if (duration > dch.max) dch.max = duration;
	dch.current = duration;
}

static int8_t get_tri_state(void) {
	uint8_t center = dch.min+((dch.max-dch.min)/2);
	if (dch.current > center) {
		return (dch.max - dch.current < dch.current - center) ? 1 : 0;
	} else {
		return (dch.current - dch.min < center - dch.current) ? -1 : 0;
	}
}

static void serial_write(char c) {
	while (!(UCSRA & (1 << UDRE)));
	UDR = c;
}

int main(void) {
	DDRD = (1<<PD5|1<<PD4|1<<PD3);
	UCSRB = (1<<RXEN | 1<<TXEN);
	UCSRC = (0<<USBS)|(3<<UCSZ0);
	UCSRA = (0<<U2X);
	UBRRL = 12;

	GIMSK = 1<<PCIE;
	PCMSK = 1<<PCINT0;

	TCCR0B = (1<<CS01|1<<CS00);

	PORTD &= ~(1<<PD5|1<<PD4|1<<PD3);

	sei();
	while (1) {
		int8_t state = get_tri_state();
		switch (state) {
			case -1:
				PORTD &= ~(1<<PD3|1<<PD4);
				PORTD |= 1<<PD5;
				break;
			case 0:
				PORTD &= ~(1<<PD3|1<<PD5);
				PORTD |= 1<<PD4;
				break;
			case 1:
				PORTD &= ~(1<<PD4|1<<PD5);
				PORTD |= 1<<PD3;
				break;
		}
		decoder_feed(state);
		if (decoder_complete()) {
			uint8_t b = decoder_get();
			decoder_reset();
			serial_write(':');
			serial_write(b);
			serial_write('\n');
		}
	}
}

SIGNAL(PCINT_vect) {
	/* measure the time between impulses */
	if (PINB & 1<<PB0) {
		/* the pin went up, so we reset the clock */
		TCNT0=0;
	} else {
		/* the pin went down, so we save the clock value */
		set_duration(TCNT0);
	}
}
