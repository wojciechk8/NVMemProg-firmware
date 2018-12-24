/*
 * Copyright (C) 2018 Wojciech Krutnik <wojciechk8@gmail.com>
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
 * logic.c
 * Interface module for logic analyser
 *
 */

#include <fx2regs.h>
#include <fx2types.h>

#include "../fpga.h"
#include "../utils.h"
#include "module.h"


#define SYNCDELAY SYNCDELAY3


const char fw_signature[FW_SIGNATURE_SIZE] = "logic";

enum IFC_STATE{
  STATE_IDLE,
  STATE_RUNNING,
}state;

const __code BYTE wave_data[128] =
{
// Wave 0
/* LenBr */ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 1
/* LenBr */ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 2
/* LenBr */ 0x01,     0x01,     0x01,     0x03,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x02,     0x00,     0x02,     0x01,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x00,     0x01,     0x01,     0x00,     0x00,     0x00,     0x00,     0x00,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x3F,
// Wave 3
/* LenBr */ 0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x01,     0x07,
/* Opcode*/ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,
/* Output*/ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,
/* LFun  */ 0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x00,     0x3F
};

static volatile __bit busy = FALSE;


//******************************************************************************
// Private Functions
//******************************************************************************



//******************************************************************************
// Public Functions
//******************************************************************************

void ifc_init(void)
{
  // GPIF config
  IFCONFIG = bmIFCLKSRC // internal IFCLK
             |bmIFGPIF; // ports in GPIF master mode

  GPIFABORT = 0xFF;       // abort any waveforms pending

  GPIFREADYCFG = 0xC0;
  GPIFCTLCFG = 0x00;
  GPIFIDLECS = 0x00;
  GPIFIDLECTL = 0x00;
  GPIFWFSELECT = 0x4E;
  GPIFREADYSTAT = 0x00;
  GPIFHOLDAMOUNT = 0x01;  // 1/2 IFCLK hold time

  // Load WaveData
  LOAD_AUTOPTR1(wave_data);
  LOAD_AUTOPTR2(GPIF_WAVE_DATA);
  AUTOPTR_TRANSFER(sizeof(wave_data));

  // Don't use flowstates
  FLOWSTATE = 0x00;
}


// Data pointed by the autopointer 1
BOOL ifc_set_config(IFC_CFG_TYPE type, WORD param, BYTE data_len)
{
  data_len;

  switch(type){
    default:
      return FALSE;
  }

  return TRUE;
}


BOOL ifc_read_id(IFC_ID_TYPE type, BYTE *id)
{
  type;
  id;
  return FALSE;
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

  // Init Transaction Counter (not used, but must be set to start the waveform)
  GPIFTCB0 = 0x01;       SYNCDELAY;

  // Reset EP6 FIFO
  FIFORESET = bmNAKALL;  SYNCDELAY;
  FIFORESET = 0x06;      SYNCDELAY;
  // Switch the FIFO to auto mode
  EP6FIFOCFG = bmAUTOIN; SYNCDELAY;

  GPIFTRIG = bmBIT2 | 0x2;

  state = STATE_RUNNING;
  busy = TRUE;

  return TRUE;
}


BOOL ifc_prepare_write(void)
{
    return FALSE;
}


BOOL ifc_busy(void)
{
  return busy;
}


void ifc_abort(void)
{
  GPIFABORT = 0xFF;   // abort any waveforms pending
  GPIFIDLECTL = 0x00; // reset control signals

  // Switch FIFOs to manual mode
  FIFORESET = bmNAKALL; SYNCDELAY;
  EP6FIFOCFG = 0x00;    SYNCDELAY;
  FIFORESET = 0x00;

  state = STATE_IDLE;
  busy = FALSE;
}


void ifc_process(void)
{
  return;
}
