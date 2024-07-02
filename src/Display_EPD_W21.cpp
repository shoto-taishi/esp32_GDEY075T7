#include "Display_EPD_W21_spi.h"
#include "Display_EPD_W21.h"

unsigned char PartImage[1000]; // Define Partial canvas space

// full screen update LUT 0~3 gray
const unsigned char lut_20_vcom0_4G[] =
    {
        0x00,
        0x0A,
        0x00,
        0x00,
        0x00,
        0x01,
        0x60,
        0x14,
        0x14,
        0x00,
        0x00,
        0x01,
        0x00,
        0x14,
        0x00,
        0x00,
        0x00,
        0x01,
        0x00,
        0x13,
        0x0A,
        0x01,
        0x00,
        0x01,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
};
// R21
const unsigned char lut_21_ww_4G[] =
    {
        0x40,
        0x0A,
        0x00,
        0x00,
        0x00,
        0x01,
        0x90,
        0x14,
        0x14,
        0x00,
        0x00,
        0x01,
        0x10,
        0x14,
        0x0A,
        0x00,
        0x00,
        0x01,
        0xA0,
        0x13,
        0x01,
        0x00,
        0x00,
        0x01,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
};
// R22H  r
const unsigned char lut_22_bw_4G[] =
    {
        0x40,
        0x0A,
        0x00,
        0x00,
        0x00,
        0x01,
        0x90,
        0x14,
        0x14,
        0x00,
        0x00,
        0x01,
        0x00,
        0x14,
        0x0A,
        0x00,
        0x00,
        0x01,
        0x99,
        0x0C,
        0x01,
        0x03,
        0x04,
        0x01,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
};
// R23H  w
const unsigned char lut_23_wb_4G[] =
    {
        0x40,
        0x0A,
        0x00,
        0x00,
        0x00,
        0x01,
        0x90,
        0x14,
        0x14,
        0x00,
        0x00,
        0x01,
        0x00,
        0x14,
        0x0A,
        0x00,
        0x00,
        0x01,
        0x99,
        0x0B,
        0x04,
        0x04,
        0x01,
        0x01,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
};
// R24H  b
const unsigned char lut_24_bb_4G[] =
    {
        0x80,
        0x0A,
        0x00,
        0x00,
        0x00,
        0x01,
        0x90,
        0x14,
        0x14,
        0x00,
        0x00,
        0x01,
        0x20,
        0x14,
        0x0A,
        0x00,
        0x00,
        0x01,
        0x50,
        0x13,
        0x01,
        0x00,
        0x00,
        0x01,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
};

// experimental partial screen update LUTs with balanced charge
// LUTs are filled with zeroes

#define T1 30 // charge balance pre-phase
#define T2 5  // optional extension
#define T3 30 // color change phase (b/w)
#define T4 5  // optional extension for one color

