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
  if (a < b) {
    return a;
  }
  return b;
}

static void fast_forward_time(struct tm* now) {
  now->tm_hour = 12;
  now->tm_min = now->tm_sec;
}

static int get_hour(struct tm* now, bool force_12h) {
  int hour = now->tm_hour;
  if (force_12h || !clock_is_24h_style()) {
    hour = now->tm_hour % 12;
    if (hour == 0) {
      hour = 12;
    }
  }
  return hour;
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