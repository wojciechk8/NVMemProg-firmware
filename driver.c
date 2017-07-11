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

#include "gpio.h"
#include "driver.h"


#define ID_EEPROM_ADDR 0x57


static __bit configured = 0;


BOOL driver_read_id(BYTE *id)
{
  BYTE mem_addr = 0;

  if(!i2c_write(ID_EEPROM_ADDR, 1, &mem_addr, 0, NULL)){
    return FALSE;
  }
  if(!i2c_read(ID_EEPROM_ADDR, 1, id)){
    return FALSE;
  }

  return TRUE;
}


// Data pointed by the autopointer 1
BOOL driver_write_config(BYTE len)
{
  BYTE temp;
  
  if(GPIO_DRIVER_EN_STATE()){
    return FALSE;
  }

  // Set cpu frequency to 12MHz, so that serial baud is limited to 1MHz
  temp = CPUCS;
  CPUCS = (temp & ~bmCLKSPD);
  while(len--){
    while(!TI1)
      ;
    TI1 = 0;
    SBUF1 = XAUTODAT1;
  }
  CPUCS = temp;

  configured = TRUE;
  return TRUE;
}


// Enabling allowed only after configuration.
BOOL driver_enable(void)
{
  if(!configured){
    return FALSE;
  }

  GPIO_DRIVER_EN_SET();

  return TRUE;
}


void driver_disable(void)
{
  GPIO_DRIVER_EN_UNSET();
}


void driver_reset_status(void)
{
  configured = 0;
}
