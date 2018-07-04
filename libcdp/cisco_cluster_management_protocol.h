#ifndef cisco_cluster_management_protocol_H
#define cisco_cluster_management_protocol_H

#include "platform_types.h"
#include "platform_socket.h"

/** A container for data from the Cisco Cluster Management Protocol in CDP for cluster management
  */
struct cisco_cluster_management_protocol
{
	/** The organizationally unique identifier for the protocol */
	uint32_t oui;

	/** The protocol ID, should be 0x112 */
	uint16_t protocol_id;

	/** The cluster master IP */
	struct sockaddr_in cluster_master_ip;

	/** Cluster netmask */
	struct sockaddr_in netmask;

	/** Version and subversion number */
	uint16_t version;

	/** Status field, not sure about the details */
	uint8_t status;

	/** Cluster commander MAC */
	uint8_t cluster_commander_mac[6];

	/** Local MAC */
	uint8_t local_mac[6];

	/** Management VLAN */
	uint16_t management_vlan;
};

/** Constructor
  *  @return the new structure or NULL on error
  */
struct cisco_cluster_management_protocol *cisco_cluster_management_protocol_new(void);

/** Deconstructor
  *  @clusterManagementProtocol: The cisco cluster management protocol object
  */
void cisco_cluster_management_protocol_delete(struct cisco_cluster_management_protocol *clusterManagementProtocol);

#endif
