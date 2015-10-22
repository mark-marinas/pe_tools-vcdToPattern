#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "vcd.h"
#include "errors.h"
#include "signal_defs.h"
#include "utils.h"
#include "hooks.h"

static int set_timescale(timescale_t *t, char *timescale) ;
static int hash_set_pin_value( hashtable_t *hash, void *key, void *value) ;
static vcd_t *thisv;

int get_a_section(FILE *vcd_file, char **section) {
	int size = 0;
	char *_section=0;
	int end_section = 0;
	int start_section = 0;

	size_t ssize;
	char *line=0;
	char *orig_line=0;
	error_t err = NO_ERROR;

	while ( getline(&line, &ssize, vcd_file) > 0 ) {
		orig_line = line;
		trim(&line);
		char *tmp = strtok(line, " \t");
		while ( tmp ) {
			trim(&tmp);
			if (strcmp(tmp, "$end") == 0) {
				char *new_section = vmalloc(strlen(_section) + strlen(tmp) + 2, 0);

				strcpy(new_section, _section);
				strcat(new_section, " ");
				strcat(new_section, tmp);

				free(_section) ;
				_section = new_section;

				end_section = 1;
			} else {
				if (tmp[0] == '$') {
					start_section = 1;
				}
				if (_section == 0) {
					_section = vmalloc(strlen(tmp) + 1, 0);
				} else {
					char *new_section = vmalloc(strlen(_section) + strlen(tmp) + 2, 0);
					strcpy(new_section, _section);

					free(_section) ;
					_section = new_section;

					strcat(_section, " ");
				}
				strcat(_section, tmp);
			}	
			tmp = strtok(0, " \t");
		}
		line = 0;
		free(orig_line);
		if (end_section) {
			break;
		}
	}
	if (ssize <= 0) {
		return END_OF_FILE;
	}

	if (start_section == 0 || end_section == 0) {
		return MALFORMATTED_VCD_SECTION;
	}
	*section = _section;
	return NO_ERROR;
}


static int set_timescale(timescale_t *t, char *timescale) {
	long double multiplier;
	char unit[3]; 
	long _time;	
	error_t err;

	if        (strstr(timescale, "ms")) {
		multiplier = 1e-3;
		strcpy(unit, "ms");
	} else if (strstr(timescale, "us")) {
		multiplier = 1e-6;
		strcpy(unit, "us");
	} else if (strstr(timescale, "ns")) {
		multiplier = 1e-9;
		strcpy(unit, "ns");
	} else if (strstr(timescale, "ps")) {
		multiplier = 1e-12;
		strcpy(unit, "ps");
	} else if (strstr(timescale, "fs")) {
		multiplier = 1e-15;
		strcpy(unit, "fs");
	} else if  (strstr(timescale, "s")) {
		multiplier = 1;
		strcpy(unit, " s");
	} else {
		return INVALID_TIMESCALE_UNIT;
	}

	char *tmp = vmalloc(strlen(timescale) + 1, 0) ;
	if (tmp == 0) {
		return MALLOC_ERROR; 
	}

	strcpy(tmp, timescale);

	replace(&tmp, unit, "  ");	
	if ( (_time=atol(tmp)) == 0) {
		free(tmp);
		return INVALID_TIMESCALE_UNIT ;
	}
	t->value = _time;
	t->multiplier = multiplier;

	free(tmp);
	return NO_ERROR;	

}

int parse_timescale(vcd_t *v, char *line) {
	error_t error = NO_ERROR;
	replace(&line, "\n", " ");
	char *token = strtok(line, " \t");

	token = strtok(0," \t");

	if (token == 0) {
		return MALFORMATTED_VCD_SECTION;
	}

	if ( (error = set_timescale( &(v->timescale ), token)) != NO_ERROR) {
		return error;
	}

	token = strtok(0, " \t");
	if (token == 0) {
		return MALFORMATTED_VCD_SECTION;
	} else {
		trim(&token);
		if ( strcmp(token,"$end") != 0) {
			return MALFORMATTED_VCD_SECTION;
		}
	}
	return NO_ERROR;
	
}


