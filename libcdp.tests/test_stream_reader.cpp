#include "pch.h"

extern "C" {
#include "../libcdp/cdp_packet.h"
#include "../libcdp/cdp_packet_parser.h"
#include "../libcdp/ecdptlv.h"
#include "../libcdp/cdp_software_version_string.h"
#include "../libcdp/platform/platform.h"
}

#include "cdp_sample_data.h"

TEST(StreamReader, ValidateChecksum) {
	// Create a stream reader object
	struct stream_reader *reader = stream_reader_new(cdp_sample_data_csr1000v, sizeof(cdp_sample_data_csr1000v));
	ASSERT_NE(nullptr, reader);

	// Validate the checksum
	bool rc = stream_reader_validate_checksum(reader);
	ASSERT_EQ(true, rc);

	// Delete the stream reader
	stream_reader_delete(reader);
}
