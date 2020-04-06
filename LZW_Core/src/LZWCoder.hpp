#pragma once
#include "DictionaryCoder.hpp"

#include <string>

class LZWCoder : public DictionaryCoder
{
public:
	void encode(std::string path_in, std::string path_out) override;
	void decode(std::string path_in, std::string path_out) override;
};
