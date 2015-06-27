/**
 * Example implementation of the checkbox list UI pattern.
 */

#include <pebble.h>

#define RADIO_BUTTON_WINDOW_NUM_ROWS     4
#define RADIO_BUTTON_WINDOW_CELL_HEIGHT  30
#define RADIO_BUTTON_WINDOW_RADIO_RADIUS 6
  
static Window *s_main_window;
static MenuLayer *s_menu_layer;

static int s_current_selection = 0;
static time_t time_at_last_change = 0; 

static int time_zero_selected_minutes = 0;
static int time_one_selected_minutes = 0;
static int time_two_selected_minutes = 0;
static int time_three_selected_minutes = 0;

// This is a custom defined key for saving our count field
#define TIME_ZERO_SELECTED_KEY 1
#define TIME_ONE_SELECTED_KEY 2
#define TIME_TWO_SELECTED_KEY 3
#define TIME_THREE_SELECTED_KEY 4
  
// You can define defaults for values in persistent storage
#define TIME_ZERO_SELECTED_DEFAULT 0
#define TIME_ONE_SELECTED_DEFAULT 0
#define TIME_TWO_SELECTED_DEFAULT 0
#define TIME_THREE_SELECTED_DEFAULT 0

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return RADIO_BUTTON_WINDOW_NUM_ROWS + 1;
}

static void draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *context) {
  if(cell_index->row == RADIO_BUTTON_WINDOW_NUM_ROWS) {
    // This is the submit item
    menu_cell_basic_draw(ctx, cell_layer, "Submit", NULL, NULL);
  } else {
    // This is a choice item
    static char s_buff[16];
    snprintf(s_buff, sizeof(s_buff), "Choice %d", (int)cell_index->row);
    menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);

    GRect bounds = layer_get_bounds(cell_layer);
    GPoint p = GPoint(bounds.size.w - (3 * RADIO_BUTTON_WINDOW_RADIO_RADIUS), (bounds.size.h / 2));

    // Selected?
    if(menu_cell_layer_is_highlighted(cell_layer)) {
      graphics_context_set_stroke_color(ctx, GColorWhite);
      graphics_context_set_fill_color(ctx, GColorWhite);
    } else {
      graphics_context_set_fill_color(ctx, GColorBlack);
    }

    // Draw radio filled/empty
    graphics_draw_circle(ctx, p, RADIO_BUTTON_WINDOW_RADIO_RADIUS);
    if(cell_index->row == s_current_selection) {
      // This is the selection
      graphics_fill_circle(ctx, p, RADIO_BUTTON_WINDOW_RADIO_RADIUS - 3);
    }
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  return RADIO_BUTTON_WINDOW_CELL_HEIGHT;
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  if(cell_index->row == RADIO_BUTTON_WINDOW_NUM_ROWS) {
    // Do something with user choice
    APP_LOG(APP_LOG_LEVEL_INFO, "Submitted choice %d", s_current_selection);
    APP_LOG(APP_LOG_LEVEL_INFO, "Minutes spent : %d, %d, %d, %d", time_zero_selected_minutes, time_one_selected_minutes, time_two_selected_minutes, time_three_selected_minutes);
  } else {
    // Change selection
    int time_spent = (time(NULL) - time_at_last_change)/60;
    time_at_last_change = time(NULL);
    switch (s_current_selection) {
      case TIME_ZERO_SELECTED_KEY:
        time_zero_selected_minutes += time_spent;
        break;
      case TIME_ONE_SELECTED_KEY:
        time_one_selected_minutes += time_spent;
        break;
      case TIME_TWO_SELECTED_KEY:
        time_two_selected_minutes += time_spent;
        break;
      case TIME_THREE_SELECTED_KEY:
        time_three_selected_minutes += time_spent;
        break;
    }
    s_current_selection = cell_index->row;
    menu_layer_reload_data(menu_layer);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback)get_num_rows_callback,
      .draw_row = (MenuLayerDrawRowCallback)draw_row_callback,
      .get_cell_height = (MenuLayerGetCellHeightCallback)get_cell_height_callback,
      .select_click = (MenuLayerSelectCallback)select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);

  window_destroy(window);
  s_main_window = NULL;
}

void radio_button_window_push() {
  if(!s_main_window) {
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(s_main_window, true);
}

static void init() {

  // Get the count from persistent storage for use if it exists, otherwise use the default
  time_zero_selected_minutes = persist_exists(TIME_ZERO_SELECTED_KEY) ? persist_read_int(TIME_ZERO_SELECTED_KEY) : TIME_ZERO_SELECTED_DEFAULT;
  time_one_selected_minutes = persist_exists(TIME_ONE_SELECTED_KEY) ? persist_read_int(TIME_ONE_SELECTED_KEY) : TIME_ONE_SELECTED_DEFAULT;
  time_two_selected_minutes = persist_exists(TIME_TWO_SELECTED_KEY) ? persist_read_int(TIME_TWO_SELECTED_KEY) : TIME_TWO_SELECTED_DEFAULT;
  time_three_selected_minutes = persist_exists(TIME_THREE_SELECTED_KEY) ? persist_read_int(TIME_THREE_SELECTED_KEY) : TIME_THREE_SELECTED_DEFAULT;
  time_at_last_change = time(NULL);
  APP_LOG(APP_LOG_LEVEL_INFO, "Time %ld", time_at_last_change);  
}

static void deinit() {
  // Save the count into persistent storage on app exit
  persist_write_int(TIME_ZERO_SELECTED_KEY, time_zero_selected_minutes);
  persist_write_int(TIME_ONE_SELECTED_KEY, time_one_selected_minutes);
  persist_write_int(TIME_TWO_SELECTED_KEY, time_two_selected_minutes);
  persist_write_int(TIME_THREE_SELECTED_KEY, time_three_selected_minutes);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}