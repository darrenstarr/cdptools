#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include "cdp.h"
#include "platform.h"

struct s_cdp_neighbor* cdp_neighbor_new(uint8_t version, uint8_t ttl, uint16_t checksum)
{
	struct s_cdp_neighbor *result;

	result = ALLOC_NEW(struct s_cdp_neighbor);
	if (result == NULL)
	{
		LOG_ERROR("cdp_neighbor_new: Failed to allocate memory for result\n");
		return NULL;
	}

	result->remote_ethernet = NULL;
	result->local_iface = NULL;

	result->timestamp.tv_sec = 0;
	result->timestamp.tv_usec = 0;

	result->cdp_proto_ver = version;
	result->cdp_ttl = ttl;
	result->cdp_checksum = checksum;
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

	result->next = NULL;
	result->prev = NULL;

	return result;
}

void cdp_neighbor_delete(struct s_cdp_neighbor *neighbor)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_delete: neighbor is null\n");
		return;
	}

	if (neighbor->device_id != NULL)
		FREE_ARRAY(neighbor->device_id);

	if (neighbor->addresses != NULL)
		ip_address_array_clear_and_delete(neighbor->addresses);

	if (neighbor->software_version != NULL)
		FREE_ARRAY(neighbor->software_version);

	if (neighbor->capabilities != NULL)
		FREE(neighbor->capabilities);

	if (neighbor->platform != NULL)
		FREE_ARRAY(neighbor->platform);

	if (neighbor->cluster_management_protocol != NULL)
		cisco_hello_protocol_delete(neighbor->cluster_management_protocol);

	if (neighbor->vtp_management_domain != NULL)
		FREE_ARRAY(neighbor->vtp_management_domain);

	if (neighbor->port_id != NULL)
		FREE_ARRAY(neighbor->port_id);

	if (neighbor->management_addresses != NULL)
		ip_address_array_clear_and_delete(neighbor->addresses);

	if (neighbor->native_vlan != NULL)
		FREE(neighbor->native_vlan);

	if (neighbor->trust_bitmap != NULL)
		FREE(neighbor->trust_bitmap);

	if (neighbor->untrusted_port_cos != NULL)
		FREE(neighbor->untrusted_port_cos);

	if (neighbor->poe_availability != NULL)
		power_over_ethernet_availability_delete(neighbor->poe_availability);

	FREE(neighbor);
}

int cdp_neighbor_set_device_id(struct s_cdp_neighbor *neighbor, const char *deviceId)
{
	size_t bufferLength;

	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_device_id: neighbor is null\n");
		return -1;
	}

	if (neighbor->device_id != NULL)
		FREE_ARRAY(neighbor->device_id);

	if (deviceId == NULL)
	{
		neighbor->device_id = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(deviceId) + 1;

		neighbor->device_id = ALLOC_NEW_ARRAY(char, bufferLength);

		if (neighbor->device_id == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_device_id: Failed to allocate memory for the new device ID string.\n");
			return -1;
		}

		COPY_MEMORY(deviceId, neighbor->device_id, bufferLength);
	}

	return 0;
}

int cdp_neighbor_clear_addresses(struct s_cdp_neighbor *neighbor)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_clear_addresses: neighbor is null\n");
		return -1;
	}

	if (neighbor->addresses != NULL)
	{
		if (ip_address_array_clear_and_delete(neighbor->addresses) < 0)
		{
			LOG_ERROR("cdp_neighbor_clear_addresses: Failed to dispose of old addresses\n");
			return -1;
		}

		neighbor->addresses = NULL;
	}

	return 0;
}

int cdp_neighbor_provision_address_array(struct s_cdp_neighbor *neighbor, size_t count)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_provision_address_array: neighbor is null\n");
		return -1;
	}

	if (neighbor->addresses != NULL)
	{
		if (cdp_neighbor_clear_addresses(neighbor) < 0)
		{
			LOG_ERROR("cdp_neighbor_provision_address_array: Failed to clear the array before allocating\n");
			return -1;
		}
	}

	neighbor->addresses = ip_address_array_new(count);
	if (neighbor->addresses == NULL)
	{
		LOG_ERROR("Failed to provision storage for the CDP IP addresses\n");
		return -1;
	}

	return 0;
}

int cdp_neighbor_set_address(struct s_cdp_neighbor *neighbor, off_t index, struct sockaddr *address)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_address: neighbor is null\n");
		return -1;
	}

	if (neighbor->addresses == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_address: neighbor address array is null\n");
		return -1;
	}

	return ip_address_set_into(neighbor->addresses, index, address);
}

int cdp_neighbor_set_capabilities(struct s_cdp_neighbor *neighbor, uint32_t capabilities)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_capabilities: neighbor is null\n");
		return -1;
	}

	/* TODO: Consider using flags for whether properties are present instead of using allocation. */
	if (neighbor->capabilities == NULL)
	{
		neighbor->capabilities = ALLOC_NEW(uint32_t);
		if (neighbor->capabilities == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_capabilities: Failed to allocate memory to store capabilities.\n");
			return -1;
		}
	}

	*(neighbor->capabilities) = capabilities;

	return 0;
}

