#ifndef __STIL_WRITER_H__
#define __STIL_WRITER_H__

#include "linklist.h"

int stil_set_outfile(char *filename);
int stil_write_vector(char *vector_line, int force);
int stil_write_header(linklist_t *pins);
int stil_write_footer();
int stil_setrepeat_mode(char *mode);

#endif
