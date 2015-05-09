// Modest
// Made by Ben Chapman-Kish from 2015-05-05 to 2015-05-08
#include "pebble.h"
#include "math.h"
// Features to add: customizeable colours, optional bluetooth vibration

#define M_PI 3.14159265358979323846
#define ANIM_TIMER_MS 20
#define ANIM_FRAMES_H 20

static Window *s_main_window;
static Layer *s_date_layer, *s_time_layer;
static GFont s_font_small, s_font_large;
static GColor background_color, foreground_color, text_color;
static int time_x_i, time_x_offset, date_x_i, date_x_offset;
static int first_run;

static char s_time_text[] = "12:00";
static char s_cur_time[] = "12:00";
static char s_date_text[] = "Wednesday,\nSeptember 21";
static char s_cur_date[] = "Wednesday,\nSeptember 21";

#ifdef PBL_PLATFORM_APLITE
	static GColor bwcolors[2];
	static bool aplite_inverted = false;
#endif

static void timer_date_draw(void *data) {
	--date_x_i;
	if (date_x_i <= -ANIM_FRAMES_H) {
		strcpy(s_date_text, s_cur_date);
		date_x_i = ANIM_FRAMES_H - 1;
	}
	
	date_x_offset = (int)((1 - cos((M_PI * date_x_i) / (ANIM_FRAMES_H * 2))) * 144);
	if (date_x_i < 0) {
		date_x_offset *= -1;
	}
	
	layer_mark_dirty(s_date_layer);
	if (date_x_i != 0) {
		app_timer_register(ANIM_TIMER_MS, timer_date_draw, NULL);
	}
}

static void timer_time_draw(void *data) {
	--time_x_i;
	if (time_x_i <= -ANIM_FRAMES_H) {
		strcpy(s_time_text, s_cur_time);
		time_x_i = ANIM_FRAMES_H - 1;
	}
	
	time_x_offset = (int)((1 - cos((M_PI * time_x_i) / (ANIM_FRAMES_H * 2))) * 144);
	if (time_x_i < 0) {
		time_x_offset *= -1;
	}
	
	layer_mark_dirty(s_time_layer);
	if (time_x_i != 0) {
		app_timer_register(ANIM_TIMER_MS, timer_time_draw, NULL);
	}
}

static void date_layer_draw(Layer *layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, foreground_color);
	graphics_fill_rect(ctx, GRect(0 - date_x_offset, 0, 144-16, 45), 0, GCornerNone);
	graphics_context_set_text_color(ctx, text_color);
	graphics_draw_text(ctx, s_date_text, s_font_small, GRect(4 - date_x_offset, 3, 144-16-4, 45-3), 
										 GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}

static void time_layer_draw(Layer *layer, GContext *ctx) {
	graphics_context_set_fill_color(ctx, foreground_color);
	graphics_fill_rect(ctx, GRect(16 + time_x_offset, 0, 144-16, 45), 0, GCornerNone);
	graphics_context_set_text_color(ctx, text_color);
	graphics_draw_text(ctx, s_time_text, s_font_large, GRect(16 + time_x_offset, -4, 144-16-4, 45+4), 
										 GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	strftime(s_cur_date, sizeof(s_cur_date), "%A,\n%B %e", tick_time);
	if (first_run > 0) {
		//strcpy(s_date_text, s_cur_date);
		layer_mark_dirty(s_date_layer);
	} else if (strcmp(s_cur_date, s_date_text) != 0) {
		date_x_i = -1;
		date_x_offset = (int)((1 - cos((M_PI * date_x_i) / (ANIM_FRAMES_H * 2))) * -144);
		layer_mark_dirty(s_date_layer);
		app_timer_register(ANIM_TIMER_MS, timer_date_draw, NULL);
	}
	
	char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }
  strftime(s_cur_time, sizeof(s_cur_time), time_format, tick_time);

  // Handle lack of non-padded hour format string for twelve hour clock.
  if (!clock_is_24h_style() && (s_cur_time[0] == '0')) {
    memmove(s_cur_time, &s_cur_time[1], sizeof(s_cur_time) - 1);
  }
	
	if (first_run > 0) {
		--first_run;
		//strcpy(s_time_text, s_cur_time);
		layer_mark_dirty(s_time_layer);
	} else {
		time_x_i = -1;
		time_x_offset = (int)((1 - cos((M_PI * time_x_i) / (ANIM_FRAMES_H * 2))) * -144);
		layer_mark_dirty(s_time_layer);
		app_timer_register(ANIM_TIMER_MS, timer_time_draw, NULL);
	}
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
	
 	s_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONTSERRAT_17));
	s_font_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONTSERRAT_41));
	
  s_date_layer = layer_create((GRect) { .origin = {0, 26}, .size = {144, 45}});
	layer_set_update_proc(s_date_layer, date_layer_draw);
  layer_add_child(window_layer, s_date_layer);

  s_time_layer = layer_create((GRect) { .origin = {0, 97}, .size = {144, 45}});
	layer_set_update_proc(s_time_layer, time_layer_draw);
  layer_add_child(window_layer, s_time_layer);
}

static void main_window_unload(Window *window) {
	layer_destroy(s_date_layer);
  layer_destroy(s_time_layer);
}

static void init() {
  s_main_window = window_create();
	
	#ifdef PBL_PLATFORM_APLITE
		if (aplite_inverted) {
			bwcolors[0] = GColorWhite;
			bwcolors[1] = GColorBlack;
		} else {
			bwcolors[0] = GColorBlack;
			bwcolors[1] = GColorWhite;
		}
	#endif
	
	background_color = COLOR_FALLBACK(GColorDarkGreen, bwcolors[0]);
	foreground_color = COLOR_FALLBACK(GColorVividCerulean, bwcolors[1]);
	text_color = COLOR_FALLBACK(GColorWhite, bwcolors[0]);
	
	window_set_background_color(s_main_window, background_color);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
 
	time_x_i = time_x_offset = date_x_i = date_x_offset = 0;
	first_run = 2;
	
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