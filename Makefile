CC = g++
CFLAGS  = -std=c++14 -Wall -Wextra -Werror
CFLAGS += -g3
#release: CFLAGS += -O3

AR = ar
ARFLAGS = rvs

INCLUDES =
# -I/home/newhall/include  -I../include

LFLAGS =
# -L/home/newhall/lib  -L../lib

LIBS =
# -lmylib -lm

#SRCS = header.cpp

# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#OBJS = $(SRCS:.c=.o)

TARGET = mpeg
HEADER = header
STREAM = stream
TEST = test

# Common dependencies
DEPS = $(TARGET).h $(HEADER).h $(HEADER)_raw.h common.h

# the first target is executed by default
default: $(TARGET).a

# Archive
$(TARGET).a: $(TARGET).o $(STREAM).o $(HEADER).o
	# Delete an old archive to avoid strange warnings
	rm -f $(TARGET).a
	@echo "#" generate \"$(TARGET)\" archive
	$(AR) $(ARFLAGS) $(TARGET).a $(HEADER).o $(STREAM).o $(TARGET).o

$(TARGET).o: $(TARGET).cpp $(STREAM).h $(DEPS)
	@echo "#" generate \"$(TARGET)\"
	$(CC) $(CFLAGS) -c $(INCLUDES) $(TARGET).cpp $(LFLAGS) $(LIBS)

# Stream
$(STREAM).o: $(STREAM).cpp $(STREAM).h $(DEPS)
	@echo "#" generate \"$(STREAM)\"
	$(CC) $(CFLAGS) -c $(INCLUDES) $(STREAM).cpp $(LFLAGS) $(LIBS)

# Header
$(HEADER).o: $(HEADER).cpp $(DEPS)
	@echo "#" generate \"$(HEADER)\"
	$(CC) $(CFLAGS) -c $(INCLUDES) $(HEADER).cpp $(LFLAGS) $(LIBS)

# Test
test: $(TEST).cpp $(TARGET).a $(TARGET).h
	@echo "#" generate \"$(TEST)\"
	$(CC) $(CFLAGS) -o $(TEST) $(TEST).cpp $(TARGET).a

clean: 
	$(RM) *.o *~ $(TARGET).a $(TEST)
	$(RM) -r $(TEST).dSYM
