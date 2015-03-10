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

TARGET = header
TEST = test

# the first target is executed by default
default: $(TARGET)
	@echo  \"$(TARGET)\" generated.

$(TARGET): $(TARGET).o
	$(AR) $(ARFLAGS) $(TARGET).a $(TARGET).o

$(TARGET).o: $(TARGET).cpp $(TARGET).h
	$(CC) $(CFLAGS) $(INCLUDES) -c $(TARGET).cpp $(LFLAGS) $(LIBS)

test: $(TEST).cpp $(TARGET).a
	$(CC) $(CFLAGS) -o $(TEST) $(TEST).cpp $(TARGET).a
	@echo  \"$(TEST)\" generated.

clean: 
	$(RM) *.o *~ $(TARGET).a
