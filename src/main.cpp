/**
  \file main.cpp
  \brief Code for the board for Noisey
  \author Nicolas Lopes, Jeremie Zarca, Laurent Louf
*/
#include <Arduino.h>
#include <Ticker.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
// Libraries for the ESP
#include <ESP8266WiFi.h>
// WifiManager with its dependencies
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "color.h"
#include "config.h"
#include "server.h"

#define HOST_API "noisey"
#define NUMPIXELS      24 /*!< The number of pixels in the LED strip */
#define PIN_NEOPIXEL   12 /*!< The PIN linked to the data input of the LED */
#define SCALE_DELTA    10 /*!< Number of bits to shift the value of hue to apply delta between current and next values */

const int16_t delayAnimation            = 80 ;                                /*!< Delay betwwen two states of the animation of the LED strip, in ms */
const int32_t delayUpdateValue          = NUMPIXELS * delayAnimation ;        /*!< Delay betwwen two updates of the color to be displayed, in ms */
const int16_t nbAnimationBetweenUpdates = delayUpdateValue / delayAnimation ; /*!< Number of animations of the LED strip between two updates of the color */
const int16_t runningAverageBitScale    = 8 ;                                 /*!< The number of bits for the running average factor */
const int16_t runningAverageFactorOld   = 230 ;                               /*!< The factor to apply to the old value for the running average */
const int16_t runningAverageFactorNew   = (1 << runningAverageBitScale) - runningAverageFactorOld ; /*!< The factor to apply to the new value for the running average */

char shortID[7]     = {'\0'} ;
byte wheelpos       = 0 ;
int16_t sample      = 0 ;
int16_t runningAverage = 0 ;
int16_t previousRunningAverage = 0 ;
int16_t maxValueRunningAverage = 0 ;
int16_t previousMaxValueRunningAverage = 0 ;
int32_t shiftedHue  = 120 << SCALE_DELTA ;
int16_t deltaHue    = 0 ;

int8_t  offsetSignal ;
int8_t  sensitivitySignal ;
int32_t delayDataServer ;   /*!< Delay betwwen two POST requests to the distant server, in ms */
uint8_t brightness ;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Ticker tickerLED, tickerMeasure, tickerUpdateColor, tickerAnimate ;

/**
 * \fn void tick()
 * \brief Toggle the state of the built-in LED
*/
void tick()
{
  int state = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !state);
}

/**
 * \fn void configModeCallback (WiFiManager *myWiFiManager)
 * \param[in] myWiFiManager WifiManager object
 * \brief Print the AP IP and SSID to Serial and make LED blink fast
 *
 * This function is the callback function called when the board does not manage to connect to a network and enters into AP mode.
*/
void configModeCallback (WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  tickerLED.attach(0.2, tick);
}

/**
 * \fn void animate()
 * \brief Animate the LED strip
 *
 * Animate the LED strip according to the following rules :
 *   - light the LEDs between wheelpos and wheelpos+11 (modulo the number of LEDs in the strip)
 *   - do not light other LEDs
 * The current color of the LED strip is updated at each call with the delta computed periodically. At the end, increment the position of the beginning of the wheel (wheelpos).
*/
void animate()
{
  uint8_t red, green, blue ;
  int16_t hue ;
  uint32_t colorOn, colorOff ;

  // Update the hue and create the color corresponding
  shiftedHue += deltaHue ;
  hue = shiftedHue >> SCALE_DELTA ;
  HSBToRGB(hue, 255, brightness, &red, &green, &blue) ;
  colorOn   = pixels.Color(red, green, blue);
  colorOff  = pixels.Color(0, 0, 0);

  // Set the colors for each pixel of the strip. Half will be lighted, the other half no.
  for ( byte i = 0 ; i < NUMPIXELS >> 1 ; i++ )
  {
    pixels.setPixelColor(( wheelpos + i ) % NUMPIXELS, colorOn ) ;
    pixels.setPixelColor(( wheelpos - i + NUMPIXELS ) % NUMPIXELS, colorOff ) ;
  }

  // Show the current state of pixels and update the wheel position
  pixels.show();
  wheelpos = ( wheelpos + 1 ) % ( NUMPIXELS - 1 ) ;
}

/**
 * \fn int measure()
 * \brief Measure the average and max strength of the signal over a window and over long periods of time
 *
 * During the width of the window, get a value from the ADC and update the average for this set of samples. At the end, update the running averages of the average and max values of the signal.
*/
void measure()
{
  unsigned long startMillis   = millis() ;
  int16_t numberSamples = 0 ;
  int32_t sampleSum     = 0 ;
  int16_t sampleAverage = 0 ;
  int32_t runningAverageUnscaled ;
  int16_t maxLvl = 0 ;

  // Sample window width in mS
  while ( millis() - startMillis < 20 )
  {
    int16_t sample = analogRead(A0);
    numberSamples++ ;
    sampleSum += sample ;

    maxLvl = sample > maxLvl ? sample : maxLvl ;
  }

  sampleAverage           = sampleSum / numberSamples ;
  // Running average for the average value of samples
  runningAverageUnscaled  = sampleAverage * runningAverageFactorNew + runningAverage * runningAverageFactorOld ;
  runningAverage          = runningAverageUnscaled >> runningAverageBitScale ;
  // Running average for the max value of samples
  runningAverageUnscaled  = maxLvl * runningAverageFactorNew + maxValueRunningAverage * runningAverageFactorOld ;
  maxValueRunningAverage  = runningAverageUnscaled >> runningAverageBitScale ;
}

