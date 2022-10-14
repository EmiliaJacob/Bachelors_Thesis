#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
  struct timeval stop, start;
  gettimeofday(&start, NULL);
  sleep(2);
  gettimeofday(&stop, NULL);
  printf("took %lu us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);
  return 0;
}
