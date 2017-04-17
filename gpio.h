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
#include <delay.h>

#define SYNCDELAY SYNCDELAY3


#define GPIO_LEDG PA7
#define GPIO_LEDG_ON() PA7 = 1
#define GPIO_LEDG_OFF() PA7 = 0
#define GPIO_LEDR PA6
#define GPIO_LEDR_ON() PA6 = 1
#define GPIO_LEDR_OFF() PA6 = 0
#define GPIO_SW_STATE() (!PA5)
#define GPIO_DCOK_STATE() PA4
#define GPIO_PWRSW_CLK_SET() PA3 = 1
#define GPIO_PWRSW_CLK_UNSET() PA3 = 0
#define GPIO_PWRSW_D_SET() PA2 = 1
#define GPIO_PWRSW_D_UNSET() PA2 = 0
#define GPIO_PWRSW_STATE() (PA1)
#define GPIO_DRIVER_EN_SET() IOE |= bmBIT5
#define GPIO_DRIVER_EN_UNSET() IOE &= ~bmBIT5
#define GPIO_DRIVER_EN_STATE() (IOE&bmBIT5)
#define GPIO_FPGA_CONFIG_SET() IOE &= ~bmBIT2
#define GPIO_FPGA_CONFIG_UNSET() IOE |= bmBIT2
#define GPIO_FPGA_CONFDONE_STATE() (IOE&bmBIT1)
#define GPIO_FPGA_STATUS_STATE() (!(IOE&bmBIT0))


#define GPIO_SET_PORTBD_FD() IFCONFIG = (IFCONFIG&~bmIFCFGMASK)|bmIFCFG1
#define GPIO_SET_PORTBD_PORT() IFCONFIG = (IFCONFIG&~bmIFCFGMASK)
#define GPIO_SET_PORTC_GPIFADR() do{PORTCCFG = 0xFF; PORTECFG |= bmBIT7;}while(0)
#define GPIO_SET_PORTC_PORT() do{PORTCCFG = 0x00; PORTECFG &= ~bmBIT7;}while(0)


inline void gpio_init(void)
{
  // PORTA
  // LEDG, LEDR, SW, DC_OK, PWRSW_CLK, PWRSW_D, OCPROT#(INT1#), N/C
  PORTACFG = bmINT1;
  IOA = bmBIT3;
  OEA = bmBIT7|bmBIT6|bmBIT3|bmBIT2|bmBIT0;

  // PORTB
  // FD[7:0]
  IOB = 0;
  OEB = 0;

  // PORTC
  // GPIFADR[7:0]
  IOC = 0;
  OEC = 0xFF;

  // PORTD
  // FD[15:8]
  IOD = 0;
  OED = 0;

  // PORTE
  // GPIFADR[8], N/C, DRIVER_EN, DRIVER_DATA(RXD1OUT),
  // FPGA_DATA(RXD0OUT), FPGA_nCONFIG, FPGA_CONF_DONE, FPGA_nSTATUS
  PORTECFG = bmRXD1OUT|bmRXD0OUT;
  IOE = bmBIT2;
  OEE = bmBIT7|bmBIT6|bmBIT5|bmBIT2;
}

