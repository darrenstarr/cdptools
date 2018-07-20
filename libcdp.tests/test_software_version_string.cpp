#include "pch.h"

extern "C" {
#include "../libcdp/cdp_packet.h"
#include "../libcdp/cdp_packet_parser.h"
#include "../libcdp/ecdptlv.h"
#include "../libcdp/cdp_software_version_string.h"
#include "../libcdp/platform/platform.h"
}

TEST(SoftwareVersionString, SoftwareVersionString) {
	static char *cdp_software_version_string = NULL;

	int rc = generate_cdp_software_version_string(&cdp_software_version_string);

	EXPECT_EQ(0, rc);
	ASSERT_NE(nullptr, cdp_software_version_string);

	FREE_ARRAY(cdp_software_version_string);
}

TEST(SoftwareVersionString, DeviceIdString) {
	static char *cdp_device_id_string = NULL;

	int rc = generate_cdp_device_id_string(&cdp_device_id_string);

	EXPECT_EQ(0, rc);
	ASSERT_NE(nullptr, cdp_device_id_string);

	FREE_ARRAY(cdp_device_id_string);
}
