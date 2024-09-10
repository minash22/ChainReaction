
CC = gcc


CFLAGS = -Wvla -Wextra -D_GNU_SOURCE -w


SRCS = main.c


TARGET = pop


all: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)
	@echo "Compilation successful. Executable $(TARGET) generated."

run: $(TARGET)
	./$(TARGET)


clean:
	rm -f $(TARGET)
