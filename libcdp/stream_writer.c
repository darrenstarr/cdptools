#include "stream_writer.h"
#include "platform/platform.h"
#include "platform/checksum.h"
#include "platform/string.h"

struct stream_writer *stream_writer_new(uint8_t *buffer, size_t size)
{
	struct stream_writer *result;

	if (buffer == NULL)
	{
		LOG_CRITICAL("stream_writer_new: buffer is NULL\n");
		return NULL;
	}

	if (size == 0)
	{
		LOG_CRITICAL("stream_writer_new: length is 0\n");
		return NULL;
	}

	result = ALLOC_NEW(struct stream_writer);
	if (result == NULL)
	{
		LOG_CRITICAL("stream_writer_new: failed to allocate memory for new stream writer\n");
		return NULL;
	}

	result->buffer = buffer;
	result->size = size;
	result->position = buffer;

	return result;
}

int stream_writer_delete(struct stream_writer *writer)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_delete: Failed to delete stream writer\n");
		return -1;
	}

	FREE(writer);

	return 0;
}

ssize_t stream_writer_length(const struct stream_writer *writer)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_length: writer is NULL\n");
		return -1;
	}

	return writer->position - writer->buffer;
}

ssize_t stream_writer_remaining(const struct stream_writer *writer)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_remaining: writer is NULL\n");
		return -1;
	}

	return (ssize_t)((ssize_t)writer->size - stream_writer_length(writer));
}

bool stream_writer_need(const struct stream_writer *writer, size_t needed)
{
	ssize_t remaining;

	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_need: writer is NULL\n");
		return -1;
	}

	remaining = stream_writer_remaining(writer);

	if (remaining < 0)
	{
		LOG_CRITICAL("stream_writer_need: position past end\n");
		return -1;
	}

	return (needed <= (size_t)remaining) ? true : false;
}

int stream_writer_put8(struct stream_writer *writer, uint8_t value)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_put8: writer is NULL\n");
		return -1;
	}

	if (!stream_writer_need(writer, 1))
	{
		LOG_CRITICAL("stream_writer_put8: Output past end\n");
		return -1;
	}

	*(writer->position)++ = value;

	return 0;
}

int stream_writer_put16(struct stream_writer *writer, uint16_t value)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_put16: writer is NULL\n");
		return -1;
	}

	if (!stream_writer_need(writer, 2))
	{
		LOG_CRITICAL("stream_writer_put16: Output past end\n");
		return -1;
	}

	*(writer->position)++ = (uint8_t)((value >> 8) & 0xFF);
	*(writer->position)++ = (uint8_t)(value & 0xFF);

	return 0;
}

int stream_writer_put24(struct stream_writer *writer, uint32_t value)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_put24: writer is NULL\n");
		return -1;
	}

	if (!stream_writer_need(writer, 3))
	{
		LOG_CRITICAL("stream_writer_put24: Output past end\n");
		return -1;
	}

	*(writer->position)++ = (uint8_t)((value >> 16) & 0xFF);
	*(writer->position)++ = (uint8_t)((value >> 8) & 0xFF);
	*(writer->position)++ = (uint8_t)(value & 0xFF);

	return 0;
}

int stream_writer_put32(struct stream_writer *writer, uint32_t value)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_put32: writer is NULL\n");
		return -1;
	}

	if (!stream_writer_need(writer, 4))
	{
		LOG_CRITICAL("stream_writer_put32: Output past end\n");
		return -1;
	}

	*(writer->position)++ = (uint8_t)((value >> 24) & 0xFF);
	*(writer->position)++ = (uint8_t)((value >> 16) & 0xFF);
	*(writer->position)++ = (uint8_t)((value >> 8) & 0xFF);
	*(writer->position)++ = (uint8_t)(value & 0xFF);

	return 0;
}

int stream_writer_put_buffer(struct stream_writer *writer, const uint8_t *value, size_t length)
{
	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_put_buffer: writer is NULL\n");
		return -1;
	}

	if (value == NULL)
	{
		LOG_CRITICAL("stream_writer_put_buffer: value is NULL\n");
		return -1;
	}

	if (length == 0)
	{
		LOG_DEBUG("stream_writer_put_buffer: length is 0\n");
		return 0;
	}

	if (!stream_writer_need(writer, length))
	{
		LOG_CRITICAL("stream_writer_put_buffer: Output past end\n");
		return -1;
	}

	memcpy(writer->position, value, length);

	writer->position += length;

	return 0;
}

int stream_writer_put_string(struct stream_writer *writer, const char *value)
{
	size_t length;

	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_put_string: writer is NULL\n");
		return -1;
	}

	if (value == NULL)
	{
		LOG_CRITICAL("stream_writer_put_string: value is NULL\n");
		return -1;
	}

	length = strlen(value);

	if (!stream_writer_need(writer, length))
	{
		LOG_CRITICAL("stream_writer_put_string: Output past end\n");
		return -1;
	}

	memcpy(writer->position, value, length);

	writer->position += length;

	return 0;
}

int stream_writer_inject_checksum(struct stream_writer *writer, off_t position)
{
	ssize_t length;
	uint16_t checksum;

	if (writer == NULL)
	{
		LOG_CRITICAL("stream_writer_inject_checksum: writer is NULL\n");
		return -1;
	}

	length = stream_writer_length(writer);
	if(length < 0)
	{
		LOG_CRITICAL("stream_writer_inject_checksum: failed to get the length of the buffer\n");
		return -1;
	}

	if (length < (position + 2))
	{
		LOG_CRITICAL("stream_writer_inject_checksum: buffer overflow error\n");
		return -1;
	}

	checksum = ip_compute_csum(writer->buffer, (size_t)length);

	writer->buffer[position+1] = (uint8_t)(checksum >> 8);
	writer->buffer[position] = (uint8_t)(checksum & 0xFF);

	return 0;
}
