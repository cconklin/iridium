#include "ir_object.h"
#include <pthread.h>

struct ThreadArg
{
    object thread;
    struct IridiumContext * context;
};

void * ir_dispatch_thread(void *);
void IR_init_Thread(struct IridiumContext *);

