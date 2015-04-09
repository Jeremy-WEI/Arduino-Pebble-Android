#include <pebble.h>

#define NUM_MENU_SECTIONS 3
#define NUM_MENU_ICONS 3
#define NUM_FIRST_MENU_ITEMS 4
#define NUM_SECOND_MENU_ITEMS 2
#define NUM_THIRD_MENU_ITEMS 2

static Window *window;
static TextLayer *text_layer;
static char msg[100];
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
    }
}



void out_sent_handler(DictionaryIterator *sent, void *context) {
    // outgoing message was delivered -- do nothing
    text_layer_set_text(text_layer, "Waiting...");
}


void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    // outgoing message failed
    APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %i - %s", reason, translate_error(reason));
    text_layer_set_text(text_layer, "Message Not Send Successfully!");
}


void in_received_handler(DictionaryIterator *received, void *context) {
    //    incoming message received
    //    looks for key #0 in the incoming message
    int key = 0;
    Tuple *text_tuple = dict_find(received, key);
    if (text_tuple) {
        if (text_tuple->value) {
            // put it in this global variable
            strcpy(msg, text_tuple->value->cstring);
        }
        else strcpy(msg, "Not Getting Response from Server :(");
        text_layer_set_text(text_layer, msg);
    }
    else {
        text_layer_set_text(text_layer, "Not Getting Response from Server :(");
    }
}

void in_dropped_handler(AppMessageResult reason, void *context) {
    //    incoming message dropped
    text_layer_set_text(text_layer, "Error Handling Incoming Message!");
}

void send_outbox(char *param) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    int key = 0;
    Tuplet value = TupletCString(key, param);
    dict_write_tuplet(iter, &value);
    app_message_outbox_send();
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    // Use the row to specify which item will receive the select action
    switch (cell_index->section) {
        case 0:
            // Use the row to specify which item we'll draw
            switch (cell_index->row) {
                case 0:
                    send_outbox("curTemp");
                    text_layer_set_text(text_layer, "Query Current Temperature.");
                    break;
                case 1:
                    send_outbox("avgTemp");
                    text_layer_set_text(text_layer, "Query Average Temperature in Last Hour.");
                    break;
                case 2:
                    send_outbox("highTemp");
                    text_layer_set_text(text_layer, "Query Highest Temperature in Last Hour.");
                    break;
                case 3:
                    send_outbox("lowTemp");
                    text_layer_set_text(text_layer, "Query Lowest Temperature in Last Hour.");
                    break;
            }
            break;
        case 1:
            switch (cell_index->row) {
                case 0:
                    send_outbox("showC");
                    text_layer_set_text(text_layer, "Show Temperature in Celsius.");
                    break;
                case 1:
                    send_outbox("showF");
                    text_layer_set_text(text_layer, "Show Temperature in Fahrenheit.");
                    break;
            }
            break;
        case 2:
            switch (cell_index->row) {
                case 0:
                    send_outbox("stop");
                    text_layer_set_text(text_layer, "Arduino Stany-by Mode.");
                    break;
                case 1:
                    send_outbox("resume");
                    text_layer_set_text(text_layer, "Arduino Active Mode.");
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
    
    text_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { width , height * 1 / 3  } });
    text_layer_set_text(text_layer, "Welcome To PebbleDream!");
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));
    // Here we load the bitmap assets
    s_menu_icons[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_BIG_WATCH);
    s_menu_icons[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_SECTOR_WATCH);
    s_menu_icons[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_BINARY_WATCH);
    
    s_menu_layer = menu_layer_create((GRect) { .origin = { 0, height * 1 / 3}, .size = { width , height * 2 / 3  } });
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
    text_layer_destroy(text_layer);
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

