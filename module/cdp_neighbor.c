#include <linux/slab.h>
#include <linux/string.h>

#include "cdp_neighbor.h"

struct cdp_neighbor *cdp_neighbor_new(void)
{
    struct cdp_neighbor *result = kmalloc(sizeof(struct cdp_neighbor), GFP_ATOMIC);

    if(result == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_new: failed to allocate memory for CDP neighbor.\n");
        return NULL;
    }

    result->device_type = 0;
    result->device_name = NULL;
    result->remote_mac = NULL;
    result->remote_mac_length = 0;
    result->received_at.tv_sec = 0;
    result->received_at.tv_nsec = 0;
    result->frame_buffer_size = 0;
    result->frame_buffer_length = 0;
    result->frame_buffer = NULL;
    result->next = NULL;
    result->prev = NULL;

    return result;
}

void cdp_neighbor_delete(struct cdp_neighbor *neighbor)
{
    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_delete: neighbor is NULL.\n");
        return;
    }

    if(neighbor->next != NULL || neighbor->prev != NULL)
        printk(KERN_CRIT "cdp_neighbor_delete: deleting neighbor which appears to still be in a list.\n");

    if(neighbor->device_name != NULL)
        kfree(neighbor->device_name);

    if(neighbor->remote_mac != NULL)
        kfree(neighbor->remote_mac);

    if(neighbor->frame_buffer != NULL)
        kfree(neighbor->frame_buffer);
}

int cdp_neighbor_set_device_type(struct cdp_neighbor *neighbor, int device_type)
{
    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_device_type: neighbor is NULL.\n");
        return -1;
    }

    neighbor->device_type = device_type;

    return 0;
}

int cdp_neighbor_set_device_name(struct cdp_neighbor *neighbor, const char *device_name)
{
    size_t device_name_length;

    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_device_name: neighbor is NULL.\n");
        return -1;
    }

    if(device_name == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_device_name: device_name is NULL.\n");
        return -1;
    }

    if(neighbor->device_name != NULL)
        kfree(neighbor->device_name);

    device_name_length = strlen(device_name);

    neighbor->device_name = kmalloc((device_name_length + 1) * sizeof(char), GFP_ATOMIC);
    
    if(neighbor->device_name == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_device_name: failed to allocate memory to store the new device_name\n");
        return -1;
    }

    strcpy(neighbor->device_name, device_name);

    return 0;
}

int cdp_neighbor_set_remote_mac(struct cdp_neighbor *neighbor, const unsigned char *remote_mac, size_t remote_mac_length)
{
    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_remote_mac: neighbor is NULL.\n");
        return -1;
    }

    if(remote_mac == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_remote_mac: remote_mac is NULL.\n");
        return -1;
    }

    if(remote_mac_length == 0)
    {
        printk(KERN_CRIT "cdp_neighbor_set_remote_mac: remote_mac_length is zero.\n");
        return -1;
    }

    if(neighbor->remote_mac != NULL)
    {
        if(neighbor->remote_mac_length == remote_mac_length)
        {
            memcpy(neighbor->remote_mac, remote_mac, remote_mac_length);
            return 0;
        }

        kfree(neighbor->remote_mac);
    }

    neighbor->remote_mac = kmalloc(remote_mac_length, GFP_ATOMIC);

    if(neighbor->remote_mac == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_remote_mac: failed to allocate memory to store the new remote MAC address\n");
        return -1;
    }

    memcpy(neighbor->remote_mac, remote_mac, remote_mac_length);
    neighbor->remote_mac_length = remote_mac_length;

    return 0;
}

int cdp_neighbor_set_received_at(struct cdp_neighbor *neighbor, struct timespec received_at)
{
    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_received_at: neighbor is NULL.\n");
        return -1;
    }

    neighbor->received_at = received_at;

    return 0;
}

