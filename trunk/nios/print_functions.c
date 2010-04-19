/*
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
#
# File version: 1.818
#
# Modification history for this file:
# v1.818(2009-18-05) Modified to only hold prinf-critical functions
#
################################################################
*/

#define DE2_UART_0_BASE  ( (volatile unsigned int *) 0x860 )

void out_char_uart_0( int c )
{
  /* Wait until transmitter is ready */
  while( (DE2_UART_0_BASE[2] & 0x40) == 0 );
  /* Now send character */
  DE2_UART_0_BASE[1] = c & 0xff;
}

int oslab_hexasc( int num )
{
  int tmp = num & 15;
  if( tmp > 9 ) tmp += ( 'A' - 10 );
  else tmp += '0';
  return( tmp );
}

void out_string_uart_0( char * cp )
{
  while( *cp )
  {
    out_char_uart_0( *cp );
    cp += 1;
  }
}

#define ITOA_BUFSIZ ( 24 )
void out_number_uart_0( int num )
{
  int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";
  
  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  out_string_uart_0( &itoa_buffer[ i + 1 ] );
}

void out_hex_uart_0( int num )
{
  out_char_uart_0( oslab_hexasc( num >> 28 ) );
  out_char_uart_0( oslab_hexasc( num >> 24 ) );
  out_char_uart_0( oslab_hexasc( num >> 20 ) );
  out_char_uart_0( oslab_hexasc( num >> 16 ) );
  out_char_uart_0( oslab_hexasc( num >> 12 ) );
  out_char_uart_0( oslab_hexasc( num >>  8 ) );
  out_char_uart_0( oslab_hexasc( num >>  4 ) );
  out_char_uart_0( oslab_hexasc( num       ) );
}







