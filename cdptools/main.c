#include "../libcdp/cdp_packet.h"
#include "../libcdp/cdp_packet_parser.h"
#include "../libcdp/ecdptlv.h"
#include "../libcdp/cdp_software_version_string.h"
#include "../libcdp/platform/platform.h"

#include <stdio.h>
#include <string.h>

/** This is the software version string sent to all CDP neighbors to describe this device */
static char *cdp_software_version_string = NULL;

int main()
{
	struct cdp_packet *eth0;

	uint8_t frame_buffer[1500];
	ssize_t frame_buffer_length;

	struct stream_reader *reader;
	struct cdp_packet *parsed;
	int rc = 0;

	struct sockaddr_in eth0_address;

	if (generate_cdp_software_version_string(&cdp_software_version_string) < 0)
	{
		LOG_CRITICAL("Failed to generate software version\n");
		return -1;
	}

	eth0 = cdp_packet_new(2, 180, 0);
	if (eth0 == NULL)
	{
		LOG_CRITICAL("Failed to allocate memory for creating a new CDP packet\n");
		return -1;
	}

	if (cdp_packet_set_device_id(eth0, "MyDogIsBetterThanYourDog") < 0)
	{
		LOG_CRITICAL("Failed to set the device ID for the CDP frame.\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	if (cdp_packet_set_software_version(eth0, cdp_software_version_string) < 0)
	{
		LOG_CRITICAL("Failed to set the software version string for the CDP frame.\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	if (cdp_packet_set_platform(eth0, cdp_platform_string) < 0)
	{
		LOG_CRITICAL("Failed to set the platform string for the CDP frame.\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	if (cdp_packet_set_capabilities(eth0, CdpCapabilityHost | CdpCapabilityIGMP) < 0)
	{
		LOG_CRITICAL("Failed to set the capabilities for the CDP frame.\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	if (cdp_packet_set_port_id(eth0, "eth0") < 0)
	{
		LOG_CRITICAL("Failed to set the port ID for the CDP frame.\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	if (cdp_packet_set_duplex(eth0, DuplexFull) < 0)
	{
		LOG_CRITICAL("Failed to set the port duplex for the CDP frame.\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	if (cdp_packet_provision_address_array(eth0, 1) < 0)
	{
		LOG_CRITICAL("Failed to provision storage for addresses\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	memset(&eth0_address, 0, sizeof(struct sockaddr_in));
	eth0_address.sin_family = AF_INET;
	eth0_address.sin_addr.s_addr = htonl(0x0A640101);

	if (cdp_packet_set_address_copy(eth0, 0, (struct sockaddr *)&eth0_address))
	{
		LOG_CRITICAL("Failed to set interface address for eth0\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	frame_buffer_length = cdp_packet_serialize(eth0, frame_buffer, 1500);
	if (frame_buffer_length < 0)
	{
		LOG_CRITICAL("Failed to serialize the CDP frame\n");
		cdp_packet_delete(eth0);
		return -1;
	}

	cdp_packet_delete(eth0);

	reader = stream_reader_new(frame_buffer, (size_t)frame_buffer_length);
	if (reader == NULL)
	{
		LOG_CRITICAL("Failed to allocate memory for stream reader\n");
		return -1;
	}

	if (cdp_parse_packet(reader, &parsed) < 0)
	{
		LOG_CRITICAL("Failed to allocate memory for stream reader\n");
		rc = -1;
	}

	stream_reader_delete(reader);

	if (parsed != NULL)
		cdp_packet_delete(parsed);

	return rc;
}
