#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x4D, 0x54, 0xE0, 0x37, 0x35, 0x17, 0x49, 0xE2, 0xA3, 0x1A, 0xB9, 0xC7, 0x69, 0xED, 0x41, 0x76 }
PBL_APP_INFO(MY_UUID,
						 "Cody Watch", "Computing Eureka",
						 1, 0, /* App version */
						 DEFAULT_MENU_ICON,
						 APP_INFO_WATCH_FACE);

// Structs
Window window;
TextLayer tl_Hour10;
TextLayer tl_Hour1;
TextLayer tl_Min10;
TextLayer tl_Min1;
TextLayer tl_Date;

// Strings
char zero[]=" ", one[]="one", two[]="two", three[]="three", four[]="four", five[]="five", six[]="six", seven[]="seven", eight[]="eight", nine[]="nine";
char ten[]="ten", eleven[]="eleven", twelve[]="twelve", thirteen[]="thirt", fourteen[]="four", fifteen[]="fifteen", sixteen[]="six", seventeen[]="seven", eighteen[]="eight", nineteen[]="nine";
char teen[]="teen";
char zerozero[]="o'clock", zerolead[]="o'";
char twenty[]="twenty", thirty[]="thirty", fourty[]="forty", fifty[]="fifty";

// 0-9 as an array
char* counting[] = {zero, one, two, three, four, five, six, seven, eight, nine};

// The regular tens place as an array
char* strsMin10s[] = {twenty, thirty, fourty, fifty};

// The 24 hours as a lookup table
char* strsHour10[] = {zero, zero, zero, zero, zero, zero, zero, zero, zero, zero, zero, zero, zero, thirteen, fourteen, zero, sixteen, seventeen, eighteen, nineteen, zero, twenty, twenty, twenty};
char* strsHour1[] = {zerolead, one, two, three, four, five, six, seven, eight, nine, ten, eleven, twelve, teen, teen, fifteen, teen, teen, teen, teen, twenty, one, two, three};

// The first 19 minutes as a lookup table
char* strsMin10[] = {zerozero, zerolead, zerolead, zerolead, zerolead, zerolead, zerolead, zerolead, zerolead, zerolead, ten, eleven, twelve, thirteen, fourteen, fifteen, sixteen, seventeen, eighteen, nineteen};
char* strsMin1[] = {zero, one, two, three, four, five, six, seven, eight, nine, zero, zero, zero, teen, teen, zero, teen, teen, teen, teen};

