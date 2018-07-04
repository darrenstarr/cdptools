#include <linux/time.h>

#include "cdp_proc.h"
#include "cdp_module.h"

#include "../libcdp/cdp_packet.h"
#include "../libcdp/cdp_packet_parser.h"

/** Print to a sequential file an escaped json string. Based on seq_escape
  *  @param seq the sequential file to stream to
  *  @param source the source string to escape and output
  *  @return 0 or a negative value on error.
  */
static int json_escape(struct seq_file *seq, const char *source)
{
	char *end = seq->buf + seq->size;
    char *out;
	char c;

    for (out = seq->buf + seq->count; (c = *source) != '\0' && out < end; source++)
    {
        switch(c)
        {
            case '\n':
                if (out + 2 < end)
                {
                    *out++ = '\\';
                    *out++ = 'n';
                    continue;
                }
                break;

            case '\r':
                continue;

            case '\t':
                if (out + 2 < end)
                {
                    *out++ = '\\';
                    *out++ = 't';
                    continue;
                }
                break;

            case '"':
                if (out + 2 < end)
                {
                    *out++ = '\\';
                    *out++ = c;
                    continue;
                }
                break;

            default:
                *out++ = c;
                continue;
        }

		seq->count = seq->size; // Set overflow
		return -1;
    }
	
    seq->count = out - seq->buf;

    return 0;
}

/** Output a JSON string value to a sequential file stream
  *  @param seq the sequential file stream
  *  @param value the value to output
  *  @param name the JSON variable name
  *  @return 0 on success or a negative value on error.
  */
static int json_string_out(struct seq_file *seq, const char *value, const char *name)
{
    seq_puts(seq, "    \"");
    seq_puts(seq, name);
    seq_puts(seq, "\": ");
    if(value == NULL)
        seq_puts(seq, "null");
    else
    {
        seq_putc(seq, '"');
        json_escape(seq, value);
        seq_putc(seq, '"');
    }
    
    seq_puts(seq, ",\n");

    return 0;
}

/** Output a JSON integer value to a sequential file stream
  *  @param seq the sequential file stream
  *  @param value the value to output
  *  @param name the JSON variable name
  *  @return 0 on success or a negative value on error.
  */
static int json_int_out(struct seq_file *seq, int value, const char *name)
{
    seq_puts(seq, "    ");
    seq_printf(seq, "\"%s\": %d,\n", name, value);

    return 0;
}

/** Output an optional JSON integer value to a sequential file stream.
  *  @param seq the sequential file stream.
  *  @param value the value to output.
  *  @param name the JSON variable name.
  *  @param force true if the variable should be present as null even if the value is null.
  *  @return 0 on success or a negative value on error.
  */
static int json_u8px_out(struct seq_file *seq, const uint8_t *value, const char *name, bool force)
{
    if(value == NULL)
    {
        if(force)
            seq_printf(seq, "    \"%s\": null,\n", name);
    }
    else
        seq_printf(seq, "    \"%s\": 0x%02x,\n", name, *value);

    return 0;
}

/** Output an optional JSON integer value to a sequential file stream.
  *  @param seq the sequential file stream.
  *  @param value the value to output.
  *  @param name the JSON variable name.
  *  @param force true if the variable should be present as null even if the value is null.
  *  @return 0 on success or a negative value on error.
  */
static int json_u16p_out(struct seq_file *seq, const uint16_t *value, const char *name, bool force)
{
    if(value == NULL)
    {
        if(force)
            seq_printf(seq, "    \"%s\": null,\n", name);
    }
    else
        seq_printf(seq, "    \"%s\": %d,\n", name, *value);

    return 0;
}

/** Output an network duplex string value to a sequential file stream.
  *  @param seq the sequential file stream.
  *  @param value the value to output.
  *  @param name the JSON variable name.
  *  @return 0 on success or a negative value on error.
  */
static int json_duplex_out(struct seq_file *seq, ECdpNetworkDuplex value, const char *name)
{
    switch(value)
    {
        case DuplexFull:
            seq_printf(seq, "    \"%s\": \"full\",\n", name);
            break;
        case DuplexHalf:
            seq_printf(seq, "    \"%s\": \"half\",\n", name);
            break;
        default:
            seq_printf(seq, "    \"%s\": null,\n", name);
            break;
    }

    return 0;
}

/** Output an optional array of addresses as JSON to a sequential file stream.
  *  @param seq the sequential file stream.
  *  @param array the array to output
  *  @param name the JSON variable name.
  *  @param force true if the variable should be present as null even if the value is null.
  *  @param last true if this is the last item in the list and should not have a trailing comma
  *  @return 0 on success or a negative value on error.
  */
