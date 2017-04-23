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
 * nvmemprog.c
 * 
 * Main code
 *
 */

#include <i2c.h>
#include <setupdat.h>
#include <eputils.h>
#include <fx2macros.h>
#include <delay.h>

#include "driver.h"
#include "power.h"
#include "fpga.h"
#include "gpio.h"
#include "delay_us.h"
#include "common.h"
#include "ifc_mod/module.h"


#define FW_VERSION 0x0001

#define SYNCDELAY SYNCDELAY3


enum{
  EP1STATE_NOTHING,
  EP1STATE_FPGA_CONFIG,
  EP1STATE_FPGA_REGS,
  EP1STATE_DRIVER_CONFIG,
  EP1STATE_IFC_CONFIG
}ep1state = EP1STATE_NOTHING;

volatile __bit ocprot=FALSE;



BOOL handle_get_descriptor()
{
  return FALSE;
}


//********************** CONFIGURATION HANDLERS ************************

// change to support as many interfaces as you need
//volatile xdata BYTE interface=0;
//volatile xdata BYTE alt=0; // alt interface

// set *alt_ifc to the current alt interface for ifc
BOOL handle_get_interface(BYTE ifc, BYTE* alt_ifc)
{
  //*alt_ifc=alt;
  ifc;
  alt_ifc;
  return TRUE;
}
// return TRUE if you set the interface requested
// NOTE this function should reconfigure and reset the endpoints
// according to the interface descriptors you provided.
BOOL handle_set_interface(BYTE ifc, BYTE alt_ifc)
{
  //printf ( "Set Interface.\n" );
  //interface=ifc;
  //alt=alt_ifc;
  ifc;
  alt_ifc;
  return TRUE;
}

// handle getting and setting the configuration
// 1 is the default.  If you support more than one config
// keep track of the config number and return the correct number
// config numbers are set int the dscr file.
//volatile BYTE config=1;
BYTE handle_get_configuration()
{
  return 1;
}

// NOTE changing config requires the device to reset all the endpoints
BOOL handle_set_configuration(BYTE cfg)
{
  //printf ( "Set Configuration.\n" );
  cfg;
  //config=cfg;
  return TRUE;
}


//********************* VENDOR COMMAND HANDLERS ************************


