#! /bin/sh

build_bin="${TOP_BUILDDIR:-..}/src/dawgdic-build"
find_bin="${TOP_BUILDDIR:-..}/src/dawgdic-find"
test_dir="${TOP_SRCDIR:-..}/test"

if [ ! -f "$build_bin" ]
then
  echo "error: $build_bin: not found"
  exit 1
fi

if [ ! -f "$find_bin" ]
then
  echo "error: $build_bin: not found"
  exit 1
fi

## Builds a dictionary from a lexicon.
$build_bin -gt "${test_dir}/lexicon" lexicon.dic
if [ $? -ne 0 ]
then
  exit 1
fi

## Finds prefix keys from a lexicon.
$find_bin -g lexicon.dic < "${test_dir}/query" > guide-result
if [ $? -ne 0 ]
then
  exit 1
fi

## Checks the result.
cmp guide-result "${test_dir}/guide-answer"
if [ $? -ne 0 ]
then
  exit 1
fi

## Removes temporary files.
rm -f lexicon.dic guide-result
