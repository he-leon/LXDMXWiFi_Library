/**************************************************************************/
/*!
    @file     ESP-DMXNeoPixels.ino
    @author   Claude Heintz
    @license  BSD (see LXDMXWiFi.h)
    @copyright 2016 by Claude Heintz All Rights Reserved

    Example using LXDMXWiFi_Library for output of Art-Net or E1.31 sACN from
    ESP8266 Adafruit Huzzah WiFi connection to an Adafruit NeoPixel Ring.
    
    Art-Net(TM) Designed by and Copyright Artistic Licence (UK) Ltd
    sACN E 1.31 is a public standard published by the PLASA technical standards program
    
    NOTE:  This example requires the Adafruit NeoPixel Library and WiFi101 Library

           Remote config is supported using the configuration utility in the examples folder.
           Otherwise edit initConfig() in LXDMXWiFiConfig.cpp.

    @section  HISTORY

    v1.0 - First release

*/
/**************************************************************************/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "LXDMXWiFi.h"
#include <LXWiFiArtNet.h>
#include <LXWiFiSACN.h>
#include "LXDMXWiFiConfig.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define STARTUP_MODE_PIN 16      // pin for force default setup when low (use 10k pullup to insure high)
#define LED_PIN BUILTIN_LED

#define PIN 14
#define NUM_LEDS 12
Adafruit_NeoPixel ring = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);


const int total_pixels = 3 * NUM_LEDS;
byte pixels[NUM_LEDS][3];

/*         
 *  Edit the LXDMXWiFiConfig.initConfig() function in LXDMXWiFiConfig.cpp to configure the WiFi connection and protocol options
 */

// dmx protocol interface for parsing packets (created in setup)
LXDMXWiFi* interface;

// An EthernetUDP instance to let us send and receive UDP packets
WiFiUDP wUDP;

// direction output from network/input to network
uint8_t dmx_direction = 0;


/* 
   utility function to toggle indicator LED on/off
*/
uint8_t led_state = 0;

void blinkLED() {
  if ( led_state ) {
    digitalWrite(LED_PIN, HIGH);
    led_state = 0;
  } else {
    digitalWrite(LED_PIN, LOW);
    led_state = 1;
  }
}

/* 
   artAddress callback allows storing of config information
   artAddress may or may not have set this information
   but relevant fields are copied to config struct (and stored to EEPROM ...not yet)
*/
void artAddressReceived() {
  DMXWiFiConfig.setArtNetUniverse( ((LXWiFiArtNet*)interface)->universe() );
  DMXWiFiConfig.setNodeName( ((LXWiFiArtNet*)interface)->longName() );
  DMXWiFiConfig.commitToPersistentStore();
}

/*
  sends pixel buffer to ring
*/

void sendPixels() {
  uint16_t r,g,b;
  for (int p=0; p<NUM_LEDS; p++) {
    r = pixels[p][0];
    g = pixels[p][1];
    b = pixels[p][2];
    r = (r*r)/255;    //gamma correct
    g = (g*g)/255;
    b = (b*b)/255;
    ring.setPixelColor(p, r, g, b);
  }
  ring.show();
}

void setPixelSlot(uint8_t slot, uint8_t value) {
  uint8_t si = slot-1; //zero based, not 1 based like dmx
  uint8_t pixel = si/3;
  uint8_t color = si%3;
  pixels[pixel][color] = value;
}

