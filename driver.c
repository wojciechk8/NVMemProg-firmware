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
 * driver.c
 *
 */


#include <i2c.h>
#include <fx2types.h>

#include "driver.h"


#define ID_EEPROM_ADDR 0x57


BOOL driver_read_id(BYTE *id, BYTE len)
{
  BYTE mem_addr = 0;

  if(!i2c_write(ID_EEPROM_ADDR, 1, &mem_addr, 0, NULL)){
    return FALSE;
  }
  if(!i2c_read(ID_EEPROM_ADDR, len, id)){
    return FALSE;
  }

  return TRUE;
}

BOOL driver_write_id(BYTE *id, BYTE len)
{
  BYTE mem_addr = 0;

  if(!i2c_write(ID_EEPROM_ADDR, 1, &mem_addr, len, id)){
    return FALSE;
  }

  return TRUE;
}
