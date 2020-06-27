/*
 *
 * Copyright (C) 2018 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <arch/types.h>
/* buts is a programmed IO byte puts, which you need to write for all platforms. */
void buts(char *s);

void hex4(u8 c);
void hex8(u8 c);
void hex16(u16 c);
void hex32(u32 c);

void timercheck(void);

void db(void);
void ice(int argc, uint64_t *args);

void sio_putc(u8 data);
u8 sio_getc(void);
void sio_putstring(const char *string);
void sio_put8(u8 data);
void sio_put16(u16 data);
void sio_put32(u32 data);
u8 sio_get_nibble(void);
u8 sio_get8(void);
u16 sio_get16(void);
u32 sio_get32(void);
