#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "nanika/dawgdic/dictionary.h"
#include "nanika/dawgdic/explorer.h"

// Loads a dictionary file.
bool LoadDictionary(const char *dic_file_name,
	nanika::dawgdic::Dictionary *dic)
{
	std::cerr << "output: " << dic_file_name << std::endl;

	// Creates an output file.
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

// Finds keys.
bool FindKeys(const nanika::dawgdic::Dictionary &dic, std::istream *input)
{
	std::vector<std::size_t> lengths;
	nanika::dawgdic::Explorer explorer(dic);

	std::string line;
	while (std::getline(*input, line))
	{
		// Finds keys from prefixes of a line.
		explorer.Reset();
		lengths.clear();
		for (std::size_t i = 0; i < line.length(); ++i)
		{
			if (!explorer.Follow(line[i]))
				break;

			// Reads a value.
			if (explorer.has_value())
				lengths.push_back(i + 1);
		}

		if (lengths.empty())
			std::cout << line << ": not found" << std::endl;
		else
		{
			std::cout << line << ": found, num = " << lengths.size() << ',';
			for (std::size_t i = 0; i < lengths.size(); ++i)
				std::cout << ' ' << lengths[i];
			std::cout << std::endl;
		}
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

	nanika::dawgdic::Dictionary dic;
	if (!LoadDictionary(dic_file_name, &dic))
		return 1;

	FindKeys(dic, &std::cin);

	return 0;
}
