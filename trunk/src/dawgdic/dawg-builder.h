#ifndef DAWGDIC_DAWG_BUILDER_H
#define DAWGDIC_DAWG_BUILDER_H

#include <algorithm>
#include <stack>
#include <vector>

#include "dawg.h"
#include "dawg-unit.h"

namespace dawgdic {

// DAWG builder.
class DawgBuilder
{
public:
	typedef ObjectPool<BaseType> BasePoolType;
	typedef ObjectPool<UCharType> LabelPoolType;
	typedef ObjectPool<DawgUnit> UnitPoolType;

	enum { DEFAULT_INITIAL_HASH_TABLE_SIZE = 1 << 8 };

	explicit DawgBuilder(SizeType initial_hash_table_size =
		DEFAULT_INITIAL_HASH_TABLE_SIZE)
		: initial_hash_table_size_(initial_hash_table_size),
		state_pool_(), label_pool_(), unit_pool_(), hash_table_(),
		unfixed_units_(), unused_units_(),
		num_of_states_(0), num_of_merged_states_(0) {}

	// Number of fixed transitions.
	SizeType size() const { return state_pool_.size(); }
	// Number of transitions.
	SizeType num_of_transitions() const { return state_pool_.size(); }
	// Number of states.
	SizeType num_of_states() const { return num_of_states_; }
	// Number of merged states.
	SizeType num_of_merged_states() const { return num_of_merged_states_; }

	// Initializes a builder.
	void Clear()
	{
		state_pool_.Clear();
		label_pool_.Clear();
		unit_pool_.Clear();
		std::vector<BaseType>(0).swap(hash_table_);
		while (!unfixed_units_.empty())
			unfixed_units_.pop();
		while (!unused_units_.empty())
			unused_units_.pop();
		num_of_states_ = 0;
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

		// Initializes a builder if not initialized.
		if (hash_table_.empty())
			Init();

		BaseType index = 0;
		SizeType key_pos = 0;

		// Finds a separate state.
		for ( ; key_pos <= length; ++key_pos)
		{
			BaseType child_index = unit_pool_[index].child();
			if (!child_index)
				break;

			UCharType key_label = static_cast<UCharType>(
				(key_pos < length) ? key[key_pos] : '\0');
			UCharType unit_label = unit_pool_[child_index].label();

			// Checks the order of keys.
			if (key_label < unit_label)
				return false;
			else if (key_label > unit_label)
			{
				unit_pool_[child_index].set_has_sibling(true);
				FixStates(child_index);
				break;
			}

			index = child_index;
		}

		// Adds new states.
		for ( ; key_pos <= length; ++key_pos)
		{
			UCharType key_label = static_cast<UCharType>(
				(key_pos < length) ? key[key_pos] : '\0');
			BaseType child_index = AllocateUnit();

			if (!unit_pool_[index].child())
				unit_pool_[child_index].set_is_state(true);
			unit_pool_[child_index].set_sibling(unit_pool_[index].child());
			unit_pool_[child_index].set_label(key_label);
			unit_pool_[index].set_child(child_index);
			unfixed_units_.push(child_index);

			index = child_index;
		}
		unit_pool_[index].set_value(value);
		return true;
	}

	// Finishes building a dawg.
	bool Finish(Dawg *dawg)
	{
		// Initializes a builder if not initialized.
		if (hash_table_.empty())
			Init();

		FixStates(0);
		state_pool_[0] = unit_pool_[0].base();
		label_pool_[0] = unit_pool_[0].label();

		Dawg(&state_pool_, &label_pool_,
			num_of_states_, num_of_merged_states_).Swap(dawg);
		Clear();
		return true;
	}

private:
	const SizeType initial_hash_table_size_;
	BasePoolType state_pool_;
	LabelPoolType label_pool_;
	UnitPoolType unit_pool_;
	std::vector<BaseType> hash_table_;
	std::stack<BaseType> unfixed_units_;
	std::stack<BaseType> unused_units_;
	SizeType num_of_states_;
	SizeType num_of_merged_states_;

	// Disallows copies.
	DawgBuilder(const DawgBuilder &);
	DawgBuilder &operator=(const DawgBuilder &);

	// Initializes an object.
	void Init()
	{
		hash_table_.resize(initial_hash_table_size_, 0);
		AllocateUnit();
		AllocateState();
		unit_pool_[0].set_label(0xFF);
		unfixed_units_.push(0);
	}

