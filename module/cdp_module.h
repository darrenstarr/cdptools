#ifndef CDP_MODULE_H
#define CDP_MODULE_H

#include "../libcdp/cdp_neighbor.h"

/** A read/write spin-lock for controlling access to cdp_neighbors */
extern rwlock_t cdp_neighbors_rw_lock;

/** A list of the known CDP neighbor entries */
extern struct cdp_neighbor_list *cdp_neighbors;

#endif
