CC = clang
CFLAGS = -Wall -Wextra -O2
TARGET = echokill

SRC = main.c icmp.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
        $(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
        $(CC) $(CFLAGS) -c $< -o $@

clean:
        rm -f $(OBJ) $(TARGET)

.PHONY: all clean