#include "keypad.h"

// Key mapping
static const char keyMap[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// Small debounce delay
static void Keypad_Delay(void)
{
		volatile int i;
    for(i=0;i<30000;i++);
}

void Keypad_Init(KEYPAD_Handle_t *keypad)
{
		int i;
    // Configure rows as output
    for(i=0;i<4;i++)
    {
        keypad->rowPort->FIODIR |= (1 << keypad->rowPins[i]);
        keypad->rowPort->FIOSET |= (1 << keypad->rowPins[i]); // Default HIGH
    }

    // Configure columns as input
    for(i=0;i<4;i++)
    {
        keypad->colPort->FIODIR &= ~(1 << keypad->colPins[i]);
    }
}

char Keypad_GetKey(KEYPAD_Handle_t *keypad)
{
		int row,i,col;
    for(row=0; row<4; row++)
    {
        // Drive all rows HIGH
        for(i=0;i<4;i++)
            keypad->rowPort->FIOSET = (1 << keypad->rowPins[i]);

        // Drive current row LOW
        keypad->rowPort->FIOCLR = (1 << keypad->rowPins[row]);

        Keypad_Delay();

        // Check columns
        for(col=0; col<4; col++)
        {
            if(!(keypad->colPort->FIOPIN & (1 << keypad->colPins[col])))
            {
                Keypad_Delay(); // Debounce

                // Wait until key released
                while(!(keypad->colPort->FIOPIN & (1 << keypad->colPins[col])));

                return keyMap[row][col];
            }
        }
    }

    return 0; // No key pressed
}
