#ifndef __STRING_MANIP_H__
#define __STRING_MANIP_H__


#define match(str1, str2)       ( !strcmp(str1, str2) )

/*
WARNING:
	Do not pass a constant char * .
Description:
	ltrim removes any leading white space from a string.
	rtrim removes any trailing white space from a string.
	trim  removes both leading and trailing white space.
Parameters:
	A double pointer to the string to be modified.
*/
void ltrim(char **str);
void rtrim(char **str);
void  trim(char **str);

/*
WARNING:
	This is a simple implementation of replace. The what and the by parameters should be of the same length.
Description:
	Replaces from str all occurrences of delim with by. Then trim, just in case it results in a leading or trailing space.
Parameters:	
	str - the string to search
	delim - the string that needs to be replaced.
	by - the replacement.
*/
void replace(char **str, char *delim, char *by);


#endif
