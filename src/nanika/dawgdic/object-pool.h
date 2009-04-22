#ifndef NANIKA_DAWGDIC_OBJECT_POOL_H
#define NANIKA_DAWGDIC_OBJECT_POOL_H

#include <vector>

#include "base-types.h"

namespace nanika {
namespace dawgdic {

// Object pool.
template <typename OBJECT_TYPE, BaseType BLOCK_SIZE = 1 << 10>
class ObjectPool
{
public:
	typedef OBJECT_TYPE ObjectType;

	ObjectPool() : blocks_(), size_(0) {}
	~ObjectPool() { Clear(); }

	// The number of objects.
	BaseType size() const { return size_; }

	// Accessors.
	ObjectType &operator[](BaseType index)
	{ return blocks_[index / BLOCK_SIZE][index % BLOCK_SIZE]; }
	const ObjectType &operator[](BaseType index) const
	{ return blocks_[index / BLOCK_SIZE][index % BLOCK_SIZE]; }

	// Deletes all blocks.
	void Clear()
	{
		for (SizeType i = 0; i < blocks_.size(); ++i)
			delete [] blocks_[i];

		std::vector<ObjectType *>(0).swap(blocks_);
		size_ = 0;
	}
	// Swaps objects.
	void Swap(ObjectPool *pool)
	{
		blocks_.swap(pool->blocks_);
		std::swap(size_, pool->size_);
	}

	// Gets the ID of a new object.
	BaseType Allocate()
	{
		if (size_ == static_cast<BaseType>(BLOCK_SIZE * blocks_.size()))
			blocks_.push_back(new ObjectType[BLOCK_SIZE]);
		return size_++;
	}

private:
	std::vector<ObjectType *> blocks_;
	BaseType size_;

	// Disallows copies.
	ObjectPool(const ObjectPool &);
	ObjectPool &operator=(const ObjectPool &);
};

}  // namespace dawgdic
}  // namespace nanika

#endif  // NANIKA_DAWGDIC_OBJECT_POOL_H
