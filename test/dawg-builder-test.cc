#include <cassert>
#include <iostream>
#include <string>

#include <dawgdic/dawg-builder.h>
#include <dawgdic/dictionary-builder.h>

int main() {
  dawgdic::DawgBuilder dawg_builder;
  assert(dawg_builder.Insert("apple"));
  assert(dawg_builder.Insert("cherry"));
  assert(!dawg_builder.Insert("banana"));
  assert(dawg_builder.Insert("durian"));
  assert(!dawg_builder.Insert("green\0apple", 11, 0));
  assert(dawg_builder.Insert("green\0apple"));
  assert(dawg_builder.Insert("mandarin orange", 8, 0));
  assert(dawg_builder.Insert("mandarin"));

  dawgdic::Dawg dawg;
  dawg_builder.Finish(&dawg);

  dawgdic::Dictionary dawg_dic;
  dawgdic::DictionaryBuilder::Build(dawg, &dawg_dic);

  assert(dawg_dic.Contains("apple"));
  assert(dawg_dic.Contains("cherry"));
  assert(dawg_dic.Contains("durian"));
  assert(dawg_dic.Contains("green"));
  assert(dawg_dic.Contains("mandarin"));

  return 0;
}
