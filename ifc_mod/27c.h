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
 * 27c.c
 * 27Cxxx EPROM interface module
 *
 */

#pragma once


void ifc_init(void)
{
  
}

void ifc_read_memory(void);
void ifc_write_memory(void);
void ifc_set_config(void);

/*
    case CMD_WAVEFORM_SETUP:
      if(GPIFTRIG & bmBIT7){  // check if GPIF is IDLE
        GPIFADRH = 0x00;
        SYNCDELAY;
        GPIFADRL = 0x00;  // set address to 0
        dummy = XGPIFSGLDATLX; // single byte read transaction trigger
      }else{
        STALLEP0();
      }
      break;

    case CMD_WAVEFORM_RUN:
      if(GPIFTRIG & bmBIT7){  // check if GPIF is IDLE
        GPIFADRH = 0x00;
        SYNCDELAY;
        GPIFADRL = 0x00;  // set address to 0
        dummy = XGPIFSGLDATLX; // single byte read transaction trigger
      }else{
        STALLEP0();
      }
      break;

    case CMD_WAVEFORM_ABORT:
      if(!(GPIFTRIG & bmBIT7)){  // check if GPIF is not IDLE
        GPIFABORT = 0xFF;
      }else{
        STALLEP0();
      }
      break;
 */
