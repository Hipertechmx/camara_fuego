#include "camera_mlx90640.h"
#include "mlx90640_image.h"
#include "SPIFFS.h"
#include <math.h>

uint8_t MLX90640_address = 0x33;
#define TA_SHIFT 8

#define COLS   32
#define ROWS   24
#define INTERPOLATED_COLS 96
#define INTERPOLATED_ROWS 72

float pixels[COLS * ROWS];
float pixels_2[INTERPOLATED_COLS * INTERPOLATED_ROWS];
float reversePixels[COLS * ROWS];
uint16_t pixels_colored [ROWS][COLS] ;
byte speed_setting = 2;
bool reverseScreen = false;

static const char * TAG = "MLX90640" ;
static float mlx90640To[COLS * ROWS];
paramsMLX90640 mlx90640;
bool dataValid = false ;
float medianTemp ;
float meanTemp ;

int MINTEMP   = 24;
float min_v     = 24;
int min_cam_v = -40;
int MAXTEMP      = 35;
float max_v        = 35;
int max_cam_v    = 300;
int resetMaxTemp = 45;

const uint16_t camColors[] = {
    // Tabla de colores simulada
    0x0000, 0x001F, 0x07E0, 0xF800, 0xFFFF
};

std::string payload ;
long loopTime, startTime, endTime, fps;

// === FUNCIONES AUXILIARES ===
float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
  if (x < 0 || x >= cols || y < 0 || y >= rows) return 0;
  return p[y * cols + x];
}

void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f) {
  if (x < 0 || x >= cols || y < 0 || y >= rows) return;
  p[y * cols + x] = f;
}

// === FILTROS ===
void apply_threshold(float *pixels, int cols, int rows, float threshold) {
  for (int i = 0; i < cols * rows; i++) {
    if (pixels[i] > threshold) {
      pixels[i] = max_v;
    }
  }
}

void apply_edge_detection(float *pixels, int cols, int rows) {
  for (int y = 1; y < rows - 1; y++) {
    for (int x = 1; x < cols - 1; x++) {
      float gx = get_point(pixels, rows, cols, x+1, y) - get_point(pixels, rows, cols, x-1, y);
      float gy = get_point(pixels, rows, cols, x, y+1) - get_point(pixels, rows, cols, x, y-1);
      float gradient = sqrt(gx * gx + gy * gy);
      if (gradient > 1.5) {
        set_point(pixels, rows, cols, x, y, max_v);
      }
    }
  }
}

void mark_hotspot(float *pixels, int cols, int rows) {
  int max_idx = 0;
  float max_temp = pixels[0];
  for (int i = 1; i < cols * rows; i++) {
    if (pixels[i] > max_temp) {
      max_temp = pixels[i];
      max_idx = i;
    }
  }
  int x_hot = max_idx % cols;
  int y_hot = max_idx / cols;
  for (int dx = -2; dx <= 2; dx++) {
    set_point(pixels, rows, cols, x_hot + dx, y_hot, max_v);
    set_point(pixels, rows, cols, x_hot, y_hot + dx, max_v);
  }
}

// === ACTUALIZACIÓN ===
void mlx_update() {
  for (byte x = 0; x < speed_setting; x++) {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    if (status < 0) {
      ESP_LOGE(TAG,"GetFrame Error: %d",status);
    }

    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta  = MLX90640_GetTa(mlx90640Frame, &mlx90640);
    float tr = Ta - TA_SHIFT;
    float emissivity = 0.95;
    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, pixels);
    int mode_ = MLX90640_GetCurMode(MLX90640_address);
    MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, pixels, mode_, &mlx90640);
  }

  medianTemp = (pixels[165]+pixels[180]+pixels[176]+pixels[192]) / 4.0;
  max_v = MINTEMP;
  min_v = MAXTEMP;
  float total = 0;
  for (int i = 0; i < COLS * ROWS; i++) {
    if (pixels[i] > max_v) max_v = pixels[i];
    if (pixels[i] < min_v) min_v = pixels[i];
    total += pixels[i];
  }
  meanTemp = total / (COLS * ROWS);

  interpolate_image(pixels, ROWS, COLS, pixels_2, INTERPOLATED_ROWS, INTERPOLATED_COLS);

  apply_threshold(pixels_2, INTERPOLATED_COLS, INTERPOLATED_ROWS, 40.0);
  apply_edge_detection(pixels_2, INTERPOLATED_COLS, INTERPOLATED_ROWS);
  mark_hotspot(pixels_2, INTERPOLATED_COLS, INTERPOLATED_ROWS);

  ThermalImageToWeb(pixels_2, camColors, min_v, max_v);
}
