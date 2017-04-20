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
 * vendor_req.h
 *
 */

#pragma once


typedef enum{                  // wValue         wIndex          IN/OUT data
  
  CMD_LED=0x10,                // 0=off 1=on     0=green 1=red   ---
  CMD_SW=0x11,                 // ---            ---             SW, DCOK state
                               //                                (byte 0: SW,
                               //                                 byte 1: DCOK)


  CMD_FPGA_START_CONFIG=0x20,  // ---            ---             ---(data via EP1)
  CMD_FPGA_CONFIG_STATUS=0x21, // ---            ---             fpga status
  CMD_FPGA_WRITE_REG=0x22,     // reg value      reg addr        ---
  CMD_FPGA_WRITE_REGS=0x23,    // ---            ---             ---(data via EP1)


  CMD_DRIVER_ENABLE=0x30,      // 0xA5=en.       ---             ---
                               // else dis.
  CMD_DRIVER_READ_ID=0x31,     // ---            ---             driver id
  CMD_DRIVER_CONFIG=0x32,      // ---            ---             data


  CMD_PWR_SET_DAC=0x40,        // raw value      LSB: DAC channel, ---
                               //                MSB = 0: DAC EEPROM unaffected
                               //                MSB = 1: store value in DAC EEPROM
  CMD_PWR_SET_VOLTAGE=0x41,    // voltage(LSB),  0=VPP 1=VCC     ---
                               // slew rate(MSB)
  CMD_PWR_SET_CURRENT=0x42,    // value          0=IPP 1=ICC     ---
  CMD_PWR_SWITCH=0x43,         // 0=off 1=on     ---             ---
  CMD_PWR_STATE=0x44,          // ---            ---             PWR switch state
                               //                                (1=on)
  CMD_PWR_RESET=0x45,          // ---            ---             ---


  CMD_EEPROM_READ=0x50,        // length         addr            data
  CMD_EEPROM_WRITE=0x51,       // ---            addr            data
  
  
  CMD_IFC_SET_CONFIG=0x60      // ---            config_type     data
  
}VENDOR_CMD;
