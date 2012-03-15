#include <stdlib.h>
#include <stdint.h>

#define DECODER_BUFFER_SIZE 4

struct datensprung_packet_t {
	uint8_t seq; /* sequence number */
	uint8_t cmd; /* command type */
	uint8_t data; /* payload */
	uint8_t chk; /* checksum (XOR all other fields) */
};

void decoder_reset(void);
void decoder_feed(int8_t state);
uint8_t decoder_complete(void);
struct datensprung_packet_t *decoder_get(void);
