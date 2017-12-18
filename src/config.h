#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <Arduino.h>

const int8_t configPasswordMaxLength = 20 ;

const int32_t addrOffset            = 0 ;
const int32_t addrSensitivity       = 1 ;
const int32_t addrPasswordLength    = 2 ;
const int32_t addrPasswordValue     = 3 ;
const int32_t addrDelayDataServer   = addrPasswordValue + configPasswordMaxLength ;

const int8_t  configOffsetMin = 0 ;
const int8_t  configOffsetMax = 100 ;
const int8_t  configSensitivityMin = 1 ;
const int8_t  configSensitivityMax = 10 ;
const int32_t configDelayDataServerMin = 10000 ;
const int32_t configDelayDataServerMax = 300000 ;
static const char configPasswordAP[] PROGMEM = "iot-makers";


bool writeOffsetToMemory( int8_t i_offset ) ;
bool writeSensitivityToMemory( int8_t i_sensitivity ) ;
bool writeDelayDataServerToMemory( int32_t i_delayDataServer ) ;
int8_t readOffsetFromMemory( void ) ;
int8_t readSensitivityFromMemory( void ) ;
int32_t readDelayDataServerFromMemory( void ) ;
String getAPPassword() ;

#endif
