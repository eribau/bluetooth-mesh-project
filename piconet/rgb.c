#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "iocontroller_rgb.h"

void delay(int);

int main(void){
	init_gpio();
	while(1){
		red_on();
		delay(2000);
		red_off();
		delay(500);
		green_on();
		delay(2000);
		green_off();
		delay(500);
		blue_on();
		delay(2000);
		blue_off();
		printf("fiesta inc");
		delay(500);
		red_on();
		delay(500);
		green_on();
		delay(500);
		red_off();
		blue_on();
		delay(500);
		green_off();
		red_on();
		delay(500);
		blue_off();
		red_off();
	}
}
