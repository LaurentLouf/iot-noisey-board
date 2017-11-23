#include "config.h"
#include <EEPROM.h>
#include <Arduino.h>

bool writeOffsetToMemory( int8_t i_offset )
{
  if ( i_offset >= 0 && i_offset < 100 && EEPROM.read(addrOffset) != i_offset )
  {
    EEPROM.write(addrOffset, i_offset) ;
    return true ;
  }
  return false ;
}


bool writeSensitivityToMemory( int8_t i_sensitivity )
{
  if ( i_sensitivity >= 1 && i_sensitivity < 10 && EEPROM.read(addrSensitivity) != i_sensitivity )
  {
    EEPROM.write(addrSensitivity, i_sensitivity) ;
    return true ;
  }
  return false ;
}


int8_t readOffsetFromMemory( void )
{
  return EEPROM.read(addrOffset) ;
}


int8_t readSensitivityFromMemory( void )
{
  return EEPROM.read(addrSensitivity) ;
}
