#include <pebble.h>
#include <ctype.h>


static Window *s_main_window;
static Layer *window_layer;
static TextLayer *s_time_layer;
static TextLayer *battery_text_layer;

static TextLayer      *text_ampm_layer;
static TextLayer      *text_daynum_layer;
static TextLayer      *text_date_layer;

static GFont font_heavy;
static GFont small_batt;

int charge_percent = 0;

static GBitmap        *battery_image;
static GBitmap        *bluetooth_image;
static BitmapLayer    *battery_layer;
static BitmapLayer    *bluetooth_layer;

static const uint8_t batt_offset_top_percent = 85;
static const uint8_t face_offset_percent = 25;

// Define placeholders for time and date
static char time_text[] = "00:00";
static char ampm_text[] = "XXX";
static char daynum_text[] = "00";
static char date_text[] = "                    ";

// Previous bluetooth connection status
static bool prev_bt_status = false;



uint8_t relative_pixel(uint8_t percent, uint8_t max) {
  return (max * percent) / 100;
}

void uppercase ( char *sPtr ) {
    while ( *sPtr != '\0' )
    {
    *sPtr = toupper ( ( unsigned char ) *sPtr );
    ++sPtr;
    }
}

/*
  Handle battery events
*/
void handle_battery( BatteryChargeState charge_state ) {
	static char battery_text[] = "    Emoji is 100% happy    ";

  if ( battery_image != NULL ) {
    gbitmap_destroy( battery_image );
  }

  if ( charge_state.is_charging ) {
	  layer_set_hidden(text_layer_get_layer(battery_text_layer), false);
    battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_CHARGING );
	snprintf(battery_text, sizeof(battery_text), "Emoji charging, +%d%%", charge_state.charge_percent);

  } else {

    switch ( charge_state.charge_percent ) {
      case 0 ... 10:		
		battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_000_010 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, charge me!", charge_state.charge_percent);

        break;
      case 11 ... 20:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_010_020 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, not happy", charge_state.charge_percent);

        break;
      case 21 ... 30:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_020_030 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, upset", charge_state.charge_percent);

        break;
      case 31 ... 40:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_030_040 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, worried", charge_state.charge_percent);

        break;
      case 41 ... 50:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_040_050 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, whatever", charge_state.charge_percent);

	    break;
	  case 51 ... 60:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_050_060 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, just ok", charge_state.charge_percent);

        break;
      case 61 ... 70:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_060_070 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, all good", charge_state.charge_percent);

        break;
      case 71 ... 80:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_070_080 );
		snprintf(battery_text, sizeof(battery_text), "Emoji is %d%% happy", charge_state.charge_percent);

        break;
      case 81 ... 90:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_080_090 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, very happy", charge_state.charge_percent);

        break;
      case 91 ... 100:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_090_100 );
		snprintf(battery_text, sizeof(battery_text), "Emoji %d%%, super!", charge_state.charge_percent);

        break;
      }
  }
	bitmap_layer_set_bitmap( battery_layer, battery_image );
	charge_percent = charge_state.charge_percent;   
	text_layer_set_text(battery_text_layer, battery_text);
}

	
/*
  Handle bluetooth events
*/
void handle_bluetooth( bool connected ) {
  if ( bluetooth_image != NULL ) {
    gbitmap_destroy( bluetooth_image );
  }

  if ( connected ) {
   // bluetooth_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BLUETOOTH );
	  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), true);
  } else {
    bluetooth_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_NO_BLUETOOTH );
	  layer_set_hidden(bitmap_layer_get_layer(bluetooth_layer), false);
  }

  prev_bt_status = connected;
  bitmap_layer_set_bitmap( bluetooth_layer, bluetooth_image );
}

	
	/*
  Update status indicators for the battery and bluetooth connection
*/
void update_status( void ) {
  handle_battery( battery_state_service_peek() );
  handle_bluetooth( bluetooth_connection_service_peek() );
}
	
