
# CSE170 glutapp makefile v2.1 - M. Kallmann 2009 (not tested for a long time)

SOURCES := $(shell echo $(./)*.cpp) $(shell echo $(./)*/*.cpp)
DEPENDS = $(SOURCES:.cpp=.d)
OBJECTS = $(SOURCES:.cpp=.o)
OS = $(shell uname) # For Mac OS detection (by Rolando Yanez, Raymond Harris)
PROGRAM = lab01

#######################################################################

OPTFLAGS = -g  # for debug
#OPTFLAGS =  -O3 -s
CC       = g++
CFLAGS   = -Wall -Wno-format $(OPTFLAGS) -I./ -I/usr/local/include -std=c++11

#ifeq ($(strip $(OS)),Darwin)
	#LDFLAGS = -framework GLUT -framework OpenGL
#else
	LDFLAGS = -L/usr/local/lib -framework GLUT -lGLEW -framework OpenGL
#endif

#######################################################################

all: $(PROGRAM)

$(PROGRAM):$(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJECTS) $(DEPENDS)
	$(RM) $(PROGRAM)
	$(RM) *~

#######################################################################

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.cpp
	$(CC) -MM $(CFLAGS) $< > $@

#######################################################################

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

#######################################################################
