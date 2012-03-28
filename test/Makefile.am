AM_CXXFLAGS = -Wall -I$(top_srcdir)/src/

TESTS = \
  dic-test.sh \
  guide-test.sh \
  ranked-test.sh

TESTS_ENVIRONMENT = \
  TOP_SRCDIR="$(top_srcdir)" \
  TOP_BUILDDIR="$(top_builddir)"

noinst_PROGRAMS = \
  ranked-completer-test

ranked_completer_test_SOURCES = ranked-completer-test.cc

dist_noinst_DATA = $(TESTS) \
  lexicon \
  query \
  dic-answer \
  guide-answer \
  ranked-answer
