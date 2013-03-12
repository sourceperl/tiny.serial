/* 
  sample UART software
  transmit serial data at 9600,N,8,1
  code for avr-gcc
           ATTiny85 at 8 MHz


  code in public domain
*/

#define F_CPU 8000000UL
#define USE_PRINTF 
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) // clear bit
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))  // set bit

/* ATTiny85 IO pins

             ___^___
           -|PB5 VCC|-
LED        -|PB3 PB2|-
serial out -|PB4 PB1|-
           -|GND PB0|-
             -------
*/

/* prototypes */
// main routines
void setup(void);
void loop(void);
int main(void);
// misc routines
void init_printf(void);
int serial_putc(char c, FILE *file);
void serial_write(uint8_t tx_byte);
uint64_t millis(void);

/* some vars */
volatile uint64_t _millis    = 0;
volatile uint16_t _1000us    = 0;
uint64_t old_millis = 0;

// must be volatile (change and test in main and ISR)
volatile uint8_t tx_buzy = 0;
volatile uint8_t bit_index;
volatile uint8_t _tx_buffer; 

/*** ISR ***/

// compare match interrupt service for OCR0A
// call every 103us
ISR(TIM0_COMPA_vect) { 
  // software UART
  // send data
  if (tx_buzy) {
    if (bit_index == 0) {
      // start bit (= 0)
      cbi(PORTB, PB4);
    } else if (bit_index <=8) {
      // LSB to MSB
      if (_tx_buffer & 1) {
        sbi(PORTB, PB4);
      } else {
        cbi(PORTB, PB4);
      }
      _tx_buffer >>= 1;        
    } else if (bit_index >= 9) {
      // stop bit (= 1)
      sbi(PORTB, PB4);
      tx_buzy = 0;
    }
    bit_index++;
  }
  // millis update
  _1000us += 103;
  while (_1000us > 1000) {
    _millis++;
    _1000us -= 1000;
  }
}

/*** UART routines ***/
// send serial data to software UART, block until UART buzy
void serial_write(uint8_t tx_byte) {
  while(tx_buzy);
  bit_index  = 0;
  _tx_buffer = tx_byte;
  tx_buzy = 1;
}
#ifdef USE_PRINTF
/*** connect software UART to stdio.h ***/
void init_printf() {
  fdevopen(&serial_putc, 0);
}

int serial_putc(char c, FILE *file) {
  serial_write(c);
  return c;
}
#else
void serial_print(const char *str) {
  uint8_t i;
  for (i = 0; str[i] != 0; i++) {
    serial_write(str[i]);
  }
}
#endif

/*** misc routines ***/

// safe access to millis counter
uint64_t millis() {
  uint64_t m;
  cli();
  m = _millis;
  sei();
  return m;
}

/*** main routines ***/

void setup(void) {
  // LED IO
  sbi(DDRB,  PB3); // set LED pin as output
  sbi(PORTB, PB3); // turn the LED on
  // Software UART IO
  sbi(DDRB,  PB4); // PB4 as output
  sbi(PORTB, PB4); // serial idle level is '1'
  /* interrup setup */
  // call ISR(TIM0_COMPA_vect) every 103us (for 9600 bauds)
  // set CTC mode : clear timer on comapre match
  // -> reset TCNTO (timer 0) when TCNTO == OCR0A
  sbi(TCCR0A, WGM01);
  // prescaler : clk/8 (1 tic = 1us for 8MHz clock)
  sbi(TCCR0B, CS01);
  // compare register A at 103 us
  OCR0A = 103;
  // interrupt on compare match OCROA == TCNT0
  sbi(TIMSK, OCIE0A);
  // Enable global interrupts
  sei();
  #ifdef USE_PRINTF
  // init stdout = serial
  init_printf();
  #endif
}

void loop(void) { 
  // every 100ms toggle LED
  if ((millis() - old_millis) > 2000) {
   // Toggle Port B pin 3 output state
   PORTB ^= 1<<PB3;
   old_millis = millis();
   #ifdef USE_PRINTF   
   printf("toggle LED\r\n");
   #else 
   serial_print("toggle LED\r\n");
   #endif
  }
}

/*
  Arduino like
*/
int main(void) {
  setup();
  for(;;) {
    loop();
  }
};
