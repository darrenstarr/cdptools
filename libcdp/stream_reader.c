#include "buffer_stream.h"
#include "platform.h"
#include "platform_socket.h"
#include "stream_reader.h"

struct stream_reader *stream_reader_new(const uint8_t *data, size_t length)
{
	struct stream_reader *result;
	struct s_buffer_stream *stream;

	LOG_DEBUG("Allocating new stream reader\n");

	stream = s_buffer_stream_new(data, length);
	if (stream == NULL)
	{
		LOG_ERROR("stream_reader_new: failed to allocate new buffer stream\n");
		return NULL;
	}

	result = ALLOC_NEW(struct stream_reader);
	if (result == NULL)
	{
		LOG_ERROR("stream_reader_new: failed to allocate new stream reader. Failed to allocate memory\n");
		s_buffer_stream_delete(stream);

		return NULL;
	}

	result->stream = stream;
	result->position = 0;

	return result;
}

void stream_reader_delete(struct stream_reader *reader)
{
	LOG_DEBUG("stream_reader_delete: deleting reader\n");

	if (reader == NULL)
	{
		LOG_ERROR("stream_reader_delete: reader is NULL\n");
		return;
	}

	if(reader->stream != NULL)
		s_buffer_stream_delete(reader->stream);

	FREE(reader);
}

size_t stream_reader_remaining(const struct stream_reader *reader)
{
	if (reader == NULL)
	{
		LOG_CRITICAL("stream_reader_remaining: reader is NULL\n");
		return false;
	}

	if (reader->stream == NULL)
	{
		LOG_CRITICAL("stream_reader_remaining: stream is NULL\n");
		return false;
	}

	/* Cast off_t to size_t here since this class is supposed to ensure that the buffer is never past end. */
	return (s_buffer_stream_length(reader->stream) - (size_t)reader->position);
}

bool stream_reader_at_end(const struct stream_reader *reader)
{
	return stream_reader_remaining(reader) == 0;
}

bool stream_reader_need(const struct stream_reader *reader, size_t bytesNeeded)
{
	if (reader == NULL)
	{
		LOG_CRITICAL("stream_reader_need: reader is NULL\n");
		return false;
	}

	if (reader->stream == NULL)
	{
		LOG_CRITICAL("stream_reader_need: stream is NULL\n");
		return false;
	}
	
	return (stream_reader_remaining(reader) >= bytesNeeded) ? true : false;
}

off_t stream_reader_get_position(const struct stream_reader *reader)
{
	if (reader == NULL)
	{
		LOG_CRITICAL("stream_reader_set_position: reader is NULL\n");
		return 0;
	}

	return reader->position;
}

int stream_reader_set_position(struct stream_reader *reader, off_t newPosition)
{
	if (reader == NULL)
	{
		LOG_ERROR("stream_reader_set_position: reader is NULL\n");
		return -1;
	}

	if (reader->stream == NULL)
	{
		LOG_ERROR("stream_reader_set_position: stream is NULL\n");
		return -1;
	}

	if (newPosition > reader->stream->length)
	{
		LOG_ERROR("stream_reader_set_position: newPosition is past the end of the stream.\n");
		return -1;
	}

	reader->position = newPosition;

	return 0;
}

int stream_reader_skip(struct stream_reader *reader, off_t toSkip)
{
	if (reader == NULL)
	{
		LOG_ERROR("stream_reader_skip: reader is NULL\n");
		return -1;
	}

	return stream_reader_set_position(reader, stream_reader_get_position(reader) + toSkip);
}

int stream_reader_get8(struct stream_reader * reader, uint8_t *result)
{
	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get8: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, 1))
	{
		LOG_ERROR("stream_reader_get8: Input past end of buffer\n");
		return -1;
	}

	*result =
		(((uint8_t)reader->stream->data[reader->position]) & 0xFF);

	reader->position++;

	return 0;
}

int stream_reader_get16(struct stream_reader * reader, uint16_t *result)
{
	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get16: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, 2))
	{
		LOG_ERROR("stream_reader_get16: Input past end of buffer\n");
		return -1;
	}

	*result =
		(uint16_t)(
			((((uint16_t)reader->stream->data[reader->position]) & 0xFF) << 8) |
			(((uint16_t)reader->stream->data[reader->position + 1]) & 0xFF)
		);

	reader->position += 2;

	return 0;
}

