/*
This example will receive multiple universes via Artnet and control a strip of ws2811 leds via 
Adafruit's NeoPixel library: https://github.com/adafruit/Adafruit_NeoPixel
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>

#include <NeoPixelBus.h>

//Wifi settings
const char* ssid = "dummenachbarn";
const char* password = "1nachbar";

// NeopixelBus init
const int numLeds = 600; // change for your setup
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)

NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> leds(numLeds);

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 450 + ((numberOfChannels % 450) ? 1 : 0); // tom: 450 instead of 512 because we use 150 pixel per universe = 2,5m - make this configurable
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}

void initTest()
{
  for (int i = 0 ; i < numLeds ; i++)
    leds.SetPixelColor(i, RgbColor(127, 0, 0));
  leds.Show();
  delay(300);
  for (int i = 0 ; i < numLeds ; i++)
    leds.SetPixelColor(i, RgbColor(0, 127, 0));
  leds.Show();
  delay(300);
  for (int i = 0 ; i < numLeds ; i++)
    leds.SetPixelColor(i, RgbColor(0, 0, 127));
  leds.Show();
  delay(300);
  for (int i = 0 ; i < numLeds ; i++)
    leds.SetPixelColor(i, RgbColor(0, 0, 0));
  leds.Show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  //tom: !!!
  length = 450; //tom: hacked to be just the first 450 dmx values
  
  sendFrame = 1;
  // set brightness of the whole strip 
  if (universe == 15)
  {
//tom:noidea!?//
//    leds.setBrightness(data[0]);
//    leds.Show();
  }

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0 ; i < maxUniverses ; i++)
  {
    if (universesReceived[i] == 0)
    {
      //Serial.println("Broke");
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < numLeds)

      //tom: test to output 4 universes on 300 leds
      if (i >= 75) led = i + (universe - startUniverse) * 75 + 300 - 75;
      if (i < 75) led = i + (universe - startUniverse) * 75;
      //
      
      leds.SetPixelColor(led, RgbColor(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]));
  }
  previousDataLength = length;     
  
  if (sendFrame)
  {
    leds.Show();
     Serial.println(previousDataLength);
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

void setup()
{
  Serial.begin(115200);
  ConnectWifi();
  artnet.begin();
  leds.Begin();
  initTest();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
