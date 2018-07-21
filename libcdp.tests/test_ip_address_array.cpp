#include <gtest/gtest.h>

extern "C" {
#include "../libcdp/cdp_packet.h"
#include "../libcdp/cdp_packet_parser.h"
#include "../libcdp/ecdptlv.h"
#include "../libcdp/cdp_software_version_string.h"
#include "../libcdp/platform/platform.h"
}

#include "cdp_sample_data.h"

TEST(IpAddressArray, Constructor) {
	struct ip_address_array *array = ip_address_array_new(4);

	// Verify it was allocated
	ASSERT_NE(nullptr, array);

	// Verify the proper number of addresses was set
	ASSERT_EQ(4, array->count);

	// Verify the address array was constructed
	ASSERT_NE(nullptr, array->addresses);

	// Verify that the values in the array were initialized to null
	ASSERT_EQ(nullptr, array->addresses[0]);
	ASSERT_EQ(nullptr, array->addresses[1]);
	ASSERT_EQ(nullptr, array->addresses[2]);
	ASSERT_EQ(nullptr, array->addresses[3]);

	// Delete the array
	ip_address_array_delete(array);
}

TEST(IpAddressArray, SetIPv6Raw) {
	struct ip_address_array *array = ip_address_array_new(3);

	// Verify it was allocated
	ASSERT_NE(nullptr, array);

	// Verify the proper number of addresses was set
	ASSERT_EQ(3, array->count);

	// Verify the address array was constructed
	ASSERT_NE(nullptr, array->addresses);

	// Verify that the values in the array were initialized to null
	ASSERT_EQ(nullptr, array->addresses[0]);
	ASSERT_EQ(nullptr, array->addresses[1]);
	ASSERT_EQ(nullptr, array->addresses[2]);

	// Set a test address
	const uint8_t test_address[16] = { 0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16 };
	int rc = ip_address_array_set_into_ipv6_raw(array, 1, test_address);
	ASSERT_GE(rc, 0);

	// Verify that only one address is set now
	ASSERT_EQ(nullptr, array->addresses[0]);
	ASSERT_NE(nullptr, array->addresses[1]);
	ASSERT_EQ(nullptr, array->addresses[2]);

	// Verify the address family is IPv6
	ASSERT_EQ(array->addresses[1]->sa_family, AF_INET6);

	// Verify the address is a copy, not a reference
	ASSERT_NE(IPv6Octets((struct sockaddr_in6 *)array->addresses[1]), test_address);

	// Verify the address is set correctly
	ASSERT_EQ(0, memcmp(IPv6Octets((struct sockaddr_in6 *)array->addresses[1]), test_address, 16));

	// Delete the array
	ip_address_array_delete(array);
}
