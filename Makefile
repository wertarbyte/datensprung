MCU = attiny2313
F_CPU = 1000000
TARGET = datenwurf
SRC = datenwurf.c decoder.c
COMBINE_SRC = 0

include avr-tmpl.mk