BOOL handle_vendorcommand(BYTE cmd)
{
  switch((VENDOR_CMD)cmd){
    case CMD_LED:
      if(SETUPDAT[4] == 0){ // wIndex
        GPIO_LEDG = SETUPDAT[2]; // wValue
      }else if(SETUPDAT[4] == 1){
        GPIO_LEDR = SETUPDAT[2];
      }
      break;
    
    case CMD_VERSION:
      EP0BUF[0] = LSB(FW_VERSION);
      EP0BUF[1] = MSB(FW_VERSION);
      EP0BCH=0;
      EP0BCL=2;
      break;

    case CMD_FPGA_START_CONFIG:
      if(fpga_start_config()){
        EP1OUTBC = 0; // arm EP1OUT
        ep1state = EP1STATE_FPGA_CONFIG;
      }else{
        STALLEP0();
      }
      break;

    case CMD_FPGA_WRITE_REG:
      if(SETUPDAT[4] < FPGA_REG_NUM){
        fpga_regs.reg[SETUPDAT[4]] = SETUPDAT[2];
      }else{
        STALLEP0();
      }
      break;

    case CMD_FPGA_WRITE_REGS:
      EP1OUTBC = 0; // arm EP1OUT
      ep1state = EP1STATE_FPGA_REGS;
      break;

    case CMD_DRIVER_ENABLE:
      if(SETUPDAT[2] == 0xA5){
        if(!driver_enable()){
          STALLEP0();
        }
      }else{
        driver_disable();
      }
      break;

    case CMD_DRIVER_READ_ID:
      if(!driver_read_id(EP0BUF)){
        STALLEP0();
      }
      EP0BCH=0;
      EP0BCL=1;
      break;

    case CMD_DRIVER_CONFIG:
      EP0BCL = 0;               // arm EP0
      while (EP0CS & bmEPBUSY)  // wait for OUT data
        ;
      __asm
        ; source
        mov	_AUTOPTRH1,(#_EP0BUF >> 8)
        mov	_AUTOPTRL1,#_EP0BUF
      __endasm;
      if(!driver_write_config(EP0BCL)){
        STALLEP0();
      }
      break;

    case CMD_PWR_SET_DAC:
      if(!pwr_set_dac(SETUPDAT[4], SETUP_VALUE(), SETUPDAT[5])){
        STALLEP0();
      }
      break;
    
    case CMD_PWR_SET_VOLTAGE:
      if(!pwr_ramp_voltage(SETUPDAT[4], SETUPDAT[2], SETUPDAT[3])){
        STALLEP0();
      }
      break;

    case CMD_PWR_SET_CURRENT:
      if(!pwr_set_current(SETUPDAT[4]+2, SETUPDAT[2])){
        STALLEP0();
      }
      break;

    case CMD_PWR_SWITCH:
      if(SETUPDAT[2] == 0x00){
        // Suppress OCPROT# interrupt when safely switching power off 
        EX1 = 0;
        pwr_switch_off();
        delay_us(10);
        EX1 = 1;
      }else if(SETUPDAT[2] == 0x01){
        GPIO_LEDR_OFF();
        pwr_switch_on();
      }else{
        STALLEP0();
      }
      break;

    case CMD_PWR_RESET:
      if(!pwr_reset()){
        STALLEP0();
      }
      break;

    case CMD_EEPROM_READ:
      if(eeprom_read(EEPROM_ADDR, SETUP_INDEX(), SETUPDAT[2], EP0BUF)){
        EP0BCH = 0;
        EP0BCL = SETUPDAT[2];
      }else{
        STALLEP0();
      }
      break;

    case CMD_EEPROM_WRITE:
      EP0BCL = 0;               // arm EP0
      while (EP0CS & bmEPBUSY)  // wait for OUT data
        ;
      if(!eeprom_write(EEPROM_ADDR, SETUP_INDEX(), EP0BCL, EP0BUF)){
        STALLEP0();
      }
      break;
  }
  return TRUE;
}


//*********************** ENDPOINT HANDLERS ****************************


void handle_ep1out(void)
{
  BYTE i;

  switch(ep1state){
    case EP1STATE_FPGA_CONFIG:
      __asm
        ; source
        mov	_AUTOPTRH1,(#_EP1OUTBUF >> 8)
        mov	_AUTOPTRL1,#_EP1OUTBUF
      __endasm;
      if(!fpga_write_config(EP1OUTBC)){
        EP1OUTCS |= bmEPSTALL;
      }
      if(fpga_get_status() == FPGA_STATUS_CONFIGURED){
        ep1state = EP1STATE_NOTHING;
      }else{
        EP1OUTBC = 0; // arm EP for further data
      }
      break;

    case EP1STATE_FPGA_REGS:
      if(EP1OUTBC <= FPGA_REG_NUM){
        __asm
          ; source
          mov	_AUTOPTRH1,(#_EP1OUTBUF >> 8)
          mov	_AUTOPTRL1,#_EP1OUTBUF
          ; destination
          mov	_AUTOPTRH2,(#_fpga_regs >> 8)
          mov	_AUTOPTRL2,#_fpga_regs
        __endasm;
        // transfer
        for (i = 0x00; i < EP1OUTBC; i++){
          XAUTODAT2 = XAUTODAT1;
        }
      }else{
        EP1OUTCS |= bmEPSTALL;
      }
      ep1state = EP1STATE_NOTHING;
      break;

    default:
      EP1OUTCS |= bmEPSTALL;
  }
}


void handle_ep1in(void)
{
  static __bit sw_last=FALSE;

  __asm
    mov	_AUTOPTRH2,(#_EP1INBUF >> 8)
    mov	_AUTOPTRL2,#_EP1INBUF
    mov	dptr,#_XAUTODAT2
  __endasm;
  
  if((!sw_last) && GPIO_SW_STATE()){
    __asm
      mov	a,#0x01
      movx	@dptr,a
    __endasm;
  }else{
    __asm
      clr	a
      movx	@dptr,a
    __endasm;
  }
  sw_last = GPIO_SW_STATE();
  
  // XAUTODAT2 = GPIO_DCOK_STATE();
  // XAUTODAT2 = ocprot;
  __asm
    mov	c,_PA4
    clr	a
    rlc	a
    movx	@dptr,a
    mov	c,_ocprot
    clr	a
    rlc	a
    movx	@dptr,a
  __endasm;
  
  XAUTODAT2 = fpga_get_status();
  //XAUTODAT2 = ifc_get_data_count();
  
  EP1INBC = sizeof(DEVICE_STATUS);  // arm EP1IN

  ocprot = 0;
}


//************************** OTHER HANDLERS ****************************


void handle_ocprot(void)
{
  GPIO_LEDR_ON();
  ocprot = 1;
}


//******************************* INIT *********************************

void main_init(void)
{
  SETCPUFREQ(CLK_48M);
  SYNCDELAY;
  REVCTL=3;
  I2CTL = bm400KHZ;                     // Set I2C to 400kHz
  AUTOPTRSETUP = bmBIT2|bmBIT1|bmBIT0;  // enable autoptr; increment both

  gpio_init();
  pwr_switch_off();
  driver_init();
  pwr_init();

  // Endpoints configuration
  EP1OUTCFG = bmVALID|bmTYPE1;        // BULK 64B
  SYNCDELAY;
  EP1INCFG = bmVALID|bmTYPE1|bmTYPE0; // INT 64B
  SYNCDELAY;
  EP2CFG = bmVALID|bmTYPE1|0x2;       // BULK OUT 512B X2
  SYNCDELAY;
  EP6CFG = bmVALID|bmDIR|bmTYPE1|0x2; // BULK IN 512B X2
  SYNCDELAY;
  EP4CFG = bmTYPE1;  // OFF
  SYNCDELAY;
  EP8CFG = bmTYPE1;  // OFF
  
  handle_ep1in();
}


void main_loop(void)
{

}