int set_vcd_steps(vcd_t *v, char *time) {
	error_t err;
	timescale_t t;


	if ( v->timescale.multiplier == 0) {
		return UNINITIALIZED_TIMESCALE_ERROR;
	}

	if ((err = set_timescale(&t, time) ) != NO_ERROR ) {
		return err;
	}	

	v->steps = t.value * (t.multiplier / v->timescale.multiplier ); 
	v->current_time = 0;

	return NO_ERROR;
}

/* TODO 
	By default, all pins will be read from the vcd file, and will be written on the output file in the same order as they appear in the vcd file.
	Direction are all set to INOUT.
	Add the option to read an input file that defines the order of the pins, as well as the direction. In this case, this function will only be used to make sure that all pins that appear
		in the  vcd file also appears in the input file.

*/
int parse_signal(vcd_t *v, char *line) {
	error_t err = NO_ERROR;
	char *tmp = strtok(line, " \t"); //$var
	      tmp = strtok(0," \t");     //wire
	      tmp = strtok(0," \t");     //width
	trim(&tmp);
	int width = atoi(tmp) ; 
	if (width == 0) {
		return INVALID_WIDTH;	
	}
	      tmp = strtok(0, " \t");    //alias
	      trim(&tmp);
	char *alias = vmalloc(strlen(tmp)+1, 0);
	strcpy(alias, tmp) ;
	       tmp = strtok(0," \t");	 //name
	       trim(&tmp);
	char *name = vmalloc(strlen(tmp) + 1, 0); 
	strcpy(name, tmp);	
	       tmp = strtok(0, " \t");
	       trim(&tmp);
	if ( strcmp(tmp, "$end") != 0 ) {
		err = MALFORMATTED_VCD_SECTION ;
	}

	if (err != NO_ERROR) {
		free(alias);
		free(name);
		return err;
	}

	if (v->pin_file) {
		linklistE_t *head = (v->signals).head;

		while (head) {
			pin_t *pin = (pin_t *) head->data;	
			if ( strcmp(pin->name, name) == 0) {
				if ( pin->width != width) {
					err = MISMATCH_SIGNAL_WIDTH;
				} 			
				pin->alias = alias;	
				v->signal_count++;
				v->all_signal_width += width;
				break;
			}
			head = head->next;
		}
		if (head == 0 ) {
			return UNDEFINED_SIGNAL;
		} else {
			return err;
		}
	}




	pin_t *pin = vmalloc(sizeof(pin_t), 0);
	pin->name = name;
	pin->alias = alias;
	pin->width = width;
	pin->value = 0; 


	pin->direction = INOUT;
	enqueue_linklist(&(v->signals), pin);
	v->signal_count++;
	v->all_signal_width += width;
	return NO_ERROR;
}

int Update_Signal_Value(vcd_t *v, char *value, char *alias) {
	error_t err = NO_ERROR;
	if (value == alias) { //Single bit signal
		char * _value = vmalloc(2,0) ;
		_value[0] = value[0];
		_value[1] = 0;

		char *_alias = vmalloc(strlen(&alias[1]) + 1, 0);
		strcpy(_alias, &alias[1]);

		if ( ( err = v->signalValues.fn_set(&(v->signalValues), _alias, _value ) ) != NO_ERROR ) {
			return UPDATE_HASHTABLE_ERROR;
		}
		free(_value);
		free(_alias);	
	} else {
		if ( ( err = v->signalValues.fn_set(&(v->signalValues), alias, value ) ) != NO_ERROR ) {
			return UPDATE_HASHTABLE2_ERROR;
		}
	}	
	return NO_ERROR;
}

