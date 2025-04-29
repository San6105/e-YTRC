#define F_CPU 14745600
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include "lcd.c"

void port_init();
void timer5_init();
void velocity(unsigned char, unsigned char);
void motors_delay();

unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
unsigned char flag = 0;
unsigned char Left_white_line = 0;
unsigned char Center_white_line = 0;
unsigned char Right_white_line = 0;
unsigned char IR1 = 0;
unsigned char IR5 = 0;
unsigned char node = 0;
unsigned char CF_buzzer_count = 0;
unsigned char HU_buzzer_count = 0;
unsigned char RC_buzzer_count = 0;
unsigned int value;

void lcd_port_config(void) {
	DDRC = DDRC | 0xF7;
	PORTC = PORTC & 0x80;
}

void adc_pin_config(void) {
	DDRF = 0x00;
	PORTF = 0x00;
	DDRK = 0x00;
	PORTK = 0x00;
}

void motion_pin_config(void) {
	DDRA = DDRA | 0x0F;
	PORTA = PORTA & 0xF0;
	DDRL = DDRL | 0x18;   
	PORTL = PORTL | 0x18; 
}

void port_init() {
	lcd_port_config();
	adc_pin_config();
	motion_pin_config();
}

void timer5_init() {
	TCCR5B = 0x00; 
	TCNT5H = 0xFF; 
	TCNT5L = 0x01; 
	OCR5AH = 0x00; 
	OCR5AL = 0xFF; 
	OCR5BH = 0x00; 
	OCR5BL = 0xFF; 
	OCR5CH = 0x00; 
	OCR5CL = 0xFF; 
	TCCR5A = 0xA9; 
    TCCR5B = 0x0B; 
}

void adc_init() {
    ADCSRA = 0x00;
    ADCSRB = 0x00; 
    ADMUX = 0x20;  
    ACSR = 0x80;
    ADCSRA = 0x86; 
}

unsigned char ADC_Conversion(unsigned char Ch) {
    unsigned char a;
    if (Ch > 7) {
        ADCSRB = 0x08;
    }
    Ch = Ch & 0x07;  
    ADMUX = 0x20 | Ch;  
    ADCSRA = ADCSRA | 0x40; 
    while ((ADCSRA & 0x10) == 0); 
    a = ADCH;
    ADCSRA = ADCSRA | 0x10; 
    ADCSRB = 0x00;
    return a;
}

void print_sensor(char row, char column, unsigned char channel) {
    ADC_Value = ADC_Conversion(channel);
    lcd_print(row, column, ADC_Value, 3);
}

void velocity(unsigned char left_motor, unsigned char right_motor) {
    OCR5AL = (unsigned char)left_motor;
    OCR5BL = (unsigned char)right_motor;
}

void motion_set(unsigned char Direction) {
    unsigned char PortARestore = 0;

    Direction &= 0x0F;  
    PortARestore = PORTA;  
    PortARestore &= 0xF0;  
    PortARestore |= Direction; 
    PORTA = PortARestore; 
}

void forward(void) {
    motion_set(0x06);
	velocity(130,130);
}

void left(void) {
	motion_set(0x06);
	velocity(10,200);
}
void right(void) {
	motion_set(0x06);
	velocity(200,10);
}

void stop(void) {
    motion_set(0x00);
}

void buzzer_on(void) {
    unsigned char port_restore = 0;
    port_restore = PINC;
    port_restore = port_restore | 0x08;
    PORTC = port_restore;
}

void buzzer_off(void) {
    unsigned char port_restore = 0;
    port_restore = PINC;
    port_restore = port_restore & 0xF7;
    PORTC = port_restore;
}

void init_devices(void) {
    cli(); 
    port_init();
    adc_init();
    timer5_init();
    sei();  
}

