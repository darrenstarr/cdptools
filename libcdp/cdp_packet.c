#include "cdp_packet.h"
#include "platform/platform.h"

struct cdp_packet* cdp_packet_new(uint8_t version, uint8_t ttl, uint16_t checksum)
{
	struct cdp_packet *result;

	result = ALLOC_NEW(struct cdp_packet);
	if (result == NULL)
	{
		LOG_ERROR("cdp_packet_new: Failed to allocate memory for result\n");
		return NULL;
	}

	result->cdp_proto_ver = version;
	result->cdp_ttl = ttl;
	result->cdp_checksum = checksum;
	result->device_id = NULL;
	result->software_version = NULL;
	result->addresses = NULL;
	result->capabilities = NULL;
	result->platform = NULL;
	result->cluster_management_protocol = NULL;
	result->port_id = NULL;
	result->management_addresses = NULL;
	result->odr_prefixes = NULL;
	result->native_vlan = NULL;
	result->duplex = DuplexUnset;
	result->trust_bitmap = NULL;
	result->untrusted_port_cos = NULL;
	result->poe_availability = NULL;
	result->vtp_management_domain = NULL;
	result->startup_native_vlan = NULL;

	return result;
}

void cdp_packet_delete(struct cdp_packet *packet)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_delete: neighbor is null\n");
		return;
	}

	if (packet->device_id != NULL)
		FREE_ARRAY(packet->device_id);

	if (packet->addresses != NULL)
		ip_address_array_clear_and_delete(packet->addresses);

	if (packet->software_version != NULL)
		FREE_ARRAY(packet->software_version);

	if (packet->capabilities != NULL)
		FREE(packet->capabilities);

	if (packet->platform != NULL)
		FREE_ARRAY(packet->platform);

	if (packet->odr_prefixes != NULL)
		ip_prefix_array_clear_and_delete(packet->odr_prefixes);

	if (packet->cluster_management_protocol != NULL)
		cisco_cluster_management_protocol_delete(packet->cluster_management_protocol);

	if (packet->vtp_management_domain != NULL)
		FREE_ARRAY(packet->vtp_management_domain);

	if (packet->port_id != NULL)
		FREE_ARRAY(packet->port_id);

	if (packet->management_addresses != NULL)
		ip_address_array_clear_and_delete(packet->management_addresses);

	if (packet->native_vlan != NULL)
		FREE(packet->native_vlan);

	if (packet->trust_bitmap != NULL)
		FREE(packet->trust_bitmap);

	if (packet->untrusted_port_cos != NULL)
		FREE(packet->untrusted_port_cos);

	if (packet->poe_availability != NULL)
		power_over_ethernet_availability_delete(packet->poe_availability);

	if (packet->startup_native_vlan != NULL)
		FREE_ARRAY(packet->startup_native_vlan);

	FREE(packet);
}

int cdp_packet_set_device_id(struct cdp_packet *packet, const char *deviceId)
{
	size_t bufferLength;

	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_device_id: neighbor is null\n");
		return -1;
	}

	if (packet->device_id != NULL)
		FREE_ARRAY(packet->device_id);

	if (deviceId == NULL)
	{
		packet->device_id = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(deviceId) + 1;

		packet->device_id = ALLOC_NEW_ARRAY(char, bufferLength);

		if (packet->device_id == NULL)
		{
			LOG_ERROR("cdp_packet_set_device_id: Failed to allocate memory for the new device ID string.\n");
			return -1;
		}

		COPY_MEMORY(deviceId, packet->device_id, bufferLength);
	}

	return 0;
}

int cdp_packet_clear_addresses(struct cdp_packet *packet)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_clear_addresses: neighbor is null\n");
		return -1;
	}

	if (packet->addresses != NULL)
	{
		if (ip_address_array_clear_and_delete(packet->addresses) < 0)
		{
			LOG_ERROR("cdp_packet_clear_addresses: Failed to dispose of old addresses\n");
			return -1;
		}

		packet->addresses = NULL;
	}

	return 0;
}

int cdp_packet_provision_address_array(struct cdp_packet *packet, size_t count)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_provision_address_array: neighbor is null\n");
		return -1;
	}

	if (packet->addresses != NULL)
	{
		if (cdp_packet_clear_addresses(packet) < 0)
		{
			LOG_ERROR("cdp_packet_provision_address_array: Failed to clear the array before allocating\n");
			return -1;
		}
	}

	packet->addresses = ip_address_array_new(count);
	if (packet->addresses == NULL)
	{
		LOG_ERROR("Failed to provision storage for the CDP IP addresses\n");
		return -1;
	}

	return 0;
}

