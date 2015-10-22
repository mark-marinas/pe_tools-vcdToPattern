#include <stdlib.h>
#include <string.h>

#include "utils.h"


void *vmalloc(int size, char init_value) {
	void *mem = malloc(size);
	if (mem == 0) {
		return (void *) 0;
	}	
	memset(mem, init_value, size);
	return mem;
}
