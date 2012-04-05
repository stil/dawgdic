#include <dawgdic/completer.h>
#include <dawgdic/dictionary.h>
#include <dawgdic/ranked-completer.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

class CommandOptions {
 public:
  CommandOptions()
    : help_(false), guide_(false), ranked_(false),
      dic_file_name_(), lexicon_file_name_() {}

  // Reads options.
  bool help() const {
    return help_;
  }
  bool guide() const {
    return guide_;
  }
  bool ranked() const {
    return ranked_;
  }
  const std::string &dic_file_name() const {
    return dic_file_name_;
  }
  const std::string &lexicon_file_name() const {
    return lexicon_file_name_;
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
      } else if (dic_file_name_.empty()) {
        dic_file_name_ = argv[i];
      } else if (lexicon_file_name_.empty()) {
        lexicon_file_name_ = argv[i];
      } else {
        // Too many arguments.
        return false;
      }
    }

    // Uses default settings for file names.
    if (dic_file_name_.empty()) {
      dic_file_name_ = "-";
    }
    if (lexicon_file_name_.empty()) {
      lexicon_file_name_ = "-";
    }
    return true;
  }

  static void ShowUsage(std::ostream *output) {
    *output << "Usage: - [Options] [DicFile] [LexiconFile]\n"
               "\n"
               "Options:\n"
               "  -h  display this help and exit\n"
               "  -g  load dictionary with guide\n"
               "  -r  load dictionary with ranked guide\n";
    *output << std::endl;
  }

private:
  bool help_;
  bool guide_;
  bool ranked_;
  std::string dic_file_name_;
  std::string lexicon_file_name_;

  // Disallows copies.
  CommandOptions(const CommandOptions &);
  CommandOptions &operator=(const CommandOptions &);
};

// Example of finding prefix keys from each line of an input text.
void FindPrefixKeys(const dawgdic::Dictionary &dic, std::istream *input) {
  std::string line;
  while (std::getline(*input, line)) {
    std::cout << line << ':';

    dawgdic::BaseType index = dic.root();
    for (std::size_t i = 0; i < line.length(); ++i) {
      if (!dic.Follow(line[i], &index)) {
        break;
      }

      // Reads a value.
      if (dic.has_value(index)) {
        std::cout << ' ';
        std::cout.write(line.c_str(), i + 1);
        std::cout << " = " << dic.value(index) << ';';
      }
    }
    std::cout << std::endl;
  }
}

// Example of completing ranked keys from each line of an input text.
template <typename COMPLETER_TYPE, typename GUIDE_TYPE>
void CompleteKeys(const dawgdic::Dictionary &dic,
                  const GUIDE_TYPE &guide, std::istream *input) {
  COMPLETER_TYPE completer(dic, guide);
  std::string line;
  while (std::getline(*input, line)) {
    std::cout << line << ':';

    dawgdic::BaseType index = dic.root();
    if (dic.Follow(line.c_str(), line.length(), &index)) {
      completer.Start(index);
      while (completer.Next()) {
        std::cout << ' ' << line << completer.key()
                  << " = " << completer.value();
      }
    }
    std::cout << std::endl;
  }
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

  const std::string &dic_file_name = options.dic_file_name();
  const std::string &lexicon_file_name = options.lexicon_file_name();

  std::istream *dic_stream = &std::cin;
  std::istream *lexicon_stream = &std::cin;

  // Opens a dictionary file.
  std::ifstream dic_file;
  if (dic_file_name != "-") {
    dic_file.open(dic_file_name.c_str(), std::ios::binary);
    if (!dic_file) {
      std::cerr << "error: failed to open DicFile: "
                << dic_file_name << std::endl;
      return 1;
    }
    dic_stream = &dic_file;
  }

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

  dawgdic::Dictionary dic;
  if (!dic.Read(dic_stream)) {
    std::cerr << "error: failed to read Dictionary" << std::endl;
    return 1;
  }

  if (options.ranked()) {
    dawgdic::RankedGuide guide;
    if (!guide.Read(dic_stream)) {
      std::cerr << "error: failed to read RankedGuide" << std::endl;
      return 1;
    }
    CompleteKeys<dawgdic::RankedCompleter>(dic, guide, lexicon_stream);
  } else if (options.guide()) {
    dawgdic::Guide guide;
    if (!guide.Read(dic_stream)) {
      std::cerr << "error: failed to read Guide" << std::endl;
      return 1;
    }
    CompleteKeys<dawgdic::Completer>(dic, guide, lexicon_stream);
  } else {
    FindPrefixKeys(dic, lexicon_stream);
  }

  return 0;
}
