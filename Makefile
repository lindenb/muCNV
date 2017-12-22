#
# Generic Makefile for muCNV
#
#   Author: Goo Jun (goo.jun@uth.tmc.edu)

TARGET=  bin/muCNV
SRCS := $(wildcard muCNV/*.cpp)
OBJS := $(addprefix obj/,$(notdir $(SRCS:.cpp=.o)))

CFLAGS= -g -Wall -O2 -fPIC -std=c++0x
DFLAGS= -D_FILE_OFFSET_BITS=64
CC= gcc
CXX= g++ 
INCLUDES= -I./tclap-1.2.1/include -I./htslib

DEPS := $(OBJS:%.o=%.d)

# .SUFFIXES:.cpp .o

all: $(TARGET) 

$(TARGET) : $(OBJS)
	$(CXX) $^ -lhts -lm -lz -o $@ 

-include $(DEPS)

obj/%.o: muCNV/%.cpp 
	$(CXX) -c $< $(CFLAGS) $(DFLAGS) $(INCLUDES) -MMD -o $@

clean :
	-rm -f $(OBJS) $(TARGET) *~
