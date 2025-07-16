#include "camera_mlx90640.h"
#include "mlx90640_image.h"
#include "SPIFFS.h"

#define TA_SHIFT 8
#define COLS 32
#define ROWS 24
#define INTERP_COLS 196
#define INTERP_ROWS 144

static const char *TAG = "MLX90640";

uint8_t MLX90640_address = 0x33;
float mlx90640To[COLS * ROWS];
float pixels_2[INTERP_COLS * INTERP_ROWS];
paramsMLX90640 mlx90640;

extern const uint16_t camColors[];
extern void ThermalImageToWeb(float *image, const uint16_t *palette, float min_temp, float max_temp);

namespace esphome {
namespace mlx90640_app {

MLX90640::MLX90640(web_server_base::WebServerBase *base) : base_(base) {}

void MLX90640::setup() {
  ESP_LOGI(TAG, "SDA PIN %d ", this->sda_);
  ESP_LOGI(TAG, "SCL PIN %d ", this->scl_);
  ESP_LOGI(TAG, "I2C Frequency %d", this->frequency_);
  ESP_LOGI(TAG, "Address %d ", this->addr_);

  MLX90640_address = this->addr_;
  Wire.begin((int)this->sda_, (int)this->scl_, (uint32_t)this->frequency_);
  Wire.setClock(this->frequency_);
  MLX90640_I2CInit(&Wire);

  uint16_t eeMLX90640[832];
  if (!MLX90640_isConnected(MLX90640_address)) {
    ESP_LOGE(TAG, "Sensor not connected");
    return;
  }

  if (MLX90640_DumpEE(MLX90640_address, eeMLX90640) != 0 ||
      MLX90640_ExtractParameters(eeMLX90640, &mlx90640) != 0) {
    ESP_LOGE(TAG, "Failed to initialize MLX90640 params");
    return;
  }

  int rate = this->refresh_rate_ >= 0 ? this->refresh_rate_ : 0x05;
  MLX90640_SetRefreshRate(MLX90640_address, rate);

  if (!SPIFFS.begin(true)) {
    ESP_LOGE(TAG, "SPIFFS mount failed");
  }

  this->base_->get_server()->on("/thermal-camera", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/thermal.bmp", "image/bmp", false);
  });
}

void MLX90640::filter_outlier_pixel(float *pixels, int size, float level) {
  for (int i = 1; i < size - 1; i++) {
    if (abs(pixels[i] - pixels[i - 1]) >= level && abs(pixels[i] - pixels[i + 1]) >= level) {
      pixels[i] = (pixels[i - 1] + pixels[i + 1]) / 2.0f;
    }
  }
  if (abs(pixels[0] - pixels[1]) >= level && abs(pixels[0] - pixels[2]) >= level)
    pixels[0] = (pixels[1] + pixels[2]) / 2.0f;
  if (abs(pixels[size - 1] - pixels[size - 2]) >= level && abs(pixels[size - 1] - pixels[size - 3]) >= level)
    pixels[size - 1] = (pixels[size - 2] + pixels[size - 3]) / 2.0f;
}

void MLX90640::mlx_update() {
  uint16_t frame[834];
  if (MLX90640_GetFrameData(MLX90640_address, frame) < 0) return;

  MLX90640_CalculateTo(frame, &mlx90640, 0.95f, 23.15f, mlx90640To);
  interpolate_image(mlx90640To, ROWS, COLS, pixels_2, INTERP_ROWS, INTERP_COLS);

  if (this->filter_level_ > 0.0f)
    this->filter_outlier_pixel(pixels_2, INTERP_ROWS * INTERP_COLS, this->filter_level_);

  float min_temp = this->mintemp_;
  float max_temp = this->maxtemp_;

  if (this->min_temperature_sensor_)
    this->min_temperature_sensor_->publish_state(min_temp);
  if (this->max_temperature_sensor_)
    this->max_temperature_sensor_->publish_state(max_temp);

  float sum = 0.0f;
  for (int i = 0; i < INTERP_ROWS * INTERP_COLS; i++)
    sum += pixels_2[i];
  float mean = sum / (INTERP_ROWS * INTERP_COLS);

  if (this->mean_temperature_sensor_)
    this->mean_temperature_sensor_->publish_state(mean);

  ThermalImageToWeb(pixels_2, camColors, min_temp, max_temp);
}

void MLX90640::update() {
  this->mlx_update();
}

}  // namespace mlx90640_app
}  // namespace esphome
