#ifndef STREAM_WRITER_H
#define STREAM_WRITER_C

#include "platform/types.h"

/** A memory buffer stream writer for serialization */
struct stream_writer
{
	/** The buffer to write to */
	uint8_t *buffer;

	/** The size of the buffer in bytes */
	size_t size;

	/** The current index within the buffer */
	uint8_t *position;
};

/** Constructor
  *  @param buffer The buffer to create a stream writer against.
  *  @param size The size of the buffer available to the writer in bytes.
  *  @return Either the new stream writer or NULL on error.
  */
struct stream_writer *stream_writer_new(uint8_t *buffer, size_t size);

/** Destructor
  *  @param writer The writer object to delete.
  *  @return 0 on success, -1 on failure.
  */
int stream_writer_delete(struct stream_writer *writer);

/** Returns the consumed length of the writer buffer
  *  @param writer The writer object
  *  @return the length of the data written to the buffer or a negative value on error.
  */
ssize_t stream_writer_length(const struct stream_writer *writer);

/** Returns the amount of buffer space remaining.
  *  @param writer The writer object.
  *  @return Either the number of bytes remaining in the buffer or a negative value on error.
  */
ssize_t stream_writer_remaining(const struct stream_writer *writer);

/** Returns whether the given number of bytes are available to be written into the buffer.
  *  @param writer The writer object.
  *  @param needed The number of bytes to check for availability.
  *  @return true if available or false if not or on error.
  */
bool stream_writer_need(const struct stream_writer *writer, size_t needed);

/** Writes a single byte to the end of the buffer.
  *  @param writer The writer object.
  *  @param value The value to write.
  *  @return 0 on success or a negative value on error.
  */
int stream_writer_put8(struct stream_writer *writer, uint8_t value);

/** Writes a two byte unsigned integer to the end of the buffer as big-endian.
  *  @param writer The writer object.
  *  @param value The value to write.
  *  @return 0 on success or a negative value on error.
  */
int stream_writer_put16(struct stream_writer *writer, uint16_t value);

/** Writes a three byte unsigned integer to the end of the buffer as big-endian.
  *  @param writer The writer object.
  *  @param value The value to write.
  *  @return 0 on success or a negative value on error.
  */
int stream_writer_put24(struct stream_writer *writer, uint32_t value);

/** Writes a four byte unsigned integer to the end of the buffer as big-endian.
  *  @param writer The writer object.
  *  @param value The value to write.
  *  @return 0 on success or a negative value on error.
  */
int stream_writer_put32(struct stream_writer *writer, uint32_t value);

/** Appends the contents of a given buffer to the end of the stream.
  *  @param writer The writer object.
  *  @param value The value to write.
  *  @param length The number of bytes to write.
  *  @return 0 on success or a negative value on error.
  */
int stream_writer_put_buffer(struct stream_writer *writer, const uint8_t *value, size_t length);

/** Appends the contents of null terminated string to the end of the stream.
  *  @param writer The writer object.
  *  @param value The value to write.
  *  @param length The number of bytes to write.
  *  @return 0 on success or a negative value on error.
  */
int stream_writer_put_string(struct stream_writer *writer, const char *value);

/** Calculate the checksum for the buffer represented by the writer and inject it at the given position.
  *  @param writer The writer object.
  *  @param position The position to inject the checksum.
  *  @return 0 on success or a negative value on error.
  */
int stream_writer_inject_checksum(struct stream_writer *writer, off_t position);

#endif
