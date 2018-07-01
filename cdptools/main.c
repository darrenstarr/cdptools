#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cdp_packet_parser.h"
#include "platform.h"
#include "sample.h"
#include "stream_reader.h"


void Test(const uint8_t *data, size_t length)
{
	struct s_cdp_neighbor *neighbor;
	struct stream_reader *reader;

	int result;

	reader = stream_reader_new(data, length);
	if (reader == NULL)
	{
		LOG_ERROR("Failed to allocate reader object for packet. Exiting\n");
		exit(-1);
	}

	result = cdp_parse_packet(reader, &neighbor);
	if (result < 0)
	{
		printf("Failed to parse packet, error code : %d\n", result);
		exit(-1);
	}

	cdp_neighbor_dump(neighbor);

	stream_reader_delete(reader);
	reader = NULL;

	cdp_neighbor_delete(neighbor);
	neighbor = NULL;

	printf("Packet parsed");
}

int main()
{
	const uint8_t *bufferStart = packet_bytes + 0x16;
	const uint8_t *bufferEnd = packet_bytes + sizeof(packet_bytes);
	//Test(bufferStart, (size_t)(bufferEnd - bufferStart));

	Test(WiresharkCSR1000VIPv4, sizeof(WiresharkCSR1000VIPv4));
	Test(WiresharkCSR1000VIPv4, sizeof(WiresharkCSR1000VIPv4));

	return 0;
}
