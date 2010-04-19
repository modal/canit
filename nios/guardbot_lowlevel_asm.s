# File "print_functions.c"
# Copyright (c) 2008 F Lundevall
#    E-mail: flu at kth dot se
# 
#    This file is part of the OSLAB micro-operating system.
#
#   Modified for VR09 2009
#   //Mikael Bark
# 
#    OSLAB is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3 of the License, or
#    (at your option) any later version.
# 
#    OSLAB is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
# 
# This file contains low-level interrupt handling.
#
# The label alt_main is defined in this file.
# There is a call to this label in the Altera-supplied startup code for Nios II.
# At label alt_main, interrupts and handlers are initialized; thereafter,
# the label main is called, starting the main program.
#
# File version: 1.818
# OSLAB version: 1.7
#
# Modification history for this file:
# v1.818(2009-18-05) Modified to only handle timer interrupt and print functions
# v1.7  (2008-07-07) Cleaned up comments and added whitespace to improve readability.
# v1.6  (2008-07-04) Removed blank line from yield-info printout to save screen space.
# v1.5  (2008-07-03) Comments improved and cleaned up. Code unchanged.
# v1.4  (2008-06-09) Timer now ticks several times before a thread-switch;
#                    each thread gets approximately the same length of its time-slice
#                    whether or not the previous thread yielded or not.
#                    Oh yes, the time-out value has been changed, too.
# v1.3  (2008-06-03) Changed time-out value, and improved comment before printf.
# v1.2  (2008-04-24) Added function oslab_get_internal_globaltime.
#       (2008-04-24) Increased time-out value for timer_1 again.
# v1.1  (2008-03-05) Changed time-out value for timer_1, to increase the time-slice length.
# v1.0  (2008-02-18) First released version.
#
################################################################
#
# Definitions for important devices and addresses in this system.
#

.macro PUSH reg
    subi sp,sp,4    # reserve space on stack
    stw  \reg,0(sp) # store register
.endm

# POP  reg - pop a single register from the stack

.macro POP  reg
    ldw  \reg,0(sp) # fetch top of stack contents
    addi sp,sp,4    # return previously reserved space
.endm

.equ de2_uart_0_base,0x860
#.equ de2_uart_0_base,0x800

.equ DE2_PIO_JP1_IN1_5_BASE, 0x000008a0
.equ DE2_PIO_JP1_OUT1_5_BASE, 0x000008d0

# Timer_1 at 0x920, interrupt index 10 (mask 2^10 = 0x400)

.equ de2_timer_1_base,0x920
.equ de2_timer_1_intmask,0x400

.equ de2_timer_2_base,0x940

# Timeout value for 0,01 ms tick-count interval (CHANGED in every version)

.equ de2_timer_1_timeout_value,499

# Interrupt address at 0x800020

.equ de2_nios2_interrupt_address,0x800020

#
# End of device-address definitions
#
################################################################

################################################################
#
# Definition of variables for keeping system time etcetera.
#

.data
.align 2
.global oslab_internal_globaltime
oslab_internal_globaltime:  .word 0

# Definition of system (interrupt) stack, sp, and gp

.data
.align 2
oslab_internal_gp:  .word 0
oslab_internal_sp:  .word 0
oslab_system_stack: .fill 256,1,0
oslab_system_stacktop:

#
# End of system-time variable definitions.
#
################################################################

################################################################
#
# Interrupt handling code.
#

# Stub for interrupt handler

.text
oslab_internal_stub:
    movia   et,oslab_exception_handler
    jmp     et

# The interrupt handler

oslab_exception_handler:
    # If control comes here, we have established that the
    # exception was caused by an interrupt.
    # Subtract 4 from ea, so that the interrupted instruction
    # will be re-run when we return.

    subi    ea,ea,4

    # Check the source of the interrupt.
    # Possible source No. 1: Timer_1 (currently the only source).

    rdctl   et,ipending
    andi    et,et,de2_timer_1_intmask
    bne     et,r0,oslab_timer_1_interrupt

    # If control comes here, we have an interrupt from an unknown source.
    # This condition is IGNORED in this version of OSLAB.

    eret

