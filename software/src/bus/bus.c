#include "bus.h"

bool bus_acqure(void) {
    // kal_gpio_write(&gpio_busreq_pin, BUSREQ_ACTIVE_LEVEL)
    // while (1) {
    //      if (timeout) break;
    //      if (kal_gpio_read(&gpio_busack_pin) == BUSACK_ACTIVE_LEVEL) {
    //          return true;
    //      }
    // }
    // return false;
}

bool bus_release(void) {
    // ^ vice-versa
}
