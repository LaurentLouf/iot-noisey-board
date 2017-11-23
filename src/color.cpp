#include "color.h"

void HSBToRGB( int16_t i_hue, uint8_t i_saturation, uint8_t i_brightness, uint8_t *o_red, uint8_t *o_green, uint8_t *o_blue )
{
  if( i_saturation == 0 )
  {
    *o_red    = i_brightness ;
    *o_green  = i_brightness ;
    *o_blue   = i_brightness ;
  }
  else
  {
    float hh, p, q, t, ff;
    long  sector ;
    float saturation = i_saturation / 255.0, brightness = i_brightness / 255.0 ;

    hh = i_hue;
    hh /= 60.0;
    sector = (long)hh;

    ff = hh - sector;
    p = brightness  * (1.0 -  saturation);
    q = brightness  * (1.0 - ( saturation * ff));
    t = brightness  * (1.0 - ( saturation * (1.0 - ff)));

    switch(sector)
    {
      case 0:
        *o_red    = gamma8[(uint8_t) (brightness * 255)] ;
        *o_green  = gamma8[(uint8_t) (t * 255)] ;
        *o_blue   = gamma8[(uint8_t) (p * 255)] ;
      break;

      case 1:
        *o_red    = gamma8[(uint8_t) (q * 255)] ;
        *o_green  = gamma8[(uint8_t) (brightness * 255)] ;
        *o_blue   = gamma8[(uint8_t) (p * 255)] ;
      break;

      case 2:
        *o_red    = gamma8[(uint8_t) (p * 255)] ;
        *o_green  = gamma8[(uint8_t) (brightness * 255)] ;
        *o_blue   = gamma8[(uint8_t) (t * 255)] ;
      break;

      case 3:
        *o_red    = gamma8[(uint8_t) (p * 255)] ;
        *o_green  = gamma8[(uint8_t) (q * 255)] ;
        *o_blue   = gamma8[(uint8_t) (brightness * 255)] ;
      break;

      case 4:
        *o_red    = gamma8[(uint8_t) (t * 255)] ;
        *o_green  = gamma8[(uint8_t) (p * 255)] ;
        *o_blue   = gamma8[(uint8_t) (brightness * 255)] ;
      break;

      case 5:
      default:
        *o_red    = gamma8[(uint8_t) (brightness * 255)] ;
        *o_green  = gamma8[(uint8_t) (p * 255)] ;
        *o_blue   = gamma8[(uint8_t) (q * 255)] ;
      break;
    }
  }
}
