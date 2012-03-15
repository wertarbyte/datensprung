struct ds_frame_t {
	uint8_t seq; /* sequence number */
	uint8_t cmd; /* command type */
	uint8_t data; /* payload */
	uint8_t chk; /* checksum (XOR all other fields) */
};
