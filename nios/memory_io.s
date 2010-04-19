/*
# File "memory_io.s"
# Copyright (c) 2009 M Bark
#    E-mail: mikbar at kth dot se
#
#    modified by: Jorge Miró, Lyudmilla Gerlakh, Mikael Bark, Paul Hill, Dimitrios Tachtsioglou
# 
#    This file is part of the VR 09 system.
# 
#    VR09 is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3 of the License, or
#    (at your option) any later version.
# 
#    VR09 is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
#
# File version: 1.0
# VR09 version: 1.0
#
# Modification history for this file:
# v1.0  (2009-05-15) First released version.
#
################################################################
*/

.data
.align 2

.data
.align 2
.global status_PORTS
status_PORTS:  .word 0

.text
.global signalon
.global signaloff

.global signalread

.global clearSignals

#Read signal at address r4. Returns the bit specified as r5 at adress r4.
signalread:
	ldw r11, 0(r4)

	movi r8, 1
	sll r8, r8, r5
	
	and r8, r8, r11
	
	cmpne r2, r8, r0 
	
	ret

signalon:
	movia r10, status_PORTS
	
	ldw r11, 0(r10)
	
	movi r8, 1
	sll r8, r8, r5
	
	or r8, r8, r11
	
	stw r8, 0(r4)
	stw r8, 0(r10)

	ret

signaloff:
	movia r10, status_PORTS
	
	ldw r11, 0(r10)
	
	movi r8, 1
	sll r8, r8, r5
	xori r8, r8, 0xffff
	
	and r8, r8, r11
	
	stw r8, 0(r4)
	stw r8, 0(r10)

	ret
	
clearSignals:
	movia r10, status_PORTS
	stw r0, 0(r10)
	
	ret
	
	.end
	
	
	
	
	
	
	