#pragma once
#include "DictionaryCoder.hpp"

#include <string>
#include <vector>

class LZWCoder : public DictionaryCoder
{
public:
	void encode(const std::string & path_in, const std::string & path_out) override;
	void decode(const std::string & path_in, const std::string & path_out) override;
};
