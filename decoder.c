#include <decoder.h>

#define BUFFER_BITS 8

static uint8_t buffer = 0;
static uint8_t buffer_pos = 0;
static uint8_t buffer_completed = 0;

static int8_t last_state = 0;

static void shift_bit(int8_t val) {
	buffer_completed = 0;
	if (val) {
		buffer |= 1<<buffer_pos;
	} else {
		buffer &= ~(1<<buffer_pos);
	}
	buffer_completed = (buffer_pos == BUFFER_BITS-1);
	buffer_pos = (buffer_pos+1)%BUFFER_BITS;
}

void decoder_reset(void) {
	buffer_completed = 0;
	buffer = 0;
	buffer_pos = 0;
	last_state = 0;
}

void decoder_feed(int8_t state) {
	/* if we are dropping back to neutral, check the last state */
	if (state == 0) {
		if (last_state == 1) {
			shift_bit(1);
		} else if (last_state == -1) {
			shift_bit(0);
		}
	}
	last_state = state;
}

uint8_t decoder_complete(void) {
	/* do we have a complete byte? */
	return buffer_completed;
}

uint8_t decoder_get(void) {
	return buffer;
}
