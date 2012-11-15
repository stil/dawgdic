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
$build_bin -t "${test_dir}/lexicon" lexicon.dic
if [ $? -ne 0 ]
then
  exit 1
fi

## Finds prefix keys from a lexicon.
$find_bin lexicon.dic < "${test_dir}/query" > dictionary-result
if [ $? -ne 0 ]
then
  exit 1
fi

## Checks the result.
cmp dictionary-result "${test_dir}/dictionary-answer"
if [ $? -ne 0 ]
then
  exit 1
fi

## Removes temporary files.
rm -f lexicon.dic dictionary-result
