#ifndef DAWGDIC_RANKED_COMPLETER_CANDIDATE_H
#define DAWGDIC_RANKED_COMPLETER_CANDIDATE_H

#include "base-types.h"

namespace dawgdic {

class RankedCompleterCandidate
{
public:
	RankedCompleterCandidate() : node_index_(0), value_(-1) {}

	void set_node_index(BaseType node_index) { node_index_ = node_index; }
	void set_value(ValueType value) { value_ = value; }

	BaseType node_index() const { return node_index_; }
	ValueType value() const { return value_; }

	class Comparer
	{
	public:
		bool operator()(const RankedCompleterCandidate &lhs,
			const RankedCompleterCandidate &rhs) const
		{
			if (lhs.value() != rhs.value())
				return lhs.value() < rhs.value();
			return lhs.node_index() > rhs.node_index();
		}
	};

private:
	BaseType node_index_;
	ValueType value_;

	// Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_RANKED_COMPLETER_CANDIDATE_H