int cdp_packet_set_address(struct cdp_packet *packet, off_t index, struct sockaddr *address)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_address: neighbor is null\n");
		return -1;
	}

	if (packet->addresses == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_address: neighbor address array is null\n");
		return -1;
	}

	return ip_address_array_set_into(packet->addresses, index, address);
}

int cdp_packet_set_address_copy(struct cdp_packet *packet, off_t index, struct sockaddr *address)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_address_copy: neighbor is null\n");
		return -1;
	}

	if (packet->addresses == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_address_copy: neighbor address array is null\n");
		return -1;
	}

	return ip_address_array_copy_into(packet->addresses, index, address);
}

int cdp_packet_set_address_ipv4_uint32(struct cdp_packet *packet, off_t index, uint32_t address)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_address_ipv4_uint32: neighbor is null\n");
		return -1;
	}

	if (packet->addresses == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_address_ipv4_uint32: neighbor address array is null\n");
		return -1;
	}

	return ip_address_array_set_into_ipv4_uint32(packet->addresses, index, address);
}

int cdp_packet_set_capabilities(struct cdp_packet *packet, uint32_t capabilities)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_capabilities: neighbor is null\n");
		return -1;
	}

	/* TODO: Consider using flags for whether properties are present instead of using allocation. */
	if (packet->capabilities == NULL)
	{
		packet->capabilities = ALLOC_NEW(uint32_t);
		if (packet->capabilities == NULL)
		{
			LOG_ERROR("cdp_packet_set_capabilities: Failed to allocate memory to store capabilities.\n");
			return -1;
		}
	}

	*(packet->capabilities) = capabilities;

	return 0;
}

int cdp_packet_set_port_id(struct cdp_packet *packet, const char *portId)
{
	size_t bufferLength;

	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_port_id: neighbor is null\n");
		return -1;
	}

	if (packet->port_id != NULL)
		FREE_ARRAY(packet->port_id);

	if (portId == NULL)
	{
		packet->port_id = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(portId) + 1;

		packet->port_id = ALLOC_NEW_ARRAY(char, bufferLength);

		if (packet->port_id == NULL)
		{
			LOG_ERROR("cdp_packet_set_device_id: Failed to allocate memory for the new port_id string.\n");
			return -1;
		}

		COPY_MEMORY(portId, packet->port_id, bufferLength);
	}

	return 0;
}

int cdp_packet_set_software_version(struct cdp_packet *packet, const char *softwareVersion)
{
	size_t bufferLength;

	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_software_version: neighbor is null\n");
		return -1;
	}

	if (packet->software_version != NULL)
		FREE_ARRAY(packet->software_version);

	if (softwareVersion == NULL)
	{
		packet->software_version = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(softwareVersion) + 1;

		packet->software_version = ALLOC_NEW_ARRAY(char, bufferLength);

		if (packet->software_version == NULL)
		{
			LOG_ERROR("cdp_packet_set_device_id: Failed to allocate memory for the new software version string.\n");
			return -1;
		}

		COPY_MEMORY(softwareVersion, packet->software_version, bufferLength);
	}

	return 0;
}

int cdp_packet_set_platform(struct cdp_packet *packet, const char *platform)
{
	size_t bufferLength;

	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_platform: neighbor is null\n");
		return -1;
	}

	if (packet->platform != NULL)
		FREE_ARRAY(packet->platform);

	if (platform == NULL)
	{
		packet->platform = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(platform) + 1;

		packet->platform = ALLOC_NEW_ARRAY(char, bufferLength);

		if (packet->platform == NULL)
		{
			LOG_ERROR("cdp_packet_set_device_id: Failed to allocate memory for the new platform string.\n");
			return -1;
		}

		COPY_MEMORY(platform, packet->platform, bufferLength);
	}

	return 0;
}

int cdp_packet_clear_odr_prefixes(struct cdp_packet *packet)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_clear_odr_prefixes: neighbor is null\n");
		return -1;
	}

	if (packet->odr_prefixes != NULL)
	{
		if (ip_prefix_array_clear_and_delete(packet->odr_prefixes) < 0)
		{
			LOG_ERROR("cdp_packet_clear_odr_prefixes: Failed to dispose of old ODR IP prefixes\n");
			return -1;
		}

		packet->odr_prefixes = NULL;
	}

	return 0;
}

int cdp_packet_provision_odr_ip_prefix_array(struct cdp_packet *packet, size_t count)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_provision_odr_ip_prefix_array: neighbor is null\n");
		return -1;
	}

	if (packet->odr_prefixes != NULL)
	{
		if (cdp_packet_clear_odr_prefixes(packet) < 0)
		{
			LOG_ERROR("cdp_packet_provision_odr_ip_prefix_array: Failed to clear the array before allocating\n");
			return -1;
		}
	}

	packet->odr_prefixes = ip_prefix_array_new(count);
	if (packet->odr_prefixes == NULL)
	{
		LOG_ERROR("cdp_packet_provision_odr_ip_prefix_array: Failed to provision storage for the CDP ODRP IP prefixes\n");
		return -1;
	}

	return 0;
}

