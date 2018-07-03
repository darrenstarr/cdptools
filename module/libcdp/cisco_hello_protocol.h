#ifndef CISCO_HELLO_PROTOCOL_H
#define CISCO_HELLO_PROTOCOL_H

#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>

/** A container for data from the Cisco Hello Protocol in CDP for cluster management
  */
struct cisco_hello_protocol
{
	/** The organizationally unique identifier for the hello */
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
struct cisco_hello_protocol *cisco_hello_protocol_new(void);

/** Deconstructor
  *  @hello: The cisco hello protocol object
  */
void cisco_hello_protocol_delete(struct cisco_hello_protocol *hello);

#endif
