#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>

class Statistics
{
public:
	Statistics(std::vector<uint64_t> in, std::vector<uint64_t> out)
		: in(in), out(out) 
	{
		if (in.size() != 256 || out.size() != 256)
			throw std::invalid_argument("Invalid vector size. Must be 256!");

		totalInputBits = 0;
		totalOutputBits = 0;
		for (int i = 0; i < 256; i++)
		{
			totalInputBits += in[i];
			totalOutputBits += out[i];
		}
	}

	double outputEntropy()
	{
		double h = 0;
		for (size_t i = 0; i < 256; i++)
		{
			double p = (double)out[i] / totalOutputBits;
			if (p > 0)
				h += out[i] * -log2(p);
		}
		return h / totalOutputBits;
	}

	double inputEntropy()
	{
		double h = 0;
		for (size_t i = 0; i < 256; i++)
		{
			double p = (double)in[i] / totalInputBits;
			if (p > 0)
				h += in[i] * -log2(p);
		}
		return h / totalInputBits;
	}

	double compressionRatio()
	{
		return (1.0 - (double)totalOutputBits / totalInputBits);
	}

private:
	std::vector<uint64_t> in;
	std::vector<uint64_t> out;
	uint64_t totalInputBits;
	uint64_t totalOutputBits;
};