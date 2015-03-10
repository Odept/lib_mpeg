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

# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#OBJS = $(SRCS:.c=.o)

TARGET = mpeg
TARGET_HEADER = header
TARGET_STREAM = stream
TEST = test

# the first target is executed by default
default: $(TARGET)
	@echo  \"$(TARGET)\" generated.

$(TARGET): $(TARGET_HEADER).o $(TARGET_STREAM).o
	$(AR) $(ARFLAGS) $(TARGET).a $(TARGET_HEADER).o $(TARGET_STREAM).o

$(TARGET_HEADER).o: $(TARGET_HEADER).cpp $(TARGET_HEADER).h
	$(CC) $(CFLAGS) $(INCLUDES) -c $(TARGET_HEADER).cpp $(LFLAGS) $(LIBS)

$(TARGET_STREAM).o: $(TARGET_STREAM).cpp $(TARGET_STREAM).h
	$(CC) $(CFLAGS) $(INCLUDES) -c $(TARGET_STREAM).cpp $(LFLAGS) $(LIBS)

test: $(TEST).cpp $(TARGET).a
	$(CC) $(CFLAGS) -o $(TEST) $(TEST).cpp $(TARGET).a
	@echo  \"$(TEST)\" generated.

clean: 
	$(RM) *.o *~ $(TARGET).a $(TEST)
