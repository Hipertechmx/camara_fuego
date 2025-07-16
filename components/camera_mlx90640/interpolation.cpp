#include <algorithm>
using std::min;

void interpolate_image(float *src, uint8_t src_rows, uint8_t src_cols,
                       float *dest, uint8_t dest_rows, uint8_t dest_cols) {
  for (int y = 0; y < dest_rows; y++) {
    float gy = ((float)(y) * (src_rows - 1)) / (dest_rows - 1);
    int gyi = (int)gy;
    float dy = gy - gyi;

    for (int x = 0; x < dest_cols; x++) {
      float gx = ((float)(x) * (src_cols - 1)) / (dest_cols - 1);
      int gxi = (int)gx;
      float dx = gx - gxi;

      float a = src[gyi * src_cols + gxi];
      float b = src[gyi * src_cols + min(gxi + 1, src_cols - 1)];
      float c = src[min(gyi + 1, src_rows - 1) * src_cols + gxi];
      float d = src[min(gyi + 1, src_rows - 1) * src_cols + min(gxi + 1, src_cols - 1)];

      dest[y * dest_cols + x] = a * (1 - dx) * (1 - dy) +
                                b * dx * (1 - dy) +
                                c * (1 - dx) * dy +
                                d * dx * dy;
    }
  }
}
}
