/**************************************************************************/
/*!
    @file     LXDMXWiFiconfig.h
    @author   Claude Heintz
    @license  BSD (see LXDMXWiFi.h or http://lx.claudeheintzdesign.com/opensource.html)
    @copyright 2016 by Claude Heintz All Rights Reserved
    
    
*/
/**************************************************************************/

#include <inttypes.h>
#include <string.h>

/* 
	structure for storing WiFi and Protocol configuration settings
*/

typedef struct dmxwifiConfig {
   char    ident[8];      // ESP-DMX\0
   uint8_t opcode;		  // data = 0, query = '?', set ='!'
   char    ssid[64];      // max is actually 32
   char    pwd[64];       // depends on security 8, 13, 8-63
   uint8_t wifi_mode;
   uint8_t protocol_mode;
   uint8_t ap_chan;			//unimplemented
   uint32_t ap_address;
   uint32_t ap_gateway;		//140
   uint32_t ap_subnet;
   uint32_t sta_address;
   uint32_t sta_gateway;
   uint32_t sta_subnet;
   uint32_t multi_address;
   uint8_t sacn_universe;   //should match multicast address
   uint8_t artnet_subnet;
   uint8_t artnet_universe;
   uint8_t node_name[32];
   uint32_t input_address;
   uint8_t reserved[25];
} DMXWiFiconfig;

#define CONFIG_PACKET_IDENT "ESP-DMX"
#define DMXWiFiConfigSIZE 232
#define DMXWiFiConfigMinSIZE 171

#define STATION_MODE 0
#define AP_MODE 1

#define ARTNET_MODE 0
#define SACN_MODE 1
#define STATIC_MODE 2
#define MULTICAST_MODE 4

#define OUTPUT_FROM_NETWORK_MODE 0
#define INPUT_TO_NETWORK_MODE 8

/*!   
@class DMXwifiConfig
@abstract
   DMXwifiConfig abstracts WiFi and Protocol configuration settings so that they can
   be saved and retrieved from persistent storage.
*/

class DMXwifiConfig {

  public:
  
	  DMXwifiConfig ( void );
	 ~DMXwifiConfig( void );
	 
	 /* 
	 	handles init of config data structure, reading from persistent if desired.
	 */
	 void begin ( uint8_t mode );
	 
	 /*
	 initConfig initializes the DMXWiFiConfig structure with default settings
	 The default is to receive Art-Net with the WiFi configured as an access point.
	 (Modify the initConfig to change default settings.  But, highly recommend leaving AP_MODE for default startup.)
	 */
	 void initConfig(void);
	 
	 /* 
	 	WiFi setup parameters
	 */
	 char* SSID(void);
	 char* password(void);
	 bool APMode(void);
	 bool staticIPAddress(void);
	 
	 /* 
	 	protocol modes
	 */
    bool artnetMode(void);
    bool sACNMode(void);
    bool multicastMode(void);
    bool inputToNetworkMode(void);
    
    /* 
	 	stored IPAddresses
	 */
    IPAddress apIPAddress(void);
    IPAddress apGateway(void);
	 IPAddress apSubnet(void);
	 IPAddress stationIPAddress(void);
    IPAddress stationGateway(void);
	 IPAddress stationSubnet(void);
	 IPAddress multicastAddress(void);
	 IPAddress inputAddress(void);
	 
	 /* 
	 	protocol settings
	 */
	 uint8_t sACNUniverse(void);
	 uint8_t artnetSubnet(void);
	 uint8_t artnetUniverse(void);
	 void setArtNetUniverse(int u);
	 char* nodeName(void);
	 void setNodeName(char* nn);
	 
	 /* 
	 	copyConfig from uint8_t array
	 */
	 void copyConfig(uint8_t* pkt, uint8_t size);
	 
	 /* 
	 	read from EEPROM or flash
	 */
	 void readFromPersistentStore(void);
	 
	 /* 
	 	write to EEPROM or flash
	 */
	 void commitToPersistentStore(void);
	 
	 /* 
	 	pointer and size for UDP.write()
	 */
	 uint8_t* config(void);
	 uint8_t configSize(void);

	 /* 
	 	Utility function. WiFi station password should never be returned by query.
	 */
	 void hidePassword(void);
	 void restorePassword(void);

  private:
   
    /* 
	 	pointer to configuration structure
	 */
     DMXWiFiconfig* _wifi_config;
	  char    _save_pwd[64];
};

extern DMXwifiConfig DMXWiFiConfig;
