/*
 * Copyright (C) 2016 Wojciech Krutnik <wojciechk8@gmail.com>
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
 * 28f.c
 * Interface module for 28F... memories
 *
 */

#include <fx2regs.h>
#include <fx2types.h>

#include "../fpga.h"
#include "module.h"


#define SYNCDELAY SYNCDELAY3


enum MEMORY_CMD{
  CMD_AUTO_ERASE_CHIP=0x30,
  CMD_READ_ID=0x90,
  CMD_RESET=0xFF
};

enum IFC_STATE{
  STATE_IDLE,
  STATE_ERASE,
  STATE_READ_DATA,
  STATE_WRITE_DATA
}state;

const __code BYTE wave_data[128] =
{
// Wave 0
/* LenBr */ 0x08,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x02,     0x01,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x02,     0x02,     0x07,     0x07,     0x07,     0x07,     0x07,     0x07,
/* LFun  */ 0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 1
/* LenBr */ 0x05,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x02,     0x00,     0x01,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x24,     0x26,     0x07,     0x07,     0x07,     0x07,     0x07,     0x07,
/* LFun  */ 0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 2
/* LenBr */ 0x04,     0x01,     0x02,     0x3F,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x0A,     0x00,     0x01,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x03,     0x03,     0x07,     0x07,     0x07,     0x07,     0x07,     0x07,
/* LFun  */ 0x00,     0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x3F,
// Wave 3
/* LenBr */ 0x01,     0x05,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x0E,     0x02,     0x00,     0x01,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x27,     0x25,     0x27,     0x07,     0x07,     0x07,     0x07,     0x07,
/* LFun  */ 0x00,     0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x3F
};

__xdata BYTE hiaddr_map[16];
__xdata BYTE hiaddr_size;
__xdata WORD hiaddr;



// Private Functions ---------------------------------------------------

void update_hiaddr(void)
{
  BYTE i;
  register WORD tmp = hiaddr;
  __bit lb;

  __asm
    mov	_AUTOPTRH1,#(_hiaddr_map >> 8)
    mov	_AUTOPTRL1,#_hiaddr_map
  __endasm;
  for(i = 0x00; i < hiaddr_size; i++){
    lb = ((BYTE)hiaddr)&0x01;
    fpga_regs.reg[XAUTODAT1] = 0x3E | lb;
    hiaddr >>= 1;
  }
}

/*
BOOL poll_dq7(void)
{
  __bit actual_dq7;

  actual_dq7 = XGPIFSGLDATLX;   // trigger read sequence
  while(!(GPIFTRIG & bmDONE))
    ;
  actual_dq7 = XGPIFSGLDATLNOX & 0x80;

  return !(actual_dq7 ^ expected_dq7)
}
*/

BOOL poll_dq6(void)
{
  BYTE dq6, next_dq6;

  dq6 = GPIFSGLDATLX;   // trigger read sequence
  while(!(GPIFTRIG & bmDONE))
    ;
  dq6 = GPIFSGLDATLX & 0x40;
  while(!(GPIFTRIG & bmDONE))
    ;
  next_dq6 = GPIFSGLDATLNOX & 0x40;

  return !(dq6 ^ next_dq6);
}


// Public Functions ----------------------------------------------------

void ifc_init(void)
{
  BYTE i;

  // GPIF config
  IFCONFIG = bmIFCLKSRC|bm3048MHZ   // IFCLK = Internal 48MHz
             |bmIFGPIF;             // ports in GPIF master mode

  GPIFABORT = 0xFF;  // abort any waveforms pending

  GPIFREADYCFG = 0xC0;
  GPIFCTLCFG = 0x00;
  GPIFIDLECS = 0x00;
  GPIFIDLECTL = 0x07;
  GPIFWFSELECT = 0x4E;
  GPIFREADYSTAT = 0x00;
  GPIFHOLDAMOUNT = 0x01;  // 1/2 IFCLK hold time

  // Configure endpoints FIFOs
  EP2FIFOCFG = bmAUTOOUT;
  SYNCDELAY;
  EP6FIFOCFG = bmAUTOIN;
  // AUTOIN/OUT packet length is set to default size 512B after reset

  // Load WaveData
  __asm
    ; source
    mov	_AUTOPTRH1,#(_wave_data >> 8)
    mov	_AUTOPTRL1,#_wave_data
    ; destination
    mov	_AUTOPTRH2,#0xE4
    mov	_AUTOPTRL2,#0x00
  __endasm;

  // transfer
  for(i = 0x00; i < 128; i++){
    XAUTODAT2 = XAUTODAT1;
  }

  // Configure GPIF Address pins, output initial value,
  PORTCCFG = 0xFF;    // [7:0] as alt. func. GPIFADR[7:0]
  OEC = 0xFF;         // and as outputs
  PORTECFG |= 0x80;   // [8] as alt. func. GPIFADR[8]
  OEE |= 0x80;        // and as output

  // Don't use flowstates
  FLOWSTATE = 0x00;

  ifc_abort();
}


// Data pointed by the autopointer 1
BOOL ifc_set_config(IFC_CFG_TYPE type, BYTE param)
{
  BYTE i;

  switch(type){
    case IFC_CFG_ADDRESS_MAPPING:
      __asm
        ; destination
        mov	_AUTOPTRH2,#(_hiaddr_map >> 8)
        mov	_AUTOPTRL2,#_hiaddr_map
      __endasm;
      for(i = 0x00; i < param; i++){
        XAUTODAT2 = XAUTODAT1;
      }
      hiaddr_size = param;
      break;
  }

  return TRUE;
}


