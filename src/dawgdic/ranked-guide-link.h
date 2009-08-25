#ifndef DAWGDIC_RANKED_GUIDE_LINK_H
#define DAWGDIC_RANKED_GUIDE_LINK_H

namespace dawgdic {

class RankedGuideLink
{
public:
	RankedGuideLink() : label_('\0'), value_(-1) {}
	RankedGuideLink(UCharType label, ValueType value)
		: label_(label), value_(value) {}

	void set_label(UCharType label) { label_ = label; }
	void set_value(ValueType value) { value_ = value; }

	UCharType label() const { return label_; }
	ValueType value() const { return value_; }

	// For sortings links in descending value order.
	class Comparer
	{
	public:
		bool operator()(const RankedGuideLink &lhs,
			const RankedGuideLink &rhs) const
		{
			if (lhs.value() != rhs.value())
				return lhs.value() > rhs.value();
			return lhs.label() < rhs.label();
		}
	};

private:
	UCharType label_;
	ValueType value_;

	// Copyable.
};

}  // namespace dawgdic

#endif  // DAWGDIC_RANKED_GUIDE_LINK_H
