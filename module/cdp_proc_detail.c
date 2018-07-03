#include <linux/in.h>
#include <linux/in6.h>
#include <linux/time.h>

#include "cdp_module.h"
#include "cdp_proc.h"

#include "libcdp/cdp_packet.h"
#include "libcdp/cdp_packet_parser.h"

/** Prints the contents of a socket address if the format is known and understood
  *  @param seq the sequential file handle to print to
  *  @param address the address to print
  */
static void seq_print_sockaddr(struct seq_file *seq, const struct sockaddr *address)
{
	if (address->sa_family == AF_INET)
	{
		uint32_t n;
		const struct sockaddr_in *address4 = (const struct sockaddr_in *)address;
		
		n = address4->sin_addr.s_addr;
		seq_printf(seq, "%d.%d.%d.%d", (n & 0xFF), ((n >> 8) & 0xFF), ((n >> 16) & 0xFF), ((n >> 24) & 0xFF));
	}
	else if (address->sa_family == AF_INET6)
	{
		int i;
		const struct sockaddr_in6 *address6 = (const struct sockaddr_in6 *)address;
        int hextets[8];
        int longestIndex = -1;
        int longest = 0;

        /* Convert the address into hextets */
		for (i = 0; i < 8; i++)
        {
            hextets[i] =
                (((int)address6->sin6_addr.in6_u.u6_addr8[i << 1]) << 8) |
                ((int)address6->sin6_addr.in6_u.u6_addr8[(i << 1) + 1]);
        }

        /* Identify the longest run of zeros in the list of hextets */
        /* TODO: optimize searching for runs. */
        for (i = 0; (i + longest) < 8; i++)
        {
            int count = 0;
            while (((count + i) < 8) && hextets[i + count] == 0)
                count++;

            if (count > longest)
            {
                longest = count;
                longestIndex = i;
            }
        }

        /* Print the output */
        if(longest == 8)
            seq_puts(seq, "::");
        else if(longest == 0)
        {
            seq_printf(
                seq,
                "%X:%X:%X:%X:%X:%X:%X:%X",
                hextets[0],
                hextets[1],
                hextets[2],
                hextets[3],
                hextets[4],
                hextets[5],
                hextets[6],
                hextets[7]
            );
        }
        else
        {
            for(i=0; i<longestIndex; i++)
                seq_printf(
                    seq,
                    "%X:",
                    hextets[i]
                );

            for(i=longestIndex + longest; i<8; i++)
                seq_printf(
                    seq,
                    ":%X",
                    hextets[i]
                );
        }
	}
	else
	{
		seq_puts(seq, "<unknown format>");
	}
}

