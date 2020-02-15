CC=gcc
CXX=clang++
RM=rm -f
CPPFLAGS=-Wall -std=c++17 -Iinclude -Iexternal
ADDED_CPPFLAGS=
LDFLAGS=-lstdc++fs
LDLIBS=

MAIN=conway.cpp
TEST=tests/test.cpp tests/test-bitwise.cpp
SRCS=
OBJS=$(subst .cpp,.o,$(SRCS))

release: ADDED_CPPFLAGS=-O3 -flto=thin

all release: conway test

test: $(SRCS) $(TEST)
	$(CXX) -o $@ $(TEST) $(SRCS) $(LDLIBS) $(LDFLAGS) $(CPPFLAGS) $(ADDED_CPPFLAGS)

conway: $(SRCS) $(MAIN)
	$(CXX) -o $@ $(MAIN) $(SRCS) $(LDLIBS) $(LDFLAGS) $(CPPFLAGS) $(ADDED_CPPFLAGS)

clean:
	$(RM) $(subst .cpp, ,$(SRCS))
	$(RM) $(subst .cpp, ,$(MAIN))
	$(RM) test
	$(RM) $(subst .cpp,.o,$(SRCS))
	$(RM) $(subst .cpp,.o,$(MAIN))
	$(RM) $(subst .cpp,.o,$(TEST))