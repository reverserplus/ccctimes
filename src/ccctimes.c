#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"

//OS Specific UUID Generation (see httpebble documentation)
/* If compiling this for iOS, set ANDROID to be false. */
#define ANDROID false

#if ANDROID
#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x10, 0x34, 0xBF, 0xBE, 0x12, 0x98 }
#else
#define MY_UUID HTTP_UUID
#endif

//Cookie for HTTP Request Coordination
#define HTTP_COOKIE 4887
//Cookie for Timer Coordination
#define COOKIE_MY_TIMER 1

//Application Definition	
PBL_APP_INFO(MY_UUID, "CCC Times", "reverser+", 1, 0,  DEFAULT_MENU_ICON, APP_INFO_STANDARD_APP);

//Function Declarations
void handle_init(AppContextRef ctx);
void http_success(int32_t request_id, int http_status, DictionaryIterator* received, void* context);
void http_failure(int32_t request_id, int http_status, void* context);
void window_appear(Window* me);
void httpebble_error(int error_code);

//Global Variable Declations
Window window;
TextLayer balt_time_text;
TextLayer how_time_text;
TextLayer balt_label_text;
TextLayer how_label_text;
AppTimerHandle timer_handle;

//Function to update JSON dict result set, and use get/send call to trigger http_success or http_failure
void ccc_http_refresh(){
    DictionaryIterator* dict;
  	HTTPResult result = http_out_get("http://www.snbl-cpc.com/ccctime.php", HTTP_COOKIE, &dict);
  	if (result != HTTP_OK) {
    	httpebble_error(result);
    return;
  }
  
//JSON dictionary out write / send
  dict_write_cstring(dict, 1, "Hello!");
  result = http_out_send();
  if (result != HTTP_OK) {
    httpebble_error(result);
    return;	
  }
}

//Click handler to refresh and update result text layers with the most current web scrape (select button)
void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	ccc_http_refresh();
}

//Timer handler
void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {

  if (cookie == COOKIE_MY_TIMER) {
	ccc_http_refresh();
  }
	// Call timer handler again for continuous refresh
	timer_handle = app_timer_send_event(ctx, 30000 /* milliseconds */, COOKIE_MY_TIMER);
}

//Click config provider (needs more detail)
void click_config_provider(ClickConfig **config, Window *window) {
  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;
}

//Main application function
void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init, //register init handler
	.timer_handler = &handle_timer, //register timer handler
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 124,
        .outbound = 256,
      }
    }
  };
 
  app_event_loop(params, &handlers);
}

//http_out_send() SUCCESS handler
void http_success(int32_t request_id, int http_status, DictionaryIterator* received, void* context) {
  if (request_id != HTTP_COOKIE) {
    return;
  }

//Accessing the data from the returned tuple and write the text layers with the result data
  Tuple* tuple1 = dict_find(received, 0);
  text_layer_set_text(&balt_time_text, tuple1->value->cstring);
 
  Tuple* tuple2 = dict_find(received, 1);
  text_layer_set_text(&how_time_text, tuple2->value->cstring);
}

//HTTP error handling
void http_failure(int32_t request_id, int http_status, void* context) {
  httpebble_error(http_status >= 1000 ? http_status - 1000 : http_status);
}

//Window appearance handler
void window_appear(Window* me) {
	ccc_http_refresh();
}
 
