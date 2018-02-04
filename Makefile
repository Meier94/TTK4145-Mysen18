# Which compiler to use
CC = gcc

# Compiler flags go here.
CFLAGS = -std=gnu11 -g -Wall -Wextra 

# Linker flags go here.
LDFLAGS = -lcomedi -lm -lpthread

# list of sources
ELEVSRC1 = main.c network.c communication.c ./io/io.c
TARGET1 = elevator
ELEVSRC2 = main2.c network.c communication.c
TARGET2 = elevator2
one:$(TARGET1)

two:$(TARGET2)


# Define all object files.
ELEVOBJ1 = $(ELEVSRC1:.c=.o)
ELEVOBJ2 = $(ELEVSRC2:.c=.o)

# rule to link the program
$(TARGET1): $(ELEVOBJ1)
	$(CC) $^ -o $@ $(LDFLAGS)
# rule to link the program
$(TARGET2): $(ELEVOBJ2)
	$(CC) $^ -o $@ $(LDFLAGS)

# Compile: create object files from C source files.
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

# rule for cleaning re-compilable files.
clean1:
	rm -f $(TARGET1) $(ELEVOBJ1)
clean2:
	rm -f $(TARGET2) $(ELEVOBJ2)

rebuild:	clean all

.PHONY: all rebuild clean
