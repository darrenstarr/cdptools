#ifndef CDP_PACKET_PARSER_H
#define CDP_PACKET_PARSER_H

#include "cdp_packet.h"
#include "stream_reader.h"

int cdp_parse_packet(struct stream_reader *reader, struct cdp_packet **neighbor);

#endif
