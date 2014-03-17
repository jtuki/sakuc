#include <stdio.h>
#include "test/ringbuffer_test.h"
#include "test/deque_test.h"
#include "test/multi_pattern_match_test.h"

int main()
{
    printf("START MODULES' UNIT TEST ...\n");
    
    if (test_ringbuffer() == -1)
        printf("*** FAILED! - ringbuffer\n");
    else
        printf("* PASSED! - ringbuffer\n");
        
    if (test_deque() == -1)
        printf("*** FAILED! - deque\n");
    else
        printf("* PASSED! - deque\n");
    
    if (test_multi_pattern_match() == -1)
        printf("*** FAILED! - multi-pattern match\n");
    else
        printf("* PASSED! - multi-pattern match\n");
        
    printf("FINISH MODULES' UNIT TEST!\n\n"
           "Note: If there is no 'FAILED', then congratulations! :) \n");
    
    return 0;
}
