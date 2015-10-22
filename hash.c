#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hash.h"
#include "errors.h"
#include "utils.h"

static int hash_CalcIndex_simple(void *key) ;
static int hash_set_simple( hashtable_t *hash, void *key, void *value) ;
static void *hash_get_simple( hashtable_t *hash, void *key);


int init_hashtable
(
int size, 
hashtable_t *hash,  
int (*fn_calcHashIndex)(void *key),
int (*fn_set)( hashtable_t *hash, void *key, void *value),
void *(*fn_get)( hashtable_t *hash, void *key) 
) 

{
	hash->size = size;
	if ( (hash->table = vmalloc(sizeof(linklist_t)*size, 0)) == 0) {
		return MALLOC_ERROR;
	}


	if ( fn_calcHashIndex == DEFAULT_FUNCTION ) {
		hash->fn_calcHashIndex = hash_CalcIndex_simple; 
	} else {
		hash->fn_calcHashIndex = fn_calcHashIndex;
	}

	if (fn_set == DEFAULT_FUNCTION) {
		hash->fn_set = hash_set_simple ;
	} else {
		hash->fn_set = fn_set;
	}

	if (fn_get == DEFAULT_FUNCTION) {
		hash->fn_get = hash_get_simple ;
	} else {
		hash->fn_get = fn_get;
	}

	//Initialize the hash table.
	int i;
	for (i=0; i<size; i++) {
		init_linklist( &((hash->table)[0]) );
	}	
	return NO_ERROR;	
}


int hash_CalcIndex_simple(void *key) {
	int nkey=0;
	char *k = (char *)key;
	while (*k) {
		nkey += *k;	
		k++;
	} 

	return nkey;
}


int hash_set_simple( hashtable_t *hash, void *key, void *value) {
	//Is a value existing for this key?
	pair_t *oldPair = hash_get_simple(hash, key);

	if (oldPair) {
		char *oldPairValue = (char *)oldPair->value;
		char *newPairValue = (char *)value;
		if ( strlen(oldPairValue) != strlen(newPairValue) ) {
			free(oldPair->value);
			oldPair->value = vmalloc( strlen((char *)value)+1, 0);
			if (oldPair->value == 0) {
				return MALLOC_ERROR;
			}
		}
		strcpy(oldPair->value, newPairValue);
	} else {
		int index = hash_CalcIndex_simple(key) % hash->size;	

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

void *hash_get_simple( hashtable_t *hash, void *key) {
	char *cKey = (char *)key;
	int index = hash_CalcIndex_simple(cKey) % hash->size;	

	linklist_t * l= &((hash->table)[index]);
	if (l == 0) {
		return 0;
	}
	if ( (l->head == 0 ) ) {
		return 0;
	}

	linklistE_t *head = l->head;
	pair_t *pair;
	while ( head != 0 ) {
		pair = (pair_t *) head->data;	
		char *_key = (char *) pair->key;
		if ( strcmp(cKey, _key) == 0 ) {
			break;
		}
		head = head->next;
	}			
	if (head == 0) {
		return 0;
	}	

	return ( (void *) head->data ) ;

}


#ifdef __HASH_TEST__

int main() {
	hashtable_t sigs;
	char *key = vmalloc(3, 0);
	char *value = vmalloc(3, 0);

	init_hashtable(10, &sigs, DEFAULT_FUNCTION, DEFAULT_FUNCTION, DEFAULT_FUNCTION);
	/* Test1. Try key ab, value cd */	
	strcpy(key,"ab");
	strcpy(value,"cd");

	sigs.fn_set(&sigs, key, value);

	pair_t *p = sigs.fn_get(&sigs, key);
	char *val = (char *)p->value;
	if ( strcmp(val, value) != 0 ) {
		printf ("Failed test 1: Expected %d, Received %s\n", value, val);
	} else {
		printf ("Test 1 passed: key = %s, value = %s\n", key, val);
	}

	/* Test2. Try value ef. This should overwrite the old value, but still should be able to get correct value. */
	strcpy(value,"ef");
	sigs.fn_set(&sigs, key, value);

	p = sigs.fn_get(&sigs, key);
	val = (char *)p->value;
	if ( strcmp(val, value) != 0 ) {
		printf ("Failed test 2: Expected %d, Received %s\n", value, val);
	} else { 
		printf ("Test 2 passed: key = %s, value = %s\n", key, val);
	}

	// Test3. Key ba, value gh. This will use the same linklist as Test 1, but it should still give correct value. 
	strcpy(key,"ba");
	strcpy(value,"gh");
	sigs.fn_set(&sigs, key, value);

	p = sigs.fn_get(&sigs, key);
	val = (char *)p->value;
	if ( strcmp(val, value) != 0 ) {
		printf ("Failed test 3: Expected %d, Received %s\n", value, val);
	} else { 
		printf ("Test 3 passed: key = %s, value = %s\n", key, val);
	}

	// Test4. Get value for key ba, should return ef.
	strcpy(key,"ba");
	strcpy(value,"gh");
	p = sigs.fn_get(&sigs, key);
	val = (char *)p->value;
	if ( strcmp(val, value) != 0 ) {
		printf ("Failed test 4: Expected %d, Received %s\n", value, val);
	} else { 
		printf ("Test 4 passed: key = %s, value = %s\n", key, val);

	}
	// Test5. Get value for key ab, should return ef.
	strcpy(key,"ab");
	strcpy(value,"ef");
	p = sigs.fn_get(&sigs, key);
	val = (char *)p->value;
	if ( strcmp(val, value) != 0 ) {
		printf ("Failed test 5: Expected %d, Received %s\n", value, val);
	} else { 
		printf ("Test 5 passed: key = %s, value = %s\n", key, val);
	}

	return 0;
}




#endif
