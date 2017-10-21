/*
 * Copyright (C) 2017 Wojciech Krutnik <wojciechk8@gmail.com>
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
 * driver_default.c
 *
 */


#include <i2c.h>
#include <fx2types.h>
#include <string.h>

#include "gpio.h"
#include "delay_us.h"
#include "driver.h"
#include "utils.h"


#define RESET_CFG      DRIVER_PIN_CONFIG_IO|DRIVER_PIN_CONFIG_PULL_UP_DISABLE
#define RESET_CFG_2X   RESET_CFG, RESET_CFG
#define RESET_CFG_4X   RESET_CFG_2X, RESET_CFG_2X
#define RESET_CFG_8X   RESET_CFG_4X, RESET_CFG_4X
#define RESET_CFG_16X  RESET_CFG_8X, RESET_CFG_8X
#define RESET_CFG_32X  RESET_CFG_16X, RESET_CFG_16X
const __code DRIVER_CONFIG reset_cfg = {{RESET_CFG_32X, RESET_CFG_16X}};
static __xdata DRIVER_CONFIG current_config;

typedef struct{
  BYTE data[18];
}CONFIG_SERIALIZED;
__xdata CONFIG_SERIALIZED config_ser;

#define SERIALIZE_CONFIG(in, out)                                      \
  do{                                                                  \
    BYTE i;                                                            \
    DWORD out_buf = 0;                                                 \
    BYTE out_cnt = 0;                                                  \
    for(i = 48; i != 0; i--){                                          \
      BYTE in_buf = (in)->pin_config[i-1];                             \
      out_buf >>= 1;                                                   \
      out_buf |= (in_buf&0x4) ? 0x800000 : 0x000000;                   \
      in_buf <<= 1; out_buf >>= 1;                                     \
      out_buf |= (in_buf&0x4) ? 0x800000 : 0x000000;                   \
      in_buf <<= 1; out_buf >>= 1;                                     \
      out_buf |= (in_buf&0x4) ? 0x800000 : 0x000000;                   \
      if(((i-1) & 0x7) == 0){                                          \
        (out)->data[out_cnt++] = out_buf&0x0000FF;                     \
        (out)->data[out_cnt++] = (out_buf&0x00FF00)>>8;                \
        (out)->data[out_cnt++] = (out_buf&0xFF0000)>>16;               \
        out_buf = 0;                                                   \
      }                                                                \
    }                                                                  \
  }while(0)


void driver_init(void)
{
  GPIO_DRIVER_EN_UNSET();

  // Serial1 mode 0, CLKOUT/12
  SCON1 = 0;
  
  driver_config(&reset_cfg);
}


BOOL driver_config(DRIVER_CONFIG* config)
{
  BYTE temp, i;
  
  if(GPIO_DRIVER_EN_STATE())
    return FALSE;
  
  if(GPIO_PWRSW_STATE())
    return FALSE;
  
  SERIALIZE_CONFIG(config, &config_ser);

  LOAD_AUTOPTR1(config_ser);
  // Set cpu frequency to 12MHz, so that serial baud is limited to 1MHz
  temp = CPUCS;
  CPUCS = (temp & ~bmCLKSPD);
  for(i = 0; i < sizeof(CONFIG_SERIALIZED); i++){
    TI1 = 0;
    SBUF1 = XAUTODAT1;
    while(!TI1)
      ;
  }
  delay_us(10);
  CPUCS = temp;
  
  for(i = 0; i < sizeof(DRIVER_CONFIG); i++)
    current_config.pin_config[i] = config->pin_config[i];

  return TRUE;
}


BOOL driver_config_pin(BYTE pin_num, DRIVER_PIN_CONFIG pin_cfg)
{
  current_config.pin_config[pin_num] = pin_cfg;
  return driver_config(&current_config);
}


BOOL driver_enable(void)
{
  if(GPIO_PWRSW_STATE())
    return FALSE;

  GPIO_DRIVER_EN_SET();

  return TRUE;
}

void driver_disable(void)
{
  GPIO_DRIVER_EN_UNSET();
  driver_config(&reset_cfg);
}
