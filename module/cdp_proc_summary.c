#include <linux/time.h>

#include "cdp_proc.h"
#include "cdp_module.h"

#include "libcdp/cdp_packet.h"
#include "libcdp/cdp_packet_parser.h"

static const char *format_capabilities_brief(const uint32_t *capabilities, char *resultBuffer)
{
    if(capabilities == NULL)
        return "<null>";
    
    resultBuffer[0] = 0;
    if(*capabilities & CdpCapabilityRouting)
        strcat(resultBuffer, " R");
    if(*capabilities & CdpCapabilityTransparentBridging)
        strcat(resultBuffer, " T");
    if(*capabilities & CdpCapabilitySourceRouteBridging)
        strcat(resultBuffer, " B");
    if(*capabilities & CdpCapabilitySwitching)
        strcat(resultBuffer, " S");
    if(*capabilities & CdpCapabilityHost)
        strcat(resultBuffer, " H");
    if(*capabilities & CdpCapabilityIGMP)
        strcat(resultBuffer, " I");
    if(*capabilities & CdpCapabilityRepeater)
        strcat(resultBuffer, " r");

    return resultBuffer;
}

int cdp_seq_summary_show(struct seq_file *seq, void *v)
{
    struct cdp_neighbor *neighbor = (struct cdp_neighbor *)v;

    if(neighbor == cdp_neighbors->head)
        seq_printf(seq, "Device ID        Local Intrfce     Holdtme    Capability  Platform  Port ID\n");

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
            char formatting_buffer[32];
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
                seq_printf(seq, "Frame: <Failed to parse packet>\n");
                return 0;
            }

            seq_printf(seq, "%s\n", (packet->device_id == NULL) ? "<device-id is null>" : packet->device_id);
            seq_printf(seq, "                 %-17s ", (neighbor->device_name == NULL) ? "<local port null>" : neighbor->device_name);
            seq_printf(seq, "%-3d ", packet->cdp_ttl - seconds_since_receive);
            seq_printf(seq, "%17s  ", format_capabilities_brief(packet->capabilities, formatting_buffer));
            seq_printf(seq, "%10s ", (packet->platform == NULL) ? "<null>" : packet->platform);
            seq_printf(seq, "%s", (packet->port_id == NULL) ? "<port id null>" : packet->port_id);
            seq_printf(seq, "\n");
            
            cdp_packet_delete(packet);
        }
    }

    return 0;
}
