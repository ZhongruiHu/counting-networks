#include <stdio.h>

#include "network.h"

int
main(int argc, char **argv)
{
  struct counting_network *cn;
  unsigned int i;
  uint64_t v;
  cn = counting_network_alloc_and_init(32);
  for (i = 0; i < 25; i++) {
    v = counting_network_next_value(cn);
    printf("v: %llu\n", (unsigned long long) v);
  }
  counting_network_free(cn);
  return 0;
}
