#include <pebble.h>

static Window *s_main_window;
static GBitmap *s_bitmap;
static BitmapLayer *s_battery_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static GFont s_time_font;

static uint8_t battery_percent;
static bool battery_state;

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
//   static char date_text[] = "Xxxxxxxxx 00";
//   strftime(date_text, sizeof(date_text), "%B %e", tick_time);
//   // Set date text
//   text_layer_set_text(s_date_layer, date_text);
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

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Setup the window layer
  Layer *window_layer = window_get_root_layer(window);
  // Set the bounds for the main window layer
  GRect bounds = layer_get_bounds(window_layer);

  // Set default bitmap image for battery indicator
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY100);

  // Create bitmap layer within the bounds of the main window layer
  s_battery_layer = bitmap_layer_create(bounds);
  // Apply the battery bitmap to the bitmap layer
  bitmap_layer_set_bitmap(s_battery_layer, s_bitmap);
  // Align the bitmap the bottom of the bitmap layer bounds
  bitmap_layer_set_alignment(s_battery_layer, GAlignBottom);
  // Set correct compositing mode based upon pebble type
#ifdef PBL_PLATFORM_APLITE
  bitmap_layer_set_compositing_mode(s_battery_layer, GCompOpAssign);
#elif PBL_PLATFORM_BASALT
  bitmap_layer_set_compositing_mode(s_battery_layer, GCompOpSet);
#endif
  // Add the battery bitmap layet to the main window layer
  layer_add_child(window_layer, bitmap_layer_get_layer(s_battery_layer));
  
  // Create GFont for Time Layer
  s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS);
  // Figure out dimensions for Time Layer
  GSize time_max_size = graphics_text_layout_get_content_size("00:00", s_time_font, GRect(7, 41, 132, 88), GTextOverflowModeWordWrap , GTextAlignmentCenter);
  
  // Default padding values
  int bg_header_size = 1;
  int bg_window_size = 88;
  int textlayer_padding = 26;

  // Setup time text and layer
  int time_text_y = bg_header_size + (bg_window_size / 2) - (time_max_size.h / 2) - textlayer_padding;
  
  s_time_layer = text_layer_create(GRect(7, time_text_y, 132, 88));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");

  // Apply Time to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // Get the current battery level
//   battery_handler(battery_state_service_peek());
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy the bitmap layer
  bitmap_layer_destroy(s_battery_layer);
  // Destroy the bitmap object
  gbitmap_destroy(s_bitmap);
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}

static void init() {
  // Create the main window
  s_main_window = window_create();
  // Set fullscreen mode if old pebble SDK
#ifdef PBL_SDK_2
  window_set_fullscreen(s_main_window, true);
#endif
  // Set the background color. First color is primary, fallback to second color if unsupported
  window_set_background_color(s_main_window, COLOR_FALLBACK(GColorBlack, GColorBlack));
  // Setup the window handlers
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  // Use the main window object
  window_stack_push(s_main_window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Subscribe to the Battery State Service
//   battery_state_service_subscribe(battery_handler);
}

static void deinit() {
  // Kill the main window
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}