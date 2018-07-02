#include <stdint.h>

#include "cdp.h"
#include "cdp_packet_parser.h"
#include "cisco_hello_protocol.h"
#include "ip_address_array.h"
#include "platform.h"
#include "power_over_ethernet_availability.h"

int cdp_parse_packet(struct stream_reader*reader, struct s_cdp_neighbor **neighbor)
{
	uint8_t cdpVersion;
	uint8_t ttl;
	uint16_t checksum;
	struct s_cdp_neighbor *result;

	LOG_DEBUG("cdp_parse_packet: Reading CDP version\n");
	if (stream_reader_get8(reader, &cdpVersion) < 0)
		return -1;

	if (cdpVersion != 2)
	{
		LOG_ERROR("cdp_parse_packet: CDP version is not 2.\n");
		return -1;
	}

	LOG_DEBUG("cdp_parse_packet: Reading TTL\n");
	if (stream_reader_get8(reader, &ttl) < 0)
		return -1;

	LOG_DEBUG("cdp_parse_packet: TTL is %d\n", ttl);

	LOG_DEBUG("cdp_parse_packet: Reading checksum\n");
	if (stream_reader_get16(reader, &checksum) < 0)
		return -1;

	LOG_DEBUG("cdp_parse_packet: Checksum is %04X\n", checksum);

	result = cdp_neighbor_new(cdpVersion, ttl, checksum);
	if (result == NULL)
	{
		LOG_ERROR("cdp_parse_packet: Failed to allocate resulting object\n");
		return -1;
	}

	while (!stream_reader_at_end(reader))
	{
		uint16_t tlvType;
		uint16_t tlvLength;
		off_t initialPosition;

		initialPosition = stream_reader_get_position(reader);

		LOG_DEBUG("cdp_parse_packet: Reading TLV type (%zd)\n", stream_reader_get_position(reader));
		if (stream_reader_get16(reader, &tlvType) < 0)
		{
			LOG_ERROR("cdp_parse_packet: Failed to read TLV type\n");
			cdp_neighbor_delete(result);
			return -1;
		}

		LOG_DEBUG("cdp_parse_packet: Reading TLV length (%zd)\n", stream_reader_get_position(reader));
		if (stream_reader_get16(reader, &tlvLength) < 0)
		{
			LOG_ERROR("cdp_parse_packet: Failed to read TLV length\n");
			cdp_neighbor_delete(result);
			return -1;
		}

		switch (tlvType)
		{
			case CdpTlvDeviceId:
				{
					char *deviceId;

					if (stream_reader_get_string(reader, &deviceId, (size_t)(tlvLength - 4)) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read device ID string\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_device_id(result, deviceId) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to set the device ID\n");
						FREE_ARRAY(deviceId);
						cdp_neighbor_delete(result);

						return -1;
					}

					FREE_ARRAY(deviceId);
				}
				break;

			case CdpTlvAddresses:
				{
					uint32_t addressCount;
					uint32_t i;

					if (stream_reader_get32(reader, &addressCount) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read address count\n");
						cdp_neighbor_delete(result);
						return -1;
					}

					if (cdp_neighbor_provision_address_array(result, addressCount) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to allocate IP address array\n");
						cdp_neighbor_delete(result);
						return -1;
					}

					for (i = 0; i < addressCount; i++)
					{
						struct sockaddr *item = NULL;

						if (stream_reader_get_address(reader, &item) < 0)
						{
							LOG_ERROR("cdp_parse_packet: Failed to read address\n");
							cdp_neighbor_delete(result);
							return -1;
						}

						cdp_neighbor_set_address(result, i, item);
					}
				}
				break;

			case CdpTlvPortId:
				{
					char *portId;

					if (stream_reader_get_string(reader, &portId, (size_t)(tlvLength - 4)) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read port ID string\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_port_id(result, portId) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to set the port ID\n");
						FREE_ARRAY(portId);
						cdp_neighbor_delete(result);

						return -1;
					}

					FREE_ARRAY(portId);
				}
				break;

			case CdpTlvCapabilities:
				{
					uint32_t capabilities;

					if (stream_reader_get32(reader, &capabilities) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to read capabilities\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_capabilities(result, capabilities) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to set capabilities\n");
						return -1;
					}
				}
				break;

			case CdpTlvSoftwareVersion:
				{
					char *softwareVersion;

					if (stream_reader_get_string(reader, &softwareVersion, (size_t)(tlvLength - 4)) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read software version string\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_software_version(result, softwareVersion) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to set the software version\n");
						FREE_ARRAY(softwareVersion);
						cdp_neighbor_delete(result);

						return -1;
					}

					FREE_ARRAY(softwareVersion);
				}
				break;

			case CdpTlvPlatform:
				{
					char *platform;

					if (stream_reader_get_string(reader, &platform, (size_t)(tlvLength - 4)) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read platform string\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_platform(result, platform) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to set the platform\n");
						FREE_ARRAY(platform);
						cdp_neighbor_delete(result);

						return -1;
					}

					FREE_ARRAY(platform);
				}
				break;

			case CdpTlvProtocolHello:
				{
					struct cisco_hello_protocol *hello;

					if (stream_reader_get_cisco_cluster_management_protocol(reader, &hello) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read cisco cluster management protocol tlv\n");
						return -1;
					}

					if (cdp_neighbor_set_cisco_cluster_management_protocol(result, hello) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to set the cisco cluster management protocol\n");
						cisco_hello_protocol_delete(hello);
						return -1;
					}
				}
				break;

			case CdpTlvVtpManagementDomain:
				{
					char *vtpManagementDomain;

					if (stream_reader_get_string(reader, &vtpManagementDomain, (size_t)(tlvLength - 4)) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read VTP management domain string\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_vtp_management_domain(result, vtpManagementDomain) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to set the VTP management domain\n");
						FREE_ARRAY(vtpManagementDomain);
						cdp_neighbor_delete(result);

						return -1;
					}

					FREE_ARRAY(vtpManagementDomain);
				}
				break;

			case CdpTlvNativeVlan:
				{
					uint16_t nativeVlan;

					if (stream_reader_get16(reader, &nativeVlan) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to read the native VLAN\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_native_vlan(result, nativeVlan) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to set native VLAN\n");
						return -1;
					}
				}
				break;

			case CdpTlvDuplex:
				{
					uint8_t duplex;

					if (stream_reader_get8(reader, &duplex) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to read the link duplex\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_duplex(result, duplex) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to set the link duplex\n");
						return -1;
					}
				}
				break;

			case CdpTlvTrustBitmap:
				{
					uint8_t trustBitmap;

					if (stream_reader_get8(reader, &trustBitmap) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to read the trust bitmap\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_trust_bitmap(result, trustBitmap) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to set the trust bitmap\n");
						return -1;
					}
				}
				break;

			case CdpTlvUntrustedPortCoS:
				{
					uint8_t untrustedPortCoS;

					if (stream_reader_get8(reader, &untrustedPortCoS) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to read the untrusted port CoS\n");
						cdp_neighbor_delete(result);

						return -1;
					}

					if (cdp_neighbor_set_untrusted_port_cos(result, untrustedPortCoS) < 0)
					{
						LOG_ERROR("cdp_parse_packet: failed to set the untrusted port CoS\n");
						return -1;
					}
				}
				break;

			case CdpTlvManagementAddesses:
				{
					uint32_t addressCount;
					uint32_t i;

					if (stream_reader_get32(reader, &addressCount) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read management address count\n");
						cdp_neighbor_delete(result);
						return -1;
					}

					if (cdp_neighbor_provision_management_address_array(result, addressCount) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to allocate management address array\n");
						cdp_neighbor_delete(result);
						return -1;
					}

					for (i = 0; i < addressCount; i++)
					{
						struct sockaddr *item = NULL;

						if (stream_reader_get_address(reader, &item) < 0)
						{
							LOG_ERROR("cdp_parse_packet: Failed to read address\n");
							cdp_neighbor_delete(result);
							return -1;
						}

						cdp_neighbor_set_management_address(result, i, item);
					}
				}
				break;

			case CdpTlvPowerAvailable:
				{
					struct power_over_ethernet_availability *poe;

					if (power_over_ethernet_availability_read(reader, &poe) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to read PoE availability\n");
						cdp_neighbor_delete(result);
						return -1;
					}

					if (cdp_neighbor_set_poe_availability(result, poe) < 0)
					{
						LOG_ERROR("cdp_parse_packet: Failed to set PoE availability\n");
						power_over_ethernet_availability_delete(poe);
						return -1;
					}
				}
				break;

			default:
				LOG_INFORMATIONAL(
					"cdp_parse_packet: Encountered unknown TLV (0x%04X) at position %zd (0x%zX) with length %d bytes\n",
					tlvType,
					initialPosition,
					initialPosition,
					tlvLength
				);

				break;
		}

		stream_reader_set_position(reader, initialPosition + tlvLength);
	}

	*neighbor = result;

	return 0;
}