int cdp_packet_set_odr_ip_prefix(struct cdp_packet *packet, off_t index, struct ip_prefix *prefix)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_odr_ip_prefix: neighbor is null\n");
		return -1;
	}

	if (packet->odr_prefixes == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_odr_ip_prefix: neighbor ODR IP prefix array is null\n");
		return -1;
	}

	return ip_prefix_array_set_into(packet->odr_prefixes, index, prefix);
}

int cdp_packet_set_cisco_cluster_management_protocol(struct cdp_packet *packet, struct cisco_cluster_management_protocol *clusterProtocol)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_platform: neighbor is null\n");
		return -1;
	}

	if (packet->cluster_management_protocol != NULL)
		cisco_cluster_management_protocol_delete(packet->cluster_management_protocol);

	packet->cluster_management_protocol = clusterProtocol;
	return 0;
}

int cdp_packet_set_vtp_management_domain(struct cdp_packet *packet, const char *vtpManagementDomain)
{
	size_t bufferLength;

	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_platform: neighbor is null\n");
		return -1;
	}

	if (packet->vtp_management_domain != NULL)
		FREE_ARRAY(packet->vtp_management_domain);

	if (vtpManagementDomain == NULL)
	{
		packet->vtp_management_domain = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(vtpManagementDomain) + 1;

		packet->vtp_management_domain = ALLOC_NEW_ARRAY(char, bufferLength);

		if (packet->vtp_management_domain == NULL)
		{
			LOG_ERROR("cdp_packet_set_device_id: Failed to allocate memory for the new platform string.\n");
			return -1;
		}

		COPY_MEMORY(vtpManagementDomain, packet->vtp_management_domain, bufferLength);
	}

	return 0;
}

int cdp_packet_set_native_vlan(struct cdp_packet *packet, uint16_t nativeVlan)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_native_vlan: neighbor is null\n");
		return -1;
	}

	if (nativeVlan > 4095)
	{
		LOG_CRITICAL("cdp_packet_set_native_vlan: Invalid native VLAN (%d)\n", nativeVlan);
		return -1;
	}

	/* TODO: Consider using flags for whether properties are present instead of using allocation. */
	if (packet->native_vlan == NULL)
	{
		packet->native_vlan = ALLOC_NEW(uint16_t);
		if (packet->native_vlan == NULL)
		{
			LOG_ERROR("cdp_packet_set_capabilities: Failed to allocate memory to store the native VLAN.\n");
			return -1;
		}
	}

	*(packet->native_vlan) = nativeVlan;

	return 0;
}

int cdp_packet_set_duplex(struct cdp_packet *packet, ECdpNetworkDuplex duplex)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_duplex: neighbor is null\n");
		return -1;
	}

	packet->duplex = duplex;

	return 0;
}

int cdp_packet_set_trust_bitmap(struct cdp_packet *packet, uint8_t trustBitmap)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_trust_bitmap: neighbor is null\n");
		return -1;
	}

	/* TODO: Consider using flags for whether properties are present instead of using allocation. */
	if (packet->trust_bitmap == NULL)
	{
		packet->trust_bitmap = ALLOC_NEW(uint8_t);
		if (packet->trust_bitmap == NULL)
		{
			LOG_ERROR("cdp_packet_set_capabilities: Failed to allocate memory to store the trust bitmap.\n");
			return -1;
		}
	}

	*(packet->trust_bitmap) = trustBitmap;

	return 0;
}

int cdp_packet_set_untrusted_port_cos(struct cdp_packet *packet, uint8_t untrustedPortCoS)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_trust_bitmap: neighbor is null\n");
		return -1;
	}

	/* TODO: Consider using flags for whether properties are present instead of using allocation. */
	if (packet->untrusted_port_cos == NULL)
	{
		packet->untrusted_port_cos = ALLOC_NEW(uint8_t);
		if (packet->untrusted_port_cos == NULL)
		{
			LOG_ERROR("cdp_packet_set_capabilities: Failed to allocate memory to store the untrusted port CoS.\n");
			return -1;
		}
	}

	*(packet->untrusted_port_cos) = untrustedPortCoS;

	return 0;
}

int cdp_packet_clear_management_addresses(struct cdp_packet *packet)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_clear_management_addresses: neighbor is null\n");
		return -1;
	}

	if (packet->management_addresses != NULL)
	{
		if (ip_address_array_clear_and_delete(packet->management_addresses) < 0)
		{
			LOG_ERROR("cdp_packet_clear_management_addresses: Failed to dispose of old addresses\n");
			return -1;
		}

		packet->management_addresses = NULL;
	}

	return 0;
}

