#ifndef NANIKA_IOS_LINE_READER_H
#define NANIKA_IOS_LINE_READER_H

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace nanika {
namespace ios {

// For reading lines from an input stream.
class LineReader
{
public:
	// Lines must be shorter than this size.
	enum { DEFAULT_BLOCK_SIZE = 1 << 12 };

	explicit LineReader(std::istream *stream,
		std::size_t block_size = DEFAULT_BLOCK_SIZE)
		: stream_(stream), block_size_(block_size), buf_(), pos_(0) {}

	// Reads a line.
	bool Read(std::string *line)
	{
		const char *start;
		std::size_t length;
		if (!Read(&start, &length))
			return false;
		line->assign(start, length);
		return true;
	}

	// Reads a line (this buffer will be broken by the next call).
	bool Read(const char **line, std::size_t *length = NULL)
	{
		size_t start = pos_;
		while (pos_ < buf_.size() && buf_[pos_] != '\n')
			++pos_;
		if (pos_ == buf_.size())
		{
			if (!Fill(&start))
				return false;

			while (pos_ < buf_.size() && buf_[pos_] != '\n')
				++pos_;
		}
		if (buf_[pos_] != '\n')
			return false;

		*line = &buf_[start];
		if (length != NULL)
			*length = pos_ - start;
		buf_[pos_] = '\0';
		++pos_;
		return true;
	}

private:
	std::istream *stream_;
	std::size_t block_size_;
	std::vector<char> buf_;
	std::size_t pos_;

	// Copies are not allowed.
	LineReader(const LineReader &);
	LineReader &operator=(const LineReader &);

private:
	// Fills an internal buffer.
	bool Fill(std::size_t *start = 0)
	{
		if (!*stream_)
			return false;

		std::size_t avail = (buf_.size() - (start ? *start : 0));
		if (!buf_.empty() && avail)
			std::memmove(&buf_[0], &buf_[(start ? *start : 0)], avail);

		if (start)
		{
			pos_ -= *start;
			*start = 0;
		}

		buf_.resize(block_size_);
		stream_->read(&buf_[avail], block_size_ - avail);
		buf_.resize(avail + stream_->gcount());
		return true;
	}
};

}  // namespace ios
}  // namespace nanika

#endif  // NANIKA_IOS_LINE_READER_H
