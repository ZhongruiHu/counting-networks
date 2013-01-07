#include "bitonic.h"

int
main(int argc, char **argv)
{
  struct network *b2, *b4, *b8;
  b2 = bitonic_alloc_and_init(2);
  b4 = bitonic_alloc_and_init(4);
  b8 = bitonic_alloc_and_init(8);

  network_free(b2);
  network_free(b4);
  network_free(b8);
  return 0;
}
