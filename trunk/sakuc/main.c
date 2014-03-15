#include <stdio.h>
#include "test/ringbuffer_test.h"

int main()
{
    printf("Start modules' unit test ...\n");
    
    if (test_ringbuffer() == -1)
        printf("* ringbuffer unit test failed.\n");
        
    printf("Finish modules' unit test!\n");
    return 0;
}
