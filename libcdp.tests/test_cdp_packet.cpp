#include <gtest/gtest.h>

extern "C" {
#include "../libcdp/cdp_packet.h"
#include "../libcdp/cdp_packet_parser.h"
#include "../libcdp/ecdptlv.h"
#include "../libcdp/cdp_software_version_string.h"
#include "../libcdp/platform/platform.h"
}

#include "cdp_sample_data.h"

/// Verify cdp_packet_new constructs and destructs
TEST(CdpPacket, CdpPacketNew) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	cdp_packet_delete(packet);
}

/// Verify that the device id can be set
TEST(CdpPacket, SetDeviceId) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_device_id(packet, "MyDogIsBetterThanYourDog");

	ASSERT_GE(rc, 0);

	ASSERT_STREQ(packet->device_id, "MyDogIsBetterThanYourDog");

	cdp_packet_delete(packet);
}

/// Verify that the software version string can be set
TEST(CdpPacket, SetSoftwareVersionString) {
	static char *cdp_software_version_string = NULL;

	int rc = generate_cdp_software_version_string(&cdp_software_version_string);

	EXPECT_EQ(0, rc);
	ASSERT_NE(nullptr, cdp_software_version_string);

	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	rc = cdp_packet_set_software_version(packet, cdp_software_version_string);

	ASSERT_GE(rc, 0);

	ASSERT_STREQ(packet->software_version, cdp_software_version_string);

	cdp_packet_delete(packet);
	FREE_ARRAY(cdp_software_version_string);
}

/// Verify that the platform string can be set
TEST(CdpPacket, SetPlatformString) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_platform(packet, cdp_platform_string);

	ASSERT_GE(rc, 0);

	ASSERT_STREQ(packet->platform, cdp_platform_string);

	cdp_packet_delete(packet);
}

/// Verify that the capabilities can be set
TEST(CdpPacket, SetCapabilities) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_capabilities(packet, CdpCapabilityHost | CdpCapabilityIGMP);

	ASSERT_GE(rc, 0);

	ASSERT_NE(nullptr, packet->capabilities);
	ASSERT_EQ(*(packet->capabilities), CdpCapabilityHost | CdpCapabilityIGMP);

	cdp_packet_delete(packet);
}

/// Verify that the port id string can be set
TEST(CdpPacket, SetPortId) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_port_id(packet, "eth0");

	ASSERT_GE(rc, 0);

	ASSERT_STREQ(packet->port_id, "eth0");

	cdp_packet_delete(packet);
}

/// Verify the duplex can be set
TEST(CdpPacket, SetDuplex) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	ASSERT_EQ(packet->duplex, DuplexUnset);

	int rc = cdp_packet_set_duplex(packet, DuplexFull);
	ASSERT_GE(rc, 0);
	ASSERT_EQ(packet->duplex, DuplexFull);

	rc = cdp_packet_set_duplex(packet, DuplexHalf);
	ASSERT_GE(rc, 0);
	ASSERT_EQ(packet->duplex, DuplexHalf);

	cdp_packet_delete(packet);
}

/// Verify that the interface addresses can be allocated and initialized properly
TEST(CdpPacket, ProvisionAddresses) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	// Allocate enough space for three addresses
	int rc = cdp_packet_provision_address_array(packet, 3);
	ASSERT_GE(rc, 0);
	ASSERT_NE(nullptr, packet->addresses);
	ASSERT_NE(nullptr, packet->addresses->addresses);
	ASSERT_EQ(3, packet->addresses->count);

	// Varify all the addresses are initialized to null
	for (int i = 0; i < packet->addresses->count; i++)
		ASSERT_EQ(nullptr, packet->addresses->addresses[i]);

	cdp_packet_delete(packet);
}

