#include "ip_address_array.h"
#include "platform/platform.h"

struct ip_address_array *ip_address_array_new(size_t count)
{
	struct ip_address_array *result;
	struct sockaddr **addresses;
	int i;

	addresses = ALLOC_NEW_ARRAY(struct sockaddr *, count);
	if (addresses == NULL)
	{
		LOG_ERROR("ip_address_array_new: failed to allocate memory for address storage\n");
		return NULL;
	}

	result = ALLOC_NEW(struct ip_address_array);
	if (result == NULL)
	{
		LOG_ERROR("ip_address_array_new: failed to allocate memory for container.\n");
		FREE_ARRAY(addresses);

		return NULL;
	}

	for (i = 0; i < count; i++)
		addresses[i] = NULL;

	result->addresses = addresses;
	result->count = count;

	return result;
}

void ip_address_array_delete(struct ip_address_array *array)
{
	if (array == NULL)
	{
		LOG_CRITICAL("ip_address_array_delete: array is NULL\n");
		return;
	}

	if (array->addresses == NULL)
	{
		LOG_CRITICAL("ip_address_array_delete: array contents is NULL\n");
		return;
	}

	FREE_ARRAY(array->addresses);

	FREE(array);
}

int ip_address_array_clear(struct ip_address_array *array)
{
	off_t index;

	if (array == NULL)
	{
		LOG_CRITICAL("ip_address_array_clear: array is NULL\n");
		return -1;
	}

	if (array->addresses == NULL)
	{
		LOG_CRITICAL("ip_address_array_clear: array contents is NULL\n");
		return -1;
	}

	for (index = 0; index < array->count; index++)
	{
		if (array->addresses[index] != NULL)
		{
			FREE(array->addresses[index]);
			array->addresses[index] = NULL;
		}
	}

	return 0;
}

int ip_address_array_clear_and_delete(struct ip_address_array *array)
{
	if (array == NULL)
	{
		LOG_CRITICAL("ip_address_array_clear_and_delete: array is NULL\n");
		return -1;
	}

	if (ip_address_array_clear(array) < 0)
	{
		LOG_ERROR("ip_address_array_clear_and_delete: Failed to clear the array\n");
		return -1;
	}

	ip_address_array_delete(array);

	return 0;
}

int ip_address_array_set_into(struct ip_address_array *array, off_t index, struct sockaddr *address)
{
	if (array == NULL)
	{
		LOG_CRITICAL("ip_address_set_into: array is NULL\n");
		return -1;
	}

	if (index >= array->count)
	{
		LOG_CRITICAL("ip_address_set_into: input past end\n");
		return -1;
	}

	if (array->addresses[index] != NULL)
		FREE(array->addresses[index]);

	array->addresses[index] = address;

	return 0;
}
