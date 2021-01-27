// ATtiny85

void setup()
{
  // setup
  DDRB |= (1 << PORTB3); // Set PB3 as Output
  
  DDRB &= ~(1 << PORTB4); // Set PB4 as Input
  PORTB |= (1 << PORTB4); // Internal Pull up PB4
  
  while(1){
    
    if(!(PINB & (1 << PORTB4))){ // if logic high
      PORTB ^= (1 << PORTB3);
      while(!(PINB & (1 << PORTB4)));
      _delay_ms(30);
    }
    
  }
}

void loop()
{
}
