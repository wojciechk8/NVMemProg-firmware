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


#define CMD_READ_ID 0x90

#define SYNCDELAY SYNCDELAY3
                
const __code BYTE wave_data[96] =
{
// Wave 0
/* LenBr */ 0x08,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x02,     0x01,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x02,     0x02,     0x06,     0x06,     0x06,     0x06,     0x06,     0x06,
/* LFun  */ 0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 1
/* LenBr */ 0x05,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x02,     0x00,     0x01,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x24,     0x26,     0x06,     0x06,     0x06,     0x06,     0x06,     0x06,
/* LFun  */ 0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 2
/* LenBr */ 0x04,     0x01,     0x02,     0x3F,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x0A,     0x00,     0x01,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x02,     0x02,     0x06,     0x06,     0x06,     0x06,     0x06,     0x06,
/* LFun  */ 0x00,     0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x3F,
};

__xdata BYTE addr_map[20-8];


void ifc_reset(void)
{
  BYTE i;
  
  GPIFABORT = 0xFF;  // abort any waveforms pending
  
  // GPIF address pins update when GPIFADRH/L written
  GPIFADRH = 0x00;    // bits[7:1] always 0
  SYNCDELAY;
  GPIFADRL = 0x00;    // point to PERIPHERAL address 0x0000
  
  __asm
    mov	_AUTOPTRH1,(#_addr_map >> 8)
    mov	_AUTOPTRL1,#_addr_map
  __endasm;
  for(i = 0x00; i < sizeof(addr_map); i++){
    fpga_regs.reg[XAUTODAT1] = 0x1E;    // 0x1E = low level on pin
  }
}


void ifc_init(void)
{
  BYTE i;
  
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
  
  // source
  __asm
    ; source
    mov	_AUTOPTRH1,(#_wave_data >> 8)
    mov	_AUTOPTRL1,#_wave_data
    ; destination
    mov	_AUTOPTRH2,#0xE4
    mov	_AUTOPTRL2,#0x00
  __endasm;
  
  // transfer
  for(i = 0x00; i < 96; i++){
    XAUTODAT2 = XAUTODAT1;
  }
  
  // Configure GPIF Address pins, output initial value,
  PORTCCFG = 0xFF;    // [7:0] as alt. func. GPIFADR[7:0]
  OEC = 0xFF;         // and as outputs
  PORTECFG |= 0x80;   // [8] as alt. func. GPIFADR[8]
  OEE |= 0x80;        // and as output
  
  // Don't use flowstates
  FLOWSTATE = 0x00;
  
  ifc_reset();
}


// Data pointed by the autopointer 1
BOOL ifc_set_config(IFC_CFG_TYPE type)
{
  BYTE i;
  
  switch(type){
    case IFC_CFG_ADDRESS_MAPPING:
      __asm
        ; destination
        mov	_AUTOPTRH2,(#_addr_map >> 8)
        mov	_AUTOPTRL2,#_addr_map
      __endasm;
      for(i = 0x00; i < sizeof(addr_map); i++){
        XAUTODAT2 = XAUTODAT1;
      }
      break;
  }
}


BOOL ifc_read_id(BYTE *id)
{
  BYTE dummy;
  
  GPIFIDLECTL = 0x06;   // enable CE#
  
  XGPIFSGLDATLX = CMD_READ_ID;
  while(!(GPIFTRIG & bmDONE))
    ;
  
  dummy = XGPIFSGLDATLX;
  while(!(GPIFTRIG & bmDONE))
    ;
  id[0] = XGPIFSGLDATLX;
  
  GPIFADRL = 0x01;
  SYNCDELAY;
  while(!(GPIFTRIG & bmDONE))
    ;
  id[1] = XGPIFSGLDATLNOX;
  
  GPIFIDLECTL = 0x07;   // disable CE#
}
 