int cdp_neighbor_set_port_id(struct s_cdp_neighbor *neighbor, const char *portId)
{
	size_t bufferLength;

	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_port_id: neighbor is null\n");
		return -1;
	}

	if (neighbor->port_id != NULL)
		FREE_ARRAY(neighbor->port_id);

	if (portId == NULL)
	{
		neighbor->port_id = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(portId) + 1;

		neighbor->port_id = ALLOC_NEW_ARRAY(char, bufferLength);

		if (neighbor->port_id == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_device_id: Failed to allocate memory for the new port_id string.\n");
			return -1;
		}

		COPY_MEMORY(portId, neighbor->port_id, bufferLength);
	}

	return 0;
}

int cdp_neighbor_set_software_version(struct s_cdp_neighbor *neighbor, const char *softwareVersion)
{
	size_t bufferLength;

	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_software_version: neighbor is null\n");
		return -1;
	}

	if (neighbor->software_version != NULL)
		FREE_ARRAY(neighbor->software_version);

	if (softwareVersion == NULL)
	{
		neighbor->software_version = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(softwareVersion) + 1;

		neighbor->software_version = ALLOC_NEW_ARRAY(char, bufferLength);

		if (neighbor->software_version == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_device_id: Failed to allocate memory for the new software version string.\n");
			return -1;
		}

		COPY_MEMORY(softwareVersion, neighbor->software_version, bufferLength);
	}

	return 0;
}

int cdp_neighbor_set_platform(struct s_cdp_neighbor *neighbor, const char *platform)
{
	size_t bufferLength;

	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_platform: neighbor is null\n");
		return -1;
	}

	if (neighbor->platform != NULL)
		FREE_ARRAY(neighbor->platform);

	if (platform == NULL)
	{
		neighbor->platform = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(platform) + 1;

		neighbor->platform = ALLOC_NEW_ARRAY(char, bufferLength);

		if (neighbor->platform == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_device_id: Failed to allocate memory for the new platform string.\n");
			return -1;
		}

		COPY_MEMORY(platform, neighbor->platform, bufferLength);
	}

	return 0;
}

int cdp_neighbor_set_cisco_cluster_management_protocol(struct s_cdp_neighbor *neighbor, struct cisco_hello_protocol *hello)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_platform: neighbor is null\n");
		return -1;
	}

	if (neighbor->cluster_management_protocol != NULL)
		cisco_hello_protocol_delete(neighbor->cluster_management_protocol);

	neighbor->cluster_management_protocol = hello;
	return 0;
}

int cdp_neighbor_set_vtp_management_domain(struct s_cdp_neighbor *neighbor, const char *vtpManagementDomain)
{
	size_t bufferLength;

	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_platform: neighbor is null\n");
		return -1;
	}

	if (neighbor->vtp_management_domain != NULL)
		FREE_ARRAY(neighbor->vtp_management_domain);

	if (vtpManagementDomain == NULL)
	{
		neighbor->vtp_management_domain = NULL;
	}
	else
	{
		/* TODO: Is there a "safe way" to find the string length? Should I make a string class? */
		bufferLength = strlen(vtpManagementDomain) + 1;

		neighbor->vtp_management_domain = ALLOC_NEW_ARRAY(char, bufferLength);

		if (neighbor->vtp_management_domain == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_device_id: Failed to allocate memory for the new platform string.\n");
			return -1;
		}

		COPY_MEMORY(vtpManagementDomain, neighbor->vtp_management_domain, bufferLength);
	}

	return 0;
}

int cdp_neighbor_set_native_vlan(struct s_cdp_neighbor *neighbor, uint16_t nativeVlan)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_native_vlan: neighbor is null\n");
		return -1;
	}

	if (nativeVlan > 4095)
	{
		LOG_CRITICAL("cdp_neighbor_set_native_vlan: Invalid native VLAN (%d)\n", nativeVlan);
		return -1;
	}

	/* TODO: Consider using flags for whether properties are present instead of using allocation. */
	if (neighbor->native_vlan == NULL)
	{
		neighbor->native_vlan = ALLOC_NEW(uint16_t);
		if (neighbor->native_vlan == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_capabilities: Failed to allocate memory to store the native VLAN.\n");
			return -1;
		}
	}

	*(neighbor->native_vlan) = nativeVlan;

	return 0;
}

int cdp_neighbor_set_duplex(struct s_cdp_neighbor *neighbor, ECdpNetworkDuplex duplex)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_duplex: neighbor is null\n");
		return -1;
	}

	neighbor->duplex = duplex;

	return 0;
}

