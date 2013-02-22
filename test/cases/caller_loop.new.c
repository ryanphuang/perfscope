#include <stdio.h>
int bar1(int c)
{
  if (c < 50)
    c += 10;
  else
    c -= 10;
  return c * c;
}
int bar2(int c)
{
  if (c < 5)
    c++;
  else
    c--;
  return c << 1;
}
int foo(int c)
{
  int i;
  int sum = 0;
  for (i = 0; i < 10; i++) {
    sum += bar1(i + c);
  }
  sum = bar2(sum);
  return sum;
}
int main()
{
  printf("%d\n", foo(10));
}
