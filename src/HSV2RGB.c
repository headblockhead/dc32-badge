#include <math.h>
#include <stdint.h>

void hsv2rgb(uint8_t *r, uint8_t *g, uint8_t *b, float H, float S, float V) {
  float h = H / 360;
  float s = S / 100;
  float v = V / 100;

  int i = floor(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);

  switch (i % 6) {
  case 0:
    *r = 255 * v, *g = 255 * t, *b = 255 * p;
    break;
  case 1:
    *r = 255 * q, *g = 255 * v, *b = 255 * p;
    break;
  case 2:
    *r = 255 * p, *g = 255 * v, *b = 255 * t;
    break;
  case 3:
    *r = 255 * p, *g = 255 * q, *b = 255 * v;
    break;
  case 4:
    *r = 255 * t, *g = 255 * p, *b = 255 * v;
    break;
  case 5:
    *r = 255 * v, *g = 255 * p, *b = 255 * q;
    break;
  }
}
