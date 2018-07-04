#ifndef BUFFER_STREAM_H
#define BUFFER_STREAM_H

#include "platform_types.h"

/**
* struct s_buffer_stream - A simple buffer class that should not be accessed except through operators
*/
struct s_buffer_stream
{
	const uint8_t *data;
	size_t length;
};

/** Constructs a new buffer stream object from a buffer.
  *  @data: A pointer to the data to pass to the buffer.
  *  @length: The length of the buffer.
  */
struct s_buffer_stream *s_buffer_stream_new(const uint8_t *data, size_t length);

/**
  * Deletes a buffer stream object.
  * @buffer: The buffer to delete.
  */
void s_buffer_stream_delete(struct s_buffer_stream *buffer);

/**
  * Returns the length of the buffer.
  * @buffer: The buffer stream.
  * @return: The length of the buffer.
  */
size_t s_buffer_stream_length(const struct s_buffer_stream *buffer);

#endif
