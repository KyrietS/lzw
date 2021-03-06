#pragma once

#include <string>
#include <array>
#include <cstdint>
#include <fstream>

#include "BitReader.hpp"

// Reader that reads a value from given number of bits.
class Reader
{
public:
	std::array<uint64_t, 256> stats = {};

	Reader(const std::string& path);
	uint64_t read(unsigned int bits);

	// Checks if previous read operation was successful.
	bool success();
	// Checks if end of file was encountered.
	bool eof();
private:
	BitReader bitReader;
	bool notEnoughBits = false;
	BitBuffer statsBuffer;
};

