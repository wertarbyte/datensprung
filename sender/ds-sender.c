#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../common/ds_cmd.h"
#include "../common/ds_frame.h"

/* this pin must be connected to
 * the RC.
 * Two identical resistors must also
 * be placed between this connector
 * and VCC/GND to pull the voltage to
 * a middle value when not driven
 * by the microcontroller:
 *          RC o
 *             |
 * GND o--[R2]-+-[R1]--o VCC
 *             |
 *  R1==R2     o µC
 */
#define OUTPUT_DDR  DDRA
#define OUTPUT_PORT PORTA
#define OUTPUT_BIT  PA0

#define BIT_DELAY    50
#define FRAME_DELAY 100

static void set_tri_state(int8_t s) {
	switch (s) {
		case 0: /* neutral */
			OUTPUT_DDR &= ~(1<<OUTPUT_BIT);
			break;
		case 1: /* max */
			OUTPUT_DDR |= (1<<OUTPUT_BIT);
			OUTPUT_PORT |= (1<<OUTPUT_BIT);
			break;
		case -1: /* min */
			OUTPUT_DDR |= (1<<OUTPUT_BIT);
			OUTPUT_PORT &= ~(1<<OUTPUT_BIT);
			break;
	}
}

static void transmit_byte(uint8_t b) {
	uint8_t i;
	for (i=0; i<8; i++) {
		if (b & 1<<i) {
			set_tri_state(1);
		} else {
			set_tri_state(-1);
		}
		_delay_ms(BIT_DELAY);
		set_tri_state(0);
		_delay_ms(BIT_DELAY);
	}
}

static void send_frame(uint8_t cmd, uint8_t *payload, uint8_t l) {
	uint8_t chk = 0;
	chk ^= cmd;
	transmit_byte(cmd);
	uint8_t i;
	/* transmit payload and/or filler bytes */
	for (i=0; i<DS_FRAME_PAYLOAD_SIZE; i++) {
		uint8_t b = (i>=l ? 0x00 : payload[i]);
		chk ^= b;
		transmit_byte(b);
	}
	/* send checksum */
	transmit_byte(chk);
	/* and wait for the frame delay */
	_delay_ms(FRAME_DELAY);
}

int main(void) {
	/* we like to transmit PINB to the other side, enable pullups */
	PORTB |= (1<<PB0 | 1<<PB1 | 1<<PB2 | 1<<PB3 | 1<<PB4 | 1<<PB5 | 1<<PB6 | 1<<PB7);

	while (1) {
		/* check switches and construct frame */
		uint8_t sw_payload[2];
		sw_payload[0] = PINB; // read values
		sw_payload[1] = 0xFF; // we do not mask any values

		send_frame(DS_CMD_OUTPUT_PINS, sw_payload, 2);
	}
}
