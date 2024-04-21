/*#include "gfx.h"*/
#include "hardware/gpio.h"
#include "hardware/pio.h"
/*#include "hardware/pwm.h"*/
/*#include "hardware/spi.h"*/
#include "pico/multicore.h"
/*#include "st7789.h"*/
#include "ws2812.pio.h"

void hsv2rgb(uint8_t *r, uint8_t *g, uint8_t *b, float H, float S, float V);

// neopixel helpers
#define NUM_PIXELS 9
#define WS2812_PIN 4

static inline void put_pixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// LED strip

PIO led_pio = pio0;

void led_init(void) {
  uint led_pio_offset = pio_add_program(led_pio, &ws2812_program);
  uint led_sm = pio_claim_unused_sm(led_pio, true);
  ws2812_program_init(led_pio, led_sm, led_pio_offset, WS2812_PIN, 800000,
                      false);
}

float t = 0;
uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;

void led_task(void) {
  // Update the LED strip with the new data.
  for (int i = 1; i < 10; i++) {
    // This makes a nice rainbow. Adjust values to taste :)
    hsv2rgb(&r, &g, &b, (100 * (t / 24)) + (i * 96) / 8, 100, 100);
    put_pixel(urgb_u32(r, g, b));
  }
  sleep_ms(25);
  t++;
}

// Core 1 deals with the display.
void core1_main() {
  while (true) {
    /*    GFX_clearScreen();*/
    /*GFX_fillScreen(0xF0F0);*/
    /*GFX_flush();*/
  }
}

// Core 0 deals with the LEDs
void core0_main() {
  while (true) {
    led_task();
  }
}

// The main function. Display and speaker code is temporarily disabled as I
// can't get it working, and it drains the battery.
int main(void) {
  led_init(); // Initialize the WS2812 LED strip

  //  DC CS RST SCK TX
  /*LCD_setPins(5, 9, -1, 6, 8);*/
  /*LCD_setSPIperiph(spi1);*/
  /*LCD_initDisplay(320, 240);*/
  /*GFX_createFramebuf();*/

  /*GFX_clearScreen();*/
  /*GFX_fillScreen(0xF0F0);*/
  /*GFX_flush();*/

  gpio_init(
      11); // Self power - board is powered while pin 11 is high. Pin 11 is held
           // high by the power button on the back too (but only while pressed).
  gpio_set_dir(11, GPIO_OUT);
  gpio_put(11, 1);

  /*  gpio_init(10); // LCD backlight*/
  /*gpio_set_dir(10, GPIO_OUT);*/
  /*gpio_put(10, 1);*/

  /*gpio_init(25); // Speaker*/
  /*gpio_set_function(25, GPIO_FUNC_PWM);*/
  /*uint slice_num = pwm_gpio_to_slice_num(25);*/

  /*// Set period of 4 cycles (0 to 3 inclusive)*/
  /*pwm_set_wrap(slice_num, 2);*/
  /*// Set channel A output high for one cycle before dropping*/
  /*pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);*/
  /*// Set the PWM running*/
  /*pwm_set_enabled(slice_num, true);*/

  // Core 1 loop
  multicore_launch_core1(core1_main);
  // Core 0 loop
  core0_main();
}
