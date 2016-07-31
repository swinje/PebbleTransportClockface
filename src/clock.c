#include "clock.h"
#include "pebble.h"

static Window *s_window;
static Layer *s_simple_bg_layer, *s_date_layer, *s_hands_layer;
static TextLayer *s_day_label, *s_num_label;
// Added for messaging text
static TextLayer *s_text_label1, *s_text_label2;;

static GPath *s_tick_paths[NUM_CLOCK_TICKS];
static GPath *s_minute_arrow, *s_hour_arrow;
static char s_num_buffer[4], s_day_buffer[6];

// Start app messaging
static AppSync s_sync;
static uint8_t s_sync_buffer[64];

enum DirectionKey {
  DIRECTION_KEY = 0x0,      // TUPLE_INT
  DEPTIME_KEY = 0x1,        // TUPLE_CSTRING
};

// Error Callback for messaging
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

// Callback for messaging
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  
  static uint8_t direction = 0;
  static char label1[] = "        ";
  static char label2[] = "        ";
  
  switch (key) {
    case DIRECTION_KEY:
      // App Sync keeps new_tuple in s_sync_buffer, so we may use it directly
      direction= new_tuple->value->uint8;
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Received direction message %i", direction);
      break;   
    case DEPTIME_KEY:
       //APP_LOG(APP_LOG_LEVEL_DEBUG, "Received time message <%s>", new_tuple->value->cstring);
      if (strlen(new_tuple->value->cstring)>1) {
        if(direction==1) 
          strcpy(label1, new_tuple->value->cstring);
        if(direction==2) 
          strcpy(label2, new_tuple->value->cstring);
      }
      text_layer_set_text(s_text_label1, label1);
      text_layer_set_text(s_text_label2, label2);
      break;
  }
}

// Send message to JS
static void request_js(void) {
  DictionaryIterator *iter;
  AppMessageResult ar = app_message_outbox_begin(&iter);

  if (!iter) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "failed sending message %i", ar);
    // Error creating outbound message
    return;
  }

  // Write in values to send
  int value = 1;
  dict_write_int(iter, 1, &value, sizeof(int), true);
  dict_write_end(iter);

  app_message_outbox_send();  
}

// Runs as result of layer marked dirty
static void bg_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
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
  graphics_context_set_stroke_color(ctx, GColorWhite);
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
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(bounds.size.w / 2 - 1, bounds.size.h / 2 - 1, 3, 3), 0, GCornerNone);
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
    GRect(63, 114, 27, 20),
    GRect(46, 114, 27, 20)));
  text_layer_set_text(s_day_label, s_day_buffer);
  text_layer_set_background_color(s_day_label, GColorBlack);
  text_layer_set_text_color(s_day_label, GColorWhite);
  text_layer_set_font(s_day_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));

  layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

  s_num_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(90, 114, 18, 20),
    GRect(73, 114, 18, 20)));
  text_layer_set_text(s_num_label, s_num_buffer);
  text_layer_set_background_color(s_num_label, GColorBlack);
  text_layer_set_text_color(s_num_label, GColorWhite);
  text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));

  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
    
  // Load data for messaging
  s_text_label1 = text_layer_create(GRect(0, 30, bounds.size.w, 32));
  text_layer_set_text_color(s_text_label1, GColorWhite);
  text_layer_set_background_color(s_text_label1, GColorClear);
  text_layer_set_font(s_text_label1, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_text_label1, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_label1));
 
  s_text_label2 = text_layer_create(GRect(0, 50, bounds.size.w, 32));
  text_layer_set_text_color(s_text_label2, GColorWhite);
  text_layer_set_background_color(s_text_label2, GColorClear);
  text_layer_set_font(s_text_label2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_text_label2, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_label2));
    
  Tuplet initial_values[] = {
    TupletInteger(DIRECTION_KEY, (uint8_t) 1),      
    TupletCString(DEPTIME_KEY, "     "),          
  };

  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  // End messaging

}

static void window_unload(Window *window) {
  layer_destroy(s_simple_bg_layer);
  layer_destroy(s_date_layer);

  text_layer_destroy(s_day_label);
  text_layer_destroy(s_num_label);
  text_layer_destroy(s_text_label1);
  text_layer_destroy(s_text_label2);

  layer_destroy(s_hands_layer);
}


static void init() {
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
  
  app_message_open(128, 128);
  
   
}

static void deinit() {
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);

  for (int i = 0; i < NUM_CLOCK_TICKS; ++i) {
    gpath_destroy(s_tick_paths[i]);
  }

  tick_timer_service_unsubscribe();
  window_destroy(s_window);
  
  // End messaging
  app_sync_deinit(&s_sync);

}

int main() {
  init();
  app_event_loop();
  deinit();
}
