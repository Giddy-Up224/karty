#include "ESP32_NOW_Serial.h"
#include "MacAddress.h"
#include "WiFi.h"
#include <LVGL_CYD.h>

/* Takes position of USB connector relative to screen. These are synonyms
   as defined in LVGL_CYD.h:

      #define USB_DOWN  LV_DISPLAY_ROTATION_0
      #define USB_RIGHT LV_DISPLAY_ROTATION_90
      #define USB_UP    LV_DISPLAY_ROTATION_180
      #define USB_LEFT  LV_DISPLAY_ROTATION_270
*/ 
#define SCREEN_ORIENTATION USB_DOWN
#define HORZ_RES 240
#define VERT_RES 320
#define SCREEN_CENTER_X (HORZ_RES / 2)
#define SCREEN_CENTER_Y (VERT_RES / 2)
#define JOY_DIAM 120
#define JOY_RADIUS JOY_DIAM / 2

lv_obj_t * lbl_header;

const uint8_t joy_centered_pos[] = {SCREEN_CENTER_X - JOY_RADIUS, SCREEN_CENTER_Y - JOY_RADIUS};
const uint8_t joy_forw_rev_min_max[] = {(SCREEN_CENTER_X - 5)}; // edge padding

#include "esp_wifi.h"

// 0: AP mode, 1: Station mode
#define ESPNOW_WIFI_MODE_STATION 0

// Channel to be used by the ESP-NOW protocol
#define ESPNOW_WIFI_CHANNEL 1

#if ESPNOW_WIFI_MODE_STATION          // ESP-NOW using WiFi Station mode
#define ESPNOW_WIFI_MODE WIFI_STA     // WiFi Mode
#define ESPNOW_WIFI_IF   WIFI_IF_STA  // WiFi Interface
#else                                 // ESP-NOW using WiFi AP mode
#define ESPNOW_WIFI_MODE WIFI_AP      // WiFi Mode
#define ESPNOW_WIFI_IF   WIFI_IF_AP   // WiFi Interface
#endif

// Set the MAC address of the device that will receive the data
//                           C8:   C9:   A3:   CB:   B0:   B0
const MacAddress peer_mac({0xC8, 0xC9, 0xA3, 0xCB, 0xB0, 0xB0});

ESP_NOW_Serial_Class NowSerial(peer_mac, ESPNOW_WIFI_CHANNEL, ESPNOW_WIFI_IF);

void setup() {
  LVGL_CYD::begin(SCREEN_ORIENTATION);

  // LVGL has multiple layers, one below and two above the 'active screen'.
  // I use the 'bottom' layer for the background color and keep all further
  // objects on transparent background. I use the 'top' layer for the screen
  // header and the exit button.

  // bottom layer opaque and white
  lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lv_layer_bottom(), lv_color_white(), LV_PART_MAIN);

  // page header
  lbl_header = lv_label_create(lv_layer_top());
  lv_obj_set_style_text_font(lbl_header, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_align(lbl_header, LV_ALIGN_TOP_LEFT, 5, 3);

  go_touch();
  
  Serial.begin(115200);

  Serial.print("WiFi Mode: ");
  Serial.println(ESPNOW_WIFI_MODE == WIFI_AP ? "AP" : "Station");
  WiFi.mode(ESPNOW_WIFI_MODE);

  Serial.print("Channel: ");
  Serial.println(ESPNOW_WIFI_CHANNEL);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  while (!(WiFi.STA.started() || WiFi.AP.started())) {
    delay(100);
  }

  Serial.print("MAC Address: ");
  Serial.println(ESPNOW_WIFI_MODE == WIFI_AP ? WiFi.softAPmacAddress() : WiFi.macAddress());

  // Start the ESP-NOW communication
  Serial.println("ESP-NOW communication starting...");
  NowSerial.begin(115200);
  Serial.printf("ESP-NOW version: %d, max data length: %d\n", ESP_NOW.getVersion(), ESP_NOW.getMaxDataLen());
  Serial.println("You can now send data to the peer device using the Serial Monitor.\n");
}

// unsigned long last_time = 0;
// const long delay_time = 1000;

void loop() {
  
  lv_task_handler();  // let the GUI do its work
  
  #if 0
  unsigned long now = millis();
  if(now - last_time >= delay_time) {
    last_time = now; // save current time
  
    // Send command frame
    if(NowSerial.availableForWrite()) {
      if(NowSerial.write(x_y, sizeof(x_y)) <= 0) {
        Serial.println("Failed to send data");
        // break;
      }
    }
  }
  #endif

  delay(1);
}

// creates a new obj to use as base screen, and set some properties, such as flex
// vertical arrangement, centered horizontally, transparent background, etc.
lv_obj_t * new_screen(lv_obj_t * parent) {

  lv_obj_t * obj = lv_obj_create(parent);
  lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(obj, 0, 0);
  lv_obj_set_layout(obj, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_top(obj, 20, LV_PART_MAIN);
  lv_obj_set_style_pad_row(obj, 10, LV_PART_MAIN);

  return obj;
}

lv_obj_t * scr_touch;

// These need to be global as they are used in the callback.
lv_obj_t *base_joyStick;  // Background for joystick
lv_obj_t *rc_joyStick;    // Center object of joystick

void go_touch() {

  if (!scr_touch) {

    scr_touch = new_screen(NULL);
    lv_obj_set_layout(scr_touch, LV_LAYOUT_NONE);         // need to place children (the crosshairs) manually
    lv_obj_set_style_pad_top(scr_touch, 0, LV_PART_MAIN);

    rc_joyStick = lv_btn_create(scr_touch);
    lv_obj_set_size(rc_joyStick, JOY_DIAM, JOY_DIAM);
    lv_obj_set_style_radius(rc_joyStick, LV_RADIUS_CIRCLE, 0);
    // Start out centered on screen
    lv_obj_set_pos(rc_joyStick, joy_centered_pos[0], joy_centered_pos[1]);
    
    lv_obj_add_event_cb(rc_joyStick, [](lv_event_t * e) -> void {
      uint16_t joy_x;
      uint16_t joy_y;
      if (lv_event_get_code(e) == LV_EVENT_PRESSED || lv_event_get_code(e) == LV_EVENT_PRESSING) {
        lv_point_t point;
        lv_indev_get_point(lv_indev_get_act(), &point); // Get the current touch point
        joy_x = point.x - JOY_RADIUS;
        joy_y = point.y - JOY_RADIUS;
        lv_obj_set_pos(rc_joyStick, joy_x, joy_y);
        // Convert to forward and reverse rather than x and y
        uint8_t comm[2];
        send_x_y(comm, sizeof(comm), point.x, point.y);
      } else if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
        lv_obj_set_pos(rc_joyStick, joy_centered_pos[0], joy_centered_pos[1]);
      }
    }, LV_EVENT_ALL, NULL);
  }
  lv_label_set_text(lbl_header, "Joy Stick");
  lv_screen_load(scr_touch);
}

void send_x_y(uint8_t *x_y, size_t size, uint64_t x, uint64_t y) {
  // int division by 2
  x_y[0] = x >> 1;
  x_y[1] = y >> 1;
  // Send command frame
  if(NowSerial.availableForWrite()) {
    if(NowSerial.write(x_y, size) <= 0) {
      Serial.println("Failed to send data");
    }
  }
}