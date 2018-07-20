#ifndef CDP_MODULE_H
#define CDP_MODULE_H

#include "../libcdp/cdp_neighbor.h"
#include <linux/netdevice.h>
#include <net/datalink.h>

/** A read/write spin-lock for controlling access to cdp_neighbors */
extern rwlock_t cdp_neighbors_rw_lock;

/** A list of the known CDP neighbor entries */
extern struct cdp_neighbor_list *cdp_neighbors;

/** This is the software version string sent to all CDP neighbors to describe this device */
extern char *cdp_software_version_string;

/** This is the device ID string sent to all CDP neighbors to describe this device */
extern char *cdp_device_id_string;

/** The Multicast MAC address to which CDP frames are sent. */
extern /*const*/ uint8_t cdp_multicast_address[];

struct datalink_proto;

/** Handle to the datalink protocol for CDP */
extern struct datalink_proto *cdp_snap_datalink_protocol;

#endif
