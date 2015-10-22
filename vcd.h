#ifndef __VCD_H__
#define __VCD_H__

#include <stdio.h>


#include "hash.h"
#include "linklist.h"

typedef struct {
	long value;
	long double multiplier;
} timescale_t;

typedef struct {
	char 		*pin_file;
	int 		signal_count;
	int 		all_signal_width;
	timescale_t	timescale;
	linklist_t 	signals;
	long double	steps;
	long double	current_time;
	hashtable_t	signalValues; 		//alias-value hash, used, in general, to set/get the value of a pin
	hashtable_t	signalProperties;	//alias-pin_t hash, use to get the pointer to pin_t 
	hashtable_t	Name2Alias;		//name-alias hash, use to get the alias of a given pin name.
} vcd_t;


/*
Description: 
	Initializes a vcd structure.
*/
void init_vcd(vcd_t *v);

/*
Description:
	Returns a vector line, in string.
	Signals are returned in the same order as they appear in the VCD, and all signals are grouped into 1 group.
*/
char *Get_Vector(vcd_t *v) ;

/*
Description
	Parsers a dumpvar section of a vcd
Parameters:
	The dumpvars section should be of format 
	$dumpvars <value><alias> <value><alias> $end
*/
int parse_dumpvars(vcd_t *v, char *dumpvars_section);

/*
Description
	Updates the value of a signal.
*/
int Update_Signal_Value(vcd_t *v, char *value, char *alias);


/*
Description:
	Parses a the timescale from a timescale section
Parameters:
	The line parameter, which refers to the timescale section, should be in the format
	$timescale <time><suffix> $end
	where suffix is any of:
		s
		ms
		us
		ns
		ps
		fs
*/
int parse_timescale(vcd_t *v, char *line);

/*
Description:
	Sets the tester period
Parameters:
	The time parameter should be a string containing <value><suffix>	
	where suffix is any of:
		s
		ms
		us
		ns
		ps
		fs
*/
int set_vcd_steps(vcd_t *v, char *time);

/*
Description:
	Parses a signal value from a line.
Parameters:
	The line parameter should be of the form: 
	b<value> <alias>
	<value><alias>
*/
int parse_signal(vcd_t *v, char *line);

/*
Description:
	Gets a vcd section.    
*/

int get_a_section(FILE *vcd_file, char **section);

#endif