/**
 * \fn void updateColor()
 * \brief Update the color to be displayed according to the signal strength
*/
void updateColor()
{
  int16_t nextHue ;

  // Compute the value of the next hue and delta between current and next hue
  nextHue = sensitivitySignal * ( maxValueRunningAverage - runningAverage - offsetSignal) ;
  nextHue = std::max<int16_t>( std::min<int16_t>(nextHue, 120), 0 ) ;
  nextHue = map(-nextHue, -120, 0, 0, 120); // Map the strength of the signal to a hue value : green is at 120 and red at 0
  deltaHue = ( (nextHue << SCALE_DELTA) - shiftedHue ) / nbAnimationBetweenUpdates ;

  addDataSendServer(maxValueRunningAverage - runningAverage) ;

  // Update the previous values of the running averages
  previousRunningAverage          = runningAverage ;
  previousMaxValueRunningAverage  = maxValueRunningAverage ;
}

/**
 * \fn void setup()
 * \brief
*/
void setup()
{
  WiFiManager wifiManager;
  String response ;
  StaticJsonBuffer<256> jsonBuffer;
  int16_t HTTPCode ;
  bool changeInMemory = false ;

  // Initialize serial communication, Wifi Manager and the pin of the built-in LED as an output pin
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setDebugOutput(true) ;

  // Initiate LED strip
  pixels.begin();
  pixels.show() ;

  EEPROM.begin(64) ;

  // Start blinking the built-in LED repeatedly
  tickerLED.attach(0.6, tick);

  // Tries to autoconnect to a network called "Noisey"
  if ( !wifiManager.autoConnect( "Noisey", getAPPassword().c_str() ) )
  {
    Serial.println(F("failed to connect and hit timeout"));
    ESP.reset();
    delay(1000);
  }

  // If it managed to connect, send a message to the server
  char messageToApi[256] ;
  sprintf(messageToApi, "{\"id\":\"%d\"}", ESP.getChipId() ) ;
  Serial.printf("Connected to %s, sending ID to server.\n", WiFi.SSID().c_str() );
  sendPostRequest(HOST_API, "/api/device/", messageToApi, &HTTPCode, &response);

  // If the server answered
  if ( HTTPCode == 200 )
  {
    JsonObject& root = jsonBuffer.parseObject(response);
    strncpy(shortID, root["shortID"], 6) ;
    Serial.printf("Received short ID from server %s\n", shortID) ;

    // Get the config from the message or the EEPROM if not in the message
    JsonVariant serverOffset      = root["config"]["offset"] ;
    JsonVariant serverSensitivity = root["config"]["sensitivity"] ;
    JsonVariant serverBrightness  = root["config"]["luminosity"] ;
    JsonVariant serverDelayDataServer   = root["config"]["updateRate"] ;
    if (serverOffset.success() )
      changeInMemory |= writeOffsetToMemory(root["config"]["offset"]) ;
    if (serverSensitivity.success() )
      changeInMemory |= writeSensitivityToMemory(root["config"]["sensitivity"]) ;
    if (serverDelayDataServer.success() )
      changeInMemory |= writeDelayDataServerToMemory(root["config"]["updateRate"]) ;
    if (serverBrightness.success() )
      changeInMemory |= writeBrightnessToMemory(root["config"]["luminosity"]) ;

    if ( changeInMemory )
      EEPROM.commit() ;
  }
  else
  {
    Serial.printf("Communication with server failed, code %d, message %s\n", HTTPCode, response.c_str() ) ;
  }

  offsetSignal      = readOffsetFromMemory() ;
  sensitivitySignal = readSensitivityFromMemory() ;
  delayDataServer   = readDelayDataServerFromMemory() ;
  brightness        = readBrightnessFromMemory() ;
  Serial.printf("Sensi %d, offset %d, delay %d, brightness %d\n", sensitivitySignal, offsetSignal, delayDataServer, brightness) ;

  // Perform a first measure to initialize the running averages
  measure() ;
  previousRunningAverage          = runningAverage ;
  previousMaxValueRunningAverage  = maxValueRunningAverage ;

  //
  tickerLED.attach_ms(delayUpdateValue, tick);
  tickerMeasure.attach_ms(delayAnimation, measure) ;
  tickerAnimate.attach_ms(delayAnimation, animate) ;
  tickerUpdateColor.attach_ms(delayUpdateValue, updateColor) ;
}


/**
 * \fn void loop()
 * \brief
*/
void loop()
{
  static uint32_t lastEventMillis = 0 ;

  if ( (uint32_t) (millis() - lastEventMillis) >= delayDataServer )
  {
    lastEventMillis = millis() ;
    sendDataServer(HOST_API, shortID, delayUpdateValue) ;
  }
  delay(1) ;
}