int stream_reader_get24(struct stream_reader * reader, uint32_t *result)
{
	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get24: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, 3))
	{
		LOG_ERROR("stream_reader_get24: Input past end of buffer\n");
		return -1;
	}

	*result =
		(uint32_t)(
			((((uint32_t)reader->stream->data[reader->position]) & 0xFF) << 16) |
			((((uint32_t)reader->stream->data[reader->position + 1]) & 0xFF) << 8) |
			(((uint32_t)reader->stream->data[reader->position + 2]) & 0xFF)
		);

	reader->position += 3;

	return 0;
}

int stream_reader_get32(struct stream_reader * reader, uint32_t *result)
{
	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get32: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, 4))
	{
		LOG_ERROR("stream_reader_get32: Input past end of buffer\n");
		return -1;
	}

	*result =
		(uint32_t)(
			((((uint32_t)reader->stream->data[reader->position]) & 0xFF) << 24) |
			((((uint32_t)reader->stream->data[reader->position + 1]) & 0xFF) << 16) |
			((((uint32_t)reader->stream->data[reader->position + 2]) & 0xFF) << 8) |
			(((uint32_t)reader->stream->data[reader->position + 3]) & 0xFF)
		);

	reader->position += 4;

	return 0;
}

int stream_reader_get_variable_length_integer(struct stream_reader *reader, int length, uint32_t *result)
{
	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get_variable_length_integer: result is null\n");
		return -1;
	}

	if (length < 1 || length > 4)
	{
		LOG_CRITICAL("stream_reader_get_variable_length_integer: length must be between 1 and 4 bytes\n");
		return -1;
	}

	switch (length)
	{
		case 1:
			{
				uint8_t temp8;
				if (stream_reader_get8(reader, &temp8) < 0)
				{
					LOG_ERROR("stream_reader_get_variable_length_integer: could not read a byte\n");
					return -1;
				}

				*result = temp8;
			}
			return 0;

		case 2:
			{
				uint16_t temp16;
				if (stream_reader_get16(reader, &temp16) < 0)
				{
					LOG_ERROR("stream_reader_get_variable_length_integer: could not read a short\n");
					return -1;
				}

				*result = temp16;
			}
			return 0;

		case 3:
			return stream_reader_get24(reader, result);

		default:
			return stream_reader_get32(reader, result);
	}
}

int stream_reader_get_string(struct stream_reader *reader, char **result, size_t maximumLength)
{
	size_t stringLength = 0;
	
	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get_string: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, maximumLength))
	{
		LOG_ERROR("stream_reader_get_string: Input past end of buffer\n");
		return -1;
	}

	while (stringLength < maximumLength && reader->stream->data[reader->position + (off_t)stringLength] != 0)
		stringLength++;


	*result = ALLOC_NEW_ARRAY(char, stringLength + 1);
	if (*result == NULL)
	{
		LOG_ERROR("stream_reader_get_string: Failed to allocate result buffer\n");
		return -1;
	}

	COPY_MEMORY(reader->stream->data + reader->position, *result, stringLength);
	(*result)[stringLength] = '\0';

	if (stream_reader_skip(reader, (off_t)maximumLength) < 0)
	{
		LOG_ERROR("stream_reader_get_string: Failed to advance reader pointer\n");
		FREE_ARRAY(*result);
		*result = 0;

		return -1;
	}

	return 0;
}

int stream_reader_get_buffer(struct stream_reader *reader, uint8_t *result, size_t count)
{
	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get_buffer: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, count))
	{
		LOG_ERROR("stream_reader_get_buffer: Input past end of buffer\n");
		return -1;
	}

	COPY_MEMORY(reader->stream->data + reader->position, result, count);

	if (stream_reader_skip(reader, (off_t)count) < 0)
	{
		LOG_ERROR("stream_reader_get_buffer: Failed to advance reader pointer\n");
		return -1;
	}

	return 0;
}

