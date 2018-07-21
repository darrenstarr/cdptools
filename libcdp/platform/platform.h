#ifndef PLATFORM_H
#define PLATFORM_H

/* The following section is related to how code should compile when running in the Linux Kernel */
#ifdef __KERNEL__
#include <linux/slab.h>
#include <linux/kernel.h>

#define LOG_CRITICAL(...) printk(__VA_ARGS__)
#define LOG_ERROR(...) printk(__VA_ARGS__)
#define LOG_INFORMATIONAL(...) printk(__VA_ARGS__)
#define LOG_INFORMATIONAL(...) printk(__VA_ARGS__)
//#define LOG_DEBUG(...) printk(__VA_ARGS__)
#define LOG_DEBUG(...)
#define _P printk

#define ALLOC_NEW(AllocationType) (kmalloc(sizeof(AllocationType), GFP_ATOMIC))
#define ALLOC_NEW_ARRAY(AllocationType, Count) (kmalloc(sizeof(AllocationType) * Count, GFP_ATOMIC))
#define FREE kfree
#define FREE_ARRAY kfree
#define COPY_MEMORY(source, destination, count) memcpy(destination, source, count)
#define ZERO_BUFFER(buffer, BufferType) memset((buffer), 0, sizeof(BufferType))

#define FORMAT_OFF_T "%zd"
#define FORMAT_HEX_OFF_T "%zX"

#endif

/* The following section is related to how code should compile when running within user mode Linux */
#if defined(__linux__) && !defined(__KERNEL__)
#include <malloc.h>
#include <memory.h>
#include <stdio.h>

#define LOG_CRITICAL(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#define LOG_INFORMATIONAL(...) fprintf(stdout, __VA_ARGS__)
//#define LOG_DEBUG(...) fprintf(stdout, __VA_ARGS__)
#define LOG_DEBUG(...)
#define _P printf

#define ALLOC_NEW(AllocationType) (malloc(sizeof(AllocationType)))
#define ALLOC_NEW_ARRAY(AllocationType, Count) (malloc(sizeof(AllocationType) * Count))
#define FREE free
#define FREE_ARRAY free
#define COPY_MEMORY(source, destination, count) memcpy(destination, source, count)
#define ZERO_BUFFER(buffer, BufferType) memset((buffer), 0, sizeof(BufferType))

#define FORMAT_OFF_T "%zd"
#define FORMAT_HEX_OFF_T "%zX"
#endif

#if defined(_WIN64)
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <malloc.h>
#include <memory.h>
#include <stdio.h>

#define LOG_CRITICAL(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#define LOG_INFORMATIONAL(...) fprintf(stdout, __VA_ARGS__)
//#define LOG_DEBUG(...) fprintf(stdout, __VA_ARGS__)
#define LOG_DEBUG(...)
#define _P printf

#define ALLOC_NEW(AllocationType) (malloc(sizeof(AllocationType)))
#define ALLOC_NEW_ARRAY(AllocationType, Count) (malloc(sizeof(AllocationType) * Count))
#define FREE free
#define FREE_ARRAY free
#define COPY_MEMORY(source, destination, count) memcpy(destination, source, count)
#define ZERO_BUFFER(buffer, BufferType) memset((buffer), 0, sizeof(BufferType))

#define FORMAT_OFF_T "%d"
#define FORMAT_HEX_OFF_T "%X"
#endif

#endif