static int json_address_array_out(struct seq_file *seq, const struct ip_address_array *array, const char *name, bool force, bool last)
{
    if(array == NULL)
    {
        if(force)
        {
            if(last)
                seq_printf(seq, "    \"%s\": null\n", name);
            else
                seq_printf(seq, "    \"%s\": null,\n", name);
        }
    }
    else
    {
        size_t i;

        seq_printf(seq, "    \"%s\": [\n", name);
        for(i=0; i<array->count; i++)
        {
            seq_puts(seq, "      \"");

            seq_print_sockaddr(seq, array->addresses[i]);

            if(i < (array->count - 1))
                seq_puts(seq, "\",\n");
            else
                seq_puts(seq, "\"\n");
        }

        if(last)
            seq_puts(seq, "    ]\n");
        else
            seq_puts(seq, "    ],\n");
    }

    return 0;
}

/** Output an optional array of IP prefixes as JSON to a sequential file stream.
  *  @param seq the sequential file stream.
  *  @param array the array to output
  *  @param name the JSON variable name.
  *  @param force true if the variable should be present as null even if the value is null.
  *  @param last true if this is the last item in the list and should not have a trailing comma
  *  @return 0 on success or a negative value on error.
  */
static int json_prefix_array_out(struct seq_file *seq, const struct ip_prefix_array *array, const char *name, bool force, bool last)
{
    if(array == NULL)
    {
        if(force)
        {
            if(last)
                seq_printf(seq, "    \"%s\": null\n", name);
            else
                seq_printf(seq, "    \"%s\": null,\n", name);
        }
    }
    else
    {
        size_t i;

        seq_printf(seq, "    \"%s\": [\n", name);
        for(i=0; i<array->count; i++)
        {
            seq_puts(seq, "      \"");

            seq_print_sockaddr(seq, array->prefixes[i]->network);
            seq_printf(seq, "/%d", array->prefixes[i]->length);

            if(i < (array->count - 1))
                seq_puts(seq, "\",\n");
            else
                seq_puts(seq, "\"\n");
        }

        if(last)
            seq_puts(seq, "    ]\n");
        else
            seq_puts(seq, "    ],\n");
    }

    return 0;
}

/** Output a boolean value based on a bit mask as JSON to a sequential file stream
  *  @param seq the sequential file stream.
  *  @param value the value to test against
  *  @param mask the bit mask to test the bits of
  *  @param name the JSON variable name.
  *  @param last true if this is the last item in the list and should not have a trailing comma
  *  @return 0 on success or a negative value on error.
  */
static int json_cdp_capability_out(struct seq_file *seq, uint32_t value, uint32_t mask, const char *name, bool last)
{
    seq_puts(seq, "      \"");
    seq_puts(seq, name);
    seq_puts(seq, "\": ");
    if((value & mask) == mask)
        seq_puts(seq, "true");
    else
        seq_puts(seq, "false");

    if(last)
        seq_putc(seq, ',');

    seq_putc(seq, '\n');

    return 0;
}

/** Output CDP capability values as individual JSON booleans to a sequential file stream.
  *  @param seq the sequential file stream.
  *  @param value the capabilities value to output.
  *  @param name the JSON variable name.
  *  @return 0 on success or a negative value on error.
  */
static int json_cdp_capabilities_out(struct seq_file *seq, const uint32_t *value, const char *name)
{
    if(value == NULL)
        seq_printf(seq, "    \"%s\": null,\n", name);
    else
    {
        seq_printf(seq, "    \"%s\": {\n", name);
        json_cdp_capability_out(seq, *value, CdpCapabilityRouting, "routing", false);
        json_cdp_capability_out(seq, *value, CdpCapabilityTransparentBridging, "transparentBridging", false);
        json_cdp_capability_out(seq, *value, CdpCapabilitySourceRouteBridging, "sourceRouteBridging", false);
        json_cdp_capability_out(seq, *value, CdpCapabilitySwitching, "switching", false);
        json_cdp_capability_out(seq, *value, CdpCapabilityHost, "host", false);
        json_cdp_capability_out(seq, *value, CdpCapabilityIGMP, "igmp", false);
        json_cdp_capability_out(seq, *value, CdpCapabilityRepeater, "repeater", true);
        seq_puts(seq, "    ],\n");
    }

    return 0;
}

/** Output the CDP Power Over Ethernet availability structure in JSON format to a sequential file stream
  *  @param seq the sequential file stream.
  *  @param value the value to output.
  *  @param name the JSON variable name.
  *  @param force true if the variable should be present as null even if the value is null.
  *  @return 0 on success or a negative value on error.
  */
