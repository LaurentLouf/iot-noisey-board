#include "server.h"
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>


// Circular buffer to send data to the server
int16_t noiseBufferServer[SERVER_SIZE_BUFFER_DATA] ;
uint8_t iReadNoiseBufferServer  = 0 ;
uint8_t iWriteNoiseBufferServer = 0 ;

const char* fingerprint = "FB:AD:09:02:B3:19:A5:F7:5F:27:A8:93:16:98:D7:A0:9C:27:D9:AE";


/**
 * \fn void sendPostRequest(char *i_hostURL, char *i_endPoint, char *i_message, int16_t *o_HTTPCode, String *o_payload)
 * \param[in] i_hostURL URL of the server
 * \param[in] i_endPoint End point to send the message to
 * \param[in] i_message Message to be sent in JSON format
 * \param[out] o_HTTPCode HTTP code returned by the server
 * \param[out] o_payload Payload received from the server
 * \brief Send a message (JSON) to an URL using HTTP POST
*/
void sendPostRequest(char *i_hostURL, char *i_endPoint, char *i_message, int16_t *o_HTTPCode, String *o_payload)
{
  char completeURL[256] ;
  HTTPClient http ;

  // Perform the POST
  http.begin(i_hostURL, 443, i_endPoint, true, fingerprint);
  http.addHeader("Content-Type", "application/json");
  *o_HTTPCode = http.POST(i_message);
  if ( o_payload != NULL )
    *o_payload  = http.getString();
  http.end();
}

/**
 * \fn void sendDataServer(char *i_hostURL, char *i_shortID, int32_t i_delayUpdateValue )
 * \param[in] i_hostURL URL of the server
 * \param[in] i_shortID ID of the device in short version
 * \param[in] i_delayUpdateValue Delay between two updates of the value displayed by the device
 * \brief Send data to the server
*/
void sendDataServer(char *i_hostURL, char *i_shortID, int32_t i_delayUpdateValue )
{
  bool firstMessage = true ;
  int16_t HTTPCode  = 0, nbElements = 0 ;
  int32_t millisSend ;

  nbElements = iWriteNoiseBufferServer - iReadNoiseBufferServer ;
  if ( nbElements < 0 )
    nbElements += SERVER_SIZE_BUFFER_DATA ;

  do
  {
    char message[256] ;
    StaticJsonBuffer<512> jsonBuffer;

    JsonObject& root   = jsonBuffer.createObject();
    JsonArray& data    = root.createNestedArray("noise") ;
    root["id"]         = i_shortID ;
    root["interval"]   = i_delayUpdateValue ;
    root["nbElements"] = nbElements ;
    root["first"]      = firstMessage ;

    for ( int8_t iData = 0 ; iData < SERVER_SIZE_MESSAGE_DATA && iReadNoiseBufferServer != iWriteNoiseBufferServer ; iData++ )
    {
      data.add( noiseBufferServer[iReadNoiseBufferServer] ) ;
      iReadNoiseBufferServer = ( iReadNoiseBufferServer + 1 ) % SERVER_SIZE_BUFFER_DATA ;
    }

    root.printTo(message, sizeof(message)) ;
    millisSend  = millis()  ;
    sendPostRequest(i_hostURL, "/api/data/", message, &HTTPCode, NULL) ;
    firstMessage = false ;

  } while( iReadNoiseBufferServer != iWriteNoiseBufferServer );

}

/**
 * \fn void sendDataServer()
 * \param[in] i_data Data to add to the circular buffer that will be sent to the server
 * \brief Add data to the buffer that will be sent to the server
*/
void addDataSendServer(int16_t i_data)
{
  noiseBufferServer[iWriteNoiseBufferServer] = i_data ;
  iWriteNoiseBufferServer = ( iWriteNoiseBufferServer + 1 ) % SERVER_SIZE_BUFFER_DATA ;
}
