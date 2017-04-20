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

#include <fx2macros.h>
#include <fx2ints.h>
#include <autovector.h>
#include <delay.h>
#include <setupdat.h>

#include "gpio.h"


volatile __bit dosud=FALSE;
volatile __bit doep1=FALSE;

// custom functions
extern void main_loop(void);
extern void main_init(void);
extern void handle_ep1out(void);


void sudav_isr() __interrupt SUDAV_ISR
{
  dosud = TRUE;
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

void ep1out_isr() __interrupt EP1OUT_ISR
{
  doep1 = TRUE;
  CLEAR_EP1OUT();
}

void int1_isr() __interrupt IE1_ISR
{
  GPIO_LEDR_ON();
}


void main()
{

  main_init();

  // set up interrupts.
  USE_USB_INTS();
  ENABLE_SUDAV();
  ENABLE_USBRESET();
  ENABLE_HISPEED();
  ENABLE_EP1OUT();
  // OCPROT# external interrupt on falling edge
  EX1 = 1;
  IT1 = 1;

  // enable interrupts
  EA=1;

  // iic files (c2 load) don't need to renumerate/delay
  // trm 3.6
  #ifndef NORENUM
    RENUMERATE();
  #else
    USBCS &= ~bmDISCON;
  #endif

  while(TRUE){
    main_loop();

    if(dosud){
      dosud=FALSE;
      handle_setupdata();
    }
    if(doep1){
      doep1 = FALSE;
      handle_ep1out();
    }
  }

}
