#pragma once
/* Stands in for pokefirered's global.h when librfu's SIO layer is built on the host:
   the types, register macros and constants that layer uses, nothing else. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "sim_gba.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef volatile uint8_t vu8;
typedef volatile uint16_t vu16;
typedef volatile uint32_t vu32;
typedef volatile int8_t vs8;
typedef volatile int16_t vs16;
typedef volatile int32_t vs32;
typedef uint8_t bool8;

#define TRUE 1
#define FALSE 0
#define REVISION 0
#define COMMON_DATA
#define EWRAM_DATA
#define IWRAM_DATA
#define NAKED

#define REG_SIODATA32 (*(vu32 *)sim_gba_reg(0x120))
#define REG_SIOCNT (*(vu16 *)sim_gba_reg(0x128))
#define REG_RCNT (*(vu16 *)sim_gba_reg(0x134))
#define REG_IE (*(vu16 *)sim_gba_reg(0x200))
#define REG_IF (*(vu16 *)sim_gba_reg(0x202))
#define REG_IME (*(vu16 *)sim_gba_reg(0x208))
#define REG_TMCNT_L(n) (*(vu16 *)sim_gba_reg(0x100 + (n) * 4))
#define REG_TMCNT_H(n) (*(vu16 *)sim_gba_reg(0x102 + (n) * 4))
#define REG_TMCNT(n) (*(vu32 *)sim_gba_reg(0x100 + (n) * 4))
#define REG_TM0CNT_L REG_TMCNT_L(0)
#define REG_TM0CNT_H REG_TMCNT_H(0)

#define TIMER_1CLK 0x00
#define TIMER_64CLK 0x01
#define TIMER_256CLK 0x02
#define TIMER_1024CLK 0x03
#define TIMER_INTR_ENABLE 0x40
#define TIMER_ENABLE 0x80

#define SIO_32BIT_MODE 0x1000
#define SIO_MULTI_MODE 0x2000
#define SIO_9600_BPS 0x0000
#define SIO_38400_BPS 0x0001
#define SIO_57600_BPS 0x0002
#define SIO_115200_BPS 0x0003
#define SIO_MULTI_SI 0x0004
#define SIO_MULTI_SD 0x0008
#define SIO_MULTI_BUSY 0x0080
#define SIO_ENABLE 0x0080
#define SIO_INTR_ENABLE 0x4000
#define SIO_MULTI_SI_SHIFT 2

#define INTR_FLAG_TIMER0 (1 << 3)
#define INTR_FLAG_TIMER3 (1 << 6)
#define INTR_FLAG_SERIAL (1 << 7)

#define CpuFill32(value, dest, size)                                                  \
    do {                                                                              \
        uint32_t fill_ = (value);                                                     \
        for (size_t i_ = 0; i_ < (size) / 4; i_++) ((uint32_t *)(dest))[i_] = fill_;  \
    } while (0)
#define CpuCopy32(src, dest, size) memcpy((dest), (src), (size))
#define DmaCopy16(channel, src, dest, size) memcpy((dest), (src), (size))
#define IntrEnable(flags) sim_gba_intr_enable(flags)
