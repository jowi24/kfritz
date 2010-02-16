LIB = libfritz++.a
OBJS = CallList.o Config.o Fonbooks.o Fonbook.o FonbookManager.o FritzClient.o FritzFonbook.o Listener.o Tools.o LocalFonbook.o Nummerzoeker.o OertlichesFonbook.o md5.o

CXXFLAGS ?= -g -O2 -Wall -fPIC

### libpthread++
INCLUDES += -I../libpthread++
#LIBS += libpthread++/libpthread++.a

### libtcpclient++
INCLUDES += -I../libtcpclient++
#LIBS += libtcpclient++/tcpclient++.a

all: $(LIB)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

md5.o: md5.c
	$(CC) $(CCFLAGS) -c $(DEFINES) $(INCLUDES) $<

$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	@-echo Built $(LIB).

clean:
	@-rm $(LIB) $(OBJS) $(DEPFILE)
	
# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)