#include "ip_prefix.h"
#include "platform.h"

struct ip_prefix *ip_prefix_new(void)
{
	struct ip_prefix *result;

	result = ALLOC_NEW(struct ip_prefix);
	if (result == NULL)
	{
		LOG_ERROR("ip_prefix_new: failed to allocate memory for storing and ip_prefix.\n");
		return NULL;
	}

	result->network = NULL;
	result->length = 0;

	return result;
}

void ip_prefix_delete(struct ip_prefix *prefix)
{
	if (prefix == NULL)
	{
		LOG_CRITICAL("ip_prefix_delete: prefix is NULL\n");
		return;
	}

	if (prefix->network != NULL)
		FREE(prefix->network);

	FREE(prefix);
}

int ip_prefix_set(struct ip_prefix *prefix, struct sockaddr *network, int length)
{
	if (prefix == NULL)
	{
		LOG_CRITICAL("ip_prefix_delete: prefix is NULL\n");
		return -1;
	}

	if (prefix->network != NULL)
		FREE(prefix->network);

	prefix->network = network;
	prefix->length = length;

	return 0;
}

