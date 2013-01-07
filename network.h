#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stdint.h>

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

struct counting_bucket {
  volatile uint64_t value;
};

struct counting_network {
  struct network *n;
  struct counting_bucket **buckets;
};

/**
 *
 * Precondition: n >= 2 is a power of 2
 */
extern struct counting_network *
counting_network_alloc_and_init(unsigned int n);

extern void
counting_network_free(struct counting_network *cn);

extern uint64_t
counting_network_next_value(struct counting_network *cn);

#endif /* _NETWORK_H_ */
