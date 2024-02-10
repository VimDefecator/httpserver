ALLDIRS := common
ALLUNITS := test imgbrd serverloop tcplistener taskpool http html common/args

TARGETS := test
UNITS_test := test imgbrd serverloop tcplistener taskpool http html common/args

include Makefile.template
