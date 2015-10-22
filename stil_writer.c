#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vector.h"
#include "errors.h"
#include "linklist.h"
#include "signal_defs.h"
#include "utils.h"

static int _stil_write_vector();
static vector_line_t current_vector_line = { 0, 0, 0 };
static char *filename = 0;
static FILE *fp = 0;
static int repeat_on = 1;

int stil_set_outfile(char *_filename) {
	if (_filename == 0) {
		#ifdef _PRINT_TO_STDOUT_		
			fp = stdout;
		#else
			return OUTPUT_WRITE_ERROR;
		#endif
	} else {
		fp = fopen(_filename, "w");
		if (fp == 0) {
			return OUTPUT_FILE_OPEN_ERROR;
		}	
		filename = vmalloc(strlen(_filename) + 1, 0);
		strcpy(filename, _filename); 
	}
	return NO_ERROR;
}

static int _stil_write_vector() {
	int expected_bytes_written = strlen(current_vector_line.data);
	if ( current_vector_line.loopcnt == 0) {

	} else if ( current_vector_line.loopcnt == 1 ) {
		if ( fprintf (fp, "\tV { ALL_PINS = %s ; }\n", current_vector_line.data) < expected_bytes_written ) {
			return OUTPUT_WRITE_ERROR;
		}
	} else {
		fprintf(fp, "\tLoop %d {\n", current_vector_line.loopcnt );
		if ( fprintf (fp, "\t\tV { ALL_PINS = %s ; }\n", current_vector_line.data) < expected_bytes_written) {
			return OUTPUT_WRITE_ERROR;
		}
		if ( (fprintf(fp, "\t}\n") ) < 3) {
			return OUTPUT_WRITE_ERROR;
		}
	} 

	current_vector_line.loopcnt = 0;
	return NO_ERROR;

}


int stil_write_vector(char *vector_line, int force) {
	static int first_vector = 1;

	if (current_vector_line.data == 0) {
		current_vector_line.data = vmalloc(strlen(vector_line) + 1,0);
		current_vector_line.datawidth = strlen(vector_line) ;
	} else if ( current_vector_line.datawidth != strlen(vector_line) ) {
		return VECTOR_LENGTH_ERROR;
	}

	if (repeat_on == 0) {
		current_vector_line.loopcnt = 1;
		strcpy(current_vector_line.data, vector_line) ;
		return (_stil_write_vector());
	}

	if (force) {
		//Write any vectors, if any.
		_stil_write_vector();
		//Then write this vector.
		current_vector_line.loopcnt = 1;
		strcpy(current_vector_line.data, vector_line) ;
		return (_stil_write_vector());
	}


	if ( strcmp(current_vector_line.data, vector_line) == 0 || first_vector ) {
		//This is the same as previous vector.
		strcpy(current_vector_line.data, vector_line) ;
		first_vector = 0;
		current_vector_line.loopcnt++ ;
	} else {
		//This vector is different.
		//Write any vectors, if any.
		_stil_write_vector();
		//Then write it to the buffer.
		current_vector_line.loopcnt = 1;
		strcpy(current_vector_line.data, vector_line) ;
	} 
	return NO_ERROR;
}

int stil_write_header(linklist_t *pins) {
	//write the signals	
	fprintf (fp, "Signals {\n");
	linklistE_t *head = pins->head;
	char *sdirection = vmalloc(strlen("INOUT")+1, 0) ;
	while (head) {
		pin_t *pin = head->data;
		signal_direction_t direction = pin->direction;

                switch ( pin->direction ) {
                	case IN:
				strcpy(sdirection, "IN" ); 
				break;
                        case OUT:
				strcpy(sdirection,"OUT"); 
                                break;
                        case INOUT:
				strcpy(sdirection,"INOUT"); 
                                break;
                        case Z:
				strcpy(sdirection,"HIGHZ"); 
                                break;
                        default:
                           	return UNKNOWN_DIRECTION;
		}

		if ( pin->width > 1) {
			int i;
			for (i=0; i<pin->width; i++) {
				fprintf(fp, "\t%s_%d\t%s;\n", pin->name, i, sdirection);
			} 
		} else { 
			fprintf(fp, "\t%s\t%s;\n", pin->name, sdirection);
		}		
		head = head->next;
	}
	free(sdirection);

	fprintf(fp, "}\n\n");


	//Signal Groups.
	fprintf (fp, "SignalGroups {\n");
	head = pins->head;
	while (head) {
		pin_t *pin = head->data;	
		if (pin->width > 1) {
			fprintf (fp, "\t%s\t'%s_%d", pin->name, pin->name, pin->width - 1 );
			int i;
			for (i=pin->width - 2; i>=0; i--) {
				fprintf(fp, " + %s_%d", pin->name, i);
			} 
			fprintf (fp, "';\n");
		}
		head = head->next;
	}

	head = pins->head;
	pin_t *pin = head->data;
	fprintf (fp, "\tALL_PINS\t'%s", pin->name);
	head = head->next;
	while (head) {
		pin = head->data;
		fprintf (fp, " + %s", pin->name);
		head = head->next;
	}
	fprintf (fp, "';\n") ;

	fprintf (fp, "}\n\n" );

	fprintf(fp, "Pattern {\n");
	fprintf (fp, "\tW your_timing;\n");

	return NO_ERROR;
	
}

int stil_write_footer() {
	error_t error;
	//Write any unwritten vectors.
	if ( (error = _stil_write_vector() ) != NO_ERROR) {
		return error;	
	}
	//then write the footer, then close the file
	fprintf (fp, "}\n");
	free(filename);
	fclose(fp);

	return NO_ERROR;
}

int stil_setrepeat_mode(char *mode) {
	if (mode == 0) {
		repeat_on = 0;
	} else if ( strcasecmp(mode,"ON") == 0) {
		repeat_on = 1;
	} else if ( strcasecmp(mode,"OFF") == 0) {
		repeat_on = 0;
	} else {
		return UNKOWN_OPTION;
	}
	return NO_ERROR;
}
