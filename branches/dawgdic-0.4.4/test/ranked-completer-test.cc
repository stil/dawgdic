#include <dawgdic/dawg-builder.h>
#include <dawgdic/dictionary-builder.h>
#include <dawgdic/guide-builder.h>
#include <dawgdic/ranked-completer.h>
#include <dawgdic/ranked-guide-builder.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

namespace {

static const std::size_t NUM_KEYS = 1 << 16;
static const std::size_t KEY_LENGTH = 6;
static const int MAX_VALUE = 100;

class Comparer {
 public:
  explicit Comparer(const std::vector<int> &values) : values_(&values) {}

  bool operator()(const dawgdic::ValueType &lhs,
                  const dawgdic::ValueType &rhs) const {
    return (*values_)[lhs] < (*values_)[rhs];
  }

 private:
  const std::vector<int> *values_;
};

void GenerateRandomKeys(std::size_t num_keys, std::size_t length,
                        std::vector<std::string> *keys) {
  std::vector<char> key(length);
  keys->resize(num_keys);
  for (std::size_t key_id = 0; key_id < num_keys; ++key_id) {
    for (std::size_t i = 0; i < length; ++i) {
      key[i] = 'A' + (std::rand() % ('Z' - 'A' + 1));
    }
    (*keys)[key_id].assign(&key[0], length);
  }

  // Sorts keys, and then removes repeated keys.
  std::sort(keys->begin(), keys->end());
  std::vector<std::string>::iterator unique_keys_end =
      std::unique(keys->begin(), keys->end());
  keys->erase(unique_keys_end, keys->end());
}

void GenerateRandomValues(std::size_t num_values, std::vector<int> *values) {
  values->resize(num_values);
  for (std::size_t i = 0; i < num_values; ++i) {
    (*values)[i] = std::rand() % MAX_VALUE;
  }
}

bool BuildDictionary(const std::vector<std::string> &keys,
                     const std::vector<int> &values, dawgdic::Dictionary *dic,
                     dawgdic::RankedGuide *guide) {
  dawgdic::DawgBuilder builder;
  for (std::size_t i = 0; i < keys.size(); ++i) {
    if (!builder.Insert(keys[i].c_str(), static_cast<dawgdic::ValueType>(i))) {
      std::cerr << "error: failed to insert key: "
                << keys[i] << std::endl;
      return false;
    }
  }

  dawgdic::Dawg dawg;
  if (!builder.Finish(&dawg)) {
    std::cerr << "error: failed to finish building Dawg" << std::endl;
    return false;
  }

  if (!dawgdic::DictionaryBuilder::Build(dawg, dic)) {
    std::cerr << "error: failed to build Dictionary" << std::endl;
    return false;
  }

  if (!dawgdic::RankedGuideBuilder::Build(dawg, *dic, guide,
                                          Comparer(values))) {
    std::cerr << "error: failed to build RankedGuide" << std::endl;
    return false;
  }

  return true;
}

bool TestDictionary(const dawgdic::Dictionary &dic,
                    const std::vector<std::string> &keys) {
  for (std::size_t i = 0; i < keys.size(); ++i) {
    dawgdic::ValueType value;
    if (!dic.Find(keys[i].c_str(), &value)) {
      std::cerr << "error: failed to find key: " << keys[i] << std::endl;
      return false;
    } else if (value != static_cast<dawgdic::ValueType>(i)) {
      std::cerr << "error: wrong value: "
        << value << '/' << i << std::endl;
      return false;
    }
  }
  return true;
}

bool TestCompleter(const dawgdic::Dictionary &dic,
                   const dawgdic::RankedGuide &guide,
                   const std::vector<std::string> &keys,
                   const std::vector<int> &values) {
  dawgdic::RankedCompleterBase<Comparer> completer(dic, guide,
                                                   Comparer(values));

  for (char first_label = 'A'; first_label <= 'Z'; ++first_label) {
    dawgdic::BaseType index = dic.root();
    if (!dic.Follow(first_label, &index)) {
      continue;
    }

    int prev_value = MAX_VALUE;
    completer.Start(index, &first_label, 1);
    while (completer.Next()) {
      int value = values[completer.value()];
      if (value > prev_value) {
        std::cerr << "error: invalid value order: "
          << value << " -> " << prev_value << std::endl;
        return false;
      }
      prev_value = value;

//      std::cout << completer.key() << ": " << value << std::endl;
    }
  }

  return true;
}

}  // namespace

int main() {
  // Initializes random number generator's seed.
  std::srand(std::time(NULL));

  std::vector<std::string> keys;
  GenerateRandomKeys(NUM_KEYS, KEY_LENGTH, &keys);
  std::cerr << "no. unique keys: " << keys.size() << std::endl;

  std::vector<int> values;
  GenerateRandomValues(keys.size(), &values);

//  for (std::size_t i = 0; i < keys.size(); ++i)
//    std::cout << i << ": " << keys[i] << ": " << values[i] << std::endl;

  dawgdic::Dictionary dic;
  dawgdic::RankedGuide guide;
  if (!BuildDictionary(keys, values, &dic, &guide)) {
    return 2;
  }

  if (!TestDictionary(dic, keys)) {
    return 3;
  }

  if (!TestCompleter(dic, guide, keys, values)) {
    return 4;
  }

  return 0;
}
