#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "network.h"

static void
test_cn(struct counting_network *cn)
{
  unsigned int i;
  uint64_t v;
  for (i = 0; i < 1000; i++) {
    v = counting_network_next_value(cn);
    assert(v == i);
  }
}

int
main(int argc, char **argv)
{
  struct counting_network *cn;
  unsigned int i, p;
  p = 2;
  for (i = 0; i < 8; i++, p *= 2) {
    cn = counting_network_alloc_and_init(p);
    printf("testing bitonic[%d]...", p);
    test_cn(cn);
    printf("passed!\n");
    counting_network_free(cn);
  }
  return 0;
}
