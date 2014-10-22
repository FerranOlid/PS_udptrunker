#Makefile
PROGNAME ?= trunker

all: trunker.c
	gcc trunker.c -o ${PROGNAME}

clean:
	rm ${PROGNAME}