BOOL ifc_read_id(BYTE size, BYTE *id)
{
  BYTE dummy;

  size;

  if(state != STATE_IDLE){
    return FALSE;
  }

  GPIFSGLDATLX = CMD_READ_ID;
  while(!(GPIFTRIG & bmDONE))
    ;

  dummy = GPIFSGLDATLX;  // trigger read sequence
  while(!(GPIFTRIG & bmDONE))
    ;
  id[0] = GPIFSGLDATLX;

  GPIFADRL = 0x01;
  SYNCDELAY;
  while(!(GPIFTRIG & bmDONE))
    ;
  id[1] = GPIFSGLDATLNOX;

  ifc_abort();

  return TRUE;
}


BOOL ifc_erase_chip(void)
{
  if(state != STATE_IDLE){
    return FALSE;
  }

  GPIFSGLDATLX = CMD_AUTO_ERASE_CHIP;
  while(!(GPIFTRIG & bmDONE))
    ;
  GPIFSGLDATLX = CMD_AUTO_ERASE_CHIP;
  while(!(GPIFTRIG & bmDONE))
    ;

  state = STATE_ERASE;

  return TRUE;
}


BOOL ifc_prepare_read(void)
{
  if(state != STATE_IDLE){
    return FALSE;
  }

  SYNCDELAY;
  GPIFTCB1 = 0x01;          // Transaction Counter = 512B
  SYNCDELAY;
  GPIFTCB0 = 0xFF;

  GPIFIDLECTL = 0x06;       // enable CE#

  // Reset EP6 FIFO
  SYNCDELAY;
  EP6FIFOCFG = 0x00;        // switch to manual mode
  SYNCDELAY;
  FIFORESET = bmNAKALL;
  SYNCDELAY;
  FIFORESET = bmNAKALL|6;   // reset EP6 FIFO
  SYNCDELAY;
  FIFORESET = 0x00;
  SYNCDELAY;
  EP6FIFOCFG = bmAUTOIN;    // switch back to auto mode

  SYNCDELAY;
  GPIFTRIG = bmBIT2 | 0x6;  // trigger FIFO read transaction on EP6

  state = STATE_READ_DATA;

  return TRUE;
}


BOOL ifc_prepare_write(void)
{
  if(state != STATE_IDLE){
    return FALSE;
  }

  GPIFIDLECTL = 0x06;       // enable CE#

  // Starting from addr 0x1FF, because the address will be incremented
  // at the beggining of the waveform
  SYNCDELAY;
  GPIFADRH = 0x01;
  SYNCDELAY;
  GPIFADRL = 0xFF;

  // Reset EP2 FIFO
  SYNCDELAY;
  EP2FIFOCFG = 0x00;        // switch to manual mode
  SYNCDELAY;
  FIFORESET = bmNAKALL;
  SYNCDELAY;
  FIFORESET = bmNAKALL|2;   // reset EP2 FIFO
  SYNCDELAY;
  FIFORESET = 0x00;
  SYNCDELAY;
  EP2FIFOCFG = bmAUTOOUT;   // switch back to auto mode

  SYNCDELAY;
  GPIFTRIG = 0x2;           // trigger FIFO write transaction on EP2

  state = STATE_WRITE_DATA;

  return TRUE;
}


BOOL ifc_busy(void)
{
  return (state != STATE_IDLE);
}


WORD ifc_get_data_count(void)
{
  return hiaddr;
}


void ifc_abort(void)
{
  GPIFABORT = 0xFF;   // abort any waveforms pending
  GPIFIDLECTL = 0x07; // reset control signals

  // Reset memory command
  GPIFSGLDATLX = CMD_RESET;
  while(!(GPIFTRIG & bmDONE))
    ;
  GPIFSGLDATLX = CMD_RESET;
  while(!(GPIFTRIG & bmDONE))
    ;

  // Reset GPIF address
  SYNCDELAY;
  GPIFADRH = 0x00;
  SYNCDELAY;
  GPIFADRL = 0x00;

  // Reset high bits of the address
  hiaddr = 0x0000;
  update_hiaddr();

  // Reset Transaction Counter
  GPIFTCB0 = 0x00;
  SYNCDELAY;
  GPIFTCB1 = 0x00;

  state = STATE_IDLE;
}


void ifc_process(void)
{
  switch(state){
    case STATE_IDLE:
      return;

    case STATE_ERASE:
      if(poll_dq6()){     // Erase completed
        state = STATE_IDLE;
      }
      break;

    case STATE_READ_DATA:
      if(GPIFTRIG & bmDONE){
        hiaddr++;
        update_hiaddr();
        GPIFTCB1 = 0x01;  // Transaction Counter = 512B
        SYNCDELAY;
        GPIFTCB0 = 0xFF;
        SYNCDELAY;
        GPIFTRIG = bmBIT2 | 0x6; // trigger next transaction (read)
      }
      break;

    case STATE_WRITE_DATA:
      if(GPIFTRIG & bmDONE){
        if((GPIFADRH == 0x01) && (GPIFADRL == 0xFF)){
          hiaddr++;
          update_hiaddr();
        }
        GPIFTRIG = 0x2;   // trigger next transaction
      }
      break;
  }
}
