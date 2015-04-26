#include <pebble.h>
#include <string.h>
#include <stdlib.h> 

#define NUM_MENU_SECTIONS 4
#define NUM_MENU_ICONS 3
#define NUM_FIRST_MENU_ITEMS 5
#define NUM_SECOND_MENU_ITEMS 2
#define NUM_THIRD_MENU_ITEMS 2
#define NUM_FOURTH_MENU_ITEMS 2

static Window *window;
static Layer *canvas_layer;
static char msg[100];
static double numbers[10];
static MenuLayer *s_menu_layer;
static GBitmap *s_menu_icons[NUM_MENU_ICONS];

char *translate_error(AppMessageResult result) {
    switch (result) {
        case APP_MSG_OK: return "APP_MSG_OK";
        case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
        case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
        case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
        case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
        case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
        case APP_MSG_BUSY: return "APP_MSG_BUSY";
        case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
        case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
        case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
        case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
        case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
        case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
        case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
        default: return "UNKNOWN ERROR";
    }
}

static void layer_text_update_callback(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int height = bounds.size.h - 10;
  int width = bounds.size.w - 10;
  graphics_draw_line(ctx, GPoint(2, 2), GPoint(2, height +8));
  graphics_draw_line(ctx, GPoint(2, 2), GPoint(width + 8, 2));
  graphics_draw_line(ctx, GPoint(2, height + 8), GPoint(width + 8, height + 8));
  graphics_draw_line(ctx, GPoint(width + 8, 2), GPoint(width + 8, height + 8));
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, 
    msg,
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
    bounds,
    GTextOverflowModeWordWrap,
    GTextAlignmentCenter,
    NULL
  );
}

static double myatof(char *number) {
  double res = 0;
  char num[5];
  int j = 0;
  int len = strlen(number);
  for (int i = 0; i < len;i++) {
    if (number[i] != '.')
      num[j++] = number[i];
    else {
      num[j] = '\0';
      res = atoi(num);
      j = 0;
    }
  }
  num[j] = '\0';
  res += atoi(num) / (double) 100;
  return res;
}

static void layer_graph_update_callback(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int height = bounds.size.h - 10;
  int width = bounds.size.w - 10;
  graphics_draw_line(ctx, GPoint(2, 2), GPoint(2, height +8));
  graphics_draw_line(ctx, GPoint(2, 2), GPoint(width + 8, 2));
  graphics_draw_line(ctx, GPoint(2, height + 8), GPoint(width + 8, height + 8));
  graphics_draw_line(ctx, GPoint(width + 8, 2), GPoint(width + 8, height + 8));
//   char buf[] = "60.21,67.21,25.21,90.21,61.21,44.21,50.21,61.21,30.21,70.00,";
  char number[10];
  int len = strlen(msg);
  for (int i = 0, k = 0, j = 0; i < len; i++) {
    if (msg[i] != ',')
      number[k++] = msg[i];
    else {
      number[k] = '\0';
      numbers[j++] = myatof(number);
      k = 0;
    }
  }
  double min = 1000, max = -1000;
  for (int i = 0; i < 10; i++) {
    if (numbers[i] < min) min = numbers[i];
    if (numbers[i] > max) max = numbers[i];
  }
  double range = max - min;
  GPoint points[10];
  for (int i = 0; i < 10; i++) {
    points[i] = GPoint(7 + width / 9 * i, 5 + (max - numbers[i]) / range * height);
    graphics_fill_circle(ctx, points[i], 3);
    if (i > 0) {
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx, points[i - 1], points[i]);
    }
  }
  graphics_context_set_stroke_color(ctx, GColorBlack);
}

void send_outbox(char *param) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    int key = 0;
    Tuplet value = TupletCString(key, param);
    dict_write_tuplet(iter, &value);
    app_message_outbox_send();
}

static void send_message(char *msg1, char *msg2) {
  send_outbox(msg1);
  strcpy(msg, msg2);
  layer_set_update_proc(canvas_layer, layer_text_update_callback);
  layer_mark_dirty(canvas_layer);
}

static void show_message(char *message) {
  strcpy(msg, message);
  layer_set_update_proc(canvas_layer, layer_text_update_callback);
  layer_mark_dirty(canvas_layer);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
    return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    switch (section_index) {
        case 0:
            return NUM_FIRST_MENU_ITEMS;
        case 1:
            return NUM_SECOND_MENU_ITEMS;
        case 2:
            return NUM_THIRD_MENU_ITEMS;
        case 3:
            return NUM_FOURTH_MENU_ITEMS;
        default:
            return 0;
    }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
    // Determine which section we're working with
    switch (section_index) {
        case 0:
            // Draw title text in the section header
            menu_cell_basic_header_draw(ctx, cell_layer, "Temperature Query");
            break;
        case 1:
            menu_cell_basic_header_draw(ctx, cell_layer, "°C/°F Convert");
            break;
        case 2:
            menu_cell_basic_header_draw(ctx, cell_layer, "Arduino Control");
        case 3:
            menu_cell_basic_header_draw(ctx, cell_layer, "Additional Features");
            break;
    }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    // Determine which section we're going to draw in
    switch (cell_index->section) {
        case 0:
            // Use the row to specify which item we'll draw
            switch (cell_index->row) {
                case 0:
                // There is title draw for something more simple than a basic menu item
                    menu_cell_basic_draw(ctx, cell_layer, "Current Temp", "query...", s_menu_icons[2]);
                    break;
                case 1:
                    menu_cell_basic_draw(ctx, cell_layer, "Average Temp", "query...", s_menu_icons[2]);
                    break;
                case 2:
                    menu_cell_basic_draw(ctx, cell_layer, "Highest Temp", "query...", s_menu_icons[2]);
                    break;
                case 3:
                    menu_cell_basic_draw(ctx, cell_layer, "Lowest Temp", "query...", s_menu_icons[2]);
                    break;
                case 4:
                    menu_cell_basic_draw(ctx, cell_layer, "Temp History", "In Last Minute...", s_menu_icons[2]);
                    break;
            }
            break;
        case 1:
            switch (cell_index->row) {
                case 0:
                    menu_cell_basic_draw(ctx, cell_layer, "Celsius", "convert...", s_menu_icons[2]);
                    break;
                case 1:
                    menu_cell_basic_draw(ctx, cell_layer, "Fahrenheit", "convert...", s_menu_icons[2]);
                    break;
            }
            break;
        case 2:
            switch (cell_index->row) {
                case 0:
                    menu_cell_basic_draw(ctx, cell_layer, "StandBy Mode", "control...", s_menu_icons[2]);
                    break;
                case 1:
                    menu_cell_basic_draw(ctx, cell_layer, "Active Mode", "control...", s_menu_icons[2]);
                    break;
            }
            break;
        case 3:
            switch (cell_index->row) {
                case 0:
                    menu_cell_basic_draw(ctx, cell_layer, "Weather Report", "connect...", s_menu_icons[2]);
                    break;
                case 1:
                    menu_cell_basic_draw(ctx, cell_layer, "Stock Price", "look up...", s_menu_icons[2]);
                    break;
            }
            break;
    }
}



