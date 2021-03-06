#pragma once
#include "DictionaryCoder.hpp"

#include <string>

class LZWCoder : public DictionaryCoder
{
public:
	LZWCoder(bool printProgress = false)
		: printProgress(printProgress) {}

	Statistics encode(const std::string & path_in, const std::string & path_out) override;
	void decode(const std::string & path_in, const std::string & path_out) override;
private:
	bool printProgress;
	int currentProgress = 0;
	const int progressbarWidth = 70;

	inline void updateProgress(double progress);
	inline void clearProgress();
};