int stream_reader_get_protocol_from_snap(struct stream_reader *reader, int *result)
{
	uint8_t ssap;
	uint8_t dsap;
	uint8_t control;
	uint32_t oui;
	uint16_t pid;

	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get_protocol_from_snap: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, 5))
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Input past end of buffer\n");
		return -1;
	}

	if (stream_reader_get8(reader, &dsap) < 0)
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Failed to read DSAP\n");
		return -1;
	}

	if (dsap != 0xAA)
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Unknown DSAP type (%02X)\n", dsap);
		return -1;
	}

	if (stream_reader_get8(reader, &ssap) < 0)
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Failed to read SSAP\n");
		return -1;
	}

	if (ssap != 0xAA)
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Unknown SSAP type (%02X)\n", ssap);
		return -1;
	}

	if (stream_reader_get8(reader, &control) < 0)
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Failed to read SSAP\n");
		return -1;
	}

	/* TODO: Consider validating control, it should be 0x3, but I don't know what 0x3 means. */

	if (stream_reader_get24(reader, &oui) < 0)
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Failed to read protocol vendor OUI from SNAP\n");
		return -1;
	}

	if (oui != 0x000000)
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Vendor OUI is not generic (00:00:00) as expected\n");
		return -1;
	}

	if (stream_reader_get16(reader, &pid) < 0)
	{
		LOG_ERROR("stream_reader_get_protocol_from_snap: Failed to read PID\n");
		return -1;
	}

	switch (pid)
	{
		case 0x0800:
			*result = AF_INET;
			return 0;

		case 0x86DD:
			*result = AF_INET6;
			return 0;
	}

	LOG_ERROR("stream_reader_get_protocol_from_snap: Unrecognized PID (0x%4X)\n", pid);
	return -1;
}

int stream_reader_get_protocol_type(struct stream_reader *reader, int *result)
{
	uint8_t protocolType;
	uint8_t protocolLength;

	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get_protocol_type: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, 2))
	{
		LOG_ERROR("stream_reader_get_protocol_type: Input past end of buffer\n");
		return -1;
	}

	if (stream_reader_get8(reader, &protocolType) < 0)
	{
		LOG_ERROR("stream_reader_get_protocol_type: failed to read the protocol type of the address\n");
		return -1;
	}

	if (stream_reader_get8(reader, &protocolLength) < 0)
	{
		LOG_ERROR("stream_reader_get_protocol_type: failed to read the protocol length of the address\n");
		return -1;
	}

	switch (protocolType)
	{
		case 0x01:  // NLPID
			{
				uint32_t protocolId;

				if (stream_reader_get_variable_length_integer(reader, protocolLength, &protocolId) < 0)
				{
					LOG_ERROR("stream_reader_get_protocol: failed to read the protocol ID of an address\n");
					return -1;
				}

				switch (protocolId)
				{
					case 0xCC:  // IPv4 (RFC6328)
						*result = AF_INET;
						return 0;
				}
			}
			break;

		case 0x02:  // 802.2 SNAP
			return stream_reader_get_protocol_from_snap(reader, result);
	}

	LOG_ERROR("stream_reader_get_protocol: cannot identify the protocol type to read\n");
	return -1;
}

int stream_reader_get_inet_address(struct stream_reader *reader, struct sockaddr **result)
{
	uint32_t address;

	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get_inet_address: result is null\n");
		return -1;
	}

	if (stream_reader_get32(reader, &address))
	{
		LOG_ERROR("stream_reader_get_inet_address: failed to read address\n");
		return -1;
	}

	/* If there is no address already allocated, allocate one. */
	if (*result == NULL)
	{
		*result = ALLOC_NEW(struct sockaddr_in);
		if (*result == NULL)
		{
			LOG_ERROR("stream_reader_get_inet_address: failed to allocate enough memory for the address\n");
			return -1;
		}
	}

	ZERO_BUFFER(*result, struct sockaddr_in);

	(*result)->sa_family = AF_INET;
	((struct sockaddr_in *)(*result))->sin_addr.s_addr = ntohl(address);

	return 0;
}

