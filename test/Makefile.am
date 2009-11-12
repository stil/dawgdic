AM_CXXFLAGS = -Wall -I../src/

TESTS = \
	dic-test.sh \
	guide-test.sh \
	ranked-test.sh

noinst_PROGRAMS = \
	ranked-completer-test

ranked_completer_test_SOURCES = ranked-completer-test.cc

dist_noinst_DATA = $(TESTS) \
	lexicon \
	query \
	dic-answer \
	guide-answer \
	ranked-answer
