#pragma once
#include <string>
#include "Statistics.hpp"

class DictionaryCoder
{
public:
	virtual Statistics encode(const std::string & path_in, const std::string & path_out) = 0;
	virtual void decode(const std::string & path_in, const std::string & path_out) = 0;
};