static int hash_set_pin_value( hashtable_t *hash, void *key, void *value) {
        //Is a value existing for this key?
	error_t error;
        pair_t *oldPair = hash->fn_get(hash, key);

        if (oldPair) {
                char *oldPairValue = (char *)oldPair->value;
                char *newPairValue = (char *)value;

		#ifdef _SETPINVAL_HOOK_
			if ( (error = setpinval_hook(thisv, key, newPairValue)) != NO_ERROR ) {
				return error;
			}
		#endif

		#if (1)
		pair_t *__pair = thisv->signalProperties.fn_get ( &(thisv->signalProperties), key) ;
		pin_t *__pin = __pair->value;
	
		if (__pin->direction == OUT) {
			replace(&newPairValue, "1","H");
			replace(&newPairValue, "0","L");
		}
		#endif

                if ( strlen(oldPairValue) > strlen(newPairValue) ) {
			int pad =  strlen(oldPairValue) - strlen(newPairValue) ;
			memset(oldPair->value, 0, strlen(oldPairValue)) ;			
			memset(oldPair->value, '0', pad) ;			
                	strcat(oldPair->value, newPairValue);
			return NO_ERROR;
                } else if ( strlen(oldPairValue) < strlen(newPairValue) ) {
			return SET_PIN_VAL_ERROR;
		}
                strcpy(oldPair->value, newPairValue);
        } else {
                int index = hash->fn_calcHashIndex(key) % hash->size;

                pair_t *newPair = vmalloc(sizeof(pair_t), 0);
                char *_key = vmalloc(strlen((char *)key) + 1, 0);
                char *_value = vmalloc(strlen((char *)value) + 1, 0);

                if ( ( newPair == 0) || (_key == 0) || ( _value == 0) ) {
                        return MALLOC_ERROR;
                }
                strcpy(_key, key);
                strcpy(_value, value);


                newPair->key = _key;
                newPair->value = _value;

                enqueue_linklist( &((hash->table)[index]), (void *)newPair );
        }
        return NO_ERROR;


}

static int hash_set_pin_properties( hashtable_t *hash, void *key, void *value) {
        int index = hash->fn_calcHashIndex(key) % hash->size;

        pair_t *newPair = vmalloc(sizeof(pair_t), 0);
        char *_key = vmalloc(strlen((char *)key) + 1, 0);
        pin_t *_value = vmalloc(sizeof(pin_t), 0);

        if ( ( newPair == 0) || (_key == 0) || ( _value == 0) ) {
       	 	return MALLOC_ERROR;
        }
        strcpy(_key, key);
	memcpy(_value, value, sizeof(pin_t));


       	newPair->key = _key;
       	newPair->value = _value;

       	enqueue_linklist( &((hash->table)[index]), (void *)newPair );

        return NO_ERROR;
}

static int hash_set_name_alias_map( hashtable_t *hash, void *key, void *value) {
        int index = hash->fn_calcHashIndex(key) % hash->size;

        pair_t *newPair = vmalloc(sizeof(pair_t), 0);
        char *_key = vmalloc(strlen((char *)key) + 1, 0);
        char *_value = vmalloc(strlen((char *)value), 0);

        if ( ( newPair == 0) || (_key == 0) || ( _value == 0) ) {
                return MALLOC_ERROR;
        }
        strcpy(_key, key);
        strcpy(_value, value);


        newPair->key = _key;
        newPair->value = _value;

        enqueue_linklist( &((hash->table)[index]), (void *)newPair );

        return NO_ERROR;
}



int parse_dumpvars(vcd_t *v, char *dumpvars_section) {
	error_t err = NO_ERROR;

	//Create the hashtable first.
	if ( (err = init_hashtable(v->signal_count, &(v->signalValues), DEFAULT_FUNCTION, hash_set_pin_value, DEFAULT_FUNCTION ) ) != NO_ERROR ) {
		return HASHTABLE_INIT_ERROR;
	}
	if ( (err = init_hashtable(v->signal_count, &(v->signalProperties), DEFAULT_FUNCTION, hash_set_pin_properties, DEFAULT_FUNCTION ) ) != NO_ERROR ) {
		return HASHTABLE_INIT_ERROR;
	}
	if ( (err = init_hashtable(v->signal_count, &(v->Name2Alias), DEFAULT_FUNCTION, hash_set_name_alias_map, DEFAULT_FUNCTION ) ) != NO_ERROR ) {
		return HASHTABLE_INIT_ERROR;
	}


	linklistE_t *e = v->signals.head;	
	while (e) {
		pin_t *p = e->data;		
		char *value = vmalloc(p->width + 1,'X');
		value[p->width] = 0;

		if ( ( err = v->signalValues.fn_set(&(v->signalValues), p->alias, value ) ) != NO_ERROR ) {
			return HASHTABLE_INIT2_ERROR;
		}
		p->value = ( (pair_t *)v->signalValues.fn_get ( &(v->signalValues), p->alias))->value ;
		if (p->value == 0) {
			return HASHTABLE_INIT3_ERROR;
		}
		//Signal properties.
		if ( ( err = v->signalProperties.fn_set(&(v->signalProperties), p->alias, p ) ) != NO_ERROR ) {
			return HASHTABLE_INIT2_ERROR;
		}

		//Nametoalias map
		if ( ( err = v->Name2Alias.fn_set(&(v->Name2Alias), p->name, p->alias ) ) != NO_ERROR ) {
			return HASHTABLE_INIT2_ERROR;
		}

		free(value);
		e = e->next;
	}

	//Now time to put the dumpvars to the hashtable.
	trim(&dumpvars_section);	
	char *token = strtok(dumpvars_section, " \t"); //$dumpvars
	token = strtok(0, " \t");


	char *signal_alias;
	char *signal_value;
	while (token) {
		trim(&token);
		char marker[2] = { 0 };
		if ( (marker[0] = token[0]) == 'B' || (marker[0] = token[0]) == 'b') {
			replace(&token, marker, " ");
			signal_value = token; 	
			signal_alias = strtok(0, " \t");
			trim(&signal_alias);	
		} else {
			signal_value = token;
			signal_alias = token ;	
		}
		if ( (err = Update_Signal_Value(v, signal_value, signal_alias)) != NO_ERROR ) {	
			return err;
		}

		token = strtok(0, " \t");
		if (strcmp(token, "$end") == 0 ) {
			break;
		}
	}
	

	return NO_ERROR;
}

