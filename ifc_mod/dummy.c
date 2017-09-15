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
 * dummy.c
 * Dummy interface
 *
 */
 
#include <fx2types.h>

#include "module.h"


const char signature[SIGNATURE_SIZE] = "dummy";


//******************************************************************************
// Private Functions
//******************************************************************************



//******************************************************************************
// Public Functions
//******************************************************************************

void ifc_init(void)
{
  return;
}


// Data pointed by the autopointer 1
BOOL ifc_set_config(IFC_CFG_TYPE type, BYTE param)
{
  type;
  param;
  return FALSE;
}


BOOL ifc_read_id(IFC_ID_TYPE type, BYTE *id)
{
  type;
  id;
  return FALSE;
}


BOOL ifc_erase_chip(void)
{
  return FALSE;
}


BOOL ifc_prepare_read(void)
{
  return FALSE;
}


BOOL ifc_prepare_write(void)
{
  return FALSE;
}


BOOL ifc_busy(void)
{
  return FALSE;
}


void ifc_abort(void)
{
  return;
}


void ifc_process(void)
{
  return;
}
