#include <linux/in.h>
#include <linux/in6.h>

#include "cdp_proc.h"

int seq_print_sockaddr(struct seq_file *seq, const struct sockaddr *address)
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

    return 0;
}
