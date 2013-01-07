#include <assert.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "network.h"

#define NSECSTEST 1

static unsigned int bitonic_n;
static unsigned int num_threads;

static volatile uint64_t value;
static volatile bool running;

/**
 * returns the number of seconds the test executed for
 */
static double
run_test(void *(*runner)(void *), void *p)
{
  pthread_t *workers;
  unsigned int i;
  workers = (pthread_t *) malloc(sizeof(workers[0]) * num_threads);
  running = true;
  for (i = 0; i < num_threads; i++)
    pthread_create(&workers[i], NULL, runner, p);
  sleep(NSECSTEST);
  running = false;
  for (i = 0; i < num_threads; i++)
    pthread_join(workers[i], NULL);
  free(workers);
  return (double) NSECSTEST;
}

static void
report_results(uint64_t nincrs, double seconds)
{
  double r;
  r = ((double) nincrs)/seconds;
  printf("%f\n", r);
}

static void *
spinlock_worker(void *p)
{
  pthread_spinlock_t *spinlock;
  spinlock = (pthread_spinlock_t *) p;
  while (running) {
    pthread_spin_lock(spinlock);
    value++;
    pthread_spin_unlock(spinlock);
  }
  return 0;
}

static void
spinlock_runner()
{
  pthread_spinlock_t spinlock;
  double seconds;
  pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
  value = 0;
  seconds = run_test(spinlock_worker, (void *) &spinlock);
  report_results(value, seconds);
  pthread_spin_destroy(&spinlock);
}

static void *
cas_worker(void *p)
{
  while (running)
    __sync_fetch_and_add(&value, 1);
  return 0;
}

static void
cas_runner()
{
  double seconds;
  value = 0;
  seconds = run_test(cas_worker, 0);
  report_results(value, seconds);
}

static void *
bitonic_worker(void *p)
{
  struct counting_network *cn;
  cn = (struct counting_network *) p;
  while (running)
    counting_network_next_value(cn);
  return 0;
}

static void
bitonic_runner()
{
  struct counting_network *cn;
  double seconds;
  cn = counting_network_alloc_and_init(bitonic_n);
  seconds = run_test(bitonic_worker, cn);
  report_results(counting_network_next_value(cn), seconds);
  counting_network_free(cn);
}

int
main(int argc, char **argv)
{
  int c;
  void (*runner)(void);
  runner = 0;
  for (;;) {
    static struct option long_options[] =
    {
      {"counter",      required_argument, 0, 'c'},
      {"num-threads",  required_argument, 0, 'n'},
      {0, 0, 0, 0}
    };
    int option_index = 0;
    c = getopt_long(argc, argv, "c:n:", long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 'c':
      if (!strcasecmp("spinlock", optarg))
        runner = spinlock_runner;
      else if (!strcasecmp("cas", optarg))
        runner = cas_runner;
      else if (!strncasecmp("bitonic:", optarg, 8)) {
        bitonic_n = strtol(optarg + 8, NULL, 10);
        assert(bitonic_n >= 2);
        // XXX: check bitonic_n is a power of 2 >= 2
        runner = bitonic_runner;
      } else {
        fprintf(stderr, "Error: `%s' is invalid for counter type\n", optarg);
        return 1;
      }
      break;

    case 'n':
      num_threads = strtol(optarg, NULL, 10);
      if (num_threads == 0) {
        fprintf(stderr, "Error: num_threads must be > 0\n");
        return 1;
      }
      break;

    case '?':
      /* getopt_long already printed an error message. */
      break;

    default:
      abort();
    }
  }
  runner();
  return 0;
}
