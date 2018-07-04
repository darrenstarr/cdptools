#include "cisco_cluster_management_protocol.h"
#include "platform.h"

struct cisco_cluster_management_protocol *cisco_cluster_management_protocol_new(void)
{
	struct cisco_cluster_management_protocol *result;

	result = ALLOC_NEW(struct cisco_cluster_management_protocol);

	if (result == NULL)
	{
		LOG_ERROR("cisco_cluster_management_protocol_new: Failed to allocate buffer\n");
		return NULL;
	}

	ZERO_BUFFER(result, struct cisco_cluster_management_protocol);

	return result;
}

void cisco_cluster_management_protocol_delete(struct cisco_cluster_management_protocol *clusterManagementProtocol)
{
	if (clusterManagementProtocol == NULL)
	{
		LOG_CRITICAL("cisco_cluster_management_protocol_delete: clusterManagementProtocol is null\n");
		return;
	}

	FREE(clusterManagementProtocol);
}

