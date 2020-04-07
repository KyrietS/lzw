#pragma once
#include <string>

class DictionaryCoder
{
public:
	virtual void encode(const std::string & path_in, const std::string & path_out) = 0;
	virtual void decode(const std::string & path_in, const std::string & path_out) = 0;
};
