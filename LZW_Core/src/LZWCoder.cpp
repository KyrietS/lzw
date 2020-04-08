//
// Copyright (c) 2020 Sebastian Fojcik
//

#include "LZWCoder.hpp"
#include "Statistics.hpp"
#include "Utils/Writer.hpp"
#include "Utils/Reader.hpp"

#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include <cmath>

using byte_t = unsigned char;

Statistics LZWCoder::encode(const std::string & path_in, const std::string &path_out)
{
	std::ifstream in(path_in, std::ifstream::binary);
	std::vector<uint64_t> inStats(256);
	Writer out(path_out);
	std::map<std::vector<byte_t>, uint64_t> dict;

	// Initialise dictionary with all bytes
	for (int byte = 0; byte <= 255; byte++)
		dict[{(byte_t)byte}] = byte;

	while (!in.eof())
	{
		// Find longest prefix
		std::vector<byte_t> prefix;
		uint64_t prefixCode{};
		while (in.peek() != EOF)
		{
			byte_t byte = in.peek();

			prefix.push_back(byte);
			if (dict.count(prefix) == 0) // if prefix doesn't belong to dictionary
				break;
			prefixCode = dict[prefix];

			in.get(); // read byte from stream
			inStats[byte]++;
		}
		// emit id of prefix on 'numOfBits' bits.
		unsigned int numOfBits = (unsigned int)ceil(log2(dict.size()));
		out.write(prefixCode, numOfBits);

		// add longer prefix to dictionary
		dict[prefix] = dict.size();
	}

	// Fill sats object and return
	return Statistics(inStats, std::vector<uint64_t>(out.stats.begin(), out.stats.end()));
}

void LZWCoder::decode(const std::string & path_in, const std::string & path_out)
{
	Reader in(path_in);
	std::ofstream out(path_out, std::ofstream::binary);
	std::vector<std::vector<byte_t>> dict;

	// Initialise dictionary with all bytes
	for (int byte = 0; byte <= 255; byte++)
		dict.push_back({ (byte_t)byte });

	// Read first byte
	uint64_t n = in.read(8);
	auto prefix = dict[n];
	out.put(prefix[0]);

	while (!in.eof())
	{
		unsigned int numOfBits = (unsigned int)ceil(log2(dict.size()+1));
		n = in.read(numOfBits);
		if (!in.success())
			break;

		if (n < dict.size()) // if n is contained in dictionary
		{
			prefix.push_back(dict[n][0]);
		}
		else // n is not contained in dictionary
		{
			prefix.push_back(prefix[0]);
		}
		dict.push_back(prefix);
		prefix = dict[n];

		// Emit prefix
		for (auto byte : prefix)
			out.put(byte);
	}
}
