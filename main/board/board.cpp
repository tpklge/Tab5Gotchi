#include "board.hpp"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "freertos/semphr.h"
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <sys/time.h>
namespace board {
bool sdReady = false;
lv_display_t *display = nullptr;
static SemaphoreHandle_t io = nullptr;
void lockIO() {
  if (io)
    xSemaphoreTake(io, portMAX_DELAY);
}
void unlockIO() {
  if (io)
    xSemaphoreGive(io);
}
bool init() {
  gpio_hold_dis(BSP_LCD_BACKLIGHT);
  gpio_set_direction(BSP_LCD_BACKLIGHT, GPIO_MODE_OUTPUT);
  gpio_set_level(BSP_LCD_BACKLIGHT, 0);
  io = xSemaphoreCreateMutex();
  bsp_display_cfg_t cfg{};
  cfg.lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
  cfg.lvgl_port_cfg.task_stack = 24576;
  cfg.buffer_size = BSP_LCD_H_RES * 60;
  cfg.double_buffer = true;
  cfg.flags.buff_spiram = true;
  cfg.flags.sw_rotate = true;
  display = bsp_display_start_with_config(&cfg);
  if (!display)
    return false;
  bsp_display_lock(0);
  bsp_display_rotate(display, LV_DISPLAY_ROTATION_90);
  bsp_display_unlock();
  bsp_display_brightness_set(70);
  sdReady = bsp_sdcard_mount() == ESP_OK;
  if (sdReady) {
    mkdir("/sdcard/tamagotchi/saves", 0775);
    mkdir("/sdcard/tamagotchi/album", 0775);
  }
  return true;
}
static std::atomic<int> tilt{0};
int tiltOffset() { return tilt.load(); }
void sensorInit() {
  bsp_sensor_config_t c{};
  c.type = IMU_ID;
  c.mode = MODE_POLLING;
  c.period = 100;
  sensor_handle_t handle = nullptr;
  if (bsp_sensor_init(&c, &handle) != ESP_OK)
    return;
  iot_sensor_handler_register(
      handle,
      [](void *, sensor_event_base_t, int32_t event, void *payload) {
        if (event == SENSOR_ACCE_DATA_READY && payload) {
          auto *d = (sensor_data_t *)payload;
          tilt.store(std::clamp(int(d->acce.x * 12), -14, 14));
        }
      },
      nullptr);
  iot_sensor_start(handle);
}
static bool rtcRead() {
  i2c_master_dev_handle_t dev = nullptr;
  i2c_device_config_t c{};
  c.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  c.device_address = 0x32;
  c.scl_speed_hz = 100000;
  if (i2c_master_bus_add_device(bsp_i2c_get_handle(), &c, &dev) != ESP_OK)
    return false;
  uint8_t reg = 0x1d, flags = 2, b[7]{};
  bool ok =
      i2c_master_transmit_receive(dev, &reg, 1, &flags, 1, 100) == ESP_OK &&
      !(flags & 2);
  reg = 0x10;
  ok = ok && i2c_master_transmit_receive(dev, &reg, 1, b, 7, 100) == ESP_OK;
  i2c_master_bus_rm_device(dev);
  if (!ok)
    return false;
  auto bcd = [](int v) { return (v >> 4) * 10 + (v & 15); };
  tm t{};
  t.tm_sec = bcd(b[0] & 127);
  t.tm_min = bcd(b[1] & 127);
  t.tm_hour = bcd(b[2] & 63);
  t.tm_mday = bcd(b[4] & 63);
  t.tm_mon = bcd(b[5] & 31) - 1;
  t.tm_year = 100 + bcd(b[6]);
  if (t.tm_sec > 59 || t.tm_min > 59 || t.tm_hour > 23 || t.tm_mday < 1 ||
      t.tm_mday > 31 || t.tm_mon < 0 || t.tm_mon > 11)
    return false;
  setenv("TZ", "UTC0", 1);
  tzset();
  timeval tv{};
  tv.tv_sec = mktime(&t);
  settimeofday(&tv, nullptr);
  setenv("TZ", "AMT4", 1);
  tzset();
  return true;
}
int64_t wall() {
  static bool attempted = false;
  if (!attempted) {
    attempted = true;
    rtcRead();
  }
  return time(nullptr);
}
int batteryMv() { // INA226 bus-voltage register: 1.25mV per bit, 2S pack.
  i2c_master_dev_handle_t dev = nullptr;
  i2c_device_config_t cfg{};
  cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  cfg.device_address = 0x41;
  cfg.scl_speed_hz = 100000;
  if (i2c_master_bus_add_device(bsp_i2c_get_handle(), &cfg, &dev) != ESP_OK)
    return -1;
  uint8_t reg = 2, v[2]{};
  auto err = i2c_master_transmit_receive(dev, &reg, 1, v, 2, 30);
  i2c_master_bus_rm_device(dev);
  return err == ESP_OK ? int(((v[0] << 8) | v[1]) * 1.25) : -1;
}
void launcher() {
  const esp_partition_t *p = esp_partition_find_first(
      ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, nullptr);
  if (p && esp_ota_set_boot_partition(p) == ESP_OK)
    esp_restart();
}
bool screenshot(const char *path, lv_obj_t *target) {
  auto *snap = lv_snapshot_take(target ? target : lv_screen_active(),
                                LV_COLOR_FORMAT_RGB565);
  if (!snap)
    return false;
  uint32_t w = snap->header.w, h = snap->header.h,
           stride = ((w * 3 + 3) / 4) * 4, size = 54 + stride * h;
  uint8_t head[54]{};
  head[0] = 'B';
  head[1] = 'M';
  auto put = [&](int o, uint32_t v) {
    for (int i = 0; i < 4; i++)
      head[o + i] = v >> (i * 8);
  };
  put(2, size);
  put(10, 54);
  put(14, 40);
  put(18, w);
  put(22, h);
  head[26] = 1;
  head[28] = 24;
  lockIO();
  FILE *f = fopen(path, "wb");
  bool ok = f && fwrite(head, 1, 54, f) == 54;
  auto *row = (uint8_t *)calloc(stride, 1);
  if (!row)
    ok = false;
  for (int y = int(h) - 1; ok && y >= 0; y--) {
    auto *src = (const uint16_t *)(snap->data + y * snap->header.stride);
    for (uint32_t x = 0; x < w; x++) {
      auto c = src[x];
      row[x * 3] = (c & 31) * 255 / 31;
      row[x * 3 + 1] = ((c >> 5) & 63) * 255 / 63;
      row[x * 3 + 2] = (c >> 11) * 255 / 31;
    }
    ok = fwrite(row, 1, stride, f) == stride;
  }
  free(row);
  if (f)
    fclose(f);
  unlockIO();
  lv_draw_buf_destroy(snap);
  return ok;
}
} // namespace board
