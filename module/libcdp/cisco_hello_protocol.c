#include "cisco_hello_protocol.h"
#include "platform.h"

struct cisco_hello_protocol *cisco_hello_protocol_new(void)
{
	struct cisco_hello_protocol *result;

	result = ALLOC_NEW(struct cisco_hello_protocol);

	if (result == NULL)
	{
		LOG_ERROR("cisco_hello_protocol_new: Failed to allocate buffer\n");
		return NULL;
	}

	ZERO_BUFFER(result, struct cisco_hello_protocol);

	return result;
}

void cisco_hello_protocol_delete(struct cisco_hello_protocol *hello)
{
	if (hello == NULL)
	{
		LOG_CRITICAL("cisco_hello_protocol_delete: hello is null\n");
		return;
	}

	FREE(hello);
}

