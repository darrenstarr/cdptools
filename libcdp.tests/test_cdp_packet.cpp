#include "pch.h"

extern "C" {
#include "../libcdp/cdp_packet.h"
#include "../libcdp/cdp_packet_parser.h"
#include "../libcdp/ecdptlv.h"
#include "../libcdp/cdp_software_version_string.h"
#include "../libcdp/platform/platform.h"
}

TEST(CdpPacket, CdpPacketNew) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	cdp_packet_delete(packet);
}

TEST(CdpPacket, SetDeviceId) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_device_id(packet, "MyDogIsBetterThanYourDog");

	ASSERT_GE(rc, 0);

	ASSERT_STREQ(packet->device_id, "MyDogIsBetterThanYourDog");

	cdp_packet_delete(packet);
}

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

TEST(CdpPacket, SetPlatformString) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_platform(packet, cdp_platform_string);

	ASSERT_GE(rc, 0);

	ASSERT_STREQ(packet->platform, cdp_platform_string);

	cdp_packet_delete(packet);
}

TEST(CdpPacket, SetCapabilities) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_capabilities(packet, CdpCapabilityHost | CdpCapabilityIGMP);

	ASSERT_GE(rc, 0);

	ASSERT_NE(nullptr, packet->capabilities);
	ASSERT_EQ(*(packet->capabilities), CdpCapabilityHost | CdpCapabilityIGMP);

	cdp_packet_delete(packet);
}

TEST(CdpPacket, SetPortId) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_port_id(packet, "eth0");

	ASSERT_GE(rc, 0);

	ASSERT_STREQ(packet->port_id, "eth0");

	cdp_packet_delete(packet);
}

TEST(CdpPacket, SetDuplex) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_set_duplex(packet, DuplexFull);

	ASSERT_GE(rc, 0);

	ASSERT_EQ(packet->duplex, DuplexFull);

	cdp_packet_delete(packet);
}

TEST(CdpPacket, ProvisionAddressAddress) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_provision_address_array(packet, 3);
	ASSERT_GE(rc, 0);

	ASSERT_NE(nullptr, packet->addresses);
	ASSERT_NE(nullptr, packet->addresses->addresses);
	ASSERT_EQ(3, packet->addresses->count);

	for (int i = 0; i < packet->addresses->count; i++)
		ASSERT_EQ(nullptr, packet->addresses->addresses[i]);

	cdp_packet_delete(packet);
}

TEST(CdpPacket, SetIPv4Addresses) {
	struct cdp_packet *packet = cdp_packet_new(2, 180, 0);

	ASSERT_NE(nullptr, packet);

	int rc = cdp_packet_provision_address_array(packet, 3);
	ASSERT_GE(rc, 0);

	ASSERT_NE(nullptr, packet->addresses);
	ASSERT_NE(nullptr, packet->addresses->addresses);

	struct sockaddr_in ipv4_address;
	memset(&ipv4_address, 0, sizeof(struct sockaddr_in));
	ipv4_address.sin_family = AF_INET;
	ipv4_address.sin_addr.s_addr = htonl(0x0A640101);

	rc = cdp_packet_set_address_copy(packet, 1, (struct sockaddr *)&ipv4_address);
	ASSERT_GE(rc, 0);

	ASSERT_EQ(nullptr, packet->addresses->addresses[0]);
	ASSERT_NE(nullptr, packet->addresses->addresses[1]);
	ASSERT_EQ(nullptr, packet->addresses->addresses[2]);

	ASSERT_EQ(AF_INET, packet->addresses->addresses[1]->sa_family);

	struct sockaddr_in *test = (struct sockaddr_in *)packet->addresses->addresses[1];
	ASSERT_EQ(htonl(0x0A640101), test->sin_addr.s_addr);

	ASSERT_NE(&ipv4_address, test);

	cdp_packet_delete(packet);
}

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
	rc = cdp_packet_provision_address_array(packet, 2);
	ASSERT_GE(rc, 0);
	ASSERT_NE(nullptr, packet->addresses);
	ASSERT_NE(nullptr, packet->addresses->addresses);
	ASSERT_EQ(2, packet->addresses->count);

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
	
	// Verify the first address
	struct sockaddr_in *test = (struct sockaddr_in *)packet->addresses->addresses[0];
	ASSERT_EQ(htonl(0x0A640101), test->sin_addr.s_addr);
	
	// Verify the second address
	test = (struct sockaddr_in *)packet->addresses->addresses[1];
	ASSERT_EQ(htonl(0xC0A80101), test->sin_addr.s_addr);

	// Serialize the packet into a buffer
	uint8_t frame_buffer[1500];
	ssize_t frame_buffer_length = cdp_packet_serialize(packet, frame_buffer, sizeof(frame_buffer));
	//printf("Frame buffer length %zd\n", frame_buffer_length);
	ASSERT_GE(frame_buffer_length, 0);
	ASSERT_EQ(frame_buffer_length, 188);

	// Delete the packet
	cdp_packet_delete(packet);

	// Free the version string
	FREE_ARRAY(cdp_software_version_string);
}

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
	rc = cdp_packet_provision_address_array(packet, 2);
	ASSERT_GE(rc, 0);
	ASSERT_NE(nullptr, packet->addresses);
	ASSERT_NE(nullptr, packet->addresses->addresses);
	ASSERT_EQ(2, packet->addresses->count);

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

	// Verify the first address
	struct sockaddr_in *test = (struct sockaddr_in *)packet->addresses->addresses[0];
	ASSERT_EQ(htonl(0x0A640101), test->sin_addr.s_addr);

	// Verify the second address
	test = (struct sockaddr_in *)packet->addresses->addresses[1];
	ASSERT_EQ(htonl(0xC0A80101), test->sin_addr.s_addr);

	// Serialize the packet into a buffer
	uint8_t frame_buffer[1500];
	ssize_t frame_buffer_length = cdp_packet_serialize(packet, frame_buffer, sizeof(frame_buffer));
	//printf("Frame buffer length %zd\n", frame_buffer_length);
	ASSERT_GE(frame_buffer_length, 0);
	ASSERT_EQ(frame_buffer_length, 188);

	// Delete the packet
	cdp_packet_delete(packet);

	// Create a stream reader object
	struct stream_reader *reader = stream_reader_new(frame_buffer, (size_t)frame_buffer_length);
	ASSERT_NE(nullptr, reader);

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

