#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static uint8_t battery_percent;
static bool battery_state;

// // Debugging var
// static GSize s_time_layer_size;
static GFont s_time_font;
static GFont s_status_font;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static BitmapLayer *s_battery_layer;


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);

  // Create buffer and write date into buffer
  static char date_text[] = "Xxxxxxxxx 00";
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  // Set date text
  text_layer_set_text(s_date_layer, date_text);
}

static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  battery_percent = new_state.charge_percent;
  battery_state = new_state.is_charging;

  GBitmap *battery_image;

  if (battery_state)
  {
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERYCHARGING);
  }
  else
  {
    switch(battery_percent) {
      case 100:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY100);
        break;
      case 90:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY090);
        break;
      case 80:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY080);
        break;
      case 70:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY070);
        break;
      case 60:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY060);
        break;
      case 50:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY050);
        break;
      case 40:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY040);
        break;
      case 30:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY030);
        break;
      case 20:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY020);
        break;
      case 10:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY010);
        break;
      case 0:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY000);
        break;
      default:
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY100);
        break;
      }
  }

  bitmap_layer_set_bitmap(s_battery_layer, battery_image);
  layer_mark_dirty(bitmap_layer_get_layer(s_battery_layer));
}

static void main_window_load(Window *window) {
  //Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));


  // Create Battery Bitmap and layer
  s_battery_layer = bitmap_layer_create(GRect(0, 85, 144, 75));
  bitmap_layer_set_alignment(s_battery_layer, GAlignBottom);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_battery_layer));

  // // Change battery bitmap to show current battery status
  // bitmap_layer_set_bitmap(s_battery_layer, s_battery050_bitmap);
  // layer_mark_dirty(bitmap_layer_get_layer(s_battery_layer));

  // Create GFont for Time Layer
  s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);


  // Figure out dimensions for Time Layer
  GSize time_max_size = graphics_text_layout_get_content_size("00:00", s_time_font, GRect(7, 41, 132, 88), GTextOverflowModeWordWrap , GTextAlignmentCenter);
//   // Debugging
//   APP_LOG(APP_LOG_LEVEL_INFO , "Max Layer Height is %i", time_max_size.h);
//   APP_LOG(APP_LOG_LEVEL_INFO , "Max Layer Width is %i", time_max_size.w);

  // Background Image section sizes
  int bg_header_size = 5;
  int bg_window_size = 88;

  // Default textlayer padding value
  int textlayer_padding = 5;

  int time_text_y = bg_header_size + (bg_window_size / 2) - (time_max_size.h / 2) - textlayer_padding;
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(7, time_text_y, 132, 88));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");


  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);


  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
//   // Debugging Code
//   GRect time_layer_frame = layer_get_frame(text_layer_get_layer(s_time_layer));
//   APP_LOG(APP_LOG_LEVEL_INFO , "Layer Height is %i", s_time_layer_size.h);
//   APP_LOG(APP_LOG_LEVEL_INFO , "Layer Width is %i", s_time_layer_size.w);
//   APP_LOG(APP_LOG_LEVEL_INFO , "Frame Origin Y is %i", time_layer_frame.origin.y);
//   APP_LOG(APP_LOG_LEVEL_INFO , "Frame Origin x is %i", time_layer_frame.origin.y);

  // Create second font, apply it and add to Window
  s_status_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);


  // Create Date Layer
  s_date_layer = text_layer_create(GRect(8, 0, 128, 88));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "January 1, 1900");

  // Set font for date layer
  text_layer_set_font(s_date_layer, s_status_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  // Get the current battery level
  battery_handler(battery_state_service_peek());

  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_battery_layer);

  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Subscribe to the Battery State Service
  battery_state_service_subscribe(battery_handler);

}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
