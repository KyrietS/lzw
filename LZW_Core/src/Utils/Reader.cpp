#include "Reader.hpp"

#include <string>
#include <cstdint>
#include <exception>

Reader::Reader(const std::string& path)
	: bitReader(path) {}

uint64_t Reader::read(unsigned int bits)
{
	// Reading a number on 64 bits or more is probably a mistake.
	if (bits >= sizeof(uint64_t) * 8)
	{
		throw std::length_error("number of bits is too big and cannot be represented on a standard type");
	}

	uint64_t result{ 0 };
	uint64_t bit{ 0 };
	for (unsigned int i = 0; i < bits; i++)
	{
		notEnoughBits |= bitReader.eof();
		bit = (uint64_t)bitReader.read();
		result <<= 1;
		result = result | bit;
	}

	return result;
}

bool Reader::success()
{
	return notEnoughBits == false;
}

bool Reader::eof()
{
	return bitReader.eof();
}