static int json_cdp_poe_availability_out(struct seq_file *seq, const struct power_over_ethernet_availability *value, const char *name, bool force)
{
    if(value == NULL)
    {
        if(force)
            seq_printf(seq, "    \"%s\": null,\n", name);
    }
    else
    {
        seq_printf(seq, "    \"%s\": {\n", name);
        seq_printf(seq, "      \"requestId\": %d,\n", value->request_id);
        seq_printf(seq, "      \"managementId\": %d,\n", value->management_id);
        seq_printf(seq, "      \"availableMilliwatts\": %u,\n", value->availableMilliwatts);
        seq_printf(seq, "      \"powerManagementLevel\": %d\n", value->powerManagementLevel);
        seq_printf(seq, "    },\n");
    }

    return 0;
}

/** Output the CDP Cisco cluster management protocol structure in JSON format to a sequential file stream
  *  @param seq the sequential file stream.
  *  @param value the value to output.
  *  @param name the JSON variable name.
  *  @param force true if the variable should be present as null even if the value is null.
  *  @return 0 on success or a negative value on error.
  */
static int json_cdp_cluster_management_protocol_out(struct seq_file *seq, const struct cisco_cluster_management_protocol *value, const char *name, bool force)
{
    if(value == NULL)
    {
        if(force)
            seq_printf(seq, "    \"%s\": null,\n", name);
    }
    else
    {
        seq_printf(seq, "    \"%s\": {\n", name);
        seq_printf(seq, "      \"oui\": 0x%06X,\n", value->oui);
        seq_printf(seq, "      \"protocolId\": 0x%04X,\n", value->protocol_id);
        seq_puts(seq, "      \"clusterMasterIp\": ");
        seq_print_sockaddr(seq, (struct sockaddr *)&(value->cluster_master_ip));
        seq_puts(seq, ",\n");
        seq_puts(seq, "      \"clusterNetmask\": ");
        seq_print_sockaddr(seq, (struct sockaddr *)&(value->netmask));
        seq_puts(seq, ",\n");
        seq_printf(seq, "      \"version\": 0x%04X,\n", value->version);
        seq_printf(seq, "      \"status\": 0x%02X,\n", value->status);
        seq_printf(seq, "      \"clusterCommanderMac\": \"%02X:%02X:%02X:%02X:%02X:%02X\",\n", 
            value->cluster_commander_mac[0],
            value->cluster_commander_mac[1],
            value->cluster_commander_mac[2],
            value->cluster_commander_mac[3],
            value->cluster_commander_mac[4],
            value->cluster_commander_mac[5]
        );
        seq_printf(seq, "      \"managementVlan\": %d\n", value->management_vlan);
        seq_printf(seq, "    },\n");
    }

    return 0;
}

int cdp_seq_json_show(struct seq_file *seq, void *v)
{
    struct cdp_neighbor *neighbor = (struct cdp_neighbor *)v;

    if(neighbor == cdp_neighbors->head)
        seq_printf(seq, "\"cdpNeighbors\": [\n");

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
                //seq_printf(seq, "Frame: <Failed to parse packet>\n");
                return 0;
            }

            seq_puts(seq, "  {\n");
            json_string_out(seq, neighbor->device_name, "localInterface");
            json_int_out(seq, packet->cdp_proto_ver, "cdpVersion");
            json_int_out(seq, packet->cdp_ttl, "holdTime");
            json_int_out(seq, packet->cdp_ttl - seconds_since_receive, "holdTimeRemaining");
            json_string_out(seq, packet->device_id, "deviceId");
            json_address_array_out(seq, packet->addresses, "addresses", false, false);
            json_string_out(seq, packet->port_id, "portId");
            json_cdp_capabilities_out(seq, packet->capabilities, "capabilities");
            json_string_out(seq, packet->software_version, "softwareVersion");
            json_string_out(seq, packet->platform, "platform");
            json_cdp_cluster_management_protocol_out(seq, packet->cluster_management_protocol, "clusterManagement", false);
            json_prefix_array_out(seq, packet->odr_prefixes, "odrPrefixes", false, false);
            json_string_out(seq, packet->vtp_management_domain, "vtpDomain");
            json_duplex_out(seq, packet->duplex, "duplex");
            json_u16p_out(seq, packet->native_vlan, "nativeVlan", false);
            json_u8px_out(seq, packet->trust_bitmap, "trustBitmap", false);
            json_u8px_out(seq, packet->untrusted_port_cos, "untrustedPortCoS", false);
            json_cdp_poe_availability_out(seq, packet->poe_availability, "poeAvailability", false);
            json_string_out(seq, packet->startup_native_vlan, "startupNativeVlan");
            json_address_array_out(seq, packet->management_addresses, "managementAddresses", true, true);

            if(neighbor == cdp_neighbors->tail)
                seq_puts(seq, "  }\n");
            else
                seq_puts(seq, "  },\n");
            
            cdp_packet_delete(packet);
        }
    }

    if(neighbor == cdp_neighbors->tail)
        seq_printf(seq, "]\n");

    return 0;
}
