CC = g++
CFLAGS  = -g -Wall

AR = ar
ARFLAGS = rvs

INCLUDES =
# -I/home/newhall/include  -I../include

LFLAGS =
# -L/home/newhall/lib  -L../lib

LIBS =
# -lmylib -lm

#SRCS = header.cpp

DEPS = common.h

# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#OBJS = $(SRCS:.c=.o)

TARGET = mpeg
TARGET_HEADER = header
TARGET_STREAM = stream
TEST = test

# the first target is executed by default
default: $(TARGET).a

$(TARGET).a: $(TARGET_HEADER).o $(TARGET_STREAM).o
	$(AR) $(ARFLAGS) $(TARGET).a $(TARGET_HEADER).o $(TARGET_STREAM).o
	@echo "###" \"$(TARGET)\" generated

# Header
$(TARGET_HEADER).o: $(TARGET_HEADER).cpp $(TARGET_HEADER).h $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(TARGET_HEADER).cpp $(LFLAGS) $(LIBS)
	@echo "###" \"$(TARGET_HEADER)\" generated

# Stream
$(TARGET_STREAM).o: $(TARGET_STREAM).cpp $(TARGET_STREAM).h $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(TARGET_STREAM).cpp $(LFLAGS) $(LIBS)
	@echo "###" \"$(TARGET_STREAM)\" generated

# Test
test: $(TEST).cpp $(TARGET).a
	$(CC) $(CFLAGS) -o $(TEST) $(TEST).cpp $(TARGET).a
	@echo "###" \"$(TEST)\" generated

clean: 
	$(RM) *.o *~ $(TARGET).a $(TEST)
