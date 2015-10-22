#include <stdio.h>

#include <string.h>
#include "pattern_writer.h"
#include "errors.h"
#include "stil_writer.h"

int init_pattern_writer(pattern_writer_t *p, char *outfilename) {
	memset(p, 0, sizeof(pattern_writer_t) );

	if ( strstr(outfilename, ".stil") ) {	
		p->fn_header_writer = stil_write_header;
		p->fn_vector_writer = stil_write_vector;
		p->fn_footer_writer = stil_write_footer;
		p->fn_set_repeat    = stil_setrepeat_mode;
		return ( stil_set_outfile(outfilename));
	} else {
		return OUTPUT_WRITE_ERROR;
	}

	return NO_ERROR;
}

