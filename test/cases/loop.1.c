#include<stdio.h>

int foo(int c)
{
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            c += i;
            c *= j;
        }
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

int main()
{
    foo(add());
    mul(10);
}
