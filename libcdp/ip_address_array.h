#ifndef IP_ADDRESS_H
#define IP_ADDRESS_H

#include "platform/socket.h"
#include "platform/types.h"

/** Container for an array of IP addresses */
struct ip_address_array
{
	struct sockaddr **addresses;
	size_t count;
};

/** Constructs a new array
  *  @param count The fixed number of IP addresses to be stored in the array
  *  @return The array on success, NULL on failure
  */
struct ip_address_array *ip_address_array_new(size_t count);

/** Delete and array of IP addresess
  *  @param array The array to delete.
  *
  * This does not delete the addresses themselves, call ip_address_array_clear_and_delete for that
  */
void ip_address_array_delete(struct ip_address_array *array);

/** Deletes and sets to null all the addresses in the array.
  *  @param array The array to clear
  *  @return 0 on success, a negative number on failure
  */
int ip_address_array_clear(struct ip_address_array *array);

/** Performs both a clear and a delete on the array
  *  @param array The array to clear
  *  @return 0 on success, a negative number on failure
  */
int ip_address_array_clear_and_delete(struct ip_address_array *array);

/** Sets the value of an index within the array. This takes ownership of the address passed.
  *  @param array The array to alter
  *  @param index The index of the slot to occupy
  *  @param address The address to place at the index
  *  @return 0 on success, a negative number on failure
  */
int ip_address_array_set_into(struct ip_address_array *array, off_t index, struct sockaddr *address);

/** Sets the value of an index within the array.
  *  @param array The array to alter
  *  @param index The index of the slot to occupy
  *  @param address The address to place at the index
  *  @return 0 on success, a negative number on failure
  */
int ip_address_array_copy_into(struct ip_address_array *array, off_t index, struct sockaddr *address);

/** Sets the value of an index within the array.
  *  @param array The array to alter
  *  @param index The index of the slot to occupy
  *  @param address The address to place at the index (this should be host order)
  *  @return 0 on success, a negative number on failure
  */
int ip_address_array_set_into_ipv4_uint32(struct ip_address_array *array, off_t index, uint32_t address);

/** Sets the value of an index within the array.
  *  @param array The array to alter
  *  @param index The index of the slot to occupy
  *  @param address The address to place at the index (this should be host order)
  *  @return 0 on success, a negative number on failure
  */
int ip_address_array_set_into_ipv6_raw(struct ip_address_array *array, off_t index, const uint8_t *address);

#endif
