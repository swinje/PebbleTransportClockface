#include <pebble.h>
#include "digital.h"
#include "message.h"

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;
//static TextLayer *s_connection_layer;
static Layer *s_date_layer;
static Layer *s_blue_layer;
static Layer *s_red_layer;
static TextLayer *s_blue_text_layer;
static TextLayer *s_red_text_layer;
char blue_buffer[10];
char red_buffer[10];
static char s_num_buffer[4], s_day_buffer[6];
static TextLayer *s_day_label, *s_num_label;

GFont custom_font;

// Weather
static TextLayer *s_temperature_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;

static int depTime1 = 0, depTime2 = 0;

void switch_digital_icon(int icon_index) {
 if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }

      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Icon is <%d>", atoi(new_tuple->value->cstring));
      s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[icon_index-1]);
      bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
}

void set_digital_temp(const char *tmpstr) {
  text_layer_set_text(s_temperature_layer, tmpstr);
}

void set_digital_deptime(int dir, int tm) {
  if (dir ==1) {
    depTime1 = tm;
    snprintf(blue_buffer, 10, "%d", depTime1);
    text_layer_set_text(s_blue_text_layer, blue_buffer);
  }
  else {
    depTime2 = tm;
    snprintf(red_buffer, 10, "%d", depTime2);
    text_layer_set_text(s_red_text_layer, red_buffer);
  }
  
  layer_mark_dirty(window_get_root_layer(s_main_window));
}


static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00:00:00";
  
  //static int depTick = 30; // update departure every 30 seconds, start at once
  //if (depTick == 30) {
  //  depTick = 0;
    request_js();  
  //} 
  //depTick++;

  strftime(s_time_text, sizeof(s_time_text), "%R", tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_font(s_time_layer, custom_font);
  
}

/*
static void handle_bluetooth(bool connected) {
  text_layer_set_text(s_connection_layer, connected ? "connected" : "disconnected");
}
*/

static void blue_update_proc(Layer *layer, GContext *ctx) {
  // Custom drawing happens here!
  // Set the line color
  graphics_context_set_stroke_color(ctx, GColorBlack);
  // Set the fill color
  graphics_context_set_fill_color(ctx, GColorBlue);
  // Set the stroke width (must be an odd integer value)
  graphics_context_set_stroke_width(ctx, 5);
  
  GRect rect_bounds = layer_get_bounds(layer);
  
  if (depTime1 <= 10) {
    int steps = (rect_bounds.size.h-rect_bounds.origin.y)/10;
    rect_bounds.origin.y +=  steps * (10-depTime1);
  }

  // Draw a rectangle
  graphics_draw_rect(ctx, rect_bounds);
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  // Disable antialiasing (enabled by default where available)
  graphics_context_set_antialiased(ctx, false);

}

static void red_update_proc(Layer *layer, GContext *ctx) {
  // Custom drawing happens here!
  // Set the line color
  graphics_context_set_stroke_color(ctx, GColorBlack);
  // Set the fill color
  graphics_context_set_fill_color(ctx, GColorRed);
  // Set the stroke width (must be an odd integer value)
  graphics_context_set_stroke_width(ctx, 5);
  
  GRect rect_bounds = layer_get_bounds(layer);
   
  if (depTime2 <= 10) {
    int steps = (rect_bounds.size.h-rect_bounds.origin.y)/10;
    rect_bounds.origin.y +=  steps * (10-depTime2);
  }
 
  // Draw a rectangle
  graphics_draw_rect(ctx, rect_bounds);
  graphics_fill_rect(ctx, rect_bounds, 0, GCornerNone);

  // Disable antialiasing (enabled by default where available)
  graphics_context_set_antialiased(ctx, false);
  
}

// Runs as result of layer marked dirty
static void date_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  strftime(s_day_buffer, sizeof(s_day_buffer), "%a", t);
  text_layer_set_text(s_day_label, s_day_buffer);

  strftime(s_num_buffer, sizeof(s_num_buffer), "%d", t);
  text_layer_set_text(s_num_label, s_num_buffer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_time_layer = text_layer_create(GRect(0, 55, bounds.size.w, 50));
  text_layer_set_text_color(s_time_layer, GColorYellow);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  /*
  s_connection_layer = text_layer_create(GRect(0, 110, bounds.size.w, 34));
  text_layer_set_text_color(s_connection_layer, GColorWhite);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentCenter);
  handle_bluetooth(connection_service_peek_pebble_app_connection());
  */

  s_battery_layer = text_layer_create(GRect(0, 140, bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "100%");
  
  // need to use bounds here
  s_blue_layer = layer_create(GRect(5, 30, 10, 120));
  layer_set_update_proc(s_blue_layer, blue_update_proc);
  layer_add_child(window_layer, s_blue_layer);
  
  s_blue_text_layer = text_layer_create(GRect(0, 0, 20, 34));
  text_layer_set_text_color(s_blue_text_layer, GColorBlue);
  text_layer_set_background_color(s_blue_text_layer, GColorClear);
  text_layer_set_font(s_blue_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_blue_text_layer, GTextAlignmentLeft);
  
  // need to use bounds here
  s_red_layer = layer_create(GRect(130, 30, 10, 120));
  layer_set_update_proc(s_red_layer, red_update_proc);
  layer_add_child(window_layer, s_red_layer);
  
  s_red_text_layer = text_layer_create(GRect(123, 0, 20, 34));
  text_layer_set_text_color(s_red_text_layer, GColorRed);
  text_layer_set_background_color(s_red_text_layer, GColorClear);
  text_layer_set_font(s_red_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_red_text_layer, GTextAlignmentRight); 
  
  // Weather
  s_icon_layer = bitmap_layer_create(GRect(0, 5, bounds.size.w, 50));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_temperature_layer = text_layer_create(GRect(0, 35, bounds.size.w, 32));
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));
  
  // Date and time
  s_date_layer = layer_create(bounds);
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_layer, s_date_layer);

  s_day_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(30, 114, 60, 30),
    GRect(30, 114, 60, 30)));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorBlack);
  text_layer_set_text_color(s_day_label, GColorWhite);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_day_label, GTextAlignmentCenter);

  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

  s_num_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(75, 114, 28, 30),
    GRect(75, 114, 28, 30)));
  text_layer_set_text(s_num_label, s_num_buffer);
  text_layer_set_background_color(s_num_label, GColorBlack);
  text_layer_set_text_color(s_num_label, GColorWhite);
  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_num_label, GTextAlignmentCenter);

  layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));
  

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  battery_state_service_subscribe(handle_battery);

  /*
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = handle_bluetooth
  });
  */

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  //layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_blue_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_red_text_layer));

  handle_battery(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  text_layer_destroy(s_time_layer);
  //text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_battery_layer);
  layer_destroy(s_blue_layer);
  layer_destroy(s_red_layer);
  text_layer_destroy(s_blue_text_layer);
  text_layer_destroy(s_red_text_layer);
 
  layer_destroy(s_date_layer);
  text_layer_destroy(s_day_label);
  text_layer_destroy(s_num_label);
  
  // Weather
  if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }

  text_layer_destroy(s_temperature_layer);
  bitmap_layer_destroy(s_icon_layer);
}

void digital_init() {
  s_main_window = window_create();
  
  custom_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FERRO_ROSSO_48));
  
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
}

void digital_deinit() {
  window_destroy(s_main_window);
}


