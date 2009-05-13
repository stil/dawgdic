#ifndef NANIKA_DAWGDIC_DAWG_H
#define NANIKA_DAWGDIC_DAWG_H

#include "base-types.h"
#include "object-pool.h"

namespace nanika {
namespace dawgdic {

class Dawg
{
public:
	typedef ObjectPool<BaseType> BasePoolType;
	typedef ObjectPool<UCharType> LabelPoolType;

	Dawg() : state_pool_(), label_pool_(),
		num_of_states_(0), num_of_merged_states_(0) {}
	Dawg(BasePoolType *state_pool, LabelPoolType *label_pool,
		SizeType num_of_states, SizeType num_of_merged_states)
		: state_pool_(), label_pool_(), num_of_states_(num_of_states),
		num_of_merged_states_(num_of_merged_states)
	{
		state_pool_.Swap(state_pool);
		label_pool_.Swap(label_pool);
	}

	BaseType root() const { return 0; }

	// Number of fixed transitions.
	SizeType size() const { return state_pool_.size(); }
	// Number of transitions.
	SizeType num_of_transitions() const { return state_pool_.size(); }
	// Number of states.
	SizeType num_of_states() const { return num_of_states_; }
	// Number of merged states.
	SizeType num_of_merged_states() const { return num_of_merged_states_; }

	// Reads values.
	BaseType child(BaseType index) const
	{ return state_pool_[index] >> 2; }
	BaseType sibling(BaseType index) const
	{ return (state_pool_[index] & 1) ? (index + 1) : 0; }
	ValueType value(BaseType index) const
	{ return static_cast<ValueType>(state_pool_[index] >> 1); }
	bool is_leaf(BaseType index) const
	{ return label(index) == '\0'; }
	UCharType label(BaseType index) const
	{ return label_pool_[index]; }

	// Clears an object pool.
	void Clear()
	{
		state_pool_.Clear();
		label_pool_.Clear();
		num_of_states_ = 0;
		num_of_merged_states_ = 0;
	}

	// Swaps object pools.
	void Swap(Dawg *dawg)
	{
		state_pool_.Swap(&dawg->state_pool_);
		label_pool_.Swap(&dawg->label_pool_);
		std::swap(num_of_states_, dawg->num_of_states_);
		std::swap(num_of_merged_states_, dawg->num_of_merged_states_);
	}

private:
	BasePoolType state_pool_;
	LabelPoolType label_pool_;
	SizeType num_of_states_;
	SizeType num_of_merged_states_;
};

}  // namespace dawgdic
}  // namespace nanika

#endif  // NANIKA_DAWGDIC_DAWG_H
