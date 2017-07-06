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
 * common.h
 * Defines and types common with the host software
 *
 */

#pragma once


#ifndef SDCC
  #define VID 0x04b4
  #define PID 0x8613
  #define DEVICE_STATUS_SIZE sizeof (DEVICE_STATUS)
  typedef unsigned char BYTE;
  typedef unsigned short WORD;
#endif

// Vendor USB Commands
typedef enum{                 // wValue         wIndex          IN/OUT data
  CMD_LED=0x10,               // 0=off 1=on     0=green 1=red   ---
  CMD_FIRMWARE=0x11,          // ---            ---             FW signature (8 bytes)


  CMD_FPGA_START_CONFIG=0x20, // ---            ---             ---(data via EP1)
  CMD_FPGA_WRITE_REGS=0x21,   // ---            addr            data


  CMD_DRIVER_ENABLE=0x30,     // 0xA5=en.       ---             ---
                              // else dis.
  CMD_DRIVER_READ_ID=0x31,    // ---            ---             driver id
  CMD_DRIVER_CONFIG=0x32,     // ---            ---             data


  CMD_PWR_SET_DAC=0x40,       // raw value      LSB: DAC channel, ---
                              //                MSB = 0: DAC EEPROM unaffected
                              //                MSB = 1: store value in DAC EEPROM
  CMD_PWR_SET_VOLTAGE=0x41,   // voltage(LSB),  0=VPP 1=VCC     ---
                              // slew rate(MSB)
  CMD_PWR_SET_CURRENT=0x42,   // value          0,2=IPP 1,3=ICC ---
  CMD_PWR_SWITCH=0x43,        // 0=off 1=on     ---             ---
  CMD_PWR_RESET=0x44,         // ---            ---             ---
  CMD_PWR_SW_STATE=0x4F,      // ---            ---             pwr_sw state


  CMD_EEPROM_READ=0x50,       // length         addr            data
  CMD_EEPROM_WRITE=0x51,      // ---            addr            data
  
  
  CMD_IFC_SET_CONFIG=0x60,    // param          config_type     data
  CMD_IFC_READ_ID=0x61,       // size           ---             id
  CMD_IFC_ERASE_CHIP=0x62,    // ---            ---             ---
  CMD_IFC_READ_DATA=0x63,     // ---            ---             ---(data via EP6)
  CMD_IFC_WRITE_DATA=0x64,    // ---            ---             ---(data via EP2)
  CMD_IFC_ABORT=0x6F          // ---            ---             ---
}VENDOR_CMD;


// FPGA
typedef enum{
  FPGA_STATUS_UNCONFIGURED,
  FPGA_STATUS_CONFIGURING,
  FPGA_STATUS_CONFIGURED
}FPGA_CFG_STATUS;


// Power
typedef enum{
  PWR_CH_VPP=0x0,
  PWR_CH_VCC=0x1,
  PWR_CH_IPP=0x2,
  PWR_CH_ICC=0x3
}PWR_CH;


// EEPROM
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


// Device Status
typedef struct{
  BYTE sw;
  BYTE dcok;
  BYTE ocprot;
  BYTE fpga;
  BYTE ifc_busy;
}DEVICE_STATUS;


// Memory Interface
typedef enum{
  IFC_CFG_ADDRESS_MAPPING
}IFC_CFG_TYPE;
