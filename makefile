CC=gcc
CFLAGS=-g -Wall -std=c89

TARGET=buildrooms

all: $(TARGET)

$(TARGET): greendan.buildrooms.c
	$(RM)r $(TARGET) greendan.room*
	$(CC) $(CFLAGS) -o $(TARGET) greendan.buildrooms.c

clean:
	$(RM)r $(TARGET) greendan.room*
