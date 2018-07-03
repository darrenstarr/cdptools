#ifndef PLATFORM_H
#define PLATFORM_H

/*
#include <malloc.h>
#include <memory.h>
#include <stdio.h>

#define LOG_CRITICAL(...) fprintf(stderr, __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#define LOG_INFORMATIONAL(...) fprintf(stdout, __VA_ARGS__)
#define LOG_DEBUG(...) fprintf(stdout, __VA_ARGS__)
#define _P printf

#define ALLOC_NEW(AllocationType) (malloc(sizeof(AllocationType)))
#define ALLOC_NEW_ARRAY(AllocationType, Count) (malloc(sizeof(AllocationType) * Count))
#define FREE free
#define FREE_ARRAY free
#define COPY_MEMORY(source, destination, count) memcpy(destination, source, count)
#define ZERO_BUFFER(buffer, BufferType) memset((buffer), 0, sizeof(BufferType))

*/

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

#endif