/************************************************************************

  Setup creates the WiFi connection.
  
  It also creates the network protocol object,
  either an instance of LXWiFiArtNet or LXWiFiSACN.
  
  if OUTPUT_FROM_NETWORK_MODE:
     Starts listening on the appropriate UDP port.
  
     And, it starts the SAMD21DMX sending serial DMX via the UART1 TX pin.
     (see the SAMD21DMX library documentation for driver details)
     
   if INPUT_TO_NETWORK_MODE:
     Starts SAMD21DMX listening for DMX ( received as serial on UART0 RX pin. )

*************************************************************************/

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(STARTUP_MODE_PIN, INPUT);
 // while ( ! Serial ) {}     //force wait for serial connection.  Sketch will not continue until Serial Monitor is opened.
  Serial.begin(9600);         //debug messages
  Serial.println("_setup_");
  
  DMXWiFiConfig.begin(digitalRead(STARTUP_MODE_PIN));

  int wifi_status = WL_IDLE_STATUS;
  if ( DMXWiFiConfig.APMode() ) {                      // WiFi startup
    WiFi.mode(WIFI_AP);
    WiFi.softAP(DMXWiFiConfig.SSID());
    WiFi.softAPConfig(DMXWiFiConfig.apIPAddress(), DMXWiFiConfig.apGateway(), DMXWiFiConfig.apSubnet());
    Serial.print("Access Point IP Address: ");
  } else {                                             // Station Mode
    WiFi.mode(WIFI_STA);
    WiFi.begin(DMXWiFiConfig.SSID(), DMXWiFiConfig.password());

    if ( DMXWiFiConfig.staticIPAddress() ) {  
      WiFi.config(DMXWiFiConfig.stationIPAddress(), (uint32_t)0, DMXWiFiConfig.stationGateway(), DMXWiFiConfig.stationSubnet());
    }
     
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      blinkLED();
    }
    
    Serial.print("Station IP Address: ");
  }
  
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  if ( DMXWiFiConfig.sACNMode() ) {         // Initialize network<->DMX interface
    interface = new LXWiFiSACN();
    interface->setUniverse(DMXWiFiConfig.sACNUniverse());
  } else {
    interface = new LXWiFiArtNet(WiFi.localIP(), WiFi.subnetMask());
    ((LXWiFiArtNet*)interface)->setSubnetUniverse(DMXWiFiConfig.artnetSubnet(), DMXWiFiConfig.artnetUniverse());
    ((LXWiFiArtNet*)interface)->setArtAddressReceivedCallback(&artAddressReceived);
    char* nn = DMXWiFiConfig.nodeName();
    if ( nn[0] != 0 ) {
      strcpy(((LXWiFiArtNet*)interface)->longName(), nn);
    }
  }
  Serial.print("interface created;");

  dmx_direction = ( DMXWiFiConfig.inputToNetworkMode() );
  
  // if OUTPUT from network, start wUDP listening for packets
  if ( dmx_direction == OUTPUT_FROM_NETWORK_MODE ) {  
    if ( ( DMXWiFiConfig.multicastMode() ) ) { // Start listening for UDP on port
      if ( DMXWiFiConfig.APMode() ) {
			wUDP.beginMulticast(WiFi.softAPIP(), DMXWiFiConfig.multicastAddress(), interface->dmxPort());
		} else {
			wUDP.beginMulticast(WiFi.localIP(), DMXWiFiConfig.multicastAddress(), interface->dmxPort());
		}
    } else {
      wUDP.begin(interface->dmxPort());
    }
    Serial.print("udp listening started;");

    if ( DMXWiFiConfig.artnetMode() ) { //if needed, announce presence via Art-Net Poll Reply
      ((LXWiFiArtNet*)interface)->send_art_poll_reply(&wUDP);
    }

    ring.begin();
    ring.show();
  } else {                    //direction is INPUT to network
    // doesn't do anything in this mode
  }

  

  Serial.println("setup complete.");
}

/************************************************************************

  Main loop
  
  if OUTPUT_FROM_NETWORK_MODE:
    checks for and reads packets from WiFi UDP socket
    connection.  readDMXPacket() returns true when a DMX packet is received.
    In which case, the data is copied to the SAMD21DMX object which is driving
    the UART serial DMX output.
  
    If the packet is an CONFIG_PACKET_IDENT packet, the config struct is modified and stored in EEPROM
  
  if INPUT_TO_NETWORK_MODE:
    if serial dmx has been received, sends an sACN or Art-Net packet containing the dmx data.
    Note:  does not listen for incoming packets for remote configuration in this mode.

*************************************************************************/

void loop() {
  
  if ( dmx_direction == OUTPUT_FROM_NETWORK_MODE ) {
    uint8_t good_dmx = interface->readDMXPacket(&wUDP);
  
    if ( good_dmx ) {
       for (int i = 1; i <= total_pixels; i++) {
          setPixelSlot(i , interface->getSlot(i));
       }
       sendPixels();
       blinkLED();
    } else {
      if ( strcmp(CONFIG_PACKET_IDENT, (const char *) interface->packetBuffer()) == 0 ) {  //match header to config packet
        Serial.print("config packet received, ");
        uint8_t reply = 0;
        if ( interface->packetBuffer()[8] == '?' ) {  //packet opcode is query
          DMXWiFiConfig.readFromPersistentStore();
          reply = 1;
        } else if (( interface->packetBuffer()[8] == '!' ) && (interface->packetSize() >= 171)) { //packet opcode is set
          Serial.println("upload packet");
          DMXWiFiConfig.copyConfig( interface->packetBuffer(), interface->packetSize());
          DMXWiFiConfig.commitToPersistentStore();
          reply = 1;
        } else {
          Serial.println("packet error.");
        }
        if ( reply) {
          DMXWiFiConfig.hidePassword();                  //don't transmit password!
          wUDP.beginPacket(wUDP.remoteIP(), interface->dmxPort());
          wUDP.write((uint8_t*)DMXWiFiConfig.config(), DMXWiFiConfigSIZE);
          wUDP.endPacket();
          Serial.println("reply complete.");
			  DMXWiFiConfig.restorePassword();
        }
        interface->packetBuffer()[0] = 0; //insure loop without recv doesn't re-trgger
        blinkLED();
        delay(100);
        blinkLED();
        delay(100);
        blinkLED();
      }     // packet has config packet header
    }       // not good_dmx
    
  } //output mode
  
}           // loop()

