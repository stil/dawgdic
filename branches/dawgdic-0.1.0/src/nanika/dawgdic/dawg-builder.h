#ifndef NANIKA_DAWGDIC_DAWG_BUILDER_H
#define NANIKA_DAWGDIC_DAWG_BUILDER_H

#include <algorithm>
#include <vector>

#include "dawg.h"

namespace nanika {
namespace dawgdic {

// DAWG builder.
class DawgBuilder
{
public:
	typedef std::pair<BaseType, BaseType> PairType;
	typedef ObjectPool<PairType> PairPoolType;
	typedef ObjectPool<UCharType> LabelPoolType;

	enum OrderType { UNFIXED, ASCENDING, DESCENDING };

	enum { DEFAULT_INITIAL_HASH_TABLE_SIZE = 1 << 8 };

	explicit DawgBuilder(SizeType initial_hash_table_size =
		DEFAULT_INITIAL_HASH_TABLE_SIZE)
		: initial_hash_table_size_(initial_hash_table_size),
		order_(UNFIXED), state_pool_(), label_pool_(), hash_table_(),
		unfixed_states_(), unused_states_(), num_of_merged_states_(0) {}

	// Gets the size of a state pool.
	BaseType size() const
	{ return static_cast<BaseType>(state_pool_.size()); }
	// Gets the number of states.
	BaseType num_of_states() const
	{ return static_cast<BaseType>(size() - unused_states_.size()); }
	// Gets the number of merged states.
	SizeType num_of_merged_states() const
	{ return num_of_merged_states_; }

	// Clears pools.
	void Clear()
	{
		order_ = UNFIXED;
		state_pool_.Clear();
		label_pool_.Clear();
		std::vector<BaseType>(0).swap(hash_table_);
		std::vector<BaseType>(0).swap(unfixed_states_);
		std::vector<BaseType>(0).swap(unused_states_);
		num_of_merged_states_ = 0;
	}

	// Inserts a key.
	bool Insert(const CharType *key, ValueType value = 0)
	{
		SizeType length = 0;
		while (key[length])
			++length;
		return Insert(key, length, value);
	}

	// Inserts a key.
	bool Insert(const CharType *key, SizeType length, ValueType value)
	{
		if (value < 0 || length <= 0)
			return false;

		// Initializes a builder.
		if (hash_table_.empty())
			Init();

		BaseType index = 0;
		BaseType key_pos = 0;

		// Finds a separate state.
		for ( ; key_pos <= length; ++key_pos)
		{
			BaseType child_index = child(index);
			if (!child_index)
				break;

			UCharType key_label = static_cast<UCharType>(
				(key_pos < length) ? key[key_pos] : '\0');

			// Checks the order of keys.
			if (key_label != label(child_index))
			{
				OrderType key_order = key_label > label(child_index)
					? ASCENDING : DESCENDING;
				if (order_ == UNFIXED)
					order_ = key_order;

				if (key_order != order_)
					return false;

				MergeStates(index);
				break;
			}

			index = child_index;
		}

		// Adds new states.
		for ( ; key_pos <= length; ++key_pos)
		{
			UCharType key_label = static_cast<UCharType>(
				(key_pos < length) ? key[key_pos] : '\0');
			BaseType child_index = Allocate();

			set_sibling(child_index, child(index));
			set_label(child_index, key_label);
			unfixed_states_.push_back(child_index);
			set_child(index, child_index);
			if (key_label == '\0')
				set_has_leaf(index);
			index = child_index;
		}
		set_value(index, value);
		return true;
	}

	// Finishes building a dawg and returns the number of merged states.
	bool Finish(Dawg *dawg, SizeType *num_of_merged_states = NULL)
	{
		// Initializes a builder.
		if (hash_table_.empty())
			Init();

		MergeStates(0);
		if (num_of_merged_states != NULL)
			*num_of_merged_states = num_of_merged_states_;
		Dawg(&state_pool_, &label_pool_, num_of_states()).Swap(dawg);
		Clear();
		return true;
	}

private:
	SizeType initial_hash_table_size_;
	OrderType order_;
	PairPoolType state_pool_;
	LabelPoolType label_pool_;
	std::vector<BaseType> hash_table_;
	std::vector<BaseType> unfixed_states_;
	std::vector<BaseType> unused_states_;
	SizeType num_of_merged_states_;

	// Disallows copies.
	DawgBuilder(const DawgBuilder &);
	DawgBuilder &operator=(const DawgBuilder &);

