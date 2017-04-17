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
 * driver.h
 *
 */

#pragma once

#include <fx2regs.h>
#include <fx2types.h>

#include "gpio.h"


inline void driver_init(void)
{
  GPIO_DRIVER_EN_UNSET();

  // Serial1 mode 0, CLKOUT/12
  SCON1 = bmBIT1;   // TI = 1
}


BOOL driver_read_id(BYTE *id);
BOOL driver_write_config(BYTE len, BYTE *data);
// Enabling allowed only after configuration.
BOOL driver_enable(void);
void driver_disable(void);
void driver_reset_status(void);
