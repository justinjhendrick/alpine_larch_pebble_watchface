#pragma once
#include <pebble.h>

static GPoint cartesian_from_polar(GPoint center, int radius, int trigangle) {
  GPoint ret = {
    .x = (int16_t)(sin_lookup(trigangle) * radius / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(trigangle) * radius / TRIG_MAX_RATIO) + center.y,
  };
  return ret;
}

static GRect rect_from_midpoint(GPoint midpoint, GSize size) {
  GRect ret;
  ret.origin.x = midpoint.x - size.w / 2;
  ret.origin.y = midpoint.y - size.h / 2;
  ret.size = size;
  return ret;
}

static int min(int a, int b) {
  if (a < b) return a;
  return b;
}

static int max(int a, int b) {
  if (a > b) return a;
  return b;
}

static void fast_forward_time(struct tm* now) {
  now->tm_wday = now->tm_sec % 7;
  now->tm_mday = now->tm_sec % 32;
  now->tm_hour = now->tm_sec % 24;
  now->tm_min = now->tm_sec;
}

static void format_hour(char* buffer, int len, struct tm* now) {
  if (clock_is_24h_style()) {
    strftime(buffer, len, "%H", now);
  } else {
    strftime(buffer, len, "%I", now);
  }
}

static void format_date(char* buffer, int len, struct tm* now) {
  char* ordinal = NULL;
  int day = now->tm_mday;
  int last = day % 10;
  if (day == 11 || day == 12 || day == 13) {
    ordinal = "th";
  } else if (last == 1) {
    ordinal = "st";
  } else if (last == 2) {
    ordinal = "nd";
  } else if (last == 3) {
    ordinal = "rd";
  } else {
    ordinal = "th";
  }
  snprintf(buffer, len, "%d%s", now->tm_mday, ordinal);
}

static void draw_text_shifted(GContext* ctx, const char* buffer, GRect bbox, GFont font, int shift_up) {
  GRect fixed_bbox = GRect(bbox.origin.x, bbox.origin.y - shift_up, bbox.size.w, bbox.size.h);
  graphics_draw_text(ctx, buffer, font, fixed_bbox, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void draw_text_midalign(GContext* ctx, const char* buffer, GRect bbox) {
  int h = bbox.size.h;
  int font_height = 0;
  int top_pad = 0;
  GFont font;
  if (h < 14) {
    return;
  } else if (h < 18) {
    font_height = 9;
    top_pad = 4;
    font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  } else if (h < 24) {
    font_height = 11;
    top_pad = 6;
    font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  } else if (h < 28) {
    font_height = 14;
    top_pad = 9;
    font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  } else {
    font_height = 18;
    top_pad = 9;
    font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  }
  int bot_pad = h - font_height - top_pad;
  int shift_up = (top_pad - bot_pad) / 2 + 1;
  GRect fixed_bbox = GRect(bbox.origin.x, bbox.origin.y - shift_up, bbox.size.w, bbox.size.h);
  graphics_draw_text(ctx, buffer, font, fixed_bbox, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}