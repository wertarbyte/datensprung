#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "../common/ds_cmd.h"
#include "../common/ds_frame.h"
#include "process.h"
#include "serial.h"

static void cmd_serial_string(struct ds_frame_t *p) {
	static uint8_t last_str_seq = UINT8_MAX;
	/* this operation is not idem-potent, so we filter out retransmissions
	 * by using a sequence number as first byte of the payload
	 */
	if (p->data[0] == last_str_seq) return;
	last_str_seq = p->data[0];
	for (uint8_t i=1; i<DS_FRAME_PAYLOAD_SIZE && p->data[i]; i++) {
		serial_write(p->data[i]);
	}
	// serial_write('\n');
}

static void cmd_output_pins(struct ds_frame_t *p) {
	/* switch output pins */
	uint8_t levels = p->data[0]; // set these pins to high
	uint8_t mask = p->data[1]; // only change these pins
	PORTB |= (levels & mask);
	PORTB &= (~mask | levels);
}

void process_packet(struct ds_frame_t *p) {
	switch (p->cmd) {
		case DS_CMD_SERIAL_STRING: /* print characters */
			cmd_serial_string(p);
			break;
		case DS_CMD_OUTPUT_PINS:
			cmd_output_pins(p);
			break;
	}
}
