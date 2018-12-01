.SUFFIXES: .c .o

SRCDIR=src/ 
INC=include/ 
LIBS=lib/

SRCS:=$(wildcard src/*.cpp) 
OBJS:=$(SRCS:.cpp=.o) 

# main source file 
TARGET_SRC:=$(SRCDIR)main.cpp 
TARGET_OBJ:=$(SRCDIR)main.o 

# Include more files if you write another source file. 
SRCS_FOR_LIB:=$(SRCS) 
OBJS_FOR_LIB:=$(SRCS_FOR_LIB:.cpp=.o) 

CFLAGS+= -g -fPIC -I $(INC) 
CPPFLAGS+= -g -fPIC -I $(INC) 
TARGET=main

all: $(TARGET) 
$(TARGET): $(OBJS) 	
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lpthread 	
	make static_library 	
	$(CC) $(CFLAGS) -o $@ $^ -L $(LIBS) -lbpt -lpthread 

clean: 	
	rm $(TARGET) $(TARGET_OBJ) $(LIBS)* 

library: 
	g++ -shared -Wl,-soname,libbpt.so -o $(LIBS)libbpt.so $(OBJS_FOR_LIB)

static_library: 	
	ar cr $(LIBS)libbpt.a $(OBJS_FOR_LIB) 


















