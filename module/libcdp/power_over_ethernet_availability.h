#ifndef POWER_OVER_ETHERNET_AVAILABILITY_H
#define POWER_OVER_ETHERNET_AVAILABILITY_H

#include <linux/types.h>
#include "stream_reader.h"

/** Representation of PoE availability from the switch */
struct power_over_ethernet_availability
{
	/** The request which this is responding to. 0 is gratuitous */
	uint16_t request_id;

	/** The management ID of the frame. Not sure what this is. */
	uint16_t management_id;

	/** Available power */
	uint32_t availableMilliwatts;

	/** Power management level */
	int32_t powerManagementLevel;
};

/** Constructor for the class
  *  @return: a new object or NULL on error.
  */
struct power_over_ethernet_availability *power_over_ethernet_availability_new(void);

/** Destructor for the class
  *  @poe: the instance to delete.
  */
void power_over_ethernet_availability_delete(struct power_over_ethernet_availability *poe);

/** Read and deserialize and instance of the structure from a stream
  *  @reader: The reader
  *  @result: The result or NULL on error.
  *  @return: 0 on success, a negative value on error.
  */
int power_over_ethernet_availability_read(struct stream_reader *reader, struct power_over_ethernet_availability **result);

#endif
