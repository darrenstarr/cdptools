#ifndef	_NET_CDP_H
#define	_NET_CDP_H

#include "cisco_cluster_management_protocol.h"
#include "ecdpnetworkduplex.h"
#include "ecdptlv.h"
#include "ip_address_array.h"
#include "ip_prefix_array.h"
#include "power_over_ethernet_availability.h"
#include "platform/socket.h"
#include "platform/types.h"

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
struct cdp_packet {
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

	/** Cluster management protocol */
	struct cisco_cluster_management_protocol *cluster_management_protocol;

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

  /** The Cisco PnP Startup Native VLAN, formerly Web Management Port */
  char *startup_native_vlan;
};

/** Constructs a new CDP neighbor object
  *  @version: The CDP version of the packet
  *  @ttl: The TTL in seconds of the packet
  *  @checksum: The checksum from within the packet
  *  @return A new CDP neighbor object or a NULL on error
  */
struct cdp_packet *cdp_packet_new(uint8_t version, uint8_t ttl, uint16_t checksum);

/** Deletes a CDP neighbor object
  *  @neighbor: The neighbor object to delete.
  */
void cdp_packet_delete(struct cdp_packet *neighbor);

/** Sets the device ID of the neighbor
  *  @neighbor The CDP neighbor object to alter.
  *  @deviceId The new device Id string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_device_id(struct cdp_packet *neighbor, const char *deviceId);

/** Clears all the addresses from the neighbor
  *  @neighbor The CDP neighbor object
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_clear_addresses(struct cdp_packet *neighbor);

/** Clears and allocates a new IP address array
  *  @neighbor: The CDP neighbor object
  *  @count: The number of IP addresses to create an array for.
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_provision_address_array(struct cdp_packet *neighbor, size_t count);

/** Sets an IP address on the neighbor
  *  @neighbor: The CDP neighbor object.
  *  @index: The index to set.
  *  @address: The address to set.
  *  @return: 0 on success, a negative value on failure
  */
int cdp_packet_set_address(struct cdp_packet *neighbor, off_t index, struct sockaddr *address);

/** Sets the capabilities flags on a neighbor
  *  @neighbor: The CDP neighbor object.
  *  @capabilities: The flags bitmask to set.
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_set_capabilities(struct cdp_packet *neighbor, uint32_t capabilities);

/** Sets the port ID string of the neighbor
  *  @neighbor The CDP neighbor object to alter.
  *  @portId The new port ID string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_port_id(struct cdp_packet *neighbor, const char *portId);

/** Sets the software version string of the neighbor
  *  @neighbor The CDP neighbor object to alter.
  *  @softwareVersion The new software version string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_software_version(struct cdp_packet *neighbor, const char *softwareVersion);

/** Sets the platform string of the neighbor
  *  @neighbor The CDP neighbor object to alter.
  *  @platform The new platform string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_platform(struct cdp_packet *neighbor, const char *platform);

/** Clears all the ODR prefixes from the neighbor
*  @neighbor The CDP neighbor object
*  @return: 0 on success, a negative number on failure.
*/
int cdp_packet_clear_odr_prefixes(struct cdp_packet *neighbor);

/** Clears and allocates a new ODR prefix array
*  @neighbor: The CDP neighbor object
*  @count: The number of ODR IP prefixes to create an array for.
*  @return: 0 on success, a negative number on failure.
*/
int cdp_packet_provision_odr_ip_prefix_array(struct cdp_packet *neighbor, size_t count);

/** Sets an ODR IP prefix on the neighbor
*  @neighbor: The CDP neighbor object.
*  @index: The index to set.
*  @prefix: The ODR IP prefix to set.
*  @return: 0 on success, a negative value on failure
*/
int cdp_packet_set_odr_ip_prefix(struct cdp_packet *neighbor, off_t index, struct ip_prefix *prefix);

/** Sets the Cisco cluster management protocol information and takes possession of the pointer.
  *  @neighbor: The CDP neighbor object.
  *  @clusterProtocol: The cluster management protocol message
  *  @return: 0 on success, a negative value on failure.
  */
int cdp_packet_set_cisco_cluster_management_protocol(struct cdp_packet *neighbor, struct cisco_cluster_management_protocol *clusterProtocol);

/** Sets the VTP management domain
 *  @neighbor The CDP neighbor object to alter.
 *  @vtpManagementDomain The new VTP management domain string to set.
 *  @return 0 on success, a negative number upon failure.
 */
int cdp_packet_set_vtp_management_domain(struct cdp_packet *neighbor, const char *vtpManagementDomain);

/** Sets the native VLAN on a neighbor.
  *  @neighbor: The CDP neighbor object.
  *  @nativeVlan: The native vlan value
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_set_native_vlan(struct cdp_packet *neighbor, uint16_t nativeVlan);

/** Sets the network duplex on a neighbor
  *  @neighbor: The CDP neighbor object.
  *  @duplex: The value to set.
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_set_duplex(struct cdp_packet *neighbor, ECdpNetworkDuplex duplex);

/** No idea what this is
  *  @neighbor: The CDP neighbor object.
  *  @trustBitmap: The trust bitmap
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_set_trust_bitmap(struct cdp_packet *neighbor, uint8_t trustBitmap);

/** Set the COS available on the port when it's not trusted.
  *  @neighbor: The CDP neighbor object.
  *  @untrustedPortCoS
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_set_untrusted_port_cos(struct cdp_packet *neighbor, uint8_t untrustedPortCoS);

/** Clears all the management addresses from the neighbor
  *  @neighbor The CDP neighbor object
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_clear_management_addresses(struct cdp_packet *neighbor);

/** Clears and allocates a new management IP address array
  *  @neighbor: The CDP neighbor object
  *  @count: The number of IP addresses to create an array for.
  *  @return: 0 on success, a negative number on failure.
  */
int cdp_packet_provision_management_address_array(struct cdp_packet *neighbor, size_t count);

/** Sets a management IP address on the neighbor
  *  @neighbor: The CDP neighbor object.
  *  @index: The index to set.
  *  @address: The address to set.
  *  @return: 0 on success, a negative value on failure
  */
int cdp_packet_set_management_address(struct cdp_packet *neighbor, off_t index, struct sockaddr *address);

/** Sets the power over Ethernet availability information for the link.
  *  @neighbor: The CDP neighbor object.
  *  @poe: The power over Ethernet availability information.
  *  @return: 0 on success or a negative number on error.
  *
  *  This call takes ownership of the poe pointer.
  */
int cdp_packet_set_poe_availability(struct cdp_packet *neighbor, struct power_over_ethernet_availability *poe);

/** Sets the startup neighbor VLAN
  *  @neighbor The CDP neighbor object to alter.
  *  @startupNativeVlan The new device Id string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_startup_native_vlan(struct cdp_packet *neighbor, const char *startupNativeVlan);

/** Prints the contents of a CDP neighbor for debugging
  *  @neighbor The CDP neighbor object
  */
void cdp_packet_dump(struct cdp_packet *neighbor);

#endif
