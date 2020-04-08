//
// Copyright (c) 2020 Sebastian Fojcik
//

#include "LZWCoder.hpp"
#include "Statistics.hpp"
#include "VectorHash.hpp"
#include "Utils/Writer.hpp"
#include "Utils/Reader.hpp"
#include "Utils/FileSize.hpp"

#include <fstream>
#include <vector>
#include <iostream>
#include <cmath>
#include <unordered_map> // hash map used to be better than map for dictionary key

using byte_t = unsigned char;

Statistics LZWCoder::encode(const std::string & path_in, const std::string &path_out)
{
	// Standard implementation of hash function for std::string used to be better than
	// my own located in file "VectorHash.hpp". In case when the above is not true you
	// can use std::vector<byte_t> as dictionary key to increase encoding speed.

	//using DictKey = std::vector<byte_t>;
	using DictKey = std::string;

	std::ifstream in(path_in, std::ifstream::binary);
	std::vector<uint64_t> inStats(256);
	Writer out(path_out);
	std::unordered_map<DictKey, uint64_t> dict; 

	// for progress bar
	long filesize = getFileSize(path_in);
	uint64_t bytesRead = 0;

	// Initialise dictionary with all bytes
	for (int byte = 0; byte <= 255; byte++)
		dict[DictKey(1, byte)] = byte;

	while (!in.eof())
	{
		// Find longest prefix
		DictKey prefix;
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

			// update progress bar
			bytesRead++;
			updateProgress((double)(bytesRead) / filesize);
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

	// for progress bar
	long filesize = getFileSize(path_in);
	uint64_t bitsRead = 0;

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

		// update progress bar
		bitsRead += numOfBits;
		updateProgress((double)(bitsRead + 16) / filesize / 8);
	}
}

void LZWCoder::updateProgress(double progress)
{
	if (!printProgress || int(progress * 100.0) == currentProgress)
		return;

	currentProgress = int(progress * 100.0);

	int barWidth = 70;
	std::cout << "[";
	int pos = (int)((double)barWidth * progress);
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << currentProgress << " %\r";
	std::cout.flush();
}