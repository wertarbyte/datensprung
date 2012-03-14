#include <decoder.h>

#define BUFFER_SIZE sizeof(struct datenwurf_packet_t)
#define BUFFER_BITS (BUFFER_SIZE*8)

static struct datenwurf_packet_t buffer = {0};
static uint8_t buffer_pos = 0;
static uint8_t buffer_completed = 0;

static int8_t last_state = 0;

static void shift_bit(int8_t val) {
	uint8_t *bbuf = (uint8_t*)&buffer;
	buffer_completed = 0;
	uint8_t *b = &bbuf[buffer_pos/8];
	if (val) {
		*b |= 1<<(buffer_pos%8);
	} else {
		*b &= ~(1<<(buffer_pos%8));
	}
	buffer_completed = (buffer_pos == BUFFER_BITS-1);
	buffer_pos = (buffer_pos+1)%BUFFER_BITS;
}

void decoder_reset(void) {
	buffer_completed = 0;
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

struct datenwurf_packet_t *decoder_get(void) {
	return &buffer;
}