int cdp_neighbor_set_frame_buffer(struct cdp_neighbor *neighbor, const unsigned char *frame_buffer, size_t frame_buffer_length)
{
    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_frame_buffer: neighbor is NULL.\n");
        return -1;
    }

    if(frame_buffer == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_set_frame_buffer: frame_buffer is NULL.\n");
        return -1;
    }

    if(frame_buffer_length == 0)
    {
        printk(KERN_CRIT "cdp_neighbor_set_frame_buffer: frame_buffer_length is 0.\n");
        return -1;
    }

    /* If there is a frame buffer already but it's too small to contain the new frame buffer, then delete the old one */
    if(neighbor->frame_buffer != NULL && neighbor->frame_buffer_size < frame_buffer_length)
    {
        kfree(neighbor->frame_buffer);
        neighbor->frame_buffer_length = 0;
        neighbor->frame_buffer_size = 0;
    }

    /* If there is no frame buffer, then allocate a new one */
    if(neighbor->frame_buffer == NULL)
    {
        if(neighbor->frame_buffer_size > 0)
        {
            printk(KERN_CRIT "cdp_neighbor_set_frame_buffer: neighbor->frame_buffer is null but neighbor->frame_buffer_size is greater than 0, frame buffer corrupt!\n");
            return -1;
        }

        if(neighbor->frame_buffer_length > 0)
        {
            printk(KERN_CRIT "cdp_neighbor_set_frame_buffer: neighbor->frame_buffer is null but neighbor->frame_buffer_length is greater than 0, frame buffer corrupt!\n");
            return -1;
        }

        neighbor->frame_buffer = kmalloc(frame_buffer_length, GFP_ATOMIC);

        if(neighbor->frame_buffer == NULL)
        {
            printk(KERN_CRIT "cdp_neighbor_set_frame_buffer: Failed to allocate memory to store frame buffer\n");
            return -1;
        }

        neighbor->frame_buffer_size = frame_buffer_length;
    }

    if(neighbor->frame_buffer_size < frame_buffer_length)
    {
        printk(KERN_CRIT "cdp_neighbor_set_frame_buffer: There isn't enough memory allocated for the new frame buffer at the point. Frame buffer corrupted!\n");
        return -1;
    }

    memcpy(neighbor->frame_buffer, frame_buffer, frame_buffer_length);
    neighbor->frame_buffer_length = frame_buffer_length;

    return 0;
}

bool cdp_neighbor_device_name_equals(const struct cdp_neighbor *neighbor, const char *device_name)
{
    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_device_name_equals: neighbor is NULL.\n");
        return false;
    }

    if(neighbor->device_name == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_device_name_equals: neighbor->device_name is NULL.\n");
        return false;
    }

    if(device_name == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_device_name_equals: device_name is NULL.\n");
        return false;
    }

    return (strcmp(neighbor->device_name, device_name) == 0) ? true : false;
}

bool cdp_neighbor_remote_mac_equals(const struct cdp_neighbor *neighbor, const unsigned char *remote_mac, size_t remote_mac_length)
{
    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_remote_mac_equals: neighbor is NULL.\n");
        return false;
    }

    if(neighbor->remote_mac == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_remote_mac_equals: neighbor->remote_mac is NULL.\n");
        return false;
    }

    if(remote_mac == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_remote_mac_equals: remote_mac is NULL.\n");
        return false;
    }

    if(neighbor->remote_mac_length != remote_mac_length)
        return false;

    return (memcmp(neighbor->remote_mac, remote_mac, remote_mac_length) == 0) ? true : false;
}

int cdp_neighbor_get_hold_time(const struct cdp_neighbor *neighbor)
{
    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_get_hold_time: neighbor is NULL.\n");
        return -1;
    }

    if(neighbor->frame_buffer == NULL)
        return 0;

    if(neighbor->frame_buffer_size < 4)
        return 0;

    if(neighbor->frame_buffer_length < 2)
        return 0;

    return (int)neighbor->frame_buffer[1];
}

bool cdp_neighbor_is_expired(const struct cdp_neighbor *neighbor, struct timespec now)
{
    int seconds_since_update;
    int hold_time;

    if(neighbor == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_is_expired: neighbor is NULL.\n");
        return false;
    }

    hold_time = cdp_neighbor_get_hold_time(neighbor);
    if(hold_time < 0)
    {
        printk(KERN_CRIT "cdp_neighbor_is_expired: could not read the hold time from the frame.\n");
        return false;
    }

    seconds_since_update = now.tv_sec - neighbor->received_at.tv_sec;

    return ((seconds_since_update + 1) >= hold_time) ? true : false;
}


struct cdp_neighbor_list *cdp_neighbor_list_new(void)
{
    struct cdp_neighbor_list *result;