int cdp_packet_provision_management_address_array(struct cdp_packet *packet, size_t count)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_provision_management_address_array: management neighbor is null\n");
		return -1;
	}

	if (packet->management_addresses != NULL)
	{
		if (cdp_packet_clear_management_addresses(packet) < 0)
		{
			LOG_ERROR("cdp_packet_provision_management_address_array: Failed to clear the array before allocating\n");
			return -1;
		}
	}

	packet->management_addresses = ip_address_array_new(count);
	if (packet->management_addresses == NULL)
	{
		LOG_ERROR("cdp_packet_provision_management_address_array: Failed to provision storage for the CDP IP addresses\n");
		return -1;
	}

	return 0;
}

int cdp_packet_set_management_address(struct cdp_packet *packet, off_t index, struct sockaddr *address)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_management_address: neighbor is null\n");
		return -1;
	}

	if (packet->management_addresses == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_management_address: neighbor management address array is null\n");
		return -1;
	}

	return ip_address_array_set_into(packet->management_addresses, index, address);
}

int cdp_packet_set_poe_availability(struct cdp_packet *neighbor, struct power_over_ethernet_availability *poe)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_poe_availability: neighbor is null\n");
		return -1;
	}

	if (neighbor->poe_availability != NULL)
		power_over_ethernet_availability_delete(neighbor->poe_availability);

	neighbor->poe_availability = poe;

	return 0;
}

int cdp_packet_set_startup_native_vlan(struct cdp_packet *packet, const char *startupNativeVlan)
{
	size_t bufferLength;

	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_set_startupNativeVlan: neighbor is null\n");
		return -1;
	}

	if (packet->startup_native_vlan != NULL)
		FREE_ARRAY(packet->startup_native_vlan);

	if (startupNativeVlan == NULL)
	{
		packet->startup_native_vlan = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(startupNativeVlan) + 1;

		packet->startup_native_vlan = ALLOC_NEW_ARRAY(char, bufferLength);

		if (packet->startup_native_vlan == NULL)
		{
			LOG_ERROR("cdp_packet_set_startupNativeVlan: Failed to allocate memory for the new startup native VLAN string.\n");
			return -1;
		}

		COPY_MEMORY(startupNativeVlan, packet->startup_native_vlan, bufferLength);
	}

	return 0;
}

int cdp_packet_write_version(const struct cdp_packet *packet, struct stream_writer *writer)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_version: packet is NULL\n");
		return -1;
	}

	if (writer == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_version: writer is NULL\n");
		return -1;
	}

	return stream_writer_put8(writer, packet->cdp_proto_ver);
}

int cdp_packet_write_ttl(const struct cdp_packet *packet, struct stream_writer *writer)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_ttl: packet is NULL\n");
		return -1;
	}

	if (writer == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_ttl: writer is NULL\n");
		return -1;
	}

	return stream_writer_put8(writer, packet->cdp_ttl);
}

int cdp_packet_write_string_tlv(struct stream_writer *writer, ECdpTlv tlv, const char *value, bool is_required)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_string_tlv: writer is NULL\n");
		return -1;
	}

	if (value == NULL)
	{
		if (is_required)
		{
			LOG_ERROR("cdp_packet_write_string_tlv: value is required but is NULL\n");
			return -1;
		}

		LOG_DEBUG("cdp_packet_write_string_tlv: value is required but is optional\n");
		return 0;
	}

	if (stream_writer_put16(writer, (uint16_t)tlv) < 0)
	{
		LOG_DEBUG("cdp_packet_write_string_tlv: failed to serialize TLV type\n");
		return -1;
	}

	if (stream_writer_put16(writer, (uint16_t)(strlen(value) + 4)) < 0)
	{
		LOG_DEBUG("cdp_packet_write_string_tlv: failed to serialize TLV length\n");
		return -1;
	}

	if (stream_writer_put_string(writer, value) < 0)
	{
		LOG_DEBUG("cdp_packet_write_string_tlv: failed to serialize TLV value\n");
		return -1;
	}

	return 0;
}

int cdp_packet_write_uint8_tlv(struct stream_writer *writer, ECdpTlv tlv, const uint8_t value)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_uint8_tlv: writer is NULL\n");
		return -1;
	}

	if (stream_writer_put16(writer, (uint16_t)tlv) < 0)
	{
		LOG_DEBUG("cdp_packet_write_uint8_tlv: failed to serialize TLV type\n");
		return -1;
	}

	if (stream_writer_put16(writer, (uint16_t)5) < 0)
	{
		LOG_DEBUG("cdp_packet_write_uint8_tlv: failed to serialize TLV length\n");
		return -1;
	}

	if (stream_writer_put8(writer, value) < 0)
	{
		LOG_DEBUG("cdp_packet_write_uint8_tlv: failed to serialize TLV value\n");
		return -1;
	}

	return 0;
}

