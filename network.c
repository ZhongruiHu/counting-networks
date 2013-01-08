#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "balancer.h"
#include "bitonic.h"
#include "network.h"

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
  free(n);
}

struct counting_network *
counting_network_alloc_and_init(unsigned int n)
{
  struct counting_network *cn;
  unsigned int i;
  cn = (struct counting_network *) malloc(sizeof(*cn));
  if (!cn)
    return 0;
  cn->n = 0;
  cn->buckets = 0;
  cn->n = bitonic_alloc_and_init(n);
  if (!cn->n)
    goto free_cn;
  cn->buckets = (struct counting_bucket **) malloc(sizeof(cn->buckets[0]) * n);
  if (!cn->buckets)
    goto free_cn;
  memset(cn->buckets, 0, sizeof(cn->buckets[0]) * n);
  for (i = 0; i < n; i++) {
    cn->buckets[i] = (struct counting_bucket *) malloc(sizeof(*cn->buckets[0]));
    if (!cn->buckets[i])
      goto free_cn;
    cn->buckets[i]->value = i;
    cn->n->outputs[i]->links[cn->n->hilo[i]].opaque = cn->buckets[i];
  }

  return cn;

free_cn:
  counting_network_free(cn);
  return 0;
}

void
counting_network_free(struct counting_network *cn)
{
  unsigned int i;
  if (!cn)
    return;
  if (cn->buckets) {
    for (i = 0; i < cn->n->size; i++)
      free(cn->buckets[i]);
    free(cn->buckets);
  }
  network_free(cn->n);
  free(cn);
}

static volatile unsigned int g_counter = 0;
static __thread unsigned int tl_idx = 0;

void
counting_network_assign_thread(const struct counting_network *cn)
{
  tl_idx = __sync_fetch_and_add(&g_counter, 1) % cn->n->size;
}

uint64_t
counting_network_next_value(struct counting_network *cn)
{
  struct counting_bucket *cb;
  cb = (struct counting_bucket *) balancer_traverse(cn->n->inputs[tl_idx]);
  return __sync_fetch_and_add(&cb->value, cn->n->size);
}
