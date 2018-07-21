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

int ip_address_array_copy_into(struct ip_address_array *array, off_t index, struct sockaddr *address)
{
	struct sockaddr *copy;

	if (array == NULL)
	{
		LOG_CRITICAL("ip_address_array_copy_into: array is NULL\n");
		return -1;
	}

	if (index >= array->count)
	{
		LOG_CRITICAL("ip_address_array_copy_into: input past end\n");
		return -1;
	}

	if (address == NULL)
	{
		copy = NULL;
	}
	else
	{
		switch (address->sa_family)
		{
			case AF_INET:
				copy = ALLOC_NEW(struct sockaddr_in);
				if (copy == NULL)
				{
					LOG_CRITICAL("ip_address_array_copy_into: failed to allocate memory to copy address\n");
					return -1;
				}
				memcpy(copy, address, sizeof(struct sockaddr_in));
				break;

			case AF_INET6:
				copy = ALLOC_NEW(struct sockaddr_in6);
				if (copy == NULL)
				{
					LOG_CRITICAL("ip_address_array_copy_into: failed to allocate memory to copy address\n");
					return -1;
				}
				memcpy(copy, address, sizeof(struct sockaddr_in6));
				break;

			default:
				LOG_CRITICAL("ip_address_array_copy_into: unknown address type\n");
				return -1;
		}
	}

	if (array->addresses[index] != NULL)
		FREE(array->addresses[index]);

	array->addresses[index] = copy;

	return 0;
}

int ip_address_array_set_into_ipv4_uint32(struct ip_address_array *array, off_t index, uint32_t address)
{
	struct sockaddr_in new_item;

	if (array == NULL)
	{
		LOG_CRITICAL("ip_address_array_copy_into_ipv4_uint32: array is NULL\n");
		return -1;
	}

	if (index >= array->count)
	{
		LOG_CRITICAL("ip_address_array_copy_into_ipv4_uint32: input past end\n");
		return -1;
	}

	memset(&new_item, 0, sizeof(struct sockaddr_in));
	new_item.sin_family = AF_INET;
	new_item.sin_addr.s_addr = address;

	return ip_address_array_copy_into(array, index, (struct sockaddr *)&new_item);
}

int ip_address_array_set_into_ipv6_raw(struct ip_address_array *array, off_t index, const uint8_t *address)
{
	struct sockaddr_in6 new_item;

	if (array == NULL)
	{
		LOG_CRITICAL("ip_address_array_set_into_ipv6_raw: array is NULL\n");
		return -1;
	}

	if (address == NULL)
	{
		LOG_CRITICAL("ip_address_array_set_into_ipv6_raw: address is NULL\n");
		return -1;
	}

	if (index >= array->count)
	{
		LOG_CRITICAL("ip_address_array_set_into_ipv6_raw: input past end\n");
		return -1;
	}

	memset(&new_item, 0, sizeof(struct sockaddr_in6));
	new_item.sin6_family = AF_INET6;
	memcpy(IPv6Octets(&new_item), address, 16);

	return ip_address_array_copy_into(array, index, (struct sockaddr *)&new_item);
}
