; Copyright (C) 2009 Ubixum, Inc.
; Copyright (C) 2016, 2017 Wojciech Krutnik <wojciechk8@gmail.com>
;
; This library is free software; you can redistribute it and/or
; modify it under the terms of the GNU Lesser General Public
; License as published by the Free Software Foundation; either
; version 2.1 of the License, or (at your option) any later version.
;
; This library is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
; Lesser General Public License for more details.
;
; You should have received a copy of the GNU Lesser General Public
; License along with this library; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

; this is a the default
; full speed and high speed
; descriptors found in the TRM
; change however you want but leave
; the descriptor pointers so the setupdat.c file works right


.module DEV_DSCR

; descriptor types
; same as setupdat.h
DSCR_DEVICE_TYPE=1
DSCR_CONFIG_TYPE=2
DSCR_STRING_TYPE=3
DSCR_INTERFACE_TYPE=4
DSCR_ENDPOINT_TYPE=5
DSCR_DEVQUAL_TYPE=6

; for the repeating interfaces
DSCR_INTERFACE_LEN=9
DSCR_ENDPOINT_LEN=7

; endpoint types
ENDPOINT_TYPE_CONTROL=0
ENDPOINT_TYPE_ISO=1
ENDPOINT_TYPE_BULK=2
ENDPOINT_TYPE_INT=3

    .globl	_dev_dscr, _dev_qual_dscr, _highspd_dscr, _fullspd_dscr, _dev_strings, _dev_strings_end
; These need to be in code memory.  If
; they aren't you'll have to manully copy them somewhere
; in code memory otherwise SUDPTRH:L don't work right
    .area	DSCR_AREA	(CODE)

_dev_dscr:
	.db	dev_dscr_end-_dev_dscr    ; len
	.db	DSCR_DEVICE_TYPE		  ; type
	.dw	0x0002					  ; usb 2.0
	.db	0xff  					  ; class (vendor specific)
	.db	0xff					  ; subclass (vendor specific)
	.db	0xff					  ; protocol (vendor specific)
	.db	64						  ; packet size (ep0)
	.dw	0x0912					  ; vendor id
	.dw	0x0148					  ; product id
	.dw	0x0100					  ; version id
	.db	1		                  ; manufacturer str idx
	.db	2				          ; product str idx
	.db	0				          ; serial str idx
	.db	1			              ; n configurations
dev_dscr_end:

_dev_qual_dscr:
	.db	dev_qualdscr_end-_dev_qual_dscr
	.db	DSCR_DEVQUAL_TYPE
	.dw	0x0002                              ; usb 2.0
	.db	0xff
	.db	0xff
	.db	0xff
	.db	64                                  ; max packet
	.db	1									; n configs
	.db	0									; extra reserved byte
dev_qualdscr_end:

_highspd_dscr:
	.db	highspd_dscr_end-_highspd_dscr      ; dscr len											;; Descriptor length
	.db	DSCR_CONFIG_TYPE
    ; can't use .dw because byte order is different
	.db	(highspd_dscr_realend-_highspd_dscr) % 256 ; total length of config lsb
	.db	(highspd_dscr_realend-_highspd_dscr) / 256 ; total length of config msb
	.db	1								 ; n interfaces
	.db	1								 ; config number
	.db	0								 ; config string
	.db	0x80                             ; attrs = bus powered, no wakeup
	.db	0xFA                             ; max power = 500ma
highspd_dscr_end:

; all the interfaces next
; NOTE the default TRM actually has more alt interfaces
; but you can add them back in if you need them.
; here, we just use the default alt setting 1 from the trm
	.db	DSCR_INTERFACE_LEN
	.db	DSCR_INTERFACE_TYPE
	.db	0				 ; index
	.db	0				 ; alt setting idx
	.db	3				 ; n endpoints
	.db	0xff			 ; class
	.db	0xff
	.db	0xff
	.db	0	             ; string index

; endpoint 1 in
	.db	DSCR_ENDPOINT_LEN
	.db	DSCR_ENDPOINT_TYPE
	.db	0x81				;  ep1 dir=in and address
	.db	ENDPOINT_TYPE_INT	; type
	.db	0x00				; max packet LSB
	.db	0x02				; max packet size=512 bytes
	.db	0x08				; polling interval (16ms)

; endpoint 2 out
	.db	DSCR_ENDPOINT_LEN
	.db	DSCR_ENDPOINT_TYPE
	.db	0x02				;  ep2 dir=OUT and address
	.db	ENDPOINT_TYPE_BULK	; type
	.db	0x00				; max packet LSB
	.db	0x02				; max packet size=512 bytes
	.db	0x00				; polling interval

