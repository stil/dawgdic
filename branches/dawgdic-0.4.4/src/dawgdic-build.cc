#include <dawgdic/dawg-builder.h>
#include <dawgdic/dictionary-builder.h>
#include <dawgdic/guide-builder.h>
#include <dawgdic/ranked-guide-builder.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

namespace {

class CommandOptions {
 public:
  CommandOptions()
    : help_(false), tab_(false), guide_(false), ranked_(false),
      lexicon_file_name_(), dic_file_name_() {}

  // Reads options.
  bool help() const {
    return help_;
  }
  bool tab() const {
    return tab_;
  }
  bool guide() const {
    return guide_;
  }
  bool ranked() const {
    return ranked_;
  }
  const std::string &lexicon_file_name() const {
    return lexicon_file_name_;
  }
  const std::string &dic_file_name() const {
    return dic_file_name_;
  }

  bool Parse(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
      // Parses options.
      if (argv[i][0] == '-' && argv[i][1] != '\0') {
        for (int j = 1; argv[i][j] != '\0'; ++j) {
          switch (argv[i][j]) {
            case 'h': {
              help_ = true;
              break;
            }
            case 't': {
              tab_ = true;
              break;
            }
            case 'g': {
              guide_ = true;
              break;
            }
            case 'r': {
              ranked_ = true;
              break;
            }
            default: {
              // Invalid option.
              return false;
            }
          }
        }
      } else if (lexicon_file_name_.empty()) {
        lexicon_file_name_ = argv[i];
      } else if (dic_file_name_.empty()) {
        dic_file_name_ = argv[i];
      } else {
        // Too many arguments.
        return false;
      }
    }

    // Uses default settings for file names.
    if (lexicon_file_name_.empty()) {
      lexicon_file_name_ = "-";
    }
    if (dic_file_name_.empty()) {
      dic_file_name_ = "-";
    }
    return true;
  }

  static void ShowUsage(std::ostream *output) {
    *output << "Usage: - [Options] [LexiconFile] [DicFile]\n"
               "\n"
               "Options:\n"
               "  -h  display this help and exit\n"
               "  -t  handle tab as separator\n"
               "  -g  build dictionary with guide\n"
               "  -r  build dictionary with ranked guide\n";
    *output << std::endl;
  }

private:
  bool help_;
  bool tab_;
  bool guide_;
  bool ranked_;
  std::string lexicon_file_name_;
  std::string dic_file_name_;

  // Disallows copies.
  CommandOptions(const CommandOptions &);
  CommandOptions &operator=(const CommandOptions &);
};

// Builds a dawg from a sorted lexicon.
bool BuildDawg(std::istream *lexicon_stream,
               dawgdic::Dawg *dawg, bool tab_on) {
  dawgdic::DawgBuilder dawg_builder;

  // Reads keys from an input stream and inserts them into a dawg.
  std::string key;
  std::size_t key_count = 0;
  while (std::getline(*lexicon_stream, key)) {
    std::string::size_type delim_pos = std::string::npos;
    if (tab_on) {
      delim_pos = key.find_first_of('\t');
    }

    if (delim_pos == std::string::npos) {
      if (!dawg_builder.Insert(key.c_str())) {
        std::cerr << "error: failed to insert key: "
                  << key << std::endl;
        return false;
      }
    } else {
      static const dawgdic::ValueType MAX_VALUE =
          std::numeric_limits<dawgdic::ValueType>::max();

      // Fixes an invalid record value.
      long long record = std::strtoll(key.c_str() + delim_pos + 1, NULL, 10);
      dawgdic::ValueType value =
          static_cast<dawgdic::ValueType>(record);
      if (record < 0) {
        std::cerr << "warning: negative value is replaced by 0: "
                  << record << std::endl; 
        value = 0;
      } else if (record > MAX_VALUE) {
        std::cerr << "warning: too large value is replaced by "
                  << MAX_VALUE << ": " << record << std::endl; 
        value = MAX_VALUE;
      }

      if (!dawg_builder.Insert(key.c_str(), delim_pos, value)) {
        std::cerr << "error: failed to insert key: "
                  << key << std::endl;
        return false;
      }
    }

    if (++key_count % 10000 == 0) {
      std::cerr << "no. keys: " << key_count << '\r';
    }
  }

  dawg_builder.Finish(dawg);

  std::cerr << "no. keys: " << key_count << std::endl;
  std::cerr << "no. states: "
            << dawg->num_of_states() << std::endl;
  std::cerr << "no. transitions: "
            << dawg->num_of_transitions() << std::endl;
  std::cerr << "no. merged states: "
            << dawg->num_of_merged_states() << std::endl;
  std::cerr << "no. merging states: "
            << dawg->num_of_merging_states() << std::endl;
  std::cerr << "no. merged transitions: "
            << dawg->num_of_merged_transitions() << std::endl;

  return true;
}