int cdp_packet_write_uint32_tlv(struct stream_writer *writer, ECdpTlv tlv, const uint32_t value)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_uint32_tlv: writer is NULL\n");
		return -1;
	}

	if (stream_writer_put16(writer, (uint16_t)tlv) < 0)
	{
		LOG_DEBUG("cdp_packet_write_uint32_tlv: failed to serialize TLV type\n");
		return -1;
	}

	if (stream_writer_put16(writer, (uint16_t)8) < 0)
	{
		LOG_DEBUG("cdp_packet_write_uint32_tlv: failed to serialize TLV length\n");
		return -1;
	}

	if (stream_writer_put32(writer, value) < 0)
	{
		LOG_DEBUG("cdp_packet_write_uint32_tlv: failed to serialize TLV value\n");
		return -1;
	}

	return 0;
}

int cdp_packet_write_uint32ptr_tlv(struct stream_writer *writer, ECdpTlv tlv, const uint32_t *value, bool is_required)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_uint32ptr_tlv: writer is NULL\n");
		return -1;
	}

	if (value == NULL)
	{
		if (is_required)
		{
			LOG_ERROR("cdp_packet_write_uint32ptr_tlv: value is required but is NULL\n");
			return -1;
		}

		LOG_DEBUG("cdp_packet_write_uint32ptr_tlv: value is required but is optional\n");
		return 0;
	}

	return cdp_packet_write_uint32_tlv(writer, tlv, *value);
}

int cdp_packet_write_addresses_tlv(struct stream_writer *writer, ECdpTlv tlv, struct ip_address_array *value, bool is_required)
{
	size_t length = 8;
	size_t i;

	if (writer == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_addresses_tlv: writer is NULL\n");
		return -1;
	}

	if (value == NULL)
	{
		if (is_required)
		{
			LOG_ERROR("cdp_packet_write_addresses_tlv: value is required but is NULL\n");
			return -1;
		}

		LOG_DEBUG("cdp_packet_write_addresses_tlv: value is required but is optional\n");
		return 0;
	}

	if (is_required && value->count == 0)
	{
		LOG_ERROR("cdp_packet_write_addresses_tlv: value is required but contains no entries\n");
		return -1;
	}

	if (!is_required && value->count == 0)
	{
		LOG_DEBUG("cdp_packet_write_addresses_tlv: value is not required but contains no entries.\n");
		return 0;
	}

	if (value->addresses == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_addresses_tlv: addresses is uninitialized\n");
		return -1;
	}

	for (i = 0; i < value->count; i++)
	{
		if (value->addresses[i] == 0)
		{
			LOG_CRITICAL("cdp_packet_write_addresses_tlv: address at index %zd is NULL\n", i);
			return -1;
		}

		switch (value->addresses[i]->sa_family)
		{
		case AF_INET:
			/* Protocol Type:   (NLPID) 0x01 (1 byte)
			* Protocol Length: 0x01 (1 byte)
			* Protocol:        0xCC (1 byte)
			* Address length:  0x0004 (2 bytes)
			* Address:         (4 bytes)
			*/
			length += 9;
			break;

		case AF_INET6:
			/* Protocol Type:   (802.2) 0x02 (1 byte)
			* Protocol Length: 8 (1 byte)
			* Protocol:
			*   SSAP:          0xAA (1 byte)
			*   DSAP:          0xAA (1 byte)
			*   Control:       0x03 (1 byte)
			*   Protocol ID:   0x86DD (2 bytes)
			* Protocol length: 0x0010 (2 bytes)
			* Address:         (16 bytes)
			*/
			length += 26;
			break;

		default:
			LOG_CRITICAL("cdp_packet_write_addresses_tlv: unknown address family at index %zd\n", i);
			return -1;
		}
	}

	if (stream_writer_put16(writer, (uint16_t)tlv) < 0)
	{
		LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write TLV type\n");
		return -1;
	}

	if (stream_writer_put16(writer, (uint16_t)length) < 0)
	{
		LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write TLV length\n");
		return -1;
	}

	if (stream_writer_put32(writer, (uint32_t)value->count) < 0)
	{
		LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write address count\n");
		return -1;
	}

	for (i = 0; i < value->count; i++)
	{
		switch (value->addresses[i]->sa_family)
		{
		case AF_INET:
			if (stream_writer_put8(writer, 0x01) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write NLPID on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put8(writer, 0x01) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write protocol length on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put8(writer, 0xCC) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write protocol ID on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put16(writer, 4) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write address length on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put32(writer, htonl(((struct sockaddr_in *)value->addresses[i])->sin_addr.s_addr)) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write address on index %zd\n", i);
				return -1;
			}
			break;

		case AF_INET6:
			if (stream_writer_put8(writer, 0x02) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write 802.2 protocol type on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put8(writer, 0x08) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write protocol length on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put8(writer, 0xAA) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write SSAP on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put8(writer, 0xAA) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write DSAP on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put8(writer, 0x03) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write SNAP control on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put16(writer, 0x86DD) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write protocol ID on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put8(writer, 0x0010) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write address length on index %zd\n", i);
				return -1;
			}

			if (stream_writer_put_buffer(writer, IPv6Octets((struct sockaddr_in6 *)value->addresses[i]), 16) < 0)
			{
				LOG_ERROR("cdp_packet_write_addresses_tlv: failed to write address on index %zd\n", i);
				return -1;
			}

			break;
		}
	}

	return 0;
}

