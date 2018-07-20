#ifdef __KERNEL__
#include <linux/types.h>

#elif defined(_MSC_VER)

#if defined(_WIN64)
typedef long long ssize_t;
#endif

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#else
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#endif
