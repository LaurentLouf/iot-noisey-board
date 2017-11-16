#include "color.h"

void HSBToRGB( unsigned int i_hue, unsigned int i_saturation, unsigned int i_brightness, unsigned int *o_red, unsigned int *o_green, unsigned int *o_blue )
{
    if (i_saturation == 0)
    {
        // achromatic (grey)
        *o_red = *o_green = *o_blue = i_brightness;
    }
    else
    {
        unsigned int scaledHue = (i_hue * 6);
        unsigned int sector = scaledHue >> 8; // sector 0 to 5 around the color wheel
        unsigned int offsetInSector = scaledHue - (sector << 8);	// position within the sector
        unsigned int p = (i_brightness * ( 255 - i_saturation )) >> 8;
        unsigned int q = (i_brightness * ( 255 - ((i_saturation * offsetInSector) >> 8) )) >> 8;
        unsigned int t = (i_brightness * ( 255 - ((i_saturation * ( 255 - offsetInSector )) >> 8) )) >> 8;

        switch( sector ) {
        case 0:
            *o_red = i_brightness;
            *o_green = t;
            *o_blue = p;
            break;
        case 1:
            *o_red = q;
            *o_green = i_brightness;
            *o_blue = p;
            break;
        case 2:
            *o_red = p;
            *o_green = i_brightness;
            *o_blue = t;
            break;
        case 3:
            *o_red = p;
            *o_green = q;
            *o_blue = i_brightness;
            break;
        case 4:
            *o_red = t;
            *o_green = p;
            *o_blue = i_brightness;
            break;
        default:    // case 5:
            *o_red = i_brightness;
            *o_green = p;
            *o_blue = q;
            break;
        }
    }
}
