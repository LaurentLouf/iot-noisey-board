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
const int8_t  configSensitivityServerMin = 1 ;
const int8_t  configSensitivityServerMax = 10 ;
const int8_t  configDelayDataServerMin = 0 ;
const int8_t  configDelayDataServerMax = 2 ;
static const char configPasswordAP[] PROGMEM = "iot-makers";

const int32_t configDelayDataServer[3] = {300000, 60000, 10000} ;


bool writeOffsetToMemory( int8_t i_offset ) ;
bool writeSensitivityToMemory( int8_t i_sensitivity ) ;
bool writeDelayDataServerToMemory( int8_t i_iDelayDataServer ) ;
int8_t readOffsetFromMemory( void ) ;
int8_t readSensitivityFromMemory( void ) ;
int32_t readDelayDataServerFromMemory( void ) ;
int8_t  mapSensitivityServerToValue ( int8_t i_sensitivityServer ) ;
String getAPPassword() ;

#endif
