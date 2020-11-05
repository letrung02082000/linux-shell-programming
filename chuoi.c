#include"stdio.h"

void main(void)
{
    char* s[100];
    *s = "Hello world!";
    printf("%s", *(s+1));
}