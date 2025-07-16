#include "SPIFFS.h"

static const char * TAG ="MLX90640_IMAGE";
void ThermalImageToWeb(float mlx90640To[], const uint16_t camColors[], float MinTemp, float MaxTemp)
{
  // --- SAVE BMP FILE --- //
  uint8_t colorIndex = 0;
  uint16_t color = 0;
  unsigned int headers[13];
  int extrabytes;
  int paddedsize;
  int x = 0; 
  int y = 0; 
  int n = 0;
  int red = 0;
  int green = 0;
  int blue = 0;

  int WIDTH = 96;
  int HEIGHT = 72;

  extrabytes = 4 - ((WIDTH * 3) % 4);
  if (extrabytes == 4)
    extrabytes = 0;

  paddedsize = ((WIDTH * 3) + extrabytes) * HEIGHT;

  headers[0]  = paddedsize + 54;
  headers[1]  = 0;
  headers[2]  = 54;
  headers[3]  = 40;
  headers[4]  = WIDTH;
  headers[5]  = HEIGHT;
  headers[7]  = 0;
  headers[8]  = paddedsize;
  headers[9]  = 0;
  headers[10] = 0;
  headers[11] = 0;
  headers[12] = 0;

  File file =  SPIFFS.open("/thermal.bmp", "wb");
  if (!file) {
    ESP_LOGI(TAG,"There was an error opening the file for writing");
    return;
  }

  file.print("BM");

  for (n = 0; n <= 5; n++) {
    file.printf("%c", headers[n] & 0x000000FF);
    file.printf("%c", (headers[n] & 0x0000FF00) >> 8);
    file.printf("%c", (headers[n] & 0x00FF0000) >> 16);
    file.printf("%c", (headers[n] & (unsigned int) 0xFF000000) >> 24);
  }

  file.printf("%c", 1);
  file.printf("%c", 0);
  file.printf("%c", 24);
  file.printf("%c", 0);

  for (n = 7; n <= 12; n++) {
    file.printf("%c", headers[n] & 0x000000FF);
    file.printf("%c", (headers[n] & 0x0000FF00) >> 8);
    file.printf("%c", (headers[n] & 0x00FF0000) >> 16);
    file.printf("%c", (headers[n] & (unsigned int) 0xFF000000) >> 24);
  }

  for (y = HEIGHT - 1; y >= 0; y--) {
    for (x = 0; x < WIDTH; x++) {
      colorIndex = map(mlx90640To[x + (WIDTH * y)], MinTemp - 5.0, MaxTemp + 5.0, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);
      color = camColors[colorIndex];

      red = ((((color >> 11) & 0x1F) * 527) + 23) >> 6;
      green = ((((color >> 5) & 0x3F) * 259) + 33) >> 6;
      blue = (((color & 0x1F) * 527) + 23) >> 6;

      file.printf("%c", blue);
      file.printf("%c", green);
      file.printf("%c", red);
    }
    for (n = 1; n <= extrabytes; n++) {
      file.printf("%c", 0);
    }
  }

  file.close();
  ESP_LOGI(TAG, "Image saved to SPIFFS as /thermal.bmp");
}
