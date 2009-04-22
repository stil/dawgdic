#include <fstream>
#include <iostream>

#include "nanika/ios/line-reader.h"
#include "nanika/dawgdic/dawg-builder.h"
#include "nanika/dawgdic/dictionary-builder.h"

// Builds a dictionary from a set of keys.
bool BuildDictionary(const char *key_file_name,
	nanika::dawgdic::Dictionary *dic)
{
	std::cerr << "input: " << key_file_name << std::endl;

	// Opens an input file.
	std::ifstream key_file(key_file_name, std::ios::binary);
	if (!key_file.is_open())
	{
		std::cerr << "error: failed to open file: "
			<< key_file_name << std::endl;
		return false;
	}

	// Reads keys and inserts them into a dawg.
	nanika::ios::LineReader key_reader(&key_file);
	nanika::dawgdic::DawgBuilder dawg_builder;
	const char *key;
	while (key_reader.Read(&key))
	{
		if (!dawg_builder.Insert(key))
		{
			std::cerr << "error: failed to insert key: " << key << std::endl;
			return false;
		}
	}
	nanika::dawgdic::Dawg dawg;
	nanika::dawgdic::SizeType num_of_merged_states;
	dawg_builder.Finish(&dawg, &num_of_merged_states);
	std::cout << "no. states: " << dawg.size() << std::endl;
	std::cout << "no. merged states: " << num_of_merged_states << std::endl;

	// Builds a dictionary from a dawg.
	if (!nanika::dawgdic::DictionaryBuilder::Build(dawg, dic))
	{
		std::cerr << "error: failed to build dictionary" << std::endl;
		return false;
	}
	std::cout << "no. elements: " << dic->size() << std::endl;
	std::cout << "dictionary size: " << dic->total_size() << std::endl;

	return true;
}

// Writes a dictionary to a file.
bool SaveDictionary(const nanika::dawgdic::Dictionary &dic,
	const char *dic_file_name)
{
	std::cerr << "output: " << dic_file_name << std::endl;

	// Creates an output file.
	std::ofstream dic_file(dic_file_name, std::ios::binary);
	if (!dic_file.is_open())
	{
		std::cerr << "error: failed to open file: "
			<< dic_file_name << std::endl;
		return false;
	}

	if (!dic.Write(&dic_file))
	{
		std::cerr << "error: failed to write dictionary to file: "
			<< dic_file_name << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char *argv[])
{
	// Checks arguments.
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " KeyFile DicFile" << std::endl;
		return 1;
	}

	const char *key_file_name = argv[1];
	const char *dic_file_name = argv[2];

	nanika::dawgdic::Dictionary dic;
	if (!BuildDictionary(key_file_name, &dic))
		return 1;

	if (!SaveDictionary(dic, dic_file_name))
		return 1;

	return 0;
}
