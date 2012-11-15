#! /bin/sh

build_bin="${TOP_BUILDDIR:-..}/src/dawgdic-build"
find_bin="${TOP_BUILDDIR:-..}/src/dawgdic-find"
test_dir="${TOP_SRCDIR:-..}/test"

"${test_dir}/dawg-builder-test"
if [ $? -ne 0 ]
then
  exit 1
fi
