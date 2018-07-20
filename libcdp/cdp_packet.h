#ifndef	_NET_CDP_H
#define	_NET_CDP_H

#include "cisco_cluster_management_protocol.h"
#include "ecdpnetworkduplex.h"
#include "ecdptlv.h"
#include "ip_address_array.h"
#include "ip_prefix_array.h"
#include "power_over_ethernet_availability.h"
#include "stream_writer.h"
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
  *  @param version The CDP version of the packet
  *  @param ttl The TTL in seconds of the packet
  *  @param checksum The checksum from within the packet
  *  @return A new CDP neighbor object or a NULL on error
  */
struct cdp_packet *cdp_packet_new(uint8_t version, uint8_t ttl, uint16_t checksum);

/** Deletes a CDP neighbor object
  *  @param packet The neighbor object to delete.
  */
void cdp_packet_delete(struct cdp_packet *packet);

/** Sets the device ID of the neighbor
  *  @param packet The CDP neighbor object to alter.
  *  @param deviceId The new device Id string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_device_id(struct cdp_packet *packet, const char *deviceId);

/** Clears all the addresses from the neighbor
  *  @param packet The CDP neighbor object
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_clear_addresses(struct cdp_packet *packet);

/** Clears and allocates a new IP address array
  *  @param packet The CDP neighbor object
  *  @param count The number of IP addresses to create an array for.
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_provision_address_array(struct cdp_packet *packet, size_t count);

/** Sets an IP address on the neighbor. This takes ownership of the pointer passed.
  *  @param packet The CDP neighbor object.
  *  @param index The index to set.
  *  @param address The address to set.
  *  @return 0 on success, a negative value on failure
  */
int cdp_packet_set_address(struct cdp_packet *packet, off_t index, struct sockaddr *address);

/** Sets an IP address on the neighbor
  *  @param packet The CDP neighbor object.
  *  @param index The index to set.
  *  @param address The address to set.
  *  @return 0 on success, a negative value on failure
  */
int cdp_packet_set_address_copy(struct cdp_packet *packet, off_t index, struct sockaddr *address);

/** Sets an IP address on the neighbor.
  *  @param packet The CDP neighbor object.
  *  @param index The index to set.
  *  @param address The address to set. (this should be host order)
  *  @return 0 on success, a negative value on failure
  */
int cdp_packet_set_address_ipv4_uint32(struct cdp_packet *packet, off_t index, uint32_t address);

/** Sets the capabilities flags on a neighbor
  *  @param packet The CDP neighbor object.
  *  @param capabilities The flags bitmask to set.
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_set_capabilities(struct cdp_packet *packet, uint32_t capabilities);

/** Sets the port ID string of the neighbor
  *  @param packet The CDP neighbor object to alter.
  *  @param portId The new port ID string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_port_id(struct cdp_packet *packet, const char *portId);

/** Sets the software version string of the neighbor
  *  @param packet The CDP neighbor object to alter.
  *  @param softwareVersion The new software version string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_software_version(struct cdp_packet *packet, const char *softwareVersion);

/** Sets the platform string of the neighbor
  *  @param packet The CDP neighbor object to alter.
  *  @param platform The new platform string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_platform(struct cdp_packet *packet, const char *platform);

/** Clears all the ODR prefixes from the neighbor
*  @param packet The CDP neighbor object
*  @return 0 on success, a negative number on failure.
*/
int cdp_packet_clear_odr_prefixes(struct cdp_packet *packet);

/** Clears and allocates a new ODR prefix array
*  @param packet The CDP neighbor object
*  @param count The number of ODR IP prefixes to create an array for.
*  @return 0 on success, a negative number on failure.
*/
int cdp_packet_provision_odr_ip_prefix_array(struct cdp_packet *packet, size_t count);

/** Sets an ODR IP prefix on the neighbor
*  @param packet The CDP neighbor object.
*  @param index The index to set.
*  @param prefix The ODR IP prefix to set.
*  @return 0 on success, a negative value on failure
*/
int cdp_packet_set_odr_ip_prefix(struct cdp_packet *packet, off_t index, struct ip_prefix *prefix);

/** Sets the Cisco cluster management protocol information and takes possession of the pointer.
  *  @param packet The CDP neighbor object.
  *  @param clusterProtocol The cluster management protocol message
  *  @return 0 on success, a negative value on failure.
  */
int cdp_packet_set_cisco_cluster_management_protocol(struct cdp_packet *packet, struct cisco_cluster_management_protocol *clusterProtocol);

/** Sets the VTP management domain
 *  @param packet The CDP neighbor object to alter.
 *  @param vtpManagementDomain The new VTP management domain string to set.
 *  @return 0 on success, a negative number upon failure.
 */
int cdp_packet_set_vtp_management_domain(struct cdp_packet *packet, const char *vtpManagementDomain);

/** Sets the native VLAN on a neighbor.
  *  @param packet The CDP neighbor object.
  *  @param nativeVlan The native vlan value
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_set_native_vlan(struct cdp_packet *packet, uint16_t nativeVlan);

/** Sets the network duplex on a neighbor
  *  @param packet The CDP neighbor object.
  *  @param duplex The value to set.
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_set_duplex(struct cdp_packet *packet, ECdpNetworkDuplex duplex);

/** No idea what this is
  *  @param packet The CDP neighbor object.
  *  @param trustBitmap The trust bitmap
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_set_trust_bitmap(struct cdp_packet *packet, uint8_t trustBitmap);

/** Set the COS available on the port when it's not trusted.
  *  @param packet The CDP neighbor object.
  *  @param untrustedPortCoS
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_set_untrusted_port_cos(struct cdp_packet *packet, uint8_t untrustedPortCoS);

/** Clears all the management addresses from the neighbor
  *  @param packet The CDP neighbor object
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_clear_management_addresses(struct cdp_packet *packet);

/** Clears and allocates a new management IP address array
  *  @param packet The CDP neighbor object
  *  @param count The number of IP addresses to create an array for.
  *  @return 0 on success, a negative number on failure.
  */
