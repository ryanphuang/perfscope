#include<stdio.h>
static int id = 2;

int foo(int c)
{
    c++;
    if (c > 10) 
{
        c *= c;
        return c;
}
    else {
        id++;
        if (id > 10) {
            c++;
        }
        else {
            c--;
        }
    }
    int x = c * id;
    x *= x;
    return x;
}

int bar()
{
    int i;
    int sum = 0;
    for (i = 1; i < 10; i++) {
       sum += i; 
    }
    return sum;
}

int main()
{
    foo(bar());
}
