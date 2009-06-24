#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "dawgdic/dictionary.h"
#include "dawgdic/dictionary-explorer.h"

// Loads a dictionary file.
bool LoadDictionary(const char *dic_file_name, dawgdic::Dictionary *dic)
{
	std::cerr << "output: " << dic_file_name << std::endl;

	// Opens an input file.
	std::ifstream dic_file(dic_file_name, std::ios::binary);
	if (!dic_file.is_open())
	{
		std::cerr << "error: failed to open file: "
			<< dic_file_name << std::endl;
		return false;
	}

	if (!dic->Read(&dic_file))
	{
		std::cerr << "error: failed to read dictionary from file: "
			<< dic_file_name << std::endl;
		return false;
	}

	return true;
}

// Example of finding prefix keys from each line of an input text.
// There are two ways to perform prefix matching.
// One uses dawgdic::DictionaryExplorer,
// and the other uses only dawgdic::Dictionary.
bool FindKeys(const dawgdic::Dictionary &dic, std::istream *input)
{
	std::vector<std::size_t> lengths;
	std::string line;
	while (std::getline(*input, line))
	{
		// The following two examples give the same results.
#ifdef USE_DICTIONARY_EXPLORER
		dawgdic::DictionaryExplorer explorer(dic);
		for (std::size_t i = 0; i < line.length(); ++i)
		{
			if (!explorer.Follow(line[i]))
				break;

			// Reads a value.
			if (explorer.has_value())
				lengths.push_back(i + 1);
		}
#else  // USE_DICTIONARY_EXPLORER
		dawgdic::BaseType index = dic.root();
		for (std::size_t i = 0; i < line.length(); ++i)
		{
			if (!dic.Follow(line[i], &index))
				break;

			// Reads a value.
			if (dic.has_value(index))
				lengths.push_back(i + 1);
		}
#endif  // USE_DICTIONARY_EXPLORER

		if (lengths.empty())
			std::cout << line << ": not found" << std::endl;
		else
		{
			std::cout << line << ": found, num = " << lengths.size() << ',';
			for (std::size_t i = 0; i < lengths.size(); ++i)
				std::cout << ' ' << lengths[i];
			std::cout << std::endl;
		}
		lengths.clear();
	}

	return true;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " DicFile" << std::endl;
		return 1;
	}

	const char *dic_file_name = argv[1];

	dawgdic::Dictionary dic;
	if (!LoadDictionary(dic_file_name, &dic))
		return 1;

	FindKeys(dic, &std::cin);

	return 0;
}
