/**
 * Copyright (C) 2009 Ubixum, Inc.
 * Copyright (C) 2016 Wojciech Krutnik <wojciechk8@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#include <fx2regs.h>
#include <fx2macros.h>

#include "delay_us.h"
#include "delay.h"


/**
   * It takes 12 crystal pulses to make 1 machine cycle (8051.com)
   * ez-usb trm 1.13
   *  83.3 ns at 48mhz per instruction cycle
   *  (assume 166.6ns at 24mhz)
   *  (assume 333.3ns at 12mhz)
   * ez-usb trm 12.1
   *  Includes the cycles for each instruction
   **/

void delay_us(BYTE us) {
  // sdcc generated assembly
  /*
    00101$:
    ;	delay_us.c:59: SYNCDELAY9;
      nop 
      nop 
      nop 
      nop 
      nop 
      nop 
      nop 
      nop 
      nop 
    ;	delay_us.c:60: }while(--us);
      djnz	r6,00101$

  * loop: 12 cycles = 999,6ns for 48MHz
  */

  if(CPUFREQ == CLK_48M){
    do{
      SYNCDELAY9;
    }while(--us);
  }else if(CPUFREQ == CLK_24M){
    do{
      SYNCDELAY3;
    }while(--us);
  }else{
    do{
      ;
    }while(--us);
  }
}
