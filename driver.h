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
 * driver.h
 *
 */

#pragma once

#include <fx2regs.h>
#include <fx2types.h>

#include "gpio.h"
#include "common.h"


void driver_init(void);
BOOL driver_read_id(BYTE *id, BYTE len);
BOOL driver_write_id(BYTE *id, BYTE len);
BOOL driver_config(DRIVER_CONFIG* config);
BOOL driver_config_pin(BYTE pin_num, DRIVER_PIN_CONFIG pin_cfg);
BOOL driver_enable(void);
void driver_disable(void);