void out_sent_handler(DictionaryIterator *sent, void *context) {
    // outgoing message was delivered -- do nothing
    show_message("Waiting...");
}


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    // outgoing message failed
    APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %i - %s", reason, translate_error(reason));
    show_message("Ooops! Message not sent successfully.");
}


void in_received_handler(DictionaryIterator *received, void *context) {
    //    incoming message received
    //    looks for key #0 in the incoming message
    int key = 0;
    Tuple *text_tuple = dict_find(received, key);
    if (text_tuple) {
        if (text_tuple->value) {
            // put it in this global variable
            char *message = text_tuple->value->cstring;
            if (message[strlen(message) - 1] == ',') {
              strcpy(msg, message);
              layer_set_update_proc(canvas_layer, layer_graph_update_callback);
              layer_mark_dirty(canvas_layer);
            }
            else {
              show_message(message);
            }
        }
        else  
          show_message("Not Getting Response from Server :(");
    }
    else {
        show_message("Not Getting Response from Server :(");
    }
    vibes_short_pulse();
}

void in_dropped_handler(AppMessageResult reason, void *context) {
    //    incoming message dropped
    show_message("Error Handling Incoming Message!");
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    // Use the row to specify which item will receive the select action
    switch (cell_index->section) {
        case 0:
            // Use the row to specify which item we'll draw
            switch (cell_index->row) {
                case 0:
                    send_message("curTemp", "Query Current Temperature.");
                    break;
                case 1:
                    send_message("avgTemp", "Query Average Temperature in Last Hour.");
                    break;
                case 2:
                    send_message("highTemp", "Query Highest Temperature in Last Hour.");
                    break;
                case 3:
                    send_message("lowTemp", "Query Lowest Temperature in Last Hour.");
                    break;
                case 4:
                    send_message("temps", "Lookup Temps in Last Minute.");
                    break;
            }
            break;
        case 1:
            switch (cell_index->row) {
                case 0:
                    send_message("showC", "Show Temperature in Celsius.");
                    break;
                case 1:
                    send_message("showF", "Show Temperature in Fahrenheit.");
                    break;
            }
            break;
        case 2:
            switch (cell_index->row) {
                case 0:
                    send_message("stop", "Arduino Stany-by Mode.");
                    break;
                case 1:
                    send_message("resume", "Arduino Active Mode.");
                    break;
            }
            break;
         case 3:
            switch (cell_index->row) {
                case 0:
                    send_message("weather", "Query Weather in Philly.");
                    break;
                case 1:
                    send_message("stock", "Query Yahoo! Stock Price.");
                    break;
            }
            break;
    }
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    //   APP_LOG(APP_LOG_LEVEL_DEBUG, "%d %d", bounds.size.w, bounds.size.h);
    int width = bounds.size.w;
    int height = bounds.size.h;
    
    canvas_layer = layer_create((GRect) { .origin = { 0, 0 }, .size = { width, height * 2 / 5  } });
    layer_add_child(window_layer, canvas_layer);
    layer_set_update_proc(canvas_layer, layer_graph_update_callback);
    layer_mark_dirty(canvas_layer);
  
    show_message("Welcome to PebbleDream!");
  
    // Here we load the bitmap assets
    s_menu_icons[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_BIG_WATCH);
    s_menu_icons[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_SECTOR_WATCH);
    s_menu_icons[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_BINARY_WATCH);
    
    s_menu_layer = menu_layer_create((GRect) { .origin = { 0, height * 2 / 5}, .size = { width , height * 3 / 5  } });
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
    });
    // Bind the menu layer's click config provider to the window for interactivity
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {
    layer_destroy(canvas_layer);
    menu_layer_destroy(s_menu_layer);
    // Cleanup the menu icons
    for (int i = 0; i < NUM_MENU_ICONS; i++) {
        gbitmap_destroy(s_menu_icons[i]);
    }
}

static void init(void) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });

    // for registering AppMessage handlers
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_register_outbox_sent(out_sent_handler);
    app_message_register_outbox_failed(out_failed_handler);
    
    const uint32_t inbound_size = 100;
    const uint32_t outbound_size = 100;
    app_message_open(inbound_size, outbound_size);
    const bool animated = true;
    window_stack_push(window, animated);
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}

