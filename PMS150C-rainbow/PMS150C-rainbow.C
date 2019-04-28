/* 
  2019 abnielsen.com
  
  Thanks to Josh Levine for a bit of inspiration for the rainbow routine. 
  https://github.com/bigjosh/SimpleNeoPixelDemo/blob/master/SimpleNeopixelDemo/SimpleNeopixelDemo.ino
  */


#include	"extern.h"
	.WRITER package 8, 1, 7, 5, 6, 4, 3, 2, 8, 0x3F, 0x3F, 4 
 //SO8, checks disabled, connecting SOT-23 package with jumper wires instead

byte		red, green, blue; //Could save these three bytes by using the rgb EWORD directly ( rgb$0, rgb$1, rgb$2)
byte		mode;
byte		hueinc;
byte		firstinc;
EWORD		rgb;
word		pixels; //Only has to be a word if number of pixels > 255
word		firstPixelHue;
#define definedPIXELS 300;

bit			LED :	pa.6;
bit			BTN :	pa.4;

int count;

send1 MACRO
		SET1 LED;
		.DELAY 5;
		$ LED low;
	//	.DELAY 1; //Going around is enough delay
ENDM

send0 MACRO
		SET1 LED;
		.DELAY 2;
		$ LED low;
		.DELAY 2;
ENDM


void SendRGB (void) {
	DISGINT; //Let's not get interrupted

	.FOR bitno, <23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0> //Regular for() loop doesn't work, but at least the compiler can do the hard work
	if (rgb.bitno == 0) {
			send0;
		} else {
			send1;
		}
	.ENDM
	ENGINT;
}

void show (void) {
	rgb$0 = blue; //I lost track of MSB, LSB and endians.. This is what works. 
	rgb$1 = red;
	rgb$2 = green;
	SendRGB();
}

void clearLED (void) {
rgb = 0;
//pixels = definedPIXELS;
pixels = 300; //Debug
do {
SendRGB();
} while (--pixels);
.delay 2000; //If you want to make sure the LED-reset is caught, use a longer one. 

}		

void	FPPA0 (void)
{
	.ADJUST_IC	SYSCLK=IHRC/2		//	SYSCLK=IHRC/2

	count = 0;

	$  T16M     IHRC, /4, BIT15;    // BIT15 Time increment of each T16M  = 16MHz / 4 = 4 MHz¡C
                                        // generate INTRQ.T16 = 16,384 uS every 2^16 times
	ENGINT;
	$ INTEN T16;  // Enable the T16M interrupt

	$ LED out,low;
	$ BTN in, pull;
	count = 0;
	unsigned word hue = 0;
	firstPixelHue = 0;
	byte current;
	mode = 0;
	firstinc = 1;

	//Let's start by clearing LED's and going to sleep - we don't want anything to consume current if we restarted by mistake
	clearLED();
	rgb = 0;
	SendRGB();
	$ LED high;
	CLKMD           =   0xF4;   //  -> ILRC
    CLKMD.En_IHRC   =   0;      //  close IHRC
               while (1)
                {
                    STOPSYS;

					if (BTN == 0) break;  //  examine and determine whether toggle to STOPSYS or execute at high speed.
                }
                CLKMD   =   0x34;           //  -> IHRC / 2
				
	count = 0;


 
while (1) //Main loop
{
if ( BTN == 1) { //If button is not pressed
	pixels = definedPIXELS;

	if (mode < 3) { //Rainbow	
	hue = firstPixelHue;
	if (mode == 0) {
		hueinc = 5;
		firstinc = 1;
	}
	if (mode == 1) hueinc = 0;
	if (mode == 2) {
		hueinc = 10;
		firstinc = 0;
	}

	do {
	if (hue>=768) {  
      hue -= 768;
    }
	current = (hue & 0xFF);

	if (hue < 256) {
	red = ~current;
	green = current;
	blue = 0;
	show();
	}
 
	if (hue > 255 && hue < 512) {
	red = 0;
	green = ~current;
	blue = current;
	show();
	}
 
	if (hue > 511 && hue < 768) {
	red = current;
	green = 0;
	blue = ~current;
	show();
	}
	hue+=hueinc;
	} while (--pixels);


	.delay(8000); //Should be increased if fewer LED's are used
	firstPixelHue+=firstinc;
	if (firstPixelHue > 3072) firstPixelHue = 0; //Has to be reset sometime. 

	} //End rainbow

	if (mode == 3) { //Red - not too bright
		red = 150;
		green = 0;
		blue = 0;
		do {
	
		show();
		} while (--pixels);
	.delay(2000);
	}

	if (mode == 4) { //Green - not too bright
		red = 0;
		green = 150;
		blue = 0;
		do {
		show();
		} while (--pixels);
	.delay(2000);
	}

		if (mode == 5) { //Blue - not too bright
		red = 0;
		green = 0;
		blue = 125;
		do {
		show();
		} while (--pixels);
	.delay(2000);
	}

	if (mode == 6) { //Princess! - not too bright
		green = 0;
		red = 200;
		blue = 200;
		do {
		show();
		} while (--pixels);
	.delay(2000);
	}

	} else { //Button pressed - go to sleep
	clearLED();
	rgb = 0;
	SendRGB();
	$ LED high; //I think I remember something about setting the WS2812B signal line high, reduces leak current. Maybe not.

	if (count > 10) { //Unless we just woke up go to sleep

	//Maybe disable wakeup from other pins - PADIER
	CLKMD           =   0xF4;   //  -> ILRC
    CLKMD.En_IHRC   =   0;      //  close IHRC
               while (1)
                {
                    STOPSYS;

					if (BTN == 0) break;  //  examine and determine whether toggle to STOPSYS or execute at high speed.
                }
                CLKMD   =   0x34;           //  -> IHRC / 2

			mode++;
			if (mode > 6) mode = 0;
			}

/*	//Change mode if button held longer when coming out of sleep
	count = 0;
	while (count < 30) {
		if (BTN == 1) {
		
		break;
		}
	}
*/
	count = 0;

	}


//      ...
//      wdreset;

	} 
}


void	Interrupt (void)
{
	pushaf;
	
	if (Intrq.T16)
	{	
		Intrq.T16	=	0;

		count ++;  // 16,384uS 61 == 999,424 uS ¡Ü 1S

	}


	popaf;
}