const unsigned char lut_20_LUTC_partial[] =
    {
        0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char lut_21_LUTWW_partial[] =
    {
        // 10 w
        0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char lut_22_LUTKW_partial[] =
    {
        // 10 w
        0x48, T1, T2, T3, T4, 1, // 01 00 10 00
                                 // 0x5A, T1, T2, T3, T4, 1, // 01 01 10 10 more white
};

const unsigned char lut_23_LUTWK_partial[] =
    {
        // 01 b
        0x84, T1, T2, T3, T4, 1, // 10 00 01 00
                                 // 0xA5, T1, T2, T3, T4, 1, // 10 10 01 01 more black
};

const unsigned char lut_24_LUTKK_partial[] =
    {
        // 01 b
        0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

const unsigned char lut_25_LUTBD_partial[] =
    {
        0x00, T1, T2, T3, T4, 1, // 00 00 00 00
};

////////////////////////////////////E-paper demo//////////////////////////////////////////////////////////
// Busy function
void lcd_chkstatus(void)
{
  while (1)
  { //=0 BUSY
    if (isEPD_W21_BUSY == 1)
      break;
  }
}
// Full screen refresh initialization
void EPD_Init(void)
{
  EPD_W21_RST_0; // Module reset
  delay(10);     // At least 10ms delay
  EPD_W21_RST_1;
  delay(10); // At least 10ms delay

  EPD_W21_WriteCMD(0x01); // POWER SETTING
  EPD_W21_WriteDATA(0x07);
  EPD_W21_WriteDATA(0x07); // VGH=20V,VGL=-20V
  EPD_W21_WriteDATA(0x3f); // VDH=15V
  EPD_W21_WriteDATA(0x3f); // VDL=-15V

  // Enhanced display drive(Add 0x06 command)
  EPD_W21_WriteCMD(0x06); // Booster Soft Start
  EPD_W21_WriteDATA(0x17);
  EPD_W21_WriteDATA(0x17);
  EPD_W21_WriteDATA(0x28);
  EPD_W21_WriteDATA(0x17);

  EPD_W21_WriteCMD(0x04); // POWER ON
  delay(100);
  lcd_chkstatus(); // waiting for the electronic paper IC to release the idle signal

  EPD_W21_WriteCMD(0X00);  // PANNEL SETTING
  EPD_W21_WriteDATA(0x1F); // KW-3f   KWR-2F BWROTP 0f BWOTP 1f

  EPD_W21_WriteCMD(0x61);  // tres
  EPD_W21_WriteDATA(0x03); // source 800
  EPD_W21_WriteDATA(0x20);
  EPD_W21_WriteDATA(0x01); // gate 480
  EPD_W21_WriteDATA(0xE0);

  EPD_W21_WriteCMD(0X15);
  EPD_W21_WriteDATA(0x00);

  EPD_W21_WriteCMD(0X50); // VCOM AND DATA INTERVAL SETTING
  EPD_W21_WriteDATA(0x10);
  EPD_W21_WriteDATA(0x07);

  EPD_W21_WriteCMD(0X60); // TCON SETTING
  EPD_W21_WriteDATA(0x22);
}

void EPD_Init_4G(void)
{
  EPD_Init();

  EPD_W21_WriteCMD(0x00);  // panel setting
  EPD_W21_WriteDATA(0x3f); // 4G update LUT from registers

  EPD_W21_WriteCMD(0x50);  // VCOM AND DATA INTERVAL SETTING
  EPD_W21_WriteDATA(0x31); // LUTBD
  EPD_W21_WriteDATA(0x07);

  EPD_W21_WriteCMD(0x20);
  for (int i = 0; i < sizeof(lut_20_vcom0_4G); i++)
  {
    EPD_W21_WriteDATA(lut_20_vcom0_4G[i]);
  }

  EPD_W21_WriteCMD(0x21);
  for (int i = 0; i < sizeof(lut_21_ww_4G); i++)
  {
    EPD_W21_WriteDATA(lut_21_ww_4G[i]);
  }

  EPD_W21_WriteCMD(0x22);
  for (int i = 0; i < sizeof(lut_22_bw_4G); i++)
  {
    EPD_W21_WriteDATA(lut_22_bw_4G[i]);
  }

  EPD_W21_WriteCMD(0x23);
  for (int i = 0; i < sizeof(lut_23_wb_4G); i++)
  {
    EPD_W21_WriteDATA(lut_23_wb_4G[i]);
  }

  EPD_W21_WriteCMD(0x24);
  for (int i = 0; i < sizeof(lut_24_bb_4G); i++)
  {
    EPD_W21_WriteDATA(lut_24_bb_4G[i]);
  }

  EPD_W21_WriteCMD(0x25);
  for (int i = 0; i < sizeof(lut_25_LUTBD_partial); i++)
  {
    EPD_W21_WriteDATA(lut_25_LUTBD_partial[i]);
  }
  for (int i = 0; i < 42 - sizeof(lut_25_LUTBD_partial); i++)
  {
    EPD_W21_WriteDATA(0x00);
  }
}

// Fast refresh 1 initialization
void EPD_Init_Fast(void)
{
  EPD_W21_RST_0; // Module reset
  delay(10);     // At least 10ms delay
  EPD_W21_RST_1;
  delay(10); // At least 10ms delay

  EPD_W21_WriteCMD(0X00);  // PANNEL SETTING
  EPD_W21_WriteDATA(0x1F); // KW-3f   KWR-2F BWROTP 0f BWOTP 1f

  EPD_W21_WriteCMD(0X50); // VCOM AND DATA INTERVAL SETTING
  EPD_W21_WriteDATA(0x10);
  EPD_W21_WriteDATA(0x07);

  EPD_W21_WriteCMD(0x04); // POWER ON
  delay(100);
  lcd_chkstatus(); // waiting for the electronic paper IC to release the idle signal

  // Enhanced display drive(Add 0x06 command)
  EPD_W21_WriteCMD(0x06); // Booster Soft Start
  EPD_W21_WriteDATA(0x27);
  EPD_W21_WriteDATA(0x27);
  EPD_W21_WriteDATA(0x18);
  EPD_W21_WriteDATA(0x17);

  EPD_W21_WriteCMD(0xE0);
  EPD_W21_WriteDATA(0x02);
  EPD_W21_WriteCMD(0xE5);
  EPD_W21_WriteDATA(0x5A);
}
// Partial refresh  initialization
void EPD_Init_Part(void)
{
  EPD_W21_RST_0; // Module reset
  delay(10);     // At least 10ms delay
  EPD_W21_RST_1;
  delay(10); // At least 10ms delay

  EPD_W21_WriteCMD(0X00);  // PANNEL SETTING
  EPD_W21_WriteDATA(0x3F); // KW-3f   KWR-2F BWROTP 0f BWOTP 1f

  EPD_W21_WriteCMD(0x04); // POWER ON
  delay(100);
  lcd_chkstatus(); // waiting for the electronic paper IC to release the idle signal

  EPD_W21_WriteCMD(0xE0);
  EPD_W21_WriteDATA(0x02);
  EPD_W21_WriteCMD(0xE5);
  EPD_W21_WriteDATA(0x6E);
}

//////////////////////////////Display Update Function///////////////////////////////////////////////////////
// Full screen refresh update function
void EPD_Update(void)
{
  // Refresh
  EPD_W21_WriteCMD(0x12); // DISPLAY REFRESH
  delay(1);               //!!!The delay here is necessary, 200uS at least!!!
  lcd_chkstatus();        // waiting for the electronic paper IC to release the idle signal
}

//////////////////////////////Display Data Transfer Function////////////////////////////////////////////
// Full screen refresh display function
void EPD_WhiteScreen_ALL(const unsigned char *datas)
{
  unsigned int i;
  EPD_W21_WriteCMD(0x10); // write old data
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0x00);
  }
  EPD_W21_WriteCMD(0x13); // write new data
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_Update();
}
// Fast refresh display function
void EPD_WhiteScreen_ALL_Fast(const unsigned char *datas)
{
  unsigned int i;
  EPD_W21_WriteCMD(0x10); // write old data
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0x00);
  }
  EPD_W21_WriteCMD(0x13); // write new data
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_Update();
}

