#ifndef NANIKA_DAWGDIC_DICTIONARY_H
#define NANIKA_DAWGDIC_DICTIONARY_H

#include <iostream>
#include <vector>

#include "base-types.h"
#include "dictionary-unit.h"

namespace nanika {
namespace dawgdic {

// Dictionary class for retrieval and binary I/O.
class Dictionary
{
public:
	Dictionary() : units_(0), size_(0), units_buf_() {}
	explicit Dictionary(std::vector<DictionaryUnit> *units_buf)
		: units_(0), size_(0), units_buf_() { set_units_buf(units_buf); }

	const DictionaryUnit *units() const { return units_; }
	SizeType size() const { return size_; }
	SizeType total_size() const { return sizeof(DictionaryUnit) * size_; }

	// Reads a dictionary from an input stream.
	bool Read(std::istream *input)
	{
		BaseType base_size;
		if (!input->read(reinterpret_cast<char *>(&base_size),
			sizeof(BaseType)))
			return false;

		SizeType size = base_size;
		std::vector<DictionaryUnit> units_buf(size);
		if (!input->read(reinterpret_cast<char *>(&units_buf[0]),
			sizeof(DictionaryUnit) * size))
			return false;

		set_units_buf(&units_buf);
		return true;
	}

	// Writes a dictionry to an output stream.
	bool Write(std::ostream *output) const
	{
		BaseType base_size = static_cast<BaseType>(size_);
		if (!output->write(reinterpret_cast<const char *>(&base_size),
			sizeof(BaseType)))
			return false;

		if (!output->write(reinterpret_cast<const char *>(units_),
			sizeof(DictionaryUnit) * size_))
			return false;

		return true;
	}

	// Exact matching.
	bool Contains(const CharType *key) const
	{
		BaseType index = 0;
		DictionaryUnit unit = units_[index];
		return Traverse(key, &index, &unit) && unit.has_leaf();
	}
	bool Contains(const CharType *key, SizeType length) const
	{
		BaseType index = 0;
		DictionaryUnit unit = units_[index];
		return Traverse(key, length, &index, &unit) && unit.has_leaf();
	}

	// Exact matching.
	ValueType Find(const CharType *key) const
	{
		BaseType index = 0;
		DictionaryUnit unit = units_[index];
		if (Traverse(key, &index, &unit) && unit.has_leaf())
			return units_[index ^ unit.offset()].value();
		return -1;
	}
	ValueType Find(const CharType *key, SizeType length) const
	{
		BaseType index = 0;
		DictionaryUnit unit = units_[index];
		if (Traverse(key, length, &index, &unit) && unit.has_leaf())
			return units_[index ^ unit.offset()].value();
		return -1;
	}
	bool Find(const CharType *key, ValueType *value) const
	{
		BaseType index = 0;
		DictionaryUnit unit = units_[index];
		if (!Traverse(key, &index, &unit) || !unit.has_leaf())
			return false;
		*value = units_[index ^ unit.offset()].value();
		return true;
	}
	bool Find(const CharType *key, SizeType length, ValueType *value) const
	{
		BaseType index = 0;
		DictionaryUnit unit = units_[index];
		if (!Traverse(key, length, &index, &unit) || !unit.has_leaf())
			return false;
		*value = units_[index ^ unit.offset()].value();
		return true;
	}

	// Maps memory with its size.
	void Map(const void *address)
	{
		Clear();
		units_ = reinterpret_cast<const DictionaryUnit *>(
			static_cast<const BaseType *>(address) + 1);
		size_ = *static_cast<const BaseType *>(address);
	}

	// Maps memory with its size.
	void Map(const void *address, SizeType size)
	{
		Clear();
		units_ = static_cast<const DictionaryUnit *>(address);
		size_ = size;
	}

	// Initializes a dictionary.
	void Clear()
	{
		units_ = 0;
		size_ = 0;
		std::vector<DictionaryUnit>(0).swap(units_buf_);
	}

	// Swaps dictionaries.
	void Swap(Dictionary *dic)
	{
		std::swap(units_, dic->units_);
		std::swap(size_, dic->size_);
		units_buf_.swap(dic->units_buf_);
	}

	// Shrinks a vector.
	void Shrink()
	{
		if (units_buf_.size() == units_buf_.capacity())
			return;

		std::vector<DictionaryUnit> units_buf(units_buf_);
		set_units_buf(&units_buf);
	}

private:
	const DictionaryUnit *units_;
	SizeType size_;
	std::vector<DictionaryUnit> units_buf_;

	// Disallows copies.
	Dictionary(const Dictionary &);
	Dictionary &operator=(const Dictionary &);

	// Traverses a dawg.
	bool Traverse(const CharType *key, BaseType *index,
		DictionaryUnit *unit) const
	{
		for ( ; *key != '\0'; ++key)
		{
			*index ^= unit->offset() ^ static_cast<UCharType>(*key);
			*unit = units_[*index];
			if (unit->label() != static_cast<UCharType>(*key))
				return false;
		}
		return true;
	}
	bool Traverse(const CharType *key, SizeType length,
		BaseType *index, DictionaryUnit *unit) const
	{
		for (SizeType i = 0; i < length; ++i)
		{
			*index ^= unit->offset() ^ static_cast<UCharType>(key[i]);
			*unit = units_[*index];
			if (unit->label() != static_cast<UCharType>(key[i]))
				return false;
		}
		return true;
	}

	// Swaps buffers for units.
	void set_units_buf(std::vector<DictionaryUnit> *units_buf)
	{
		units_ = &(*units_buf)[0];
		size_ = static_cast<BaseType>(units_buf->size());
		units_buf_.swap(*units_buf);
	}
};

}  // namespace dawgdic
}  // namespace nanika

#endif  // NANIKA_DAWGDIC_DICTIONARY_H