int cdp_neighbor_set_trust_bitmap(struct s_cdp_neighbor *neighbor, uint8_t trustBitmap)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_trust_bitmap: neighbor is null\n");
		return -1;
	}

	/* TODO: Consider using flags for whether properties are present instead of using allocation. */
	if (neighbor->trust_bitmap == NULL)
	{
		neighbor->trust_bitmap = ALLOC_NEW(uint8_t);
		if (neighbor->trust_bitmap == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_capabilities: Failed to allocate memory to store the trust bitmap.\n");
			return -1;
		}
	}

	*(neighbor->trust_bitmap) = trustBitmap;

	return 0;
}

int cdp_neighbor_set_untrusted_port_cos(struct s_cdp_neighbor *neighbor, uint8_t untrustedPortCoS)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_trust_bitmap: neighbor is null\n");
		return -1;
	}

	/* TODO: Consider using flags for whether properties are present instead of using allocation. */
	if (neighbor->untrusted_port_cos == NULL)
	{
		neighbor->untrusted_port_cos = ALLOC_NEW(uint8_t);
		if (neighbor->untrusted_port_cos == NULL)
		{
			LOG_ERROR("cdp_neighbor_set_capabilities: Failed to allocate memory to store the untrusted port CoS.\n");
			return -1;
		}
	}

	*(neighbor->untrusted_port_cos) = untrustedPortCoS;

	return 0;
}

int cdp_neighbor_clear_management_addresses(struct s_cdp_neighbor *neighbor)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_clear_management_addresses: neighbor is null\n");
		return -1;
	}

	if (neighbor->management_addresses != NULL)
	{
		if (ip_address_array_clear_and_delete(neighbor->management_addresses) < 0)
		{
			LOG_ERROR("cdp_neighbor_clear_management_addresses: Failed to dispose of old addresses\n");
			return -1;
		}

		neighbor->management_addresses = NULL;
	}

	return 0;
}

int cdp_neighbor_provision_management_address_array(struct s_cdp_neighbor *neighbor, size_t count)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_provision_management_address_array: management neighbor is null\n");
		return -1;
	}

	if (neighbor->management_addresses != NULL)
	{
		if (cdp_neighbor_clear_management_addresses(neighbor) < 0)
		{
			LOG_ERROR("cdp_neighbor_provision_management_address_array: Failed to clear the array before allocating\n");
			return -1;
		}
	}

	neighbor->management_addresses = ip_address_array_new(count);
	if (neighbor->management_addresses == NULL)
	{
		LOG_ERROR("cdp_neighbor_provision_management_address_array: Failed to provision storage for the CDP IP addresses\n");
		return -1;
	}

	return 0;
}

int cdp_neighbor_set_management_address(struct s_cdp_neighbor *neighbor, off_t index, struct sockaddr *address)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_management_address: neighbor is null\n");
		return -1;
	}

	if (neighbor->management_addresses == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_management_address: neighbor management address array is null\n");
		return -1;
	}

	return ip_address_set_into(neighbor->management_addresses, index, address);
}

int cdp_neighbor_set_poe_availability(struct s_cdp_neighbor *neighbor, struct power_over_ethernet_availability *poe)
{
	if (neighbor == NULL)
	{
		LOG_CRITICAL("cdp_neighbor_set_poe_availability: neighbor is null\n");
		return -1;
	}

	if (neighbor->poe_availability != NULL)
		power_over_ethernet_availability_delete(neighbor->poe_availability);

	neighbor->poe_availability = poe;

	return 0;
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

			_P("%02X%02X", address6->sin6_addr.__in6_u.__u6_addr8[i], address6->sin6_addr.__in6_u.__u6_addr8[i + 1]);
		}
	}
	else
	{
		_P("<unknown format>");
	}
}

void printCapabilities(uint32_t caps)
{
	if ((caps & CDP_CAPABILITY_L3R) == CDP_CAPABILITY_L3R)
		_P("(Routing) ");

	if ((caps & CDP_CAPABILITY_L2TB) == CDP_CAPABILITY_L2TB)
		_P("(Transparent bridging) ");

	if ((caps & CDP_CAPABILITY_L2SRB) == CDP_CAPABILITY_L2SRB)
		_P("(Source-Route bridging) ");

	if ((caps & CDP_CAPABILITY_L2SW) == CDP_CAPABILITY_L2SW)
		_P("(Switching) ");

	if ((caps & CDP_CAPABILITY_L3TXRX) == CDP_CAPABILITY_L3TXRX)
		_P("(Host) ");

	if ((caps & CDP_CAPABILITY_IGRP) == CDP_CAPABILITY_IGRP)
		_P("(IGMP) ");

	if ((caps & CDP_CAPABILITY_L1) == CDP_CAPABILITY_L1)
		_P("(Repeater) ");

	_P("\n");
}

void cdp_neighbor_dump(struct s_cdp_neighbor *neighbor)
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

	_P("Protocol Hello: ");
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
}
