#define F_computer 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ST7735.h"
#include "LCD_GFX.h"
#include <stdlib.h>

int x_mid = 80;
int y_mid = 64;
int radius = 2;
int boundary = 1;
int paddle_thickness = 1;
int paddle_speed = 4;

int computer_paddle_y0 = 51;
int computer_paddle_y1 = 77;
int computer_paddle_x0 = 4;
int computer_paddle_x1 = 5;
int player_paddle_y0 = 51;
int player_paddle_y1 = 77;
int player_paddle_x0 = 154;
int player_paddle_x1 = 155;

int boundary_limit = 1;

int computer_score = 0, player_score = 0, score_limit = 3;
int computer_roundScore = 0, player_roundScore = 0, max_roundScore = 3;
int computer_score_x = 9, computer_score_y = 5;
int player_score_x = 146, player_score_y = 5;

int gameMode;

void timer2_initialize()
{
	//Timer 2 setup for Passive Buzzer

	//Prescale Timer 2 by 256
	TCCR2B &= ~(1 << CS20);
	TCCR2B |= (1 << CS21);
	TCCR2B |= (1 << CS22);

	//CTC Waveform Generation Mode with OCR2A
	TCCR2A &= ~(1 << WGM20);
	TCCR2A |= (1 << WGM21);
	TCCR2B &= ~(1 << WGM22);

	//Set Timer 2 Compare Match to toggle PIN OC2B (PD3) on OCR2B
	TCCR2A |= (1<<COM2B0);
	TCCR2A &= ~(1<<COM2B1);

	OCR2A = 14;
	OCR2B = 8;

	//Clear Output Compare Interrupt Flag
	TIFR2 |= (OCF2A);
}

void adc_initialize()
{
	PRR &= ~(1<<PRADC);

	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);

	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);

	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);

	ADCSRA |= (1<<ADATE);
	ADCSRB &= ~(1<<ADTS0);
	ADCSRB &= ~(1<<ADTS1);
	ADCSRB &= ~(1<<ADTS2);

	DIDR0 |= (1<<ADC0D);
	ADCSRA |= (1<<ADEN);

	ADCSRA |= (1<< ADSC);
}

void Initialize()
{
	cli();

	DDRC &= ~(1 << DDC4);
	DDRC &= ~(1 << DDC5);
	
	DDRD &= ~(1 << DDD3); // Timer 2 OC2B ie PIN D3 for Buzzer

	DDRD &= ~(1 << DDD4); //PIN for Switch taking joystick select line for mode
	
	PORTD |= (1 << PORTD4); //Pulled up High as joystick default is floating

	DDRD &= ~(1 << DDD7); //PIN Taking switch input for pausing the gametmer

	//Enable Pin Change Interrupt at D7
	PCICR |= (1 << PCIE2);
	PCMSK2 |= (1 << PCINT23);

	DDRC |= (1 << DDC2); //PIN D2 for LED

	//PIN B4 for LED
	//DDRB |= (1 << DDB4);
	DDRC |= (1<<DDC1);
	lcd_init();
	adc_initialize();
	timer2_initialize();

	sei();
}

int pauseFlag = 0, pause = 0;

ISR(PCINT2_vect)
{
	
	if(!pauseFlag)
	{
		pauseFlag = 1;
		pause = !pause;
	}
	else
	pauseFlag = 0;
}


void game_setup()
{
	LCD_setScreen(CYAN); //Game Board Background

	LCD_drawBlock(0, 0, 0, LCD_HEIGHT - boundary, RED); //boundary
	LCD_drawBlock(LCD_WIDTH - boundary, 0, LCD_WIDTH - boundary, LCD_HEIGHT - boundary, RED); //boundary
	LCD_drawBlock(0, 0, LCD_WIDTH - boundary, 0, RED); //boundary
	LCD_drawBlock(0, LCD_HEIGHT - boundary, LCD_WIDTH - boundary, LCD_HEIGHT - boundary, RED); //boundary

	LCD_drawBlock(computer_paddle_x0, computer_paddle_y0, computer_paddle_x1, computer_paddle_y1, BLUE); //computer Paddle (LEFT)
	LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, BLUE); //Player Paddle (RIGHT)

	drawCircle(x_mid, y_mid, radius, BLACK); //Bresenham Filled ball
}

