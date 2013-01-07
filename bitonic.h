#ifndef _BITONIC_H_
#define _BITONIC_H_

/**
 * Bitonic counting networks, as described in "Counting Networks" by Aspnes,
 * Herlihy, and Shavit 93.
 */

#include "network.h"

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