int stream_reader_get_inet6_address(struct stream_reader *reader, struct sockaddr **result)
{
	int i;
	uint8_t *addressBuffer;

	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get_inet6_address: result is null\n");
		return -1;
	}

	if (!stream_reader_need(reader, 16))
	{
		LOG_ERROR("stream_reader_get_inet6_address: input past end\n");
		return -1;
	}

	if (*result == NULL)
	{
		*result = ALLOC_NEW(struct sockaddr_in6);
		if (*result == NULL)
		{
			LOG_ERROR("stream_reader_get_inet6_address: failed to allocate enough memory for the address\n");
			return -1;
		}
	}

	ZERO_BUFFER(*result, struct sockaddr_in6);

	(*result)->sa_family = AF_INET6;
	
	/* TODO: Consider making a buffer copy function instead */
	addressBuffer = IPv6Octets((struct sockaddr_in6 *)(*result));
	for (i = 0; i < 16; i++)
	{
		if (stream_reader_get8(reader, addressBuffer + i) < 0)
		{
			LOG_ERROR("stream_reader_get_int6_address: failed to read address\n");
			return -1;
		}
	}

	return 0;
}

int stream_reader_get_address(struct stream_reader *reader, struct sockaddr **result)
{
	int addressFamily;
	uint16_t addressLength;

	if (result == NULL)
	{
		LOG_CRITICAL("stream_reader_get_address: result is null\n");
		return -1;
	}

	if (stream_reader_get_protocol_type(reader, &addressFamily) < 0)
	{
		LOG_ERROR("stream_reader_get_address: unknown or unhandled address family\n");
		return -1;
	}

	if (stream_reader_get16(reader, &addressLength) < 0)
	{
		LOG_ERROR("stream_reader_get_address: failed to read address length\n");
		return -1;
	}

	if (!stream_reader_need(reader, addressLength))
	{
		LOG_ERROR("stream_reader_get_address: input is past end\n");
		return -1;
	}

	switch (addressFamily)
	{
		case AF_INET:
			if (stream_reader_get_inet_address(reader, result) < 0)
			{
				LOG_ERROR("stream_reader_get_address: failed to read an IPv4 address\n");
				return -1;
			}
			return 0;

		case AF_INET6:
			if (stream_reader_get_inet6_address(reader, result) < 0)
			{
				LOG_ERROR("stream_reader_get_address: failed to read an IPv6 address\n");
				return -1;
			}
			return 0;
	}

	LOG_ERROR("stream_reader_get_address: unknown address type\n");
	return -1;
}

/* TODO: Move this to the file. It doesn't feel right here. */
int stream_reader_get_cisco_cluster_management_protocol(struct stream_reader *reader, struct cisco_cluster_management_protocol **result)
{
	struct cisco_cluster_management_protocol *clusterProtocol;

	clusterProtocol = cisco_cluster_management_protocol_new();
	if (clusterProtocol == NULL)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: Failed to allocate a buffer for the cisco cluster management protocol\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_get24(reader, &clusterProtocol->oui) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: Failed to read the OUI for the Cisco cluster management protocol\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (clusterProtocol->oui != 0x00000C)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: The OUI specified for the cluster management protocol is not 00:00:0C\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_get16(reader, &clusterProtocol->protocol_id) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: Failed to read cluster management protocol ID\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	/*
	if (clusterProtocol->protocol_id != 0x0112)
	{
	}
	*/

	if (stream_reader_get_inet_address(reader, (struct sockaddr **)&(clusterProtocol->cluster_master_ip)) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to read cluster master IP\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_get_inet_address(reader, (struct sockaddr **)&(clusterProtocol->netmask)) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to read cluster netmask\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_get16(reader, &clusterProtocol->version) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to read cluster management protocol version\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	/*
	if (clusterProtocol->version != 0x0102)
	{
	}
	*/

	if (stream_reader_get8(reader, &clusterProtocol->status) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to read cluster management protocol status\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_skip(reader, 1) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to advance position\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_get_buffer(reader, clusterProtocol->cluster_commander_mac, 6) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to read the cluster commander MAC\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_get_buffer(reader, clusterProtocol->local_mac, 6) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to read the local switch MAC\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}	

	if (stream_reader_skip(reader, 1) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to advance position\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_skip(reader, 1) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to advance position\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	if (stream_reader_get16(reader, &clusterProtocol->management_vlan) < 0)
	{
		LOG_ERROR("stream_reader_get_cisco_cluster_management_protocol: failed to read cluster management vlan\n");
		cisco_cluster_management_protocol_delete(clusterProtocol);
		return -1;
	}

	*result = clusterProtocol;

	return 0;
}
