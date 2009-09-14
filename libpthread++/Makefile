LIB = libpthread++.a
OBJS = PThread++.o

CXXFLAGS = -g -O2 -Wall -fPIC

all: $(LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	@-echo Built $(LIB).

clean:
	@-rm $(OBJS) $(LIB)