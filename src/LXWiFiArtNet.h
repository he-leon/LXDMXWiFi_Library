/* LXWiFiArtNet.h
   Copyright 2016 by Claude Heintz Design
   see LXDMXWiFi.h for LICENSE

	Art-Net(TM) Designed by and Copyright Artistic Licence Holdings Ltd.
*/

#ifndef LXWIFIARTNET_H
#define LXWIFIARTNET_H

#include <Arduino.h>
#include "LXDMXWiFi.h"

#define ARTNET_PORT 0x1936
#define ARTNET_BUFFER_MAX 530
#define ARTNET_REPLY_SIZE 240
#define ARTNET_IPPROG_SIZE 34
#define ARTNET_ADDRESS_OFFSET 17
#define ARTNET_SHORT_NAME_LENGTH 18
#define ARTNET_LONG_NAME_LENGTH 64


#define ARTNET_ART_POLL 0x2000
#define ARTNET_ART_POLL_REPLY 0x2100
#define ARTNET_ART_DMX 0x5000
#define ARTNET_ART_ADDRESS 0x6000
#define ARTNET_ART_IPPROG 0xF800
#define ARTNET_ART_IPPROG_REPLY 0xF900
#define ARTNET_NOP 0

#define ARTADDRESS_NO_CHANGE 0x7f
#define ARTADDRESS_PROG_BIT 0x80

#define ARTNET_STATUS1_PORT_PROG 0x20
#define ARTNET_STATUS1_FACTORY_BOOT 0x04
#define ARTNET_STATUS2_SACN_CAPABLE 0x10
#define ARTNET_STATUS2_ARTNET3_CAPABLE 0x08
#define ARTNET_STATUS2_DHCP_CAPABLE 0x04
#define ARTNET_STATUS2_DHCP_USED 0x02

typedef void (*ArtAddressRecvCallback)(void);
typedef void (*ArtIpProgRecvCallback)(uint8_t cmd, IPAddress ipaddr, IPAddress subnet);

/*!
*  @class LXWiFiArtNet
*  @abstract 
*     LXWiFiArtNet partially implements the Art-Net Ethernet Communication Standard.
*  
*  	LXWiFiArtNet is primarily a node implementation.  It supports output of a single universe
*     of DMX data from the network.  It does not support merge and will only accept
*     packets from the first IP address from which it receives an ArtDMX packet.
*     This can be reset by sending an ArtAddress cancel merge command.
*     
*     When reading packets, LXWiFiArtNet will automatically respond to ArtPoll packets.
*     Depending on the constructor used, it will either broadcast the reply or will
*     reply directly to the sender of the poll.
*  
*     http://www.artisticlicence.com
*/
class LXWiFiArtNet : public LXDMXWiFi {

  public:
/*!
* @brief constructor with address used for ArtPollReply
* @param address sent in ArtPollReply
*/   
   LXWiFiArtNet  ( IPAddress address );
   
/*!
* @brief constructor creates broadcast address for Poll Reply
* @param address sent in ArtPollReply
* @param subnet_mask used to set broadcast address
*/  
	LXWiFiArtNet  ( IPAddress address, IPAddress subnet_mask );
	
/*!
* @brief constructor creates instance with external buffer for UDP packet
* @param address sent in ArtPollReply
* @param subnet_mask used to set broadcast address
* @param buffer external buffer for UDP packets
*/ 
	LXWiFiArtNet ( IPAddress address, IPAddress subnet_mask, uint8_t* buffer );
	
/*!
* @brief destructor for LXWiFiArtNet (frees packet buffer if allocated with constructor)
*/ 
   ~LXWiFiArtNet ( void );

/*!
* @brief UDP port used by protocol
*/   
   uint16_t dmxPort ( void ) { return ARTNET_PORT; }

/*!
* @brief universe for sending and receiving dmx
* @discussion First universe is zero for Art-Net.  High nibble is subnet, low nibble is universe.
* @return universe 0-255
*/   
   uint16_t universe           ( void );
/*!
* @brief set universe for sending and receiving
* @discussion First universe is zero for Art-Net.  High nibble is subnet, low nibble is universe.
* @param u universe 0-255
*/
   void    setUniverse        ( uint16_t u );
/*!
* @brief set subnet/universe for sending and receiving
* @discussion First universe is zero for Art-Net.  Sets separate nibbles: high/subnet, low/universe.
* @param s subnet 0-16
* @param u universe 0-16
*/
   void    setSubnetUniverse  ( uint8_t s, uint8_t u );
/*!
* @brief set universe for sending and receiving
* @discussion First universe is zero for Art-Net.  High nibble is subnet, low nibble is universe.
* 0x7f is no change, otherwise if high bit is set, low nibble becomes universe (subnet remains the same)
* This method is primarily used by parse_art_address().
* @param u universe 0-16 + flag 0x80
*/
   void    setUniverseAddress ( uint8_t u );
/*!
* @brief set subnet for sending and receiving
* @discussion First universe is zero for Art-Net.  High nibble is subnet, low nibble is universe.
* 0x7f is no change, otherwise if high bit is set, low nibble becomes subnet (universe remains the same)
* This method is primarily used by parse_art_address().
* @param s subnet 0-16 + flag 0x80
*/
   void    setSubnetAddress   ( uint8_t u );
   
/*!
* @brief set net for sending and receiving
* @discussion upper 7 bits of Port-Address=> net-subnet-universe
* 0x7f is no change, otherwise if high bit is set, low nibble becomes subnet (universe remains the same)
* This method is primarily used by parse_art_address().
* @param s subnet 0-16 + flag 0x80
*/
   void    setNetAddress   ( uint8_t u );

