#ifndef _BITONIC_H_
#define _BITONIC_H_

/**
 * Bitonic counting networks, as described in "Counting Networks" by Aspnes,
 * Herlihy, and Shavit 93.
 */

#include "balancer.h"

struct network {
  unsigned int size;
  struct balancer **inputs;
  struct balancer **outputs;
  uint8_t *hilo;
  struct balancer *bhead; // a LL of balancers in this network
  struct balancer *btail;
};

/**
 * Allocate a network of n inputs and n outputs, with all balancer
 * pointers initialized to 0
 *
 * Precondition: n >= 1
 */
extern struct network *
network_alloc_and_init(unsigned int n);

extern void
network_clear(struct network *n);

/**
 * Frees all memory associated with the network, including
 * the network structure itself and the all the balancers.
 * Does not free opaque pointers for the leaves.
 */
extern void
network_free(struct network *n);

/**
 * Allocate memory for and create a merger[k] network.
 *
 * Precondition: k is a power of 2, k >= 2
 */
extern struct network *
merger_alloc_and_init(unsigned int k);

/**
 * Allocate memory for and create a bitonic[k] network.
 *
 * Precondition: k is a power of 2, k >= 2
 */
extern struct network *
bitonic_alloc_and_init(unsigned int k);

#endif /* _BITONIC_H_ */
