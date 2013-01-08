#include <assert.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <unistd.h>

#include "network.h"
#include "macros.h"

#define NSECSTEST 10

static unsigned int bitonic_n;
static unsigned int num_threads;

static volatile uint64_t value CACHE_ALIGNED;
static volatile bool running CACHE_ALIGNED;

static int verbose;

static inline uint64_t
tv_usec(const struct timeval *tv)
{
  return ((uint64_t)tv->tv_sec) * 1000000 + tv->tv_usec;
}

/**
 * returns the number of seconds the test executed for
 */
static double
run_test(void *(*runner)(void *), void *p)
{
  struct timeval tvbegin, tvend;
  pthread_t *workers;
  unsigned int i;
  double s;
  workers = (pthread_t *) malloc(sizeof(workers[0]) * num_threads);
  running = true;
  gettimeofday(&tvbegin, 0);
  for (i = 0; i < num_threads; i++)
    pthread_create(&workers[i], NULL, runner, p);
  sleep(NSECSTEST);
  running = false;
  for (i = 0; i < num_threads; i++)
    pthread_join(workers[i], NULL);
  gettimeofday(&tvend, 0);
  s = ((double)(tv_usec(&tvend) - tv_usec(&tvbegin)))/1000000.0;
  free(workers);
  return s;
}

static void
report_results(uint64_t nincrs, double seconds)
{
  double r;
  r = ((double) nincrs)/seconds;
  if (verbose)
    fprintf(stderr, "Test ran for: %.3f seconds\n", seconds);
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
  counting_network_assign_thread(cn);
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
  verbose = 0;
  for (;;) {
    static struct option long_options[] =
    {
      {"verbose",      no_argument,       &verbose, 1  },
      {"counter",      required_argument, 0,        'c'},
      {"num-threads",  required_argument, 0,        'n'},
      {0, 0, 0, 0}
    };
    int option_index = 0;
    c = getopt_long(argc, argv, "c:n:", long_options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
      abort();
    case 'c':
      if (!strcasecmp("spinlock", optarg)) {
        if (verbose)
          fprintf(stderr, "counter: %s\n", optarg);
        runner = spinlock_runner;
      } else if (!strcasecmp("cas", optarg)) {
        if (verbose)
          fprintf(stderr, "counter: %s\n", optarg);
        runner = cas_runner;
      } else if (!strncasecmp("bitonic:", optarg, 8)) {
        bitonic_n = strtol(optarg + 8, NULL, 10);
        assert(bitonic_n >= 2);
        if (verbose)
          fprintf(stderr, "counter: %s\n", optarg);
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
      if (verbose)
        fprintf(stderr, "num_threads: %d\n", num_threads);
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
