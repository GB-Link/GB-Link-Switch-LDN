#!/usr/bin/env python3
"""Copies librfu's SIO layer out of a pokefirered checkout and adapts it to the host
simulation. The game's own code is never kept in this repository: the test builds it
from the checkout each time.

The changes are mechanical and every one is checked: timer registers reached through
pointers go through the register macros instead (each access is a point where the
simulated hardware catches up), the three naked ARM trampolines become plain calls,
the ID exchange's idle loop spends simulated cycles, and one byte-wide store goes
through the 16-bit register."""
import sys
from pathlib import Path

TIMER_L = 'REG_TMCNT_L(gSTWIStatus->timerSelect)'
TIMER_H = 'REG_TMCNT_H(gSTWIStatus->timerSelect)'

PATCHES = {
    'librfu_sio32id.c': [
        ('    vu16 *regTMCNTL;\n', '', 1),
        ('    regTMCNTL = &REG_TMCNT_L(gSTWIStatus->timerSelect);\n', '', 1),
        ('regTMCNTL[1] = ', TIMER_H + ' = ', 3),
        ('regTMCNTL[0] = ', TIMER_L + ' = ', 2),
        ('while (regTMCNTL[0] < 32)', 'while (' + TIMER_L + ' < 32)', 1),
        ('*(vu8 *)&REG_SIOCNT |= SIO_ENABLE;', 'REG_SIOCNT |= SIO_ENABLE;', 1),
        ('for (delay = 0; delay < 600; ++delay)\n            ;',
         'sim_gba_cycles(600 * 6);\n        (void)delay;', 1),
    ],
    'librfu_intr.c': [
        ('    vu16* regTMCNTL = (vu16*)(REG_ADDR_TMCNT_L + gSTWIStatus->timerSelect * 4);\n', '', 1),
        ('    vu16* regTMCNTH = (vu16*)(REG_ADDR_TMCNT_H + gSTWIStatus->timerSelect * 4);\n', '', 1),
        ('*regTMCNTL = ', TIMER_L + ' = ', 4),
        ('*regTMCNTH = ', TIMER_H + ' = ', 1),
        ('NAKED\n', '', 3),
        ('asm("bx r2");', 'callbackM(reqCommandId, error);', 1),
        ('asm("bx r1");', 'callbackS(reqCommandId);', 1),
        ('asm("bx r0");', 'callbackId();', 1),
    ],
    'librfu_stwi.c': [
        ('    vu16 *timerL;\n    vu16 *timerH;\n', '', 1),
        ('    timerL = &REG_TMCNT_L(gSTWIStatus->timerSelect);\n    timerH = &REG_TMCNT_H(gSTWIStatus->timerSelect);\n', '', 1),
        ('    vu16 *timerL = &REG_TMCNT_L(gSTWIStatus->timerSelect);\n    vu16 *timerH = &REG_TMCNT_H(gSTWIStatus->timerSelect);\n', '', 1),
        ('while (*timerL <= 0x11)', 'while (' + TIMER_L + ' <= 0x11)', 1),
        ('*timerL = ', TIMER_L + ' = ', 5),
        ('*timerH = ', TIMER_H + ' = ', 4),
    ],
}


def main():
    if len(sys.argv) != 3:
        sys.exit('usage: patch_librfu.py <pokefirered> <out>')
    root, out = Path(sys.argv[1]), Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    (out / 'librfu.h').write_text((root / 'include' / 'librfu.h').read_text())
    for name, patches in PATCHES.items():
        text = (root / 'src' / name).read_text()
        for old, new, count in patches:
            found = text.count(old)
            if found != count:
                sys.exit(f'{name}: expected {count} of {old!r}, found {found}')
            text = text.replace(old, new)
        if 'regTMCNT' in text or 'timerL' in text or 'timerH' in text or 'asm(' in text:
            sys.exit(f'{name}: an unpatched timer pointer or asm block remains')
        (out / name).write_text('#include "sim_gba.h"\n' + text)


if __name__ == '__main__':
    main()