/// Verify that IPv4 addresses can be set from sockaddr_in
TEST(CdpPacket, SetIPv4Addresses) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	// Allocate enough space for three addresses
	int rc = cdp_packet_provision_address_array(packet, 3);
	ASSERT_GE(rc, 0);
	ASSERT_NE(nullptr, packet->addresses);
	ASSERT_NE(nullptr, packet->addresses->addresses);

	// Prepare the structure
	struct sockaddr_in ipv4_address;
	memset(&ipv4_address, 0, sizeof(struct sockaddr_in));
	ipv4_address.sin_family = AF_INET;
	ipv4_address.sin_addr.s_addr = htonl(0x0A640101);

	// Set the second address
	rc = cdp_packet_set_address_copy(packet, 1, (struct sockaddr *)&ipv4_address);
	ASSERT_GE(rc, 0);

	// Verify the addresses are either null or not null appropriately
	ASSERT_EQ(nullptr, packet->addresses->addresses[0]);
	ASSERT_NE(nullptr, packet->addresses->addresses[1]);
	ASSERT_EQ(nullptr, packet->addresses->addresses[2]);

	// Verify the type of the second address
	ASSERT_EQ(AF_INET, packet->addresses->addresses[1]->sa_family);

	// Verify the value of the second address
	struct sockaddr_in *test = (struct sockaddr_in *)packet->addresses->addresses[1];
	ASSERT_EQ(htonl(0x0A640101), test->sin_addr.s_addr);

	// Verify that the address value is not a pointer to the input structure
	ASSERT_NE(&ipv4_address, test);

	cdp_packet_delete(packet);
}

/// Test that an CDP frame can be serialized
TEST(CdpPacket, SerializePacket) {
	// Create a software version string
	static char *cdp_software_version_string = NULL;
	int rc = generate_cdp_software_version_string(&cdp_software_version_string);
	EXPECT_EQ(0, rc);
	ASSERT_NE(nullptr, cdp_software_version_string);
	
	// Create the packet object
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);
	ASSERT_NE(nullptr, packet);

	// Set the device ID
	rc = cdp_packet_set_device_id(packet, "MyDogIsBetterThanYourDog");
	ASSERT_GE(rc, 0);

	// Set the software version string
	rc = cdp_packet_set_software_version(packet, cdp_software_version_string);
	ASSERT_GE(rc, 0);

	// Set the platform string
	rc = cdp_packet_set_platform(packet, cdp_platform_string);
	ASSERT_GE(rc, 0);

	// Set the capabilities
	rc = cdp_packet_set_capabilities(packet, CdpCapabilityHost | CdpCapabilityIGMP);
	ASSERT_GE(rc, 0);

	// Set the port ID
	rc = cdp_packet_set_port_id(packet, "eth0");
	ASSERT_GE(rc, 0);

	// Set the port duplex
	rc = cdp_packet_set_duplex(packet, DuplexFull);
	ASSERT_GE(rc, 0);

	// Set the port addresses
	rc = cdp_packet_provision_address_array(packet, 3);
	ASSERT_GE(rc, 0);
	ASSERT_NE(nullptr, packet->addresses);
	ASSERT_NE(nullptr, packet->addresses->addresses);
	ASSERT_EQ(3, packet->addresses->count);

	struct sockaddr_in ipv4_address;
	memset(&ipv4_address, 0, sizeof(struct sockaddr_in));
	ipv4_address.sin_family = AF_INET;

	// Set the first address
	ipv4_address.sin_addr.s_addr = htonl(0x0A640101);
	rc = cdp_packet_set_address_copy(packet, 0, (struct sockaddr *)&ipv4_address);
	ASSERT_GE(rc, 0);
	
	// Set the second address
	ipv4_address.sin_addr.s_addr = htonl(0xC0A80101);
	rc = cdp_packet_set_address_copy(packet, 1, (struct sockaddr *)&ipv4_address);
	ASSERT_GE(rc, 0);

	// Set the third address
	const uint8_t test_address[16] = { 0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16 };
	rc = ip_address_array_set_into_ipv6_raw(packet->addresses, 2, test_address);
	ASSERT_GE(rc, 0);
	
	// Verify the first address
	struct sockaddr_in *test = (struct sockaddr_in *)packet->addresses->addresses[0];
	ASSERT_EQ(htonl(0x0A640101), test->sin_addr.s_addr);
	
	// Verify the second address
	test = (struct sockaddr_in *)packet->addresses->addresses[1];
	ASSERT_EQ(htonl(0xC0A80101), test->sin_addr.s_addr);

	// Verify the third address
	struct sockaddr_in6 *test6 = (struct sockaddr_in6 *)packet->addresses->addresses[2];
	ASSERT_EQ(0, memcmp(IPv6Octets(test6), test_address, 16));

	// Serialize the packet into a buffer
	uint8_t frame_buffer[1500];
	ssize_t frame_buffer_length = cdp_packet_serialize(packet, frame_buffer, sizeof(frame_buffer));
	//printf("Frame buffer length %zd\n", frame_buffer_length);
	ASSERT_GE(frame_buffer_length, 0);
	ASSERT_EQ(frame_buffer_length, 215);

	// Delete the packet
	cdp_packet_delete(packet);

	// Free the version string
	FREE_ARRAY(cdp_software_version_string);
}