int main() {
    init_devices();
    lcd_set_4bit();
    lcd_init();
	    
    while (1) {
        Left_white_line = ADC_Conversion(3);    
        Center_white_line = ADC_Conversion(2);  
        Right_white_line = ADC_Conversion(1); 
		IR1 = ADC_Conversion(4);  
		IR5 = ADC_Conversion(8);
        flag = 0;
		
        if (((Center_white_line > 0x28) && (Left_white_line > 0x28)) || 
            ((Center_white_line > 0x28) && (Right_white_line > 0x28)) || 
            ((Left_white_line > 0x28) && (Center_white_line > 0x28) && (Right_white_line > 0x28))) 
        {
            node = node + 1;
            lcd_print(2, 1, node, 3);

            switch (node) {
                case 1:
                    forward();
                    _delay_ms(1500);
                    break;

                case 2:
                    forward();
                    _delay_ms(500);
                    break;

                case 3:
                    forward();
                    _delay_ms(200);
                    stop();
                    _delay_ms(500);
                    if(IR1<50)
                    {
	                    buzzer_on();
	                    _delay_ms(1000);
	                    buzzer_off();
	                    _delay_ms(1000);
	                    RC_buzzer_count++;
                    }
                    break;

                case 4:
                    forward();
                    _delay_ms(200);
					stop();
					_delay_ms(500);
					if(IR1<50)
					{
						buzzer_on();
						_delay_ms(1000);
						buzzer_off();
						_delay_ms(1000);
						RC_buzzer_count++;
					}
					break;

                case 5:
                    stop();
					_delay_ms(500);
                    left();
                    _delay_ms(1600);
                    break;

                case 6:
				    forward();
				    _delay_ms(140);
                    stop();
                    _delay_ms(500);
                    left();
                    _delay_ms(1550);
                    break;

                case 7:
                  forward();
                  _delay_ms(220);
                  stop();
                  _delay_ms(350);
                  if(IR1<50 && IR5<50 )
                  {
	                  for(i=1;i=2;i++)
	                  {
		                  buzzer_on();
		                  _delay_ms(1000);
		                  buzzer_off();
		                  _delay_ms(1000);
		                  HU_buzzer_count=HU_buzzer_count+2;
	                  }
	                  break;
                  }
                  
                  if(IR1<50 || IR5<50 )
                  {
	                  buzzer_on();
	                  _delay_ms(1000);
	                  buzzer_off();
	                  _delay_ms(1000);
	                  HU_buzzer_count++;
	                  break;
                  }
                  break;

                case 8:
					forward();
					_delay_ms(220);
					stop();
					_delay_ms(350);
					if(IR1<50 && IR5<50 )
					{
						for(i=1;i=2;i++)
						{
							buzzer_on();
							_delay_ms(1000);
							buzzer_off();
							_delay_ms(1000);
							RC_buzzer_count=RC_buzzer_count+2;
						}
						break;
					}
					
					if(IR1<50 || IR5<50 )
					{
						buzzer_on();
						_delay_ms(1000);
						buzzer_off();
						_delay_ms(1000);
						RC_buzzer_count++;
					}
					break;

                case 9:
                   forward();
                   _delay_ms(220);
                   stop();
                   _delay_ms(350);
                   if(IR1<50 && IR5<50 )
                   {
	                   for(i=1;i=2;i++)
	                   {
		                   buzzer_on();
		                   _delay_ms(1000);
		                   buzzer_off();
		                   _delay_ms(1000);
		                   CF_buzzer_count=CF_buzzer_count+2;
	                   }
	                   break;
                   }
                   
                   if(IR1<50 || IR5<50 )
                   {
	                   buzzer_on();
	                   _delay_ms(1000);
	                   buzzer_off();
	                   _delay_ms(1000);
	                   CF_buzzer_count++;
                   }
                   break;
				   
                case 10:
				    forward();
				    _delay_ms(150);
                    stop();
                    _delay_ms(500);
                    right();
                    _delay_ms(1600);
                    break;

                case 11:
				    forward();
				    _delay_ms(150);
				    stop();
				    _delay_ms(500);
				    right();
				    _delay_ms(1600);
				    break;

                case 12:
                    forward();
                    _delay_ms(200);
                    stop();
                    _delay_ms(500);
                    if(IR5<50)
                    {
	                    buzzer_on();
	                    _delay_ms(1000);
	                    buzzer_off();
	                    _delay_ms(1000);
	                    CF_buzzer_count++;
                    }
                    break;

                case 13:
                    forward();
                    _delay_ms(200);
                    stop();
                    _delay_ms(500);
                    if(IR5<50)
                    {
	                    buzzer_on();
	                    _delay_ms(1000);
	                    buzzer_off();
	                    _delay_ms(1000);
	                    HU_buzzer_count++;
                    }
                    break;

                case 14:
                    stop();
                    _delay_ms(500);
                    left();
                    _delay_ms(1600);
                    break;

                case 15:
                    stop();
                    _delay_ms(500);
                    left();
                    _delay_ms(1600);
                    break;

                case 16:
                   forward();
                   _delay_ms(200);
                   stop();
                   _delay_ms(500);
                   for (unsigned char i = 0; i < CF_buzzer_count; i++)
                   {
	                   buzzer_on();
	                   _delay_ms(1000);
	                   buzzer_off();
	                   _delay_ms(1000);
                   }
                   break;

                case 17:
                    forward();
                    _delay_ms(200);
                    stop();
                    _delay_ms(500);
                    for (unsigned char i = 0; i < HU_buzzer_count; i++) {
	                    buzzer_on();
	                    _delay_ms(1000);
	                    buzzer_off();
	                    _delay_ms(1000);
                    }
                    for (unsigned char i = 0; i < HU_buzzer_count; i++) {
	                    buzzer_on();
	                    _delay_ms(1000);
	                    buzzer_off();
	                    _delay_ms(1000);
                    }
                    break;

                case 18:
                    forward();
                    _delay_ms(200);
                    stop();
                    _delay_ms(500);
                    forward();
                    _delay_ms(200);
                    stop();
                    _delay_ms(500);
                    for (unsigned char i = 0; i < RC_buzzer_count; i++)
                    {
	                    buzzer_on();
	                    _delay_ms(1000);
	                    buzzer_off();
	                    _delay_ms(1000);
                    }
                    break;

                case 19:
                    forward();
                    _delay_ms(500);
                    break;

                case 20:
                    forward();
                    _delay_ms(500);
					stop();
					_delay_ms(50000);
                    break;
            }
        }

        if ((Center_white_line > 0x28) && (flag==0)) {
            flag = 1;
            forward();
            velocity(140, 140);
        }

        if ((Left_white_line > 0x28) && (flag == 0)) {
            flag = 1;
            forward();
            velocity(55, 100);
        }

        if ((Right_white_line > 0x28) && (flag == 0)) {
            flag = 1;
            forward();
            velocity(100, 55);
        }
    }
}