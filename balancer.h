#ifndef _BALANCER_H_
#define _BALANCER_H_

#include <stdint.h>

typedef union balancer_bits {
  struct {
    uint8_t s : 1;
    uint8_t l : 1;
  } bits;
  uint8_t raw;
} balancer_bits_t;

typedef union balancer_link_or_opaque {
  struct balancer *b;
  void *opaque;
} balancer_link_or_opaque_t;

struct balancer {
  volatile balancer_bits_t d;
  balancer_link_or_opaque_t links[2];
  struct balancer *next; // for LL threading
};

/**
 * Initializes input balancer b as a leaf balancer
 */
extern void balancer_init_leaf(struct balancer *b,
                               void *op_first,
                               void *op_second);

/**
 * Initializes input balancer b as a non-leaf balancer
 */
extern void balancer_init_nonleaf(struct balancer *b,
                                  struct balancer *first,
                                  struct balancer *second);


/**
 * Traverses the network pointed to by input balancer b,
 * returning the opaque pointer reached after traversing
 * the last leaf node
 */
extern void *balancer_traverse(struct balancer *b);

#endif /* _BALANCER_H_ */