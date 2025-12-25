#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "LGFX_ST7796S.h"
#include "screen_config_4.0_ST7796S.h"

// LVGL buffer (1/10 of screen size for partial rendering)
static lv_color_t buf1[LVGL_BUFFER_SIZE];

// LVGL tick source - use Arduino's millis()
static uint32_t my_tick(void)
{
    return millis();
}

// LVGL flush
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  display.pushImage(
      area->x1, area->y1,
      area->x2 - area->x1 + 1,
      area->y2 - area->y1 + 1,
      reinterpret_cast<lgfx::rgb565_t *>(px_map)
  );
  lv_disp_flush_ready(disp);
}

static int32_t display_width = SCREEN_WIDTH;
static int32_t display_height = SCREEN_HEIGHT;

void my_touch_read(lv_indev_t * indev, lv_indev_data_t * data)
{
  int32_t x, y;
  bool t = display.getTouch(&x, &y);
  
  if (t) {
    // Clamp to display bounds
    if (x < 0) x = 0;
    if (x >= display_width) x = display_width - 1;
    if (y < 0) y = 0;
    if (y >= display_height) y = display_height - 1;
    
    data->point.x = x;
    data->point.y = y;
  }
  
  data->state = t ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// Button grid - 5 columns x 10 rows = 50 buttons
#define BUTTON_COLS 5
#define BUTTON_ROWS 10
#define BUTTON_COUNT (BUTTON_COLS * BUTTON_ROWS)
#define VIEW_CAL_BUTTON_INDEX 48  // Button 49 (index 48) is the view calibration values button
#define CAL_BUTTON_INDEX 49       // Button 50 (index 49) is the calibration button

static lv_obj_t *buttons[BUTTON_COUNT] = {nullptr};
static int button_press_count[BUTTON_COUNT] = {0};
static bool calibration_done = false;
static uint16_t cal_values[8] = {0};
static lv_obj_t *cal_label = nullptr;
static const int CALIBRATION_RUNS = 4;
static const uint16_t CAL_LOW_THRESHOLD = 2000;

static void pick_best_calibration(const uint16_t samples[CALIBRATION_RUNS][8], uint16_t out[8])
{
  for (int i = 0; i < 8; i++) {
    uint16_t min_v = samples[0][i];
    uint16_t max_v = samples[0][i];
    for (int r = 1; r < CALIBRATION_RUNS; r++) {
      if (samples[r][i] < min_v) min_v = samples[r][i];
      if (samples[r][i] > max_v) max_v = samples[r][i];
    }
    // Low values (near 0) -> take lowest, high values (near max) -> take highest.
    out[i] = (max_v < CAL_LOW_THRESHOLD) ? min_v : max_v;
  }
}

static void print_calibration_values(const uint16_t values[8])
{
  Serial.print("Touch calibration values: {");
  for (int i = 0; i < 8; i++) {
    Serial.print(values[i]);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("}");
}

// Toggle calibration values display on screen
void toggle_calibration_values()
{
  // If label exists and is visible, hide it
  if (cal_label && lv_obj_has_flag(cal_label, LV_OBJ_FLAG_HIDDEN) == false) {
    lv_obj_add_flag(cal_label, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  
  // If label doesn't exist or is hidden, show/create it
  if (!cal_label) {
    cal_label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(cal_label, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_set_style_bg_color(cal_label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(cal_label, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(cal_label, 5, LV_PART_MAIN);
    lv_obj_align(cal_label, LV_ALIGN_TOP_RIGHT, -5, 5);
  } else {
    // Label exists but is hidden, just show it
    lv_obj_clear_flag(cal_label, LV_OBJ_FLAG_HIDDEN);
  }
  
  // Update the text
  char cal_text[200];
  if (calibration_done) {
    snprintf(cal_text, sizeof(cal_text), 
      "Cal Values:\n%d, %d, %d, %d,\n%d, %d, %d, %d, \n Set these in the screen_config_{display_name}.h",
      cal_values[0], cal_values[1], cal_values[2], cal_values[3],
      cal_values[4], cal_values[5], cal_values[6], cal_values[7]);
  } else {
    snprintf(cal_text, sizeof(cal_text), 
      "Not Calibrated\nRun CAL first");
  }
  
  lv_label_set_text(cal_label, cal_text);
}

bool check_skip_calibration()
{
  if (!display.touch()) return false;
  
  // Apply existing calibration values if available for touch detection
  bool has_valid_cal = false;
  for (int i = 0; i < 8; i++) {
    if (TOUCH_CAL_VALUES[i] != 0) {
      has_valid_cal = true;
      break;
    }
  }
  if (has_valid_cal) {
    display.setTouchCalibrate((uint16_t*)TOUCH_CAL_VALUES);
  }
  
  // Draw skip screen
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.println("Touch Calibration");
  display.println("");
  display.println("Touch anywhere to SKIP");
  display.println("(use existing cal)");
  display.println("");
  display.println("Or wait 5 seconds");
  display.println("to calibrate");
  
  int btn_width = 200;
  int btn_height = 80;
  int btn_x = (display_width - btn_width) / 2;
  int btn_y = (display_height - btn_height) / 2;
  
  display.fillRect(btn_x, btn_y, btn_width, btn_height, TFT_GREEN);
  display.drawRect(btn_x, btn_y, btn_width, btn_height, TFT_WHITE);
  display.setTextColor(TFT_BLACK, TFT_GREEN);
  display.setTextSize(2);
  display.setCursor(btn_x + (btn_width - 8 * 12) / 2, btn_y + (btn_height - 16) / 2);
  display.println("TOUCH SKIP");
  
  unsigned long start_time = millis();
  while (millis() - start_time < 5000) {
    int32_t x, y;
    if (display.getTouch(&x, &y)) {
      display.fillRect(btn_x, btn_y, btn_width, btn_height, TFT_YELLOW);
      delay(200);
      return true;
    }
    delay(50);
  }
  
  return false;
}

void run_calibration(bool lvgl_initialized = false)
{
  if (!display.touch()) return;
  
  if (!lvgl_initialized && check_skip_calibration()) {
    memcpy(cal_values, TOUCH_CAL_VALUES, sizeof(cal_values));
    display.setTouchCalibrate(cal_values);
    calibration_done = true;
    return;
  }
  
  uint16_t cal_samples[CALIBRATION_RUNS][8] = {0};
  for (int run = 0; run < CALIBRATION_RUNS; run++) {
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.println("Touch Calibration");
    display.println("Touch the corners");
    display.print("Run ");
    display.print(run + 1);
    display.print("/");
    display.println(CALIBRATION_RUNS);
    display.calibrateTouch(cal_samples[run], TFT_WHITE, TFT_BLACK, 10);
  }
  pick_best_calibration(cal_samples, cal_values);
  display.setTouchCalibrate(cal_values);
  calibration_done = true;
  print_calibration_values(cal_values);
  
  if (lvgl_initialized) {
    lv_obj_invalidate(lv_screen_active());
    for (int i = 0; i < 10; i++) {
      lv_timer_handler();
      delay(20);
    }
    lv_refr_now(lv_display_get_default());
    for (int i = 0; i < 5; i++) {
      lv_timer_handler();
      delay(20);
    }
    lv_refr_now(lv_display_get_default());
  }
}

void button_event_handler(lv_event_t * e)
{
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  
  lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
  int btn_index = -1;
  
  for (int i = 0; i < BUTTON_COUNT; i++) {
    if (buttons[i] == btn) {
      btn_index = i;
      break;
    }
  }
  
  if (btn_index < 0) return;
  
  if (btn_index == VIEW_CAL_BUTTON_INDEX) {
    toggle_calibration_values();
    return;
  }
  
  if (btn_index == CAL_BUTTON_INDEX) {
    run_calibration(true);
    return;
  }
  
  button_press_count[btn_index]++;
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  if (label) {
    char text[16];
    snprintf(text, sizeof(text), "%d", button_press_count[btn_index]);
    lv_label_set_text((lv_obj_t*)label, text);
  }
  
  static bool is_blue = false;
  lv_obj_set_style_bg_color(btn, lv_color_hex(is_blue ? 0x2196F3 : 0xFF9800), LV_PART_MAIN);
  is_blue = !is_blue;
}

void setup()
{
  Serial.begin(115200);
  // Ensure touch CS is high (inactive) before display init to prevent interference
  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(TOUCH_CS, HIGH);
  delay(10);
  
  display.init();
  display.setBrightness(255);

  // Set display rotation
  display.setRotation(SCREEN_DEFAULT_ROTATION);

  display.fillScreen(TFT_RED);   delay(150);
  display.fillScreen(TFT_GREEN); delay(150);
  display.fillScreen(TFT_BLUE);  delay(150);
  display.fillScreen(TFT_BLACK);

  display_width = display.width();
  display_height = display.height();
  
  if (display.touch()) {
    bool has_valid_cal = false;
    for (int i = 0; i < 8; i++) {
      if (TOUCH_CAL_VALUES[i] != 0) {
        has_valid_cal = true;
        break;
      }
    }
    
    if (has_valid_cal) {
      run_calibration(false);
    } else {
      run_calibration(false);
    }
  }

  lv_init();

  // Set tick source for LVGL timing
  lv_tick_set_cb(my_tick);

  // Create display with actual display dimensions
  lv_display_t *disp = lv_display_create(display_width, display_height);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, buf1, nullptr, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t *touch = lv_indev_create();
  lv_indev_set_type(touch, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(touch, my_touch_read);

  const int btn_width = (display_width - 20) / BUTTON_COLS - 4;
  const int btn_height = (display_height - 20) / BUTTON_ROWS - 4;
  const int spacing_x = (display_width - 20 - (BUTTON_COLS * btn_width)) / (BUTTON_COLS - 1);
  const int spacing_y = (display_height - 20 - (BUTTON_ROWS * btn_height)) / (BUTTON_ROWS - 1);
  
  for (int row = 0; row < BUTTON_ROWS; row++) {
    for (int col = 0; col < BUTTON_COLS; col++) {
      int index = row * BUTTON_COLS + col;
      int x = 10 + col * (btn_width + spacing_x);
      int y = 10 + row * (btn_height + spacing_y);
      
      lv_obj_t *btn = lv_button_create(lv_screen_active());
      lv_obj_set_size(btn, btn_width, btn_height);
      lv_obj_set_pos(btn, x, y);
      
      lv_obj_t *label = lv_label_create(btn);
      char text[8];
      if (index == VIEW_CAL_BUTTON_INDEX) {
        snprintf(text, sizeof(text), "VIEW");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x00FF00), LV_PART_MAIN);
      } else if (index == CAL_BUTTON_INDEX) {
        snprintf(text, sizeof(text), "CAL");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF0000), LV_PART_MAIN);
      } else {
        snprintf(text, sizeof(text), "%d", index + 1);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2196F3), LV_PART_MAIN);
      }
      lv_label_set_text(label, text);
      lv_obj_center(label);
      buttons[index] = btn;
      lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_CLICKED, nullptr);
    }
  }
}

void loop()
{
  lv_timer_handler();
  delay(5);
}
