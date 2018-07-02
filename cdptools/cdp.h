/********************************************************************

Based on documentation from Cisco, detailing the CDP packet format,
which is available from:
http://www.cisco.com/univercd/cc/td/doc/product/lan/trsrb/frames.htm#xtocid842812

Portions Copyright (c) 2001 Chris Crowther and tom burkart
The authors admit no liability nor provide any warranty for this
software.  This material is provided "AS-IS" and at no charge.

This software is released under the the GNU Public Licence (GPL).

*******************************************************************/

#ifndef	_NET_CDP_H
#define	_NET_CDP_H

#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "cisco_hello_protocol.h"
#include "ip_address_array.h"
#include "ip_prefix_array.h"
#include "power_over_ethernet_availability.h"

/* timer values */
#define CDP_POLL		HZ*5	/* poll the neighbor list every 5
* seconds for expired entires */

enum ECdpTlv
{
	CdpTlvDeviceId = 1,
	CdpTlvAddresses = 2,
	CdpTlvPortId = 3,
	CdpTlvCapabilities = 4,
	CdpTlvSoftwareVersion = 5,
	CdpTlvPlatform = 6,
	CdpTlvODRPrefixes = 7,
	CdpTlvProtocolHello = 8,
	CdpTlvVtpManagementDomain = 9,
	CdpTlvNativeVlan = 10,
	CdpTlvDuplex = 11,
	CdpTlvTrustBitmap = 18,
	CdpTlvUntrustedPortCoS = 19,
	CdpTlvManagementAddesses = 22,
	CdpTlvPowerAvailable = 26
};

typedef enum
{
	DuplexUnset = 0,
	DuplexHalf = 2,
	DuplexFull = 1
} ECdpNetworkDuplex;

/* the capability masks */

/** Layer-3 routing */
static const uint32_t CdpCapabilityRouting = 0x01;

/** Layer-2 transparent bridging */
static const uint32_t CdpCapabilityTransparentBridging = 0x02;

/** Source-Route bridging */
static const uint32_t CdpCapabilitySourceRouteBridging = 0x04;

/** Layer-2 switching */
static const uint32_t CdpCapabilitySwitching = 0x08;

/** Host device */
static const uint32_t CdpCapabilityHost = 0x10;

/** IGMP multicast capable */
static const uint32_t CdpCapabilityIGMP = 0x20;

/** Layer-1 repeater */
static const uint32_t CdpCapabilityRepeater = 0x40;

/** A class for storing a CDP neighbor record */
struct s_cdp_neighbor {
	/** Remote Ethernet MAC address */
	uint8_t *remote_ethernet;

	/** The interface we received the frame on */
	char *local_iface;

	/** The arrival time of the packet */
	struct timeval timestamp;

	/** Protocol version of the CDP packet */
	uint8_t cdp_proto_ver;

	/** The time to live of the CDP packet in seconds */
	uint8_t cdp_ttl;

	/** Standard IP packet checksum */
	uint16_t cdp_checksum;
	
	/** Remote device identification */ // *
	char *device_id;

	/** copy of the address info from the packet */ // *
	struct ip_address_array *addresses;

	/** Remote device port */ // *
	char *port_id;

	/** Reported capabilities */ // *
	uint32_t *capabilities;

	/** Software version on neighbor */ // *
	char *software_version;

	/** Model name */ // *
	char *platform;

	/** Cluster managment protocol */
	struct cisco_hello_protocol *cluster_management_protocol;

	/** IP address prefix */
	struct ip_prefix_array *odr_prefixes;

	/** VTP Management Domain */ // *
	char *vtp_management_domain;

	/** Native VLAN */
	uint16_t *native_vlan;

	/** Network duplex on link */
	ECdpNetworkDuplex duplex;

	/** Trust bitmap */
	uint8_t *trust_bitmap;

	/** Untrusted port CoS */
	uint8_t *untrusted_port_cos;

	/** copy of the management address info from the packet */ // *
	struct ip_address_array *management_addresses;

	/** The power of ethernet information for the link */
	struct power_over_ethernet_availability *poe_availability;

	/** The next neighbor in the linked list */
	struct s_cdp_neighbor *next;

	/** The previous neighbor in the linked list */
	struct s_cdp_neighbor *prev;
};

/** Constructs a new CDP neighbor object
  *  @version: The CDP version of the packet
  *  @ttl: The TTL in seconds of the packet
  *  @checksum: The checksum from within the packet
  *  @return A new CDP neighbor object or a NULL on error
  */
struct s_cdp_neighbor *cdp_neighbor_new(uint8_t version, uint8_t ttl, uint16_t checksum);

/** Deletes a CDP neighbor object
  *  @neighbor: The neighbor object to delete.
  */
void cdp_neighbor_delete(struct s_cdp_neighbor *neighbor);

/** Sets the device ID of the neighbor
*  @neighbor The CDP neighbor object to alter.
*  @deviceId The new device Id string to set.
*  @return 0 on success, a negative number upon failure.
*/
int cdp_neighbor_set_device_id(struct s_cdp_neighbor *neighbor, const char *deviceId);

/** Clears all the addresses from the neighbor
  *  @neighbor The CDP neighbor object
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_clear_addresses(struct s_cdp_neighbor *neighbor);

/** Clears and allocates a new IP address array
  *  @neighbor: The CDP neighbor object
  *  @count: The number of IP addresses to create an array for.
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_provision_address_array(struct s_cdp_neighbor *neighbor, size_t count);

/** Sets an IP address on the neighbor
  *  @neighbor: The CDP neighbor object.
  *  @index: The index to set.
  *  @address: The address to set.
  *  @return: 0 on success, a negative value on failure
  */
