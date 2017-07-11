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
 * 28f.c
 * Interface module for 28F... memories
 *
 */

#include <fx2regs.h>
#include <fx2types.h>

#include "../fpga.h"
#include "module.h"


#define SYNCDELAY SYNCDELAY3


const char signature[SIGNATURE_SIZE] = "28f";

enum MEMORY_CMD{
  CMD_AUTO_ERASE_CHIP=0x30,
  CMD_AUTO_PROGRAM=0x40,
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
/* Opcode*/ 0x00,     0x02,     0x05,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x02,     0x02,     0x06,     0x06,     0x06,     0x06,     0x06,     0x06,
/* LFun  */ 0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 1
/* LenBr */ 0x05,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x02,     0x04,     0x01,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x24,     0x26,     0x06,     0x06,     0x06,     0x06,     0x06,     0x06,
/* LFun  */ 0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 2
/* LenBr */ 0x04,     0x01,     0x02,     0x3F,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x0A,     0x04,     0x01,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x02,     0x02,     0x06,     0x06,     0x06,     0x06,     0x06,     0x06,
/* LFun  */ 0x00,     0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x3F,
// Wave 3
/* LenBr */ 0x01,     0x05,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x0A,     0x02,     0x00,     0x05,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x26,     0x24,     0x26,     0x06,     0x06,     0x06,     0x06,     0x06,
/* LFun  */ 0x00,     0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x3F
};

__xdata BYTE hiaddr_map[16];
__xdata BYTE hiaddr_size;
__xdata WORD hiaddr;



//******************************************************************************
// Private Functions
//******************************************************************************

void update_hiaddr(void)
{
  BYTE i;
  WORD tmp = hiaddr;
  __bit lb;

  __asm
    mov	_AUTOPTRH1,#(_hiaddr_map >> 8)
    mov	_AUTOPTRL1,#_hiaddr_map
  __endasm;
  for(i = 0x00; i < hiaddr_size; i++){
    lb = ((BYTE)tmp)&0x01;
    fpga_regs.reg[XAUTODAT1] = 0x3E | lb;
    tmp >>= 1;
  }
}

/*
BOOL poll_dq7(void)
{
  __bit actual_dq7;

  actual_dq7 = XGPIFSGLDATLX;   // trigger read sequence
  while(!(GPIFTRIG & bmBIT7))
    ;
  actual_dq7 = XGPIFSGLDATLNOX & 0x80;

  return !(actual_dq7 ^ expected_dq7)
}
*/

BOOL poll_dq6(void)
{
  BYTE dq6, next_dq6;

  dq6 = GPIFSGLDATLX;   // trigger read sequence
  while(!(GPIFTRIG & bmBIT7))
    ;
  dq6 = GPIFSGLDATLX & 0x40;
  while(!(GPIFTRIG & bmBIT7))
    ;
  next_dq6 = GPIFSGLDATLNOX & 0x40;

  return !(dq6 ^ next_dq6);
}


//******************************************************************************
// Public Functions
//******************************************************************************

void ifc_init(void)
{
  BYTE i;

  // GPIF config
  IFCONFIG = bmIFCLKSRC|bm3048MHZ   // IFCLK = Internal 48MHz
             |bmIFGPIF;             // ports in GPIF master mode

  GPIFABORT = 0xFF;       // abort any waveforms pending

  GPIFREADYCFG = 0xC0;
  GPIFCTLCFG = 0x00;
  GPIFIDLECS = 0x00;
  GPIFIDLECTL = 0x07;
  GPIFWFSELECT = 0x4E;
  GPIFREADYSTAT = 0x00;
  GPIFHOLDAMOUNT = 0x01;  // 1/2 IFCLK hold time

  // Load WaveData
  __asm
    ; source
    mov	_AUTOPTRH1,#(_wave_data >> 8)
    mov	_AUTOPTRL1,#_wave_data
    ; destination
    mov	_AUTOPTRH2,#0xE4
    mov	_AUTOPTRL2,#0x00
  __endasm;
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

  GPIFIDLECTL = 0x06;   // enable CE#

  GPIFSGLDATLX = CMD_READ_ID;
  while(!(GPIFTRIG & bmBIT7))
    ;

  dummy = GPIFSGLDATLX;  // trigger read sequence
  while(!(GPIFTRIG & bmBIT7))
    ;
  id[0] = GPIFSGLDATLNOX;


  GPIFADRL = 0x01; SYNCDELAY;
  GPIFSGLDATLX = CMD_READ_ID;
  while(!(GPIFTRIG & bmBIT7))
    ;

  dummy = GPIFSGLDATLX;  // trigger read sequence
  while(!(GPIFTRIG & bmBIT7))
    ;
  id[1] = GPIFSGLDATLNOX;

  GPIFIDLECTL = 0x07;   // disable CE#

  ifc_abort();

  return TRUE;
}


