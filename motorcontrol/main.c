// Används för att gå framåt
void forward() {
 PORTC.F3 = 1;
 Delay_ms(1);
 PORTC.F3 = 0;
}

// För backning
void back() {
 PORTC.F3 = 1;
 Delay_ms(2);
 PORTC.F3 = 0;
}

void main() {
  // Gör port B och C digital
  TRISB = 0; TRISC = 0;

  // Sätt portB som ingång
  PORTB = 0xFF;
  // Sätt portC som utgång
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
