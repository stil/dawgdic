AM_CXXFLAGS = -Wall -I$(top_srcdir)/src/

TESTS = \
  dictionary-test.sh \
  completer-test.sh \
  ranked-completer-test.sh

TESTS_ENVIRONMENT = \
  TOP_SRCDIR="$(top_srcdir)" \
  TOP_BUILDDIR="$(top_builddir)"

noinst_PROGRAMS = \
  ranked-completer-test

ranked_completer_test_SOURCES = ranked-completer-test.cc

dist_noinst_DATA = $(TESTS) \
  lexicon \
  query \
  dictionary-answer \
  completer-answer \
  ranked-completer-answer
