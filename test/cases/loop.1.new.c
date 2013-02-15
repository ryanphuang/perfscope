#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int foo(int c)
{
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            c += i;
            c *= j;
            pid_t pid = getpid();
        }
        if (c > 100)
          return c;
    }
    c++;
    return c;
}

int add()
{
    int i;
    int sum = 0;
    for (i = 1; i * i < 100; i++) {
       sum += i; 
    }
    if (sum > 10000)
      return 10000;
    return sum;
}

int mul(int n)
{
  int product = 1;
  for (int i = 2; i < n; i++) {
    product *= i;
  }
  return product;
}

int bar(int n)
{
  int ret;
  if (n > 10)
    ret = 10;
  else
    ret = n / 2;
  return ret;
}

int main()
{
    foo(add());
    mul(10);
}
