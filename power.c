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
 * power.c
 *
 */


#include <i2c.h>
#include <fx2types.h>
#include <fx2macros.h>

#include "gpio.h"
#include "power.h"
#include "eeprom.h"
#include "delay_us.h"


#define DAC_ADDR 0x60
#define DAC_CMD_MULTIWRITE 0x08
#define DAC_CMD_SINGLEWRITE 0x0B
#define DAC_CMD_RESET 0x06

/* DAC = b - a*voltage
 * default values:
 * VPP: a = 318.698
 *      b = 5577
 * VCC: a = 645.615
 *      b = 4892
 */

/* DAC = a*current + b
 * default values:
 * IPP: a = 16.741
 *      b = 268/a
 * ICC: a = 15.393
 *      b = -44/a
 */

typedef struct{
  uint16_t v[2];
  uint8_t i[2];
}calib;

//                               VPP     VCC       IPP   ICC
//                              (I12Q4) (I12Q4)   (I5Q3)(I5Q3)
static __code calib _calib_a = {{0x13EB, 0x285A}, {0x86, 0x7B}};
static __xdata calib *calib_a = (void __xdata*)&_calib_a;
//                              (I16)   (I16)     (I8)  (I8)
static __code calib _calib_b = {{0x15C9, 0x131C}, {0x10, 0xfd}};
static __xdata calib *calib_b = (void __xdata*)&_calib_b;

static __xdata WORD dac_value[2]; // VPP, VCC


void pwr_init(void)
{
  BYTE header[2];
  
  // Reset DAC
  pwr_reset();

  // Load calibration data from EEPROM
  if(eeprom_read(EEPROM_ADDR, EEPROM_CALIB_HEADER, 2, header) == FALSE){
    return;
  }
  if((header[0] != 0x1B) || (header[1] != 0xCA)){
    return;
  }

  eeprom_read(EEPROM_ADDR, EEPROM_CALIB_A, sizeof(calib), (void*)calib_a);
  eeprom_read(EEPROM_ADDR, EEPROM_CALIB_B, sizeof(calib), (void*)calib_b);
}


BOOL pwr_set_dac(PWR_CH ch, WORD val, BYTE store)
{
  BYTE cmd[3];

  if(ch > 3){
    return FALSE;
  }

  if((ch < 2) && (val < 450)){
    return FALSE;
  }
  
  // Single Write Command Data Frame (data stored in EEPROM)
  // 1st byte: command [7:3], channel [2:1], UDAC# [0]
  // 2nd byte: VREF [7], PD1:0 [6:5], Gx [4], D11:8 [3:0]
  // 3rd byte: D7:0
  if(store){
    cmd[0] = (DAC_CMD_SINGLEWRITE<<3)|(ch<<1);
  }else{
    cmd[0] = (DAC_CMD_MULTIWRITE<<3)|(ch<<1);
  }
  cmd[1] = MSB(val)&0x0F;
  cmd[2] = LSB(val);
  return i2c_write(DAC_ADDR, 0, NULL, 3, cmd);
}


BOOL pwr_set_current(PWR_CH ch, BYTE current)
{
  BYTE cmd[3];
  WORD dac_val;

  if(((ch-2) > 1) || (current < 10)){
    return FALSE;
  }
  
  current += calib_b->i[ch-2];
  dac_val = (((WORD)(calib_a->i[ch-2])*current)>>3);  // a*(current+b)
  
  // Multiwrite Command Data Frame
  // 1st byte: command [7:3], channel [2:1], UDAC# [0]
  // 2nd byte: VREF [7], PD1:0 [6:5], Gx [4], D11:8 [3:0]
  // 3rd byte: D7:0
  cmd[0] = (DAC_CMD_MULTIWRITE<<3)|(ch<<1);
  cmd[1] = MSB(dac_val)&0x0F;
  cmd[2] = LSB(dac_val);
  return i2c_write(DAC_ADDR, 0, NULL, 3, cmd);
}


BOOL pwr_ramp_voltage(PWR_CH ch, BYTE voltage, int8_t rate)
{
  BYTE cmd[3];
  int16_t dac_remain;
  uint32_t tmp;
  
  if((ch>1)
     || (voltage < (ch==PWR_CH_VPP?0x70:0x18))   // voltage limit
     || (voltage > (ch==PWR_CH_VPP?0xF0:0x68))){ // 7-15V for VPP; 1.5-6.5V for VCC
    return FALSE;
  }
  
  tmp = ((DWORD)(calib_a->v[ch])*voltage) + 0x80;
  dac_remain = (calib_b->v[ch]) - (WORD)(tmp>>8);  // b - a*voltage
  dac_remain = dac_value[ch] - dac_remain;

  if(rate <= 0){
    return FALSE;
  }
  if((BYTE)(dac_remain>>8)&0x80){  // is negative?
    rate = -rate;
  }
  
  while(dac_remain){
    //if(abs(dac_remain) < abs(rate)) rate = dac_remain;
    if(rate > 0){
      if(dac_remain < rate){
        rate = dac_remain;
      }
    }else{
      if(dac_remain > rate){
        rate = dac_remain;
      }
    }
 
    /*
    if(((dac_remain<0) ? -dac_remain : dac_remain) <
       ((rate<0) ? -rate : rate)){
      rate = dac_remain;
    }
    */

    dac_value[ch] -= rate;
    dac_remain -= rate;

    if(dac_value[ch] < 450){  // high voltage protection
      GPIO_LEDR_ON();
      return FALSE;
    }

    // Multiwrite Command Data Frame
    // 1st byte: command [7:3], channel [2:1], UDAC# [0]
    // 2nd byte: VREF [7], PD1:0 [6:5], Gx [4], D11:8 [3:0]
    // 3rd byte: D7:0
    cmd[0] = (DAC_CMD_MULTIWRITE<<3)|(ch<<1);
    cmd[1] = MSB(dac_value[ch])&0x0F;
    cmd[2] = LSB(dac_value[ch]);
    if(i2c_write(DAC_ADDR, 0, NULL, 3, cmd) == FALSE){
      return FALSE;
    }
  }

  return TRUE;
}


BOOL pwr_reset(void)
{
  BYTE cmd = DAC_CMD_RESET;
  BYTE buf[9];

  // General Call Reset
  if(i2c_write(0x00, 1, &cmd, 0, NULL) == FALSE){
    return FALSE;
  }

  // Read the actual DAC values after reset
  if(i2c_read(DAC_ADDR, 9, buf) == FALSE){
    return FALSE;
  }
  dac_value[0] = ((WORD)(buf[1]&0x0F)<<8) | buf[2];
  dac_value[1] = ((WORD)(buf[7]&0x0F)<<8) | buf[8];

  return TRUE;
}


void pwr_switch_off(void)
{
  GPIO_PWRSW_CLK_UNSET();
  GPIO_PWRSW_D_UNSET();
  GPIO_PWRSW_CLK_SET();
  delay_us(10);
  GPIO_LEDR_OFF();
}


void pwr_switch_on(void)
{
  GPIO_LEDR_OFF();
  GPIO_PWRSW_CLK_UNSET();
  GPIO_PWRSW_D_SET();
  GPIO_PWRSW_CLK_SET();
}
