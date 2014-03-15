#include "../ringbuffer.h"
#include "common_test_defs.h"
#include "ringbuffer_test.h"

typedef struct vehicle_info
{
    float pos_x; float pos_y; float pos_z;
    float speed_x; float speed_y; float speed_z;
    int id;
    const char *model;
} vehicle_info_t;

// refer to http://goo.gl/yF8rvG
static const char *model_list[] = {
    "DongFeng", "GreatWall", "Skoda", "Peugeot", "BMW", "benz", "audi"
};

static const int model_list_length = sizeof(model_list) / sizeof(model_list[0]);

int test_ringbuffer (void)
{
    // ## test part 1
    ringbuffer_t *rbuf = rbuf_new(1000, sizeof(struct vehicle_info));
    for (size_t i=0; i < 1000; i++) {
        struct vehicle_info veh = {
            .id = i,
            .model = model_list[i % model_list_length],
        };
        rbuf_push(rbuf, &veh, sizeof(struct vehicle_info));
    }
    sakuc_assert (rbuf_length(rbuf) == 1000);
    
    // ## test part 2
    struct vehicle_info veh;
    int count = 0;
    while (rbuf_popleft(rbuf, &veh, sizeof(struct vehicle_info)) != nullptr) {
        if (veh.id == count && veh.model == model_list[count % model_list_length])
            ++count;
        else
            break;
    }
    sakuc_assert (rbuf_length(rbuf) == 0 && count == 1000);
    
    // ## test part 3
    for (size_t i=0; i < 3000; i++) {
        struct vehicle_info veh = {
            .id = i,
            .model = model_list[i % model_list_length],
        };
        rbuf_push(rbuf, &veh, sizeof(struct vehicle_info));
    }
    sakuc_assert (rbuf_length(rbuf) == 1000);
    
    // ## test part 4
    count = 2000;
    while (rbuf_popleft(rbuf, &veh, sizeof(struct vehicle_info)) != nullptr) {
        if (veh.id == count && veh.model == model_list[count % model_list_length])
            ++count;
        else
            break;
    }
    sakuc_assert (rbuf_length(rbuf) == 0 && count == 3000);
    
    sakuc_assert (rbuf_destroy(rbuf) != -1);
    
    return 0;
    
sakuc_assert_failed:
    return -1;
}