	// Writes values.
	void set_has_leaf(BaseType index)
	{ state_pool_[index].second |= 1; }
	void set_child(BaseType index, BaseType child)
	{ state_pool_[index].first = child; }
	void set_sibling(BaseType index, BaseType sibling)
	{
		state_pool_[index].second &= 1;
		state_pool_[index].second |= sibling << 1;
	}
	void set_value(BaseType index, ValueType value)
	{ state_pool_[index].first = value; }
	void set_label(BaseType index, UCharType label)
	{ label_pool_[index] = label; }

	// Reads values.
	BaseType child(BaseType index) const
	{ return state_pool_[index].first; }
	BaseType sibling(BaseType index) const
	{ return state_pool_[index].second >> 1; }
	ValueType value(BaseType index) const
	{ return static_cast<ValueType>(state_pool_[index].first); }
	UCharType label(BaseType index) const
	{ return label_pool_[index]; }

	// Initializes an object.
	void Init()
	{
		hash_table_.resize(initial_hash_table_size_, 0);
		Allocate();
		set_label(0, 0xFF);
		unfixed_states_.push_back(0);
	}

	// Merges common states recursively.
	void MergeStates(BaseType index)
	{
		while (unfixed_states_.back() != index)
		{
			BaseType unfixed_index = unfixed_states_.back();

			if (size() >= hash_table_.size() - (hash_table_.size() >> 2))
				ExpandHashTable();

			SizeType hash_id;
			BaseType matched_index = FindState(unfixed_index, &hash_id);
			if (matched_index)
			{
				Free(unfixed_index);
				unfixed_index = matched_index;
				++num_of_merged_states_;
			}
			else
				hash_table_[hash_id] = unfixed_index;

			unfixed_states_.resize(unfixed_states_.size() - 1);
			set_child(unfixed_states_.back(), unfixed_index);
		}
	}

	// Finds a state from a hash table.
	BaseType FindState(BaseType index, SizeType *hash_id) const
	{
		const PairType &state = state_pool_[index];
		UCharType label = label_pool_[index];

		*hash_id = Hash(state.first, state.second, label) % hash_table_.size();
		for ( ; ; *hash_id = (*hash_id + 1) % hash_table_.size())
		{
			BaseType state_id = hash_table_[*hash_id];
			if (!state_id)
				break;

			if (state == state_pool_[state_id]
				&& label == label_pool_[state_id])
				return state_id;
		}

		return 0;
	}

	// Expands a hash table.
	void ExpandHashTable()
	{
		std::vector<BaseType> free_states(unfixed_states_);
		free_states.insert(free_states.end(),
			unused_states_.begin(), unused_states_.end());
		std::sort(free_states.begin(), free_states.end());

		SizeType hash_table_size = hash_table_.size() << 1;
		std::vector<BaseType>(0).swap(hash_table_);
		hash_table_.resize(hash_table_size, 0);

		// Inserts states into a new hash table.
		BaseType state_id = 0;
		for (SizeType i = 0; i < free_states.size(); ++i, ++state_id)
		{
			for ( ; state_id < free_states[i]; ++state_id)
			{
				SizeType hash_id;
				FindState(state_id, &hash_id);
				hash_table_[hash_id] = state_id;
			}
		}
		for ( ; state_id < size(); ++state_id)
		{
			SizeType hash_id;
			FindState(state_id, &hash_id);
			hash_table_[hash_id] = state_id;
		}
	}

	// Hash function.
	static BaseType Hash(BaseType a, BaseType b, BaseType c)
	{
		a = a - b; a = a - c; a = a ^ (c >> 13);
		b = b - c; b = b - a; b = b ^ (a << 8);
		c = c - a; c = c - b; c = c ^ (b >> 13);
		a = a - b; a = a - c; a = a ^ (c >> 12);
		b = b - c; b = b - a; b = b ^ (a << 16);
		c = c - a; c = c - b; c = c ^ (b >> 5);
		a = a - b; a = a - c; a = a ^ (c >> 3);
		b = b - c; b = b - a; b = b ^ (a << 10);
		c = c - a; c = c - b; c = c ^ (b >> 15);

		return c;
	}

	// Gets an object from an object pool.
	BaseType Allocate()
	{
		BaseType index = 0;
		if (unused_states_.empty())
		{
			index = state_pool_.Allocate();
			label_pool_.Allocate();
		}
		else
		{
			index = unused_states_.back();
			unused_states_.resize(unused_states_.size() - 1);
		}
		state_pool_[index] = PairType(0, 0);
		return index;
	}
	// Puts an object into an object pool.
	void Free(BaseType index) { unused_states_.push_back(index); }
};

}  // namespace dawgdic
}  // namespace nanika

#endif  // NANIKA_DAWGDIC_DAWG_BUILDER_H
