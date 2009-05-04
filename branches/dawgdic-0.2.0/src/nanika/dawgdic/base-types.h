#ifndef NANIKA_DAWGDIC_BASE_TYPES_H
#define NANIKA_DAWGDIC_BASE_TYPES_H

#include <cstddef>

namespace nanika {
namespace dawgdic {

// 8-bit characters.
typedef char CharType;
typedef unsigned char UCharType;

// 32-bit integer.
typedef int ValueType;

// 32-bit unsigned integer.
typedef unsigned int BaseType;

// 32 or 64-bit unsigned integer.
typedef std::size_t SizeType;

}  // namespace dawgdic
}  // namespace nanika

#endif  // NANIKA_DAWGDIC_BASE_TYPES_H
