#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static int counter = 1;

void * routine(void *arg)
{
  unsigned i;
  for (i = 0; i < (0xfffff); i++);
  printf("hello %d start\n", counter);

  for (i = 0; i < (0xfffff); i++);
  printf("hello %d end\n", counter);
  counter++;
  return NULL;
}

int main()
{
  pthread_t thr[2];
  int i;
  for (i = 0; i < 2; i++) {
    if (pthread_create(&thr[i], NULL, &routine, NULL)) {
      printf("Cannot create thread %d\n", i);
      exit(1);
    }
  }
  for (i = 0; i < 2; i++) {
    if (pthread_join(thr[i], NULL))
      printf("Cannot join thread %d\n", i);
  }
  return 0;
}
