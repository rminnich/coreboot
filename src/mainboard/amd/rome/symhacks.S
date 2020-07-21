/* SPDX-License-Identifier: GPL-2.0-or-later */
.org 0x1000

.globl _eearlyram_stack
_eearlyram_stack:
.globl _early_reserved_dram
_early_reserved_dram:
.long 0
.align 4096
.globl _eearly_reserved_dram
_eearly_reserved_dram:

