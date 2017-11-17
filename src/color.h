#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

void HSBToRGB( int16_t i_hue, uint8_t i_saturation, uint8_t i_brightness, uint8_t *o_red, uint8_t *o_green, uint8_t *o_blue ) ;

#endif
