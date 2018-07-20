#ifdef __KERNEL__
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/socket.h>

#define IPv6Octet(addr, index) ((addr)->sin6_addr.in6_u.u6_addr8[(index)])
#define IPv6Octets(addr) ((addr)->sin6_addr.in6_u.u6_addr8)

#elif defined(_MSC_VER)

#include <WinSock2.h>
//#include <ws2def.h>
#include <ws2ipdef.h>

#define IPv6Octet(addr, index) ((addr)->sin6_addr.u.Byte[(index)])
#define IPv6Octets(addr) ((addr)->sin6_addr.u.Byte)

#else
#include <netinet/in.h>
#include <sys/socket.h>

#define IPv6Octet(addr, index) ((addr)->sin6_addr.__in6_u.__u6_addr8[(index)])
#define IPv6Octets(addr) ((addr)->sin6_addr.__in6_u.__u6_addr8)
#endif
