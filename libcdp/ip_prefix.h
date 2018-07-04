#ifndef IP_PREFIX_H
#define IP_PREFIX_H

#include <linux/socket.h>

/** @summary A representation of a network prefix */
struct ip_prefix
{
	/** @summary The address component of the prefix */
	struct sockaddr *network;

	/** @summary The length in bits of the prefix */
	int length;
};

/** @summary Constructor
  * @return The new instance or NULL on error
  */
struct ip_prefix *ip_prefix_new(void);

/** @summary Destructor
  * @param prefix The prefix to delete
  */
void ip_prefix_delete(struct ip_prefix *prefix);

/** @summary Sets the network address and length
  *
  * This function takes ownership of the network address and calls
  * to ip_prefix_delete will free the address.
  *
  * @param prefix The prefix to update.
  * @param network The network address to set.
  * @param length The length of the prefix.
  * @return Either 0 on success or a negative value on failure.
  */
int ip_prefix_set(struct ip_prefix *prefix, struct sockaddr *network, int length);

#endif