int cdp_neighbor_set_address(struct s_cdp_neighbor *neighbor, off_t index, struct sockaddr *address);

/** Sets the capabilities flags on a neighbor
  *  @neighbor: The CDP neighbor object.
  *  @capabilities: The flags bitmask to set.
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_set_capabilities(struct s_cdp_neighbor *neighbor, uint32_t capabilities);

/** Sets the port ID string of the neighbor
  *  @neighbor The CDP neighbor object to alter.
  *  @portId The new port ID string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_neighbor_set_port_id(struct s_cdp_neighbor *neighbor, const char *portId);

/** Sets the software version string of the neighbor
  *  @neighbor The CDP neighbor object to alter.
  *  @softwareVersion The new software version string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_neighbor_set_software_version(struct s_cdp_neighbor *neighbor, const char *softwareVersion);

/** Sets the platform string of the neighbor
  *  @neighbor The CDP neighbor object to alter.
  *  @platform The new platform string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_neighbor_set_platform(struct s_cdp_neighbor *neighbor, const char *platform);

/** Clears all the ODR prefixes from the neighbor
*  @neighbor The CDP neighbor object
*  @return: 0 on success, a negative number on failure.
*/
int cdp_neighbor_clear_odr_prefixes(struct s_cdp_neighbor *neighbor);

/** Clears and allocates a new ODR prefix array
*  @neighbor: The CDP neighbor object
*  @count: The number of ODR IP prefixes to create an array for.
*  @return: 0 on success, a negative number on failure.
*/
int cdp_neighbor_provision_odr_ip_prefix_array(struct s_cdp_neighbor *neighbor, size_t count);

/** Sets an ODR IP prefix on the neighbor
*  @neighbor: The CDP neighbor object.
*  @index: The index to set.
*  @prefix: The ODR IP prefix to set.
*  @return: 0 on success, a negative value on failure
*/
int cdp_neighbor_set_odr_ip_prefix(struct s_cdp_neighbor *neighbor, off_t index, struct ip_prefix *prefix);

/** Sets the Cisco cluster management protocol information and takes possession of the pointer.
  *  @neighbor: The CDP neighbor object.
  *  @hello: The cluster management protocol hello message
  *  @return: 0 on success, a negative value on failure.
  */
int cdp_neighbor_set_cisco_cluster_management_protocol(struct s_cdp_neighbor *neighbor, struct cisco_hello_protocol *hello);

/** Sets the VTP management domain
 *  @neighbor The CDP neighbor object to alter.
 *  @vtpManagementDomain The new VTP management domain string to set.
 *  @return 0 on success, a negative number upon failure.
 */
int cdp_neighbor_set_vtp_management_domain(struct s_cdp_neighbor *neighbor, const char *vtpManagementDomain);

/** Sets the native VLAN on a neighbor.
  *  @neighbor: The CDP neighbor object.
  *  @nativeVlan: The native vlan value
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_set_native_vlan(struct s_cdp_neighbor *neighbor, uint16_t nativeVlan);

/** Sets the network duplex on a neighbor
  *  @neighbor: The CDP neighbor object.
  *  @duplex: The value to set.
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_set_duplex(struct s_cdp_neighbor *neighbor, ECdpNetworkDuplex duplex);

/** No idea what this is
  *  @neighbor: The CDP neighbor object.
  *  @trustBitmap: The trust bitmap
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_set_trust_bitmap(struct s_cdp_neighbor *neighbor, uint8_t trustBitmap);

/** Set the COS available on the port when it's not trusted.
  *  @neighbor: The CDP neighbor object.
  *  @untrustedPortCoS
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_set_untrusted_port_cos(struct s_cdp_neighbor *neighbor, uint8_t untrustedPortCoS);

/** Clears all the management addresses from the neighbor
  *  @neighbor The CDP neighbor object
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_clear_management_addresses(struct s_cdp_neighbor *neighbor);

/** Clears and allocates a new management IP address array
  *  @neighbor: The CDP neighbor object
  *  @count: The number of IP addresses to create an array for.
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_neighbor_provision_management_address_array(struct s_cdp_neighbor *neighbor, size_t count);

/** Sets a management IP address on the neighbor
  *  @neighbor: The CDP neighbor object.
  *  @index: The index to set.
  *  @address: The address to set.
  *  @return: 0 on success, a negative value on failure
  */
int cdp_neighbor_set_management_address(struct s_cdp_neighbor *neighbor, off_t index, struct sockaddr *address);

/** Sets the power over Ethernet availability information for the link.
  *  @neighbor: The CDP neighbor object.
  *  @poe: The power over Ethernet availability information.
  *  @return: 0 on success or a negative number on error.
  *
  *  This call takes ownership of the poe pointer.
  */
int cdp_neighbor_set_poe_availability(struct s_cdp_neighbor *neighbor, struct power_over_ethernet_availability *poe);

/** Prints the contents of a CDP neighbor for debugging
  *  @neighbor The CDP neighbor object
  */
void cdp_neighbor_dump(struct s_cdp_neighbor *neighbor);

/* head struct */
struct s_cdp_neighbors {
	struct s_cdp_neighbor *head;
	struct s_cdp_neighbor *foot;
};

#endif
