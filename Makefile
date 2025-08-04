CC := gcc
CFLAGS := -Wall

all: part_vulcan

part_vulcan: part_vulcan.c
	$(CC) $(CFLAGS) -o $@ $<

clean::
	$(RM) part_vulcan
