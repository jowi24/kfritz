LIB = libtcpclient++.a
OBJS = TcpClient++.o

CXXFLAGS = -g -O2 -Wall

### libpthread++
INCLUDES += -I../libpthread++
#LIBS += libpthread++/libpthread++.a

all: $(LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	@-echo Built $(LIB).

clean:
	@-rm $(OBJS) $(LIB)



