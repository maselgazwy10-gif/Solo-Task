#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>

#define SET_PIN_LOW(port, pin)  ((port) &= ~(1 << (pin)))
#define SET_PIN_HIGH(port, pin) ((port) |= (1 << (pin)))
#define TOGGLE_PIN(port, pin)   ((port) ^= (1 << (pin)))

void setup_sensor(void)
{
	DDRD |= (1 << PD5);   // Trigger output (Pin 5)
	DDRD &= ~(1 << PD6);  // Echo input (Pin 6)
}

void timer_init(void)
{
	TCCR1A = 0;           // Normal mode
	TCCR1B = (1 << CS11);
	TCNT1 = 0;            // Reset timer
}

void send_trigger_pulse(void)
{
	// --- Send Trigger Pulse ---
	SET_PIN_LOW(PORTD, PD5);
	_delay_us(2);
	SET_PIN_HIGH(PORTD, PD5);
	_delay_us(10);
	SET_PIN_LOW(PORTD, PD5);
}

int calculate_distance(void)
{
	uint32_t start, end, duration;
	int distance;
	// --- Measure Echo ---
	while (!(PIND & (1 << PD6))); // Wait for Echo HIGH
	start = TCNT1;
	while (PIND & (1 << PD6));    // Wait for Echo LOW
	end = TCNT1;

	// Calculate duration in microseconds
	duration = (end - start) * 0.5; // prescaler=8, tick=0.5us

	// Convert duration to distance in cm using HC-SR04 approximation
	distance = duration / 58;
	return distance;
}

// --- UART Setup ---
void UART_setup(uint32_t baud)
{
	uint16_t ubrr = F_CPU/16/baud-1;
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)ubrr;

	UCSR0B = (1 << TXEN0);  // Enable transmitter
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

// --- Send one char via UART ---
void UART_send_char(char c)
{
	while (!(UCSR0A & (1 << UDRE0))); // Wait for empty transmit buffer
	UDR0 = c;
}

// --- Send distance as char ---
void send_distance(int distance)
{
	if(distance <= 50)
	{
		UART_send_char('1');
	}
	else
	{
		UART_send_char('0');
	}
}

int main(void)
{
	int distance;

	// --- Sensor Setup ---
	setup_sensor();
	timer_init();
	UART_setup(9600);

	while (1)
	{
		send_trigger_pulse();
		distance = calculate_distance();
		send_distance(distance);

		_delay_ms(100); // Adjust delay if needed
	}

	return 0;
}
