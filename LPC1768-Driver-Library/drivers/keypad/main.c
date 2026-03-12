#include <LPC17xx.h>
#include "keypad.h"
#include "lcd.h"

KEYPAD_Handle_t keypad =
{
    LPC_GPIO2,        // rowPort
    LPC_GPIO2,        // colPort
    {0,1,2,3},        // rowPins
    {4,5,6,7}         // colPins
};

int main(void)
{
    SystemInit();

    Keypad_Init(&keypad);
    LCD_Init();

    LCD_String("Press Key:");

    while(1)
    {
        char key = Keypad_GetKey(&keypad);

        if(key)
        {
            LCD_SetCursor(1,0);
            LCD_Data(key);
        }
    }
}
