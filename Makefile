CC = gcc
CC_FLAGS = -Wall -Werror

REMOVE = rm -f

all : build/vgd_parse.o

unit_testing : build/vgd_parse.o
	$(CC) $(CC_FLAGS) unit_tests.c build/vgd_parse.o -o build/unit_tests

build/vgd_parse.o :
	$(CC) $(CC_FLAGS) -c src/vgd_log_parse.c -o build/vgd_parse.o

clean :
	$(REMOVE) build/*



.PHONY: all unit_testing clean