// Builds a dictionary from a dawg.
bool BuildDictionary(const dawgdic::Dawg &dawg, dawgdic::Dictionary *dic) {
  dawgdic::BaseType num_of_unused_units = 0;
  if (!dawgdic::DictionaryBuilder::Build(dawg, dic, &num_of_unused_units)) {
    std::cerr << "error: failed to build Dictionary" << std::endl;
    return false;
  }
  double unused_ratio = 100.0 * num_of_unused_units / dic->size();

  std::cerr << "no. elements: " << dic->size() << std::endl;
  std::cerr << "no. unused elements: " << num_of_unused_units
            << " (" << unused_ratio << "%)" << std::endl;
  std::cerr << "dictionary size: " << dic->total_size() << std::endl;

  return true;
}

// Builds a ranked guide from a dawg and its dictionary.
bool BuildRankedGuide(const dawgdic::Dawg &dawg,
                      const dawgdic::Dictionary &dic,
                      dawgdic::RankedGuide *guide) {
  if (!dawgdic::RankedGuideBuilder::Build(dawg, dic, guide)) {
    std::cerr << "failed to build RankedGuide" << std::endl;
    return false;
  }

  std::cerr << "no. units: " << guide->size() << std::endl;
  std::cerr << "guide size: " << guide->total_size() << std::endl;

  return true;
}

// Builds a guide from a dawg and its dictionary.
bool BuildGuide(const dawgdic::Dawg &dawg,
                const dawgdic::Dictionary &dic, dawgdic::Guide *guide) {
  if (!dawgdic::GuideBuilder::Build(dawg, dic, guide)) {
    std::cerr << "failed to build Guide" << std::endl;
    return false;
  }

  std::cerr << "no. units: " << guide->size() << std::endl;
  std::cerr << "guide size: " << guide->total_size() << std::endl;

  return true;
}

}  // namespace

int main(int argc, char *argv[]) {
  CommandOptions options;
  if (!options.Parse(argc, argv)) {
    CommandOptions::ShowUsage(&std::cerr);
    return 1;
  } else if (options.help()) {
    CommandOptions::ShowUsage(&std::cerr);
    return 0;
  }

  const std::string &lexicon_file_name = options.lexicon_file_name();
  const std::string &dic_file_name = options.dic_file_name();

  std::istream *lexicon_stream = &std::cin;
  std::ostream *dic_stream = &std::cout;

  // Opens a lexicon file.
  std::ifstream lexicon_file;
  if (lexicon_file_name != "-") {
    lexicon_file.open(lexicon_file_name.c_str(), std::ios::binary);
    if (!lexicon_file) {
      std::cerr << "error: failed to open LexiconFile: "
                << lexicon_file_name << std::endl;
      return 1;
    }
    lexicon_stream = &lexicon_file;
  }

  // Opens a dictionary file.
  std::ofstream dic_file;
  if (dic_file_name != "-") {
    dic_file.open(dic_file_name.c_str(), std::ios::binary);
    if (!dic_file)   {
      std::cerr << "error: failed to open DicFile: "
                << dic_file_name << std::endl;
      return 1;
    }
    dic_stream = &dic_file;
  }

  dawgdic::Dawg dawg;
  if (!BuildDawg(lexicon_stream, &dawg, options.tab())) {
    return 1;
  }

  dawgdic::Dictionary dic;
  if (!BuildDictionary(dawg, &dic)) {
    return 1;
  }

  if (!dic.Write(dic_stream)) {
    std::cerr << "error: failed to write Dictionary" << std::endl;
    return 1;
  }

  // Builds a guide.
  if (options.ranked()) {
    dawgdic::RankedGuide guide;
    if (!BuildRankedGuide(dawg, dic, &guide)) {
      return 1;
    }
    if (!guide.Write(dic_stream)) {
      std::cerr << "error: failed to write RankedGuide" << std::endl;
      return 1;
    }
  } else if (options.guide()) {
    dawgdic::Guide guide;
    if (!BuildGuide(dawg, dic, &guide)) {
      return 1;
    }
    if (!guide.Write(dic_stream)) {
      std::cerr << "error: failed to write Guide" << std::endl;
      return 1;
    }
  }

  return 0;
}
