#ifdef __KERNEL__
#include <linux/utsname.h>
#define UTS_STRUCT struct new_utsname *
#define UTS_RELEASE_LENGTH __NEW_UTS_LEN
#define UTS_VERSION_LENGTH __NEW_UTS_LEN
#define UTS_NODE_NAME_LENGTH __NEW_UTS_LEN
#define UTS_DOMAIN_NAME_LENGTH __NEW_UTS_LEN

#define UTS_RELEASE(uts) uts->release
#define UTS_VERSION(uts) uts->version
#define UTS_NODE_NAME(uts) uts->nodename
#define UTS_DOMAIN_NAME(uts) uts->domainname

#define GET_UTSNAME_IF(uts) uts = utsname(); if(uts == NULL)

#else
#include <sys/utsname.h>
#define UTS_STRUCT struct utsname
#define UTS_RELEASE_LENGTH _UTSNAME_RELEASE_LENGTH
#define UTS_VERSION_LENGTH _UTSNAME_VERSION_LENGTH
#define UTS_NODE_NAME_LENGTH _UTSNAME_NODENAME_LENGTH
#define UTS_DOMAIN_NAME_LENGTH _UTSNAME_DOMAIN_LENGTH

#define UTS_RELEASE(uts) uts.release
#define UTS_VERSION(uts) uts.version
#define UTS_NODE_NAME(uts) uts.nodename
#define UTS_DOMAIN_NAME(uts) uts.__domainname

#define GET_UTSNAME_IF(uts) if(uname(&uts) < 0)
#endif
