#ifndef __HASH__    
#define __HASH__    

/*
	Hash table where every element is a linklist, thus collision is handled by chaining.
*/


#include "linklist.h"


#define DEFAULT_FUNCTION	0


typedef struct {
	void *key;
	void *value;
} pair_t ;


struct hashtable_t{
	int size;
	linklist_t *table;		
	int (*fn_calcHashIndex)(void *key);
	int (*fn_set)(struct hashtable_t *hash, void *key, void *value);
	void *(*fn_get)(struct hashtable_t *hash,void *key);
};

typedef struct hashtable_t hashtable_t;


/*
Description:
	Initialize a hash table.
Parameters:
	size - Size of the hash table
	hash - the hashtable.
	fn_calcHashIndex - function pointer that handles the algorithm in calculating the index to the hash table.
	fn_set - function pointer that handles keying a key-value pair into the hash table.
	fn_get - function pointer that handles getting the value given a key. 

	Using DEFAULT_FUNCTION will use the default functions. These default functions assumes a string key, string value.
*/
int init_hashtable( int size, hashtable_t *hash,  int (*fn_calcHashIndex)(void *key), int (*fn_set)(hashtable_t *hash, void *key, void *value), void *(*fn_get)(hashtable_t *hash, void *key)) ;




#endif

