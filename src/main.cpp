#include <SPI.h>
#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"
#include "image.h"

void setup()
{
    pinMode(A14, INPUT);  // BUSY
    pinMode(A15, OUTPUT); // RES
    pinMode(A16, OUTPUT); // DC
    pinMode(A17, OUTPUT); // CS
    // SPI
    SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
    SPI.begin();
}

void loop()
{
    unsigned int i;

    /* Test four grayscale */

    // byte d1[4] = {0x00, 0x00, 0xff, 0xff};
    // byte d2[4] = {0x00, 0xff, 0x00, 0xff};
    // int number_of_fields = EPD_ARRAY / 4;

    // EPD_Init_4G(); // Full screen refresh initialization.

    // EPD_W21_WriteCMD(0x10); // write old data
    // for (int j = 0; j < 4; j++)
    // {
    //     for (i = j * number_of_fields; i < (j + 1) * number_of_fields; i++)
    //     {
    //         EPD_W21_WriteDATA(d1[j]);
    //     }
    // }

    // EPD_W21_WriteCMD(0x13); // write new data
    // for (int j = 0; j < 4; j++)
    // {
    //     for (i = j * number_of_fields; i < (j + 1) * number_of_fields; i++)
    //     {
    //         EPD_W21_WriteDATA(d2[j]);
    //     }
    // }

    // EPD_Update();
    // EPD_DeepSleep(); // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
    // delay(2000);     // Delay for 2s.

    /* Display image */

    EPD_Init_4G(); 

    EPD_W21_WriteCMD(0x10);
    for(int i = 0; i < EPD_ARRAY ; i++){
        EPD_W21_WriteDATA(bitmap1[i]);
    }

    EPD_W21_WriteCMD(0x13);
    for(int i = 0; i < EPD_ARRAY ; i++){
        EPD_W21_WriteDATA(bitmap2[i]);
    }

    EPD_Update();

    EPD_DeepSleep(); // Enter the sleep mode and please do not delete it, otherwise it will reduce the lifespan of the screen.
    
    while(1){
        // end
    }
}
