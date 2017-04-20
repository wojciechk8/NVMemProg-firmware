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
#include "eeprom.h"
#include "vendor_req.h"
#include "ifc_mod/module.h"


#define SYNCDELAY SYNCDELAY3


enum{
  EP1STATE_NOTHING,
  EP1STATE_FPGA_CONFIG,
  EP1STATE_FPGA_REGS,
  EP1STATE_DRIVER_CONFIG,
  EP1STATE_IFC_CONFIG
}ep1state = EP1STATE_NOTHING;



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

    case CMD_SW:
      EP0BUF[0] = GPIO_SW_STATE();
      EP0BUF[1] = GPIO_DCOK_STATE();
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

    case CMD_FPGA_CONFIG_STATUS:
      EP0BUF[0] = fpga_get_status();
      EP0BCH=0;
      EP0BCL=1;
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
      if(!driver_write_config(EP0BUF, EP0BCL)){
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
        pwr_switch_off();
      }else if(SETUPDAT[2] == 0x01){
        pwr_switch_on();
      }else{
        STALLEP0();
      }
      break;
    
    case CMD_PWR_STATE:
      EP0BUF[0] = GPIO_PWRSW_STATE();
      EP0BCH=0;
      EP0BCL=1;
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


//********************* ENDPOINT HANDLERS ******************************


void handle_ep1out(void)
{
  BYTE i;

  switch(ep1state){
    case EP1STATE_FPGA_CONFIG:
      if(!fpga_write_config(EP1OUTBUF, EP1OUTBC)){
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
        // source
        AUTOPTRH1 = MSB(EP1OUTBUF);
        AUTOPTRL1 = LSB(EP1OUTBUF);
        // destination
        AUTOPTRH2 = MSB(fpga_regs.reg);
        AUTOPTRL2 = LSB(fpga_regs.reg);
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


//***************************** INIT ***********************************


void gpif_init(void)
{
  BYTE i;

  // GPIF config
  GPIFABORT = 0xFF;  // abort any waveforms pending

  GPIFREADYCFG = bmBIT7;    // IntReady
  GPIFCTLCFG = 0x00;
  GPIFIDLECS = 0x00;
  GPIFIDLECTL = 0x00;
  GPIFWFSELECT = 0x00;      // All triggers to waveform 0

  // WaveForm
  // source
  //AUTOPTRH1 = MSB(&wave_data);
  //AUTOPTRL1 = LSB(&wave_data);
  // destination
  AUTOPTRH2 = 0xE4;
  AUTOPTRL2 = 0x00;
  // transfer
  /*for(i = 0x00; i < sizeof(wave_data); i++){
    XAUTODAT2 = XAUTODAT1;
  }*/

  GPIO_SET_PORTC_GPIFADR();
  // update GPIF address pins
  SYNCDELAY;
  GPIFADRH = 0x00;
  SYNCDELAY;
  GPIFADRL = 0x00;

  // Configure GPIF FlowStates
  FLOWSTATE = 0;
  FLOWLOGIC = 0;
  FLOWEQ0CTL = 0;
  FLOWEQ1CTL = 0;
  FLOWHOLDOFF = 0;
  FLOWSTB = 0;
  FLOWSTBEDGE = 0;
  FLOWSTBHPERIOD = 0;
}


void main_init(void)
{
  SETCPUFREQ(CLK_48M);
  SYNCDELAY;
  REVCTL=3;
  SYNCDELAY;
  IFCONFIG = bmIFCLKSRC|bm3048MHZ;      // IFCLK = Internal 48MHz
  SYNCDELAY;
  I2CTL = bm400KHZ;                     // Set I2C to 400kHz
  AUTOPTRSETUP = bmBIT2|bmBIT1|bmBIT0;  // enable autoptr; increment both

  gpio_init();
  pwr_switch_off();
  driver_init();
  pwr_init();


  // Endpoints configuration
  EP1OUTCFG = 0xA0;
  SYNCDELAY;
  EP1INCFG = 0xA0;
  SYNCDELAY;
  EP2CFG = 0xA2;  // BULK OUT 512B
  SYNCDELAY;
  EP6CFG = 0xE2;  // BULK IN 512B
  SYNCDELAY;
  EP4CFG = 0x20;  // OFF
  SYNCDELAY;
  EP8CFG = 0x20;  // OFF
  SYNCDELAY;
  
  //ifc_init();
}


void main_loop(void)
{

}