int cdp_packet_write_tlvs(const struct cdp_packet *packet, struct stream_writer *writer)
{
	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_tlvs: packet is NULL\n");
		return -1;
	}

	if (writer == NULL)
	{
		LOG_CRITICAL("cdp_packet_write_tlvs: writer is NULL\n");
		return -1;
	}

	if (cdp_packet_write_string_tlv(writer, CdpTlvDeviceId, packet->device_id, true) < 0)
	{
		LOG_CRITICAL("cdp_packet_write_tlvs: device_id could not be serialized.\n");
		return -1;
	}

	if (cdp_packet_write_string_tlv(writer, CdpTlvSoftwareVersion, packet->software_version, true) < 0)
	{
		LOG_CRITICAL("cdp_packet_write_tlvs: software version could not be serialized.\n");
		return -1;
	}

	if (cdp_packet_write_string_tlv(writer, CdpTlvPlatform, packet->platform, true) < 0)
	{
		LOG_CRITICAL("cdp_packet_write_tlvs: platform could not be serialized.\n");
		return -1;
	}

	if (cdp_packet_write_string_tlv(writer, CdpTlvPortId, packet->port_id, true) < 0)
	{
		LOG_CRITICAL("cdp_packet_write_tlvs: port ID could not be serialized.\n");
		return -1;
	}

	if (cdp_packet_write_uint32ptr_tlv(writer, CdpTlvCapabilities, packet->capabilities, true) < 0)
	{
		LOG_CRITICAL("cdp_packet_write_tlvs: capabilities could not be serialized.\n");
		return -1;
	}

	if (cdp_packet_write_addresses_tlv(writer, CdpTlvAddresses, packet->addresses, true) < 0)
	{
		LOG_CRITICAL("cdp_packet_write_tlvs: addresses could not be serialized.\n");
		return -1;
	}

	if (packet->cdp_proto_ver == 2)
	{
		if (cdp_packet_write_uint8_tlv(writer, CdpTlvDuplex, (uint8_t)packet->duplex) < 0)
		{
			LOG_CRITICAL("cdp_packet_write_tlvs: duplex could not be serialized.\n");
			return -1;
		}
	}

	return 0;
}

ssize_t cdp_packet_serialize(const struct cdp_packet *packet, uint8_t *buffer, size_t size)
{
	struct stream_writer *writer;
	ssize_t result;

	if (packet == NULL)
	{
		LOG_CRITICAL("cdp_packet_serialize: packet is NULL\n");
		return -1;
	}

	if (packet->cdp_proto_ver == 2 && packet->duplex == DuplexUnset)
	{
		LOG_ERROR("cdp_packet_serialize: CDP verison 2 requires that port duplex is set.\n");
		return -1;
	}

	writer = stream_writer_new(buffer, size);
	if (writer == NULL)
	{
		LOG_ERROR("cdp_packet_serialize: failed to allocate memory for the stream writer\n");
		return -1;
	}

	if (cdp_packet_write_version(packet, writer) < 0)
	{
		LOG_ERROR("cdp_packet_serialize: failed to write the CDP version to the packet.\n");
		stream_writer_delete(writer);
		return -1;
	}

	if (cdp_packet_write_ttl(packet, writer) < 0)
	{
		LOG_ERROR("cdp_packet_serialize: failed to write the CDP hold time to the packet.\n");
		stream_writer_delete(writer);
		return -1;
	}

	if (stream_writer_put16(writer, 0) < 0)
	{
		LOG_ERROR("cdp_packet_serialize: failed to write a placeholder for the frame checksum.\n");
		stream_writer_delete(writer);
		return -1;
	}

	if (cdp_packet_write_tlvs(packet, writer) < 0)
	{
		LOG_ERROR("cdp_packet_serialize: failed to write the CDP TLVs to the packet.\n");
		stream_writer_delete(writer);
		return -1;
	}

	result = stream_writer_length(writer);

	if (stream_writer_inject_checksum(writer, 2) < 0)
	{
		LOG_ERROR("cdp_packet_serialize: failed to inject checksum at position 2.\n");
		stream_writer_delete(writer);
		return -1;
	}

	if (stream_writer_delete(writer) < 0)
	{
		LOG_ERROR("cdp_packet_serialize: failed to do delete the stream writer.\n");
		return -1;
	}

	return result;
}

