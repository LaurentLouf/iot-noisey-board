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
  if ( i_sensitivity >= configSensitivityMin && i_sensitivity <= configSensitivityMax && EEPROM.read(addrSensitivity) != i_sensitivity )
  {
    EEPROM.write(addrSensitivity, i_sensitivity) ;
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
  if ( sensitivity >= configSensitivityMin && sensitivity <= configSensitivityMax )
    return sensitivity ;
  else
    return configSensitivityMin ;
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
