#ifndef CDP_TRANSMIT_H
#define CDP_TRANSMIT_H

/** Transmits a CDP packet to each interface recognized by CDP.
  *  @return 0 on success, -1 on failure.
  */
int cdp_transmit_packets(void);

#endif