ssize_t cdp_create_packet(
	const char *outgoing_interface_name,
	const char *device_id_string,
	const char *platform_string,
	const char *software_version_string,
	const struct ip_address_array *addresses,
	int8_t *buffer,
	size_t buffer_length
)
{
	struct cdp_packet *result;
	ssize_t consumed;

	result = cdp_packet_new(2, 180, 0);
	if (result == NULL)
	{
		LOG_CRITICAL("cdp_create_packet: Failed to allocate memory for creating a new CDP packet\n");
		return -1;
	}

	if (cdp_packet_set_device_id(result, device_id_string) < 0)
	{
		LOG_CRITICAL("cdp_create_packet: Failed to set the device ID for the CDP frame.\n");
		cdp_packet_delete(result);
		return -1;
	}

	if (cdp_packet_set_software_version(result, software_version_string) < 0)
	{
		LOG_CRITICAL("cdp_create_packet: Failed to set the software version string for the CDP frame.\n");
		cdp_packet_delete(result);
		return -1;
	}

	if (cdp_packet_set_platform(result, platform_string) < 0)
	{
		LOG_CRITICAL("cdp_create_packet: Failed to set the platform string for the CDP frame.\n");
		cdp_packet_delete(result);
		return -1;
	}

	if (cdp_packet_set_capabilities(result, CdpCapabilityHost | CdpCapabilityIGMP) < 0)
	{
		LOG_CRITICAL("cdp_create_packet: Failed to set the capabilities for the CDP frame.\n");
		cdp_packet_delete(result);
		return -1;
	}

	if (cdp_packet_set_port_id(result, outgoing_interface_name) < 0)
	{
		LOG_CRITICAL("cdp_create_packet: Failed to set the port ID for the CDP frame.\n");
		cdp_packet_delete(result);
		return -1;
	}

	if (cdp_packet_set_duplex(result, DuplexFull) < 0)
	{
		LOG_CRITICAL("cdp_create_packet: Failed to set the port duplex for the CDP frame.\n");
		cdp_packet_delete(result);
		return -1;
	}

	if (addresses != NULL)
	{
		off_t i;
		if (addresses->addresses == NULL)
		{
			LOG_CRITICAL("cdp_create_packet: The addresses structure within addresses is NULL");
			cdp_packet_delete(result);
			return -1;
		}

		if (cdp_packet_provision_address_array(result, addresses->count) < 0)
		{
			LOG_CRITICAL("cdp_create_packet: Failed to provision storage for addresses\n");
			cdp_packet_delete(result);
			return -1;
		}

		for (i = 0; i < addresses->count; i++)
		{
			if (cdp_packet_set_address_copy(result, i, addresses->addresses[i]) < 0)
			{
				LOG_ERROR("cdp_create_packet: Failed to copy addresses at index " FORMAT_OFF_T " into addresses\n", i);
				cdp_packet_delete(result);
				return -1;
			}
		}
	}

	consumed = cdp_packet_serialize(result, buffer, buffer_length);

	cdp_packet_delete(result);

	if (consumed < 0)
	{
		LOG_CRITICAL("cdp_create_packet: Failed to serialize the CDP frame\n");
		return -1;
	}

	return consumed;
}

void printAddress(const struct sockaddr *address)
{
	if (address->sa_family == AF_INET)
	{
		uint32_t n;
		const struct sockaddr_in *address4 = (const struct sockaddr_in *)address;
		
		n = address4->sin_addr.s_addr;
		_P("%d.%d.%d.%d", (n & 0xFF), ((n >> 8) & 0xFF), ((n >> 16) & 0xFF), ((n >> 24) & 0xFF));
	}
	else if (address->sa_family == AF_INET6)
	{
		int i;
		const struct sockaddr_in6 *address6 = (const struct sockaddr_in6 *)address;

		for (i = 0; i < 16; i += 2)
		{
			if (i > 0)
				_P(":");

			_P("%02X%02X", IPv6Octet(address6, i), IPv6Octet(address6, i + 1));
		}
	}
	else
	{
		_P("<unknown format>");
	}
}

