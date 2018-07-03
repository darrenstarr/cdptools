#ifndef CDP_RECEIVE_H
#define CDP_RECEIVE_H

#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/psnap.h>

/** Called by PSNAP to process incoming CDP packets received on any interface
  *  This function stores the packet data and the information necessary to identify who sent it.
  *  @param skb The kernel packet buffer containing all the headers and data that's been parse so far.
  *  @param dev The network device upon which the frame was received.
  *  @param pt The packet type
  *  @param orig_dev TODO: Not really sure.
  *  @return 0 on success or a negative value on error
  */
int cdp_receive(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, struct net_device *orig_dev);

#endif
