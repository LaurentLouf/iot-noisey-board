#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include <Arduino.h>

#define SERVER_SIZE_BUFFER_DATA 10 

void sendPostRequest(char *i_hostURL, char *i_endPoint, char *i_message, int16_t *o_HTTPCode, String *o_payload) ;

void sendDataServer(char *i_hostURL, char *i_shortID, int32_t i_delayUpdateValue ) ;

void addDataSendServer(int16_t i_data) ;

#endif
