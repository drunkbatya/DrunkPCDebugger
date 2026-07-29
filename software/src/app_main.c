#include <app_main.h>

#include <bus/bus.h>
#include <kal/kal.h>
#include <rpc/rpc.h>

void app_main(void) {
    kal_init();
    bus_init();
    rpc_init();

    while(1) {
        rpc_process();
    }
}
