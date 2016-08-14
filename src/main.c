/*
Copyright (C) 2016 Mark Reed / Little Gem Software

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <pebble.h>
#include <ctype.h>

// All UI elements
static Window         *window;
static Layer 		  *window_layer;

static GBitmap        *battery_image;
static GBitmap        *bluetooth_image;

static BitmapLayer    *battery_layer;
static BitmapLayer    *bluetooth_layer;

static TextLayer      *text_time_layer;
static TextLayer      *text_ampm_layer;
static TextLayer      *text_daynum_layer;
static TextLayer      *text_date_layer;
static TextLayer	  *battery_text_layer;

static GFont          small_batt;
static GFont          font_heavy;

int charge_percent = 0;


// Forward declarations
void update_time();


// This works around the inability to use the current GRect macro for constants
#define ConstantGRect(x, y, w, h) {{(x), (y)}, {(w), (h)}}


// Define layer rectangles (x, y, width, height)

#ifdef PBL_PLATFORM_CHALK
GRect TIME_RECT      = ConstantGRect(  0,   30, 116,  42 );
GRect AMPM_RECT      = ConstantGRect( 123,  38,  30,  21 );
GRect DATE_RECT      = ConstantGRect(  119, 55,  30,  30 );
GRect DAYNUM_RECT     = ConstantGRect( 150, 55,  30,  60 );
GRect BATT_RECT      = ConstantGRect(  57,  65, 70,  90 );
GRect BT_RECT        = ConstantGRect(  57, 65,  70,  90 );
#else
GRect TIME_RECT      = ConstantGRect(  0,  17, 91,  42 );
GRect AMPM_RECT      = ConstantGRect(  98, 26,  30,  21 );
GRect DATE_RECT      = ConstantGRect(  93, 41,  30,  50 );
GRect DAYNUM_RECT    = ConstantGRect(  124, 41,  30,  50 );
GRect BATT_RECT      = ConstantGRect(  38,  55,  70,  90 );
GRect BT_RECT        = ConstantGRect(  38,  55,  70,  90 );
#endif

// Define placeholders for time and date
static char time_text[] = "00:00";
static char ampm_text[] = "XXX";
static char daynum_text[] = "00";
static char date_text[] = "                    ";
 
// Previous bluetooth connection status
static bool prev_bt_status = false;


void uppercase ( char *sPtr ) {
    while ( *sPtr != '\0' )
    {
    *sPtr = toupper ( ( unsigned char ) *sPtr );
    ++sPtr;
    }
}

/*
  Setup new TextLayer
*/
static TextLayer * setup_text_layer( GRect rect, GTextAlignment align , GFont font ) {
  TextLayer *newLayer = text_layer_create( rect );
  text_layer_set_text_color( newLayer, GColorWhite );
  text_layer_set_background_color( newLayer, GColorClear );
  text_layer_set_text_alignment( newLayer, align );
  text_layer_set_font( newLayer, font );

  return newLayer;
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
  Handle battery events
*/
void handle_battery( BatteryChargeState charge_state ) {
	static char battery_text[] = "+100%";

  if ( battery_image != NULL ) {
    gbitmap_destroy( battery_image );
  }
#ifdef PBL_PLATFORM_CHALK

  if ( charge_state.is_charging ) {
	  layer_set_hidden(text_layer_get_layer(battery_text_layer), false);
    battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_CHARGING );
	snprintf(battery_text, sizeof(battery_text), "+%d%%", charge_state.charge_percent);
#else
  if ( charge_state.is_charging ) {
	  layer_set_hidden(text_layer_get_layer(battery_text_layer), false);
    battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_CHARGING );
	snprintf(battery_text, sizeof(battery_text), "+%d%%", charge_state.charge_percent);
#endif
  } else {
	snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);

    switch ( charge_state.charge_percent ) {
      case 0 ... 10:		
		battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_000_010 );
        break;
      case 11 ... 20:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_010_020 );
        break;
      case 21 ... 30:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_020_030 );
        break;
      case 31 ... 40:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_030_040 );
        break;
      case 41 ... 50:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_040_050 );
	    break;
	  case 51 ... 60:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_050_060 );
        break;
      case 61 ... 70:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_060_070 );
        break;
      case 71 ... 80:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_070_080 );
        break;
      case 81 ... 90:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_080_090 );
        break;
      case 91 ... 100:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_090_100 );
        break;
      }
  }
	bitmap_layer_set_bitmap( battery_layer, battery_image );
	charge_percent = charge_state.charge_percent;   
	text_layer_set_text(battery_text_layer, battery_text);
}
	
