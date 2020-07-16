//* SPDX-License-Identifier: GPL-2.0-only */

#ifndef SERIALICE_H
#define SERIALICE_H

#define ECHO_MODE	1
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
#endif