	// Fixes states corresponding to the last inserted key.
	// Also, merges equivalent states.
	void FixStates(BaseType index)
	{
		while (unfixed_units_.top() != index)
		{
			BaseType unfixed_index = unfixed_units_.top();
			unfixed_units_.pop();

			if (num_of_states_ >= hash_table_.size()
				- (hash_table_.size() >> 2))
				ExpandHashTable();

			BaseType num_of_siblings = 0;
			for (BaseType i = unfixed_index; i; i = unit_pool_[i].sibling())
				++num_of_siblings;

			BaseType hash_id;
			BaseType matched_index = FindUnit(unfixed_index, &hash_id);
			if (matched_index)
				num_of_merged_states_ += num_of_siblings;
			else
			{
				// Fixes units.
				// A fixed unit is converted to a pair of a state and a label.
				BaseType state_index = 0;
				for (BaseType i = 0; i < num_of_siblings; ++i)
					state_index = AllocateState();
				for (BaseType i = unfixed_index; i != 0;
					i = unit_pool_[i].sibling())
				{
					state_pool_[state_index] = unit_pool_[i].base();
					label_pool_[state_index] = unit_pool_[i].label();
					--state_index;
				}
				matched_index = state_index + 1;
				hash_table_[hash_id] = matched_index;
				++num_of_states_;
			}

			// Deletes fixed units.
			for (BaseType current = unfixed_index, next; current;
				current = next)
			{
				next = unit_pool_[current].sibling();
				FreeUnit(current);
			}

			unit_pool_[unfixed_units_.top()].set_child(matched_index);
		}
		unfixed_units_.pop();
	}

	// Expands a hash table.
	void ExpandHashTable()
	{
		SizeType hash_table_size = hash_table_.size() << 1;
		std::vector<BaseType>(0).swap(hash_table_);
		hash_table_.resize(hash_table_size, 0);

		// Builds a new hash table.
		BaseType count = 0;
		for (SizeType i = 1; i < state_pool_.size(); ++i)
		{
			BaseType index = static_cast<BaseType>(i);
			if (label_pool_[index] == '\0' || state_pool_[index] & 2)
			{
				BaseType hash_id;
				FindState(index, &hash_id);
				hash_table_[hash_id] = index;
				++count;
			}
		}
	}

	// Finds a state from a hash table.
	BaseType FindState(BaseType state_index, BaseType *hash_id) const
	{
		*hash_id = HashState(state_index) % hash_table_.size();
		for ( ; ; *hash_id = (*hash_id + 1) % hash_table_.size())
		{
			BaseType state_id = hash_table_[*hash_id];
			if (!state_id)
				break;

			// There must not be equivalent states.
		}
		return 0;
	}

	// Finds a unit from a hash table.
	BaseType FindUnit(BaseType unit_index, BaseType *hash_id) const
	{
		*hash_id = HashUnit(unit_index) % hash_table_.size();
		for ( ; ; *hash_id = (*hash_id + 1) % hash_table_.size())
		{
			BaseType state_id = hash_table_[*hash_id];
			if (!state_id)
				break;

			if (AreEqual(unit_index, state_id))
				return state_id;
		}
		return 0;
	}

	// Compares a unit and a state.
	bool AreEqual(BaseType unit_index, BaseType state_index) const
	{
		// Compares the numbers of transitions.
		for (BaseType i = unit_pool_[unit_index].sibling(); i;
			i = unit_pool_[i].sibling())
		{
			if (!(state_pool_[state_index] & 1))
				return false;
			++state_index;
		}
		if (state_pool_[state_index] & 1)
			return false;

		// Compares out-transitions.
		for (BaseType i = unit_index; i;
			i = unit_pool_[i].sibling(), --state_index)
		{
			if (unit_pool_[i].base() != state_pool_[state_index] ||
				unit_pool_[i].label() != label_pool_[state_index])
				return false;
		}
		return true;
	}

	// Calculates a hash value from a state.
	BaseType HashState(BaseType state_index) const
	{
		BaseType hash_value = 0;
		for ( ; state_index; state_index = state_pool_[state_index] & 1 ?
			state_index + 1 : 0)
		{
			BaseType base = state_pool_[state_index];
			UCharType label = label_pool_[state_index];
			hash_value ^= Hash((label << 24) ^ base);
		}
		return hash_value;
	}

	// Calculates a hash value from a unit.
	BaseType HashUnit(BaseType unit_index) const
	{
		BaseType hash_value = 0;
		for ( ; unit_index; unit_index = unit_pool_[unit_index].sibling())
		{
			BaseType base = unit_pool_[unit_index].base();
			UCharType label = unit_pool_[unit_index].label();
			hash_value ^= Hash((label << 24) ^ base);
		}
		return hash_value;
	}

	// 32-bit mix function.
	// http://www.concentric.net/~Ttwang/tech/inthash.htm
	static BaseType Hash(BaseType key)
	{
		key = ~key + (key << 15);  // key = (key << 15) - key - 1;
		key = key ^ (key >> 12);
		key = key + (key << 2);
		key = key ^ (key >> 4);
		key = key * 2057;  // key = (key + (key << 3)) + (key << 11);
		key = key ^ (key >> 16);
		return key;
	}

	// Gets a state object from an object pool.
	BaseType AllocateState()
	{
		state_pool_.Allocate();
		return static_cast<BaseType>(label_pool_.Allocate());
	}

	// Gets a unit object from an object pool.
	BaseType AllocateUnit()
	{
		BaseType index = 0;
		if (unused_units_.empty())
			index = static_cast<BaseType>(unit_pool_.Allocate());
		else
		{
			index = unused_units_.top();
			unused_units_.pop();
		}
		unit_pool_[index].Clear();
		return index;
	}

	// Returns a unit object to an object pool.
	void FreeUnit(BaseType index) { unused_units_.push(index); }
};

}  // namespace dawgdic

#endif  // DAWGDIC_DAWG_BUILDER_H
