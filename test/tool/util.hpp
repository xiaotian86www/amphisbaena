#include <ctime>
#include <gtest/gtest.h>

#define EXPECT_SPEND_LT(func, ns)                                              \
  timespec start, stop;                                                        \
  clock_gettime(CLOCK_MONOTONIC, &start);                                      \
  func;                                                                        \
  clock_gettime(CLOCK_MONOTONIC, &stop);                                       \
  EXPECT_LT((stop.tv_sec - start.tv_sec) * 1000000000 +                        \
              (stop.tv_nsec - start.tv_nsec),                                  \
            ns);

#define EXPECT_SPEND_GT(func, ns)                                              \
  timespec start, stop;                                                        \
  clock_gettime(CLOCK_MONOTONIC, &start);                                      \
  func;                                                                        \
  clock_gettime(CLOCK_MONOTONIC, &stop);                                       \
  EXPECT_GT((stop.tv_sec - start.tv_sec) * 1000000000 +                        \
              (stop.tv_nsec - start.tv_nsec),                                  \
            ns);