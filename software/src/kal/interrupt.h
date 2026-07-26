#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*KalInterruptIsr)(void* context);

typedef enum {
    KalInterruptIdTim2,
    KalInterruptIdTim5,
    KalInterruptIdTim16,
    KalInterruptIdTim17,

    KalInterruptIdMax,
} KalInterruptId;

typedef enum {
    KalInterruptPriorityLowest = 15,
    KalInterruptPriorityLow = 12,
    KalInterruptPriorityNormal = 8,
    KalInterruptPriorityHigh = 4,
    KalInterruptPriorityHighest = 0,
} KalInterruptPriority;

void kal_interrupt_init(void);

void kal_interrupt_set_isr(KalInterruptId id, KalInterruptIsr isr, void* context);

void kal_interrupt_set_isr_ex(
    KalInterruptId id,
    KalInterruptPriority priority,
    KalInterruptIsr isr,
    void* context);

void kal_interrupt_critical_enter(void);

void kal_interrupt_critical_exit(void);

bool kal_interrupt_is_in_isr(void);

#ifdef __cplusplus
}
#endif
