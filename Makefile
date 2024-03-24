CXX := clang++
CXXFLAGS := -std=c++20
CPPFLAGS := -MMD -MP

TARGETS := test
UNITS_test := test imgbrd serverloop tcplistener taskpool http html common/args
LDFLAGS_test := -lpthread

include Makefile.template