    result = kmalloc(sizeof(struct cdp_neighbor_list), GFP_ATOMIC);
    if(result == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_new: failed to allocate memory for CDP neighbor list.\n");
        return NULL;
    }

    result->head = NULL;
    result->tail = NULL;
    result->count = 0;

    return result;
}

void cdp_neighbor_list_delete(struct cdp_neighbor_list *list)
{
    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_delete: list is NULL\n");
        return;
    }

    kfree(list);
}

void cdp_neighbor_list_clean(struct cdp_neighbor_list *list)
{
    struct cdp_neighbor *entry;

    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_clean: list is NULL\n");
        return;
    }

    entry = cdp_neighbor_list_take_first(list);
    while(entry != NULL)
    {
        cdp_neighbor_delete(entry);

        entry = cdp_neighbor_list_take_first(list);
    }
}

void cdp_neighbor_list_clean_and_delete(struct cdp_neighbor_list *list)
{
    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_clean_and_delete: list is NULL\n");
        return;
    }

    cdp_neighbor_list_clean(list);

    cdp_neighbor_list_delete(list);
}

struct cdp_neighbor *cdp_neighbor_list_take_first(struct cdp_neighbor_list *list)
{
    struct cdp_neighbor *result;

    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_take_first: list is NULL\n");
        return NULL;
    }

    if(list->head == NULL)
        return NULL;

    result = list->head;

    // This should always be null;
    if(result->prev == NULL)
        list->head = result->next;
    else
    {
        printk(KERN_CRIT "cdp_neighbor_list_take_first: list head has previous element\n");
        result->prev->next = result->next;
    }

    if (result->next == NULL)
        list->tail = result->prev;
    else
        result->next->prev = result->prev;

    result->next = NULL;
    result->prev = NULL;

    list->count--;

    return result;
}

struct cdp_neighbor *cdp_neighbor_list_get_by_index(struct cdp_neighbor_list *list, int index)
{
    struct cdp_neighbor *result;

    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_by_index: list is NULL\n");
        return NULL;
    }

    if(index < 0)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_by_index: negative index requested\n");
        return NULL;
    }

    /* Check for index past end */
    if(index >= list->count)
        return NULL;

    result = list->head;
    while(index > 0 && result != NULL)
    {
        index--;
        result = result->next;
    }

    return result;
}

struct cdp_neighbor *cdp_neighbor_list_get_by_identity(
    struct cdp_neighbor_list *list,
    const char *device_name,
    const unsigned char *remote_mac,
    size_t remote_mac_length)
{
    struct cdp_neighbor *result;

    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_by_identity: list is NULL\n");
        return NULL;
    }

    result = list->head;
    while(result != NULL)
    {
        if(            
            cdp_neighbor_device_name_equals(result, device_name) &&
            cdp_neighbor_remote_mac_equals(result, remote_mac, remote_mac_length)
        )
            return result;

        result = result->next;
    }

    return NULL;
}

struct cdp_neighbor *cdp_neighbor_list_get_or_create_by_identity(
    struct cdp_neighbor_list *list,
    int device_type,
    const char *device_name,
    const unsigned char *remote_mac,
    size_t remote_mac_length)
{
    struct cdp_neighbor *result;

    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: list is NULL\n");
        return NULL;
    }

    if(device_name == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: device_name is NULL\n");
        return NULL;
    }

    if(remote_mac == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: remote_mac is NULL\n");
        return NULL;
    }

    if(remote_mac_length == 0)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: remote_mac_length is NULL\n");
        return NULL;
    }

    result = cdp_neighbor_list_get_by_identity(list, device_name, remote_mac, remote_mac_length);

    if(result != NULL)
        return result;

    result = cdp_neighbor_new();

    if(result == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: failed to allocate new entry.\n");
        return NULL;
    }

    if(cdp_neighbor_set_device_type(result, device_type) != 0)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: failed to set the device type.\n");
        cdp_neighbor_delete(result);
        return NULL;
    }

    if(cdp_neighbor_set_device_name(result, device_name) != 0)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: failed to set the device name.\n");
        cdp_neighbor_delete(result);
        return NULL;
    }

    if(cdp_neighbor_set_remote_mac(result, remote_mac, remote_mac_length) != 0)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: failed to set the remote MAC address.\n");
        cdp_neighbor_delete(result);
        return NULL;
    }

    if(cdp_neighbor_list_append(list, result) != 0)
    {
        printk(KERN_CRIT "cdp_neighbor_list_get_or_create_by_identity: failed to add new item to the list. List may be corrupt!!!\n");
        cdp_neighbor_delete(result);
        return NULL;
    }

    return result;
}

