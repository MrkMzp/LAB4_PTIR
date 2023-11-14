#define F_CPU 16000000UL

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

typedef uint16_t temperature_table_entry_type;
typedef uint8_t temperature_table_index_type;

#define TEMPERATURE_UNDER -300
#define TEMPERATURE_OVER 340
#define TEMPERATURE_TABLE_START -300
#define TEMPERATURE_TABLE_STEP 20
#define TEMPERATURE_TABLE_READ(i) pgm_read_word(&termo_table[i])

#define ADC_VOLT(x) (x * (5.0 / 1024.0))

volatile uint8_t dig[3] = {0 , 0 , 0};
volatile uint8_t temperature = 0;
volatile uint16_t adc_volts = 0; 



void DrawDigit(uint8_t digit)
{
	if(digit == 0)
	{
		PORTD &= 0xD0;
		PORTC &=  0x34;
	}
	if(digit == 1)
	{
		PORTD &= 0x80;
		PORTC &=  0x04;
	}
	if(digit == 2)
	{
		PORTD &= 0xB0;
		PORTC &=  0x30;
	}
	if(digit == 3)
	{
		PORTD &= 0xB0;
		PORTC &=  0x14;
	}
	if(digit == 4)
	{
		PORTD &= 0xC0;
		PORTC &=  0x14;
	}
	if(digit == 5)
	{
		PORTD &= 0x70;
		PORTC &=  0x14;
	}
	if(digit == 6)
	{
		PORTD &= 0x70;
		PORTC &=  0x34;
	}
	if(digit == 7)
	{
		PORTD &= 0xA0;
		PORTC &=  0x04;
	}
	if(digit == 8)
	{
		PORTD &= 0xF0;
		PORTC &=  0x34;
	}
	if(digit == 9)
	{
		PORTD &= 0xF0;
		PORTC &=  0x14;
	}
	return;
}

void ClearDisplay()
{
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	return;
}

void ShowDegrees()
{
	//first digit (x 0 0)
	PORTB = 0x04;
	if(dig[2] == 0)
	{
		PORTD = 0x10;
	}
	ClearDisplay();
	
	//second (0 x 0)
	PORTD = 0x02;
	DrawDigit(dig[1]);
	ClearDisplay();
	
	//third (0 0 x)
	PORTB = 0x02;
	DrawDigit(dig[0]);
	ClearDisplay();
	
	return;
}

const temperature_table_entry_type termo_table[] PROGMEM = 
{
    960, 952, 944, 935, 925, 914, 903, 891,
    878, 865, 850, 835, 819, 802, 785, 767,
    748, 729, 709, 689, 669, 648, 627, 606,
    585, 564, 543, 522, 502, 482, 462, 442,
    423
};

int16_t calc_temperature(temperature_table_entry_type adcsum) 
{
  temperature_table_index_type l = 0;
  temperature_table_index_type r = (sizeof(termo_table) / sizeof(termo_table[0])) - 1;
  temperature_table_entry_type thigh = TEMPERATURE_TABLE_READ(r);
  
  if (adcsum <= thigh) 
  {
    #ifdef TEMPERATURE_UNDER
      if (adcsum < thigh) 
        return TEMPERATURE_UNDER;
    #endif
    return TEMPERATURE_TABLE_STEP * r + TEMPERATURE_TABLE_START;
  }
  temperature_table_entry_type tlow = TEMPERATURE_TABLE_READ(0);
  if (adcsum >= tlow) {
    #ifdef TEMPERATURE_OVER
      if (adcsum > tlow)
        return TEMPERATURE_OVER;
    #endif
    return TEMPERATURE_TABLE_START;
  }


  while ((r - l) > 1) 
  {
    temperature_table_index_type m = (l + r) >> 1;
    temperature_table_entry_type mid = TEMPERATURE_TABLE_READ(m);
    if (adcsum > mid) 
    {
      r = m;
    } else 
    {
      l = m;
    }
  }
  temperature_table_entry_type vl = TEMPERATURE_TABLE_READ(l);
  if (adcsum >= vl) {
    return l * TEMPERATURE_TABLE_STEP + TEMPERATURE_TABLE_START;
  }
  temperature_table_entry_type vr = TEMPERATURE_TABLE_READ(r);
  temperature_table_entry_type vd = vl - vr;
  int16_t res = TEMPERATURE_TABLE_START + r * TEMPERATURE_TABLE_STEP; 
  if (vd) 
  {
    res -= ((TEMPERATURE_TABLE_STEP * (int32_t)(adcsum - vr) + (vd >> 1)) / vd);
  }
  return res;
}

void initADC() 
{
    ADMUX |= (1 << REFS0);
    ADMUX |= (1 << MUX0);
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRA |= (1 << ADIE);
    ADCSRA |= (1 << ADEN);
}

uint16_t read_adc() {

    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC));
    
    return  (ADCH << 8) | ADCL;
}

ISR(ADC_vect) 
{ 
	adc_volts = ADC_VOLT(read_adc());
}



int main()
{
	// Init ports for LEDs
	DDRB = 0x07;
	PORTB = 0x00;
	
	DDRD = 0xF4;
	PORTD = 0x00;
	
	DDRC = 0x3C;
	PORTC = 0x00;
	
	initADC();
    sei();
    
    int something;
    
    
	short degree = 0;
    while (1) 
    {		
		degree = calc_temperature(adc_volts) / 10;
		
		dig[0] = degree %10;
		dig[1] = ( degree / 10 ) % 10;
		if(degree >= 0)
		{
			 dig[2] = 0;
		}else
		{
			 dig[2] = 1;
		}
		
		ShowDegrees();
		
		if(degree <= 10)
		{
			PORTB |= 0x04;
		}else if(degree > 10)
		{
			PORTB &= 0xFB;	
		}
    }
    
	return 0;
}
    
