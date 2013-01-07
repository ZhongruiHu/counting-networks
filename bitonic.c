#include "bitonic.h"

#include <stdlib.h>

struct network *
network_alloc_and_init(unsigned int size)
{
  struct network *n;
  n = (struct network *) malloc(sizeof(*n));
  if (!n)
    return 0;
  n->size = size;
  n->inputs = 0;
  n->outputs = 0;
  n->hilo = 0;
  n->bhead = n->btail = 0;
  n->inputs = (struct balancer **) malloc(sizeof(n->inputs[0]) * size);
  if (!n->inputs)
    goto free_n;
  n->outputs = (struct balancer **) malloc(sizeof(n->outputs[0]) * size);
  if (!n->outputs)
    goto free_n;
  n->hilo = (uint8_t *) malloc(sizeof(n->hilo[0]) * size);
  if (!n->hilo)
    goto free_n;
  return n;
free_n:
  if (n->inputs)
    free(n->inputs);
  if (n->outputs)
    free(n->outputs);
  if (n->hilo)
    free(n->hilo);
  return 0;
}

void
network_clear(struct network *n)
{
  unsigned int i;
  for (i = 0; i < n->size; i++) {
    n->inputs[i] = 0;
    n->outputs[i] = 0;
    n->hilo[i] = 0;
  }
  n->bhead = n->btail = 0;
}

void
network_free(struct network *n)
{
  if (!n)
    return;
  struct balancer *b, *b0;
  b = n->bhead;
  while (b) {
    b0 = b->next;
    free(b);
    b = b0;
  }
  free(n->inputs);
  free(n->outputs);
  free(n->hilo);
}

struct network *
merger_alloc_and_init(unsigned int k)
{
  struct network *n, *n0, *n1;
  struct balancer *b;
  unsigned int i;
  n = network_alloc_and_init(k);
  n0 = n1 = 0;
  if (!n)
    return 0;
  if (k == 2) {
    // merger[2] is defined as single balancer
    b = (struct balancer *) malloc(sizeof(*b));
    if (!b)
      goto free_n;
    b->next = 0;
    n->bhead = n->btail = b;
    n->inputs[0] = n->inputs[1] = b;
    n->outputs[0] = n->outputs[1] = b;
    n->hilo[0] = 0;
    n->hilo[1] = 1;
    return n;
  }

  // merger[2k] is defined recursively:
  //
  // Let the input x_0, x_1, ..., x_{2k-1} be the input to merger[2k].
  // Divide this input into two k-length sequences X and X', such that
  // X  = x_0, x_1, ..., x_{k-1}
  // X' = x_k, x_{k+1}, ..., x_{2k-1}
  //
  // Now, we take X_0, X_2, ..., X_{k-2} and merge with X'_1, X'_3, ...,
  // X'_{k-1} with a merger[k]. We also take X_1, X_3, ..., X_{k-1} and merge
  // with X'_0, X'_2, ..., X'_{k-2} with another merger[k]. We then join
  // the outputs.

  n0 = merger_alloc_and_init(k / 2);
  if (!n0)
    goto free_n;
  n1 = merger_alloc_and_init(k / 2);
  if (!n1)
    goto free_n;

  for (i = 0; i < k / 2; i += 2)
    n->inputs[i] = n0->inputs[i / 2];
  for (i = 1; i < k / 2; i += 2)
    n->inputs[k + i] = n0->inputs[(k / 4) + (i / 2)];
  for (i = 1; i < k / 2; i += 2)
    n->inputs[i] = n1->inputs[i / 2];
  for (i = 0; i < k / 2; i += 2)
    n->inputs[k + i] = n1->inputs[(k / 4) + (i / 2)];

  for (i = 0; i < k / 2; i++) {
    b = (struct balancer *) malloc(sizeof(*b));
    if (!b)
      goto free_n;
    b->next = n->bhead;
    n->bhead = b;
    n0->outputs[i]->links[n0->hilo[i]].b = b;
    n0->outputs[i]->d.bits.l = 0;
    n1->outputs[i]->links[n1->hilo[i]].b = b;
    n1->outputs[i]->d.bits.l = 0;
    n->hilo[2 * i] = 0;
    n->hilo[2 * i + 1] = 1;
  }

  n0->btail->next = n->bhead;
  n->bhead = n0->bhead;
  n1->btail->next = n->bhead;
  n->bhead = n1->bhead;
  network_clear(n0); network_free(n0);
  network_clear(n1); network_free(n1);
  return n;

free_n:
  network_free(n);
  network_free(n0);
  network_free(n1);
  return 0;
}

struct network *
bitonic_alloc_and_init(unsigned int k)
{
  struct network *n, *n0, *n1, *n2;
  unsigned int i;
  if (k == 2) {
    // a bitonic[2] network is the same as a
    // merger[2]
    return merger_alloc_and_init(2);
  }

  n = network_alloc_and_init(k);
  n0 = n1 = n2 = 0;
  if (!n)
    goto free_n;
  n0 = merger_alloc_and_init(k);
  if (!n0)
    goto free_n;
  n1 = bitonic_alloc_and_init(k / 2);
  if (!n1)
    goto free_n;
  n2 = bitonic_alloc_and_init(k / 2);
  if (!n2)
    goto free_n;

  for (i = 0; i < k / 2; i++) {
    n->inputs[i] = n1->inputs[i];
    n->outputs[i] = n0->outputs[i];
    n->hilo[i] = n0->hilo[i];
    n1->outputs[i]->links[n1->hilo[i]].b = n0->inputs[i];
    n1->outputs[i]->d.bits.l = 0;
  }
  for (i = 0; i < k / 2; i++) {
    n->inputs[i + (k / 2)] = n2->inputs[i];
    n->outputs[i + (k / 2)] = n0->outputs[i + (k / 2)];
    n->hilo[i + (k / 2)] = n0->hilo[i + (k / 2)];
    n2->outputs[i]->links[n2->hilo[i]].b = n0->inputs[i + (k / 2)];
    n2->outputs[i]->d.bits.l = 0;
  }

  n0->btail->next = n->bhead;
  n->bhead = n0->bhead;
  n1->btail->next = n->bhead;
  n->bhead = n1->bhead;
  n2->btail->next = n->bhead;
  n->bhead = n2->bhead;
  network_clear(n0); network_free(n0);
  network_clear(n1); network_free(n1);
  network_clear(n2); network_free(n2);
  return n;

free_n:
  network_free(n);
  network_free(n0);
  network_free(n1);
  network_free(n2);
  return 0;
}
