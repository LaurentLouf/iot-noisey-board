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
#define THRESHOLD          1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

const int delayAnimation        = 80 ;
const int delayUpdateValue      = 5000 ;
unsigned int  wheelpos  = 0;
unsigned int  strength  = 0;
unsigned int  sample    = 0;
unsigned int  sampleavg = 1024;
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
void animate ()
{
  for ( int i = 0 ; i < NUMPIXELS ; i++ )
  {
    if ( strength <= THRESHOLD )
      pixels.setPixelColor( i, pixels.Color(0,0,0) ) ;
    else
    {
      if ( i == wheelpos )
      {
        pixels.setPixelColor(i,                pixels.Color(constrain(1*strength-THRESHOLD,1,255),    0, 0) ) ;
        pixels.setPixelColor((i+1)%NUMPIXELS,  pixels.Color(constrain(1.15*strength-THRESHOLD,1,255), 0, 0) ) ;
        pixels.setPixelColor((i+2)%NUMPIXELS,  pixels.Color(constrain(1.3*strength-THRESHOLD,1,255),  0, 0) ) ;
        pixels.setPixelColor((i+3)%NUMPIXELS,  pixels.Color(constrain(1.45*strength-THRESHOLD,2,255), 0, 0) ) ;
        pixels.setPixelColor((i+4)%NUMPIXELS,  pixels.Color(constrain(1.6*strength-THRESHOLD,2,255),  0, 0) ) ;
        pixels.setPixelColor((i+5)%NUMPIXELS,  pixels.Color(constrain(1.75*strength-THRESHOLD,2,255), 0, 0) ) ;
        pixels.setPixelColor((i+6)%NUMPIXELS,  pixels.Color(constrain(2*strength-THRESHOLD,3,255),    0, 0) ) ;
        pixels.setPixelColor((i+7)%NUMPIXELS,  pixels.Color(constrain(2.25*strength-THRESHOLD,3,255), 0, 0) ) ;
        pixels.setPixelColor((i+8)%NUMPIXELS,  pixels.Color(constrain(2.5*strength-THRESHOLD,3,255),  0, 0) ) ;
        pixels.setPixelColor((i+9)%NUMPIXELS,  pixels.Color(constrain(2.75*strength-THRESHOLD,5,255), 0, 0) ) ;
        pixels.setPixelColor((i+10)%NUMPIXELS, pixels.Color(constrain(2.85*strength-THRESHOLD,5,255), 0, 0) ) ;
        pixels.setPixelColor((i+11)%NUMPIXELS, pixels.Color(constrain(3*strength-THRESHOLD,10,255),   0, 0) ) ;
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
 * \return The last value read of the strength of the signal
 *
 * During 50ms, get a value from the ADC and update the min and max values accordingly.
*/
int measure()
{
  unsigned long startMillis   = millis() ;
  unsigned int  signalMax     = 0 ;
  unsigned int  signalMin     = 1024 ;
  unsigned int  numberSamples = 0 ;
  unsigned int  sampleSum     = 0 ;

  // Sample window width in mS (50 mS = 20Hz)
  while ( millis() - startMillis < 50 )
  {
    int sample = analogRead(A0);
    numberSamples++ ;
    if ( sample < 1024 )
    {
      sampleSum+= sample ;
      if (sample > signalMax)
        signalMax = sample ;
      else if (sample < signalMin)
        signalMin = sample ;
    }
  }

  if ( signalMax - signalMin < sampleavg )
    sampleavg = constrain ( signalMax - signalMin, 0, 1023 ) ; // 10 = bruit intrinsèque

/*
  Serial.print(numberSamples);
  Serial.print("\t") ;
  Serial.print(signalMin);
  Serial.print("\t") ;
  Serial.print(signalMax) ;
  Serial.print("\t") ;
  Serial.print(sampleavg) ;
  Serial.print("\t") ;*/
  sampleavg = sampleSum / numberSamples ;/*
  Serial.print(sampleavg) ;
  Serial.print("\n") ;*/

  return sample;
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

  strength=sampleavg-6;
  Serial.print(strength);
  Serial.print("\t");
  strength=map(strength,0,1023,0, 255);
  Serial.print(strength);
  Serial.print("\n");

  sampleavg=1024;
  stat=!stat;
}

/**
 * \fn void setup()
 * \brief
*/
void setup()
{
  WiFiManager wifiManager;

  // Initialize serial communication, LED strip, Wifi Manager and the pin of the built-in LED as an output pin
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);
  pixels.begin();
  pixels.show() ;
  wifiManager.setAPCallback(configModeCallback);

  // Start blinking the built-in LED repeatedly
  ticker.attach(0.6, tick);

  // Add timer functions to animate the LED strip and update ...
  t.every(delayAnimation, animate) ;
  t.every(delayUpdateValue, updateStrength) ;

  // Tries to autoconnect to a network called "Noisey"
  if (!wifiManager.autoConnect("Noisey"))
  {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
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
  // put your main code here, to run repeatedly:
  int measureResult = 0;
  char test[] = "";
  measureResult = measure();
  //test += measureResult;
  t.update();
  //sendPostRequest(urlApi, test);
}
