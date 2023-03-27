BIN:=tuya_iot_daemon
SRC:=$(wildcard *.c)
OBJ:=$(SRC:.c=.o)
CC:=gcc
CPPFLAGS:=-I$(CURDIR)#../utils -I../interface -I../include
#LDFLAGS:=-L../lib \
-Wl,-rpath,../lib,--no-as-needed
LDLIBS:=\
	-llink_core \
	-lplatform_port \
	-lmiddleware_implementation \
	-lutils_modules \
	-largp \
	-lubus \
	-lubox \
	-lblobmsg_json

.PHONY: all clean $(BIN)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
clean:
	$(RM) $(OBJ) $(BIN) example.txt