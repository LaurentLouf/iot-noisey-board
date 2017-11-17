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

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

const int16_t delayAnimation        = 80 ;
const int32_t delayUpdateValue      = NUMPIXELS * delayAnimation ;
const int16_t runningAverageMax     = 256 ;
const int16_t runningAverageFactor  = 230 ;

byte wheelpos       = 0 ;
int16_t sample      = 0 ;
int16_t runningAverage = 0 ;
int16_t previousRunningAverage = 0 ;
int16_t maxValueRunningAverage = 0 ;
int16_t strength = 0 ;
bool stat = 0;

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
  int16_t hue ;
  uint8_t red, green, blue ;

  hue = map(-strength, -120, 0, 0, 120); // Map the strength of the signal to a hue value : green is at 120 and red at 0

  for ( byte i = 0 ; i < NUMPIXELS ; i++ )
  {
    if ( runningAverage <= THRESHOLD )
      pixels.setPixelColor( i, pixels.Color(0,0,0) ) ;
    else
    {
      if ( i == wheelpos )
      {
        HSBToRGB(hue, 255, 127, &red, &green, &blue) ;
        pixels.setPixelColor(i,                red, green, blue ) ;
        HSBToRGB(hue, 255,  138, &red, &green, &blue) ;
        pixels.setPixelColor((i+1)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  148, &red, &green, &blue) ;
        pixels.setPixelColor((i+2)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  159, &red, &green, &blue) ;
        pixels.setPixelColor((i+3)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  169, &red, &green, &blue) ;
        pixels.setPixelColor((i+4)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  180, &red, &green, &blue) ;
        pixels.setPixelColor((i+5)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  191, &red, &green, &blue) ;
        pixels.setPixelColor((i+6)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  201, &red, &green, &blue) ;
        pixels.setPixelColor((i+7)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  212, &red, &green, &blue) ;
        pixels.setPixelColor((i+8)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  222, &red, &green, &blue) ;
        pixels.setPixelColor((i+9)%NUMPIXELS,  red, green, blue ) ;
        HSBToRGB(hue, 255,  233, &red, &green, &blue) ;
        pixels.setPixelColor((i+10)%NUMPIXELS, red, green, blue ) ;
        HSBToRGB(hue, 255,  244, &red, &green, &blue) ;
        pixels.setPixelColor((i+11)%NUMPIXELS, red, green, blue ) ;
      }
      else if ( i > wheelpos+11 || i < wheelpos )
        pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
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

    if ( sample > maxLvl )
      maxLvl = sample ;
  }

  sampleAverage           = sampleSum / numberSamples ;
  runningAverageUnscaled  = sampleAverage * runningAverageFactor + runningAverage * (runningAverageMax - runningAverageFactor) ;
  runningAverage          = runningAverageUnscaled / runningAverageMax ;
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
  if(stat)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);

  stat=!stat;

  strength = maxValueRunningAverage - runningAverage ;
  strength = strength > 120 ? 120 : strength ;

  // Update the strength global var
  Serial.printf("previousAvg : %d, currentAvg %d, maxAvg %d, diff %d\n", previousRunningAverage, runningAverage, maxValueRunningAverage, maxValueRunningAverage - runningAverage );

  previousRunningAverage = runningAverage ;
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

  // Add timer functions to animate the LED strip and update ...
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
