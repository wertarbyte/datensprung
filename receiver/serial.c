#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"

void serial_write(char c) {
	while (!(UCSRA & (1 << UDRE)));
	UDR = c;
}
