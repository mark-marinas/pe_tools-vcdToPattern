#ifndef __SIGNAL_DEFSH__
#define __SIGNAL_DEFSH__

typedef enum {
        POSITIVE,
        NEGATIVE,
	HIGHZ,
	ABSENT
} signal_polarity_t;


typedef enum {
	IN,
	INOUT,
	OUT,
	Z
} signal_direction_t ;




typedef struct {
	char *name;
	char *alias;
	char *value;
	int width;
	signal_direction_t direction;
} pin_t ;

extern char *signal_direction_string[] ;

#define toString_direction(direction) signal_direction_string[direction]


int arrange_pins(pin_t *dst, pin_t *src, int size);

#endif
