#include "Writer.hpp"

#include <string>
#include <fstream>
#include <exception>
#include <cstdint>
#include <limits>
#include <cassert>

constexpr uint64_t powerOf(uint64_t a, uint64_t n)
{
	return n == 0 ? 1 : a * powerOf(a, n - 1);
}

Writer::Writer(const std::string& path)
	: bitWriter(path) {}

void Writer::write(uint64_t n, unsigned int bits)
{
	// Writing a number on 64 bits or more is probably a mistake.
	assert(bits < sizeof(uint64_t) * 8);

	// Check if it's possible to represent `n` on `bits` bits.
	uint64_t max = powerOf(2, (uint64_t)bits) - 1;
	if (n > max)
		throw std::length_error("number " + std::to_string(n) + 
			" cannot be represented on " + std::to_string(bits) + " bits");

	uint64_t mask = 1;
	mask <<= bits;
	for (unsigned int i = 0; i < bits; i++)
	{
		mask >>= 1;
		bool bit = n & mask;
		bitWriter.write(bit);
	}
}

void Writer::flush()
{
	bitWriter.flush();
}