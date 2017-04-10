#include "pebble.h"
#include "analog.h"
#include "message.h"

static Window *s_window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer, *s_dot_layer;
static TextLayer *s_day_label, *s_num_label;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;
static char s_num_buffer[4], s_day_buffer[6];

// Weather
static TextLayer *s_temperature_layer;
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;



static int depTime1 = -1, depTime2 = -1;

void switch_analog_icon(int icon_index) {
 if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }

      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Icon is <%d>", atoi(new_tuple->value->cstring));
      s_icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[icon_index-1]);
      bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
}

void set_analog_temp(const char *tmpstr) {
  text_layer_set_text(s_temperature_layer, tmpstr);
}

void set_analog_deptime(int dir, int tm) {
  if (dir ==1)
    depTime1 = tm;
  else
    depTime2 = tm;
}


// Runs as result of layer marked dirty
static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorLiberty);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  // To add more tick lines would need to change offset and add more ticks
  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    const int x_offset = PBL_IF_ROUND_ELSE(18, 0);
    const int y_offset = PBL_IF_ROUND_ELSE(6, 0);
    gpath_move_to(s_tick_paths[i], GPoint(x_offset, y_offset));
    gpath_draw_filled(ctx, s_tick_paths[i]);
  }
}

// Runs as result of layer marked dirty
static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  const int16_t second_hand_length = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, bounds.size.w / 2);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  GPoint second_hand = {
    .x = (int16_t)(sin_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(second_angle) * (int32_t)second_hand_length / TRIG_MAX_RATIO) + center.y,
  };

  // second hand
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, second_hand, center);

  // minute/hour hand
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  gpath_rotate_to(s_minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);

  gpath_rotate_to(s_hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);

  // dot in the middle
  graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
}

static GPoint calc_point(int depTime, GRect bounds, GPoint center) {
  time_t now = time(NULL);
  now += depTime * 60;
  struct tm *t = localtime(&now);
  const int16_t dot_distance = PBL_IF_ROUND_ELSE((bounds.size.w / 2) - 19, bounds.size.w / 2 -5);
  uint16_t dot_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  GPoint point = {
    .x = (int16_t)(sin_lookup(dot_angle) * (int32_t)dot_distance / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(dot_angle) * (int32_t)dot_distance / TRIG_MAX_RATIO) + center.y,
  };
  return point;
}

// Runs as result of layer marked dirty
static void dot_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);    
  uint16_t radius = 4;

  // Point leaving from home
  if (depTime1 >= 0) {
    GPoint home_point = calc_point(depTime1, bounds, center);
    graphics_context_set_stroke_color(ctx, GColorCyan);
    graphics_context_set_fill_color(ctx, GColorCyan);
    graphics_draw_circle(ctx, home_point, radius);
    graphics_fill_circle(ctx, home_point, radius);
  }
  // Point leaving from work
  if (depTime2 >= 0) {
    GPoint away_point = calc_point(depTime2, bounds, center);
    graphics_context_set_stroke_color(ctx, GColorRed);
    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_draw_circle(ctx, away_point, radius);
    graphics_fill_circle(ctx, away_point, radius);
  }   
 
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

// Runs as result of layer marked dirty
static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  static int depTick = 30; // update departure every 30 seconds, start at once
  if (depTick == 30) {
    depTick = 0;
    request_js();  
  } 
  depTick++;
  
  // Tells the window to redraw itself and therefore fires all the update procs
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_simple_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_simple_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_simple_bg_layer);

  s_date_layer = layer_create(bounds);
  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_layer, s_date_layer);

  s_day_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(30, 104, 60, 30),
    GRect(30, 104, 60, 30)));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorLiberty);
  text_layer_set_text_color(s_day_label, GColorBlack);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_day_label, GTextAlignmentCenter);

  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

  s_num_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(75, 104, 28, 30),
    GRect(75, 104, 28, 30)));
  text_layer_set_text(s_num_label, s_num_buffer);
  text_layer_set_background_color(s_num_label, GColorLiberty);
  text_layer_set_text_color(s_num_label, GColorBlack);
  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_num_label, GTextAlignmentCenter);

  layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));

  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
  
  s_dot_layer = layer_create(bounds);
  layer_set_update_proc(s_dot_layer, dot_update_proc);
  layer_add_child(window_layer, s_dot_layer);
    
  
  // Weather
  s_icon_layer = bitmap_layer_create(GRect(0, 10, bounds.size.w, 50));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  s_temperature_layer = text_layer_create(GRect(0, 45, bounds.size.w, 32));
  text_layer_set_text_color(s_temperature_layer, GColorBlack);
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_temperature_layer));

}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_date_layer);

  text_layer_destroy(s_day_label);
  text_layer_destroy(s_num_label);

  layer_destroy(s_hands_layer);
  layer_destroy(s_dot_layer);
  
  // Weather
    if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }

  text_layer_destroy(s_temperature_layer);
  bitmap_layer_destroy(s_icon_layer);
}


void analog_init() {
  
  s_window = window_create();
    
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  // Load
  window_stack_push(s_window, true);

  s_day_buffer[0] = '\0';
  s_num_buffer[0] = '\0';

  // init hand paths
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);

  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_arrow, center);
  gpath_move_to(s_hour_arrow, center);
 
  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    s_tick_paths[i] = gpath_create(&ANALOG_BG_POINTS[i]);
  }
  
  // Timer to update
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
   
}

void analog_deinit() {
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);

  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    gpath_destroy(s_tick_paths[i]);
  }

  tick_timer_service_unsubscribe();
  window_destroy(s_window);
  
}


