#ifndef __PATTERN_WRITER_H__
#define __PATTERN_WRITER_H__

#include "linklist.h"

typedef struct {
	int (*fn_header_writer)(linklist_t *);
	int (*fn_vector_writer)(char *, int);
	int (*fn_footer_writer)(void);
	int (*fn_set_repeat)(char *);
} pattern_writer_t;

int init_pattern_writer(pattern_writer_t *p, char *outfilename);


#endif
