#include <stdio.h>


typedef int (*hello)(int c);
typedef int (*pippo)(int c);

typedef struct
{
    hello hello;
    pippo pippo;
} test;


int Hello_f(int c)
{
    printf("hello %d\n", c);
    return 0;
}

int Pippo_f(int c)
{
    printf("pippo %d\n", c);
    return 0;
}




int main()
{
    test t;

    t.hello = Hello_f;
    t.pippo = Pippo_f;

    int a = t.hello(1);
    int b = t.pippo(2);
    return 0;
}