#ifdef __KERNEL__
#include <asm/checksum.h>
#else

#include "types.h"

static inline uint16_t ip_compute_csum(const uint8_t *buffer, size_t buffer_length)
{
	size_t i;
	uint32_t checksum = 0;

	for (i = 0; i < buffer_length; i++)
	{
		if (i & 1)
			checksum += (uint32_t)buffer[i];
		else
			checksum += ((uint32_t)buffer[i]) << 8;
	}

	checksum = (checksum & 0xffff) + (checksum >> 16);
	checksum = (checksum & 0xffff) + (checksum >> 16);

	return (uint16_t)(~checksum);
}

#endif
