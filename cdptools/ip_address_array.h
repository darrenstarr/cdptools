#ifndef IP_ADDRESS_H
#define IP_ADDRESS_H

#include <sys/socket.h>

/** Container for an array of IP addresses */
struct ip_address_array
{
	struct sockaddr **addresses;
	size_t count;
};

/** Constructs a new array
  *  @count: The fixed number of IP addresses to be stored in the array
  *  @return: The array on success, NULL on failure
  */
struct ip_address_array *ip_address_array_new(size_t count);

/** Delete and array of IP addresess
  *  @array: The array to delete.
  *
  * This does not delete the addresses themselves, call ip_address_array_clear_and_delete for that
  */
void ip_address_array_delete(struct ip_address_array *array);

/** Deletes and sets to null all the addresses in the array.
  *  @array: The array to clear
  *  @return: 0 on success, a negative number on failure
  */
int ip_address_array_clear(struct ip_address_array *array);

/** Performs both a clear and a delete on the array
  *  @array: The array to clear
  *  @return: 0 on success, a negative number on failure
  */
int ip_address_array_clear_and_delete(struct ip_address_array *array);

/** Sets the value of an index within the array. This takes ownership of the address passed.
  *  @array: The array to alter
  *  @index: The index of the slot to occupy
  *  @address: The address to place at the index
  */
int ip_address_set_into(struct ip_address_array *array, off_t index, struct sockaddr *address);

#endif
