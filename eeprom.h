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
 * eeprom.h
 *
 */

#pragma once

#define EEPROM_ADDR 0x51

enum{
  EEPROM_HEADER=0x00,
  EEPROM_VID=0x01,
  EEPROM_PID=0x03,
  EEPROM_DID=0x05,
  EEPROM_CONFIG=0x07,
  EEPROM_CALIB_HEADER=0x08,
  EEPROM_CALIB_A=0x0A,
  EEPROM_CALIB_B=0x10
};