BOOL ifc_erase_chip(void)
{
  if(state != STATE_IDLE){
    return FALSE;
  }

  GPIFSGLDATLX = CMD_AUTO_ERASE_CHIP;
  while(!(GPIFTRIG & bmBIT7))
    ;
  GPIFSGLDATLX = CMD_AUTO_ERASE_CHIP;
  while(!(GPIFTRIG & bmBIT7))
    ;

  state = STATE_ERASE;

  return TRUE;
}


BOOL ifc_prepare_read(void)
{
  if(state != STATE_IDLE){
    return FALSE;
  }

  // Init Transaction Counter to 512B
  GPIFTCB1 = 0x02;       SYNCDELAY;
  GPIFTCB0 = 0x00;       SYNCDELAY;

  // enable CE#
  GPIFIDLECTL = 0x06;    SYNCDELAY;

  // Reset EP6 FIFO
  FIFORESET = bmNAKALL;  SYNCDELAY;
  FIFORESET = 0x06;      SYNCDELAY;
  // Switch the FIFO to auto mode
  EP6FIFOCFG = bmAUTOIN; SYNCDELAY;

  GPIFTRIG = bmBIT2 | 0x2;

  state = STATE_READ_DATA;

  return TRUE;
}


BOOL ifc_prepare_write(void)
{
  if(state != STATE_IDLE){
    return FALSE;
  }

  // Init Transaction Counter
  GPIFTCB1 = 0x00;       SYNCDELAY;
  GPIFTCB0 = 0x01;       SYNCDELAY;

  // Starting from the highest address, because will be incremented
  // at the beggining of the waveform
  GPIFADRH = 0x01;       SYNCDELAY;
  GPIFADRL = 0xFF;
  hiaddr = 0xFFFF;

  // Enable CE#
  GPIFIDLECTL = 0x06;    SYNCDELAY;

  // Reset EP2 FIFO
  FIFORESET = bmNAKALL;  SYNCDELAY;
  FIFORESET = 0x82;      SYNCDELAY;
  FIFORESET = 0x00;      SYNCDELAY;

  // Arm both EP2 buffers
  OUTPKTEND = 0x82;      SYNCDELAY;
  OUTPKTEND = 0x82;      SYNCDELAY;
  // Switch the FIFO to auto mode
  EP2FIFOCFG = bmAUTOOUT;

  state = STATE_WRITE_DATA;

  return TRUE;
}


BOOL ifc_busy(void)
{
  return (state != STATE_IDLE)
         && ((state != STATE_WRITE_DATA) || !(EP24FIFOFLGS & bmBIT1));
}


void ifc_abort(void)
{
  GPIFABORT = 0xFF;   // abort any waveforms pending
  GPIFIDLECTL = 0x07; // reset control signals

  // Reset GPIF address
  GPIFADRH = 0x00; SYNCDELAY;
  GPIFADRL = 0x00; SYNCDELAY;

  // Reset high bits of the address
  hiaddr = 0x0000;
  update_hiaddr();

  // Reset memory command
  GPIFIDLECTL = 0x06;   // enable CE#
  GPIFSGLDATLX = CMD_RESET;
  while(!(GPIFTRIG & bmBIT7))
    ;
  GPIFSGLDATLX = CMD_RESET;
  while(!(GPIFTRIG & bmBIT7))
    ;
  GPIFIDLECTL = 0x07;   // disable CE#

  // Switch FIFOs to manual mode
  FIFORESET = bmNAKALL; SYNCDELAY;
  EP2FIFOCFG = 0x00;    SYNCDELAY;
  EP6FIFOCFG = 0x00;    SYNCDELAY;
  FIFORESET = 0x00;

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
      if(GPIFTRIG & bmBIT7){
        hiaddr++;
        update_hiaddr();
        // Reload Transaction Counter
        GPIFTCB1 = 0x02; SYNCDELAY;
        GPIFTCB0 = 0x00; SYNCDELAY;
        GPIFTRIG = bmBIT2 | 0x2; // trigger next transaction (read)
      }
      break;

    case STATE_WRITE_DATA:
      if(GPIFTRIG & bmBIT7){  // if GPIF done
        if(EP24FIFOFLGS & bmBIT1) // if EP2FIFO empty
          break;

        if(!poll_dq6())
          break;

        if((GPIFADRH == 0x01) && (GPIFADRL == 0xFF)){
          hiaddr++;
          update_hiaddr();
        }

        GPIFSGLDATLX = CMD_AUTO_PROGRAM;  // program byte command
        while(!(GPIFTRIG & bmBIT7))
          ;

        GPIFTCB0 = 0x01; SYNCDELAY;

        GPIFTRIG = 0x0;   // trigger EP2 write data transaction
      }
      break;
  }
}
