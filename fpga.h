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
 * fpga.h
 *
 */

#pragma once

#include <fx2types.h>

#include "gpio.h"
#include "common.h"


extern volatile __xdata __at 0x4000 BYTE fpga_regs[64];

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


BOOL fpga_start_config(void);
BOOL fpga_write_config(BYTE len); // Data pointed by the autopointer 1
FPGA_CFG_STATUS fpga_get_status(void);