 /*!
 * @brief number of slots (aka addresses or channels)
 * @discussion Should be minimum of ~24 depending on actual output speed.  Max of 512.
 * @return number of slots/addresses/channels
 */     
   int  numberOfSlots    ( void );
 /*!
 * @brief set number of slots (aka addresses or channels)
 * @discussion Should be minimum of ~24 depending on actual output speed.  Max of 512.
 * @param n number of slots 1 to 512
 */  
   void setNumberOfSlots ( int n );
 /*!
 * @brief get merged level data from slot/address/channel
 * @param slot 1 to 512
 * @return level for slot (0-255)
 */  
   uint8_t  getSlot      ( int slot );
 /*!
 * @brief set level data (0-255) for slot/address/channel
 * @param slot 1 to 512
 * @param level level 0 to 255
 */  
   void     setSlot      ( int slot, uint8_t level );
 /*!
 * @brief direct pointer to dmx portion of packet buffer uint8_t[]
 * @return uint8_t* to dmx data portion of packet buffer
 */ 
   uint8_t* dmxData      ( void );
   
 /*!
 * @brief direct pointer to packet buffer uint8_t[]
 * @return uint8_t* to packet buffer
 */ 
   uint8_t* packetBuffer      ( void );
   
/*!
 * @brief size of last packet received with readDMXPacket
 * @return uint16_t last packet size
 */ 
   uint16_t packetSize      ( void );
   
/*!
 * @brief direct pointer to poll reply packet contents
 * @return uint8_t* to poll reply packet contents
 */ 
   uint8_t* replyData      ( void );
   
/*!
 * @brief direct pointer to short name
 * @return char* to short name 
 */  
   char* shortName();
   
/*!
 * @brief direct pointer to short name
 * @return char* to short name 
 */  
   char* longName();

 /*!
 * @brief read UDP packet
 * @param wUDP WiFiUDP object to be used for getting UDP packet
 * @return 1 if packet contains dmx
 */    
   uint8_t  readDMXPacket       ( UDP* wUDP );
 /*!
 * @brief read contents of packet from _packet_buffer
 * @discussion _packet_buffer should already contain packet payload when this is called
 * @param wUDP pointer to UDP object
 * @param packetSize size of received packet
 * @return 1 if packet contains dmx
 */      
   uint8_t readDMXPacketContents ( UDP* wUDP, uint16_t packetSize );
 /*!
 * @brief process packet, reading it into _packet_buffer
 * @param wUDP pointer to UDP object (used for Poll Reply if applicable)
 * @return Art-Net opcode of packet
 */
   uint16_t readArtNetPacket    ( UDP* wUDP );  
 /*!
 * @brief read contents of packet from _packet_buffer
 * @param wUDP pointer to UDP object (used for Poll Reply if applicable)
 * @param packetSize size of received packet
 * @return Art-Net opcode of packet
 */   
   uint16_t readArtNetPacketContents ( UDP* wUDP, uint16_t packetSize );
 /*!
 * @brief send Art-Net ArtDMX packet for dmx output from network
 * @param wUDP pointer to UDP object to be used for sending UDP packet
 * @param to_ip target address
 * @param interfaceAddr multicast unused for Art-Net
 */    
   void     sendDMX ( UDP* wUDP, IPAddress to_ip, IPAddress interfaceAddr );
 /*!
 * @brief send ArtPoll Reply packet for dmx output from network
 * @discussion If broadcast address is defined by passing subnet to constructor, reply is broadcast
 *             Otherwise, reply is unicast to remoteIP belonging to the sender of the poll
 * @param wUDP pointer to UDP object to be used for sending UDP packet
 */  
   void     send_art_poll_reply ( UDP* wUDP );
   