void move(int x_curr, int y_curr, int x_r, int y_r)
{
	int paddle_regen_flag = 0;
	
	while(1) //Loop to update circles and exit when the ball reaches a boundary
	{
		while(pause == 1);
		drawCircle(x_curr, y_curr, radius, CYAN);

		if(x_curr + radius >= LCD_WIDTH - boundary - 1 - 1)
		{
			computer_score++;
			outSound();
			lightLED(0);
			break;
		}
		else if(x_curr - radius <= 0 + boundary + 1 + 1)
		{
			player_score++;
			outSound();
			lightLED(1);
			break;
		}
		else if(y_curr + radius >= LCD_HEIGHT - 1 - boundary || y_curr - radius <= 0 + boundary + 1)
		{
			bounceSound();
			y_r = -y_r;
		}
		else if(x_curr + x_r + radius >= player_paddle_x0 - 1 && ((y_curr + y_r + radius >= player_paddle_y0) && (y_curr + y_r + radius <= player_paddle_y1)) || ((y_curr + y_r - radius <= player_paddle_y0) && (y_curr + y_r - radius >= player_paddle_y1))) //
		{
			bounceSound();
			x_r = -x_r;
			LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, BLUE);

			if(paddle_regen_flag == 0)
			paddle_regen_flag = 1;
		}
		else if(x_curr + x_r - radius <= computer_paddle_x1 + 1 && ((y_curr + y_r + radius >= computer_paddle_y0) && (y_curr + y_r + radius <= computer_paddle_y1)) || ((y_curr + y_r - radius <= computer_paddle_y0) && (y_curr + y_r - radius >= computer_paddle_y1))) //
		{
			bounceSound();
			x_r = -x_r;
			LCD_drawBlock(computer_paddle_x0, computer_paddle_y0, computer_paddle_x1, computer_paddle_y1, BLUE);

			if(paddle_regen_flag == 0)
			paddle_regen_flag = 1;
		}
		
		x_curr += x_r;
		y_curr += y_r;

		drawCircle(x_curr, y_curr, radius, BLACK);

		if(paddle_regen_flag == 1)
		{
			LCD_drawBlock(computer_paddle_x0, computer_paddle_y0, computer_paddle_x1, computer_paddle_y1, BLUE);
			LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, BLUE);
			paddle_regen_flag = 0;
		}

		//computer PADDLE
/*
		if(boundary_limit == 1)
		computer_paddle_up();
		else
		computer_paddle_down();
*/
		//PLAYER PADDLE
		player_paddle();

		updateScore(computer_score_x, computer_score_y, computer_score);
		updateScore(player_score_x, player_score_y, player_score);
	}

	Delay_ms(5000);

	updateScore(computer_score_x, computer_score_y, computer_score);
	updateScore(player_score_x, player_score_y, player_score);

	if(player_score == score_limit)
	{
		computer_score = 0;
		player_score = 0;
		player_roundScore++;
		updateRoundScore();
		if(player_roundScore == max_roundScore)
		{
			computer_roundScore = 0;
			player_roundScore = 0;
			winBanner(0);
		}
	}
	else if(computer_score == score_limit)
	{
		computer_score = 0;
		player_score = 0;
		computer_roundScore++;
		updateRoundScore();
		if(computer_roundScore == max_roundScore)
		{
			computer_roundScore = 0;
			player_roundScore = 0;
			winBanner(1);
		}
	}
}

void bounceSound()
{
	DDRD |= (1 << DDD3);
	Delay_ms(100);
	DDRD &= ~(1 << DDD3);
}

void outSound()
{
	for(int i = 0; i < 10; i++)
	{
		DDRD |= (1 << DDD3);
		Delay_ms(200);
		DDRD &= ~(1 << DDD3);
		Delay_ms(200);
	}
}

void lightLED(int computer_player)
{
	if(computer_player == 0)
	{
		PORTC |= (1 << PORTC2);
		Delay_ms(1000);
		PORTC &= ~(1 << PORTC2);
	}
	else
	{
		PORTC |= (1 << PORTC1);
		Delay_ms(1000);
		PORTC &= ~(1 << PORTC1);
	}
}