// Days and months as a lookup table
char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char* months[] = {"January", "Feburary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Holds the current values for the time and date
char dateStr[24];
char hour10Str[7];
char hour1Str[8];
char min10Str[8];
char min1Str[6];

// Holds the new values for the time and date
char newDateStr[24];
char newHour10Str[7];
char newHour1Str[8];
char newMin10Str[8];
char newMin1Str[6];

PropertyAnimation paLeave[12];
PropertyAnimation paReturn[12];

bool first;

// int err;
// char msg[15];

int getFirstPaIndex(PropertyAnimation pa[], int len)
{
	int i;

	for (i = 0; i < len && animation_is_scheduled(&pa[i].animation); i++);
	
	if (i==len)
		return -1;

	return i;
}

void animationStopped(struct Animation *animation, bool finished, void *context)
{
	int paIndex = getFirstPaIndex(paReturn, 5);
	if(paIndex == -1)
		return;

	TextLayer* tl = (TextLayer*)context;
	property_animation_init_layer_frame(&paReturn[paIndex], &tl->layer, &GRect(144, tl->layer.frame.origin.y, 144, 49), &GRect(0, tl->layer.frame.origin.y, 144, 49));

	animation_set_duration(&paReturn[paIndex].animation, paIndex * 100 + 500);
	animation_set_curve(&paReturn[paIndex].animation, AnimationCurveEaseOut);
	animation_schedule(&paReturn[paIndex].animation);

	if(tl == &tl_Hour10)
		strcpy(hour10Str, newHour10Str);
	if(tl == &tl_Hour1)
		strcpy(hour1Str, newHour1Str);
	if(tl == &tl_Min10)
		strcpy(min10Str, newMin10Str);
	if(tl == &tl_Min1)
		strcpy(min1Str, newMin1Str);
	if(tl == &tl_Date)
		strcpy(dateStr, newDateStr);
}

void move(TextLayer* tl)
{
	// err++;
	// snprintf(msg, 15, "%d", err);
	int paIndex = getFirstPaIndex(paLeave, 5);
	if(paIndex == -1)
		return;

	int duration = (first) ? paIndex * 100 + 500 : 50;

	static AnimationHandlers aniHandlers = {
		.stopped = &animationStopped
	};

	property_animation_init_layer_frame(&paLeave[paIndex], &tl->layer, NULL, &GRect(-144, tl->layer.frame.origin.y, 144, 49));

	animation_set_delay(&paLeave[paIndex].animation, paIndex * 50);
	animation_set_duration(&paLeave[paIndex].animation, duration);
	animation_set_curve(&paLeave[paIndex].animation, AnimationCurveEaseIn);
	animation_set_handlers(&paLeave[paIndex].animation, aniHandlers, tl);
	animation_schedule(&paLeave[paIndex].animation);
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent* t)
{
	(void) ctx;
	(void) t;

	char* min;

	PblTm time;
	get_time(&time);

	snprintf(newHour10Str, 7, "%s", strsHour10[time.tm_hour]);

	snprintf(newHour1Str, 8, "%s", strsHour1[time.tm_hour]);

	min = (time.tm_min<20) ? strsMin10[time.tm_min] : strsMin10s[time.tm_min/10 - 2];
	snprintf(newMin10Str, 8, "%s", min);

	min = (time.tm_min<20) ? strsMin1[time.tm_min] : counting[time.tm_min%10];
	snprintf(newMin1Str, 6, "%s", min);

	snprintf(newDateStr, 24, "%s, %s %d", days[time.tm_wday], months[time.tm_mon], time.tm_mday);

	if (!getFirstPaIndex(paReturn, 5))
	{
		if (strcmp(newHour10Str, hour10Str))
			move(&tl_Hour10);
		if (strcmp(newHour1Str, hour1Str))
			move(&tl_Hour1);
		if (strcmp(newMin10Str, min10Str))
			move(&tl_Min10);
		if (strcmp(newMin1Str, min1Str))
			move(&tl_Min1);
		if (strcmp(newDateStr, dateStr))
			move(&tl_Date);
	}

	if(first)
	{
		first = false;
		// strcpy(hour10Str, newHour10Str);
		// strcpy(hour1Str, newHour1Str);
		// strcpy(min10Str, newMin10Str);
		// strcpy(min1Str, newMin1Str);
		// strcpy(dateStr, newDateStr);
	}
}

void handle_init(AppContextRef ctx)
{
	first = true;
	// err = 0;

	resource_init_current_app(&RES_FOR_TEXTWATCH24);

	window_init(&window, "Cody Watch");
	window_set_background_color(&window, GColorBlack);
	window_stack_push(&window, true);

	text_layer_init(&tl_Date, GRect(0,156,144,12));
	// text_layer_set_text(&tl_Date, "Wednesday September 10");
	text_layer_set_font(&tl_Date, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_10)));
	tl_Date.text_color = GColorWhite;
	tl_Date.background_color = GColorBlack;
	layer_add_child(&window.layer, &tl_Date.layer);

	text_layer_init(&tl_Min1, GRect(0,107,144,49));
	// text_layer_set_text(&tl_Min1, "fifteen");
	text_layer_set_font(&tl_Min1, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	tl_Min1.text_color = GColorWhite;
	tl_Min1.background_color = GColorBlack;
	layer_add_child(&window.layer, &tl_Min1.layer);

	text_layer_init(&tl_Min10, GRect(0,68,144,49));
	// text_layer_set_text(&tl_Min10, "thirteen");
	text_layer_set_font(&tl_Min10, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	tl_Min10.text_color = GColorWhite;
	tl_Min10.background_color = GColorBlack;
	layer_add_child(&window.layer, &tl_Min10.layer);

	text_layer_init(&tl_Hour1, GRect(0,29,144,49));
	// text_layer_set_text(&tl_Hour1, "teen");
	text_layer_set_font(&tl_Hour1, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	tl_Hour1.text_color = GColorWhite;
	tl_Hour1.background_color = GColorBlack;
	layer_add_child(&window.layer, &tl_Hour1.layer);

	text_layer_init(&tl_Hour10, GRect(0,-10,144,49));
	// text_layer_set_text(&tl_Hour10, "thirt");
	text_layer_set_font(&tl_Hour10, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	tl_Hour10.text_color = GColorWhite;
	tl_Hour10.background_color = GColorBlack;
	layer_add_child(&window.layer, &tl_Hour10.layer);

	strcpy(hour10Str, " ");
	strcpy(hour1Str, " ");
	strcpy(min10Str, " ");
	strcpy(min1Str, " ");
	strcpy(dateStr, " ");

	text_layer_set_text(&tl_Hour10, hour10Str);
	text_layer_set_text(&tl_Hour1, hour1Str);
	text_layer_set_text(&tl_Min10, min10Str);
	text_layer_set_text(&tl_Min1, min1Str);
	text_layer_set_text(&tl_Date, dateStr);

	// text_layer_set_text(&tl_Hour10, msg);

	// handle_minute_tick(ctx, NULL);
}

void pbl_main(void *params)
{
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.tick_info = {
			.tick_handler = &handle_minute_tick,
			.tick_units = MINUTE_UNIT
			}
	};
	app_event_loop(params, &handlers);
}