struct cdp_neighbor *cdp_neighbor_list_take_by_identity(
    struct cdp_neighbor_list *list,
    const char *device_name,
    const unsigned char *remote_mac,
    size_t remote_mac_length)
{
    struct cdp_neighbor *result;

    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_take_by_identity: list is NULL\n");
        return NULL;
    }

    result = cdp_neighbor_list_get_by_identity(list, device_name, remote_mac, remote_mac_length);

    if(result == NULL)
        return NULL;

    if(cdp_neighbor_list_remove_item(list, result) < 0)
    {
        printk(KERN_CRIT "cdp_neighbor_list_take_by_identity: failed to remove item from list\n");
        return NULL;
    }

    return result;
}

int cdp_neighbor_list_append(struct cdp_neighbor_list *list, struct cdp_neighbor *item)
{
    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_append: list is NULL\n");
        return -1;
    }

    if(item == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_append: item is NULL\n");
        return -1;
    }

    if (item->next != NULL || item->prev != NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_append: item appears to already be in a list.\n");
        return -1;
    }

    if(list->tail == NULL)
    {
        if(list->head != NULL)
        {
            printk(KERN_CRIT "cdp_neighbor_list_append: tail is null but head isn't. List is corrupt!\n");
            return -1;
        }

        list->head = item;
        list->tail = item;
        list->count++;

        return 0;
    }

    if(list->tail->next != NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_append: tail is not null but list tail has a next entry. List is corrupt!\n");
        return -1;
    }

    list->tail->next = item;
    item->prev = list->tail;
    list->tail = item;

    return 0;
}

int cdp_neighbor_list_remove_item(struct cdp_neighbor_list *list, struct cdp_neighbor *item)
{
    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_remove_item: list is NULL\n");
        return -1;
    }

    if(item == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_remove_item: item is NULL\n");
        return -1;
    }

    if(item->prev == NULL)
    {
        if(list->head != item)
        {
            printk(KERN_CRIT "cdp_neighbor_list_remove_item: found item has no previous but the list head doesn't point to it. List is corrupt!\n");
            return -1;
        }

        list->head = item->next;
    }
    else
    {
        if(item->prev->next != item)
        {
            printk(KERN_CRIT "cdp_neighbor_list_remove_item: found item's previous doesn't point back to it. List is corrupt!\n");
            return -1;
        }

        item->prev->next = item->next;
    }

    if(item->next == NULL)
    {
        if(list->tail != item)
        {
            printk(KERN_CRIT "cdp_neighbor_list_remove_item: result has no next, but the tail of the list didn't point to it. List is corrupt!\n");
            return -1;
        }

        list->tail = item->prev;
    }
    else
    {
        if(item->next->prev != item)
        {
            printk(KERN_CRIT "cdp_neighbor_list_remove_item: found item's next doesn't point back to it. List is corrupt!\n");
            return -1;
        }

        item->next->prev = item->prev;
    }

    item->next = NULL;
    item->prev = NULL;

    return 0;
}

int cdp_neighbor_list_purge_expired_neighbors(struct cdp_neighbor_list *list, struct timespec now)
{
    struct cdp_neighbor *item;

    if(list == NULL)
    {
        printk(KERN_CRIT "cdp_neighbor_list_purge_expired_neighbors: list is NULL\n");
        return -1;
    }

    item = list->head;
    while(item != NULL)
    {
        if(cdp_neighbor_is_expired(item, now))
        {
            struct cdp_neighbor *expired_item;

            expired_item = item;
            item = item->prev;
            if(cdp_neighbor_list_remove_item(list, expired_item) < 0)
            {
                printk(KERN_CRIT "cdp_neighbor_list_purge_expired_neighbors: failed to remove item. List  is most likely corrupted now\n");
                return -1;
            }

            cdp_neighbor_delete(expired_item);
        }

        if(item != NULL)
            item = item->next;
    }

    return 0;
}