static void update_ui(void) {
  // Adapt the layout based on any obstructions
  GRect full_bounds = layer_get_bounds(window_layer);
  GRect unobstructed_bounds = layer_get_unobstructed_bounds(window_layer);

  if (!grect_equal(&full_bounds, &unobstructed_bounds)) {
    // Screen is obstructed
 //   layer_set_hidden(bitmap_layer_get_layer(s_background_layer), true);
 //   text_layer_set_text_color(s_time_layer, GColorWhite);
  } else {
    // Screen is unobstructed
  //  layer_set_hidden(bitmap_layer_get_layer(s_background_layer), false);
  //  text_layer_set_text_color(s_time_layer, GColorBlack);
  }

#ifdef PBL_RECT
	
GRect battery_frame = layer_get_frame(text_layer_get_layer(battery_text_layer));
  battery_frame.origin.y = relative_pixel(batt_offset_top_percent, unobstructed_bounds.size.h);
  layer_set_frame(text_layer_get_layer(battery_text_layer), battery_frame);
	
  GRect bt_frame = layer_get_frame(bitmap_layer_get_layer(bluetooth_layer));
  bt_frame.origin.y = relative_pixel(face_offset_percent, unobstructed_bounds.size.h);
  layer_set_frame(bitmap_layer_get_layer(bluetooth_layer), bt_frame);
	
  GRect face_frame = layer_get_frame(bitmap_layer_get_layer(battery_layer)); 
  face_frame.origin.y = relative_pixel(face_offset_percent, unobstructed_bounds.size.h);
  layer_set_frame(bitmap_layer_get_layer(battery_layer), face_frame);
	
#endif
	
}
	
/*
  Handle tick events
*/
void handle_tick( struct tm *tick_time, TimeUnits units_changed ) {
	

  // Update text layer for current day

  strftime( date_text, sizeof( date_text ), "%b", tick_time );
	uppercase(date_text);
  text_layer_set_text( text_date_layer, date_text );

	
  // Display day num 
  strftime( daynum_text, sizeof( daynum_text ), "%e", tick_time );
  text_layer_set_text( text_daynum_layer, daynum_text );	
	

  // Display hours (i.e. 18 or 06)
  strftime( time_text, sizeof( time_text ), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time );

  // Remove leading zero (only in 12h-mode)
  if ( !clock_is_24h_style() && (time_text[0] == '0') ) {
    memmove( time_text, &time_text[1], sizeof( time_text ) - 1 );
  }
  text_layer_set_text( s_time_layer, time_text );

  // Update AM/PM indicator (i.e. AM or PM or nothing when using 24-hour style)
  strftime( ampm_text, sizeof( ampm_text ), clock_is_24h_style() ? "" : "%p", tick_time );
  text_layer_set_text( text_ampm_layer, ampm_text );

}
	
/*
  Force update of time
*/
void update_time() {

  // Get current time
  time_t now = time( NULL );
  struct tm *tick_time = localtime( &now );

  // Force update to avoid a blank screen at startup of the watchface
  handle_tick( tick_time, MONTH_UNIT + DAY_UNIT + HOUR_UNIT + MINUTE_UNIT );

}
	
static void initialise_ui(void) {
  GRect bounds = layer_get_bounds(window_layer);

  // Create GFonts
  font_heavy = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_38));
  small_batt = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_14));
	

  // Create BitmapLayer to display the GBitmap
  battery_layer = bitmap_layer_create(GRect(0, relative_pixel(face_offset_percent, bounds.size.h), bounds.size.w, 90));
  bitmap_layer_set_bitmap(battery_layer, battery_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));	  
	  

  // Setup bluetooth layer
  bluetooth_layer = bitmap_layer_create(GRect(0, relative_pixel(face_offset_percent, bounds.size.h), bounds.size.w, 90));
  layer_add_child( window_layer, bitmap_layer_get_layer( bluetooth_layer ) );
	
	
	// Create time TextLayer
	
#ifdef PBL_PLATFORM_CHALK
  s_time_layer = text_layer_create(GRect(  0,   20, 116,  42 ));
#else
  s_time_layer = text_layer_create(GRect(  0,  0, 93,  40 ));
#endif
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
//  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, font_heavy);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // set up battery text layer
	
#ifdef PBL_PLATFORM_CHALK
  battery_text_layer = text_layer_create(GRect(  56,  120,  66,  60 ));
#else
	battery_text_layer = text_layer_create(GRect(0, relative_pixel(batt_offset_top_percent, bounds.size.h), bounds.size.w, 25));
