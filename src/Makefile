OBJ_PATH := ../build
MAINSOURCE := Main.cpp
SOURCE  := $(wildcard *.cpp mylib/*.cpp)
TARGET  := WebServer
# patsubst 替换通配符，将.cpp的全部替换成.o
OBJS    := $(patsubst %.cpp, $(OBJ_PATH)/%.o,$(SOURCE))

CC      := g++
LIBS    := -lpthread
INCLUDE:= -I./usr/local/lib
CFLAGS  := -std=c++11 -g -Wall -O3 -D_PTHREADS
CXXFLAGS:= $(CFLAGS)

all : $(TARGET)
clean :
	find . -name '*.o' | xargs rm -f
	find ../build -name '*.o' | xargs rm -f

../build/%.o : %.cpp
	g++ -std=c++11 -g -Wall -O3 -D_PTHREADS   -c -o $@ $^

# ../build/Server.o: Server.cpp
# 	g++ -std=c++11 -g -Wall -O3 -D_PTHREADS   -c -o $@ $^

$(TARGET) : $(OBJS) 
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
	find ../build -name '*.o' | xargs rm -f


# $@代表目标，这里是$(TARGET)
# $^表示所有的依赖文件，这里应该是 $(OBJS) 和 Main.o