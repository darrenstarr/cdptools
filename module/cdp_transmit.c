#include "cdp_module.h"
#include "cdp_transmit.h"
#include "../libcdp/cdp_packet.h"
#include "../libcdp/cdp_software_version_string.h"

#include <linux/if_arp.h>
#include <linux/inetdevice.h>

/** This is the length of a standard 802.2 frame header */
static const size_t ethernet_header_length = 14;

/** This is the length of a standard 802.2 + SNAP header */
static const size_t snap_header_length = 8;

static ssize_t get_ip_address_list_from_net_device(const struct net_device *network_device, struct ip_address_array **result)
{
    size_t address_count = 0;

    *result = NULL;

    if (network_device->ip_ptr != NULL)
    {
        struct in_ifaddr *current_address = network_device->ip_ptr->ifa_list;
        while(current_address != NULL)
        {
            address_count++;
            current_address = current_address->ifa_next;
        }
    }

    if(address_count == 0)
    {
        printk(KERN_INFO "Failed to enumerate IP addresses on this interface\n");
		return -1;
    }

    *result = ip_address_array_new(address_count);
    if(*result == NULL)
	{
		printk(KERN_CRIT "Failed to provision storage for addresses\n");
		return -1;
	}

    if (network_device->ip_ptr != NULL)
    {
        off_t index = 0;
        struct in_ifaddr *current_address = network_device->ip_ptr->ifa_list;

        while(current_address != NULL)
        {
            if(ip_address_array_set_into_ipv4_uint32(*result, index, current_address->ifa_local) < 0)
            {
                printk(KERN_CRIT "Failed to set address\n");
                ip_address_array_delete(*result);
                *result = NULL;
                return -1;
            }

            current_address = current_address->ifa_next;
            index++;
        }
    }

    return (ssize_t)address_count;
}

static int cdp_transmit_packet(struct net_device *network_device)
{
    struct sk_buff *skb;
    struct ip_address_array *addresses;
    size_t len = 1536;
    ssize_t consumed;
    uint8_t *buffer;
    int rc;    

    printk(KERN_INFO "cdp: Transmit on interface %s\n", network_device->name);

    skb = netdev_alloc_skb(network_device, len);
    if(skb == NULL)
    {
        printk(KERN_CRIT "cdp_transmit_packet: failed to allocated a packet buffer for transmission\n");
        return -1;
    }

    skb_reserve(skb, ethernet_header_length + snap_header_length);

    if(get_ip_address_list_from_net_device(network_device, &addresses) < 0)
    {
        printk(KERN_INFO "cdp_transmit_packet: there seems to be no IP addresses on this interface. skipping\n");
        kfree_skb(skb);
        return -1;
    }

    buffer = skb_put(skb, 0);
    consumed = cdp_create_packet(
        network_device->name,
        cdp_device_id_string,
        cdp_platform_string,
        cdp_software_version_string,
        addresses,
        buffer, 
        len
    );

    ip_address_array_delete(addresses);

    if(consumed < 1)
    {
        printk(KERN_CRIT "cdp_transmit_packet: failed to generate frame\n");
        kfree_skb(skb);
        return -1;
    }

    skb_put(skb, consumed);

    rc = cdp_snap_datalink_protocol->request(cdp_snap_datalink_protocol, skb, cdp_multicast_address);

    if(rc < 0)
    {
        printk(KERN_CRIT "cdp_packet_transmit: failed to transmit frame\n");
        return -1;
    }

    return 0;
}

int cdp_transmit_packets(void)
{
    struct net_device *dev;

    read_lock(&dev_base_lock);

    dev = first_net_device(&init_net);
    while (dev) {
        int rc;

        if(netif_carrier_ok(dev))
        {
            if(dev->type == ARPHRD_ETHER)
                rc = cdp_transmit_packet(dev);

            if(rc < 0)
            {
                printk(KERN_CRIT "cdp: Failed to transmit frame on %s\n", dev->name);
                return -1;
            }
        }

        dev = next_net_device(dev);
    }

    read_unlock(&dev_base_lock);

    return 0;
}