/*
  Update status indicators for the battery and bluetooth connection
*/
void update_status( void ) {
  handle_battery( battery_state_service_peek() );
  handle_bluetooth( bluetooth_connection_service_peek() );
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
  text_layer_set_text( text_time_layer, time_text );

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

/*
  Initialization
*/
void handle_init( void ) {
	
  memset(&battery_layer, 0, sizeof(battery_layer));
  memset(&battery_image, 0, sizeof(battery_image));
	
// international support
  setlocale(LC_ALL, "");
	
  window = window_create();
  window_stack_push( window, true );
  Layer *window_layer = window_get_root_layer( window );

  window_set_background_color(window, GColorBlack);
 
  // Load fonts

  small_batt = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_LCARS_14) );
  font_heavy = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_ROBOTO_38 ) );
	

  // Setup battery layer
  battery_layer = bitmap_layer_create( BATT_RECT );
  layer_add_child( window_layer, bitmap_layer_get_layer( battery_layer ) );

  // Setup bluetooth layer
  bluetooth_layer = bitmap_layer_create( BT_RECT );
  layer_add_child( window_layer, bitmap_layer_get_layer( bluetooth_layer ) );
	
  // Setup time layer
	
#ifdef PBL_PLATFORM_CHALK
  text_time_layer = setup_text_layer( TIME_RECT, GTextAlignmentRight, font_heavy );		
  layer_add_child( window_layer, text_layer_get_layer( text_time_layer ) );
#else
  text_time_layer = setup_text_layer( TIME_RECT, GTextAlignmentRight, font_heavy );		
  layer_add_child( window_layer, text_layer_get_layer( text_time_layer ) );
#endif

	
  // Setup AM/PM layer
	
#ifdef PBL_PLATFORM_CHALK
  text_ampm_layer = setup_text_layer( AMPM_RECT, GTextAlignmentLeft, small_batt );
  layer_add_child( window_layer, text_layer_get_layer( text_ampm_layer ) );	
#else
  text_ampm_layer = setup_text_layer( AMPM_RECT, GTextAlignmentLeft, small_batt );
  text_layer_set_background_color(text_ampm_layer, GColorClear);
  layer_add_child( window_layer, text_layer_get_layer( text_ampm_layer ) );
#endif


  // Setup date layer
	
#ifdef PBL_PLATFORM_CHALK
 text_date_layer = setup_text_layer(  DATE_RECT  , GTextAlignmentRight, small_batt );		
 text_layer_set_background_color(text_date_layer, GColorClear);
#else
 text_date_layer = setup_text_layer(  DATE_RECT, GTextAlignmentRight, small_batt );		
#endif
  layer_add_child( window_layer, text_layer_get_layer( text_date_layer ) );



  // Setup daynum layer
	
#ifdef PBL_PLATFORM_CHALK
  text_daynum_layer = setup_text_layer( DAYNUM_RECT, GTextAlignmentLeft, small_batt );
  text_layer_set_background_color(text_daynum_layer, GColorClear);
  text_layer_set_text_color(text_daynum_layer, GColorWhite );
  layer_add_child( window_layer, text_layer_get_layer( text_daynum_layer ) );
#else
  text_daynum_layer = setup_text_layer( DAYNUM_RECT, GTextAlignmentLeft, small_batt );
  text_layer_set_background_color(text_daynum_layer, GColorClear);
  text_layer_set_text_color(text_daynum_layer, GColorWhite );
  layer_add_child( window_layer, text_layer_get_layer( text_daynum_layer ) );
#endif	
	
  // set up battery text layer
	
#ifdef PBL_PLATFORM_CHALK
  battery_text_layer = text_layer_create(GRect(0, 156, 180, 20));	
  text_layer_set_background_color(battery_text_layer, GColorClear);
  text_layer_set_text_color(battery_text_layer, GColorWhite);
  text_layer_set_font(battery_text_layer, small_batt);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));		
#else
  battery_text_layer = text_layer_create(GRect(0, 144, 144, 20));	
  text_layer_set_background_color(battery_text_layer, GColorClear);
  text_layer_set_text_color(battery_text_layer, GColorWhite);
  text_layer_set_font(battery_text_layer, small_batt);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));	
#endif
	
	
  // Subscribe to services
  tick_timer_service_subscribe( MINUTE_UNIT, handle_tick );
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);


  // Force update to avoid a blank screen at startup of the watchface
  update_time();
  update_status();

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

/*
  dealloc
*/
void handle_deinit( void ) {
  // Unsubscribe from services
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  // Destroy image objects
  destroy_graphics( battery_image, battery_layer );
  destroy_graphics( bluetooth_image, bluetooth_layer );

  // Destroy text objects
  text_layer_destroy( text_time_layer );
  text_layer_destroy( text_ampm_layer );
  text_layer_destroy( text_daynum_layer );	
  text_layer_destroy( text_date_layer );
  text_layer_destroy( battery_text_layer );

  // Destroy font objects
  fonts_unload_custom_font( small_batt );
  fonts_unload_custom_font( font_heavy );

  // Destroy window
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
	
  window_destroy( window );
}

/*
  Main process
*/
int main( void ) {
  handle_init();
  app_event_loop();
  handle_deinit();
}