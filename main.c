#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "errors.h"





typedef struct {
	char *in;
	char *out;
	char *pinfile;
	char *period;
	char *repeat;
} opts_t;

#define match(str1, str2)	(strcasecmp(str1, str2) == 0)


void showUsage(char **argv) {
	printf ("Usage\n");
	printf ("\t%s -in <input_vcd> -out <output_pattern> -pinfile <pin_mapping_file> -period <tester_period> -repeat <ON/OFF>\n", argv[0]);
	printf ("\tWhere:\n");
	printf ("\t\tInput vcd file - vcd file to process \n");
	printf ("\t\toutput pattern file, ie .stil file \n");
	printf ("\t\t(optional) pinfile, defines the order of pins in the output. Must much the signal names in the vcd\n");
	printf ("\t\tperiod, tester period\n");
	printf ("\t\t(optional)repeat, use loop for vectors that didnt change between cycles\n");

	printf ("\texample: %s -in sample.vcd -out sample.stil -pinfile pinfile.txt -period 100ps -repeat ON\n", argv[0]);
	
}

int parse_options (char argc, char *argv[], opts_t *opts) {
	int i;
	error_t err = NO_ERROR;	

	memset(opts, 0, sizeof(opts_t) );
	for (i=1; i<argc; i++) {
		if ( match(argv[i],"-in" ) ) {
			opts->in = ( (i+1)<argc ) ? argv[i+1]:(char *)1;
			i++;
		} else if ( match(argv[i], "-out") ) {
			opts->out = ( (i+1)<argc ) ? argv[i+1]:(char *)1;
			i++;
		} else if ( match(argv[i], "-pinfile") ) {
			opts->pinfile = ( (i+1)<argc ) ? argv[i+1]:(char *)1;
			i++;
		} else if ( match(argv[i], "-period") ) {
			opts->period = ( (i+1)<argc ) ? argv[i+1]:(char *)1;
			i++;
		} else if ( match(argv[i], "-repeat") ) {
			opts->repeat = ( (i+1)<argc ) ? argv[i+1]:(char *)1;
			i++;
		} else if ( match(argv[i], "-h") ) {
			showUsage(argv);
			exit(0);
		} else {
			return UNKOWN_OPTION;
		}
	}
	if ( (opts->in <= 1 ) || (opts->out <= 1) || (opts->pinfile == 1) || (opts->period <= 1) || (opts->repeat == 1) ) {
		return EXCESS_OPTION;
	}

	return NO_ERROR;
}


#include "vcd.h"
#include "pattern_writer.h"
#include "stil_writer.h"


int main(char argc, char *argv[]) {
	opts_t options;
	error_t error;

	//Parse command line inputs.
	error = parse_options(argc, argv, &options);	
	if (error != NO_ERROR) {
		printf ("ERROR: (%d) %s\n",error,  errToString(error) );
		return error;
	}	

	//Initialize the vcd reader.
	vcd_t v;
	init_vcd(&v);

	if ( options.pinfile ) {
		error = read_pin_file(options.pinfile, &v) ;
		if (error != NO_ERROR) {
			printf ("ERROR: (%d) %s\n",error,  errToString(error) );
			return error;
		}
		printf ("o Info: Finished Reading pinfile %s\n", options.pinfile);
	}

	//Parse initial vcd sections.
	char *section = 0;	
        FILE *fp = fopen(options.in, "r");
        if (fp == 0) {
                printf ("Failed to open input vcd file %s\n", options.in );
        }
 	while ( (error = get_a_section(fp, &section)) == NO_ERROR ) {
       		if ( strncmp(section,"$timescale", strlen("$timescale")) == 0) {
       	        	if ( (error = parse_timescale(&v, section)) != NO_ERROR) {
                                break;
                        } else {
                               //printf ("o Info: TimeScale set to %d %s\n", v.timescale.value, timescale_unit );
                        }
                } else if ( strncmp(section,"$dumpvars", strlen("$dumpvars")) == 0 ) {
                        error = parse_dumpvars(&v, section);
                        break;
                } else if ( strncmp(section, "$var", strlen("$var")) == 0 ) {
                        if ( (error = parse_signal(&v, section)) != NO_ERROR ) {
                                break;
                        }
                }
		if (section != 0) {
			free(section);
                	section = 0;
		}
       	}
    	printf ("o Info: Finished reading sections\n");	 
       	if (section != 0) {
		free(section);
       		section = 0;
       	}

	if (error != NO_ERROR) {
		 printf ("ERROR: (%d) %s\n",error,  errToString(error) );
	}

	//Now we can set the vcd steps since we already know the VCDs timescale.
        error = set_vcd_steps(&v, options.period);
        if (error != NO_ERROR) {
                printf ("ERROR: (%d) %s\n",error,  errToString(error) );
                return error;
        }
	printf ("o Info: Steps set to %Lf\n", v.steps );


        //Now we are almost ready to write the pattern.
	//But first, initialize the writer.
        pattern_writer_t pattern_writer;
        error = init_pattern_writer(&pattern_writer, options.out);
        if (error != NO_ERROR) {
                printf ("ERROR: (%d) %s\n",error,  errToString(error) );
                return error;
        }
	printf ("o Info: Pattern writer initialized\n");

	//Then the repeat mode.
	error = pattern_writer.fn_set_repeat(options.repeat);
        if (error != NO_ERROR) {
                printf ("ERROR: (%d) %s\n",error,  errToString(error) );
                return error;
        }
	printf ("o Info: Repeat Mode set to %s\n", options.repeat) ;

	//Then write the pattern header.
	error = pattern_writer.fn_header_writer(&v.signals);
	if (error != NO_ERROR) {
                printf ("ERROR: (%d) %s\n",error,  errToString(error) );
                return error;
	}


	printf ("o Info: Pattern Header written\n");



	//This is the main loop that parses the value changes.
        char *line = 0;
        char *orig_line = 0;
        size_t s;

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
                                if ( (error = pattern_writer.fn_vector_writer ( Get_Vector(&v),  0 )) != NO_ERROR) { 
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
                                if ( (error = pattern_writer.fn_vector_writer(Get_Vector(&v), 0 ))  != NO_ERROR) {
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

	//Force to write the vector if there is any remaining unwritten vectors.
	error = pattern_writer.fn_footer_writer();

	if (error != NO_ERROR) {
        	printf ("ERROR: (%d) %s\n",error,  errToString(error) );
		return error; 
	} 
	printf ("Pattern %s written successfully\n", options.out);
}
