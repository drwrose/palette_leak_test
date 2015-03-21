#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static BitmapLayer *bitmap_layer;
static GBitmap *bitmap;

static void update_mem_display() {
   static char buffer[512];
  snprintf(buffer, 512, "%u", heap_bytes_used());
  text_layer_set_text(text_layer, buffer);
}

static void flip_palette() {
#define PALETTE_SIZE 16
  GColor *orig_palette = gbitmap_get_palette(bitmap);
  GColor *new_palette = (GColor *)malloc(PALETTE_SIZE * sizeof(GColor));

  for (int i = 0; i < PALETTE_SIZE; ++i) {
    new_palette[i].argb = orig_palette[i].argb ^ 0x3f;
  }

  gbitmap_set_palette(bitmap, new_palette, true);
  APP_LOG(APP_LOG_LEVEL_INFO, "New palette, mem used: %u", heap_bytes_used());
  update_mem_display();
}


static void reload_bitmap() {
  if (bitmap != NULL) {
    gbitmap_destroy(bitmap);
  }
  bitmap = gbitmap_create_with_resource(RESOURCE_ID_BM16);
  bitmap_layer_set_bitmap(bitmap_layer, bitmap);

  APP_LOG(APP_LOG_LEVEL_INFO, "Reloaded, mem used: %u", heap_bytes_used());
  update_mem_display();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  flip_palette();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  reload_bitmap();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  bitmap_layer = bitmap_layer_create((GRect) { .origin = { 40, 22 }, .size = { 64, 64 } });
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));

  text_layer = text_layer_create((GRect) { .origin = { 0, 128 }, .size = { bounds.size.w, 20 } });

  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  reload_bitmap();
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  bitmap_layer_destroy(bitmap_layer);
  text_layer_destroy(text_layer);
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
