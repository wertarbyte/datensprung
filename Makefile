MCU = attiny2313
F_CPU = 1000000
TARGET = datensprung
SRC = datensprung.c decoder.c
COMBINE_SRC = 0

include avr-tmpl.mk
