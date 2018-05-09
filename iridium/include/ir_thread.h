#include "ir_object.h"
#include <pthread.h>

struct ThreadStatus {
    int is_ok;
    object result;
};

void * ir_dispatch_thread(void *);
void IR_init_Thread(struct IridiumContext *);

