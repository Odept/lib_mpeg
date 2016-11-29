CC = g++
CFLAGS  = -std=c++11 -Wall -Wextra -Werror
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
TARGET_HEADER = header
TARGET_STREAM = stream
TEST = test

# Common dependencies
DEPS = $(TARGET).h common.h

# the first target is executed by default
default: $(TARGET).a

# Lib
$(TARGET).a: $(TARGET).o $(TARGET_STREAM).o $(TARGET_HEADER).o
	@echo "#" generate \"$(TARGET)\" library
	$(AR) $(ARFLAGS) $(TARGET).a $(TARGET_HEADER).o $(TARGET_STREAM).o $(TARGET).o

$(TARGET).o: $(TARGET).cpp $(TARGET).h $(TARGET_STREAM).h $(TARGET_HEADER).h
	@echo "#" generate \"$(TARGET)\"
	$(CC) $(CFLAGS) -c $(INCLUDES) $(TARGET).cpp $(LFLAGS) $(LIBS)

# Stream
$(TARGET_STREAM).o: $(TARGET_STREAM).cpp $(TARGET_STREAM).h $(TARGET).h $(DEPS)
	@echo "#" generate \"$(TARGET_STREAM)\"
	$(CC) $(CFLAGS) -c $(INCLUDES) $(TARGET_STREAM).cpp $(LFLAGS) $(LIBS)

# Header
$(TARGET_HEADER).o: $(TARGET_HEADER).cpp $(TARGET_HEADER).h $(DEPS)
	@echo "#" generate \"$(TARGET_HEADER)\"
	$(CC) $(CFLAGS) -c $(INCLUDES) $(TARGET_HEADER).cpp $(LFLAGS) $(LIBS)

# Test
test: $(TEST).cpp $(TARGET).a $(TARGET).h
	@echo "#" generate \"$(TEST)\"
	$(CC) $(CFLAGS) -o $(TEST) $(TEST).cpp $(TARGET).a

clean: 
	$(RM) *.o *~ $(TARGET).a $(TEST)
	$(RM) -r $(TEST).dSYM