/// Test that a CDP frame can be serialized and parsed.
TEST(CdpPacket, SerializeAndDeserializePacket) {
	// Create a software version string
	static char *cdp_software_version_string = NULL;
	int rc = generate_cdp_software_version_string(&cdp_software_version_string);
	EXPECT_EQ(0, rc);
	ASSERT_NE(nullptr, cdp_software_version_string);

	// Create the packet object
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);
	ASSERT_NE(nullptr, packet);

	// Set the device ID
	rc = cdp_packet_set_device_id(packet, "MyDogIsBetterThanYourDog");
	ASSERT_GE(rc, 0);

	// Set the software version string
	rc = cdp_packet_set_software_version(packet, cdp_software_version_string);
	ASSERT_GE(rc, 0);

	// Set the platform string
	rc = cdp_packet_set_platform(packet, cdp_platform_string);
	ASSERT_GE(rc, 0);

	// Set the capabilities
	rc = cdp_packet_set_capabilities(packet, CdpCapabilityHost | CdpCapabilityIGMP);
	ASSERT_GE(rc, 0);

	// Set the port ID
	rc = cdp_packet_set_port_id(packet, "eth0");
	ASSERT_GE(rc, 0);

	// Set the port duplex
	rc = cdp_packet_set_duplex(packet, DuplexFull);
	ASSERT_GE(rc, 0);

	// Set the port addresses
	rc = cdp_packet_provision_address_array(packet, 3);
	ASSERT_GE(rc, 0);
	ASSERT_NE(nullptr, packet->addresses);
	ASSERT_NE(nullptr, packet->addresses->addresses);
	ASSERT_EQ(3, packet->addresses->count);

	struct sockaddr_in ipv4_address;
	memset(&ipv4_address, 0, sizeof(struct sockaddr_in));
	ipv4_address.sin_family = AF_INET;

	// Set the first address
	ipv4_address.sin_addr.s_addr = htonl(0x0A640101);
	rc = cdp_packet_set_address_copy(packet, 0, (struct sockaddr *)&ipv4_address);
	ASSERT_GE(rc, 0);

	// Set the second address
	ipv4_address.sin_addr.s_addr = htonl(0xC0A80101);
	rc = cdp_packet_set_address_copy(packet, 1, (struct sockaddr *)&ipv4_address);
	ASSERT_GE(rc, 0);

	// Set the third address
	const uint8_t test_address[16] = { 0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16 };
	rc = ip_address_array_set_into_ipv6_raw(packet->addresses, 2, test_address);
	ASSERT_GE(rc, 0);

	// Serialize the packet into a buffer
	uint8_t frame_buffer[1500];
	ssize_t frame_buffer_length = cdp_packet_serialize(packet, frame_buffer, sizeof(frame_buffer));
	//printf("Frame buffer length %zd\n", frame_buffer_length);
	ASSERT_GE(frame_buffer_length, 0);
	ASSERT_EQ(frame_buffer_length, 215);

	// Delete the packet
	cdp_packet_delete(packet);

	// Create a stream reader object
	struct stream_reader *reader = stream_reader_new(frame_buffer, (size_t)frame_buffer_length);
	ASSERT_NE(nullptr, reader);

	// Validate the packet checksum
	bool ok = stream_reader_validate_checksum(reader);
	ASSERT_EQ(true, ok);

	// Parse the packet
	struct cdp_packet *parsed;
	rc = cdp_parse_packet(reader, &parsed);
	ASSERT_GE(rc, 0);	

	// Delete the stream reader
	stream_reader_delete(reader);

	// Verify the parser returned an object
	ASSERT_NE(nullptr, parsed);

	// Delete the parsed packet
	cdp_packet_delete(parsed);

	// Free the version string
	FREE_ARRAY(cdp_software_version_string);
}

