#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0xED, 0x64, 0x74, 0x79, 0x19, 0x7B, 0x41, 0x53, 0x8C, 0x10, 0x11, 0xD3, 0xB7, 0x88, 0x0A, 0xBE }
PBL_APP_INFO(MY_UUID,
             "Planetarium", "Dev Discoveries",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

BmpContainer background_image_container;

RotBmpContainer hour_hand_image_container;
RotBmpContainer minute_hand_image_container;
RotBmpContainer second_hand_image_container;

int32_t yHour = 70;
int32_t yMin = 45;
int32_t ySec = 25;

GPoint hourOffset;
GPoint minOffset;
GPoint secOffset;

bool init;

void set_init_coords(RotBmpContainer *image, GPoint initCoords) {
    GRect r = layer_get_frame(&image->layer.layer);
    r.origin.x = initCoords.x + 144 / 2;
    r.origin.y = -initCoords.y + 168 / 2;
    layer_set_frame(&image->layer.layer, r);
}

void set_angled_position(RotBmpContainer *image, int32_t angle, int32_t length, GPoint offset) {
    GPoint r =
    GPoint((length * sin_lookup(angle) / TRIG_MAX_RATIO) - offset.x, (length * cos_lookup(angle) / TRIG_MAX_RATIO) + offset.y);
    set_init_coords(image, r);
}

void update_hand_positions() {
    PblTm t;
    get_time(&t);
    
    int sec = t.tm_sec;
    int min = t.tm_min;
    int hour = t.tm_hour;
    
    int32_t aSec = TRIG_MAX_ANGLE * (sec * 6) /360;
    set_angled_position(&second_hand_image_container, aSec, ySec, secOffset);
    layer_mark_dirty(&second_hand_image_container.layer.layer);
    
    if (!init || sec % 10 == 0) {
        int32_t aMin = TRIG_MAX_ANGLE * ((min * 6) + sec / 10) /360;
        set_angled_position(&minute_hand_image_container, aMin, yMin, minOffset);
        minute_hand_image_container.layer.rotation = aMin;
        layer_mark_dirty(&minute_hand_image_container.layer.layer);
    }
    
    if (!init || min % 5 == 0) {
        int32_t aHour = TRIG_MAX_ANGLE * (((hour % 12) * 30 + min / 2)) / 360;
        set_angled_position(&hour_hand_image_container, aHour, yHour, hourOffset);
        hour_hand_image_container.layer.rotation = aHour;
        layer_mark_dirty(&hour_hand_image_container.layer.layer);
    }
}

void handle_second_tick(AppContextRef ctx, PebbleTickEvent *t) {
    (void)t;
    update_hand_positions(); // TODO: Pass tick event
}

void handle_init(AppContextRef ctx) {
  (void)ctx;
    init = false;

    window_init(&window, "Planets Window");
    window_stack_push(&window, true /* Animated */);
    
    resource_init_current_app(&APP_RESOURCES);
    
    // Set up a layer for the static watch face background
    bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &background_image_container);
    layer_add_child(&window.layer, &background_image_container.layer.layer);
    
    // Set up a layer for the hour hand
    rotbmp_init_container(RESOURCE_ID_IMAGE_HOUR_HAND, &hour_hand_image_container);
    GSize hourSize = layer_get_frame(&hour_hand_image_container.layer.layer).size;
    hourOffset = GPoint(hourSize.w / 2, hourSize.h / 2);
    layer_add_child(&window.layer, &hour_hand_image_container.layer.layer);
    
    
    // Set up a layer for the minute hand
    rotbmp_init_container(RESOURCE_ID_IMAGE_MINUTE_HAND, &minute_hand_image_container);
    GSize minSize = layer_get_frame(&minute_hand_image_container.layer.layer).size;
    minOffset = GPoint(minSize.w / 2, minSize.h / 2);
    layer_add_child(&window.layer, &minute_hand_image_container.layer.layer);
    
    
    // Set up a layer for the second hand
    rotbmp_init_container(RESOURCE_ID_IMAGE_SECOND_HAND, &second_hand_image_container);
    GSize secSize = layer_get_frame(&second_hand_image_container.layer.layer).size;
    secOffset = GPoint(secSize.w / 2, secSize.h / 2);
    layer_add_child(&window.layer, &second_hand_image_container.layer.layer);
    
    update_hand_positions();
    init = true;
}

void handle_deinit(AppContextRef ctx) {
    (void)ctx;
    bmp_deinit_container(&background_image_container);
    rotbmp_deinit_container(&second_hand_image_container);
    rotbmp_deinit_container(&minute_hand_image_container);
    rotbmp_deinit_container(&hour_hand_image_container);
}

void pbl_main(void *params) {
  AppContextRef ctx = (AppContextRef) params;
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
      
    .tick_info = {
          .tick_handler = &handle_second_tick,
          .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(ctx, &handlers);
}