void printCapabilities(uint32_t caps)
{
	if ((caps & CdpCapabilityRouting) == CdpCapabilityRouting)
		_P("(Routing) ");

	if ((caps & CdpCapabilityTransparentBridging) == CdpCapabilityTransparentBridging)
		_P("(Transparent bridging) ");

	if ((caps & CdpCapabilitySourceRouteBridging) == CdpCapabilitySourceRouteBridging)
		_P("(Source-Route bridging) ");

	if ((caps & CdpCapabilitySwitching) == CdpCapabilitySwitching)
		_P("(Switching) ");

	if ((caps & CdpCapabilityHost) == CdpCapabilityHost)
		_P("(Host) ");

	if ((caps & CdpCapabilityIGMP) == CdpCapabilityIGMP)
		_P("(IGMP) ");

	if ((caps & CdpCapabilityRepeater) == CdpCapabilityRepeater)
		_P("(Repeater) ");

	_P("\n");
}

void cdp_packet_dump(struct cdp_packet *neighbor)
{
	int i;

	_P("CDP Neighbor\n");
	_P("-------------\n");
	
	_P("Device ID: ");
	if (neighbor->device_id == NULL)
		_P("<null>\n");
	else
		_P("%s\n", neighbor->device_id);

	_P("Addresses: ");
	if (neighbor->addresses == NULL)
		_P("<null>\n");
	else
	{
		for (i = 0; i < neighbor->addresses->count; i++)
		{
			if (i > 0)
				_P(", ");

			printAddress(neighbor->addresses->addresses[i]);
		}
		_P("\n");
	}

	_P("Remote Port ID: ");
	if (neighbor->port_id == NULL)
		_P("<null>\n");
	else
		_P("%s\n", neighbor->port_id);

	_P("Software version: ");
	if (neighbor->software_version == NULL)
		_P("<null>\n");
	else
		_P("\n%s\n", neighbor->software_version);

	_P("Capabilities: ");
	if (neighbor->capabilities == NULL)
		_P("<null>\n");
	else
		printCapabilities(*(neighbor->capabilities));

	_P("Platform: ");
	if (neighbor->platform == NULL)
		_P("<null>\n");
	else
		_P("%s\n", neighbor->platform);

	_P("Native VLAN: ");
	if (neighbor->native_vlan == NULL)
		_P("<null>\n");
	else
		_P("%d\n", *(neighbor->native_vlan));

	switch (neighbor->duplex)
	{
		case DuplexUnset:
			_P("Duplex: <unset>\n");
			break;

		case DuplexHalf:
			_P("Duplex: Half\n");
			break;

		case DuplexFull:
			_P("Duplex: Full\n");
			break;

		default:
			_P("Duplex: <invalid state>\n");
			break;
	}

	_P("Trust bitmap: ");
	if (neighbor->trust_bitmap == NULL)
		_P("<null>\n");
	else
		_P("0x%02X\n", *(neighbor->trust_bitmap));

	_P("Untrusted port CoS: ");
	if (neighbor->untrusted_port_cos == NULL)
		_P("<null>\n");
	else
		_P("0x%02X\n", *(neighbor->untrusted_port_cos));

	_P("VTP Management Domain: ");
	if (neighbor->vtp_management_domain == NULL)
		_P("<null>\n");
	else
		_P("%s\n", neighbor->vtp_management_domain);

	_P("Management Addresses: ");
	if (neighbor->management_addresses == NULL)
		_P("<null>\n");
	else
	{
		for (i = 0; i < neighbor->management_addresses->count; i++)
		{
			if (i > 0)
				_P(", ");

			printAddress(neighbor->management_addresses->addresses[i]);
		}
		_P("\n");
	}

	_P("Cluster Management Protocol: ");
	if (neighbor->cluster_management_protocol == NULL)
		_P("<null>\n");
	else
	{
		_P("OUI=0x%06X, ", neighbor->cluster_management_protocol->oui);
		_P("Protocol ID=%04X; ", neighbor->cluster_management_protocol->protocol_id);
		_P("\n");	
	}

	_P("Power over Ethernet availability: ");
	if (neighbor->poe_availability == NULL)
		_P("<null>\n");
	else
		_P("\n Power request id: %d, Power management id: %d, Power available: %d, Power management level: %d\n",
			neighbor->poe_availability->request_id,
			neighbor->poe_availability->management_id,
			neighbor->poe_availability->availableMilliwatts,
			neighbor->poe_availability->powerManagementLevel
		);

	_P("ODR IP Prefixes: ");
	if (neighbor->odr_prefixes == NULL)
		_P("<null>\n");
	else
	{
		for (i = 0; i < neighbor->odr_prefixes->count; i++)
		{
			const struct ip_prefix *prefix = neighbor->odr_prefixes->prefixes[i];

			_P("\n    ");
			printAddress(prefix->network);
			_P("/%d", prefix->length);
		}
		_P("\n");
	}
}
