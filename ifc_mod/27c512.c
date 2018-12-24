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
 * 27c512.c
 * Interface module for *27C512 memories
 * 27C512 has common OE#/VPP pin; it's not possible to switch from VPP
 * to low state, so there is no way to verify programmed byte while
 * programming. This implies constant (max.) byte programming time.
 *
 */

#include <fx2regs.h>
#include <fx2types.h>

#include "../fpga.h"
#include "../delay_us.h"
#include "../utils.h"
#include "module.h"


#define SYNCDELAY SYNCDELAY3


const char fw_signature[FW_SIGNATURE_SIZE] = "27c512";

enum MEMORY_CMD{
  CMD_AUTO_ERASE_CHIP=0x30,
  CMD_AUTO_PROGRAM=0x40,
  CMD_READ_ID=0x90,
  CMD_RESET=0xFF
};

enum IFC_STATE{
  STATE_IDLE,
  STATE_READ_DATA,
  STATE_WRITE_DATA
}state;

const __code BYTE wave_data[128] =
{
// Wave 0
/* LenBr */ 0x08,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x02,     0x05,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x00,     0x00,     0x04,     0x04,     0x04,     0x04,     0x04,     0x04,
/* LFun  */ 0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 1
/* LenBr */ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x04,     0x04,     0x04,     0x04,     0x04,     0x04,     0x04,     0x04,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 2
/* LenBr */ 0x06,     0x01,     0x01,     0x3F,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x0A,     0x04,     0x01,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x00,     0x00,     0x00,     0x04,     0x04,     0x04,     0x04,     0x04,
/* LFun  */ 0x00,     0x00,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x3F,
// Wave 3
/* LenBr */ 0x64,     0x11,     0x64,     0x3F,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x0A,     0x03,     0x02,     0x05,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x25,     0x24,     0x25,     0x05,     0x05,     0x05,     0x05,     0x04,
/* LFun  */ 0x00,     0x3F,     0x00,     0x3F,     0x00,     0x00,     0x00,     0x3F
};

__xdata BYTE hiaddr_map[16];
__xdata BYTE hiaddr_size;
__xdata WORD hiaddr;

static volatile __bit busy = FALSE;

// CE# low time in program-cycle (100us units)
// (TODO: make this configurable via ifc_set_config)
#define PROGRAM_PULSE_DURATION 5

static BYTE program_pulse_cnt;

//******************************************************************************
// Private Functions
//******************************************************************************

void update_hiaddr(void)
{
  BYTE i;
  WORD tmp = hiaddr;
  __bit lb;

  LOAD_AUTOPTR1(hiaddr_map);
  for(i = 0x00; i < hiaddr_size; i++){
    lb = ((BYTE)tmp)&0x01;
    ((FPGA_UNIV_REGISTERS*)fpga_regs)->mem_mux_selector[XAUTODAT1] = FPGA_UNIV_MUX_ENABLE | FPGA_UNIV_MUX_LOW | lb;
    tmp >>= 1;
  }
}


//******************************************************************************
// Public Functions
//******************************************************************************

void ifc_init(void)
{
  // GPIF config
  IFCONFIG = bmIFCLKSRC|bm3048MHZ   // IFCLK = Internal 48MHz
             |bmIFGPIF;             // ports in GPIF master mode

  GPIFABORT = 0xFF;       // abort any waveforms pending

  GPIFREADYCFG = 0xC0;
  GPIFCTLCFG = 0x00;
  GPIFIDLECS = 0x00;
  GPIFIDLECTL = 0x05;
  GPIFWFSELECT = 0x4E;
  GPIFREADYSTAT = 0x00;
  GPIFHOLDAMOUNT = 0x01;  // 1/2 IFCLK hold time

  // Load WaveData
  LOAD_AUTOPTR1(wave_data);
  LOAD_AUTOPTR2(GPIF_WAVE_DATA);
  AUTOPTR_TRANSFER(sizeof(wave_data));

  // Configure GPIF Address pins, output initial value,
  PORTCCFG = 0xFF;    // [7:0] as alt. func. GPIFADR[7:0]
  OEC = 0xFF;         // and as outputs
  PORTECFG |= 0x80;   // [8] as alt. func. GPIFADR[8]
  OEE |= 0x80;        // and as output

  // Don't use flowstates
  FLOWSTATE = 0x00;
}


// Data pointed by the autopointer 1
BOOL ifc_set_config(IFC_CFG_TYPE type, WORD param, BYTE data_len)
{
  switch(type){
    case IFC_CFG_ADDRESS_PIN_MAPPING:
      LOAD_AUTOPTR2(hiaddr_map);
      AUTOPTR_TRANSFER(data_len);
      hiaddr_size = data_len;
      break;
  }

  return TRUE;
}


