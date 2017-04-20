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
 * fpga.h
 *
 */

#pragma once

#include <fx2types.h>

#include "gpio.h"


/* FPGA registers
 *
 * mem_mux (addr 0..47):
 *   0-15: data_fx [0-15]
 *   16-24: addr_fx [0-8]
 *   25-29: ctl_fx [0-4]
 *   30: '0'
 *   31: '1'
 *   
 *   The MSB (bit 5) is output enable
 *   (the actual OE depends also on data_dir(ctl5),
 *    if selected signal is data_fx)
 *
 * rdy_fx_mux (addr 48..49):
 *   0-47: mem_pin [0:47]
 * 
 * reset (addr >= 64):
 *   writing anything to this address will reset the OE bit of all
 *   memory pins
 */
typedef union{
  struct{
    BYTE mem_mux[48];
    BYTE rdy_fx_mux[2];
    BYTE reserved[14];
    BYTE reset;
  };
  BYTE reg[65];
}FPGA_REGS;

#define FPGA_REG_NUM sizeof(FPGA_REGS)
extern volatile __xdata __at 0x4000 FPGA_REGS fpga_regs;


typedef enum{
  FPGA_STATUS_UNCONFIGURED,
  FPGA_STATUS_CONFIGURING,
  FPGA_STATUS_CONFIGURED
}FPGA_CFG_STATUS;



inline void fpga_init(void)
{
  GPIO_FPGA_CONFIG_UNSET();

  // Serial0 mode 0, CLKOUT/4
  //SCON0 = bmBIT5;   // SM2 = 1; TI = 0
  SCON0 = 0;
}


BOOL fpga_start_config();
BOOL fpga_write_config(BYTE *data, BYTE len);
FPGA_CFG_STATUS fpga_get_status();


inline void fpga_reset_regs()
{
  fpga_regs.reset = 0xFF;
}

