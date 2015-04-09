#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static char msg[100];

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

/* This is called when the up button is clicked */
void up_button_click_handler(ClickRecognizerRef recognizer, void *context) {
   send_outbox("curTemp");
   text_layer_set_text(text_layer, "Query Current Temperature.");
}

/* This is called when the down button is clicked */
void down_button_click_handler(ClickRecognizerRef recognizer, void *context) {
   send_outbox("avgTemp");
   text_layer_set_text(text_layer, "Query Average Temperature in Last Hour.");
}

/* This is called when the select button is clicked */
void select_button_click_handler(ClickRecognizerRef recognizer, void *context) {
   send_outbox("showC");
   text_layer_set_text(text_layer, "Show Temperature in Celsius.");
}

/* This is called when the up button is double-clicked */
void up_double_click_handler(ClickRecognizerRef recognizer, void *context) {
   send_outbox("highTemp");
   text_layer_set_text(text_layer, "Query Highest Temperature in Last Hour.");
}

/* This is called when the down button is double-clicked */
void down_double_click_handler(ClickRecognizerRef recognizer, void *context) {
   send_outbox("lowTemp");
   text_layer_set_text(text_layer, "Query Lowest Temperature in Last Hour.");
}

/* This is called when the select button is double-clicked */
void select_double_click_handler(ClickRecognizerRef recognizer, void *context) {
   send_outbox("showF");
   text_layer_set_text(text_layer, "Show Temperature in Fahrenheit.");
}

/* This is called when the up button is double-clicked */
void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
   send_outbox("stop");
   text_layer_set_text(text_layer, "Arduino Stany-by Mode.");
}

/* This is called when the down button is double-clicked */
void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
   send_outbox("resume");
   text_layer_set_text(text_layer, "Arduino Active Mode.");
}

/* this registers the appropriate function to the appropriate button */
void config_provider(void *context) {
   window_single_click_subscribe(BUTTON_ID_UP, up_button_click_handler);
   window_single_click_subscribe(BUTTON_ID_DOWN, down_button_click_handler);
   window_single_click_subscribe(BUTTON_ID_SELECT, select_button_click_handler);
   window_multi_click_subscribe(BUTTON_ID_UP, 2, 0, 0, false, up_double_click_handler);
   window_multi_click_subscribe(BUTTON_ID_DOWN, 2, 0, 0, false, down_double_click_handler);
   window_multi_click_subscribe(BUTTON_ID_SELECT, 2, 0, 0, false, select_double_click_handler);
   window_long_click_subscribe(BUTTON_ID_UP, 1000, NULL, up_long_click_handler);
   window_long_click_subscribe(BUTTON_ID_DOWN, 1000, NULL, down_long_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

//   APP_LOG(APP_LOG_LEVEL_DEBUG, "%d %d", bounds.size.w, bounds.size.h);
  int width = bounds.size.w;
  int height = bounds.size.h;
  
  text_layer = text_layer_create((GRect) { .origin = { width / 5, height / 3 }, .size = { width * 3 / 5 , height * 2/ 3  } });
  text_layer_set_text(text_layer, "Welcome To PebbleDream!");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  // need this for adding the listener
  window_set_click_config_provider(window, config_provider);
  
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