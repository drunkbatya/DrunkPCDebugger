#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void bus_power_set_enabled(bool enabled);

bool bus_power_is_enabled(void);

#ifdef __cplusplus
}
#endif
