/*
 * Copyright (C) 2017 Wojciech Krutnik <wojciechk8@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * utils.h
 *
 */

#pragma once

// NOTE: these autopointer loaders work only for global variables (and probably only in sdcc)
#define LOAD_AUTOPTR1(ptr) __asm__("mov _AUTOPTRH1,#(_"#ptr" >> 8)\nmov _AUTOPTRL1,#_"#ptr)
#define LOAD_AUTOPTR2(ptr) __asm__("mov _AUTOPTRH2,#(_"#ptr" >> 8)\nmov _AUTOPTRL2,#_"#ptr)
#define AUTOPTR_TRANSFER(len) do{                                      \
    BYTE i;                                                            \
    for(i = 0; i < len; i++)                                           \
      XAUTODAT2 = XAUTODAT1;                                           \
  }while(0)

#define SETUP_VALUE_LSB() (SETUPDAT[2])
#define SETUP_VALUE_MSB() (SETUPDAT[3])
#define SETUP_INDEX_LSB() (SETUPDAT[4])
#define SETUP_INDEX_MSB() (SETUPDAT[5])
#define SETUP_LENGTH_LSB() (SETUPDAT[6])
#define SETUP_LENGTH_MSB() (SETUPDAT[7])

#define WAIT_FOR_EP0_DATA() do{                                        \
    EP0BCL = 0; SYNCDELAY;    /* arm EP0 */                            \
    while (EP0CS & bmEPBUSY)  /* wait for OUT data */                  \
      ;                                                                \
  }while(0)

#define GPIF_INT_READY_SET() GPIFREADYCFG |= 0x80
#define GPIF_INT_READY_UNSET() GPIFREADYCFG &= ~0x80
#define IS_GPIF_DONE() (GPIFTRIG & bmBIT7)
#define WAIT_FOR_GPIF_DONE() do{ while(!IS_GPIF_DONE()); }while(0)

#define IS_EP2_FIFO_EMPTY() (EP24FIFOFLGS & bmBIT1)

