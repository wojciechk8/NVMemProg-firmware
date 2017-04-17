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
 * power.h
 *
 */

#pragma once

#include <fx2types.h>
#include <stdint.h>


typedef enum{
  PWR_CH_VPP=0x0,
  PWR_CH_VCC=0x1,
  PWR_CH_IPP=0x2,
  PWR_CH_ICC=0x3
}PWR_CH;


void pwr_init(void);

// Set raw value to DAC and dependently on store arg, save it in EEPROM
BOOL pwr_set_dac(PWR_CH ch, WORD val, BYTE store);

// current in 2mA units for ICC and 1mA units for IPP
BOOL pwr_set_current(PWR_CH ch, BYTE current);

// Voltage in unsigned I4Q4 format
// Voltage slew rate:
//   VPP: 12.5mV/us / rate
//   VCC: 6.25mV/us / rate
// Direction of the ramp can be up or down, depending on the target
//   voltage
BOOL pwr_ramp_voltage(PWR_CH ch, BYTE voltage, int8_t rate);

// resets DAC (VCC=1.5V; VPP=7V)
// MUST BE CALLED at the initialization of the device
BOOL pwr_reset(void);

void pwr_switch_off(void);
void pwr_switch_on(void);
