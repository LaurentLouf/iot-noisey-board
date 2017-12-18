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
const int32_t addrBrightness        = addrDelayDataServer + 1 ;


const int8_t  configOffsetMin = 0 ;
const int8_t  configOffsetMax = 100 ;
const int8_t  configSensitivityServerMin = 1 ;
const int8_t  configSensitivityServerMax = 10 ;
const int8_t  configDelayDataServerMin = 0 ;
const int8_t  configDelayDataServerMax = 2 ;
const int8_t  configBrightnessServerMin = 0 ;
const int8_t  configBrightnessServerMax = 10 ;
static const char configPasswordAP[] PROGMEM = "iot-makers";

const int32_t configDelayDataServer[3] = {300000, 60000, 10000} ;


bool writeOffsetToMemory( int8_t i_offset ) ;
bool writeSensitivityToMemory( int8_t i_sensitivity ) ;
bool writeDelayDataServerToMemory( int8_t i_iDelayDataServer ) ;
bool writeBrightnessToMemory( int8_t i_brightness ) ;
int8_t readOffsetFromMemory( void ) ;
int8_t readSensitivityFromMemory( void ) ;
int32_t readDelayDataServerFromMemory( void ) ;
uint8_t readBrightnessFromMemory( void ) ;
int8_t  mapSensitivityServerToValue ( int8_t i_sensitivityServer ) ;
uint8_t  mapBrightnessServerToValue ( int8_t i_brightnessServer ) ;
String getAPPassword() ;

#endif
