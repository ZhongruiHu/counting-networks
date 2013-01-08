#ifndef _MACROS_H_
#define _MACROS_H_

#define CACHELINE_SIZE 64 // XXX: don't assume x86
#define CACHE_ALIGNED __attribute__((aligned(CACHELINE_SIZE)))
#define PACKED_CACHE_ALIGNED __attribute__((packed, aligned(CACHELINE_SIZE)))

#endif /* _MACROS_H_ */
