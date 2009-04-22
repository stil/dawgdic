#ifndef NANIKA_DAWGDIC_DAWG_H
#define NANIKA_DAWGDIC_DAWG_H

#include "base-types.h"
#include "object-pool.h"

namespace nanika {
namespace dawgdic {

class Dawg
{
public:
	typedef std::pair<BaseType, BaseType> PairType;
	typedef ObjectPool<PairType> PairPoolType;
	typedef ObjectPool<UCharType> LabelPoolType;

	Dawg() : state_pool_(), label_pool_(), size_(0) {}
	Dawg(PairPoolType *state_pool, LabelPoolType *label_pool, BaseType size)
		: state_pool_(), label_pool_(), size_(size)
	{
		state_pool_.Swap(state_pool);
		label_pool_.Swap(label_pool);
	}

	BaseType size() const { return size_; }
	BaseType capacity() const { return state_pool_.size(); }

	// Reads values.
	bool has_leaf(BaseType index) const
	{ return (state_pool_[index].second & 1) ? true : false; }
	BaseType child(BaseType index) const
	{ return state_pool_[index].first; }
	BaseType sibling(BaseType index) const
	{ return state_pool_[index].second >> 1; }
	ValueType value(BaseType index) const
	{ return static_cast<ValueType>(state_pool_[index].first); }
	bool is_leaf(BaseType index) const
	{ return label(index) == '\0'; }
	UCharType label(BaseType index) const
	{ return label_pool_[index]; }

	// Clears an object.
	void Clear()
	{
		state_pool_.Clear();
		label_pool_.Clear();
		size_ = 0;
	}
	// Swaps objects.
	void Swap(Dawg *dawg)
	{
		state_pool_.Swap(&dawg->state_pool_);
		label_pool_.Swap(&dawg->label_pool_);
		std::swap(size_, dawg->size_);
	}

private:
	PairPoolType state_pool_;
	LabelPoolType label_pool_;
	BaseType size_;
};

}  // namespace dawgdic
}  // namespace nanika

#endif  // NANIKA_DAWGDIC_DAWG_H
