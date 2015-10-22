#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "string_manip.h"
#include "utils.h"

void replace(char **str, char *delim, char *by) {
	char *loc;
	while ( (loc = strstr(*str, delim)) ) {
		strncpy(loc, by, strlen(by) );				
	}	
	trim(str);

}

void ltrim(char **str) {
	if (*str == 0) {
		return;
	}

	int i;
	for (i=0; i<strlen(*str); i++) {
		if ( isspace( (*str)[i]) ) {
			//do nothing.
		} else {
			break;
		}
	}
	*str = &((*str)[i]);
}

void rtrim(char **str) {
	if (*str == 0) {
		return;
	}

	int i=strlen(*str)-1;

	for (; i>=0; i--) {
		if ( isspace( (*str)[i] ) ) {
			(*str)[i] = 0;
		} else {
			break;
		}
	}

}


void trim(char **str) {
	ltrim(str);
	rtrim(str);
} 



#ifdef __STRING_MANIP_TEST__

int main() {
	char *str1 = "((i am m)))ark marinas)";
	char *str = vmalloc(strlen(str1) + 1, 0 );
	strcpy(str, str1);

	replace(&str,"("," ");
	replace(&str,")"," ");
	printf ("%s\n", str);
	return 0;

}

#endif
