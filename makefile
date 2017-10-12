CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g 
LDFLAGS=-g

SRCS=diana.cpp
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
DEPEND=DnsStats.h pcap_reader.h linktype.h DnsStatHash.h

all: diana

diana: $(OBJS)
	$(CXX) $(LDFLAGS) -o pcap4dns $(OBJS)
 
clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) tool
