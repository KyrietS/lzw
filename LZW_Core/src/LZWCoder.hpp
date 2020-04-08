#pragma once
#include "DictionaryCoder.hpp"

#include <string>

class LZWCoder : public DictionaryCoder
{
public:
	Statistics encode(const std::string & path_in, const std::string & path_out) override;
	void decode(const std::string & path_in, const std::string & path_out) override;
};
