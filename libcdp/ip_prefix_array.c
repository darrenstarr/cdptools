#include "ip_prefix_array.h"
#include "platform/platform.h"

struct ip_prefix_array *ip_prefix_array_new(size_t count)
{
	struct ip_prefix_array *result;
	struct ip_prefix **prefixes;
	int i;

	prefixes = ALLOC_NEW_ARRAY(struct ip_prefix *, count);
	if (prefixes == NULL)
	{
		LOG_ERROR("ip_prefix_array_new: failed to allocate memory for prefix storage\n");
		return NULL;
	}

	result = ALLOC_NEW(struct ip_prefix_array);
	if (result == NULL)
	{
		LOG_ERROR("ip_prefix_array_new: failed to allocate memory for container.\n");
		FREE_ARRAY(prefixes);

		return NULL;
	}

	for (i = 0; i < count; i++)
		prefixes[i] = NULL;

	result->prefixes = prefixes;
	result->count = count;

	return result;
}

void ip_prefix_array_delete(struct ip_prefix_array *array)
{
	if (array == NULL)
	{
		LOG_CRITICAL("ip_prefix_array_delete: array is NULL\n");
		return;
	}

	if (array->prefixes == NULL)
	{
		LOG_CRITICAL("ip_prefix_array_delete: array contents is NULL\n");
		return;
	}

	FREE_ARRAY(array->prefixes);

	FREE(array);
}

int ip_prefix_array_clear(struct ip_prefix_array *array)
{
	off_t index;

	if (array == NULL)
	{
		LOG_CRITICAL("ip_prefix_array_clear: array is NULL\n");
		return -1;
	}

	if (array->prefixes == NULL)
	{
		LOG_CRITICAL("ip_prefix_array_clear: array contents is NULL\n");
		return -1;
	}

	for (index = 0; index < array->count; index++)
	{
		if (array->prefixes[index] != NULL)
		{
			ip_prefix_delete(array->prefixes[index]);
			array->prefixes[index] = NULL;
		}
	}

	return 0;
}

int ip_prefix_array_clear_and_delete(struct ip_prefix_array *array)
{
	if (array == NULL)
	{
		LOG_CRITICAL("ip_prefix_array_clear_and_delete: array is NULL\n");
		return -1;
	}

	if (ip_prefix_array_clear(array) < 0)
	{
		LOG_ERROR("ip_prefix_array_clear_and_delete: Failed to clear the array\n");
		return -1;
	}

	ip_prefix_array_delete(array);

	return 0;
}

int ip_prefix_array_set_into(struct ip_prefix_array *array, off_t index, struct ip_prefix *prefix)
{
	if (array == NULL)
	{
		LOG_CRITICAL("ip_prefix_set_into: array is NULL\n");
		return -1;
	}

	if (index >= array->count)
	{
		LOG_CRITICAL("ip_prefix_set_into: input past end\n");
		return -1;
	}

	if (array->prefixes[index] != NULL)
		ip_prefix_delete(array->prefixes[index]);

	array->prefixes[index] = prefix;

	return 0;
}

