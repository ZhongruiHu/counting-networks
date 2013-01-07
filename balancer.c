#include "balancer.h"

void
balancer_init_leaf(struct balancer *b,
                   void *op_first,
                   void *op_second)
{
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
    vnew.bits.s = vold.bits.s ? 0 : 1;
    if (!__sync_bool_compare_and_swap(&b->d.raw, vold.raw, vnew.raw))
      continue;
    if (vold.bits.l)
      return b->links[vnew.bits.s].opaque;
    b = b->links[vnew.bits.s].b;
  }
}
