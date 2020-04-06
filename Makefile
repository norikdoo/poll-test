# compiler to use
#CC = gcc
CC = $(CROSS_COMPILE)gcc

# compiler flags:
CFLAGS = -g -Wall

# linker flags
#LDFLAGS = -static

# target
TARGET = poll-test
  
all: $(TARGET)
  
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LDFLAGS)
	file $(TARGET)
clean:
	rm -f *.o $(TARGET)
