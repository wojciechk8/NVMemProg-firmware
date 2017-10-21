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
 * nvmemprog.c
 *
 * Main and initialization code, USB commands handling
 *
 */

#include <fx2macros.h>
#include <fx2ints.h>
#include <autovector.h>
#include <setupdat.h>
#include <eputils.h>
#include <i2c.h>
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


volatile __bit ocprot = FALSE;
volatile __bit do_ocprot = FALSE;
volatile __bit do_sud = FALSE;
volatile enum EP0State{
  EP0STATE_
}ep0out_state;


//******************************************************************************
// CONFIGURATION HANDLERS
//******************************************************************************

BOOL handle_get_descriptor()
{
  return FALSE;
}

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


//******************************************************************************
// VENDOR COMMANDS HANDLER
//******************************************************************************

BOOL handle_vendorcommand(BYTE cmd)
{
  BYTE i;
  WORD cnt;
  
  switch((VENDOR_CMD)cmd){
    case CMD_LED:
      if(SETUPDAT[4] == 0){       // wIndex
        GPIO_LEDG = SETUPDAT[2];  // wValue
      }else if(SETUPDAT[4] == 1){
        GPIO_LEDR = SETUPDAT[2];
      }
      break;

    case CMD_FW_SIGNATURE:
      for (i = 0; i < SETUPDAT[6]; i++) {
        EP0BUF[i] = signature[i];
      }
      EP0BCL = SIGNATURE_SIZE;
      break;

    case CMD_FPGA_START_CONFIG:
      if(!fpga_start_config()){
        STALLEP0();
      }
      break;
    
    case CMD_FPGA_WRITE_CONFIG:
      if(fpga_get_status() != FPGA_STATUS_CONFIGURING){
        STALLEP0();
      }
      for(cnt = SETUP_LENGTH(); cnt; cnt -= EP0BCL){
        EP0BCL = 0; SYNCDELAY;    // arm EP0
        while (EP0CS & bmEPBUSY)  // wait for OUT data
          ;
        __asm
          ; source
          mov	_AUTOPTRH1,#(_EP0BUF >> 8)
          mov	_AUTOPTRL1,#_EP0BUF
        __endasm;
        fpga_write_config(EP0BCL);
        if(fpga_get_status() == FPGA_STATUS_UNCONFIGURED){
          STALLEP0();
          break;
        }
      }
      break;

    case CMD_FPGA_GET_STATUS:
      EP0BUF[0] = fpga_get_status();
      EP0BCL = 1;
      break;

    case CMD_FPGA_READ_REGS:
      __asm
        ; source
        mov	_AUTOPTRH1,#(_fpga_regs >> 8)
        ; destination
        mov	_AUTOPTRH2,#(_EP0BUF >> 8)
        mov	_AUTOPTRL2,#_EP0BUF
      __endasm;
      AUTOPTRL1 = SETUPDAT[4];
      for (i = 0x00; i < SETUPDAT[6]; i++){
        XAUTODAT2 = XAUTODAT1;
      }
      EP0BCL = SETUPDAT[2];
      break;
    
    case CMD_FPGA_WRITE_REGS:
      EP0BCL = 0; SYNCDELAY;    // arm EP0
      while (EP0CS & bmEPBUSY)  // wait for OUT data
        ;
      __asm
        ; source
        mov	_AUTOPTRH1,#(_EP0BUF >> 8)
        mov	_AUTOPTRL1,#_EP0BUF
        ; destination
        mov	_AUTOPTRH2,#(_fpga_regs >> 8)
      __endasm;
      AUTOPTRL2 = SETUPDAT[4];
      for (i = 0x00; i < SETUPDAT[6]; i++){
        XAUTODAT2 = XAUTODAT1;
      }
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
      if(!driver_read_id(EP0BUF, SETUPDAT[6])){
        STALLEP0();
      }
      EP0BCL=1;
      break;

    case CMD_DRIVER_WRITE_ID:
      EP0BCL = 0; SYNCDELAY;    // arm EP0
      while (EP0CS & bmEPBUSY)  // wait for OUT data
        ;
      if(!driver_write_id(EP0BUF, SETUPDAT[6])){
        STALLEP0();
      }
      break;

    case CMD_DRIVER_CONFIG:
      EP0BCL = 0; SYNCDELAY;    // arm EP0
      while (EP0CS & bmEPBUSY)  // wait for OUT data
        ;
      if(!driver_config((DRIVER_CONFIG*)EP0BUF)){
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
      if(!pwr_set_current(SETUPDAT[4]+(SETUPDAT[4]<2 ? 2 : 0),
                          SETUPDAT[2])){
        STALLEP0();
      }
      break;

    case CMD_PWR_SWITCH:
      if(SETUPDAT[2] == 0x00){
        // Suppress OCPROT# interrupt when safely switching power off
        EX1 = 0;
        pwr_switch_off();
        delay_us(10);
        IE1 = 0;
        EX1 = 1;
        GPIO_LEDG_ON();
        GPIO_LEDR_OFF();
      }else if(SETUPDAT[2] == 0x01){
        GPIO_LEDG_OFF();
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

    case CMD_PWR_SW_STATE:
      EP0BUF[0] = GPIO_PWRSW_STATE();
      EP0BCL = 1;
      break;

    case CMD_EEPROM_READ:
      if(eeprom_read(EEPROM_ADDR, SETUP_INDEX(), SETUPDAT[6], EP0BUF)){
        EP0BCL = SETUPDAT[6];
      }else{
        STALLEP0();
      }
      break;

    case CMD_EEPROM_WRITE:
      EP0BCL = 0; SYNCDELAY;    // arm EP0
      while (EP0CS & bmEPBUSY)  // wait for OUT data
        ;
      if(!eeprom_write(EEPROM_ADDR, SETUP_INDEX(), SETUPDAT[6], EP0BUF)){
        STALLEP0();
      }
      break;

    case CMD_IFC_SET_CONFIG:
      EP0BCL = 0; SYNCDELAY;    // arm EP0
      while (EP0CS & bmEPBUSY)  // wait for OUT data
        ;
      __asm
        ; source
        mov	_AUTOPTRH1,#(_EP0BUF >> 8)
        mov	_AUTOPTRL1,#_EP0BUF
      __endasm;
      if(!ifc_set_config(SETUPDAT[4], SETUPDAT[2])){
        STALLEP0();
      }
      break;

    case CMD_IFC_READ_ID:
      if(ifc_read_id(SETUPDAT[4], EP0BUF)){
        EP0BCL = SETUPDAT[6];
      }else{
        STALLEP0();
      }
      break;

    case CMD_IFC_ERASE_CHIP:
      if(!ifc_erase_chip()){
        STALLEP0();
      }
      break;

    case CMD_IFC_READ_DATA:
      if(!ifc_prepare_read()){
        STALLEP0();
      }
      break;

    case CMD_IFC_WRITE_DATA:
      if(!ifc_prepare_write()){
        STALLEP0();
      }
      break;

    case CMD_IFC_ABORT:
      ifc_abort();
      break;
  }
  return TRUE;
}


//******************************************************************************
// ENDPOINT HANDLERS
//******************************************************************************

void handle_ep1ibn(void)
{
  __asm
    mov	_AUTOPTRH1,#(_EP1INBUF >> 8)
    mov	_AUTOPTRL1,#_EP1INBUF
  __endasm;

  XAUTODAT1 = GPIO_SW_STATE();
  XAUTODAT1 = GPIO_DCOK_STATE();
  XAUTODAT1 = ocprot;
  XAUTODAT1 = ifc_busy();

  EP1INBC = sizeof(DEVICE_STATUS);  // arm EP1IN

  ocprot = 0;
}


//******************************************************************************
// INIT
//******************************************************************************

void device_init(void)
{
  IFCONFIG = bmIFCLKSRC|bm3048MHZ;      // IFCLK = Internal 48MHz
  SETCPUFREQ(CLK_48M);
  REVCTL = 3; SYNCDELAY;
  I2CTL = bm400KHZ;                     // Set I2C to 400kHz
  AUTOPTRSETUP = bmBIT2|bmBIT1|bmBIT0;  // enable autoptr; increment both

  gpio_init();
  pwr_switch_off();
  driver_init();
  pwr_init();

  // Endpoints configuration
  EP1INCFG = bmVALID|bmTYPE1|bmTYPE0; SYNCDELAY;  // INT 64B
  EP2CFG = bmVALID|bmTYPE1|0x2;       SYNCDELAY;  // BULK OUT 512B X2
  EP6CFG = bmVALID|bmDIR|bmTYPE1|0x2; SYNCDELAY;  // BULK IN 512B X2
  EP4CFG = bmTYPE1;                   SYNCDELAY;  // OFF
  EP8CFG = bmTYPE1;                               // OFF
}


//******************************************************************************
// ISR
//******************************************************************************

void sudav_isr() __interrupt SUDAV_ISR
{
  do_sud = TRUE;
  CLEAR_SUDAV();
}

void usbreset_isr() __interrupt USBRESET_ISR
{
  handle_hispeed(FALSE);
  CLEAR_USBRESET();
}

void hispeed_isr() __interrupt HISPEED_ISR
{
  handle_hispeed(TRUE);
  CLEAR_HISPEED();
}

void ibn_isr() __interrupt IBN_ISR
{
  CLEAR_USBINT();
  if(IBNIRQ & bmEP1IBN){
    handle_ep1ibn();
    IBNIRQ = bmEP1IBN;
  }
  NAKIRQ = bmIBN;
}

void ie1_isr() __interrupt IE1_ISR
{
  do_ocprot = 1;
}


//******************************************************************************
// MAIN
//******************************************************************************

void main()
{
  device_init();
  ifc_init();

  // Set up interrupts
  USE_USB_INTS();
  ENABLE_SUDAV();
  ENABLE_USBRESET();
  ENABLE_HISPEED();
  IBNIE = bmEP1IBN;
  NAKIE = bmIBN;
  // OCPROT# external interrupt on falling edge
  IE1 = 0;  // clear int flag
  EX1 = 1;
  IT1 = 1;

  // Enable interrupts
  EA=1;

  RENUMERATE();
  
  GPIO_LEDG_ON();

  while(TRUE){
    ifc_process();

    if(do_sud){
      do_sud = FALSE;
      handle_setupdata();
    }

    if(do_ocprot){
      do_ocprot = FALSE;
      pwr_reset();
      driver_disable();
      ifc_abort();
      GPIO_LEDR_ON();
      ocprot = 1;
    }
  }
}
