// Modest
// Made by Ben Chapman-Kish on 2015-05-05
#include "pebble.h"
// Features to add: animations when the time or date change

static Window *s_main_window;
static Layer *s_date_layer, *s_time_layer;
static GFont s_font_small, s_font_large;
static GColor foreground_color, text_color;

static char s_time_text[] = "12:00";
static char s_date_text[] = "Wednesday,\nSeptember 10";

static void date_layer_draw(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_frame(layer);
	
	graphics_context_set_fill_color(ctx, foreground_color);
	graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 0, GCornerNone);
	
	graphics_context_set_text_color(ctx, text_color);
	graphics_draw_text(ctx, s_date_text, s_font_small, GRect(4, 3, bounds.size.w-4, bounds.size.h-3), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void time_layer_draw(Layer *layer, GContext *ctx) {
	GRect bounds = layer_get_frame(layer);
	
	graphics_context_set_fill_color(ctx, foreground_color);
	graphics_fill_rect(ctx, GRect(0, 0, bounds.size.w, bounds.size.h), 0, GCornerNone);
	
	graphics_context_set_text_color(ctx, text_color);
	graphics_draw_text(ctx, s_time_text, s_font_large, GRect(0, -4, bounds.size.w-4, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	// Date is currently updated every minute, should be changed later.
  strftime(s_date_text, sizeof(s_date_text), "%A,\n%B %e", tick_time);
	layer_mark_dirty(s_date_layer);
	
	char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }
  strftime(s_time_text, sizeof(s_time_text), time_format, tick_time);

  // Handle lack of non-padded hour format string for twelve hour clock.
  if (!clock_is_24h_style() && (s_time_text[0] == '0')) {
    memmove(s_time_text, &s_time_text[1], sizeof(s_time_text) - 1);
  }
	
	layer_mark_dirty(s_time_layer);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
	
	foreground_color = COLOR_FALLBACK(GColorVividCerulean, GColorWhite);
	text_color = COLOR_FALLBACK(GColorWhite, GColorBlack);
 	s_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONTSERRAT_17));
	s_font_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONTSERRAT_41));
	
  s_date_layer = layer_create((GRect) { .origin = {0, 26}, .size = {126, 45}});
	layer_set_update_proc(s_date_layer, date_layer_draw);
  layer_add_child(window_layer, s_date_layer);

  s_time_layer = layer_create((GRect) { .origin = {18, 97}, .size = {126, 45}});
	layer_set_update_proc(s_time_layer, time_layer_draw);
  layer_add_child(window_layer, s_time_layer);
}

static void main_window_unload(Window *window) {
	layer_destroy(s_date_layer);
  layer_destroy(s_time_layer);
}

static void init() {
  s_main_window = window_create();
  #ifdef PBL_COLOR
		window_set_background_color(s_main_window, GColorIslamicGreen);
	#else
  	window_set_background_color(s_main_window, GColorBlack);
	#endif
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  
  // Prevent starting blank
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  handle_minute_tick(t, MINUTE_UNIT);
}

static void deinit() {
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}