  /*!
 * @brief send ArtIpProgReply packet for dmx output from network
 * @discussion Reply is unicast to remoteIP belonging to the sender of ArtIpProg
 * @param wUDP pointer to UDP object to be used for sending UDP packet
 */    
   void     send_art_ipprog_reply ( UDP* wUDP );
   
/*!
 * @brief Function called when ArtAddress packet is received
 * @discussion Sets a pointer to a function that is called
 *             when an ArtAddress packet is received
*/
   void setArtAddressReceivedCallback(ArtAddressRecvCallback callback);
   
/*!
 * @brief Function called when ArtIpProg packet is received
 * @discussion Sets a pointer to a function that is called
 *             when an ArtIpProg packet is received and it 
 *             is a programming command.
*/
   void setArtIpProgReceivedCallback(ArtIpProgRecvCallback callback);
   
/*!
* @brief set flags for ArtPollReply
*/   
	void  setStatus1Flag ( uint8_t flag, uint8_t set );
	
/*!
* @brief set flags for ArtPollReply
*/	
	void  setStatus2Flag ( uint8_t flag, uint8_t set );
   
  private:
/*!
* @brief array that holds contents of incoming or outgoing packet
* @discussion DMX data is written directly into packet buffer when sending.
*             When receiving, data is extracted into one of two buffers
*             depending on the source IP of the sender.  These source buffers are merged
				  into a composite buffer.
				  The packet buffer is created and kept private unless it is supplied to the constructor.
				  When used, an external buffer can be shared between instances so that each can
				  read a packet's data.
*/
  	uint8_t*   _packet_buffer;
  	
/*!
* @brief indicates the _packet_buffer was allocated by the constructor and is private.
*/
	uint8_t   _owns_buffer;

/*!
* @brief size of last packet that was read with readDMXPacket
*/	uint16_t  _packetSize;

/*!
* @brief array that holds contents of outgoing ArtPollReply packet
*/	
	static uint8_t _reply_buffer[ARTNET_REPLY_SIZE];
	
/*!
* @brief string containing short name of node for poll reply
*/	
	char _short_name[ARTNET_SHORT_NAME_LENGTH];
	
/*!
* @brief string containing long name of node for poll reply
*/	
	char _long_name[ARTNET_LONG_NAME_LENGTH];
	
/*!
* @brief buffers that hold DMX data from source a, source b and HTP composite
* @discussion data is read into _dmx_buffer_a or _dmx_buffer_b depending on the
*             IP address of the sender, at the same time it is merged into _dmx_buffer_c.
*/
  	uint8_t   _dmx_buffer_a[DMX_UNIVERSE_SIZE];
  	uint8_t   _dmx_buffer_b[DMX_UNIVERSE_SIZE];
  	uint8_t   _dmx_buffer_c[DMX_UNIVERSE_SIZE];
  	
/// number of slots/address/channels
  	int       _dmx_slots;
  	int       _dmx_slots_a;
  	int       _dmx_slots_b;

/// high nibble subnet, low nibble universe
  	uint8_t   _portaddress_lo;
/// upper 7 bits of Port-Address
  	uint8_t   _portaddress_hi;
/// sequence number for sending ArtDMX packets
  	uint8_t   _sequence;
/// sequence number for sending ArtDMX packets
  	uint16_t   _poll_reply_counter;

/// address included in poll reply 	
  	IPAddress _my_address;
/// address included in IpProg reply	
  	IPAddress _my_subnetmask;
/// if subnet is supplied in constructor, holds address to broadcast poll replies
  	IPAddress _broadcast_address;
/// first sender of an ArtDMX packet
  	IPAddress _dmx_sender_a;
/// second sender of an ArtDMX packet (3rd and subsequent senders ignored until cancelMerge)
  	IPAddress _dmx_sender_b;

/// flag factory boot, network programmable
	uint8_t _status1;  	
/// flag sacn, artnet3, dhcp capable, dhcp assigned
	uint8_t _status2;
  	
	/*!
    * @brief Pointer to art address received callback function
   */
  	ArtAddressRecvCallback _artaddress_receive_callback;
  	
  	/*!
    * @brief Pointer to artIpProg received callback function
   */
  	ArtIpProgRecvCallback _artip_receive_callback;

/*!
* @brief checks packet for "Art-Net" header
* @return opcode if Art-Net packet
*/
  	uint16_t  parse_header        ( void );	
/*!
* @brief utility for parsing ArtAddress packets
* @return opcode in case command changes dmx data
*/
   uint16_t  parse_art_address   ( void );
   
/*!
* @brief utility for parsing ArtIPProg packets
*/  
   void parse_art_ipprog( UDP* wUDP );
   
/*!
* @brief initialize data structures
*/
   void  initialize  ( uint8_t* b );
   
/*!
* @brief set address for ArtPollReply/ArtIpProg
*/   
	void  setLocalAddress ( IPAddress address );
	
/*!
* @brief set address for ArtPollReply/ArtIpProg
*/   
	void  setLocalAddressMask ( IPAddress address, IPAddress subnet_mask );
   
/*!
* @brief initialize poll reply buffer
*/
   void  initializePollReply  ( void );
   
};

#endif // ifndef LXWIFIARTNET_H