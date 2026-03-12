#include <LPC17xx.h>
#include "lcd.h"

void delay_ms(int ms)
{
    int i,j;
    for(i=0;i<ms;i++)
        for(j=0;j<6000;j++);
}

int main(void)
{
	int count = 0;	
    SystemInit();

    LCD_Init();

    // Test 1 : Simple Message
    LCD_String("LCD DRIVER OK");

    LCD_SetCursor(1,0);
    LCD_String("TEST START");

    delay_ms(2000);

    // Test 2 : Clear Test
    LCD_Clear();
    LCD_String("CLEAR WORKING");

    delay_ms(2000);

    // Test 3 : Cursor Test
    LCD_Clear();
    LCD_SetCursor(0,3);
    LCD_String("CURSOR");

    LCD_SetCursor(1,5);
    LCD_String("TEST");

    delay_ms(2000);

    // Test 4 : Character Printing Test
    LCD_Clear();
    LCD_String("CHAR TEST");

    delay_ms(2000);

    // Test 5 : Counter Test
    

    while(1)
    {
        LCD_Clear();

        LCD_String("Counter:");

        LCD_SetCursor(1,0);

        LCD_Data((count/100)%10 + '0');
        LCD_Data((count/10)%10 + '0');
        LCD_Data((count)%10 + '0');

        count++;

        if(count > 999)
            count = 0;

        delay_ms(500);
    }
}
