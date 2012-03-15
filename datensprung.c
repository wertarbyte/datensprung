#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ds_frame.h"
#include "decoder.h"

struct rcpin_t {
	uint8_t max;
	uint8_t min;
	uint8_t current;
};

static volatile struct rcpin_t dch = {0};
/* this variable is flagged when anough time has passed to reset the decoder */
static volatile uint8_t decoder_timeout = 0;

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

static void postpone_reset(void) {
	TCNT1 = 0;
}

static void process_packet(struct ds_frame_t *p) {
	static uint8_t last_seq = UINT8_MAX;
	/* does the checksum work out? */
	if (p->chk != ( (p->seq)^(p->cmd)^(p->data) )) return;
	/* have we already seen the packet? */
	if (p->seq == last_seq) return;
	last_seq = p->seq;
	switch (p->cmd) {
		case 0xF0: /* print character */
			serial_write(p->data);
			serial_write('\n');
			break;
	}
}

int main(void) {
	DDRD = (1<<PD5|1<<PD4|1<<PD3);
	UCSRB = (1<<RXEN | 1<<TXEN);
	UCSRC = (0<<USBS)|(3<<UCSZ0);
	UCSRA = (0<<U2X);
	UBRRL = 12;

	GIMSK = 1<<PCIE;
	PCMSK = 1<<PCINT0;

	/* Timer 0 used for measuring the PWM signal */
	TCCR0B = (1<<CS01|1<<CS00);
	/* Timer 1 to create a decoder timeout signal (1/100s) */
	TCCR1A = 1<<WGM12;
	TCCR1B = 1<<CS11 | 0<<CS10;
	OCR1A = 0x3D09;
	TIMSK = 1<<OCIE1A;

	PORTD &= ~(1<<PD5|1<<PD4|1<<PD3);

	sei();
	struct ds_frame_t pbuf = {0};
	while (1) {
		static int8_t last_state = 0;
		/* reset timeout counter? */
		if (decoder_timeout) {
			decoder_reset();
			decoder_timeout = 0;
			/* if we are stuck on the low/high level, the calibration might be off */
			if (last_state != 0) {
				dch.max = 0;
				dch.min = UINT8_MAX;
			}
#if RESET_DEBUG
			serial_write('\n');
			serial_write('R');
			serial_write('\n');
#endif
		}
		int8_t state = get_tri_state();
		/* if we detect a level change, we do not have to reset */
		if (state != last_state) postpone_reset();
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
		last_state = state;
		decoder_feed(state);
		if (decoder_get_frame(&pbuf)) {
			process_packet(&pbuf);
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

SIGNAL(TIMER1_COMPA_vect) {
	decoder_timeout = 1;
}