char *Get_Vector(vcd_t *v) {
	static char *vector_line = 0;
	if (vector_line == 0) {
		vector_line = vmalloc(v->all_signal_width + 1, 0);
	}
	memset(vector_line, 0, v->all_signal_width + 1 );

	linklistE_t *e = v->signals.head;	
	int signal_num = 0;
        while (e) {
                pin_t *p = e->data;
		char *value = p->value; 
		if (signal_num == 0) {
			strcpy(vector_line, value);
			signal_num++;
		} else {
			strcat(vector_line, value);
		}
                e = e->next;
        }	
	return vector_line;
}

int read_pin_file(char *filename, vcd_t *v) {
	FILE *fp = fopen(filename, "r") ;
	if (fp == 0) {
		return INPUT_FILE_OPEN_ERROR;
	}
	v->pin_file = vmalloc(strlen(filename) + 1, 0);
	strcpy(v->pin_file, filename);

	char *line = 0, *orig_line = 0;
	size_t	n;
	char *token = 0;

	while ( getline(&line, &n, fp) > 0 ) {
		orig_line = line;
		trim(&line) ;
		if (strlen(line) == 0 || line[0] == '#' ) {
			line = 0;
			free(orig_line);
			continue;
		}
		char *pin_name = strtok(line, " \t"); 
		trim(&pin_name);
		char *name = vmalloc(strlen(pin_name) + 1, 0);


		strcpy(name, pin_name);

		char *width = strtok(0, " \t");
		trim(&width);
		int  nwidth = 0;
		if (width == 0 || (nwidth = atoi(width) ) == 0 ) {
			fclose(fp);
			return MALFORMATTED_INPUT_PIN_FILE;
		}

		char *direction = strtok(0, " \t");
		trim(&direction);
		signal_direction_t sdirection;
		
		if (direction == 0) {
			fclose(fp);
			return MALFORMATTED_INPUT_PIN_FILE;
		} else {
			if (strcmp(direction, "IN") == 0) {
				sdirection = IN;
			} else if (strcmp(direction, "OUT") == 0) {
				sdirection = OUT;
			} else if (strcmp(direction, "INOUT") == 0) {
				sdirection = INOUT;
			} else {
				sdirection = Z;
			}
		}
		if (strtok(0," \t") != 0) {
			fclose(fp);
			return MALFORMATTED_INPUT_PIN_FILE;
		}

		pin_t *pin = vmalloc(sizeof(pin_t), 0);

		pin->name = name;
		pin->direction = sdirection;
		pin->width = nwidth;

		enqueue_linklist(&(v->signals), (void *)pin);

		line = 0;
		free(orig_line);
	}

	fclose(fp);
	return NO_ERROR;
}

void init_vcd(vcd_t *v) {
	memset(v, 0, sizeof(vcd_t));
	thisv = v;
}




