#ifndef NANIKA_DAWGDIC_EXPLORER_H
#define NANIKA_DAWGDIC_EXPLORER_H

#include "dictionary.h"

namespace nanika {
namespace dawgdic {

// Class for exploring a dawg dictionary.
class DictionaryExplorer
{
public:
	DictionaryExplorer(const Dictionary &dic)
		: units_(dic.units()), index_(0) {}

	// Initializes a matching position.
	void Reset() { index_ = 0; }
	void Reset(const Dictionary &dic) { units_ = dic.units(); Reset(); }

	// Follows a transition.
	bool Follow(CharType label)
	{
		BaseType next_index = index_ ^ units_[index_].offset()
			^ static_cast<UCharType>(label);
		if (units_[next_index].label() != static_cast<UCharType>(label))
			return false;
		index_ = next_index;
		return true;
	}

	// Follows transitions.
	bool Follow(const CharType *s)
	{
		while (*s != '\0' && Follow(*s))
			++s;
		return *s == '\0';
	}
	bool Follow(const CharType *s, SizeType *count)
	{
		while (*s != '\0' && Follow(*s))
			++s, ++*count;
		return *s == '\0';
	}

	// Follows transitions.
	bool Follow(const CharType *s, SizeType length)
	{
		for (SizeType i = 0; i < length; ++i)
		{
			if (!Follow(s[i]))
				return false;
		}
		return true;
	}
	bool Follow(const CharType *s, SizeType length, SizeType *count)
	{
		for (SizeType i = 0; i < length; ++i, ++*count)
		{
			if (!Follow(s[i]))
				return false;
		}
		return true;
	}

	// Checks if a current state is related to the end of a key.
	bool has_value() const { return units_[index_].has_leaf(); }
	// Gets a value from a corresponding leaf.
	ValueType value() const
	{ return units_[index_ ^ units_[index_].offset()].value(); }

private:
	const DictionaryUnit *units_;
	BaseType index_;
};

}  // namespace dawgdic
}  // namespace nanika

#endif  // NANIKA_DAWGDIC_EXPLORER_H
