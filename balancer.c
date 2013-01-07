#include "balancer.h"

void
balancer_init_leaf(struct balancer *b,
                   void *op_first,
                   void *op_second)
{
  b->d.raw = 0; // make valgrind happy
  b->d.bits.s = 0;
  b->d.bits.l = 1;
  b->links[0].opaque = op_first;
  b->links[1].opaque = op_second;
  b->next = 0;
}

void
balancer_init_nonleaf(struct balancer *b,
                      struct balancer *first,
                      struct balancer *second)
{
  b->d.raw = 0; // make valgrind happy
  b->d.bits.s = 0;
  b->d.bits.l = 0;
  b->links[0].b = first;
  b->links[1].b = second;
  b->next = 0;
}

void *
balancer_traverse(struct balancer *b)
{
  for (;;) {
    balancer_bits_t vold = b->d;
    balancer_bits_t vnew = vold;
    vnew.bits.s = (vold.bits.s + 1) % 2;
    if (!__sync_bool_compare_and_swap(&b->d.raw, vold.raw, vnew.raw))
      continue;
    if (vold.bits.l)
      return b->links[vold.bits.s].opaque;
    b = b->links[vold.bits.s].b;
  }
}
