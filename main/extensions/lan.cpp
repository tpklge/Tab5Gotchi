#include "lan.hpp"
#include "bsp/m5stack_tab5.h"
#include "esp_event.h"
#include "esp_hosted.h"
#include "esp_hosted_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <unistd.h>
namespace lan {
static constexpr int PORT = 25005;
struct Tx {
  char ip[16];
  visit::Packet packet;
};
static SemaphoreHandle_t guard = nullptr, transport = nullptr;
static QueueHandle_t queue = nullptr;
static TaskHandle_t worker = nullptr;
static std::atomic<bool> connected{false}, stop{false};
static char ssid[33]{}, password[65]{};
static uint32_t ownCode = 0;
static std::string info = "Desconectado";
static visit::Packet incoming{};
static bool waiting = false;
static void text(const char *s) {
  xSemaphoreTake(guard, portMAX_DELAY);
  info = s;
  xSemaphoreGive(guard);
}
std::string status() {
  if (!guard)
    return "Desconectado";
  xSemaphoreTake(guard, portMAX_DELAY);
  auto s = info;
  xSemaphoreGive(guard);
  return s;
}
bool online() { return connected.load(); }
void disconnect() { stop = true; }
static std::atomic<bool> active{false}, hostReady{false};
static std::atomic<uint32_t> localIP{0}, localMask{0};
static bool ensureNetwork() {
  static bool netif = false, wifi = false;
  if (!netif) {
    auto netifResult = esp_netif_init();
    if (netifResult != ESP_OK && netifResult != ESP_ERR_INVALID_STATE)
      return false;
    esp_event_loop_create_default();
    if (!esp_netif_create_default_wifi_sta())
      return false;
    esp_event_handler_register(
        ESP_HOSTED_EVENT, ESP_HOSTED_EVENT_TRANSPORT_UP,
        [](void *, esp_event_base_t, int32_t, void *) {
          hostReady = true;
          xSemaphoreGive(transport);
        },
        nullptr);
    esp_event_handler_register(
        ESP_HOSTED_EVENT, ESP_HOSTED_EVENT_TRANSPORT_DOWN,
        [](void *, esp_event_base_t, int32_t, void *) {
          hostReady = false;
          connected = false;
          text("Conexão com C6 perdida");
        },
        nullptr);
    netif = true;
  }
  if (!hostReady) {
    if (bsp_feature_enable(BSP_FEATURE_WIFI, true) != ESP_OK)
      return false;
    vTaskDelay(pdMS_TO_TICKS(500));
    text("Conectando ao C6...");
    xSemaphoreTake(transport, 0);
    if (esp_hosted_connect_to_slave() != ESP_OK ||
        xSemaphoreTake(transport, pdMS_TO_TICKS(15000)) != pdTRUE) {
      text("C6 sem resposta; jogo offline preservado");
      return false;
    }
  }
  if (!wifi) {
    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    init.nvs_enable = false;
    if (esp_wifi_init(&init) != ESP_OK) {
      text("Falha na inicialização do Wi-Fi");
      return false;
    }
    esp_event_handler_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        [](void *, esp_event_base_t, int32_t, void *payload) {
          auto *e = (ip_event_got_ip_t *)payload;
          localIP = e->ip_info.ip.addr;
          localMask = e->ip_info.netmask.addr;
          char b[64];
          snprintf(b, sizeof b, "Conectado: " IPSTR, IP2STR(&e->ip_info.ip));
          connected = true;
          text(b);
        },
        nullptr);
    esp_event_handler_register(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
        [](void *, esp_event_base_t, int32_t, void *) {
          connected = false;
          text("Wi-Fi desconectado");
        },
        nullptr);
    wifi = true;
  }
  return true;
}
static void task(void *) {
  if (!ensureNetwork())
    goto end;
  {
    wifi_config_t cfg{};
    memcpy(cfg.sta.ssid, ssid, strlen(ssid));
    memcpy(cfg.sta.password, password, strlen(password));
    cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
    cfg.sta.pmf_cfg.capable = true;
    if (esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK ||
        esp_wifi_set_config(WIFI_IF_STA, &cfg) != ESP_OK ||
        esp_wifi_start() != ESP_OK || esp_wifi_connect() != ESP_OK) {
      text("Não foi possível conectar");
      goto end;
    }
    memset(password, 0, sizeof password);
    int64_t deadline = esp_timer_get_time() + 25000000;
    while (!connected && !stop && esp_timer_get_time() < deadline)
      vTaskDelay(pdMS_TO_TICKS(100));
    if (!connected) {
      text("Tempo de conexão esgotado");
      goto shutdown;
    }
    {
      int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (fd < 0) {
        text("Socket indisponível");
        goto shutdown;
      }
      timeval tv{0, 150000};
      setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
      sockaddr_in local{};
      local.sin_family = AF_INET;
      local.sin_port = htons(PORT);
      local.sin_addr.s_addr = htonl(INADDR_ANY);
      if (bind(fd, (sockaddr *)&local, sizeof local)) {
        close(fd);
        text("Porta LAN ocupada");
        goto shutdown;
      }
      int64_t window = 0;
      unsigned received = 0;
      while (!stop) {
        Tx tx{};
        while (xQueueReceive(queue, &tx, 0) == pdTRUE) {
          sockaddr_in target{};
          target.sin_family = AF_INET;
          target.sin_port = htons(PORT);
          if (inet_pton(AF_INET, tx.ip, &target.sin_addr) == 1) {
            auto data = visit::encode(tx.packet);
            sendto(fd, data.data(), data.size(), 0, (sockaddr *)&target,
                   sizeof target);
          }
        }
        uint8_t bytes[256];
        int n = recv(fd, bytes, sizeof bytes, 0);
        if (n > 0) {
          int64_t now = esp_timer_get_time();
          if (now - window > 1000000) {
            received = 0;
            window = now;
          }
          if (++received > 4)
            continue;
          visit::Packet packet{};
          if (visit::decode(bytes, n, ownCode, packet)) {
            xSemaphoreTake(guard, portMAX_DELAY);
            if (!waiting) {
              incoming = packet;
              waiting = true;
            }
            xSemaphoreGive(guard);
          }
        }
      }
      close(fd);
    }
  shutdown:
    esp_wifi_disconnect();
    esp_wifi_stop();
  }
end:
  connected = false;
  worker = nullptr;
  memset(password, 0, sizeof password);
  active = false;
  vTaskDelete(nullptr);
}
void connect(const char *s, const char *p, uint32_t code) {
  if (active || !s || !p || strlen(s) > 32 || strlen(p) > 64 || !strlen(s) ||
      code < 100000 || code > 999999)
    return;
  if (!guard)
    guard = xSemaphoreCreateMutex();
  if (!transport)
    transport = xSemaphoreCreateBinary();
  if (!queue)
    queue = xQueueCreate(4, sizeof(Tx));
  if (!guard || !transport || !queue)
    return;
  snprintf(ssid, sizeof ssid, "%s", s);
  snprintf(password, sizeof password, "%s", p);
  ownCode = code;
  stop = false;
  active = true;
  xQueueReset(queue);
  xTaskCreate(task, "gotchi_lan", 12288, nullptr, 3, &worker);
}
bool send(const char *ip, const visit::Packet &p) {
  if (!online() || !ip || strlen(ip) > 15)
    return false;
  in_addr target{};
  if (inet_pton(AF_INET, ip, &target) != 1 || !localMask ||
      (target.s_addr & localMask.load()) !=
          (localIP.load() & localMask.load()) ||
      target.s_addr == localIP.load())
    return false;
  Tx tx{};
  snprintf(tx.ip, sizeof tx.ip, "%s", ip);
  tx.packet = p;
  return xQueueSend(queue, &tx, 0) == pdTRUE;
}
bool poll(visit::Packet &p) {
  if (!guard)
    return false;
  xSemaphoreTake(guard, portMAX_DELAY);
  bool ok = waiting;
  if (ok) {
    p = incoming;
    waiting = false;
  }
  xSemaphoreGive(guard);
  return ok;
}
} // namespace lan