int cdp_packet_provision_management_address_array(struct cdp_packet *packet, size_t count);

/** Sets a management IP address on the neighbor
  *  @param packet The CDP neighbor object.
  *  @param index The index to set.
  *  @param address The address to set.
  *  @return 0 on success, a negative value on failure
  */
int cdp_packet_set_management_address(struct cdp_packet *packet, off_t index, struct sockaddr *address);

/** Sets the power over Ethernet availability information for the link.
  *  @param packet The CDP neighbor object.
  *  @param poe The power over Ethernet availability information.
  *  @return 0 on success or a negative number on error.
  *
  *  This call takes ownership of the poe pointer.
  */
int cdp_packet_set_poe_availability(struct cdp_packet *packet, struct power_over_ethernet_availability *poe);

/** Sets the startup neighbor VLAN
  *  @param packet The CDP neighbor object to alter.
  *  @param startupNativeVlan The new device Id string to set.
  *  @return 0 on success, a negative number upon failure.
  */
int cdp_packet_set_startup_native_vlan(struct cdp_packet *packet, const char *startupNativeVlan);

/** Writes the CDP packet version to a stream writer.
  *  @param packet The CDP packet object.
  *  @param writer The writer object.
  *  @return 0 on success or a negative value on error.
  */
int cdp_packet_write_version(const struct cdp_packet *packet, struct stream_writer *writer);

/** Writes the CDP packet hold time to a stream writer.
  *  @param packet The CDP packet object.
  *  @param writer The writer object.
  *  @return 0 on success or a negative value on error.
  */
int cdp_packet_write_ttl(const struct cdp_packet *packet, struct stream_writer *writer);

/** Writes a null terminated string value to a writer as a TLV object
  *  @param writer The writer object.
  *  @param tlv The TLV identifier.
  *  @param value The string to write.
  *  @param is_required If set to true will generate a critical error if value is NULL.
  *  @return 0 on success or a negative value on error.
  */
int cdp_packet_write_string_tlv(struct stream_writer *writer, ECdpTlv tlv, const char *value, bool is_required);

/** Writes an 8-bit integer value to a writer as a TLV object
  *  @param writer The writer object.
  *  @param tlv The TLV identifier.
  *  @param value The value to write.
  *  @return 0 on success or a negative value on error.
  */
int cdp_packet_write_uint8_tlv(struct stream_writer *writer, ECdpTlv tlv, const uint8_t value);

/** Writes an 32-bit integer value to a writer as a TLV object
  *  @param writer The writer object.
  *  @param tlv The TLV identifier.
  *  @param value The value to write.
  *  @return 0 on success or a negative value on error.
  */
int cdp_packet_write_uint32_tlv(struct stream_writer *writer, ECdpTlv tlv, const uint32_t value);

/** Writes an 32-bit integer value from a pointer to a writer as a TLV object
  *  @param writer The writer object.
  *  @param tlv The TLV identifier.
  *  @param value The value to write.
  *  @param is_required If set to true will generate a critical error if value is NULL.
  *  @return 0 on success or a negative value on error.
  */
int cdp_packet_write_uint32ptr_tlv(struct stream_writer *writer, ECdpTlv tlv, const uint32_t *value, bool is_required);

/** Writes an IP address list value to a writer as a TLV object
  *  @param writer The writer object.
  *  @param tlv The TLV identifier.
  *  @param value The value to write.
  *  @param is_required If set to true will generate a critical error if value is NULL.
  *  @return 0 on success or a negative value on error.
  */
int cdp_packet_write_addresses_tlv(struct stream_writer *writer, ECdpTlv tlv, struct ip_address_array *value, bool is_required);

/** Writes the defined values (some are required) from a packet to a stream writer as TLV objects.
  *  @param packet The CDP packet object.
  *  @param writer The writer object.
  *  @return 0 on success or a negative value on error.
  */
int cdp_packet_write_tlvs(const struct cdp_packet *packet, struct stream_writer *writer);

/** Serializes a CDP packet into the buffer provided.
  *  @param packet The packet to serialize.
  *  @param buffer The buffer to write to.
  *  @param size The size of the buffer in bytes.
  *  @return The number of bytes written to the buffer or a negative value on error.
  */
ssize_t cdp_packet_serialize(const struct cdp_packet *packet, uint8_t *buffer, size_t size);

/** Creates a simple CDP packet. This is basically a helper function to do it "easy"
  *  @param outgoing_interface_name The name of the interface to include in the header.
  *  @param device_id_string The name of the device transmit. Typically the FQDN.
  *  @param platform_string The platform identifier to transmit.
  *  @param software_version_string The software version information. This is a multiline free-form string.
  *  @param addresses The list of IP and IPv6 addresses associated with this interface
  *  @param buffer The buffer to write the packet into.
  *  @param buffer_length The length of the buffer in bytes.
  *  @return The number of bytes in the buffer consumed by the packet or a negative value on error.
  */
ssize_t cdp_create_packet(
	const char *outgoing_interface_name,
	const char *device_id_string,
	const char *platform_string,
	const char *software_version_string,
	const struct ip_address_array *addresses,
	int8_t *buffer,
	size_t buffer_length
);

/** Prints the contents of a CDP neighbor for debugging
  *  @param packet The CDP neighbor object
  */
void cdp_packet_dump(struct cdp_packet *packet);

#endif
