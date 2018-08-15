
all:	libsmtp.a

libsmtp.a:	libsmtp.o
	ar -r libsmtp.a libsmtp.o

libsmtp.o:	libsmtp.c
	gcc -c -O2 -o libsmtp.o libsmtp.c
