#include "board/board.hpp"
#include "esp_log.h"
#include "ui/app.hpp"
extern "C" void app_main() {
  if (!board::init()) {
    ESP_LOGE("gotchi", "Display unavailable");
    return;
  }
  board::audioInit();
  board::sensorInit();
  ui::start();
}