oslab_timer_1_interrupt:

    # Acknowledge the timer_1 interrupt.

    movia   et,de2_timer_1_base
    stw     r0,0(et)

    # Save contents of R8, to get a free register for
    # temporary values.

    subi    sp,sp,4
    stw     r8,0(sp)        # PUSH r8

    # Increase system clock.

    movia   r8,oslab_internal_globaltime
    ldw     et,0(r8)
    addi    et,et,1
    stw     et,0(r8)

   	# Restore original contents of R8.

    ldw     r8,0(sp)        # POP r8
    addi    sp,sp,4

    eret

#
# End of exception handling code.
#
################################################################

################################################################
#
# Startup code.
#
# When the system is started, Altera-supplied code initializes the
# Nios II CPU and cache memories, and then calls alt_main.
#

.global alt_main
alt_main:
    wrctl   status,r0       # Disable interrupts.
    wrctl   ienable,r0      # Clear all bits in IENABLE.

    # Now copy the stub.

    movia   r8,oslab_internal_stub
    movia   r9,de2_nios2_interrupt_address
    ldw     r10,0(r8)
    stw     r10,0(r9)
    ldw     r10,4(r8)
    stw     r10,4(r9)
    ldw     r10,8(r8)
    stw     r10,8(r9)

    # Initialize timer_1.

    movia   r8,de2_timer_1_base
    movia   r9,de2_timer_1_timeout_value
    srli    r10,r9,16
    stw     r10,12(r8)      # Write periodh
    andi    r10,r9,0xffff
    stw     r10,8(r8)       # Write periodl
    movi    r10,7           # Continuous, interrupt on timeout, and start
    stw     r10,4(r8)

    # Initialize CPU for interrupts from timer_1.
	
    movi    r10,de2_timer_1_intmask
    wrctl   ienable,r10
    movi    r10,1
    wrctl   status,r10

    # Call to main. Do not jump, main is a subroutine,
    # and may execute a ret instruction.

    subi    sp,sp,4
    stw     ra,0(sp)        # PUSH r31
    movia   r8,main
    callr   r8
    ldw     ra,0(sp)        # POP r31
    addi    sp,sp,4

    # If main returns, we will return directly to the routine
    # that called us (that called alt_main).

    ret

#
# End of startup code.
#
################################################################

################################################################
#
# Helper functions for initialization and thread handling.
#

.global oslab_begin_critical_region
oslab_begin_critical_region:
    wrctl   status,r0
    ret

.global oslab_end_critical_region
oslab_end_critical_region:
    movi    r8,1
    wrctl   status,r8
    ret

.global oslab_get_internal_globaltime
oslab_get_internal_globaltime:
    movia   r2,oslab_internal_globaltime
    ldw     r2,0(r2)
    ret

.global clearGlobaltime
clearGlobaltime:
	movia r10, oslab_internal_globaltime
	stw r0, 0(r10)
	
	ret
	
.global disableInterrupt
disableInterrupt:
    wrctl   status,r0
    ret

.global enableInterrupt
enableInterrupt:
    movi    r8,1
    wrctl   status,r8
    ret

#
# End of helper functions.
#
################################################################
#
# ********************************************************
# *** You don't have to study the code below this line ***
# ********************************************************
#
################################################################
#
# A simplified printf() replacement.
# Implements the following conversions: %c, %d, %s and %x.
# No format-width specifications are allowed,
# for example "%08x" is not implemented.
# Up to four arguments are accepted, i.e. the format string
# and three more. Any extra arguments are silently ignored.
#
# The printf() replacement relies on routines
# out_char_uart_0, out_hex_uart_0,
# out_number_uart_0 and out_string_uart_0
# in file oslab_lowlevel_c.c
#
# We need the macros PUSH and POP - definitions follow.

# PUSH reg - push a single register on the stack



.text
.global printf
printf:
    PUSH    ra		# PUSH return address register r31.
    PUSH    r16     # R16 will point into format string.
    PUSH    r17     # R17 will contain the argument number.
    PUSH    r18     # R18 will contain a copy of r5.
    PUSH    r19     # R19 will contain a copy of r6.
    PUSH    r20     # R20 will contain a copy of r7.
    mov     r16,r4  # Get format string argument
    movi    r17,0   # Clear argument number.
    mov     r18,r5  # Copy r5 to safe place.
    mov     r19,r6  # Copy r6 to safe place.
    mov     r20,r7  # Copy r7 to safe place.
