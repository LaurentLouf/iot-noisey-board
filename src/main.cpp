/**
  \file main.cpp
  \brief Code for the board for Noisey
  \author Nicolas Lopes, Jeremie Zarca, Laurent Louf
*/
#include <Arduino.h>
#include <Ticker.h>
#include <Adafruit_NeoPixel.h>
// Libraries for the ESP
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// WifiManager with its dependencies
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "Timer.h"
#include "color.h"

#define urlApi "http://0360276d.ngrok.io/"
#define NUMPIXELS      24 /*!< The number of pixels in the LED strip */
#define PIN_NEOPIXEL   12 /*!< The PIN linked to the data input of the LED */
#define THRESHOLD      1
#define SCALE_DELTA    10 /*!< Number of bits to shift the value of hue to apply delta between current and next values */

const int16_t delayAnimation            = 80 ;
const int32_t delayUpdateValue          = NUMPIXELS * delayAnimation ;
const int16_t nbAnimationBetweenUpdates = delayUpdateValue / delayAnimation ;
const int16_t runningAverageMax         = 256 ;
const int16_t runningAverageFactor      = 230 ;

byte wheelpos       = 0 ;
int16_t sample      = 0 ;
int16_t runningAverage = 0 ;
int16_t previousRunningAverage = 0 ;
int16_t maxValueRunningAverage = 0 ;
int16_t previousMaxValueRunningAverage = 0 ;
int32_t shiftedHue = 0 ;
int16_t deltaHue = 0 ;
bool stat = 0;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Ticker ticker;
Timer t;

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
  ticker.attach(0.2, tick);
}

/**
 * \fn void sendPostRequest(char *url, char *message)
 * \param[in] url URL to send the message to
 * \param[in] message Message to be sent
 * \brief Callback called when WiFiManager enters configuration mode
 *
 * Print the AP IP and SSID to Serial and make LED blink faster. Send a message to a url in JSON format
*/
void sendPostRequest(char *url, char *message)
{
  HTTPClient http;
  Serial.println("sendPostRequest");
  Serial.println(message);
  Serial.println(url);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.POST(message);
  http.writeToStream(&Serial);
  http.end();
}


/**
 * \fn void animate()
 * \brief Animate the LED strip according to the strength of the signal received
 * \note The code could use a bit of rework to better reflect the logic behind it
 *
 * Animate the LED strip according to the following rules :
 *   - if the strength is below the threshold, there should be no light at all
 *   - if the strength is above the threshold :
 *     - light the LEDs between wheelpos and wheelpos+11 (modulo the number of LEDs in the strip) according to the strength
 *     - do not light other LEDs
 * At the end, increment the position of the beginning of the wheel (wheelpos)
*/
void animate()
{
  uint8_t red, green, blue ;
  int16_t hue ;

  shiftedHue += deltaHue ;
  hue = shiftedHue >> SCALE_DELTA ;
  HSBToRGB(hue, 255, 255, &red, &green, &blue) ;
  uint32_t colorOn = pixels.Color(red, green, blue);
  uint32_t colorOff = pixels.Color(0, 0, 0);
  //Serial.println(shiftedHue) ;

  for ( byte i = 0 ; i < NUMPIXELS >> 1 ; i++ )
  {
    pixels.setPixelColor(( wheelpos + i ) % NUMPIXELS, colorOn ) ;
    pixels.setPixelColor(( wheelpos - i + NUMPIXELS ) % NUMPIXELS, colorOff ) ;
  }
  pixels.show();
  wheelpos = ( wheelpos + 1 ) % ( NUMPIXELS - 1 ) ;
}

/**
 * \fn int measure()
 * \brief Measure the average strength of the signal over a window of 50ms
 *
 * During 50ms, get a value from the ADC and update the average for this set of samples
*/
int16_t measure()
{
  unsigned long startMillis   = millis() ;
  int16_t numberSamples = 0 ;
  int32_t sampleSum     = 0 ;
  int16_t sampleAverage = 0 ;
  int32_t runningAverageUnscaled ;
  int16_t maxLvl = 0 ;

  // Sample window width in mS (50 mS = 20Hz)
  while ( millis() - startMillis < 50 )
  {
    int sample = analogRead(A0);
    numberSamples++ ;
    sampleSum += sample ;

    maxLvl = sample > maxLvl ? sample : maxLvl ;
  }

  sampleAverage           = sampleSum / numberSamples ;
  // Running average for the average value of samples
  runningAverageUnscaled  = sampleAverage * runningAverageFactor + runningAverage * (runningAverageMax - runningAverageFactor) ;
  runningAverage          = runningAverageUnscaled / runningAverageMax ;
  // Running average for the max value of samples
  runningAverageUnscaled  = maxLvl * runningAverageFactor + maxValueRunningAverage * (runningAverageMax - runningAverageFactor) ;
  maxValueRunningAverage  = runningAverageUnscaled / runningAverageMax ;

  return sampleAverage ;
}

/**
 * \fn void updateStrength()
 * \brief
*/
void updateStrength()
{
  int16_t nextHue ;

  // Change the state of the built-in LED
  stat=!stat;
  if(stat)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);

  // Compute the value of the next hue and delta between current and next hue
  nextHue = maxValueRunningAverage - runningAverage ;
  nextHue = nextHue > 120 ? 120 : nextHue ;
  nextHue = map(-nextHue, -120, 0, 0, 120); // Map the strength of the signal to a hue value : green is at 120 and red at 0
  deltaHue = ( (nextHue << SCALE_DELTA) - shiftedHue ) / nbAnimationBetweenUpdates ;

  Serial.printf("cH %d nH %d, d %d\n", shiftedHue, nextHue << SCALE_DELTA, deltaHue );

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

  // Initialize serial communicatio, Wifi Manager and the pin of the built-in LED as an output pin
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);
  wifiManager.setAPCallback(configModeCallback);
  // Initiate LED strip
  pixels.begin();
  pixels.show() ;

  // Start blinking the built-in LED repeatedly
  ticker.attach(0.6, tick);

  // Add timer functions to animate the LED strip and update the colors to be displayed
  t.every(delayAnimation, animate) ;
  t.every(delayUpdateValue, updateStrength) ;

  // Tries to autoconnect to a network called "Noisey"
  if (!wifiManager.autoConnect("Noisey"))
  {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  }

  // If it managed to connect, send a message to serial and to the server. Then stop the blinking of the LED and make sure it is off
  char messageToApi[] = "{\"message\":\"Coucou\"}";
  Serial.println("connected...yeey :)");
  sendPostRequest(urlApi, messageToApi);
  ticker.detach();
  digitalWrite(BUILTIN_LED, LOW);

}


/**
 * \fn void loop()
 * \brief
*/
void loop()
{
  measure();
  t.update();
}
