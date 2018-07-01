#include "power_over_ethernet_availability.h"
#include "platform.h"

struct power_over_ethernet_availability *power_over_ethernet_availability_new()
{
	struct power_over_ethernet_availability *result;

	result = ALLOC_NEW(struct power_over_ethernet_availability);
	if (result == NULL)
	{
		LOG_ERROR("power_over_ethernet_availability_new: Failed to allocate memory for result\n");
		return NULL;
	}

	ZERO_BUFFER(result, struct power_over_ethernet_availability);

	return result;
}

void power_over_ethernet_availability_delete(struct power_over_ethernet_availability *poe)
{
	if (poe == NULL)
	{
		LOG_CRITICAL("power_over_ethernet_availability_delete: poe is null\n");
		return;
	}

	FREE(poe);
}

int power_over_ethernet_availability_read(struct stream_reader *reader, struct power_over_ethernet_availability **result)
{
	uint16_t request_id;
	uint16_t management_id;
	uint32_t availableMilliwatts;
	uint32_t powerManagementLevel;

	if (reader == NULL)
	{
		LOG_CRITICAL("power_over_ethernet_availability_read: reader is null\n");
		return -1;
	}

	if (result == NULL)
	{
		LOG_CRITICAL("power_over_ethernet_availability_read: result is null\n");
		return -1;
	}

	if (stream_reader_get16(reader, &request_id) < 0)
	{
		LOG_ERROR("power_over_ethernet_availability_read: failed to read request ID\n");
		return -1;
	}

	if (stream_reader_get16(reader, &management_id) < 0)
	{
		LOG_ERROR("power_over_ethernet_availability_read: failed to read management ID\n");
		return -1;
	}

	if (stream_reader_get32(reader, &availableMilliwatts) < 0)
	{
		LOG_ERROR("power_over_ethernet_availability_read: failed to the amount of power available\n");
		return -1;
	}

	if (stream_reader_get32(reader, &powerManagementLevel) < 0)
	{
		LOG_ERROR("power_over_ethernet_availability_read: failed to the power management level\n");
		return -1;
	}

	*result = power_over_ethernet_availability_new();
	if (*result == NULL)
	{
		LOG_ERROR("power_over_ethernet_availability_read: failed to allocate memory to store the result\n");
		return -1;
	}

	(*result)->request_id = request_id;
	(*result)->management_id = management_id;
	(*result)->availableMilliwatts = availableMilliwatts;
	(*result)->powerManagementLevel = (int32_t)powerManagementLevel;

	return 0;
}
