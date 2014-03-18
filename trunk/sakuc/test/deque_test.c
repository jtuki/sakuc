// 2014-3-16 - created by jtuki@foxmail.com

#include "../src/deque.h"
#include "common_test_defs.h"
#include "deque_test.h"

#include <string.h>

// 1638 characters without suffix '\0'. (1638 = 18 * 91)
static char mit_license[] = 
    "The MIT License (MIT)                                                         "
    "                                                                              "
    "Copyright (c) <year> <copyright holders>                                      "
    "                                                                              "
    "Permission is hereby granted, free of charge, to any person obtaining a copy  "
    "of this software and associated documentation files (the 'Software'), to deal "
    "in the Software without restriction, including without limitation the rights  "
    "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     "
    "copies of the Software, and to permit persons to whom the Software is         "
    "furnished to do so, subject to the following conditions:                      "
    "                                                                              "
    "The above copyright notice and this permission notice shall be included in    "
    "all copies or substantial portions of the Software.                           "
    "                                                                              "
    "THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    "
    "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      "
    "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   "
    "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        "
    "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
    "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     "
    "THE SOFTWARE.                                                                 "
    ;
    
#define  mit_license_length (sizeof(mit_license) - 1)

static char buffer[mit_license_length + 1];

int test_deque(void)
{
    // ============================== part 1 ==============================
    // ## test part 1.1 - sakuc_deque_push_back, sakuc_deque_pop_front
    sakuc_deque_t *dq = sakuc_deque_new(9, sizeof(char), SAKUC_DEQUE_SWEEP_MANUALLY);
    size_t n_sub_deque = (mit_license_length % 9 == 0) ?
                         (mit_license_length / 9) : (mit_license_length / 9 + 1);
                         
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_push_back(dq, &mit_license[i], sizeof(char)) == -1)
            break;
    }
    sakuc_assert(sakuc_deque_size(dq) == mit_license_length && dq->num_sub_deque == n_sub_deque);
    
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_pop_front(dq, &buffer[i], sizeof(char)) == -1)
            break;
    }
    buffer[mit_license_length] = '\0';
    sakuc_assert(sakuc_deque_size(dq) == 0 && strcmp(buffer, mit_license) == 0);
    
    // ## test part 1.2 - manually sweep
    sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_IMMEDIATELY);
    sakuc_assert(sakuc_deque_size(dq) == 0 && dq->num_sub_deque == 1);
    
    // ## test part 1.3 - once again, sakuc_deque_push_back, sakuc_deque_pop_front
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_push_back(dq, &mit_license[i], sizeof(char)) == -1)
            break;
    }
    sakuc_assert(sakuc_deque_size(dq) == mit_license_length && dq->num_sub_deque == n_sub_deque + 1);
    
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_pop_front(dq, &buffer[i], sizeof(char)) == -1)
            break;
    }
    buffer[mit_license_length] = '\0';
    sakuc_assert(sakuc_deque_size(dq) == 0 && strcmp(buffer, mit_license) == 0);
    
    // ## test part 1.4 - once again, manually sweep.
    sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_IMMEDIATELY);
    sakuc_assert(sakuc_deque_size(dq) == 0 && dq->num_sub_deque == 1);
    
    // ## test part 1.5 - destroy.
    sakuc_deque_destroy(dq);
    
    // ============================== part 2 ==============================
    // ## test part 2.1 - sakuc_deque_push_front, sakuc_deque_pop_back
    dq = sakuc_deque_new(9, sizeof(char), SAKUC_DEQUE_SWEEP_MANUALLY);
    n_sub_deque = (mit_license_length % 9 == 0) ?
                  (mit_license_length / 9) : (mit_license_length / 9 + 1);
                         
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_push_front(dq, &mit_license[i], sizeof(char)) == -1)
            break;
    }
    sakuc_assert(sakuc_deque_size(dq) == mit_license_length && dq->num_sub_deque == n_sub_deque + 1);
    
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_pop_back(dq, &buffer[i], sizeof(char)) == -1)
            break;
    }
    buffer[mit_license_length] = '\0';
    sakuc_assert(sakuc_deque_size(dq) == 0 && strcmp(buffer, mit_license) == 0);
    
    // ## test part 2.2 - manually sweep
    sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_IMMEDIATELY);
    sakuc_assert(sakuc_deque_size(dq) == 0 && dq->num_sub_deque == 1);
    
    // ## test part 2.3 - once again, sakuc_deque_push_front, sakuc_deque_pop_back
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_push_front(dq, &mit_license[i], sizeof(char)) == -1)
            break;
    }
    sakuc_assert(sakuc_deque_size(dq) == mit_license_length && dq->num_sub_deque == n_sub_deque + 1);
    
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_pop_back(dq, &buffer[i], sizeof(char)) == -1)
            break;
    }
    buffer[mit_license_length] = '\0';
    sakuc_assert(sakuc_deque_size(dq) == 0 && strcmp(buffer, mit_license) == 0);
    
    // ## test part 2.4 - once again, manually sweep.
    sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_IMMEDIATELY);
    sakuc_assert(sakuc_deque_size(dq) == 0 && dq->num_sub_deque == 1);
    
    // ## test part 2.5 - destroy.
    sakuc_deque_destroy(dq);
    
    // ============================== part 3 ==============================
    // ## test part 3.1 - sakuc_deque_push_front, sakuc_deque_pop_front
    dq = sakuc_deque_new(9, sizeof(char), SAKUC_DEQUE_SWEEP_MANUALLY);
    n_sub_deque = (mit_license_length % 9 == 0) ?
                  (mit_license_length / 9) : (mit_license_length / 9 + 1);
                         
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_push_front(dq, &mit_license[i], sizeof(char)) == -1)
            break;
    }
    sakuc_assert(sakuc_deque_size(dq) == mit_license_length && dq->num_sub_deque == n_sub_deque + 1);
    
    for (size_t i = mit_license_length; i > 0; i--) {
        if (sakuc_deque_pop_front(dq, &buffer[i-1], sizeof(char)) == -1)
            break;
    }
    buffer[mit_license_length] = '\0';
    sakuc_assert(sakuc_deque_size(dq) == 0 && strcmp(buffer, mit_license) == 0);
    
    // ## test part 3.2 - manually sweep
    sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_IMMEDIATELY);
    sakuc_assert(sakuc_deque_size(dq) == 0 && dq->num_sub_deque == 2);
    
    // ## test part 3.3 - once again, sakuc_deque_push_front, sakuc_deque_pop_front
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_push_front(dq, &mit_license[i], sizeof(char)) == -1)
            break;
    }
    sakuc_assert(sakuc_deque_size(dq) == mit_license_length && dq->num_sub_deque == n_sub_deque + 1);
    
    for (size_t i = mit_license_length; i > 0; i--) {
        if (sakuc_deque_pop_front(dq, &buffer[i-1], sizeof(char)) == -1)
            break;
    }
    buffer[mit_license_length] = '\0';
    sakuc_assert(sakuc_deque_size(dq) == 0 && strcmp(buffer, mit_license) == 0);
    
    // ## test part 3.4 - once again, manually sweep.
    sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_IMMEDIATELY);
    sakuc_assert(sakuc_deque_size(dq) == 0 && dq->num_sub_deque == 2);
    
    // ## test part 3.5 - destroy.
    sakuc_deque_destroy(dq);
    
    // ============================== part 4 ==============================
    // ## test part 4.1 - sakuc_deque_push_back, sakuc_deque_pop_back
    dq = sakuc_deque_new(9, sizeof(char), SAKUC_DEQUE_SWEEP_MANUALLY);
    n_sub_deque = (mit_license_length % 9 == 0) ?
                  (mit_license_length / 9) : (mit_license_length / 9 + 1);
                         
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_push_back(dq, &mit_license[i], sizeof(char)) == -1)
            break;
    }
    sakuc_assert(sakuc_deque_size(dq) == mit_license_length && dq->num_sub_deque == n_sub_deque);
    
    for (size_t i = mit_license_length; i > 0; i--) {
        if (sakuc_deque_pop_back(dq, &buffer[i-1], sizeof(char)) == -1)
            break;
    }
    buffer[mit_license_length] = '\0';
    sakuc_assert(sakuc_deque_size(dq) == 0 && strcmp(buffer, mit_license) == 0);
    
    // ## test part 4.2 - manually sweep
    sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_IMMEDIATELY);
    sakuc_assert(sakuc_deque_size(dq) == 0 && dq->num_sub_deque == 1);
    
    // ## test part 4.3 - once again, sakuc_deque_push_back, sakuc_deque_pop_back
    for (size_t i=0; i < mit_license_length; i++) {
        if (sakuc_deque_push_back(dq, &mit_license[i], sizeof(char)) == -1)
            break;
    }
    sakuc_assert(sakuc_deque_size(dq) == mit_license_length && dq->num_sub_deque == n_sub_deque);
    
    for (size_t i = mit_license_length; i > 0; i--) {
        if (sakuc_deque_pop_back(dq, &buffer[i-1], sizeof(char)) == -1)
            break;
    }
    buffer[mit_license_length] = '\0';
    sakuc_assert(sakuc_deque_size(dq) == 0 && strcmp(buffer, mit_license) == 0);
    
    // ## test part 4.4 - once again, manually sweep.
    sakuc_deque_sweep(dq, SAKUC_DEQUE_SWEEP_IMMEDIATELY);
    sakuc_assert(sakuc_deque_size(dq) == 0 && dq->num_sub_deque == 1);
    
    // ## test part 4.5 - destroy.
    sakuc_deque_destroy(dq);
    
    return 0;
sakuc_assert_failed:
    return -1;
}
