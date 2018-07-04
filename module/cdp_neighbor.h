#ifndef MOD_CDP_H
#define MOD_CDP_H

#include <linux/time.h>
#include <linux/types.h>

/** Cisco Discovery Protocol neighbor */
struct cdp_neighbor
{
    /** The type of device which the frame was received from.
      * This aligns with the values from /include/linux/if_arp.h
      */
    int device_type;

    /** The name of the interface upon which the neighbor exists. */
    char *device_name;

    /** The neighbor's MAC address */
    unsigned char *remote_mac;

    /** The neighbor's MAC address length */
    size_t remote_mac_length;

    /** The time when the current frame was received */
    struct timespec received_at;

    /** The size of the frame buffer as it was allocated */
    size_t frame_buffer_size;

    /** The length of the consumed portion of the frame buffer */
    size_t frame_buffer_length;

    /** The frame buffer itself. */
    unsigned char *frame_buffer;

    /** The next item in the linked list of neighbors */
    struct cdp_neighbor *next;

    /** The previous item in the linked list of neighbors */
    struct cdp_neighbor *prev;
};

/** Constructor
  *  @return Either the new object or NULL on error.
  */
struct cdp_neighbor *cdp_neighbor_new(void);

/** Destructor
  *  @param neighbor The neighbor to delete
  */
void cdp_neighbor_delete(struct cdp_neighbor *neighbor);

/** Set the device type.
  *  @param neighbor The neighbor object.
  *  @param device_type The device type.
  *  @return 0 on success, a negative value on failure.
  */
int cdp_neighbor_set_device_type(struct cdp_neighbor *neighbor, int device_type);

/** Set the device name.
  *  @param neighbor The neighbor object.
  *  @param device_name The value to set
  *  @return 0 on success, a negative value on failure.
  */
int cdp_neighbor_set_device_name(struct cdp_neighbor *neighbor, const char *device_name);

/** Set the device's remote MAC address
  *  @param neighbor The neighbor object.
  *  @param remote_mac The remote MAC address buffer.
  *  @param remote_mac_length The length of the remote MAC address in bytes.
  *  @return 0 on success, a negative value on failure.
  */
int cdp_neighbor_set_remote_mac(struct cdp_neighbor *neighbor, const unsigned char *remote_mac, size_t remote_mac_length);

/** Set the neighbor's received at time.
  *  @param neighbor The neighbor object.
  *  @param received_at The time the last CDP message was received.
  *  @return 0 on success, a negative value on failure.
  */
int cdp_neighbor_set_received_at(struct cdp_neighbor *neighbor, struct timespec received_at);

/** Sets the content of the neighbor's received CDP message buffer
  *  @param neighbor The neighbor object.
  *  @param frame_buffer The buffer containing the frame (will be copied).
  *  @param frame_buffer_length The length of the frame buffer in bytes.
  *  @return 0 on success or a negative value on failure.
  */
int cdp_neighbor_set_frame_buffer(struct cdp_neighbor *neighbor, const unsigned char *frame_buffer, size_t frame_buffer_length);

/** Tests to see whether the entry's device name matches the given.
  *  @param neighbor The neighbor entry.
  *  @param device_name The name to test against.
  *  @return true on match or false otherwise.
  */
bool cdp_neighbor_device_name_equals(const struct cdp_neighbor *neighbor, const char *device_name);

/** Tests to see whether the entry's remote MAC address matches the given.
  *  @param neighnor The neighbor entry.
  *  @param remote_mac The remote MAC address to test.
  *  @param remote_mac_length The remote mac address length.
  *  @return true on match or false otherwise.
  */
bool cdp_neighbor_remote_mac_equals(const struct cdp_neighbor *neighbor, const unsigned char *remote_mac, size_t remote_mac_length);

/** Extracts the hold time from the frame_buffer if it's available.
  *  This is a helper function that is present so that it's not necessary to
  *  parse the entire frame buffer just to check for expiration.
  *  @param neighbor The neighbor object
  *  @return the hold time or a negative value on error.
  */
int cdp_neighbor_get_hold_time(const struct cdp_neighbor *neighbor);

/** Checks whether the neighbor's record is expired
  *  @param neighbor The neighbor object.
  *  @param now The time relative to the received_at times.
  *  @return true if the record is expired, false otherwise.
  */
bool cdp_neighbor_is_expired(const struct cdp_neighbor *neighbor, struct timespec now);

