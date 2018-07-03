#include "cdp_module.h"
#include "cdp_receive.h"

int cdp_receive(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev)
{
    struct cdp_neighbor *neighbor;

    if(dev->type == ARPHRD_ETHER)
    {
        struct ethhdr *mac_header = eth_hdr(skb);
        unsigned long flags;

        /*
        printk(
            "Received from : %02X:%02X:%02X:%02X:%02X:%02X on interface %s with %d bytes, device type Ethernet\n",
            mac_header->h_source[0],
            mac_header->h_source[1],
            mac_header->h_source[2],
            mac_header->h_source[3],
            mac_header->h_source[4],
            mac_header->h_source[5],
            dev->name,
            (int)(skb_tail_pointer(skb) - skb->data)
            );
        */

        write_lock_irqsave(&cdp_neighbors_rw_lock, flags);
        neighbor = cdp_neighbor_list_get_or_create_by_identity(
            cdp_neighbors,
            dev->type,
            dev->name,
            mac_header->h_source,
            ETH_ALEN);

        if(neighbor == NULL)
        {
            printk(KERN_CRIT "Failed to find or create a new CDP neighbor entry record\n");
        }
        else
        {
            struct timespec now;
            getnstimeofday(&now);

            cdp_neighbor_set_received_at(neighbor, now);
            cdp_neighbor_set_frame_buffer(neighbor, skb->data, (size_t)(skb_tail_pointer(skb) - skb->data));
        }

        write_unlock_irqrestore(&cdp_neighbors_rw_lock, flags);
    }
    /*
    else
    {
        printk(
            "Received from interface %s with %d bytes, device type %d (unknown, ignoring)\n",
            dev->name,
            (int)(skb_tail_pointer(skb) - skb->data),
            dev->type
            );

    }
    */

    return 0;
}
