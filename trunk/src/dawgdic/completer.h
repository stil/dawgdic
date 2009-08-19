#ifndef DAWGDIC_COMPLETER_H
#define DAWGDIC_COMPLETER_H

#include "dictionary.h"
#include "guide.h"

#include <vector>

namespace dawgdic {

class Completer
{
public:
	Completer() : dic_(NULL), guide_(NULL),
		key_(), index_stack_(), last_index_(0) {}
	Completer(const Dictionary &dic, const Guide &guide)
		: dic_(&dic), guide_(&guide), key_(), index_stack_(), last_index_(0) {}

	void set_dic(const Dictionary &dic) { dic_ = &dic; }
	void set_guide(const Guide &guide) { guide_ = &guide; }

	const Dictionary &dic() const { return *dic_; }
	const Guide &guide() const { return *guide_; }
	const char *key() const
	{
		return key_.empty() ? "" : reinterpret_cast<const char *>(&key_[0]);
	}
	SizeType length() const { return key_.empty() ? 0 : (key_.size() - 1); }
	ValueType value() const
	{
		return dic_->has_value(last_index_) ? dic_->value(last_index_) : -1;
	}

	// Starts completing keys from given index and prefix.
	void Start(BaseType index, const char *prefix = "")
	{
		SizeType length = 0;
		for (const char *p = prefix; *p != '\0'; ++p)
			++length;

		Start(index, prefix, length);
	}
	void Start(BaseType index, const char *prefix, SizeType length)
	{
		key_.resize(length + 1);
		for (SizeType i = 0; i < length; ++i)
			key_[i] = prefix[i];
		key_[length] = '\0';

		index_stack_.clear();
		index_stack_.push_back(index);
		last_index_ = dic_->root();
	}

	// Gets the next key.
	bool Next()
	{
		if (index_stack_.empty())
			return false;
		BaseType index = index_stack_.back();

		if (last_index_ != dic_->root())
		{
			UCharType child_label = guide_->child(index);
			if (child_label != '\0')
			{
				// Follows a transition to the first child.
//std::cerr << "Move to child: " << index << " -" << (int)child_label;
				if (!dic_->Follow(child_label, &index))
					return false;
//std::cerr << "-> " << index << std::endl;

				key_.back() = child_label;
				key_.push_back('\0');
				index_stack_.push_back(index);
			}
			else
			{
				for ( ; ; )
				{
					// Moves to the previous node.
					UCharType sibling_label = guide_->sibling(index);

					index_stack_.resize(index_stack_.size() - 1);
					index = index_stack_.back();

//std::cerr << "From: " << index << ": " << (int)sibling_label << std::endl;
					if (sibling_label != '\0')
					{
						// Follows a transition to the next sibling.
//std::cerr << "Move to sibling: " << index << " -" << (int)sibling_label;
						if (!dic_->Follow(sibling_label, &index))
							return false;
//std::cerr << "-> " << index << std::endl;

						key_[key_.size() - 2] = sibling_label;
						index_stack_.push_back(index);
						break;
					}
					key_[key_.size() - 2] = '\0';
					key_.resize(key_.size() - 1);

					if (index_stack_.empty())
						return false;
				}
			}
		}

		// Finds a terminal.
		while (!dic_->has_value(index))
		{
			UCharType label = guide_->child(index);
			if (!dic_->Follow(label, &index))
				return false;
//std::cerr << "Follow: " << index << std::endl;

			key_.back() = label;
			key_.push_back('\0');
			index_stack_.push_back(index);
		}
		last_index_ = index;

		return true;
	}

private:
	const Dictionary *dic_;
	const Guide *guide_;
	std::vector<UCharType> key_;
	std::vector<BaseType> index_stack_;
	BaseType last_index_;

	// Disallows copies.
	Completer(const Completer &);
	Completer &operator=(const Completer &);
};

}  // namespace dawgdic

#endif  // DAWGDIC_COMPLETER_H