asm_printf_loop:
    ldb     r4,0(r16)   # Get a byte of format string.
    addi    r16,r16,1   # Point to next byte
    # End of format string is marked by a zero-byte.
    beq     r4,r0,asm_printf_end
    cmpeqi  r9,r4,92    # Check for backslash escape.
    bne     r9,r0,asm_printf_backslash
    cmpeqi  r9,r4,'%'   # Check for percent-sign escape.
    bne     r9,r0,asm_printf_percentsign
asm_printf_doprint:
    # No escapes present, just print the character.
    movia   r8,out_char_uart_0
    callr   r8
    br      asm_printf_loop
asm_printf_backslash:
    # Preload address to out_char_uart_0 into r8.
    movia   r8,out_char_uart_0
    ldb     r4,0(r16)	# Get byte after backslash
    addi    r16,r16,1   # Increase byte count.
    # Having a backslash at the end of the format string
    # is illegal, but must not crash our printf code.
    beq     r4,r0,asm_printf_end
    cmpeqi  r9,r4,'n'   # Newline
    beq     r9,r0,asm_printf_backslash_not_newline
    movi    r4,10       # Newline
    callr   r8
    br      asm_printf_loop
asm_printf_backslash_not_newline:
    cmpeqi  r9,r4,'r'   # Return
    beq     r9,r0,asm_printf_backslash_not_return
    movi    r4,13       # Return
    callr   r8
    br      asm_printf_loop
asm_printf_backslash_not_return:
    # Unknown character after backslash - ignore.
    br      asm_printf_loop
asm_printf_percentsign:
    addi    r17,r17,1	# Increase argument count.
    cmpgei  r8,r17,4    # Check against maximum argument count.
    # If maximum argument count exceeded, print format string.
    bne     r8,r0,asm_printf_doprint
    cmpeqi  r9,r17,1    # Is argument number equal to 1?
    beq     r9,r0,asm_printf_not_r5	# beq jumps if cmpeqi false
    mov     r4,r18      # If yes, get argument from saved copy of r5.
    br      asm_printf_do_conversion
asm_printf_not_r5:
    cmpeqi  r9,r17,2    # Is argument number equal to 2?
    beq     r9,r0,asm_printf_not_r6	# beq jumps if cmpeqi false
    mov     r4,r19      # If yes, get argument from saved copy of r6.
    br      asm_printf_do_conversion
asm_printf_not_r6:
    cmpeqi  r9,r17,3    # Is argument number equal to 3?
    beq     r9,r0,asm_printf_not_r7	# beq jumps if cmpeqi false
    mov     r4,r20       # If yes, get argument from saved copy of r7.
    br      asm_printf_do_conversion
asm_printf_not_r7:
    # This should not be possible.
    # If this strange error happens, print format string.
    br      asm_printf_doprint
asm_printf_do_conversion:
    ldb     r8,0(r16)	# Get byte after percent-sign.
    addi    r16,r16,1   # Increase byte count.
    cmpeqi  r9,r8,'x'   # Check for %x (hexadecimal).
    beq     r9,r0,asm_printf_not_x
    movia   r8,out_hex_uart_0
    callr   r8
    br      asm_printf_loop
asm_printf_not_x:
    cmpeqi  r9,r8,'d'   # Check for %d (decimal).
    beq     r9,r0,asm_printf_not_d
    movia   r8,out_number_uart_0
    callr   r8
    br      asm_printf_loop
asm_printf_not_d:
    cmpeqi  r9,r8,'c'   # Check for %c (character).
    beq     r9,r0,asm_printf_not_c
    # Print character argument.
    br      asm_printf_doprint
asm_printf_not_c:
    cmpeqi  r9,r8,'s'   # Check for %s (string).
    beq     r9,r0,asm_printf_not_s
    movia   r8,out_string_uart_0
    callr   r8
    br      asm_printf_loop
asm_printf_not_s:
asm_printf_unknown:
    # We do not know what to do with other formats.
    # Print the format string text.
    movi    r4,'%'
    movia   r8,out_char_uart_0
    callr   r8
    ldb     r4,-1(r16)
    br      asm_printf_doprint
asm_printf_end:
    POP     r20
    POP     r19
    POP     r18
    POP     r17
    POP     r16
    POP     ra
    ret

#
# End of simplified printf() replacement code.
#
################################################################
.end










