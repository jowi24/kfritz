CXXFLAGS = -g -ggdb -O0 -Wall -fPIC

INCLUDES += -I.. -I../../libpthread++/

all: TcpClientExample.o
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -lpthread -lrt TcpClientExample.o ../libtcpclient++.a ../../libpthread++/libpthread++.a -o TcpClientExample 
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

