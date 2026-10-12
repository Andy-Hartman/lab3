######################################################################
# "Hello World" Makefile
######################################################################
# (C) 2022--2026 Dr. Adam C. Champion
######################################################################
# Based on Prof. Neil Kirby's Systems I Makefile plus shell and awk
# scripts. Also, I used:
#
# https://stackoverflow.com/questions/54082715/how-to-add-lm-ldflags-
#   correctly-to-this-makefile ,
#
# which I retrieved 19 March 2022.
######################################################################

######################################################################
# Global variables.
######################################################################
# Lab 0 links NO extra libraries with the final executable. *However*,
# future labs will require libraries such as Pthreads (POSIX threads).
# Define these libraries via the LIBS variable.
######################################################################
CC=gcc
LD=ld
WARNS=-Wall -Wextra -pedantic
CFLAGS=-g3 ${WARNS}
LIBS=

all: tags headers shell

# This builds visual symbol (.vs) files and the header files.
headers: *.c tags
	./headers.sh

# Tags (for C code) are too handy not to keep up to date.
# Vim users: use ctags with vim via Control-]  (the default)
# Emacs users: replace "ctags -R ." with "etags -R .".
tags: *.c
	ctags -R .


# This is a link rule, we have a universal compile rule down below
# Output is the target of the rule : -o $@
# I want to link all of the dependencies: $^
shell: shell.o
	${CC} -g -o $@ $^ ${LIBS}

shell.o: shell.c
	${CC} -g -c $<

# This is our master compiler rule to generate .o files.
# It needs all 4 warnings (see WARNS variable defined above)
%.o:%.c *.h
	${CC} ${CFLAGS} -c $< -o $@