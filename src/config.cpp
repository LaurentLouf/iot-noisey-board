#include "config.h"
#include <EEPROM.h>
#include <Arduino.h>

bool writeOffsetToMemory( int8_t i_offset )
{
  if ( i_offset >= configOffsetMin && i_offset <= configOffsetMax && EEPROM.read(addrOffset) != i_offset )
  {
    EEPROM.write(addrOffset, i_offset) ;
    return true ;
  }
  return false ;
}


bool writeSensitivityToMemory( int8_t i_sensitivity )
{
  if ( i_sensitivity >= configSensitivityServerMin && i_sensitivity <= configSensitivityServerMax && EEPROM.read(addrSensitivity) != i_sensitivity )
  {
    EEPROM.write(addrSensitivity, i_sensitivity) ;
    return true ;
  }
  return false ;
}


bool writeDelayDataServerToMemory( int8_t i_iDelayDataServer )
{
  if ( i_iDelayDataServer >= configDelayDataServerMin && i_iDelayDataServer <= configDelayDataServerMax && readDelayDataServerFromMemory() != configDelayDataServer[i_iDelayDataServer] )
  {
    EEPROM.write(addrDelayDataServer, i_iDelayDataServer) ;
    return true ;
  }
  return false ;
}


bool writeBrightnessToMemory( int8_t i_brightness )
{
  if ( i_brightness >= configBrightnessServerMin && i_brightness <= configBrightnessServerMax && EEPROM.read(addrBrightness) != i_brightness )
  {
    EEPROM.write(addrBrightness, i_brightness) ;
    return true ;
  }
  return false ;
}


int8_t readOffsetFromMemory( void )
{
  int8_t offset = EEPROM.read(addrOffset) ;
  if ( offset >= configOffsetMin && offset <= configOffsetMax )
    return offset ;
  else
    return configOffsetMin ;
}


int8_t readSensitivityFromMemory( void )
{
  int8_t sensitivity = EEPROM.read(addrSensitivity) ;
  if ( sensitivity >= configSensitivityServerMin && sensitivity <= configSensitivityServerMax )
    return mapSensitivityServerToValue(sensitivity) ;
  else
    return mapSensitivityServerToValue(configSensitivityServerMax) ;
}


int32_t readDelayDataServerFromMemory( void )
{
  int8_t iDelayDataServer = EEPROM.read(addrDelayDataServer) ;

  if ( iDelayDataServer >= configDelayDataServerMin && iDelayDataServer <= configDelayDataServerMax )
  {
    return configDelayDataServer[iDelayDataServer] ;
  }
  else
  {
    return configDelayDataServer[configDelayDataServerMin] ;
  }
}


uint8_t readBrightnessFromMemory( void )
{
  int8_t brightness = EEPROM.read(addrBrightness) ;
  if ( brightness >= configBrightnessServerMin && brightness <= configBrightnessServerMax )
    return mapBrightnessServerToValue(brightness) ;
  else
    return mapBrightnessServerToValue(configBrightnessServerMin + 1) ;
}


int8_t  mapSensitivityServerToValue ( int8_t i_sensitivityServer )
{
  return (i_sensitivityServer * -1) + 11 ;
}

uint8_t  mapBrightnessServerToValue ( int8_t i_brightnessServer )
{
  if( i_brightnessServer == 0 )
    return 0 ;
  else
    return 48 + 16 * i_brightnessServer ;
}

String getAPPassword()
{
  String password = "" ;
  int8_t lengthPasswordEEPROM = EEPROM.read(addrPasswordLength), iPasswordChar ;

  if ( lengthPasswordEEPROM > 0 )
  {
    for ( iPasswordChar = 0 ; iPasswordChar < configPasswordMaxLength ; iPasswordChar++ )
    {
      char passwordChar = EEPROM.read(addrPasswordValue + iPasswordChar) ;
      if ( passwordChar != '\0' )
        password += passwordChar ;
      else
        break ;
    }
  }
  else
    password = FPSTR(configPasswordAP) ;

  return password ;
}
