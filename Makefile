CC=gcc 

SRCS_VCD=errors.c  hash.c  linklist.c  main.c  pattern_writer.c  stil_writer.c  string_manip.c  utils.c  vcd.c hooks.c
HEADS_VCD=errors.h  hash.h  linklist.h  main.h  pattern_writer.h  stil_writer.h  string_manip.h  utils.h  vcd.h	hooks.h
EXE_VCD=vcdToPattern.exe

SRCS_HASH=hash.c linklist.c utils.c
HEADS_HASH=hash.h linklist.h utils.h
EXE_HASH=hash.exe


vcd2pat:
	$(CC) $(SRCS_VCD) -o $(EXE_VCD) -D_SETPINVAL_HOOK_


hash:
	$(CC) $(SRCS_HASH) -o $(EXE_HASH) -D__HASH_TEST__

clean:
	rm -f *.exe	