#ifdef __VCD_TESTER__

#include "vcd.h"
#include "errors.h"
//TODO:
#include "stil_writer.h"

int main() {
	FILE *fp = fopen("sample.vcd", "r");
	if (fp == 0) {
		printf ("Failed to open file\n");
	}
	char *section = 0;
	error_t error;

	vcd_t v; 
	init_vcd(&v);

	error = read_pin_file("pinfile.txt", &v);

	if (error == NO_ERROR ) { 
		while ( (error = get_a_section(fp, &section)) == NO_ERROR ) {
			if ( strncmp(section,"$timescale", strlen("$timescale")) == 0) {
				if ( (error = parse_timescale(&v, section)) != NO_ERROR) {
					printf ("o Error in Timescale section\n");
					break;
				} else {
					//printf ("o Info: TimeScale set to %u %d\n", v.timescale.value, ratio );
				}
			} else if ( strncmp(section,"$dumpvars", strlen("$dumpvars")) == 0 ) {
				parse_dumpvars(&v, section);
				break;
			} else if ( strncmp(section, "$var", strlen("$var")) == 0 ) {
				if ( (error = parse_signal(&v, section)) != NO_ERROR ) {
					printf ("o Error Parsing %s\n", section);
					break;
				}
			}
			section = 0;

		}	
	} 

	if (error != NO_ERROR ) { 
		printf ("%s\n", errToString(error));
		return error;
	}


	error = set_vcd_steps(&v, "100ps");  
	printf ("o Info: Steps set to %Lf\n", v.steps );

	char *line = 0;
	char *orig_line = 0;
	size_t s;

	stil_set_outfile(0); //TODO:this should be in a struct, ie. filewriter.fn_setout();
	stil_write_header(&v.signals) ; //TODO: Same as above

	while ( getline(&line, &s, fp) > 0 ) {
		orig_line = line;
		trim(&line);		
		if (strlen(line) == 0) {
			line = 0;
			free(orig_line);
			continue;
		}

		if (line[0] == '#') {
			replace(&line, "#"," ");
			long double _time = atof(line) * v.timescale.value;
			
			char *line2=0, *orig_line2=0;
			ssize_t n;	
			size_t s2;
			int changes = 0;
			int end_of_section = 0;
			
			while ( v.current_time < _time ) {
				if ( (error = stil_write_vector ( Get_Vector(&v),  0 )) != NO_ERROR) { //TODO:
					break;
				}
				v.current_time += v.steps;
			} 		


			while ( (n = getline(&line2, &s2, fp)) > 0 ) {
				orig_line2 = line2;
				trim(&line2);
				if (strlen(line2) == 0) {
					line2 = 0;
					free(orig_line2);
					continue;
				}
				if ( line2[0] == '#' ) {
					end_of_section = 1;
				} else {
                        		char *token = strtok(line2, " \t");
                        		char *signal_alias;
                        		char *signal_value;

                        		char marker[2] = { 0 };
                        		if ( (marker[0] = token[0]) == 'B' || (marker[0] = token[0]) == 'b') {
                                		replace(&token, marker, " ");
                                		signal_value = token;
                                		signal_alias = strtok(0, " \t");
                        		} else {
                                		signal_value = token;
                                		signal_alias = token ;
                        		}
					trim(&signal_value);
                                	trim(&signal_alias);
                        		if ( (error = Update_Signal_Value(&v, signal_value, signal_alias)) != NO_ERROR ) {
                                		break;
                        		}
				}

				line2 = 0;
				free(orig_line2);

				if (end_of_section) {
					//go back; 
					fseek(fp, -1*n, SEEK_CUR) ;
					break;
				}
				changes++;
			}
			if (changes > 0 && _time >= v.current_time ) {
				if ( (error = stil_write_vector(Get_Vector(&v), 0 ))  != NO_ERROR) { //TODO:
					break;
				}

				while ( v.current_time <= _time ) {
					v.current_time += v.steps;
				}
			} 

		}
		line = 0;
		free(orig_line);
	}

	error = stil_write_vector ( Get_Vector(&v), 1 ); //TODO:
	//printf ("%Lf %s\n", v.current_time, Get_Vector(&v) );	
	printf ("\n\n%s\n", errToString(error));

} 
#endif