// Clear screen display
void EPD_WhiteScreen_White(void)
{
  unsigned int i;
  // Write Data
  EPD_W21_WriteCMD(0x10);
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0x00);
  }
  EPD_W21_WriteCMD(0x13);
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0x00);
  }
  EPD_Update();
}
// Clear screen display
void EPD_WhiteScreen_White_Basemap(void)
{
  unsigned int i;
  // Write Data
  EPD_W21_WriteCMD(0x10);
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0xFF); // is  different
  }
  EPD_W21_WriteCMD(0x13);
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0x00);
  }
  EPD_Update();
}
// Display all black
void EPD_WhiteScreen_Black(void)
{
  unsigned int i;
  // Write Data
  EPD_W21_WriteCMD(0x10);
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0x00);
  }
  EPD_W21_WriteCMD(0x13);
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0xff);
  }
  EPD_Update();
}
// Partial refresh of background display, this function is necessary, please do not delete it!!!
void EPD_SetRAMValue_BaseMap(const unsigned char *datas)
{
  unsigned int i;
  EPD_W21_WriteCMD(0x10); // write old data
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(0xFF); // is  different
  }
  EPD_W21_WriteCMD(0x13); // write new data
  for (i = 0; i < EPD_ARRAY; i++)
  {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_Update();
}
// Partial refresh display
void EPD_Dis_Part(unsigned int x_start, unsigned int y_start, const unsigned char *datas, unsigned int PART_COLUMN, unsigned int PART_LINE)
{
  unsigned int i;
  unsigned int x_end, y_end;

  x_end = x_start + PART_LINE - 1;
  y_end = y_start + PART_COLUMN - 1;

  EPD_W21_WriteCMD(0x50);
  EPD_W21_WriteDATA(0xA9);
  EPD_W21_WriteDATA(0x07);

  EPD_W21_WriteCMD(0x91); // This command makes the display enter partial mode
  EPD_W21_WriteCMD(0x90); // resolution setting
  EPD_W21_WriteDATA(x_start / 256);
  EPD_W21_WriteDATA(x_start % 256); // x-start

  EPD_W21_WriteDATA(x_end / 256);
  EPD_W21_WriteDATA(x_end % 256 - 1); // x-end

  EPD_W21_WriteDATA(y_start / 256); //
  EPD_W21_WriteDATA(y_start % 256); // y-start

  EPD_W21_WriteDATA(y_end / 256);
  EPD_W21_WriteDATA(y_end % 256 - 1); // y-end
  EPD_W21_WriteDATA(0x01);

  EPD_W21_WriteCMD(0x13); // writes New data to SRAM.
  for (i = 0; i < PART_COLUMN * PART_LINE / 8; i++)
  {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_Update();
}
// Full screen partial refresh display
void EPD_Dis_PartAll(const unsigned char *datas)
{
  unsigned int i;
  unsigned int x_start = 0, y_start = 0, x_end, y_end;
  unsigned int PART_COLUMN = EPD_HEIGHT, PART_LINE = EPD_WIDTH;

  x_end = x_start + PART_LINE - 1;
  y_end = y_start + PART_COLUMN - 1;

  EPD_W21_WriteCMD(0x50);
  EPD_W21_WriteDATA(0xA9);
  EPD_W21_WriteDATA(0x07);

  EPD_W21_WriteCMD(0x91); // This command makes the display enter partial mode
  EPD_W21_WriteCMD(0x90); // resolution setting
  EPD_W21_WriteDATA(x_start / 256);
  EPD_W21_WriteDATA(x_start % 256); // x-start

  EPD_W21_WriteDATA(x_end / 256);
  EPD_W21_WriteDATA(x_end % 256 - 1); // x-end

  EPD_W21_WriteDATA(y_start / 256); //
  EPD_W21_WriteDATA(y_start % 256); // y-start

  EPD_W21_WriteDATA(y_end / 256);
  EPD_W21_WriteDATA(y_end % 256 - 1); // y-end
  EPD_W21_WriteDATA(0x01);

  EPD_W21_WriteCMD(0x13); // writes New data to SRAM.
  for (i = 0; i < PART_COLUMN * PART_LINE / 8; i++)
  {
    EPD_W21_WriteDATA(datas[i]);
  }
  EPD_Update();
}
// Deep sleep function
void EPD_DeepSleep(void)
{
  EPD_W21_WriteCMD(0X50);  // VCOM AND DATA INTERVAL SETTING
  EPD_W21_WriteDATA(0xf7); // WBmode:VBDF 17|D7 VBDW 97 VBDB 57    WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

  EPD_W21_WriteCMD(0X02); // power off
  lcd_chkstatus();        // waiting for the electronic paper IC to release the idle signal
  EPD_W21_WriteCMD(0X07); // deep sleep
  EPD_W21_WriteDATA(0xA5);
}

// Partial refresh write address and data
void EPD_Dis_Part_RAM(unsigned int x_start, unsigned int y_start,
                      const unsigned char *datas_A, const unsigned char *datas_B,
                      const unsigned char *datas_C, const unsigned char *datas_D, const unsigned char *datas_E,
                      unsigned char num, unsigned int PART_COLUMN, unsigned int PART_LINE)
{
  unsigned int i, j;
  unsigned int x_end, y_end;

  x_end = x_start + PART_LINE * num - 1;
  y_end = y_start + PART_COLUMN - 1;

  EPD_W21_WriteCMD(0x50);
  EPD_W21_WriteDATA(0xA9);
  EPD_W21_WriteDATA(0x07);

  EPD_W21_WriteCMD(0x91); // This command makes the display enter partial mode
  EPD_W21_WriteCMD(0x90); // resolution setting
  EPD_W21_WriteDATA(x_start / 256);
  EPD_W21_WriteDATA(x_start % 256); // x-start

  EPD_W21_WriteDATA(x_end / 256);
  EPD_W21_WriteDATA(x_end % 256 - 1); // x-end

  EPD_W21_WriteDATA(y_start / 256); //
  EPD_W21_WriteDATA(y_start % 256); // y-start

  EPD_W21_WriteDATA(y_end / 256);
  EPD_W21_WriteDATA(y_end % 256 - 1); // y-end
  EPD_W21_WriteDATA(0x01);

  EPD_W21_WriteCMD(0x13); // writes New data to SRAM.
  for (i = 0; i < PART_COLUMN; i++)
  {
    for (j = 0; j < PART_LINE / 8; j++)
      EPD_W21_WriteDATA(datas_A[i * PART_LINE / 8 + j]);
    for (j = 0; j < PART_LINE / 8; j++)
      EPD_W21_WriteDATA(datas_B[i * PART_LINE / 8 + j]);
    for (j = 0; j < PART_LINE / 8; j++)
      EPD_W21_WriteDATA(datas_C[i * PART_LINE / 8 + j]);
    for (j = 0; j < PART_LINE / 8; j++)
      EPD_W21_WriteDATA(datas_D[i * PART_LINE / 8 + j]);
    for (j = 0; j < PART_LINE / 8; j++)
      EPD_W21_WriteDATA(datas_E[i * PART_LINE / 8 + j]);
  }
}
// Clock display
void EPD_Dis_Part_Time(unsigned int x_start, unsigned int y_start,
                       const unsigned char *datas_A, const unsigned char *datas_B,
                       const unsigned char *datas_C, const unsigned char *datas_D, const unsigned char *datas_E,
                       unsigned char num, unsigned int PART_COLUMN, unsigned int PART_LINE)
{
  EPD_Dis_Part_RAM(x_start, y_start, datas_A, datas_B, datas_C, datas_D, datas_E, num, PART_COLUMN, PART_LINE);
  EPD_Update();
  EPD_W21_WriteCMD(0X92); // This command makes the display exit partial mode and enter normal mode.
}

////////////////////////////////Other newly added functions////////////////////////////////////////////
// Display rotation 180 degrees initialization
void EPD_Init_180(void)
{
  EPD_W21_RST_0; // Module reset
  delay(10);     // At least 10ms delay
  EPD_W21_RST_1;
  delay(10); // At least 10ms delay

  EPD_W21_WriteCMD(0x01); // POWER SETTING
  EPD_W21_WriteDATA(0x07);
  EPD_W21_WriteDATA(0x07); // VGH=20V,VGL=-20V
  EPD_W21_WriteDATA(0x3f); // VDH=15V
  EPD_W21_WriteDATA(0x3f); // VDL=-15V

  // Enhanced display drive(Add 0x06 command)
  EPD_W21_WriteCMD(0x06); // Booster Soft Start
  EPD_W21_WriteDATA(0x17);
  EPD_W21_WriteDATA(0x17);
  EPD_W21_WriteDATA(0x28);
  EPD_W21_WriteDATA(0x17);

  EPD_W21_WriteCMD(0x04); // POWER ON
  delay(100);
  lcd_chkstatus(); // waiting for the electronic paper IC to release the idle signal

  EPD_W21_WriteCMD(0X00);  // PANNEL SETTING
  EPD_W21_WriteDATA(0x13); // KW-3f   KWR-2F BWROTP 0f BWOTP 1f

  EPD_W21_WriteCMD(0x61);  // tres
  EPD_W21_WriteDATA(0x03); // source 800
  EPD_W21_WriteDATA(0x20);
  EPD_W21_WriteDATA(0x01); // gate 480
  EPD_W21_WriteDATA(0xE0);

  EPD_W21_WriteCMD(0X15);
  EPD_W21_WriteDATA(0x00);

  EPD_W21_WriteCMD(0X50); // VCOM AND DATA INTERVAL SETTING
  EPD_W21_WriteDATA(0x10);
  EPD_W21_WriteDATA(0x07);

  EPD_W21_WriteCMD(0X60); // TCON SETTING
  EPD_W21_WriteDATA(0x22);
}

/***********************************************************
            end file
***********************************************************/
