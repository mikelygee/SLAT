all: main unit_tests doc

clean:
	rm -f *.so *.o main unit_tests

CC=g++
CFLAGS=-g -Wall -Werror -fbounds-check -Warray-bounds -std=gnu++11 -DBOOST_ALL_DYN_LINK -fPIC
LDFLAGS=-lgsl -lgslcblas -lm -lboost_log -lboost_thread -lboost_system -lpthread

LIBSRCS=functions.cpp relationships.cpp maq.cpp
LIBOBJS=$(LIBSRCS:.cpp=.o)

UNIT_SRCS = functions_test.cpp relationships_test.cpp unit_test.cpp maq_test.cpp
UNIT_OBJS = $(UNIT_SRCS:.cpp=.o)

functions.o: functions.cpp functions.h
	g++ -c $(CFLAGS) -o $@ $<
relationships.o: relationships.cpp relationships.h functions.h maq.h
	g++ -c $(CFLAGS) -o $@ $<
maq.o: maq.cpp maq.h 
	g++ -c $(CFLAGS) -o $@ $<
libslat.so: functions.o relationships.o maq.o
	g++ -fPIC -shared -Wl,-soname,libslat.so -o libslat.so $(LIBOBJS) ${LDFLAGS}

main.o: main.cpp functions.h relationships.h maq.h libslat.so
	g++ -c $(CFLAGS) -o $@ $<
main: main.o libslat.so
	g++ -fPIC main.o -L. -lslat -o main ${LDFLAGS}

functions_test.o: functions_test.cpp functions.h
	g++ -c $(CFLAGS) -o $@ $<
relationships_test.o: relationships_test.cpp relationships.h functions.h
	g++ -c $(CFLAGS) -o $@ $<
maq_test.o: maq_test.cpp maq.h relationships.h functions.h
	g++ -c $(CFLAGS) -o $@ $<
unit_test.o: unit_test.cpp
	g++ -c $(CFLAGS) -o $@ $<
unit_tests: unit_test.o maq_test.o relationships_test.o functions_test.o libslat.so
	g++ -fPIC $(UNIT_OBJS) -L. -lslat -o unit_tests ${LDFLAGS} -lboost_unit_test_framework

doc: $(OBJS) $(HEADERS)
	doxygen
