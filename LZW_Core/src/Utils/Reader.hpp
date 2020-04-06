#pragma once

#include <string>
#include <cstdint>
#include <fstream>

#include "BitReader.hpp"

// Reader that reads a value from given number of bits.
class Reader
{
public:
	Reader(const std::string& path);
	uint64_t read(unsigned int bits);

	bool eof();
private:
	BitReader bitReader;
};

