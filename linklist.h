#ifndef __LINKLIST_H__
#define __LINKLIST_H__

typedef struct linklistE_t linklistE_t;
typedef struct linklist_t  linklist_t ;

struct linklistE_t {
        void *data;
        linklistE_t *next;
};

struct linklist_t {
        linklistE_t *head;
        linklistE_t *tail;
};


void init_linklist(linklist_t *l);
void enqueue_linklist(linklist_t *l, void *d) ;

/*
Description: Deque the content of a linklist.
Parameters:
	l - linklist to deque 
Return:
	pointer to the *data* element of the linklistE structure.
	Note that the first call to deque will return the *data* of the head.
	and subsequent calls to dequeue will return the *data* of the next element.
	the *data* will not be reset to the HEAD, until it reaches the tail.
	So the proper use of this is to ENQUEUE all elements, then DEQUEUE until it hits the tail.
	Another thing to note is that it does clean up the linklist when it reaches the TAIL, so all *data* should be dynamically allocated
*/ 

void *dequeue_linklist(linklist_t *l);


/*


*/
void *gettail_linklist(linklist_t *l);


#endif
