#include "buffer_stream.h"
#include "platform/platform.h"

struct s_buffer_stream *s_buffer_stream_new(const uint8_t *data, size_t length)
{
	struct s_buffer_stream *result;

	if (data == NULL)
	{
		LOG_ERROR("s_buffer_new: data is null\n");
		return NULL;
	}

	if (length == 0)
	{
		LOG_ERROR("s_buffer_new: length is zero\n");
		return NULL;
	}

	result = ALLOC_NEW(struct s_buffer_stream);
	if (result == NULL)
	{
		LOG_ERROR("s_buffer_new: failed to allocate buffer\n");
		return NULL;
	}

	result->data = data;
	result->length = length;

	return result;
}

void s_buffer_stream_delete(struct s_buffer_stream *buffer)
{
	LOG_DEBUG("s_buffer_delete: deleting buffer\n");

	if (buffer == NULL)
	{
		LOG_ERROR("s_buffer_delete: buffer is NULL\n");
		return;
	}

	/* TODO: Is there a way to check for double deletion? */

	FREE(buffer);
}

size_t s_buffer_stream_length(const struct s_buffer_stream *buffer)
{
	if (buffer == NULL)
	{
		LOG_CRITICAL("s_buffer_stream_length: buffer is NULL\n");
		return 0;
	}

	return buffer->length;
}
