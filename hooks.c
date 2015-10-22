#include "string_manip.h"
#include "signal_defs.h"
#include "hash.h"
#include "errors.h"
#include "vcd.h"

int setpinval_hook(void *v, void *key, void *value) {
	vcd_t *thisv = v;
	char *_key = key;

	pair_t *__pair = thisv->signalProperties.fn_get ( &(thisv->signalProperties), _key); 
	pin_t *__pin = __pair->value;

	char *_value  = value;

	if (__pin->direction == OUT) {
        	replace(&_value, "1","H");
                replace(&_value, "0","L");
        }

	#if (0)
	/* Below is an example how to Get the value of a pin.
	   Start by getting the alias of the pin (using pin name as a key) from the Name2Alias hash
	*/
	__pair = thisv->Name2Alias.fn_get ( &(thisv->Name2Alias), __pin->name); 
	char *__alias= __pair->value;
	printf ("%s %s ", __pin->name, __alias );

	/* Then, get the value of the pin, using alias (obtained from above) as the key 
	   Then maybe change the value of a the parameter *value* based on this. (ie if a pin is inout, if OE is asserted, then change to H/L, else maintain as 0/1
	*/
	
	__pair = thisv->signalValues.fn_get ( &(thisv->signalValues), __alias); 
	char *__value = __pair->value;

	printf ("%s\n ", __value  );
	#endif

	return NO_ERROR;
}
