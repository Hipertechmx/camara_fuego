#ifndef __MLX_IMAGE__
#define __MLX_IMAGE__

#include <Arduino.h>

void ThermalImageToWeb(float mlx90640To[], const uint16_t camColors[], float MinTemp, float MaxTemp);

// DECLARACIÓN DE LA INTERPOLACIÓN
void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols,
                       float *dest, uint8_t dest_rows, uint8_t dest_cols);
 
#endif
