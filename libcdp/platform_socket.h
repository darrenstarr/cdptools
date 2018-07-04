#ifdef __KERNEL__
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/socket.h>

#define IPv6Octet(addr, index) ((addr)->sin6_addr.in6_u.u6_addr8[(index)])
#define IPv6Octets(addr) ((addr)->sin6_addr.in6_u.u6_addr8)

#else
#include <netinet/in.h>
#include <sys/socket.h>

#define IPv6Octet(addr, index) ((addr)->sin6_addr.__in6_u.__u6_addr8[(index)])
#define IPv6Octets(addr) ((addr)->sin6_addr.__in6_u.__u6_addr8)
#endif
