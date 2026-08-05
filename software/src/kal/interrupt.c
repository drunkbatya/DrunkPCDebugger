#include <kal/interrupt.h>

#include <kal/kal.h>

#include <main.h>

typedef struct {
    KalInterruptIsr isr;
    void* context;
} KalInterrupt;

static volatile KalInterrupt kal_interrupt[KalInterruptIdMax];
static volatile uint32_t kal_interrupt_critical_nesting = 0;
static volatile uint32_t kal_interrupt_critical_primask = 0;

static const IRQn_Type kal_interrupt_irqn[KalInterruptIdMax] = {
    [KalInterruptIdTim2] = TIM2_IRQn,
    [KalInterruptIdTim5] = TIM5_IRQn,
    [KalInterruptIdTim16] = TIM16_IRQn,
    [KalInterruptIdTim17] = TIM17_IRQn,
};

static const IRQn_Type kal_interrupt_exti_irqn[] = {
    EXTI0_IRQn,
    EXTI1_IRQn,
    EXTI2_IRQn,
    EXTI3_IRQn,
    EXTI4_IRQn,
    EXTI9_5_IRQn,
    EXTI15_10_IRQn,
};

static void kal_interrupt_init_exti(void) {
    const size_t count = sizeof(kal_interrupt_exti_irqn) / sizeof(kal_interrupt_exti_irqn[0]);

    for(size_t i = 0; i < count; i++) {
        NVIC_SetPriority(kal_interrupt_exti_irqn[i], KalInterruptPriorityNormal);
        NVIC_EnableIRQ(kal_interrupt_exti_irqn[i]);
    }
}

void kal_interrupt_init(void) {
    for(size_t id = 0; id < KalInterruptIdMax; id++) {
        kal_interrupt[id].isr = NULL;
        kal_interrupt[id].context = NULL;
    }
    kal_interrupt_init_exti();
}

void kal_interrupt_set_isr(KalInterruptId id, KalInterruptIsr isr, void* context) {
    kal_interrupt_set_isr_ex(id, KalInterruptPriorityNormal, isr, context);
}

void kal_interrupt_set_isr_ex(
    KalInterruptId id,
    KalInterruptPriority priority,
    KalInterruptIsr isr,
    void* context) {
    if(id >= KalInterruptIdMax) kal_crash();
    if(isr != NULL && kal_interrupt[id].isr != NULL) kal_crash();

    const IRQn_Type irqn = kal_interrupt_irqn[id];

    kal_interrupt_critical_enter();

    kal_interrupt[id].isr = isr;
    kal_interrupt[id].context = context;

    if(isr != NULL) {
        NVIC_SetPriority(irqn, priority);
        NVIC_EnableIRQ(irqn);
    } else {
        NVIC_DisableIRQ(irqn);
        NVIC_ClearPendingIRQ(irqn);
    }

    kal_interrupt_critical_exit();
}

void kal_interrupt_critical_enter(void) {
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if(kal_interrupt_critical_nesting == 0) {
        kal_interrupt_critical_primask = primask;
    }
    kal_interrupt_critical_nesting++;
}

void kal_interrupt_critical_exit(void) {
    if(kal_interrupt_critical_nesting == 0) kal_crash();

    kal_interrupt_critical_nesting--;
    if(kal_interrupt_critical_nesting == 0 && kal_interrupt_critical_primask == 0) {
        __enable_irq();
    }
}

bool kal_interrupt_is_in_isr(void) {
    return __get_IPSR() != 0;
}

static inline void kal_interrupt_call(KalInterruptId id) {
    if(kal_interrupt[id].isr) {
        kal_interrupt[id].isr(kal_interrupt[id].context);
    }
}

void TIM2_IRQHandler(void) {
    kal_interrupt_call(KalInterruptIdTim2);
}

void TIM5_IRQHandler(void) {
    kal_interrupt_call(KalInterruptIdTim5);
}

void TIM16_IRQHandler(void) {
    kal_interrupt_call(KalInterruptIdTim16);
}

void TIM17_IRQHandler(void) {
    kal_interrupt_call(KalInterruptIdTim17);
}