; endpoint 6 in
	.db	DSCR_ENDPOINT_LEN
	.db	DSCR_ENDPOINT_TYPE
	.db	0x86				;  ep6 dir=in and address
	.db	ENDPOINT_TYPE_BULK	; type
	.db	0x00				; max packet LSB
	.db	0x02				; max packet size=512 bytes
	.db	0x00				; polling interval
highspd_dscr_realend:

.even
_fullspd_dscr:
	.db	fullspd_dscr_end-_fullspd_dscr      ; dscr len
	.db	DSCR_CONFIG_TYPE
    ; can't use .dw because byte order is different
	.db	(fullspd_dscr_realend-_fullspd_dscr) % 256 ; total length of config lsb
	.db	(fullspd_dscr_realend-_fullspd_dscr) / 256 ; total length of config msb
	.db	1								 ; n interfaces
	.db	1								 ; config number
	.db	0								 ; config string
	.db	0x80                             ; attrs = bus powered, no wakeup
	.db	0xFA                             ; max power = 500ma
fullspd_dscr_end:

; all the interfaces next
; NOTE the default TRM actually has more alt interfaces
; but you can add them back in if you need them.
; here, we just use the default alt setting 1 from the trm
	.db	DSCR_INTERFACE_LEN
	.db	DSCR_INTERFACE_TYPE
	.db	0				 ; index
	.db	0				 ; alt setting idx
	.db	4				 ; n endpoints
	.db	0xff			 ; class
	.db	0xff
	.db	0xff
	.db	0	             ; string index

; endpoint 1 in
	.db	DSCR_ENDPOINT_LEN
	.db	DSCR_ENDPOINT_TYPE
	.db	0x81				;  ep1 dir=in and address
	.db	ENDPOINT_TYPE_INT	; type
	.db	0x40				; max packet LSB
	.db	0x00				; max packet size=64 bytes
	.db	0x14				; polling interval (20ms)

; endpoint 2 out
	.db	DSCR_ENDPOINT_LEN
	.db	DSCR_ENDPOINT_TYPE
	.db	0x02				;  ep2 dir=OUT and address
	.db	ENDPOINT_TYPE_BULK	; type
	.db	0x40				; max packet LSB
	.db	0x00				; max packet size=64 bytes
	.db	0x00				; polling interval

; endpoint 6 in
	.db	DSCR_ENDPOINT_LEN
	.db	DSCR_ENDPOINT_TYPE
	.db	0x86				;  ep6 dir=in and address
	.db	ENDPOINT_TYPE_BULK	; type
	.db	0x40				; max packet LSB
	.db	0x00				; max packet size=64 bytes
	.db	0x00				; polling interval
fullspd_dscr_realend:

.even
_dev_strings:
; sample string
_string0:
	.db	string0end-_string0 ; len
	.db	DSCR_STRING_TYPE
    .db 0x09, 0x04 ; 0x0409 is the language code for English.  Possible to add more codes after this.
string0end:
; add more strings here

; manufacturer
_string1:
    .db string1end-_string1
    .db DSCR_STRING_TYPE
    .ascii 'w'
    .db 0
    .ascii 'o'
    .db 0
    .ascii 'j'
    .db 0
    .ascii 'c'
    .db 0
    .ascii 'i'
    .db 0
    .ascii 'e'
    .db 0
    .ascii 'c'
    .db 0
    .ascii 'h'
    .db 0
    .ascii 'k'
    .db 0
    .ascii '8'
    .db 0
    .ascii '@'
    .db 0
    .ascii 'g'
    .db 0
    .ascii 'm'
    .db 0
    .ascii 'a'
    .db 0
    .ascii 'i'
    .db 0
    .ascii 'l'
    .db 0
    .ascii '.'
    .db 0
    .ascii 'c'
    .db 0
    .ascii 'o'
    .db 0
    .ascii 'm'
    .db 0
string1end:

; product
_string2:
    .db string2end-_string2
    .db DSCR_STRING_TYPE
    .ascii 'N'
    .db 0
    .ascii 'V'
    .db 0
    .ascii 'M'
    .db 0
    .ascii 'e'
    .db 0
    .ascii 'm'
    .db 0
    .ascii 'P'
    .db 0
    .ascii 'r'
    .db 0
    .ascii 'o'
    .db 0
    .ascii 'g'
    .db 0
string2end:


_dev_strings_end:
    .dw 0x0000  ; in case you wanted to look at memory between _dev_strings and _dev_strings_end
