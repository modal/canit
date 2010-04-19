// Anv�nds f�r att g� fram�t
void forward() {
 PORTC.F3 = 1;
 Delay_ms(1);
 PORTC.F3 = 0;
}

// F�r backning
void back() {
 PORTC.F3 = 1;
 Delay_ms(2);
 PORTC.F3 = 0;
}

void main() {
  // G�r port B och C digital
  TRISB = 0; TRISC = 0;

  // S�tt portB som ing�ng
  PORTB = 0xFF;
  // S�tt portC som utg�ng
  PORTC = 0;
  
  // Huvudloop
  while(1 == 1) {
    if (PORTB.F7 & 1) {
      if (PORTB.F7 & 2) {
        forward();
      } else {
        back();
      }
    }
    Delay_ms(20);
  }
}