int cdp_seq_detail_show(struct seq_file *seq, void *v)
{
    struct cdp_neighbor *neighbor = (struct cdp_neighbor *)v;

    //Device ID        Local Intrfce     Holdtme    Capability  Platform  Port ID

    seq_printf(seq, "Detail\n");
    if(neighbor == NULL)
    {
        seq_printf(seq, "Buffer appears to be null\n");
    }
    else 
    {
        if(neighbor->frame_buffer == NULL)
            seq_printf(seq, "Frame buffer: <null>\n");
        else if(neighbor->frame_buffer_size == 0)
            seq_printf(seq, "Frame buffer size: 0\n");
        else if(neighbor->frame_buffer_length == 0)
            seq_printf(seq, "Frame buffer length: 0\n");
        else
        {
            struct cdp_packet *packet;
            struct stream_reader *reader;
            int parse_result;
            struct timespec now;
            int seconds_since_receive;
            size_t i;
            
            getnstimeofday(&now);

            seconds_since_receive = now.tv_sec - neighbor->received_at.tv_sec;

            reader = stream_reader_new(neighbor->frame_buffer, neighbor->frame_buffer_length);
            if(reader == NULL)
            {
                printk("cdp_seq_summary_show: failed to allocate stream reader\n");                
                return 0;
            }

            parse_result = cdp_parse_packet(reader, &packet);

            stream_reader_delete(reader);

            if(packet == NULL || parse_result < 0)
            {
                seq_printf(seq, "Frame: <Failed to parse packet>\n");
                return 0;
            }

            seq_printf(seq, "Device ID: %s\n", (packet->device_id == NULL) ? "<device-id is null>" : packet->device_id);
            seq_printf(seq, "Entry address(es):\n");
            if(packet->addresses != NULL)
            {
                for(i=0; i<packet->addresses->count; i++)
                {
                    if(packet->addresses->addresses[i] == NULL)
                        seq_printf(seq, "  <address is null>\n");
                    else
                    {
                        if(packet->addresses->addresses[i]->sa_family == AF_INET)
                            seq_puts(seq, "  IP address: ");
                        else if(packet->addresses->addresses[i]->sa_family == AF_INET6)
                            seq_puts(seq, "  IPv6 address: ");
                        else
                            seq_puts(seq, "  ");

                        seq_print_sockaddr(seq, packet->addresses->addresses[i]);
                        seq_puts(seq, "\n");                        
                    }
                }
            }

            seq_printf(seq, "Platform: %s, ", (packet->platform == NULL) ? "<not sent>" : packet->platform);

            seq_puts(seq, "Capabilities: ");
            if(packet->capabilities == NULL)
            {
                seq_puts(seq, "<not sent>");
            }
            else
            {
                if(*packet->capabilities & CdpCapabilityRouting)
                    seq_puts(seq, "Router ");
                if(*packet->capabilities & CdpCapabilityTransparentBridging)
                    seq_puts(seq, "Transparent-Bridge ");
                if(*packet->capabilities & CdpCapabilitySourceRouteBridging)
                    seq_puts(seq, "Source-Route-Bridge ");
                if(*packet->capabilities & CdpCapabilitySwitching)
                    seq_puts(seq, "Switch ");
                if(*packet->capabilities & CdpCapabilityHost)
                    seq_puts(seq, "Host ");
                if(*packet->capabilities & CdpCapabilityIGMP)
                    seq_puts(seq, "IGMP ");
                if(*packet->capabilities & CdpCapabilityRepeater)
                    seq_puts(seq, "Repeater ");
            }
            seq_puts(seq, "\n");

            seq_printf(
                seq,
                "Interface: %s,  Port-Id (outgoing port): %s\n",
                (neighbor->device_name == NULL) ? "<local port null>" : neighbor->device_name,
                (packet->port_id == NULL) ? "<port id null>" : packet->port_id
            );

            seq_printf(seq, "Holdtime : %3d sec\n\n", packet->cdp_ttl - seconds_since_receive);

            seq_printf(seq, "Version :\n%s\n\n", (packet->software_version == NULL) ? "<not sent>" : packet->software_version);

            seq_printf(seq, "advertisement version: %d\n", packet->cdp_proto_ver);

            if(packet->native_vlan != NULL)
                seq_printf(seq, "Native VLAN: %d\n", *packet->native_vlan);

            switch(packet->duplex)
            {
                case DuplexHalf:
                    seq_puts(seq, "Duplex: half\n");
                    break;

                case DuplexFull:
                    seq_puts(seq, "Duplex: full\n");
                    break;

                default:
                    seq_puts(seq, "Duplex: <invalid format>\n");
                    break;
            }

            if(packet->poe_availability != NULL)
            {
                seq_printf(
                    seq,
                    "Power Available TLV:\n\nPower request id: %d, Power management id: %d, Power available: %umw, Power management level: %d\n",
                    packet->poe_availability->request_id,
                    packet->poe_availability->management_id,
                    packet->poe_availability->availableMilliwatts,
                    packet->poe_availability->powerManagementLevel
                );
            }

            if(packet->management_addresses != NULL)
            {
                seq_printf(seq, "Management address(es):\n");
                for(i=0; i<packet->management_addresses->count; i++)
                {
                    if(packet->management_addresses->addresses[i] == NULL)
                        seq_printf(seq, "  <address is null>\n");
                    else
                    {
                        if(packet->management_addresses->addresses[i]->sa_family == AF_INET)
                            seq_puts(seq, "  IP address: ");
                        else if(packet->management_addresses->addresses[i]->sa_family == AF_INET6)
                            seq_puts(seq, "  IPv6 address: ");
                        else
                            seq_puts(seq, "  ");

                        seq_print_sockaddr(seq, packet->management_addresses->addresses[i]);
                        seq_puts(seq, "\n");                        
                    }
                }
            }

            cdp_packet_delete(packet);
        }
    }

    return 0;
}
