#define DS_FRAME_PAYLOAD_SIZE 4
struct ds_frame_t {
	uint8_t seq; /* sequence number */
	uint8_t cmd; /* command type */
	uint8_t data[DS_FRAME_PAYLOAD_SIZE]; /* payload */
	uint8_t chk; /* checksum (XOR all other fields) */
};
