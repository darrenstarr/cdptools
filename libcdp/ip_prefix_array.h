#ifndef IP_PREFIX_ARRAY_H
#define IP_PREFIX_ARRAY_H

#include "ip_prefix.h"
#include "platform/types.h"

/** A fixed length array of ip_prefixes */
struct ip_prefix_array
{
	/** The prefixes array */
	struct ip_prefix **prefixes;

	/** The number of prefixes */
	size_t count;
};

/** Constructs a new array
  *  @count: The fixed number of IP prefixes to be stored in the array
  *  @return: The array on success, NULL on failure
  */
struct ip_prefix_array *ip_prefix_array_new(size_t count);

/** Delete and array of IP prefixes
  *  @param array: The array to delete.
  *
  * This does not delete the prefixes themselves, call ip_prefix_array_clear_and_delete for that
  */
void ip_prefix_array_delete(struct ip_prefix_array *array);

/** Deletes and sets to null all the prefixes in the array.
  *  @param array: The array to clear
  *  @return: 0 on success, a negative number on failure
  */
int ip_prefix_array_clear(struct ip_prefix_array *array);

/** Performs both a clear and a delete on the array
  *  @param array: The array to clear
  *  @return: 0 on success, a negative number on failure
  */
int ip_prefix_array_clear_and_delete(struct ip_prefix_array *array);

/** Sets the value of an index within the array. This takes ownership of the prefix.
  *  @param array: The array to alter
  *  @param index: The index of the slot to occupy
  *  @param prefix: The address to place at the index
  *  @return: 0 on success, a negative number on failure
  */
int ip_prefix_array_set_into(struct ip_prefix_array *array, off_t index, struct ip_prefix *prefix);

#endif
