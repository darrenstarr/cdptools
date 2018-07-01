#ifndef STREAM_READER_H
#define STREAM_READER_H

#include <stdbool.h>
#include <sys/socket.h>
#include "buffer_stream.h"
#include "cisco_hello_protocol.h"

/** struct stream_reader: An abstraction of a buffer for parsing data as a stream
  * @stream: The pointer to the buffer to parse
  * @position: The current position within the buffer
  */
struct stream_reader
{
	struct s_buffer_stream *stream;
	off_t position;
};

/** Constructs a new stream reader object.
  *  @data: A pointer to the data buffer itself.
  *  @length: The length of the buffer in bytes.
  *  @return: The new object or NULL on error
  */
struct stream_reader *stream_reader_new(const uint8_t *data, size_t length);

/** Delete a stream reader object.
  *  @reader: The reader object to delete.
  */
void stream_reader_delete(struct stream_reader *reader);

/** Returns the number of bytes remaining before the end of the buffer
  *  @reader: The reader object.
  *  @return: The number of bytes remaining
  */
size_t stream_reader_remaining(const struct stream_reader *reader);

/** Returns whether the position of the stream reader is at the end of the stream.
  *  @reader: The reader object.
  *  @return: true if at the end of the stream.
  */
bool stream_reader_at_end(const struct stream_reader *reader);

/** Returns whether there is the requested number of bytes remaining to be read in the buffer
  *  @reader: The reader object
  *  @bytesNeeded: The number of bytes needed to be present
  */
bool stream_reader_need(const struct stream_reader *reader, size_t bytesNeeded);

/** Returns the current position of the stream.
  *  @reader: The reader object.
  *  @return: The current position of the stream reader.
  */
off_t stream_reader_get_position(const struct stream_reader *reader);

/** Sets the current position of the stream manually.
  *  @reader: The reader object.
  *  @newPosition: The new byte position in the stream.
  *  @return 0 on success or a negative value on error.
  */
int stream_reader_set_position(struct stream_reader *reader, off_t newPosition);

/** Sets the current position relative to the current position by the given number of bytes.
*  @reader: The reader object.
*  @toSkip: The number of bytes to advance the stream.
*  @return 0 on success or a negative value on error.
*/
int stream_reader_skip(struct stream_reader *reader, off_t toSkip);

/** Reads an 8-bit value from the stream
  *  @reader: The reader object
  *  @result: The resulting value
  *  @return: 0 or a negative value representing an error
  */
int stream_reader_get8(struct stream_reader *reader, uint8_t *result);

/** Reads a 16-bit big endian value from the stream
  *  @reader: The reader object
  *  @result: The resulting value
  *  @return: 0 or a negative value representing an error
  */
int stream_reader_get16(struct stream_reader *reader, uint16_t *result);

/** Reads a 24-bit big endian value from the stream
  *  @reader: The reader object
  *  @result: The resulting value
  *  @return: 0 or a negative value representing an error
  */
int stream_reader_get24(struct stream_reader *reader, uint32_t *result);

/** Reads a 32-bit big endian value from the stream
  *  @reader: The reader object
  *  @result: The resulting value
  *  @return: 0 or a negative value representing an error
  */
int stream_reader_get32(struct stream_reader *reader, uint32_t *result);

/** Reads a zero terminated string up to the given number of characters from the stream
  *  @reader: The reader object
  *  @result: A resulting zero terminated string or NULL
  *  @maximumLength: The maximum length of the string in bytes.
  *  @return: 0 on success or a negative value on error
  *
  * Even if the string is null terminated, the position will be advanced to the position
  * signified by maximumLength.
  */
int stream_reader_get_string(struct stream_reader *reader, char **result, size_t maximumLength);

/** Reads fixed size buffer from the stream
  *  @reader: The reader object
  *  @result: The buffer, it must be pre-allocated this function won't do it itself.
  *  @count: The number of bytes to read
  *  @return: 0 on success or a negative value on error
  */
int stream_reader_get_buffer(struct stream_reader *reader, uint8_t *result, size_t count);

/** Reads an IPv4 or IPv6 address from the stream.
  *  @reader: The reader object
  *  @result: A resulting IP address
  *  @return: 0 on success or a negative value on error
  *
  * This function identifies the address family of the address and allocates the proper sockaddr 
  * structure type.
  */
int stream_reader_get_address(struct stream_reader *reader, struct sockaddr **result);

/** Read the Cisco cluster management protocol TLV.
  *  @reader: The reader object
  *  @result: The resulting cluster management protocol information or NULL on error.
  *  @return: 0 on success or a negative value on error.
  */
int stream_reader_get_cisco_cluster_management_protocol(struct stream_reader *reader, struct cisco_hello_protocol **result);

#endif
