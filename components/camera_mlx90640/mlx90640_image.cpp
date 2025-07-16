#include "SPIFFS.h"

static const char *TAG = "MLX90640_IMAGE";

void ThermalImageToWeb(float mlx90640To[], const uint16_t camColors[], float MinTemp, float MaxTemp) {
  const int WIDTH = 196;
  const int HEIGHT = 144;
  const int extrabytes = (4 - ((WIDTH * 3) % 4)) % 4;
  const int paddedsize = ((WIDTH * 3) + extrabytes) * HEIGHT;

  unsigned int headers[13];
  File file = SPIFFS.open("/thermal.bmp", "wb");

  if (!file) {
    ESP_LOGE(TAG, "Failed to open thermal.bmp for writing");
    return;
  }

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

  file.print("BM");

  for (int i = 0; i <= 5; i++) {
    file.write((uint8_t *)&headers[i], 4);
  }

  file.write((uint8_t)1);
  file.write((uint8_t)0);
  file.write((uint8_t)24);
  file.write((uint8_t)0);

  for (int i = 7; i <= 12; i++) {
    file.write((uint8_t *)&headers[i], 4);
  }

  for (int y = HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = x + y * WIDTH;
      int colorIndex = map(mlx90640To[idx], MinTemp - 5.0f, MaxTemp + 5.0f, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);
      uint16_t color = camColors[colorIndex];

      uint8_t r = (((color >> 11) & 0x1F) * 527 + 23) >> 6;
      uint8_t g = (((color >> 5) & 0x3F) * 259 + 33) >> 6;
      uint8_t b = ((color & 0x1F) * 527 + 23) >> 6;

      file.write((uint8_t)b);
      file.write((uint8_t)g);
      file.write((uint8_t)r);
    }
    for (int i = 0; i < extrabytes; i++) {
      file.write((uint8_t)0);
    }
  }

  file.close();
  ESP_LOGI(TAG, "thermal.bmp written: %d x %d", WIDTH, HEIGHT);
}
#include "SPIFFS.h"

static const char *TAG = "MLX90640_IMAGE";

void ThermalImageToWeb(float mlx90640To[], const uint16_t camColors[], float MinTemp, float MaxTemp) {
  const int WIDTH = 196;
  const int HEIGHT = 144;
  const int extrabytes = (4 - ((WIDTH * 3) % 4)) % 4;
  const int paddedsize = ((WIDTH * 3) + extrabytes) * HEIGHT;

  unsigned int headers[13];
  File file = SPIFFS.open("/thermal.bmp", "wb");

  if (!file) {
    ESP_LOGE(TAG, "Failed to open thermal.bmp for writing");
    return;
  }

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

  file.print("BM");

  for (int i = 0; i <= 5; i++) {
    file.write((uint8_t *)&headers[i], 4);
  }

  file.write((uint8_t)1);
  file.write((uint8_t)0);
  file.write((uint8_t)24);
  file.write((uint8_t)0);

  for (int i = 7; i <= 12; i++) {
    file.write((uint8_t *)&headers[i], 4);
  }

  for (int y = HEIGHT - 1; y >= 0; y--) {
    for (int x = 0; x < WIDTH; x++) {
      int idx = x + y * WIDTH;
      int colorIndex = map(mlx90640To[idx], MinTemp - 5.0f, MaxTemp + 5.0f, 0, 255);
      colorIndex = constrain(colorIndex, 0, 255);
      uint16_t color = camColors[colorIndex];

      uint8_t r = (((color >> 11) & 0x1F) * 527 + 23) >> 6;
      uint8_t g = (((color >> 5) & 0x3F) * 259 + 33) >> 6;
      uint8_t b = ((color & 0x1F) * 527 + 23) >> 6;

      file.write((uint8_t)b);
      file.write((uint8_t)g);
      file.write((uint8_t)r);
    }
    for (int i = 0; i < extrabytes; i++) {
      file.write((uint8_t)0);
    }
  }

  file.close();
  ESP_LOGI(TAG, "thermal.bmp written: %d x %d", WIDTH, HEIGHT);
}