BOOL ifc_read_id(IFC_ID_TYPE type, BYTE *id)
{
  BYTE dummy;

  if(state != STATE_IDLE){
    return FALSE;
  }

  switch (type) {
    case IFC_ID_MANUFACTURER:
      GPIFADRL = 0x00; SYNCDELAY;
      break;
    case IFC_ID_DEVICE:
      GPIFADRL = 0x01; SYNCDELAY;
      break;
    default:
      return FALSE;
  }

  dummy = GPIFSGLDATLX;  // trigger read sequence
  while(!IS_GPIF_DONE())
    ;
  id[0] = GPIFSGLDATLNOX;

  ifc_abort();

  return TRUE;
}


BOOL ifc_erase_chip(void)
{
  return FALSE;
}


BOOL ifc_prepare_read(void)
{
  if(state != STATE_IDLE){
    return FALSE;
  }

  // Init Transaction Counter to 512B
  GPIFTCB1 = 0x02;       SYNCDELAY;
  GPIFTCB0 = 0x00;       SYNCDELAY;

  // Enable CE#
  GPIFIDLECTL = 0x04;    SYNCDELAY;

  // Reset EP6 FIFO
  FIFORESET = bmNAKALL;  SYNCDELAY;
  FIFORESET = 0x06;      SYNCDELAY;
  // Switch the FIFO to auto mode
  EP6FIFOCFG = bmAUTOIN; SYNCDELAY;

  GPIFTRIG = bmBIT2 | 0x2;

  state = STATE_READ_DATA;
  busy = TRUE;

  return TRUE;
}


BOOL ifc_prepare_write(void)
{
  if(state != STATE_IDLE){
    return FALSE;
  }

  // Timer 0
  TMOD = 0x02;    // Mode 2 (8bit with auto-reload); timer
  CKCON = 0x01;   // Timer source: CLKOUT/12 (4MHz)
  ET0 = 0;        // Disable interrupt

  // Init Transaction Counter
  GPIFTCB1 = 0x00;       SYNCDELAY;
  GPIFTCB0 = 0x01;       SYNCDELAY;

  // Starting from the highest address, because will be incremented
  // at the beggining of the waveform
  GPIFADRH = 0x01;       SYNCDELAY;
  GPIFADRL = 0xFF;
  hiaddr = 0xFFFF;

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
  program_pulse_cnt = 0;

  return TRUE;
}


BOOL ifc_busy(void)
{
  return busy;
}


void ifc_abort(void)
{
  GPIFABORT = 0xFF;   // abort any waveforms pending
  GPIFIDLECTL = 0x05; // reset control signals

  // Reset GPIF address
  GPIFADRH = 0x00; SYNCDELAY;
  GPIFADRL = 0x00; SYNCDELAY;

  // Reset high bits of the address
  hiaddr = 0x0000;
  update_hiaddr();

  // Switch FIFOs to manual mode
  FIFORESET = bmNAKALL; SYNCDELAY;
  EP2FIFOCFG = 0x00;    SYNCDELAY;
  EP6FIFOCFG = 0x00;    SYNCDELAY;
  FIFORESET = 0x00;

  state = STATE_IDLE;
  busy = FALSE;
}


void ifc_process(void)
{
  switch(state){
    case STATE_IDLE:
      return;

    case STATE_READ_DATA:
      if(IS_GPIF_DONE()){
        hiaddr++;
        update_hiaddr();
        // Reload Transaction Counter
        GPIFTCB1 = 0x02; SYNCDELAY;
        GPIFTCB0 = 0x00; SYNCDELAY;
        GPIFTRIG = bmBIT2 | 0x2; // trigger next transaction (read)
      }
      break;

    case STATE_WRITE_DATA:
      if(IS_GPIF_DONE()){
        if(IS_EP2_FIFO_EMPTY()){
          busy = FALSE;
          break;
        }

        if((GPIFADRH == 0x01) && (GPIFADRL == 0xFF)){
          hiaddr++;
          update_hiaddr();
        }

        GPIF_INT_READY_UNSET();
        GPIFTCB0 = 0x01; SYNCDELAY; // 1 transaction
        GPIFTRIG = 0x0;             // trigger EP2 write data transaction
        busy = TRUE;

        // Timer
        TH0 = 256 - 200; TL0 = 256 - 200; // 200 cycles = 50us
        TF0 = 0;  // clear overflow flag
        TR0 = 1;  // start timer
      }
      if(TF0){  // 50us elapsed
        TF0 = 0;  // clear overflow flag
        TH0 = 256 - 200; TL0 = 256 - 200; // 200 cycles = 50us
        if(++program_pulse_cnt == PROGRAM_PULSE_DURATION*2){
          GPIF_INT_READY_SET();
          program_pulse_cnt = 0;
          TR0 = 0;  // stop timer
        }
      }
      break;
  }
}
