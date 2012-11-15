AM_CXXFLAGS = -Wall -I$(top_srcdir)/src/

TESTS = \
  dawg-builder-test.sh \
  dictionary-test.sh \
  completer-test.sh \
  ranked-completer-test.sh

TESTS_ENVIRONMENT = \
  TOP_SRCDIR="$(top_srcdir)" \
  TOP_BUILDDIR="$(top_builddir)"

noinst_PROGRAMS = \
  dawg-builder-test \
  ranked-completer-test

dawg_builder_test_SOURCES = dawg-builder-test.cc
ranked_completer_test_SOURCES = ranked-completer-test.cc

dist_noinst_DATA = $(TESTS) \
  lexicon \
  query \
  dictionary-answer \
  completer-answer \
  ranked-completer-answer