void player_paddle()
{
	int control;
	int control_pvp;
	////////////////////////////////////////////////////////////////////////

	if(gameMode == 0)
	{
	control = ADC;
	}
	else
	{
		control_pvp = ADC;
		if(PINC & (1<<PINC4))
		{
			control = 1023;
		}
		else if(PINC & (1<<PINC5))
		{
			control = 0;
		}
		else
		{
			control = 512;
		}
		
	}

	if (control >= 1000) //PADDLE UP
	{
		if (player_paddle_y0 - paddle_speed <= 1);
		else
		{
			LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, CYAN);
			player_paddle_y0 = player_paddle_y0 - paddle_speed;
			player_paddle_y1 = player_paddle_y1 - paddle_speed;
			LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, BLUE);
		}
	}
	
	else if (control_pvp >= 1000) //PADDLE UP
	{
		computer_paddle_up();
	}
	
	else if(control <= 24) //PADDLE DOWN
	{
		computer_paddle_down();
		if (player_paddle_y1 + paddle_speed > 126);
		else
		{
			LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, CYAN);
			player_paddle_y0 = player_paddle_y0 + paddle_speed;
			player_paddle_y1 = player_paddle_y1 + paddle_speed;
			LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, BLUE);
		}
	}
	
		else if (control_pvp <= 24) //PADDLE DOWN
		{
			computer_paddle_down();
		}
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
/*
if(PINC & (1<<PINC4))
{
	control = 1023;
}
*/
/*				if (player_paddle_y0 - paddle_speed <= 1);
				else
				{
					LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, CYAN);
					player_paddle_y0 = player_paddle_y0 - paddle_speed;
					player_paddle_y1 = player_paddle_y1 - paddle_speed;
					LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, BLUE);
				}
}
*/
/*
else if(PINC & (1<<PINC5))
{
	control = 0;
}
*/
/*
if (player_paddle_y1 + paddle_speed > 126);
				else
				{
					LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, CYAN);
					player_paddle_y0 = player_paddle_y0 + paddle_speed;
					player_paddle_y1 = player_paddle_y1 + paddle_speed;
					LCD_drawBlock(player_paddle_x0, player_paddle_y0, player_paddle_x1, player_paddle_y1, BLUE);
				}
}
*/
/*
else
{
	control = 512;
}
}
*/
}

void computer_paddle_up()
{
	if (computer_paddle_y0 - paddle_speed <= 1)
	{
		boundary_limit = -1;
	}
	else
	{
		LCD_drawBlock(computer_paddle_x0, computer_paddle_y0, computer_paddle_x1, computer_paddle_y1, CYAN);
		computer_paddle_y0 = computer_paddle_y0 - paddle_speed;
		computer_paddle_y1 = computer_paddle_y1 - paddle_speed;
		LCD_drawBlock(computer_paddle_x0, computer_paddle_y0, computer_paddle_x1, computer_paddle_y1, BLUE);
	}
}

void computer_paddle_down()
{
	if (computer_paddle_y1 + paddle_speed > 126)
	{
		boundary_limit = 1;
	}
	else
	{
		LCD_drawBlock(computer_paddle_x0, computer_paddle_y0, computer_paddle_x1, computer_paddle_y1, CYAN);
		computer_paddle_y0 = computer_paddle_y0 + paddle_speed;
		computer_paddle_y1 = computer_paddle_y1 + paddle_speed;
		LCD_drawBlock(computer_paddle_x0, computer_paddle_y0, computer_paddle_x1, computer_paddle_y1, BLUE);
	}
}

void setRand(int *setVar)
{
	int temp = (rand() % radius) + 1;

	if(temp > radius/2)
	{
		*setVar = radius/2 - temp - (radius % 2 == 0 ? 0:1);
	}
	else
	{
		*setVar = temp;
	}

	*setVar = 3 * (*setVar);
}
///////////////////////////////

