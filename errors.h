#ifndef __ERRORS_H__
#define __ERRORS_H__

typedef enum {
	 NO_ERROR
	,MALLOC_ERROR
	,KEY_NOT_FOUND_ERROR
	,INVALID_TIMESCALE_UNIT
	,MALFORMATTED_VCD_SECTION
	,INVALID_WIDTH
	,HASHTABLE_INIT_ERROR
	,HASHTABLE_INIT2_ERROR
	,HASHTABLE_INIT3_ERROR
	,UPDATE_HASHTABLE_ERROR
	,UPDATE_HASHTABLE2_ERROR
	,UNINITIALIZED_TIMESCALE_ERROR
	,SET_PIN_VAL_ERROR
	,OUTPUT_FILE_OPEN_ERROR
	,INPUT_FILE_OPEN_ERROR
	,VECTOR_LENGTH_ERROR
	,OUTPUT_WRITE_ERROR
	,MALFORMATTED_INPUT_PIN_FILE
        ,MISMATCH_SIGNAL_WIDTH
	,UNDEFINED_SIGNAL
	,UNKNOWN_DIRECTION
	,EXCESS_OPTION
	,UNKOWN_OPTION
	,END_OF_FILE
} error_t;




char *errToString(error_t errCode);







#endif
