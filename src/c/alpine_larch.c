#include <pebble.h>
#include "utils.h"

#define DEBUG_TIME (false)
#define DEBUG_BBOX (false)
#define BUFFER_LEN (20)
#define SETTINGS_KEY (1)

typedef struct ClaySettings {
  GColor color_background;
  GColor color_hour_circle;
  GColor color_hour;
  GColor color_minute;
  GColor color_date;
  uint8_t buffer[40];  // save 40 bytes for later expansion
} __attribute__((__packed__)) ClaySettings;

ClaySettings settings;

static void default_settings() {
  settings.color_background        = GColorDarkCandyAppleRed;
  settings.color_hour_circle       = GColorBlack;
  settings.color_hour              = GColorDarkCandyAppleRed;
  settings.color_minute            = GColorBlack;
  settings.color_date              = GColorDarkCandyAppleRed;
}

static Window* s_window;
static Layer* s_layer;
static char s_buffer[BUFFER_LEN];
static GFont s_font_lg = NULL;
static GFont s_font_sm = NULL;

static void draw_minute(GContext* ctx, const char* buffer, GRect bbox) {
  int h = bbox.size.h;
  int font_height = 18;
  int top_pad = 9;
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  int bot_pad = h - font_height - top_pad;
  int shift_up = (top_pad - bot_pad) / 2 + 1;
  GRect fixed_bbox = GRect(bbox.origin.x, bbox.origin.y - shift_up, bbox.size.w, bbox.size.h);
  graphics_draw_text(ctx, buffer, font, fixed_bbox, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void draw_numbers(GContext* ctx, GPoint center, int vcr, struct tm* now) {
  // Hour
  int min_width = 28;
  int min_height = 28;
  int min_size = max(min_width, min_height);
  int hour_height = 85;
  int hour_radius = vcr - min_size + 1;
  GRect hour_bbox = rect_from_midpoint(center, GSize(vcr * 2, hour_height));

  graphics_context_set_fill_color(ctx, settings.color_hour_circle);
  graphics_fill_circle(ctx, center, hour_radius);
  graphics_context_set_text_color(ctx, settings.color_hour);
  format_hour(s_buffer, BUFFER_LEN, now);
  graphics_draw_text(ctx, s_buffer, s_font_lg, hour_bbox, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  // Day of week above the hour
  int date_height = 23;
  GRect above_hour = rect_from_midpoint(
    (GPoint){
      .x = center.x,
      .y = center.y - hour_radius * 5 / 8,
    },
    (GSize){
      .w = vcr * 2,
      .h = date_height,
    }
  );
  graphics_context_set_text_color(ctx, settings.color_date);
  strftime(s_buffer, BUFFER_LEN, "%a", now);
  graphics_draw_text(ctx, s_buffer, s_font_sm, above_hour, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  // Monthdate below the hour
  GRect below_hour = rect_from_midpoint(
    (GPoint){
      .x = center.x,
      .y = center.y + hour_radius * 5 / 8,
    },
    (GSize){
      .w = vcr * 2,
      .h = date_height,
    }
  );
  graphics_context_set_text_color(ctx, settings.color_date);
  format_date(s_buffer, BUFFER_LEN, now);
  graphics_draw_text(ctx, s_buffer, s_font_sm, below_hour, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  // Minute
  const int MIN_MAX = 60;
  int min = now->tm_min;
  int min_angle = min * (TRIG_MAX_RATIO / MIN_MAX);
  int min_text_center_radius = hour_radius + min_size / 2 + 2;
  graphics_context_set_text_color(ctx, settings.color_minute);
  strftime(s_buffer, BUFFER_LEN, "%M", now);
  GPoint min_text_center = cartesian_from_polar(center, min_text_center_radius, min_angle);
  GRect min_bbox = rect_from_midpoint(min_text_center, GSize(min_width, min_height));
  draw_minute(ctx, s_buffer, min_bbox);

  if (DEBUG_BBOX) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    //graphics_draw_rect(ctx, above_hour);
    //graphics_draw_rect(ctx, hour_bbox);
    //graphics_draw_rect(ctx, below_hour);
    graphics_draw_rect(ctx, min_bbox);
  }
}

static void update_layer(Layer* layer, GContext* ctx) {
  time_t temp = time(NULL);
  struct tm* now = localtime(&temp);
  if (DEBUG_TIME) {
    fast_forward_time(now);
  }

  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, settings.color_background);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  int vcr = min(bounds.size.h, bounds.size.w) / 2;
  GPoint center = grect_center_point(&bounds);
  draw_numbers(ctx, center, vcr, now);
}

static void window_load(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(s_window, settings.color_background);
  s_layer = layer_create(bounds);
  layer_set_update_proc(s_layer, update_layer);
  layer_add_child(window_layer, s_layer);
}

static void window_unload(Window* window) {
  if (s_layer) layer_destroy(s_layer);
}

static void tick_handler(struct tm* now, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void load_settings() {
  default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t;
  if ((t = dict_find(iter, MESSAGE_KEY_color_background       ))) { settings.color_background         = GColorFromHEX(t->value->int32); }
  if ((t = dict_find(iter, MESSAGE_KEY_color_hour_circle      ))) { settings.color_hour_circle        = GColorFromHEX(t->value->int32); }
  if ((t = dict_find(iter, MESSAGE_KEY_color_hour             ))) { settings.color_hour               = GColorFromHEX(t->value->int32); }
  if ((t = dict_find(iter, MESSAGE_KEY_color_minute           ))) { settings.color_minute             = GColorFromHEX(t->value->int32); }
  if ((t = dict_find(iter, MESSAGE_KEY_color_date             ))) { settings.color_date               = GColorFromHEX(t->value->int32); }
  save_settings();
  // Update the display based on new settings
  layer_mark_dirty(window_get_root_layer(s_window));
}

static void init(void) {
  s_font_lg = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MICHROMA_68));
  s_font_sm = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MICHROMA_18));
  load_settings();
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
  tick_timer_service_subscribe(DEBUG_TIME ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
  if (s_window) window_destroy(s_window);
  if (s_font_lg) { fonts_unload_custom_font(s_font_lg); }
  if (s_font_sm) { fonts_unload_custom_font(s_font_sm); }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}