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
 * speedtest.c
 * Data transfer speed test
 *
 */

#include <fx2regs.h>
#include <fx2types.h>

#include "../fpga.h"
#include "../utils.h"
#include "module.h"


#define SYNCDELAY SYNCDELAY3


const char fw_signature[FW_SIGNATURE_SIZE] = "speedtest";

enum IFC_STATE{
  STATE_IDLE,
  STATE_RUNNING,
}state;

static volatile __bit busy = FALSE;


//******************************************************************************
// Private Functions
//******************************************************************************



//******************************************************************************
// Public Functions
//******************************************************************************

void ifc_init(void)
{

}


// Data pointed by the autopointer 1
BOOL ifc_set_config(IFC_CFG_TYPE type, WORD param, BYTE data_len)
{
  type;
  param;
  data_len;
  return FALSE;
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

  // Reset EP6 FIFO
  FIFORESET = bmNAKALL;       SYNCDELAY;
  FIFORESET = bmNAKALL|0x06;  SYNCDELAY;
  FIFORESET = 0x00;           SYNCDELAY;

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
  state = STATE_IDLE;
  busy = FALSE;
}


void ifc_process(void)
{
  if (state == STATE_RUNNING) {
    if (!IS_EP6_FIFO_FULL()) {
      // Arm EP6 (send 512 bytes of dummy data to the host)
      EP6BCH = 0x02;  SYNCDELAY;
      EP6BCL = 0x00;  SYNCDELAY;
    }
  }
  return;
}
