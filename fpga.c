/*
 * Copyright (C) 2016, 2017 Wojciech Krutnik <wojciechk8@gmail.com>
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
 * fpga.c
 *
 */


#include <fx2types.h>

#include "delay_us.h"
#include "gpio.h"
#include "fpga.h"


volatile __xdata __at 0x4000 FPGA_REGS fpga_regs;

static FPGA_CFG_STATUS status;


// Altera Cyclone PS Configuration (AN 250)

BOOL fpga_start_config()
{
  BYTE timeout = 20;

  GPIO_FPGA_CONFIG_SET();
  delay_us(41);
  GPIO_FPGA_CONFIG_UNSET();

  if(!GPIO_FPGA_STATUS_STATE()){
    return FALSE;
  }

  while(GPIO_FPGA_STATUS_STATE()){
    if(!--timeout){
      return FALSE;
    }
  }

  status = FPGA_STATUS_CONFIGURING;
  return TRUE;
}


// Data pointed by the autopointer 1
BOOL fpga_write_config(BYTE len)
{
  while(len--){
    TI = 0;
    SBUF0 = XAUTODAT1;
    if(GPIO_FPGA_STATUS_STATE()){
      status = FPGA_STATUS_UNCONFIGURED;
      return FALSE;
    }
    while(!TI)
      ;
  }

  if(GPIO_FPGA_CONFDONE_STATE()){
    delay_us(100);
    status = FPGA_STATUS_CONFIGURED;
  }

  return TRUE;
}


FPGA_CFG_STATUS fpga_get_status(void)
{
  return status;
}

