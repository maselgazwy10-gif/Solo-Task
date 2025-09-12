#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define SET_PIN_LOW(port, pin)  ((port) &= ~(1 << (pin)))
#define SET_PIN_HIGH(port, pin) ((port) |= (1 << (pin)))
#define TOGGLE_PIN(port, pin)   ((port) ^= (1 << (pin)))

void LED_setup(int pin)
{
	DDRD |= (1 << pin);
}

void buzzer_setup(int pin)
{
	DDRD |= (1 << pin);
}

// --- UART Setup ---
void UART_setup(uint32_t baud)
{
	uint16_t ubrr = F_CPU/16/baud-1;
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)ubrr;

	UCSR0B = (1 << RXEN0);  // Enable receiver
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

// --- Receive one char ---
char UART_receive_char(void)
{
	while (!(UCSR0A & (1 << RXC0))); // Wait until data received
	return UDR0;
}

// --- Receive distance as char ---
char receive_distance(void)
{
	char c;
	c = UART_receive_char();
	return c; // '1' for <=50cm, '0' for >50cm
}

int main(void) {
	char trigger;

	// LED output (Pin 7)
	LED_setup(PD7);
	buzzer_setup(PD2);

	UART_setup(9600);

	while (1) {

		trigger = receive_distance();

		// --- Toggle LED/Buzzer exactly like original code ---
		if (trigger == '1') {
			TOGGLE_PIN(PORTD, PD7);  // Toggle LED
			TOGGLE_PIN(PORTD, PD2);  // Toggle Buzzer
			} else {
			SET_PIN_LOW(PORTD, PD7); // LED OFF
			SET_PIN_LOW(PORTD, PD2); // Buzzer OFF
		}

		_delay_ms(100);
	}

	return 0;
}
