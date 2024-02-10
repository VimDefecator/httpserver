CXX := clang++
CXXFLAGS := -std=c++20
CPPFLAGS := -MMD -MP
LDFLAGS := -lpthread

ALLDIRS := common
ALLUNITS := test imgbrd serverloop tcplistener taskpool http html common/args

TARGETS := test
UNITS_test := test imgbrd serverloop tcplistener taskpool http html common/args

include Makefile.template