///////////////////////////////
void startScreen()
{
	char str[25];

	LCD_setScreen(BLACK); //Game Board Background

	sprintf(str, "ESE 519 : LAB 4");
	LCD_drawString(35, 25, str, WHITE, BLACK);
	sprintf(str, "PING PONG");
	LCD_drawString(35, 35, str, WHITE, BLACK);
	sprintf(str, "Mode Select:");
	LCD_drawString(35, 45, str, WHITE, BLACK);

	sprintf(str, "MODE 1: JOYSTICK");
	LCD_drawString(35, 60, str, WHITE, BLACK);

	sprintf(str, "MODE 2: BLYNK WIRELESS");
	LCD_drawString(35, 75, str, WHITE, BLACK);

	while(1)
	{
		if(!(PIND & (1 << PIND4)))
		{
			Delay_ms(50);
			while((PIND & (1 << PIND4)));
			break;
		}

		if(ADC >= 1000)
		{
			LCD_drawBlock(35, 84, 70, 85, BLACK);
			while(1)
			{
				Delay_ms(2000);
				LCD_drawBlock(35, 68, 85, 68, WHITE); //RED SELECT UNDERLINE
				Delay_ms(2000);
				LCD_drawBlock(35, 68, 85, 68, BLACK);
				if(ADC <= 24)
				break;
				if(!(PIND & (1 << PIND4)))
				{
					Delay_ms(50);
					while((PIND & (1 << PIND4)));
					gameMode = 0;
					break;
				}
			}
		}
		else if(ADC <= 24)
		{
			LCD_drawBlock(35, 68, 85, 69, BLACK);
			while(1)
			{
				Delay_ms(2000);
				LCD_drawBlock(35, 83, 70, 83, WHITE);
				Delay_ms(2000);
				LCD_drawBlock(35, 83, 70, 83, BLACK);

				if(ADC >= 1000)
				break;
				if(!(PIND & (1 << PIND4)))
				{
					Delay_ms(50);
					// if(!(PIND & (1 << PIND4)))
					while((PIND & (1 << PIND4)));
					gameMode = 1;
					break;
				}
			}
		}
		else if(ADC <= 24)
		{
			LCD_drawBlock(35, 68, 85, 69, BLACK);
			while(1)
			{
				Delay_ms(2000);
				LCD_drawBlock(35, 83, 70, 83, RED);
				Delay_ms(2000);
				LCD_drawBlock(35, 83, 70, 83, BLACK);
				if(ADC >= 1000)
				break;
				
				
				
				
				if(!(PIND & (1 << PIND4)))
				{
					Delay_ms(50);
					//if(!(PIND & (1 << PIND4)))
					while((PIND & (1 << PIND4)));
					gameMode = 1;
					break;
				}
			}
		}
	}

	Delay_ms(2000);
}

void winBanner(int winner)
{
	char *str = "You Win !!";
	char *str1 = "You Lose !!";

	LCD_drawBlock(30, 44, 130, 74, YELLOW); //Scoreboard
	if(winner == 0)
	LCD_drawString(45, 55, str, BLACK, YELLOW);
	else
	LCD_drawString(45, 55, str1, BLACK, YELLOW);

	Delay_ms(50000);

	LCD_drawBlock(30, 44, 130, 74, CYAN); //Clear Scoreboard
}

void updateRoundScore()
{
	char value;
	char *str = "ROUND SCORE: ";

	LCD_drawBlock(30, 44, 130, 74, GREEN); //Scoreboard

	LCD_drawString(35, 55, str, BLACK, GREEN);
	value = computer_roundScore + '0';
	LCD_drawChar(100, 55, value, BLACK, GREEN);
	LCD_drawChar(105, 55, ' ', BLACK, GREEN);
	LCD_drawChar(110, 55, '-', BLACK, GREEN);
	LCD_drawChar(115, 55, ' ', BLACK, GREEN);
	value = player_roundScore + '0';
	LCD_drawChar(120, 55, value, BLACK, GREEN);

	Delay_ms(50000);

	LCD_drawBlock(30, 44, 130, 74, CYAN); //Clear Scoreboard
}

void updateScore(int score_x, int score_y, int score)
{
	char value;
	value = score + '0';
	LCD_drawChar(score_x, score_y, value, BLACK, CYAN);
}

int main(void)
{
	Initialize();
	startScreen();
	game_setup();
	srand(0);

	int x_curr = x_mid, y_curr = y_mid;
	int x_r, y_r;

	while (1)
	{
		setRand(&x_r);
		setRand(&y_r);
		move(x_curr, y_curr, x_r, y_r);
	}
}