#endif
	text_layer_set_background_color(battery_text_layer, GColorClear);
  text_layer_set_text_color(battery_text_layer, GColorWhite);
  text_layer_set_font(battery_text_layer, small_batt);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));		
	
  // Setup AM/PM layer
	
#ifdef PBL_PLATFORM_CHALK
  text_ampm_layer = text_layer_create(GRect(  120,  28,  30,  21 ));
#else
	text_ampm_layer = text_layer_create(GRect(  98, 9,  30,  21 ));	
#endif
  text_layer_set_background_color(text_ampm_layer, GColorClear);
  text_layer_set_text_color(text_ampm_layer, GColorWhite);
  text_layer_set_font(text_ampm_layer, small_batt);
  text_layer_set_text_alignment(text_ampm_layer, GTextAlignmentLeft);
  layer_add_child( window_layer, text_layer_get_layer( text_ampm_layer ) );	

  // Setup date (month) layer
	
#ifdef PBL_PLATFORM_CHALK
  text_date_layer = text_layer_create(GRect(  116, 45,  30,  30 ));
#else
	text_date_layer = text_layer_create(GRect(  93, 24,  30,  50 ));		
#endif
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_font(text_date_layer, small_batt);
  text_layer_set_text_alignment(text_date_layer, GTextAlignmentRight);	
  layer_add_child( window_layer, text_layer_get_layer( text_date_layer ) );
	


  // Setup daynum layer
	
	if (clock_is_24h_style()) {
#ifdef PBL_PLATFORM_CHALK
  text_daynum_layer = text_layer_create(GRect(  120,  28,  30,  21 ));
#else
	text_daynum_layer = text_layer_create(GRect(  98, 9,  30,  21 ));
#endif
	} else {	
#ifdef PBL_PLATFORM_CHALK
  text_daynum_layer = text_layer_create(GRect(  147, 45,  30,  60 ));
#else
	text_daynum_layer = text_layer_create(GRect(  124, 24,  30,  50 ));
#endif			
	}
  text_layer_set_background_color(text_daynum_layer, GColorClear);
  text_layer_set_text_color(text_daynum_layer, GColorWhite );
  text_layer_set_font(text_daynum_layer, small_batt);
  text_layer_set_text_alignment(text_daynum_layer, GTextAlignmentLeft);	
  layer_add_child( window_layer, text_layer_get_layer( text_daynum_layer ) );
	

// Check for obstructions
  update_ui();
}

/*
  Destroy GBitmap and BitmapLayer
*/
void destroy_graphics( GBitmap *image, BitmapLayer *layer ) {
  layer_remove_from_parent( bitmap_layer_get_layer( layer ) );
  bitmap_layer_destroy( layer );
  if ( image != NULL ) {
    gbitmap_destroy( image );
  }
}

static void destroy_ui(void) {
  // Destroy image objects
  destroy_graphics( battery_image, battery_layer );
  destroy_graphics( bluetooth_image, bluetooth_layer );
}


static void app_unobstructed_change(AnimationProgress progress, void *context) {
  update_ui();
}

static void main_window_load(Window *window) {
  window_layer = window_get_root_layer(window);

  // Create the UI elements
  initialise_ui();

  // Make sure the time is displayed from the start
  update_time();

  // Subscribe to the change event
  UnobstructedAreaHandlers handlers = {
    .change = app_unobstructed_change
  };
  unobstructed_area_service_subscribe(handlers, NULL);

}

static void main_window_unload(Window *window) {
  text_layer_destroy( s_time_layer );
  text_layer_destroy( text_ampm_layer );
  text_layer_destroy( text_daynum_layer );	
  text_layer_destroy( text_date_layer );
  text_layer_destroy( battery_text_layer );
	
  fonts_unload_custom_font(small_batt);
  fonts_unload_custom_font(font_heavy);

  // Unsubscribe from services
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  unobstructed_area_service_unsubscribe();

  // Clean up the unused UI elenents
  destroy_ui();
}


static void init() {
	
  // international support
  setlocale(LC_ALL, "");	
	
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  // Subscribe to services
  tick_timer_service_subscribe( MINUTE_UNIT, handle_tick );
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);


  // Force update to avoid a blank screen at startup of the watchface
  update_time();
  update_status();

}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
