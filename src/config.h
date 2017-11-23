#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

const int32_t addrOffset = 0 ;
const int32_t addrSensitivity = 1 ;

bool writeOffsetToMemory( int8_t i_offset ) ;
bool writeSensitivityToMemory( int8_t i_sensitivity ) ;
int8_t readOffsetFromMemory( void ) ;
int8_t readSensitivityFromMemory( void ) ;

#endif
