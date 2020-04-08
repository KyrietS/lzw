#pragma once

#include <string>
#include <array>
#include <cstdint>
#include <fstream>

#include "BitWriter.hpp"

// Writer that writes to a file any value on a given number of bits.
class Writer
{
public:
	std::array<uint64_t, 256> stats = {};

	Writer(const std::string& path);
	void write(uint64_t n, unsigned int bits);

	void flush();
private:
	BitWriter bitWriter;
	BitBuffer statsBuffer;
};

