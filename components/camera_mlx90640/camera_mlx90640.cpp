#include "camera_mlx90640.h"
#include "mlx90640_image.h"
#include "SPIFFS.h"
#include <math.h>

uint8_t MLX90640_address = 0x33;
#define TA_SHIFT 8

#define COLS   32
#define ROWS   24
#define COLS_2 (COLS * 2)
#define ROWS_2 (ROWS * 2)
#define INTERPOLATED_COLS 96
#define INTERPOLATED_ROWS 72

float pixelsArraySize = COLS * ROWS;
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
    0x480F, 0x400F, 0x400F, 0x400F, 0x4010, 0x3810, 0x3810, 0x3810, 0x3810,
    0x3010, 0x3010, 0x3010, 0x2810, 0x2810, 0x2810, 0x2810, 0x2010, 0x2010,
    0x2010, 0x1810, 0x1810, 0x1811, 0x1811, 0x1011, 0x1011, 0x1011, 0x0811,
    0x0811, 0x0811, 0x0011, 0x0011, 0x0011, 0x0011, 0x0011, 0x0031, 0x0031,
    0x0051, 0x0072, 0x0072, 0x0092, 0x00B2, 0x00B2, 0x00D2, 0x00F2, 0x00F2,
    0x0112, 0x0132, 0x0152, 0x0152, 0x0172, 0x0192, 0x0192, 0x01B2, 0x01D2,
    0x01F3, 0x01F3, 0x0213, 0x0233, 0x0253, 0x0253, 0x0273, 0x0293, 0x02B3,
    0x02D3, 0x02D3, 0x02F3, 0x0313, 0x0333, 0x0333, 0x0353, 0x0373, 0x0394,
    0x03B4, 0x03D4, 0x03D4, 0x03F4, 0x0414, 0x0434, 0x0454, 0x0474, 0x0474,
    0x0494, 0x04B4, 0x04D4, 0x04F4, 0x0514, 0x0534, 0x0534, 0x0554, 0x0554,
    0x0574, 0x0574, 0x0573, 0x0573, 0x0573, 0x0572, 0x0572, 0x0572, 0x0571,
    0x0591, 0x0591, 0x0590, 0x0590, 0x058F, 0x058F, 0x058F, 0x058E, 0x05AE,
    0x05AE, 0x05AD, 0x05AD, 0x05AD, 0x05AC, 0x05AC, 0x05AB, 0x05CB, 0x05CB,
    0x05CA, 0x05CA, 0x05CA, 0x05C9, 0x05C9, 0x05C8, 0x05E8, 0x05E8, 0x05E7,
    0x05E7, 0x05E6, 0x05E6, 0x05E6, 0x05E5, 0x05E5, 0x0604, 0x0604, 0x0604,
    0x0603, 0x0603, 0x0602, 0x0602, 0x0601, 0x0621, 0x0621, 0x0620, 0x0620,
    0x0620, 0x0620, 0x0E20, 0x0E20, 0x0E40, 0x1640, 0x1640, 0x1E40, 0x1E40,
    0x2640, 0x2640, 0x2E40, 0x2E60, 0x3660, 0x3660, 0x3E60, 0x3E60, 0x3E60,
    0x4660, 0x4660, 0x4E60, 0x4E80, 0x5680, 0x5680, 0x5E80, 0x5E80, 0x6680,
    0x6680, 0x6E80, 0x6EA0, 0x76A0, 0x76A0, 0x7EA0, 0x7EA0, 0x86A0, 0x86A0,
    0x8EA0, 0x8EC0, 0x96C0, 0x96C0, 0x9EC0, 0x9EC0, 0xA6C0, 0xAEC0, 0xAEC0,
    0xB6E0, 0xB6E0, 0xBEE0, 0xBEE0, 0xC6E0, 0xC6E0, 0xCEE0, 0xCEE0, 0xD6E0,
    0xD700, 0xDF00, 0xDEE0, 0xDEC0, 0xDEA0, 0xDE80, 0xDE80, 0xE660, 0xE640,
    0xE620, 0xE600, 0xE5E0, 0xE5C0, 0xE5A0, 0xE580, 0xE560, 0xE540, 0xE520,
    0xE500, 0xE4E0, 0xE4C0, 0xE4A0, 0xE480, 0xE460, 0xEC40, 0xEC20, 0xEC00,
    0xEBE0, 0xEBC0, 0xEBA0, 0xEB80, 0xEB60, 0xEB40, 0xEB20, 0xEB00, 0xEAE0,
    0xEAC0, 0xEAA0, 0xEA80, 0xEA60, 0xEA40, 0xF220, 0xF200, 0xF1E0, 0xF1C0,
    0xF1A0, 0xF180, 0xF160, 0xF140, 0xF100, 0xF0E0, 0xF0C0, 0xF0A0, 0xF080,
    0xF060, 0xF040, 0xF020, 0xF800,
};

std::string payload ;

std::string payload ;
long loopTime, startTime, endTime, fps;

// Forward declarations
float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y);
void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y, float f);

// === FUNCIONES DE FILTROS ADICIONALES ===
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
      float center = get_point(pixels, rows, cols, x, y);
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
// === FIN FUNCIONES DE FILTROS ===

namespace esphome{
namespace mlx90640_app{

void MLX90640::mlx_update() {
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

  filter_outlier_pixel(pixels, sizeof(pixels) / sizeof(pixels[0]), this->filter_level_);

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

  // Aplicar filtros térmicos adicionales
  apply_threshold(pixels_2, INTERPOLATED_COLS, INTERPOLATED_ROWS, 40.0);
  apply_edge_detection(pixels_2, INTERPOLATED_COLS, INTERPOLATED_ROWS);
  mark_hotspot(pixels_2, INTERPOLATED_COLS, INTERPOLATED_ROWS);

  ThermalImageToWeb(pixels_2, camColors, min_v, max_v);

  if (max_v > max_cam_v || max_v < min_cam_v) {
    ESP_LOGE(TAG, "MLX READING VALUE ERRORS");
    dataValid = false;
  } else {
    ESP_LOGI(TAG, "Min temperature : %.2f C ", min_v);
    ESP_LOGI(TAG, "Max temperature : %.2f C ", max_v);
    ESP_LOGI(TAG, "Mean temperature : %.2f C ", meanTemp);
    ESP_LOGI(TAG, "Median temperature : %.2f C ", medianTemp);
    dataValid = true;
  }

  loopTime = millis();
  endTime  = loopTime;
  fps      = 1000 / (endTime - startTime);
}

} // namespace mlx90640_app
} // namespace esphome

}