void handle_init(AppContextRef ctx) {
  http_set_app_id(49862702);
 
//HTTP Callback Registration (see httpebble docs)
  http_register_callbacks((HTTPCallbacks) {
    .success = http_success,
    .failure = http_failure
  }, NULL);
 
  window_init(&window, "CCC Times");
  window_stack_push(&window, true);
  window_set_window_handlers(&window, (WindowHandlers){
    .appear  = window_appear //set window appear handler
  });
 
  text_layer_init(&balt_label_text, GRect(0, 10, 144, 30));
  text_layer_set_text_color(&balt_label_text, GColorBlack);
  text_layer_set_background_color(&balt_label_text, GColorClear);
  text_layer_set_font(&balt_label_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(&balt_label_text, GTextAlignmentCenter);
  layer_add_child(&window.layer, &balt_label_text.layer);
	
  text_layer_init(&balt_time_text, GRect(0, 30, 144, 30));
  text_layer_set_text_color(&balt_time_text, GColorBlack);
  text_layer_set_background_color(&balt_time_text, GColorClear);
  text_layer_set_font(&balt_time_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(&balt_time_text, GTextAlignmentCenter);
  layer_add_child(&window.layer, &balt_time_text.layer);
 
  text_layer_init(&how_label_text, GRect(0, 80, 144, 30));
  text_layer_set_text_color(&how_label_text, GColorBlack);
  text_layer_set_background_color(&how_label_text, GColorClear);
  text_layer_set_font(&how_label_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(&how_label_text, GTextAlignmentCenter);
  layer_add_child(&window.layer, &how_label_text.layer);
	
  text_layer_init(&how_time_text, GRect(0, 100, 144, 30));
  text_layer_set_text_color(&how_time_text, GColorBlack);
  text_layer_set_background_color(&how_time_text, GColorClear);
  text_layer_set_font(&how_time_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(&how_time_text, GTextAlignmentCenter);
  layer_add_child(&window.layer, &how_time_text.layer);

  text_layer_set_text(&balt_label_text, "Baltimore St.");
  text_layer_set_text(&how_label_text, "Howard St.");	
  
  //Start timer on program init
  timer_handle = app_timer_send_event(ctx, 20000 /* milliseconds */, COOKIE_MY_TIMER);
  //Set click config handler
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
}
 
void httpebble_error(int error_code) {
 
  static char error_message[] = "UNKNOWN_HTTP_ERRROR_CODE_GENERATED";
 
  switch (error_code) {
    case HTTP_SEND_TIMEOUT:
      strcpy(error_message, "HTTP_SEND_TIMEOUT");
    break;
    case HTTP_SEND_REJECTED:
      strcpy(error_message, "HTTP_SEND_REJECTED");
    break;
    case HTTP_NOT_CONNECTED:
      strcpy(error_message, "HTTP_NOT_CONNECTED");
    break;
    case HTTP_BRIDGE_NOT_RUNNING:
      strcpy(error_message, "HTTP_BRIDGE_NOT_RUNNING");
    break;
    case HTTP_INVALID_ARGS:
      strcpy(error_message, "HTTP_INVALID_ARGS");
    break;
    case HTTP_BUSY:
      strcpy(error_message, "HTTP_BUSY");
    break;
    case HTTP_BUFFER_OVERFLOW:
      strcpy(error_message, "HTTP_BUFFER_OVERFLOW");
    break;
    case HTTP_ALREADY_RELEASED:
      strcpy(error_message, "HTTP_ALREADY_RELEASED");
    break;
    case HTTP_CALLBACK_ALREADY_REGISTERED:
      strcpy(error_message, "HTTP_CALLBACK_ALREADY_REGISTERED");
    break;
    case HTTP_CALLBACK_NOT_REGISTERED:
      strcpy(error_message, "HTTP_CALLBACK_NOT_REGISTERED");
    break;
    case HTTP_NOT_ENOUGH_STORAGE:
      strcpy(error_message, "HTTP_NOT_ENOUGH_STORAGE");
    break;
    case HTTP_INVALID_DICT_ARGS:
      strcpy(error_message, "HTTP_INVALID_DICT_ARGS");
    break;
    case HTTP_INTERNAL_INCONSISTENCY:
      strcpy(error_message, "HTTP_INTERNAL_INCONSISTENCY");
    break;
    case HTTP_INVALID_BRIDGE_RESPONSE:
      strcpy(error_message, "HTTP_INVALID_BRIDGE_RESPONSE");
    break;
    default: {
      strcpy(error_message, "HTTP_ERROR_UNKNOWN");
    }
  }
 
  text_layer_set_text(&balt_time_text, error_message);
}