/// Parses a CDP frame from a CSR1000V and validates its contents
TEST(CdpPacket, ValidateParserCsr1000v) {
	// Create a stream reader object
	struct stream_reader *reader = stream_reader_new(cdp_sample_data_csr1000v, sizeof(cdp_sample_data_csr1000v));
	ASSERT_NE(nullptr, reader);

	// Validate the packet checksum
	bool ok = stream_reader_validate_checksum(reader);
	ASSERT_EQ(true, ok);

	// Parse the packet
	struct cdp_packet *parsed;
	int rc = cdp_parse_packet(reader, &parsed);
	ASSERT_GE(rc, 0);

	// Delete the stream reader
	stream_reader_delete(reader);

	// Verify the parser returned an object
	ASSERT_NE(nullptr, parsed);

	// Verify the device id
	ASSERT_STREQ(parsed->device_id, cdp_sample_data_csr1000v_device_id);

	// Verify the software version string
	ASSERT_STREQ(parsed->software_version, cdp_sample_data_csr1000v_software_version);

	// Verify the platform ID
	ASSERT_STREQ(parsed->platform, cdp_sample_data_csr1000v_platform);

	// Verify addresses were parsed
	ASSERT_NE(nullptr, parsed->addresses);

	// Verify the number of addresses
	ASSERT_EQ(parsed->addresses->count, cdp_sample_data_csr1000v_address_count);

	// Verify the addresses have been allocated
	ASSERT_NE(nullptr, parsed->addresses->addresses);
	ASSERT_NE(nullptr, parsed->addresses->addresses[0]);
	ASSERT_NE(nullptr, parsed->addresses->addresses[1]);
	ASSERT_NE(nullptr, parsed->addresses->addresses[2]);

	// Verify the content of the address (0)
	ASSERT_EQ(parsed->addresses->addresses[0]->sa_family, cdp_sample_data_csr1000v_address0_type);
	ASSERT_EQ(((struct sockaddr_in *)parsed->addresses->addresses[0])->sin_addr.s_addr, htonl(cdp_sample_data_csr1000v_address0));

	// Verify the content of the address (1)
	ASSERT_EQ(parsed->addresses->addresses[1]->sa_family, cdp_sample_data_csr1000v_address1_type);
	ASSERT_EQ(0, memcmp(IPv6Octets((struct sockaddr_in6 *)parsed->addresses->addresses[1]), cdp_sample_data_csr1000v_address1, 16));

	// Verify the content of the address (2)
	ASSERT_EQ(parsed->addresses->addresses[2]->sa_family, cdp_sample_data_csr1000v_address2_type);
	ASSERT_EQ(0, memcmp(IPv6Octets((struct sockaddr_in6 *)parsed->addresses->addresses[2]), cdp_sample_data_csr1000v_address2, 16));

	// Verify the port ID
	ASSERT_STREQ(parsed->port_id, cdp_sample_data_csr1000v_port_id);

	// Verify the capabilities
	ASSERT_NE(nullptr, parsed->capabilities);
	ASSERT_EQ(*(parsed->capabilities), cdp_sample_data_csr1000v_capabilities);

	// Verify management addresses were parsed
	ASSERT_NE(nullptr, parsed->management_addresses);

	// Verify the number of management addresses
	ASSERT_EQ(parsed->management_addresses->count, cdp_sample_data_csr1000v_management_address_count);

	// Verify the management addresses have been allocated
	ASSERT_NE(nullptr, parsed->management_addresses->addresses);
	ASSERT_NE(nullptr, parsed->management_addresses->addresses[0]);

	// Verify the content of the management address (0)
	ASSERT_EQ(parsed->management_addresses->addresses[0]->sa_family, cdp_sample_data_csr1000v_management_address0_type);
	ASSERT_EQ(((struct sockaddr_in *)parsed->management_addresses->addresses[0])->sin_addr.s_addr, htonl(cdp_sample_data_csr1000v_management_address0));

	// Delete the parsed packet
	cdp_packet_delete(parsed);
}
