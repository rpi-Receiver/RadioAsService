TARGET  = radio
VERSION = 0.0.1
CFLAGS  = -Wall -g -DVERSION=\"$(VERSION)\"
LIBS    = -lm -lasound
CC      = gcc
DESTDIR = /usr

SYSTEMCTL:=$(shell which systemctl)

.PHONY: default all clean install uninstall

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

install:
	if test -d "$(DESTDIR)/bin"; then cp $(TARGET) "$(DESTDIR)/bin"; fi
	if test -x "$(SYSTEMCTL)" && ! test -d "$(DESTDIR)/lib/systemd/system"; then mkdir -p "$(DESTDIR)/lib/systemd/system"; fi
	if test -x "$(SYSTEMCTL)" && test -d "$(DESTDIR)/lib/systemd/system"; then install -m0644 radio.service $(DESTDIR)/lib/systemd/system && $(SYSTEMCTL) daemon-reload; fi

uninstall:
	[ ! -f $(DESTDIR)/lib/systemd/system/radio.service ] || rm -f radio.service $(DESTDIR)/lib/systemd/system/radio.service
