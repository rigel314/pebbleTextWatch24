#include "pebble.h"

#define DATE 1
#define OH 0

#define DATE_LEADING_SPACE 2 // I know, 'two pixels', right?  Well, it was bothering me!

// For keeping track of stuff and making sure to free alloc'ed memory
struct aniInfo
{
	TextLayer* tl;
	PropertyAnimation* pa;
};

// UI elements
Window* window;
TextLayer* tl_Hour10;
TextLayer* tl_Hour1;
TextLayer* tl_Min10;
TextLayer* tl_Min1;
TextLayer* tl_Date;

// Strings
char zero[]=" ", one[]="one", two[]="two", three[]="three", four[]="four", five[]="five", six[]="six", seven[]="seven", eight[]="eight", nine[]="nine";
char ten[]="ten", eleven[]="eleven", twelve[]="twelve", thirteen[]="thirt", fourteen[]="four", fifteen[]="fifteen", sixteen[]="six", seventeen[]="seven", eighteen[]="eight", nineteen[]="nine";
char teen[]="teen";
char zerozero[]="o'clock";
#if (OH == 0)
	char zerolead[]="o'";
#elif (OH == 1)
	char zerolead[]="oh";
#endif
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

// Days and months as lookup tables
char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char* months[] = {"January", "Feburary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Holds the current values for the time and date
char dateStr[25];
char hour10Str[7];
char hour1Str[8];
char min10Str[8];
char min1Str[6];

// Holds the new values for the time and date
char newDateStr[25];
char newHour10Str[7];
char newHour1Str[8];
char newMin10Str[8];
char newMin1Str[6];

PropertyAnimation* paLeave[12];
PropertyAnimation* paReturn[12];

bool first;
int numAnimations;

int getFirstPaIndex(PropertyAnimation* pa[], int len)
{
	int i;

	for (i = 0; i < len && animation_is_scheduled(&pa[i]->animation); i++);
	
	if (i==len)
		return -1;

	return i;
}

void freeReturnAnimations(struct Animation *animation, bool finished, void *context)
{
	property_animation_destroy((PropertyAnimation*)context);
}

void animationStopped(struct Animation *animation, bool finished, void *context)
{
	numAnimations--;

	int paIndex = getFirstPaIndex(paReturn, 5);
	if(paIndex == -1)
		return;

	static AnimationHandlers aniHandlers = {
		.stopped = &freeReturnAnimations
	};

	TextLayer* tl = ((struct aniInfo*)context)->tl;
	GRect dest, src;

	if(tl == tl_Date)
		dest = GRect(DATE_LEADING_SPACE, layer_get_frame(text_layer_get_layer(tl)).origin.y, 144, 49);
	else
		dest = GRect(0, layer_get_frame(text_layer_get_layer(tl)).origin.y, 144, 49);

	src = GRect(144, layer_get_frame(text_layer_get_layer(tl)).origin.y, 144, 49);
	paReturn[paIndex] = property_animation_create_layer_frame(text_layer_get_layer(tl), &src, &dest);
	// return;

	animation_set_duration(&paReturn[paIndex]->animation, paIndex * 200 + 1000);
	animation_set_curve(&paReturn[paIndex]->animation, AnimationCurveEaseOut);
	animation_set_handlers(&paReturn[paIndex]->animation, aniHandlers, paReturn[paIndex]);
	animation_schedule(&paReturn[paIndex]->animation);

	if(tl == tl_Hour10)
		strcpy(hour10Str, newHour10Str);
	if(tl == tl_Hour1)
		strcpy(hour1Str, newHour1Str);
	if(tl == tl_Min10)
		strcpy(min10Str, newMin10Str);
	if(tl == tl_Min1)
		strcpy(min1Str, newMin1Str);
	if(tl == tl_Date)
		strcpy(dateStr, newDateStr);

	property_animation_destroy(((struct aniInfo*)context)->pa);
	free(context);
}

void move(TextLayer* tl)
{
	numAnimations++;

	int paIndex = getFirstPaIndex(paLeave, 5);
	if(paIndex == -1)
		return;

	struct aniInfo* ai = malloc(sizeof(struct aniInfo));

	int duration = (first) ? 50 : paIndex * 200 + 1000;

	static AnimationHandlers aniHandlers = {
		.stopped = &animationStopped
	};

	paLeave[paIndex] = property_animation_create_layer_frame(text_layer_get_layer(tl), NULL, &GRect(-144, layer_get_frame(text_layer_get_layer(tl)).origin.y, 144, 49));

	ai->tl = tl;
	ai->pa = paLeave[paIndex];

	animation_set_delay(&paLeave[paIndex]->animation, paIndex * 50);
	animation_set_duration(&paLeave[paIndex]->animation, duration);
	animation_set_curve(&paLeave[paIndex]->animation, AnimationCurveEaseIn);
	animation_set_handlers(&paLeave[paIndex]->animation, aniHandlers, ai);
	animation_schedule(&paLeave[paIndex]->animation);
}

void handle_minute_tick(struct tm *now, TimeUnits units_changed)
{
	char* min;

	snprintf(newHour10Str, 7, "%s", strsHour10[now->tm_hour]);

	snprintf(newHour1Str, 8, "%s", strsHour1[now->tm_hour]);

	min = (now->tm_min<20) ? strsMin10[now->tm_min] : strsMin10s[now->tm_min/10 - 2];
	snprintf(newMin10Str, 8, "%s", min);

	min = (now->tm_min<20) ? strsMin1[now->tm_min] : counting[now->tm_min%10];
	snprintf(newMin1Str, 6, "%s", min);

	#if (DATE)
		snprintf(newDateStr, 25, "%s, %s %d", days[now->tm_wday], months[now->tm_mon], (int) now->tm_mday);
	#endif

	/**
	*	tick_handler gets called automatically at the next second after switching to a watchface.
	*	I call tick_handler in my init_handler to start updating immediatly.  This causes tick_handler
	*	to get called again.  Which, in turn, causes these animations to go haywire.  All
	*	animations shouldn't take longer than a minute, so if the numAnimations isn't 0, a minute
	*	passed too quickly.
	*/
	if (numAnimations == 0)
	{
		if (strcmp(newHour10Str, hour10Str))
			move(tl_Hour10);
		if (strcmp(newHour1Str, hour1Str))
			move(tl_Hour1);
		if (strcmp(newMin10Str, min10Str))
			move(tl_Min10);
		if (strcmp(newMin1Str, min1Str))
			move(tl_Min1);
		if (strcmp(newDateStr, dateStr))
			move(tl_Date);
	}

	if(first)
		first = false;
}

void init(void)
{
	first = true;
	int dO = 6; // date Offset: number of pixels to offset everything if there is no date.
	// err = 0;
	static struct tm* now;
	time_t unix_now;

	window = window_create();
	window_set_background_color(window, GColorBlack);
	window_stack_push(window, true);

	#if (DATE)
		tl_Date = text_layer_create(GRect(DATE_LEADING_SPACE,156,144,49));
		// text_layer_set_text(tl_Date, "Wednesday September 10");
		text_layer_set_font(tl_Date, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MONACO_10)));
		text_layer_set_text_color(tl_Date, GColorWhite);
		text_layer_set_background_color(tl_Date, GColorBlack);
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(tl_Date));
		dO = 0;
	#endif

	tl_Min1 = text_layer_create(GRect(0,107+dO,144,49));
	// text_layer_set_text(tl_Min1, "fifteen");
	text_layer_set_font(tl_Min1, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_color(tl_Min1, GColorWhite);
	text_layer_set_background_color(tl_Min1, GColorBlack);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(tl_Min1));

	tl_Min10 = text_layer_create(GRect(0,68+dO,144,49));
	// text_layer_set_text(tl_Min10, "thirteen");
	text_layer_set_font(tl_Min10, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_color(tl_Min10, GColorWhite);
	text_layer_set_background_color(tl_Min10, GColorBlack);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(tl_Min10));

	tl_Hour1 = text_layer_create(GRect(0,29+dO,144,49));
	// text_layer_set_text(tl_Hour1, "teen");
	text_layer_set_font(tl_Hour1, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_color(tl_Hour1, GColorWhite);
	text_layer_set_background_color(tl_Hour1, GColorBlack);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(tl_Hour1));

	tl_Hour10 = text_layer_create(GRect(0,-10+dO,144,49));
	// text_layer_set_text(tl_Hour10, "thirt");
	text_layer_set_font(tl_Hour10, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_color(tl_Hour10, GColorWhite);
	text_layer_set_background_color(tl_Hour10, GColorBlack);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(tl_Hour10));

	strcpy(hour10Str, " ");
	strcpy(hour1Str, " ");
	strcpy(min10Str, " ");
	strcpy(min1Str, " ");
	strcpy(dateStr, " ");

	text_layer_set_text(tl_Hour10, hour10Str);
	text_layer_set_text(tl_Hour1, hour1Str);
	text_layer_set_text(tl_Min10, min10Str);
	text_layer_set_text(tl_Min1, min1Str);
	text_layer_set_text(tl_Date, dateStr);

	// text_layer_set_text(tl_Hour10, msg);

	tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);

	unix_now = time(NULL);
	now = localtime(&unix_now);
	handle_minute_tick(now, MINUTE_UNIT);
}

void deinit(void)
{
	text_layer_destroy(tl_Hour10);
	text_layer_destroy(tl_Hour1);
	text_layer_destroy(tl_Min10);
	text_layer_destroy(tl_Min1);
	text_layer_destroy(tl_Date);

	window_destroy(window);
}

int main(void)
{
	init();

	app_event_loop();

	deinit();
}
