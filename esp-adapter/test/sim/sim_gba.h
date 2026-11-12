#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The simulated GBA as librfu sees it. Every register access goes through
   sim_gba_reg(), which lets the hardware catch up (a write made since the last access
   takes effect, read-only bits refresh, pending interrupts are taken) and spends the
   access's share of CPU time. */
void *sim_gba_reg(uint32_t offset);
void sim_gba_cycles(uint32_t cycles);
void sim_gba_intr_enable(uint16_t flags);

#ifdef __cplusplus
}
#endif