/** A container for Cisco Discovery Protocol neighbors. */
struct cdp_neighbor_list
{
    /** The head or first item of the list */
    struct cdp_neighbor *head;

    /** The tail or last item of the list */
    struct cdp_neighbor *tail;

    /** The number of items known to be in the list. */
    int count;
};

/** Constructor
  *  @return Either the new initialized list or a NULL reference
  */
struct cdp_neighbor_list *cdp_neighbor_list_new(void);

/** Destructor, this simply deletes the list object
  *  @param list The list to delete.
  */
void cdp_neighbor_list_delete(struct cdp_neighbor_list *list);

/** Delete all the items in the list
  *  @param list the list to delete the items from
  */
void cdp_neighbor_list_clean(struct cdp_neighbor_list *list);

/** Destructor, this deletes all the child items in the list as well.
  *  @param list the list to clean and delete.
  */
void cdp_neighbor_list_clean_and_delete(struct cdp_neighbor_list *list);

/** Removes the first item from the list and returns its value.
  *  @param list the list to take from
  *  @return either the item which was at the head of the list or NULL if the list is empty.
  */
struct cdp_neighbor *cdp_neighbor_list_take_first(struct cdp_neighbor_list *list);

/** Returns the neighbor from the list at the given index
  *  @param list The list object.
  *  @param index The index of the neighbor in the list.
  *  @return The requested item or NULL if past end.
  */
struct cdp_neighbor *cdp_neighbor_list_get_by_index(struct cdp_neighbor_list *list, int index);

/** Finds the neighbor entry by the network device name it was received on and the remote MAC address.
  *  @param list The list to search.
  *  @param device_name The name of the network interface to search on.
  *  @param remote_mac The MAC address on the interface to search for.
  *  @param remote_mac_length The length of the remote_mac in bytes.
  *  @return Either the CDP neighbor entry or NULL if it wasn't found.
  */
struct cdp_neighbor *cdp_neighbor_list_get_by_identity(
    struct cdp_neighbor_list *list,
    const char *device_name,
    const unsigned char *remote_mac,
    size_t remote_mac_length);

/** Finds the neighbor entry by the network device name it was received on and the remote MAC address.
  *  If the neighbor doesn't exist, it will create a new entry and insert it into the list.
  *  @param list The list to search.
  *  @param device_type The device type of the network interface.
  *  @param device_name The name of the network interface to search on.
  *  @param remote_mac The MAC address on the interface to search for.
  *  @param remote_mac_length The length of the remote_mac in bytes.
  *  @return Either the CDP neighbor entry or NULL if it wasn't found.
  */
struct cdp_neighbor *cdp_neighbor_list_get_or_create_by_identity(
    struct cdp_neighbor_list *list,
    int device_type,
    const char *device_name,
    const unsigned char *remote_mac,
    size_t remote_mac_length);

/** Finds the neighbor entry by the network device name it was received on and the remote MAC
  *  address and removes it from the list before returning.
  *  @param list The list to search.
  *  @param device_name The name of the network interface to search on.
  *  @param remote_mac The MAC address on the interface to search for.
  *  @param remote_mac_length The length of the remote_mac in bytes.
  *  @return Either the CDP neighbor entry or NULL if it wasn't found.
  */
struct cdp_neighbor *cdp_neighbor_list_take_by_identity(
    struct cdp_neighbor_list *list,
    const char *device_name,
    const unsigned char *remote_mac,
    size_t remote_mac_length);

/** Appends a new item to the end of the list
  *  @param list The list to append to.
  *  @param item The item to append to the end of the list.
  *  @return 0 on success, a negative value on failure.
  */
int cdp_neighbor_list_append(struct cdp_neighbor_list *list, struct cdp_neighbor *item);

/** Removes the given item from the list
  *  @param list The list object.
  *  @param item The item to remove.
  *  @return 0 on success, a negative value on error.
  */
int cdp_neighbor_list_remove_item(struct cdp_neighbor_list *list, struct cdp_neighbor *item);

/** Iterates over the list and purges the neighbors whose hold timers have expired
  *  @param list The list object
  *  @param now The current time relative to received_at
  *  @return 0 on success, a negative value on error.
  */
int cdp_neighbor_list_purge_expired_neighbors(struct cdp_neighbor_list *list, struct timespec now);

#endif
